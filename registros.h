#ifndef __REGISTROS_H__
#define __REGISTROS_H__

//******************************************

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Defino los Codigo de Operaciones
enum cod_operacion {
	EJECUTAR_INSTRUCCION = 1,
	RESPUESTA_EJECUTAR_INSTRUCCION = 2,
	NOTIFICAR_USO_RECURSO = 3,
	HANDSHAKE = 4,
	INSTANCIA_DESCONECTADA = 5,
	RECURSO_LIBRE = 6,
	RECURSO_OCUPADO = 7,	
	FINALIZACION_EJECUCION_ESI = 8,
	NOTIFICAR_LIBERACION_RECURSO = 9,
	OBTENER_STATUS_CLAVE = 10,
	ESI_MUERE = 11,
	IS_ALIVE = 12,	
	OBTENCION_CONFIG_ENTRADAS = 13,
};

// Defino los Tipos de Procesos
enum tipo_proceso {
	ESI = 1,
	PLANIFICADOR = 2,
	INSTANCIA = 3,
	COORDINADOR = 4,
};

//Defino los algoritmos de distribucion
enum algoritmo_distribucion{
	LSU=1,
	EL=2,
	KE=3,
};

//Defino los Resultados de una Operacion
enum resultado_operaciones{
	EJECUCION_EXITOSA =1,
	EJECUCION_FALLIDA=2,
	EJECUCION_FALLIDA_FINALIZAR_ESI=3,	
};


typedef struct //Paquete
{
	void* buffer;
	int tam_buffer;
}Paquete;

typedef struct //Encabezado
{
	char proceso;
	int cod_operacion;
	int tam_payload;
}Encabezado;


typedef struct
{
	char* nombreEsiOrigen;
	int operacion;
	char key[40];
	char* dato;
}Instruccion;

typedef struct
{
	char* nombreEsiDestino;
	int operacion;
	char key[40];
	int resultado;
	char* contenido;
}ResultadoEjecucion;

typedef struct
{
	char* nombreProceso;
	int operacion;
	char key[40];
	char* dato;	
}KeyBloqueada;


typedef struct
{
	char* nombreProceso;
	int tipoProceso;
	int socketProceso;	
}Proceso;

typedef struct
{
	int rafagaAnterior;
	float estimacionRafagaAnterior;
	float proximaEstimacion;
	int tiempoDeEsperaDeCpu;
}Rafagas;

typedef struct
{
	char* nombreProceso;
	float proximaEstimacion;
}ProcesoConRafaga;

typedef struct
{
	char* nombreProceso;
	int socketProceso;
	int entradasLibres;
}Instancia;


//Instancia
typedef struct{ //tipo entrada
	char* clave;
	int numeroDeEntrada;
	int tamanioValorAlmacenado;
} t_entrada;

typedef struct{ 
	int cantEntrada;
	int tamanioEntrada;
} EntradasIntancias;

#endif
