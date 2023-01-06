/*
 * server.c
 *
 *  Created on: 29 abr. 2022
 *      Author: utnso
 */

#include "server.h"

void prepararServer(char* puerto){
	pthread_t ConsolaServer;
	pthread_create( &ConsolaServer , NULL ,  iniciarServer , (void*) puerto);
}

void *iniciarServer(void* puerto){
	printf("Iniciando servidor\n");
	socket_server_sem = (sem_t*) malloc(sizeof(sem_t));
	sem_init(socket_server_sem,1,0);

	int server_socket = connection_start_server(NULL,(char*) puerto);
	if(server_socket<0){
		connection_server_error(server_socket);
	} else {
		printf(PRINT_COLOR_BLUE "Servidor listo. Esperando Modulos\n" PRINT_COLOR_RESET);

		int status = connection_handler(server_socket,serverListenner,socket_server_sem);
		if(server_socket<0)
			connection_server_error(status);
	}
	return puerto;
}

void serverListenner(void* arg){
	int Sock = *(int*)arg;
	sem_post(socket_server_sem); //libera la espera del server para estar seguro de que no se sobreescriba la variable arg
	printf(PRINT_COLOR_GREEN"Cliente conectado en socket %d \n"PRINT_COLOR_RESET,Sock);

	int operacion = 0;
	int connectionStatus;

	connectionStatus = connection_recive_check(Sock);

	connection_packet* p;

	while(connectionStatus != -1){
		operacion = connection_recive_int(Sock);
		switch(operacion){
			case OP_NUEVO_PCB:
				recibirPCB(Sock);
				break;
			case OP_EXIT_PCB:
				liberarPCB(Sock);
				break;
			case OP_PROCESO_SUSPENDIDO:
				suspenderProceso(Sock);
				break;
			case OP_PROCESO_DESUSPENDIDO:
				connection_recive_int(Sock);
				p = connection_packet_create();
				int confirm = OP_CONFIRMACION;
				connection_packet_add(p, &confirm, sizeof(uint32_t));
				connection_packet_send(p,Sock);
				connection_packet_destroy(p);
				break;
			case 67:
				handshakeCPU(Sock);
				break;
			case 456:
				responder1erNivel(Sock);
				break;
			case 457:
				responder2doNivel(Sock);
				break;
			case 458:
				responderMemoriaUsuario(Sock);
				break;
			case 500:
				escribirMemoriaUsuario(Sock);
				break;
			case 501:
				escribirMemoriaUsuario(Sock);
				break;
			default:
				break;
		}
		connectionStatus = connection_recive_check(Sock);
	}

	printf(PRINT_COLOR_YELLOW"Conexion perdida con cliente\n"PRINT_COLOR_RESET);
	connection_close(Sock);
}

void recibirPCB(int Sock) {
	PCB* pcb = malloc(sizeof(PCB));
	pcb->estimacion_rafaga = connection_recive_int(Sock);
	pcb->id = connection_recive_int(Sock);
	int cantidadI = connection_recive_int(Sock);
	for(int i = 0; i < cantidadI; i++){
		connection_recive_int(Sock); // id
		int cantidadParametros = connection_recive_int(Sock); // cantidadParametros
		for (int j = 0; j < cantidadParametros; j++){
			connection_recive_int(Sock); // parametros[j]
		}
	}
	pcb->program_counter = connection_recive_int(Sock);
	pcb->tamanio = connection_recive_int(Sock);

	printf(PRINT_COLOR_GREEN"Nueva PCB\n"PRINT_COLOR_RESET);
	printf("ID: %d, Tamaño en bytes: %d\n", pcb->id, pcb->tamanio);
	log_info(logger, "Nueva PCB\nID: %d, Tamaño en bytes: %d\n", pcb->id, pcb->tamanio);

	prepararAccionSwap(SWAP_CREAR, &pcb->id);
	sem_post(swap_action_sem);

	TamanioYSocket * ts = malloc(sizeof(TamanioYSocket));
	ts->tamanio = pcb->tamanio;
	ts->Sock = Sock;
	prepararAccionMemoria(CREAR_PRIMER_NIVEL, ts);
	sem_post(memoria_sem);

	sem_wait(swap_actividad_sem);
	sem_wait(memoria_actividad_sem);
	free(pcb);
}

void liberarPCB(int Sock) {
	uint32_t pcbId = connection_recive_int(Sock);

	int * accesos = list_get(accesosADiscoPorProceso, pcbId);
	printf(PRINT_COLOR_RED"PCB %d liberada con %d accesos a disco\n"PRINT_COLOR_RESET, pcbId, *accesos);
	log_info(logger, "PCB %d liberada con %d accesos a disco\n", pcbId, *accesos);

	prepararAccionSwap(SWAP_ELIMINAR, &pcbId);
	sem_post(swap_action_sem);

	prepararAccionMemoria(LIBERAR_PRIMER_NIVEL, &pcbId);
	sem_post(memoria_sem);
}

void handshakeCPU(int Sock) {
	printf("Enviando configuraciones al CPU\n");
	connection_packet * p = connection_packet_create();
	connection_packet_add(p, &ENTRADAS_POR_TABLA, sizeof(uint32_t));
	connection_packet_add(p, &TAM_PAGINA, sizeof(uint32_t));
	connection_packet_send(p, Sock);
	connection_packet_destroy(p);
}

void responder1erNivel(int Sock){
	IndiceEntradaSock * ies = malloc(sizeof(IndiceEntradaSock));
	ies->indice = connection_recive_int(Sock);
	ies->entrada = connection_recive_int(Sock);
	ies->Sock = Sock;
	prepararAccionMemoria(RESPONDER_PRIMER_NIVEL, ies);
	sem_post(memoria_sem);
}

void responder2doNivel(int Sock){
	IndiceEntradaSock * ies = malloc(sizeof(IndiceEntradaSock));
	ies->indice = connection_recive_int(Sock);
	ies->entrada = connection_recive_int(Sock);
	ies->Sock = Sock;
	prepararAccionMemoria(RESPONDER_SEGUNDO_NIVEL, ies);
	sem_post(memoria_sem);
}

void responderMemoriaUsuario(int Sock){
	IndiceEntradaSock * ies = malloc(sizeof(IndiceEntradaSock));
	ies->indice = connection_recive_int(Sock);
	ies->entrada = connection_recive_int(Sock);
	ies->Sock = Sock;
	prepararAccionMemoria(RESPONDER_MEMORIA_USUARIO, ies);
	sem_post(memoria_sem);
}

void escribirMemoriaUsuario(int Sock){
	//TODO: ver qué hacer si falla (CPU)
	MarcoDesplazamientoPalabraSock * mdps = malloc(sizeof(MarcoDesplazamientoPalabraSock));
	mdps->marco = connection_recive_int(Sock);
	mdps->desplazamiento = connection_recive_int(Sock);
	mdps->palabra = connection_recive_int(Sock);
	mdps->Sock = Sock;
	prepararAccionMemoria(ESCRIBIR_MEMORIA_USUARIO, mdps);
	sem_post(memoria_sem);
}

void suspenderProceso(int Sock){
	uint32_t pcbId = connection_recive_int(Sock);
	TamanioYSocket * ts = malloc(sizeof(TamanioYSocket));
	ts->tamanio = pcbId;
	ts->Sock = Sock;
	prepararAccionMemoria(SUSPENDER_PROCESO, ts);
	sem_post(memoria_sem);
}
