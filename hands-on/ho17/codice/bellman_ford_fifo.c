#include <fcntl.h>
#include <sys/stat.h>

#include "graph.h"
#include "common.h"

static Message msg(int node_id, int distance) {
	Message message;

	message.from = node_id;
	message.distance = distance;

	return message;
}

static void stf(int node_id, const Message received[], int received_count, int* distance, int* parent) {
	//si assume che la distanza migliore sia quella già nota
	int best_distance = *distance;
	int best_parent = *parent;

	// si analizzano tutti i messaggi ricevuti dai vicini entranti
	for(int i = 0; i < received_count; i++) {
		int sender = received[i].from;
		int sender_distance = received[i].distance;

		//se il vicino ha distanza INF significa che nemmeno lui conosce un cammino valido
		if(sender_distance == INF) {
			continue;
		}

		int weight = get_weight(sender, node_id);

		//se l'arco non esiste si ignora il messaggio
		if(weight == INF) {
			continue;
		}

		//questa somma si ha se il vicino ha distanza sender_distance e l'arco pesa weight
		int candidate = sender_distance + weight;

		//se il nuovo cammino è migliore si aggiorna la scelta
		if(candidate < best_distance) {
			best_distance = candidate;
			best_parent = sender;
		}
	}


	//alla fine del round si aggiorna lo stato del nodo
	*distance = best_distance;
	*parent = best_parent;
}


void create_fifos(void) {
	char fifo_name[FIFO_NAME_SIZE];

	for(int i = 0; i < EDGES; i++) {
		make_fifo_name(fifo_name, graph[i].from, graph[i].to);

		unlink(fifo_name);

		//0666 indica i permessi di lettura/scrittura
		if(mkfifo(fifo_name, 0666) == -1) {
			die("mkfifo");
		}
	}

	unlink(RESULT_FIFO);

	if(mkfifo(RESULT_FIFO, 0666) == -1) {
		die("mkfifo result");
	}
}


void remove_fifos(void) {
	char fifo_name[FIFO_NAME_SIZE];

	for(int i = 0; i < EDGES; i++) {
		make_fifo_name(fifo_name, graph[i].from, graph[i].to);
		unlink(fifo_name);
	}

	unlink(RESULT_FIFO);
}


void node_process(int node_id) {
	int distance, parent;

	int input_fd[EDGES]; //file descriptor usato se l'arco i arriva al nodo corrente
	int output_fd[EDGES]; //file descriptor usato se l'arco i parte dal nodo corrente

	for(int i = 0; i < EDGES; i++) {
		input_fd[i] = -1;
		output_fd[i] = -1;
	}

	//il nodo sorgente conosce la distanza 0 da se stesso, gli altri partono da INF
	if(node_id == SOURCE) {
		distance = 0;
		parent = SOURCE;
	} else {
		distance = INF;
		parent = -1;
	}

	for(int i = 0; i < EDGES; i++) {
		char fifo_name[FIFO_NAME_SIZE];

		//se l'arco parte dal nodo corrente, il nodo corrente può scrivere sulla FIFO
		if(graph[i].from == node_id) {
			make_fifo_name(fifo_name, graph[i].from, graph[i].to);

			output_fd[i] = open(fifo_name, O_RDWR);

			if(output_fd[i] == -1) {
				die("open output fifo");
			}
		}

		//se l'arco arriva al nodo corrente, il nodo corrente può leggere dalla FIFO
		if(graph[i].to == node_id) {
			make_fifo_name(fifo_name, graph[i].from, graph[i].to);

			input_fd[i] = open(fifo_name, O_RDWR);

			if(input_fd[i] == -1) {
				die("open input fifo");
			}
		}

	}

	for(int round = 0; round < NODES; round++){
		Message received[EDGES];
		int received_count = 0;

		//FASE 1 del round:
		//il nodo invia la propria distanza a tutti i vicini raggiungibili tramite archi uscenti
		for(int i = 0; i < EDGES; i++) {
			if(graph[i].from == node_id) {
				Message message = msg(node_id, distance);

				write_exact(output_fd[i], &message, sizeof(Message));
			}
		}

		//FASE 2:
		//il nodo legge i messaggi provenienti da tutti gli archi entranti
		for(int i = 0; i < EDGES; i++) {
			if(graph[i].to == node_id) {
				Message message;

				read_exact(input_fd[i], &message, sizeof(Message));

				received[received_count] = message;
				received_count++;
			}
		}


		//FASE 3:
		//usando tutti i messaggi ricevuti si aggiorna lo stato del nodo
		stf(node_id, received, received_count, &distance, &parent);

	}

	//le FIFO vengono chiuse
	for(int i = 0; i < EDGES; i ++) {
		if(input_fd[i] != -1) {
			close(input_fd[i]);
		}

		if(output_fd[i] != -1) {
			close(output_fd[i]);
		}
	}

	//ogni nodo invia il proprio risultato al padre
	int result_fd = open(RESULT_FIFO, O_WRONLY);

	if(result_fd == -1) {
		die("open result fifo");
	}

	Result result;
	result.node = node_id;
	result.parent = parent;
	result.distance = distance;

	write_exact(result_fd, &result, sizeof(Result));

	close(result_fd);

	exit(EXIT_SUCCESS);

}






































