#include "bank.h"
#include "common.h"
#include "protocol.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static BankDB bank;
static sem_t client_slots;  //semaforo per limitare i client contemporanei
static int next_client_id = 1; //numero progressivo dei client

//informazioni del client
typedef struct {
    int client_fd;
    int client_id;
    struct sockaddr_in client_addr;
} ClientInfo;

//Uso consigliato di come usare il server, se non viene inserita la porta sarà utilizzata di default quella in common.h
static void print_usage(const char* program_name) {
    fprintf(stderr, "Uso: %s [porta]\n", program_name);
}

//questa funzione decide su quale porta avviare il server
static int parse_port(int argc, char* argv[]) {
    if (argc == 1) {    //porta di default
        return SERVER_PORT;
    }

    if (argc != 2) {
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);

    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Porta non valida.\n");
        exit(EXIT_FAILURE);
    }

    return port;
}

static void build_help(char* response, size_t size) {
    snprintf(response, size,
             "OK Comandi disponibili:\n"
             "ADD <id> <importo> <causale>\n"
             "DELETE <id>\n"
             "UPDATE <id> <nuovo_importo> <nuova_causale>\n"
             "LIST\n"
             "HELP\n"
             "QUIT\n"
             "END\n");
}

//restituisce 0 per le risposte di una riga, 1 per la multilinea, -1 per chiudere la connessione

//aggiunta, eliminazione e modifica dell'array vengono fatti fare a bank.c
static int handle_command(const char* request, char* response, size_t response_size) {
    char command[16];

    //estra la prima parola
    if (sscanf(request, "%15s", command) != 1) {
        snprintf(response, response_size, "ERR Comando vuoto.");
        return 0;
    }
    if (strcmp(command, "ADD") == 0) {
        int id;
        double importo;
        char causale[MAX_CAUSALE];
      
        //estrae id, importo e causale (%127[^\n] vuol dire che legge tutta la causale fino alla fine della riga)
        if (sscanf(request, "ADD %d %lf %127[^\n]", &id, &importo, causale) != 3) {
            snprintf(response, response_size, "ERR Sintassi corretta: ADD <id> <importo> <causale>");
            return 0;
        }


        int result = bank_add(&bank, id, importo, causale);

        if (result == 0) {
            snprintf(response, response_size, "OK Movimento aggiunto.");
        } else if (result == -2) {
            snprintf(response, response_size, "ERR Archivio pieno.");
        } else if (result == -3) {
            snprintf(response, response_size,
                     "ERR Esiste già un movimento con ID %d.", id);
        } else {
            snprintf(response, response_size, "ERR Errore interno durante ADD.");
        }

        return 0;
    }

    if (strcmp(command, "DELETE") == 0) {
        int id;
        int pos = 0;

         if (sscanf(request, "DELETE %d %n", &id, &pos) != 1 || request[pos] != '\0') {
            snprintf(response, response_size, "ERR Sintassi corretta: DELETE <id>");
            return 0;
        }

        int result = bank_delete(&bank, id);

        if (result == 0) {
            snprintf(response, response_size, "OK Movimento eliminato.");
        } else {
            snprintf(response, response_size,
                     "ERR Movimento con ID %d non trovato.", id);
        }

        return 0;
    }

    if (strcmp(command, "UPDATE") == 0) {
        int id;
        double importo;
        char causale[MAX_CAUSALE];

        if (sscanf(request, "UPDATE %d %lf %127[^\n]", &id, &importo, causale) != 3) {
            snprintf(response, response_size,
                     "ERR Sintassi corretta: UPDATE <id> <nuovo_importo> <nuova_causale>");
            return 0;
        }

        int result = bank_update(&bank, id, importo, causale);

        if (result == 0) {
            snprintf(response, response_size, "OK Movimento aggiornato.");
        } else {
            snprintf(response, response_size,
                     "ERR Movimento con ID %d non trovato.", id);
        }

        return 0;
    }

    if (strcmp(command, "LIST") == 0) {
        int pos = 0;

        if (sscanf(request, "LIST %n", &pos) != 0 || request[pos] != '\0') {
            snprintf(response, response_size, "ERR Sintassi corretta: LIST");
            return 0;
        }

        bank_list(&bank, response, response_size);
        return 1;
    }

    if (strcmp(command, "HELP") == 0) {
        int pos = 0;

        if (sscanf(request, "HELP %n", &pos) != 0 || request[pos] != '\0') {
            snprintf(response, response_size, "ERR Sintassi corretta: HELP");
            return 0;
        }

        build_help(response, response_size);
        return 1;
    }

    if (strcmp(command, "QUIT") == 0) {
      int pos = 0;

      if (sscanf(request, "QUIT %n", &pos) != 0 || request[pos] != '\0') {
          snprintf(response, response_size, "ERR Sintassi corretta: QUIT");
          return 0;
      }

      snprintf(response, response_size, "OK Connessione chiusa.");
      return -1;
  }
    snprintf(response, response_size, "ERR Comando sconosciuto. Usa HELP.");
    return 0;
}

