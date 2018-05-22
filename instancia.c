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


    // -----------------------------------------------------------------------
    //    TODO ESTE CODIGO SE PUEDE BORRAR PORQUE YA ESTA IMPLEMENTADO EN EL MENSAJE OBTENCION_CONFIG_ENTRADAS
    //    SE DEJA PARA QUE SE PUEDE SEGUIR TESTEANDO HARCODEADO
    // -----------------------------------------------------------------------


                // Defino la Cantidad de Entradas y el Tamaño de las Entradas
                int entradas=5;
                int espacioPorEntrada=15;

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

    // -----------------------------------------------------------------------


	// -----------------------------------------------------------------------
	//    Prueba de funciones 1
	// -----------------------------------------------------------------------


	////////////////////////////////////////////
	
    // Con un puntero a DIR abro el directorio 
    DIR *dir;
    // en *ent habrá información sobre el archivo que se está "sacando" a cada momento 
    struct dirent *ent;
    t_list* listaEntradas = list_create();

    /* Empezaremos a leer en el directorio entradas */
    dir = opendir (config_get_string_value(cfg,"PUNTO_MONTAJE"));

    // Miramos que no haya error 
    if (dir == NULL)
        error("No se puede abrir el directorio");

    

    // Una vez nos aseguramos de que no hay error... 
    // Leyendo uno a uno todos los archivos que hay 
    while ((ent = readdir (dir)) != NULL)
    {
        // Nos devolverá el directorio actual (.) y el anterior (..), como hace ls //
        if ( (strcmp(ent->d_name, ".")!=0) && (strcmp(ent->d_name, "..")!=0) )
        {
            char* nombreArchivoProcesar = string_new();
            string_append_with_format(&nombreArchivoProcesar, "%s", ent->d_name);

            // Una vez tenemos el archivo, lo pasamos a una función para procesarlo. //
            procesoArchivo(nombreArchivoProcesar, listaEntradas, config_get_string_value(cfg,"PUNTO_MONTAJE"));  

            free(nombreArchivoProcesar);
        }
    }
    closedir (dir);

	//////////////////////////////////////



	//PRUEBA TABLA ENTRADAS IMPLEMENTACION CON LISTAS + nueva funcion cargarTablaEntradas
	t_list* tablaEntradas = list_create(); //creo lista tabla de entradas
	
	
//**PARA TESTEAR EL RECUPERO DE INFORMACION SI LA INSTANCIA MUERE HAY QUE COMENTAR ESTE PEDAZO DE CODIGO****** //
	
	//hardcodeo una instruccion
	Instruccion* nuevaInstruccion = NULL;
	Instruccion* otraInstruccion = NULL;
	nuevaInstruccion = malloc(sizeof(Instruccion));
	otraInstruccion = malloc(sizeof(Instruccion));

	//carga de datos
	nuevaInstruccion->dato = malloc(strlen("MESSI")+1);
	strcpy(nuevaInstruccion->dato, "MESSI");
	nuevaInstruccion->dato[strlen("MESSI")] = '\0';

	otraInstruccion->dato = malloc(strlen("NARDIELLO")+1);
	strcpy(otraInstruccion->dato, "NARDIELLO");
	otraInstruccion->dato[strlen("NARDIELLO")] = '\0';


	//carga de claves
	strcpy(nuevaInstruccion->key,"FUTBOL");
	strcpy(otraInstruccion->key,"OPERATIVOS");
	//carga de operaciones
	nuevaInstruccion->operacion = 1;
	otraInstruccion->operacion = 2;
	

	//cargo tabla con entrada hardcodeada
	cargarTablaEntradas(tablaEntradas,nuevaInstruccion);
	cargarTablaEntradas(tablaEntradas,otraInstruccion);

	t_entrada* primerElemento;
	t_entrada* segundoElemento;
	primerElemento = list_get(tablaEntradas,0);
	segundoElemento = list_get(tablaEntradas,1);
	
	//muestro entrada hardcodeada
  	printf("Clave:%s - Numero:%d - Tamanio:%d \n",primerElemento->clave,primerElemento->numeroDeEntrada,primerElemento->tamanioValorAlmacenado); //prueba imprimir por pantalla el elemento obtenido

    // Persisto dos Entradas
    //persistirEntrada(primerElemento);
    //persistirEntrada(segundoElemento);

  	//testeo de dump
  	dump(tablaEntradas);



//**PARA TESTEAR EL RECUPERO DE INFORMACION SI LA INSTANCIA MUERE HAY QUE COMENTAR ESTE PEDAZO DE CODIGO****** //
  	

//PRUEBA ARCHIVO BINARIO					/////
	// creo el archivo binario
   if(list_size(listaEntradas)>1){
        FILE* binario= fopen("storage.bin","r+b");
        FILE* vectorBin = fopen("vectorBin.txt","r+");

        //----------ESCRIBO Y LEO LA PRIMERA POSICION EN EL BINARIO----/
        t_entrada* entrada= list_get(listaEntradas,0);
        fseek(vectorBin,0,SEEK_SET);	
        fwrite(&uno,1,sizeof(char),vectorBin);
        char *buffer;

        // TODO 

        // El 4to parametro es el Valor de la Key que ahora hay que consultarlo del Archivo Binario
        // Se pasa NULL para que no falle
        escribirBinarioEnPosicion(binario,0,espacioPorEntrada, "NULO");       
        buffer= leerBinarioEnPosicion(binario,0,espacioPorEntrada);

        printf("\n el primer valor en el binario es:   %s  \n",buffer);	

        //escribo en 2da posicion del binario y luego la leo
        entrada= list_get(listaEntradas,1);
        escribirBinarioEnPosicion(binario,1,espacioPorEntrada, "NULO");
        buffer=leerBinarioEnPosicion(binario,1,espacioPorEntrada);
        printf("\n el segundo valor en el binario es:   %s  \n",buffer);

        //cargo el NumeroEntrada de las Estructuras de la lista, segun su posicion en el bin.
/*        
        int numeroDeEntrada = buscarPosicionEnBin(binario, espacioPorEntrada, entrada->valor);	
        entrada->numeroDeEntrada=numeroDeEntrada;
        printf("\n es posicion %d     y deberia ser posicion  1 tomando en cuenta la posicion 0\n",numeroDeEntrada);
*/

		int Bool=buscarPosicionesEnBin(binario,espacioPorEntrada,listaEntradas, "NULO");

		if(Bool){
		  puts("se encontraron todas las entradas en el bin\n");	}
		else{
		  puts("no se encontro alguna entrada");	
		}


        // Cierro los FD
        fclose(binario);      
        fclose(vectorBin);
	}


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


    if(persistirDatos(datosInstruccion, algoritmoAlmacenamiento)){
        // Proceso a realizar si se persistiron correctamente los datos
    }else{
        // Proceso a realizar si fallo la persistencia
    }

    free(algoritmoAlmacenamiento);


// -----------------------------------------------------------------------

    EntradasIntancias registroEntradasIntancias;

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
                            
                                // TODO

								log_info(infoLogger,"Pedido de Ejecución de Instruccion recibido del Coordinador.");
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

