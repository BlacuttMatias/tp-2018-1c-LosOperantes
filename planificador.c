#include <arpa/inet.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <netinet/in.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <readline/rltypedefs.h>
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

    t_queue* colaReady;
    t_queue* colaEjecucion;
    t_queue* colaBloqueados;
    t_queue* colaTerminados;
    t_list* listaClavesBloqueadas;
    t_list* listaReady;
    t_list* listaESIconectados;

    bool planificadorPausado = false;
    bool planificarProcesos = false;
    char* algoritmoPlanificacion = NULL;

/* ---------------------------------------- */
/*  Consola interactiva                     */
/* ---------------------------------------- */

void hiloConsolaInteractiva(void * unused) {

    // Mensaje de Bienvenida de la Consola
    printf("Bienvenido a la Consola Interactiva del Planificador 1.0\n");
    printf("Para obtener ayuda de los comandos permitidos, escriba 'help'.\n");
    log_trace(infoLogger, "Inicio de la Consola Interactiva" );

    char** parametrosConsola;
    char** parametrosConsolaOriginal;
    char* comandoConsolaOriginal;
    bool comandoAceptado;
    char* comandoConsola;

    while (1) {
        comandoAceptado = false;
        comandoConsola = readline("#> ");
        if (!comandoConsola || string_equals_ignore_case(comandoConsola, "")) {
            continue;
        } else {
            add_history(comandoConsola);

            //genero una copia del comando ingresado
            comandoConsolaOriginal = malloc(sizeof(char) * strlen(comandoConsola)+1);
            strcpy(comandoConsolaOriginal, comandoConsola);

            //convierto lo escrito en mayuscula para poder comparar
            string_to_upper(comandoConsola);

            // Elimino espacios a izquierda y derecha
            string_trim(&comandoConsola);

            /* -------------------------------------------------------------------- */
            // Defino los comandos aceptados por la Consola Interactiva
            /* -------------------------------------------------------------------- */

                // Reconozco los parametros ingresados
                parametrosConsola = string_split(comandoConsola, " ");
                parametrosConsolaOriginal = string_split(comandoConsolaOriginal, " ");
                free(comandoConsolaOriginal);

                if(string_starts_with(comandoConsola,"HELP")){
                    comandoAceptado = true;

                    printf("Los comandos aceptados por esta consola son:\n\n");

                    printf("\tPausar/Continuar \t- El Planificador no le dará nuevas órdenes de ejecución a ningún ESI mientras se encuentre pausado. \n\n");

                    printf("\n\tbloquear <clave> <ID> \t- Se bloqueará el proceso ESI hasta ser desbloqueado (ver más adelante), especificado por dicho <ID> en la cola del recurso <clave>. Vale recordar que cada línea del script a ejecutar es atómica, y no podrá ser interrumpida; si no que se bloqueará en la próxima oportunidad posible. Solo se podrán bloquear de esta manera ESIs que estén en el estado de listo o ejecutando. \n\n");

                    printf("\n\tdesbloquear <clave> \t- Se desbloqueara el proceso ESI con el ID especificado. Solo se bloqueará ESIs que fueron bloqueados con la consola. Si un ESI está bloqueado esperando un recurso, no podrá ser desbloqueado de esta forma. \n\n");

                    printf("\n\tlistar <recurso> \t- Lista los procesos encolados esperando al recurso. \n\n");

                    printf("\n\tkill <ID> \t- finaliza el proceso. Recordando la atomicidad mencionada en “bloquear”. \n\n");

                    printf("\n\tstatus <clave> \t- Debido a que para la correcta coordinación de las sentencias de acuerdo a los algoritmos de distribución se requiere de cierta información sobre las instancias del sistema, el Coordinador proporcionará una consola que permita consultar esta información. \n\n");

                    printf("\n\tdeadlock\t- Esta consola también permitirá analizar los deadlocks que existan en el sistema y a que ESI están asociados. Pudiendo resolverlos manualmente con la sentencia de kill previamente descrita.\n\n");
                }



                if(string_starts_with(comandoConsola,"PLANIFICAR")){
                    comandoAceptado = true;

                    colaReady =  planificarReady(listaReady, config_get_string_value(cfg,"ALGORITMO_PLANIFICACION"));
                    showContenidocolaReady(colaReady);

                }




                if(string_starts_with(comandoConsola,"PAUSAR")){
                    comandoAceptado = true;
                    printf("Comando no implementado...\n");
                }

                if(string_starts_with(comandoConsola,"CONTINUAR")){
                    comandoAceptado = true;
                    printf("Comando no implementado...\n");
                }

                if(string_starts_with(comandoConsola,"BLOQUEAR")){
                    comandoAceptado = true;
                    printf("Comando no implementado...\n");
                }

                if(string_starts_with(comandoConsola,"DESBLOQUEAR")){
                    comandoAceptado = true;
                    printf("Comando no implementado...\n");
                }

                if(string_starts_with(comandoConsola,"LISTAR")){
                    comandoAceptado = true;
                    printf("Comando no implementado...\n");
                }

                if(string_starts_with(comandoConsola,"KILL")){
                    comandoAceptado = true;
                    printf("Comando no implementado...\n");
                }

                if(string_starts_with(comandoConsola,"STATUS")){
                    comandoAceptado = true;
                    printf("Comando no implementado...\n");
                }

                if(string_starts_with(comandoConsola,"DEADLOCK")){
                    comandoAceptado = true;
                    printf("Comando no implementado...\n");
                }

            /* -------------------------------------------------------------------- */

            // Si el comando no es reconocido por la consola
            if(!comandoAceptado){
                printf("Comando no reconocido. Escriba 'help' para obtener ayuda.\n");
            }

            free(comandoConsola);
            free(parametrosConsola);
            free(parametrosConsolaOriginal);
        }
    }
}

