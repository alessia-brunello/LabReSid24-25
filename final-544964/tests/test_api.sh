#!/bin/bash

BASE_URL="http://localhost:8080"

echo "---------------------------------"
echo "TicketFlow REST Server - API Test"
echo "---------------------------------"
echo

echo "[1] Health check"
curl -i -s "$BASE_URL/health"
echo -e "\n"

echo "[2] Lista ticket iniziale"
curl -i -s "$BASE_URL/tickets"
echo -e "\n"

echo "[3] Creazione ticket"
CREATE_RESPONSE=$(curl -s -X POST "$BASE_URL/tickets" \
	-H "Content-Type: application/json" \
	-d '{"title":"Problema Wi-Fi", "description":"La rete non funziona in ufficio", "location":"Ufficio 2", "category":"network",
		"priority":"high","status":"open"}')
		
echo "$CREATE_RESPONSE"
echo

TICKET_ID=$(echo "$CREATE_RESPONSE" | sed -n 's/.*"id":[[:space:]]*\([0-9][0-9]*\).*/\1/p')

if [ -z "$TICKET_ID" ]; then
	echo "Errore: impossibile leggere l'ID del ticket creato."
	exit 1
fi

echo "ID ticket creato: $TICKET_ID"
echo

echo "[4] Lista ticket dopo inserimento"
curl -i -s "$BASE_URL/tickets"
echo -e "\n"

echo "[5] Dettaglio ticket $TICKET_ID"
curl -i -s "$BASE_URL/tickets/$TICKET_ID"
echo -e "\n"

echo "[6] Aggiornamento ticket $TICKET_ID"
curl -i -s -X PUT "$BASE_URL/tickets/$TICKET_ID" \
	-H "Content-Type: application/json" \
	-d '{"title":"Problema Wi-Fi", "description":"Ticket preso in carico dal tecnico", "location":"Ufficio 2", "category":"network",
                "priority":"high","status":"in_progress"}'
echo -e "\n"

echo "[7] Dettaglio ticket $TICKET_ID aggiornato"
curl -i -s "$BASE_URL/tickets/$TICKET_ID"
echo -e "\n"

echo "[8] Eliminazione ticket $TICKET_ID"
curl -i -s -X DELETE "$BASE_URL/tickets/$TICKET_ID"
echo -e "\n"

echo "[9] Rotta inesistente"
curl -i -s "$BASE_URL/rotta_inesistente"
echo -e "\n"

echo "[10] Metodo non consentito"
curl -i -s -X POST "$BASE_URL/health"
echo -e "\n"

echo "[11] Body JSON incompleto"
curl -i -s -X POST "$BASE_URL/tickets" \
	-H "Content-Type: application/json" \
	-d '{"title":"Ticket incompleto"}'
echo -e "\n"

echo "[12] Ticket inesistente"
curl -i -s "$BASE_URL/tickets/999999"
echo -e "\n"

echo "Test completato"
