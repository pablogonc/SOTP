/*
 * consolaServer.h
 *
 *  Created on: 28 abr. 2022
 *      Author: utnso
 */

#ifndef CONSOLASERVER_H_
#define CONSOLASERVER_H_

#include "kernel.h"
#include <semaphore.h>
#include <pthread.h>
#include "consolaServer.h"
#include <utils/connection/connection.h>
#include <utils/globals.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

sem_t* sockets_consola_sem;

void prepararConsolaServer(char*);
//Funciones internas
void* iniciarServerConsola(void*);
void consolaListenner(void*);
void recibirInstrucciones(int);

#endif /* CONSOLASERVER_H_ */
