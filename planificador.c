#include <arpa/inet.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
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
    t_queue* colaFinalizados;
    t_dictionary* diccionarioClavesBloqueadas;
    t_dictionary* diccionarioRafagas;
    t_list*  listaClavesBloqueadasRequeridas;    
    t_list* listaReady;
    t_list* listaESIconectados;

    bool respuestaEjecucionInstruccionEsi;
    bool ejecutarAlgoritmoPlanificacion;
    bool planificadorPausado;
    bool planificarProcesos;
    char* algoritmoPlanificacion = NULL;
    float alfa;
    int coordinador_fd;
    int rafagaActual;
    Proceso* procesoAnterior;
    char** arregloClavesInicialmenteBloqueadas = NULL;

/* ---------------------------------------- */
/*  Consola interactiva                     */
/* ---------------------------------------- */

void* hiloConsolaInteractiva(void * unused) {

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

                    printf("\tpausar/continuar \t- El Planificador no le dará nuevas órdenes de ejecución a ningún ESI mientras se encuentre pausado. \n\n");

                    printf("\n\tbloquear <clave> <ID> \t- Se bloqueará el proceso ESI hasta ser desbloqueado, especificado por dicho <ID> en la cola del recurso <clave>. \n\n");

                    printf("\n\tdesbloquear <clave> \t- Se desbloqueara el proceso ESI con el ID especificado. \n\n");

                    printf("\n\tlistar <clave> \t- Lista los procesos encolados esperando al recurso. \n\n");

                    printf("\n\tkill <ID> \t- finaliza el proceso. Recordando la atomicidad mencionada en “bloquear”. \n\n");

                    printf("\n\tstatus <clave> \t- Debido a que para la correcta coordinación de las sentencias de acuerdo a los algoritmos de distribución se requiere de cierta información sobre las instancias del sistema, el Coordinador proporcionará una consola que permita consultar esta información. \n\n");

                    printf("\n\tdeadlock\t- Permitirá analizar los deadlocks que existan en el sistema y a que ESI están asociados.\n\n");
                }

                // Comando interno para conocer el estado de las Estructuras Administrativas
                if(string_starts_with(comandoConsola,"EXIT")){
                    pthread_exit(EXIT_SUCCESS);
                    return EXIT_SUCCESS;
                }

                // Comando interno para conocer el estado de las Estructuras Administrativas
                if(string_starts_with(comandoConsola,"INFO")){
                    comandoAceptado = true;

                    // Muestro por pantalla el contenido de las Listas y Colas
                    showContenidolistaProcesosConectados(listaESIconectados);
                    showContenidolistaReady(listaReady);
                    showContenidoCola(colaReady, "READY");
                    showContenidoCola(colaEjecucion, "EJECUCION");                    
                    showContenidoCola(colaBloqueados, "BLOQUEADOS");
                    showContenidoCola(colaFinalizados, "FINALIZADOS");
                    showContenidoDiccionario(diccionarioClavesBloqueadas, "CLAVES BLOQUEADAS");
                    showContenidolistaClavesBloqueadasRequeridas(listaClavesBloqueadasRequeridas);
                }

                if(string_starts_with(comandoConsola,"PAUSAR")){
                    comandoAceptado = true;
                    planificadorPausado = true;
                    printf("Planificador Pausado...\n");
                }

                if(string_starts_with(comandoConsola,"CONTINUAR")){
                    comandoAceptado = true;
                    planificadorPausado = false;
                    printf("Planificador Resumido...\n");

                    // Armo el Paquete
                    Paquete paquete = crearHeader('P', IS_ALIVE, COORDINADOR);

                    // Envio el Paquete al Coordinador para activar el Socket y poder Planificar
                    if(send(coordinador_fd,paquete.buffer,paquete.tam_buffer,0) != -1){
                        free(paquete.buffer);
                    }
                }

                if(string_starts_with(comandoConsola,"BLOQUEAR")){
                    comandoAceptado = true;

                    // Si se ingreso un solo parametro
                    if(countParametrosConsola(comandoConsola) == 2){

                        // obtengo el Socket del Proceso a Bloquear
                        int socketProcesoaBloquear = obtenerSocketProceso(listaESIconectados, parametrosConsolaOriginal[2]);

                        // Si el Proceso esta conectado
                        if(socketProcesoaBloquear == 0){
                            printf("[Error] Proceso ESI %s no se encuentra conectado.\n", parametrosConsolaOriginal[2]);
                        }else{

                            // Si el Recurso esta bloqueado por otro Proceso, entonces agrego al proceso en la Cola de Bloqueados
                            if(dictionary_has_key(diccionarioClavesBloqueadas, parametrosConsolaOriginal[1]) ){

                                // Cargar el Proceso en la Cola de Bloqueados
                                cargarProcesoCola(listaESIconectados, colaBloqueados, socketProcesoaBloquear);

                                printf("El Recurso %s ya se encontraba bloqueado por otro Proceso. El Proceso ESI %s se pasó a la Cola de Bloqueados.\n", parametrosConsolaOriginal[1], parametrosConsolaOriginal[2]);                                

                            }else{ // Si el Recurso no esta bloqueado, lo bloqueo

                                Proceso* registroKeyProcesoAux = NULL;
                                registroKeyProcesoAux = malloc(sizeof(Proceso));

                                // Cargo el Registro
                                registroKeyProcesoAux->socketProceso = 0;
                                registroKeyProcesoAux->tipoProceso = ESI;
                                registroKeyProcesoAux->nombreProceso = malloc(strlen(parametrosConsolaOriginal[2])+1);
                                strcpy(registroKeyProcesoAux->nombreProceso , parametrosConsolaOriginal[2]);
                                registroKeyProcesoAux->nombreProceso[strlen(parametrosConsolaOriginal[2])] = '\0';

                                // Bloqueo el Recurso y lo cargo en la Lista de Claves Bloqueadas
                                dictionary_put(diccionarioClavesBloqueadas, parametrosConsolaOriginal[1], registroKeyProcesoAux);

                                printf("Se agrego el Recurso %s en la Lista de Claves Bloqueadas asociado al Proceso ESI %s.\n", parametrosConsolaOriginal[1], parametrosConsolaOriginal[2]);
                            }
                        }

                    }else{
                        printf("[Error] Cantidad de parámetros incorrectos\n");
                    }
                }

                if(string_starts_with(comandoConsola,"DESBLOQUEAR")){
                    comandoAceptado = true;

                    // Si se ingreso un solo parametro
                    if(countParametrosConsola(comandoConsola) == 1){

                        // Libero un Recurso de la Lista de Claves Bloqueadas
                        dictionary_remove(diccionarioClavesBloqueadas, parametrosConsolaOriginal[1]);

                    }else{
                        printf("[Error] Cantidad de parámetros incorrectos\n");
                    }

                }

                if(string_starts_with(comandoConsola,"LISTAR")){
                    comandoAceptado = true;

                    // Si se ingreso un solo parametro
                    if(countParametrosConsola(comandoConsola) == 1){

                        // Lista los Procesos que quiere usar el Recurso indicado por consola
                        listarRecursosBloqueados(listaClavesBloqueadasRequeridas, parametrosConsolaOriginal[1]);
                    }else{
                        printf("[Error] Cantidad de parámetros incorrectos\n");
                    }
                }

                if(string_starts_with(comandoConsola,"KILL")){
                    comandoAceptado = true;


                    // Si se ingreso un solo parametro
                    if(countParametrosConsola(comandoConsola) == 1){
                        // obtengo el Socket del Proceso a Matar
                        int socketProcesoaMatar = obtenerSocketProceso(listaESIconectados, parametrosConsolaOriginal[1]);

                        // Si el Proceso esta conectado
                        if(socketProcesoaMatar == 0){
                            printf("[Error] Proceso ESI %s no se encuentra conectado.\n", parametrosConsolaOriginal[1]);
                        }else{
                            liberarRecursosProceso(diccionarioClavesBloqueadas, parametrosConsolaOriginal[1]);
                            cargarProcesoCola(listaESIconectados, colaFinalizados, socketProcesoaMatar);
                            dictionary_remove(diccionarioRafagas, parametrosConsolaOriginal[1]);
                            eliminarProcesoLista(listaESIconectados,socketProcesoaMatar);
                            eliminarProcesoLista(listaReady, socketProcesoaMatar);
                            
                            //elimino al proceso de ccualquier cola en la que pueda estar
                            
                            eliminarProcesoCola(colaReady, socketProcesoaMatar);
                            eliminarProcesoCola(colaBloqueados, socketProcesoaMatar);
                            eliminarProcesoCola(colaEjecucion, socketProcesoaMatar);
                            
                            //envio el msj al esi para que muera
                            Paquete paquete= crearHeader('P',ESI_MUERE,1);
                            if( send(socketProcesoaMatar,paquete.buffer,paquete.tam_buffer,1) != -1 ){
                                log_info(infoLogger,"Se avios a esi de que muera");
                            }else{
                                log_info(infoLogger,"No se pudo avisar a esi de que muera");
                                printf("\n\n NO SE PUDO ENVIAR ORDEN A ESI %s DE QUE MUERA \n\n",parametrosConsolaOriginal[1]);
                            }

                                         
                            
                                          
                        }



                    }else{
                        printf("[Error] Cantidad de parámetros incorrectos\n");
                    }
                    
                }

                if(string_starts_with(comandoConsola,"STATUS")){
                    comandoAceptado = true;
                    
                    // Si se ingreso un solo parametro
                    if(countParametrosConsola(comandoConsola) == 1){

                        // Serializado el Proceso y la Key
                        Paquete paquete = srlz_datosKeyBloqueada('P', OBTENER_STATUS_CLAVE, "PLANIFICADOR", 1, parametrosConsolaOriginal[1], "_");

                        // Envio el Paquetea al Coordinador
                        if(send(coordinador_fd,paquete.buffer,paquete.tam_buffer,0) != -1){

                            free(paquete.buffer);
                            log_info(infoLogger, "Se le pide al COORDINADOR infomarcion sobre el Recurso %s.", parametrosConsolaOriginal[1]);

                            // Recibo la informacion de la Instancia 
                            Encabezado encabezado=recibir_header(&coordinador_fd);
                            paquete = recibir_payload(&coordinador_fd,&encabezado.tam_payload);
                            Instancia registroInstancia = dsrlz_datosInstancia(paquete.buffer);
                            free(paquete.buffer);

                            // Determino si el Recurso fue distribuido en alguna Instancia
                            if(strcmp(registroInstancia.nombreProceso, "NONE") == 0){
                                printf("El Recurso %s no se encuentra distribuido aun\n", parametrosConsolaOriginal[1]);
                            }else{
                                printf("El Recurso %s se encuentra distribuido en la Instancia %s\n", parametrosConsolaOriginal[1],registroInstancia.nombreProceso);                                
                            }

                        }else{
                            printf("No se pudo pedir infomacion al COORDINADOR sobre el Recurso %s\n", parametrosConsolaOriginal[1]);
                        }
                    }else{
                        printf("[Error] Cantidad de parámetros incorrectos\n");
                    }
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
    return 0;
}


