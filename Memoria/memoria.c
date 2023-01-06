/*
 * memoria.c xd
 *
 *  Created on: 23 abr. 2022
 *      Author: utnso
 */

/*
	TODO Generales:
	- Sustitución
*/

#include "memoria.h"
#include "math.h"

int main(){
	printf("Iniciando Memoria\n\n");

	logger = log_create("./logs/memoria.log", "cpu", 0, LOG_LEVEL_DEBUG);

	leerConfig();

	mkdir(PATH_SWAP,0777);

	prepararServer(PUERTO_ESCUCHA);

	prepararSwap(PATH_SWAP, RETARDO_SWAP);

	prepararMemoria();

	iniciarMemoriaPrincipal();

	iniciarListadoTablasPaginas();

	while(1){
		sem_wait(memoria_sem);
		realizarAccionMemoria();
	}

	return 0;
}

void realizarAccionMemoria() {
	siguienteAccionMemoria = list_remove(acciones_memoria, 0);
	switch(siguienteAccionMemoria->accion){
		case CREAR_PRIMER_NIVEL: {
			TamanioYSocket * ts = (TamanioYSocket*) siguienteAccionMemoria->data;
			int indexPrimerNivel = crearTablaPrimerNivel(ts->tamanio);
			sem_post(memoria_actividad_sem);
			connection_packet* p = connection_packet_create();
			connection_packet_add(p,&indexPrimerNivel,sizeof(int));
			//latenciaMemoria();
			connection_packet_send(p,ts->Sock);
			connection_packet_destroy(p);
			free(ts);
			break;
		}
		case LIBERAR_PRIMER_NIVEL: {
			uint32_t * pcbId = (uint32_t*) siguienteAccionMemoria->data;
			liberarMarcosDelProceso(*pcbId);
			break;
		}
		case RESPONDER_PRIMER_NIVEL: {
			IndiceEntradaSock * ies = (IndiceEntradaSock*) siguienteAccionMemoria->data;
			int indice = getIndice2doNivel(ies->indice, ies->entrada);
			connection_packet * p = connection_packet_create();
			connection_packet_add(p, &indice, sizeof(int));
			latenciaMemoria();
			connection_packet_send(p, ies->Sock);
			connection_packet_destroy(p);
			free(ies);
			break;
		}
		case RESPONDER_SEGUNDO_NIVEL: {
			IndiceEntradaSock * ies = (IndiceEntradaSock*) siguienteAccionMemoria->data;
			int marco = getMarco(ies->indice, ies->entrada);
			connection_packet * p = connection_packet_create();
			connection_packet_add(p, &marco, sizeof(int));
			latenciaMemoria();
			connection_packet_send(p, ies->Sock);
			connection_packet_destroy(p);
			free(ies);
			break;
		}
		case RESPONDER_MEMORIA_USUARIO: {
			IndiceEntradaSock * ies = (IndiceEntradaSock*) siguienteAccionMemoria->data;
			uint32_t palabra = leerPalabraMemoria(ies->indice, ies->entrada);
			connection_packet * p = connection_packet_create();
			connection_packet_add(p, &palabra, sizeof(uint32_t));
			latenciaMemoria();
			connection_packet_send(p, ies->Sock);
			connection_packet_destroy(p);
			free(ies);
			break;
		}
		case ESCRIBIR_MEMORIA_USUARIO: {
			MarcoDesplazamientoPalabraSock * mdps = (MarcoDesplazamientoPalabraSock*) siguienteAccionMemoria->data;
			escribirPalabraMemoria(mdps->marco, mdps->desplazamiento, mdps->palabra);
			connection_packet * p = connection_packet_create();
			int confirm = OP_CONFIRMACION;
			connection_packet_add(p, &confirm, sizeof(uint32_t));
			latenciaMemoria();
			connection_packet_send(p, mdps->Sock);
			connection_packet_destroy(p);
			free(mdps);
			break;
		}
		case SUSPENDER_PROCESO: {
			TamanioYSocket * ts = (TamanioYSocket*) siguienteAccionMemoria->data;
			mandarPaginasASwap(ts->tamanio);
			connection_packet * p = connection_packet_create();
			int confirm = OP_CONFIRMACION;
			connection_packet_add(p, &confirm, sizeof(uint32_t));
			//latenciaMemoria();
			connection_packet_send(p,ts->Sock);
			connection_packet_destroy(p);
			free(ts);
			break;
		}
	}
	free(siguienteAccionMemoria);
}

