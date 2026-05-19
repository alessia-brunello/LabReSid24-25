CC = gcc
CFLAGS = -g -Wall -Wextra -pthread
LDFLAGS = -pthread

TARGETS = barrier prod_cons

BARRIER_OBJS = barrier_main.o barrier.o
PC_OBJS = pc_main.o pc_buffer.o

all: $(TARGETS)

barrier: $(BARRIER_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

prod_cons: $(PC_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: run-barrier run-prod-cons

run-barrier: barrier
	./barrier 5

run-prod-cons: prod_cons
	./prod_cons

check: $(TARGETS)
	@echo "\n=== Simboli oggetti ==="
	nm $(BARRIER_OBJS) $(PC_OBJS)
	@echo "\n=== Dimensione segmenti ==="
	size --format=GNU $(TARGETS)

clean:
	rm -f *.o $(TARGETS)

.PHONY: all clean check run run-barrier run-prod-cons
