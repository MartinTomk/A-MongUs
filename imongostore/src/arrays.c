#include "utils.h"
#include "arrays.h"

int longitud_array(char ** array){
	int i=0;
	while(array[i]!=NULL){
		i++;
	}
	return i;
}



void remover_de_array(char ** array){
	array[longitud_array(array)-1]=NULL;
}

char ** agregar_en_array(char ** array,char * valor_a_insertar){
	int longitud = longitud_array(array);
	char ** nuevo = malloc(sizeof(char*)*(longitud+2));
	memcpy(nuevo,array,sizeof(char*)*(longitud+1));
	free(array);
	nuevo[longitud] =string_new();
	string_append(&(nuevo[longitud]),valor_a_insertar);
	nuevo[longitud+1]=NULL;
	return nuevo;
}

char * array_to_string(char ** array){
	char * cadena = string_new();
	string_append(&cadena,"[");
	int i = 0;
	while(array[i]!=NULL){
		string_append(&cadena,array[i]);
		string_append(&cadena,",");
		//string_append_with_format(&cadena,"%s,",array[i]); // TODO VER ESTE LEAK
		i++;
	}
	if(i>0){
		char * substring = string_substring_until(cadena,string_length(cadena)-1);
		free(cadena);
		cadena = string_new();
		string_append(&cadena,substring);
		free(substring);
	}
	string_append(&cadena,"]");

	return cadena;
}
