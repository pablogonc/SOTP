/*
 * consola.h
 *
 *  Created on: 23 abr. 2022
 *      Author: utnso
 */

#ifndef CONSOLA_H_
#define CONSOLA_H_



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

t_config* consolaConfig;
t_log* logger;
FILE* fileTarea;
t_list* listaTareas;

char* IP_KERNEL;
char* PUERTO_KERNEL;
char* rutaTareas;
int tamanioTareas;

//Funciones internas
void obtenerArgumentos(int,char**);
int conectarKernel(void);
void leerConfig(void);
void parsearTareas(void);
void enviarTareas(int);
void esperarRespuesta(int);
void agregarInstruccion(char*);

#endif /* CONSOLA_H_ */
