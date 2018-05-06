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

	Instruccion dsrlz_instruccion (void* buffer);
	Paquete srlz_instruccion (char proceso, int codigoOperacion, Instruccion instruccion);

/* ---------------------------------------- */
/*  Funciones de ESI 						*/
/* ---------------------------------------- */

	Instruccion* sacarSiguienteInstruccion(t_list* listaInstrucciones);
	Instruccion pasarAEstructura(Instruccion* puntero);
	bool procesarScript(char* pathScript, t_list* listaInstrucciones);

	char* obtenerProximaInstruccion(t_list* listaInstrucciones);
	int obtenerTamanoProximaInstruccion(t_list* listaInstrucciones);
	void cargarInstruccion(Instruccion* registro,int codigo,char* key, char* valor);
	void mostrarInstruccion(Instruccion* instruccion);
	bool existeArchivo(char *filename);


/* ---------------------------------------- */
/*  Funciones de Planificador 				*/
/* ---------------------------------------- */

	void showContenidolistaReady(t_list* listaReady);
	void showContenidocolaReady(t_queue* colaReady);
	Proceso* obtenerProximoProcesoPlanificado(t_list* listaESIconectados, t_list* listaReady, char* algoritmoPlanificacion);
	t_queue* planificarReady(t_list* listaReady, char* algoritmoPlanificacion);
	char* obtenerNombreProceso(t_list* listaProcesosConectados, int socketProcesoConsultar);
	void eliminarProcesoLista(t_list* listaProcesosConectados, int socketProcesoEliminar);
	void eliminarProcesoCola(t_queue* colaReady, int socketProcesoEliminar);

/* ---------------------------------------- */
/*  Funciones de Coordinador 				*/
/* ---------------------------------------- */

	void registrarLogOperaciones(t_list* listaProcesosConectados, Instruccion* datosInstruccion, int socketProceso);
	char* procesarSolicitudEjecucion(Instruccion* datosInstruccion, char* algoritmoDistribucion);
	void cargarListaProcesosConectados(t_list *listaProcesosConectados, Proceso* nuevoProceso);
	void showContenidolistaProcesosConectados(t_list* listaProcesosConectados);


/* ---------------------------------------- */
/*  Funciones de Instancia 					*/
/* ---------------------------------------- */

	bool persistirDatos(Instruccion* datosInstruccion, char* algoritmoAlmacenamiento);
	void cargarTablaEntradas(t_list *tablaEntradas,Instruccion* estructuraInstruccion);

/* ---------------------------------------- */
#endif
