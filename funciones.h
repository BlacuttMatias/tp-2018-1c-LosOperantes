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

	Paquete srlz_datosStatusRecurso(char proceso, int codigoOperacion, char* nombreInstanciaActual, char* nombreInstanciaFutura, char* valorRecurso, char key[40]);
	StatusRecurso dsrlz_datosStatusRecurso(void* buffer);

	Paquete srlz_datosEntradas(char proceso, int codigoOperacion, int cantEntrada, int tamanioEntrada);
	EntradasIntancias dsrlz_datosEntradas(void* buffer);

	Paquete srlz_resultadoEjecucion(char proceso, int codigoOperacion, char* nombreEsiDestino, int resultado, char* contenido, int operacion, char key[40]);
	ResultadoEjecucion dsrlz_resultadoEjecucion(void* buffer);

/* ---------------------------------------- */
/*  Funciones de ESI 						*/
/* ---------------------------------------- */

	Instruccion* sacarSiguienteInstruccion(t_list* listaInstrucciones);
	Instruccion pasarAEstructura(Instruccion* puntero, char* nombreEsi);
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
	void showContenidoCola(t_queue* cola, char* nombreCola);
	void showContenidoDiccionario(t_dictionary * dictionario, char* nombreDiccionario);
	void showContenidolistaClavesBloqueadasRequeridas(t_list* listaClavesBloqueadasRequeridas);

	Proceso* obtenerProximoProcesoPlanificado(t_list* listaReady, t_queue* colaReady, t_dictionary* diccionarioRafagas, char* algoritmoPlanificacion, int alfa);
	char* obtenerNombreProceso(t_list* listaProcesosConectados, int socketProcesoConsultar);
	int obtenerSocketProceso(t_list* listaProcesosConectados, char* nombreProcesoBuscado);
	Proceso* obtenerRegistroProceso(t_list* listaProcesosConectados, int socketProcesoConsultar);
	void eliminarProcesoLista(t_list* listaProcesosConectados, int socketProcesoEliminar);
	void eliminarProcesoCola(t_queue* colaReady, int socketProcesoEliminar);
	void cargarProcesoCola(t_list* listaUtilizar, t_queue* colaUtilizar, int socketProcesoConsultar);
	void cargarProcesoLista(t_list* listaUtilizar, t_list* listaProcesar, int socketProcesoCargar);

	KeyBloqueada* sacarProcesoConClaveBloqueadaDeLaLista(t_list* listaClavesBloqueadasRequeridas, char* key);
	void liberarKeyBloqueada(KeyBloqueada* registroKeyBloqueadaAux);
	KeyBloqueada* crearNodoDeUnaKeyBloqueada(KeyBloqueada keyBloqueada);
	float estimarRafaga(float estimacionAnterior, int rafagaAnterior, int alfa);
	int countParametrosConsola(char * string);
	void listarRecursosBloqueados(t_list* listaClavesBloqueadasRequeridas, t_dictionary* diccionarioClavesBloqueadas, char* key);
	void liberarRecursosProceso(t_dictionary * dictionario, char* nombreProceso);
	int cargarClavesInicialmenteBloqueadas(t_dictionary* diccionarioClavesBloqueadas, char** arregloClavesInicialmenteBloqueadas);

/* ---------------------------------------- */
/*  Funciones de Coordinador 				*/
/* ---------------------------------------- */

	void registrarLogOperaciones(t_list* listaProcesosConectados, int operacion, char key[40], char* dato, int socketProcesoConsultar);
	Instancia* obtenerInstanciaNueva(t_list* listaInstanciasConectadas, Instruccion* datosInstruccion, char* algoritmoDistribucion);
	Instancia* obtenerInstanciaAsignada(t_dictionary * dictionario, char* key);
	void cargarListaProcesosConectados(t_list *listaProcesosConectados, Proceso* nuevoProceso);
	void cargarListaInstanciasConectadas(t_list *listaInstanciasConectadas, Proceso* nuevoProceso);
	void showContenidolistaProcesosConectados(t_list* listaProcesosConectados);
	void showContenidolistaInstanciasConectadas(t_list* listaInstanciasConectadas);
	bool enviarInstruccionInstancia(Instruccion registroInstruccion, int socketInstancia);
	void actualizarDiccionarioClavesInstancias(t_dictionary* diccionarioClavesInstancias, char key[40], Instancia* proximaInstancia);

/* ---------------------------------------- */
/*  Funciones de Instancia 					*/
/* ---------------------------------------- */

	int persistirDatos(Almacenamiento almacenamiento,Instruccion* datosInstruccion ,char* algoritmoAlmacenamiento, int* puntero);
	void cargarTablaEntradas(t_list *tablaEntradas,Instruccion* estructuraInstruccion, Almacenamiento almacenamiento);
	void persistirEntrada(t_entrada* unaEntrada, char* puntoMontaje, Almacenamiento almacenamiento);
	/* Función para devolver un error */
	void error(const char *s);
	void procesoArchivoDump(char *archivo, char* punto_montaje, Almacenamiento almacenamiento);
	void dump(t_list* tablaEntradas, char* puntoMontaje, Almacenamiento almacenamiento);
	int cantidadDirectoriosPath(char* pathDirectorio);
	bool crearEstructuraDirectorios(char* pathArchivo);

	bool esIgualAKey(t_entrada* unaEntrada);
	bool esIgualA(t_entrada* unaEntrada, char key[40]);
	bool existeEntradaEnTabla(t_list* tablaEntradas, char key[40]);
	void ejecutarCadaXTiempo(void funcionAEjecutar(), int tiempo );

	char* leerBinarioEnPosicion(Almacenamiento almacenamiento, int posicion);
	void escribirBinarioEnPosicion(Almacenamiento almacenamiento, int posicion, char* valor);
	char* valorEntrada(t_entrada* entrada);
	int buscarPosicionEnBin(Almacenamiento almacenamiento, char* valor);
	void preCargarTablaEntradas(char* puntoMontaje, Almacenamiento almacenamiento);
	bool realizarCompactacionLocal(Almacenamiento almacenamiento);
	void grabarPosicionEnVector(Almacenamiento almacenamiento, int posicion);
	void liberarPosicionEnVector(Almacenamiento almacenamiento, int posicion);
	void liberarEntradaEnVector(Almacenamiento almacenamiento, t_entrada* entrada);
	void grabarEntradaEnVector(Almacenamiento almacenamiento, int posicion, t_entrada* entrada);

	int espacioLibre(Almacenamiento almacenamiento);
	void liberarUnEspacio(Almacenamiento almacenamiento, int* puntero);
	void incrementarPuntero(Almacenamiento almacenamiento, int* puntero);
	bool esEntradaAtomica(Almacenamiento almacenamiento,t_entrada* entrada);
	void destruirEntradaEnPosicion(Almacenamiento almacenamiento, int posicion);

	void showContenidoTablaEntradas(t_list* tablaEntradas);
	void limpiarInstancia(char* puntoMontaje);

	t_entrada* ultimaEntrada(Almacenamiento almacenamiento);
	t_entrada* obtenerEntrada(t_list* tablaEntradas, char* clave);
/* ---------------------------------------- */
#endif
