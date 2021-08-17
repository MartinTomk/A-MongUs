/*
 * estructuras.h
 *
 *  Created on: 6 jun. 2021
 *      Author: utnso
 */

#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdint.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include "ADMIN_MIRAM.h"

typedef struct {
	char * nombre_tarea;
}t_tarea;


typedef struct
{
	uint32_t identificador_pcb;
	t_tarea * tareas;
} pcb3;

typedef struct
{
	uint32_t identificador_tripulante;
	int socket_tcb; //VER
	char estado;
	uint32_t posicion_x;
	uint32_t posicion_y;
	uint32_t proxima_instruccion;
} tcb;

typedef struct
{
	uint32_t identificador_tripulante;
	char * id_posicion;
	int socket_tcb; //VER
	char estado;
	uint32_t proxima_instruccion;
	int patotaid;
	uint32_t cantidad_tripulantes;
	t_list * tareas_list;
} pcb;




typedef struct _infoHilos{
	int socket;
	pthread_t hiloAtendedor;
} infoHilos;

typedef struct {
	uint32_t tamanio;
	char* esquema;
	uint32_t tamanioPagina;
	uint32_t tamanioSwap;
	char* pathSwap;
	char* algoritmo;
	char* criterio;
	uint32_t puerto;
}conf_t;

conf_t confDatos;
t_config* configuracion;
int crear_configuracion(void);
#endif /* ESTRUCTURAS_H_ */
