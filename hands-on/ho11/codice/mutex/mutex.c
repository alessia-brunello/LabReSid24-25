#include "../header.h"
#include "mutex.h"

static int counter = 0;
static pthread_mutex_t mutex;

static void init_mutex(void){
	if(pthread_mutex_init(&mutex, NULL) != 0){
		perror("Errore inizializzazione mutex");
		exit(EXIT_FAILURE);
	}
}

static void destroy_mutex(void){
	if (pthread_mutex_destroy(&mutex) != 0){
		perror("Errore distruzione mutex");
		exit(EXIT_FAILURE);
	}
}

static void* thread_func(void* arg){
	int id = *(int*)arg;

	printf("\n[Thread %d] AVVIATO\n",id);

	for(int i = 0; i < MAX_VALUE; i++){
		pthread_mutex_lock(&mutex);

		int before = counter;
		counter ++;
		int after = counter;

		pthread_mutex_unlock(&mutex);

		if (i % 3000 == 0){
			printf("\n[Thread %d] LOCK --> Incremento protetto: %d -> %d --> UNLOCK\n\n",id, before, after);
		}
	}

	printf("\n[Thread %d] TERMINATO\n", id);
	return NULL;
}



static void create_threads(pthread_t threads[], int ids[]){
	for (int i = 0; i < NUM_THREADS_2; i++){
		ids[i] = i;

		if(pthread_create(&threads[i], NULL, thread_func, &ids[i]) != 0){
			perror("Errore nella creazione del thread");
			exit(EXIT_FAILURE);
		}

		printf("\n[MAIN] creato thread %d\n", i);
	}
}


static void join_threads(pthread_t threads[]){
	for (int i = 0; i < NUM_THREADS_2; i++){
		if(pthread_join(threads[i], NULL) != 0){
			perror("Errore nella join del thread");
			exit(EXIT_FAILURE);
		}
	}
}


void run_mutex(void) {
	pthread_t threads[NUM_THREADS_2];
 	int ids[NUM_THREADS_2];

 	counter = 0;
  	init_mutex();

	printf("\n-------------------------------\n");
	printf("SINCRONIZZAZIONE CON MUTEX\n");
	printf("-------------------------------\n\n");
 	printf("[MAIN] Counter iniziale = %d\n", counter);
 	printf("[MAIN] Numero thread = %d\n", NUM_THREADS_2);
 	printf("[MAIN] Incrementi per thread = %d\n\n", MAX_VALUE);

 	create_threads(threads, ids);

  	join_threads(threads);
  	
	printf("\n\n---------------------------------\n");
  	printf("[MAIN] COUNTER FINALE = %d\n", counter);
  	printf("[MAIN] VALORE ATTESO = %d\n", NUM_THREADS_2 * MAX_VALUE);
	printf("---------------------------------\n");
	
 	destroy_mutex();
}





















