/*
 * consolaDiscordiador.c
 *
 *  Created on: 14 jun. 2021
 *      Author: utnso
 */

#include "consolaDiscordiador.h"
int programa_ejecucion = 1;
void leer_consola() {
	log_info(logger,"INGRESE UN COMANDO: ");

	char* leido = readline(">");
	while(strncmp(leido, "TERMINAR", 7) != 0) {
		log_info(logger, leido);

		if(strncmp(leido, "DETENER", 2) == 0){
			log_info(logger,"leer_consola(): PLANIFICACION DETENIDA !!!: ");
			sem_wait(&detenerReaunudarEjecucion);
		}
		else if(strncmp(leido, "REANUDAR", 2) == 0){
			log_debug(logger,"leer_consola(): PLANIFICACION REANUDADA !!!: ");
			log_info(logger,"PLANIFICACION REANUDADA!: ");
			sem_post(&detenerReaunudarEjecucion);
		}
		else if(strncmp(leido, "LISTAR_TRIP", 7) == 0){
			log_debug(logger,"leer_consola(): LISTAR TRIPULANTES");
			log_info(logger,"------LISTA TRIPULANTES----------------------: ");
			hilo_mostrar_tripulantes();
		}
		else if(strncmp(leido, "PLANIFICAR", 9) == 0){
			log_debug(logger,"PLANIFICACION INICIADA !!!: ");
			log_info(logger,"PLANIFICACION INICIADA! ");
			sem_post(&iniciar_planificacion);
		}
		else if(strncmp(leido, "AYUDA", 5) == 0){
			log_info(logger,"1) INICIAR_PATOTA -> Ej : INICIAR_PATOTA plantas.txt 2 1|2 5|5");
			log_info(logger,"2) PLANIFICAR");
			log_info(logger,"3) LISTAR_TRIP");
			log_info(logger,"4) REANUDAR");
			log_info(logger,"5) DETENER");
			log_info(logger,"6) REANUDAR");
			log_info(logger,"7) PEDIR_BITACORA N");
			log_info(logger,"8) AYUDA");
		}
		else if(strncmp(leido, "PEDIR_BITACORA", 7) == 0){
				log_info(logger,"PEDIR_BITACORA! ");
				char ** parametros = string_n_split(leido,2," ");
				sendDeNotificacion(socketServerIMongoStore,PEDIR_BITACORA);
				sendDeNotificacion(socketServerIMongoStore,atoi(parametros[1]));
				//TODO FREE

		}else if(strncmp(leido, "INICIAR_PATOTA", 14) == 0){
			//EJEMPLO COMANDO: INICIAR_PATOTA oxigeno.txt 2 1|2
			//EJEMPLO COMANDO: INICIAR_PATOTA plantas.txt 1 6|2
			//EJEMPLO COMANDO: INICIAR_PATOTA plantas.txt 2 6|2 3|5
			//EJEMPLO COMANDO: INICIAR_PATOTA plantas.txt 2 1|2 4|5
			//EJEMPLO COMANDO: INICIAR_PATOTA oxigeno.txt 2 5|2 3|6

			//EJEMPLO COMANDO: INICIAR_PATOTA tareas/pag_a.txt 1 0|0
			//EJEMPLO COMANDO: INICIAR_PATOTA tareas/pag_b.txt 1 0|0
			//EJEMPLO COMANDO: INICIAR_PATOTA tareas/pag_c.txt 1 0|0
			//EJEMPLO COMANDO: INICIAR_PATOTA pag_b.txt 1 0|0
			//EJEMPLO COMANDO: INICIAR_PATOTA pag_c.txt 1 0|0

			//EJEMPLO COMANDO: INICIAR_PATOTA tareasLargas.txt 1 0|0

			//EJEMPLO COMANDO: INICIAR_PATOTA segA.txt 4 0|0
			//EJEMPLO COMANDO: INICIAR_PATOTA segB.txt 2 0|0
			//EJEMPLO COMANDO: INICIAR_PATOTA segC.txt 1 0|0

			//EJEMPLO COMANDO: INICIAR_PATOTA plantas.txt 2 5|2 3|6

			crear_patota(leido);
		}
		else if(strncmp(leido, "GET_DATOS", 3) == 0){
			sendDeNotificacion(socketServerMiRam, GET_PCB);
			sendDeNotificacion(socketServerMiRam,1);
		}
		else if(strncmp(leido, "DUMP", 3) == 0){
			sendDeNotificacion(socketServerMiRam, DUMP);
		}
		else if(strncmp(leido, "COMPACTAR", 3) == 0){
			sendDeNotificacion(socketServerMiRam, COMPACTACION);
		}
		else if(strncmp(leido, "EXPULSAR_TRIPULANTE", 9) == 0){
			char** separado = string_split(leido," ");
			t_tripulante* tripulante = buscarTripulante(atoi(separado[1]));
			if(tripulante != NULL) {
				socketServerMiRam = reConectarAServer(IP_MIRAM, PUERTO_MIRAM);
				log_info(logger,"El tripulante %i fue expulsado de la nave",tripulante->id);
				//actualizar_estado(socketServerMiRam,tripulante,FIN);
				sendDeNotificacion(socketServerMiRam,EXPULSAR_TRIPULANTE);
				sendDeNotificacion(socketServerMiRam,tripulante->id);
				sendDeNotificacion(socketServerMiRam,tripulante->patota_id);
				sendDeNotificacion(socketServerMiRam,tripulante->direccionLogica);
				//recvDeNotificacion(socketServerMiRam);
			}
			else{
				log_error(logger,"No pudo ser encontrado el tripulante");
			}
			free(separado[0]);
			free(separado[1]);
			free(separado);
		}
		else{
			log_info(logger,"COMANDO INVALIDO");
		}
		free(leido);
		leido = readline(">");
	}
	sem_post(&terminarPrograma);
	free(leido);
}

