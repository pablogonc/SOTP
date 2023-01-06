/*
 * cpu.c
 *
 *  Created on: 23 abr. 2022
 *      Author: utnso
 */


#include "cpu.h"

int main(){
	printf(PRINT_COLOR_GREEN "INICIANDO MODULO CPU..."PRINT_COLOR_RESET "\n" );

	iniciarVariables();

	logger = log_create("./logs/cpu.log", "cpu", 0, LOG_LEVEL_DEBUG);
	loggerMMU = log_create("./logs/mmu.log", "mmu", 0, LOG_LEVEL_DEBUG);
	leerConfig();

	prepararDispatchServer(PUERTO_ESCUCHA_DISPATCH);

	prepararInterruptServer(PUERTO_ESCUCHA_INTERRUPT);

	memoriaSocket = conectarMemoria();

	runCpu();


	char * rta = readline("\n\nPresione enter para salir");
	free(rta);
	config_destroy(cpuConfig);
	log_destroy(logger);
	log_destroy(loggerMMU);

	close(memoriaSocket);
}

void iniciarVariables(){
	sem_run = (sem_t*) malloc(sizeof(sem_t));
	sem_init(sem_run,1,0);
	interrupt=0;
	cache = list_create();
}

void runCpu(){
	pthread_t t_runCpu;
	pthread_create( &t_runCpu , NULL ,  cpu , NULL);
}

void* cpu(){
	int* estado=NULL;


	while(1){
		sem_wait(sem_run);

		ejecutarProceso();
	}
	return estado;
}


int conectarMemoria(){
	ENTRADAS_POR_TABLA = -1;
	TAM_PAGINA = -1;
	printf(PRINT_COLOR_BLUE "Conectando con mododulo Memoria..."PRINT_COLOR_RESET "\n" );
	log_info(logger,"Conectando con mododulo Memoria...");
	int server_socket = connection_connect(IP_MEMORIA,PUERTO_MEMORIA);
	if(server_socket > 0){
		printf(PRINT_COLOR_GREEN "conexion con Memoria aceptada"PRINT_COLOR_RESET "\n\n" );
		log_info(logger,"conexion con Memoria aceptada");
		realizarHandShake(server_socket);
	}else{
		printf(PRINT_COLOR_RED"Conexion con Memoria rechazada" PRINT_COLOR_RESET "\n\n");
		log_error(logger,"Conexion con Memoria rechazada");
	}
	return server_socket;
}

void leerConfig(){

	cpuConfig = config_create("./configs/CPU.config");

	ENTRADAS_TLB = config_get_int_value(cpuConfig,"ENTRADAS_TLB");
	REEMPLAZO_TLB = config_get_string_value(cpuConfig,"REEMPLAZO_TLB");
	RETARDO_NOOP = config_get_int_value(cpuConfig,"RETARDO_NOOP");
	IP_MEMORIA = config_get_string_value(cpuConfig,"IP_MEMORIA");
	PUERTO_MEMORIA = config_get_string_value(cpuConfig,"PUERTO_MEMORIA");
	PUERTO_ESCUCHA_DISPATCH = config_get_string_value(cpuConfig,"PUERTO_ESCUCHA_DISPATCH");
	PUERTO_ESCUCHA_INTERRUPT = config_get_string_value(cpuConfig,"PUERTO_ESCUCHA_INTERRUPT");

	printf("=== Archivo de configuracion ====\n"PRINT_COLOR_YELLOW
			" ENTRADAS TLB: %d \n"
			" REEMPLAZO TLB: %s \n"
			" RETARDO NOOP: %d \n"
			" IP MEMORIA: %s \n"
			" PUERTO MEMORIA: %s \n"
			" PUERTO ESCUCHA_DISPATCH: %s \n"
			" PUERTO ESCUCHA INTERRUPT: %s \n"
			PRINT_COLOR_RESET,
			ENTRADAS_TLB, REEMPLAZO_TLB,RETARDO_NOOP,IP_MEMORIA,PUERTO_MEMORIA,PUERTO_ESCUCHA_DISPATCH,PUERTO_ESCUCHA_INTERRUPT);
}

