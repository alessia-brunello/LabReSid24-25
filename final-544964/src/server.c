#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <strings.h>
#include <time.h>
#include <fcntl.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#include "server.h"
#include "http.h"
#include "router.h"

#define REQUEST_INCOMPLETE  0                
#define REQUEST_COMPLETE  1           
#define REQUEST_BAD_CONTENT_LENGTH -1  
#define REQUEST_TOO_LARGE  -2            

#define SESSION_ACTIVE  0
#define SESSION_CLOSED 1


//ogni client connesso è rappresentato da una sessione 
typedef struct ClientSession {
        int fd;                             //file descriptor della socket del client
        char ip[INET_ADDRSTRLEN];           //indirizzo IP del client
        int port;                           //porta del client
        
        char request_buffer[BUFFER_SIZE];   //buffer delle richieste ricevute ma non ancora elaborate
        size_t request_size;                //numero di byte presenti in request_buffer
        
        char response_buffer[RESPONSE_SIZE]; //buffer delle risposte da inviare
        size_t response_size;                //numero totale di byte della risposta
        size_t response_sent;                //numero di byte già inviati dal client
        
        int close_after_response;
        time_t last_activity;               //l'ultimo istante di attività usato per il timeout
        
        struct ClientSession* next;         //puntatore alla prossima sessione
} ClientSession;

static ClientSession* sessions = NULL;      //lista di sessioni attive
static int active_sessions = 0;             ///numero di sessioni attive

static int process_ready_requests(int epoll_fd, ClientSession* session);

//imposta la socket non bloccante
static int set_non_blocking(int fd) {
        int flags = fcntl(fd, F_GETFL, 0);
        
        if(flags < 0) {
                return -1;
        }
        
        if(fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
                return -1;
        }
        return 0;
}


//invia tutti i byte presenti nel buffer sulla socket
static int send_all(int fd, const char* buffer, size_t length) {
        size_t total_sent = 0;
        
        while(total_sent < length) {
                ssize_t sent = send(fd, buffer + total_sent, length - total_sent, MSG_NOSIGNAL);
                
                if(sent < 0) {
                        if(errno == EINTR) {
                                continue;
                        }
                        if(errno == EAGAIN || errno == EWOULDBLOCK) {
                                break;
                        }
                        
                        return -1;
                }
                if(sent == 0) {
                        return -1;
                }
                total_sent += (size_t)sent;
        }
        return total_sent == length ? 0 : -1;
}

