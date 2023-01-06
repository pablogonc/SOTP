.ONESHELL:

all:
	@mkdir debug_terminal -p
	@cd debug_terminal
	@gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"consola.d" -MT"consola.o" -o "consola.o" "../consola.c"
	@gcc  -o "Consola"  ./consola.o   -lreadline -lcommons -lutils
	@echo "Consola listorti"

clean:
	@rm -r debug_terminal
	@echo "Consola arafue"