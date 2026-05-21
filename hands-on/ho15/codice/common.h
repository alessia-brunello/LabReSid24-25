#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>

#define MAX_NODES 32
#define MAX_EDGES 256
#define MAX_DEG 32 //massimo numero di vicini entranti o uscenti
#define FIFO_NAME_SIZE 128 //dimensione massima della stringa che contiene il nome della fifo

//struttura dati di un arco, è formato da un nodo sorgnte e uno di destinazione
typedef struct {
	int src;
	int dst;
} Edge;


//struttura dati del grafo
typedef struct {
	int n_nodes;
	int n_edges;
	Edge edges[MAX_EDGES];

	//vicini uscenti
	int out_count[MAX_NODES + 1]; //quanti archi uscenti ha il nodo
	int out_neighbors[MAX_NODES + 1][MAX_DEG]; //lista dei nodi raggiungibili

	//vicini entranti
	int in_count[MAX_NODES + 1]; //quanti archi entranti ha il nodo
	int in_neighbors[MAX_NODES + 1][MAX_DEG]; //lista dei nodi che possono inviare messaggi
} Graph;


typedef struct {
	int sender; //chi ha inviato il messaggio
	int is_null;
	int data; //contenuto del messaggio (token del flooding)
} Message;


//stato locale di un agente
typedef struct {
	int parent; //nodo da cui l'agente ha ricevuto il token la prima volta
	int has_data; //se possiede già il token
	int data; //token
	bool snd_flag; //indica se il token deve essere mandato ai vicini
} State;


//stampa l'errore e termina il programma
void die(const char* msg);

void read_graph(const char* filename, Graph* g);

void create_fifos(const Graph* g, pid_t run_id);
void remove_fifos(const Graph* g, pid_t run_id);

void agent_process(int id, const Graph* g, pid_t run_id, int rounds, int token);

#endif
