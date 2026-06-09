#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <strings.h>
#include <signal.h>

#include <netdb.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "client.h"

//struct per raccogliere i dati inseriti dall'utente
typedef struct {
	char title[INPUT_SIZE];
	char description[INPUT_SIZE];
	char location[INPUT_SIZE];
	char category[INPUT_SIZE];
	char priority[INPUT_SIZE];
	char status[INPUT_SIZE];
} TicketForm;

//sessione TCP
typedef struct {
        int sock_fd;        //filedescriptor della socket connessa al server
        const char* host;   //indirizzo del server
        int port;           //porta
} ClientSession;  


//rimuove il carattere di nuova riga da fgets
static void trim_newline(char* text) {
	if(text == NULL) {
		return;
	}

	size_t len = strlen(text);

	while(len > 0 && (text[len - 1] == '\n' || text[len - 1] == '\r')) {
		text[len - 1] = '\0';
		len --;
	}
}

//vengono sostituiti virgolette e backslash inseriti dall'utente
static void sanitize_json_string(const char* input, char* output, int output_size) {
	int j = 0;

	if (input == NULL || output == NULL || output_size <= 0) {
        	return;
    	}

	for(int i = 0; input[i] != '\0' && j < output_size - 1; i++) {
		if(input[i] == '"') {
			output[j++] = '\'';
       		} else if(input[i] == '\\') {
           		 output[j++] = '/';
        	} else if(input[i] == '\n' || input[i] == '\r' || input[i] == '\t') {
            		output[j++] = ' ';
       		} else {
			 output[j++] = input[i];
        	}
   	}

	output[j] = '\0';
}

//Apre la connessione TCP verso il server
static int connect_to_server(const char* host, int port) {
	struct addrinfo hints;
	struct addrinfo* result;
	struct addrinfo* rp;
	char port_string[16];
	int sock_fd = -1;

        //la porta viene resa una stringa per poterla passare a getaddrinfo()
	snprintf(port_string, sizeof(port_string), "%d", port);

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if(getaddrinfo(host, port_string, &hints, &result) != 0) {
		return -1;
	}

        //prova gli indirizzi ricevuti da gettaddrinfo finchè la connect va a buon fine
	for(rp = result; rp != NULL; rp = rp->ai_next) {
		sock_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

		if(sock_fd < 0) {
			continue;
		}

		if(connect(sock_fd, rp->ai_addr, rp->ai_addrlen) == 0) {
			break;
		}

		close(sock_fd);
		sock_fd = -1;
	}

	freeaddrinfo(result);

	return sock_fd;
}

//inizializza la sessione del client
static int open_client_session(ClientSession* session, const char* host, int port) {
        if(session ==  NULL) {
                return -1;
        }
        
        session->host = host;
        session->port = port;
        session->sock_fd = connect_to_server(host, port);
        
        if(session->sock_fd < 0) {
                printf("\nErrore: impossibile connettersi al server %s : %d\n", host, port);
                printf("Assicurati che il server sia avviato con: make run\n");
                return -1;
        }
        printf("\nConnessione aperta verso %s : %d\n", host, port);
        return 0;
}


//l'utente chiude la connessione quando l'utente clicca 0
static void close_client_session(ClientSession* session) {
        if(session != NULL && session->sock_fd >= 0) {
                close(session->sock_fd);
                session->sock_fd = -1;
        }
}


//invia tutti i byte della richiesta HTTP
static int send_all(int sock_fd, const char* buffer, int lenght) {
	int total_sent = 0;

	while(total_sent < lenght) {
		ssize_t sent = send(sock_fd, buffer + total_sent, lenght - total_sent, MSG_NOSIGNAL);

		if(sent < 0) {
		        if(errno == EINTR) {
		                continue;
		        }
		        return -1;
		}
		if(sent == 0) {
		        return -1;
		}

		total_sent += (int)sent;
	}

	return 0;
}

