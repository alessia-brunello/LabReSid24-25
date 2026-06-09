# TicketFlow REST Server

## Introduzione

**TicketFlow REST Server** è il progetto finale realizzato per il corso di **Laboratorio di Reti e Sistemi Distribuiti**.

Il progetto consiste nella realizzazione di un **server REST concorrente in C** per la gestione di ticket.
Il server comunica tramite socket TCP, interpreta richieste HTTP, produce risposte in formato JSON e utilizza SQLite come backend persistente.

L'obiettivo principale è gestire manualmente i principali aspetti di un server REST, senza utilizzare framework web esterni: dalla comunicazione di rete al parsing HTTP, fino al routing delle richieste e alla persistenza dei dati.

---

## Autore

| Campo           | Informazione                              |
| --------------- | ----------------------------------------- |
| Nome            | Alessia Smeralda Brunello                 |
| Corso           | Laboratorio di Reti e Sistemi Distribuiti |
| Progetto        | Progetto 4 - Server REST                  |
| Nome progetto   | TicketFlow REST Server                    |
| Anno accademico | 2024/2025                                 |

---

## Descrizione del progetto

Il progetto implementa un sistema client-server per la gestione di ticket.

Il server riceve richieste HTTP da client, interpreta il metodo e l'URI richiesto, esegue l'operazione corrispondente e restituisce una risposta HTTP in formato JSON.

Le operazioni principali riguardano la gestione completa dei ticket tramite operazioni CRUD.

| Operazione        | Descrizione                                              |
| ----------------- | -------------------------------------------------------- |
| Creazione         | Inserimento di un nuovo ticket nel database.             |
| Lettura lista     | Visualizzazione di tutti i ticket presenti.              |
| Lettura dettaglio | Visualizzazione delle informazioni di un singolo ticket. |
| Aggiornamento     | Modifica dei dati di un ticket esistente.                |
| Eliminazione      | Rimozione di un ticket dal database.                     |

Il server supporta connessioni persistenti, permettendo al client di inviare più richieste HTTP sulla stessa connessione TCP.
Inoltre, le connessioni inattive vengono chiuse automaticamente dopo un timeout.

---

## Struttura del progetto

```text
final-544964/
├── src/
│   ├── main.c
│   ├── server.c
│   ├── http.c
│   ├── router.c
│   ├── db.c
│   └── utils.c
│
├── include/
│   ├── server.h
│   ├── http.h
│   ├── router.h
│   ├── db.h
│   └── utils.h
│
├── client/
│   ├── client.c
│   └── client.h
│
├── tests/
│   ├── test_api.sh
│   └── load_test.sh
│
├── data/
│   └── .gitkeep
│
├── Makefile
├── .gitignore
└── README.md
```

---

## Tecnologie utilizzate

| Categoria              | Tecnologie |
| ---------------------- | ---------- |
| Linguaggio             | C          |
| Comunicazione di rete  | Socket TCP |
| Protocollo applicativo | HTTP       |
| Architettura API       | REST       |
| Concorrenza            | `epoll`    |
| Backend                | SQLite     |
| Formato dati           | JSON       |
| Compilazione           | Makefile   |
| Test                   | Bash, curl |
| Sistema operativo      | Linux      |

---

## Funzionalità implementate

| Funzionalità            | Descrizione                                                                   |
| ----------------------- | ----------------------------------------------------------------------------- |
| Server TCP              | Il server accetta connessioni tramite socket TCP.                             |
| Parsing HTTP            | Le richieste vengono interpretate manualmente.                                |
| Routing REST            | Le richieste vengono smistate in base a metodo e URI.                         |
| CRUD ticket             | Sono supportate creazione, lettura, aggiornamento ed eliminazione dei ticket. |
| Risposte JSON           | Le risposte del server sono restituite in formato JSON.                       |
| SQLite                  | I ticket vengono salvati in un database locale persistente.                   |
| `epoll`                 | Il server gestisce più connessioni in modo concorrente.                       |
| Connessione persistente | Il client può inviare più richieste sulla stessa connessione TCP.             |
| Timeout                 | Le connessioni inattive vengono chiuse automaticamente.                       |
| Client interattivo      | È presente un client da terminale con menu testuale.                          |
| Test funzionali         | Sono presenti script per verificare le principali operazioni REST.            |
| Test di carico          | È presente uno script per testare il comportamento con più richieste.         |