void servidorPlanificador(void* puerto){

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
    Proceso* procesoSeleccionado;
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

                        // Si el mensaje proviene de ESI
                        if(encabezado.proceso == 'E'){
                            switch(encabezado.cod_operacion){

                                case HANDSHAKE:

                                    // Recibo los datos del Nodo
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
                                    registroProcesoAux->socketProceso = i;
                                    registroProcesoAux->nombreProceso = malloc(strlen(registroProceso.nombreProceso)+1);
                                    strcpy( registroProcesoAux->nombreProceso ,registroProceso.nombreProceso);
                                    registroProcesoAux->nombreProceso[strlen(registroProceso.nombreProceso)] = '\0';

                                    // Cargo el Proceso en la listaReady
                                    list_add(listaReady, registroProcesoAux);

                                    // Cargo los ESIs conectados en la Lista de ESIs conectados al Planificador
                                    cargarListaProcesosConectados(listaESIconectados, registroProcesoAux);


                                    // Muestro por pantalla el contenido de la listaReady
                                    showContenidolistaReady(listaReady);

                                    // Activo la Planificacion de los Procesos
                                    planificarProcesos = true;
                                    break;

                                case RESPUESTA_EJECUTAR_INSTRUCCION:

                                    log_info(infoLogger,"Respuesta sobre la Ejecución de Instruccion recibida del Proceso ESI %s.", obtenerNombreProceso(listaESIconectados, i));

                                    // Activo la Planificacion de los Procesos
                                    planificarProcesos = true;
                                    break;

                                case FINALIZACION_EJECUCION_ESI:
                                    log_info(infoLogger,"Notificacion sobre la finalizacion del Proceso ESI %s.", obtenerNombreProceso(listaESIconectados, i));


showContenidolistaProcesosConectados(listaESIconectados);
showContenidolistaReady(listaReady);
showContenidocolaReady(colaReady);
                                    // Elimino el Proceso ESI de las estrucutras Administrativas
                                    eliminarProcesoLista(listaESIconectados, i);
                                    eliminarProcesoLista(listaReady, i);
                                    eliminarProcesoCola(colaReady, i);

showContenidolistaProcesosConectados(listaESIconectados);
showContenidolistaReady(listaReady);
showContenidocolaReady(colaReady);

                                    // Activo la Planificacion de los Procesos
                                    //planificarProcesos = true;
                                    break;                                
                            }
                        }

                        // Si el mensaje proviene del COORDINADOR
                        if(encabezado.proceso == 'C'){
                            switch(encabezado.cod_operacion){

                                case NOTIFICAR_USO_RECURSO:
                                    // TODO

                                    log_info(infoLogger,"Respuesta sobre el uso de un Recurso por un Proceso recibida del COORDINADOR.");
                                    break;

                                case RECURSO_TOMADO:
                                    // TODO
                                    // Encolar el Proceso en cola de bloqueados

                                    log_info(infoLogger,"El Proceso no pudo ejecutarse porque el Recurso estaba tomado por otro Proceso.");
                                    break;

                                case INSTANCIA_INEXISTENTE:
                                    // TODO
                                    log_info(infoLogger,"Respuesta sobre la una Instancia que no existe recibida del COORDINADOR.");
                                    break;
                                    
                            }
                        }

                    }
                }

                // Planifica los Procesos de la ColaReady
                if(!planificadorPausado && planificarProcesos){


                    // Desactivo la Planificacion de los Procesos
                    planificarProcesos = false;                                    

                    // TODO
                    procesoSeleccionado = obtenerProximoProcesoPlanificado(listaESIconectados, listaReady, algoritmoPlanificacion);


                    // Armo el Paquete de la orden de Ejectuar la proxima Instruccion
                    paquete = crearHeader('P', EJECUTAR_INSTRUCCION, 1);

                    // Envio el Paquetea a ESI
                    if(send(procesoSeleccionado->socketProceso,paquete.buffer,paquete.tam_buffer,0) != -1){

                        free(paquete.buffer);
                        log_info(infoLogger, "Se le pidio al ESI %s que ejecute la proxima Instruccion", procesoSeleccionado->nombreProceso);
                    }else{
                        log_error(infoLogger, "No se pudo enviar al ESI %s la orden de ejecucion de la proxima Instruccion", procesoSeleccionado->nombreProceso);
                    }

                }


            }
        }
    }

    close(servidor);
    FD_CLR(servidor, &master);
}


