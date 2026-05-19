#include "common.h"
#include "protocol.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

//consiglio sull'utilizzo del client
static void print_usage(const char* program_name) {
    fprintf(stderr, "Uso: %s [ip_server] [porta]\n", program_name);
}

//crea la socket del client e si collega al server
static int create_client_socket(const char* server_ip, int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); //socket

    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    //struttura indirizzo del server
    struct sockaddr_in server_addr;

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons((uint16_t)port);

    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        fprintf(stderr, "Indirizzo IP non valido: %s\n", server_ip);
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    //prova a collegarsi al server, va a buon fine solo se il server è già attivo
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    return sockfd;
}

static void print_commands(void) {
    printf("\nComandi disponibili: (ricorda di scriverli in maiuscolo)\n");
    printf("  ADD <id> <importo> <causale>\n");
    printf("  DELETE <id>\n");
    printf("  UPDATE <id> <nuovo_importo> <nuova_causale>\n");
    printf("  LIST\n");
    printf("  HELP\n");
    printf("  QUIT\n\n");
}

//controlla se il comando inviato al server produce una risposta a più righe, deve capire se basta una sola recv_line o leggere finchè non incontra END
static int command_expects_multiline_response(const char* line) {
    char command[16];
    int pos = 0;

    if (sscanf(line, "%15s %n", command, &pos) != 1) {
        return 0;
    }

    if (line[pos] != '\0') {
        return 0;
    }

    return strcmp(command, "LIST") == 0 || strcmp(command, "HELP") == 0;
}

//controlla se il comando è QUIT
static int is_quit_command(const char* line) {
    char command[16];
    int pos = 0;

    if (sscanf(line, "%15s %n", command, &pos) != 1) {
        return 0;
    }

    if (line[pos] != '\0') {
        return 0;
    }

    return strcmp(command, "QUIT") == 0;
}

int main(int argc, char* argv[]) {
    //valori di default
    const char* server_ip = "127.0.0.1";
    int port = SERVER_PORT;
    
    //buffer per leggere i comandi dell'utente e per ricevere le risposte dal server
    char line[LINE_SIZE];

    if (argc == 2) {
        server_ip = argv[1];
    } else if (argc == 3) {
        server_ip = argv[1];
        port = atoi(argv[2]);
    } else if (argc > 3) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Porta non valida.\n");
        return EXIT_FAILURE;
    }

    //connessione al server
    int sockfd = create_client_socket(server_ip, port);

    ssize_t n;

    while (1) {
        n = recv_line(sockfd, line, sizeof(line));

        if (n <= 0) {
            printf("Connessione chiusa dal server.\n");
            close(sockfd);
            return EXIT_FAILURE;
        }

        printf("%s\n", line);

        //Se il server è pieno, il client mostra il messaggio e resta in attesa della conferma di connessione.
        
        if (strncmp(line, "WAIT", 4) != 0) {
            break;
        }
    }


    //stampa i comandi disponibili
    print_commands();

    while (1) {
        printf("> ");
        fflush(stdout);

        //legge una riga dell'utente e la salva dentro line, se restituisce NULL c'è stato un errore
        if (fgets(line, sizeof(line), stdin) == NULL) {
            break;
        }

        //sostituisce \n con \0
        line[strcspn(line, "\n")] = '\0';

        //ignora le righe vuote (se l'utente clicca invio il server non riceve nulla)
        if (strlen(line) == 0) {
            continue;
        }
        
        //capisce se è multilinea o quit
        int multiline = command_expects_multiline_response(line);
        int quit = is_quit_command(line);


        //a questo punto manda il comando al server
        if (send_line(sockfd, line) < 0) {
            perror("send_line");
            break;
        }
        
        //gestione comandi che generano una risposta multilinea
        if (multiline) {
            while (1) {
                n = recv_line(sockfd, line, sizeof(line));

                if (n <= 0) {
                    printf("Connessione chiusa dal server.\n");
                    close(sockfd);
                    return EXIT_FAILURE;
                }

                if (strcmp(line, "END") == 0) {
                    break;
                }

                printf("%s\n", line);
            }
            
        //gestione comandi che generano risposta singola
        } else {
            n = recv_line(sockfd, line, sizeof(line));

            if (n <= 0) {
                printf("Connessione chiusa dal server.\n");
                break;
            }

            printf("%s\n", line);
        }
        
        //se è quit esce
        if (quit) {
            break;
        }
    }
    
    //chiusura socket e vengono liberate le risorse di rete
    close(sockfd);

    return EXIT_SUCCESS;
}
