PYTHON = python3
VENV = .venv
PIP = $(VENV)/bin/pip
PY = $(VENV)/bin/python

WORKERS ?= 6
ROUNDS ?= 5
LOCAL_EPOCHS ?= 1
BATCH_SIZE ?= 64
TEST_BATCH_SIZE ?= 1000
LR ?= 0.001
DEVICE ?= cpu

.PHONY:  help setup run clean clean-data

help:
	@echo "Comandi disponibili:"
	@echo "  make setup		Crea l'ambiente virtuale e installa le dipendenze"
	@echo "  make run		Esegue il programma"
	@echo "  make clean		Rimuove cache Python e file temporanei di Ray"
	@echo "  make clean-data	Rimuove anche il dataset MNIST scaricato"
	@echo "							"
	@echo "Esempi:"
	@echo "  make run ROUNDS=N LOCAL_EPOCHS=N		per cambiare i parametri"

setup:
	$(PYTHON) -m venv $(VENV)
	$(PIP) install --upgrade pip
	$(PIP) install -r requirements.txt

run:
	$(PY) main.py \
		--num-workers $(WORKERS) \
		--rounds $(ROUNDS) \
		--local-epochs $(LOCAL_EPOCHS) \
		--batch-size $(BATCH_SIZE) \
		--test-batch-size $(TEST_BATCH_SIZE) \
		--learning-rate $(LR) \
		--device $(DEVICE)

clean:
	rm -rf __pycache__
	rm -rf .ray_tmp
	find . -type d -name "__pycache__" -exec rm -rf {} +

clean-data:
	rm -rf data
