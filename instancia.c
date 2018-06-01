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
    int* puntero;
    int entradas;
    int espacioPorEntrada;
    t_list* tablaEntradas;
    Almacenamiento almacenamiento;
    char* puntoMontaje;
    int intervaloDump;

/* ---------------------------------------- */


// Gestor del Intervalo del Dump
void handle_alarm(int sig) {

    printf("Dump automatizado cada %d segundos...\n", intervaloDump);

    // Realizo el Dump de la Tabla de Entradas
    dump(tablaEntradas, puntoMontaje, almacenamiento);

    alarm(intervaloDump);
}


int main(int argc, char* argv[]){

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

                        // TODO
                        // Persistir el Valor en el Archivo Binario
						//Prototipado

                    	if(existeEntradaEnTabla(tablaEntradas,registroInstruccion.key)){
                    		//si ya existe la entrada en la tabla

                    		bool esIgualA(t_entrada* unaEntrada){
                    			if(strcmp(unaEntrada->clave,registroInstruccion.key) == 0){
                    				printf(" Existe ");
                    				return true;
                    			}else{
                    				printf(" No Existe ");
                    				return false;
                    			}
                    		}

                    		//la busco dentro de la tabla asi saco su numero de entrada por ende su posicion en el binario
                    		t_entrada* entradaEncontrada = list_find(tablaEntradas,(void*)esIgualA);

                        	//int posicion = entradaEncontrada->numeroDeEntrada; //posicion del binario
                            liberarEntradaEnVector(almacenamiento,entradaEncontrada);

                        	//escribo en el binario
                        	persistirDatos(almacenamiento,&registroInstruccion,algoritmoReemplazo,puntero);





                    	}else{

                    		//si NO existe la entrada en la tabla
                    		//hay que ver si hay entradas libres
                    		//si hay se persiste en la siguiente entrada libre
                    		//si no hay hay que correr algoritmo de reemplazo


                            //escribo en el binario
                            persistirDatos(almacenamiento,&registroInstruccion,algoritmoReemplazo,puntero);                            
                    	}


                        // Cargo la Tabla de Entradas
                        cargarTablaEntradas(tablaEntradas,&registroInstruccion, almacenamiento);                      
                    }

                    if(registroInstruccion.operacion == STORE){                                

                        // Realizo el Dump de la Tabla de Entradas
                        //dump(tablaEntradas, puntoMontaje, almacenamiento);
                    }


                    // Muestro el contenido de la Tabla de Entradas
                    showContenidoTablaEntradas(tablaEntradas);


/*
PARA UTILIZAR PARA INICIAR LA COMPACTACION GLOBAL

                    log_info(infoLogger,"Se realizo la Compactacion Local en la Instancia %s", config_get_string_value(cfg,"INSTANCIA_NOMBRE"));

                    // Le aviso al Coordinador para que las demas Instancias realicen sus Compactaciones
                    Paquete paquete= crearHeader('I',COMPACTACION_GLOBAL,1);
                    if( send(coordinador_fd,paquete.buffer,paquete.tam_buffer,1) != -1 ){
                        log_info(infoLogger,"Se le envio al COORDINADOR el aviso para que las demas Instancias realicen sus Compactaciones Locales");
                    }else{
                        log_info(infoLogger,"No se pudo enviar al COORDINADOR el aviso para que las demas Instancias realicen sus Compactaciones Locales");
                    }
*/

                    // TODO
                    // Determinar si fallo o no y corregir el mensaje de abajo. Ahora esta harcodeado a EJECUCION_EXITOSA

                    // Armo el Paquete del Resultado de la Ejecucion de la Instruccion
                    paquete = srlz_resultadoEjecucion('I', RESPUESTA_EJECUTAR_INSTRUCCION, registroInstruccion.nombreEsiOrigen, EJECUCION_EXITOSA, "", registroInstruccion.operacion, registroInstruccion.key);

                    // Envio el Paquete al Coordinador
                    if(send(coordinador_fd,paquete.buffer,paquete.tam_buffer,0) != -1){

                        free(paquete.buffer);
                        log_info(infoLogger, "Se le notificó al COORDINADOR el resultado de la ejecución de la Instrucción");

                    }else{
                        log_error(infoLogger, "No se pudo notificar al COORDINADOR el resultado de la ejecución de la Instrucción");
                    }
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

                    // Creo el Storage.bin si no existe
                    if (!existeArchivo("storage.bin")){
                        FILE* binario= fopen("storage.bin","wb+");
                        ftruncate(fileno(binario),entradas*espacioPorEntrada);

                        // Cierro los FD
                        fclose(binario);
                    }

                    // Creo el Bitmap si no existe
                    if (!existeArchivo("vectorBin.txt")){
                        puts("abro vectorBin");
                        FILE* vectorBin = fopen("vectorBin.txt","w");
                        puts("fort");
                        for(contador=0;contador<entradas; contador=contador+1){
                            fseek(vectorBin,sizeof(char)*contador,SEEK_SET);
                            fwrite(&cero,sizeof(char),1,vectorBin);
                        }

                        // Cierro los FD
                        fclose(vectorBin);
                    }

                    //creo estructura de datos con info de almacenamiento
                    almacenamiento.cantidadEntradas=entradas;
                    almacenamiento.tamPorEntrada=espacioPorEntrada;
                    almacenamiento.binario=string_new();
                    almacenamiento.vector=string_new();
                    strcpy(almacenamiento.binario,"storage.bin");
                    strcpy(almacenamiento.vector,"vectorBin.txt");                  
                    almacenamiento.tablaEntradas=tablaEntradas;

                    // Se precarga la Tabla de Entradas con datos del Dump
                    puts("precargo");
                    preCargarTablaEntradas(puntoMontaje, almacenamiento);
                    puts("precargué");
                    break;

                case COMPACTACION_LOCAL:
                    // Realizar la Compactacion del Archivo Binario a pedido del Coordinador
                    realizarCompactacionLocal(almacenamiento);                            

                    log_info(infoLogger,"Se realizo la Compactacion Local en la Instancia %s a pedido del COORDINADOR", config_get_string_value(cfg,"INSTANCIA_NOMBRE"));
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