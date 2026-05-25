#include "common.h"


void die(const char* message) {
	perror(message);
	exit(EXIT_FAILURE);
}


//ripete la read finchè non riceve tutto
void read_exact(int fd, void* buffer, size_t size) {
	char* ptr = buffer;
	size_t total_read = 0;

	while (total_read < size) {
		ssize_t bytes_read = read(fd, ptr + total_read, size - total_read);

		if(bytes_read == -1) {
			if(errno == EINTR) { //EINTR significa che  si è stati interrotti da un segnale ma in questo caso non è un errore quindi si può continuare
				continue;
			}

			die("read");
		}

		if(bytes_read == 0) {
			fprintf(stderr, "Errore: FIFO dei risultati chiusa troppo presto\n");
			exit(EXIT_FAILURE);
		}

		total_read += bytes_read;
	}
}

//stessa cosa per la write
void write_exact(int fd, const void* buffer, size_t size) {
	const char* ptr = buffer;
	size_t total_written = 0;

	while (total_written < size) {
		ssize_t written = write(fd, ptr + total_written, size - total_written);

		if(written == -1) {
			if(errno == EINTR) {
				continue;
			}

			die("write");
		}

		total_written += written;
	}
}

