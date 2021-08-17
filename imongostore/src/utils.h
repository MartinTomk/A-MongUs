/*
 * conexiones.h
 *
 *  Created on: 3 mar. 2019
 *      Author: utnso
 */

#ifndef CONEXIONES_H_
#define CONEXIONES_H_

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/config.h>
#include<commons/collections/list.h>
#include<commons/txt.h>
#include<commons/string.h>
#include<commons/bitarray.h>
#include<string.h>
#include "socket.h"
#include<time.h>
#include "estructurasFileSystem.h"
#include "bitacora.h"
#include "sabotaje.h"

int fs_server;

int socketDiscordiador;

sem_t detener_sincro;

//VARIABLES DEL ARCHIVO DE CONFIGURACION
int salidaEstandar;
char* conf_PUNTO_MONTAJE;
uint32_t conf_PUERTO_IMONGO;
uint32_t conf_TIEMPO_SICRONIZACION;
char** conf_POSICIONES_SABOTAJE;
int conf_TIEMPO_SABOTAJE;
uint32_t conf_PUERTO_DISCORDIADOR;
char* conf_IP_DISCORDIADOR;
char* conf_ARCHIVO_OXIGENO_NOMBRE;
char* conf_ARCHIVO_COMIDA_NOMBRE;
char* conf_ARCHIVO_BASURA_NOMBRE;
char* conf_PATH_BITACORA;
char* conf_PATH_FILES;
char* conf_BYTES_BLOQUE;
char* conf_CANTIDAD_BLOQUES;
int conf_TIEMPO_SINCRONIZACION;
char* conf_LOG_LEVEL;


typedef struct _infoHilos{
	int socket;
	pthread_t hiloAtendedor;
} infoHilos;

t_list * hilosParaConexiones;
pthread_mutex_t mutexHilos;

pthread_t thread_manejador;
pthread_t thread_sincronizador;

t_log* logger;

t_config* config;

int sabotajes_realizados;

int exitSincro;
//-------------------------------------------------
void iniciar_configuracion();
t_config* leer_config();

void init_server();
void manejadorDeHilos();
void *atenderNotificacion(void * paqueteSocket);
void *sincronizar_blocks();


////////FUNCIONES DE TAREAS/////////
void generarOxigeno(uint32_t cantidad);
void generarComida(uint32_t cantidad);
void generarBasura(uint32_t cantidad);
void consumirOxigeno(uint32_t cantidad);
void consumirComida(uint32_t cantidad);
void descartarBasura();
char *devolverTarea(char* tarea);
void ejecutarTarea(char* tarea, uint32_t cantidad);
void tipoTarea(char* tarea,uint32_t id_trip);
void ejecutar_tarea(char * tarea,char caracter_tarea,_archivo * archivo,uint32_t id_trip);
////////FUNCIONES DE TAREAS/////////

/* Bitacora */
void escribirBitacora(char* tarea, uint32_t idTripulante);
char* generarIdArchivo(uint32_t idTripulante);
char* generarPath(char* archivoTripulante);
/* Bitacora */

void enviar_bitacora(int socket,char * bitacora);


#endif /* CONEXIONES_H_ */