void* atenderConexiones(void* socketConexion){

    pthread_informacion* data_hilo = (pthread_informacion*) socketConexion;
    int i = data_hilo->socketHilo;
    free(data_hilo);

    int nbytes;

    Proceso registroProceso;
    KeyBloqueada registroKeyBloqueada;
    Proceso* procesoSeleccionado=NULL;
    int indice = 0, resultadoEjecucion;
    bool recursoOcupado;
    Paquete paquete;
    Encabezado encabezado;

    while(true){

//printf("recibir_header...Socket %d\n", i);
        // Recibo el Encabezado del Paquete
        encabezado=recibir_header(&i);

//printf("MENSAJE RECIBIDO DE %c:%d\n", encabezado.proceso, encabezado.cod_operacion);

        if((nbytes = encabezado.tam_payload) <= 0){
            // error o conexión cerrada por el cliente
            if(nbytes == 0){
                printf("Socket %d se ha caído\n", i);
            }else{
                perror("recv failed");
            }
            close(i); // Cierro el Socket por desconexion
            pthread_exit(EXIT_SUCCESS); // Finalizo el Hilo
            return 0;
        }

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

                    // Cargo los ESIs conectados en la Lista de ESIs conectados al Planificador
                    cargarListaProcesosConectados(listaESIconectados, registroProcesoAux);

                    // Cargo el Proceso en la listaReady
                    cargarProcesoLista(listaESIconectados, listaReady, i);

                    //cargo el proceso en dictionary
                    Rafagas* registroRafagaAux= malloc(sizeof(Rafagas));
                    registroRafagaAux->rafagaAnterior=0;
                    registroRafagaAux->estimacionRafagaAnterior=config_get_int_value(cfg,"ESTIMACION_INICIAL");
                    registroRafagaAux->proximaEstimacion=estimarRafaga(registroRafagaAux->estimacionRafagaAnterior, registroRafagaAux->rafagaAnterior, alfa);
                    registroRafagaAux->tiempoDeEsperaDeCpu=0;
                    dictionary_put(diccionarioRafagas,registroProcesoAux->nombreProceso,registroRafagaAux);


                    // Activo la Planificacion de los Procesos
                    planificarProcesos = true;
                    if(list_size(listaReady)==1 && queue_size(colaEjecucion)==0) ejecutarAlgoritmoPlanificacion=true;
                    break;

                case RESPUESTA_EJECUTAR_INSTRUCCION:

                    log_info(infoLogger,"Respuesta sobre la Ejecución de Instruccion recibida del Proceso ESI %s.", obtenerNombreProceso(listaESIconectados, i));


                    resultadoEjecucion = encabezado.tam_payload;

                    // Si la ejecucion de la instruccion no fallo
                    if(resultadoEjecucion == EJECUCION_EXITOSA){
                        rafagaActual += 1;

                        void actualizarTiempoDeEsperaDeLosProcesos(Proceso* registroProcesoAux){
                               Rafagas* registroRafagaAux;
                               registroRafagaAux = dictionary_get(diccionarioRafagas,registroProcesoAux->nombreProceso);
                               registroRafagaAux->tiempoDeEsperaDeCpu++;
                        }
                        //actualizo el tiempo de espera de CPU de cada proceso en la lista de ready
                        list_iterate(listaReady, (void *) actualizarTiempoDeEsperaDeLosProcesos);

                        // Pongo el proceso en la Cola Ready
                        cargarProcesoCola(listaESIconectados, colaReady, i);
                        cargarProcesoLista(listaESIconectados, listaReady, i);

                        log_info(infoLogger,"Respuesta sobre la Ejecución EXITOSA de la Instruccion recibida por el Proceso ESI.");

                    }else{ // Si la ejecucion de la instruccion fallo

                        log_info(infoLogger,"Respuesta sobre la Ejecución FALLIDA de la Instruccion recibida por el Proceso ESI.");

                        // Si el Resultado es fallido, puede ser porque quizo acceder a un Recurso que estaba tomado por otro proceso. En este caso, cambio al proceso a la ColaBloqueados
                        
                        // Cargar el Proceso en la Cola de Bloqueados
                        cargarProcesoCola(listaESIconectados, colaBloqueados, i);
                        eliminarProcesoLista(listaReady, i);
                        eliminarProcesoCola(colaReady, i);
                        //si se bloquea, activo la planificacion
                        ejecutarAlgoritmoPlanificacion=true;

                        log_info(infoLogger,"Actualizacion de las Estructuras Administrativas");
                    }

                    // Saco el Proceso de la Cola de Ejecucion y lo saco de
                    eliminarProcesoCola(colaEjecucion, i);

                    // Activo la Planificacion de los Procesos
                    planificarProcesos = true;
                    respuestaEjecucionInstruccionEsi=true;

                    break;

                case FINALIZACION_EJECUCION_ESI:
                    log_info(infoLogger,"Notificacion sobre la finalizacion del Proceso ESI %s.", obtenerNombreProceso(listaESIconectados, i));

                    // Libero los Recursos que tenia asignado en Lista de Claves Bloqueadas
                    liberarRecursosProceso(diccionarioClavesBloqueadas, obtenerNombreProceso(listaESIconectados, i));

                    // Cargar el Proceso en la Cola de Finalizados
                    cargarProcesoCola(listaESIconectados, colaFinalizados, i);

                    //si se estaba ejecutando y finalizo el proceso, el procesoSeleccionado lo pongo en NULL
                    //para evitarme problemas a la hora de planificar
                    if(obtenerRegistroProceso(listaESIconectados, i) == procesoSeleccionado) procesoSeleccionado = NULL;

                    // Elimino el Proceso ESI de las estrucutras Administrativas
                    dictionary_remove(diccionarioRafagas, obtenerNombreProceso(listaESIconectados,i));
                    eliminarProcesoLista(listaESIconectados, i);
                    eliminarProcesoLista(listaReady, i);
                    eliminarProcesoCola(colaReady, i);
                    eliminarProcesoCola(colaEjecucion, i);
                    eliminarProcesoCola(colaBloqueados, i);

                    log_info(infoLogger,"Actualizacion de las Estructuras Administrativas");

                    // Activo la Planificacion de los Procesos
                    planificarProcesos = true;
                    ejecutarAlgoritmoPlanificacion=true;
                    respuestaEjecucionInstruccionEsi=true;
                    break;                                
            }
        }

        // Si el mensaje proviene del COORDINADOR
        if(encabezado.proceso == 'C'){
            switch(encabezado.cod_operacion){

                case NOTIFICAR_LIBERACION_RECURSO:

                    // Recibo los datos del Key y Proceso
                    paquete = recibir_payload(&i,&encabezado.tam_payload);
                    registroKeyBloqueada = dsrlz_datosKeyBloqueada(paquete.buffer);
                    free(paquete.buffer);

                    log_info(infoLogger,"Notificacion del COORDINADOR que el Proceso ESI %s libera el Recurso %s.", registroKeyBloqueada.nombreProceso, registroKeyBloqueada.key);

                    // Libero un Recurso de la Lista de Claves Bloqueadas
                    dictionary_remove(diccionarioClavesBloqueadas, registroKeyBloqueada.key);

                    log_info(infoLogger,"Se libero el Recurso %s de la Lista de Claves Bloqueadas que lo tenia tomado el Proceso ESI %s.", registroKeyBloqueada.key, registroKeyBloqueada.nombreProceso);
                    break;


                case NOTIFICAR_USO_RECURSO:

                    // Recibo los datos del Key y Proceso
                    paquete = recibir_payload(&i,&encabezado.tam_payload);
                    registroKeyBloqueada = dsrlz_datosKeyBloqueada(paquete.buffer);
                    free(paquete.buffer);

                    log_info(infoLogger,"Notificacion del COORDINADOR que el Proceso ESI %s quiere acceder al Recurso %s.", registroKeyBloqueada.nombreProceso, registroKeyBloqueada.key);

                    recursoOcupado = false;

                    // Si el Recurso esta bloqueado por otro Proceso
                    if(dictionary_has_key(diccionarioClavesBloqueadas, registroKeyBloqueada.key) ){

                        // Guardo una Lista de las Claves Bloqueadas que quieren ser usadas por otros Procesos
                        list_add(listaClavesBloqueadasRequeridas, &registroKeyBloqueada);

                        log_info(infoLogger,"El Recurso %s ya se encuentra tomado por otro Proceso ESI.", registroKeyBloqueada.key);

                        // Serializado el Proceso y la Key
                        paquete = srlz_datosKeyBloqueada('P', RECURSO_OCUPADO, registroKeyBloqueada.nombreProceso, registroKeyBloqueada.operacion,registroKeyBloqueada.key,registroKeyBloqueada.dato);

                        recursoOcupado = true;


                    }else{ // Si el Recurso no esta bloqueado

                        // Bloqueo el Recurso y lo cargo en la Lista de Claves Bloqueadas
                        dictionary_put(diccionarioClavesBloqueadas, registroKeyBloqueada.key, &registroKeyBloqueada);

                        log_info(infoLogger,"Se agrego el Recurso %s en la Lista de Claves Bloqueadas que lo tomo el Proceso ESI %s.", registroKeyBloqueada.key, registroKeyBloqueada.nombreProceso);

                        // Serializado el Proceso y la Key
                        paquete = srlz_datosKeyBloqueada('P', RECURSO_LIBRE, registroKeyBloqueada.nombreProceso, registroKeyBloqueada.operacion,registroKeyBloqueada.key,registroKeyBloqueada.dato);
                    }


                    // Envio el Paquete al Coordinador con la notificacion
                    if(send(i,paquete.buffer,paquete.tam_buffer,0) != -1){

                        free(paquete.buffer);

                        if(recursoOcupado){
                            log_info(infoLogger, "Se le notifica al COORDINADOR que el Recurso %s ya estaba tomado por otro Proceso.", registroKeyBloqueada.key);    
                        }else{
                            log_info(infoLogger, "Se le notifica al COORDINADOR que el Recurso %s estaba libre y ahora quedo tomado por el Proceso ESI %s.",registroKeyBloqueada.key, registroKeyBloqueada.nombreProceso);    
                        }
                        
                    }else{
                        log_error(infoLogger, "No se pudo enviar la notificacion al COORDINADOR sobre el estado de  uso del Recurso %s.", registroKeyBloqueada.key);
                    }
                    break;

                case INSTANCIA_DESCONECTADA:
                    // TODO
                    log_info(infoLogger,"Respuesta sobre la Instancia que no existe recibida del COORDINADOR.");
                    break;

                case IS_ALIVE: 
                    break;                                     
            }
        }


            // Planifica los Procesos de la ColaReady
            if(!planificadorPausado && planificarProcesos && respuestaEjecucionInstruccionEsi){

                //ejecuta el algoritmo de planificacion
                if(ejecutarAlgoritmoPlanificacion){

                    //si no es nulo el proceso seleccionado que estaba ejecutando, se le actualizan las rafagas
                    //antes de cambiar de proceso por la planificacion
                    if(procesoSeleccionado !=NULL){
                        Rafagas* registroRafagaAux=NULL;
                        registroRafagaAux = dictionary_get(diccionarioRafagas,procesoSeleccionado->nombreProceso);

                        registroRafagaAux->rafagaAnterior = rafagaActual;
                        registroRafagaAux->proximaEstimacion = estimarRafaga(registroRafagaAux->estimacionRafagaAnterior, registroRafagaAux->rafagaAnterior, alfa);
                        registroRafagaAux->estimacionRafagaAnterior = registroRafagaAux->proximaEstimacion;
                        //al salir de ejecutarse, se resetea su tiempo de espera de cpu
                        registroRafagaAux->tiempoDeEsperaDeCpu = 0;
                    }

                    procesoSeleccionado = obtenerProximoProcesoPlanificado(listaReady, colaReady, diccionarioRafagas, algoritmoPlanificacion, alfa);

                    //este if solo lo pongo para informar qué proceso se selecciono en la planificacion
                    Rafagas* rafagasAux2=NULL;
                    if(procesoSeleccionado !=NULL){
                        rafagasAux2= dictionary_get(diccionarioRafagas, procesoSeleccionado->nombreProceso);
                        printf("\nProceso planificado para ejecutar: %s\n", procesoSeleccionado->nombreProceso);
                    }
                    rafagaActual=0;
                }

                // Desactivo la Planificacion de los Procesos
                planificarProcesos = false;
                ejecutarAlgoritmoPlanificacion=false;
                /*
                //si cambia el proceso, guarda nuevas rafagas
                if(procesoAnterior == NULL){
                    procesoAnterior=procesoSeleccionado;
                    //puts("asigno actual al anterior");
                }
                   // puts("\n\n entro a if \n\n");
                if(procesoSeleccionado == NULL){
                    //puts("proceso seleccionado es nulo");
                }else{
                    if(procesoSeleccionado != procesoAnterior){
                        //chekeo que el procesi anterior siga conectado
                        
                        printf(" proceso seleccionado %s y proceso anterior %s  \n" ,procesoSeleccionado->nombreProceso, procesoAnterior->nombreProceso);
                            if( obtenerSocketProceso(listaESIconectados, procesoAnterior->nombreProceso) != 0){
                                Rafagas* rafagasAux=NULL;
                                rafagasAux= dictionary_get(diccionarioRafagas, procesoAnterior->nombreProceso);
                                rafagasAux->estimacionRafagaAnterior= rafagasAux->proximaEstimacion;
                                rafagasAux->rafagaAnterior= rafagaActual;
                                rafagasAux->proximaEstimacion= estimarRafaga(rafagasAux->estimacionRafagaAnterior,rafagaActual,alfa);
                                rafagaActual=0;
                        }
                        procesoAnterior= procesoSeleccionado;
                    }
                }*/

                // Si existe un Proceso para planificar
                if(procesoSeleccionado != NULL){

                    // Cargo el Proceso en la Cola de Ejecucion y lo saco de la Cola Ready
                    eliminarProcesoCola(colaReady, procesoSeleccionado->socketProceso);
                    eliminarProcesoLista(listaReady, procesoSeleccionado->socketProceso);
                    cargarProcesoCola(listaESIconectados, colaEjecucion, procesoSeleccionado->socketProceso);

//printf("Proximo Procesos a Planificar: Nombre: %s - Socket: %d\n", procesoSeleccionado->nombreProceso, procesoSeleccionado->socketProceso);                    
                    // Armo el Paquete de la orden de Ejectuar la proxima Instruccion
                    paquete = crearHeader('P', EJECUTAR_INSTRUCCION, 1);

                    // Envio el Paquetea a ESI
                    if(send(procesoSeleccionado->socketProceso,paquete.buffer,paquete.tam_buffer,0) != -1){

                        free(paquete.buffer);
                        log_info(infoLogger, "Se le pidio al ESI %s (Socket %d) que ejecute la proxima Instruccion", procesoSeleccionado->nombreProceso, procesoSeleccionado->socketProceso);
                        respuestaEjecucionInstruccionEsi=false;
                    }else{
                        log_error(infoLogger, "No se pudo enviar al ESI %s la orden de ejecucion de la proxima Instruccion", procesoSeleccionado->nombreProceso);
                    }
                }
            }

    }

    pthread_exit(EXIT_SUCCESS);

    return 0;
}


