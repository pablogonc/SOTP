
# TP 2022 1C Somos de late


<p align="center">
	<a href="https://docs.google.com/document/d/17WP76Vsi6ZrYlpYT8xOPXzLf42rQgtyKsOdVkyL5Jj0/edit">
  <img 
    width="400"
    height="400"
		src="https://lh6.googleusercontent.com/b2eYHgc3y0PfB3s6iZhLZX9Z8bOTnZYEqkkUXOUYnzN65i5foovgCgsagsvWhidBeNkf9dIxbqMqHSup4f1xuaypJvMynobRoIkpmDyIZWRSBlfjx6iELe7BB6VvcHbz8pz_UfiO">
	</a>
</p>
 
## Tests
[Documento de pruebas](https://docs.google.com/document/d/1SBBTCweMCiBg6TPTt7zxdinRh4ealRasbu0bVlkty5o/edit)
### Ejecucion
**Cambiar desde chrome las ips:**
 **repo &rarr; pre-req &rarr; ips.config**

**Orden de ejecución**: Memoria &rarr; CPU &rarr; Kernel &rarr; Consola

    make install
    make [cpu|memoria|kernel|consola]
    cd ./run
    ./ConfigIP [test] [algoritmoKernel] [algoritmoCPU] [algoritmoMemoria]  
    ./[cpu|memoria|kernel] || ./consola [tarea] [size]
 ----
 ## ConfigIP 
 ####  Prueba Base
    ./ConfigIP prueba-Base FIFO LRU CLOCK-M
 #### Kernel - Planificación 
     ./ConfigIP kernel-plani FIFO LRU CLOCK-M 
     ./ConfigIP kernel-plani SRT LRU CLOCK-M
 #### Kernel - Suspensión
     ./ConfigIP kernel-susp FIFO LRU CLOCK-M 
     ./ConfigIP kernel-susp SRT LRU CLOCK-M
 #### Memoria
     ./ConfigIP memoria FIFO LRU CLOCK 
     ./ConfigIP memoria FIFO LRU CLOCK-M
#### TLB
     ./ConfigIP tlb FIFO FIFO CLOCK 
     ./ConfigIP tlb  FIFO LRU CLOCK 
#### Estabilidad Integral
     ./ConfigIP estabilidad SRT LRU CLOCK-M
## Comandos Consola
 ####  Prueba Base
    ./Consola ./Tareas/BASE_1 4
    ./Consola ./Tareas/BASE_2 4
    ./Consola ./Tareas/BASE_2 4
    
 #### Kernel - Planificación
    ./Consola ./Tareas/PLANI_1 4
    ./Consola ./Tareas/PLANI_1 4
    ./Consola ./Tareas/PLANI_2 4  
 #### Kernel - Suspensión
    ./Consola ./Tareas/SUSPE_1 4
    ./Consola ./Tareas/SUSPE_2 4
    ./Consola ./Tareas/SUSPE_3 4  
 #### Memoria
    ./Consola ./Tareas/MEMORIA_1 4096
#### TLB
    ./Consola ./Tareas/TLB_1 2048
    ./Consola ./Tareas/TLB_2 2048	
#### Estabilidad Integral
    ./Consola ./Tareas/INTEGRAL_1 2048
    ./Consola ./Tareas/INTEGRAL_2 2048
    ./Consola ./Tareas/INTEGRAL_3 2048
    ./Consola ./Tareas/INTEGRAL_4 2048
    ./Consola ./Tareas/INTEGRAL_5 2048
    
## PRUEBAS
[Gants](https://docs.google.com/spreadsheets/d/1PenQ2lGgxTceN09a-5BUhfjMMv2HoNt-qQuIQE_HStE/edit?usp=sharing)
 - [x] BASE (COINCIDE CON MI GANTT)
 - [x] PLANIFICACION SRT (COINCIDE ORDEN QUE DICE Y CON MI GANTT)
 - [x] PLANIFICACION FIFO (COICIDE CON MI GANTT)
 - [x] SUSPENSION FIFO (Coincide exacto GANT CATEDRA)
 - [x] SUSPENSION SRT (Coincide exacto GANT CATEDRA)
 - [X] Memoria - Clock  ( 8 accesos)
 - [X] Memoria  Clock-M (4 accesos)
 - [x] TLB  FIFO (6 PF antes del IO)
 - [x] TLB LRU (5 PF antes del IO)
 - [x] Prueba de Estabilidad Integral ( No se que debería esperar pero termina sin errores) 

## Librerias
### Commons Library
https://github.com/sisoputnfrba/so-commons-library.git
### Librería con utilidades

*Tiene funciones para hacer las conexiones entre los módulos, variables globales*
