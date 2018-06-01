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
/* ---------------------------------------- */


// Gestor del Intervalo del Dump
void handle_alarm(int sig) {

    // Realizo el Dump de la Tabla de Entradas
    dump(tablaEntradas);

     printf("Dump automatizado cada %d segundos...\n", config_get_int_value(cfg,"INTERVALO_DUMP") );
     alarm(config_get_int_value(cfg,"INTERVALO_DUMP"));
}


int main(int argc, char* argv[]){

    /* Creo la instancia del Archivo de Configuracion y del Log */
    cfg = config_create("config/config.cfg");
    infoLogger = log_create("log/instancia.log", "INSTANCIA", false, LOG_LEVEL_INFO);

    log_info(infoLogger, "Iniciando INSTANCIA" );
    

    // Me aseguro que la estructura de directorios del Punto de Montaje este creada previamente
    if(crearEstructuraDirectorios( config_get_string_value(cfg,"PUNTO_MONTAJE") ) ){
        log_info(infoLogger, "Se creo la estructura del Punto de Montaje" );
    }

    char cero='0';
    int uno='1';
    int contador=0;

	//creo lista tabla de entradas
	tablaEntradas = list_create(); 
	

    // Defino el Intervalo del DUMP y lo planifico
    struct sigaction sa;
    sa.sa_handler = &handle_alarm;
    sa.sa_flags = SA_RESTART;
    sigfillset(&sa.sa_mask);
    sigaction(SIGALRM, &sa, NULL);
    alarm(config_get_int_value(cfg,"INTERVALO_DUMP"));
    puts("alarma");
//**PARA TESTEAR EL RECUPERO DE INFORMACION SI LA INSTANCIA MUERE HAY QUE COMENTAR ESTE PEDAZO DE CODIGO****** //
 	

//PRUEBA ARCHIVO BINARIO					/////
	// creo el archivo binario
  

/*
Almacenamiento almacenamiento;
        almacenamiento.cantidadEntradas=10;
        almacenamiento.tamPorEntrada=4;
         almacenamiento.binario=malloc(13);
        almacenamiento.vector=malloc(15);
        strcpy(almacenamiento.binario,"storage.bin");
        strcpy(almacenamiento.vector,"vectorBin.txt");
        almacenamiento.tablaEntradas=tablaEntradas;

    //hardcodeo la posicion 0 y 4  del binario 
    puts("abro binario");
            printf("\n\n el vector bin se escribió asi %s  \n\n",almacenamiento.vector);
       FILE* binario= fopen(almacenamiento.binario,"wb+");
       puts("abri bin");
        ftruncate(fileno(binario),almacenamiento.cantidadEntradas*almacenamiento.tamPorEntrada);
        int longitud= string_length("NARDIELLO");
        puts("trunque");
        
        char* cadena1 = string_new();
        strcpy(cadena1,"NARDIELLO");
        puts(cadena1);
        fseek(binario,0,SEEK_SET);
        puts("write bin");
        fwrite(cadena1,strlen(cadena1),1,binario);
        strcpy(cadena1,"MESSI");
        escribirBinarioEnPosicion(almacenamiento,4,cadena1);
        fclose(binario);
        puts("cierro");
        char* lecturaBinario = string_new();
        strcpy(lecturaBinario,leerBinarioEnPosicion(almacenamiento,0));
        printf("en la primer posicion del binario esta el dato %s\n",lecturaBinario);




    preCargarTablaEntradas(config_get_string_value(cfg,"PUNTO_MONTAJE"), almacenamiento);
    t_entrada* entrada1= list_get(tablaEntradas,0);
    printf("\nla primer clave de la lista es     %s", entrada1->clave);

   if(list_size(tablaEntradas)>1){
        
        char *buffer=string_new();

        buffer=leerBinarioEnPosicion(almacenamiento,1);
        printf("\n el segundo valor en el binario es:   %s  \n",buffer);
        grabarPosicionEnVector(almacenamiento,8);
	
        printf("\n en el vector Bin deberían estas las posiciones 0 y 4 escritas con 1\n");

        liberarEntradaEnVector(almacenamiento,entrada1);
	}
    //*/

/*
	//TEST PARA INSTRUCCION SET TODAVIA YA FUNCIONA
	t_entrada* nuevaEntrada = NULL;
	nuevaEntrada=malloc(sizeof(t_entrada));
 	nuevaEntrada->clave = malloc(strlen("a")+1);
	strcpy(nuevaEntrada->clave, "a");
	nuevaEntrada->clave[strlen("a")] = '\0';
	list_add(tablaEntradas,nuevaEntrada);

	if(existeEntradaEnTabla(tablaEntradas,"a")){
		printf(" EXISTE ");
	}else{printf(" NO EXISTE ");}

	bool esIgualA(t_entrada* unaEntrada){
		if(strcmp(nuevaEntrada->clave,"a") == 0){;
			printf(" Existe ");
			return true;
		}else{
			printf(" No Existe ");
			return false;
		}
	}

*/

	//TERMINA TEST PARA INSTRUCCION SET

	//TEST PARA EJECUTAR DUMP CADA CIERTO TIEMPO
	//ejecutarCadaXTiempo(mostrarHola,10);

	//TERMINA TEST EXITOSO PROBADO CON MOSTRAR HOLA, TOMA EL VALOR "10" DE LA CONFIG (ESTIMAMOS EN SEGUNDOS SINO SE MODIFICA SIN PROBLEMA) Y LO APLICA.
	//config_get_int_value(cfg,"INTERVALO_DUMP") TOMA EL VALOR DE LA CONFIG,


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
                    			if(strcmp(unaEntrada->clave,registroInstruccion.key) == 0){;
                    				printf(" Existe ");
                    				return true;
                    			}else{
                    				printf(" No Existe ");
                    				return false;
                    			}
                    		}

                    		//la busco dentro de la tabla asi saco su numero de entrada por ende su posicion en el binario
                    		t_entrada* entradaEncontrada = list_find(tablaEntradas,(void*)esIgualA);

                        	int posicion = entradaEncontrada->numeroDeEntrada; //posicion del binario
                            liberarEntradaEnVector(almacenamiento,entradaEncontrada);

                        	//escribo en el binario
                        	persistirDatos(almacenamiento,&registroInstruccion,algoritmoReemplazo,puntero);





                    	}else{

                    		//si NO existe la entrada en la tabla
                    		//hay que ver si hay entradas libres
                    		//si hay se persiste en la siguiente entrada libre
                    		//si no hay hay que correr algoritmo de reemplazo
                    	}





                        // Cargo la Tabla de Entradas
                        cargarTablaEntradas(tablaEntradas,&registroInstruccion);

                        // TODO
                        // Modificar el valor de la Key

                        // Realizo el Dump de la Tabla de Entradas
                        dump(tablaEntradas);

                    }

                    if(registroInstruccion.operacion == STORE){                                

                        // TODO
                        // Implementar

                        // Realizo el Dump de la Tabla de Entradas
                        dump(tablaEntradas);
                    }



                    // Realizo la Compactacion del Archivo Binario
                    realizarCompactacionLocal(almacenamiento);

                    log_info(infoLogger,"Se realizo la Compactacion Local en la Instancia %s", config_get_string_value(cfg,"INSTANCIA_NOMBRE"));

                    // Le aviso al Coordinador para que las demas Instancias realicen sus Compactaciones
                    Paquete paquete= crearHeader('I',COMPACTACION_GLOBAL,1);
                    if( send(coordinador_fd,paquete.buffer,paquete.tam_buffer,1) != -1 ){
                        log_info(infoLogger,"Se le envio al COORDINADOR el aviso para que las demas Instancias realicen sus Compactaciones Locales");
                    }else{
                        log_info(infoLogger,"No se pudo enviar al COORDINADOR el aviso para que las demas Instancias realicen sus Compactaciones Locales");
                    }



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
                    puts("obtener config");

                    // Recibo los datos de las Entradas
                    paquete = recibir_payload(&coordinador_fd,&encabezado.tam_payload);
                    registroEntradasIntancias = dsrlz_datosEntradas(paquete.buffer);
                    free(paquete.buffer);

                    log_info(infoLogger,"Recepcipión del Coordinador de la Cantidad (%d) y Tamaño de las Entradas (%d).", registroEntradasIntancias.cantEntrada , registroEntradasIntancias.tamanioEntrada);

                    // Guardo los datos recibidos
                    entradas=registroEntradasIntancias.cantEntrada;
                    espacioPorEntrada=registroEntradasIntancias.tamanioEntrada;
                    puts("aj");
                    // Creo el Storage.bin si no existe
                    if (!existeArchivo("storage.bin")){
                        FILE* binario= fopen("storage.bin","wb+");
                        ftruncate(fileno(binario),entradas*espacioPorEntrada);

                        // Cierro los FD
                        fclose(binario);
                    }
                    puts("if2");
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
                        puts("vector");
                        fclose(vectorBin);
                        puts("era el vect");

                        
                    }
                    puts("almacenamiento abro");
                    //creo estructura de datos con info de almacenamiento
                    almacenamiento.cantidadEntradas=entradas;
                    almacenamiento.tamPorEntrada=espacioPorEntrada;
                    almacenamiento.binario=string_new();
                    almacenamiento.vector=string_new();
                    almacenamiento.binario=malloc(strlen("vectorBin.txt")+1);
                    strcpy(almacenamiento.binario,"vectorBin.txt");
                    almacenamiento.binario[strlen("vectorBin.txt")] = '\0';
                    almacenamiento.vector=malloc(strlen("storage.bin")+1);
                    strcpy(almacenamiento.vector,"storage.bin");
                    almacenamiento.vector[strlen("storage.bin")] = '\0';                    
                    almacenamiento.tablaEntradas=tablaEntradas;

                    // Se precarga la Tabla de Entradas con datos del Dump
                    puts("precargo");
                    preCargarTablaEntradas(config_get_string_value(cfg,"PUNTO_MONTAJE"), almacenamiento);
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
    close(coordinador_fd);

    return 0;
}