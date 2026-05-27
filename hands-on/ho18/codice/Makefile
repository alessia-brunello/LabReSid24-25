PYTHON = python3
VENV = .venv
PY = $(VENV)/bin/python
PIP = $(VENV)/bin/pip

INPUT ?= lotr.txt
OUTPUT ?= word_count.txt
CHUNK_SIZE ?= 20000
TOP ?= 100
PRINT_TOP ?= 20
MIN_LEN ?= 1
STOPWORDS ?=
NUM_CPUS ?=
RAY_ADDRESS ?=
REDUCE_BATCH_SIZE ?= 16

ARGS = --input $(INPUT) \
	--output $(OUTPUT) \
	--chunk-size $(CHUNK_SIZE) \
	--top $(TOP) \
	--print-top $(PRINT_TOP)\
	--min-len $(MIN_LEN) \
	--reduce-batch-size $(REDUCE_BATCH_SIZE)

ifdef STOPWORDS
ARGS += --stopwords $(STOPWORDS)
endif

ifdef NUM_CPUS
ARGS += --num-cpus $(NUM_CPUS)
endif

ifdef RAY_ADDRESS
ARGS += --ray-address $(RAY_ADDRESS)
endif

.PHONY: help install run clean

help:
	@echo "Comandi disponibili:"
	@echo "  make install	Crea l'ambiente virtuale e installa le dipendenze"
	@echo "  make run	Esegue il programma"
	@echo "  make clean	Rimuove l'ambiente virtuale e i file creati"
	@echo "							"
	@echo "Esempi:"
	@echo "  make run PRINT_TOP=N				stampa le N parole più frequenti"
	@echo "  make run MIN_LEN=N				lunghezza di parole da ignorare"
	@echo "  make run STOPWORDS=stopwords_en.txt		permette di togliere articoli e congiunzioni" 
	@echo "  make run INPUT=lotr.txt OUTPUT=word_count.txt	per specificare file di input e di output"


$(VENV):
	$(PYTHON) -m venv $(VENV)

install: $(VENV)
	$(PIP) install --upgrade pip
	$(PIP) install -r requirements.txt

run: install
	$(PY) main.py $(ARGS)

clean:
	rm -rf $(VENV)
	rm -rf __pycache__
	rm -f word_count.txt