static int get_response_content_length(const char* response, size_t* content_length) {
        const char* headers_end = strstr(response, "\r\n\r\n");
        const char* line;
        const char prefix[] = "Content-Length:";
        const size_t prefix_len = strlen(prefix);
        
        
        if(response == NULL || content_length == NULL) {
                return -1;
        }
        *content_length = 0;
        
        if(headers_end == NULL) {
                return -1;
        }
        
        line = response;
        while(line < headers_end) {
                const char* line_end = strstr(line, "\r\n");
                size_t line_len;
                
                if(line_end == NULL || line_end > headers_end) {
                        line_end = headers_end;
                }
                line_len = (size_t)(line_end - line);
                
                if(line_len >= prefix_len && strncasecmp(line, prefix, prefix_len) == 0) {
                        const char* value = line + prefix_len;
                        char* endptr = NULL;
                        unsigned long parsed;
                        
                        while(value < line_end && isspace((unsigned char)*value)) {
                                value++;
                        }
                        
                        errno = 0;
                        parsed = strtoul(value, &endptr, 10);
                        
                        if(errno != 0 || endptr == value) {
                                return -1;
                        }
                        
                        *content_length = (size_t)parsed;
                        return 0;
                }
                if(line_end == headers_end) {
                        break;
                }
                line = line_end + 2;
        }
        return -1;
}


//legge una risposta HTTP usando Content-Length

static int read_http_response(int sock_fd, char* response, int response_size) {
        int total_read = 0;
        
        if(response == NULL || response_size == 0) {
                return -1;
        }
        response[0] = '\0';
        
        while(total_read < response_size - 1) {
                ssize_t bytes_read = recv(sock_fd, response + total_read, response_size - total_read - 1, 0);
                
                if(bytes_read < 0) {
                        if(errno == EINTR) {
                                continue;
                        }
                        return -1;
                }
                
                if(bytes_read == 0) {
                        return -1;
                }
                
                total_read += (int)bytes_read;
                response[total_read] = '\0';
                
                const char* body = strstr(response, "\r\n\r\n");
                
                if(body != NULL) {
                        size_t header_size = (size_t)(body - response) + 4;
                        size_t body_length = 0;
                        size_t expected_length;
                        
                        if(get_response_content_length(response, &body_length) != 0) {
                                return -1;
                        }
                        
                        expected_length = header_size + body_length;
                        
                        if(expected_length >= (size_t)response_size) {
                                return -1;
                        }
                        
                        //quando sono arrivati header + body completo viene stampata la risposta
                        if((size_t)total_read >= expected_length) {
                                response[expected_length] = '\0';
                                return 0;
                        }
                }
        }
        return -1;
}

//rende la risposta json più leggibile dal client
static void pretty_response_json(const char* json) {
	int indent = 0;
	int in_string = 0;

        //vede se si è dentro una stringa JSON per non formattare caratteri interni al testo
	for(int i = 0; json[i] != '\0'; i++) {
		char c = json[i];

		if(c == '"' && (i == 0 || json[i - 1] != '\\')) {
			in_string = !in_string;
			putchar(c);
			continue;
		}

		if(!in_string) {
			if(c == '{' || c == '[') {
				putchar(c);
				putchar('\n');
				indent++;

				for(int j = 0; j < indent; j++) {
					printf(" ");
				}
			}else if (c == '}' || c == ']') {
				putchar('\n');
				indent--;

				for(int j = 0; j < indent; j++) {
					printf(" ");
				}

				putchar(c);

			}else if(c == ',') {
				putchar(c);
				putchar('\n');

				for(int j = 0; j < indent; j++) {
					printf(" ");
				}
			}else if(c == ':') {
				printf(": ");
			}else {
				putchar(c);
			}
		}else{
			putchar(c);
		}

        }

	putchar('\n');
}


//estrae e stampa il body della risposta HTTP in modo che l'utente veda il JSON e non gli header HTTP
static void print_response_body(const char* response) {
	const char* body = strstr(response, "\r\n\r\n");

	if(body != NULL) {
		body += 4;
	} else {
		body = response;
	}

	if(body[0] == '{' || body[0] == '[') {
		pretty_response_json(body);
	}else {
		printf("%s\n", body);
	}
}

//chiude la connessione quando il server ha chiuso la connessione
static void close_session_after_remote_close(ClientSession* session) {
        if(session != NULL && session->sock_fd >= 0) {
                close(session->sock_fd);
                session->sock_fd = -1;
        }
}

