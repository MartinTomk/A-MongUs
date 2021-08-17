#include <nivel-gui/nivel-gui.h>
#include <nivel-gui/tad_nivel.h>
#include <stdlib.h>
#include <curses.h>
#include <commons/collections/list.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include "utils.h"

int err;
NIVEL* nivel;
int cols, rows;
pthread_t mapaHilo;
sem_t actualizarMapa;
int iniciar_mapa(void);
void* mapa(void* arg);
int crear_tareas(char* tareas);
