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

#include <sys/types.h>
#include <dirent.h>

#include "funciones.h"
#include "registros.h"
#include "sockets.h"


/* ---------------------------------------- */

int main(int argc, char* argv[]){


    /* Creo la instancia del Archivo de Configuracion y del Log */
    cfg = config_create("config/config.cfg");
    infoLogger = log_create("log/instancia.log", "INSTANCIA", false, LOG_LEVEL_INFO);

    log_info(infoLogger, "Iniciando INSTANCIA" );
    

    // Me aseguro que la estructura de directorios del Punto de Montaje este creada previamente
    if(crearEstructuraDirectorios( config_get_string_value(cfg,"PUNTO_MONTAJE") ) ){
        log_info(infoLogger, "Se creo la estructura del Punto de Montaje" );
    }

	// -----------------------------------------------------------------------
	//    Prueba de funciones 1
	// -----------------------------------------------------------------------


	////////////////////////////////////////////

	  /* Con un puntero a DIR abro el directorio */
	  DIR *dir;
	  /* en *ent habrá información sobre el archivo que se está "sacando" a cada momento */
	  struct dirent *ent;

	  /* Empezaremos a leer en el directorio entradas */
	  dir = opendir (config_get_string_value(cfg,"PUNTO_MONTAJE"));

	  /* Miramos que no haya error */
	  if (dir == NULL)
	    error("No se puede abrir el directorio");

	  /* Una vez nos aseguramos de que no hay error... */
	  /* Leyendo uno a uno todos los archivos que hay */
	  while ((ent = readdir (dir)) != NULL)
	    {
	      /* Nos devolverá el directorio actual (.) y el anterior (..), como hace ls */
	      if ( (strcmp(ent->d_name, ".")!=0) && (strcmp(ent->d_name, "..")!=0) )
	    {
	      /* Una vez tenemos el archivo, lo pasamos a una función para procesarlo. */
	      procesoArchivo(ent->d_name);
	    }
	    }
	  closedir (dir);



	//////////////////////////////////////



	//PRUEBA TABLA ENTRADAS IMPLEMENTACION CON LISTAS + nueva funcion cargarTablaEntradas
	t_list* tablaEntradas = list_create(); //creo lista tabla de entradas
	
	//hardcodeo una instruccion
	Instruccion* nuevaInstruccion = NULL;
	Instruccion* otraInstruccion = NULL;
	nuevaInstruccion = malloc(sizeof(Instruccion));
	otraInstruccion = malloc(sizeof(Instruccion));

	//carga de datos
	nuevaInstruccion->dato = malloc(strlen("MESSI")+1);
	strcpy(nuevaInstruccion->dato, "MESSI");
	nuevaInstruccion->dato[strlen("MESSI")] = '\0';

	otraInstruccion->dato = malloc(strlen("NARDIELLO")+1);
	strcpy(otraInstruccion->dato, "NARDIELLO");
	otraInstruccion->dato[strlen("NARDIELLO")] = '\0';


	//carga de claves
	strcpy(nuevaInstruccion->key,"FUTBOL");
	strcpy(otraInstruccion->key,"OPERATIVOS");
	//carga de operaciones
	nuevaInstruccion->operacion = 1;
	otraInstruccion->operacion = 2;
	

	//cargo tabla con entrada hardcodeada
	cargarTablaEntradas(tablaEntradas,nuevaInstruccion);
	cargarTablaEntradas(tablaEntradas,otraInstruccion);

	t_entrada* primerElemento;
	t_entrada* segundoElemento;
	primerElemento = list_get(tablaEntradas,0);
	segundoElemento = list_get(tablaEntradas,1);
	
	//muestro entrada hardcodeada
  	printf("Clave:%s - Valor:%s - Numero:%d - Tamanio:%d \n",primerElemento->clave,primerElemento->valor,primerElemento->numeroDeEntrada,primerElemento->tamanioValorAlmacenado); //prueba imprimir por pantalla el elemento obtenido

    // Persisto dos Entradas
    //persistirEntrada(primerElemento);
    //persistirEntrada(segundoElemento);

  	//testeo de dump
  	dump(tablaEntradas);
  	
  	

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
//    Prueba de funciones 2
// -----------------------------------------------------------------------

    Instruccion* datosInstruccion;


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

