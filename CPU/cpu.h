/*
 * cpu.h
 *
 *  Created on: 23 abr. 2022
 *      Author: utnso
 */

#ifndef CPU_H_
#define CPU_H_

#include "interruptServer.h"
#include "dispatchServer.h"
#include <utils/connection/connection.h>
#include <utils/globals.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <readline/readline.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdint.h>
#include <math.h>
#include <sys/time.h>

#define MMU_PRINT_COLOR   PRINT_COLOR_MAGENTA

t_config* cpuConfig;
t_log* logger;
t_log* loggerMMU;

typedef struct{
	int marco;
	int desplazamiento;
}direccionFisica;

typedef struct{
	int pagina;
	int marco;
	int instanteCarga;
	int instanteUltimaReferencia;
}traduccion;

struct timeval stop , start;

//
t_list* cache;

//Variables config

uint32_t ENTRADAS_TLB;
char* REEMPLAZO_TLB;
uint32_t RETARDO_NOOP;
char* IP_MEMORIA;
char* PUERTO_MEMORIA;
char* PUERTO_ESCUCHA_DISPATCH;
char* PUERTO_ESCUCHA_INTERRUPT;

int rafagaCPU;

int memoriaSocket;
uint32_t ENTRADAS_POR_TABLA;
uint32_t TAM_PAGINA;

//Funciones internas
void iniciarVariables();
void runCpu();
void* cpu();
int conectarMemoria(void);
void leerConfig(void);
void realizarHandShake();
direccionFisica* mmu(int);

int tlbmiss;


void timeInit();
int getTime();

#endif /* CPU_H_ */
