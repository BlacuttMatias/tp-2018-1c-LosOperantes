#include <arpa/inet.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <netinet/in.h>
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

    t_list* listaProcesosConectados;
    t_list* listaInstanciasConectadas;
    t_dictionary* diccionarioClavesInstancias;
    t_dictionary* diccionarioClavesBloqueadas;
    int fd_planificador;
    pthread_t hiloConexiones;

/* ---------------------------------------- */

void* atenderConexiones(void* socketConexion){

    pthread_informacion* data_hilo = (pthread_informacion*) socketConexion;
    int i = data_hilo->socketHilo;
    free(data_hilo);

    Encabezado encabezado;
    Paquete paquete;
    struct sockaddr_in servidor_addr,my_addr,master_addr; // información de la dirección de destino
    int nbytes;


    Proceso registroProceso;
    ResultadoEjecucion registroResultadoEjecucion;
    Instruccion registroInstruccion;
    KeyBloqueada registroKeyBloqueada;
    Instancia* proximaInstancia;
    int indice = 0;
    int socketESI = 0;


    while(true){
        // Recibo el Encabezado del Paquete
        encabezado=recibir_header(&i);

        // gestionar datos de un cliente
        if ((nbytes = encabezado.tam_payload) <= 0) {
            // error o conexión cerrada por el cliente
            if (nbytes == 0) {
                // conexión cerrada
                printf("Socket %d se ha caído\n", i);
            } else {
                perror("recv");
            }
            close(i); // Cierro el Socket por desconexion

            // Si el Socket caido es del Planificador, finaliza el Coordinador
            if(fd_planificador == i){
                printf("Se cayo el Planificador. El Coordinador finaliza.\n");
                exit(EXIT_SUCCESS);
            }

            pthread_exit(EXIT_SUCCESS); // Finalizo el Hilo
            return 0;
            break;
        } 

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

                    // Cargo el Registro del Proceso
                    Proceso* registroProcesoAux = NULL;
                    registroProcesoAux = malloc(sizeof(Proceso));

                    // Cargo el Registro del Proceso
                    registroProcesoAux->tipoProceso = registroProceso.tipoProceso;
                    registroProcesoAux->socketProceso = i;
                    registroProcesoAux->nombreProceso = malloc(strlen(registroProceso.nombreProceso)+1);
                    strcpy( registroProcesoAux->nombreProceso ,registroProceso.nombreProceso);
                    registroProcesoAux->nombreProceso[strlen(registroProceso.nombreProceso)] = '\0';


                    // Cargo el Proceso en la Lista de Procesos conectados al Planificador
                    cargarListaProcesosConectados(listaProcesosConectados, registroProcesoAux);

                    // Muestro por pantalla el contenido de listaProcesosConectados
                    showContenidolistaProcesosConectados(listaProcesosConectados);

                    // Guardo el FD del Planificador para conexiones futuras
                    fd_planificador = i;
                    break;

                case RECURSO_LIBRE:

                    // Recibo los datos del Key y Proceso
                    paquete = recibir_payload(&i,&encabezado.tam_payload);
                    registroKeyBloqueada = dsrlz_datosKeyBloqueada(paquete.buffer);
                    free(paquete.buffer);

                    log_info(infoLogger,"El PLANIFICADOR notifica que el Recurso %s esta LIBRE.", registroKeyBloqueada.key);

                    // Cargo el Recurso en la Lista de Claves Bloqueadas
                    dictionary_put(diccionarioClavesBloqueadas, registroKeyBloqueada.key, &registroKeyBloqueada);

                    // Averiguo el Socket del Proceso ESI para notificarle que no fallo la Ejecucion de la Instruccion
                    socketESI = obtenerSocketProceso(listaProcesosConectados, registroKeyBloqueada.nombreProceso);

                    // Genero el Log de Operaciones
                    registrarLogOperaciones(listaProcesosConectados, registroKeyBloqueada.operacion, registroKeyBloqueada.key, registroKeyBloqueada.dato, socketESI);
                    log_info(infoLogger,"Operacion guardada en el Log de Operaciones:  %s %i %s %s", obtenerNombreProceso(listaProcesosConectados, socketESI), registroKeyBloqueada.operacion, registroKeyBloqueada.key, registroKeyBloqueada.dato);

                    // Armo el Paquete del Resultado de la Ejecucion de la Instruccion
                    paquete = srlz_resultadoEjecucion('C', RESPUESTA_EJECUTAR_INSTRUCCION, registroKeyBloqueada.nombreProceso, EJECUCION_EXITOSA, "", registroKeyBloqueada.operacion, registroKeyBloqueada.key);

                    // Envio el Paquete a ESI
                    if(send(socketESI,paquete.buffer,paquete.tam_buffer,0) != -1){

                        free(paquete.buffer);

                        log_info(infoLogger, "Se le notifico al ESI %s que el Recurso %s estaba LIBRE", obtenerNombreProceso(listaProcesosConectados, socketESI), registroKeyBloqueada.key);

                    }else{
                        log_error(infoLogger, "No se pudo notificar al ESI %s que el Recurso %s estaba LIBRE", obtenerNombreProceso(listaProcesosConectados, socketESI), registroKeyBloqueada.key);
                    }
                    break;

                case RECURSO_OCUPADO:

                    // Recibo los datos del Key y Proceso
                    paquete = recibir_payload(&i,&encabezado.tam_payload);
                    registroKeyBloqueada = dsrlz_datosKeyBloqueada(paquete.buffer);
                    free(paquete.buffer);

                    log_info(infoLogger,"El PLANIFICADOR notifica que el Recurso %s esta tomado por otro Proceso.", registroKeyBloqueada.key);

                    // Averiguo el Socket del Proceso ESI para notificarle que fallo la Ejecucion de la Instruccion
                    socketESI = obtenerSocketProceso(listaProcesosConectados, registroKeyBloqueada.nombreProceso);

                    // Armo el Paquete del Resultado de la Ejecucion de la Instruccion
                    paquete = srlz_resultadoEjecucion('C', RESPUESTA_EJECUTAR_INSTRUCCION, registroKeyBloqueada.nombreProceso, EJECUCION_FALLIDA, "", registroKeyBloqueada.operacion, registroKeyBloqueada.key);

                    // Envio el Paquetea a ESI
                    if(send(socketESI,paquete.buffer,paquete.tam_buffer,0) != -1){

                        free(paquete.buffer);

                        log_info(infoLogger, "Se le notifico al ESI %s que el Recurso %s estaba OCUPADO por otro Proceso ESI", obtenerNombreProceso(listaProcesosConectados, socketESI), registroKeyBloqueada.key);
                    }else{
                        log_error(infoLogger, "No se pudo notificar al ESI %s que el Recurso %s estaba OCUPADO por otro Proceso ESI", obtenerNombreProceso(listaProcesosConectados, socketESI), registroKeyBloqueada.key);
                    }
                    break;

                case OBTENER_STATUS_CLAVE:

                    // Recibo los datos del Key y Proceso
                    paquete = recibir_payload(&i,&encabezado.tam_payload);
                    registroKeyBloqueada = dsrlz_datosKeyBloqueada(paquete.buffer);
                    free(paquete.buffer);

                    log_info(infoLogger,"El PLANIFICADOR solicita informacion sobre el Recurso %s.", registroKeyBloqueada.key);


                    // TODO 
                    // Falta generar la informacion de la Instancia y devolverlo al Planificador. Hoy esta harcodeado

                    // Serializado la Respuesta (ESTE MENSAJE LLEGA DIRECTAMENTE A LA CONSOLA)
                    paquete = srlz_datosInstancia('C', OBTENER_STATUS_CLAVE, "Instancia1", 2, 3);

                    // Envio el Paquete al Planificador
                    if(send(i,paquete.buffer,paquete.tam_buffer,0) != -1){

                        free(paquete.buffer);
                        log_info(infoLogger, "Se le envio informacion al PLANIFICADOR sobre la Instancia");

                    }else{
                        log_error(infoLogger, "No se pudo enviar informacion al PLANIFICADOR sobre la Instancia");
                    }
                    break;   

                case IS_ALIVE: 

                    // Armo el Paquete del Resultado de la Ejecucion de la Instruccion
                    paquete = crearHeader('C', IS_ALIVE, PLANIFICADOR);

                    // Envio el Paquetea
                    if(send(i,paquete.buffer,paquete.tam_buffer,0) != -1){
                        free(paquete.buffer);
                    }
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

                    // Cargo el Registro del Proceso
                    Proceso* registroProcesoAux = NULL;
                    registroProcesoAux = malloc(sizeof(Proceso));

                    // Cargo el Registro del Proceso
                    registroProcesoAux->tipoProceso = registroProceso.tipoProceso;
                    registroProcesoAux->socketProceso = i;
                    registroProcesoAux->nombreProceso = malloc(strlen(registroProceso.nombreProceso)+1);
                    strcpy( registroProcesoAux->nombreProceso ,registroProceso.nombreProceso);
                    registroProcesoAux->nombreProceso[strlen(registroProceso.nombreProceso)] = '\0';


                    // Cargo el Proceso en la Lista de Procesos conectados al Planificador
                    cargarListaProcesosConectados(listaProcesosConectados, registroProcesoAux);

                    // Cargo el Proceso en la Lista de Instancias conectadas al Planificador
                    cargarListaInstanciasConectadas(listaInstanciasConectadas, registroProcesoAux);

                    // Muestro por pantalla el contenido de listaProcesosConectados
                    showContenidolistaProcesosConectados(listaProcesosConectados);

                    // Obtengo la Cantidad y Tamano de Entradas y se lo envio a la Instancia
                    paquete = srlz_datosEntradas('C', OBTENCION_CONFIG_ENTRADAS, config_get_int_value(cfg,"CANTIDAD_ENTRADAS"), config_get_int_value(cfg,"TAMANO_ENTRADA"));

                    // Envio el Paquete a la Instancia
                    if(send(i,paquete.buffer,paquete.tam_buffer,0) != -1){

                        free(paquete.buffer);

                        log_info(infoLogger, "Se le envió a la INSTANCIA los datos de Cantidad y Tamaño de Entradas.");    
                    }else{
                        log_error(infoLogger, "No se pudo enviar a la INSTANCIA los datos de Cantidad y Tamaño de Entradas");
                    }
                    break;

                case RESPUESTA_EJECUTAR_INSTRUCCION:

                    paquete=recibir_payload(&i,&encabezado.tam_payload);
                    registroResultadoEjecucion=dsrlz_resultadoEjecucion(paquete.buffer);
                    free(paquete.buffer);

                    log_info(infoLogger,"Respuesta de la Ejecución de una Instrucción recibida de la Instancia %s.", obtenerNombreProceso(listaProcesosConectados, i));

                    socketESI = obtenerSocketProceso(listaProcesosConectados, registroResultadoEjecucion.nombreEsiDestino);

                    // Si la Operacion fue STORE y resulto Exitosa, libero el Recurso
                    if(registroResultadoEjecucion.operacion == STORE && registroResultadoEjecucion.resultado == EJECUCION_EXITOSA){
                        
                        // Libero un Recurso de la Lista de Claves Bloqueadas
                        dictionary_remove(diccionarioClavesBloqueadas, registroResultadoEjecucion.key);

                        // Serializado el Proceso y la Key
                        paquete = srlz_datosKeyBloqueada('C', NOTIFICAR_LIBERACION_RECURSO, registroResultadoEjecucion.nombreEsiDestino, registroResultadoEjecucion.operacion, registroResultadoEjecucion.key, registroResultadoEjecucion.contenido);

                        // Envio el Paquetea al Planificador
                        if(send(fd_planificador,paquete.buffer,paquete.tam_buffer,0) != -1){
                            log_info(infoLogger, "Se le notifica al PLANIFICADOR que el Proceso ESI %s liberó el Recurso %s.", registroResultadoEjecucion.nombreEsiDestino, registroInstruccion.key);
                        }else{
                            log_error(infoLogger, "No se pudo enviar mensaje al PLANIFICADOR sobre la liberación  del Recurso %s por el Proceso ESI %s.", registroInstruccion.key, registroResultadoEjecucion.nombreEsiDestino);
                        }

                        // Libero el Buffer
                        free(paquete.buffer);
                    }

                    // Armo el Paquete del Resultado de la Ejecucion de la Instruccion
                    paquete = srlz_resultadoEjecucion('C', RESPUESTA_EJECUTAR_INSTRUCCION, registroResultadoEjecucion.nombreEsiDestino, registroResultadoEjecucion.resultado, registroResultadoEjecucion.contenido, registroResultadoEjecucion.operacion, registroResultadoEjecucion.key);

                    // Envio el Paquete a ESI
                    if(send(socketESI,paquete.buffer,paquete.tam_buffer,0) != -1){
                        free(paquete.buffer);

                        log_info(infoLogger, "Se le envió al ESI %s el Resultado de la Ejecución de la Instrucción",registroResultadoEjecucion.nombreEsiDestino);
                    }else{
                        log_error(infoLogger, "No se pudo enviar al ESI %s el Resultado de la Ejecución de la Instrucción", registroResultadoEjecucion.nombreEsiDestino);
                    }
                    break;

                case COMPACTACION_GLOBAL:

                    log_info(infoLogger,"Recepcion de la Instancia %s (Socket %d) del pedido de Compactacion Global", obtenerNombreProceso(listaProcesosConectados, i), i);

                    // Determino todas las Instancias que deben realizar la Compactacion Local
                    if(list_size(listaInstanciasConectadas) > 0){

                        void _each_elemento_(Instancia* registroProcesoAux)
                        {

                            // Si la Instancia es diferente de la que pidio la Compactacion Global
                            if(registroProcesoAux->socketProceso != i){

                                // Le aviso a las demas Instancias que realicen sus Compactaciones Locales
                                Paquete paquete= crearHeader('C',COMPACTACION_LOCAL,1);
                                if( send(registroProcesoAux->socketProceso,paquete.buffer,paquete.tam_buffer,1) != -1 ){
                                    log_info(infoLogger,"Se le envio a la Instancia %s el aviso para que realice su Compactacion Local", obtenerNombreProceso(listaProcesosConectados, registroProcesoAux->socketProceso));
                                }else{
                                    log_info(infoLogger,"No se pudo enviar a la Instancia %s (Socket %d) el aviso para que realice su Compactacion Local", obtenerNombreProceso(listaProcesosConectados, registroProcesoAux->socketProceso), registroProcesoAux->socketProceso);
                                }
                            }
                        }
                        list_iterate(listaInstanciasConectadas, (void*)_each_elemento_);
                    }
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
                    registroProcesoAux->socketProceso = i;
                    registroProcesoAux->nombreProceso = malloc(strlen(registroProceso.nombreProceso)+1);
                    strcpy( registroProcesoAux->nombreProceso ,registroProceso.nombreProceso);
                    registroProcesoAux->nombreProceso[strlen(registroProceso.nombreProceso)] = '\0';


                    // Cargo el Proceso en la Lista de Procesos conectados al Planificador
                    cargarListaProcesosConectados(listaProcesosConectados, registroProcesoAux);

                    // Muestro por pantalla el contenido de listaProcesosConectados
                    showContenidolistaProcesosConectados(listaProcesosConectados);
                    break;

                case EJECUTAR_INSTRUCCION:
                    paquete=recibir_payload(&i,&encabezado.tam_payload);
                    registroInstruccion=dsrlz_instruccion(paquete.buffer);
                    free(paquete.buffer);

                    log_info(infoLogger,"Pedido de Ejecución de una Instruccion recibida del Proceso ESI %s: %d %s %s", obtenerNombreProceso(listaProcesosConectados, i), registroInstruccion.operacion, registroInstruccion.key, registroInstruccion.dato);

                    // Aplico Retardo de Ejecucion segun Archivo de Configuracion
                    usleep(config_get_int_value(cfg,"RETARDO"));

                    // Si la operacion es GET, notificar al Planificador de la toma del recurso y la Instancia no participa
                    if(registroInstruccion.operacion == GET){

                        // Serializado el Proceso y la Key
                        paquete = srlz_datosKeyBloqueada('C', NOTIFICAR_USO_RECURSO, obtenerNombreProceso(listaProcesosConectados, i), registroInstruccion.operacion, registroInstruccion.key, registroInstruccion.dato);

                        // Si el Recurso no fue creado en el diccionarioClavesInstancias (primera vez), lo creo
                        if(!dictionary_has_key(diccionarioClavesInstancias, registroInstruccion.key) ){

                            // Creo Registro Nuevo pero VACIO
                            proximaInstancia = malloc(sizeof(Instancia));
                            proximaInstancia->nombreProceso = NULL;
                            proximaInstancia->socketProceso = 0;
                            proximaInstancia->entradasLibres = 0;

                            // Guardo en el Dictionary la Key
                            dictionary_put(diccionarioClavesInstancias, registroInstruccion.key, proximaInstancia);
                        }

                        // Envio el Paquetea al Planificador
                        if(send(fd_planificador,paquete.buffer,paquete.tam_buffer,0) != -1){

                            free(paquete.buffer);
                            log_info(infoLogger, "Se le notifica al PLANIFICADOR (Socket %d) que el Proceso ESI %s quiere acceder al Recurso %s.", fd_planificador, obtenerNombreProceso(listaProcesosConectados, i), registroInstruccion.key);
                        }else{
                            log_error(infoLogger, "No se pudo enviar mensaje al PLANIFICADOR sobre el uso de un Recurso por el Proceso ESI %s.", obtenerNombreProceso(listaProcesosConectados, i));
                        }
                        break;

                    }else{ // Si la operacion es SET o STORE

                        // Le aviso al Planificador para que Libere el Recurso
                        if(registroInstruccion.operacion == STORE){
                            

                            // Si el Recurso no fue creado previamente, entonces se cancela el ESI por "Error de Clave no Identificada"
                            if(!dictionary_has_key(diccionarioClavesInstancias, registroInstruccion.key) ){

                                // Armo el Paquete del Resultado de la Ejecucion de la Instruccion
                                paquete = srlz_resultadoEjecucion('C', RESPUESTA_EJECUTAR_INSTRUCCION, "COORDINADOR", EJECUCION_FALLIDA_FINALIZAR_ESI, "Error de Clave no Identificada", registroInstruccion.operacion, registroInstruccion.key);

                                // Envio el Paquetea a ESI
                                if(send(i,paquete.buffer,paquete.tam_buffer,0) != -1){

                                    free(paquete.buffer);
                                    log_info(infoLogger, "Se cancela el ESI %s por Error de Clave %s no Identificada", obtenerNombreProceso(listaProcesosConectados, i), registroKeyBloqueada.key);
                                }else{
                                    log_error(infoLogger, "No se pudo notificar al ESI %s por Error de Clave %s no Identificada", obtenerNombreProceso(listaProcesosConectados, i), registroKeyBloqueada.key);
                                }
                                break;
                            }
                        }

                        if(registroInstruccion.operacion == SET){

                            // Si el Recurso NO esta bloqueado previamente, se cancela el ESI por "Error de Clave no Bloqueada"
                            if(!dictionary_has_key(diccionarioClavesBloqueadas, registroInstruccion.key) ){


                                // Armo el Paquete del Resultado de la Ejecucion de la Instruccion
                                paquete = srlz_resultadoEjecucion('C', RESPUESTA_EJECUTAR_INSTRUCCION, "COORDINADOR", EJECUCION_FALLIDA_FINALIZAR_ESI, "Error de Clave no Bloqueada", registroInstruccion.operacion, registroInstruccion.key);

                                // Envio el Paquetea a ESI
                                if(send(i,paquete.buffer,paquete.tam_buffer,0) != -1){

                                    free(paquete.buffer);
                                    log_info(infoLogger, "Se cancela el ESI %s por Error de Clave %s no Bloqueada", obtenerNombreProceso(listaProcesosConectados, i), registroInstruccion.key);
                                }else{
                                    log_error(infoLogger, "No se pudo notificar al ESI %s por Error de Clave %s no Bloqueada", obtenerNombreProceso(listaProcesosConectados, i), registroInstruccion.key);
                                }
                                break;
                            }
                        }

                        // Si el Recurso ya fue recibido previamente por el Coordinador (2da y demas veces)
                        if(dictionary_has_key(diccionarioClavesInstancias, registroInstruccion.key) ){

                            // Obtener la Instancia ya asignado al Recurso
                            proximaInstancia = obtenerInstanciaAsignada(diccionarioClavesInstancias,registroInstruccion.key);

                            // Si el Recurso habia tenido un GET pero todavia nunca se asigno una Instancia
                            if(proximaInstancia->nombreProceso == NULL){

                                // Defino el Algoritmo de Distribucion a utlizar
                                char* algoritmoDistribucion = string_new();
                                string_append(&algoritmoDistribucion,config_get_string_value(cfg,"ALGORITMO_DISTRIBUCION"));


                                // Obtengo la Instancia segun el Algoritmo de Distribucion
                                //proximaInstancia = NULL;
                                proximaInstancia = obtenerInstanciaNueva(listaInstanciasConectadas,&registroInstruccion,algoritmoDistribucion);

                                log_info(infoLogger, "Se aplico el Algoritmo de Distribución %s y se obtuvo la Instancia %s", algoritmoDistribucion, proximaInstancia->nombreProceso);

                                // Actualizo el diccionario con la Instancia nueva asignada
                                actualizarDiccionarioClavesInstancias(diccionarioClavesInstancias, registroInstruccion.key, proximaInstancia);

                                free(algoritmoDistribucion);
                            }

                            log_info(infoLogger, "Se reconoció que el Recurso %s lo tiene asignado la Instancia %s", registroInstruccion.key, proximaInstancia->nombreProceso);

                        }else{ // Si el Recurso nunca fue atendido por una Instancia

                            // Defino el Algoritmo de Distribucion a utlizar
                            char* algoritmoDistribucion = string_new();
                            string_append(&algoritmoDistribucion,config_get_string_value(cfg,"ALGORITMO_DISTRIBUCION"));


                            // Obtengo la Instancia segun el Algoritmo de Distribucion
                            proximaInstancia = obtenerInstanciaNueva(listaInstanciasConectadas,&registroInstruccion,algoritmoDistribucion);


                            log_info(infoLogger, "Se aplico el Algoritmo de Distribución %s y se obtuvo la Instancia %s", algoritmoDistribucion, proximaInstancia->nombreProceso);

                            // Guardo en el Dictionary que Instancia posee un Key
                            dictionary_put(diccionarioClavesInstancias, registroInstruccion.key, proximaInstancia);  
                            free(algoritmoDistribucion);
                        }
                        
                        // Envio el Paquetea a la Instancia
                        if(enviarInstruccionInstancia(registroInstruccion, proximaInstancia->socketProceso)){
                            log_info(infoLogger, "Se le envio a la instancia %s la proxima Instruccion a ejecutar", proximaInstancia->nombreProceso);

                            // Genero el Log de Operaciones
                            registrarLogOperaciones(listaProcesosConectados, registroInstruccion.operacion, registroInstruccion.key, registroInstruccion.dato, i);
                            log_info(infoLogger,"Operacion guardada en el Log de Operaciones:  %s %i %s %s", obtenerNombreProceso(listaProcesosConectados, i), registroInstruccion.operacion, registroInstruccion.key, registroInstruccion.dato);

                        }else{
                            log_error(infoLogger, "Se detecto que la Instancia %s esta desconectada.",proximaInstancia->nombreProceso);

                            // ESI Aborta por desconexión de la instancia

                            // Armo el Paquete del Resultado de la Ejecucion de la Instruccion
                            paquete = srlz_resultadoEjecucion('C', RESPUESTA_EJECUTAR_INSTRUCCION, "COORDINADOR", EJECUCION_FALLIDA_FINALIZAR_ESI, "Error de Clave Innaccesible", registroInstruccion.operacion, registroInstruccion.key);

                            // Envio el Paquetea a ESI
                            if(send(i,paquete.buffer,paquete.tam_buffer,0) != -1){

                                free(paquete.buffer);
                                log_info(infoLogger, "Se cancela el ESI %s por Desconexión de la Instancia %s", obtenerNombreProceso(listaProcesosConectados, i), proximaInstancia->nombreProceso);
                            }else{
                                log_error(infoLogger, "No se pudo notificar al ESI %s por Desconexión de la Instancia %s", obtenerNombreProceso(listaProcesosConectados, i), proximaInstancia->nombreProceso);
                            }
                        }

                    }
                    break;

                case FINALIZACION_EJECUCION_ESI:
                    log_info(infoLogger,"Notificacion sobre la finalizacion del Proceso ESI %s.", obtenerNombreProceso(listaProcesosConectados, i));

                    // Elimino el Proceso ESI de la listaProcesosConectados
                    eliminarProcesoLista(listaProcesosConectados, i);

                    // Libero los Recursos que tenia asignado en Lista de Claves Bloqueadas
                    liberarRecursosProceso(diccionarioClavesBloqueadas, obtenerNombreProceso(listaInstanciasConectadas, i));                                    
                    break;  

                case ESI_MUERE:
                    log_info(infoLogger,"Notificacion sobre la finalizacion del Proceso ESI %s via el comando KILL.", obtenerNombreProceso(listaProcesosConectados, i));

                    // Elimino el Proceso ESI de la listaProcesosConectados
                    eliminarProcesoLista(listaProcesosConectados, i);

                    // Libero los Recursos que tenia asignado en Lista de Claves Bloqueadas
                    liberarRecursosProceso(diccionarioClavesBloqueadas, obtenerNombreProceso(listaInstanciasConectadas, i));   

                    break;                              
            }
        }
    }                    

    pthread_exit(EXIT_SUCCESS);
    return 0;
}


