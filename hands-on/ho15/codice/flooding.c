#include "common.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

//crea il nome dlla fifo
static void fifo_name(char* buffer, size_t size, pid_t run_id, int src, int dst) {
	snprintf(buffer, size, "/tmp/lab15_fifo_%ld_%d_%d", (long)run_id, src, dst);
}

//crea una fifo per ogni arco del grafo
void create_fifos(const Graph* g, pid_t run_id) {
	for(int i = 0; i < g->n_edges; i++) {
		char name[FIFO_NAME_SIZE];

		fifo_name(name, sizeof(name), run_id, g->edges[i].src, g->edges[i].dst);

		//cancella fifo vecchie con lo stesso nome, se prima un esecuzione è andata male
		unlink(name);

		if(mkfifo(name, 0666) == -1) {
			die("mkfifo");
		}
	}
}


//rimuove le fifo create
void remove_fifos(const Graph* g, pid_t run_id) {
	for(int i = 0; i < g->n_edges; i++) {
		char name[FIFO_NAME_SIZE];

		fifo_name(name, sizeof(name), run_id, g->edges[i].src, g->edges[i].dst);

		unlink(name);
	}
}



/*siccome la write non sempre scrive tutto in una sola volta,
quindi questa funzione ripete la write finchè tutti i byte sono stati scritti
*/

static void write_exact(int fd, const void* buffer, size_t size) {
	const char* p = (const char* )buffer;

	size_t written = 0;

	while(written < size) {
		ssize_t n = write(fd, p + written, size - written);

		if (n == -1) {
			if(errno == EINTR) {
				continue;
			}

			die("write");
		}

		written += (size_t)n;
	}

}


//anche read non legge sempre tutto in una sola volta per questo la funzione ripete read finchè il messaggio è completo
static void read_exact(int fd, void* buffer, size_t size){
	char* p = (char* )buffer;
	size_t received = 0;

	while(received < size) {
		ssize_t n = read(fd, p + received, size - received);

		if(n == -1) {
			if(errno == EINTR) {
				continue;
			}
			die("read");
		}

		if(n == 0) {
			fprintf(stderr, "FIFO chiusa prima di ricevere un messaggio completo.\n");
			exit(EXIT_FAILURE);
		}

		received += (size_t)n;
	}
}



//w è lo stato locale dell'agente, id dell'agente che sta inviando, vicino destinatario
static Message msg_function(const State* w, int my_id, int destination) {
	Message m;

	m.sender = my_id;
	m.is_null = 1;
	m.data = 0;

	//se l'agente ha il token, snd_flag è true e il destinatario non è parent allora manda data altrimenti manda NULL
	if(w->has_data && w->snd_flag && w->parent != destination) {
		m.is_null = 0;
		m.data = w->data;
	}

	return m;
}


//la funzione prende lo stato corrente e i messaggi ricevuti nelo round e restituisce il nuovo stato dell'agente
static State stf_function(State w, const Message received[], int received_count){
	State next = w;

	//se l'agente non ha ancora il token controlla tutti i messaggi ricevuti nel round
	if(!w.has_data) {
		int found_non_null = 0;
		int chosen_parent = 0;
		int chosen_data = 0;

                /*se trova almeno un messaggio non nullo sceglie un parent
		(mittende con id più piccolo) e salva il token*/
		
		for(int i = 0; i < received_count; i++) {
			if(!received[i].is_null) {
				if(!found_non_null || received[i].sender < chosen_parent) {
					found_non_null = 1;
					chosen_parent = received[i].sender;
					chosen_data = received[i].data;
				}
			}
		}

		//l'agente non aveva il token ma riceve almeno un DATA
		if(found_non_null) {
			next.parent = chosen_parent;	//salvo da chi l'ho ricevuto
			next.has_data = 1;
			next.data = chosen_data;	//lo salvo come data
			next.snd_flag = true;		//al prossimo round devo propagarlo
		}

		//l'agente non aveva il token e riceve solo NULL vuol dire che non ha ricevuto nulla di utile, è senza token e non deve inviare nulla
		else {
			next.parent = 0;
			next.has_data = 0;
			next.data = 0;
			next.snd_flag = false;
		}
	}

	//caso in cui l'agente ha già il token quindi mantiene data, parent e dopo aver inviato il token disattiva snd_flag
	else {
		next.parent = w.parent;
		next.has_data = w.has_data;
		next.data = w.data;
		next.snd_flag = false;
	}

	return next;
}



