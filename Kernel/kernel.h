/*
 * kernel.h
 *
 *  Created on: 23 abr. 2022
 *      Author: utnso
 */

#ifndef KERNEL_H_
#define KERNEL_H_

#include "consolaServer.h"
#include <semaphore.h>
#include <utils/connection/connection.h>
#include <utils/globals.h>
#include <commons/config.h>
#include <commons/log.h>
#include <readline/readline.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <commons/string.h>
#include <sys/stat.h>
#include <sys/time.h>

typedef enum{
	FIFO,
	SRT
}Algoritmo;



typedef struct{
	PCB* pcb;
	estado estado;
	int socket;
	int ioTime;
	int lockInit;
	int ioInit;
	int tExec;

}Proceso;

struct timeval stopblock , startblock;

//
t_list* procesos;
t_list* procesosNew;
t_list* procesosReady;
t_list* procesosExec;
t_list* procesosBlock; // procesos bloqueados y procesos suspendidos-bloqueados
t_list* procesosExit;
t_list* procesosSuspendedReady; // procesos suspendidos-ready

//semaforos
sem_t* semMutexPlanificar;
sem_t* sem_interrupt;
sem_t* sem_bloqueado;
sem_t* sem_io;
//variables de archivos
t_log* logger;
t_config* kernelConfig;

//variables conexion
int memoriaSocket;
int cpuDSocket;
int cpuISocket;

//Variables config
char* IP_MEMORIA;
char* PUERTO_MEMORIA;
char* IP_CPU;
char* PUERTO_CPU_DISPATCH;
char* PUERTO_CPU_INTERRUPT;
char* PUERTO_ESCUCHA;
Algoritmo ALGORITMO;
uint32_t ESTIMACION_INICIAL;
float ALFA;
uint32_t GRADOMP;
uint32_t TMAXBLOQUEO;

//funciones internas
int procesosBloqueados();
void leerConfig(void);
int conectarCon(char*,char*,char*);
void iniciarVariables(void);
void* planificar();
void enviarPCB(PCB*, int);
void mover(Proceso*, t_list*,t_list*, int);
int buscarProceso(Proceso*, t_list*);
Proceso* calcularSJF(t_list*);
int tiempoExect(Proceso*);
void correrBloqueado();
void correrDispatcher();
void* bloqueado();
void avisarSuspencion (Proceso*, int);
char* verProceso(Proceso*);
char* getname(int);
void liberarVariablies(void);
int interrupt;
int ready;
int getTime();
int minTSusp();
int getTimeToSus(Proceso*);
int getTimeToIOEnd(Proceso*);


void bloquearProceso(Proceso*);
void* bloqueoP(void*);
void suspenderProcesos();

/*
 *
 *
void correrMemoriaKernel();
void *comMemoriaKernel();
Proceso* buscaryRetornarProceso(int, t_list*);
*/
#endif /* KERNEL_H_ */
