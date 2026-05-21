CC = gcc
CFLAGS = -g -Wall -Wextra -std=c11 -Wpedantic

OBJS = main.o graph.o flooding.o
TARGET = flooding_fifo

all:$(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c common.h
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET) graph.txt 6 42

clean:
	rm -f *.o $(TARGET)
	rm -f /tmp/lab15_fifo_*

.PHONY: all clean run

