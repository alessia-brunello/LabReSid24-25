#include "common.h"

#include <unistd.h>

static Message lcr_msg(LcrState state, int round) {
	Message msg;

	msg.sender = state.my_id;
	msg.round = round;

	if(state.snd_flag) {
		msg.kind = MESSAGE_VALUE;
		msg.data = state.max_id;
	} else {
		msg.kind = MESSAGE_NULL;
		msg.data = -1;
	}
	return msg;
}


static LcrState lcr_stf(LcrState state, const Message* received) {
	if(received->kind == MESSAGE_NULL || received->data < state.my_id) {
		state.snd_flag = 0;
	} else if(received->data == state.my_id) {
		state.leader = LEADER_TRUE;
		state.snd_flag = 0;
	} else {
		state.max_id = received->data;
		state.leader = LEADER_FALSE;
		state.snd_flag = 1;
	}

	return state;
}


void run_lcr_process(int id, const Graph* g, pid_t run_id, int rounds) {
	FifoNet net;
	char msg_buffer[32];

	setvbuf(stdout, NULL, _IONBF, 0);

	open_node_fifos(id, g, run_id, &net);

	if(g->in_count[id] != 1 || g->out_count[id] != 1) {
		fprintf(stderr, "Errore: LCR richiede un ring. P%d deve avere un arco entrante e uno uscente.\n", id);

		close_node_fifos(&net);
		exit(EXIT_FAILURE);
	}

	int prev = g->in_neighbors[id][0];
	int next = g->out_neighbors[id][0];

	LcrState state;
	state.my_id = id;
	state.max_id = id;
	state.leader = LEADER_UNKNOWN;
	state.snd_flag = 1;

	for(int r = 0; r < rounds; r++) {
		Message out = lcr_msg(state, r);
		Message in;

		send_message(&net, next, &out);

		printf("[LCR][round %d] P%d -> P%d : %s\n", r, id, next,
				message_to_string(&out, msg_buffer, sizeof(msg_buffer)));

		receive_message(&net, prev, &in);

		printf("[LCR][round %d] P%d riceve da P%d : %s\n", r, id, prev,
				message_to_string(&in, msg_buffer, sizeof(msg_buffer)));

		state = lcr_stf(state, &in);

		printf("[LCR][stato] P%d: max_id = %d, leader = %s, snd_flag = %d\n",
			id, state.max_id, leader_state_to_string(state.leader), state.snd_flag);
	}

	printf("[FINE][LCR] P%d: max_id = %d, leader = %s\n", id, state.max_id,
						leader_state_to_string(state.leader));

	close_node_fifos(&net);
	exit(EXIT_SUCCESS);
}




























