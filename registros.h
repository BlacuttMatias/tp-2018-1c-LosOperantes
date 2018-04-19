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



// Ejemplo de definicion de Struct
typedef struct {
	int id_job;
	int master;
	char* nodo;
	int bloque;
	char* etapa;
	char* archivo_tmp;
	int estado; // EN_PROCESO=1, ERROR=2, FINALIZADO=3
	int uso;//1 fue usado para la reduccion local
	int replanificado;//1 se replanifico
}t_estados;



#endif
