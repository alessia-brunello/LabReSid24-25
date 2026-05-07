#include "../header.h"
#include "semaphore.h"
#include <semaphore.h>

#define MAX_INSIDE_CRIT_SEC 2

static sem_t access_limiter;

static void setup_limited_access(void) {
	if (sem_init(&access_limiter, 0, MAX_INSIDE_CRIT_SEC) != 0) {
		perror ("Errore inizializzazione semaforo");
		exit(EXIT_FAILURE);
	}
}


static void close_limited_access(void) {
	if(sem_destroy(&access_limiter) != 0) {
		perror ("Errore distruzione semaforo");
                exit(EXIT_FAILURE);
	}
}


static void enter_limited_area(int thread_id) {
	printf("[Thread %d] richiede l'accesso alla risorsa condivisa\n\n", thread_id);

	sem_wait(&access_limiter);

	printf("[Thread %d] >>> accesso consentito\n\n", thread_id);
}

static void leave_limited_area(int thread_id) {
	printf("[Thread %d] <<< rilascio della risorsa condivisa\n\n", thread_id);

	sem_post(&access_limiter);
}

static void use_shared_resource(int thread_id){
	printf("[Thread %d] sta usando la risorsa condivisa\n\n", thread_id);
	sleep(2);
	printf("[Thread %d] ha terminato l'uso della risorsa condivisa\n\n", thread_id);
}


static void* worker(void* arg){
	int thread_id = *(int*)arg;

	printf("[Thread %d] avviato\n\n", thread_id);

	enter_limited_area(thread_id);

	use_shared_resource(thread_id);

	leave_limited_area(thread_id);

	printf("[Thread %d] terminato\n\n", thread_id);

	return NULL;
}


static void start_workers(pthread_t workers[], int ids[]) {
	for (int i = 0; i < NUM_THREADS_3; i++) {
		ids[i] = i;
		if(pthread_create(&workers[i], NULL, worker, &ids[i]) != 0) {
			perror("Errore creazione thread");
			exit(EXIT_FAILURE);
		}
	}
}


static void wait_workers(pthread_t workers[]){
	for (int i = 0; i < NUM_THREADS_3; i++){
		if(pthread_join(workers[i], NULL) != 0) {
			perror("Errore join thread");
			exit(EXIT_FAILURE);
		}
	}
}


void run_semaphore(void) {
	pthread_t workers[NUM_THREADS_3];
	int ids[NUM_THREADS_3];

	printf("\n==============================\n");
	printf("ACCESSO LIMITATO CON SEMAFORO\n");
        printf("==============================\n\n");
	printf("[MAIN] Thread totali = %d\n", NUM_THREADS_3);
	printf("[MAIN] Thread ammessi contemporaneamente = %d\n\n", MAX_INSIDE_CRIT_SEC);

	setup_limited_access();

	start_workers(workers, ids);

	printf("[MAIN] Tutti i threads sono stati avviati\n\n");

	wait_workers(workers);

	close_limited_access();
	
	printf("==============================\n");
	printf("[MAIN] PROGRAMMA TERMINATO\n");
	printf("==============================\n");












}






