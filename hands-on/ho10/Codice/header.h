#ifndef HEADER_H
#define HEADER_H

typedef struct {
        int B;
        int C;
} data_D;

typedef struct { 
        int A;
        int D;
} data_Y;

void* calc_A(void* arg);
void* calc_B(void* arg);
void* calc_C(void* arg);
void* calc_D(void* arg);
void* calc_Y(void* arg);

#endif
