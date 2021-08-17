/*
 * estructurasFileSystem.h
 *
 *  Created on: 5 jul. 2021
 *      Author: utnso
 */

#ifndef ESTRUCTURASFILESYSTEM_H_
#define ESTRUCTURASFILESYSTEM_H_



typedef t_config str_metadata;

typedef struct {
	uint32_t tamanio;
	uint32_t cantidadBloques;
	char* bloques;
	char caracterLlenado;
	char* md5;
} str_metadata2;

typedef struct _archivo{
	char* clave;
	t_list * blocks;
	FILE * file;
	char * contenido;
	t_config * metadata;
	pthread_mutex_t mutex_file;
} _archivo;

typedef struct _archivo_bitacora{
	char* clave;
	FILE * file;
	t_config * metadata;
	pthread_mutex_t mutex_file;
} _archivo_bitacora;

_archivo * archivo_oxigeno;

_archivo * archivo_comida;
_archivo * archivo_basura;

pthread_mutex_t mutex_archivos_bitacora;
t_list * archivos_bitacora;


typedef struct blocks{
	FILE * file_blocks;
	void * fs_bloques; // TODO : SI HAY ALGUN PROBLEMA INESPERADO VOLVER A CHAR *
	void * original_blocks; // TODO : SI HAY ALGUN PROBLEMA INESPERADO VOLVER A CHAR *
	pthread_mutex_t mutex_blocks;
} blocks;

blocks _blocks;

typedef struct  {
	uint32_t tamanio_bloque;
	uint32_t cantidad_bloques;
	t_bitarray	* bitmap;
	char * bitmapstr;
	pthread_mutex_t mutex_superbloque;
	FILE * file_superblock;
} str_superblock;

str_superblock superblock;



typedef struct {
	char campo[100];
} __attribute__((packed)) t_registros_metadata;





#endif /* ESTRUCTURASFILESYSTEM_H_ */
