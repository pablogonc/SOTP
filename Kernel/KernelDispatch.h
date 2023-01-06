/*
 * KernelDispatch.h
 *
 *  Created on: 30 may. 2022
 *      Author: utnso
 */

#ifndef KERNELDISPATCH_H_
#define KERNELDISPATCH_H_
#include "kernel.h"


void correrDispatcher();
void* dispatcher();
Proceso* getProceso(int socket);
void avisarExit(int socket,Proceso* proceso);



#endif /* KERNELDISPATCH_H_ */