void prepararMemoria(){
	acciones_memoria = list_create();

	memoria_sem = (sem_t*) malloc(sizeof(sem_t));
	sem_init(memoria_sem, 0, 0);

	memoria_actividad_sem = (sem_t*) malloc(sizeof(sem_t));
	sem_init(memoria_actividad_sem, 0, 0);
}

void prepararAccionMemoria(memoria_accion accion, void* data) {
	AccionMemoria * nuevaAccion = malloc(sizeof(AccionMemoria));
	nuevaAccion->accion = accion;
	nuevaAccion->data = data;
	list_add(acciones_memoria, nuevaAccion);
}

void leerConfig(){
	memoriaConfig = config_create("./configs/memoria.config");

	PUERTO_ESCUCHA = config_get_string_value(memoriaConfig,"PUERTO_ESCUCHA");
	TAM_MEMORIA = config_get_int_value(memoriaConfig,"TAM_MEMORIA");
	TAM_PAGINA = config_get_int_value(memoriaConfig,"TAM_PAGINA");
	ENTRADAS_POR_TABLA = config_get_int_value(memoriaConfig,"ENTRADAS_POR_TABLA");
	RETARDO_MEMORIA = config_get_int_value(memoriaConfig,"RETARDO_MEMORIA");
	ALGORITMO_REEMPLAZO = config_get_string_value(memoriaConfig,"ALGORITMO_REEMPLAZO");;
	MARCOS_POR_PROCESO = config_get_int_value(memoriaConfig,"MARCOS_POR_PROCESO");
	RETARDO_SWAP = config_get_int_value(memoriaConfig,"RETARDO_SWAP");
	PATH_SWAP = config_get_string_value(memoriaConfig,"PATH_SWAP");

	printf("=== Archivo de configuracion ====\n"PRINT_COLOR_YELLOW
			" PUERTO ESCUCHA: %s \n"
			" TAM MEMORIA: %d \n"
			" TAM PAGINA: %d \n"
			" ENTRADAS POR TABLA: %d \n"
			" RETARDO MEMORIA: %d \n"
			" ALGORITMO REEMPLAZO: %s \n"
			" MARCOS POR PROCESO: %d \n"
			" RETARDO SWAP: %d \n"
			" PATH SWAP: %s \n\n"
			PRINT_COLOR_RESET,PUERTO_ESCUCHA,TAM_MEMORIA,TAM_PAGINA,ENTRADAS_POR_TABLA,RETARDO_MEMORIA,ALGORITMO_REEMPLAZO,MARCOS_POR_PROCESO,RETARDO_SWAP,PATH_SWAP);
}

void iniciarMemoriaPrincipal(){
	memoria_principal = malloc(TAM_MEMORIA);
}

void iniciarListadoTablasPaginas(){
	tablasPaginasPrimerNivel = list_create();
	tablasPaginasSegundoNivel = list_create();

	marcosAsignados =  list_create();
	TIMESTAMP_INICIO = 0;
	TIMESTAMP_INICIO = milis();
	
	CANTIDAD_DE_MARCOS = TAM_MEMORIA/TAM_PAGINA;

	marco_asignado * m;
	for(int i = 0; i < CANTIDAD_DE_MARCOS; i++){
		m = malloc(sizeof(marco_asignado));
		m->libre = true;
		list_add(marcosAsignados, m);
	}

	paginasSwapeadasPorProceso = list_create();
	accesosADiscoPorProceso = list_create();
}

bool verificarProcesoValido(int tam){
	return tam <= ENTRADAS_POR_TABLA * ENTRADAS_POR_TABLA;
}

