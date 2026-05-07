CC=gcc
CFALGS=-Wall -Wextra -pthread

all: barrier prod_cons

barrier: barrier_main.o barrier.o
	$(CC) $(CFLAGS) -o barrier barrier_main.o barrier.o

prod_cons: pc_main.o pc_buffer.o
	$(CC) $(CFALGS) -o prod_cons pc_main.o pc_buffer.o

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o barrier prod_con
