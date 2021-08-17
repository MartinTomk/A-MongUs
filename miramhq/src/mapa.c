
#include "mapa.h"
#define ASSERT_CREATE(nivel, id, err)                                                   \
    if(err) {                                                                           \
        nivel_destruir(nivel);                                                          \
        nivel_gui_terminar();                                                           \
        fprintf(stderr, "Error al crear '%c': %s\n", id, nivel_gui_string_error(err));  \
        return EXIT_FAILURE;                                                            \
    }
int creadas = 0;

void* mapa(void* arg) {

	nivel_gui_inicializar();

	nivel_gui_get_area_nivel(&cols, &rows);

	nivel = nivel_crear("Nave_AFIRM");

	sem_init(&actualizarMapa,0,0);
	while ( 1 ) {
		nivel_gui_dibujar(nivel);
		sem_wait(&actualizarMapa);
		if(err) {
			printf("WARN: %s\n", nivel_gui_string_error(err));
		}
	}

}
int iniciar_mapa(void){
	pthread_create(&mapaHilo , NULL, mapa , NULL);
	return 0;
}
int crear_tareas(char* tareas){
	char** tareas_separadas = string_split(tareas,"\n");
	uint32_t i = 0;
	while (tareas_separadas[i] != NULL){
		char** aux = string_split(tareas_separadas[i],";");
		uint32_t x = atoi(aux[1]);
		uint32_t y = atoi(aux[2]);
		caja_crear(nivel,30+creadas,x,y,10);
		creadas++;
		i++;
	}
	return 0;
}