int crearTablaPrimerNivel(int tamProceso){
	int tamEnPaginas = ((tamProceso - 1) / TAM_PAGINA) + 1;;

	printf("Creando nueva tabla de primer nivel para %d paginas\n", tamEnPaginas);
	log_info(logger, "Creando nueva tabla de primer nivel para %d paginas\n", tamEnPaginas);

	if(verificarProcesoValido(tamEnPaginas)){
		t_list* nuevaTablaPrimerNivel = list_create();

		int cantidadTablasACrearSN = ((tamEnPaginas - 1) / ENTRADAS_POR_TABLA) + 1;
		for(int i = 0; i< cantidadTablasACrearSN; i++){
			int cantidadPaginasSN = (tamEnPaginas >= ENTRADAS_POR_TABLA) ? ENTRADAS_POR_TABLA : tamEnPaginas;
			int indexTablaNuevaSN = crearTablaSegundoNivel(cantidadPaginasSN);
			int * iLista = malloc(sizeof(int));
			memcpy(iLista, &indexTablaNuevaSN, sizeof(int));
			list_add(nuevaTablaPrimerNivel, iLista);
			tamEnPaginas = tamEnPaginas - cantidadPaginasSN;

			printf(PRINT_COLOR_BLUE"Indice de la nueva tabla de segundo nivel: %d\n"PRINT_COLOR_RESET, indexTablaNuevaSN);
			log_info(logger, "Indice de la nueva tabla de segundo nivel: %d\n", indexTablaNuevaSN);
		}

		t_list* paginasSwapeadas = list_create();
		list_add(paginasSwapeadasPorProceso, paginasSwapeadas);
		int* cantidad = malloc(sizeof(int));
		*cantidad = 0;
		list_add(accesosADiscoPorProceso, cantidad);

		int indiceNuevo = list_add(tablasPaginasPrimerNivel, nuevaTablaPrimerNivel);

		printf(PRINT_COLOR_GREEN"Indice de la nueva tabla de primer nivel: %d\n"PRINT_COLOR_RESET, indiceNuevo);
		log_info(logger, "Indice de la nueva tabla de primer nivel: %d\n", indiceNuevo);

		return indiceNuevo;
	} else {
		return -1;
	}
}

int crearTablaSegundoNivel(int cantPaginas){
	t_list* tablaNuevaSegundoNivel = list_create();

	for(int i = 0; i < cantPaginas; i++){
		pagina_segundo_nivel * paginaNueva = malloc(sizeof(pagina_segundo_nivel));
		paginaNueva->presencia = false;
		list_add(tablaNuevaSegundoNivel, paginaNueva);
	}

	int indiceNuevo = list_add(tablasPaginasSegundoNivel, tablaNuevaSegundoNivel);
	return indiceNuevo;
}

void escribirPalabraMemoria(int marco, int desplazamiento, uint32_t palabra){
	int offsetTotal = marco * TAM_PAGINA + desplazamiento;

	printf("Escribo %d en la direccion fisica %d\n", palabra, offsetTotal);
	log_info(logger, "Escribo %d en la direccion fisica %d\n", palabra, offsetTotal);

	memcpy(memoria_principal+offsetTotal, &palabra, sizeof(uint32_t));
	modificarBitsPaginaSegundoNivelSegunMarco(marco, true);
}

uint32_t leerPalabraMemoria(int marco, int desplazamiento){
	int offsetTotal = marco * TAM_PAGINA + desplazamiento;
	uint32_t* response = malloc(sizeof(uint32_t));
	memcpy(response, memoria_principal+offsetTotal, sizeof(uint32_t));
	modificarBitsPaginaSegundoNivelSegunMarco(marco, false);
	uint32_t r = *response;

	printf("Leo %d de la direccion fisica %d\n", r, offsetTotal);
	log_info(logger, "Leo %d de la direccion fisica %d\n", r, offsetTotal);

	free(response);
	return r;
}

void modificarBitsPaginaSegundoNivelSegunMarco(int marco, bool escribiendo){
	marco_asignado* m = list_get(marcosAsignados, marco);
	uint32_t pcbId = m->proceso;
	int paginaCorrespondiente = m->pagina; 
	t_list* tablaP1N = list_get(tablasPaginasPrimerNivel, pcbId);
	int* indiceTP2N = list_get(tablaP1N, paginaCorrespondiente/ENTRADAS_POR_TABLA);
	t_list* paginas2N = list_get(tablasPaginasSegundoNivel, *indiceTP2N);
	pagina_segundo_nivel* pagina = list_get(paginas2N, paginaCorrespondiente%ENTRADAS_POR_TABLA);
	pagina->uso = true;
	if(escribiendo)
		pagina->modificado = true;
}


void escribirMarcoMemoria(int marco, void* pagina){
	int offsetTotal = marco * TAM_PAGINA;
	memcpy(memoria_principal+offsetTotal, pagina, TAM_PAGINA);
}

void* leerMarcoMemoria(int marco){
	int offsetTotal = marco * TAM_PAGINA;
	void* response = malloc(TAM_PAGINA);
	memcpy(response, memoria_principal+offsetTotal, TAM_PAGINA);
	return response;
}

