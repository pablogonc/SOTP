/*
 * connection.c
 *
 *  Created on: 5 ago. 2021
 *      Author: utnso
 */

#include"connection.h"

#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<string.h>
#include "../globals.h"


int connection_handler(int,void*,sem_t*);
int connection_wait_client(int);
int connection_wait_client(int server_socket);
//errores
void connection_server_error(int serverStatus){
	switch(serverStatus){
		case connection_serverError_crear_socket:
			printf(PRINT_COLOR_RED "Error al crear el socket\n" PRINT_COLOR_RESET);
			break;
		case connection_serverError_puerto_cupado:
			printf(PRINT_COLOR_RED "Error socket Ocupado\n" PRINT_COLOR_RESET);
			break;
		case connection_serverError_crear_hilo:
			printf(PRINT_COLOR_RED "Error al crear el hilo\n" PRINT_COLOR_RESET);
			break;
		case connection_serverError_Conexion_rechazada: //creo que esto no rompe igual
			printf(PRINT_COLOR_RED "Error Conexion_rechazada\n" PRINT_COLOR_RESET);
			break;
	}
}


//conexiones
int connection_start_server(char * ip,char * port){
	int server_socket;
	struct addrinfo hints, *servinfo, *p;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, port, &hints, &servinfo);

	for (p=servinfo; p != NULL; p = p->ai_next)
	{
		if ((server_socket = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
			return connection_serverError_crear_socket;
			printf("connection_error_crear_socket");
		}

		int yes = 1;
		setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);
		if (bind(server_socket, p->ai_addr, p->ai_addrlen) == -1) {
			close(server_socket);
			//puerto ocupado creo
			return connection_serverError_puerto_cupado;
		}
		break;
	}




	freeaddrinfo(servinfo);


	return server_socket;
}

int connection_handler(int serverSocket,void* callFunction,sem_t* sockets_sem){

	listen(serverSocket, SOMAXCONN);
	pthread_t clientThread;

	while(1){
		int clientSocket;
		clientSocket = connection_wait_client(serverSocket);
		if(clientSocket<0){
			//conexion rechazada
			return connection_serverError_Conexion_rechazada;
		}
		if( pthread_create( &clientThread , NULL ,  callFunction , (void*) &clientSocket) < 0){
			return connection_serverError_crear_hilo;
			//no se pudo crear el hilo
		}
		sem_wait(sockets_sem);
	}
	return 1;
}



int connection_wait_client(int server_socket){

	struct sockaddr_in dir_cliente;
	socklen_t tam_direccion = sizeof(struct sockaddr_in);

	int socket_cliente = accept(server_socket, (void*) &dir_cliente, &tam_direccion);

	return socket_cliente;


}

int connection_connect(char *ip, char* port)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, port, &hints, &server_info);

	int socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	if(connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1)
		return -1;//error al conectar

	freeaddrinfo(server_info);

	return socket_cliente;
}

void connection_close(int socket){
	close(socket);
}

//recepcion de mensajes
int connection_recive_check(int socket){
	int status;
	 if(recv(socket, &status, sizeof(int), MSG_WAITALL)==0){
		 return -1;
	 }else{
		 return status;
	 }
}

void* connection_recive(int socket,int size){

	void * buffer = malloc(size);

	recv(socket, buffer, size, MSG_WAITALL);

	return buffer;
}

int connection_recive_int(int socket){
	void* mem = connection_recive(socket,sizeof(int));
	int valor = *(int*) mem;
	free(mem);
	return valor ;

}
char* connection_recive_string(int socket){

	int length = connection_recive_int(socket);

	return (char*) connection_recive(socket,length);
}



//Tratado de paquetes

connection_packet* connection_packet_create(){
	connection_packet* packet = malloc(sizeof(connection_packet));
	packet->size = 0;
	packet->stream = NULL;

	return packet;
}

connection_packet* connection_packet_create_with_opCode(int op_code){
	connection_packet* packet = malloc(sizeof(connection_packet));
	packet->size = 0;
	packet->stream = NULL;
	int status = 1;
	connection_packet_add(packet,&status,sizeof(int));

	connection_packet_add(packet,&op_code,sizeof(int));

	return packet;
}

void connection_packet_add(connection_packet* packet, void* value, int valueSize)
{
	packet->stream = realloc(packet->stream, packet->size + valueSize);

	memcpy(packet->stream + packet->size, value, valueSize);

	packet->size += valueSize;
}

void connection_packet_add_with_size(connection_packet* packet, void* value, int valueSize)
{
	packet->stream = realloc(packet->stream, packet->size + valueSize + sizeof(int));

	memcpy(packet->stream + packet->size, &valueSize, sizeof(int));
	memcpy(packet->stream + packet->size + sizeof(int), value, valueSize);

	packet->size += valueSize + sizeof(int);
}

void connection_packet_send(connection_packet* packet, int port){

	send(port, packet->stream, packet->size, 0);

}

void connection_packet_destroy(connection_packet* packet){
	free(packet->stream);
	free(packet);
}
