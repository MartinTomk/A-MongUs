#ifndef SOCKET_H_
#define SOCKET_H_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <signal.h>
#include <netdb.h>
#include <math.h>
#include <semaphore.h>
#include <readline/readline.h>
#include <readline/history.h>
#include<commons/string.h>
#include "utils.h"



int conectarAServer(char *ip, int puerto);
int iniciarServidor(int);
void verificarErrorSocket(int);
void verificarErrorSetsockopt(int);
void verificarErrorBind(int, struct sockaddr_in);
void verificarErrorListen(int);
int aceptarConexionDeCliente(int);
int calcularSocketMaximo(int, int);
void sendRemasterizado(int, int, int, void*);
void sendDeNotificacion(int, uint32_t);
uint32_t recibirUint(int);
int recibirInt(int);
char* recibirString(int);
int recvDeNotificacion(int);
int conectarAServer(char*, int);
void realizarHandshake(int, uint32_t,uint32_t);
void liberar_conexion (int socket);


#endif /* SOCKET_H_ */
