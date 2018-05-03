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
#include <pthread.h>

#include "funciones.h"
#include "registros.h"
#include "sockets.h"

/* ---------------------------------------- */
/*  Variables Globales                      */
/* ---------------------------------------- */

    t_list* listaProcesosConectados;

/* ---------------------------------------- */

void servidorCoordinador(void* puerto){

    Encabezado encabezado;
    Paquete paquete;
    struct sockaddr_in servidor_addr,my_addr,master_addr; // información de la dirección de destino
    int servidor,escucha_master,fd_maximo,nuevo_fd,i,size, nbytes;

    fd_set master,temporales;
    FD_ZERO(&master);
    FD_ZERO(&temporales);

    // Creo el Servidor para escuchar conexiones
    servidor=crearServidor((int)puerto);
    log_trace(infoLogger, "Escuchando conexiones" );

    FD_SET(servidor, &master);
    fd_maximo = servidor;   


    Proceso registroProceso;
    int indice = 0;

    while(1){

        temporales=master;

        if (select(fd_maximo + 1, &temporales, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(1);
        }

        for(i = 0; i <= fd_maximo; i++) {
            if (FD_ISSET(i, &temporales)) { // ¡¡tenemos datos!!
                if (i == servidor) {
                    // gestionar nuevas conexiones
                    size = sizeof(master_addr);
                    if ((nuevo_fd = accept(servidor, (struct sockaddr *)&master_addr,
                                                             &size)) == -1) {
                        perror("accept");
                    } else {
                        FD_SET(nuevo_fd, &master); // añadir al conjunto maestro
                        if (nuevo_fd > fd_maximo) {    // actualizar el máximo
                            fd_maximo = nuevo_fd;
                        }
                    }
                } else { // Atiendo las conexiones ya establecidas, es decir, clientes

                    // Recibo el Encabezado del Paquete
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

                        // Si el mensaje proviene del Planificador
                        if(encabezado.proceso == 'P'){
                            switch(encabezado.cod_operacion){

                                case HANDSHAKE:

                                    // Recibo los datos del Proceso
                                    paquete = recibir_payload(&i,&encabezado.tam_payload);
                                    registroProceso = dsrlz_datosProceso(paquete.buffer);
                                    free(paquete.buffer);

                                    // Recibo de Planificador el nombre del Proceso
                                    log_info(infoLogger,"Proceso Planificador conectado.");
                                    break;
                            }
                        }

                        // Si el mensaje proviene de la Instancia
                        if(encabezado.proceso == 'I'){
                            switch(encabezado.cod_operacion){

                                case HANDSHAKE:
                                    // Recibo los datos del Proceso
                                    paquete = recibir_payload(&i,&encabezado.tam_payload);
                                    registroProceso = dsrlz_datosProceso(paquete.buffer);
                                    free(paquete.buffer);

                                    // Recibo de Instancia el nombre del Proceso
                                    log_info(infoLogger,"Proceso Instancia %s conectado.", registroProceso.nombreProceso);
                                    break;
                            }
                        }

                        // Si el mensaje proviene de ESI
                        if(encabezado.proceso == 'E'){
                            switch(encabezado.cod_operacion){

                                case HANDSHAKE:

                                    // Recibo los datos del Proceso
                                    paquete = recibir_payload(&i,&encabezado.tam_payload);
                                    registroProceso = dsrlz_datosProceso(paquete.buffer);
                                    free(paquete.buffer);

                                    // Recibo de ESI el nombre del Proceso
                                    log_info(infoLogger,"Proceso ESI %s conectado.", registroProceso.nombreProceso);

                                    // Cargo el Registro del Proceso
                                    Proceso* registroProcesoAux = NULL;
                                    registroProcesoAux = malloc(sizeof(Proceso));

                                    // Cargo el Registro del Proceso
                                    registroProcesoAux->tipoProceso = registroProceso.tipoProceso;
                                    registroProcesoAux->socketProceso = registroProceso.socketProceso;
                                    registroProcesoAux->nombreProceso = malloc(strlen(registroProceso.nombreProceso)+1);
                                    strcpy( registroProcesoAux->nombreProceso ,registroProceso.nombreProceso);
                                    registroProcesoAux->nombreProceso[strlen(registroProceso.nombreProceso)] = '\0';

                                    // Cargo el Proceso en la listaReady
                                    //list_add(listaReady, registroProcesoAux);

                                    // Muestro por pantalla el contenido de la listaReady
                                    //showContenidolistaReady(listaReady);
                                    break;
                            }
                        }

                    }
                }
            }
        }
    }

    close(servidor);
    FD_CLR(servidor, &master);
}


int main(int argc, char* argv[]){

	/* Creo la instancia del Archivo de Configuracion y del Log */
	cfg = config_create("config/config.cfg");
	infoLogger = log_create("log/coordinador.log", "Coordinador", false, LOG_LEVEL_INFO);

	log_info(infoLogger, "Iniciando COORDINADOR" );
    printf("Iniciando COORDINADOR\n");

    // Creo las Listas de los Procesos Conectados al Planificador
    listaProcesosConectados = list_create();

// -----------------------------------------------------------------------
//    Prueba de funciones
// -----------------------------------------------------------------------

/*
    inicializarEstructurasAdministrativas();


    Instruccion* datosInstruccion;

    datosInstruccion->operacion = STORE;
    strcpy(datosInstruccion->key, "clave\0");
    datosInstruccion->dato=NULL;

    if(registrarLogOperaciones(datosInstruccion, "PROCESO1")){

    }

    // Defino el Algoritmo de Distribucion a utlizar
    char* algoritmoDistribucion = string_new();
    string_append(&algoritmoDistribucion,"CIRCULAR");
    char* instanciaElegida = string_new();

    instanciaElegida = procesarSolicitudEjecucion(datosInstruccion, algoritmoDistribucion);

    free(algoritmoDistribucion);
    free(instanciaElegida);
*/
// -----------------------------------------------------------------------

    pthread_t hiloServidor;
    pthread_create(&hiloServidor, NULL, (void*) servidorCoordinador, (void*)config_get_int_value(cfg,"COORDINADOR_PUERTO"));

    pthread_join(hiloServidor, NULL);

    /* ---------------------------------------- */
    /*  Libero Memoria de Log y Config          */
    /* ---------------------------------------- */
    log_destroy(infoLogger);
    config_destroy(cfg);

    list_destroy(listaProcesosConectados);

    return EXIT_SUCCESS;
}
