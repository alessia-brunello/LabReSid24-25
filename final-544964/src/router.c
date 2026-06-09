#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "router.h"
#include "http.h"
#include "db.h"
#include "utils.h"


//risposta di errore in formato JSON
static void send_error(char *response, size_t response_size,int keep_alive, int code, const char *status_text, const char *message) {
        char body[256];

        snprintf(body, sizeof(body), "{\"error\":\"%s\"}\n", message);
        build_http_response(response, response_size, code, status_text, body, keep_alive);

        printf("[RESPONSE] %d %s\n", code, status_text);
}

//risposta per i messaggi di successo in JSON
static void send_message(char *response, size_t response_size, int keep_alive, int status_code, const char *status_text, const char *message) {
        char body[256];

        snprintf(body, sizeof(body), "{\"message\":\"%s\"}\n", message);

        build_http_response(response, response_size, status_code, status_text, body, keep_alive);

        printf("[RESPONSE] %d %s\n", status_code, status_text);
}


//legge il body JSON della richiesta e riempe la struct Ticket
//se anche solo un campo manca la funzione restituisce errore
static int parse_ticket_body(const char* body, Ticket* ticket) {
        if(body == NULL || ticket == NULL) {
                return -1;
        }
        
	if(get_json_value(body, "title", ticket->title, sizeof(ticket->title)) != 0){
		return -1;
	}

	if(get_json_value(body, "description", ticket->description, sizeof(ticket->description)) != 0){
                return -1;
        }

	if(get_json_value(body, "location", ticket->location, sizeof(ticket->location)) != 0){
                return -1;
        }

	if(get_json_value(body, "category", ticket->category, sizeof(ticket->category)) != 0){
                return -1;
        }

	if(get_json_value(body, "priority", ticket->priority, sizeof(ticket->priority)) != 0){
                return -1;
        }

	if(get_json_value(body, "status", ticket->status, sizeof(ticket->status)) != 0){
                return -1;
        }

	return 0;
}

//controlla se il path ha la forma /tickets/{id}
static int is_ticket_id_path(const char* path) {
        return extract_id_from_path(path, "/tickets/") > 0;
}

//distingue tra rotta inesistente e metodo HTTP non consentito
static int is_known_path(const char* path) {
        if(strcmp(path, "/health") == 0) {
                return 1;        
        }
        
        if(strcmp(path, "/tickets") == 0) {
                return 1;
        }
        
        if(is_ticket_id_path(path)) {
                return 1;
        }
        return 0;
}

