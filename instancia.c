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


//prueba persistir una entrada
void persistirEntrada(t_entrada* unaEntrada){

	FILE* archivoTexto;

	char *nombreEntrada;
	strcpy(nombreEntrada,unaEntrada->clave);
	char *nombre_formato_archivo = strcat(nombreEntrada,".txt"); //clave como nombre de archivo mas formato ".txt"

	archivoTexto = fopen(nombre_formato_archivo,"w+"); //abrimos / creamos archivo en carpeta RAIZ (hay que cambiar esto)


	char *valorIdentificado = unaEntrada->valor;

	fputs(valorIdentificado, archivoTexto );

	fclose ( archivoTexto );
}


/* ---------------------------------------- */

int main(int argc, char* argv[]){

	/* Creo la instancia del Archivo de Configuracion y del Log */
	cfg = config_create("config/config.cfg");
	infoLogger = log_create("log/instancia.log", "INSTANCIA", false, LOG_LEVEL_INFO);

	log_info(infoLogger, "Iniciando INSTANCIA" );
	
	//PRUEBA TABLA ENTRADAS IMPLEMENTACION CON LISTAS + nueva funcion cargarTablaEntradas
	t_list* tablaEntradas = list_create(); //creo lista tabla de entradas
	
	//hardcodeo una instruccion
	Instruccion* nuevaInstruccion;
	nuevaInstruccion->dato = "MESSI";
	strcpy(nuevaInstruccion->key,"FUTBOL");
	nuevaInstruccion->operacion = 1;
	
	//cargo tabla con entrada hardcodeada
	cargarTablaEntradas(tablaEntradas,nuevaInstruccion);
	t_entrada* primerElemento;
	primerElemento = list_get(tablaEntradas,0);
	
	//muestro entrada hardcodeada
  	printf("Clave:%s - Valor:%s - Numero:%d - Tamanio:%d \n",primerElemento->clave,primerElemento->valor,primerElemento->numeroDeEntrada,primerElemento->tamanioValorAlmacenado); //prueba imprimir por pantalla el elemento obtenido

	printf("Iniciando INSTANCIA\n");

	
	
	
	
	

	Encabezado encabezado;
	Paquete paquete;
    struct sockaddr_in servidor_addr,my_addr,master_addr; // información de la dirección de destino
    int numbytes,escucha_master,fd_maximo,nuevo_fd,i,size, nbytes;
    fd_set master,temporales;
    FD_ZERO(&master);
    FD_ZERO(&temporales);

    // Creo conexión con el Coordinador
    int coordinador_fd = conectarseAservidor(config_get_string_value(cfg,"COORDINADOR_IP"),config_get_int_value(cfg,"COORDINADOR_PUERTO"));

    if(coordinador_fd == -1){
        printf("Error de conexion con el Coordinador\n");
        return EXIT_FAILURE;        
    }else{
        log_info(infoLogger, "Conexion establecida con el Coordinador");        
    }


    // Serializado el Proceso
    paquete = srlz_datosProceso('I', HANDSHAKE, config_get_string_value(cfg,"INSTANCIA_NOMBRE"), INSTANCIA, 0);

    // Envio al Coordinador el Handshake y Serializado el Proceso
    send(coordinador_fd,paquete.buffer,paquete.tam_buffer,0);
    free(paquete.buffer);

    FD_SET(coordinador_fd, &master);
    fd_maximo = coordinador_fd;   


// -----------------------------------------------------------------------
//    Prueba de funciones
// -----------------------------------------------------------------------


    Instruccion* datosInstruccion;
    
/*
    datosInstruccion->operacion = STORE;
    strcpy(datosInstruccion->key, "clave\0");
    datosInstruccion->dato=NULL;
*/

    //datosInstruccion->texto_instruccion = malloc(strlen("STORE clave")+1);
    //strcpy( datosInstruccion->texto_instruccion ,"STORE clave");
    //datosInstruccion->texto_instruccion[strlen("STORE clave")] = '\0';

    // Defino el Algoritmo de Almacenamiento a utlizar
    char* algoritmoAlmacenamiento = string_new();
    string_append(&algoritmoAlmacenamiento,"CIRCULAR");


    if(persistirDatos(datosInstruccion, algoritmoAlmacenamiento)){
        // Proceso a realizar si se persistiron correctamente los datos
    }else{
        // Proceso a realizar si fallo la persistencia
    }

    free(algoritmoAlmacenamiento);


// -----------------------------------------------------------------------

    while(1){
    	temporales=master;

    	if(select(fd_maximo+1,&temporales,NULL,NULL,NULL)==-1){
    		perror("select");
    		exit (1);
    	}
    	for(i = 0; i <= fd_maximo; i++) {
            if (FD_ISSET(i, &temporales)) { // ¡¡tenemos datos!!
                if (i == escucha_master) {
                    // gestionar nuevas conexiones
                    size = sizeof(master_addr);
                    if ((nuevo_fd = accept(escucha_master, (struct sockaddr *)&master_addr,
                                                             &size)) == -1) {
                        perror("accept");
                    } else {
                        FD_SET(nuevo_fd, &master); // añadir al conjunto maestro
                        if (nuevo_fd > fd_maximo) {    // actualizar el máximo
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
                            
                                // TODO

								log_info(infoLogger,"Pedido de Ejecución de Instruccion recibido del Coordinador.");
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

    return 0;
}
