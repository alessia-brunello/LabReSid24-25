#ifndef PC_BUFFER_H
#define PC_BUFFER_H

#include <semaphore.h>

#define BUFFER_SIZE 5

typedef struct {
	int buffer[BUFFER_SIZE];
	int in;
	int out;
	sem_t empty; //spazi liberi nel buffer
	sem_t full; // elementi disponibili
	sem_t mutex; //accesso al buffer (esclusivo)
} buffer_t;


void buffer_init(buffer_t* b);
void buffer_insert(buffer_t* b, int item);
int buffer_remove(buffer_t* b);
void buffer_destroy(buffer_t* b);

#endif