int longitud_array(char ** array){
	int i=0;
	while(array[i]!=NULL){
		i++;
	}
	return i;
}

void escuchoIMongo() {
	log_info(logger,"escuchoIMongo(): Escuchando OK");
	while(1) {
		uint32_t nroNotificacion = recvDeNotificacion(socketServerIMongoStore);
		log_debug(logger,"escuchoIMongo(): Llego una notificacion de iMongo");
		if(nroNotificacion==INFORMAR_SABOTAJE){
			//Recibe notificacion de sabojate
			char * posicion = recibirString(socketServerIMongoStore);
			char ** xy = string_split(posicion,"|");
			ubic_sab_x = atoi(xy[0]);
			ubic_sab_y = atoi(xy[1]);
			log_info(logger,"Llego un SABOTAJE en la posicion: %s",posicion);
			sabotaje = 1;

			t_tripulante * masCercano = (t_tripulante *)buscarTripulantePorUbicacion(ubic_sab_x,ubic_sab_y);
			if(masCercano!=NULL){
				masCercano->elegido = true;
				sem_wait(&detenerReaunudarEjecucion);
				log_info(logger,"el Mas cercano %d",masCercano->id);

			}else{
				log_info(logger,"No hay tripulantes para atender el sabotaje");
				sabotaje = 0;
			}

			free(posicion);

		}
		else if(nroNotificacion==ENVIAR_BITACORA){
			//Recibe informacion de bitacora
			char * bitacora = recibirString(socketServerIMongoStore);
			char ** bitacora_logs = string_split(bitacora,"."); //TODO : liberar
			char * bitacora_salto_de_linea = string_new();
			log_info(logger,"BITACORA PEDIDA:");
			for(int i = 0;i<longitud_array(bitacora_logs);i++){
				string_append(&bitacora_salto_de_linea,bitacora_logs[i]);
				string_append(&bitacora_salto_de_linea,"\n");
			}
			printf(bitacora_salto_de_linea);
			free(bitacora_salto_de_linea);
			free(bitacora);

		}
		else{
			//Notificacion desconocida -> IMongo caido
			log_debug(logger,"escuchoIMongo(): mensaje desconocido");
			close(socketServerIMongoStore);

			while(1){
				sleep(5);
				log_info(logger,"escuchoIMongo(): Intentando reestablecer la comunicacion con iMongo");
				socketServerIMongoStore = reConectarAServer(IP_MONGO, PUERTO_MONGO);
				if(socketServerIMongoStore>0){
					log_info(logger,"escuchoIMongo(): se pudo reconectar");
					break;
				}else
				{
					log_info(logger,"escuchoIMongo(): no se pudo reconectar");
				}
			}
		}
	}

}




