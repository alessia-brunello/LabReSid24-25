#include <stdio.h>
#include "graph.h"

/*
Numerazione dei nodi per facilitare la visualizzazione
0 = nodo sorgente
1 = nodo in alto a destra
2 = nodo in basso a sinistra
3 = nodo in basso a destra

Archi:
0 -> 1 (peso 6)
0 -> 2 (peso 1)
2 -> 3 (peso 2)
3 -> 0 (peso 3)
1 -> 3 (peso 1)
3 -> 1 (peso 2)
*/

const Edge graph[EDGES] = {
	{0, 1, 6},
        {0, 2, 1},
        {2, 3, 2},
        {3, 0, 3},
        {1, 3, 1},
        {3, 1, 2}
};

void make_fifo_name(char* buffer, int from, int to) {
	snprintf(buffer, FIFO_NAME_SIZE, "fifo_%d_%d", from, to);
}


//la funzione restituisce il peso di un arco, se l'arco non esiste restituisce INF
int get_weight(int from, int to) {
	for(int i = 0; i < EDGES; i++) {
		if(graph[i].from == from && graph[i].to == to) {
			return graph[i].weight;
		}
	}
	return INF;
}