//funzione di stampa per rendere il tutto più leggibile
static void print_message(const char* action, int round, int my_id, int other_id, Message m) {
	if(m.is_null) {
		printf("[round %d] agente %d %s %d: NULL\n", round, my_id, action, other_id);
	}else {
		printf("[round %d] agente %d %s %d: DATA = %d\n", round, my_id, action, other_id, m.data);
	}
}

void agent_process(int id, const Graph* g, pid_t run_id, int rounds, int token) {
	setvbuf(stdout, NULL, _IONBF, 0);

	int in_fds[MAX_DEG];
	int out_fds[MAX_DEG];

	//apertura fifo entranti (da cui l'agente deve ricevere)
	for(int i = 0; i < g->in_count[id]; i++) {
		int src = g->in_neighbors[id][i];
		char name[FIFO_NAME_SIZE];

		fifo_name(name, sizeof(name), run_id, src, id);

		//O_RDWR serve a non fare bloccare l'apertura
		in_fds[i] = open(name, O_RDWR);

		if(in_fds[i] == -1) {
			die("open FIFO input");
		}
	}

	//apertura fifo uscenti (sulle quali deve scrivere)
	for(int i = 0; i < g->out_count[id]; i++) {
		int dst = g->out_neighbors[id][i];
		char name[FIFO_NAME_SIZE];

		fifo_name(name, sizeof(name), run_id, id, dst);

		out_fds[i] = open(name, O_RDWR);

		if(out_fds[i] == -1) {
			die("open FIFO output");
		}
	}

	State state;

	//se è il primo nodo parte già con il token
	if(id == 1) {
		state.parent = 1;
		state.has_data = 1;
		state.data = token;
		state.snd_flag = true;
	//gli altri non hanno nulla ancora
	}else {
		state.parent = 0;
		state.has_data = 0;
		state.data = 0;
		state.snd_flag = false;
	}

	printf("[init] agente %d: parent = %d, data = %s, snd_flag = %s\n", id, state.parent, state.has_data ? "TOKEN" : "NULL",
									state.snd_flag ? "true" : "false");

	//fasi dei round
	for(int round = 0; round < rounds; round++) {

		//Fase 1: invio del messaggio calcolato tramite msg_function che può essere DATA o NULL per ogni vicino uscente
		for(int i = 0; i < g->out_count[id]; i++) {
			int dst = g->out_neighbors[id][i];

			Message m = msg_function(&state, id, dst);

			write_exact(out_fds[i], &m, sizeof(m));
			print_message("manda a", round, id, dst, m);
		}

		//Fase 2: ricezione: l'agente legge un messaggio da ogni FIFO entrante, raccoglie tutti i messaggi del round
		Message received[MAX_DEG];

		for(int i = 0; i < g->in_count[id]; i++) {
			int src = g->in_neighbors[id][i];

			read_exact(in_fds[i], &received[i], sizeof(received[i]));
			print_message("riceve da ", round, id, src, received[i]);
		}

		//Fase 3: transizione di stato
		//dopo che l'agente riceve tutti i messaggi aggiorna lo stato con stf
		state = stf_function(state, received, g->in_count[id]);

		printf("[round %d] agente %d nuovo stato: parent = %d, data = %s, snd_flag = %s\n", round, id, state.parent,
									state.has_data ? "TOKEN" : "NULL",
									state.snd_flag ? "true" : "false");
	}
	
	if(id == 1) {
	        printf("[fine] agente %d: parent = %d, data = %s, radice dell'albero\n\n", id, state.parent, state.has_data ? "TOKEN" : "NULL");	        
	}else if(state.has_data) {
	        printf("[fine] agente %d: parent = %d, data = TOKEN, %d -> %d\n\n", id, state.parent, state.parent, id);
	}else {
	      	printf("[fine] agente %d: parent = %d, data = NULL, arco non presente\n\n", id, state.parent);  
	}

	//chiusura del file descriptor delle FIFO
	for(int i = 0; i < g->in_count[id]; i++) {
		close(in_fds[i]);
	}

	for(int i = 0; i < g->out_count[id]; i++) {
		close(out_fds[i]);
	}

	exit(EXIT_SUCCESS);


}






















