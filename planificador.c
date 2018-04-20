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

            printf("\tPausar/Continuar \t- El Planificador no le dará nuevas órdenes de ejecución a ningún ESI mientras se encuentre pausado. \n\n");

            printf("\n\tbloquear <clave> <ID> \t- Se bloqueará el proceso ESI hasta ser desbloqueado (ver más adelante), especificado por dicho <ID> en la cola del recurso <clave>. Vale recordar que cada línea del script a ejecutar es atómica, y no podrá ser interrumpida; si no que se bloqueará en la próxima oportunidad posible. Solo se podrán bloquear de esta manera ESIs que estén en el estado de listo o ejecutando. \n\n");

            printf("\n\tdesbloquear <clave> \t- Se desbloqueara el proceso ESI con el ID especificado. Solo se bloqueará ESIs que fueron bloqueados con la consola. Si un ESI está bloqueado esperando un recurso, no podrá ser desbloqueado de esta forma. \n\n");

            printf("\n\tlistar <recurso> \t- Lista los procesos encolados esperando al recurso. \n\n");

            printf("\n\tkill <ID> \t- finaliza el proceso. Recordando la atomicidad mencionada en “bloquear”. \n\n");

            printf("\n\tstatus <clave> \t- Debido a que para la correcta coordinación de las sentencias de acuerdo a los algoritmos de distribución se requiere de cierta información sobre las instancias del sistema, el Coordinador proporcionará una consola que permita consultar esta información. \n\n");

            printf("\n\tdeadlock\t- Esta consola también permitirá analizar los deadlocks que existan en el sistema y a que ESI están asociados. Pudiendo resolverlos manualmente con la sentencia de kill previamente descrita.\n\n");
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



    // Variables para la Consola Interactiva
    const char* promptConsola = "#> "; // Prompt
    struct timeval to; // Timer

    // Mensaje de Bienvenida de la Consola
    printf("Bienvenido a la Consola Interactiva del Planificador 1.0\n");
    printf("Para obtener ayuda de los comandos permitidos, escriba 'help'.\n");

    log_info(infoLogger, "Inicio de la Consola Interactiva" );
    rl_callback_handler_install(promptConsola, (rl_vcpfunc_t*) &consola_interactiva);
    to.tv_sec = 0;
    to.tv_usec = 1;

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

        if (select(fd_maximo + 1, &temporales, NULL, NULL, &to) == -1) {
            perror("select");
            exit(1);
        }

        // Para la Consola Interactiva
        rl_callback_read_char();

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
