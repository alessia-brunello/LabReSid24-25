#include "pc_buffer.h"

void buffer_init(buffer_t* b) {
	b->in = 0;
	b->out = 0;

	sem_init(&b->empty, 0, BUFFER_SIZE);
	sem_init(&b->full, 0, 0);
	sem_init(&b->mutex, 0, 1);
}

void buffer_insert(buffer_t* b, int item) {
	sem_wait(&b->empty);
	sem_post(&b->mutex);

	b->buffer[b->in] = item;
	b->in = (b->in + 1) % BUFFER_SIZE;

	sem_post(&b->mutex);
	sem_post(&b->full);
}

int buffer_remove(buffer_t* b) {
	sem_wait(&b->full);
	sem_wait(&b->mutex);

	int item = b->buffer[b->out];
	b->out = (b->out +1) % BUFFER_SIZE;

	sem_post(&b->mutex);
	sem_post(&b->empty);

	return item;
}


void buffer_destroy(buffer_t* b) {
	sem_destroy(&b->empty);
	sem_destroy(&b->full);
	sem_destroy(&b->mutex);
}
