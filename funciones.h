#ifndef __FUNCIONES_H__
#define __FUNCIONES_H__

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
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

	Paquete srlz_datosKeyBloqueada(char proceso, int codigoOperacion, char* nombreProceso, int operacion, char key[40], char* dato);
	KeyBloqueada dsrlz_datosKeyBloqueada(void* buffer);

	Paquete srlz_datosInstancia(char proceso, int codigoOperacion, char* nombreProceso, int entradasLibres, int socketProceso);
	Instancia dsrlz_datosInstancia(void* buffer);

/* ---------------------------------------- */
/*  Funciones de ESI 						*/
/* ---------------------------------------- */


	Instruccion* sacarSiguienteInstruccion(t_list* listaInstrucciones);
	Instruccion pasarAEstructura(Instruccion* puntero);
	bool procesarScript(char* pathScript, t_list* listaInstrucciones);

	Instruccion* obtenerSiguienteInstruccion(t_list* listaInstrucciones);
	void eliminarUltimaInstruccion(t_list* listaInstrucciones);
	int obtenerTamanoProximaInstruccion(t_list* listaInstrucciones);
	void cargarInstruccion(Instruccion* registro,int codigo,char* key, char* valor);
	void mostrarInstruccion(Instruccion* instruccion);
	bool existeArchivo(char *filename);


/* ---------------------------------------- */
/*  Funciones de Planificador 				*/
/* ---------------------------------------- */

	void showContenidolistaReady(t_list* listaReady);
	void showContenidocolaReady(t_queue* colaReady);
	void showContenidocolaBloqueados(t_queue* colaBloqueados);
	void showContenidoDiccionario(t_dictionary * dictionario, char* nombreDiccionario);
	void showContenidolistaClavesBloqueadasRequeridas(t_list* listaClavesBloqueadasRequeridas);

	Proceso* obtenerProximoProcesoPlanificado(t_list* listaReady, t_queue* colaReady, t_dictionary* diccionarioRafagas, char* algoritmoPlanificacion);
	char* obtenerNombreProceso(t_list* listaProcesosConectados, int socketProcesoConsultar);
	int obtenerSocketProceso(t_list* listaProcesosConectados, char* nombreProcesoBuscado);
	Proceso* obtenerRegistroProceso(t_list* listaProcesosConectados, int socketProcesoConsultar);
	void eliminarProcesoLista(t_list* listaProcesosConectados, int socketProcesoEliminar);
	void eliminarProcesoCola(t_queue* colaReady, int socketProcesoEliminar);
	void cargarProcesoCola(t_list* listaUtilizar, t_queue* colaUtilizar, int socketProcesoConsultar);

	int estimarRafaga(int estimacionAnterior, int rafagaAnterior, int alfa);
	int countParametrosConsola(char * string);
	void listarRecursosBloqueados(t_list* listaClavesBloqueadasRequeridas, char* key);
	void liberarRecursosProceso(t_dictionary * dictionario, char* nombreProceso);

/* ---------------------------------------- */
/*  Funciones de Coordinador 				*/
/* ---------------------------------------- */

	void registrarLogOperaciones(t_list* listaProcesosConectados, int operacion, char key[40], char* dato, int socketProcesoConsultar);
	Instancia* obtenerInstanciaAsignada(t_list* listaInstanciasConectadas, Instruccion* datosInstruccion, char* algoritmoDistribucion);
	void cargarListaProcesosConectados(t_list *listaProcesosConectados, Proceso* nuevoProceso);
	void showContenidolistaProcesosConectados(t_list* listaProcesosConectados);


/* ---------------------------------------- */
/*  Funciones de Instancia 					*/
/* ---------------------------------------- */

	bool persistirDatos(Instruccion* datosInstruccion, char* algoritmoAlmacenamiento);
	void cargarTablaEntradas(t_list *tablaEntradas,Instruccion* estructuraInstruccion);
	void persistirEntrada(t_entrada* unaEntrada);
	/* Función para devolver un error */
	void error(const char *s);
	void procesoArchivo(char *archivo);
	void dump(t_list* tablaEntradas);



/* ---------------------------------------- */
#endif
