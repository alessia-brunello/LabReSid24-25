#ifndef CLIENT_H
#define CLIENT_H

#define DEFAULT_HOST "127.0.0.1"    //indirizzo ip di default
#define DEFAULT_PORT 8080           //porta di default

#define REQUEST_BUFFER_SIZE 8192    //buffer usato dal client per costruire una richiesta HTTP
#define RESPONSE_BUFFER_SIZE 65536  //buffer usato dal client per leggere una risposta HTTP
#define INPUT_SIZE 512              //dimensione massima dei campi inseriti dall'utente

void run_client(const char* host, int port);

#endif
