#include "core.h"



t_bitarray * crear_bit_array_limpio(uint32_t cantBloques){

	int tamanioBitarray=cantBloques/8;
	if(cantBloques % 8 != 0){
	  tamanioBitarray++;
	 }

	char* bits=malloc(tamanioBitarray);

	t_bitarray * bitarray = bitarray_create_with_mode(bits,tamanioBitarray,MSB_FIRST);

	int cont=0;
	for(; cont < tamanioBitarray*8; cont++){
		bitarray_clean_bit(bitarray, cont);
	}

	return bitarray;
}

t_bitarray * crear_bit_array(uint32_t cantBloques){

	int tamanioBitarray=cantBloques/8;
	if(cantBloques % 8 != 0){
	  tamanioBitarray++;
	 }

	char* bits=malloc(tamanioBitarray);

	t_bitarray * bitarray = bitarray_create_with_mode(bits,tamanioBitarray,MSB_FIRST);


	return bitarray;
}


void exit_failure() {
	perror("Error: ");
	exit(EXIT_FAILURE);
}
void iniciar_blocks(){

	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

	char *aux = NULL;
	char *path_files = NULL;
	aux = string_duplicate(conf_PUNTO_MONTAJE);
	string_append_with_format(&aux, "%s","blocks.ims");

	if (access(aux, R_OK | W_OK) != 0) {
		_blocks.file_blocks = open(aux, O_RDWR | O_CREAT | O_TRUNC, mode);
		ftruncate(_blocks.file_blocks,1);
		close(_blocks.file_blocks);
		log_warning(logger, "iniciar_blocks(): No se puede acceder al directorio %s", aux);
	}

	if (_blocks.file_blocks == (-1)) {
		perror("open");
		exit_failure();
	}


	struct stat info;
	bzero(&info, sizeof(struct stat));
	FILE *fptr;
	if (stat(aux, &info) != 0) {
		log_warning(logger, "iniciar_blocks(): No se puede hacer stat en el bitmap file");
		exit_failure();
	}

	//TODO: VER SI MODIFICAR EL 50
	if (info.st_size > 50) {
		_blocks.file_blocks = open(aux, O_RDWR | O_CREAT , mode);
		log_info(logger,"iniciar_blocks(): Usando Blocks de FILE SYSTEM EXISTENTE");
		_blocks.fs_bloques = malloc(superblock.cantidad_bloques*superblock.tamanio_bloque);
		bzero(_blocks.fs_bloques,superblock.cantidad_bloques * superblock.tamanio_bloque);
		_blocks.original_blocks = mmap ( NULL, superblock.tamanio_bloque * superblock.cantidad_bloques, PROT_READ | PROT_WRITE, MAP_SHARED , _blocks.file_blocks, 0 );
		memcpy(_blocks.fs_bloques,_blocks.original_blocks,superblock.tamanio_bloque * superblock.cantidad_bloques);
	}else
	{
		_blocks.file_blocks = open(aux, O_RDWR | O_CREAT | O_TRUNC, mode);
		log_info(logger,"iniciar_blocks(): Creando Blocks");
		ftruncate(_blocks.file_blocks,superblock.cantidad_bloques*superblock.tamanio_bloque);

		_blocks.fs_bloques = malloc(superblock.cantidad_bloques*superblock.tamanio_bloque);

		bzero(_blocks.fs_bloques,superblock.cantidad_bloques * superblock.tamanio_bloque);

		_blocks.original_blocks = mmap ( NULL, superblock.tamanio_bloque * superblock.cantidad_bloques, PROT_READ | PROT_WRITE, MAP_SHARED , _blocks.file_blocks, 0 );
	}
	free(aux);
	free(path_files);


	pthread_mutex_init(&_blocks.mutex_blocks,NULL);

}

int write_blocks(char * cadena_caracteres,int indice) {


	void * cad = malloc(superblock.tamanio_bloque);
	bzero(cad,superblock.tamanio_bloque);
	char * cadena  = string_duplicate(cadena_caracteres);
	memcpy(cad,(void*)cadena,string_length(cadena));

	memcpy(_blocks.fs_bloques + (indice*superblock.tamanio_bloque), cad, superblock.tamanio_bloque);

	free(cad);
	free(cadena);

	return 1;
}

int write_blocks_with_offset(char * cadena_caracteres,int indice,int offset) {

	char * cadena  = string_duplicate(cadena_caracteres);

//	TODO : meter la validacion de bitarray aca  ¿
	memcpy(_blocks.fs_bloques + (indice*superblock.tamanio_bloque)+offset, (void*)cadena, string_length(cadena));

	free(cadena);
	return 1;
}

int clean_block(int indice) {

	void * clean2 = malloc(superblock.tamanio_bloque);
	bzero(clean2,superblock.tamanio_bloque);
	memcpy(_blocks.fs_bloques + (indice*superblock.tamanio_bloque), clean2, superblock.tamanio_bloque);
	free(clean2);

	return 1;
}

