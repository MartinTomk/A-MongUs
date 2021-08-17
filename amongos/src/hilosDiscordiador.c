/*
 * hilosDiscordiador.c
 *
 *  Created on: 7 jun. 2021
 *      Author: utnso
 */

#include "hilosDiscordiador.h"


void planificar_tripulantes(){
	sem_wait(&iniciar_planificacion);
	hilo_cola_ready();
	hilo_cola_exec();
	hilo_cola_bloq();
	hilo_cola_replanificar();
}

void planificar_cola_ready(){
	while(1){


		//Esperamos que haya algo en new
		sem_wait(&cola_new);
		sleep(1); //TODO CAMBIAR
		sem_wait(&detenerReaunudarEjecucion);
		sem_post(&detenerReaunudarEjecucion);
		pthread_mutex_lock(&planificacion_mutex_new);
		t_tripulante * tripulante = queue_pop(planificacion_cola_new);
		//log_info(logger,"T%d - P%d : SALIENDO DE NEW", tripulante->id,tripulante->patota_id);
		pthread_mutex_unlock(&planificacion_mutex_new);

		pthread_mutex_lock(&planificacion_mutex_ready);
		queue_push(planificacion_cola_ready,tripulante);
		sem_post(&tripulante->ready);
		sem_post(&cola_ready);
		log_info(logger,"T%d - P%d : READY", tripulante->id,tripulante->patota_id);
		pthread_mutex_unlock(&planificacion_mutex_ready);

		//wait(planificacion_mutex_ready);
		//hacer algo en la cola ready
		sem_wait(&detenerReaunudarEjecucion);
		sem_post(&detenerReaunudarEjecucion);


	}
}

void planificar_cola_bloq(){
	t_tripulante * tripulante;
	while(1){

		sem_wait(&cola_bloq);
		sem_wait(&detenerReaunudarEjecucion);
		sem_post(&detenerReaunudarEjecucion);

		pthread_mutex_lock(&planificacion_mutex_bloq);
		tripulante = list_get(planificacion_cola_bloq->elements,0);
		pthread_mutex_unlock(&planificacion_mutex_bloq);
		int timer = 0;
		while(timer<tripulante->block_io_rafaga){
			sem_wait(&detenerReaunudarEjecucion);
			sem_post(&detenerReaunudarEjecucion);
			sem_wait(&sabotajeEnCurso);
			sem_post(&sabotajeEnCurso);
			log_info(logger,"T%d - P%d : [I/O-BLOCK] CICLOS TRANSCURRIDOS=%d - ***IO BOUND***", tripulante->id,tripulante->patota_id,timer, tripulante->id,tripulante->patota_id,tripulante->id,tripulante->patota_id);
			sleep(CICLO_IO);
			timer++;
		}
		if(tripulante->elegido){
					log_info(logger,"bloqueado por sabotaje");
					sleep(10);
		}
		sem_wait(&sabotajeEnCurso);
		sem_post(&sabotajeEnCurso);
		pthread_mutex_lock(&planificacion_mutex_bloq);
		tripulante = queue_pop(planificacion_cola_bloq);
		pthread_mutex_unlock(&planificacion_mutex_bloq);
		sem_post(&tripulante->bloq);

		//log_info(logger,"SACO DE BLOQ TRIPULANTE %d",tripulante->id);


		pthread_mutex_lock(&planificacion_mutex_ready);
		queue_push(planificacion_cola_ready,tripulante);
		log_info(logger,"T%d - P%d : READY", tripulante->id,tripulante->patota_id);
		sem_post(&cola_ready);
		sem_post(&tripulante->ready);
		pthread_mutex_unlock(&planificacion_mutex_ready);

		sem_wait(&detenerReaunudarEjecucion);
		sem_post(&detenerReaunudarEjecucion);



	}
}




void planificar_cola_exec(){
	while(1){
		sem_wait(&cola_ready);
		if(list_size(planificacion_cola_ready->elements)){
			sem_wait(&exec);
			sem_wait(&detenerReaunudarEjecucion);
			sem_post(&detenerReaunudarEjecucion);
			pthread_mutex_lock(&planificacion_mutex_ready);
			t_tripulante * tripulante = NULL;
				if(list_size(planificacion_cola_ready->elements)){
					tripulante = queue_pop(planificacion_cola_ready);
					//log_info(logger,"SACO de READY tripu: %d", tripulante->id);
				}
			pthread_mutex_unlock(&planificacion_mutex_ready);
			if(tripulante != NULL){
				sem_wait(&cola_exec);
				pthread_mutex_lock(&planificacion_mutex_exec);
				//queue_push(planificacion_cola_ready,tripulante);
				list_add(lista_exec,tripulante);
				log_info(logger,"T%d - P%d : EXEC", tripulante->id,tripulante->patota_id);
				pthread_mutex_unlock(&planificacion_mutex_exec);


				sem_post(&tripulante->exec);
			}
		}
		sem_wait(&detenerReaunudarEjecucion);
		sem_post(&detenerReaunudarEjecucion);
	}
}





