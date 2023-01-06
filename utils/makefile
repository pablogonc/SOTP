install: compile
	sudo cp -u libutils.so /usr/lib
	cd ..  && sudo cp --parents -u utils/connection/connection.h /usr/include 
	cd ..  && sudo cp --parents -u utils/globals.h /usr/include 

uninstall:
	sudo rm -f ./libutils.so
	sudo rm -f /usr/lib/libutils.so
	sudo rm -rf /usr/include/utils

compile:
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"connection/connection.d" -MT"connection/connection.o" -o "connection/connection.o" "./connection/connection.c"
	gcc -shared -o "libutils.so"  ./connection/connection.o   -lpthread