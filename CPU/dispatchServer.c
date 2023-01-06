/*
 * dispatchServer.c
 *
 *  Created on: 29 abr. 2022
 *      Author: utnso
 */


#include "dispatchServer.h"

void prepararDispatchServer(char* puerto){

	pthread_t ConsolaServer;
	pthread_create( &ConsolaServer , NULL ,  iniciarServerDispatch , (void*) puerto);

}

void* iniciarServerDispatch(void* puerto){
	printf("Iniciando servidor kernel Dispatch...\n");
	log_info(logger,"Iniciando servidor kernel Dispatch...");

	sockets_dispatch_sem = (sem_t*) malloc(sizeof(sem_t));
	sem_init(sockets_dispatch_sem,1,0);

	int server_socket = connection_start_server(NULL,(char*) puerto);

	if(server_socket<0){

		connection_server_error(server_socket);

	}else{

		printf(PRINT_COLOR_BLUE "Servidor kernel dispatch listo. Esperando conexion...\n" PRINT_COLOR_RESET);

		int status = connection_handler(server_socket,dispatchListenner,sockets_dispatch_sem);

		if(server_socket<0)
			connection_server_error(status);
	}
	return puerto;
}

void dispatchListenner(void* arg){
	int consolaSock = *(int*)arg;
	dispatchSock = consolaSock;

	sem_post(sockets_dispatch_sem); //libera la espera del server para estar seguro de que no se sobreescriba la variable arg
	printf(PRINT_COLOR_GREEN"Kernel conectado en socket %d \n"PRINT_COLOR_RESET,consolaSock);

	log_info(logger,"Kernel conectado en socket %d ",consolaSock);

	int operacion=0;
		int connectionStatus;

		connectionStatus = connection_recive_check(consolaSock);

		while(connectionStatus != -1){

			operacion = connection_recive_int(consolaSock);

			switch(operacion){

				case OP_NUEVO_PCB:
					pcbCPU = recibirPCB(consolaSock);
					printf("\n");
					rafagaCPU = 0;
					sem_post(sem_run);
					break;
			}

			connectionStatus = connection_recive_check(consolaSock);
		}

		printf(PRINT_COLOR_YELLOW"conexion perdida con Kernel\n"PRINT_COLOR_RESET);

		log_error(logger,"conexion perdida con Kernel");

		connection_close(consolaSock);
}

PCB* recibirPCB(int consolaSock){
	PCB* pcb = malloc(sizeof(PCB));

	pcb->estimacion_rafaga = connection_recive_int(consolaSock);
	pcb->id = connection_recive_int(consolaSock);
	pcb->tabla_paginas = connection_recive_int(consolaSock);

	int cantidadI = connection_recive_int(consolaSock);
	t_list* instrucciones = list_create();

	for(int i =0; i<cantidadI;i++){
		Instruccion* instruccion = malloc(sizeof(Instruccion));

		instruccion->id = connection_recive_int(consolaSock);
		instruccion->cantidadParametros = connection_recive_int(consolaSock);
		instruccion->parametros = malloc(sizeof(int) * instruccion->cantidadParametros );

		for (int j=0;j<instruccion->cantidadParametros;j++){
			instruccion->parametros[j] = connection_recive_int(consolaSock);
		}

		list_add(instrucciones,instruccion);
	}
	pcb->instruccioes = instrucciones;
	pcb->program_counter = connection_recive_int(consolaSock);
	pcb->tamanio = connection_recive_int(consolaSock);

	log_info(logger,"pcb %d recibido", pcb->id);

	return pcb;

}

void ejecutarProceso(){
	decodeFlag = 0;
	Instruccion* instruccion = fetch();
	int id = decode(instruccion);

	if(decodeFlag){
		fetchOperands();
	}

	execute(id);
	checkInterrupt();
}

Instruccion* fetch(){

	return list_get(pcbCPU->instruccioes,pcbCPU->program_counter);

}

int decode(Instruccion* instruccion){
	if(instruccion->id == COPY){
		decodeFlag = 1;
	}
	return instruccion->id;
}

void fetchOperands(){

	// buscar valor en memoria del segundo parametro del COPY

	Instruccion* i = list_get(pcbCPU->instruccioes,pcbCPU->program_counter);


	int valor = leerValor(i->parametros[1]);

	i->parametros[1]=valor; //todo no se si la idea era reemplazardlo

}