void route_request(const HttpRequest* request, char* response, size_t response_size) {
	printf("\n[REQUEST] %s %s\n", request->method, request->path);

        //verifica che il server sia attivo
	if(strcmp(request->method, "GET") == 0 && strcmp(request->path, "/health") == 0) {
		printf("[ROUTE] Health Check\n");

		const char* body = "{\"status\":\"ok\"," "\"service\":\"TicketFlow REST Server\"}\n";

		build_http_response(response, response_size, 200, "OK", body, request->keep_alive);

		printf("[RESPONSE] 200 OK\n");
		return;
	}

        //restituisce tutti i ticket presenti nel database
	if(strcmp(request->method, "GET") == 0 && strcmp(request->path, "/tickets") == 0) {
		printf("[ROUTE] Get all tickets\n");

		char* body = db_get_all_tickets();

		if(body == NULL) {
			send_error(response, response_size, request->keep_alive, 500, "Internal Server Error", "Database error");
			return;
		}

		build_http_response(response, response_size, 200, "OK", body, request->keep_alive);
		free(body);

		printf("[RESPONSE] 200 OK\n");
		return;
	}

        //Restituisce un singolo ticket
	if(strcmp(request->method , "GET") == 0 && strncmp(request->path, "/tickets/", 9) == 0) {
		int id = extract_id_from_path(request->path, "/tickets/");

		if(id <= 0) {
			send_error(response, response_size, request->keep_alive, 400, "Bad Request", "Invalid ticket id");
			return;
		}

		printf("[ROUTE] Get ticket by id: %d\n", id);

		char* body = db_get_ticket_by_id(id);

		if(body == NULL) {
			send_error(response, response_size, request->keep_alive, 404, "Not Found", "Ticket not found");
			return;
		}

		build_http_response(response, response_size, 200, "OK", body, request->keep_alive);
		free(body);
		printf("[RESPONSE] 200 OK\n");
		return;
	}


        // crea un nuovo ticket
	if(strcmp(request->method, "POST") == 0 && strcmp(request->path, "/tickets") == 0) {
		printf("[ROUTE] Create ticket\n");

		Ticket ticket;

		if(parse_ticket_body(request->body, &ticket) != 0) {
			send_error(response, response_size, request->keep_alive, 400, "Bad Request", "Invalid or incomplete JSON body");
			return;
		}

		int new_id = 0;

		if(db_create_ticket(&ticket, &new_id) != 0) {
			send_error(response, response_size, request->keep_alive, 500, "Internal Server Error", "Database error");
			return;
		}

		char body[256];

		snprintf(body, sizeof(body), "{\"message\":\"Ticket created\",\"id\":%d}\n", new_id);
		build_http_response(response, response_size, 201, "Created", body, request->keep_alive);

		printf("[DB] Ticket created with id %d\n", new_id);
		printf("[RESPONSE] 201 Created\n");
		return;
	}


        //aggiorna un ticket esistente
	if(strcmp(request->method, "PUT") == 0 && strncmp(request->path, "/tickets/", 9) == 0) {
		int id = extract_id_from_path(request->path, "/tickets/");

		if(id <= 0) {
			send_error(response, response_size, request->keep_alive, 400, "Bad Request", "Invalid ticket id");			
			return;
		}

		printf("[ROUTE] Update ticket id: %d\n", id);

                Ticket ticket;

		if(parse_ticket_body(request->body, &ticket) != 0) {
 			send_error(response, response_size, request->keep_alive, 400, "Bad Request", "Invalid or incomplete JSON body");                      
 			return;
		}

		int result = db_update_ticket(id, &ticket);

                if(result < 0) {
			send_error(response, response_size, request->keep_alive, 500, "Internal Server Error", "Database error");			
			return;
		}
		
		if(result == 0){
			send_error(response, response_size, request->keep_alive, 404, "Not Found", "Ticket not found");
			return;
		}

                send_message(response, response_size, request->keep_alive, 200, "OK", "Ticket updated");
                
		printf("[DB] Ticket Update with id: %d\n", id);

		return;
	}

        //elimina un ticket del database
	if(strcmp(request->method, "DELETE") == 0 && strncmp(request->path, "/tickets/", 9) == 0) {
		int id = extract_id_from_path(request->path, "/tickets/");

		if(id <= 0) {
			send_error(response, response_size, request->keep_alive, 400, "Bad Request", "Invalid ticket id");
			return;
		}

		printf("[ROUTE] Delete ticket id: %d\n", id);

		int result = db_delete_ticket(id);

		if(result < 0) {
			send_error(response, response_size, request->keep_alive, 500, "Internal Server Error", "Database error");			
			return;
		}

		if(result == 0) {
			send_error(response, response_size, request->keep_alive, 404, "Not Found", "Ticket not found");
			return;
		}

                send_message(response, response_size, request->keep_alive, 200, "OK", "Ticket deleted");

		printf("[DB] Ticket deleted with id %d\n", id);

		return;
	}

        if(is_known_path(request->path)) {
                printf("[ROUTE] Method not allowed\n");
                send_error(response, response_size, request->keep_alive, 405, "Method Not Allowed", "Method not allowed");
                return;
        }
        
        //se nessuna route corrisponde, restituisce 404
	printf("[ROUTE] Not found\n");

        send_error(response, response_size, request->keep_alive, 404, "Not Found", "Route not found");
}
















