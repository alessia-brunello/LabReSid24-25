#!/bin/bash

URL="http://localhost:8080/tickets"

echo "----------------------------------"
echo "TicketFlow REST Server - Load Test"
echo "----------------------------------"
echo

echo "Modello di concorrenza del server:"
echo "- Event loop basato su epoll"
echo "- Socket non bloccanti"
echo "- Connessioni persistenti HTTP/1.1"
echo "- Nessun thread bloccante dedicato a ogni client"
echo

if ! command -v ab > /dev/null 2>&1; then
	echo "Errore: ApacheBench non è installato."
	echo "Su Ubuntu/Debian puoi installarlo con:"
	echo "sudo apt install apache2-utils"
	exit 1
fi

echo "[1] Test medio: 500 richieste, 25 concorrenti"
ab -k -n 500 -c 20 $URL
echo

echo "[2] Test alto: 1000 richieste, 50 concorrenti"
ab -k -n 1000 -c 50 $URL
echo

echo "[3] Test stress: 2000 richieste, 100 concorrenti"
ab -k -n 2000 -c 100 $URL
echo

echo "[4] Test senza keep-alive: 1000 richieste, 50 concorrenti"
ab -n 1000 -c 50 $URL
echo

echo "Load test completato"
