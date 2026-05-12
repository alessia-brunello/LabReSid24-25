#include <stdlib.h>
#include <unistd.h>

#include "barrier.h"

#define NUM_THREADS 4

typedef struct {
	barrier_t* barrier;
	int id;
} thread_args_t;


static void* worker(void* arg) {
        thread_args_t* args = (thread_args_t*)arg;

	//mettiamo un ritardo diverso per ogni thread per visualizzare meglio l'arrivo alla barriera

	printf("[THREAD %d] inizializzazione locale in corso\n", args->id);

	// Ritardo per visualizzare l'arrivo alla barriera.
	sleep(args->id);

	printf("\n[THREAD %d] sono pronto, ma devo aspettare gli altri\n", args->id);

	barrier_wait(args->barrier, args->id);

	printf("[THREAD %d] partito\n", args->id);
	sleep(1);
	printf("[THREAD %d] lavoro completato\n", args->id);

	return NULL;
}



int main(void) {
	pthread_t threads[NUM_THREADS];
	thread_args_t args[NUM_THREADS];
	barrier_t barrier;

	if(barrier_init(&barrier, NUM_THREADS) != 0) {
		perror("barrier_init");
		return EXIT_FAILURE;
	}
	
	printf("[MAIN] creo %d thread e inizializzo la barriera\n", NUM_THREADS);

	for (int i = 0; i < NUM_THREADS; i++) {
		args[i].barrier = &barrier;
		args[i].id = i + 1;

		if(pthread_create(&threads[i], NULL, worker, &args[i]) != 0) {
			perror("pthread_create");
			barrier_destroy(&barrier);
			return EXIT_FAILURE;
		}
	}

	//qui il  pthread_join viene usato solo per far aspettare al main la fine di thread 
	for(int i = 0; i < NUM_THREADS; i++) {
		pthread_join(threads[i], NULL);
	}

	printf("[MAIN] tutti i thread hanno terminato\n");
	
	barrier_destroy(&barrier);

	return EXIT_SUCCESS;

}




















