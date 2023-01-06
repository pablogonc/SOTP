/*
 * interruptServer.c
 *
 *  Created on: 29 abr. 2022
 *      Author: utnso
 */



#include "interruptServer.h"

void prepararInterruptServer(char* puerto){

	pthread_t interruptServer;
	pthread_create( &interruptServer , NULL ,  iniciarServerInterrupt , (void*) puerto);

}

void* iniciarServerInterrupt(void* puerto){
	printf("Iniciando servidor kernel Interrupt...\n");
	log_info(logger,"Iniciando servidor kernel Interrupt...");

	sockets_interrupt_sem = (sem_t*) malloc(sizeof(sem_t));
	sem_init(sockets_interrupt_sem,1,0);

	int server_socket = connection_start_server(NULL,(char*) puerto);

	if(server_socket<0){

		connection_server_error(server_socket);

	}else{

		printf(PRINT_COLOR_BLUE "Servidorkernel Interrupt listo. Esperando conexion...\n" PRINT_COLOR_RESET);

		int status = connection_handler(server_socket,interruptListenner,sockets_interrupt_sem);

		if(server_socket<0)
			connection_server_error(status);
	}
	return puerto;
}

void interruptListenner(void* arg){
	int consolaSock = *(int*)arg;
	sem_post(sockets_interrupt_sem); //libera la espera del server para estar seguro de que no se sobreescriba la variable arg
	printf(PRINT_COLOR_GREEN"Kernel conectado en socket %d \n"PRINT_COLOR_RESET,consolaSock);
	log_info(logger,"Kernel conectado en socket %d",consolaSock);

	int operacion=0;
		int connectionStatus;

		connectionStatus = connection_recive_check(consolaSock);

		while(connectionStatus != -1){

			operacion = connection_recive_int(consolaSock);

			switch(operacion){

				case OP_INTERRUPT:
					//codigo
					interrupt=1;

					break;
			}

			connectionStatus = connection_recive_check(consolaSock);
		}

		printf(PRINT_COLOR_YELLOW"conexion perdida con Kernel\n"PRINT_COLOR_RESET);
		log_error(logger,"conexion perdida con Kernel");
		connection_close(consolaSock);
}