void hilo_mostrar_tripulantes(){
	pthread_attr_t attr1;
	pthread_attr_init(&attr1);
	pthread_attr_setdetachstate(&attr1, PTHREAD_CREATE_DETACHED);
	pthread_t hilo = (pthread_t)malloc(sizeof(pthread_t));
	pthread_create(&hilo , &attr1,(void*) listar_tripulantes,NULL);
}


void listar_tripulantes(){
	log_info(logger,"Estado de la Nave");

	pthread_mutex_lock(&planificacion_mutex_new);
	mostrar_lista_tripulantes(planificacion_cola_new,"NEW");
	pthread_mutex_unlock(&planificacion_mutex_new);

	pthread_mutex_lock(&planificacion_mutex_ready);
	mostrar_lista_tripulantes(planificacion_cola_ready,"READY");
	pthread_mutex_unlock(&planificacion_mutex_ready);

	pthread_mutex_lock(&planificacion_mutex_bloq);
	mostrar_lista_tripulantes(planificacion_cola_bloq,"BLOQ");
	pthread_mutex_unlock(&planificacion_mutex_bloq);

	pthread_mutex_lock(&mutex_cola_ejecutados);
	mostrar_lista_tripulantes_exec();
	pthread_mutex_unlock(&mutex_cola_ejecutados);

	pthread_mutex_lock(&planificacion_mutex_fin);
	mostrar_lista_tripulantes(planificacion_cola_fin,"FIN");
	pthread_mutex_unlock(&planificacion_mutex_fin);
}


void mostrar_lista_tripulantes(t_queue* queue,char * nombre_cola){
	void mostrar_patota(t_tripulante* tripulante){
		log_info(logger,"Tripulante: %d   Patota: %d    Status: %s - Ubicacion x:%d y:%d", tripulante->id,tripulante->patota_id,nombre_cola, tripulante->ubi_x, tripulante->ubi_y);
	}
	list_iterate(queue->elements, (void*) mostrar_patota);
}

