/*
 * kernel.c
 *
 *  Created on: 23 abr. 2022
 *      Author: utnso
 */


#include "kernel.h"

void verProcesos(){
	for (int i=0; i<list_size(procesos);i++){
		Proceso* p = list_get(procesos,i);

		printf("         Procesos: %d, estado:%d \n",p->pcb->id,p->estado);
	}

}

int main(void){

	printf(PRINT_COLOR_BLUE"Iniciando modulo Kernel... \n\n"PRINT_COLOR_RESET);

	iniciarVariables();
	mkdir("./logs",0700);
	logger = log_create("./logs/Kernel.log", "Kernel", 0, LOG_LEVEL_DEBUG);

	leerConfig();

	memoriaSocket = conectarCon("Memoria",IP_MEMORIA,PUERTO_MEMORIA);

	cpuDSocket = conectarCon("CPU Dispatch",IP_CPU,PUERTO_CPU_DISPATCH);

	cpuISocket = conectarCon("CPU Interrupt",IP_CPU,PUERTO_CPU_INTERRUPT);

	prepararConsolaServer(PUERTO_ESCUCHA);

	if (cpuDSocket>0){
		correrDispatcher();
		//correrBloqueado();
	}
	/*
	if(memoriaSocket>0){
		correrMemoriaKernel();
	}
	 */



	pthread_t t_runplani;
	pthread_create( &t_runplani , NULL ,  planificar , NULL);


	char* rta = readline("Presione enter para salir\n");
	free(rta);

	liberarVariablies();
	return 1;

}

void* planificar(){

	while(1){
		sem_wait(semMutexPlanificar);
		//largo plazo
		while((list_size(procesosReady) + list_size(procesosExec) + procesosBloqueados()) < GRADOMP &&
				(list_size(procesosNew) > 0 || list_size(procesosSuspendedReady) > 0) ){
			Proceso* aux;
			if(list_size(procesosSuspendedReady) > 0){
				aux = list_get(procesosSuspendedReady,0);
				mover(aux, procesosSuspendedReady, procesosReady,E_READY);
				avisarSuspencion(aux,OP_PROCESO_DESUSPENDIDO);
				printf("Kernel: Proceso %d desuspendido\n",aux->pcb->id);
				log_info(logger,"Proceso %d desuspendido. Movido de Suspended-Ready a Ready",aux->pcb->id);

			}else{

				aux = list_get(procesosNew,0);

				enviarPCB(aux->pcb, memoriaSocket);
				printf(PRINT_COLOR_YELLOW"Kernel: Proceso %d esperando indice de tabla de paginas nivel 1..."PRINT_COLOR_RESET,aux->pcb->id);
				log_info(logger,"Proceso %d esperando indice de tabla de paginas nivel 1...",aux->pcb->id);
				aux->pcb->tabla_paginas = connection_recive_int(memoriaSocket);
				mover(aux, procesosNew, procesosReady,E_READY);
				printf(PRINT_COLOR_GREEN"Kernel:Tabla de pagina para procesos %d recibida -  Valor: %d"PRINT_COLOR_RESET,aux->pcb->id, aux->pcb->tabla_paginas);
				log_info(logger,"Tabla de pagina para procesos %d recibida -  Valor: %d\n",aux->pcb->id, aux->pcb->tabla_paginas);
			}
			if(ALGORITMO == SRT && (list_size(procesosExec) > 0)){
				ready=1;
			}

		}

		if( ready && ALGORITMO == SRT && (list_size(procesosExec) > 0)){
					ready=0;
					printf(PRINT_COLOR_YELLOW"Kernel:Enviando interrupcion..."PRINT_COLOR_RESET);
					log_info(logger,"Enviando interrupcion...\n");
					connection_packet* p = connection_packet_create_with_opCode(OP_INTERRUPT);
					connection_packet_send(p,cpuISocket);
					connection_packet_destroy(p);
					interrupt=1;
					sem_wait(sem_interrupt);
				}

		// corto plazo
		while(list_size(procesosReady) > 0 && list_size(procesosExec) == 0){
			Proceso* aux = list_get(procesosReady,0);

			if(ALGORITMO){ //ALGORITMO ES FIFO = 0 = FALSE || SRT = 1 = TRUE
				aux = calcularSJF(procesosReady);
			}

			mover(aux, procesosReady, procesosExec,E_EXEC);

			printf("Kernel:Enviando a CPU Proceso: %d\n",aux->pcb->id);
			log_info(logger,"Enviando a CPU Proceso: %d \n  ",aux->pcb->id);
			enviarPCB(aux->pcb, cpuDSocket);

		}

	}

}


