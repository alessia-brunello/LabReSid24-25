#include "join.h"

int join_init(join_t* join){
	int ret;

	join->terminato = 0;

	ret = pthread_mutex_init(&join->mutex, NULL);
	if(ret != 0){
		return ret;
	}

	ret = pthread_cond_init(&join->cond, NULL);
	if (ret != 0) {
		pthread_mutex_destroy(&join->mutex);
		return ret;
	}

	return 0;
}


void  join_destroy(join_t* join){
	pthread_cond_destroy(&join->cond);
	pthread_mutex_destroy(&join->mutex);
}

void join_wait(join_t* join){
	pthread_mutex_lock(&join->mutex);

	while (!join->terminato) {
		pthread_cond_wait(&join->cond, &join->mutex);
	}

	pthread_mutex_unlock(&join->mutex);
}



void  join_signal_done(join_t* join) {
	pthread_mutex_lock(&join->mutex);

	join->terminato = 1;
	pthread_cond_signal(&join->cond);

	pthread_mutex_unlock(&join->mutex);

}