void mostrar_lista_tripulantes_exec(){
	void mostrar_patota(t_tripulante* tripulante){
		log_info(logger,"Tripulante: %d   Patota: %d    Status EXEC - Ubicacion x:%d y:%d", tripulante->id,tripulante->patota_id,tripulante->ubi_x, tripulante->ubi_y);
	}
	list_iterate(lista_exec, (void*) mostrar_patota);
}
t_tripulante* buscarTripulante(uint32_t id_trip){
	bool condicionId(void* dato){
		t_tripulante* trip = (t_tripulante*)dato;
		return trip->id == id_trip;
	}
	pthread_mutex_lock(&planificacion_mutex_new);
	pthread_mutex_lock(&planificacion_mutex_fin);
	pthread_mutex_lock(&planificacion_mutex_ready);
	pthread_mutex_lock(&planificacion_mutex_bloq);
	pthread_mutex_lock(&mutex_cola_ejecutados);
	t_tripulante* trip = NULL;
	trip = list_find(planificacion_cola_new->elements,condicionId);
	if(trip != NULL){
		log_info(logger,"El tripulante se encontraba en new");
		queue_push(planificacion_cola_fin,list_remove_by_condition(planificacion_cola_new->elements,condicionId));
	}
	else{
		trip = list_find(planificacion_cola_ready->elements,condicionId);
		if(trip != NULL){
			log_info(logger,"El tripulante se encontraba en ready");
			queue_push(planificacion_cola_fin,list_remove_by_condition(planificacion_cola_ready->elements,condicionId));
			trip->expulsado = true;
			sem_post(&trip->exec);
		}
		else{
			trip = list_find(lista_exec,condicionId);
			if(trip != NULL){
				log_info(logger,"El tripulante se encontraba en exec");
				queue_push(planificacion_cola_fin,list_remove_by_condition(lista_exec,condicionId));
				trip->expulsado = true;
			}
			else{
				trip = list_find(planificacion_cola_bloq->elements,condicionId);
				if(trip != NULL){
					log_info(logger,"El tripulante se encontraba en bloq");
					queue_push(planificacion_cola_fin,list_remove_by_condition(planificacion_cola_bloq->elements,condicionId));
				}
			}

		}
	}
	pthread_mutex_unlock(&mutex_cola_ejecutados);
	pthread_mutex_unlock(&planificacion_mutex_bloq);
	pthread_mutex_unlock(&planificacion_mutex_fin);
	pthread_mutex_unlock(&planificacion_mutex_new);
	pthread_mutex_unlock(&planificacion_mutex_ready);
	return trip;
}

t_tripulante* buscarTripulanteYMover(uint32_t id_trip,t_queue * target){
	bool condicionId(void* dato){
		t_tripulante* trip = (t_tripulante*)dato;
		return trip->id == id_trip;
	}
	pthread_mutex_lock(&planificacion_mutex_new);
	t_tripulante* trip = list_find(planificacion_cola_new->elements,condicionId);
	if(trip != NULL){
		pthread_mutex_lock(&planificacion_mutex_bloq);
		queue_push(target,list_remove_by_condition(planificacion_cola_new->elements,condicionId));
		pthread_mutex_unlock(&planificacion_mutex_bloq);
	}
	pthread_mutex_unlock(&planificacion_mutex_new);
	if(trip != NULL){
		return trip;
	}
	pthread_mutex_lock(&planificacion_mutex_ready);
	trip = list_find(planificacion_cola_ready->elements,condicionId);
	if(trip != NULL){
		pthread_mutex_lock(&planificacion_mutex_bloq);
		queue_push(target,list_remove_by_condition(planificacion_cola_ready->elements,condicionId));
		pthread_mutex_unlock(&planificacion_mutex_bloq);
	}
	pthread_mutex_unlock(&planificacion_mutex_ready);
	if(trip != NULL){
		return trip;
	}
	pthread_mutex_lock(&mutex_cola_ejecutados);
	trip = list_find(lista_exec,condicionId);
	if(trip != NULL){
		pthread_mutex_lock(&planificacion_mutex_bloq);
		queue_push(target,list_remove_by_condition(lista_exec,condicionId));
		pthread_mutex_unlock(&planificacion_mutex_bloq);
	}
	pthread_mutex_unlock(&mutex_cola_ejecutados);
	if(trip != NULL){
		return trip;
	}

	return NULL;
}

t_tripulante* buscarTripulantePorUbicacion(uint32_t x,uint32_t y){
	void * condicionPorUbicacion(void* dato,void* dato2){
		t_tripulante* trip = (t_tripulante*)dato;
		t_tripulante* trip2 = (t_tripulante*)dato2;

		double dist1 = sqrt(pow(((double)trip->ubi_x-(double)x),2) + pow(((double)trip->ubi_y-(double)y),2));
		double dist2 = sqrt(pow(((double)trip2->ubi_x-(double)x),2) + pow(((double)trip2->ubi_y-(double)y),2));

		log_debug(logger,"trip:%d distancia  %f",trip->id,dist1);
		log_debug(logger,"trip:%d distancia %f",trip2->id,dist2);

		return dist1 < dist2? dato: dato2 ;
	}
	t_list * tripulantes_disponibles = list_create();
	pthread_mutex_lock(&planificacion_mutex_exec);
	t_tripulante* trip = NULL;
	if(list_size(lista_exec)>1){
		list_add_all(tripulantes_disponibles,lista_exec);
		trip = list_get_minimum(tripulantes_disponibles,condicionPorUbicacion);
	}else if(list_size(lista_exec)==1){
		trip = list_get(lista_exec,0);
	}
	pthread_mutex_unlock(&planificacion_mutex_exec);
	return trip;
}