/* ---------------------------------------- */

int main(int argc, char* argv[]){

    /* Creo la instancia del Archivo de Configuracion y del Log */
    cfg = config_create("config/config.cfg");
    infoLogger = log_create("log/planificador.log", "PLANIFICADOR", false, LOG_LEVEL_INFO);

    log_trace(infoLogger, "Iniciando PLANIFICADOR" );

    // Creo las Colas para la Planificacion
    colaReady = queue_create();
    colaEjecucion = queue_create();
    colaBloqueados = queue_create();
    colaTerminados = queue_create();

    // Creo las Listas para la Planificacion
    listaReady = list_create();

    // Creo la Lista de Claves Bloqueadas
    listaClavesBloqueadas = list_create();

    // Creo la lista de Todos los ESIs conectados al Planificador
    listaESIconectados = list_create();

    // Carlo el Algoritmo de Planificacion del Sistema
    algoritmoPlanificacion = string_new();
    string_append(&algoritmoPlanificacion,config_get_string_value(cfg,"ALGORITMO_PLANIFICACION"));
    

    // Creo conexión con el Coordinador
    int coordinador_fd = conectarseAservidor(config_get_string_value(cfg,"COORDINADOR_IP"),config_get_int_value(cfg,"COORDINADOR_PUERTO"));

    if(coordinador_fd == -1){
        printf("Error de conexion con el Coordinador\n");
        return EXIT_FAILURE;        
    }else{
        log_info(infoLogger, "Conexion establecida con el Coordinador");        
    }

    // Serializado el Proceso
    Paquete paquete = srlz_datosProceso('P', HANDSHAKE, "PLANIFICADOR", PLANIFICADOR, 0);

    // Envio al Coordinador el Handshake y Serializado el Proceso
    send(coordinador_fd,paquete.buffer,paquete.tam_buffer,0);
    free(paquete.buffer);


    pthread_t hiloConsola;
    pthread_create(&hiloConsola, NULL, (void*) hiloConsolaInteractiva, NULL);

    pthread_t hiloServidor;
    pthread_create(&hiloServidor, NULL, (void*) servidorPlanificador, (void*)config_get_int_value(cfg,"PLANIFICADOR_PUERTO"));

    pthread_join(hiloConsola, NULL);
    pthread_join(hiloServidor, NULL);

    /* ---------------------------------------- */
    /*  Libero Memoria de Log y Config          */
    /* ---------------------------------------- */
    log_destroy(infoLogger);
    config_destroy(cfg);

    queue_destroy(colaReady);
    queue_destroy(colaEjecucion);
    queue_destroy(colaBloqueados);
    queue_destroy(colaTerminados);
    list_destroy(listaClavesBloqueadas);
    list_destroy(listaReady);
    list_destroy(listaESIconectados);    

    free(algoritmoPlanificacion);

    return EXIT_SUCCESS;
}
