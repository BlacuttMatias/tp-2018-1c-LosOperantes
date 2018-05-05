#include <arpa/inet.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

#include "funciones.h"
#include "registros.h"
#include "sockets.h"

/* ---------------------------------------- */
/*  Variables Globales 						*/
/* ---------------------------------------- */

t_list* listaInstrucciones;

/* ---------------------------------------- */



Paquete srlz_instruccion (char proceso, int codigoOperacion,Instruccion instruccion){

	const int get=1;
	const int set=2;		//se trabaja suponiendo que los codigos de operacion de las instrucciones son estos (1 para get, 2 para set y 3 para store)
	const int store=3;

	int posicion = 0;//int para ir guiando desde donde se copia
	int sizeBuffer = 0;
	int tamClave = 0;
	int tamPayload = 0;
	int tamDato = 0;
	Paquete paquete;

	tamClave = strlen(instruccion.key);

	//dependiendo de si se hace un get/store o un set, el tamaño del buffer sera uno u otro, porque en el set se agrega tambien el valor asociado a la key
	if(instruccion.operacion == get || instruccion.operacion == store){
		sizeBuffer = sizeof(int)*4 + sizeof(char) + tamClave;
	}
	else{
		tamDato = strlen(instruccion.dato);
		sizeBuffer = sizeof(int)*5 + sizeof(char) + tamClave + tamDato;	//agrego el tamaño del valor y el valor(el dato de la instruccion)
	}
	paquete.tam_buffer = sizeBuffer;
	paquete.buffer = malloc( sizeBuffer );
	tamPayload = sizeBuffer - (sizeof(int)*2) - sizeof(char);

	memcpy(paquete.buffer									,&(proceso)                     ,sizeof(char));
	memcpy(paquete.buffer + (posicion=sizeof(char))			,&(codigoOperacion)				,sizeof(int));
	memcpy(paquete.buffer + (posicion += sizeof(int))		,&(tamPayload)					,sizeof(int));

	memcpy(paquete.buffer + (posicion+=sizeof(int))			,&(instruccion.operacion)		,sizeof(int));

	memcpy(paquete.buffer + (posicion += sizeof(int))		,&(tamClave)					,sizeof(int));
	memcpy(paquete.buffer + (posicion += sizeof(int))		,instruccion.key				,tamClave);

	if(instruccion.operacion==set){	//en caso ser un set, pongo en el buffer el tamaño del dato y el dato, en get y store no hace falta porque el dato no existe
		memcpy(paquete.buffer + (posicion += tamClave)			,&(tamDato)						,sizeof(int));
		memcpy(paquete.buffer + (posicion += sizeof(int))		,instruccion.dato				,tamDato);
	}
	return paquete;

}


Instruccion dsrlz_instruccion (void* buffer){

	const int get=1;
	const int set=2;		//se trabaja suponiendo que los codigos de operacion de las instrucciones son estos (1 para get, 2 para set y 3 para store)
	const int store=3;

	int posicion = 0; //int para ir guiando desde donde se copia
	int tamClave = 0;
	Instruccion instruccion;

	memcpy(&instruccion.operacion			,buffer + posicion										,sizeof(int));
	memcpy(&tamClave						,buffer + (posicion+=sizeof(int))						,sizeof(int));
	memcpy(instruccion.key					,buffer + (posicion+=sizeof(int))						,tamClave);
	instruccion.key[tamClave] = '\0';

	if(instruccion.operacion==set){	//si es un set significa que tengo que seguir leyendo del buffer el valor asociado a la key(el dato)

		int tamDato = 0;
		memcpy(&tamDato							,buffer + (posicion+=tamClave)							,sizeof(int));
		instruccion.dato = malloc(tamDato);
		memcpy(instruccion.dato					,buffer + (posicion+=sizeof(int))						,tamDato);
		instruccion.dato[tamDato] = '\0';

	}
	else instruccion.dato = NULL;

	return instruccion;
}


