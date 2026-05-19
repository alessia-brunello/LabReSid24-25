#include "barrier.h"


int barrier_init(barrier_t* barrier, int total){
	int ret;

	barrier->arrivati = 0;
	barrier->total = total;
	barrier->aperta = 0;

	ret = pthread_mutex_init(&barrier->mutex, NULL);
	if(ret != 0) {
		return ret;
	}

	ret = pthread_cond_init(&barrier->cond, NULL);
	if(ret != 0) {
		pthread_mutex_destroy(&barrier->mutex);
		return ret;
	}
	return 0;
}

void barrier_destroy(barrier_t* barrier) {
	pthread_cond_destroy(&barrier->cond);
	pthread_mutex_destroy(&barrier->mutex);
}

void barrier_wait(barrier_t* barrier, int id_thread) {
	pthread_mutex_lock(&barrier->mutex);

	barrier->arrivati++;
	
	printf("[Barriera] thread %d arrivato alla barriera(%d/%d)\n", id_thread, barrier->arrivati, barrier->total);

	if (barrier->arrivati == barrier->total) {
	//se è l'ultimo thread apre la barriera e sveglia tutti
	        printf("\n[Thread %d] sono l'ultimo; sblocco la barriera\n", id_thread);
	        barrier->aperta = 1;
		pthread_cond_broadcast(&barrier->cond);
	}else {
	//altrimenti i thread aspettano finchè la barriera non è aperta

		while(!barrier->aperta) {
			pthread_cond_wait(&barrier->cond, &barrier->mutex);
		}
		printf("\n[Thread %d] risvegliato; la barriera è aperta\n", id_thread);
	}

	pthread_mutex_unlock(&barrier->mutex);

}
















