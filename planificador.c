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
#include <signal.h>
#include <unistd.h>
#include <sys/prctl.h>
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

    bool elAlgoritmoEsConDesalojo;
    int socketAux;

    bool respuestaEjecucionInstruccionEsi;
    bool ejecutarAlgoritmoPlanificacion;
    bool planificadorPausado;
    char* algoritmoPlanificacion = NULL;
    float alfa;
    int coordinador_fd;
    int rafagaActual;
    Proceso* procesoAnterior;
    char** arregloClavesInicialmenteBloqueadas = NULL;
    Proceso* procesoSeleccionado=NULL;
    pthread_t hiloConexiones;
    pthread_t hiloConsola;

/* ---------------------------------------------------- */
/*  Funcion para liberar recursos en el planificador    */
/* ---------------------------------------------------- */

//este liberar recursos, libera a las claves bloqueadas por el esi y tambien los procesos bloqueados que esperaban alguna de esas claves
void liberarRecursosProcesoPlanificador(t_dictionary * dictionario, char* nombreProceso){

	if(dictionary_size(dictionario) > 0){

    	void elemento_destroy(Proceso* self){
    		//free(self); // Explota!!!
    	}

    	void _each_elemento_(char* key, Proceso* registroProcesoAux){
    		if(registroProcesoAux->nombreProceso != NULL && strcmp(registroProcesoAux->nombreProceso, nombreProceso) == 0) {

				bool seEncontroProceso = false;
				KeyBloqueada* registroKeyBloqueadaAux;
				int i=0;
				//se remueven los procesos que esperaban alguna de esas claves. Para eso se recorre la lista
				//y se saca de la cola de bloqueados al primer proceso que esperara esa clave
				while(i<list_size(listaClavesBloqueadasRequeridas) && !seEncontroProceso){
					registroKeyBloqueadaAux = list_get(listaClavesBloqueadasRequeridas, i);
					if(!strcmp(registroKeyBloqueadaAux->key, key)){
					    int socketDelProcesoConClaveRequerida = obtenerSocketProceso(listaESIconectados, registroKeyBloqueadaAux->nombreProceso);
					    eliminarProcesoCola(colaBloqueados, socketDelProcesoConClaveRequerida);
					    cargarProcesoCola(listaESIconectados, colaReady, socketDelProcesoConClaveRequerida);
					    cargarProcesoLista(listaESIconectados, listaReady, socketDelProcesoConClaveRequerida);
					    list_remove_and_destroy_element(listaClavesBloqueadasRequeridas, i,(void*)liberarKeyBloqueada);
					    seEncontroProceso = true;
					}
					i++;
				}

    			//funcion auxiliar para la lista
    			bool eseMismoProcesoEstabaEnLaLista (KeyBloqueada* registroKeyBloqueadaAux){
    				return (!strcmp(registroKeyBloqueadaAux->nombreProceso, nombreProceso));
    			}
    			//se remueve ese mismo proceso de la lista. Este romeve solo daria true cuando se mata un proceso
    			list_remove_and_destroy_by_condition(listaClavesBloqueadasRequeridas, (void*)eseMismoProcesoEstabaEnLaLista, (void*)liberarKeyBloqueada);
    			dictionary_remove_and_destroy(dictionario, key, (void*) elemento_destroy);
    		}
    	}
    	dictionary_iterator(dictionario, (void*)_each_elemento_);
    }

}