void mover(Proceso* proceso, t_list* listaOrigen, t_list* listaDestino, int estado){
	int indice = buscarProceso(proceso, listaOrigen);
	Proceso* aux = list_remove(listaOrigen,indice);
	aux->estado = estado;
	list_add(listaDestino, aux);
}

int buscarProceso(Proceso* proceso, t_list* origen){
	Proceso* aux;

	for(int i = 0; i<list_size(origen); i++){
		aux = list_get(origen, i);
		if(proceso->pcb->id == aux->pcb->id){
			return i;
		}
	}
	return -1;

}

Proceso* calcularSJF(t_list* listaOrigen){ // EST = RAFAGA ANTERIOR * ALPHA + EST ANTERIOR * (1-ALPHA)

	Proceso* run ;
	int estimacionCorta;

	Proceso* aux;
	int estimacion;

	//int estimacion = tiempoEjecucion * ALFA + run->pcb->estimacion_rafaga * (1-ALFA);

	for( int i = 0; i <list_size(listaOrigen); i++){
		aux = list_get(listaOrigen,i);

		int tiempoEjecucion = aux->tExec;

		if(tiempoEjecucion == 0){
			estimacion = aux->pcb->estimacion_rafaga;
		}else{

			if(aux->estado==75){
				estimacion = aux->pcb->estimacion_rafaga - tiempoEjecucion;
			}else{
				estimacion = tiempoEjecucion * ALFA + aux->pcb->estimacion_rafaga * (1-ALFA);
			}

		}

		//printf("\n\npcb %d (%d,%d,%d) \n \n",aux->pcb->id,aux->tExec,aux->pcb->estimacion_rafaga,estimacion);

		if( i == 0 ||estimacion < estimacionCorta){
			run = aux;
			estimacionCorta = estimacion;
		}
	}
	run->pcb->estimacion_rafaga = estimacion;
	return run;
}

void enviarPCB(PCB* pcb, int moduloSocket){ // modifique para que pueda mandarle al modulo correspondiente
	connection_packet* p = connection_packet_create_with_opCode(OP_NUEVO_PCB);

	connection_packet_add(p,&pcb->estimacion_rafaga,sizeof(uint32_t));
	connection_packet_add(p,&pcb->id,sizeof(uint32_t));
	if(moduloSocket == cpuDSocket){
		connection_packet_add(p,&pcb->tabla_paginas,sizeof(uint32_t));
	}


	int cantidadI = list_size(pcb->instruccioes);
	connection_packet_add(p,&cantidadI,sizeof(int));//agrego la cantidad de instrucciones

	for(int i =0; i<cantidadI;i++){
		Instruccion* instruccion = list_get(pcb->instruccioes,i);
		connection_packet_add(p,&instruccion->id,sizeof(uint32_t));
		connection_packet_add(p,&instruccion->cantidadParametros,sizeof(int));

		for (int j=0;j<instruccion->cantidadParametros;j++){
			connection_packet_add(p,&instruccion->parametros[j],sizeof(int));
		}

	}

	connection_packet_add(p,&pcb->program_counter,sizeof(uint32_t));
	connection_packet_add(p,&pcb->tamanio,sizeof(uint32_t));

	connection_packet_send(p,moduloSocket);
	connection_packet_destroy(p);

	//TODO esperar respuesta de memoria y tabla de paginas
}

void iniciarVariables(){
	gettimeofday(&startblock,NULL);
	interrupt=0;
	ready=0;
	procesos = list_create();
	procesosNew = list_create();
	procesosReady = list_create();
	procesosExec = list_create();
	procesosExit = list_create();
	procesosBlock = list_create();
	procesosSuspendedReady = list_create();
	semMutexPlanificar = (sem_t*)malloc(sizeof(sem_t));
	sem_init(semMutexPlanificar,1,0);
	sem_interrupt = (sem_t*)malloc(sizeof(sem_t));
	sem_init(sem_interrupt,1,0);
	sem_bloqueado = (sem_t*)malloc(sizeof(sem_t));
	sem_init(sem_bloqueado,1,0);
	sem_io = (sem_t*)malloc(sizeof(sem_t));
	sem_init(sem_io,1,1);


}

