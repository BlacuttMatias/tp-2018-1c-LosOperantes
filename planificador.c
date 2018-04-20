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

#include "funciones.h"
#include "registros.h"
#include "sockets.h"

/* ---------------------------------------- */
/*  Variables Globales 						*/
/* ---------------------------------------- */

t_log* infoLogger;
t_config* cfg;

/* ---------------------------------------- */
/*  Consola interactiva                     */
/* ---------------------------------------- */

int quitConsola = false;

void consola_interactiva(char* comandoConsola) {

    char** parametrosConsola;
    char** parametrosConsolaOriginal;
    char* comandoConsolaOriginal;
    bool comandoAceptado = false;

    if(NULL == comandoConsola){
        quitConsola = true;
        return;
    }
    if(strlen(comandoConsola) > 0) add_history(comandoConsola);

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
            printf("\tformat \t- Formatear el Filesystem. \n\n");
            printf("\n\trm [path_archivo] ó rm -d [path_directorio] ó rm -b [path_archivo] [nro_bloque] [nro_copia] \t- Eliminar un Archivo/Directorio/Bloque. Si un directorio a eliminar no se encuentra vacío, la operación debe fallar. Además, si el bloque a eliminar fuera la última copia del mismo, se deberá abortar la operación informando lo sucedido. \n\n");
            printf("\n\trename [path_original] [nombre_final] \t- Renombra un Archivo o Directorio \n\n");
            printf("\n\tmv [path_original] [path_final] \t- Mueve un Archivo o Directorio \n\n");
            printf("\n\tcat [path_archivo] \t- Muestra el contenido del archivo como texto plano. \n\n");
            printf("\n\tmkdir [path_dir] \t- Crea un directorio. \n\n");
            printf("\n\tcpfrom [path_archivo_origen] [directorio_yamafs] [tipo_archivo]\t- Copia un archivo local al YamaFS. El tipo_archivo puede ser binario (b) o texto (t)\n\n");
            printf("\n\tcpto [path_archivo_yamafs] [directorio_filesystem] \t- Copia un archivo local al YamaFS. \n\n");
            printf("\n\tcpblock [path_archivo] [nro_bloque] [id_nodo] \t- Crea una copia de un bloque de un archivo en el nodo dado. \n\n");
            printf("\n\tmd5 [path_archivo_yamafs] \t- Solicitar el MD5 de un archivo en YamaFS. \n\n");
            printf("\n\tls [path_directorio] \t- Lista los archivos de un directorio. Si no se indica un directorio, se muestra la Tabla de Directorios y la Tabla de Nodos.\n\n");
            printf("\n\tinfo [path_archivo_yamafs] \t- Muestra toda la información del archivo, incluyendo tamaño, bloques, ubicación de los bloques, etc. \n\n");
        }


        if(string_starts_with(comandoConsola,"CPBLOCK")){
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

/* ---------------------------------------- */

int main(int argc, char* argv[]){

	/* Creo la instancia del Archivo de Configuracion y del Log */
	cfg = config_create("config/config.cfg");
	infoLogger = log_create("log/planificador.log", "PLANIFICADOR", false, LOG_LEVEL_INFO);

	log_info(infoLogger, "Iniciando PLANIFICADOR" );
	printf("Iniciando PLANIFICADOR\n");


	Encabezado encabezado;
	Paquete paquete;
    struct sockaddr_in servidor_addr,my_addr,master_addr; // información de la dirección de destino
    int servidor,escucha_master,fd_maximo,nuevo_fd,i,size, nbytes;

    fd_set master,temporales;
    FD_ZERO(&master);
    FD_ZERO(&temporales);


    // Mensaje de Bienvenida de la Consola
    printf("Bienvenido a la Consola Interactiva del Planificador 1.0\n");
    printf("Para obtener ayuda de los comandos permitidos, escriba 'help'.\n");

    log_info(infoLogger, "Inicio de la Consola Interactiva" );
    rl_callback_handler_install(promptConsola, (rl_vcpfunc_t*) &consola_interactiva);

    // Variables para la Consola Interactiva
    const char* promptConsola = "#> "; // Prompt


    // Creo el Servidor para escuchar conexiones
    servidor=crearServidor(config_get_int_value(cfg,"PLANIFICADOR_PUERTO"));
    log_info(infoLogger, "Escuchando conexiones" );

    FD_SET(servidor, &master);
    fd_maximo = servidor;   


    while(1){

        // Si se salio de la consola
        if(quitConsola){
            log_info(infoLogger, "Cierre de la Consola Interactiva" );
            break;
        }


    	temporales=master;

    	if(select(fd_maximo+1,&temporales,NULL,NULL,NULL)==-1){
    		perror("select");
    		exit (1);
    	}

        // Para la Consola Interactiva
        rl_callback_read_char();

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
								log_info(infoLogger,"Pedido de Ejecución de Instruccion.");
								break;

						}
					}
                }
            }
        }
    }


    // Para Cerrar la Consola Interactiva
    rl_callback_handler_remove();

    close(servidor);
    FD_CLR(servidor, &master);    

    /* ---------------------------------------- */
    /*  Libero Memoria de Log y Config          */
    /* ---------------------------------------- */
    log_destroy(infoLogger);
    config_destroy(cfg);

    return 0;
}
