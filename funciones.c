#include "funciones.h"

//*********************** SERIALIZADO Y DESERIALIZADO ********************************//

Paquete srlz_datosProceso(char proceso, int codigoOperacion, char* nombreProceso, int rafagaAnterior){

	int posicion = 0;//int para ir guiando desde donde se copia
	int sizeBuffer = 0;
	int tamString = 0;
	int tamPayload = 0;
	Paquete paquete;


	sizeBuffer =sizeof(char)+
			(sizeof(int)*4)
			+ strlen(nombreProceso);

	paquete.tam_buffer = sizeBuffer;
	paquete.buffer = malloc( sizeBuffer );
	tamPayload = sizeBuffer - (sizeof(int)*2) - sizeof(char);

	memcpy(paquete.buffer                                                   ,&(proceso)                     ,sizeof(char));
	memcpy(paquete.buffer + (posicion=sizeof(char))							,&(codigoOperacion)				,sizeof(int));
	memcpy(paquete.buffer + (posicion += sizeof(int))						,&(tamPayload)					,sizeof(int));

	tamString = strlen(nombreProceso);
	memcpy(paquete.buffer + (posicion += sizeof(tamPayload))				,&(tamString)					,sizeof(int) ); //guardo el tam del siguiente array
	memcpy(paquete.buffer + (posicion += sizeof(int) )						,nombreProceso					,strlen(nombreProceso)); //se copia el nombre

	memcpy(paquete.buffer + (posicion += strlen(nombreProceso))				,&(rafagaAnterior)				,sizeof(int));

	return paquete;
}

Proceso dsrlz_datosProceso(void* buffer)
{
	int posicion = 0; //int para ir guiando desde donde se copia
	int tamString = 0;
	Proceso solicitud;


	memcpy(&(tamString)					 	,buffer+posicion										,sizeof(int));
	solicitud.nombreProceso = malloc(sizeof(char) * tamString+1);
	memcpy(solicitud.nombreProceso			,buffer+(posicion+=sizeof(int))							,sizeof(char)*tamString);
	solicitud.nombreProceso[tamString]='\0';

	memcpy(&solicitud.rafagaAnterior 		,buffer+(posicion+=tamString)							,sizeof(int));


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


//**************************************************************************//
// Persistir Datos en el Coordinador
//**************************************************************************//
bool persistirDatos(Instruccion* datosInstruccion, char* algoritmoDistricucion){

	// Dependiendo el algoritmoDistricucion, persistir los datos localmente

	return true;
}

//**************************************************************************//
// Procesar script de Instrucciones
//**************************************************************************//
bool procesarScript(char* pathScript, t_list* listaInstrucciones){

	// Abro el Script
    FILE *archivo = fopen(pathScript, "r");
    int caracter;

	Instruccion* registroInstruccion = NULL;
	registroInstruccion = malloc(sizeof(Instruccion));

	char *unaInstruccion = string_new();

	// Si se pudo posicionar dentro del archivo
	if(fseek( archivo, 0, SEEK_SET ) == 0){


	    while ((caracter = fgetc(archivo)) != EOF) {

// printf("%c", caracter);

			// Si leyo una linea completa, agrego la instruccion a la lista
			if(caracter == '\n'){
 
 printf("%s\n",unaInstruccion);

        		// Cargo el Registro de la instruccion
        		registroInstruccion->texto_instruccion = malloc(strlen(unaInstruccion)+1);       		
        		strcpy( registroInstruccion->texto_instruccion ,unaInstruccion);        		
        		registroInstruccion->texto_instruccion[strlen(unaInstruccion)] = '\0';

        		// Agrego la instruccion a la lista
				list_add(listaInstrucciones,registroInstruccion);

				// Inicializo el string de la Instruccion
				unaInstruccion = string_new();
			}else{
				string_append_with_format(&unaInstruccion, "%c", caracter);	
			}
	    }
	}


	// Muestro por pantalla el contenido de la Lista
	int indice = 0;

	if(list_size(listaInstrucciones) > 0){

	    void _each_elemento_(Instruccion* registroInstruccionAux)
		{
			indice = indice + 1;

			// Muestro el encabezaado
			if(indice == 1) {
				printf("\nINSTRUCCIONES\n");
				printf("--------------\n");
			}

			printf("%s\n", registroInstruccionAux->texto_instruccion);

		}
	    list_iterate(listaInstrucciones, (void*)_each_elemento_);
	}



	// Cierro el FD
	fclose(archivo);

	return true;
}

//**************************************************************************//
// Generar las Estructuras Administrativas del Coordinador
//**************************************************************************//
void inicializarEstructurasAdministrativas(){

}

//**************************************************************************//
// Registrar las Instrucciones en el Log de Operaciones
//**************************************************************************//
bool registrarLogOperaciones(Instruccion* datosInstruccion, char* nombreProceso){

	return true;
}

//**************************************************************************//
// El Coordinador procesa la instruccion y elige la Instancia que lo va a ejecutar
//**************************************************************************//
char* procesarSolicitudEjecucion(Instruccion* datosInstruccion, char* algoritmoDistribucion){

	return "INSTANCIA1";
}


//**************************************************************************//
// Mostrar el contenido de la Lista Ready
//**************************************************************************//
void showContenidolistaReady(t_list* listaReady){ 

	int indice = 0;

	if(list_size(listaReady) > 0){

	    void _each_elemento_(Proceso* registroProcesoAux)
		{
			indice = indice + 1;

			// Muestro el encabezaado
			if(indice == 1) {
				printf("\nLISTA READY\n");
				printf("Proceso \t Rafaga Anterior\n");
				printf("------\t -------\n");
			}

			printf("%s \t %d\n", registroProcesoAux->nombreProceso,registroProcesoAux->rafagaAnterior);

		}
	    list_iterate(listaReady, (void*)_each_elemento_);
	}

}
