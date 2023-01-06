/*
 * consola.h
 *
 *  Created on: 23 abr. 2022
 *      Author: utnso
 */

#ifndef MEMORIA_H_
#define MEMORIA_H_

#include "server.h"
#include "swapManager.h"
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
#include <sys/stat.h>
#include <sys/time.h>
#include <stdbool.h>
#include <math.h>

t_config* memoriaConfig;
t_log* logger;

void* memoria_principal;

int TIMESTAMP_INICIO;

int cachePcbId;
int cacheEntrada1erNivel;
int cacheProcesoLista;
int cachePaginaLatencia;
int cachePagina;

// Structs

typedef struct {
	int marco;
	bool presencia;
	bool uso;
	bool modificado;
} pagina_segundo_nivel;

typedef struct {
	bool libre;
	int proceso;
	int pagina;
	int milisDesdeCarga;
} marco_asignado;

t_list* tablasPaginasPrimerNivel;
t_list* tablasPaginasSegundoNivel;
t_list* marcosAsignados;
t_list* paginasSwapeadasPorProceso;
t_list* accesosADiscoPorProceso;

//Variables config
char* PUERTO_ESCUCHA;
uint32_t TAM_MEMORIA;
uint32_t TAM_PAGINA;
uint32_t ENTRADAS_POR_TABLA;
uint32_t RETARDO_MEMORIA;
char* ALGORITMO_REEMPLAZO;
uint32_t MARCOS_POR_PROCESO;
uint32_t RETARDO_SWAP;
char* PATH_SWAP;

int CANTIDAD_DE_MARCOS;

typedef enum
{
	CREAR_PRIMER_NIVEL,
	LIBERAR_PRIMER_NIVEL,
	RESPONDER_PRIMER_NIVEL,
	RESPONDER_SEGUNDO_NIVEL,
	RESPONDER_MEMORIA_USUARIO,
	ESCRIBIR_MEMORIA_USUARIO,
	SUSPENDER_PROCESO
}memoria_accion;

sem_t* memoria_sem;
sem_t* memoria_actividad_sem;

typedef struct{
	memoria_accion accion;
	void* data;
}AccionMemoria;

t_list* acciones_memoria;
AccionMemoria * siguienteAccionMemoria;

//Funciones internas
void leerConfig(void);
void iniciarMemoriaPrincipal();
void iniciarListadoTablasPaginas();
bool verificarProcesoValido(int);
int crearTablaPrimerNivel(int);
int crearTablaSegundoNivel(int);
void realizarAccionMemoria();
void prepararMemoria();
void prepararAccionMemoria(memoria_accion,void*);
bool memoriaSuficiente();
void escribirPalabraMemoria(int,int,uint32_t);
uint32_t leerPalabraMemoria(int,int);
void modificarBitsPaginaSegundoNivelSegunMarco(int,bool);
void escribirMarcoMemoria(int,void*);
void* leerMarcoMemoria(int);
int getIndice2doNivel(int,int);
int getMarco(int,int);
void mandarPaginaASwap(uint32_t,int,bool);
void traerPaginaDeSwapYAsignarAMarco(uint32_t,int);
void _int_destroy(int*);
int milis();
int getMarcoLibreParaPagina(uint32_t,int);
bool procesoDentroLimiteMarcosPorProceso(uint32_t);
bool _estaLibre(void*);
bool _esDelProcesoYEstaCargado(void*);
void _liberarMarco(void*);
void _mandarPaginaASwap(void*);
void liberarMarcosDelProceso(uint32_t);
int getMarcoDePagina(uint32_t,int);
void mandarPaginasASwap(uint32_t);
void asignarMarcoEnTabla2doNivel(int,uint32_t,int);
void desasignarMarcoEnTabla2doNivel(uint32_t,int);
void latenciaMemoria();
bool paginaEstaEnSwap(uint32_t,int);
bool _esLaPagina(int*);
void printMarcosAsignados();
void printPaginas();
bool laPaginaEstaModificada(uint32_t,int);
bool esClock();

// SUSTITUCION

int recorrerFramesClock(t_list* frames);
int seleccionarVictimaClock();
int buscoFrameUM(t_list * frames, bool bitUso, bool bitModificado, bool reemplazoBitUsuario);
int buscoFrameU1M1(t_list * frames);
int buscoFrameU1M0(t_list * frames);
int buscoFrameU0M1(t_list * frames);
int buscoFrameU0M0(t_list * frames);
int seleccionarVictimaClockModificado();
int seleccionarFrameVictima(int pcbId);
void* paginaAMarco (void* pagina);
pagina_segundo_nivel* paginaPorMarco(t_list* marcosOcupados, int marco);
void ordenarPaginasSegundoNivelPorFifo(t_list* lista);
bool comparadorPaginas (void* pagina1, void* pagina2);

#endif /* MEMORIA_H_ */