void leerConfig(){

	kernelConfig = config_create("./configs/kernel.config");

	IP_MEMORIA = config_get_string_value(kernelConfig,"IP_MEMORIA");
	PUERTO_MEMORIA = config_get_string_value(kernelConfig,"PUERTO_MEMORIA");
	IP_CPU = config_get_string_value(kernelConfig,"IP_CPU");
	PUERTO_CPU_DISPATCH = config_get_string_value(kernelConfig,"PUERTO_CPU_DISPATCH");
	PUERTO_CPU_INTERRUPT = config_get_string_value(kernelConfig,"PUERTO_CPU_INTERRUPT");
	PUERTO_ESCUCHA = config_get_string_value(kernelConfig,"PUERTO_ESCUCHA");
	ALGORITMO = (strcmp("FIFO",config_get_string_value(kernelConfig,"ALGORITMO_PLANIFICACION")) == 0? FIFO:SRT );
	ESTIMACION_INICIAL = config_get_int_value(kernelConfig,"ESTIMACION_INICIAL");

	ALFA = atof(config_get_string_value(kernelConfig,"ALFA"));

	GRADOMP = config_get_int_value(kernelConfig,"GRADO_MULTIPROGRAMACION");
	TMAXBLOQUEO = config_get_int_value(kernelConfig,"TIEMPO_MAXIMO_BLOQUEADO");

	printf("=== Archivo de configuracion ===\n"PRINT_COLOR_YELLOW
			" IP MEMORIA: %s\n"
			" PUERTO MEMORIA: %s\n"
			" IP CPU: %s\n"
			" PUERTO CPU DISPATCH: %s\n"
			" PUERTO CPU INTERRUPT: %s\n"
			" PUERTO CPU ESCUCHA: %s\n"
			" ALGORITMO: %s\n"
			" ESTIMACION INICIAL: %d\n"
			" ALFA: %f\n"
			" GRADO MULTIPROGRAMACION: %d\n"
			" TIEMPO MAXIMO BLOQUEO: %d\n\n"PRINT_COLOR_RESET,
			IP_MEMORIA,PUERTO_MEMORIA,IP_CPU,PUERTO_CPU_DISPATCH,PUERTO_CPU_INTERRUPT,PUERTO_ESCUCHA,
			(ALGORITMO == FIFO? "FIFO":"SRT"),
			ESTIMACION_INICIAL,ALFA,GRADOMP,TMAXBLOQUEO);


}

int conectarCon(char* nombre,char* ip,char* puerto){
	printf(PRINT_COLOR_BLUE "Conectando con mododulo %s..."PRINT_COLOR_RESET "\n",nombre );

	int server_socket = connection_connect(ip,puerto);

	if(server_socket >0){
		printf(PRINT_COLOR_GREEN "conexion con %s aceptada"PRINT_COLOR_RESET "\n\n",nombre );
		log_info(logger, "Conectado con modulo %s ",nombre);

	}else{
		printf(PRINT_COLOR_RED"Conexion con %s rechazada" PRINT_COLOR_RESET "\n\n",nombre );
		log_warning(logger, "Conexion  con modulo %s rechazada",nombre);
	}

	return server_socket;
}

void correrBloqueado(){
	pthread_t bloqueadoThread;
	pthread_create( &bloqueadoThread , NULL ,  bloqueado , NULL);
}

void* bloqueado(){ //pĺani mediano plazo
	int estado=1;

	while(1){
		sem_wait(sem_bloqueado);
		Proceso* proceso = list_get(procesosBlock,0);
		Proceso* proceso2;
		int tiempo = getTime();



		if(((tiempo - proceso->ioInit) >= proceso->ioTime) && proceso->estado== E_BLOCKED ){
			mover(proceso,procesosBlock,procesosReady,E_READY);
			if(list_size(procesosBlock)>0){
				proceso2 = list_get(procesosBlock,0);
				proceso2->ioInit = tiempo;
			}
			printf(PRINT_COLOR_GREEN"Kernel: Proceso %d desbloqueado, paso a ready"PRINT_COLOR_RESET,proceso->pcb->id);
			log_info(logger,"Proceso %d desbloqueado, paso a ready\n",proceso->pcb->id);
			//verProcesos();
			ready=1;
			sem_post(semMutexPlanificar);
		}
		if(((tiempo - proceso->ioInit) >= proceso->ioTime) && proceso->estado== E_SUSPENDED_BLOCKED ){

			mover(proceso,procesosBlock,procesosSuspendedReady,E_SUSPENDEDREADY);
			if(list_size(procesosBlock)>0){
				proceso2 = list_get(procesosBlock,0);
				proceso2->ioInit = tiempo;
			}
			printf(PRINT_COLOR_BLUE"Kernel: El proceso %d, paso a Suspended-ready"PRINT_COLOR_RESET,proceso->pcb->id);
			log_info(logger,"El proceso %d, paso a Suspended-ready\n",proceso->pcb->id);
			sem_post(semMutexPlanificar);
		}

		for (int i = 0 ; i<list_size(procesosBlock);i++ ){
			Proceso* proceso = list_get(procesosBlock,i);


			if(proceso->estado == E_BLOCKED && (tiempo - proceso->lockInit) >= (TMAXBLOQUEO)){
				printf(PRINT_COLOR_YELLOW"Kernel:El proceso %d - supero el tiempo de bloqueado pasa a suspendido"PRINT_COLOR_RESET,proceso->pcb->id);
				log_info(logger,"El proceso %d - supero el tiempo de bloqueado pasa a suspendido\n",proceso->pcb->id);
				avisarSuspencion(proceso,OP_PROCESO_SUSPENDIDO);

				proceso->estado = E_SUSPENDED_BLOCKED;
				sem_post(semMutexPlanificar);
			}
		}

		if (list_size(procesosBlock)>0){
			sem_post(sem_bloqueado);
		}

		usleep(1000);
	}

	return (void*) estado;

}

