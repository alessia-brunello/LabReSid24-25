CC = gcc
CFLAGS = -g -Wall -Wextra -std=c11 -Wpedantic

OBJS = main.o graph.o fifo_net.o leader_election.o floodmax.o lcr.o
TARGET = leader_election

all:$(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c common.h
	$(CC) $(CFLAGS) -c $< -o $@

run-floodmax: $(TARGET)
	./$(TARGET) floodmax graph.txt 3

run-floodmax-sort: $(TARGET)
	./$(TARGET) floodmax graph.txt 3 | grep "^\[FINE\]" | sort


run-lcr: $(TARGET)
	./$(TARGET) lcr 5
	
run-lcr-sort: $(TARGET)
	./$(TARGET) lcr 5 | grep "^\[FINE\]" | sort
clean:
	rm -f *.o $(TARGET)
	rm -f /tmp/ho16_fifo_*

.PHONY: all clean run-floodmax run-floodmax-sort run-lcr run-lcr-sort

