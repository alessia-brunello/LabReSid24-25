CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11
TARGET = bellman_ford

OBJS = main.o graph.o bellman_ford_fifo.o common.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

main.o: graph.h common.h
graph.o: graph.h
bellman_ford_fifo.o: graph.h common.h
common.o: common.h

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET) $(OBJS)
	rm -f fifo_* fifo_result
	
.PHONY: all run clean