void replanificar(){
	t_tripulante * tripulante = NULL;
	while(1){

		sem_wait(&colaEjecutados);

		pthread_mutex_lock(&mutex_cola_ejecutados);
		if(list_size(cola_ejecutados->elements)){
			tripulante = queue_pop(cola_ejecutados);
		}
		pthread_mutex_unlock(&mutex_cola_ejecutados);
		if(tripulante != NULL){
			sacar_de_exec(tripulante->id);
			sem_post(&cola_exec);



			log_trace(logger,"DISPATCHER - TRIPULANTE: %d , ESTADO: %c", tripulante->id, tripulante->estado);
			switch(tripulante->estado) {
				case 'F':{
					//MUTEX COLA FIN LOCK
					pthread_mutex_lock(&planificacion_mutex_fin);
					queue_push(planificacion_cola_fin,tripulante);
					log_info(logger,"T%d - P%d : FIN", tripulante->id,tripulante->patota_id);
					pthread_mutex_unlock(&planificacion_mutex_fin);
					sem_post(&cola_fin);

					//MUTEX COLA FIN UNLOCK
					break;
				}
				case 'B':{
					//MUTEX COLA BLOQ LOCK
	//				log_info(logger,"T%d - P%d : ESPERANDO PARA ENTRAR A BLOCK -> DURACION DE RAFAGA(CICLOS) = %d", tripulante->id,tripulante->patota_id,tripulante->block_io_rafaga);
					pthread_mutex_lock(&planificacion_mutex_bloq);
					queue_push(planificacion_cola_bloq,tripulante);
					log_info(logger,"T%d - P%d : BLOCK -> DURACION DE RAFAGA(CICLOS) = %d", tripulante->id,tripulante->patota_id,tripulante->block_io_rafaga);
					pthread_mutex_unlock(&planificacion_mutex_bloq);
					sem_post(&cola_bloq);
					break;
					//MUTEX COLA BLOQ UNLO
				}
				case 'R':{
					pthread_mutex_lock(&planificacion_mutex_ready);
					queue_push(planificacion_cola_ready,tripulante);
					log_info(logger,"T%d - P%d : READY", tripulante->id,tripulante->patota_id);
					pthread_mutex_unlock(&planificacion_mutex_ready);
					sem_post(&cola_ready);
					break;
				}
				default:
					break;

			}
		}
	}
}

void sacar_de_exec(int id_tripulante){

	bool encontrarTripulante(t_tripulante * tripulante){
		return tripulante->id == id_tripulante;
	}
	pthread_mutex_lock(&planificacion_mutex_exec);
	t_tripulante * data = (t_tripulante*) list_remove_by_condition(lista_exec,(void*) encontrarTripulante);
	log_trace(logger,"T%d - P%d : DISPATCHER", data->id,data->patota_id);
	pthread_mutex_unlock(&planificacion_mutex_exec);

}






void hilo_cola_exec(){
	pthread_attr_t attr1;
	pthread_attr_init(&attr1);
	pthread_attr_setdetachstate(&attr1, PTHREAD_CREATE_DETACHED);
	pthread_t hilo = (pthread_t)malloc(sizeof(pthread_t));
	pthread_create(&hilo , &attr1,(void*) planificar_cola_exec,NULL);

}
void hilo_cola_replanificar(){
	pthread_attr_t attr1;
	pthread_attr_init(&attr1);
	pthread_attr_setdetachstate(&attr1, PTHREAD_CREATE_DETACHED);
	pthread_t hilo = (pthread_t)malloc(sizeof(pthread_t));
	pthread_create(&hilo , &attr1,(void*) replanificar,NULL);

}




void hilo_cola_ready(){
	pthread_attr_t attr1;
	pthread_attr_init(&attr1);
	pthread_attr_setdetachstate(&attr1, PTHREAD_CREATE_DETACHED);
	pthread_t hilo = (pthread_t)malloc(sizeof(pthread_t));
	pthread_create(&hilo , &attr1,(void*) planificar_cola_ready,NULL);

}


