#include "protocol.h"

#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

//legge dal socket un carattere alla volta dal socket e continua fino a \n perchè ogni comando è una riga

ssize_t recv_line(int fd, char* buffer, size_t max_len) {
    if (buffer == NULL || max_len == 0) {
        return -1;
    }

    size_t i = 0;

    while (i < max_len - 1) {   //lascia lo spazio per il terminatore di stringa \0
        char c;
        ssize_t n = recv(fd, &c, 1, 0);

        if (n == 0) {    //il client chiude la connessione
            if (i == 0) {
                return 0;
            }
            break;
        }

        if (n < 0) {
            if (errno == EINTR) { //se la chiamata di sistema è interrotta dal segnale, non è grave e si riprova
                continue;
            }
            return -1;
        }

        if (c == '\n') {
            break;
        }

        if (c != '\r') { //nei sistemi windows le righe terminano con \r\n quindi per semplicità si ignora \r
            buffer[i++] = c;
        }
    }

    buffer[i] = '\0';
    return (ssize_t)i;
}



//invia tutti i byte di un messaggio

int send_all(int fd, const char* buffer, size_t len) {
    if (buffer == NULL) {
        return -1;
    }

    size_t sent = 0;

    //viene fatta la send per inviare i byte finchè sent == len cioè quando tutto il messaggio è stato inviato
    while (sent < len) {
        ssize_t n = send(fd, buffer + sent, len - sent, 0);

        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }

        if (n == 0) {
            return -1;
        }

        sent += (size_t)n;
    }

    return 0;
}


/*invia la stringa e aggiunge \n così che se sul socket il mittente manda un messaggio senza \n, questa funzione la aggiunge e il destinatario potrà facilmente leggerlo tramite recv_line.

Per le stringhe multilinea di LIST si utilizza END come terminatore*/

int send_line(int fd, const char* message) {
    if (message == NULL) {
        return -1;
    }

    if (send_all(fd, message, strlen(message)) < 0) {
        return -1;
    }

    return send_all(fd, "\n", 1);
}
