/*
 * interruptServer.h
 *
 *  Created on: 29 abr. 2022
 *      Author: utnso
 */

#ifndef INTERRUPTSERVER_H_
#define INTERRUPTSERVER_H_

#include <semaphore.h>
#include <pthread.h>
#include <utils/connection/connection.h>
#include <utils/globals.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include "cpu.h"

sem_t* sockets_interrupt_sem;
int interrupt;
void prepararInterruptServer(char*);
//Funciones internas
void* iniciarServerInterrupt(void*);
void interruptListenner(void*);

#endif /* INTERRUPTSERVER_H_ */
