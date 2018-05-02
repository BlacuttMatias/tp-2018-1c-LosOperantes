COMPILER=gcc

all:
	$(COMPILER) -c -g sockets.c
	$(COMPILER) -c -g funciones.c
	$(COMPILER) -o planificador planificador.c sockets.o funciones.o  -lcommons -lpthread -lreadline -lparsi
	$(COMPILER) -o instancia instancia.c sockets.o funciones.o  -lcommons -lparsi
	$(COMPILER) -o coordinador coordinador.c sockets.o funciones.o  -lcommons -lpthread -lparsi
	$(COMPILER) -o esi esi.c sockets.o funciones.o  -lcommons -lparsi

clean:
	rm -rf bin

sockets:
	$(COMPILER) -c -g sockets.c
	
funciones:
	$(COMPILER) -c -g funciones.c
		
planificador:
	$(COMPILER) -o planificador planificador.c sockets.o funciones.o  -lcommons -lpthread -lreadline

instancia:
	$(COMPILER) -o instancia instancia.c sockets.o funciones.o  -lcommons

coordinador:
	$(COMPILER) -o coordinador coordinador.c sockets.o funciones.o  -lcommons -lpthread

esi:
	$(COMPILER) -o esi esi.c sockets.o funciones.o  -lcommons -lparsi
	
