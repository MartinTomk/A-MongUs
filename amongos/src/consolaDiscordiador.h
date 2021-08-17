/*
 * consolaDiscordiador.h
 *
 *  Created on: 14 jun. 2021
 *      Author: utnso
 */

#ifndef CONSOLADISCORDIADOR_H_
#define CONSOLADISCORDIADOR_H_

#include "utils.h"
#include "socket.h"
#include "hilosDiscordiador.h"
#include "tripulante.h"
#include "patota.h"


t_tripulante* buscarTripulantePorUbicacion(uint32_t x,uint32_t y);
void listar_tripulantes();
t_tripulante* buscarTripulanteYMover(uint32_t id_trip,t_queue * targetList);
t_tripulante* buscarTripulante(uint32_t id_trip);
void mostrar_lista_tripulantes(t_queue* queue,char * nombre_cola);
//////----
int moverTripulantes (int id);
int sacarElegido(uint32_t id_trip);
#endif /* CONSOLADISCORDIADOR_H_ */
