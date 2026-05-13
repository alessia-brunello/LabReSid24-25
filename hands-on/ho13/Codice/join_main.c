#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "join.h"


typedef struct {
	join_t* join;
	int id;
} thread_args_t;


static void simula_lavoro_figlio(int id) {
	int avanzamento = 0;

	for (int fase = 1; fase <= 3; fase++) {
		avanzamento += 33;
		printf("[Figlio %d] fase %d/3: sto elaborando dati... avanzamento circa %d%%\n",
		       id, fase, avanzamento);
		sleep(1);
	}

	printf("[Figlio %d] ultima fase: preparo il segnale per il padre\n", id);
}


static void* thread_figlio(void* arg) {
	thread_args_t* args = (thread_args_t*)arg;

	printf("[Figlio %d] thread avviato\n", args->id);

	simula_lavoro_figlio(args->id);

	printf("[Figlio %d] lavoro terminato\n", args->id);

	//segnala al padre che ha terminato, non restituisce nulla come richiesto

	join_signal_done(args->join);

	return NULL;
}


int main(void){
	pthread_t tid;
	pthread_attr_t attr;
	join_t join;
	thread_args_t args;

	if(join_init(&join) != 0) {
		perror("join_init");
		return EXIT_FAILURE;
	}

	args.join = &join;
	args.id = 1;

	//creazione thread come detached

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	if (pthread_create(&tid, &attr, thread_figlio, &args) != 0) {
		perror("pthread create");
		pthread_attr_destroy(&attr);
		join_destroy(&join);
		return EXIT_FAILURE;
	}

	pthread_attr_destroy(&attr);

	printf("[PADRE] attendo la terminazione del figlio\n");

	join_wait(&join);

	printf("[PADRE] il figlio ha terminato: posso continuare\n");

	join_destroy(&join);

	return EXIT_SUCCESS;

}







