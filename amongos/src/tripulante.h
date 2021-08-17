#ifndef _TRIPULANTE_H
#define _TRIPULANTE_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include "utils.h"
#include "socket.h"

int tripulantes_creados;
uint32_t patotas_creadas;

#define NEW 0
#define READY 1
#define BLOCK 2
#define EXEC 3
#define FIN 4
int sabotaje;
typedef struct {
	int id;
	int patota_id;
	int cantidad_tareas;
	char* tarea; //Calculo que es necesario 50 50 SEGURIDAD
	pthread_t hilo_asociado;
	int socket;
//	sem_t emergencia;
	sem_t ready;
	sem_t new;
	sem_t exec;
	sem_t bloq;
	sem_t creacion;
	int instrucciones_ejecutadas;
	pthread_mutex_t ejecutadas;
	int fin;
	char estado;
	char* ubicacionInicio;
	uint32_t ubi_x;
	uint32_t ubi_y;
	int block_io_rafaga;
	uint32_t direccionLogica;
	bool elegido;
	bool expulsado;
}t_tripulante;


void *labor_tripulante_new(void * id_tripulante);
void enviar_evento_bitacora(int socketMongo, int id, char* claveNueva);
void enviar_tarea_a_ejecutar(int socketMongo, int id, char* claveNueva);
void actualizar_ubicacion(int socketRam, t_tripulante* tripulante);
void actualizar_estado(int socketRam, t_tripulante* tripulante,int estado);
char* parsear_tarea(char* tarea,int* movX,int* movY,int* esIo,int* tiempo_tarea);
#endif