int procesosBloqueados(){
	int procesosBloqueados=0;

	for (int i = 0 ; i<list_size(procesosBlock);i++ ){
		Proceso* proceso = list_get(procesosBlock,i);

		if(proceso->estado == E_BLOCKED){
			procesosBloqueados ++;
		}

	}
	return procesosBloqueados;
}

void avisarSuspencion(Proceso* proceso,int estado){

	connection_packet* p = connection_packet_create_with_opCode(estado);
	connection_packet_add(p,&proceso->pcb->id,sizeof(uint32_t));
	connection_packet_send(p,memoriaSocket);
	connection_packet_destroy(p);
	connection_recive_int(memoriaSocket);
}

char* verProceso(Proceso* p){
	char* descripcion= string_new();
	string_append(&descripcion, string_from_format("\n================Proceso================\nID Proceso: %d\nTamaño: %d\nProgram Counter: %d\n\n==Instrucciones==\n\n",p->pcb->id,p->pcb->tamanio,p->pcb->program_counter));

	for(int i = 0 ; i< list_size(p->pcb->instruccioes); i++){

		Instruccion* ins =list_get(p->pcb->instruccioes,i);

		string_append(&descripcion, string_from_format("          ID: %d\n          Nombre: %s\n",ins->id,getname(ins->id)) );
		if(ins->cantidadParametros>0)
			string_append(&descripcion,"          Parametros:\n");

		for(int j=0; j<ins->cantidadParametros;j++){
			string_append(&descripcion,string_from_format("               Parametro %d:%d\n",j+1,ins->parametros[j]));
		}
	string_append(&descripcion,"\n----------\n\n");
	}
	string_append_with_format(&descripcion,"=======================================\n\n\n");
	return descripcion;
}

char* getname(int n){

	switch(n){
		case NO_OP:
			return "NOOP";
			break;
		case IO:
			return "IO";
			break;
		case READ:
			return "READ";
			break;
		case COPY:
			return "COPY";
			break;
		case WRITE:
			return "WRITE";
			break;
		case EXIT:
			return "EXIT";
			break;
	}
	return "DESCONOCIDO";
}

int getTime(){
	gettimeofday(&stopblock,NULL);
	return ((stopblock.tv_sec - startblock.tv_sec) * 1000000 + stopblock.tv_usec - startblock.tv_usec) / 1000;
}

void liberarVariablies(){


	void deleteProcess(void* e){
		Proceso * p = e;

		void delete(void* e2){
			Instruccion * i = e2;
			free(i->parametros);
			free(i);
		}

		list_destroy_and_destroy_elements(p->pcb->instruccioes,delete);

		free(p->pcb);
		free(p);
	}

	list_destroy(procesosNew);
	list_destroy(procesosReady);
	list_destroy(procesosExec);
	list_destroy(procesosExit);
	list_destroy(procesosBlock);
	list_destroy(procesosSuspendedReady);

	list_destroy_and_destroy_elements(procesos,deleteProcess);

	sem_destroy(sem_interrupt);
	sem_destroy(semMutexPlanificar);
	sem_destroy(sem_bloqueado);
	sem_destroy(sem_io);

	log_destroy(logger);
	config_destroy(kernelConfig);
}

void bloquearProceso(Proceso* p){
	pthread_t block;
	pthread_create( &block , NULL ,  bloqueoP ,  p);
}

