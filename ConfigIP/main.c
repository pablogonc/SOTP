/*
 * main.c
 *
 *  Created on: 20 jul. 2022
 *      Author: utnso
 */
#include <commons/config.h>
#include <string.h>
#include <stdio.h>

t_config* ipConfig;
t_config* consolaConfig;
t_config* CPUConfig;
t_config* kernelConfig;
t_config* memoriaConfig;

int main(int argc, char** argv){

	int flag =1;
	//argv[1]=TEST
	//argv[2]=Algoritmo Kernel
	//argv[3]=Algoritmo TLB
	//argv[4]=Algoritmo Memoria
	ipConfig = config_create("../pre-req/IPs.config");

	//Conseguir IPs
	char *ipKernel = config_get_string_value(ipConfig,"IP_KERNEL");
	char *ipCpu = config_get_string_value(ipConfig,"IP_CPU");
	char *ipMemoria = config_get_string_value(ipConfig,"IP_MEMORIA");

	//crear configs
	consolaConfig = config_create("./configs/consola.config");
	CPUConfig = config_create("./configs/CPU.config");
	kernelConfig = config_create("./configs/kernel.config");
	memoriaConfig = config_create("./configs/memoria.config");

	//Setear IPs
	config_set_value(consolaConfig,"IP_KERNEL",ipKernel);
	config_set_value(kernelConfig,"IP_CPU",ipCpu);
	config_set_value(kernelConfig,"IP_MEMORIA",ipMemoria);
	config_set_value(CPUConfig,"IP_MEMORIA",ipMemoria);

	//setear Algoritmos
	config_set_value(kernelConfig,"ALGORITMO_PLANIFICACION",argv[2]);
	config_set_value(CPUConfig,"REEMPLAZO_TLB",argv[3]);
	config_set_value(memoriaConfig,"ALGORITMO_REEMPLAZO",argv[4]);

	//setear pruebas
	if( strcmp(argv[1],"prueba-Base") == 0 ){
		//Kernel
		config_set_value(kernelConfig,"ESTIMACION_INICIAL","10000");
		config_set_value(kernelConfig,"GRADO_MULTIPROGRAMACION","4");
		config_set_value(kernelConfig,"ALFA","0.5");
		config_set_value(kernelConfig,"TIEMPO_MAXIMO_BLOQUEADO","100000");
		//CPU
		config_set_value(CPUConfig,"RETARDO_NOOP","1000");
		config_set_value(CPUConfig,"ENTRADAS_TLB","4");
		//MEMORIA
		config_set_value(memoriaConfig,"TAM_MEMORIA","4096");
		config_set_value(memoriaConfig,"TAM_PAGINA","64");
		config_set_value(memoriaConfig,"RETARDO_MEMORIA","1000");
		config_set_value(memoriaConfig,"MARCOS_POR_PROCESO","4");
		config_set_value(memoriaConfig,"RETARDO_SWAP","2000");

		flag=0;
	}
	if( strcmp(argv[1],"kernel-plani") == 0 ){
		//Kernel
		config_set_value(kernelConfig,"ESTIMACION_INICIAL","20000");
		config_set_value(kernelConfig,"GRADO_MULTIPROGRAMACION","4");
		config_set_value(kernelConfig,"ALFA","0.5");
		config_set_value(kernelConfig,"TIEMPO_MAXIMO_BLOQUEADO","10000");
		//CPU
		config_set_value(CPUConfig,"RETARDO_NOOP","1000");
		config_set_value(CPUConfig,"ENTRADAS_TLB","4");
		//MEMORIA
		config_set_value(memoriaConfig,"TAM_MEMORIA","4096");
		config_set_value(memoriaConfig,"TAM_PAGINA","64");
		config_set_value(memoriaConfig,"RETARDO_MEMORIA","1000");
		config_set_value(memoriaConfig,"MARCOS_POR_PROCESO","4");
		config_set_value(memoriaConfig,"RETARDO_SWAP","2000");

		flag=0;
		}
	if( strcmp(argv[1],"kernel-susp") == 0 ){
		//Kernel
		config_set_value(kernelConfig,"ESTIMACION_INICIAL","20000");
		config_set_value(kernelConfig,"GRADO_MULTIPROGRAMACION","2");
		config_set_value(kernelConfig,"ALFA","0.5");
		config_set_value(kernelConfig,"TIEMPO_MAXIMO_BLOQUEADO","8000");
		//CPU
		config_set_value(CPUConfig,"RETARDO_NOOP","1000");
		config_set_value(CPUConfig,"ENTRADAS_TLB","4");
		//MEMORIA
		config_set_value(memoriaConfig,"TAM_MEMORIA","4096");
		config_set_value(memoriaConfig,"TAM_PAGINA","64");
		config_set_value(memoriaConfig,"RETARDO_MEMORIA","1000");
		config_set_value(memoriaConfig,"MARCOS_POR_PROCESO","4");
		config_set_value(memoriaConfig,"RETARDO_SWAP","2000");

		flag=0;
		}

	if( strcmp(argv[1],"memoria") == 0 ){
		//Kernel
		config_set_value(kernelConfig,"ESTIMACION_INICIAL","20000");
		config_set_value(kernelConfig,"GRADO_MULTIPROGRAMACION","2");
		config_set_value(kernelConfig,"ALFA","0.5");
		config_set_value(kernelConfig,"TIEMPO_MAXIMO_BLOQUEADO","5000");
		//CPU
		config_set_value(CPUConfig,"RETARDO_NOOP","1000");
		config_set_value(CPUConfig,"ENTRADAS_TLB","1");
		//MEMORIA
		config_set_value(memoriaConfig,"TAM_MEMORIA","2048");
		config_set_value(memoriaConfig,"TAM_PAGINA","256");
		config_set_value(memoriaConfig,"RETARDO_MEMORIA","1000");
		config_set_value(memoriaConfig,"MARCOS_POR_PROCESO","3");
		config_set_value(memoriaConfig,"RETARDO_SWAP","5000");

		flag=0;
		}
	if( strcmp(argv[1],"tlb") == 0 ){
		//Kernel
		config_set_value(kernelConfig,"ESTIMACION_INICIAL","20000");
		config_set_value(kernelConfig,"GRADO_MULTIPROGRAMACION","2");
		config_set_value(kernelConfig,"ALFA","0.5");
		config_set_value(kernelConfig,"TIEMPO_MAXIMO_BLOQUEADO","5000");
		//CPU
		config_set_value(CPUConfig,"RETARDO_NOOP","1000");
		config_set_value(CPUConfig,"ENTRADAS_TLB","4");
		//MEMORIA
		config_set_value(memoriaConfig,"TAM_MEMORIA","8192");
		config_set_value(memoriaConfig,"TAM_PAGINA","128");
		config_set_value(memoriaConfig,"RETARDO_MEMORIA","3000");
		config_set_value(memoriaConfig,"MARCOS_POR_PROCESO","16");
		config_set_value(memoriaConfig,"RETARDO_SWAP","3000");

		flag=0;
		}
	if( strcmp(argv[1],"estabilidad") == 0 ){
		//Kernel
		config_set_value(kernelConfig,"ESTIMACION_INICIAL","50000");
		config_set_value(kernelConfig,"GRADO_MULTIPROGRAMACION","6");
		config_set_value(kernelConfig,"ALFA","0.2");
		config_set_value(kernelConfig,"TIEMPO_MAXIMO_BLOQUEADO","1000");
		//CPU
		config_set_value(CPUConfig,"RETARDO_NOOP","100");
		config_set_value(CPUConfig,"ENTRADAS_TLB","2");
		//MEMORIA
		config_set_value(memoriaConfig,"TAM_MEMORIA","10240");
		config_set_value(memoriaConfig,"TAM_PAGINA","256");
		config_set_value(memoriaConfig,"RETARDO_MEMORIA","500");
		config_set_value(memoriaConfig,"MARCOS_POR_PROCESO","4");
		config_set_value(memoriaConfig,"RETARDO_SWAP","1000");

		flag=0;
		}

	//Guardar Configs
	config_save(CPUConfig);
	config_save(kernelConfig);
	config_save(consolaConfig);
	config_save(memoriaConfig);

	config_destroy(ipConfig);
	config_destroy(consolaConfig);
	config_destroy(CPUConfig);
	config_destroy(kernelConfig);
	config_destroy(memoriaConfig);

	if(flag)
		printf("Error: nombre de prueba no reconocido.\n");

}