int getIndice2doNivel(int indice, int entrada){
	cachePcbId = indice;
	cacheEntrada1erNivel = entrada;
	t_list* tp1 = list_get(tablasPaginasPrimerNivel, indice);
	int * is2 = list_get(tp1, entrada);
	return *is2;
}

int getMarco(int indice, int entrada){
	t_list* tp2 = list_get(tablasPaginasSegundoNivel,indice);
	pagina_segundo_nivel * p = list_get(tp2,entrada);
	if(!p->presencia){
		int pagina = cacheEntrada1erNivel*ENTRADAS_POR_TABLA + entrada;
		if(paginaEstaEnSwap(cachePcbId, pagina)) {
			traerPaginaDeSwapYAsignarAMarco(cachePcbId, pagina);
		} else {
			printf(PRINT_COLOR_BLUE"Asigno un nuevo marco para la pagina %d del proceso %d\n"PRINT_COLOR_RESET, pagina, cachePcbId);
			log_info(logger, "Asigno un nuevo marco para la pagina %d del proceso %d\n", pagina, cachePcbId);
			getMarcoLibreParaPagina(cachePcbId, pagina);
		}
	}
	return p->marco;
}

void mandarPaginaASwap(uint32_t pcbId, int paginaCorrespondiente, bool hagoLatencia){
	int marco = getMarcoDePagina(pcbId,paginaCorrespondiente);
	void* contenido = leerMarcoMemoria(marco);
	marco_asignado * m = list_get(marcosAsignados, marco);
	m->libre = true;
	desasignarMarcoEnTabla2doNivel(pcbId, paginaCorrespondiente);

	void* punteroData = malloc(sizeof(uint32_t) + sizeof(int) + TAM_PAGINA + sizeof(bool) + sizeof(bool));

	bool yaEstabaEnSwap = paginaEstaEnSwap(pcbId, paginaCorrespondiente);

	if(!yaEstabaEnSwap || laPaginaEstaModificada(pcbId, paginaCorrespondiente)){
		printf(PRINT_COLOR_YELLOW"Mando a swap la pagina %d del proceso %d\n"PRINT_COLOR_RESET, paginaCorrespondiente, pcbId);
		log_info(logger, "Mando a swap la pagina %d del proceso %d\n", paginaCorrespondiente, pcbId);

		memcpy(punteroData, &pcbId, sizeof(uint32_t));
		memcpy(punteroData + sizeof(uint32_t), &paginaCorrespondiente, sizeof(int));
		memcpy(punteroData + sizeof(uint32_t) + sizeof(int), contenido, TAM_PAGINA);
		memcpy(punteroData + sizeof(uint32_t) + sizeof(int) + TAM_PAGINA, &hagoLatencia, sizeof(bool));
		memcpy(punteroData + sizeof(uint32_t) + sizeof(int) + TAM_PAGINA + sizeof(bool), &yaEstabaEnSwap, sizeof(bool));
		prepararAccionSwap(SWAP_AGREGAR_PAGINA, punteroData);
		sem_post(swap_action_sem);

		sem_wait(swap_actividad_sem);
	} else {
		printf(PRINT_COLOR_YELLOW"No es necesario mandar a swap la pagina %d del proceso %d, pero la desasigno\n"PRINT_COLOR_RESET, paginaCorrespondiente, pcbId);
		log_info(logger, "No es necesario mandar a swap la pagina %d del proceso %d, pero la desasigno\n", paginaCorrespondiente, pcbId);
	}

	// No agregar si ya estaba
	if(!yaEstabaEnSwap){
		t_list* paginasSwapeadas = list_get(paginasSwapeadasPorProceso, pcbId);
		int * paginaPorSwapear = malloc(sizeof(int));
		memcpy(paginaPorSwapear, &paginaCorrespondiente, sizeof(int));
		list_add(paginasSwapeadas, paginaPorSwapear);
	}

	free(contenido);
	free(punteroData);
}

