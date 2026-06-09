#ifndef HTTP_H
#define HTTP_H

#include <stddef.h>

#define MAX_METHOD_LEN 8      //lunghezza metodo(GET, PUT...)
#define MAX_PATH_LEN 256      //lunghezza massima del path richiesto
#define MAX_VERSION_LEN 16    //lunghezza versione HTTP
#define MAX_BODY_LEN 4096     //dimensione del body HTTP

typedef struct {
	char method[MAX_METHOD_LEN];
	char path[MAX_PATH_LEN];
	char version[MAX_VERSION_LEN];
	char body[MAX_BODY_LEN];
	int keep_alive;
}HttpRequest;


int parse_http_request(const char* raw_request, HttpRequest* request);

void build_http_response(char* response, size_t response_size, int status_code, const char* status_text, const char* body, int keep_alive);

#endif