/* ---------------------------------------- */
/*  Funcion de Planificacion de Procesos    */
/* ---------------------------------------- */
void planificarProcesos(){
    // Planifica los Procesos de la ColaReady
    if(!planificadorPausado && respuestaEjecucionInstruccionEsi){

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

 
/*            
            //este if solo lo pongo para informar qué proceso se selecciono en la planificacion
            Rafagas* rafagasAux2=NULL;
            if(procesoSeleccionado !=NULL){
                rafagasAux2= dictionary_get(diccionarioRafagas, procesoSeleccionado->nombreProceso);
                printf("\nProceso planificado para ejecutar: %s\n", procesoSeleccionado->nombreProceso);
            }
*/
            rafagaActual=0;
        }

        // Desactivo la Planificacion de los Procesos
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
            Paquete paquete = crearHeader('P', EJECUTAR_INSTRUCCION, 1);

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
    KeyBloqueada* registroKeyBloqueada;

    // Cambio el Nombre del Hilo para poder verlo en HTOP
    char* nombreHilo = string_new();
    string_append_with_format(&nombreHilo, "./P_Consola\0");

    prctl(PR_SET_NAME,nombreHilo,0,0,0);
    free(nombreHilo);

    while (true){
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

                    // Ejecutar el Planificador de Procesos
                    planificarProcesos();
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

                        //Se verifica si un proceso estaba bloqueado esperando esa clave
                        registroKeyBloqueada = sacarProcesoConClaveBloqueadaDeLaLista(listaClavesBloqueadasRequeridas, parametrosConsolaOriginal[1]);

                        //de ser asi, se elimina el proceso de la cola de bloqueados y se pone en la cola de Ready
                        //ademas, si el algoritmo de planificacion es con desalojo o si es el unico en la lista de Ready, se activa la planificacion
                        if(registroKeyBloqueada!=NULL){

                        	socketAux = obtenerSocketProceso(listaESIconectados,registroKeyBloqueada->nombreProceso);
                        	eliminarProcesoCola(colaBloqueados, socketAux);
                            cargarProcesoCola(listaESIconectados, colaReady, socketAux);
                            cargarProcesoLista(listaESIconectados, listaReady, socketAux);
                        	liberarKeyBloqueada(registroKeyBloqueada);
                        	if(list_size(listaReady)==1 && queue_size(colaEjecucion)==0) ejecutarAlgoritmoPlanificacion=true;
                        	else if(elAlgoritmoEsConDesalojo) ejecutarAlgoritmoPlanificacion=true;

                        }

                        // Ejecutar el Planificador de Procesos
                        planificarProcesos();

                    }else{
                        printf("[Error] Cantidad de parámetros incorrectos\n");
                    }

                }

                if(string_starts_with(comandoConsola,"LISTAR")){
                    comandoAceptado = true;

                    // Si se ingreso un solo parametro
                    if(countParametrosConsola(comandoConsola) == 1){

                        // Lista los Procesos que quiere usar el Recurso indicado por consola
                        listarRecursosBloqueados(listaClavesBloqueadasRequeridas, diccionarioClavesBloqueadas, parametrosConsolaOriginal[1]);

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
                            liberarRecursosProcesoPlanificador(diccionarioClavesBloqueadas, parametrosConsolaOriginal[1]);
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
                            if(list_size(listaReady)==1 && queue_size(colaEjecucion)==0) ejecutarAlgoritmoPlanificacion=true;
                            if(elAlgoritmoEsConDesalojo) ejecutarAlgoritmoPlanificacion=true;

                            planificarProcesos();
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
                            log_info(infoLogger, "Se le pide al COORDINADOR infomarción sobre el Recurso %s.", parametrosConsolaOriginal[1]);
                        }else{
                            printf("No se pudo pedir infomacion al COORDINADOR sobre el Recurso %s\n", parametrosConsolaOriginal[1]);
                        }
                    }else{
                        printf("[Error] Cantidad de parámetros incorrectos\n");
                    }
                }

                if(string_starts_with(comandoConsola,"DEADLOCK")){
                    comandoAceptado = true;
                    t_list* listaDeadlock = list_create();
                    //va almacenando cada conjunto de deadlocks
                    t_list* listaDeadlockAux = list_create();
                    t_list* listaProcesosEnInanicion = list_create();

					KeyBloqueada* elementoOriginal;
					//aca se guardaria el proceso que tiene una clave que bloquea a otro proceso. Se saca del diccionario
					KeyBloqueada* registroKeyBloqueadaPorProceso;
					//aca se guardaria el proceso que tiene una clave que bloquea a otro proceso, y ademas esta bloqueado. Se saca de la lista de claves requeridas
					KeyBloqueada* registroKeyBloqueadaAux;
					bool hayDeadlock = false;
					bool puedeHaberDeadlock = true;

					bool elElementoEstaEnListaDeKeyBloqueadas(t_list* listaDeKeyBloqueadas, KeyBloqueada* elemento){
						bool elElementoEstaEnLaLista(KeyBloqueada* keyBloqueadaAux){
							return (!strcmp(elemento->nombreProceso, keyBloqueadaAux->nombreProceso));
						}
						return list_any_satisfy(listaDeKeyBloqueadas, (void*)elElementoEstaEnLaLista);
					}

					//se analiza cada elemento de la lista en busca de deadlock y se arma la lista de procesos en deadlock
					void iterate_lista(KeyBloqueada* registroKeyBloqueada){

						//se guarda en una variable auxiliar el primer proceso por el cual se comenzo a buscar el deadlock (la espera circular)
						elementoOriginal = registroKeyBloqueada;

						//si el elemento no se encuentra el la lista de deadlock, se verifica si esta en deadlock con otros
						if(!elElementoEstaEnListaDeKeyBloqueadas(listaDeadlock, registroKeyBloqueada)){

							//por el momento se agrega en la lista de deadlock auxiliar
							list_add(listaDeadlockAux,registroKeyBloqueada);

							while(!hayDeadlock && puedeHaberDeadlock){

								//busco si la clave que necesita este proceso para desbloquearse la tiene algun proceso bloqueado. Si no la tiene ninguno, no hay deadlock
								if( dictionary_has_key(diccionarioClavesBloqueadas,registroKeyBloqueada->key)){
									//guardo el proceso que tiene esa clave
									registroKeyBloqueadaPorProceso = dictionary_get(diccionarioClavesBloqueadas,registroKeyBloqueada->key);

									//se verifica si el proceso que tiene esa clave tambien esta bloqueado. Si no lo esta, no hay deadlock
									if(elElementoEstaEnListaDeKeyBloqueadas(listaClavesBloqueadasRequeridas, registroKeyBloqueadaPorProceso)){

										bool findProceso (KeyBloqueada* keyBloqueadaAux){
											return (!strcmp(registroKeyBloqueadaPorProceso->nombreProceso,keyBloqueadaAux->nombreProceso));
										}

										//guardo el proceso que bloquea al anterior
										registroKeyBloqueadaAux = list_find(listaClavesBloqueadasRequeridas, (void*)findProceso);

										//si el nombre del proceso es igual al primero por el que se comenzo
										//significa que se hayo la espera circular, y por lo tanto hay deadlock
										if(!strcmp(registroKeyBloqueadaAux->nombreProceso, elementoOriginal->nombreProceso)){
											hayDeadlock = true;
										}
										//si ese proceso que lo bloquea esta en deadlock, significa que el proceso por el que se estaba vrificando si
										//estaba en deadlock, esta solo en inanicion, o sea, no esta en deadlock
										else if(elElementoEstaEnListaDeKeyBloqueadas(listaDeadlock, registroKeyBloqueadaAux)){
											puedeHaberDeadlock = false;
										}
										//sino, significa que todavia puede estar en deadlock
										else{
											//lo agrego a la lista auxiliar l
											list_add(listaDeadlockAux,registroKeyBloqueadaAux);
											//lo igualo para seguir buscando la espera circular
											registroKeyBloqueada = registroKeyBloqueadaAux;
										}
									}
									else {
										puedeHaberDeadlock = false;
									}
								}
								else {
									puedeHaberDeadlock = false;
								}
							}
							//si hubo deadlock, recien ahi agrego todos los procesos de la lista auxiliar a la de verdad
							if(hayDeadlock){
								list_add_all(listaDeadlock,listaDeadlockAux);
							}
							//si no esta en deadlock, esta en inanicion
							else list_add(listaProcesosEnInanicion, elementoOriginal);

							//limpio la lista para volver a buscar otro deadlock
							list_clean(listaDeadlockAux);

							//reinicio los valores
							hayDeadlock = false;
							puedeHaberDeadlock = true;
						}
					}

					//se analiza cada elemento de la lista en busca de deadlock y se arma la lista de procesos en deadlock
					list_iterate(listaClavesBloqueadasRequeridas, (void*)iterate_lista);

					void informarNombreProcesos(KeyBloqueada* keyBloqueadaAux){
						printf("%s\n", keyBloqueadaAux->nombreProceso);
					}

					//muestro por pantalla los procesos en deadlock
					if(list_size(listaDeadlock)>0){

						printf("\nPROCESOS EN DEADLOCK:\n\n");
						list_iterate(listaDeadlock, (void*)informarNombreProcesos);
						list_clean(listaDeadlock);

					}
					else printf("\nNO HAY PROCESOS EN DEADLOCK\n");
					printf("\n");

					//muestro por pantalla los procesos en inanicion
					if(list_size(listaProcesosEnInanicion)>0){
						printf("PROCESOS EN INANICION:\n\n");
						list_iterate(listaProcesosEnInanicion, (void*)informarNombreProcesos);
						list_clean(listaProcesosEnInanicion);
					}
					else printf("NO HAY PROCESOS EN INANICION\n");
					printf("\n");

					list_destroy(listaProcesosEnInanicion);
					list_destroy(listaDeadlock);
					list_destroy(listaDeadlockAux);
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
    pthread_exit(EXIT_SUCCESS);

    return 0;
}