//Uso di select su due descrittori: la tastiera e la connessione tcp con il server.
//Questo perchè il client aspetta la scelta dell'utente e deve accorgersi se il server chiude la socket per il timeout
static int read_input_session(ClientSession* session, const char* prompt, char* buffer, int size) {
        if(buffer == NULL || size <= 0) {
                return -1;
        }
        buffer[0] = '\0';
        
        if(session == NULL || session->sock_fd < 0) {
                printf("Sessione non attiva.\n");
                return -1;
        }
        printf("%s", prompt);
        fflush(stdout);
        
        while(1) {
                fd_set read_fds;
                int max_fd = session->sock_fd;
                int selected;
                
                FD_ZERO(&read_fds);
                FD_SET(STDIN_FILENO, &read_fds);     //descriptor tastiera
                FD_SET(session->sock_fd, &read_fds); //descrittore socket 
                
                if(STDIN_FILENO > max_fd) {
                        max_fd = STDIN_FILENO;
                }
                
                //select si blocca finchè l'utente scrive qualcosa oppure il server invia qualcosa o chiude
                selected = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
                
                if(selected < 0) {
                        if(errno == EINTR) {
                                continue;
                        }
                        printf("\nErrore durante l'attesa dell'input\n");
                        return -1;
                }
                
                //se è pronta la socket vuol dire che è arrivato un messaggio dal server o che la connessione è stata chiusa
                if(FD_ISSET(session->sock_fd, &read_fds)) {
                        char response[RESPONSE_BUFFER_SIZE];
                        char test;
                        ssize_t peeked = recv(session->sock_fd, &test, 1, MSG_PEEK | MSG_DONTWAIT);
                        
                        if(peeked > 0) {
                                memset(response, 0, sizeof(response));
                                if(read_http_response(session->sock_fd, response, sizeof(response)) == 0) {
                                        printf("\n\nMessaggio dal server:\n");
                                        print_response_body(response);
                                }else {
                                        printf("\nLa sessione è syaya chiusa dal server.\n");
                                }
                        }else {
                                printf("\nLa sessione è stata chiusa dal server.\n");
                        }
                        close_session_after_remote_close(session);
                        return -1;
                }
                
                //se è pronta la tastiera legge l'input dell'utente
                if(FD_ISSET(STDIN_FILENO, &read_fds)) {
                        if(fgets(buffer, size, stdin) == NULL) {
                                buffer[0] = '\0';
                                return -1;
                        }
                        trim_newline(buffer);
                        return 0;
                }
        }
}

//costruisce una richiesta HTTP, la invia sulla sessione aperta e stampa la risposta
static int send_http_request(ClientSession* session, const char* method, const char* path, const char* body) {
	char request[REQUEST_BUFFER_SIZE];
	char response[RESPONSE_BUFFER_SIZE];
	size_t body_length = 0;
	int request_length;
	
	if(session == NULL || session->sock_fd < 0) {
	        printf("\nErrore: sessione client non disponibile.\n");
	        return -1;
	}

	if(body != NULL) {
		body_length = strlen(body);
	}

        //se c'è un body inserisce content-length e content-type
	if(body_length > 0) {
		request_length = snprintf(request, sizeof(request), "%s %s HTTP/1.1\r\n" "Host: %s:%d\r\n" "Content-Type: application/json\r\n"
									"Content-Length: %lu\r\n" "Connection: keep-alive\r\n" "\r\n" "%s",
								method, path, session->host, session->port, (unsigned long)body_length, body);

	}else {
		request_length = snprintf(request, sizeof(request),"%s %s HTTP/1.1\r\n" "Host: %s:%d\r\n" "Connection: keep-alive\r\n" "\r\n",
									method, path, session->host, session->port);
	}

	if(request_length <= 0 || request_length >= REQUEST_BUFFER_SIZE) {
		printf("\nErrore: richiesta HTTP troppo grande.\n");
		return -1;
	}
	
	if(send_all(session->sock_fd, request, request_length) != 0) {
	        printf("\nErrore: la connessione con il server non è più attiva.\n");
	        close_session_after_remote_close(session);
	        return -1;
	}
	
	memset(response, 0, sizeof(response));

        if(read_http_response(session->sock_fd, response, sizeof(response)) != 0) {
                printf("\nErrore: la connessione con il server è stata chiusa.\n");
                close_session_after_remote_close(session);
                return -1;
        }

	printf("\nRisposta server:\n");
	print_response_body(response);

	return 0;
}


