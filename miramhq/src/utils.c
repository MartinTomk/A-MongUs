#include"utils.h"
#include "ADMIN_MIRAM.h"
#include "socket.h"
#include "mapa.h"

void iniciarEstructurasAdministrativas(){
	//lista_pcb = list_create();
	lista_tcb = list_create();
	pthread_mutex_init(&pthread_mutex_tcb_list,NULL);

	//pthread_mutex_init(&pthread_mutex_pcb_list,NULL);
}

void manejadorDeHilos(){
	int socketCliente;
	listaHilosAtendedores = list_create();
	// Funcion principal
	while((socketCliente = aceptarConexionDeCliente(server_fd))) { 	// hago el accept
		pthread_t * thread_id = malloc(sizeof(pthread_t));
    	pthread_attr_t attr;
    	pthread_attr_init(&attr);
    	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    	int * pcclient = malloc(sizeof(int));
    	*pcclient = socketCliente;
		//Creo hilo atendedor
		pthread_create( thread_id , &attr, (void*) atenderNotificacion , (void*) pcclient);
		list_add(listaHilosAtendedores,thread_id);
	}

	//Chequeo que no falle el accept
}

void enviar_tarea(int socket, char * tarea) {
	char* claveNueva = tarea;
	int largoClave = string_length(claveNueva);
	int tamanio = 0;
	//En el buffer mando clave y luego valor
	void* buffer = malloc(string_length(claveNueva) + sizeof(uint32_t));
	memcpy(buffer + tamanio, &largoClave, sizeof(uint32_t));
	tamanio += sizeof(uint32_t);
	memcpy(buffer + tamanio, claveNueva, string_length(claveNueva));
	tamanio += largoClave;
	sendRemasterizado(socket, ENVIAR_TAREA, tamanio, (void*) buffer);
	free(buffer);
}

