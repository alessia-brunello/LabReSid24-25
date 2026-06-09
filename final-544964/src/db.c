#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sqlite3.h>

#include "db.h"

static char database_path[DB_PATH_SIZE];
static pthread_mutex_t db_mutex = PTHREAD_MUTEX_INITIALIZER;    //mutex per l'accesso al database

//copia una stringa allocando nuova memoria. Viene usata quando una funzione deve restituire un JSON creato localmente
static char* duplicate_string(const char* text) {
	if(text == NULL) {
		return NULL;
	}

        //+1 server per includere il carattere terminatore \0
	char* copy = malloc(strlen(text) + 1);

	if(copy == NULL) {
		return NULL;
	}

	strcpy(copy, text);

	return copy;
}

//apre il database usando il percorso salvato in database_path
static int open_database(sqlite3** db) {
	if(sqlite3_open(database_path, db) != SQLITE_OK) {
		return -1;
	}

	return 0;
}


//esegue l'escape dei caratteri che potrebbero rompere il format json;
static void json_escape(const char* input, char* output, int output_size) {
	int j = 0;
	
	if(output == NULL || output_size <= 0) {
	        return;
	}

	if(input == NULL) {
		output[0] = '\0';
		return;
	}

	for(int i = 0; input[i] != '\0' && j < output_size - 1; i++) {
		if(input[i] == '"' || input[i] == '\\') {
		        //virgolette e backslash devono essere preceduti da un backslash nel JSON
			if(j < output_size - 2) {
				output[j++] = '\\';
				output[j++] = input[i];
			}
		}else if(input[i] == '\n') {
			if(j < output_size - 2) {
				output[j++] = '\\';
				output[j++] = 'n';
			}
		}else if(input[i] == '\r') {
			if(j < output_size - 2) {
				output[j++] = '\\';
				output[j++] = 'r';
			}
		}else if(input[i] == '\t') {
			if(j < output_size -2) {
				output[j++] = '\\';
				output[j++] = 't';
			}
		}else {

			output[j++] = input[i];
		}
	}

	output[j] = '\0';
}


//aggiunge testo al buffer json e controlla di non superare le dimensioni massime.
static int append_to_json(char* json, const char* text, int max_size) {
        if(json == NULL || text == NULL) {
                return -1;
        }
        
        if((int)(strlen(json) + strlen(text)) >= max_size - 1) {
                return -1;
        }
        
        strcat(json, text);
        
        return 0;
}


//converte la riga di SQLite in un oggettp JSON
static void ticket_row_to_json(sqlite3_stmt* stmt, char* output, int output_size){
        char title[512];
	char description[1024];
	char location[512];
	char category[128];
	char priority[128];
	char status[128];
	char created_at[128];
	
	//la colonna 0 è l'id, le successive sono i campi testuali del ticket
	int id = sqlite3_column_int(stmt, 0);

        //ogni campo viene reso sicuro prima di essere inserito nel JSON
	json_escape((const char*)sqlite3_column_text(stmt, 1), title, sizeof(title));
	json_escape((const char*)sqlite3_column_text(stmt, 2), description, sizeof(description));
	json_escape((const char*)sqlite3_column_text(stmt, 3), location, sizeof(location));
	json_escape((const char*)sqlite3_column_text(stmt, 4), category, sizeof(category));
	json_escape((const char*)sqlite3_column_text(stmt, 5), priority, sizeof(priority));
	json_escape((const char*)sqlite3_column_text(stmt, 6), status, sizeof(status));
	json_escape((const char*)sqlite3_column_text(stmt, 7), created_at, sizeof(created_at));
	
	//costruisce l'oggetto JSON finale corrispondente al ticket
	snprintf(output, output_size, "{""\"id\":%d,""\"title\":\"%s\",""\"description\":\"%s\","
						"\"location\":\"%s\",""\"category\":\"%s\",""\"priority\":\"%s\","
						"\"status\":\"%s\",""\"created_at\":\"%s\"""}",
						id, title, description, location, category, priority, status, created_at);

}


int db_init(const char* db_path) {
	sqlite3* db;
	char* error_message = NULL;

	if(db_path == NULL) {
		return -1;
	}

        //salva il percorso del database in una variabile globale
	strncpy(database_path, db_path, DB_PATH_SIZE - 1);
	database_path[DB_PATH_SIZE - 1] = '\0';

	if(open_database(&db) != 0) {
		fprintf(stderr, "DB Error, Cannot open database\n");
		return -1;
	}

        //tabella ticket
	const char* sql = "CREATE TABLE IF NOT EXISTS tickets ("
			  "id INTEGER PRIMARY KEY AUTOINCREMENT,"
			  "title TEXT NOT NULL,"
			  "description TEXT NOT NULL,"
			  "location TEXT NOT NULL,"
			  "category TEXT NOT NULL,"
			  "priority TEXT NOT NULL,"
			  "status TEXT NOT NULL,"
			  "created_at TEXT NOT NULL);";

        //sqlite3_exec viene usata qui perchè la query è senza parametri e senza risultati da leggere
	if(sqlite3_exec(db, sql, NULL, NULL, &error_message) != SQLITE_OK) {
		fprintf(stderr, "Error %s\n", error_message);
		sqlite3_free(error_message);
		sqlite3_close(db);
		return -1;
	}

	sqlite3_close(db);

	printf("\n[DB] Database initialized: %s\n", database_path);

	return 0;
}