void* atenderConexiones(void* socketConexion){

    pthread_informacion* data_hilo = (pthread_informacion*) socketConexion;
    int i = data_hilo->socketHilo;
    free(data_hilo);

    int nbytes;

    Proceso registroProceso;
    KeyBloqueada keyBloqueada;
	KeyBloqueada* registroKeyBloqueada;
    StatusRecurso registroStatusRecurso;

    int indice = 0, resultadoEjecucion;
    bool recursoOcupado;
    Paquete paquete;
    Encabezado encabezado;

    // Cambio el Nombre del Hilo para poder verlo en HTOP
    char* nombreHilo = string_new();

    if(coordinador_fd == i){
        string_append_with_format(&nombreHilo, "./P_Coordinador\0");
    }else{
        string_append_with_format(&nombreHilo, "./P_S_%d\0", i);
    }

    prctl(PR_SET_NAME,nombreHilo,0,0,0);
    free(nombreHilo);

    while(true){

        // Recibo el Encabezado del Paquete
        encabezado=recibir_header(&i);

        if((nbytes = encabezado.tam_payload) <= 0){
            // error o conexión cerrada por el cliente
            if(nbytes == 0){
                printf("Socket %d se ha caído\n", i);
            }else{
                perror("recv failed");
            }            
            close(i); // Cierro el Socket por desconexion


            // Si el Socket caido es del Coordinador, finaliza el Planificador
            if(coordinador_fd == i){
                printf("El Coordinador ha finalizado. El Sistema quedó en un estado INVALIDO!!.\n");

                // Fuerzo el cierre de la Consola y la Finalizacion del Proceso Principal
                pthread_kill(hiloConsola, SIGTERM);
                killpg(getpid(),SIGTERM);

                return 0;
            }
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
                    registroRafagaAux->estimacionRafagaAnterior = registroRafagaAux->proximaEstimacion;
                    registroRafagaAux->tiempoDeEsperaDeCpu=0;
                    dictionary_put(diccionarioRafagas,registroProcesoAux->nombreProceso,registroRafagaAux);


                    // Activo la Planificacion de los Procesos

                    if(list_size(listaReady)==1 && queue_size(colaEjecucion)==0) ejecutarAlgoritmoPlanificacion=true;
                    if(elAlgoritmoEsConDesalojo) ejecutarAlgoritmoPlanificacion=true;
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
                    respuestaEjecucionInstruccionEsi=true;

                    break;

                case FINALIZACION_EJECUCION_ESI:
                    log_info(infoLogger,"Notificacion sobre la finalizacion del Proceso ESI %s.", obtenerNombreProceso(listaESIconectados, i));

                    // Libero los Recursos que tenia asignado en Lista de Claves Bloqueadas
                    liberarRecursosProcesoPlanificador(diccionarioClavesBloqueadas, obtenerNombreProceso(listaESIconectados, i));

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
                    keyBloqueada = dsrlz_datosKeyBloqueada(paquete.buffer);
                    free(paquete.buffer);

                    log_info(infoLogger,"Notificacion del COORDINADOR que el Proceso ESI %s libera el Recurso %s.", keyBloqueada.nombreProceso, keyBloqueada.key);

                    // Libero un Recurso de la Lista de Claves Bloqueadas
                    dictionary_remove(diccionarioClavesBloqueadas, keyBloqueada.key);

                    //Se verifica si un proceso estaba bloqueado esperando esa clave
                    registroKeyBloqueada = sacarProcesoConClaveBloqueadaDeLaLista(listaClavesBloqueadasRequeridas, keyBloqueada.key);

                    //de ser asi, se elimina el proceso de la cola de bloqueados y se pone en la cola de Ready
                    //ademas, si el algoritmo de planificacion es con desalojo  se activa la planificacion
                    if(registroKeyBloqueada!=NULL){

                    	socketAux = obtenerSocketProceso(listaESIconectados,registroKeyBloqueada->nombreProceso);
                    	eliminarProcesoCola(colaBloqueados, socketAux);
                        cargarProcesoCola(listaESIconectados, colaReady, socketAux);
                        cargarProcesoLista(listaESIconectados, listaReady, socketAux);
                    	liberarKeyBloqueada(registroKeyBloqueada);
                    	if(elAlgoritmoEsConDesalojo) ejecutarAlgoritmoPlanificacion=true;

                    }

                    log_info(infoLogger,"Se libero el Recurso %s de la Lista de Claves Bloqueadas que lo tenia tomado el Proceso ESI %s.", keyBloqueada.key, keyBloqueada.nombreProceso);
                    break;


                case NOTIFICAR_USO_RECURSO:

                    // Recibo los datos del Key y Proceso
                    paquete = recibir_payload(&i,&encabezado.tam_payload);
                    keyBloqueada = dsrlz_datosKeyBloqueada(paquete.buffer);
                    registroKeyBloqueada = crearNodoDeUnaKeyBloqueada(keyBloqueada);
                    free(paquete.buffer);

                    log_info(infoLogger,"Notificacion del COORDINADOR que el Proceso ESI %s quiere acceder al Recurso %s.", registroKeyBloqueada->nombreProceso, registroKeyBloqueada->key);

                    recursoOcupado = false;

                    // Si el Recurso esta bloqueado por otro Proceso
                    if(dictionary_has_key(diccionarioClavesBloqueadas, registroKeyBloqueada->key) ){

                        // Guardo una Lista de las Claves Bloqueadas que quieren ser usadas por otros Procesos
                        list_add(listaClavesBloqueadasRequeridas, registroKeyBloqueada);

                        log_info(infoLogger,"El Recurso %s ya se encuentra tomado por otro Proceso ESI.", registroKeyBloqueada->key);

                        // Serializado el Proceso y la Key
                        paquete = srlz_datosKeyBloqueada('P', RECURSO_OCUPADO, registroKeyBloqueada->nombreProceso, registroKeyBloqueada->operacion,registroKeyBloqueada->key,registroKeyBloqueada->dato);

                        recursoOcupado = true;


                    }else{ // Si el Recurso no esta bloqueado

                        // Bloqueo el Recurso y lo cargo en la Lista de Claves Bloqueadas
                        dictionary_put(diccionarioClavesBloqueadas, registroKeyBloqueada->key, registroKeyBloqueada);

                        log_info(infoLogger,"Se agrego el Recurso %s en la Lista de Claves Bloqueadas que lo tomo el Proceso ESI %s.", registroKeyBloqueada->key, registroKeyBloqueada->nombreProceso);

                        // Serializado el Proceso y la Key
                        paquete = srlz_datosKeyBloqueada('P', RECURSO_LIBRE, registroKeyBloqueada->nombreProceso, registroKeyBloqueada->operacion,registroKeyBloqueada->key,registroKeyBloqueada->dato);
                    }


                    // Envio el Paquete al Coordinador con la notificacion
                    if(send(i,paquete.buffer,paquete.tam_buffer,0) != -1){

                        free(paquete.buffer);

                        if(recursoOcupado){
                            log_info(infoLogger, "Se le notifica al COORDINADOR que el Recurso %s ya estaba tomado por otro Proceso.", registroKeyBloqueada->key);
                        }else{
                            log_info(infoLogger, "Se le notifica al COORDINADOR que el Recurso %s estaba libre y ahora quedo tomado por el Proceso ESI %s.",registroKeyBloqueada->key, registroKeyBloqueada->nombreProceso);
                        }
                        
                    }else{
                        log_error(infoLogger, "No se pudo enviar la notificacion al COORDINADOR sobre el estado de  uso del Recurso %s.", registroKeyBloqueada->key);
                    }
                    break;

                case INSTANCIA_DESCONECTADA:
                    // TODO
                    log_info(infoLogger,"Respuesta sobre la Instancia que no existe recibida del COORDINADOR.");
                    break;

                case IS_ALIVE: 
                    break;

                case OBTENER_STATUS_CLAVE:
                    // Recibo la informacion del Recurso (Fue originado desde la Consola)
                    paquete = recibir_payload(&i,&encabezado.tam_payload);
                    registroStatusRecurso = dsrlz_datosStatusRecurso(paquete.buffer);
                    free(paquete.buffer);

                    printf("El Recurso %s posee el siguiente STATUS:\n", registroStatusRecurso.key);
                    printf("---------------------------------------\n");
                    printf("Instancia donde se encuentra el Recurso: %s\n", registroStatusRecurso.nombreInstanciaActual);
                    printf("Instancia donde se guardaría el Recurso: %s\n", registroStatusRecurso.nombreInstanciaFutura);
                    printf("Valor del Recurso: %s\n", registroStatusRecurso.valorRecurso);

                    log_info(infoLogger, "Se recibió del COORDINADOR la información sobre el Recurso %s. Instancia Actual: %s - Instancia Distribuida: %s - Valor: %s", registroStatusRecurso.key, registroStatusRecurso.nombreInstanciaActual, registroStatusRecurso.nombreInstanciaFutura, registroStatusRecurso.valorRecurso);                    
                    break;
            }
        }

        // Ejecutar el Planificador de Procesos
        planificarProcesos();

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
    
    if(string_starts_with(algoritmoPlanificacion,"SJF-CD")) elAlgoritmoEsConDesalojo=true;

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

    pthread_create(&hiloConsola, NULL, (void*) hiloConsolaInteractiva, NULL);
    pthread_detach(hiloConsola);

    // Creo Hilo que Atiende al Coordinador
    pthread_t hiloConexionCoordinador;

    pthread_informacion* data_hilo = (pthread_informacion*) malloc(sizeof(*data_hilo));
    data_hilo->socketHilo = coordinador_fd;

    log_info(infoLogger,"Creando un hilo para atender al Coordinador con FD %d", coordinador_fd);
    pthread_create(&hiloConexiones, NULL, (void*) atenderConexiones, data_hilo);
    pthread_detach(hiloConexiones);

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
        }else{
            // Si no se pudo aceptar la conexion, finalizo TODO
            pthread_kill(hiloConsola, SIGTERM);
            pthread_kill(hiloConexiones, SIGTERM);
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
