/*
 * consola.c
 *
 *  Created on: 23 abr. 2022
 *      Author: utnso
 */

#include "consola.h"

int main(int argc, char** argv){

	printf("Iniciando Consola...\n\n");

	obtenerArgumentos(argc,argv);

	logger = log_create("./logs/consola.log", "Consola", 0, LOG_LEVEL_DEBUG);

	leerConfig();

	int server_socket = conectarKernel();

	if (server_socket>0){

		parsearTareas();

		enviarTareas(server_socket);

		esperarRespuesta(server_socket);
	}


	char* rta = readline("\n\nPresione enter para salir");
	free(rta);

	config_destroy(consolaConfig);
	log_destroy(logger);
	connection_close(server_socket);
	return 0;

}

void obtenerArgumentos(int argc,char** argv){
	rutaTareas = "./tareas/tarea";
	tamanioTareas = 8;

	if(argc != 3) {
		printf(PRINT_COLOR_RED"Error: cantidad de argumentos incorrecta:"PRINT_COLOR_RESET"\n\n");
		//return -1;
		}else{
			rutaTareas = argv[1];
			tamanioTareas = atoi(argv[2]);
			printf(PRINT_COLOR_GREEN"Argumentos de entrada Correctos"PRINT_COLOR_RESET"\n\n");
		}


	printf("=== Argumentos de entrada ===\n ruta: %s \n tamaño: %d \n\n", rutaTareas, tamanioTareas);
}


int conectarKernel(){

	printf(PRINT_COLOR_BLUE "Conectando con mododulo Kernel..."PRINT_COLOR_RESET "\n" );
	log_info(logger,"Conectando con mododulo Kernel...");
	int server_socket = connection_connect(IP_KERNEL,PUERTO_KERNEL);
		if(server_socket >0){
			printf(PRINT_COLOR_GREEN "conexion aceptada"PRINT_COLOR_RESET "\n\n" );

		}else{
			printf(PRINT_COLOR_RED"Conexion rechazada" PRINT_COLOR_RESET "\n\n");
		}
		return server_socket;
}

void leerConfig(){

	consolaConfig = config_create("./configs/consola.config");
	IP_KERNEL = config_get_string_value(consolaConfig,"IP_KERNEL");
	PUERTO_KERNEL = config_get_string_value(consolaConfig,"PUERTO_KERNEL");
	printf("=== Archivo de configuracion ====\n IP Kernel: %s \n Puerto Kernel: %s \n\n", IP_KERNEL, PUERTO_KERNEL);
}

void parsearTareas(){
	printf("Preparando instrucciones...\n\n");
	log_info(logger,"Preparando instrucciones...");
	char* aux;
	size_t len = 0;
	listaTareas = list_create();

	fileTarea = fopen(rutaTareas,"r");

	while(!feof(fileTarea)){
		getline(&aux,&len,fileTarea);
		agregarInstruccion(aux);
	}
	fclose(fileTarea);

}

void enviarTareas(int server_socket){
	printf("Enviando instrucciones...\n\n");

	connection_packet* paquete = connection_packet_create_with_opCode(OP_LISTA_INSTRUCCIONES);

	connection_packet_add(paquete,&tamanioTareas,sizeof(int));

	int cantidadInstrucciones = list_size(listaTareas);

	//Agrego la cantidad de instrucciones del paquete
	connection_packet_add(paquete,&cantidadInstrucciones,sizeof(int));

	for(int i=0;i<cantidadInstrucciones;i++){

		Instruccion* instruccion = list_get(listaTareas,i);
		char* parametros = string_new();

		//Agrego el tamaño de la instruccion
		int tamanio = sizeof(int) * (2 + instruccion->cantidadParametros);
		connection_packet_add(paquete,&tamanio,sizeof(int));
		//Agrego datos de la intruccion
		connection_packet_add(paquete,&instruccion->id,sizeof(int));
		connection_packet_add(paquete,&instruccion->cantidadParametros,sizeof(int));

		for(int j=0 ;j< instruccion->cantidadParametros;j++){
			connection_packet_add(paquete,&instruccion->parametros[j],sizeof(int));

			char* param = string_from_format("    Parametro %d: %d\n",j+1,instruccion->parametros[j]);
			string_append(&parametros,param);
			free(param);
		}

		free(parametros);
	}

	log_info(logger,"Enviando instrucciones...");
	connection_packet_send(paquete,server_socket);
	connection_packet_destroy(paquete);



}