void *atenderNotificacion(void * paqueteSocket){

	int socket = *(int*)paqueteSocket;
	free(paqueteSocket);
	while(1){

	uint32_t nroNotificacion = recvDeNotificacion(socket);
	log_debug(logger,"recibo notificacion n# %d de socket %d",nroNotificacion, socket);

	switch(nroNotificacion){



		case DISCORDIADOR:{
			//Case para hacer HANDSHAKE = Chequear la conexion
			log_info(logger,"Se ha conectado el DISCORDIADOR");
			sendDeNotificacion(socket, MIRAM);
		}

			break;


		case CREAR_PATOTA:{
			int state;
			log_info(logger,"\n");
			log_info(logger, "----------------CREANDO PATOTA----------------");
			pthread_mutex_lock(&accesoListaTablas);
			pthread_mutex_lock(&accesoMemoria);
			state = crear_pcb(socket);
			pthread_mutex_unlock(&accesoMemoria);
			pthread_mutex_unlock(&accesoListaTablas);
			if(state == PATOTA_CREADA){
			sendDeNotificacion(socket, PATOTA_CREADA);
			}
			else {
			sendDeNotificacion(socket,ERROR);
			}
			close(socket);
			return 0;
			break;
		}
		case PEDIR_TAREA:{
			uint32_t id_trip = recvDeNotificacion(socket);
			uint32_t id_patota = recvDeNotificacion(socket);
			uint32_t direccionLogica = recvDeNotificacion(socket);
			pthread_mutex_lock(&accesoListaTablas);
			pthread_mutex_lock(&accesoMemoria);
			log_info(logger,"\n");
			log_info(logger, "----------------Pedir tarea----------------");
			tabla_t* tabla = buscarTablaId(id_patota);
			tcb_t* tcb = getDato(id_patota,sizeof(tcb_t),direccionLogica);
			log_info(logger,"Pide tarea el id_tripulante: %d DirLog: %i, Tarea: %i",tcb->id,direccionLogica,tcb->prox_tarea);
			uint32_t tamanioProximaInstruccion = reconocerTamanioInstruccion3(tcb->prox_tarea,tabla);
			char * tarea = getInstruccion(id_patota,tamanioProximaInstruccion,tcb->prox_tarea);
			tcb->prox_tarea += tamanioProximaInstruccion + 1; //ELIMINO \N;
			log_info(logger,"La tarea encontrada es %s",tarea);
			actualizarDato(tabla,tcb,sizeof(tcb_t),direccionLogica);
			pthread_mutex_unlock(&accesoListaTablas);
			pthread_mutex_unlock(&accesoMemoria);
			free(tcb);


			enviar_tarea(socket,tarea);
			if(strcmp(tarea,"FIN\0")){
				free(tarea);
			}


			break;

		}
		case PEDIR_UBICACION:{
			uint32_t trip_id = recvDeNotificacion(socket);
			uint32_t patota_id = recvDeNotificacion(socket);
			uint32_t direccionLogica = recvDeNotificacion(socket);
			pthread_mutex_lock(&accesoListaTablas);
			pthread_mutex_lock(&accesoMemoria);
			log_info(logger,"\n");
			log_info(logger,"--------------   Pedir Ubicacion -------------------");
			log_info(logger, "El tripulante %i solicita ubicacion",trip_id);
			tcb_t* tcb = getDato(patota_id,sizeof(tcb_t),direccionLogica);
			pthread_mutex_unlock(&accesoMemoria);
			pthread_mutex_unlock(&accesoListaTablas);
			sendDeNotificacion(socket, tcb->x);
			sendDeNotificacion(socket, tcb->y);
			log_info(logger, "Ubicacion enviada al discordiador");
			free(tcb);
			break;

		}

		case ACTUALIZAR_ESTADO_MIRAM:{
			uint32_t id_trip = recvDeNotificacion(socket);
			uint32_t id_patota = recvDeNotificacion(socket);
			uint32_t direccionLogica = recvDeNotificacion(socket);
			pthread_mutex_lock(&accesoListaTablas);
			pthread_mutex_lock(&accesoMemoria);
			log_info(logger,"\n");
			log_info(logger,"--------------   Actualizar Estado -------------------");
			char estadoV[5] = {'N','R','B','E','F'};
			tabla_t* tabla = buscarTablaId(id_patota);
			uint32_t estado = recvDeNotificacion(socket);
			tcb_t* tcb = getDato(id_patota,sizeof(tcb_t),direccionLogica);
			log_info(logger, "Actualizando estado anterior %c -> actual ->%c",tcb->estado,estadoV[estado]);
			tcb->estado = estadoV[estado];
			actualizarDato(tabla,tcb,sizeof(tcb_t),direccionLogica);
			pthread_mutex_unlock(&accesoMemoria);
			pthread_mutex_unlock(&accesoListaTablas);
			free(tcb);
			sendDeNotificacion(socket, ESTADO_ACTUALIZADO_MIRAM);
			log_info(logger, "Estado actualizado tripulante: %d, estado: %c",id_trip,estadoV[estado]);
			sem_post(&actualizarMapa);
			break;

		}

		case ACTUALIZAR_UBICACION:{
			uint32_t id_trip = recvDeNotificacion(socket);
			uint32_t id_patota = recvDeNotificacion(socket);
			uint32_t direccionLogica = recvDeNotificacion(socket);
			uint32_t x = recvDeNotificacion(socket);
			uint32_t y = recvDeNotificacion(socket);
			pthread_mutex_lock(&accesoListaTablas);
			pthread_mutex_lock(&accesoMemoria);
			log_info(logger,"\n");
			log_info(logger,"--------------   Actualizar Ubicacion -------------------");
			tabla_t* tabla = buscarTablaId(id_patota);
			tcb_t* tcb = getDato(id_patota,sizeof(tcb_t),direccionLogica);
			if(mapaActivo){
			int dx = x - tcb->x;
			int dy = y - tcb->y;
			item_desplazar(nivel,tcb->id+64, dx,dy);
			}
			sem_post(&actualizarMapa);
			tcb->x = x;
			tcb->y = y;

			actualizarDato(tabla,tcb,sizeof(tcb_t),direccionLogica);
			pthread_mutex_unlock(&accesoMemoria);
			pthread_mutex_unlock(&accesoListaTablas);



			sendDeNotificacion(socket, UBICACION_ACTUALIZADA);
			free(tcb);

			log_info(logger, "Ubicacion actualizada tripulante: %d, ubicacion-> X: %d ; Y: %d",id_trip,x,y);

			break;

		}

		case FIN_TAREAS: {
			uint32_t id_trip = recvDeNotificacion(socket);
			uint32_t id_patota = recvDeNotificacion(socket);
			uint32_t direccionLogica = recvDeNotificacion(socket);
			pthread_mutex_lock(&accesoListaTablas);
			pthread_mutex_lock(&accesoMemoria);
			log_info(logger,"\n");
			log_info(logger,"--------------   Fin tripulante -------------------");
			tabla_t* tabla = buscarTablaId(id_patota);
			eliminar_tripulante(tabla,direccionLogica);
			//eliminarTabla(tabla,confDatos.esquema);
			log_info(logger,"Fin de tripulante %i Patota: %i DirLog: %i",id_trip,id_patota,direccionLogica);
			pthread_mutex_unlock(&accesoListaTablas);
			pthread_mutex_unlock(&accesoMemoria);
			if(mapaActivo){
				item_borrar(nivel,id_trip+64);
			}
			sendDeNotificacion(socket,1);
			close(socket);
			//liberarMemoriaHilos();
			sem_post(&actualizarMapa);
			return 0;
			break;
		}
		case COMPACTACION: {
			pthread_mutex_lock(&accesoListaTablas);
			pthread_mutex_lock(&accesoMemoria);
			compactar_memoria();
			pthread_mutex_unlock(&accesoListaTablas);
			pthread_mutex_unlock(&accesoMemoria);
			break;
		}
		case DUMP: {
			pthread_mutex_lock(&accesoListaTablas);
			pthread_mutex_lock(&accesoMemoria);
			list_iterate(listaSegmentos,mostrarEstadoMemoria);
			pthread_mutex_unlock(&accesoListaTablas);
			pthread_mutex_unlock(&accesoMemoria);
			break;
		}
		case EXPULSAR_TRIPULANTE: {
			log_info(logger,"--------------   Expulsar tripulante -------------------");
			uint32_t id_trip = recvDeNotificacion(socket);
			uint32_t id_patota = recvDeNotificacion(socket);
			uint32_t direccionLogica = recvDeNotificacion(socket);
			pthread_mutex_lock(&accesoListaTablas);
			pthread_mutex_lock(&accesoMemoria);
			log_info(logger,"\n");
			log_info(logger,"--------------   Expulsar tripulante -------------------");
			tabla_t* tabla = buscarTablaId(id_patota);
			eliminar_tripulante(tabla,direccionLogica);
			log_info(logger,"Fin de tripulante %i Patota: %i DirLog: %i",id_trip,id_patota,direccionLogica);
			if(mapaActivo){
				item_borrar(nivel,id_trip+64);
			}
			//eliminarTabla(tabla,confDatos.esquema);
			pthread_mutex_unlock(&accesoListaTablas);
			pthread_mutex_unlock(&accesoMemoria);
			close(socket);
			sem_post(&actualizarMapa);
			return 0;
			break;
		}
		case CREAR_TRIPULANTE:{
			uint32_t id_trip = recvDeNotificacion(socket);
			uint32_t id_patota = recvDeNotificacion(socket);
			uint32_t x = recvDeNotificacion(socket);
			uint32_t y = recvDeNotificacion(socket);
			tcb_t tcb;
			tcb.x = x;
			tcb.y = y;
			tcb.id = id_trip;
			pthread_mutex_lock(&accesoListaTablas);
			pthread_mutex_lock(&accesoMemoria);
			log_info(logger,"\n");
			log_info(logger,"--------------   Crear tripulante -------------------");
			tabla_t* tabla = buscarTablaId(id_patota);
			crear_tripulante_(tcb,id_patota,tabla);
			if(!strcmp(confDatos.esquema,"PAGINACION")){
				sendDeNotificacion(socket,tabla->ocupado - sizeof(tcb_t));
			}
			if(!strcmp(confDatos.esquema,"SEGMENTACION")){
				sendDeNotificacion(socket,tabla->ocupado - 1);
			}
			if(mapaActivo){
			err = personaje_crear(nivel,tcb.id+64,tcb.x, tcb.y);
			}
			sendDeNotificacion(socket,1);
			pthread_mutex_unlock(&accesoListaTablas);
			pthread_mutex_unlock(&accesoMemoria);
			break;
		}
		case CERRAR_CONEXION:
			close(socket);
			return 0;
		default:
			log_warning(logger, "La conexion recibida es erronea");
			close(socket);
			return 0;

	}
	}

	return 0;

}