//Estrae Content-Length dagli header HTTP. Serve a capire quanti byte di body bisogna aspettare prima di poter processare una richiesta completa
static int get_content_length(const char* request, size_t* content_length) {
        const char* headers_end = strstr(request, "\r\n\r\n");
        const char* line;
        const char prefix[] = "Content-Length:";
        const size_t prefix_len = strlen(prefix);
        
        
        if(request == NULL || content_length == NULL) {
                return -1;
        }
        *content_length = 0;
        
        //se gli header non sono completi non segnala errore
        if(headers_end == NULL) {
                return 0;
        }
        
        line = request;
        while(line < headers_end) {
                const char* line_end = strstr(line, "\r\n");
                size_t line_len;
                
                if(line_end == NULL || line_end > headers_end) {
                        line_end = headers_end;
                }
                line_len = (size_t)(line_end - line);
                
                //cerca la riga Content-Length senza far caso a maiuscole o minuscole
                if(line_len >= prefix_len && strncasecmp(line, prefix, prefix_len) == 0) {
                        const char* value = line + prefix_len;
                        char* endptr = NULL;
                        unsigned long parsed;
                        
                        //salta eventuali spazi prima del valore numerico
                        while(value < line_end && isspace((unsigned char)*value)) {
                                value++;
                        }
                        if(value == line_end) {
                                return -1;
                        }
                        errno = 0;
                        parsed = strtoul(value, &endptr, 10);
                        
                        if(errno != 0 || endptr == value) {
                                return -1;
                        }
                        //dopo il numero sono ammessi solo spazi
                        while(endptr < line_end && isspace((unsigned char)*endptr)) {
                                endptr++;
                        }
                        
                        if(endptr != line_end) {
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
        return 0;
}


//Controlla se nel buffer della sessione è presente una richiesta HTTP intera. La richiesta può arrivare completa, parziale o più insieme.
//Il server mantiene un buffer per sessione e calcola la lunghezza effettiva della richiesta (header + Content-Length)
static int find_complete_request(const char* buffer, size_t buffer_size, size_t* request_length) {
        const char* headers_end;
        size_t header_size;
        size_t content_length = 0;
        
        if(buffer == NULL || request_length == NULL) {
                return REQUEST_BAD_CONTENT_LENGTH;
        }
        
        *request_length = 0;
        
        headers_end = strstr(buffer, "\r\n\r\n");
        
        if(headers_end == NULL) {
                return REQUEST_INCOMPLETE;
        }
        
        //+4 per includere la sequenza \r\n\r\n
        header_size = (size_t)(headers_end - buffer) + 4;
        
        if(get_content_length(buffer, &content_length) != 0) {
                return REQUEST_BAD_CONTENT_LENGTH;
        }
        
        //evita richieste troppo grandi rispetto ai limiti
        if(content_length > MAX_BODY_LEN || header_size + content_length >= BUFFER_SIZE) {
                return REQUEST_TOO_LARGE;
        }
        
        //se non sono arrivati tutti i byte del body la richiesta resta in attesa
        if(buffer_size < header_size + content_length) {
                return REQUEST_INCOMPLETE;
        }
        
        *request_length = header_size + content_length;
        return REQUEST_COMPLETE;
}

//Aggiorna gli eventi monitorati con epoll per una sessione, EPOLLIN rimane sempre attivo per leggere le richieste; EPOLLOUT viene abilitato solo quando c'è una risposta da inviare
static void update_session_events(int epoll_fd, ClientSession* session) {
        struct epoll_event event;
        
        memset(&event, 0, sizeof(event));
        event.events = EPOLLIN | EPOLLRDHUP | EPOLLERR;
        
        if(session->response_size > session->response_sent) {
                event.events |= EPOLLOUT;
        }
        
        event.data.ptr = session;
        
        if(epoll_ctl(epoll_fd, EPOLL_CTL_MOD, session->fd, &event) < 0) {
                perror("Errore epoll_ctl MOD client");
        }
}

//aggiunge una sessione alla lista
static void add_session_to_list(ClientSession* session) {
        session->next = sessions;
        sessions = session;
        active_sessions++;
}

//rimuove la sessione dalla lista
static void remove_session_from_list(ClientSession* session) {
        ClientSession** current = &sessions;
        
        while(*current != NULL) {
                if(*current == session) {
                        *current = session->next;
                        active_sessions --;
                        return;
                }
                current = &(*current)->next;
        }
}

//chiude la sessione. La toglie da epoll, chiude la socket, la rimuove dalla lista e libera memoria 
static void close_session(int epoll_fd, ClientSession* session, const char* reason) {
        if(reason == NULL) {
                return;
        }
        
        if(reason != NULL) {
                printf("\n[CONNECTION] Sessione %s : %d chiusa. Motivazione chiusura: %s\n", session->ip, session->port, reason); 
        }
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, session->fd, NULL);
        close(session->fd);
        remove_session_from_list(session);
        free(session);
}

//Prepara una risposta HTTP da inviare sulla sessione, viene messa su un buffer perchè non è garantito che send riesca a spedire tutto in una sola volta
static void queue_response(ClientSession* session, const char* response, int close_after_response) {
        size_t length = strlen(response);
        
        if(length >= RESPONSE_SIZE) {
                length = RESPONSE_SIZE - 1;
        }
        
        memcpy(session->response_buffer, response, length);
        session->response_buffer[length] = '\0';
        
        session->response_size = length;
        session->response_sent = 0;
        session->close_after_response = close_after_response;
}

//Se la socket non è pronta a ricevere tutti i byte la funzione salva i byte inviati e lascia EPOLLOUT attivo in modo da poter riprendere l'invio appena possibile
static int flush_response(int epoll_fd, ClientSession* session) {
        while(session->response_sent < session->response_size) {
                ssize_t sent = send(session->fd, session->response_buffer + session->response_sent, session->response_size - session->response_sent, MSG_NOSIGNAL);
                
                if(sent < 0) {
                        if(errno == EINTR) {
                                continue;
                        }
                        if(errno == EAGAIN || errno == EWOULDBLOCK) {
                                update_session_events(epoll_fd, session);
                                return SESSION_ACTIVE;
                        }
                        return SESSION_CLOSED;
                }
                if(sent == 0) {
                        return SESSION_CLOSED;            
                }
                session->response_sent += (size_t)sent;
        }
        session->response_size = 0;
        session->response_sent = 0;
        
        if(session->close_after_response) {
                return SESSION_CLOSED;
        }
        update_session_events(epoll_fd, session);
        return SESSION_ACTIVE;
}


//costruisce e mette in coda una risposta di errore legata al protocollo HTTP, la connessione viene chiusa dopo la risposta
static int queue_protocol_error(int epoll_fd, ClientSession* session, int status_code, const char* status_text, const char* body) {
        char response[RESPONSE_SIZE];
        
        build_http_response(response, sizeof(response), status_code, status_text, body, 0);
        queue_response(session, response, 1);
        
        return flush_response(epoll_fd, session);
}

//processa le richieste complete presenti nel buffer, si ferma se il buffer ha una risposta non completa o se c'è una risposta da inviare
static int process_ready_requests(int epoll_fd, ClientSession* session) {
       
        while(session->response_size == 0) {
                size_t request_length = 0;
                int request_status;
        
	        char raw_request[BUFFER_SIZE];
	        char response[RESPONSE_SIZE];

                HttpRequest request;
                size_t remaining;
                
                request_status = find_complete_request(session->request_buffer, session->request_size, &request_length);
                if(request_status == REQUEST_INCOMPLETE) {
                        break;
                }
                
                if(request_status == REQUEST_BAD_CONTENT_LENGTH) {
	                return queue_protocol_error(epoll_fd, session, 400, "Bad Request", "{\"error\":\"Invalid Content-Length\"}\n");
	        }
	        
	        if(request_status == REQUEST_TOO_LARGE) {
	                return queue_protocol_error(epoll_fd, session, 413, "Payload Too Large", "{\"error\":\"Request too large\"}\n");	        
	        }
	                        
	        memset(raw_request, 0, sizeof(raw_request));
	        memcpy(raw_request, session->request_buffer, request_length);
	        raw_request[request_length] = '\0';
	        
	        remaining = session->request_size - request_length;
	        
	        memmove(session->request_buffer, session->request_buffer + request_length, remaining);
	        
	        session->request_size = remaining;
	        session->request_buffer[session->request_size] = '\0';
	        
	        memset(&request, 0, sizeof(request));
	        memset(response, 0, sizeof(response));
	        

	        if(parse_http_request(raw_request, &request) != 0) {
		        printf("[ERROR] Invalid HTTP request from %s : %d\n", session->ip, session->port);

		        return queue_protocol_error(epoll_fd, session, 400, "Bad Request", "{\"error\":\"Invalid HTTP request\"}\n");
	        }
  
                //il router decide quale endpoint rest gestire 
	        route_request(&request, response, sizeof(response));
	        
	        //se request.keep_alive vale 0 la risposta  viene inviata e poi la sessione si chiude, altrimenti la socket rimane in epoll per ricevere nuove richieste dallo stesso client
	        queue_response(session, response, request.keep_alive ? 0 : 1);
	        
	        if(flush_response(epoll_fd, session) == SESSION_CLOSED) {
	                return SESSION_CLOSED;
	        }
	}
	
	update_session_events(epoll_fd, session);
	return SESSION_ACTIVE;
}


//legge tutti i byte disponibili della socket del client e li mette nel buffer della sessione
static int handle_client_input(int epoll_fd, ClientSession* session) {
        while(1) {
                ssize_t received;
                size_t available = BUFFER_SIZE - session->request_size - 1;
                
                if(available == 0) {
                        return queue_protocol_error(epoll_fd, session, 413, "Payload Too Large", "{\"error\":\"IRequest too large\"}\n");
                }
                
                received = recv(session->fd, session->request_buffer + session->request_size, available, 0);
                
                if(received < 0) {
                        if(errno == EINTR) {
                                continue;
                        }
                        if(errno == EAGAIN || errno == EWOULDBLOCK) {
                                break;
                        }
                        return SESSION_CLOSED;
                }
                if(received == 0) {
                        return SESSION_CLOSED;
                }
                
                session->request_size += (size_t)received;
                session->request_buffer[session->request_size] = '\0';
                session->last_activity = time(NULL);
        }
        return process_ready_requests(epoll_fd, session);
}


//se il server ha raggiunto il numero massimo di sessioni manda al client 503 
static void send_service_unavailable(int client_fd) {
        char response[RESPONSE_SIZE];
        build_http_response(response, sizeof(response), 503, "Service Unavailable", "{\"error\":\"Server busy, try again later\"}\n", 0);
        
        send_all(client_fd, response, strlen(response));
        close(client_fd);
        
        printf("[RESPONSE] 503 Service Unavailable\n");
}

//accetta tutte le connessioni in attesa sul socket server, il ciclo continua finchè non ci sono client da accettare, cioè finchè accept segnala EAGAIN/EWOULDBLOCK
static void accept_new_clients(int epoll_fd, int server_fd) {
        while(1) {
                struct sockaddr_in client_addr;
                socklen_t client_len = sizeof(client_addr);
                int client_fd;
                ClientSession* session;
                struct epoll_event event;
                
                client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
                
                if(client_fd < 0) {
                        if(errno == EAGAIN || errno == EWOULDBLOCK) {
                                return;
                        }        
                        if(errno == EINTR) {
                                continue;
                        }
                        perror("Errore accept)");
                        return;
                }
                
                //se il numero massimo di sessioni è stato raggiunto il client viene rifiutato
                if(active_sessions >= MAX_CLIENT_SESSIONS) {
                        printf("[WARNING] Limite massimo di sessioni raggiunto. Client rifiutato\n");
                        send_service_unavailable(client_fd);
                        continue;
                }
                
                //la socket viene resa non bloccante
                if(set_non_blocking(client_fd) != 0) {
                        perror("Errore set_non_blocking client");
                        close(client_fd);
                        continue;
                }
                
                //crea e inizializza la struttura che rappresenta la nuova sessione
                session = calloc(1, sizeof(ClientSession));
                
                if(session == NULL) {
                        perror("Errore calloc ClientSession");
                        send_service_unavailable(client_fd);
                        continue;
                }
                session->fd = client_fd;
                inet_ntop(AF_INET, &client_addr.sin_addr, session->ip, sizeof(session->ip));
                session->port = ntohs(client_addr.sin_port);
                session->last_activity = time(NULL);
                
                memset(&event, 0, sizeof(event));
                event.events = EPOLLIN | EPOLLRDHUP | EPOLLHUP | EPOLLERR;
                event.data.ptr = session;
                
                
                //salva la socket client in epoll e la associa al puntatore della sessione
                if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) < 0) {
                        perror("Errore epoll_ctl ADD client");
                        close(client_fd);
                        free(session);
                        continue;
                }
                add_session_to_list(session);
                printf("\n[ACCEPT] Sessione aperta da %s : %d\n", session->ip, session->port);
        }
}

