CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -pthread -g

SERVER_OBJS = server.o bank.o protocol.o
CLIENT_OBJS = client.o protocol.o

.PHONY: all clean run-server run-client

all: server client

server: $(SERVER_OBJS)
	$(CC) $(CFLAGS) -o server $(SERVER_OBJS)

client: $(CLIENT_OBJS)
	$(CC) $(CFLAGS) -o client $(CLIENT_OBJS)

server.o: server.c bank.h common.h protocol.h
	$(CC) $(CFLAGS) -c server.c

client.o: client.c common.h protocol.h
	$(CC) $(CFLAGS) -c client.c

bank.o: bank.c bank.h common.h
	$(CC) $(CFLAGS) -c bank.c

protocol.o: protocol.c protocol.h
	$(CC) $(CFLAGS) -c protocol.c

clean:
	rm -f *.o server client
