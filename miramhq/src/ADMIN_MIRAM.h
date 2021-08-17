
#ifndef ADMIN_MIRAM_H_
#define ADMIN_MIRAM_H_
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdint.h>
#include<sys/mman.h>
#include<fcntl.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<commons/temporal.h>
#include<readline/readline.h>
#include<commons/collections/queue.h>
#include<commons/bitarray.h>
#include<readline/readline.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include "utils.h"
#include "socket.h"
#include "estructuras.h"

#define DATO_PCB 0
#define DATO_TCB 1
#define DATO_TAREAS 2
#define VACIO 3
#define FF 0
#define BF 1

//Manejo de errores
#define OK 0
//#define PATOTA_CREADA 1 YA ESTAN DEFINIDOS
//#define TRIPULANTE_CREADO 2
#define ERROR_MEMORIA_LLENA -1
#define ERROR_CREACION_MEMORIA -2
#define ERROR -3
#define SWAP_LLENO -4
#define SWAP_OK 5;

//Segmentacion
typedef struct {
	uint32_t inicio;
	uint32_t tipoDato; //Que dato hay?
	uint32_t id; //A que proceso corresponde?
	uint32_t fin;
}segmento_t;
typedef struct {

	uint32_t idPatota;
	t_list* listaAsignados;
	uint32_t tamanioTareas;
	uint32_t ocupado;
}tabla_t;
typedef struct {
	uint32_t Nframe;
	uint32_t bytesOcupado;
	bool valida;
	uint32_t NframeVirtual;
	bool modificada;
	bool uso;
	tabla_t* tabla;
	uint32_t Npagina;
}pagina_t;

//Paginacion
typedef struct {
	uint32_t numeroFrame;
	bool estado; //true = Libre
	pagina_t* pagina; //PARA CLOCK
}frame_t;



typedef struct {
	uint32_t id;
	uint32_t tareas;
}pcb_t;

typedef struct {
	uint32_t id;
	char estado;
	uint32_t x;
	uint32_t y;
	uint32_t prox_tarea;
	uint32_t pcb;
}tcb_t;

t_list* listaSegmentos;
t_list* listaTablaSegmentos;
t_list* tablasPatotaPaginacion;
t_list* framesMemoria;
pthread_mutex_t accesoMemoria;
pthread_mutex_t accesoListaSegmentos;
pthread_mutex_t accesoListaTablas;

//CONFIG -----------------
uint32_t tamanioPagina;
uint32_t tamanioMemoria;
bool paginacion;
bool algoritmoReemplazo;
bool algoritmo;
//-----------------------
void* mem_ppal;
void* memoria_virt;
t_bitarray* swapFrames;
FILE* swapFile;
int swap_fd;
t_list* paginasUsadas;
frame_t* punteroClock;
//PPAL
int admin_memoria(void);
//FUNCIONES
int liberar_bytes(tabla_t* tabla,uint32_t direccionLogica,uint32_t tamanio);
int eliminar_tripulante(tabla_t* tabla,uint32_t direccionLogica);
//Criterios para listas de segmentos
bool condicionSegmentoLibrePcb(void* segmento);
bool ordenar_segun_inicio(void* primero,void* segundo);
bool condicionSegmentoLibreTcb(void* segmento);
void* condicionSegmentoLibreTcbBF(void* segmento,void* otroSegmento);
void* condicionSegmentoLibrePcbBF(void* segmento,void* otroSegmento);

//Creadores de segmentos segun tipo de dato
segmento_t* buscar_segmento(pcb_t pcb);
segmento_t* buscar_segmentoTcb(tcb_t tcb,uint32_t patotaId);
segmento_t* buscar_segmentoTareas(pcb_t pcb,uint32_t tareas);
t_list* buscarTablaPatota(uint32_t id);
//ELIMINAR Y RECIBIR TAREAS (Creacion y borrar segmentos)

void crear_segmento(uint32_t inicio,uint32_t fin);
int unificar_sg_libres(void);
void liberar_segmento(segmento_t* sg);
int eliminar_patota(tabla_t* tabla);
int compactar_memoria(void);

//PARA ITEREAR EN LAS LISTAS Y MOSTRAR COSAS
void mostrarMemoriaCompleta(void* segmento);
void mostrarEstadoMemoria(void* segmento);

//Funciones compactacion
int desplazar_segmento(segmento_t* sg,uint32_t offset);

int memoria_libre(void);


//PAGINACION!
void* getDato(uint32_t id_patota,uint32_t tamanio,uint32_t direccionLogica);
uint32_t guardarDato(tabla_t* tabla,void* dato,uint32_t tamanio,uint32_t direccionLogica);
uint32_t actualizarDato(tabla_t* tabla,void* dato,uint32_t tamanio,uint32_t direccionLogica);
uint32_t calcular_frames(uint32_t tamanioTotal);
void mostrarFrames(void* frame);
bool condicionFrameLibre(void* valor);
int buscar_frames(uint32_t id,uint32_t framesNecesarios,tabla_t* tablaPatota);
int crear_memoria_(void);
int crear_tripulante_(tcb_t tcb,uint32_t idpatota,tabla_t* tablaPatota);
int crear_patota_(pcb_t pcb,char* tareas,uint32_t cantidad_tripulantes,tabla_t* tabla);
uint32_t reconocerTamanioInstruccion(uint32_t direccionLogica,tabla_t* tabla);
uint32_t reconocerTamanioInstruccion2(uint32_t direccionLogica,tabla_t* tabla); //MODIFICADO PARA PAG
uint32_t reconocerTamanioInstruccion3(uint32_t direccionLogica,tabla_t* tabla);
void* getInstruccion(uint32_t id_patota,uint32_t tamanio,uint32_t direccionLogica);
tabla_t* buscarTablaId(uint32_t id);


////AUXILIARES DE SWAP
int calcularFramesLibres(void);
int frameLibreSwap(void);
int actualizarPagina(pagina_t* paginaBuscada);
uint32_t paginaTareas(int tamanioTarea);
//CONFIG SWAP
int inicializarAreaSwap(void); //ANDANDO LRU

//ESCOJE PAGINA SEGUN ALGORITMO
pagina_t* paginaSegun(char* algoritmoRemplazo);
//REALIZAN EL SWAP
int realizarSwap(pagina_t* paginaSwap);
int traerPaginaMemoria(pagina_t* pagina,uint32_t offsetMemoria);
int llevarPaginaASwap(pagina_t* paginaASwap,uint32_t* frameLiberado);//ANDANDO LRU 1/2
int llevarNframesSwap(uint32_t n);
///LIMPIANDO MEMORIA
int eliminarTabla(tabla_t* tabla,char* esquema);
int eliminarListaTablas(void);
int liberarBloquesMemoria(char* esquema);
int limpiarEstructurasAlgoritmo(char* algoritmo);
void limpiarSwap(void);
void liberarMemoriaHilos(void);
void manejarSignal(int signal);
void dumpMemoria(int signal);
void terminar_memoria(int signal);
#endif /* ADMIN_MIRAM_H_ */
