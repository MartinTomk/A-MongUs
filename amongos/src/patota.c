/*
 * patota.c
 *
 *  Created on: 11 jun. 2021
 *      Author: utnso
 */
#include "patota.h"


//bool

bool sortId(void* dato, void* otroDato){
	t_tripulante* primero = (t_tripulante*)dato;
	t_tripulante* segundo = (t_tripulante*)otroDato;
	return primero->id > segundo->id;
}
void crear_patota(char * comando){
	socketServerMiRam = reConectarAServer(IP_MIRAM, PUERTO_MIRAM);
	if(socketServerMiRam<0){
		log_info(logger,"no me pude conectar con mi ram");
		return ;
	}else{

	t_list * list_trip_aux = list_create();

	char ** parametros = string_n_split(comando,4," ");

	if (access(parametros[1], R_OK | W_OK) != 0) {

		log_error(logger, "La tarea no existe, verifique la ruta de la misma: %s", parametros[1]);
		return;
	}

	char * tareasX = string_new();
    FILE *archivo = fopen(parametros[1], "r"); // Modo lectura
    char bufer[1000];         // Aquí vamos a ir almacenando cada línea
    int cantidad_tareas = 0;
    while (fgets(bufer, 1000, archivo))
    {
		string_append(&tareasX,bufer);
		cantidad_tareas++;
    }
    char * tareasOk = string_substring_until(tareasX,string_length(tareasX));



    log_trace(logger,"%s",tareasOk);


	int longitud_tareas = string_length(tareasOk);

	uint32_t cantidad_tripulantes = (uint32_t)atoi(parametros[2]);


	char * posiciones = string_new();
	char** posiciones_separadas = NULL;
	int longitud_posiciones;
	bool posicionesB = false;
	if(parametros[3] != NULL){
		log_info(logger,"Posiciones %s",parametros [3]);
		posiciones_separadas = string_split(parametros[3]," ");
		log_info(logger,"Posiciones Separadas: %s",posiciones_separadas[0]);
		posicionesB = true;
	}
	string_append(&posiciones,"aaa");
	longitud_posiciones = string_length(posiciones);
	char * claveGet = string_new();
	string_append(&claveGet,tareasOk);
	string_append(&claveGet,posiciones);

	int tamanioGet = 0;
	patotas_creadas++;
	void* buffer_patota = crear_buffer_patota(longitud_tareas,
			longitud_posiciones, patotas_creadas, cantidad_tripulantes,
			&tamanioGet, tareasOk, posiciones);

	sendRemasterizado(socketServerMiRam, CREAR_PATOTA,tamanioGet,buffer_patota);
	/*if(recvDeNotificacion(socketServerMiRam) == PATOTA_CREADA){
		log_error(logger,"No se pudo crear");
		return false;
	}
	*/
	int state = recvDeNotificacion(socketServerMiRam);
	if (state == PATOTA_CREADA) {
	for(int i = 0 ; i<cantidad_tripulantes; i++){
		tripulantes_creados++;
		/*sendDeNotificacion(socketServerMiRam,tripulantes_creados);*/

		int * id = malloc(sizeof(int));
		t_tripulante * _tripulante = (t_tripulante*)malloc(sizeof(t_tripulante));
		*id = tripulantes_creados;
		_tripulante->id = tripulantes_creados;
		_tripulante->patota_id = patotas_creadas;
		if(posicionesB && parametros[3] != NULL && posiciones_separadas[i] != NULL){
			_tripulante->ubi_x = atoi((&(posiciones_separadas[i])[0]));
			_tripulante->ubi_y = atoi((&(posiciones_separadas[i])[2]));
		}
		else{
			_tripulante->ubi_x = 0;
			_tripulante->ubi_y = 0;
		}
		/*_tripulante->direccionLogica = recvDeNotificacion(socketServerMiRam);*/
		/*recvDeNotificacion(socketServerMiRam);*/
		sem_init(&_tripulante->creacion,0,0);

		list_add(list_trip_aux,_tripulante);

		log_debug(logger,"Creando tripulante: %d de la patota id: %d ",*id,patotas_creadas);
		crearHiloTripulante(_tripulante);
		free(id); //malloc linea 79 dentro de este while
	}
	}
	if(recvDeNotificacion(socketServerMiRam) == PATOTA_CREADA){
	/*if(list_size(list_trip_aux)>1){
		list_sort(list_trip_aux,sortId);
		//Para que lleguen a new por id
	}*/
	while(list_size(list_trip_aux)!=0){
		t_tripulante * trip = list_remove(list_trip_aux,0);
		sem_post(&trip->creacion);
	}
	list_destroy(list_trip_aux);
	}
	else {
		log_error(logger,"No se pudo crear la patota");
	}
	if(posicionesB)
	{
		liberarCadenaDoble(posiciones_separadas);
	}
	//crear lista de ids y enviar
	//cuando el tripulante se conecte a miram, atenderlo, buscar su tcb y agregarlo a la lista de sockets conocidos
	free(buffer_patota); //Se hace el malloc dentro de crear_buffer_patota
	free(claveGet); //malloc linea 60
	free(posiciones); //stringNew linea 52
	free(tareasX);//stringNew linea 18
	free(tareasOk); // linea 31INICIAR_PATOTA plantas.txt 2 1|2 5|5
	liberarCadenaDoble(parametros);
	}
}
//Mallocs Revisados
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
}

void* crear_buffer_patota(int longitud_tareas, int longitud_posiciones, uint32_t patotaId, uint32_t cantidad_tripulantes, int* tamanioGet, char* tareas, char* posiciones) {
	void* buffer_patota = malloc(longitud_tareas + longitud_posiciones + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(int) + sizeof(int));

	memcpy(buffer_patota + *tamanioGet, &longitud_tareas, sizeof(int));
	*tamanioGet += sizeof(int);

	memcpy(buffer_patota + *tamanioGet, tareas, longitud_tareas);
	*tamanioGet += longitud_tareas;

	memcpy(buffer_patota + *tamanioGet, &longitud_posiciones, sizeof(int));
	*tamanioGet += sizeof(int);

	memcpy(buffer_patota + *tamanioGet, posiciones, longitud_posiciones);
	*tamanioGet += longitud_posiciones;

	memcpy(buffer_patota + *tamanioGet, &patotaId, sizeof(uint32_t));
	*tamanioGet += sizeof(uint32_t);

	memcpy(buffer_patota + *tamanioGet, &cantidad_tripulantes,
			sizeof(uint32_t));
	*tamanioGet += sizeof(uint32_t);
	return buffer_patota;
}
//Mallocs Revisados

