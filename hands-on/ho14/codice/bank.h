#ifndef BANK_H
#define BANK_H

#include <stddef.h>
#include <pthread.h>
#include "common.h"

//singolo movimento bancario

typedef struct {
    int id;
    double importo;
    char causale[MAX_CAUSALE];
} Movement;

//struttura completa: array di movimenti, numero di movimenti e il mutex per la protezione
typedef struct {
    Movement movements[MAX_MOVEMENTS];
    int count;
    pthread_mutex_t mutex;
} BankDB;

void bank_init(BankDB* db);
void bank_destroy(BankDB* db);

int bank_add(BankDB* db, int id, double importo, const char* causale);
int bank_delete(BankDB* db, int id);
int bank_update(BankDB* db, int id, double new_importo, const char* new_causale);

void bank_list(BankDB* db, char* out, size_t out_size);

#endif
