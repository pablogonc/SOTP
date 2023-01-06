#include "swapManager.h"

void nuevoSwap(){
	uint32_t pcbId = *((uint32_t*) siguienteAccion->data);
	FILE * swapFile;
	char * n = nombreArchivoSwap(pcbId);
	swapFile = fopen(n,"wb");
	fclose(swapFile);
	free(n);
}

void borrarSwap(){
	uint32_t pcbId = *((uint32_t*) siguienteAccion->data);
	char * n = nombreArchivoSwap(pcbId);
	remove(n);
	free(n);
}

void escribirPaginaEnSwap(){
	uint32_t pcbId;
	memcpy(&pcbId, siguienteAccion->data, sizeof(uint32_t));
	int paginaCorrespondiente;
	memcpy(&paginaCorrespondiente, siguienteAccion->data + sizeof(uint32_t), sizeof(int));
	void* contenido = malloc(TAM_PAGINA);
	memcpy(contenido, siguienteAccion->data + sizeof(uint32_t) + sizeof(int), TAM_PAGINA);
	bool hagoLatencia;
	memcpy(&hagoLatencia, siguienteAccion->data + sizeof(uint32_t) + sizeof(int) + TAM_PAGINA, sizeof(bool));
	bool estaEnSwap;
	memcpy(&estaEnSwap, siguienteAccion->data + sizeof(uint32_t) + sizeof(int) + TAM_PAGINA + sizeof(bool), sizeof(bool));

	char * n = nombreArchivoSwap(pcbId);
	FILE * swapFile;

	if(!estaEnSwap){
		swapFile = fopen(n,"ab");
	    fwrite(&paginaCorrespondiente, sizeof(int), 1, swapFile);
	    fwrite(contenido, TAM_PAGINA, 1, swapFile);
	    fclose(swapFile);
	} else {
		swapFile = fopen(n,"rb");
		FILE * swapFileNuevo = fopen("_.swap", "wb");
		void* buffer = malloc(TAM_PAGINA);
		int paginaEnSwap;
		while(fread(&paginaEnSwap, sizeof(int), 1, swapFile) && paginaEnSwap != paginaCorrespondiente){
	        fwrite(&paginaEnSwap, sizeof(int), 1, swapFileNuevo);
		    fread(buffer, TAM_PAGINA, 1, swapFile);
		    fwrite(buffer, TAM_PAGINA, 1, swapFileNuevo);
	    }
	    fwrite(&paginaEnSwap, sizeof(int), 1, swapFileNuevo);
    	fwrite(contenido, TAM_PAGINA, 1, swapFileNuevo);
    	fseek(swapFile, TAM_PAGINA, SEEK_CUR);
    	while(fread(&paginaEnSwap, sizeof(int), 1, swapFile)){
	        fwrite(&paginaEnSwap, sizeof(int), 1, swapFileNuevo);
		    fread(buffer, TAM_PAGINA, 1, swapFile);
		    fwrite(buffer, TAM_PAGINA, 1, swapFileNuevo);
	    }
	    fclose(swapFile);
	    fclose(swapFileNuevo);
	    remove(n);
    	rename("_.swap", n);
	    free(buffer);
	}
    
    free(contenido);
    free(n);
    if(hagoLatencia){
    	latenciaSwap();
    	int* cantidad = list_get(accesosADiscoPorProceso, pcbId);
		*cantidad = *cantidad + 1;
    }
}

void leerPaginaDeSwap(){
	uint32_t pcbId;
	memcpy(&pcbId, siguienteAccion->data, sizeof(uint32_t));
	int paginaCorrespondiente;
	memcpy(&paginaCorrespondiente, siguienteAccion->data + sizeof(uint32_t), sizeof(int));

    int paginaEnSwap;
    char * n = nombreArchivoSwap(pcbId);
    FILE * swapFile = fopen(n, "rb");
    FILE * swapFileNuevo = fopen("_.swap", "wb");
    void* contenido = malloc(TAM_PAGINA);
    while(fread(&paginaEnSwap, sizeof(int), 1, swapFile) && paginaEnSwap != paginaCorrespondiente){
        fwrite(&paginaEnSwap, sizeof(int), 1, swapFileNuevo);
        fread(contenido, TAM_PAGINA, 1, swapFile);
        fwrite(contenido, TAM_PAGINA, 1, swapFileNuevo);
    }
    void* contenidoLeido = malloc(TAM_PAGINA);
    fread(contenidoLeido, TAM_PAGINA, 1, swapFile);

    // No la saco del archivo swap
    fwrite(&paginaEnSwap, sizeof(int), 1, swapFileNuevo);
    fwrite(contenidoLeido, TAM_PAGINA, 1, swapFileNuevo);
    // Si sacamos esas lineas, al buscar de swap se borra la pagina

    while(fread(&paginaEnSwap, sizeof(int), 1, swapFile)){
        fwrite(&paginaEnSwap, sizeof(int), 1, swapFileNuevo);
        fread(contenido, TAM_PAGINA, 1, swapFile);
        fwrite(contenido, TAM_PAGINA, 1, swapFileNuevo);
    }
    fclose(swapFile);
    fclose(swapFileNuevo);
    remove(n);
    rename("_.swap", n);
    memcpy(siguienteAccion->data + sizeof(uint32_t) + sizeof(int), contenidoLeido, TAM_PAGINA);
    free(contenido);
    free(contenidoLeido);
    free(n);
    latenciaSwap();
    int* cantidad = list_get(accesosADiscoPorProceso, pcbId);
	*cantidad = *cantidad + 1;
}

void prepararSwap(char* pathConfig, uint32_t retardoConfig){
	path = pathConfig;
	retardo = retardoConfig;

	acciones_swap = list_create();

	swap_action_sem = (sem_t*) malloc(sizeof(sem_t));
	sem_init(swap_action_sem, 0, 0);

	swap_actividad_sem = (sem_t*) malloc(sizeof(sem_t));
	sem_init(swap_actividad_sem, 0, 0);

	pthread_t SwapManager;
	pthread_create(&SwapManager, NULL,  iniciarSwap, NULL);
}

void *iniciarSwap(){
	while(1){
		sem_wait(swap_action_sem);
		realizarAccion();
	}
}

void prepararAccionSwap(swap_accion accion, void* data) {
	AccionSwap * nuevaAccion = malloc(sizeof(AccionSwap));
	nuevaAccion->accion = accion;
	nuevaAccion->data = data;
	list_add(acciones_swap, nuevaAccion);
}

void realizarAccion() {
	siguienteAccion = list_remove(acciones_swap, 0);
	switch(siguienteAccion->accion){
		case SWAP_CREAR:
			nuevoSwap();
			sem_post(swap_actividad_sem);
			break;
		case SWAP_AGREGAR_PAGINA:
			escribirPaginaEnSwap();
			sem_post(swap_actividad_sem);
			break;
		case SWAP_RETIRAR_PAGINA:
			leerPaginaDeSwap();
			sem_post(swap_actividad_sem);
			break;
		case SWAP_ELIMINAR:
			borrarSwap();
			break;
	}
	free(siguienteAccion);
}

char* nombreArchivoSwap(uint32_t pcbId){
	char * nombreSwap = malloc(1024);
	sprintf(nombreSwap, "%s/%d.swap", path, pcbId);
	return nombreSwap;
}

void latenciaSwap(){
	sleep(((float) RETARDO_SWAP) / 1000);
}
