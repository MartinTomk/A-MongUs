#ifndef PATOTA_H_
#define PATOTA_H_

#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<commons/collections/queue.h>
#include<readline/readline.h>
#include <pthread.h>
#include <semaphore.h>
#include "utils.h"
#include "tripulante.h"
#include "socket.h"
#include "hilosDiscordiador.h"


void crear_patota(char * comando);

void* crear_buffer_patota(int longitud_tareas, int longitud_posiciones, uint32_t patotaId, uint32_t cantidad_tripulantes, int* tamanioGet, char* tareas, char* posiciones);
void asignar_posicion(char** destino,char* posiciones,uint32_t creados);
void liberarCadenaDoble(char** cadena);
#endif /* PATOTA_H_ */
