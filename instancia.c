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

#include <sys/types.h>
#include <dirent.h>

#include "funciones.h"
#include "registros.h"
#include "sockets.h"
//test3

/* ---------------------------------------- */
/*  Variables Globales                      */
/* ---------------------------------------- */
    int* puntero;
    int entradas;
    int espacioPorEntrada;
    t_list* tablaEntradas;
    Almacenamiento almacenamiento;

/* ---------------------------------------- */

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


	printf("Iniciando INSTANCIA\n");



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


    // Serializado el Proceso
    paquete = srlz_datosProceso('I', HANDSHAKE, config_get_string_value(cfg,"INSTANCIA_NOMBRE"), INSTANCIA, 0);

    // Envio al Coordinador el Handshake y Serializado el Proceso
    send(coordinador_fd,paquete.buffer,paquete.tam_buffer,0);
    free(paquete.buffer);

    FD_SET(coordinador_fd, &master);
    fd_maximo = coordinador_fd;   


// -----------------------------------------------------------------------
//    Prueba de funciones 2
// -----------------------------------------------------------------------

    Instruccion* datosInstruccion;


    // Defino el Algoritmo de Almacenamiento a utlizar
    char* algoritmoAlmacenamiento = string_new();
    string_append(&algoritmoAlmacenamiento,"CIRCULAR");


    if(persistirDatos(almacenamiento,datosInstruccion, algoritmoAlmacenamiento,puntero)){
        // Proceso a realizar si se persistiron correctamente los datos
    }else{
        // Proceso a realizar si fallo la persistencia
    }

    free(algoritmoAlmacenamiento);


// -----------------------------------------------------------------------

    EntradasIntancias registroEntradasIntancias;
    Instruccion registroInstruccion;

    while(1){
    	temporales=master;

    	if(select(fd_maximo+1,&temporales,NULL,NULL,NULL)==-1){
    		perror("select");
    		exit (1);
    	}
    	for(i = 0; i <= fd_maximo; i++) {
            if (FD_ISSET(i, &temporales)) { // ¡¡tenemos datos!!
                if (i == escucha_master) {
                    // gestionar nuevas conexiones
                    size = sizeof(master_addr);
                    if ((nuevo_fd = accept(escucha_master, (struct sockaddr *)&master_addr,
                                                             &size)) == -1) {
                        perror("accept");
                    } else {
                        FD_SET(nuevo_fd, &master); // añadir al conjunto maestro
                        if (nuevo_fd > fd_maximo) {    // actualizar el máximo
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

						switch(encabezado.cod_operacion){

							case EJECUTAR_INSTRUCCION:

                                paquete=recibir_payload(&i,&encabezado.tam_payload);
                                registroInstruccion=dsrlz_instruccion(paquete.buffer);
                                free(paquete.buffer);

                                log_info(infoLogger,"Pedido de Ejecución de una Instruccion recibida del Coordinador: %d %s %s", registroInstruccion.operacion, registroInstruccion.key, registroInstruccion.dato);

                                if(registroInstruccion.operacion == SET){

                                    // TODO
                                    // Persistir el Valor en el Archivo Binario
									//Prototipado
                                	//int posicion = buscarPosicionEnBin(Almacenamiento almacenamiento, registroInstruccion.dato);
                                	//escribirBinarioEnPosicion(Almacenamiento almacenamiento, int posicion, registroInstruccion.dato);




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

                                // Recibo los datos de las Entradas
                                paquete = recibir_payload(&i,&encabezado.tam_payload);
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
                                    FILE* vectorBin = fopen("vectorBin.txt","w");

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
                                    strcpy(almacenamiento.binario,"vectorBin.txt");
                                    strcpy(almacenamiento.vector,"storage.bin");
                                    almacenamiento.tablaEntradas=tablaEntradas;
                                // Se precarga la Tabla de Entradas con datos del Dump
                                preCargarTablaEntradas(config_get_string_value(cfg,"PUNTO_MONTAJE"), almacenamiento);
                                break;

                            case COMPACTACION_LOCAL:
                                // Realizar la Compactacion del Archivo Binario a pedido del Coordinador
                                realizarCompactacionLocal(almacenamiento);                            

                                log_info(infoLogger,"Se realizo la Compactacion Local en la Instancia %s a pedido del COORDINADOR", config_get_string_value(cfg,"INSTANCIA_NOMBRE"));
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

    return 0;
}


