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


int main(int argc, char* argv[]){


	 	//codigo para probar la serializacion y descerializacion de una instruccion
/*
	Instruccion ins2;	//instruccion de prueba con valores cualquiera
	ins2.operacion = 1;
	strcpy(ins2.key,"key");
	ins2.dato = "valor";
	Paquete pac;
	pac = srlz_instruccion('p',1,ins2);
	Instruccion ins = dsrlz_instruccion(pac.buffer+sizeof(char)+8);
	//printf("%s\n",ins.key);
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
        return EXIT_FAILURE;
    }

    free(pathScript);

    //Probando la fc sacarSiguienteInstruccion
/*
    Instruccion* pproximaInstruccion;
    Instruccion pruebaPasarStruct;
    puts(" \n ");
    pproximaInstruccion=sacarSiguienteInstruccion(listaInstrucciones);
   mostrarInstruccion(pproximaInstruccion);
   pproximaInstruccion=sacarSiguienteInstruccion(listaInstrucciones);
     mostrarInstruccion(pproximaInstruccion);
    pruebaPasarStruct= pasarAEstructura(pproximaInstruccion);
   mostrarInstruccion(&pruebaPasarStruct);
   */
// TODO

   /**/
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

                        // Si el mensaje proviene de COORDINADOR
                        if(encabezado.proceso == 'C'){

    						switch(encabezado.cod_operacion){

                                case RESPUESTA_EJECUTAR_INSTRUCCION:
                                    log_info(infoLogger,"Respuesta sobre la Ejecución de Instruccion recibida del Coordinador.");


                                    // Armo el Paquete del Resultado de la Ejecucion de la Instruccion
                                    paquete = crearHeader('E', RESPUESTA_EJECUTAR_INSTRUCCION, 1);

                                    // Envio el Paquetea al Planificador
                                    if(send(planificador_fd,paquete.buffer,paquete.tam_buffer,0) != -1){

                                        free(paquete.buffer);
                                        log_info(infoLogger, "Se le respondio al PLANIFICADOR el resultado de la ejecucion de la Instruccion");
                                    }else{
                                        log_error(infoLogger, "No se pudo enviar mensaje al PLANIFICADOR sobre el resultado de la ejecucion de la Instruccion");
                                    }

                                    break;
    						}
                        }

                        // Si el mensaje proviene del PLANIFICADOR
                        if(encabezado.proceso == 'P'){

                            switch(encabezado.cod_operacion){

                                case EJECUTAR_INSTRUCCION:

                                    log_info(infoLogger,"Pedido de Ejecución de Instruccion recibido del Planificador.");

                                    // TODO
                                    Instruccion* aux= sacarSiguienteInstruccion(listaInstrucciones);

                                    // Si se obtuvo una Proxima Instrucion
                                    if(NULL != aux){

                                        Instruccion proximaInstruccion;
                                        proximaInstruccion=pasarAEstructura(aux);

                                        // Armo el Paquete de Ejecucion de la Proxima Instruccion
                                        paquete = srlz_instruccion('E', EJECUTAR_INSTRUCCION,proximaInstruccion);

                                        //printf("\n\n codigo %d    key  %s    dato %s  \n\n",proximaInstruccion.operacion,proximaInstruccion.key,proximaInstruccion.dato);

                                        // Envio el Paquetea al Planificador
                                        if(send(coordinador_fd,paquete.buffer,paquete.tam_buffer,0) != -1){

                                            free(paquete.buffer);
                                            log_info(infoLogger, "Se le envio al COORDINADOR la proxima Instruccion a ejecutar");
                                        }else{
                                            log_error(infoLogger, "No se pudo enviar al COORDINADOR la proxima Instruccion a ejecutar");
                                        }
                                    }else{
                                        log_info(infoLogger, "El ESI %s ya no posee mas Instrucciones ha ejecutar.", nombreProceso);

                                        // Armo el Paquete de Finalizacion de Ejecucion del Proceso ESI
                                        paquete = crearHeader('E', FINALIZACION_EJECUCION_ESI, 1);

                                        // Notifico al Planificador que voy a finalizar
                                        if(send(planificador_fd,paquete.buffer,paquete.tam_buffer,0) != -1){

                                            free(paquete.buffer);
                                            log_info(infoLogger, "Se le notifico al PLANIFICADOR que el ESI %s finalizo", nombreProceso);

                                            return EXIT_SUCCESS;
                                        }else{
                                            log_error(infoLogger, "No se pudo enviar al PLANIFICADOR la notificacion de finalizacion del ESI %s", nombreProceso);
                                        }
                                    }
                                    break;
                            }
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