char* db_get_all_tickets(void) {
	sqlite3* db;
	sqlite3_stmt* stmt;
	char* json = malloc(JSON_OUTPUT_SIZE);

	if(json == NULL) {
		return NULL;
	}

        //la risposta è un array JSON
	strcpy(json, "[");

	pthread_mutex_lock(&db_mutex);

	if(open_database(&db) != 0) {
		pthread_mutex_unlock(&db_mutex);
		free(json);
		return NULL;
	}

	const char* sql = "SELECT id, title, description, location, category, priority, status, created_at "
				"FROM tickets "
				"ORDER BY id DESC;";

        //SQLite compila la query e la rende pronta all'esecuzione
	if(sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
		sqlite3_close(db);
		pthread_mutex_unlock(&db_mutex);
		free(json);
		return NULL;
	}

	int first = 1;

        //finchè ci sono righe da leggere sqlite3_step restituisce SQLITE_ROW
	while(sqlite3_step(stmt) == SQLITE_ROW) {
		char item[4096];
		
		ticket_row_to_json(stmt, item, sizeof(item));
		
		//mette una virgola tra oggetti JSON
		if(!first) {
                        append_to_json(json, ",", JSON_OUTPUT_SIZE);
                }
                
                append_to_json(json, item, JSON_OUTPUT_SIZE);
                
                first = 0;
        }
        
        append_to_json(json, "]\n", JSON_OUTPUT_SIZE);


        //libera la query e chiude la connessione al database e libera il mutex
	sqlite3_finalize(stmt);
	sqlite3_close(db);

	pthread_mutex_unlock(&db_mutex);

	return json;
}


char* db_get_ticket_by_id(int id) {
	sqlite3* db;
	sqlite3_stmt* stmt;
	char* json = NULL;

	pthread_mutex_lock(&db_mutex);

	if(open_database(&db) != 0) {
		pthread_mutex_unlock(&db_mutex);
		return NULL;
	}

	const char* sql = "SELECT id, title, description, location, category, priority, status, created_at "
			"FROM tickets "
			"WHERE id = ?;";

	if(sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
		sqlite3_close(db);
		pthread_mutex_unlock(&db_mutex);
		return NULL;
	}

        //Il ? viene sostituito con l'id richiesto
	sqlite3_bind_int(stmt, 1, id);

        //se esiste una riga la converte in JSON
	if(sqlite3_step(stmt) == SQLITE_ROW) {
	
                char item[4096];
                
                ticket_row_to_json(stmt, item, sizeof(item));
                
		json = duplicate_string(item);
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);

	pthread_mutex_unlock(&db_mutex);

	return json;
}

int db_create_ticket(const Ticket* ticket, int* new_id) {

	sqlite3* db;
	sqlite3_stmt* stmt;
	int result = -1;
	
	if(ticket == NULL) {
	        return -1;
	}

	pthread_mutex_lock(&db_mutex);

	if(open_database(&db) != 0) {
		pthread_mutex_unlock(&db_mutex);
		return -1;
	}

	const char* sql = "INSERT INTO tickets "
			"(title, description, location, category, priority, status, created_at) "
			"VALUES (?, ?, ?, ?, ?, ?, datetime('now', 'localtime'));";

	if(sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
		sqlite3_close(db);
		pthread_mutex_unlock(&db_mutex);
		return -1;
	}

        //i bind associano i campi di Ticket al ? nella query
	sqlite3_bind_text(stmt, 1, ticket->title, -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 2, ticket->description, -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 3, ticket->location, -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 4, ticket->category, -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 5, ticket->priority, -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 6, ticket->status, -1, SQLITE_TRANSIENT);

        //SQLITE_DONE indica che l'insert è stato completato correttamente
	if(sqlite3_step(stmt) == SQLITE_DONE) {
		if(new_id != NULL) {
		        //salva l'id generato automaticamente
			*new_id = (int)sqlite3_last_insert_rowid(db);
		}

		result = 0;
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);

	pthread_mutex_unlock(&db_mutex);

	return result;
}

int db_update_ticket(int id, const Ticket* ticket) {

	sqlite3* db;
	sqlite3_stmt* stmt;
	int result = -1;
	
	if(ticket == NULL) {
                return -1;
        }

	pthread_mutex_lock(&db_mutex);

	if(open_database(&db) != 0) {
		pthread_mutex_unlock(&db_mutex);
		return -1;
	}

	const char* sql = "UPDATE tickets "
			"SET title = ?, description = ?, location = ?, category = ?, priority = ?, status = ? "
			"WHERE id = ?;";

	if(sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
		sqlite3_close(db);
		pthread_mutex_unlock(&db_mutex);
		return -1;
	}

        //aggiorna i campi
	sqlite3_bind_text(stmt, 1, ticket->title, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, ticket->description, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, ticket->location, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, ticket->category, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 5, ticket->priority, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 6, ticket->status, -1, SQLITE_TRANSIENT);
	sqlite3_bind_int(stmt, 7, id);

	if(sqlite3_step(stmt) == SQLITE_DONE) {
		result = sqlite3_changes(db); //restituisce 0 se nessuna riga è stata aggiornata, >0 se si è modificato il ticket
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);

	pthread_mutex_unlock(&db_mutex);

	return result;
}


int db_delete_ticket(int id) {
	sqlite3* db;
	sqlite3_stmt* stmt;
	int result = -1;

	pthread_mutex_lock(&db_mutex);

	if(open_database(&db) != 0) {
		pthread_mutex_unlock(&db_mutex);
		return -1;
	}

	const char* sql = "DELETE FROM tickets WHERE id = ?;";

	if(sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
		sqlite3_close(db);
		pthread_mutex_unlock(&db_mutex);
		return -1;
	}

        //quale ticket deve essere eliminato
	sqlite3_bind_int(stmt, 1, id);

	if(sqlite3_step(stmt) == SQLITE_DONE) {
		result = sqlite3_changes(db); //distingue ticket eliminato da non trovato
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);
	pthread_mutex_unlock(&db_mutex);

	return result;
}









































































































