//Controlla periodicamente le sessioni inattive, il timeout serve a evitare che una connessione resti aperta senza traffico
static void check_session_timeouts(int epoll_fd) {
        ClientSession* current = sessions;
        time_t now = time(NULL);
        
        while(current != NULL) {
                ClientSession* next = current->next;
                
                if(current->response_size == 0 && now - current->last_activity >= SOCKET_TIMEOUT) {
                        printf("[TIMEOUT] Sessione inattiva di %s : %d per oltre %d secondi\n", current->ip, current->port, SOCKET_TIMEOUT);
                        
                        if(queue_protocol_error(epoll_fd, current, 408, "Request Timeout", "{\"error\":\"Session timeout\"}\n") == SESSION_CLOSED) {
                                close_session(epoll_fd, current, "timeout");
                        }
                }
                current = next;
        }
}


void start_server(int port) {
	int server_fd;
	int epoll_fd;
	struct sockaddr_in server_addr;
	struct epoll_event event;
	struct epoll_event events[EPOLL_MAX_EVENTS];
	int opt = 1;

        //socket server
	server_fd = socket(AF_INET, SOCK_STREAM, 0);

	if(server_fd < 0) {
		perror("Errore socket");
		exit(EXIT_FAILURE);
	}

	if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		perror("Errore setsockopt");
		close(server_fd);
		exit(EXIT_FAILURE);
	}
	
	if(set_non_blocking(server_fd) != 0) {
	        perror("Errore set_non_blocking server");
		close(server_fd);
		exit(EXIT_FAILURE);
	}
	
	memset(&server_addr, 0, sizeof(server_addr));

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY; //accetta connessioni su tutte le interfacce
	server_addr.sin_port = htons(port);       //converte la porta in netword by order
	
	//associa la socket all'indirizzo e alla porta scelti
	if(bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		perror("Errore bind");
		close(server_fd);
		exit(EXIT_FAILURE);
	}

        //mette la socket in ascolto
	if(listen(server_fd, BACKLOG) < 0) {
		perror("Errore listen");
		close(server_fd);
		exit(EXIT_FAILURE);
	}
	
	//crea l'istanza epoll per monitorare le socket del server e del client
	epoll_fd = epoll_create1(0);
	
	if(epoll_fd < 0) {
	        perror("Errore epoll_create1");
		close(server_fd);
		exit(EXIT_FAILURE);
	}
	memset(&event, 0, sizeof(event));
	event.events = EPOLLIN;
	event.data.ptr = NULL; //socket server registrato con data.ptr=null, serve a distinguere nell event loop gli eventi di accept da quelli associati alle sessioni client
	
	//registra la socket server in epoll. quando è pronta in lettura vuol dire che ci sono nuove connessioni da accettare
	if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) < 0) {
	        perror("Errore epoll_ctl ADD server");
	        close(epoll_fd);
		close(server_fd);
		exit(EXIT_FAILURE);
	}

	printf("[START] TicketFlow REST Server\n");
	printf("[INFO] Listening on port %d\n", port);
	printf("[INFO] Max client sessions: %d\n", MAX_CLIENT_SESSIONS);
	printf("[INFO] Session timeout: %d seconds\n", SOCKET_TIMEOUT);
	printf("[INFO] Test single request: curl http://localhost:%d/health\n", port);
	printf("[INFO] Interactive client:  make client\n");

        //event loop principale
	while(1) {
	        int ready = epoll_wait(epoll_fd, events, EPOLL_MAX_EVENTS, 1000);
	        
	        if(ready < 0) {
	                if(errno == EINTR) {
	                        continue;
	                }
	                
	                perror("Errore epoll_wait");
	                break;
	        }
	        
	        //gestisce tutti gli eventi restituiti da epoll_wait
	        for(int i = 0; i < ready; i++) {
	                ClientSession* session = (ClientSession*)events[i].data.ptr;
	                int result = SESSION_ACTIVE;
	                
	                //significa che c'è un evento sulla socket server: bisogna accettare nuovi client
	                if(session == NULL) {
	                        accept_new_clients(epoll_fd, server_fd);
	                        continue;
	                }
	                if(events[i].events & EPOLLERR) {
	                        close_session(epoll_fd, session, "errore sulla socket");
	                        continue;
	                }
	                
	                //la socket client ha dati da leggere
	                if(events[i].events & EPOLLIN) {
	                        result = handle_client_input(epoll_fd, session);
	                }
	                if(result == SESSION_CLOSED) {
	                        close_session(epoll_fd, session, "lettura terminata");
	                        continue;
	                }
	                
	                //la socket client è pronta per continuare l'invio di una risposta parziale
	                if(events[i].events & EPOLLOUT) {
	                        result = flush_response(epoll_fd, session);
	                }
	                if(result == SESSION_CLOSED) {
	                        close_session(epoll_fd, session, "risposta completata");
	                        continue;
	                }
	                
	                //se ci sono ancora byte nel buffer e nessuna risposta in corso, prova a processare altre richieste già arrivate
	                if(session->response_size == 0 && session->request_size > 0) {
	                        result = process_ready_requests(epoll_fd, session);
	                }
	                if(result == SESSION_CLOSED) {
	                        close_session(epoll_fd, session, "richiesta completata");
	                        continue;
	                }
	                
	                //indicano che il client ha chiuso o sta chiudendo la connessione
	                if(events[i].events & (EPOLLHUP | EPOLLRDHUP)) {
	                        close_session(epoll_fd, session, "client disconnesso");
	                }
	        }
	        //viene controllato il timeout
	        check_session_timeouts(epoll_fd);
		
	}
        close(epoll_fd);
	close(server_fd);
}










































