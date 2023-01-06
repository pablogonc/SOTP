/*
 * KernelDispatch.c
 *
 *  Created on: 30 may. 2022
 *      Author: utnso
 */

#include "KernelDispatch.h"


void correrDispatcher(){
	pthread_t dispatchThread;
	pthread_create( &dispatchThread , NULL ,  dispatcher , NULL);
}

void*  dispatcher(){
	int* state = NULL;

	int tBloqueo;
	int rafagaCPU;

		while(1){
			connection_recive_int(cpuDSocket); //muere hasta que cpu manda algo
			Proceso* proceso = getProceso(cpuDSocket);
			tBloqueo = connection_recive_int(cpuDSocket);
			rafagaCPU = connection_recive_int(cpuDSocket);

			switch(proceso->estado){

				case E_EXIT:
					printf(PRINT_COLOR_RED"Kernel:Proceso %d Terminado"PRINT_COLOR_RESET,proceso->pcb->id);
					log_info(logger,"Kernek:Proceso %d Terminado\n",proceso->pcb->id);
					mover(proceso, procesosExec,procesosExit , E_EXIT);
					avisarExit(memoriaSocket,proceso);
					avisarExit(proceso->socket,proceso);

					sem_post(semMutexPlanificar);

					break;

				case E_BLOCKED:


					printf(PRINT_COLOR_YELLOW"Kernel:Proceso %d bloqueado"PRINT_COLOR_RESET,proceso->pcb->id);
					log_info(logger,"Proceso %d bloqueado\n",proceso->pcb->id);
					mover(proceso, procesosExec,procesosBlock , E_BLOCKED);

					proceso->ioTime = tBloqueo;

					proceso->lockInit = getTime();

					sem_post(semMutexPlanificar);
					bloquearProceso(proceso);

					break;

				case E_EXEC:

					mover(proceso, procesosExec,procesosReady , 75);

					break;

				default:
					//Algun error supongo
					break;
			}

			proceso->tExec = rafagaCPU;
			if (interrupt){
				interrupt=0;
				printf("Kernel:Proceso %d recibido por interrupcion"PRINT_COLOR_RESET,proceso->pcb->id );
				log_info(logger,"Proceso %d recibido por interrupcion",proceso->pcb->id);
				sem_post(sem_interrupt);
			}

		}

	return state;
}

Proceso* getProceso(int socket){
	Proceso* proceso = list_get(procesosExec,0);

	int id = connection_recive_int(socket);;
	if(proceso->pcb->id == id){
		proceso->pcb->program_counter = connection_recive_int(socket);
		proceso->estado = connection_recive_int(socket);
	}else{ //error paso algo raro

	}

	return proceso;
}

void avisarExit(int socket,Proceso* proceso){
	connection_packet* p = connection_packet_create_with_opCode(OP_EXIT_PCB);
	connection_packet_add(p,&proceso->pcb->id,sizeof(uint32_t));
	connection_packet_send(p,socket);
	connection_packet_destroy(p);
}
