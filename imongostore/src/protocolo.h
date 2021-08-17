/*
 * protocolo.h
 *
 *  Created on: 3 jul. 2021
 *      Author: utnso
 */

#ifndef PROTOCOLO_H_
#define PROTOCOLO_H_

//-------------------------HANDSHAKE-------------------------
#define DISCORDIADOR 1
#define IMONGOSTORE 3
//------------------------COMANDOS----------------------------
#define EJECUTAR_TAREA 8
#define LOGUEAR_BITACORA 9
#define AGREGAR_TRIPULANTE 24

#define FIN_TRIP 23


//------------------------SABOTAJE---------------------------
#define FSCK 100
#define INFORMAR_SABOTAJE 101
#define SABOTAJE_RESUELTO 105
//-----------------------Notificaciones-iMongo----------------
#define PEDIR_BITACORA 102
#define ENVIAR_BITACORA 103
#define TAREA_EJECUTADA 104



// CODIGO DE TAREAS
#define GENERAR_OXIGENO 90
#define GENERAR_COMIDA 91
#define GENERAR_BASURA 92
#define CONSUMIR_OXIGENO 93
#define CONSUMIR_COMIDA 94
#define DESCARTAR_BASURA 95


//--------------------CONSTANTES-------------------
#define BACKLOG 20
#endif /* PROTOCOLO_H_ */
