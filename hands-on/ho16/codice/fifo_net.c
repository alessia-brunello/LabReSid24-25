#include "common.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

static void fifo_name(char* buffer, size_t size, pid_t run_id, int src, int dst) {
	snprintf(buffer, size, "/tmp/ho16_fifo_%ld_%d_%d", (long) run_id, src, dst);
}

void create_fifos(const Graph* g, pid_t run_id) {
	char path[FIFO_NAME_SIZE];

	for(int i = 0; i < g->n_edges; i ++) {
		fifo_name(path, sizeof(path), run_id, g->edges[i].src, g->edges[i].dst);

		unlink(path);

		if(mkfifo(path, 0600) == -1) {
			die("mkfifo");
		}
	}
}


void remove_fifos(const Graph* g, pid_t run_id) {
	char path[FIFO_NAME_SIZE];

	for(int i = 0; i < g->n_edges; i++) {
		fifo_name(path, sizeof(path), run_id, g->edges[i].src, g->edges[i].dst);
		unlink(path);
	}
}

void open_node_fifos(int id, const Graph* g, pid_t run_id, FifoNet* net) {
	char path[FIFO_NAME_SIZE];

	for(int i = 0; i <= MAX_NODES; i++) {
		net->in_fd[i] = -1;
		net->out_fd[i] = -1;
	}

	for(int i = 0; i < g->n_edges; i++) {
		int src = g->edges[i].src;
		int dst = g->edges[i].dst;

		if(src == id) {
			fifo_name(path, sizeof(path), run_id, src, dst);

			net->out_fd[dst] = open(path, O_RDWR);
			if(net->out_fd[dst] == -1) {
				die("open fifo uscente");
			}
		}

		if(dst == id) {
			fifo_name(path, sizeof(path), run_id, src, dst);

			net->in_fd[src] = open(path, O_RDWR);
			if(net->in_fd[src] == -1) {
				die("open fifo entrante");
			}
		}
	}
}


void close_node_fifos(FifoNet* net) {
	for(int i = 0; i <= MAX_NODES; i++) {
		if(net->in_fd[i] != -1) {
			close(net->in_fd[i]);
			net->in_fd[i] = -1;
		}

		if(net->out_fd[i] != -1) {
			close(net->out_fd[i]);
			net->out_fd[i] = -1;
		}
	}
}


static void write_full(int fd, const void* buffer, size_t size) {
	const char* p = (const char*) buffer;
	size_t total = 0;

	while(total < size) {
		ssize_t n = write(fd, p + total, size - total);

		if(n == -1) {
			if(errno == EINTR) {
				continue;
			}

			die("write fifo");
		}

		total += (size_t) n;
	}
}


static void read_full(int fd, void* buffer, size_t size) {
	char* p = (char*) buffer;
        size_t total = 0;

        while(total < size) {
                ssize_t n = read(fd, p + total, size - total);

                if(n == -1) {
                        if(errno == EINTR) {
                                continue;
                        }

                        die("read fifo");
                }

		if(n == 0) {
			fprintf(stderr, "Errore: FIFO chiusa durante la lettura\n");
			exit(EXIT_FAILURE);
		}

                total += (size_t) n;
        }

}


void send_message(FifoNet* net, int dst, const Message* msg) {
	if(dst < 1 || dst > MAX_NODES || net->out_fd[dst] == -1) {
		fprintf(stderr, "Errore: FIFO uscente verso P%d non aperta\n", dst);
		exit(EXIT_FAILURE);
	}

	write_full(net->out_fd[dst], msg, sizeof(*msg));
}

void receive_message(FifoNet* net, int src, Message* msg) {
	if(src < 1 || src > MAX_NODES || net->in_fd[src] == -1) {
		fprintf(stderr, "Errore: FIFO entrante da P%d non aperta\n", src);
		exit(EXIT_FAILURE);
	}

	read_full(net->in_fd[src], msg, sizeof(*msg));
}





