//per ogni client il server crea un thread con questa funzione
static void* client_thread(void* arg) {
    ClientInfo* info = (ClientInfo *)arg; //conversione del parametro generico void a clientinfo
    int client_fd = info->client_fd; //client_fd è la socket usata per parlare con il client
    int client_id = info->client_id;
  
    char ip[INET_ADDRSTRLEN];
    
    //buffer per richiesta e risposta
    char request[LINE_SIZE];
    char response[LIST_BUFFER_SIZE];
    
    inet_ntop(AF_INET, &(info->client_addr.sin_addr), ip, sizeof(ip));

    printf("Client %d connesso: %s:%d\n", client_id, ip, ntohs(info->client_addr.sin_port));

    free(info); //libera memoria dopo aver copiato client_fd e aver letto l'indirizzo del client

    send_line(client_fd, "Connesso al server della banca.");

    while (1) {
        ssize_t n = recv_line(client_fd, request, sizeof(request));

        if (n == 0) { //il client ha chiuso la connessione
            break;
        }

        if (n < 0) {  //errore di ricezione
            perror("recv_line");
            break;
        }

        //il server riceve cosa vuole fare il client (add, delete ...)
        int mode = handle_command(request, response, sizeof(response));

        //se ritorna 1 vuol dire che è una multilina quindi viene gestita direttamente con send_all
        if (mode == 1) {
            if (send_all(client_fd, response, strlen(response)) < 0) {
                break;
            }
        } else {
            if (send_line(client_fd, response) < 0) {
                break;
            }
        }
        
        //vuol dire che è quit quindi esce e chiude la connessione
        if (mode == -1) {
            break;
        }
    }

    //chiude la socket del client
    close(client_fd);

    //Libera uno slot: ora un altro client può essere servito.
    sem_post(&client_slots);

    printf("Client %d disconnesso.\n", client_id);

    return NULL;
}

//crea e prepara la socket del server
static int create_server_socket(int port) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0); //socket del server

    //se fallisce il programma termina
    if (server_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    //permette di riutilizzare subito la porta dopo aver chiuso il server
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    //struttura del server
    struct sockaddr_in server_addr;

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons((uint16_t)port);


    //collega la socket alla porta scelta
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    //socket in ascolto, BACKLOG indica quante connessioni possono stare in coda di attesa
    if (listen(server_fd, BACKLOG) < 0) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    return server_fd;
}

int main(int argc, char* argv[]) {
    //scelta porta
    int port = parse_port(argc, argv);

    //connessione server
    int server_fd = create_server_socket(port);

    //inizializzazione db
    bank_init(&bank);
    
    //inizializzazione semaforo, parte con 10 slot disponibili per i client attivi
    sem_init(&client_slots, 0, MAX_NUM);

    printf("Server TCP multithread avviato sulla porta %d\n", port);
    printf("Numero massimo di client/worker contemporanei: %d\n", MAX_NUM);

    while (1) {
        ClientInfo* info = malloc(sizeof(ClientInfo));

        if (info == NULL) {
            perror("malloc");
            continue;
        }

        socklen_t addr_len = sizeof(info->client_addr);

        info->client_fd = accept(server_fd, (struct sockaddr *)&info->client_addr, &addr_len);
        
        //solo dopo accept viene creato il thread

        if (info->client_fd < 0) {
            perror("accept");
            free(info);
            continue;
        }

        //prima di creare il thread il server prende uno slot, se il semaforo vale 0 sem_wait blocca il server finchè non c'è uno slot libero
        
        if (sem_trywait(&client_slots) != 0) {
            if (errno == EAGAIN) {
                send_line(info->client_fd,
                          "WAIT Server pieno: attendi che si liberi uno slot...");

                sem_wait(&client_slots);
            } else {
                perror("sem_trywait");
                close(info->client_fd);
                free(info);
                continue;
            }
        }

        pthread_t tid;
        
        info->client_id = next_client_id++;
        
        if (pthread_create(&tid, NULL, client_thread, info) != 0) {
            perror("pthread_create");
            close(info->client_fd);
            free(info);
            sem_post(&client_slots); //libera lo slot
            continue;
        }

        //quando il thread terina libera le sue risorse
        pthread_detach(tid);
    }

    sem_destroy(&client_slots);
    bank_destroy(&bank);
    close(server_fd);

    return 0;
}

