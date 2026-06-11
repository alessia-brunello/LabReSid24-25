CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -pthread -MMD -MP
SERVER_TARGET = ticketflow
CLIENT_TARGET = ticketflow_client

SERVER_SRC = src/*.c
CLIENT_SRC = client/client.c

SERVER_OBJS = src/main.o src/server.o src/http.o src/router.o src/db.o src/utils.o
CLIENT_OBJS = client/client.o

#MMD e MP dicono a gcc di generare automaticamente i file .d cioè i file di dipendenze che indicano quali .h sono inclusi da ogni .c
DEPS = $(SERVER_OBJS:.o=.d) $(CLIENT_OBJS:.o=.d) 

all: $(SERVER_TARGET) $(CLIENT_TARGET)

$(SERVER_TARGET): $(SERVER_OBJS)
	$(CC) $(CFLAGS) -o $(SERVER_TARGET) $(SERVER_OBJS) -lsqlite3

$(CLIENT_TARGET): $(CLIENT_OBJS)
	$(CC) $(CFLAGS) -o $(CLIENT_TARGET) $(CLIENT_OBJS) 
	
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(SERVER_TARGET)
	./$(SERVER_TARGET) 8080

client: $(CLIENT_TARGET)
	./$(CLIENT_TARGET)

test:
	./tests/test_api.sh

load_test:
	./tests/load_test.sh
	
inspect: $(SERVER_TARGET)
	@echo "== file =="
	@file $(SERVER_TARGET)
	@echo "\nlibrerie dinamiche: ldd"
	@ldd $(SERVER_TARGET)
	
clean:
	rm -f $(SERVER_OBJS) $(CLIENT_OBJS) $(DEPS) $(SERVER_TARGET) $(CLIENT_TARGET)

clean-db:
	rm -f data/*.db data/*.db-shm data/*.db-wal

-include $(DEPS)

.PHONY: all run client test load_test inspect clean clean-db
