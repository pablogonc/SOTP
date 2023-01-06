.ONESHELL:

all:
	@mkdir debug_terminal -p
	@cd debug_terminal
	@gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"memoria.d" -MT"memoria.o" -o "memoria.o" "../memoria.c"
	@gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"server.d" -MT"server.o" -o "server.o" "../server.c"
	@gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"swapManager.d" -MT"swapManager.o" -o "swapManager.o" "../swapManager.c"
	@gcc  -o "Memoria"  ./memoria.o ./server.o ./swapManager.o -lpthread -lutils -lcommons -lreadline -lm
	@echo "Memoria listorti"

clean:
	@rm -r debug_terminal
	@echo "Memoria arafue"

sus:
	@gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"sustitucion.d" -MT"sustitucion.o" -o "sustitucion.o" "../sustitucion.c"