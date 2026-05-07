#ifndef BARRIER_H
#define BARRIER_H

#include <semaphore.h>


typedef struct {
	int n;
	int arrived;
	sem_t mutex;
	sem_t turnstile;
} barrier_t;

void barrier_init(barrier_t* b, int n);
void barrier_wait(barrier_t* b);
void barrier_destroy(barrier_t* b);

#endif
