#ifndef BARRIER_H
#define BARRIER_H

#include <stdio.h>
#include <pthread.h>

typedef struct {
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	int arrivati;
	int total;
	int aperta;
} barrier_t;

int barrier_init(barrier_t* barrier, int total);
void barrier_destroy(barrier_t* barrier);
void barrier_wait(barrier_t* barrier, int id_thread);

#endif