void execute(int id){

	Instruccion* i;
	direccionFisica* df;
	connection_packet* p;
	timeInit();


	switch(id){
	case NO_OP:
		usleep(RETARDO_NOOP * 1000); // quizas otra cosa que no sea usleep

		printf("PCB: %d ejecuto la instruccion" PRINT_COLOR_GREEN" NO_OP"PRINT_COLOR_RESET, pcbCPU->id);
		log_info(logger,"PCB: %d ejecuto la instruccion NO_OP", pcbCPU->id);

		processState = E_EXEC;
		break;

	case IO:
		// Se deberá devolver el PCB actualizado al Kernel junto al tiempo de bloqueo en milisegundos.
		printf("PCB: %d ejecuto la instruccion"PRINT_COLOR_BLUE " IO"PRINT_COLOR_RESET, pcbCPU->id);
		log_info(logger,"PCB: %d ejecuto la instruccion IO", pcbCPU->id);
		processState = E_BLOCKED;
		i = list_get(pcbCPU->instruccioes,pcbCPU->program_counter);
		timeBlock = i->parametros[0];
		break;

	case READ:
		// Se deberá leer el valor de memoria correspondiente a esa dirección lógica e imprimirlo por pantalla.

		i = list_get(pcbCPU->instruccioes,pcbCPU->program_counter);
		int valor = leerValor(i->parametros[0]);

		printf("PCB: %d ejecuto la instruccion READ:- Valor Leido: %d\n", pcbCPU->id,valor);
		log_info(logger,"PCB: %d ejecuto la instruccion READ: - Valor Leido: %d", pcbCPU->id,valor);

		processState = E_EXEC;
		break;
	case WRITE:
		// (dirección_lógica, valor): Se deberá escribir en memoria el valor del segundo parámetro en la dirección lógica del primer parámetro.

		i = list_get(pcbCPU->instruccioes,pcbCPU->program_counter);
		df = mmu( i->parametros[0]);
		p = connection_packet_create_with_opCode(500); //escribir
		connection_packet_add(p,&df->marco,sizeof(int));
		connection_packet_add(p,&df->desplazamiento,sizeof(int));
		connection_packet_add(p,&i->parametros[1],sizeof(int));
		connection_packet_send(p,memoriaSocket);
		connection_packet_destroy(p);
		free(df);
		connection_recive_int(memoriaSocket); //todo ver que hago con confirmacion

		printf("PCB: %d ejecuto la instruccion WRITE:- Valor enviado: %d \n", pcbCPU->id,i->parametros[1]);
		log_info(logger,"PCB: %d ejecuto la instruccion WRITE:- Valor enviado: %d", pcbCPU->id,i->parametros[1]);

		processState = E_EXEC;
		break;
	case COPY:
		// (dirección_lógica_destino, dirección_lógica_origen): Se deberá escribir en memoria el valor ubicado en la dirección
		// lógica pasada como segundo parámetro, en la dirección lógica pasada como primer parámetro.
		// A efectos de esta etapa, el accionar es similar a la instrucción WRITE ya que el valor a escribir ya se debería haber
		// obtenido en la etapa anterior.
		i = list_get(pcbCPU->instruccioes,pcbCPU->program_counter);
		df = mmu( i->parametros[0]);
		p = connection_packet_create_with_opCode(501); //copy igual hace exactamente lo mismo que write
		connection_packet_add(p,&df->marco,sizeof(int));
		connection_packet_add(p,&df->desplazamiento,sizeof(int));
		connection_packet_add(p,&i->parametros[1],sizeof(int));
		connection_packet_send(p,memoriaSocket);
		connection_packet_destroy(p);
		connection_recive_int(memoriaSocket); //todo ver que hago con confirmacion

		printf("PCB:%d ejecuto la instruccion COPY:- Valor copiado: %d\n", pcbCPU->id, i->parametros[0]);
		log_info(logger,"PCB:%d ejecuto la instruccion COPY:- Valor copiado: %d", pcbCPU->id, i->parametros[0]);

		processState = E_EXEC;
		break;

	case EXIT:
		processState = E_EXIT;
		printf("PCB: %d ejecuto la instruccion"PRINT_COLOR_RED " EXIT."PRINT_COLOR_RESET, pcbCPU->id);
		log_info(logger,"PCB: %d ejecuto la instruccion EXIT.", pcbCPU->id);
		break;
	}

	rafagaCPU += getTime();

	(pcbCPU->program_counter)++;

}

void checkInterrupt(){
	if (interrupt){
		printf(PRINT_COLOR_YELLOW"\nCPU: Interrupcion detectada, desalojando proceso...\n" PRINT_COLOR_RESET);
		log_info(logger,"Interrupcion detectada, desalojando proceso...");
		//hay una interrupcion
		//enviar PCB
		enviarPCB(pcbCPU,dispatchSock);
		interrupt=0;
	}else{
		if(processState != E_EXIT && processState != E_BLOCKED){
			sem_post(sem_run);
		}else{
			//mandar a kernel
			enviarPCB(pcbCPU,dispatchSock);
		}
	}
	// chequear interrupcion
}

void enviarPCB(PCB* pcb, int moduloSocket){ // solo copie y pegue meter en TODO utils
	printf(MMU_PRINT_COLOR"MMU: cantidad de TLBmiss del proceso %d: %d\n" PRINT_COLOR_RESET,pcb->id, tlbmiss );
	log_info(loggerMMU,"Cantidad de TLBmiss el proceso %d: %d",pcb->id,tlbmiss);
	tlbmiss=0;
	int op = OP_NUEVO_PCB;
	connection_packet* p = connection_packet_create();

	connection_packet_add(p,&op,sizeof(uint32_t));
	connection_packet_add(p,&pcb->id,sizeof(uint32_t));
	connection_packet_add(p,&pcb->program_counter,sizeof(uint32_t));
	connection_packet_add(p,&processState,sizeof(uint32_t));
	connection_packet_add(p,&timeBlock,sizeof(uint32_t));
	connection_packet_add(p,&rafagaCPU,sizeof(int));

	connection_packet_send(p,moduloSocket);
	connection_packet_destroy(p);
	log_info(logger,"Enviando proceso %d a kernel",pcb->id);
	void delete(void* e){
		Instruccion * i = e;

			free(i->parametros);

		free(i);
	}
	list_destroy_and_destroy_elements(pcb->instruccioes,delete);
	free(pcb);

	cleanCache();

}




