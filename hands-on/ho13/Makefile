CC = gcc
CFLAGS = -g -Wall -Wextra -Iinclude -pthread
LDFLAGS = -pthread

TARGETS = join_app barriera_app

JOIN_OBJS = join_main.o join.o
BARRIER_OBJS = barrier_main.o barrier.o

all: $(TARGETS)

join_app: $(JOIN_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

barriera_app: $(BARRIER_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run-join: join_app
	./join_app

run-barrier: barriera_app
	./barriera_app

check: all

	@echo "\n===Simboli oggetti esercizio 1 ==="
	nm $(JOIN_OBJS)
	@echo "\n===Simboli oggetti esercizio 2 ==="
	nm $(BARRIER_OBJS)
	@echo "\n===Dimensione segmenti eseguibili ==="
	size --format=GNU $(TARGETS)

clean:
	rm -f *.o join_app barriera_app

.PHONY: all clean check run-join run-barrier  