void hilo_cola_bloq(){
	pthread_attr_t attr1;
	pthread_attr_init(&attr1);
	pthread_attr_setdetachstate(&attr1, PTHREAD_CREATE_DETACHED);
	pthread_t hilo = (pthread_t)malloc(sizeof(pthread_t));
	pthread_create(&hilo , &attr1,(void*) planificar_cola_bloq,NULL);
}


void atender_ram(){
	uint32_t notificacion;
	while(1){

		notificacion = recibirUint(socketServerMiRam);

		switch(notificacion){
			case PATOTA_CREADA:{


				int * patotaNew = (int *) queue_pop(planificacion_cola_new);

				log_info(logger,"PATOTA ID: %d - CARGADA EN MIRAM", *patotaNew);
				log_info(logger,"PATOTA ID: %d - MOVEMOS DE NEW A READY",*patotaNew);

				//patota patota = malloc(sizeof(patota));


				queue_push(planificacion_cola_ready, patotaNew);

				int * patotaReady = (int *) queue_pop(planificacion_cola_ready);

				log_info(logger,"PATOTA ID: %d  - EN READY", *patotaReady);
			}
		}
	}
}



void iniciarHiloConsola(){
	pthread_attr_t attr2;
	pthread_attr_init(&attr2);
	pthread_attr_setdetachstate(&attr2, PTHREAD_CREATE_DETACHED);
	pthread_create(&hiloConsola , &attr2,(void*) leer_consola,NULL);

	infoHilos * datosHilo = (infoHilos*) malloc(sizeof(infoHilos));
	datosHilo->socket = 0;
	datosHilo->hiloAtendedor = hiloConsola;

	pthread_mutex_lock(&mutexHilos);
	list_add(hilosParaConexiones, datosHilo);
	pthread_mutex_unlock(&mutexHilos);
}

void iniciarHiloQueManejadorNotifiacionesIMongo(){
	pthread_attr_t attr2;
	pthread_attr_init(&attr2);
	pthread_attr_setdetachstate(&attr2, PTHREAD_CREATE_DETACHED);
	pthread_create(&hiloConsola , &attr2,(void*) escuchoIMongo,NULL);

	infoHilos * datosHilo = (infoHilos*) malloc(sizeof(infoHilos));
	datosHilo->socket = 0;
	datosHilo->hiloAtendedor = hiloConsola;

	pthread_mutex_lock(&mutexHilos);
	list_add(hilosParaConexiones, datosHilo);
	pthread_mutex_unlock(&mutexHilos);
}

void crearHiloTripulante(t_tripulante * tripulante){
	pthread_attr_t attr2;
	pthread_attr_init(&attr2);
	pthread_attr_setdetachstate(&attr2, PTHREAD_CREATE_DETACHED);
	pthread_t hiloTripulante = (pthread_t)malloc(sizeof(pthread_t));
	pthread_create(&hiloTripulante , &attr2,(void*) labor_tripulante_new,(void*) tripulante);

//	infoHilos * datosHilo = (infoHilos*) malloc(sizeof(infoHilos));
//	datosHilo->socket = socket;
//	datosHilo->hiloAtendedor = hiloTripulante;
//
//	pthread_mutex_lock(&mutexHilos);
//	list_add(hilosParaConexiones, datosHilo);
//	pthread_mutex_unlock(&mutexHilos);
}

void planificar(){
	pthread_attr_t attr1;
	pthread_attr_init(&attr1);
	pthread_attr_setdetachstate(&attr1, PTHREAD_CREATE_DETACHED);
	pthread_create(&hiloPlanificador , &attr1,(void*) planificar_tripulantes,NULL);

	infoHilos * datosHilo = (infoHilos*) malloc(sizeof(infoHilos));
	datosHilo->socket = 0;
	datosHilo->hiloAtendedor = hiloPlanificador;

	pthread_mutex_lock(&mutexHilos);
	list_add(hilosParaConexiones, datosHilo);
	pthread_mutex_unlock(&mutexHilos);
}


void atenderLaRam(){
	pthread_attr_t attr1;
	pthread_attr_init(&attr1);
	pthread_attr_setdetachstate(&attr1, PTHREAD_CREATE_DETACHED);
	pthread_create(&hiloConsola , &attr1,(void*) atender_ram,NULL);

	infoHilos * datosHilo = (infoHilos*) malloc(sizeof(infoHilos));
	datosHilo->socket = 0;
	datosHilo->hiloAtendedor = hiloConsola;

	pthread_mutex_lock(&mutexHilos);
	list_add(hilosParaConexiones, datosHilo);
	pthread_mutex_unlock(&mutexHilos);
}


