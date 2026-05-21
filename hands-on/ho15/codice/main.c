#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

void die(const char* msg) {
	perror(msg);
	exit(EXIT_FAILURE);
}

int main(int argc, char* argv[]) {

	//essendoci più processi che stampano contemporaneamente disattiviamo il buffering del pirntf
	setvbuf (stdout, NULL, _IONBF, 0);

	const char* graph_file = "graph.txt"; //file di default se non viene usato niente
	int rounds = -1; //l'utente non ha ancora specificato il numero di round
	int token = 100; //se l'utente non specifica il token verrà propagato il token 100

	//vengono assegnati gli argomenti  ai rispettivi
	if(argc >= 2) {
		graph_file = argv[1];
	}

	if(argc >= 3) {
		rounds = atoi(argv[2]);
	}

	if(argc >= 4) {
		token = atoi(argv[3]);
	}

	Graph g;
	read_graph(graph_file, &g);

	//se l'utente non specifica il numero di round o se non è valido viene utilizzato n_nodes
	if(rounds <= 0) {
		rounds = g.n_nodes;
	}

	//utilizziamo il pid  del processo corrente per creare nomi fifo univoci
	pid_t run_id = getpid();

	create_fifos(&g, run_id);

	printf("Avvio flooding con FIFO\n");
	printf("Nodi: %d,\nArchi direzionali: %d,\nRound: %d,\nToken: %d\n\n",g.n_nodes, g.n_edges, rounds, token);

	//per ogni nodo viene creato un processo
	for(int id = 1; id <= g.n_nodes; id++) {
		pid_t pid = fork();

		if(pid == -1) {
			die("fork");
		}

		/*è il processo figlio,  che diventa l'agente id quindi il processo apre la fifo, inizializza lo stato
		eseue i round del flooding, invia e riceve messaggi e termina
		*/
		if(pid == 0) {
			agent_process(id, &g, run_id, rounds, token);
		}
	}

	//il processo padre deve aspettare tutti i figli
	for(int i = 0; i < g.n_nodes; i++) {
		wait(NULL);
	}

	//quando tutti i figli terminano le FIFO possono essere eliminate
	remove_fifos(&g,run_id);

	printf("\nFlooding terminato. FIFO rimosse.\n");

	return EXIT_SUCCESS;

}













