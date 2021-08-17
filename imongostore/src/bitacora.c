/*
 * bitacora.c
 *
 *  Created on: 11 jul. 2021
 *      Author: utnso
 */

#include "bitacora.h"

_archivo_bitacora * iniciar_archivo_bitacora(char * tripulante,char * key_file){

	_archivo_bitacora *archivo;


	char *resto_path = string_new();
	string_append_with_format(&resto_path,"%s.ims",tripulante);


	char *aux = NULL;
	char *path_files = NULL;
	aux = string_duplicate(conf_PUNTO_MONTAJE);
	path_files = string_duplicate(conf_PATH_BITACORA);
	string_append_with_format(&aux, "%s%s", path_files,resto_path);
	log_trace(logger,"iniciar_archivo_bitacora(): Ruta archivo bitacora -> %s",aux);

	if( access( aux, F_OK ) == 0 ) {
		log_debug(logger,"iniciar_archivo_bitacora(): YA EXISTE ARCHIVO BITACORA : %s", tripulante);
		//Si tengo acceso al archivo, creo el config
		archivo = find_bitacora(archivos_bitacora,tripulante);
	}else
	{
		archivo = (_archivo_bitacora*)malloc(sizeof(_archivo_bitacora));
		archivo->clave = string_new();
		string_append(&(archivo->clave),tripulante);
		log_debug(logger,"iniciar_archivo_bitacora(): CREO NUEVO ARCHIVO BITACORA : %s", tripulante);
		FILE * metadata = fopen(aux,"a+");
		close(metadata);


		(archivo->metadata) = config_create(aux);
		pthread_mutex_lock(&mutex_archivos_bitacora);
		list_add(archivos_bitacora,archivo);
		pthread_mutex_unlock(&mutex_archivos_bitacora);
	}




	FILE * metadata = fopen(aux,"a+");
//	FILE * metadata = open(resto_path,  O_APPEND | O_CREAT, 0644);
	int c = fgetc(metadata);
	if (c == EOF) {

		log_debug(logger,"iniciar_archivo_bitacora(): ESTA VACIO EL ARCHIVO BITACORA DE: %s", tripulante);
		config_set_value(archivo->metadata,"SIZE","0");
		config_set_value(archivo->metadata,"BLOCKS","[]");
		config_save_in_file(archivo->metadata,aux);


	} else {
	    ungetc(c, metadata);
	}
	free(path_files);
	free(aux);
	free(resto_path);//+
	free(tripulante);


	//config_destroy(archivo->metadata);
	pthread_mutex_init(&((archivo)->mutex_file), NULL);
	return archivo;

}



uint32_t write_archivo_bitacora(char* cadenaAGuardar,_archivo_bitacora * archivo){
	string_append(&cadenaAGuardar,".");
	pthread_mutex_lock(&(archivo->mutex_file));

	uint32_t resultado;
	int bytesArchivo = config_get_int_value(archivo->metadata,"SIZE");

	log_trace(logger,"bytes archivo %s : %d",archivo->clave,bytesArchivo);

	pthread_mutex_lock(&_blocks.mutex_blocks);
	pthread_mutex_lock(&superblock.mutex_superbloque);
	//Chequeo si hay lugar en el ultimo bloque
	if(bytesArchivo%superblock.tamanio_bloque==0){

		//Si el ultiimo bloque esta completo -> creo los bloques nuevos que necesito para almacenar la cadena
		llenar_nuevo_bloque_bitacora(cadenaAGuardar, archivo);

	}else{
		//bytesArchivo : restas sucesivas para quedarme con el espacio OCUPADO del ultimo bloque.
		while(bytesArchivo>superblock.tamanio_bloque) bytesArchivo-=superblock.tamanio_bloque;

		int espacioLibreUltimoBloque = superblock.tamanio_bloque-bytesArchivo;

		char ** blocks = config_get_array_value(archivo->metadata,"BLOCKS");

		int longitud_blocks = longitud_array(blocks);
		char * last_block = blocks[longitud_blocks-1]; // TODO ESTO FUNCIONA ASI, PERO ESTA BIEN ¿¿¿

		if(string_length(cadenaAGuardar)<=espacioLibreUltimoBloque){

			log_debug(logger,"indice de BLOQUE :%d con espacio para archivo: %s, ",atoi(last_block),archivo->clave);

			write_blocks_with_offset_bitacora(cadenaAGuardar,atoi(last_block),bytesArchivo);

			//TODO: podria armar un actualizar metadata mas generico con varios if
			actualizar_metadata_sin_crear_bloque_bitacora(archivo,cadenaAGuardar);

		}else{

			char * rellenoDeUltimoBloque  = string_substring_until(cadenaAGuardar,espacioLibreUltimoBloque);

			write_blocks_with_offset_bitacora(rellenoDeUltimoBloque,atoi(last_block),bytesArchivo);

			actualizar_metadata_sin_crear_bloque_bitacora(archivo,rellenoDeUltimoBloque);

			char * contenidoProximoBloque = string_substring_from(cadenaAGuardar,espacioLibreUltimoBloque);

			llenar_nuevo_bloque_bitacora(contenidoProximoBloque, archivo);

			free(contenidoProximoBloque);//+

			free(rellenoDeUltimoBloque);



		}
		for(int i = 0 ; i<longitud_blocks; i++){

			free(blocks[i]);
		}
		free(blocks);
	}
	log_trace(logger,"%s: ----------- COPIA blocks.ims:",_blocks.fs_bloques);
	pthread_mutex_unlock(&superblock.mutex_superbloque);
	pthread_mutex_unlock(&_blocks.mutex_blocks);
	pthread_mutex_unlock(&(archivo->mutex_file));
	free(cadenaAGuardar);
	return 1;
}




