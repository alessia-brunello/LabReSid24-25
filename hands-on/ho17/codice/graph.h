#ifndef GRAPH_H
#define GRAPH_H

#define NODES 4
#define EDGES 6
#define SOURCE 0 //nodo sorgente

#define INF 1000000000 //valore usato per indicare infinito, quando un nodo non conosce nessun cammino

#define FIFO_NAME_SIZE 64 //lunghezza nome fifo
#define RESULT_FIFO "fifo_result" //fifo usata dai figli per mandare il risultato al padre

//struttura di un arco pesato
typedef struct {
	int from;
	int to;
	int weight;
} Edge;

typedef struct {
	int from;
	int distance;
} Message;

typedef struct {
	int node;
	int parent;
	int distance;
} Result;


//extern serve a dire agli altri file che esiste un array graph ma si trova in un altro file
extern const Edge graph[EDGES];

void make_fifo_name(char* buffer, int from, int to);
int get_weight(int from, int to);

void create_fifos(void);
void remove_fifos(void);
void node_process(int node_id);

#endif









