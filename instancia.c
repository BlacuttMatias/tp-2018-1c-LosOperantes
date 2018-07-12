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
#include <time.h>
#include <signal.h>

#include <sys/types.h>
#include <dirent.h>

#include "funciones.h"
#include "registros.h"
#include "sockets.h"


/* ---------------------------------------- */
/*  Variables Globales                      */
/* ---------------------------------------- */
    int entradas;
    int espacioPorEntrada;
    t_list* tablaEntradas;
    Almacenamiento almacenamiento;
    char* puntoMontaje;
    int intervaloDump;
    bool seCompacto = false;
    int entradasLibres;

/* ---------------------------------------- */


// Gestor del Intervalo del Dump
void handle_alarm(int sig) {

    printf("Dump automatizado cada %d segundos...\n", intervaloDump);

    //Realizo el Dump de la Tabla de Entradas
    dump(tablaEntradas, puntoMontaje, almacenamiento);

    alarm(intervaloDump);
}


int main(int argc, char* argv[]){
    int* puntero=malloc(sizeof(int));
    *puntero=0;
    /* Creo la instancia del Archivo de Configuracion y del Log */
    cfg = config_create("config/config.cfg");
    infoLogger = log_create("log/instancia.log", "INSTANCIA", false, LOG_LEVEL_INFO);

    // Defino el Punto de Montaje
    puntoMontaje = string_new();
    puntoMontaje = config_get_string_value(cfg,"PUNTO_MONTAJE");

    // Defino el Intervalo del Dump
    intervaloDump = config_get_int_value(cfg,"INTERVALO_DUMP");

    char* flagClean = string_new();

    /*************************************************
     *
     * Se obtienen parametros por consola
     *
     ************************************************/

        if(argc > 1){
            string_append(&flagClean,argv[1]);

            if(strcmp(flagClean,"--clean") == 0){
                printf("Limpiando la Instancia...\n");

                // Borro storage, bitmap y contenido de montaje
                limpiarInstancia(puntoMontaje);
            }

        }

    /* ************************************************/

    log_info(infoLogger, "Iniciando INSTANCIA" );


    // Me aseguro que la estructura de directorios del Punto de Montaje este creada previamente
    if(crearEstructuraDirectorios( puntoMontaje ) ){
        log_info(infoLogger, "Se creo la estructura del Punto de Montaje" );
    }

	// Creo la lista de Tabla de Entradas
	tablaEntradas = list_create(); 
	
    // Defino el Intervalo del DUMP y lo planifico con Alarm
    struct sigaction sa;
    sa.sa_handler = &handle_alarm;
    sa.sa_flags = SA_RESTART;
    sigfillset(&sa.sa_mask);
    sigaction(SIGALRM, &sa, NULL);
    alarm(intervaloDump);


	printf("Iniciando INSTANCIA\n");


	Encabezado encabezado;
	Paquete paquete;
    int nbytes;

    // Creo conexión con el Coordinador
    int coordinador_fd = conectarseAservidor(config_get_string_value(cfg,"COORDINADOR_IP"),config_get_int_value(cfg,"COORDINADOR_PUERTO"));

    if(coordinador_fd == -1){
        printf("Error de conexion con el Coordinador\n");
        return EXIT_FAILURE;        
    }else{
        log_info(infoLogger, "Conexion establecida con el Coordinador");        
    }

    // Defino el Algoritmo de Reemplazo a utlizar
    char* algoritmoReemplazo = string_new();
    string_append(&algoritmoReemplazo,config_get_string_value(cfg,"ALGORITMO_REEMPLAZO"));


    // Serializado el Proceso
    paquete = srlz_datosProceso('I', HANDSHAKE, config_get_string_value(cfg,"INSTANCIA_NOMBRE"), INSTANCIA, 0);

    // Envio al Coordinador el Handshake y Serializado el Proceso
    send(coordinador_fd,paquete.buffer,paquete.tam_buffer,0);
    free(paquete.buffer);


// -----------------------------------------------------------------------
//    Prueba de funciones 2
// -----------------------------------------------------------------------

/*

    Instruccion* datosInstruccion;




    if(persistirDatos(almacenamiento,datosInstruccion, algoritmoReemplazo,puntero)){
        // Proceso a realizar si se persistiron correctamente los datos
    }else{
        // Proceso a realizar si fallo la persistencia
    }

*/


// -----------------------------------------------------------------------

    EntradasIntancias registroEntradasIntancias;
    Instruccion registroInstruccion;
    char cero='0';
    int uno='1';
    int contador=0;
    while(true){

		encabezado=recibir_header(&coordinador_fd);

		// gestionar datos de un cliente
		if ((nbytes = encabezado.tam_payload) <= 0) {
			 // error o conexión cerrada por el cliente
			 if (nbytes == 0) {
			 // conexión cerrada
			 	printf("Coordinador se ha caído\n");
			 } else {
			 	perror("recv");
			 }
			 close(coordinador_fd);
             return 0;
		} else {

			switch(encabezado.cod_operacion){

				case EJECUTAR_INSTRUCCION:

                    paquete=recibir_payload(&coordinador_fd,&encabezado.tam_payload);
                    registroInstruccion=dsrlz_instruccion(paquete.buffer);
                    free(paquete.buffer);

                    log_info(infoLogger,"Pedido de Ejecución de una Instruccion recibida del Coordinador: %d %s %s", registroInstruccion.operacion, registroInstruccion.key, registroInstruccion.dato);


                    if(registroInstruccion.operacion == SET){
                        mostrarVectorBin(almacenamiento);
                        // TODO
                        // Persistir el Valor en el Archivo Binario
						//Prototipado
                        int nuevaPosicion;
                        t_entrada* entradaNueva;
                        t_list* entradasBorradas;
                    	if(existeEntradaEnTabla(tablaEntradas,registroInstruccion.key)){
                    		//si ya existe la entrada en la tabla

                    		bool esIgualA(t_entrada* unaEntrada){
                    			return(strcmp(unaEntrada->clave,registroInstruccion.key) == 0);
                    		}

                    		//la busco dentro de la tabla asi saco su numero de entrada por ende su posicion en el binario
                    		t_entrada* entradaEncontrada = list_find(tablaEntradas,(void*)esIgualA);
                        	int posicion = entradaEncontrada->numeroDeEntrada; //posicion del binario
                            //liberarEntradaEnVector(almacenamiento,entradaEncontrada);
                            int tamanioNuevo= entradasQueOcupaString(almacenamiento,registroInstruccion.dato);
                            if(entraEnPosicionActual(almacenamiento,entradaEncontrada,tamanioNuevo)){
                                liberarEntradaEnVector(almacenamiento,entradaEncontrada);
                                list_add(tablaEntradas,entradaEncontrada);
                                entradaEncontrada->tamanioValorAlmacenado=tamanioValorAlmacenado(registroInstruccion.dato);
                                escribirBinarioEnPosicion(almacenamiento,posicion,registroInstruccion.dato);
                                grabarEntradaEnVector(almacenamiento,posicion,entradaEncontrada);
                                entradasBorradas=list_create(); //esto para que no explote al preguntarle el size mas tarde

                            }
                        	//escribo en el binario
                            else{
                                liberarEntradaEnVector(almacenamiento,entradaEncontrada);
                                list_add(tablaEntradas,entradaEncontrada);
                        	    entradasBorradas=persistirDatos(almacenamiento,&registroInstruccion,algoritmoReemplazo,puntero,&seCompacto);
                            }




                    	}else{

                    		//si NO existe la entrada en la tabla
                    		//hay que ver si hay entradas libres
                    		//si hay se persiste en la siguiente entrada libre
                    		//si no hay hay que correr algoritmo de reemplazo


                            //escribo en el binario
                            entradasBorradas=persistirDatos(almacenamiento,&registroInstruccion,algoritmoReemplazo,puntero,&seCompacto);
                        }

                        //ahora tengo que avisarle a la instancia de todas las claves que fueron borradas
                        if(list_size(entradasBorradas) >0 ){ 

                            int i=0;
                            int cantidadEntradas= list_size(entradasBorradas);
                            //showContenidoTablaEntradas(entradasBorradas);

                            for(i=0;i<cantidadEntradas;i++){
                                t_entrada* entradaBorrada=list_remove(entradasBorradas,0);//aca va posicion 0 porque a medida que saco, la posicion 0 tiene na nueva entrada
                                borrarTxtClave(almacenamiento,entradaBorrada->clave,puntoMontaje);

                                //estoy usando la serializacion de key bloqueada para no hacer toda una nueva serializacion. lleno los strings con "nada" por si acaso para evitar errores
                                paquete = srlz_datosKeyBloqueada('I',KEY_DESTRUIDA,"nada",GET, entradaBorrada->clave,"nada");

                                if( send(coordinador_fd,paquete.buffer,paquete.tam_buffer,1) != -1 ){
                                    log_info(infoLogger,"Se le envio al COORDINADOR el aviso para que borre el Recurso %s por ser reemplazado", entradaBorrada->clave);
                                }else{
                                    log_info(infoLogger,"No se pudo enviar al COORDINADOR el aviso para que borre el Recurso %s por ser reemplazado", entradaBorrada->clave);
                                }
                                free(paquete.buffer);
                                free(entradaBorrada->clave);
                                free(entradaBorrada);
                            }
                        }
                        list_destroy(entradasBorradas);
                        mostrarVectorBin(almacenamiento);
                    }
                    if(registroInstruccion.operacion == STORE){                                
                        // Realizo el Dump de la Tabla de Entradas
                        dump(tablaEntradas, puntoMontaje, almacenamiento);

                        //saco y agrego al final a la entrada storeada, para mantener orden de entradas segun uso
                    	if(existeEntradaEnTabla(tablaEntradas,registroInstruccion.key)){
                    		//si ya existe la entrada en la tabla
                            t_entrada* entradaEncontrada = obtenerEntrada(tablaEntradas,registroInstruccion.key);
                            int posicionEntradaEncontrada=posicionEntradaEnLista(almacenamiento,entradaEncontrada);
                            list_remove(tablaEntradas,posicionEntradaEncontrada);
                            list_add(tablaEntradas,entradaEncontrada);

                    		}

                    		
                    		
                    }


                    // Muestro el contenido de la Tabla de Entradas
                    showContenidoTablaEntradas(tablaEntradas);

                    //PARA UTILIZAR PARA INICIAR LA COMPACTACION GLOBAL
                    if(seCompacto){
                        log_info(infoLogger,"Se realizo la Compactacion Local en la Instancia %s", config_get_string_value(cfg,"INSTANCIA_NOMBRE"));

                        // Le aviso al Coordinador para que las demas Instancias realicen sus Compactaciones
                        paquete= crearHeader('I',COMPACTACION_GLOBAL,1);
                        if( send(coordinador_fd,paquete.buffer,paquete.tam_buffer,1) != -1 ){
                            log_info(infoLogger,"Se le envio al COORDINADOR el aviso para que las demas Instancias realicen sus Compactaciones Locales");
                        }else{
                            log_info(infoLogger,"No se pudo enviar al COORDINADOR el aviso para que las demas Instancias realicen sus Compactaciones Locales");
                        }
                        free(paquete.buffer);
                        seCompacto=false;
                    }

                    // TODO
                    // Determinar si fallo o no y corregir el mensaje de abajo. Ahora esta harcodeado a EJECUCION_EXITOSA

                    entradasLibres = espacioLibre(almacenamiento);

                    // Armo el Paquete del Resultado de la Ejecucion de la Instruccion
                    paquete = srlz_resultadoEjecucion('I', RESPUESTA_EJECUTAR_INSTRUCCION, registroInstruccion.nombreEsiOrigen, EJECUCION_EXITOSA, "", registroInstruccion.operacion, registroInstruccion.key);

                    // Envio el Paquete al Coordinador
                    if(send(coordinador_fd,paquete.buffer,paquete.tam_buffer,0) != -1){
                        log_info(infoLogger, "Se le notificó al COORDINADOR el resultado de la ejecución de la Instrucción");

                    }else{
                        log_error(infoLogger, "No se pudo notificar al COORDINADOR el resultado de la ejecución de la Instrucción");
                    }
                    //se le envian al coordinador las entradas libres de la instancia. En el crearHeader se le suma 100 a las
                    //entradas libres porque el tamanio del payload no puede ser 0, por los sockets. Luego, cuando lo reciba el coordinador, debera restarle 100
                    paquete = crearHeader('I', INFORMAR_ENTRADAS_LIBRES, entradasLibres+100);
                    if(send(coordinador_fd,paquete.buffer,paquete.tam_buffer,0) != -1){
                    	log_info(infoLogger, "Se le informó al COORDINADOR que hay (%d) entradas libres", entradasLibres);
                    }
                    else{
                    	log_error(infoLogger, "No se pudo informar al COORDINADOR las entradas libres disponibles");
                    }

                    free(paquete.buffer);
                    //free(registroInstruccion.nombreEsiOrigen);
                    //free(registroInstruccion.dato);
					break;

                case OBTENER_STATUS_VALOR:

                    paquete=recibir_payload(&coordinador_fd,&encabezado.tam_payload);
                    registroInstruccion=dsrlz_instruccion(paquete.buffer);
                    free(paquete.buffer);

                    log_info(infoLogger,"Pedido del Coordinador sobre el Valor del Recurso %s", registroInstruccion.key);


                    char* valorObtenido = string_new();
                    t_entrada* registroEntrada;

                    // Obtengo el registro de la Tabla de Entradas para el Recurso solicitado
                    registroEntrada = obtenerEntrada(tablaEntradas, registroInstruccion.key);

                    // Obtengo el Valor del Recurso guardado en el Binario
                    valorObtenido = leerBinarioEnPosicion(almacenamiento, registroEntrada->numeroDeEntrada);

                    // Armo el Registro de la Instruccion para el Paquete
                    Instruccion registroInstruccionAux;

                    registroInstruccionAux.operacion= 1;
                    strcpy(registroInstruccionAux.key, registroInstruccion.key);
                    registroInstruccionAux.key[strlen(registroInstruccion.key)] = '\0';
                    registroInstruccionAux.dato=malloc(strlen(valorObtenido)+1);
                    strcpy(registroInstruccionAux.dato,valorObtenido);
                    registroInstruccionAux.dato[strlen(valorObtenido)] = '\0';
                    registroInstruccionAux.nombreEsiOrigen=malloc(strlen("Nada")+1);
                    strcpy(registroInstruccionAux.nombreEsiOrigen,"Nada");
                    registroInstruccionAux.nombreEsiOrigen[strlen("Nada")] = '\0';


                    // Armo el Paquete de Pedido del Valor
                    paquete = srlz_instruccion('I', DEVOLVER_STATUS_VALOR,registroInstruccionAux);

                    // Envio el Paquete al Coordinador
                    if(send(coordinador_fd,paquete.buffer,paquete.tam_buffer,0) != -1){

                        log_info(infoLogger,"Se le devuelve al COORDINADOR el valor del Recurso %s. El valor es %s", registroInstruccionAux.key, registroInstruccionAux.dato);

                    }else{
                        log_error(infoLogger, "No se pudo enviar al COORDINADOR el valor del Recurso %s. El valor es %s", registroInstruccionAux.key, registroInstruccionAux.dato);
                    }

                    free(paquete.buffer);
                    free(valorObtenido);
                    //free(registroInstruccion.nombreEsiOrigen);
                    //free(registroInstruccionAux.dato);
                    //free(registroInstruccionAux.nombreEsiOrigen);

                    //free(registroEntrada->clave);
                    //free(registroEntrada);
                    break;


                case OBTENCION_CONFIG_ENTRADAS:
                    // Recibo los datos de las Entradas
                    paquete = recibir_payload(&coordinador_fd,&encabezado.tam_payload);
                    registroEntradasIntancias = dsrlz_datosEntradas(paquete.buffer);
                    free(paquete.buffer);

                    log_info(infoLogger,"Recepcipión del Coordinador de la Cantidad (%d) y Tamaño de las Entradas (%d).", registroEntradasIntancias.cantEntrada , registroEntradasIntancias.tamanioEntrada);

                    // Guardo los datos recibidos
                    entradas=registroEntradasIntancias.cantEntrada;
                    espacioPorEntrada=registroEntradasIntancias.tamanioEntrada;

                    //creo estructura de datos con info de almacenamiento
                    almacenamiento.cantidadEntradas=entradas;
                    almacenamiento.tamPorEntrada=espacioPorEntrada;
                    almacenamiento.binario=malloc(strlen("storage.bin")+1);
                    almacenamiento.vector=malloc(strlen("vectorBin.txt")+1);
                    strcpy(&almacenamiento.binario[0],"storage.bin");
                    strcpy(&almacenamiento.vector[0],"vectorBin.txt");                  
                    almacenamiento.tablaEntradas=tablaEntradas;




                    // Creo el Storage.bin si no existe
                    //if (!existeArchivo("storage.bin")){
                        FILE* binario= fopen("storage.bin","wb+");
                        ftruncate(fileno(binario),entradas*espacioPorEntrada);
                        fseek(binario,0,SEEK_SET);
                        for(contador=0;contador< (entradas*espacioPorEntrada); contador=contador+1){
                            fwrite(&cero,sizeof(char),1,binario);
                        }
                        // Cierro los FD
                        fclose(binario);
                    //}
	
                    //if (!existeArchivo("vectorBin.txt")){
                        //mostrarBinario(almacenamiento);}

                    // Creo el Bitmap o lo reinicializo a 0
                        FILE* vectorBin = fopen("vectorBin.txt","w");
                        ftruncate(fileno(vectorBin),entradas);
                        for(contador=0;contador<entradas; contador=contador+1){
                            fseek(vectorBin,sizeof(char)*contador,SEEK_SET);
                            fwrite(&cero,sizeof(char),1,vectorBin);
                        }

                        // Cierro los FD
                        fclose(vectorBin);
	            	//}
                    
                    


                    // Se precarga la Tabla de Entradas con datos del Dump
                    preCargarTablaEntradas(puntoMontaje, almacenamiento);
                    break;

                case COMPACTACION_LOCAL:
                    // Realizar la Compactacion del Archivo Binario a pedido del Coordinador

                    realizarCompactacionLocal(almacenamiento);
                    log_info(infoLogger,"Se realizo la Compactacion Local en la Instancia %s a pedido del COORDINADOR", config_get_string_value(cfg,"INSTANCIA_NOMBRE"));

                    //se le notifica al coordinador que termino de compactar
                    paquete = crearHeader('I', FINALIZACION_COMPACTACION, 1);
                    if(send(coordinador_fd,paquete.buffer,paquete.tam_buffer,0) != -1){
                    	log_info(infoLogger, "Se le informó al COORDINADOR que se terminó de compactar");
                    }
                    else{
                    	log_error(infoLogger, "No se pudo informar al COORDINADOR que se terminó de compactar");
                    }
                    break;                                
			}
		}
    }

    /* ---------------------------------------- */
    /*  Libero Memoria de Log y Config          */
    /* ---------------------------------------- */
    log_destroy(infoLogger);
    config_destroy(cfg);
    free(algoritmoReemplazo);
    free(puntoMontaje);
    close(coordinador_fd);

    return 0;
}
