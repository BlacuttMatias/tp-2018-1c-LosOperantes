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
	INSTANCIA_INEXISTENTE = 5,
	RECURSO_TOMADO = 6,
	FINALIZACION_EJECUCION_ESI = 7,
};

// Defino los Tipos de Procesos
enum tipo_proceso {
	ESI = 1,
	PLANIFICADOR = 2,
	INSTANCIA = 3,
	COORDINADOR = 4,
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
	int operacion;
	char key[40];
	char* dato;
}Instruccion;

typedef struct
{
	char* nombreProceso;
	char key[40];
}KeyBloqueada;


typedef struct
{
	char* nombreProceso;
	int tipoProceso;
	int socketProceso;	
}Proceso;

typedef struct
{
	char* nombreProceso;
	int socketProceso;
	int entradasLibres;

}Instancia;


//Instancia
typedef struct{ //tipo entrada
	char* clave;
	char* valor;
	int numeroDeEntrada;
	int tamanioValorAlmacenado;
} t_entrada;


#endif