void traerPaginaDeSwapYAsignarAMarco(uint32_t pcbId, int paginaCorrespondiente) {
	printf(PRINT_COLOR_MAGENTA"Busco de swap la pagina %d del proceso %d\n"PRINT_COLOR_RESET, paginaCorrespondiente, pcbId);
	log_info(logger, "Busco de swap la pagina %d del proceso %d\n", paginaCorrespondiente, pcbId);

	// No la saco del archivo swap
	/*t_list* paginasSwapeadas = list_get(paginasSwapeadasPorProceso, pcbId);
	int tam = list_size(paginasSwapeadas);
	for(int i = 0; i < tam; i++){
		int * p = list_get(paginasSwapeadas, i);
		if(*p == paginaCorrespondiente){
			list_remove_and_destroy_element(paginasSwapeadas, i, (void*)_int_destroy);
			break;
		}
	}*/

	void* punteroDataLectura = malloc(sizeof(uint32_t) + sizeof(int) + TAM_PAGINA);
	memcpy(punteroDataLectura, &pcbId, sizeof(uint32_t));
	memcpy(punteroDataLectura + sizeof(uint32_t), &paginaCorrespondiente, sizeof(int));
	prepararAccionSwap(SWAP_RETIRAR_PAGINA, punteroDataLectura);
	sem_post(swap_action_sem);

	sem_wait(swap_actividad_sem);

	int marco = getMarcoLibreParaPagina(pcbId,paginaCorrespondiente);
	escribirMarcoMemoria(marco, punteroDataLectura + sizeof(uint32_t) + sizeof(int));

	free(punteroDataLectura);
}

void _int_destroy(int* self) {
    free(self);
}

int milis() {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    int m = tv.tv_usec;
    if(m < TIMESTAMP_INICIO)
    	m = TIMESTAMP_INICIO + 86400000 + m;
    return m - TIMESTAMP_INICIO;
}

int getMarcoLibreParaPagina(uint32_t pcbId, int paginaCorrespondiente){
	bool hayLibre = list_any_satisfy(marcosAsignados, (void*)_estaLibre);
	if(hayLibre && procesoDentroLimiteMarcosPorProceso(pcbId)){
		marco_asignado * m;
		for(int i = 0; i < CANTIDAD_DE_MARCOS; i++){
			m = list_get(marcosAsignados, i);
			if(m->libre){
				m->libre = false;
				m->proceso = pcbId;
				m->pagina = paginaCorrespondiente;
				m->milisDesdeCarga = milis();
				asignarMarcoEnTabla2doNivel(i, pcbId, paginaCorrespondiente);
				return i;
			}
		}
	} else {
		int indicePaginaSegundoNivel = seleccionarFrameVictima(pcbId);
		mandarPaginaASwap(pcbId, indicePaginaSegundoNivel, true);
		return getMarcoLibreParaPagina(pcbId, paginaCorrespondiente);
	}
	return -1;
}

bool procesoDentroLimiteMarcosPorProceso(uint32_t pcbId){
	cacheProcesoLista = pcbId;
	t_list* cargadas = list_filter(marcosAsignados, (void*)_esDelProcesoYEstaCargado);
	int cargados = list_size(cargadas);
	return cargados < MARCOS_POR_PROCESO;
}

bool _estaLibre(void * marcoAsignado) {
	return ((marco_asignado*)marcoAsignado)->libre;
}

bool _esDelProcesoYEstaCargado(void * marcoAsignado) {
	marco_asignado * m = (marco_asignado*) marcoAsignado;
	return m->proceso == cacheProcesoLista && !m->libre;
}

void _liberarMarco(void * marcoAsignado) {
	marco_asignado * m = (marco_asignado*) marcoAsignado;
	m->libre = true;
}

void _mandarPaginaASwap(void * marcoAsignado) {
	marco_asignado * m = (marco_asignado*) marcoAsignado;
	mandarPaginaASwap(cacheProcesoLista, m->pagina, cachePaginaLatencia == m->pagina);
}

void liberarMarcosDelProceso(uint32_t pcbId){
	cacheProcesoLista = pcbId;
	t_list * aLiberar = list_filter(marcosAsignados, (void*)_esDelProcesoYEstaCargado);
	list_iterate(aLiberar, (void*)_liberarMarco);
}

int getMarcoDePagina(uint32_t pcbId, int paginaCorrespondiente){
	marco_asignado * m;
	for(int i = 0; i < CANTIDAD_DE_MARCOS; i++){
		m = list_get(marcosAsignados, i);
		if(m->proceso == pcbId && m->pagina == paginaCorrespondiente)
			return i;
	}
	return -1;
}

