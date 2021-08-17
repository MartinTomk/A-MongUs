#ifndef UTILS_H_
#define UTILS_H_

#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<commons/collections/queue.h>
#include<readline/readline.h>
#include <pthread.h>
#include <semaphore.h>
#include "tripulante.h"
#include "protocolo.h"

char* IP_MONGO;
char* IP_MIRAM;
int PUERTO_MONGO;
int PUERTO_MIRAM;
char* valor;

int GRADO_MULTIPROGRAMACION;
int QUANTUM;
char * ALGORITMO;


int CICLO_IO;
int CICLO_CPU;

t_list * conexiones;

t_log* logger;
int socketListener,socketMaximo;	 // PARA SERVER
fd_set socketClientes, socketClientesAuxiliares; // PARA SERVER

int socketServerMiRam;

int socketServerIMongoStore;

pthread_mutex_t comuni;

t_list * hilosParaConexiones;

t_queue* planificacion_cola_new;
t_queue* planificacion_cola_ready;
t_queue* planificacion_cola_exec;
t_queue* planificacion_cola_bloq;
t_queue* planificacion_cola_fin;

t_list * lista_exec;

t_queue* cola_ejecutados;

//HILOS
pthread_t hiloConsola;
pthread_t hiloCoordinador;
pthread_t hiloPlanificador;

//HILOS PARA COLAS
pthread_t hiloColaReady;
pthread_t hiloColaNew;

//SEMAFORO CONSUMIDOR PRODUCTOR
sem_t cola_ready;
sem_t cola_new;
sem_t cola_exec;
sem_t cola_bloq;
sem_t cola_fin;

//MUTEX PARA COLAS
pthread_mutex_t planificacion_mutex_new;
pthread_mutex_t planificacion_mutex_ready;
pthread_mutex_t planificacion_mutex_exec;
pthread_mutex_t planificacion_mutex_bloq;
pthread_mutex_t planificacion_mutex_fin;

pthread_mutex_t mutex_cola_ejecutados;

//Semaforos
pthread_mutex_t mutexHilos;

sem_t iniciar_planificacion;
sem_t detenerReaunudarEjecucion;
sem_t sabotajeEnCurso;

//sem de tipo cola, N = grado multiprogramacion
sem_t exec;
sem_t colaEjecutados;


sem_t activar_actualizaciones_mongo;
sem_t terminarPrograma;
sem_t expulsarEnCurso;
typedef struct _infoHilos{
	int socket;
	pthread_t hiloAtendedor;
} infoHilos;

typedef struct {
	int id;
	char estado;
	short int trabajando;
	short int fin_tareas;
	char* tarea; //Calculo que es necesario 50 50 SEGURIDAD
	pthread_t hilo_asociado;
	sem_t ready;
	sem_t new;
	sem_t exec;
}t_nodo_tripulante;



typedef struct
{
	int size;
	void* stream;
} t_buffer;

int ubic_sab_x;
int ubic_sab_y;

void iniciar_configuracion();

void iniciarEstructurasAdministrativasPlanificador();


int terminar_programa(t_log* logger,t_config* config);

void iniciar_logger();
int cerrar_conexiones_hilos(t_log* logger);
t_config* leer_config();
int eliminar_cola(t_queue* cola, pthread_mutex_t mutex_cola,t_log* logger);
int eliminar_list(t_list* lista,pthread_mutex_t mutex_lista,t_log* logger);

int terminar_discordiador(int signal);

void liberarCadenaDoble(char** cadena);
#endif /* UTILS_H_ */
