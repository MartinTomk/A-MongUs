#include "discordiador.h"

int main(void) {
	signal(SIGSEGV,terminar_discordiador);
	sabotaje = 0;
	iniciar_logger();
	//Inicia las colas de planificacion

	iniciar_configuracion();

	iniciarEstructurasAdministrativasPlanificador();



//	realizarHandshake(socketServerMiRam, DISCORDIADOR, MIRAM);

	log_info(logger, "Planificador se conecto a MIRAM");

	socketServerIMongoStore = conectarAServer(IP_MONGO, 5003);

	list_add(conexiones,(void*)socketServerIMongoStore);
	list_add(conexiones,(void*)socketServerMiRam);

	realizarHandshake(socketServerIMongoStore, DISCORDIADOR, IMONGOSTORE);

	log_info(logger, "Planificador se conecto a IMONGOSTORE");

	iniciarHiloConsola();

	iniciarHiloQueManejadorNotifiacionesIMongo();

	planificar();

	sem_wait(&terminarPrograma);
	terminar_programa(logger,config);
	return 0;
}



