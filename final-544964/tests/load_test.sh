#!/bin/bash

BASE_URL="http://localhost:8080"

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

echo "[1] Test leggero: 100 richieste, 10 concorrenti"
ab -n 100 -c 10 "$BASE_URL/tickets"
echo

echo "[2] Test medio: 500 richieste, 25 concorrenti"
ab -n 500 -c 25 "$BASE_URL/tickets"
echo

echo "[3] Test alto: 1000 richieste, 50 concorrenti"
ab -n 1000 -c 50 "$BASE_URL/tickets"
echo

echo "[4] Test stress: 2000 richieste, 100 concorrenti"
ab -n 2000 -c 100 "$BASE_URL/tickets"
echo

echo "[5] Test con keep-alive: 1000 richieste, 50 concorrenti"
ab -k -n 1000 -c 50 "$BASE_URL/tickets"
echo

echo "Load test completato"