---

## Database

Il progetto utilizza SQLite come backend persistente.

Il database viene creato automaticamente nella cartella:

```text
data/ticketflow.db
```

Il file `data/ticketflow.db` non viene caricato nella repository, perché viene generato durante l'esecuzione del server.
La cartella `data/` viene mantenuta tramite il file `.gitkeep`.

---

## Endpoint principali

| Metodo   | Endpoint        | Descrizione                            |
| -------- | --------------- | -------------------------------------- |
| `GET`    | `/health`       | Verifica lo stato del server.          |
| `GET`    | `/tickets`      | Restituisce la lista dei ticket.       |
| `POST`   | `/tickets`      | Crea un nuovo ticket.                  |
| `GET`    | `/tickets/{id}` | Restituisce il dettaglio di un ticket. |
| `PUT`    | `/tickets/{id}` | Aggiorna un ticket esistente.          |
| `DELETE` | `/tickets/{id}` | Elimina un ticket.                     |

---

## Come eseguire

### 1. Entrare nella cartella del progetto

```bash
cd final-544964
```

### 2. Compilare il progetto

```bash
make
```

La compilazione genera gli eseguibili del server e del client.

### 3. Avviare il server

```bash
make run
```

Il server viene avviato sulla porta configurata nel Makefile, ad esempio `8080`.

È possibile verificare il corretto avvio del server con:

```bash
curl http://localhost:8080/health
```

Risposta attesa:

```json
{
  "status": "ok",
  "service": "TicketFlow REST Server"
}
```

### 4. Avviare il client

In un secondo terminale:

```bash
make client
```

Il client mostra un menu interattivo da terminale e permette di eseguire le principali operazioni sui ticket.

---

## Esempi di richieste con curl

### Creazione di un ticket

```bash
curl -X POST http://localhost:8080/tickets \
  -H "Content-Type: application/json" \
  -d '{
    "title": "Problema rete",
    "description": "Connessione non disponibile",
    "location": "Aula 1",
    "category": "network",
    "priority": "high"
  }'
```

### Lista dei ticket

```bash
curl http://localhost:8080/tickets
```

### Dettaglio di un ticket

```bash
curl http://localhost:8080/tickets/1
```

### Aggiornamento di un ticket

```bash
curl -X PUT http://localhost:8080/tickets/1 \
  -H "Content-Type: application/json" \
  -d '{
    "title": "Problema rete aggiornato",
    "description": "Connessione instabile",
    "location": "Aula 1",
    "category": "network",
    "priority": "medium",
    "status": "open"
  }'
```

### Eliminazione di un ticket

```bash
curl -X DELETE http://localhost:8080/tickets/1
```

---

## Test

La cartella `tests/` contiene gli script per verificare il funzionamento del progetto.

| Test            | Comando          | Descrizione                                                    |
| --------------- | ---------------- | -------------------------------------------------------------- |
| Test funzionali | `make test`      | Verifica le principali operazioni REST.                        |
| Test di carico  | `make load_test` | Esegue più richieste per valutare il comportamento del server. |

### Eseguire i test funzionali

```bash
make test
```

### Eseguire il test di carico

```bash
make load_test
```

---

## Pulizia dei file generati

Per rimuovere file oggetto, dipendenze ed eseguibili generati dalla compilazione:

```bash
make clean
```

Per rimuovere i file del database SQLite locale:

```bash
make clean-db
```


---

## Note finali

Il progetto mostra la realizzazione di un server REST a basso livello, gestendo direttamente socket TCP, richieste HTTP, routing, persistenza dei dati e concorrenza.

La scelta di utilizzare `epoll` permette di gestire più connessioni in modo efficiente, mentre SQLite fornisce un backend semplice e persistente per la memorizzazione dei ticket.
