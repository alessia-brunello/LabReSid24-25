#include "common.h"

#include <string.h>

#define DEFAULT_GRAPH_FILE "graph.txt"
#define DEFAULT_FLOODMAX_DIAM 3
#define DEFAULT_RING_ORDER 5

static void usage(const char* progname) {
	fprintf(stderr, "Uso: \n" " %s floodmax [graph.txt] [diametro]\n"
			" %s lcr [ordine_ring]\n" "Esempi:\n" " %s floodmax graph.txt 3\n"
			"%s lcr 5\n", progname, progname, progname, progname);
}

int main(int argc, char* argv[]) {
	setvbuf(stdout, NULL, _IONBF, 0);

	if(argc < 2) {
		usage(argv[0]);
		return EXIT_FAILURE;
	}

	if(strcmp(argv[1], "floodmax") == 0) {
		const char* graph_file = (argc >= 3) ? argv[2] : DEFAULT_GRAPH_FILE;
		int diam = (argc >= 4) ? atoi(argv[3]) : DEFAULT_FLOODMAX_DIAM;

		if(diam < 1) {
			fprintf(stderr, "Errore: il diametro deve essere positivo\n");
			return EXIT_FAILURE;
		}

		start_floodmax_election(graph_file, diam);
	} else if(strcmp(argv[1], "lcr") == 0) {
		int ring_order = (argc >= 3) ? atoi(argv[2]) : DEFAULT_RING_ORDER;

		if(ring_order < 2) {
			fprintf(stderr, "Errore: l'ordine del ring deve essere almeno 2\n");
			return EXIT_FAILURE;
		}

		start_lcr_election(ring_order);
	} else {
		usage(argv[0]);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
