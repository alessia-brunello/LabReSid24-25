#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//questo file si occupa solo di leggere il grafo, quindi di trasformare graph.txt in una struttura utilizzabile da C
void read_graph(const char* filename, Graph* g) {
	memset(g, 0, sizeof(*g)); //azzeramento della struttura

	FILE* fp = fopen(filename, "r");
	if(fp == NULL) {
		die("fopen graph.txt");
	}

	if(fscanf(fp, "%d %d", &g->n_nodes, &g->n_edges) != 2) {
		fprintf(stderr, "Formato graph.txt non valido.\n");
		exit(EXIT_FAILURE);
	}

	if(g->n_nodes <= 0 || g->n_nodes > MAX_NODES) {
		fprintf(stderr, "Numero di nodi non valido. Max = %d\n", MAX_NODES);
		exit(EXIT_FAILURE);
	}

	if(g->n_edges < 0 || g->n_edges > MAX_EDGES) {
		fprintf(stderr, "Numero di archi non valido. Max = %d\n", MAX_EDGES);
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i <  g->n_edges; i++) {
		int src, dst;

		if(fscanf(fp, "%d %d", &src, &dst) != 2){
			fprintf(stderr, "Errore nella lettura dell'arco %d\n", i + 1);
			exit(EXIT_FAILURE);
		}

		if(src < 1 || src > g->n_nodes || dst < 1 || dst > g->n_nodes) {
			fprintf(stderr, "Arco non valido: %d -> %d\n", src, dst);
			exit(EXIT_FAILURE);
		}

		if(g->out_count[src] >= MAX_DEG || g->in_count[dst] >= MAX_DEG) {
			fprintf(stderr, "Grado troppo alto. Aumentare MAX_DEG.\n");
			exit(EXIT_FAILURE);
		}

		g->edges[i].src = src;
		g->edges[i].dst = dst;

		g->out_neighbors[src][g->out_count[src]++] = dst;
		g->in_neighbors[dst][g->in_count[dst]++] = src;
	}

	fclose(fp);
}


















