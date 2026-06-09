#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "utils.h"

//estra l'id da un path del tipo /ticket/{id}
int extract_id_from_path(const char* path, const char* prefix) {

        //se il path non inizia con il prefisso atteso o se non è numerico restituisce -1 
	if(path == NULL || prefix == NULL) {
		return -1;
	}

	int prefix_len = strlen(prefix);

        //controlla che il path inizi esattamente con quel prefisso
	if(strncmp(path, prefix, prefix_len) != 0){
		return -1;
	}

        //id_str punta alla parte del path successiva al prefisso
	const char* id_str = path + prefix_len;

        //se dopo il prefisso non c'è nulla manca l id
	if(*id_str == '\0') {
		return -1;
	}

        //l'id deve contenere solo cifre
	for(int i = 0; id_str[i] != '\0'; i++) {
		if(!isdigit((unsigned char)id_str[i])) {
			return -1;
		}
	}
        
        //converte la stringa numerica in intero
  	return atoi(id_str);
}

//cerca nel body JSON il valore associato alla chiave
int get_json_value(const char* json, const char* key, char* output, int output_size) {
	if(json == NULL || key == NULL || output == NULL || output_size <= 0) {
		return -1;
	}

	char pattern[128];

        //costruisce il pattern della chiave (aggiunge le "")
	snprintf(pattern, sizeof(pattern), "\"%s\"", key);

        //cerca la chiave nel JSON
	char* key_position = strstr((char*)json, pattern);

	if(key_position == NULL) {
		return -1;
	}

        //dopo la chiave deve esserci il carattere : per separare chiave valore
	char* colon = strchr(key_position, ':');

	if(colon == NULL) {
		return -1;
	}

	colon++;

        //non considera spazi tra : e il valore
	while (*colon != '\0' && isspace((unsigned char)*colon)) {
		colon++;
	}

        //in questo progetto i valori attesi sono stringhe quindi iniziano con ""
	if(*colon != '"') {
		return -1;
	}

	colon++;
	int i = 0;
        
        //copia il valore fino alla virgoletta finale o fino al limite del buffer
	while(*colon != '\0' && *colon != '"' && i < output_size - 1) {
		output[i] = *colon;
		i++;
		colon++;
	}

	output[i] = '\0';

        //se non si trova la " di chiusura il JSON non è valido
	if(*colon != '"') {
		return -1;
	}

	return 0;
}