int main(int argc, char* argv[]){


	/* 	//codigo para probar la serializacion y descerializacion de una instruccion

	Instruccion ins2;	//instruccion de prueba con valores cualquiera
	ins2.operacion = 1;
	strcpy(ins2.key,"key");
	ins2.dato = "valor";
	Paquete pac;
	pac = srlz_instruccion('p',1,ins2);
	Instruccion ins = dsrlz_instruccion(pac.buffer+sizeof(char)+8);
	printf("%s\n",ins.key);
	mostrarInstruccion(&ins);

	exit(0);
	*/

    char* nombreProceso = string_new();
    char* pathScript = string_new();

    /*************************************************
     *
     * Se obtienen parametros por consola
     *
     ************************************************/
    if(argc > 1){
        string_append(&nombreProceso,argv[1]);
        string_append(&pathScript,argv[2]);
    }else{
        printf("Error de formato\n\nForma de Uso:\n ./esi [nombre_proceso] [path_completo_script]\n");
        return EXIT_FAILURE;
    }

    /*************************************************
     *
     * Si no existe el archivo del Script, cierro el proceso
     *
     ************************************************/
    if (!existeArchivo(pathScript)){
        printf("El Script %s no existe\n", pathScript);
        return EXIT_FAILURE;
    }

	/* Creo la instancia del Archivo de Configuracion y del Log */
	cfg = config_create("config/config.cfg");
	infoLogger = log_create("log/esi.log", "ESI", false, LOG_LEVEL_INFO);

	log_info(infoLogger, "Iniciando ESI - Proceso %s", nombreProceso );
	printf("Iniciando ESI - Proceso %s\n", nombreProceso);

    // Creo la lista de las Instrucciones del Proceso
    listaInstrucciones = list_create();

	Encabezado encabezado;
	Paquete paquete;
    struct sockaddr_in servidor_addr,my_addr,master_addr; // información de la dirección de destino
    int servidor, numbytes,escucha_master,fd_maximo,nuevo_fd,i,size, nbytes;
    fd_set master,temporales;
    FD_ZERO(&master);
    FD_ZERO(&temporales);

    // Creo conexión con Planificador
    int planificador_fd = conectarseAservidor(config_get_string_value(cfg,"PLANIFICADOR_IP"),config_get_int_value(cfg,"PLANIFICADOR_PUERTO"));

    if(planificador_fd == -1){
        printf("Error de conexion con el Planificador\n");
        return EXIT_FAILURE;
    }else{
        log_info(infoLogger, "Conexion establecida con el Planificador");
    }

    // Creo conexión con el Coordinador
    int coordinador_fd = conectarseAservidor(config_get_string_value(cfg,"COORDINADOR_IP"),config_get_int_value(cfg,"COORDINADOR_PUERTO"));

    if(coordinador_fd == -1){
        printf("Error de conexion con el Coordinador\n");
        return EXIT_FAILURE;
    }else{
        log_info(infoLogger, "Conexion establecida con el Coordinador");
    }

    FD_SET(planificador_fd, &master);
    FD_SET(coordinador_fd, &master);
    fd_maximo = coordinador_fd;

// -----------------------------------------------------------------------
//    Prueba de funciones
// -----------------------------------------------------------------------
    // Si se pudieron cargar todas las instrucciones en la Lista
    if(procesarScript(pathScript, listaInstrucciones)){

        // Serializado el Proceso
        paquete = srlz_datosProceso('E', HANDSHAKE, nombreProceso, ESI, 0);

        // Envio al Planificador el Handshake y Serializado el Proceso
        send(planificador_fd,paquete.buffer,paquete.tam_buffer,0);

        // Envio al Coordinador el Handshake y Serializado el Proceso
        send(coordinador_fd,paquete.buffer,paquete.tam_buffer,0);
        free(paquete.buffer);


    }else{ // Si fallo el proceso

    }

    free(pathScript);

    //Probando la fc sacarSiguienteInstruccion
/*    puts("anduvo todo, pruebo sacar instruccion \n \n");
    Instruccion proximaInstruccion;
    proximaInstruccion.operacion=20;
    strcpy(proximaInstruccion.key, "BALALALALALA");
    proximaInstruccion.dato= "jorojojojo";
    sacarSiguienteInstruccion(listaInstrucciones,&proximaInstruccion);
    puts("se pudo sacar instruccion, se chequea que sea lo correcto \n\n");
    printf("codigo op es: %d \n\n key es %s \n\n",proximaInstruccion.operacion,proximaInstruccion.key);
    if(proximaInstruccion.dato != NULL){
    	printf("dato es: %s \n\n",proximaInstruccion.dato);
    }
    puts("termino de mostrar");
*/
// -----------------------------------------------------------------------



    while(1){
    	temporales=master;

    	if(select(fd_maximo+1,&temporales,NULL,NULL,NULL)==-1){
    		perror("select");
    		exit (1);
    	}
    	for(i = 0; i <= fd_maximo; i++) {
            if (FD_ISSET(i, &temporales)) { // ¡¡tenemos datos!!
                if (i == escucha_master) {
                    // gestionar nuevas conexiones
                    size = sizeof(master_addr);
                    if ((nuevo_fd = accept(escucha_master, (struct sockaddr *)&master_addr,
                                                             &size)) == -1) {
                        perror("accept");
                    } else {
                        FD_SET(nuevo_fd, &master); // añadir al conjunto maestro
                        if (nuevo_fd > fd_maximo) {    // actualizar el máximo
                            fd_maximo = nuevo_fd;
                        }
						log_info(infoLogger, "Conexion nueva recibida" );
						printf("Conexion nueva recibida\n");
                    }
                } else {

					encabezado=recibir_header(&i);

					// gestionar datos de un cliente
					if ((nbytes = encabezado.tam_payload) <= 0) {
						 // error o conexión cerrada por el cliente
						 if (nbytes == 0) {
						 // conexión cerrada
						 	printf("selectserver: socket %d se ha caído\n", i);
						 } else {
						 	perror("recv");
						 }
						 close(i); // ¡Hasta luego!
						 FD_CLR(i, &master); // eliminar del conjunto maestro
					} else {

						switch(encabezado.cod_operacion){

							case EJECUTAR_INSTRUCCION:
								log_info(infoLogger,"Pedido de Ejecución de Instruccion.");
								break;

						}
					}
                }
            }
        }
    }

    /* ---------------------------------------- */
    /*  Libero Memoria de Log y Config          */
    /* ---------------------------------------- */
    log_destroy(infoLogger);
    config_destroy(cfg);
    free(nombreProceso);

    close(servidor);

    return EXIT_SUCCESS;
}

