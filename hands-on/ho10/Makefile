CC = gcc
CFLAGS = -g -Wall -Wextra -Wno-unused-parameter -pthread
LDFLAGS = -pthread

TARGET = programma
OBJS = main.o operations.o
PYTHON = python3
PY_SCRIPT = ho10.py

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c header.h
	$(CC) $(CFLAGS) -c $< -o $@

run-c: $(TARGET)
	./$(TARGET)

run-python:
	$(PYTHON) $(PY_SCRIPT)

check: all
	@echo "\n=== Esecuzione programma C ==="
	./$(TARGET)
	@echo "\n=== Controllo sintassi Python ==="
	$(PYTHON) -m py_compile $(PY_SCRIPT)
	@echo "\n=== Simboli oggetti ==="
	nm $(OBJS)
	@echo "\n=== Dimensione eseguibile ==="
	size --format=GNU $(TARGET)

clean:
	rm -f *.o $(TARGET) __pycache__/*.pyc
	rmdir __pycache__ 2>/dev/null || true

.PHONY: all run-c run-python check clean
