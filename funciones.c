#include "funciones.h"

//*********************** SERIALIZADO Y DESERIALIZADO ********************************//

Paquete srlz_solicitudTransformacion(t_solicitud_transformacion* solicitud)
{
	int codigo = INICIAR_TRANSFORMACION_ARCHIVO;
	char proceso='Y';
	int posicion = 0;//int para ir guiando desde donde se copia
	int sizeBuffer = 0;
	int tamString = 0;
	int tamPayload = 0;


	Paquete paquete;
	sizeBuffer =sizeof(char)+
			(sizeof(int)*8) // 2 son para el codigo y el "tam_buffer"; otros 3 para bloque,worker_port y bytes_ocupados; y los otros 3 para tamString
			+ strlen(solicitud->nodo)
			+ strlen(solicitud->worker_ip)
			+ strlen(solicitud->archivo_tmp);

	paquete.tam_buffer = sizeBuffer;
	paquete.buffer = malloc( sizeBuffer );
	tamPayload = sizeBuffer - (sizeof(int)*2) - sizeof(char);

	//memcpy(dir de dónde copiaré, dir de lo que copiaré, tamaño de lo que copiaré)
	memcpy(paquete.buffer                                                   ,&(proceso)                     ,sizeof(char));
	memcpy(paquete.buffer + (posicion=sizeof(proceso))						,&(codigo)						,sizeof(int)); //copio el codigo
	memcpy(paquete.buffer + (posicion += sizeof(codigo))					,&(tamPayload)					,sizeof(tamPayload)); //se copia el tamaño del buffer
	memcpy(paquete.buffer + (posicion += sizeof(tamPayload))		,&(solicitud->bytes_ocupados)			,sizeof(solicitud->bytes_ocupados)); //se copia el campo bytes ocupados
	memcpy(paquete.buffer + (posicion += sizeof(solicitud->bytes_ocupados))	,&(solicitud->bloque)			,sizeof(solicitud->bloque)); //se copia el campo bloque

	tamString = strlen(solicitud->worker_ip);
	memcpy(paquete.buffer + (posicion += sizeof(solicitud->bloque))			,&(tamString)					,sizeof(int) ); //guardo el tam del siguiente array
	memcpy(paquete.buffer + (posicion += sizeof(int))						,solicitud->worker_ip			,strlen(solicitud->worker_ip)); //se copia el campo ip


	tamString = strlen(solicitud->nodo);
	memcpy(paquete.buffer + (posicion += strlen(solicitud->worker_ip))		,&(tamString)					,sizeof(int) ); //guardo el tam del siguiente array
	memcpy(paquete.buffer + (posicion += sizeof(int) )						,solicitud->nodo				,strlen(solicitud->nodo)); //se copia el campo nodo
	tamString = strlen(solicitud->archivo_tmp);
	memcpy(paquete.buffer + (posicion += strlen(solicitud->nodo))			,&(tamString)					,sizeof(int) ); //guardo el tam del siguiente array
	memcpy(paquete.buffer + (posicion += sizeof(int) )						,solicitud->archivo_tmp			,strlen(solicitud->archivo_tmp)); //se copia el campo archivo tmp
	//tamString = strlen(solicitud->worker_port);
	//memcpy(paquete.buffer + (posicion += strlen(solicitud->archivo_tmp))	,&(tamString)					,sizeof(int) ); //guardo el tam del siguiente array
	memcpy(paquete.buffer + (posicion += strlen(solicitud->archivo_tmp) )						,&(solicitud->worker_port)			,sizeof(solicitud->worker_port)); //se copia el campo port

	return paquete;
}

t_solicitud_transformacion dsrlz_solicitudTransformacion(void* buffer) //este buffer es el buffer original - cod de op - tam buffer
{
	int posicion = 0; //int para ir guiando desde donde se copia
	int tamString = 0;
	t_solicitud_transformacion solicitud;

	//salteo los primeros 2 ints que contienen el cod_operacion y el tamano de PayLoad
	memcpy(&solicitud.bytes_ocupados 		,buffer+posicion					,sizeof(int));
	memcpy(&solicitud.bloque 				,buffer+(posicion+=sizeof(int))						,sizeof(int));

	memcpy(&tamString				  		,buffer+(posicion+=sizeof(int))						,sizeof(int));
	solicitud.worker_ip = malloc(sizeof(char) * tamString+1);
	memcpy(solicitud.worker_ip		   	   	,buffer+(posicion+=sizeof(int))				  		,sizeof(char) * tamString);
    solicitud.worker_ip[tamString]='\0';

	memcpy(&(tamString)					 	,buffer+(posicion+=sizeof(char) * tamString)		,sizeof(int));
	solicitud.nodo = malloc(sizeof(char) * tamString+1);
	memcpy(solicitud.nodo			   	    ,buffer+(posicion+=sizeof(int))						,sizeof(char)*tamString);
	solicitud.nodo[tamString]='\0';

	memcpy(&(tamString)					    ,buffer+(posicion+=sizeof(char) * tamString)		,sizeof(int));
	solicitud.archivo_tmp = malloc(sizeof(char) * tamString+1);
	memcpy(solicitud.archivo_tmp	        ,buffer+(posicion+=sizeof(int))					    ,sizeof(char)*tamString);
	solicitud.archivo_tmp[tamString]='\0';

	//memcpy(&(tamString)					    ,buffer+(posicion+=sizeof(char) * tamString)		,sizeof(int));
	//solicitud.worker_port = malloc(sizeof(char) * tamString);
	memcpy(&solicitud.worker_port	        ,buffer+(posicion+=sizeof(char) * tamString)					    ,sizeof(int));


	return solicitud;
}


//**************************************************************************//
// Creacion del Encabezado Fijo del Paquete 								
//**************************************************************************//
Paquete crearHeader(char proceso, int cod_operacion, int tamPayload){

	int posicion;
	int sizeBuffer = 0;
	Paquete paquete;
	sizeBuffer = (sizeof(int) * 2) + sizeof(char);
	paquete.tam_buffer = sizeBuffer;
	paquete.buffer = malloc( sizeBuffer );

	//memcpy(dir de dónde copiaré, dir de lo que copiaré, tamaño de lo que copiaré)
	memcpy(paquete.buffer                                           ,&proceso               ,sizeof(char));
	memcpy(paquete.buffer + (posicion  = sizeof(proceso))			,&cod_operacion			,sizeof(int));
	memcpy(paquete.buffer + (posicion += sizeof(cod_operacion))		,&tamPayload			,sizeof(int));	

	return paquete;
}