int moverTripulantes (int id){
	bool ordenarId(void* dato, void* otroDato){
		t_tripulante* unTrip =  (t_tripulante*) dato;
		t_tripulante* otroTrip = (t_tripulante*) otroDato;
		return unTrip->id < otroTrip->id;
	}
	uint32_t cantidad;
	pthread_mutex_lock(&mutex_cola_ejecutados);
	pthread_mutex_lock(&planificacion_mutex_exec);
	pthread_mutex_lock(&planificacion_mutex_ready);

	cantidad = list_size(lista_exec);
	log_debug(logger,"moverTripulantes(): Cantidad exec: %i",cantidad);
	if(cantidad > 1){
		list_sort(lista_exec,ordenarId);
	}
	log_trace(logger,"moverTripulantes(): 1");
		while(cantidad > 0){
			t_tripulante* aux = list_remove(lista_exec,0);
			log_trace(logger,"moverTripulantes(): 2");
			sem_post(&exec);
			sem_post(&cola_exec);
			pthread_mutex_lock(&planificacion_mutex_bloq);
			queue_push(planificacion_cola_bloq,aux);
			sem_post(&cola_bloq);
			pthread_mutex_unlock(&planificacion_mutex_bloq);
			cantidad--;
		}




	cantidad = list_size(planificacion_cola_ready->elements);
	log_debug(logger,"moverTripulantes(): Cantidad ready: %i",cantidad);
	if(cantidad > 1){
	list_sort(planificacion_cola_ready->elements,ordenarId);
	}
	log_trace(logger,"moverTripulantes(): 3");
		while(cantidad > 0){
			t_tripulante* aux = list_remove(planificacion_cola_ready->elements,0);
			log_info(logger,"moverTripulantes(): 4");
			sem_post(&aux->exec);
			pthread_mutex_lock(&planificacion_mutex_bloq);
			queue_push(planificacion_cola_bloq,aux);
			sem_post(&cola_bloq);
			pthread_mutex_unlock(&planificacion_mutex_bloq);
			if(cantidad-1 > 0){
				sem_wait(&cola_ready);
			}
			cantidad--;
		}
	pthread_mutex_unlock(&planificacion_mutex_ready);
	log_trace(logger,"moverTripulantes(): 5");
	pthread_mutex_unlock(&planificacion_mutex_exec);
	log_trace(logger,"moverTripulantes(): 6");
	pthread_mutex_unlock(&mutex_cola_ejecutados);
	log_trace(logger,"moverTripulantes(): 7");

	return 0;
}

int sacarElegido(uint32_t id_trip){
	bool condicionId(void* dato){
		t_tripulante* trip = (t_tripulante*)dato;
		return trip->id == id_trip;
	}
	pthread_mutex_lock(&planificacion_mutex_bloq);
	t_tripulante* tripElegido = list_remove_by_condition(planificacion_cola_bloq->elements,condicionId);
	pthread_mutex_unlock(&planificacion_mutex_bloq);
	sem_wait(&cola_bloq);
	sem_post(&tripElegido->bloq);
	tripElegido->elegido = true;
	pthread_mutex_lock(&planificacion_mutex_ready);
	queue_push(planificacion_cola_ready,tripElegido);
	sem_post(&tripElegido->ready);
	sem_post(&cola_ready);
	sem_post(&exec);
	pthread_mutex_unlock(&planificacion_mutex_ready);
	return 0;
}
