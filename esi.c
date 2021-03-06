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

    // Creo conexión con Planificador
    int planificador_fd = conectarseAservidor(config_get_string_value(cfg,"PLANIFICADOR_IP"),config_get_int_value(cfg,"PLANIFICADOR_PUERTO"));

    if(planificador_fd == -1){
        printf("Error de conexion con el Planificador\n");
        return EXIT_FAILURE;
    }else{
        log_info(infoLogger, "Conexion establecida con el Planificador");
    }

    FD_SET(coordinador_fd, &master);
    FD_SET(planificador_fd, &master);
    fd_maximo = planificador_fd;


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

    ResultadoEjecucion registroResultadoEjecucion;
    bool continuarEjecutandoESI;

    while(1){
    	temporales=master;

    	if(select(fd_maximo+1,&temporales,NULL,NULL,NULL)==-1){
    		perror("select");
    		exit (1);
    	}
    	for(i = 0; i <= fd_maximo; i++) {
            if (FD_ISSET(i, &temporales)) { // ¡¡tenemos datos!!

				encabezado=recibir_header(&i);

				// gestionar datos de un cliente
				if ((nbytes = encabezado.tam_payload) <= 0) {
					 // error o conexión cerrada por el cliente
					 if (nbytes == 0) {
					 // conexión cerrada
					 	printf("El socket %d se ha caído\n", i);
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

                                paquete=recibir_payload(&i,&encabezado.tam_payload);
                                registroResultadoEjecucion=dsrlz_resultadoEjecucion(paquete.buffer);
                                free(paquete.buffer);

                                // Dependiendo el estado de la ejecucion de la instruccion
                                switch(registroResultadoEjecucion.resultado){

                                    case EJECUCION_EXITOSA:

                                        log_info(infoLogger,"Respuesta sobre la Ejecución EXITOSA de la Instruccion recibida por el Coordinador. Clave: %s", registroResultadoEjecucion.key);

                                        // Se elimina la Instruccion Ejecutada de la Lista
                                        eliminarUltimaInstruccion(listaInstrucciones);

                                        printf(" - OK\n");

                                        continuarEjecutandoESI = true;
                                        break;

                                    case EJECUCION_FALLIDA:

                                        printf(" - FALLO\n");

                                        log_info(infoLogger,"Respuesta sobre la Ejecución FALLIDA de la Instruccion recibida por el Coordinador. Clave: %s", registroResultadoEjecucion.key);

                                        continuarEjecutandoESI = true;
                                        break;

                                    case EJECUCION_FALLIDA_FINALIZAR_ESI:
                                        log_error(infoLogger, "El ESI %s finaliza por el ERROR: %s .", nombreProceso, registroResultadoEjecucion.contenido);

                                        continuarEjecutandoESI = false;
                                        break;
                                }


                                if(continuarEjecutandoESI){

                                    // Armo el Paquete del Resultado de la Ejecucion de la Instruccion
                                    paquete = crearHeader('E', RESPUESTA_EJECUTAR_INSTRUCCION, registroResultadoEjecucion.resultado);

                                    // Envio el Paquetea al Planificador
                                    if(send(planificador_fd,paquete.buffer,paquete.tam_buffer,0) != -1){

                                        free(paquete.buffer);
                                        log_info(infoLogger, "Se le respondio al PLANIFICADOR el resultado de la ejecucion de la Instruccion");
                                    }else{
                                        log_error(infoLogger, "No se pudo enviar mensaje al PLANIFICADOR sobre el resultado de la ejecucion de la Instruccion");
                                    }

                                }else{

                                    // Armo el Paquete de Finalizacion de Ejecucion del Proceso ESI
                                    paquete = crearHeader('E', FINALIZACION_EJECUCION_ESI, 1);

                                    // Notifico al Planificador que voy a finalizar
                                    if(send(planificador_fd,paquete.buffer,paquete.tam_buffer,0) != -1){

                                        log_info(infoLogger, "Se le notifico al PLANIFICADOR que el ESI %s finalizo", nombreProceso);

                                        // Notifico al Coordinador que voy a finalizar
                                        if(send(coordinador_fd,paquete.buffer,paquete.tam_buffer,0) != -1){
                                            log_info(infoLogger, "Se le notifico al COORDINADOR que el ESI %s finalizo", nombreProceso);
                                        }else{
                                            log_error(infoLogger, "No se pudo enviar al COORDINADOR la notificacion de finalizacion del ESI %s", nombreProceso);
                                        }
                                    }else{
                                        log_error(infoLogger, "No se pudo enviar al PLANIFICADOR la notificacion de finalizacion del ESI %s", nombreProceso);
                                    }

                                    // Libero Buffer
                                    free(paquete.buffer);                                        

                                    printf("\n\nProceso %s finalizado con ERROR: %s.\n", nombreProceso, registroResultadoEjecucion.contenido);
                                    return EXIT_FAILURE;                                    
                                }
                                break;
						}
                    }

                    // Si el mensaje proviene del PLANIFICADOR
                    if(encabezado.proceso == 'P'){

                        switch(encabezado.cod_operacion){

                            case EJECUTAR_INSTRUCCION:

                                log_info(infoLogger,"Pedido de Ejecución de Instruccion recibido del Planificador.");

                                // Se obtiene la Proxima Instruccion a Ejecutar
                                Instruccion* aux= obtenerSiguienteInstruccion(listaInstrucciones);

                                // Si se obtuvo una Proxima Instrucion
                                if(NULL != aux){

                                    Instruccion proximaInstruccion;
                                    proximaInstruccion=pasarAEstructura(aux, nombreProceso);

                                    // Mostrando por pantalla la Instruccion a Ejecutar
                                    switch(proximaInstruccion.operacion){
                                        case GET:
                                            printf("Ejecutando... GET %s %s ",proximaInstruccion.key, proximaInstruccion.dato);
                                            break;
                                        case SET:
                                            printf("Ejecutando... SET %s %s ",proximaInstruccion.key, proximaInstruccion.dato);
                                            break;
                                        case STORE:
                                            printf("Ejecutando... STORE %s %s ",proximaInstruccion.key, proximaInstruccion.dato);
                                            break;                                      
                                    }

                                    // Armo el Paquete de Ejecucion de la Proxima Instruccion
                                    paquete = srlz_instruccion('E', EJECUTAR_INSTRUCCION,proximaInstruccion);

                                    // Envio el Paquetea al Coordinador
                                    if(send(coordinador_fd,paquete.buffer,paquete.tam_buffer,0) != -1){

                                        free(paquete.buffer);
                                        log_info(infoLogger, "Se le envio al COORDINADOR la proxima Instrucción a ejecutar. Key: %s - Operacion: %d - Dato: %s", proximaInstruccion.key, proximaInstruccion.operacion, proximaInstruccion.dato);
                                    }else{
                                        log_error(infoLogger, "No se pudo enviar al COORDINADOR la proxima Instrucción a ejecutar");
                                    }
                                }else{
                                    log_info(infoLogger, "El ESI %s ya no posee más Instrucciones ha ejecutar.", nombreProceso);

                                    // Armo el Paquete de Finalizacion de Ejecucion del Proceso ESI
                                    paquete = crearHeader('E', FINALIZACION_EJECUCION_ESI, 1);

                                    // Notifico al Planificador que voy a finalizar
                                    if(send(planificador_fd,paquete.buffer,paquete.tam_buffer,0) != -1){

                                        log_info(infoLogger, "Se le notifico al PLANIFICADOR que el ESI %s finalizo", nombreProceso);


                                        // Notifico al Coordinador que voy a finalizar
                                        if(send(coordinador_fd,paquete.buffer,paquete.tam_buffer,0) != -1){

                                            free(paquete.buffer);
                                            log_info(infoLogger, "Se le notifico al COORDINADOR que el ESI %s finalizo", nombreProceso);

                                            printf("Proceso %s finalizado.\n", nombreProceso);
                                            return EXIT_SUCCESS;

                                        }else{
                                            log_error(infoLogger, "No se pudo enviar al COORDINADOR la notificacion de finalizacion del ESI %s", nombreProceso);
                                        }

                                        printf("Proceso %s finalizado con ERROR.\n", nombreProceso);
                                        return EXIT_FAILURE;
                                    }else{
                                        log_error(infoLogger, "No se pudo enviar al PLANIFICADOR la notificacion de finalizacion del ESI %s", nombreProceso);

                                        printf("Proceso %s finalizado con ERROR.\n", nombreProceso);
                                        return EXIT_FAILURE;
                                    }
                                }
                                break;

                            case ESI_MUERE:

                                log_info(infoLogger, "El PLANIFICADOR envió comando KILL");
                                paquete= srlz_datosProceso('E', ESI_MUERE,nombreProceso, ESI, 0 );
                                

                                if(send(coordinador_fd, paquete.buffer, paquete.tam_buffer,0) != -1){
                                    log_info(infoLogger,"Se avisó al COORDINADOR sobre el fin de esi");
                                }
                                else {
                                    log_info(infoLogger,"no se pudo avisar al COORDINADOR fin de esi");
                                }

                                // Libero los Recursos
                                free(paquete.buffer);

                                return EXIT_FAILURE;

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
    list_destroy(listaInstrucciones);

    return EXIT_SUCCESS;
}
