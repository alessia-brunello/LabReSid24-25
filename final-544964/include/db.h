#ifndef DB_H
#define DB_H

#define DB_PATH_SIZE 256            //percorso file SQLite
#define JSON_OUTPUT_SIZE 65536      //buffer usato per costruire la risposta json

typedef struct {
    char title[256];
    char description[512];
    char location[256];
    char category[64];
    char priority[64];
    char status[64];
} Ticket;

int db_init(const char* db_path);

char* db_get_all_tickets(void);
char* db_get_ticket_by_id(int id);

int db_create_ticket(const Ticket* ticket, int* new_id);

int db_update_ticket(int id, const Ticket* ticket);

int db_delete_ticket(int id);

#endif
