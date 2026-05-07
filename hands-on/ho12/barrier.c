#include <stdio.h>
#include "barrier.h"

void barrier_init(barrier_t* b, int n) {
	b->n = n;
	b->arrived = 0;
	sem_init(&b->mutex, 0, 1);
	//turnstile è un "tornello", è il punto di blocco,
	//il semaforo che sincronizza la partenza simultanea del thread
	sem_init(&b->turnstile, 0, 0);
}

void barrier_wait(barrier_t* b) {
	sem_wait(&b->mutex);
	b->arrived++;

	if(b->arrived == b->n) {
		printf("Ultimo thread ARRIVATO --> Apro la barriera\n\n");

		for(int i = 0; i < b->n; i++) {
			sem_post(&b->turnstile);
		}
	}

	sem_post(&b->mutex);
	sem_wait(&b->turnstile);
}


void barrier_destroy(barrier_t* b) {
	sem_destroy(&b->mutex);
	sem_destroy(&b->turnstile);
}
