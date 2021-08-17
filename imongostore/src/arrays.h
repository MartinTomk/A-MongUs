/*
 * arrays.h
 *
 *  Created on: 4 jul. 2021
 *      Author: utnso
 */

#ifndef ARRAYS_H_
#define ARRAYS_H_

int longitud_array(char ** array);


void remover_de_array(char ** array);

char ** agregar_en_array(char ** array,char * valor_a_insertar);

char * array_to_string(char ** array);

#endif /* ARRAYS_H_ */
