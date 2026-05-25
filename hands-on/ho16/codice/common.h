#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <sys/types.h>

#define MAX_NODES 32
#define MAX_EDGES 128
#define FIFO_NAME_SIZE 128

typedef enum {
	MESSAGE_VALUE = 0,
	MESSAGE_NULL = 1
}MessageKind;

typedef enum {
	LEADER_UNKNOWN = -1,
	LEADER_FALSE = 0,
	LEADER_TRUE = 1
}LeaderState;

typedef struct {
	int src;
	int dst;
} Edge;

typedef struct {
	int n_nodes;
	int n_edges;

	Edge edges[MAX_EDGES];

	int out_neighbors[MAX_NODES + 1][MAX_NODES];
	int out_count[MAX_NODES + 1];

	int in_neighbors[MAX_NODES + 1][MAX_NODES];
        int in_count[MAX_NODES + 1];
} Graph;

typedef struct {
	int sender;
	int round;
	MessageKind kind;
	int data;
} Message;


typedef struct {
	int in_fd[MAX_NODES + 1];
	int out_fd[MAX_NODES + 1];
} FifoNet;

typedef struct {
	int my_id;
	int max_id;
	LeaderState leader;
	int round;
} FloodState;

typedef struct {
	int my_id;
	int max_id;
	LeaderState leader;
	int snd_flag;
} LcrState;

void die(const char* msg);
const char* leader_state_to_string(LeaderState state);
const char* message_to_string(const Message* msg, char* buffer, size_t size);

void read_graph(const char* filename, Graph* g);
void build_ring_graph(Graph* g, int n);
void print_graph(const Graph* g);


void create_fifos(const Graph* g, pid_t run_id);
void remove_fifos(const Graph* g, pid_t run_id);
void open_node_fifos(int id, const Graph* g, pid_t run_id, FifoNet* net);
void close_node_fifos(FifoNet* net);

void send_message(FifoNet* net, int dst, const Message* msg);
void receive_message( FifoNet* net, int src, Message* msg);

void start_floodmax_election(const char* graph_file, int diam);
void start_lcr_election(int ring_order);

void run_floodmax_process(int id, const Graph* g, pid_t run_id, int diam);
void run_lcr_process(int id, const Graph* g, pid_t run_id, int rounds);

#endif





































