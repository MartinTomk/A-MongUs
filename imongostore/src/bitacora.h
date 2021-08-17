/*
 * bitacora.h
 *
 *  Created on: 11 jul. 2021
 *      Author: utnso
 */

#ifndef BITACORA_H_
#define BITACORA_H_

#include "core.h"


_archivo_bitacora* find_bitacora(t_list * bitacoras, char * clave);

uint32_t write_archivo_bitacora(char* cadenaAGuardar,_archivo_bitacora * archivo);


void actualizar_metadata_sin_crear_bloque_bitacora(_archivo_bitacora * archivo,char * valorAux);

void llenar_nuevo_bloque_bitacora(char* cadenaAGuardar, _archivo_bitacora* archivo);

int obtener_indice_para_guardar_en_bloque_bitacora(char * valor);

int calcular_cantidad_bloques_requeridos_bitacora(char* cadenaAGuardar);

int write_blocks_with_offset_bitacora(char * cadena_caracteres,int indice,int offset);
#endif /* BITACORA_H_ */
