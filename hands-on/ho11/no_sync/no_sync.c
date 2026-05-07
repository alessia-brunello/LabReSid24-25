#include "../header.h"
#include "no_sync.h"

int counter = 0;

static void* thread_func(void* arg) {
	int id = *(int*)arg;
	printf("\n[Thread %d] AVVIATO\n", id);

	for (int i=0; i<MAX_VALUE; i++){
		int before = counter;
		int temp = counter;

		temp++;
		counter = temp;

		if (i % 3000 == 0){
			printf("\n[Thread %d] incremento non protetto: %d --> %d\n\n", id, before, counter);
		}
	}
	printf("\n[Thread %d] TERMINATO -> counter = %d\n", id, counter);
	return NULL;
}

static void create_threads(pthread_t threads[], int ids[]) {
	for (int i = 0; i < NUM_THREADS_1; i++){
		ids[i] = i;

		if(pthread_create(&threads[i], NULL, thread_func, &ids[i]) != 0){
			perror("Errore nella creazione del thread");
			exit(EXIT_FAILURE);
		} 
		printf("\n[MAIN] creato Thread %d\n", i);
	}
}

static void join_threads(pthread_t threads[]){
	for (int i = 0; i < NUM_THREADS_1; i++){
		if(pthread_join(threads[i], NULL) != 0){
			perror("Errore nella join del thread");
			exit(EXIT_FAILURE);
		}
	}
}

void run_no_sync(void){
	pthread_t threads[NUM_THREADS_1];
	int ids[NUM_THREADS_1];

	counter = 0;

	printf("\n-------------------------------\n");
	printf("SENZA SINCRONIZZAZIONE\n");
	printf("-------------------------------\n\n");
        printf("[MAIN] Counter iniziale: %d\n", counter);
        printf("[MAIN] Numero thread: %d\n", NUM_THREADS_1);
        printf("[MAIN] Incrementi per thread: %d\n\n", MAX_VALUE);

	create_threads(threads, ids);

	join_threads(threads);
	printf("\n\n---------------------------------\n");
        printf("[MAIN] COUNTER FINALE: %d\n", counter);
        printf("[MAIN] VALORE ATTESO: %d\n", NUM_THREADS_1*MAX_VALUE);
	printf("---------------------------------\n");
}