void mandarPaginasASwap(uint32_t pcbId){
	cacheProcesoLista = pcbId;
	t_list * aLiberar = list_filter(marcosAsignados, (void*)_esDelProcesoYEstaCargado);
	cachePaginaLatencia = -1;
	for(int i = 0; i < list_size(aLiberar); i++){
		marco_asignado * m = list_get(aLiberar, i);
		if(!paginaEstaEnSwap(pcbId, m->pagina) || laPaginaEstaModificada(pcbId, m->pagina))
			cachePaginaLatencia = m->pagina;
	}

	printf(PRINT_COLOR_YELLOW"======== Suspendo el proceso %d ========\n"PRINT_COLOR_RESET, pcbId);
	log_info(logger, "======== Suspendo el proceso %d ========\n", pcbId);

	list_iterate(aLiberar, (void*)_mandarPaginaASwap);
}

void asignarMarcoEnTabla2doNivel(int marco, uint32_t pcbId, int paginaCorrespondiente){
	int entradaLv1= floor(paginaCorrespondiente/ENTRADAS_POR_TABLA);
	int entradaLv2= paginaCorrespondiente % ENTRADAS_POR_TABLA;
	int indice2doNivel = getIndice2doNivel(pcbId, entradaLv1);
	t_list * tabla = list_get(tablasPaginasSegundoNivel, indice2doNivel);
	pagina_segundo_nivel * pag = list_get(tabla, entradaLv2);
	pag->marco = marco;
	pag->presencia = true;
	pag->uso = true;
	pag->modificado = false;
}

void desasignarMarcoEnTabla2doNivel(uint32_t pcbId, int paginaCorrespondiente){
	int entradaLv1= floor(paginaCorrespondiente/ENTRADAS_POR_TABLA);
	int entradaLv2= paginaCorrespondiente % ENTRADAS_POR_TABLA;
	int indice2doNivel = getIndice2doNivel(pcbId, entradaLv1);
	t_list * tabla = list_get(tablasPaginasSegundoNivel, indice2doNivel);
	pagina_segundo_nivel * pag = list_get(tabla, entradaLv2);
	pag->presencia = false;
}

void latenciaMemoria(){
	sleep(((float) RETARDO_MEMORIA)/1000);
}

bool paginaEstaEnSwap(uint32_t pcbId, int paginaCorrespondiente){
	t_list* paginasSwapeadas = list_get(paginasSwapeadasPorProceso, pcbId);
	cachePagina = paginaCorrespondiente;
	bool esta = list_any_satisfy(paginasSwapeadas, (void*)_esLaPagina);
	return esta;
}

bool _esLaPagina(int * p){
	return *p == cachePagina;
}

void printMarcosAsignados(){
	for(int i = 0; i < list_size(marcosAsignados); i++){
		marco_asignado* m = list_get(marcosAsignados, i);
		if(m->pagina < 128)
			printf(PRINT_COLOR_RED"Marco %d libre %d pagina asignada %d\n"PRINT_COLOR_RESET, i, m->libre, m->pagina);
	}
}

void printPaginas(){
	for(int i = 0; i < list_size(tablasPaginasSegundoNivel); i++){
		t_list* paginas = list_get(tablasPaginasSegundoNivel, i);
		printf("TABLA %d =======================\n", i);
		for(int j=0; j < list_size(paginas); j++){
			pagina_segundo_nivel * p = list_get(paginas, j);
			printf("Marco: %d, Presencia: %d, Uso: %d, Modificado: %d\n", p->marco, p->presencia, p->uso, p->modificado);
		}
	}
	printf(PRINT_COLOR_RESET);
}

bool laPaginaEstaModificada(uint32_t pcbId, int paginaCorrespondiente){
	int entradaLv1= floor(paginaCorrespondiente/ENTRADAS_POR_TABLA);
	int entradaLv2= paginaCorrespondiente % ENTRADAS_POR_TABLA;
	int indice2doNivel = getIndice2doNivel(pcbId, entradaLv1);
	t_list * tabla = list_get(tablasPaginasSegundoNivel, indice2doNivel);
	pagina_segundo_nivel * pag = list_get(tabla, entradaLv2);
	return pag->modificado || esClock();
}

bool esClock(){
	return strcmp(ALGORITMO_REEMPLAZO, "CLOCK-M") != 0;
}

// SUSTITUCION

