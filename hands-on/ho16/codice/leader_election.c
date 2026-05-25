#include "common.h"

#include <sys/wait.h>
#include <unistd.h>

static void wait_children(int n_children) {
	for(int i = 0; i < n_children; i++) {
		int status;

		if(wait(&status) == -1) {
			die("wait");
		}

		if(!WIFEXITED(status) || WEXITSTATUS(status) != EXIT_SUCCESS) {
			fprintf(stderr, "Attenzione: un processo figlio è terminato con errore\n");
		}
	}
}

void start_floodmax_election(const char* graph_file, int diam) {
	Graph g;
	pid_t run_id = getpid();

	read_graph(graph_file, &g);
	create_fifos(&g, run_id);

	printf("==== FLOODMAX ====\n");
	printf("File grafo: %s\n", graph_file);
	printf("Diametro usato: %d\n", diam);
	print_graph(&g);

	for(int id = 1; id <= g.n_nodes; id++) {
		pid_t pid = fork();

		if(pid == -1) {
			remove_fifos(&g, run_id);
			die("fork");
		}

		if(pid == 0) {
			run_floodmax_process(id, &g, run_id, diam);
		}
	}

	wait_children(g.n_nodes);
	remove_fifos(&g, run_id);
}



void start_lcr_election(int ring_order) {
	Graph g;
	pid_t run_id = getpid();

	build_ring_graph(&g, ring_order);
	create_fifos(&g, run_id);

	printf("==== LCR ====\n");
	printf("Ring orientato di ordine: %d\n", ring_order);
	print_graph(&g);

	for(int id = 1; id <= g.n_nodes; id++) {
		pid_t pid = fork();

		if(pid == -1) {
                        remove_fifos(&g, run_id);
                        die("fork");
                }

                if(pid == 0) {
                        run_lcr_process(id, &g, run_id, ring_order);
                }

	}

	wait_children(g.n_nodes);
        remove_fifos(&g, run_id);
}


