int main(int argc, char* argv[]){
	
	/* Creo la instancia del Archivo de Configuracion y del Log */
	cfg = config_create("config/config.cfg");
	infoLogger = log_create("log/coordinador.log", "Coordinador", false, LOG_LEVEL_INFO);

	log_info(infoLogger, "Iniciando COORDINADOR" );
    printf("Iniciando COORDINADOR\n");

    // Creo el Servidor para escuchar conexiones
    int servidor=crearServidor(config_get_int_value(cfg,"COORDINADOR_PUERTO"));
    log_trace(infoLogger, "Escuchando conexiones" );


    // Creo la lista de Todos los Procesos conectados al Coordinador
    listaProcesosConectados = list_create();

    // Creo la lista de Todas las Instancias Conectadas y la Cantidad de Entradas Libres
    listaInstanciasConectadas = list_create();

    // Creo el Diccionario con las Claves y que Instancia la tiene
    diccionarioClavesInstancias = dictionary_create();

    // Creo la Lista de Claves Bloqueadas (Estructura Administrativa para evitar comunicarnos con el Planificador)
    diccionarioClavesBloqueadas = dictionary_create();

// -----------------------------------------------------------------------

    int new_fd;

    while(true){

        // Acepto todas las conexiones
        new_fd = aceptarConexionCliente(servidor);

        if (new_fd != -1) {

            // Creo un Hilo por cada nueva Conexion
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

    list_destroy(listaProcesosConectados);
    list_destroy(listaInstanciasConectadas);
    dictionary_destroy(diccionarioClavesInstancias);
    dictionary_destroy(diccionarioClavesBloqueadas);

    return EXIT_SUCCESS;
}