int recorrerFramesClock(t_list* frames){
	int victima = -1;
	int cantidadFrames = list_size(frames);
	for(int i = 0; i < cantidadFrames; i++){
			pagina_segundo_nivel* paginaActual = (pagina_segundo_nivel*) list_get(frames, i);

			if(paginaActual->uso == false) {
				victima = i;
				break;
			} else if(paginaActual->uso == true) {
				paginaActual->uso = false;
				list_replace(frames, i, paginaActual);
			}
		}
	return victima;
}

void ordenarPaginasSegundoNivelPorFifo(t_list* lista){
	list_sort(lista, (void*)comparadorPaginas);
}

bool comparadorPaginas (void* pagina1, void* pagina2) { 
	marco_asignado* marco1 = paginaAMarco(pagina1);
	marco_asignado* marco2 = paginaAMarco(pagina2);
	return ((marco1->milisDesdeCarga) < (marco2->milisDesdeCarga)) ;
}

void* paginaAMarco (void* pagina) { 	
	pagina_segundo_nivel* p = (pagina_segundo_nivel*) pagina;
	int marco = p->marco;
	void* m = list_get(marcosAsignados, marco);
	return m; 
}

int seleccionarVictimaClock(t_list * frames){
	int victima = -1;
	victima = recorrerFramesClock(frames);
	if(victima == -1) victima = recorrerFramesClock(frames);
	return victima;
}

int buscoFrameUM(t_list * frames, bool bitUsoBuscado, bool bitModificadoBuscado, bool reemplazoBitUsuario){
	int res = -1;
	int cantidadFrames =  list_size(frames);
	for (int i = 0; i < cantidadFrames; i++){
		pagina_segundo_nivel* pagina = list_get(frames, i);
		if(pagina->uso == bitUsoBuscado && pagina->modificado == bitModificadoBuscado) {
			res = i;
			break;
		} else {
			if(reemplazoBitUsuario) {
				pagina->uso = false;
				list_replace(frames, i, pagina);
			}
		}
	}
	return res;
}

pagina_segundo_nivel* paginaPorMarco(t_list* marcosOcupados, int marco){
	marco_asignado* m = list_get(marcosOcupados, marco);
	uint32_t pcbId = m->proceso;
	int paginaCorrespondiente = m->pagina; 

	t_list* tablaP1N = list_get(tablasPaginasPrimerNivel, pcbId);
	int* indiceTP2N = list_get(tablaP1N, paginaCorrespondiente/ENTRADAS_POR_TABLA);
	t_list* paginas2N = list_get(tablasPaginasSegundoNivel, *indiceTP2N);

	pagina_segundo_nivel* pagina = list_get(paginas2N, paginaCorrespondiente%ENTRADAS_POR_TABLA);
	return pagina;
} 

int seleccionarVictimaClockModificado(t_list* frames){
	int victima = -1;
	victima = buscoFrameUM(frames,0,0, false);
	if(victima == -1) victima = buscoFrameUM(frames,0,1, true);
	if(victima == -1) victima = buscoFrameUM(frames,0,0, false);	
	if(victima == -1) victima = buscoFrameUM(frames,0,1, false);
	return victima;
}

bool paginaCargada(void * pagina) {
	return ((pagina_segundo_nivel*)pagina)->presencia;
}

int seleccionarFrameVictima(int idProceso){
	t_list* tablaPrimerNivel = list_get(tablasPaginasPrimerNivel, idProceso);
	int tamListaPN = list_size(tablaPrimerNivel);

	t_list* listaConPaginasEnMarcos = list_create();

	for(int i = 0; i < tamListaPN; i++){
		int* indiceTP2N = list_get(tablaPrimerNivel, i);
		t_list* paginas2N = list_get(tablasPaginasSegundoNivel, *indiceTP2N);
		t_list* paginasEnFrames = list_filter(paginas2N, (void*) paginaCargada);
		int tamP2N = list_size(paginasEnFrames);
		if(tamP2N > 0){
			list_add_all(listaConPaginasEnMarcos, paginasEnFrames);
		}
	}

	ordenarPaginasSegundoNivelPorFifo(listaConPaginasEnMarcos);
	
	int victima = -1;

	if(esClock()){
		victima = seleccionarVictimaClock(listaConPaginasEnMarcos);
	} else {
		victima = seleccionarVictimaClockModificado(listaConPaginasEnMarcos);
	}
	pagina_segundo_nivel* p = list_get(listaConPaginasEnMarcos, victima);
	int marco = p->marco;
	marco_asignado* m = list_get(marcosAsignados, marco);
	int paginaBuscada = m->pagina;
	return paginaBuscada;
}
