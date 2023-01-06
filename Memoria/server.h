/*
 * server.h
 *
 *  Created on: 29 abr. 2022
 *      Author: utnso
 */

#ifndef SERVER_H_
#define SERVER_H_

#include "swapManager.h"
#include "memoria.h"
#include <semaphore.h>
#include <pthread.h>
#include <utils/connection/connection.h>
#include <utils/globals.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    uint32_t tamanio;
    int Sock;
} TamanioYSocket;

typedef struct {
    int indice;
    int entrada;
    int Sock;
} IndiceEntradaSock;

typedef struct {
    int marco;
    int desplazamiento;
    int palabra;
    int Sock;
} MarcoDesplazamientoPalabraSock;

sem_t* socket_server_sem;

void prepararServer(char*);
//Funciones internas
void *iniciarServer(void*);
void serverListenner(void*);
void recibirPCB(int);
void liberarPCB(int);
void handshakeCPU(int);
void responder1erNivel(int);
void responder2doNivel(int);
void responderMemoriaUsuario(int);
void escribirMemoriaUsuario(int);
void suspenderProceso(int);

#endif /* SERVER_H_ */
