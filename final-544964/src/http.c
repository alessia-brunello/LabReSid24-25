#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <strings.h>

#include "http.h"

//cerca un token dentro il valore dell'header HTTP, non distingue maiuscole o minuscole. Serve a capire se Connection è keep alive o close
static int header_value_contains(const char* value_start, size_t value_len, const char* token) {
        char value[128];
        size_t copy_len;
        
        if(value_start == NULL || token == NULL) {
                return 0;
        }
        
        copy_len = value_len;
        if(copy_len >= sizeof(value)) {
                copy_len = sizeof(value) - 1;
        }
        
        for(size_t i = 0; i < copy_len; i++) {
                value[i] = (char)tolower((unsigned char)value_start[i]);        
        }
        
        value[copy_len] = '\0';
        
        //restituisce un puntatore se il token è presente, altrimenti NULL
        return strstr(value, token) != NULL;
}

//fa si che la connessione rimanga aperta dopo la risposta; se il client invia connection:close il server chiude la socket
static void parse_connection_mode(const char* raw_request, HttpRequest* request) {
        const char* headers_end;
        const char* line;
        const char prefix[] = "Connection:";
        const size_t prefix_len = strlen(prefix);
        
        request->keep_alive = 1;
        
        if(strcmp(request->version, "HTTP/1.0") == 0) {
                request->keep_alive = 0;
        }
        
        //gli header terminano con una riga vuota
        headers_end = strstr(raw_request, "\r\n\r\n");
        if(headers_end == NULL) {
                return;
        }
        
        //salta la prima riga
        line = strstr(raw_request, "\r\n");
        if(line == NULL || line >= headers_end) {
                return;
        }
        
        line += 2;
        
        //scorre tutti gli header fino alla riga vuota
        while(line < headers_end) {
                const char* line_end = strstr(line, "\r\n");
                size_t line_len;
                
                if(line_end == NULL || line_end > headers_end) {
                        line_end = headers_end;
                }
                
                line_len = (size_t)(line_end - line);
                
                //cerca l'header Connection
                if(line_len >= prefix_len && strncasecmp(line, prefix, prefix_len) == 0) {
                        const char* value = line + prefix_len;
                        size_t value_len = (size_t)(line_end - value);
                        
                        if(header_value_contains(value, value_len, "close")) {
                                request->keep_alive = 0;
                        } else if(header_value_contains(value, value_len, "keep-alive")) {
                                request->keep_alive = 1;
                        }
                        
                        return;
                }
                
                if(line_end == headers_end) {
                        break;
                }
                line = line_end + 2;
        }
}

int parse_http_request(const char* raw_request, HttpRequest* request) {
        int parsed;
        const char* body_start;
        
	if(raw_request == NULL || request == NULL) {
		return -1;
	}

        //azzera tutti i campi della stuct prima di riempirla
	memset(request, 0, sizeof(HttpRequest));

	//estrae dalla prima riga http il metodo, il path e la versione
	parsed = sscanf(raw_request, "%7s %255s %15s", request->method, request->path, request->version);

	if(parsed < 2) {
		return -1;
	}
	
	//se la versione non è presente imposta HTTP/1.1 come default
	if(parsed == 2) {
	        strncpy(request->version, "HTTP/1.1", MAX_VERSION_LEN - 1);
	        request->version[MAX_VERSION_LEN - 1] = '\0';
	}
	
	//legge l'eventuale header Connection e imposta il flag keep-alive
	parse_connection_mode(raw_request, request);

	//body di http inizia dopo la riga vuota
	body_start = strstr(raw_request, "\r\n\r\n");

	if(body_start != NULL) {
		body_start += 4;

                //copia il body nel campo request->body senza andare oltre la grandezza del buffer
		strncpy(request->body, body_start, MAX_BODY_LEN -1);
		request->body[MAX_BODY_LEN -1] = '\0';
	}

	return 0;
}


void build_http_response(char* response, size_t response_size, int status_code, const char* status_text, const char* body, int keep_alive) {
        const char* connection_header = keep_alive ? "keep-alive" : "close";
        
        if(response == NULL || response_size == 0) {
                return;
        }
        
        if(status_text == NULL) {
                status_text = "";
        }
        
	if(body == NULL) {
		body = "";
	}

        //costruisce la risposta completa
	snprintf(response, response_size, "HTTP/1.1 %d %s\r\n"
			"Content-Type: application/json\r\n"
			"Content-Length: %lu\r\n"
			"Connection: %s\r\n"
			"\r\n"
			"%s", status_code, status_text, strlen(body), connection_header, body);

}
