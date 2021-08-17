/*
 * conexiones.c
 *
 *  Created on: 2 mar. 2019
 *      Author: utnso
 */

#include "utils.h"


void iniciar_configuracion(){

	tripulantes_creados = 0;
	patotas_creadas = 0;
	sabotaje = false;
	t_config* config = leer_config();

	IP_MONGO = config_get_string_value(config, "IP_MONGO");

	IP_MIRAM = config_get_string_value(config, "IP_MIRAM");

	PUERTO_MONGO = config_get_int_value(config,"PUERTO_MONGO_STORE");

	PUERTO_MIRAM = config_get_int_value(config,"PUERTO_MIRAM");

	GRADO_MULTIPROGRAMACION = config_get_int_value(config,"GRADO_MULTIPROGRAMACION");

	ALGORITMO = config_get_string_value(config,"ALGORITMO");

	QUANTUM = config_get_int_value(config,"QUANTUM");

	CICLO_CPU = config_get_int_value(config,"CICLO_CPU");

	CICLO_IO = config_get_int_value(config,"CICLO_IO");

	conexiones = list_create();
}

void iniciarEstructurasAdministrativasPlanificador(){
	log_info(logger, "GENERANDO ESTRUCTURAS ADMINISTRATIVAS!");

	sem_init(&iniciar_planificacion, 0, 0);

	sem_init(&detenerReaunudarEjecucion, 0, 1);
	sem_init(&activar_actualizaciones_mongo, 0, 0);

	sem_init(&cola_new, 0, 0);
	sem_init(&cola_ready, 0, 0);
	sem_init(&cola_exec, 0, GRADO_MULTIPROGRAMACION);
	sem_init(&cola_bloq, 0, 0);
	sem_init(&cola_fin,0,0);

	sem_init(&colaEjecutados, 0, 0);
	sem_init(&expulsarEnCurso, 0, 1);
	sem_init(&exec, 0, GRADO_MULTIPROGRAMACION);
	sem_init(&terminarPrograma,0,0);
	sem_init(&sabotajeEnCurso,0,1);
	pthread_mutex_init(&planificacion_mutex_new,NULL);
	pthread_mutex_init(&planificacion_mutex_ready,NULL);
	pthread_mutex_init(&planificacion_mutex_exec,NULL);
	pthread_mutex_init(&planificacion_mutex_bloq,NULL);
	pthread_mutex_init(&planificacion_mutex_fin,NULL);
	pthread_mutex_init(&mutex_cola_ejecutados,NULL);

	pthread_mutex_init(&comuni,NULL);

	planificacion_cola_new = queue_create();
	planificacion_cola_ready = queue_create();
	planificacion_cola_exec = queue_create();
	planificacion_cola_fin = queue_create();
	planificacion_cola_bloq = queue_create();

	cola_ejecutados = queue_create();

	lista_exec = list_create();
	hilosParaConexiones = list_create();

	pthread_mutex_init(&mutexHilos,NULL);
}


int terminar_discordiador(int signal) {
	cerrar_conexiones_hilos(logger);
	log_destroy(logger);
	return 0;
}

int terminar_programa(t_log* logger,t_config* config) {
	cerrar_conexiones_hilos(logger);
	log_destroy(logger);
	config_destroy(config);
	return 0;
}

void liberar_conexion_e(int* conexion){
	close(*conexion);
}
int cerrar_conexiones_hilos(t_log* logger){
	//list_destroy_and_destroy_elements(conexiones,liberar_conexion_e);
	log_info(logger,"Deteniendo planificacion para terminar, por favor espere");
	sem_wait(&detenerReaunudarEjecucion);
	sleep(2);
	/*log_info(logger,"Planificacion detenida");
	log_info(logger,"Cerrando conexiones y liberando memoria");
	log_info(logger,"Liberando recursos de new");
	eliminar_cola(planificacion_cola_new,planificacion_mutex_new,logger);
	log_info(logger,"Liberando recursos de ready");
	eliminar_cola(planificacion_cola_ready,planificacion_mutex_ready,logger);
	log_info(logger,"Liberando recursos de bloq");
	eliminar_cola(planificacion_cola_bloq,planificacion_mutex_bloq,logger);
	log_info(logger,"Liberando recursos de fin");*/
	eliminar_cola(planificacion_cola_fin,planificacion_mutex_fin,logger);
	//Quizas hay problemas con la cola de ejecutados
	log_info(logger,"Liberando recursos de exec");
	/*queue_destroy(planificacion_cola_new);
	queue_destroy(planificacion_cola_ready);
	list_destroy(lista_exec);
	queue_destroy(cola_ejecutados);*/
	//config_destroy(config);
	log_info(logger,"Cerrando hilos");
	return 0;
}
int eliminar_cola(t_queue* cola, pthread_mutex_t mutex_cola,t_log* logger) {
	t_tripulante* tripulante;
	pthread_mutex_lock(&mutex_cola);
	int cantidad_elementos = queue_size(cola);
	int i = 0;
	log_info(logger,"Hay %i tripulantes en este estado", cantidad_elementos);
	while(i < cantidad_elementos) {
		tripulante = queue_pop(cola);
		close(tripulante->socket);
		//Si hilo asociado es el posta hay que matarlo
		free(tripulante);
		i++;
		log_info(logger,"%i <<<<< >>>>>> %i",cantidad_elementos,i);
	}
	queue_destroy(cola);
	pthread_mutex_unlock(&mutex_cola);
	log_info(logger,"Fueron liberados %i tripulantes", i);
	return 0;
}
int eliminar_list(t_list* lista,pthread_mutex_t mutex_lista,t_log* logger) {
	t_tripulante* tripulante;
	pthread_mutex_lock(&mutex_lista);
	int cantidad_elementos = list_size(lista);
	int i = 0;
	log_info(logger,"Hay %i tripulantes en este estado", cantidad_elementos);
	while(i < cantidad_elementos) {
		tripulante = list_get(lista,0);
		close(tripulante->socket);
		//Si hilo asociado es el posta hay que matarlo
		free(tripulante);
		i++;
	}
	list_destroy(lista);
	pthread_mutex_unlock(&mutex_lista);
	log_info(logger,"Fueron liberados %i tripulantes", i);
	return 0;
}
void iniciar_logger() {
	if( (logger = log_create("discordiador.log", "DISCORDIADOR", 1, LOG_LEVEL_INFO))==NULL){
		printf("No se pudo crear el logger. Revise parametros\n");
		exit(1);
	}
}

t_config* leer_config() {
	t_config *config;
	if((config = config_create("discordiador.config"))==NULL) {
		printf("No se pudo leer de la config. Revise. \n");
		exit(1);
	}
	return config;
}
void liberarCadenaDoble(char** cadena){
	uint32_t i = 0;
	while(cadena[i] != NULL){
		free(cadena[i]);
		i++;
	}
	free(cadena);
}