//TODO: refactor obtener ->return string
void * obtener_contenido_bloque(int indice) {
	void * bloque = malloc(superblock.tamanio_bloque);

//	TODO: PORQUE TAMANIO -1 ¿¿
	memcpy(bloque,_blocks.fs_bloques + (indice*superblock.tamanio_bloque), superblock.tamanio_bloque);

	return bloque;
}

void * obtener_contenido_bloque_with_mutex(int indice) {
	void * bloque = malloc(superblock.tamanio_bloque);
	//TODO: DEL ORIGINAL O DE LA COPIA
	pthread_mutex_lock(&_blocks.mutex_blocks);
	memcpy(bloque,_blocks.fs_bloques + (indice*superblock.tamanio_bloque), superblock.tamanio_bloque);
	pthread_mutex_unlock(&_blocks.mutex_blocks);
	return bloque;
}



//--------------------------SYNC----------------------------------------
bool menorAMayor(int* primero, int* segundo){
	return *primero < *segundo;
}

void ordenar(t_list * bloques){
	bool ordenar_blocks(int* primero,int* segundo){
		if(*primero == *segundo){
			return *primero < *segundo;
		}else{
			menorAMayor(primero,segundo);
		}
	}

	list_sort(bloques, (void*) ordenar_blocks);
}



void mostrar_blocks_ims(t_list * bloques,char * blocks,char * source){

	void imprimir_bloque(int *item) {
		char * bloque = string_new();
		bloque = string_substring(blocks, ((*item)*superblock.tamanio_bloque), superblock.tamanio_bloque);
		log_debug(logger,"SYNC: %s: NUMERO BLOQUE : %d -> %s ",source,*item,bloque);
		free(bloque);
	}

	list_iterate(bloques,(void*) imprimir_bloque);

}


void *sincronizar_blocks(){
	while(1){

		sem_wait(&detener_sincro);
		sem_post(&detener_sincro);
		log_info(logger,"SINCRONIZANDO DISCO");
		pthread_mutex_lock(&_blocks.mutex_blocks);
		log_trace(logger,"SINCRO - MUTEX_BLOCKS - BLOCKED");
		pthread_mutex_lock(&superblock.mutex_superbloque);
		log_trace(logger,"SINCRO  bitmap - BLOCKED");

		memcpy(_blocks.original_blocks, (_blocks.fs_bloques), (superblock.cantidad_bloques*superblock.tamanio_bloque));
		msync(_blocks.original_blocks, (superblock.cantidad_bloques*superblock.tamanio_bloque), MS_SYNC);

		memcpy(superblock.bitmapstr + 2*sizeof(uint32_t), (superblock.bitmap->bitarray), (superblock.cantidad_bloques/8));
		msync(superblock.bitmapstr, 2*sizeof(uint32_t)+ (superblock.cantidad_bloques/8), MS_SYNC);

		pthread_mutex_unlock(&superblock.mutex_superbloque);
		log_trace(logger,"SINCRO  bitmap - unBLOCKED");
		pthread_mutex_unlock(&_blocks.mutex_blocks);
		log_trace(logger,"SINCRO  - MUTEX_BLOCKS - UNBLOCKED");
		if(exitSincro==-1){
			log_info(logger,"Termino la sincro DESDE SINCRO");
			pthread_cancel(pthread_self());
			break;
		}

		sleep(conf_TIEMPO_SINCRONIZACION);


	}
}

void hilo_sincronizar_blocks(){
	pthread_create( &thread_sincronizador , NULL,(void*) sincronizar_blocks,NULL);




}


//---------------------------METADATA------------------------------------------------

int calcular_cantidad_bloques_requeridos(char* cadenaAGuardar){
	int cantidadBloques = string_length(cadenaAGuardar)/superblock.tamanio_bloque;

	if(string_length(cadenaAGuardar) % superblock.tamanio_bloque > 0 ){
		cantidadBloques++;
	}

	return cantidadBloques;
}


