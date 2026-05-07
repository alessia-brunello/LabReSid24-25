CC = gcc
CFLAGS = -g -Wall -pthread

NO_SYNC_OBJS = no_sync/main.o no_sync/no_sync.o
MUTEX_OBJS = mutex/main.o mutex/mutex.o
SEMAPHORE_OBJS = semaphore/main.o semaphore/semaphore.o

all: no_sync_app mutex_app semaphore_app

no_sync_app: $(NO_SYNC_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

mutex_app: $(MUTEX_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

semaphore_app: $(SEMAPHORE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

no_sync/%.o: no_sync/%.c no_sync/no_sync.h header.h
	$(CC) $(CFLAGS) -c $< -o $@
mutex/%.o: mutex/%.c mutex/mutex.h header.h
	$(CC) $(CFLAGS) -c $< -o $@
semaphore/%.o: semaphore/%.c semaphore/semaphore.h header.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f no_sync/*.o mutex/*.o semaphore/*.o no_sync_app mutex_app semaphore_app

.PHONY: all clean