int getMarco(int numeroPagina){
	traduccion* t;

	for(int i =0; i<list_size(cache);i++){
		t = list_get(cache,i);
		if( t->pagina==numeroPagina){
			t->instanteUltimaReferencia = rafagaCPU; //todo no estoy seguro si va aca
			printf(MMU_PRINT_COLOR "MMU LECTURA: (pagina,marco): (%d,%d) a TLB cache - (carga,Ultimareferencia): (%d,%d)" PRINT_COLOR_RESET,t->pagina,t->marco,t->instanteCarga,t->instanteUltimaReferencia);
			log_info(loggerMMU,"LECTURA: (pagina,marco): (%d,%d) a TLB cache - (carga,Ultimareferencia): (%d,%d)",t->pagina,t->marco,t->instanteCarga,t->instanteUltimaReferencia);
			return t->marco;
		}
	}

	return -1;
}

void addMarco(int marco,int pagina){
	traduccion * ntlb= malloc(sizeof(traduccion));
	ntlb->pagina = pagina;
	ntlb->marco = marco;
	ntlb->instanteCarga=rafagaCPU; //todo
	ntlb->instanteUltimaReferencia=rafagaCPU; //todo
	if(list_size(cache) >= ENTRADAS_TLB){//if hay que reemplazar
		int removeIndex;
		if(strcmp(REEMPLAZO_TLB,"FIFO") == 0){
			removeIndex = victimaFIFO();
		}else{
			removeIndex = victimaLRU();
		}

		traduccion* rtlb = list_remove(cache,removeIndex);

		printf(MMU_PRINT_COLOR "MMU: Removiendo (pagina,marco): (%d,%d) de TLB cache..."PRINT_COLOR_RESET,rtlb->pagina,rtlb->marco);
		log_info(loggerMMU,"Removiendo (pagina,marco): (%d,%d) de TLB cache...",rtlb->pagina,rtlb->marco);
		free(rtlb);
	}
	printf(MMU_PRINT_COLOR "MMU: Añadiendo (pagina,marco): (%d,%d) a TLB cache - (carga,Ultimareferencia): (%d,%d)" PRINT_COLOR_RESET,ntlb->pagina,ntlb->marco,ntlb->instanteCarga,ntlb->instanteUltimaReferencia);
	log_info(loggerMMU,"Añadiendo (pagina,marco): (%d,%d) a TLB cache, \n\t(carga,Ultimareferencia): (%d,%d) ...",ntlb->pagina,ntlb->marco,ntlb->instanteCarga,ntlb->instanteUltimaReferencia);
	list_add(cache,ntlb);

}

int leerValor(int direccionLogica){
	log_info(loggerMMU,"Leyendo valor de memoria ");
	printf(MMU_PRINT_COLOR "\nMMU: Leyendo valor de memoria"PRINT_COLOR_RESET);

	direccionFisica* dir = mmu(direccionLogica);
	connection_packet* p = connection_packet_create_with_opCode(458);//acceso a memoria

	connection_packet_add(p,&dir->marco,sizeof(uint32_t));
	connection_packet_add(p,&dir->desplazamiento,sizeof(uint32_t));
	connection_packet_send(p,memoriaSocket);
	connection_packet_destroy(p);
	free(dir);
	log_info(loggerMMU,"Esperando valor leido de memoria...");
	printf(MMU_PRINT_COLOR "MMU: Esperando valor leido de memoria..."PRINT_COLOR_RESET);
	int valor = connection_recive_int(memoriaSocket);

	log_info(loggerMMU,"Valor leido de memoria: %d ",valor);
	printf(MMU_PRINT_COLOR "MMU: valor leido de memoria: %d \n"PRINT_COLOR_RESET,valor);

	return valor;
}

int victimaFIFO(){

	int minimo;
	int indice;

	for (int i=0 ; i<list_size(cache);i++){
		traduccion * t = list_get(cache,i);

		if( i==0 ||  t->instanteCarga<minimo){
			minimo = t->instanteCarga;
			indice= i;
		}

	}

	return indice;
}

int victimaLRU(){

	int minimo;
	int indice;

	for (int i=0 ; i<list_size(cache);i++){
		traduccion * t = list_get(cache,i);

		if( i==0 ||  t->instanteUltimaReferencia<minimo){
			minimo = t->instanteUltimaReferencia;
			indice= i;
		}

	}

	return indice;
}

void cleanCache(){

	void deleteTlb(void* t){
		traduccion * tlb = t;
		free(tlb);
	}

	list_clean_and_destroy_elements(cache,&deleteTlb);
}