int crear_pcb(int socket) {
	char* tareaRam = recibirString(socket);
	char* id_posiciones = recibirString(socket);
	uint32_t patotaid = recibirUint(socket);
	uint32_t cantidad_patota = recibirUint(socket);

	//Nuevo
	pcb_t pcbRam;
	pcbRam.id = patotaid;
	tabla_t* tablaPatota = malloc(sizeof(tabla_t));
	if (crear_patota_(pcbRam,tareaRam,cantidad_patota,tablaPatota) != PATOTA_CREADA){
		sendDeNotificacion(socket,ERROR);
		log_error(logger,"No se pudo crear la patota");
		free(tareaRam);
		free(id_posiciones);
		free(tablaPatota);
		return ERROR;
	}
	sendDeNotificacion(socket,PATOTA_CREADA);
	/*for(int i = 0 ; i<cantidad_patota; i++){
		int * id = malloc(sizeof(int));
		*id = (int)recibirUint(socket);
		log_info(logger,"Trip Creado OK");
		t_tripulante * trip = malloc(sizeof(t_tripulante));
		trip->id =*id;
		trip->socket = socket;
		pthread_mutex_lock(&pthread_mutex_tcb_list);
		list_add(lista_tcb,trip);
		pthread_mutex_unlock(&pthread_mutex_tcb_list);
		char * coordenadas = string_new();
		asignar_posicion(&coordenadas,id_posiciones,i);
		char ** coordenadas_posicion_inicial = string_split(coordenadas,"|");

		tcb_t temp;
		temp.x = (uint32_t)atoi(coordenadas_posicion_inicial[0]);
		temp.y = (uint32_t)atoi(coordenadas_posicion_inicial[1]);
		temp.id = *id;
		free(coordenadas);
		liberarCadenaDoble(coordenadas_posicion_inicial);
		if(mapaActivo){
		err = personaje_crear(nivel,temp.id,temp.x, temp.y);
		}
		log_debug(logger,"Por crear trip %i",temp.id);
		crear_tripulante_(temp,patotaid,tablaPatota);
		if(!strcmp(confDatos.esquema,"PAGINACION")){
			log_debug(logger,"Dir log: %i",(tablaPatota->ocupado));
			sendDeNotificacion(socket,(tablaPatota->ocupado)-sizeof(tcb_t));
		}
		if(!strcmp(confDatos.esquema,"SEGMENTACION")) {
			sendDeNotificacion(socket,(tablaPatota->ocupado)-1);
		}

		log_info(logger,"Tripulante %i creado",temp.id);
		sendDeNotificacion(socket,77);

		free(id); //malloc linea 79 dentro de este while
	}
	if(mapaActivo && tareasActivas){
		crear_tareas(tareaRam); //EN MAPA!!
	}*/
	free(tareaRam);
	free(id_posiciones);
	return PATOTA_CREADA;

}

void asignar_posicion(char** destino,char* posiciones,uint32_t creados) {
	char** posiciones_separadas = string_split(posiciones," ");
	uint32_t cantidad_posiciones = 0;
	while(posiciones_separadas[cantidad_posiciones] != NULL) {
		cantidad_posiciones++;
	}
	if(cantidad_posiciones <= creados){
		string_append(destino,"0|0");
	}
	else {
		string_append(destino,posiciones_separadas[creados]);
	}
	liberarCadenaDoble(posiciones_separadas);
}


void liberarCadenaDoble(char** cadena){
	uint32_t i = 0;
	while(cadena[i] != NULL){
		free(cadena[i]);
		i++;
	}
	free(cadena[i]);
	free(cadena);
}