void esperarRespuesta(int server_socket){

	printf(PRINT_COLOR_YELLOW"Esperando confirmacion del Kernel...\n"PRINT_COLOR_RESET);
	log_info(logger,"Esperando confirmacion del Kernel...");
	int confirmacion = connection_recive_int(server_socket);

	if (confirmacion == -15 ){ //puse que el kernel devuelva -15 de momento
		printf(PRINT_COLOR_GREEN"Tareas recibidas con exito por Kernel\n"PRINT_COLOR_RESET);
		log_info(logger,"Tareas recibidas con exito por Kernel");
	}else{
		printf(PRINT_COLOR_RED"Error al esperar la confirmacion del kernel\n"PRINT_COLOR_RESET);
		log_info(logger,"Tareas recibidas con exito por Kernel");
	}
	connection_recive_int(server_socket);
	int confirmacionSalida = connection_recive_int(server_socket);

	// mensaje de finalizacion
	if (confirmacionSalida == OP_EXIT_PCB ){
		connection_recive_int(server_socket);
		printf(PRINT_COLOR_GREEN"FINALIZACION CON EXITO \n"PRINT_COLOR_RESET);
		log_info(logger,"finalizacion con exito");
	}else{
		printf(PRINT_COLOR_RED"Error al esperar la finalizacion \n "PRINT_COLOR_RESET);
		log_info(logger,"Error al esperar la finalizacion");
	}

}

void agregarInstruccion(char* instruccion){

	Instruccion * instruct= malloc(sizeof(Instruccion));

	char* parametro = strtok(instruccion," ");

	if( strcmp(parametro,"NO_OP") == 0 ){

		instruct->id = NO_OP;
		instruct->cantidadParametros=0;
		instruct->parametros = NULL;

		int veces = atoi(strtok(NULL," "));

		for (int i = 0; i<veces; i++){
			list_add(listaTareas,instruct);
		}

	}

	if( strcmp(parametro,"I/O") == 0 ){
		instruct->id = IO;
		instruct->cantidadParametros=1;
		instruct->parametros = malloc(sizeof(int));
		instruct->parametros[0]= atoi(strtok(NULL," "));
		list_add(listaTareas,instruct);

	}

	if( strcmp(parametro,"READ") == 0 ){
		instruct->id = READ;
		instruct->cantidadParametros=1;
		instruct->parametros = malloc(sizeof(int));
		instruct->parametros[0]= atoi(strtok(NULL," "));
		list_add(listaTareas,instruct);
	}

	if( strcmp(parametro,"COPY") == 0 ){
		instruct->id = COPY;
		instruct->cantidadParametros=2;
		instruct->parametros = malloc(sizeof(int)*2);
		instruct->parametros[0]= atoi(strtok(NULL," "));
		instruct->parametros[1]= atoi(strtok(NULL," "));
		list_add(listaTareas,instruct);
	}
	if( strcmp(parametro,"WRITE") == 0 ){
		instruct->id = WRITE;
		instruct->cantidadParametros=2;
		instruct->parametros = malloc(sizeof(int)*2);
		instruct->parametros[0]= atoi(strtok(NULL," "));
		instruct->parametros[1]= atoi(strtok(NULL," "));
		list_add(listaTareas,instruct);
	}
	if( strcmp(parametro,"EXIT") == 0){
		instruct->id = EXIT;
		instruct->cantidadParametros=0;
		instruct->parametros = NULL;
		list_add(listaTareas,instruct);
	}

}