void* bloqueoP(void* arg){
	Proceso* p = arg;
	sem_wait(sem_io);
	int tiempoBloqueado = getTime() - p->lockInit;
	p->lockInit =  getTime();
	if( p->estado == E_SUSPENDED_BLOCKED ){

		while(getTimeToIOEnd(p)> getTimeToSus(list_get(procesosBlock, minTSusp()) )){
			usleep(  getTimeToSus(list_get(procesosBlock, minTSusp())) * 1000);
			suspenderProcesos();
		}
		usleep(getTimeToIOEnd(p)*1000);
		//suspended -> suspendjed- Ready

		mover(p,procesosBlock,procesosSuspendedReady,E_SUSPENDEDREADY);
		printf(PRINT_COLOR_BLUE"Kernel: El proceso %d, paso a Suspended-ready"PRINT_COLOR_RESET,p->pcb->id);
		log_info(logger,"El proceso %d, paso a Suspended-ready\n",p->pcb->id);
		suspenderProcesos();

		sem_post(semMutexPlanificar);
		sem_post(sem_io);

	}else{
		int tLockTotal = p->ioTime + tiempoBloqueado;

		if(tLockTotal > TMAXBLOQUEO){
			int tToSusp = TMAXBLOQUEO - tiempoBloqueado;
			usleep( tToSusp * 1000);
			//lock - > suspended
			suspenderProcesos();
			printf("-------- IO TIME %d \n",getTimeToIOEnd(p));
			while(getTimeToIOEnd(p) > getTimeToSus(list_get(procesosBlock, minTSusp()) )){
				printf("-------- %d \n",getTimeToSus(list_get(procesosBlock, minTSusp())));
				usleep(  getTimeToSus(list_get(procesosBlock, minTSusp())) * 1000);
				suspenderProcesos();
			}
			usleep(getTimeToIOEnd(p)*1000);

			//suspended -> ReadySuspended
			mover(p,procesosBlock,procesosSuspendedReady,E_SUSPENDEDREADY);
			printf(PRINT_COLOR_BLUE"Kernel: El proceso %d, paso a Suspended-ready"PRINT_COLOR_RESET,p->pcb->id);
			log_info(logger,"El proceso %d, paso a Suspended-ready\n",p->pcb->id);
			suspenderProcesos();

			sem_post(semMutexPlanificar);
			sem_post(sem_io);
		}else{
			usleep(p->ioTime * 1000);
			//lock -> Ready

			mover(p,procesosBlock,procesosReady,E_READY);
			printf(PRINT_COLOR_GREEN"Kernel: Proceso %d desbloqueado, paso a ready"PRINT_COLOR_RESET,p->pcb->id);
			log_info(logger,"Proceso %d desbloqueado, paso a ready\n",p->pcb->id);

			ready=1;
			suspenderProcesos();

			sem_post(semMutexPlanificar);
			sem_post(sem_io);
		}

	}


	return p;
}

void suspenderProcesos(){

	for (int i =0; i<list_size(procesosBlock) ;i++){
		Proceso* p = list_get(procesosBlock,i);
		int tiempoBloqueado = getTime() - p->lockInit;

		if(p->estado == E_BLOCKED &&  tiempoBloqueado >= TMAXBLOQUEO ){
			//lock - > suspended
			printf(PRINT_COLOR_YELLOW"Kernel:El proceso %d - supero el tiempo de bloqueado pasa a suspendido"PRINT_COLOR_RESET,p->pcb->id);
			log_info(logger,"El proceso %d - supero el tiempo de bloqueado pasa a suspendido\n",p->pcb->id);
			avisarSuspencion(p,OP_PROCESO_SUSPENDIDO);
			p->estado = E_SUSPENDED_BLOCKED;

			sem_post(semMutexPlanificar);

		}
	}
}

int getTimeToSus(Proceso*p){
	return TMAXBLOQUEO - (getTime() - p->lockInit) ;
}

int getTimeToIOEnd(Proceso* p){


	int reaming = p->ioTime - (getTime() - p->ioInit);
	if(reaming <0){
		return 0;
	}
	return reaming;
}

int minTSusp(){
	Proceso* min;
	int tmin;
	int index=0;

	for(int i=0;i<list_size(procesosBlock) ; i++){
		min = list_get(procesosBlock,i);

		if( i== 0 || (tmin>getTimeToSus(min) && min->estado == E_BLOCKED )){
			index = i;
			tmin = getTimeToSus(min);
		}
	}

	return index;
}