/* ---------------------------------------- */

int main(int argc, char* argv[]){

    /* Creo la instancia del Archivo de Configuracion y del Log */
    cfg = config_create("config/config.cfg");
    infoLogger = log_create("log/planificador.log", "PLANIFICADOR", false, LOG_LEVEL_INFO);

    rafagaActual=0;
    procesoAnterior=NULL;

    log_trace(infoLogger, "Iniciando PLANIFICADOR" );

    // Creo las Colas para la Planificacion
    colaReady = queue_create();
    colaEjecucion = queue_create();
    colaBloqueados = queue_create();
    colaFinalizados = queue_create();

    // Creo las Listas para la Planificacion
    listaReady = list_create();

    // Creo la Lista de Claves Bloqueadas y Claves Bloqueadas que quieren ser usadas
    diccionarioClavesBloqueadas = dictionary_create();
    diccionarioRafagas= dictionary_create();

    listaClavesBloqueadasRequeridas = list_create();

    // Creo la lista de Todos los ESIs conectados al Planificador
    listaESIconectados = list_create();

    // Cargo el Algoritmo de Planificacion del Sistema
    algoritmoPlanificacion = string_new();
    string_append(&algoritmoPlanificacion,config_get_string_value(cfg,"ALGORITMO_PLANIFICACION"));
    
    //cargo el valor Alfa del archivo cfg
    alfa=config_get_int_value(cfg,"ALFA");

    // Obtenglo las Claves bloqueadas por archivo de configuracion
    arregloClavesInicialmenteBloqueadas = string_get_string_as_array(config_get_string_value(cfg,"CLAVES_INICIALMENTE_BLOQUEADAS"));

    // Cargo las Claves bloqueadas por archivo de configuracion en el Diccionario de Claves Bloqueadas
    if(cargarClavesInicialmenteBloqueadas(diccionarioClavesBloqueadas, arregloClavesInicialmenteBloqueadas) != 0){
        log_info(infoLogger, "Se cargaron las Claves bloqueadas definidas por el archivo de configuración");
    }

    // Inicializo los estados del Planificador
    planificadorPausado = false;
    planificarProcesos = false;
    ejecutarAlgoritmoPlanificacion=false;
    respuestaEjecucionInstruccionEsi=true;

    // Creo conexión con el Coordinador
    coordinador_fd = conectarseAservidor(config_get_string_value(cfg,"COORDINADOR_IP"),config_get_int_value(cfg,"COORDINADOR_PUERTO"));

    if(coordinador_fd == -1){
        printf("Error de conexion con el Coordinador\n");
        return EXIT_FAILURE;        
    }else{
        log_info(infoLogger, "Conexion establecida con el Coordinador");
    }

    // Creo el Servidor para escuchar conexiones
    int servidor=crearServidor(config_get_int_value(cfg,"PLANIFICADOR_PUERTO"));
    log_trace(infoLogger, "Escuchando conexiones" );

    // Serializado el Proceso
    Paquete paquete = srlz_datosProceso('P', HANDSHAKE, "PLANIFICADOR", PLANIFICADOR, 0);

    // Envio al Coordinador el Handshake y Serializado el Proceso
    send(coordinador_fd,paquete.buffer,paquete.tam_buffer,0);
    free(paquete.buffer);


    pthread_t hiloConsola;
    pthread_create(&hiloConsola, NULL, (void*) hiloConsolaInteractiva, NULL);
    pthread_detach(hiloConsola);
    //pthread_join(hiloConsola, NULL);

    // Creo Hilo que Atiende al Coordinador
    pthread_t hiloConexionCoordinador;

    pthread_informacion* data_hilo = (pthread_informacion*) malloc(sizeof(*data_hilo));
    data_hilo->socketHilo = coordinador_fd;

    log_info(infoLogger,"Creando un hilo para atender al Coordinador con FD %d", coordinador_fd);
    pthread_create(&hiloConexionCoordinador, NULL, (void*) atenderConexiones, data_hilo);
    pthread_detach(hiloConexionCoordinador);


    int new_fd;

    while(true){

        // Acepto todas las conexiones
        new_fd = aceptarConexionCliente(servidor);

        if (new_fd != -1) {

            // Creo un Hilo por cada nueva Conexion
            pthread_t hiloConexiones;

            pthread_informacion* data_hilo = (pthread_informacion*) malloc(sizeof(*data_hilo));
            data_hilo->socketHilo = new_fd;

            pthread_create(&hiloConexiones, NULL, (void*) atenderConexiones, data_hilo);
            pthread_detach(hiloConexiones);

            log_info(infoLogger,"Creando un hilo para atender el nuevo cliente con FD %d", new_fd);
        }
    }



    /* ---------------------------------------- */
    /*  Libero Memoria de Log y Config          */
    /* ---------------------------------------- */
    log_destroy(infoLogger);
    config_destroy(cfg);

    queue_destroy(colaReady);
    queue_destroy(colaEjecucion);
    queue_destroy(colaBloqueados);
    queue_destroy(colaFinalizados);
    dictionary_destroy(diccionarioClavesBloqueadas);
    dictionary_destroy(diccionarioRafagas);
    list_destroy(listaReady);
    list_destroy(listaESIconectados);    
    list_destroy(listaClavesBloqueadasRequeridas);

    free(algoritmoPlanificacion);
    free(arregloClavesInicialmenteBloqueadas);
    close(servidor);
    return EXIT_SUCCESS;
}