void actualizar_metadata_sin_crear_bloque_bitacora(_archivo_bitacora * archivo,char * valorAux){

	int bytes = string_length(valorAux);
	int size = config_get_int_value(archivo->metadata,"SIZE");
	size+=bytes;
	char * bytesString = string_itoa(size);
	config_set_value(archivo->metadata,"SIZE",bytesString);

	config_save(archivo->metadata);

	free(bytesString);

}


void actualizar_metadata_bitacora(_archivo_bitacora * archivo,int indice_bloque,char * valorAux){


	string_length(valorAux);
	int bytes = string_length(valorAux);

	int size = config_get_int_value(archivo->metadata,"SIZE");
	size+=bytes;
	char * bytesString = string_itoa(size);
	config_set_value(archivo->metadata,"SIZE",bytesString);
	free(bytesString);


	char ** array;
	array = config_get_array_value(archivo->metadata,"BLOCKS");
	char * indice_bloque_string = string_itoa(indice_bloque);
	char ** nuevo  = agregar_en_array(array,indice_bloque_string);
	free(indice_bloque_string);

	char * cadena = array_to_string(nuevo);
	int i = 0;
	while(nuevo[i]!=NULL){
		free(nuevo[i]);
		i++;
	}
	config_set_value(archivo->metadata,"BLOCKS",cadena);
	config_save(archivo->metadata);
	
	free(nuevo);
	free(cadena);
}



void llenar_nuevo_bloque_bitacora(char* cadenaAGuardar, _archivo_bitacora* archivo) {
	//Si el ultiimo bloque esta completo -> creo los bloques nuevos que necesito para almacenar la cadena
	int posicionesStorageAOcupar = calcular_cantidad_bloques_requeridos(
			cadenaAGuardar);
	int offsetBytesAlmacenados = 0;
	int bloqueslibres = calcular_bloques_libres();
	for (int i = 0; i < posicionesStorageAOcupar; i++) {
		char* valorAux = string_substring(cadenaAGuardar,
				offsetBytesAlmacenados, superblock.tamanio_bloque);
		int indice_bloque = obtener_indice_para_guardar_en_bloque_bitacora(valorAux);
		log_trace(logger, "llenar_nuevo_bloque_bitacora(): Bitacora: %s -> bloque asignado: %d", archivo->clave,
				indice_bloque);
		write_blocks(valorAux, indice_bloque);
		actualizar_metadata_bitacora(archivo, indice_bloque, valorAux);
		bitarray_set_bit_monitor(indice_bloque);
		offsetBytesAlmacenados += superblock.tamanio_bloque;
		free(valorAux);
	}
}

