#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "header.h"


int main() {
	pthread_t tA, tB, tC, tD, tY;
	int *A, *B, *C, *D, *Y;

	printf("Grafo risolto in C\n");

	//A,B,C in parallelo
	pthread_create(&tA, NULL, calc_A, NULL);
        pthread_create(&tB, NULL, calc_B, NULL);
        pthread_create(&tC, NULL, calc_C, NULL);

	pthread_join(tA, (void**)&A);
        pthread_join(tB, (void**)&B);
        pthread_join(tC, (void**)&C);

	printf("A: %d\nB: %d\nC: %d\n",*A,*B,*C);

	//D dipende da B e C
	data_D d_data = {*B, *C};

	pthread_create(&tD, NULL, calc_D, &d_data);
	pthread_join(tD, (void**)&D);

	printf("\nD = A * B --> %d\n", *D);

	//Y dipende da A e D
	data_Y y_data = {*A, *D};

	pthread_create(&tY, NULL, calc_Y, &y_data);
	pthread_join(tY, (void**)&Y);

	printf("\nRisultato finale\n");
	printf("\nY = D - A --> %d\n", *Y);

	//per liberare memoria
	free(A); free(B); free(C); free(D); free(Y);

	return 0;
}
