#include <sys/wait.h>
#include <fcntl.h>

#include "graph.h"
#include "common.h"

//stampa i risultati finali ordinati per nodo
static void print_results(const Result results[]) {
	printf("\nRisultati Bellman-Ford distribuito\n");
	printf("--------------------------------------\n");

	for(int i = 0; i < NODES; i++) {
		printf("Nodo %d: ", i);

		if(results[i].distance == INF) {
			printf("distanza = INF, padre = -\n");
		} else {
			printf("distanza = %d, padre = %d\n", results[i].distance, results[i].parent);
		}
	}
}


int main(void) {
	pid_t pids[NODES];
	Result results[NODES];

	create_fifos();

	//creazione del processo figlio per ogni nodo, ogni figlio esegue node_process(i)
	for(int i = 0; i < NODES; i++) {
		pids[i] = fork();

		if(pids[i] == -1) {
			remove_fifos();
			die("fork");
		}

		if(pids[i] == 0) {
			node_process(i);
		}
	}

	//codice eseguito dal padre: apre la fifo dei risultati e aspetta che i figli mandino la distanza
	int result_fd = open(RESULT_FIFO, O_RDONLY);

	if(result_fd == -1) {
		remove_fifos();
		die("open result fifo");
	}

	//si legge un risultato per ogni nodo
	for(int i = 0; i < NODES; i++) {
		Result result;

		read_exact(result_fd, &result, sizeof(Result));

		results[result.node] = result;
	}

	close(result_fd);

	//terminazione di tutti i figli
	for(int i = 0; i < NODES; i++) {
		waitpid(pids[i], NULL, 0);
	}

	print_results(results);

	remove_fifos();

	return 0;
}




















