#ifndef __FUNCIONES_H__
#define __FUNCIONES_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <math.h>
#include <string.h>
#include <dirent.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <parsi/parser.h>
#include "registros.h"
#include "sockets.h"

/* ---------------------------------------- */
/*  Variables Globales 						*/
/* ---------------------------------------- */

	t_log* infoLogger;
	t_config* cfg;

/* ---------------------------------------- */
// Funciones de Serializacion/Deserializacion
/* ---------------------------------------- */

	Paquete crearHeader(char proceso, int cod_operacion, int tamPayload);

	Paquete srlz_datosProceso(char proceso, int codigoOperacion, char* nombreProceso, int rafagaAnterior, int rafagaActual);
	Proceso dsrlz_datosProceso(void* buffer);

/* ---------------------------------------- */
/*  Funciones de ESI 						*/
/* ---------------------------------------- */

	bool procesarScript(char* pathScript, t_list* listaInstrucciones);

	char* obtenerProximaInstruccion(t_list* listaInstrucciones);
	int obtenerTamanoProximaInstruccion(t_list* listaInstrucciones);

/* ---------------------------------------- */
/*  Funciones de Planificador 				*/
/* ---------------------------------------- */

	void showContenidolistaReady(t_list* listaReady);
	void showContenidocolaReady(t_queue* colaReady);
	t_queue* planificarReady(t_list* listaReady, char* algoritmoPlanificacion);

/* ---------------------------------------- */
/*  Funciones de Coordinador 				*/
/* ---------------------------------------- */

	void inicializarEstructurasAdministrativas();
	bool registrarLogOperaciones(Instruccion* datosInstruccion, char* nombreProceso);
	char* procesarSolicitudEjecucion(Instruccion* datosInstruccion, char* algoritmoDistribucion);


/* ---------------------------------------- */
/*  Funciones de Instancia 					*/
/* ---------------------------------------- */

	bool persistirDatos(Instruccion* datosInstruccion, char* algoritmoAlmacenamiento);

/* ---------------------------------------- */
#endif
