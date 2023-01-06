.ONESHELL:

all: utils

commons:
	@cd /home/utnso
	@git clone https://github.com/sisoputnfrba/so-commons-library.git
	@cd /home/utnso/so-commons-library
	@make install

utils: commons
	@cd ./utils
	@sudo make install

install: commons utils configIP
	@mkdir -p run/logs
	@cp -r pre-req/Tareas run/
	@cp -r pre-req/configs run/

remove:
	@rm -r ./run
	@rm -r ./.MODULOS

cpu: 
	@cd ./.MODULOS
	@mkdir -p CPU
	@cd ./CPU
	@gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"cpu.d" -MT"cpu.o" -o "cpu.o" "../../CPU/cpu.c"
	@gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"dispatchServer.d" -MT"dispatchServer.o" -o "dispatchServer.o" "../../CPU/dispatchServer.c"
	@gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"interruptServer.d" -MT"interruptServer.o" -o "interruptServer.o" "../../CPU/interruptServer.c"
	@cd ../../run
	@gcc  -o "CPU"  ../.MODULOS/CPU/cpu.o ../.MODULOS/CPU/dispatchServer.o ../.MODULOS/CPU/interruptServer.o   -lreadline -lpthread -lcommons -lm -lutils
	
memoria: 
	@cd ./.MODULOS
	@mkdir -p MEMORIA
	@cd ./MEMORIA
	@gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"memoria.d" -MT"memoria.o" -o "memoria.o" "../../Memoria/memoria.c"
	@gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"server.d" -MT"server.o" -o "server.o" "../../Memoria/server.c"
	@gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"swapManager.d" -MT"swapManager.o" -o "swapManager.o" "../../Memoria/swapManager.c"
	@cd ../../run
	@gcc  -o "Memoria"  ../.MODULOS/MEMORIA/memoria.o   ../.MODULOS/MEMORIA/server.o   ../.MODULOS/MEMORIA/swapManager.o   -lpthread -lutils -lm -lcommons -lreadline


kernel: 
	@cd ./.MODULOS
	@mkdir -p KERNEL
	@cd ./KERNEL
	@gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"KernelDispatch.d" -MT"KernelDispatch.o" -o "KernelDispatch.o" "../../Kernel/KernelDispatch.c"
	@gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"consolaServer.d" -MT"consolaServer.o" -o "consolaServer.o" "../../Kernel/consolaServer.c"
	@gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"kernel.d" -MT"kernel.o" -o "kernel.o" "../../Kernel/kernel.c"
	@cd ../../run
	@gcc  -o "Kernel"    ../.MODULOS/KERNEL/KernelDispatch.o ../.MODULOS/KERNEL/consolaServer.o ../.MODULOS/KERNEL/kernel.o   -lcommons -lpthread -lutils -lreadline

consola: 
	@cd ./.MODULOS
	@mkdir -p CONSOLA
	@cd ./CONSOLA
	@gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"consola.d" -MT"consola.o" -o "consola.o" "../../Consola/consola.c"
	@cd ../../run
	@gcc  -o "Consola"  ../.MODULOS/CONSOLA/consola.o   -lreadline -lcommons -lutils
	@cd ../
	@make taskList

configIP: 
	@mkdir -p run
	@mkdir -p .MODULOS
	@cd ./.MODULOS
	@mkdir -p CONFIGIP
	@cd ./CONFIGIP
	@gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"main.d" -MT"main.o" -o "main.o" "../../ConfigIP/main.c"
	@cd ../../run
	@gcc  -o "ConfigIP"  ../.MODULOS/CONFIGIP/main.o   -lcommons


taskList:
	@cd ./pre-req/Tareas
	@printf "\033[34m Lista de tareas: \n\n\n    \033[1m"
	@ls
	@printf "\033[0m \n\n"