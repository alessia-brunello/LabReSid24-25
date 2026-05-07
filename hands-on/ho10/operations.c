#include <stdlib.h>
#include "header.h"

void* calc_A(void* arg) {
	int* res = malloc(sizeof(int));
	*res = 2 * 6;
	return res;
}

void* calc_B(void* arg) {
	int* res = malloc(sizeof(int));
	*res = 1 + 4;
	return res;
}

void* calc_C(void* arg) {
	int* res = malloc(sizeof(int));
	*res = 5 - 2;
	return res;
}


void* calc_D(void* arg) {
	data_D* d = (data_D*)arg;

	int* res = malloc(sizeof(int));
	*res = d->B * d->C;

	return res;
}

void* calc_Y(void* arg) {
	data_Y* y = (data_Y*) arg;

	int* res = malloc(sizeof(int));
	*res = y->A + y->D;
	return res;
}