void realizarHandShake(int Sock){
	connection_packet * p;
	p = connection_packet_create_with_opCode((int) 67); // Numero handshake
	connection_packet_send(p, Sock);
	connection_packet_destroy(p);
	ENTRADAS_POR_TABLA = connection_recive_int(Sock);
	TAM_PAGINA = connection_recive_int(Sock);
	printf(PRINT_COLOR_GREEN "CPU: Entradas por tabla y Tamaño de página recibidas"PRINT_COLOR_RESET);
	log_info(logger,"Entradas por tabla y Tamaño de página recibidas");
}

direccionFisica* mmu(int direccionLogica){ //fila 545 https://docs.google.com/spreadsheets/d/1cNmPncRPmQWAU7gPl37Z8MS-aQh1cGZ-BpXEikmocug/edit#gid=0

	log_info(loggerMMU,"Traduciendo direccion logica:'%d'",direccionLogica);
	printf(MMU_PRINT_COLOR "\nMMU: Traduciendo direccion logica:'%d'"PRINT_COLOR_RESET,direccionLogica);

	int numeroPagina= floor(direccionLogica/TAM_PAGINA) ;
	int entradaLv1= floor(numeroPagina/ENTRADAS_POR_TABLA);
	int entradaLv2= numeroPagina % ENTRADAS_POR_TABLA;
	int desplazamiento= direccionLogica - numeroPagina * TAM_PAGINA;

	log_info(loggerMMU,"\tIndice tabla nivel 1: %d\n"
			"\tNumero de Pagina:%d\n"
						"\tDesplazamiento: %d",pcbCPU->tabla_paginas,numeroPagina,desplazamiento);

	printf(MMU_PRINT_COLOR"MMU: (indice tabla nivel 1;Numero de Pagina;tDesplazamiento) = (%d;%d;%d)"
			PRINT_COLOR_RESET,pcbCPU->tabla_paginas,numeroPagina,desplazamiento);

	int marco;

	if((marco = getMarco(numeroPagina)) < 0 ){ //TLBmiss
			tlbmiss++;
			log_info(loggerMMU,"TLB miss - pagina no encontrada en TLB");
			printf(MMU_PRINT_COLOR "MMU: TLB miss - pagina no encontrada en TLB"PRINT_COLOR_RESET );

			//Acceso primer nivel
			connection_packet* p = connection_packet_create_with_opCode(456);//acceso a tabla1

			connection_packet_add(p,&pcbCPU->tabla_paginas,sizeof(uint32_t));
			connection_packet_add(p,&entradaLv1,sizeof(uint32_t));
			connection_packet_send(p,memoriaSocket);
			connection_packet_destroy(p);

			log_info(loggerMMU,"Esperando tabla LV2 de memoria...");
			printf(MMU_PRINT_COLOR "MMU: Esperando tabla LV2 de memoria..." PRINT_COLOR_RESET);
			int paginalv2 = connection_recive_int(memoriaSocket);

			log_info(loggerMMU,"Pagina LV2 recibida: %d",paginalv2);
			printf(MMU_PRINT_COLOR "MMU: Pagina LV2 recibida: %d" PRINT_COLOR_RESET,paginalv2);

			//Acceso segundo nivel
			p = connection_packet_create_with_opCode(457);//acceso a tabla2
			connection_packet_add(p,&paginalv2,sizeof(uint32_t));
			connection_packet_add(p,&entradaLv2,sizeof(uint32_t));
			connection_packet_send(p,memoriaSocket);
			connection_packet_destroy(p);
			log_info(loggerMMU,"Esperando marco de memoria...");
			printf(MMU_PRINT_COLOR "MMU: Esperando marco de memoria..."PRINT_COLOR_RESET);
			marco = connection_recive_int(memoriaSocket);

			log_info(loggerMMU,"Marco  recibido: %d",marco);
			printf(MMU_PRINT_COLOR "MMU: Marco  recibido: %d" PRINT_COLOR_RESET,marco);

			addMarco(marco,numeroPagina);
	}
	log_info(loggerMMU,"Traduccion Completa\n");
	printf(MMU_PRINT_COLOR "MMU: Traduccion Completa\n"PRINT_COLOR_RESET);

	direccionFisica* df = malloc(sizeof(direccionFisica));
	df->desplazamiento = desplazamiento;
	df->marco = marco;

	return df;

}

void timeInit(){
	gettimeofday(&start,NULL);
}

int getTime(){
	gettimeofday(&stop,NULL);
	return ((stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec) / 1000;
}
