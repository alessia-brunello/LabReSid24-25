#include "bank.h"

#include <stdio.h>
#include <string.h>

//cerca un movimento dato il suo id, viene chiamata dopo che il muetx viene bloccato in modo da evitare di bloccare due volte lo stesso mutex nello stesso thread.

static int find_index_unlocked(const BankDB* db, int id) {
    for (int i = 0; i < db->count; i++) {
        if (db->movements[i].id == id) {
            return i;
        }
    }

    return -1; //se non trova niente
}

//inizializza la struttura dati
void bank_init(BankDB* db) {
    if (db == NULL) {
        return;
    }

    db->count = 0;   //all'inizio non ci sono movimenti
    pthread_mutex_init(&db->mutex, NULL); //inizializza il mutex
}


//quando il server termina con questa funzione si distrugge il mutex
void bank_destroy(BankDB* db) {
    if (db == NULL) {
        return;
    }

    pthread_mutex_destroy(&db->mutex);
}

int bank_add(BankDB* db, int id, double importo, const char* causale) {
    if (db == NULL || causale == NULL) {
        return -1;
    }

    pthread_mutex_lock(&db->mutex);

    //se l'array è al massimo dei movimenti non può aggiungerne altri e libera il mutex
    if (db->count >= MAX_MOVEMENTS) {
        pthread_mutex_unlock(&db->mutex);
        return -2;
    }

    //controlla se l'id esiste
    if (find_index_unlocked(db, id) >= 0) {
        pthread_mutex_unlock(&db->mutex);
        return -3;
    }

    db->movements[db->count].id = id;
    db->movements[db->count].importo = importo;

    strncpy(db->movements[db->count].causale, causale, MAX_CAUSALE - 1);
    db->movements[db->count].causale[MAX_CAUSALE - 1] = '\0'; //garantisce che la stringa finisca con \0

    db->count++;

    pthread_mutex_unlock(&db->mutex);
    return 0;
}

int bank_delete(BankDB* db, int id) {
    if (db == NULL) {
        return -1;
    }

    pthread_mutex_lock(&db->mutex);

    //cerca il movimento, se non esiste restituisce l'errore e libera il mutex
    int index = find_index_unlocked(db, id);
    if (index < 0) {
        pthread_mutex_unlock(&db->mutex);
        return -2;
    }

    //quando elimina un movimento tramite id che corrisponde ad una posizione, sposta gli altri id di una posizione
    for (int i = index; i < db->count - 1; i++) {
        db->movements[i] = db->movements[i + 1];
    }

    db->count--; //decrementa il numero di movimenti

    pthread_mutex_unlock(&db->mutex);
    return 0;
}

int bank_update(BankDB* db, int id, double new_importo, const char* new_causale) {
    if (db == NULL || new_causale == NULL) {
        return -1;
    }

    pthread_mutex_lock(&db->mutex);

    int index = find_index_unlocked(db, id);
    //se trova l'id modifica i dati, altrimenti restituisce l'errore
    if (index < 0) {
        pthread_mutex_unlock(&db->mutex);
        return -2;
    }

    db->movements[index].importo = new_importo;

    strncpy(db->movements[index].causale, new_causale, MAX_CAUSALE - 1);
    db->movements[index].causale[MAX_CAUSALE - 1] = '\0';

    pthread_mutex_unlock(&db->mutex);
    return 0;
}

//costruisce una stringa dentro out e poi il server manderà la stringa al client
void bank_list(BankDB* db, char* out, size_t out_size) {
    size_t used = 0;
    int written = 0;
    
    if (db == NULL || out == NULL || out_size == 0) {
        return;
    }

    pthread_mutex_lock(&db->mutex);
    
    //database vuoto
    if (db->count == 0) {
        written = snprintf(out, out_size, "Nessun movimento registrato.\nEND\n");   //si usa snprintf per scrivere rispettando la dimensione del buffer
        if (written < 0) {
            out[0] = '\0';
        }

        pthread_mutex_unlock(&db->mutex);
        return;
    }

    written = snprintf(out + used, out_size - used,
                       "Movimenti presenti: %d\n", db->count);

    if (written < 0) {
        out[0] = '\0';
        pthread_mutex_unlock(&db->mutex);
        return;
    }

    used += (size_t)written;

    for (int i = 0; i < db->count && used < out_size; i++) {
        written = snprintf(out + used, out_size - used,
                           "ID = %d | Importo = %.2f | Causale = %s\n",
                           db->movements[i].id,
                           db->movements[i].importo,
                           db->movements[i].causale);

        if (written < 0) {
            break;
        }

        used += (size_t)written;
    }

    if (used < out_size) {
        snprintf(out + used, out_size - used, "END\n");
    } else {
        out[out_size - 1] = '\0';
    }

    pthread_mutex_unlock(&db->mutex);
}
