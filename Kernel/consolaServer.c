/*
 * consolaServer.c
 *
 *  Created on: 28 abr. 2022
 *      Author: utnso
 */

#include "consolaServer.h"

void prepararConsolaServer(char* puerto){

	pthread_t ConsolaServer;
	pthread_create( &ConsolaServer , NULL ,  iniciarServerConsola , (void*) puerto);

}

void* iniciarServerConsola(void* puerto){
	printf("Iniciando servidor para consolas...\n");

	sockets_consola_sem = (sem_t*) malloc(sizeof(sem_t));
	sem_init(sockets_consola_sem,1,0);

	int server_socket = connection_start_server(NULL,(char*) puerto);

	if(server_socket<0){

		connection_server_error(server_socket);
		log_error(logger,"Hubo un problema al crear el servidor para consolas. Codigo:\"%d\"",server_socket);
	}else{

		printf(PRINT_COLOR_BLUE "Servidor para consolas listo. Esperando conexiones de consolas..." PRINT_COLOR_RESET);
		log_info(logger,"Servidor para consolas listo. Esperando conexiones");
		int status = connection_handler(server_socket,consolaListenner,sockets_consola_sem);

		if(server_socket<0)
			connection_server_error(status);

	}
	return puerto;
}

void consolaListenner(void* arg){
	int consolaSock = *(int*)arg;
	sem_post(sockets_consola_sem); //libera la espera del server para estar seguro de que no se sobreescriba la variable arg
	printf(PRINT_COLOR_GREEN"Kernel: Consola conectada en socket %d "PRINT_COLOR_RESET,consolaSock);
	log_info(logger,"Consola conectada en socket %d",consolaSock);
	int operacion=0;
		int connectionStatus;

		connectionStatus = connection_recive_check(consolaSock);

		while(connectionStatus != -1){

			operacion = connection_recive_int(consolaSock);

			switch(operacion){

				case OP_LISTA_INSTRUCCIONES:
					recibirInstrucciones(consolaSock);
					break;
			}

			connectionStatus = connection_recive_check(consolaSock);
		}

		printf(PRINT_COLOR_YELLOW"Kernel: Conexion perdida con socket: %d"PRINT_COLOR_RESET,consolaSock);
		log_warning(logger,"Conexion perdida con socket %d",consolaSock);
		connection_close(consolaSock);
}

void recibirInstrucciones (int socket){
	printf("Kernel: Recibiendo instrucciones del socket %d...\n",socket);
	log_info(logger,"Recibiendo instrucciones del socket %d...",socket);
	int tamanioTareas = connection_recive_int(socket);

	int cantidadInstrucciones = connection_recive_int(socket);

	Proceso* proceso = malloc(sizeof(Proceso));
	proceso->pcb = malloc(sizeof(PCB));
	proceso->ioInit=0;
	proceso->lockInit = 0;
	proceso->ioTime = 0;
	proceso->tExec = 0;
	proceso->pcb->id = list_size(procesos);
	proceso->estado = E_NEW;
	proceso->pcb->tamanio = tamanioTareas;
	proceso->pcb->estimacion_rafaga = ESTIMACION_INICIAL;
	proceso->pcb->program_counter=0;

	proceso->pcb->instruccioes = list_create();

	for(int i = 0 ; i<cantidadInstrucciones; i++){
		connection_recive_int(socket);
		Instruccion* instruccion = malloc(sizeof(Instruccion));

		instruccion->id = connection_recive_int(socket);
		instruccion->cantidadParametros = connection_recive_int(socket);
		instruccion->parametros = malloc(sizeof(int) * instruccion->cantidadParametros);


		for(int j=0; j<instruccion->cantidadParametros;j++){
			instruccion->parametros[j] = connection_recive_int(socket);
		}

		list_add(proceso->pcb->instruccioes,instruccion);
	}
	proceso->socket = socket;

	//printf("%s",verProceso(proceso));
	//log_info(logger,"%s",verProceso(proceso));

	list_add(procesos,proceso);
	list_add(procesosNew,proceso);

	int estado = -15;
	connection_packet *rta = connection_packet_create();
	connection_packet_add(rta,&estado,sizeof(int));
	connection_packet_send(rta,socket);
	connection_packet_destroy(rta);

	sem_post(semMutexPlanificar);
}