//legge tutti i campi del ticket e monitora la socket della sessione
static int read_ticket_form(ClientSession* session, TicketForm* ticket) {
	if(read_input_session(session, "Titolo: ", ticket->title, sizeof(ticket->title)) != 0){
	        return -1;
	}
        if(read_input_session(session, "Descrizione: ", ticket->description, sizeof(ticket->description)) != 0){
                return -1; 
        }
        if(read_input_session(session, "Luogo: ", ticket->location, sizeof(ticket->location)) != 0){
                return -1;
        }
        if(read_input_session(session, "Categoria [Hardware/Software/Network/Other]: ", ticket->category, sizeof(ticket->category)) != 0) {
                return -1;
        }
        if(read_input_session(session, "Priorità [Low/Medium/High]: ", ticket->priority, sizeof(ticket->priority)) != 0) {
                return -1;
        }
        if(read_input_session(session, "Stato [Open/In_progress/Closed]: ", ticket->status, sizeof(ticket->status)) != 0) {
                return -1;
        }
        return 0;
}

//costruisce il body JSON a partire dai dati inseriti dall'utente
static void build_ticket_json(const TicketForm* ticket, char* json_body, int json_body_size) {
        char title[INPUT_SIZE];
        char description[INPUT_SIZE];
        char location[INPUT_SIZE];
        char category[INPUT_SIZE];
        char priority[INPUT_SIZE];
        char status[INPUT_SIZE];

        sanitize_json_string(ticket->title, title, sizeof(title));
        sanitize_json_string(ticket->description, description, sizeof(description));
        sanitize_json_string(ticket->location, location, sizeof(location));
        sanitize_json_string(ticket->category, category, sizeof(category));
        sanitize_json_string(ticket->priority, priority, sizeof(priority));
        sanitize_json_string(ticket->status, status, sizeof(status));

        snprintf(json_body, json_body_size, "{" "\"title\":\"%s\",""\"description\":\"%s\",""\"location\":\"%s\",""\"category\":\"%s\","
                                            "\"priority\":\"%s\",""\"status\":\"%s\"" "}", title, description, location, category, priority, status);  
}

//pausa tra l'operazione e il ritorno al menù
static int pause_client(ClientSession* session) {
        char temp[8];

        return read_input_session(session,"\nPremi solo INVIO per tornare alle opzioni ...", temp, sizeof(temp));
}


//legge da tastiera l'id del ticket e controlla che sia positivo
static int read_ticket_id(ClientSession* session, const char* prompt, int* id) {
        char input[64];
        if(id == NULL) {
                return -1;
        }

        *id = -1;

        if(read_input_session(session, prompt, input, sizeof(input)) != 0) {
		return -1;
        }
        *id = atoi(input);
        
        if(*id <= 0) {
                printf("ID non valido.\n");
                return 0;
        }
	return 0;
}

static int health_check(ClientSession* session) {
	return send_http_request(session, "GET", "/health", NULL);
}

static int list_tickets(ClientSession* session) {
	return send_http_request(session, "GET", "/tickets", NULL);
}

static int get_ticket_by_id(ClientSession* session) {
	int id;
	char path[128];
	if(read_ticket_id(session, "Inserisci ID ticket:\n", &id) != 0) {
	        return -1;
	}
	if(id <= 0) {
		return 0;
	}

	snprintf(path, sizeof(path), "/tickets/%d", id);
	return send_http_request(session, "GET", path, NULL);
}


static int create_ticket(ClientSession* session) {
	TicketForm ticket;
	char json_body[4096];

	printf("\nCreazione nuovo ticket\n");

        if(read_ticket_form(session, &ticket) != 0) {
                return -1;
        }
	build_ticket_json(&ticket, json_body, sizeof(json_body));

	return send_http_request(session, "POST", "/tickets", json_body);
}

