#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "barrier.h"

barrier_t barrier;

void* worker(void* arg) {
	int id = *(int*) arg;

	printf("Thread %d pronto\n", id);
	sleep(id % 3);

	barrier_wait(&barrier);

	printf("Thread %d partito!\n", id);
	return NULL;
}

int main(int argc, char* argv[]) {
	if(argc != 2) {
		printf("Devi inserire quanti therad far partire: %s N\n", argv[0]);
		return 1;
	}

	int n = atoi(argv[1]);
	pthread_t threads[n];
	int ids[n];

	barrier_init(&barrier, n);

	for(int i = 0; i < n; i++) {
		ids[i] = i +1;
		pthread_create(&threads[i], NULL, worker, &ids[i]);
	}

	for (int i = 0; i < n; i++) {
		pthread_join(threads[i], NULL);
	}

	barrier_destroy(&barrier);
	return 0;

}

