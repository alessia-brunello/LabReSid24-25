#include "common.h"

#include <unistd.h>

static Message floodmax_msg(FloodState state, int diam) {
	Message msg;

	msg.sender = state.my_id;
	msg.round = state.round;

	if(state.round < diam) {
		msg.kind = MESSAGE_VALUE;
		msg.data = state.max_id;
	} else {
		msg.kind = MESSAGE_NULL;
		msg.data = -1;
	}

	return msg;
}

static FloodState floodmax_stf(FloodState state, const Message received[], int n_received, int diam) {
	int largest = state.max_id;

	for(int i = 0; i < n_received; i ++) {
		if(received[i].kind == MESSAGE_VALUE && received[i].data > largest) {
			largest = received[i].data;
		}
	}

	state.max_id = largest;

	if(state.round < diam) {
		state.leader = LEADER_UNKNOWN;
	} else if(state.max_id == state.my_id) {
		state.leader = LEADER_TRUE;
	} else {
		state.leader = LEADER_FALSE;
	}

	state.round++;

	return state;
}


void run_floodmax_process(int id, const Graph* g, pid_t run_id, int diam) {
	FifoNet net;
	Message received[MAX_NODES];
	char msg_buffer[32];

	setvbuf(stdout, NULL, _IONBF, 0);

	open_node_fifos(id, g, run_id, &net);

	FloodState state;
	state.my_id = id;
	state.max_id = id;
	state.leader = LEADER_UNKNOWN;
	state.round = 0;

	for(int r = 0; r <= diam; r++) {
		Message out = floodmax_msg(state, diam);

		for(int i = 0; i < g->out_count[id]; i++) {
			int dst = g->out_neighbors[id][i];

			send_message(&net, dst, &out);

			printf("[FLOODMAX][round %d] P%d -> P%d : %s\n", state.round, id, dst,
						message_to_string(&out, msg_buffer, sizeof(msg_buffer)));
		}

		for(int i = 0; i < g->in_count[id]; i++) {
			int src = g->in_neighbors[id][i];

			receive_message(&net, src, &received[i]);

			printf("[FLOODMAX][round %d] P%d riceve da P%d : %s\n", state.round, id, src,
					message_to_string(&received[i], msg_buffer, sizeof(msg_buffer)));
		}

		state = floodmax_stf(state, received, g->in_count[id], diam);

		printf("[FLOODMAX][stato] P%d: round = %d, max_id = %d, leader = %s\n", id, state.round,
							state.max_id, leader_state_to_string(state.leader));
	}

	printf("[FINE][FLOODMAX] P%d: max_id = %d, leader = %s\n", id, state.max_id,
								leader_state_to_string(state.leader));

	close_node_fifos(&net);
	exit(EXIT_SUCCESS);
}















