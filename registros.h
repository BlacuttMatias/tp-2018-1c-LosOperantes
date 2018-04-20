#ifndef __REGISTROS_H__
#define __REGISTROS_H__

//******************************************

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Defino los Codigo de Operaciones
enum cod_operacion {
	EJECUTAR_INSTRUCCION = 1,
	SOLICITUD_EJECUCION_INSTRUCCION = 2,
	NOTIFICAR_USO_RECURSO = 3,
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



#endif
