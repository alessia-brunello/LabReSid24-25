#ifndef JOIN_H
#define JOIN_H

#include <pthread.h>

typedef struct {
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	int terminato;
} join_t;

int join_init(join_t* join);
void join_destroy(join_t* join);
void join_wait(join_t* join);
void join_signal_done(join_t* join);

#endif
