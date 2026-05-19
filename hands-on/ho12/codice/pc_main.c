#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "pc_buffer.h"

#define ITEMS 10

buffer_t buffer;

void* producer(void* arg) {
	(void)arg;

	for(int i = 1; i<= ITEMS; i++) {
		buffer_insert(&buffer, i);
		printf("Prodotto: %d\n", i);
		sleep(1);
	}

	return NULL;
}


void* consumer(void* arg) {
	(void)arg;

	for(int i = 1; i<= ITEMS; i++) {
		int item = buffer_remove(&buffer);
		printf("Consumato: %d\n", item);
		sleep(2);
	}
	return NULL;
}


int main() {
	pthread_t prod, cons;

	buffer_init(&buffer);

	pthread_create(&prod, NULL, producer, NULL);
	pthread_create(&cons, NULL, consumer, NULL);

	pthread_join(prod, NULL);
	pthread_join(cons, NULL);

	buffer_destroy(&buffer);
	return 0;
}
