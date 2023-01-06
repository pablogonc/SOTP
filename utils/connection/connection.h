/*
 * connection.h
 *
 *  Created on: 5 ago. 2021
 *      Author: utnso
 */

#ifndef SRC_UTILS_CONNECTION_CONNECTION_H_
#define SRC_UTILS_CONNECTION_CONNECTION_H_

#include <semaphore.h>
//MENSAJES
typedef enum //codigos de operaciones servidor
{
	connection_serverError_crear_socket = -4,
	connection_serverError_puerto_cupado,
	connection_serverError_Conexion_rechazada,
	connection_serverError_crear_hilo
}Connection_Code;
//ERRORES
void connection_server_error(int serverStatus);


//PAQUETES
typedef struct
{
	int size;
	void* stream;

} connection_packet;

//MANEJO DE PAQUETES

/*crea un paquete vacio*/
connection_packet* connection_packet_create();

/*crea el paquete con op code y un entero de estado de conexion*/
connection_packet* connection_packet_create_with_opCode(int operationCode);

/* añade el valor al paquetet*/
void connection_packet_add(connection_packet* packet, void* value, int valueSize );

/* añade el valor y tambien su tamaño como int al paquete */
void connection_packet_add_with_size(connection_packet* packet, void* value, int valueSize);

void connection_packet_send(connection_packet* packet, int port);

void connection_packet_destroy(connection_packet* packet);



//CONEXION
int connection_start_server(char * ip,char * port);
int connection_handler(int serverSocket,void* callFunction,sem_t* sockets_sem);

int connection_connect(char *ip, char* port);

void connection_close(int socket);

//COMUNICACION

int connection_send(void* value,int port);

int connection_recive_check(int socket);

void* connection_recive(int socket,int size);

int connection_recive_int(int socket);

//primero solicita al socket el tamanio del string y luego retorna el string
char* connection_recive_string(int socket);

#endif /* SRC_UTILS_CONNECTION_CONNECTION_H_ */
