#ifndef SWAPMANAGER_H_
#define SWAPMANAGER_H_

#include "memoria.h"
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
#include <semaphore.h>
#include <unistd.h>

typedef enum
{
	SWAP_CREAR,
	SWAP_AGREGAR_PAGINA,
	SWAP_RETIRAR_PAGINA,
	SWAP_ELIMINAR
}swap_accion;

sem_t* swap_action_sem;
sem_t* swap_actividad_sem;

char* path;
uint32_t retardo;

typedef struct{
	swap_accion accion;
	void* data;
}AccionSwap;

t_list* acciones_swap;
AccionSwap * siguienteAccion;

void prepararSwap(char*,uint32_t);
void prepararAccionSwap();
//Funciones internas
void nuevoSwap();
void leerSwap();
void *iniciarSwap();
void realizarAccion();
char * nombreArchivoSwap(uint32_t);
void leerPaginaDeSwap();
void escribirPaginaEnSwap();
void latenciaSwap();

#endif /* SWAPMANAGER_H_ */
