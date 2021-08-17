/*
 * estructuras.c
 *
 *  Created on: 6 jun. 2021
 *      Author: utnso
 */

#include "estructuras.h"

int crear_configuracion(void){
	configuracion = config_create("memoria.config");
	mapaActivo = config_get_int_value(configuracion,"MAPA_ACTIVO");
	tareasActivas = false;
	int log_level = config_get_int_value(configuracion,"LOG_LEVEL");
	if(mapaActivo){
		logger = log_create("MiRAMHQ.log", "MiRAMHQ",0, log_level);
	}
	else{
		logger = log_create("MiRAMHQ.log", "MiRAMHQ",1, log_level);
	}

	confDatos.esquema = config_get_string_value(configuracion,"ESQUEMA_MEMORIA");
	confDatos.pathSwap = config_get_string_value(configuracion,"PATH_SWAP");
	confDatos.algoritmo = config_get_string_value(configuracion,"ALGORITMO_REEMPLAZO");
	confDatos.criterio = config_get_string_value(configuracion,"CRITERIO_SELECCION");
	confDatos.tamanio = config_get_int_value(configuracion,"TAMANIO_MEMORIA");
	confDatos.tamanioPagina = config_get_int_value(configuracion,"TAMANIO_PAGINA");
	confDatos.tamanioSwap = config_get_int_value(configuracion,"TAMANIO_SWAP");
	confDatos.puerto = config_get_int_value(configuracion,"PUERTO");
	tamanioMemoria = confDatos.tamanio;
	tamanioPagina = confDatos.tamanioPagina;

	if(!strcmp(confDatos.criterio,"FF")){
		algoritmo = FF;
		log_debug(logger,"Criterio FF");
	}
	else {
		log_debug(logger,"Criterio BF");
		algoritmo = BF;
	}
	if(!strcmp(confDatos.esquema,"PAGINACION")){
		paginacion = true;
		log_debug(logger,"Esquema Paginacion");
	}
	else {
		paginacion = false;
		log_debug(logger,"Esquema Segmentacion");
	}
	if(!strcmp(confDatos.algoritmo,"LRU")){
		algoritmoReemplazo = true;
		log_debug(logger,"Algoritmo -> LRU");
	}
	if(!strcmp(confDatos.algoritmo,"CLOCK")){
		algoritmoReemplazo = false;
		log_debug(logger,"Algoritmo CLOCK");
	}

	return 0;
}
