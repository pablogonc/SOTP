/*
 * dispatchServer.h
 *
 *  Created on: 29 abr. 2022
 *      Author: utnso
 */

#ifndef DISPATCHSERVER_H_
#define DISPATCHSERVER_H_

#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <utils/connection/connection.h>
#include <utils/globals.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include "cpu.h"

sem_t* sockets_dispatch_sem;
PCB* pcbCPU;
uint32_t decodeFlag;
sem_t* sem_run;

int dispatchSock;
int processState; //TODO de momento para tener el estado en que queda total es un solo cpu
int timeBlock;

void prepararDispatchServer(char*);
//Funciones internas
void* iniciarServerDispatch(void*);
void dispatchListenner(void*);
PCB* recibirPCB(int);
void ejecutarProceso();
void enviarPCB(PCB*,int);
Instruccion* fetch();
int decode(Instruccion*);
void fetchOperands();
void execute(int);
void checkInterrupt();
int getMarco(int);
void addMarco(int,int);
int leerValor(int);
int victimaLRU();
int victimaFIFO();
void cleanCache();

#endif /* DISPATCHSERVER_H_ */
