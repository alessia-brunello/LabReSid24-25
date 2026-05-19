CC = gcc
CFLAGS = -g -Wall -Wextra -pthread
LDFLAGS = -pthread

TARGETS = no_sync_app mutex_app semaphore_app

NO_SYNC_OBJS = no_sync/main.o no_sync/no_sync.o

MUTEX_OBJS = mutex/main.o mutex/mutex.o

SEMAPHORE_OBJS = semaphore/main.o semaphore/semaphore.o

all: $(TARGETS)

no_sync_app: $(NO_SYNC_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

mutex_app: $(MUTEX_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

semaphore_app: $(SEMAPHORE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

no_sync/%.o: no_sync/%.c no_sync/no_sync.h header.h
	$(CC) $(CFLAGS) -c $< -o $@

mutex/%.o: mutex/%.c mutex/mutex.h header.h
	$(CC) $(CFLAGS) -c $< -o $@

semaphore/%.o: semaphore/%.c semaphore/semaphore.h header.h
	$(CC) $(CFLAGS) -c $< -o $@

run: run-no-sync run-mutex run-semaphore

run-no-sync: no_sync_app
	./no_sync_app

run-mutex: mutex_app
	./mutex_app

run-semaphore: semaphore_app
	./semaphore_app


check: $(TARGETS)
	@echo "\n=== Simboli oggetti ==="
	nm $(NO_SYNC_OBJS) $(MUTEX_OBJS) $(SEMAPHORE_OBJS)
	@echo "\n=== Dimensione segmenti ==="
	size --format=GNU $(TARGETS)

clean:
	rm -f no_sync/*.o mutex/*.o semaphore/*.o $(TARGETS)

.PHONY: all clean check run run-no-sync run-mutex run-semaphore
