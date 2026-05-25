#include "common.h"

#include <string.h>

void die(const char* msg) {
	perror(msg);
	exit(EXIT_FAILURE);
}

const char* leader_state_to_string(LeaderState state) {
	switch(state) {
		case LEADER_TRUE:
			return "true";
		case LEADER_FALSE:
			return "false";
		case LEADER_UNKNOWN:
		default:
			return "unknown";
	}
}


const char* message_to_string(const Message* msg, char* buffer, size_t size) {
	if(msg->kind == MESSAGE_NULL) {
		snprintf(buffer, size, "null");
	} else {
		snprintf(buffer, size, "%d", msg->data);
	}

	return buffer;
}

static void clear_graph(Graph* g){
	memset(g, 0, sizeof(*g));
}

static void add_edge(Graph* g, int src, int dst) {
	if(g->n_edges >= MAX_EDGES) {
		fprintf(stderr, "Errore: troppi archi. Massimo consentito: %d\n", MAX_EDGES);
		exit(EXIT_FAILURE);
	}

	if(src < 1 || src > g->n_nodes || dst < 1 || dst > g->n_nodes) {
		fprintf(stderr, "Errore: arco non valido %d -> %d\n", src, dst);
		exit(EXIT_FAILURE);
	}

	if(g->out_count[src] >= MAX_NODES || g->in_count[dst] >= MAX_NODES) {
		fprintf(stderr, "Errore: troppi vicini per un nodo\n");
		exit(EXIT_FAILURE);
	}

	g->edges[g->n_edges].src = src;
	g->edges[g->n_edges].dst = dst;
	g->n_edges++;

	g->out_neighbors[src][g->out_count[src]++] = dst;
        g->in_neighbors[dst][g->in_count[dst]++] = src;
}


void read_graph(const char* filename, Graph* g) {
	clear_graph(g);

	FILE* fp = fopen(filename, "r");
	if(fp == NULL) {
		die("fopen graph.txt");
	}

	int declared_edges;

	if(fscanf(fp, "%d %d", &g->n_nodes, &declared_edges) != 2) {
		fprintf(stderr, "Errore: formato del file %s non valido\n", filename);
		fclose(fp);
		exit(EXIT_FAILURE);
	}

	if(g->n_nodes < 1 || g->n_nodes > MAX_NODES){
		fprintf(stderr, "Errore: numero di nodi non valido: %d\n", g->n_nodes);
		fclose(fp);
                exit(EXIT_FAILURE);
	}

	for(int i = 0; i < declared_edges; i++) {
		int src, dst;

		if(fscanf(fp, "%d %d", &src, &dst) != 2) {
			fprintf(stderr, "Errore: impossibile leggere l'arco%d\n", i +1);
			fclose(fp);
                	exit(EXIT_FAILURE);
		}

		add_edge(g, src, dst);
	}

	fclose(fp);
}

void build_ring_graph(Graph* g, int n) {
	clear_graph(g);

	if(n < 2 || n > MAX_NODES) {
		fprintf(stderr, "Errore: ordine del ring non valido: %d\n", n);
		exit(EXIT_FAILURE);
	}

	g->n_nodes = n;

	for(int i = 1; i <= n; i++){
		int dst = (i == n) ? 1 : i +1;
		add_edge(g, i, dst);
	}
}


void print_graph(const Graph* g) {
	printf("Grafo con %d nodi e %d archi: \n", g->n_nodes, g->n_edges);

	for(int i = 0; i < g->n_edges; i++) {
		printf(" %d -> %d\n", g->edges[i].src, g->edges[i].dst);
	}
}