int obtener_indice_para_guardar_en_bloque_bitacora(char * valor){
	int lugares = calcular_cantidad_bloques_requeridos(valor);
	int cont = 0;
	int i;
	int resultado = 0;
	int cantidadDePosiciones = superblock.cantidad_bloques;

	for(i=0;i<cantidadDePosiciones;i++){

		if(bitarray_test_bit(superblock.bitmap,i)){
			cont = 0;
		} else{
			cont++;
		}
		if(cont >= lugares){
			return i - lugares + 1;
			break;
		}
	}
	return 99999;
}

int calcular_cantidad_bloques_requeridos_bitacora(char* cadenaAGuardar){
	int cantidadBloques = string_length(cadenaAGuardar)/superblock.tamanio_bloque;

	if(string_length(cadenaAGuardar) % superblock.tamanio_bloque > 0 ){
		cantidadBloques++;
	}

	return cantidadBloques;
}


int write_blocks_with_offset_bitacora(char * cadena_caracteres,int indice,int offset) {
//	int padding = superblock.tamanio_bloque - offset-strlen(cadena_caracteres);
//	char * pad = string_repeat('#',padding);
	void * cad = malloc(superblock.tamanio_bloque);
	bzero(cad,superblock.tamanio_bloque);
	char * cadena  = string_duplicate(cadena_caracteres);
	memcpy(cad,(void*)cadena,string_length(cadena));

//	string_append(&cadena,pad);

//	TODO : meter la validacion de bitarray aca  ¿
	memcpy(_blocks.fs_bloques + (indice*superblock.tamanio_bloque)+offset, cad, string_length(cad));

	free(cad);//+
	free(cadena);//+
	return 1;
}

_archivo_bitacora* find_bitacora(t_list * bitacoras, char * clave){
	bool encontrar_bitacora(void * archivo){
		_archivo_bitacora* archivo_bit = (_archivo_bitacora*) archivo;
		return (strcmp(archivo_bit->clave, clave)==0);
	}
	_archivo_bitacora* retorno =  (_archivo_bitacora*)list_find(bitacoras,(void*)encontrar_bitacora);

	return retorno;
}


char * obtener_bitacora(int n_tripulante){

	char * n_trip =string_itoa(n_tripulante);
	char *clave = string_from_format("tripulante_%s", n_trip);
	char * path = string_new();
	string_append_with_format(&path,"%s.ims",clave);
	//string_append(&path,clave);
	//string_append(&path,".ims");

	char *aux = NULL;
	char *path_files = NULL;
	aux = string_duplicate(conf_PUNTO_MONTAJE);
	path_files = string_duplicate(conf_PATH_BITACORA);
	string_append_with_format(&aux, "%s%s", path_files,path);
	log_trace(logger,"Ruta de bitacora pedida: %s",aux);

	pthread_mutex_lock(&mutex_archivos_bitacora);
	_archivo_bitacora * archivo_bit = find_bitacora(archivos_bitacora,clave);
	pthread_mutex_unlock(&mutex_archivos_bitacora);

	pthread_mutex_lock(&(archivo_bit->mutex_file));
	t_config * config = config_create(aux);
	char ** bloques_ocupados = config_get_array_value(config,"BLOCKS"); // TODO: LIBERAR EL DOBLE PUNTERO CON FUNCION DE MARTIN
	char * cadena = string_new();//TODO : ESTE FREE SE HACE¿ COMO ¿
	//TODO: sacar este mutex ¿
	pthread_mutex_lock(&_blocks.mutex_blocks);
	for(int i = 0 ; i<longitud_array(bloques_ocupados); i++){
		int valor;
		valor =atoi(bloques_ocupados[i]);
		char * substring = string_substring_until(_blocks.fs_bloques + superblock.tamanio_bloque*(valor),superblock.tamanio_bloque);
		string_append(&cadena,substring);
		free(substring);
		//free(bloques_ocupados[i]);
	}
	pthread_mutex_unlock(&_blocks.mutex_blocks);
//	free(cadena);
	free(aux);
	free(path_files);
	config_destroy(config);
	uint i = 0;
	while(bloques_ocupados[i] != NULL){
		free(bloques_ocupados[i]);
		i++;
	}
	free(bloques_ocupados);
	pthread_mutex_unlock(&(archivo_bit->mutex_file));
	log_debug(logger,"obtener_bitacora(): Contenido de Bitacora pedida: %s",cadena);
	free(n_trip);
	free(clave);
	free(path);
	return cadena;
}

