# Laboratorio di Reti e Sistemi Distribuiti - LabReSid24-25

## Introduzione

Questa repository contiene il materiale svolto per il corso di **Laboratorio di Reti e Sistemi Distribuiti**.

Il repository raccoglie gli handson realizzati durante il corso e il progetto finale.
Gli esercizi affrontano diversi aspetti della programmazione di rete e dei sistemi distribuiti, partendo dall'uso dei socket e della comunicazione tra processi fino ad arrivare ad algoritmi distribuiti, MapReduce e Federated Learning.

L'obiettivo della repository è organizzare in modo chiaro codice, relazioni e progetto finale, così da rendere il materiale facilmente consultabile, compilabile ed eseguibile.

---

## Autore

| Campo           | Informazione                              |
| --------------- | ----------------------------------------- |
| Nome            | Alessia Smeralda Brunello                 |
| Matricola       | 544964
| Corso           | Laboratorio di Reti e Sistemi Distribuiti |

---

## Struttura del repository

Il repository è organizzato in due cartelle principali:

| Cartella        | Descrizione                                                                                                    |
| --------------- | -------------------------------------------------------------------------------------------------------------- |
| `hands-on/`     | Contiene gli handson svolti durante il corso, con codice sorgente, file di supporto e relazioni finali in PDF. |
| `final-544964/` | Contiene il progetto finale del corso.                                                                         |

Struttura generale:

```text
LabReSid24-25/
├── hands-on/
│   ├── ho7/
│   ├── ho9/
│   ├── ho10/
│   ├── ho11/
│   ├── ho12/
│   ├── ho13/
│   ├── ho14/
│   ├── ho15/
│   ├── ho16/
│   ├── ho17/
│   ├── ho18/
│   └── ho19/
│
└── final-544964/
    ├── src/
    ├── include/
    ├── client/
    ├── tests/
    ├── data/
    ├── Makefile
    └── README.md
```

### Progetto finale

La cartella `final-544964/` contiene il progetto finale relativo al **Progetto 4**, cioè la realizzazione di un **server REST concorrente in C**.

Il progetto, chiamato **TicketFlow REST Server**, implementa un sistema client-server per la gestione di ticket.
Il server comunica tramite socket TCP, interpreta richieste HTTP, genera risposte in formato JSON e utilizza SQLite come backend persistente.

Per maggiori dettagli sulla compilazione e sull'esecuzione del progetto finale, consultare il file:

```text
final-544964/README.md
```

---

## Indice degli handson

Ogni handson contiene il materiale relativo a una specifica esercitazione, generalmente composto da codice sorgente, Makefile, file di supporto e relazione finale in formato PDF.

| ID   | Argomento                                                | Tecnologie principali          |
| ---- | -------------------------------------------------------- | ------------------------------ |
| HO7  | Esercitazione sui concetti di rete e sistemi distribuiti | C, Makefile                    |
| HO9  | Esercitazione di laboratorio                             | C, Makefile                    |
| HO10 | Esercitazione di laboratorio                             | C, Makefile                    |
| HO11 | Esercitazione di laboratorio                             | C, Makefile                    |
| HO12 | Esercitazione di laboratorio                             | C, Makefile                    |
| HO13 | Esercitazione di laboratorio                             | C, Makefile                    |
| HO14 | Server TCP bancario modulare                             | C, Socket TCP, Makefile        |
| HO15 | Flooding su grafo tramite processi e FIFO                | C, FIFO, Processi              |
| HO16 | Leader Election con LCR e FloodMax                       | C, Algoritmi distribuiti, FIFO |
| HO17 | Esercitazione di laboratorio                             | C, Makefile                    |
| HO18 | Word Count distribuito con MapReduce                     | Python, Ray                    |
| HO19 | Federated Learning su MNIST                              | Python, Ray, PyTorch           |

---

## Tecnologie utilizzate

Nel repository sono state utilizzate diverse tecnologie e librerie, in base alla specifica esercitazione o al progetto finale.

| Tecnologia | Utilizzo                                                                                |
| ---------- | --------------------------------------------------------------------------------------- |
| C          | Implementazione degli handson basati su socket, processi, FIFO e algoritmi distribuiti. |
| Python     | Implementazione degli esercizi basati su MapReduce e Federated Learning.                |
| Ray        | Esecuzione distribuita negli handson Python.                                            |
| PyTorch    | Addestramento del modello nel progetto di Federated Learning.                           |
| Mininet, Wireshark | Utilizzati per la simulazione di topologie di rete e analisi del traffico       |
| LaTeX      | Usato per le relazioni tecniche.                                                        |
| Beamer     | Usato per le presentazioni                                                              |
---

## Come eseguire

### Handson

Per gli handson sviluppati in C, entrare nella cartella del codice dell'esercitazione e utilizzare il Makefile.

Esempio generale:

```bash
cd hands-on/hoXX/codice
make
```

L'esecuzione dipende dai target definiti nel Makefile del singolo handson.

Per pulire i file generati:

```bash
make clean
```
