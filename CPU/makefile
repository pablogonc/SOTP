.ONESHELL:

all:
	@mkdir debug_terminal -p
	@cd debug_terminal
	@gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"cpu.d" -MT"cpu.o" -o "cpu.o" "../cpu.c"
	@gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"dispatchServer.d" -MT"dispatchServer.o" -o "dispatchServer.o" "../dispatchServer.c"
	@gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"interruptServer.d" -MT"interruptServer.o" -o "interruptServer.o" "../interruptServer.c"
	@gcc  -o "CPU"  ./cpu.o ./dispatchServer.o ./interruptServer.o   -lreadline -lm -lpthread -lcommons -lutils
	@echo "CPU listorti"

clean:
	@rm -r debug_terminal
	@echo "CPU arafue"