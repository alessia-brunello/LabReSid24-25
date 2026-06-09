#include <stdio.h>
#include <stdlib.h>

#include "server.h"
#include "db.h"


int main(int argc, char* argv[]) {
	int port = 8080;

	if(argc >= 2) {
		port = atoi(argv[1]);

		if(port <= 0) {
			fprintf(stderr, "Invalid port number\n");
			return EXIT_FAILURE;
		}
	}

        //inizializza il database e crea la tabella dei ticket se non esiste ancora
	if(db_init("data/ticketflow.db") != 0) {
		fprintf(stderr, "Errore, Database initialization failed\n");
		return EXIT_FAILURE;
	}
	
        //avvia il server rest sulla porta scelta
	start_server(port);
	return EXIT_SUCCESS;

}
