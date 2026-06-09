#ifndef SERVER_H
#define SERVER_H

#define BUFFER_SIZE 8192        //dimensione massima del buffer per memorizzare le richieste HTTP ricevute da client
#define RESPONSE_SIZE 65536     //buffer per costruire e inviare le risposte HTTP
#define BACKLOG 10              //numero massimo di connessioni che possono rimanere in coda prima di essere accettate dal server

#define EPOLL_MAX_EVENTS 64     //numero massimo di eventi che epoll_wait può restituire in una sola iterazione dell'event loop
#define MAX_CLIENT_SESSIONS 256 //numero massimo di sessioni che il server mantiene contemporaneamente

#define SOCKET_TIMEOUT 300      //massimo di inattività del client (5 minuti)

void start_server(int port);

#endif
