/*
 * core.h
 *
 *  Created on: 3 jul. 2021
 *      Author: utnso
 */

#ifndef CORE_H_
#define CORE_H_


#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "utils.h"
#include "socket.h"
#include <sys/mman.h>
#include "estructurasFileSystem.h"
#include "protocolo.h"
#include "arrays.h"

int obtener_tamanio_archivo_de_recurso(_archivo * archivo,char * name_file,char * caracter);
int write_blocks(char * cadena_caracteres,int indice);
uint32_t write_archivo(char* valor,_archivo * archivo,uint32_t id_trip);
t_bitarray * crear_bit_array(uint32_t cantBloques);
int calcular_bloques_libres();
uint32_t leer_contenido_archivo(char* valor,_archivo * arch);
int leer_metadata_archivo(char * cadena_caracteres,int indice, _archivo archivo);
void actualizar_metadata(_archivo * archivo,int n_block,char * valorAux);
void iniciar_archivo(char * name_file,_archivo **archivo,char * key_file,char * caracter_llenado);
void iniciar_blocks();
void consumir_arch(_archivo * archivo,int cantidadAConsumir,uint32_t id_trip);
void remover_bloque(int indice,_archivo * archivo,int cantidadAConsumir);
void * obtener_contenido_bloque(int indice);
int obtener_bloque(int indice);
void actualizar_metadata_borrado(_archivo * archivo,int cantidadABorrar);

void iniciar_super_block();
void mostrar_blocks_ims(t_list * bloques,char * blocks,char * source);

void descartar_basura(_archivo * archivo,uint32_t id_trip);

void crear_bitacora();

void liberar_bloques_bitacora_al_iniciar_fs();
bool encontrar_bloque_para_iniciar_fs(t_list * lista_bloques, int i);

void calcular_md5();
#endif /* CORE_H_ */