//--------------------------SUPER BLOQUE--------------------------------------------
void iniciar_super_block(){
	//cargar desde config la struct de super_bloque
	log_info(logger,"iniciar_super_blocks(): Iniciando superblock");
	log_info(logger,"Inicio superbloque: %s", conf_BYTES_BLOQUE);
	log_info(logger,"Tamanio de Bloque: %s", conf_BYTES_BLOQUE);
	log_info(logger,"Cantidad de Bloques: %s", conf_CANTIDAD_BLOQUES);

	superblock.tamanio_bloque = (uint32_t)atoi(conf_BYTES_BLOQUE);

	superblock.cantidad_bloques = atoi(conf_CANTIDAD_BLOQUES);

	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

	int tamanioBitarray=superblock.cantidad_bloques/8;
	if(superblock.cantidad_bloques % 8 != 0){
	  tamanioBitarray++;
	}
	int tamanioFs = tamanioBitarray+(sizeof(uint32_t))*2;

//	if( access( "superblock.ims", F_OK ) == 0 ) {
//		log_debug(logger,"iniciar_super_blocks(): Iniciando con superblock existente");
//		superblock.file_superblock = open("superblock.ims", O_RDWR | O_CREAT , mode);
//	}else{
	log_debug(logger,"iniciar_super_blocks(): Creando archivo de superblock");

	char *aux = NULL;
	char *path_files = NULL;
	aux = string_duplicate(conf_PUNTO_MONTAJE);
	string_append_with_format(&aux, "%s","superblock.ims");



	if (access(aux, R_OK | W_OK) != 0) {
		superblock.file_superblock = open(aux, O_RDWR | O_CREAT , mode);
		ftruncate(superblock.file_superblock,1);

//		superblock.bitmapstr = malloc(superblock.cantidad_bloques * superblock.tamanio_bloque); // TODO : CAMBIAR TAMAÑO
//		bzero(superblock.bitmapstr,tamanioFs);


		close(superblock.file_superblock);
		log_warning(logger, "iniciar_super_block(): No se puede acceder al directorio %s", aux);
	}


	struct stat info;
	bzero(&info, sizeof(struct stat));
	FILE *fptr;
	if (stat(aux, &info) != 0) {
		log_warning(logger, "iniciar_super_block(): No se puede hacer stat en el bitmap file");
		exit_failure();
	}

	//TODO : Buscar otra validacion para confirmar que existe un fs.
	if (info.st_size == 1){
		superblock.file_superblock = open(aux, O_RDWR | O_CREAT , mode);
		superblock.bitmap = crear_bit_array_limpio(superblock.cantidad_bloques);
		superblock.bitmapstr = mmap ( NULL, tamanioFs, PROT_READ | PROT_WRITE, MAP_SHARED , superblock.file_superblock, 0 );

		ftruncate(superblock.file_superblock,tamanioFs);
		log_debug(logger,"iniciar_super_block(): Creando metadata de FS...");


		memcpy(superblock.bitmapstr, &(superblock.cantidad_bloques), sizeof(uint32_t));
		memcpy(superblock.bitmapstr+sizeof(uint32_t), &(superblock.tamanio_bloque), sizeof(uint32_t));


		memcpy(superblock.bitmapstr+(sizeof(uint32_t))*2, (superblock.bitmap->bitarray),tamanioBitarray);
		msync(superblock.bitmapstr+ (sizeof(uint32_t))*2, tamanioFs, MS_SYNC);

		log_debug(logger,"iniciar_super_block(): Metadata creada...");
	}else{
		superblock.bitmap = crear_bit_array_limpio(superblock.cantidad_bloques);
		superblock.file_superblock = open(aux, O_RDWR | O_CREAT , mode);
		superblock.bitmapstr = mmap ( NULL, tamanioFs, PROT_READ | PROT_WRITE, MAP_SHARED , superblock.file_superblock, 0 );

		memcpy(superblock.bitmap->bitarray,superblock.bitmapstr+(sizeof(uint32_t))*2, tamanioBitarray);

		log_debug(logger,"iniciar_super_block(): Usando FS existente...");
	}


	pthread_mutex_init(&(superblock.mutex_superbloque),NULL);
	calcular_bloques_libres_ONLY();
	free(aux);
	free(path_files);
	calcular_bloques_libres_ONLY();

}

int calcular_bloques_libres_ONLY(){
	pthread_mutex_lock(&superblock.mutex_superbloque);
	int resultado = 0;
	int i;
	int cantidadDePosiciones = superblock.cantidad_bloques;

	for(i=0;i<cantidadDePosiciones;i++){

		if(!bitarray_test_bit(superblock.bitmap,i)){
			resultado++;
		}
	}
	log_info(logger,"calcular_bloques_libres: %d",resultado);
	pthread_mutex_unlock(&superblock.mutex_superbloque);
	return resultado;
}

int calcular_bloques_libres(){
	int resultado = 0;
	int i;
	int cantidadDePosiciones = superblock.cantidad_bloques;

	for(i=0;i<cantidadDePosiciones;i++){

		if(!bitarray_test_bit(superblock.bitmap,i)){
			resultado++;
		}
	}
	log_trace(logger,"calcular_bloques_libres: %d",resultado);
	return resultado;
}