static int update_ticket(ClientSession* session) {
	int id;
	TicketForm ticket;
	char json_body[4096];
	char path[128];
	
        if(read_ticket_id(session, "Inserisci l'ID del ticket da aggiornare:\n", &id) != 0) {
                return -1;
        }
	if(id <= 0) {
		return 0;
	}

	printf("Nuovi dati del ticket:\n");
	if(read_ticket_form(session, &ticket) != 0) {
	        return -1;
	}
	build_ticket_json(&ticket, json_body, sizeof(json_body));

	snprintf(path, sizeof(path), "/tickets/%d", id);

	return send_http_request(session, "PUT", path, json_body);
}


static int delete_ticket(ClientSession* session) {
	int id;
	char confirm[16];
	char path[128];

        if(read_ticket_id(session, "Inserisci l'ID del ticket da eliminare:\n", &id) != 0) {
                return -1;
        }
	if(id <= 0) {
		return 0;
	}

	if(read_input_session(session, "Confermi eliminazione? [s/n]: ", confirm, sizeof(confirm)) != 0) {
	        return -1;
	}

	if(strcmp(confirm, "s") != 0 && strcmp(confirm, "S") != 0) {
		printf("Eliminazione annullata.\n");
		return 0;
	}else {
	        printf("Eliminazione effettuata!\n");
	}
	snprintf(path, sizeof(path), "/tickets/%d", id);

	return send_http_request(session, "DELETE", path, NULL);
	
}


static void show_menu(void) {
	printf("\n");
	printf("--------------------------\n");
	printf("TicketFlow Terminal Client\n");
	printf("--------------------------\n");
	printf("[1] Visualizza tutti i ticket\n");
	printf("[2] Visualizza i ticket per ID\n");
	printf("[3] Crea un nuovo ticket\n");
	printf("[4] Aggiorna ticket\n");
	printf("[5] Elimina ticket\n");
	printf("[6] Health check server\n");
	printf("[0] Esci\n");
}

void run_client(const char* host, int port) {
        ClientSession session;
	char choice[16];
	
	if(open_client_session(&session, host, port) != 0) {
	        return;
	}
	
	while(1) {
	        int action_result = 0;
	        
		show_menu();
		if(read_input_session(&session, "Scegli un'opzione: ", choice, sizeof(choice)) != 0) {
		        printf("Chiusura client: la sessione non è più disponibile!\n");
		        close_client_session(&session);
		        return;
		}

		switch(choice[0]) {
			case '1':
				action_result = list_tickets(&session);
				break;
			case '2':
				action_result = get_ticket_by_id(&session);

				break;
			case '3':
				action_result = create_ticket(&session);
				break;
			case'4':
				action_result = update_ticket(&session);
				break;
			case '5':
				action_result = delete_ticket(&session);
				break;
			case '6':
			        action_result = health_check(&session);
			        break;
			    
			case '0':
			        close_client_session(&session);
				printf("Connessione chiusa. Chiusura client TicketFlow.\n");
				return;

			default:
				printf("Opzione non valida!!\n");
				action_result = 0;
				break;
		}
		//se un'operazione fallisce la connessione non è più sicura e il client termina
		if(action_result != 0 || session.sock_fd < 0) {
		        printf("Chiusura client: la sessione non è più disponibile!\n");
		        close_client_session(&session);
		        return;
		}
		if(pause_client(&session) != 0) {
		        printf("Chiusura client: la sessione non è più disponibile!\n");
		        close_client_session(&session);
		        return;
		}
	}
}

int main(int argc, char* argv[]) {
        //se il server chiude la socket e il client prova a scrivere ignora con SIGPIPE e l'errore viene gestito ytamite il valore di ritorno della send()
        signal(SIGPIPE, SIG_IGN);

	const char* host = DEFAULT_HOST;
	int port = DEFAULT_PORT;

	if(argc >= 2) {
		host = argv[1];
	}

	if(argc >= 3) {
		port = atoi(argv[2]);

		if(port <= 0) {
			fprintf(stderr, "Porta non valida.\n");
			return EXIT_FAILURE;
		}
	}
	run_client(host, port);

	return EXIT_SUCCESS;
}

































































































































































