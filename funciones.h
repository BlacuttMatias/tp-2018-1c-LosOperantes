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

	void sacarSiguienteInstruccion(t_list* listaInstrucciones, Instruccion* instruccion);
	bool procesarScript(char* pathScript, t_list* listaInstrucciones);

	char* obtenerProximaInstruccion(t_list* listaInstrucciones);
	int obtenerTamanoProximaInstruccion(t_list* listaInstrucciones);
	void cargarInstruccion(Instruccion* registro,int codigo,char* key, char* valor);
	void mostrarInstruccion(Instruccion* instruccion);
	bool existeArchivo(char *filename);

	Paquete srlz_datosInstruccion(Instruccion instruccion);
	Instruccion dsrlz_datosInstruccion(void* buffer);

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
	void cargarTablaEntradas(t_list *tablaEntradas,Instruccion* estructuraInstruccion);

/* ---------------------------------------- */
#endif