int obtener_indice_para_guardar_en_bloque(char * valor){
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


void iniciar_archivo(char * name_file,_archivo **archivo,char * key_file,char * caracter_llenado){
	*archivo = (_archivo*)malloc(sizeof(_archivo));

	(*archivo)->clave = string_new();
	string_append(&((*archivo)->clave),key_file);

	(*archivo)->blocks = list_create();
	char *aux = NULL;
	char *path_files = NULL;
	aux = string_duplicate(conf_PUNTO_MONTAJE);
	path_files = string_duplicate(conf_PATH_FILES);
	string_append_with_format(&aux, "/%s%s", path_files,name_file);

	if( access( aux, F_OK ) == 0 ) {
		log_debug(logger,"iniciar_archivo(): Iniciando  existente: %s",name_file);
		(*archivo)->metadata = config_create(aux);
	}else
	{
		log_debug(logger,"Creando archivo de recursos");
		FILE * metadata = open(aux, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR);
		(*archivo)->metadata = config_create(aux);
		config_set_value((*archivo)->metadata,"CARACTER_LLENADO",caracter_llenado);

		config_set_value((*archivo)->metadata,"MD5","XXXX");
		config_set_value((*archivo)->metadata,"SIZE","0");
		config_set_value((*archivo)->metadata,"BLOCKS","[]");
		config_set_value((*archivo)->metadata,"BLOCK_COUNT","0");

		config_save((*archivo)->metadata);
	}


	pthread_mutex_init(&((*archivo)->mutex_file), NULL);
	free(aux);
	free(path_files);

}

void actualizar_metadata(_archivo * archivo,int indice_bloque,char * valorAux){
	char * md5 = string_new();
	md5 = config_get_string_value (archivo->metadata, "MD5");

	string_length(valorAux);
	int bytes = string_length(valorAux);

	int size = config_get_int_value(archivo->metadata,"SIZE");
	size+=bytes;

	char * bytesString = string_itoa(size);
	config_set_value(archivo->metadata,"SIZE",bytesString);


	char ** array;
	array = config_get_array_value(archivo->metadata,"BLOCKS");
	char * indice_bloque_string = string_itoa(indice_bloque);
	char ** nuevo  = agregar_en_array(array,indice_bloque_string); // FREE

	free(indice_bloque_string);
	char * longitud_nuevo_string = string_itoa(longitud_array(nuevo));
	config_set_value(archivo->metadata,"BLOCK_COUNT",longitud_nuevo_string); // FREE
	free(longitud_nuevo_string);

	char * cadena = array_to_string(nuevo);
	int i = 0;
	while(nuevo[i]!=NULL){
		free(nuevo[i]);
		i++;
	}
	config_set_value(archivo->metadata,"BLOCKS",cadena);
	config_save(archivo->metadata);
	free(nuevo);
	free(bytesString);
	free(cadena);
	
}

void actualizar_metadata_sin_crear_bloque(_archivo * archivo,char * valorAux){
	char * md5;
	md5 = config_get_string_value (archivo->metadata, "MD5");
	log_debug(logger,"MD5: %s",md5);

	int bytes = string_length(valorAux);
	int size = config_get_int_value(archivo->metadata,"SIZE");
	size+=bytes;
	char * bytesString =string_itoa(size);

	config_set_value(archivo->metadata,"SIZE",bytesString);

	config_save(archivo->metadata);
	free(bytesString);
}

void actualizar_metadata_borrado(_archivo * archivo,int cantidadABorrar){
	int size = config_get_int_value(archivo->metadata,"SIZE");

	size-=cantidadABorrar;
	char * sizeChar = string_itoa(size);
	config_set_value(archivo->metadata,"SIZE",sizeChar);
	free(sizeChar);
	config_save(archivo->metadata);
}

void actualizar_metadata_elimina_bloque(_archivo * archivo,int cantidadABorrar){

	char ** blocks = config_get_array_value(archivo->metadata,"BLOCKS");
	int block_count = config_get_int_value(archivo->metadata,"BLOCK_COUNT");
	int size = config_get_int_value(archivo->metadata,"SIZE");
	blocks[block_count-1]=NULL;

	char * bloques_str = array_to_string(blocks);


	config_set_value(archivo->metadata,"BLOCKS",bloques_str);
	config_set_value(archivo->metadata,"BLOCK_COUNT",string_itoa(block_count-1));
	config_set_value(archivo->metadata,"SIZE",string_itoa(size-cantidadABorrar));
	config_save(archivo->metadata);

	int i = 0;
	while(blocks[i]!=NULL){
		free(blocks[i]);
		i++;
	}
	free(blocks);
}

void bitarray_set_bit_monitor(int indice_bloque) {
	//TODO: set bit a funcion
	bitarray_set_bit(superblock.bitmap, indice_bloque);
}

void llenar_nuevo_bloque(char* cadenaAGuardar, _archivo* archivo) {
	//Si el ultiimo bloque esta completo -> creo los bloques nuevos que necesito para almacenar la cadena
	int posicionesStorageAOcupar = calcular_cantidad_bloques_requeridos(
			cadenaAGuardar);
	int offsetBytesAlmacenados = 0;
	int bloqueslibres = calcular_bloques_libres();
	for (int i = 0; i < posicionesStorageAOcupar; i++) {
		char* valorAux = string_substring(cadenaAGuardar,
				offsetBytesAlmacenados, superblock.tamanio_bloque);
		int indice_bloque = obtener_indice_para_guardar_en_bloque(valorAux);
		log_trace(logger, "llenar_nuevo_bloque(): File_Recurso: %s -> bloque asignado: %d", archivo->clave,
				indice_bloque);
		write_blocks(valorAux, indice_bloque);
		actualizar_metadata(archivo, indice_bloque, valorAux);
		bitarray_set_bit_monitor(indice_bloque);
		offsetBytesAlmacenados += superblock.tamanio_bloque;
		free(valorAux);
	}
}

uint32_t write_archivo(char* cadenaAGuardar,_archivo * archivo,uint32_t id_trip){
	int tamanioCadenaAGuardar = string_length(cadenaAGuardar);

	//TODO: si no hay lugar en el fs, no permitir que escriba..
	pthread_mutex_lock(&(archivo->mutex_file));

	uint32_t resultado;
	int bytesArchivo = config_get_int_value(archivo->metadata,"SIZE");

	log_trace(logger,"bytes archivo %s : %d",archivo->clave,bytesArchivo);

	pthread_mutex_lock(&_blocks.mutex_blocks);
	pthread_mutex_lock(&superblock.mutex_superbloque);
	//Chequeo si hay lugar en el ultimo bloque
	if(bytesArchivo%superblock.tamanio_bloque==0){

		//Si el ultiimo bloque esta completo -> creo los bloques nuevos que necesito para almacenar la cadena
		llenar_nuevo_bloque(cadenaAGuardar, archivo);

	}else{
		//bytesArchivo : restas sucesivas para quedarme con el espacio OCUPADO del ultimo bloque.
		while(bytesArchivo>superblock.tamanio_bloque) bytesArchivo-=superblock.tamanio_bloque;

		int espacioLibreUltimoBloque = superblock.tamanio_bloque-bytesArchivo;

		char ** blocks = config_get_array_value(archivo->metadata,"BLOCKS");

		int count_block = config_get_int_value(archivo->metadata,"BLOCK_COUNT");

		char * last_block = blocks[count_block-1];

		if(string_length(cadenaAGuardar)<=espacioLibreUltimoBloque){

			log_trace(logger,"indice de BLOQUE :%d con espacio para archivo: %s, ",atoi(last_block),archivo->clave);

			write_blocks_with_offset(cadenaAGuardar,atoi(last_block),bytesArchivo);

			//TODO: podria armar un actualizar metadata mas generico con varios if
			actualizar_metadata_sin_crear_bloque(archivo,cadenaAGuardar);

		}else{

			char * rellenoDeUltimoBloque  = string_substring_until(cadenaAGuardar,espacioLibreUltimoBloque);

			write_blocks_with_offset(rellenoDeUltimoBloque,atoi(last_block),bytesArchivo);

			actualizar_metadata_sin_crear_bloque(archivo,rellenoDeUltimoBloque);


			char * contenidoProximoBloque = string_substring_from(cadenaAGuardar,espacioLibreUltimoBloque);

			llenar_nuevo_bloque(contenidoProximoBloque, archivo);

			free(rellenoDeUltimoBloque);
			free(contenidoProximoBloque);

		}


		for(int i = 0 ; i<longitud_array(blocks); i++){

			free(blocks[i]);
		}

		free(blocks);//stringsplit
	}
	log_info(logger,"El tripulante %d genero %d recursos de %s",id_trip,tamanioCadenaAGuardar,archivo->clave);
	log_trace(logger,"write_archivo()->Recurso: %s - Copia blocks.ims: %s ",archivo->clave,_blocks.fs_bloques);
	pthread_mutex_unlock(&(superblock.mutex_superbloque));
	pthread_mutex_unlock(&(_blocks.mutex_blocks));
	pthread_mutex_unlock(&(archivo->mutex_file));
	free(cadenaAGuardar);
	return 1;
}

void remover_bloque(int indice,_archivo * archivo, int cantidadAConsumir){

	bitarray_clean_bit(superblock.bitmap,indice);

	clean_block(indice);

	actualizar_metadata_elimina_bloque(archivo,cantidadAConsumir);

}

void actualizar_metadata_elimina_bloque_para_descartar(_archivo * archivo,int cantidadABorrar){

	char ** blocks = config_get_array_value(archivo->metadata,"BLOCKS");
	int block_count = config_get_int_value(archivo->metadata,"BLOCK_COUNT");
	int size = config_get_int_value(archivo->metadata,"SIZE");
	blocks[block_count-1]=NULL;

	char * bloques_str = array_to_string(blocks);

	config_set_value(archivo->metadata,"BLOCKS",bloques_str);
	char * numero = string_itoa(block_count-1);
	config_set_value(archivo->metadata,"BLOCK_COUNT",numero);
	free(numero);
	config_save(archivo->metadata);
	int i = 0;
	while(blocks[i]!=NULL){
		free(blocks[i]);
		i++;
	}
	free(blocks);
	free(bloques_str);
}


void remover_bloque_descartar(int indice,_archivo * archivo, int cantidadAConsumir){

	bitarray_clean_bit(superblock.bitmap,indice);

	clean_block(indice);

	actualizar_metadata_elimina_bloque_para_descartar(archivo,cantidadAConsumir);

}


void consumir_arch(_archivo * archivo,int cantidadAConsumir, uint32_t id_trip){
	int cantidadOriginalAConsumir = cantidadAConsumir;
	pthread_mutex_lock(&(archivo->mutex_file));

	char ** bloques = config_get_array_value(archivo->metadata,"BLOCKS");
	int cantidad_bloques = config_get_int_value(archivo->metadata,"BLOCK_COUNT");
	int size = config_get_int_value(archivo->metadata,"SIZE");
	int excedenteAConsumir = cantidadOriginalAConsumir - size;
	cantidad_bloques--;
	char * bloque = bloques[cantidad_bloques];

	int sizeUltimoBloque = size;
	while(sizeUltimoBloque>superblock.tamanio_bloque) sizeUltimoBloque-=superblock.tamanio_bloque;
	log_trace(logger,"consumir_arch(): sizeUltimoBloque: %d",sizeUltimoBloque);
	log_trace(logger,"consumir_arch(): Size: %d",size);
	void * contenidoBloque = NULL;
	log_info(logger,"El tripulante %d quiere consumir %d recursos de %s. ",id_trip,cantidadAConsumir,archivo->clave);
	if(size>0){

		int indice = atoi(bloque);

		pthread_mutex_lock(&_blocks.mutex_blocks);
		log_trace(logger,"consumir_arch() - MUTEX_BLOCKS - BLOCKED");
		pthread_mutex_lock(&superblock.mutex_superbloque);
		log_trace(logger,"consumir_arch() - MUTEX_SUPERBLOQUE - BLOCKED");
		contenidoBloque = obtener_contenido_bloque(indice);

		log_debug(logger,"Size: %d",sizeUltimoBloque);
		log_debug(logger,"Size: %d",size);
		while((cantidadAConsumir>0 && cantidad_bloques >= 0)){

			if(cantidadAConsumir>sizeUltimoBloque){

				cantidadAConsumir-=sizeUltimoBloque;
				remover_bloque(indice,archivo,sizeUltimoBloque);
				cantidad_bloques--;
				if(cantidad_bloques < 0 ){
					log_debug(logger,"El tripulante %d quiso consumir %d %s mas de los que habia disponibles",id_trip,excedenteAConsumir,archivo->clave);
					log_info(logger,"Habia %d %s. El tripulante %d consumio la totalidad de los recursos de %s disponibles.",size,archivo->clave,id_trip,archivo->clave);
					break;
				}
				bloque = bloques[cantidad_bloques];
				indice = atoi(bloque);
				free(contenidoBloque);
				contenidoBloque = obtener_contenido_bloque(indice);

				sizeUltimoBloque = superblock.tamanio_bloque;


			}
			else if(cantidadAConsumir==sizeUltimoBloque){
				cantidadAConsumir-=sizeUltimoBloque;
				remover_bloque(indice,archivo,sizeUltimoBloque);
			}
			else if (cantidadAConsumir<sizeUltimoBloque){
				actualizar_metadata_borrado(archivo,cantidadAConsumir);
				char* substring = string_substring_until(contenidoBloque,sizeUltimoBloque-cantidadAConsumir);
				free(contenidoBloque);
				contenidoBloque = string_new();
				string_append(&contenidoBloque,substring);
				free(substring);
				write_blocks(contenidoBloque,indice);
				cantidadAConsumir=0;
			}

		}
		if(excedenteAConsumir<=0){
			log_info(logger,"El tripulante %d consumio %d recursos de %s", id_trip,cantidadOriginalAConsumir,archivo->clave);
		}

		log_trace(logger,"consumir_arch()->Recurso: %s - Copia blocks.ims: %s ",archivo->clave,_blocks.fs_bloques);
		pthread_mutex_unlock(&(superblock.mutex_superbloque));
		pthread_mutex_unlock(&(_blocks.mutex_blocks));
		log_trace(logger,"CONSUMIR - MUTEX_BLOCKS - BLOCKED");
	}
	else{
		log_info(logger,"El tripulante %d quiso consumir: %s pero no habia recursos", id_trip,archivo->clave);
		log_debug(logger,"consumir_arch(): NO HAY RECURSOS PARA CONSUMIR");
	}
	uint32_t i = 0;
	while(bloques[i] != NULL){
		free(bloques[i]);
		i++;
	}
	free(bloques);
	free(contenidoBloque);
	pthread_mutex_unlock(&(archivo->mutex_file));

}

void descartar_basura(_archivo * archivo,uint32_t id_trip){
	pthread_mutex_lock(&(archivo->mutex_file));

	char ** bloques = config_get_array_value(archivo->metadata,"BLOCKS");
	int cantidad_bloques = config_get_int_value(archivo->metadata,"BLOCK_COUNT");
	int size = config_get_int_value(archivo->metadata,"SIZE");
	cantidad_bloques--;
	char * bloque = bloques[cantidad_bloques];
	log_trace(logger,"Cantidad de basura: %d",size);
	int sizeUltimoBloque = size;
	while(sizeUltimoBloque>superblock.tamanio_bloque) sizeUltimoBloque-=superblock.tamanio_bloque;
	log_trace(logger,"Cantidad de basura del ultimo bloque: %d",sizeUltimoBloque);
	void * contenidoBloque = NULL;
	if(size!=0){
		int indice = atoi(bloque);
		pthread_mutex_lock(&_blocks.mutex_blocks);
		log_trace(logger,"consumir_arch() - MUTEX_BLOCKS - BLOCKED");
		pthread_mutex_lock(&superblock.mutex_superbloque);
		log_trace(logger,"consumir_arch() - MUTEX_SUPERBLOQUE - BLOCKED");
		contenidoBloque = obtener_contenido_bloque(indice);

		while((cantidad_bloques>=0)){

				bloque = bloques[cantidad_bloques];
				indice = atoi(bloque);
				remover_bloque_descartar(indice,archivo,sizeUltimoBloque);
				cantidad_bloques--;

		}
		config_set_value(archivo->metadata,"SIZE","0");
		config_save(archivo->metadata);
		log_trace(logger,"descartar_basura()->Recurso: %s - Copia blocks.ims: %s ",archivo->clave,_blocks.fs_bloques);
		pthread_mutex_unlock(&(superblock.mutex_superbloque));
		pthread_mutex_unlock(&(_blocks.mutex_blocks));
		log_trace(logger,"DESCARTAR - MUTEX_BLOCKS - BLOCKED");
		log_info(logger,"El tripulante %d descarto la basura!", id_trip);
	}else{
		log_info(logger,"El tripulante %d quiso descartar la basura, pero estaba todo limpio!",id_trip);
	}
	int i = 0;
	while(bloques[i] != NULL){
		free(bloques[i]);
		i++;
	}
	free(bloques);
	free(contenidoBloque);
	pthread_mutex_unlock(&(archivo->mutex_file));
}


int obtener_tamanio_archivo_de_recurso(_archivo * archivo,char * name_file,char * caracterP){
	pthread_mutex_lock(&(archivo->mutex_file));
	char *aux = NULL;
	char *path_files = NULL;
	aux = string_duplicate(conf_PUNTO_MONTAJE);
	path_files = string_duplicate(conf_PATH_FILES);
	string_append_with_format(&aux, "%s%s", path_files,name_file);
	log_trace(logger,"aux : %s",aux);
	config_destroy(archivo->metadata);
	archivo->metadata = config_create(aux);
	char ** bloques_ocupados = config_get_array_value(archivo->metadata,"BLOCKS");
	int size_archivo = 0;
	for(int i = 0 ; i<longitud_array(bloques_ocupados); i++){
		int valor;
		valor =atoi(bloques_ocupados[i]);
		for(int j = 0; j<superblock.tamanio_bloque; j++){
			char * caracter = string_new();
			memcpy(caracter,_blocks.original_blocks + (valor*superblock.tamanio_bloque)+j, 1);
			memcpy(caracter+1,"\0",1);
			log_trace(logger,"caracter %s",caracter);
			if(string_equals_ignore_case(caracter,caracterP)){
				size_archivo++;
			}
			free(caracter);
		}


	}
	uint32_t i = 0; //FREE
	while(bloques_ocupados[i] != NULL){
		free(bloques_ocupados[i]);
		i++;
	}
	free(bloques_ocupados);
	log_debug(logger,"obtener_tamanio_archivo_de_recurso(): Tamanio real del archivo de recursos %d",size_archivo);

	int metadata_size = config_get_int_value(archivo->metadata,"SIZE");

	if(metadata_size!=size_archivo){
		log_debug(logger,"obtener_tamanio_archivo_de_recurso(): El tamanio del archivo fue saboteado, por lo que tuvo que ser restaurado");
		log_info(logger,"%s fue saboteado, se restauro el tamaño de %d -> %d ",name_file,metadata_size, size_archivo);
		config_set_value(archivo->metadata,"SIZE",string_itoa(size_archivo));
		config_save(archivo->metadata);
	}



	pthread_mutex_unlock(&(archivo->mutex_file));
	free(aux);
	free(path_files);
	return size_archivo;
}

void obtener_todos_los_bloques_de_recursos_metedata(t_list* lista_bloques) {
	bloques_ocupados_file(archivo_oxigeno, lista_bloques);
	bloques_ocupados_file(archivo_comida, lista_bloques);
	bloques_ocupados_file(archivo_basura, lista_bloques);

}

void obtener_todos_los_bloques_de_recursos_y_bitacora(){
	t_list * lista_bloques = list_create();


	obtener_todos_los_bloques_desde_metedata(lista_bloques);
	mostrar_bloques_ocupados(lista_bloques);


}

void mostrar_bloques_ocupados(t_list * bloques_ocupados){

	int cantidadDePosiciones = superblock.cantidad_bloques;
	pthread_mutex_lock(&superblock.mutex_superbloque);
	pthread_mutex_lock(&_blocks.mutex_blocks);
	char * string_bloques_ocupados = string_new();
	int ocupados = 0;
	//TODO EL -2 QUE ONDA
	for(int i=0; i<cantidadDePosiciones-2;i++){
		if(encontrar_bloque_para_iniciar_fs(bloques_ocupados,i)){
			ocupados++;
			string_append_with_format(&string_bloques_ocupados,"%s,",string_itoa(i));
		}
	}
	log_debug(logger," Hay %d bloques ocupados son:",ocupados);
	log_debug(logger,"%s",string_bloques_ocupados);
	free(string_bloques_ocupados);
	pthread_mutex_unlock(&_blocks.mutex_blocks);
	pthread_mutex_unlock(&superblock.mutex_superbloque);

}

void igualar_bitmap_contra_bloques_al_iniciar_fs(t_list * bloques_ocupados){
	calcular_bloques_libres_ONLY();
	int cantidadDePosiciones = superblock.cantidad_bloques;
	pthread_mutex_lock(&superblock.mutex_superbloque);
	pthread_mutex_lock(&_blocks.mutex_blocks);
	char * string_bloques_ocupados = string_new();
	int ocupados = 0;
	//TODO EL -2 QUE ONDA
	for(int i=0; i<(superblock.cantidad_bloques/8);i++){
		if(encontrar_bloque_para_iniciar_fs(bloques_ocupados,i)){
			bitarray_clean_bit(superblock.bitmap,i);
			bzero(_blocks.fs_bloques+i*superblock.tamanio_bloque,superblock.tamanio_bloque);
		}
	}

	memcpy(superblock.bitmapstr + 2*sizeof(uint32_t), (superblock.bitmap->bitarray), (superblock.cantidad_bloques/8));
	msync(superblock.bitmapstr, 2*sizeof(uint32_t)+ (superblock.cantidad_bloques/8), MS_SYNC);

	log_debug(logger,"Se liberaron los bloques de bitacora -> Hay %d bloques de recursos ocupados son:",ocupados);
	log_debug(logger,"%s",string_bloques_ocupados);
	free(string_bloques_ocupados);
	pthread_mutex_unlock(&_blocks.mutex_blocks);
	pthread_mutex_unlock(&superblock.mutex_superbloque);
	calcular_bloques_libres_ONLY();

}

bool encontrar_bloque_para_iniciar_fs(t_list * lista_bloques, int i){
	bool condicion_bloque(void *bloque){
		int * bloque_n = (int*)bloque;
		return  *bloque_n == i;
	}
	return list_any_satisfy(lista_bloques,condicion_bloque);
}



void liberar_bloques_bitacora_al_iniciar_fs(){
	t_list * lista_bloques = list_create();


	pthread_mutex_lock(&mutex_archivos_bitacora);
	for (int i = 0; i < list_size(archivos_bitacora); i++) {
		_archivo_bitacora* archivo = (_archivo_bitacora*) list_get(
				archivos_bitacora, i);
		bloques_file_bitacora(archivo, lista_bloques);
	}
	pthread_mutex_unlock(&mutex_archivos_bitacora);


	igualar_bitmap_contra_bloques_al_iniciar_fs(lista_bloques);

	list_destroy_and_destroy_elements(lista_bloques,free);
}

void calcular_md5(char * cadena){

    char * command = string_from_format("echo -n %s | md5sum | cut -d' ' -f1 > md5.txt",cadena);
    log_info(logger,"%s",command);
    system(command);

    char buff[1024];
    FILE *f = fopen("md5.txt", "r");
    fgets(buff, 1024, f);
    log_info(logger,"%s",buff);
    fclose(f);

}

