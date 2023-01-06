.ONESHELL:

all:
	@mkdir debug_terminal -p
	@cd debug_terminal
	@gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"KernelDispatch.d" -MT"KernelDispatch.o" -o "KernelDispatch.o" "../KernelDispatch.c"
	@gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"consolaServer.d" -MT"consolaServer.o" -o "consolaServer.o" "../consolaServer.c"
	@gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"kernel.d" -MT"kernel.o" -o "kernel.o" "../kernel.c"
	@gcc  -o "Kernel"  ./KernelDispatch.o ./consolaServer.o ./kernel.o   -lcommons -lpthread -lutils -lreadline
	@echo "Kernel listorti"

clean:
	@rm -r debug_terminal
	@echo "Kernel arafue"