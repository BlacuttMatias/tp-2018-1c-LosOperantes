#include "funciones.h"

//*********************** SERIALIZADO Y DESERIALIZADO ********************************//

Paquete srlz_resultadoEjecucion(char proceso, int codigoOperacion, char* nombreEsiDestino, int resultado, char* contenido, int operacion, char key[40]){
	int posicion = 0;
	int sizeBuffer = 0;
	int tamString = 0;
	int tamPayload = 0;
	Paquete paquete;

	sizeBuffer =sizeof(char)+
			(sizeof(int)*7) + strlen(nombreEsiDestino) + strlen(contenido) + strlen(key);

	paquete.tam_buffer = sizeBuffer;
	paquete.buffer = malloc( sizeBuffer );
	tamPayload = sizeBuffer - (sizeof(int)*2) - sizeof(char);

	memcpy(paquete.buffer                                                   ,&(proceso)                     ,sizeof(char));
	memcpy(paquete.buffer + (posicion=sizeof(char))							,&(codigoOperacion)				,sizeof(int));
	memcpy(paquete.buffer + (posicion += sizeof(int))						,&(tamPayload)					,sizeof(int));

	tamString = strlen(nombreEsiDestino);
	memcpy(paquete.buffer + (posicion += sizeof(int))						,&(tamString)					,sizeof(int) ); 
	memcpy(paquete.buffer + (posicion += sizeof(int) )						,nombreEsiDestino				,tamString); 

	memcpy(paquete.buffer + (posicion += tamString)							,&(resultado)					,sizeof(int) ); 

	tamString = strlen(contenido);
	memcpy(paquete.buffer + (posicion += sizeof(int))						,&(tamString)					,sizeof(int) ); 
	memcpy(paquete.buffer + (posicion += sizeof(int) )						,contenido						,tamString); 

	memcpy(paquete.buffer + (posicion += tamString)							,&(operacion)					,sizeof(int) ); 

	tamString = strlen(key);
	memcpy(paquete.buffer + (posicion += sizeof(int) )						,&(tamString)					,sizeof(int) );
	memcpy(paquete.buffer + (posicion += sizeof(int) )						,key							,tamString); 

	return paquete;	
}

ResultadoEjecucion dsrlz_resultadoEjecucion(void* buffer){
	int posicion = 0; 
	int tamString = 0;
	ResultadoEjecucion solicitud;


	memcpy(&(tamString)					 	,buffer+posicion											,sizeof(int));
	solicitud.nombreEsiDestino = malloc(sizeof(char) * tamString+1);
	memcpy(solicitud.nombreEsiDestino			,buffer+(posicion+=sizeof(int))							,sizeof(char)*tamString);
	solicitud.nombreEsiDestino[tamString]='\0';

	memcpy(&solicitud.resultado 			,buffer+(posicion+=sizeof(char) * tamString)				,sizeof(int));

	memcpy(&(tamString)					 	,buffer+(posicion+=sizeof(int))								,sizeof(int));
	solicitud.contenido = malloc(sizeof(char) * tamString+1);
	memcpy(solicitud.contenido			,buffer+(posicion+=sizeof(int))									,sizeof(char)*tamString);
	solicitud.contenido[tamString]='\0';

	memcpy(&solicitud.operacion 			,buffer+(posicion+=sizeof(char) * tamString)				,sizeof(int));

	memcpy(&(tamString)					 	,buffer+(posicion+=sizeof(int))								,sizeof(int));
	memcpy(solicitud.key					,buffer+(posicion+=sizeof(int))								,sizeof(char)*tamString);
	solicitud.key[tamString]='\0';

	return solicitud;
}


Paquete srlz_datosEntradas(char proceso, int codigoOperacion, int cantEntrada, int tamanioEntrada){

	int posicion = 0;
	int sizeBuffer = 0;
	int tamString = 0;
	int tamPayload = 0;
	Paquete paquete;


	sizeBuffer =sizeof(char)+
			(sizeof(int)*4);

	paquete.tam_buffer = sizeBuffer;
	paquete.buffer = malloc( sizeBuffer );
	tamPayload = sizeBuffer - (sizeof(int)*2) - sizeof(char);

	memcpy(paquete.buffer                                                   ,&(proceso)                     ,sizeof(char));
	memcpy(paquete.buffer + (posicion=sizeof(char))							,&(codigoOperacion)				,sizeof(int));
	memcpy(paquete.buffer + (posicion += sizeof(int))						,&(tamPayload)					,sizeof(int));

	memcpy(paquete.buffer + (posicion += sizeof(int))						,&(cantEntrada)					,sizeof(int) ); 
	memcpy(paquete.buffer + (posicion += sizeof(int))						,&(tamanioEntrada)				,sizeof(int) ); 

	return paquete;
}

EntradasIntancias dsrlz_datosEntradas(void* buffer)
{
	int posicion = 0; 
	EntradasIntancias solicitud;

	memcpy(&(solicitud.cantEntrada)					 	,buffer+posicion							,sizeof(int));
	memcpy(&(solicitud.tamanioEntrada)					,buffer+(posicion+=sizeof(int))				,sizeof(int));

	return solicitud;
}

Paquete srlz_datosStatusRecurso(char proceso, int codigoOperacion, char* nombreInstanciaActual, char* nombreInstanciaFutura, char* valorRecurso, char key[40]){

	int posicion = 0;
	int sizeBuffer = 0;
	int tamString = 0;
	int tamPayload = 0;
	Paquete paquete;


	sizeBuffer =sizeof(char)+
			(sizeof(int)*6)
			+ strlen(nombreInstanciaActual) + strlen(nombreInstanciaFutura) + strlen(valorRecurso) + strlen(key);

	paquete.tam_buffer = sizeBuffer;
	paquete.buffer = malloc( sizeBuffer );
	tamPayload = sizeBuffer - (sizeof(int)*2) - sizeof(char);

	memcpy(paquete.buffer                                           ,&(proceso)                     ,sizeof(char));
	memcpy(paquete.buffer + (posicion=sizeof(char))					,&(codigoOperacion)				,sizeof(int));
	memcpy(paquete.buffer + (posicion += sizeof(int))				,&(tamPayload)					,sizeof(int));

	tamString = strlen(nombreInstanciaActual);
	memcpy(paquete.buffer + (posicion += sizeof(int))				,&(tamString)					,sizeof(int) );
	memcpy(paquete.buffer + (posicion += sizeof(int) )				,nombreInstanciaActual			,strlen(nombreInstanciaActual));

	tamString = strlen(nombreInstanciaFutura);
	memcpy(paquete.buffer + (posicion += strlen(nombreInstanciaActual))			,&(tamString)					,sizeof(int) );
	memcpy(paquete.buffer + (posicion += sizeof(int) )				,nombreInstanciaFutura			,strlen(nombreInstanciaFutura));

	tamString = strlen(valorRecurso);
	memcpy(paquete.buffer + (posicion += strlen(nombreInstanciaFutura))			,&(tamString)					,sizeof(int) );
	memcpy(paquete.buffer + (posicion += sizeof(int) )				,valorRecurso					,strlen(valorRecurso));

	tamString = strlen(key);
	memcpy(paquete.buffer + (posicion += strlen(valorRecurso))		,&(tamString)					,sizeof(int) );
	memcpy(paquete.buffer + (posicion += sizeof(int) )				,key							,strlen(key));

	return paquete;
}

StatusRecurso dsrlz_datosStatusRecurso(void* buffer)
{
	int posicion = 0; 
	int tamString = 0;
	StatusRecurso solicitud;

	memcpy(&(tamString)					 	,buffer+posicion										,sizeof(int));
	solicitud.nombreInstanciaActual = malloc(sizeof(char) * tamString+1);
	memcpy(solicitud.nombreInstanciaActual			,buffer+(posicion+=sizeof(int))					,sizeof(char)*tamString);
	solicitud.nombreInstanciaActual[tamString]='\0';

	memcpy(&(tamString)					 	,buffer+(posicion+=sizeof(char)*tamString)				,sizeof(int));
	solicitud.nombreInstanciaFutura = malloc(sizeof(char) * tamString+1);
	memcpy(solicitud.nombreInstanciaFutura			,buffer+(posicion+=sizeof(int))					,sizeof(char)*tamString);
	solicitud.nombreInstanciaFutura[tamString]='\0';

	memcpy(&(tamString)					 	,buffer+(posicion+=sizeof(char)*tamString)				,sizeof(int));
	solicitud.valorRecurso = malloc(sizeof(char) * tamString+1);
	memcpy(solicitud.valorRecurso			,buffer+(posicion+=sizeof(int))							,sizeof(char)*tamString);
	solicitud.valorRecurso[tamString]='\0';

	memcpy(&(tamString)					 	,buffer+(posicion+=sizeof(char)*tamString)				,sizeof(int));
	memcpy(solicitud.key					,buffer+(posicion+=sizeof(int))							,sizeof(char)*tamString);
	solicitud.key[tamString]='\0';

	return solicitud;
}

Paquete srlz_datosKeyBloqueada(char proceso, int codigoOperacion, char* nombreProceso, int operacion, char key[40], char* dato){

	int posicion = 0;
	int sizeBuffer = 0;
	int tamString = 0;
	int tamPayload = 0;
	Paquete paquete;


	if(operacion == GET || operacion == STORE){
		sizeBuffer =sizeof(char)+
				(sizeof(int)*5)
				+ strlen(nombreProceso)
				+ strlen(key);	
	}else{
		sizeBuffer =sizeof(char)+
				(sizeof(int)*6)
				+ strlen(nombreProceso)
				+ strlen(key)
				+ strlen(dato);
	}

	paquete.tam_buffer = sizeBuffer;
	paquete.buffer = malloc( sizeBuffer );
	tamPayload = sizeBuffer - (sizeof(int)*2) - sizeof(char);


	memcpy(paquete.buffer                                                   ,&(proceso)                     ,sizeof(char));
	memcpy(paquete.buffer + (posicion=sizeof(char))							,&(codigoOperacion)				,sizeof(int));
	memcpy(paquete.buffer + (posicion += sizeof(int))						,&(tamPayload)					,sizeof(int));

	tamString = strlen(nombreProceso);
	memcpy(paquete.buffer + (posicion += sizeof(int))						,&(tamString)					,sizeof(int) );
	memcpy(paquete.buffer + (posicion += sizeof(int) )						,nombreProceso					,strlen(nombreProceso));

	memcpy(paquete.buffer + (posicion += strlen(nombreProceso))				,&(operacion)					,sizeof(int) );

	tamString = strlen(key);
	memcpy(paquete.buffer + (posicion += sizeof(int) )						,&(tamString)					,sizeof(int) );
	memcpy(paquete.buffer + (posicion += sizeof(int) )						,key							,strlen(key)); 

	if(operacion==SET){	
		tamString = strlen(dato);
		memcpy(paquete.buffer + (posicion += strlen(key))					,&(tamString)					,sizeof(int));
		memcpy(paquete.buffer + (posicion += sizeof(int))					,dato							,strlen(dato));
	}

	return paquete;
}

KeyBloqueada dsrlz_datosKeyBloqueada(void* buffer){

	int posicion = 0; 
	int tamString = 0;
	char* keyAux;
	KeyBloqueada solicitud;

	memcpy(&(tamString)					 	,buffer+posicion										,sizeof(int));
	solicitud.nombreProceso = malloc(sizeof(char) * tamString+1);
	memcpy(solicitud.nombreProceso			,buffer+(posicion+=sizeof(int))							,sizeof(char)*tamString);
	solicitud.nombreProceso[tamString]='\0';

	memcpy(&solicitud.operacion			 	,buffer+(posicion+=sizeof(char)*tamString)				,sizeof(int));

	memcpy(&(tamString)					 	,buffer+(posicion+=sizeof(int))							,sizeof(int));
	memcpy(solicitud.key					,buffer+(posicion+=sizeof(int))							,sizeof(char)*tamString);	
	solicitud.key[tamString] = '\0';

	if(solicitud.operacion==SET){	

		memcpy(&(tamString)					 	,buffer+(posicion+=sizeof(char)*tamString)				,sizeof(int));

		solicitud.dato = malloc(sizeof(char) * tamString+1);
		memcpy(solicitud.dato					,buffer+(posicion+=sizeof(int))							,sizeof(char)*tamString);	
		solicitud.dato[tamString] = '\0';

	} else {
		solicitud.dato = NULL;
	}

	return solicitud;
}

Paquete srlz_datosProceso(char proceso, int codigoOperacion, char* nombreProceso, int tipoProceso, int socketProceso){

	int posicion = 0;
	int sizeBuffer = 0;
	int tamString = 0;
	int tamPayload = 0;
	Paquete paquete;


	sizeBuffer =sizeof(char)+
			(sizeof(int)*5)
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

	memcpy(paquete.buffer + (posicion += strlen(nombreProceso))				,&(tipoProceso)				,sizeof(int));
	memcpy(paquete.buffer + (posicion += sizeof(int))						,&(socketProceso)				,sizeof(int));

	return paquete;
}

Proceso dsrlz_datosProceso(void* buffer)
{
	int posicion = 0; 
	int tamString = 0;
	Proceso solicitud;


	memcpy(&(tamString)					 	,buffer+posicion										,sizeof(int));
	solicitud.nombreProceso = malloc(sizeof(char) * tamString+1);
	memcpy(solicitud.nombreProceso			,buffer+(posicion+=sizeof(int))							,sizeof(char)*tamString);
	solicitud.nombreProceso[tamString]='\0';

	memcpy(&solicitud.tipoProceso 		,buffer+(posicion+=sizeof(char) * tamString)			,sizeof(int));
	memcpy(&solicitud.socketProceso 			,buffer+(posicion+=sizeof(int))							,sizeof(int));

	return solicitud;
}

Paquete srlz_instruccion (char proceso, int codigoOperacion,Instruccion instruccion){

	
	int posicion = 0;
	int sizeBuffer = 0;
	int tamClave = 0;
	int tamPayload = 0;
	int tamDato = 0;
	int tamNombreEsi = 0;
	Paquete paquete;

	tamClave = strlen(instruccion.key);
	tamNombreEsi = strlen(instruccion.nombreEsiOrigen);

	//dependiendo de si se hace un get/store o un set, el tamaño del buffer sera uno u otro, porque en el set se agrega tambien el valor asociado a la key
	if(instruccion.operacion == GET || instruccion.operacion == STORE){
		sizeBuffer = sizeof(int)*5 + sizeof(char) + tamClave + tamNombreEsi;
	}
	else{
		tamDato = strlen(instruccion.dato);
		sizeBuffer = sizeof(int)*6 + sizeof(char) + tamClave + tamDato + tamNombreEsi;	//agrego el tamaño del valor y el valor(el dato de la instruccion)
	}
	paquete.tam_buffer = sizeBuffer;
	paquete.buffer = malloc( sizeBuffer );
	tamPayload = sizeBuffer - (sizeof(int)*2) - sizeof(char);


	memcpy(paquete.buffer									,&(proceso)                     ,sizeof(char));
	memcpy(paquete.buffer + (posicion=sizeof(char))			,&(codigoOperacion)				,sizeof(int));
	memcpy(paquete.buffer + (posicion += sizeof(int))		,&(tamPayload)					,sizeof(int));

	
	memcpy(paquete.buffer + (posicion += sizeof(int))		,&(tamNombreEsi)				,sizeof(int));
	memcpy(paquete.buffer + (posicion += sizeof(int))		,instruccion.nombreEsiOrigen	,tamNombreEsi);


	memcpy(paquete.buffer + (posicion+=tamNombreEsi)		,&(instruccion.operacion)		,sizeof(int));

	memcpy(paquete.buffer + (posicion += sizeof(int))		,&(tamClave)					,sizeof(int));
	memcpy(paquete.buffer + (posicion += sizeof(int))		,instruccion.key				,tamClave);

	if(instruccion.operacion==SET){	//en caso ser un set, pongo en el buffer el tamaño del dato y el dato, en get y store no hace falta porque el dato no existe
		memcpy(paquete.buffer + (posicion += tamClave)			,&(tamDato)						,sizeof(int));
		memcpy(paquete.buffer + (posicion += sizeof(int))		,instruccion.dato				,tamDato);
	}

	return paquete;
}


Instruccion dsrlz_instruccion (void* buffer){

	
	int posicion = 0; 
	int tamClave = 0;
	int tamDato = 0;	
	Instruccion instruccion;

	memcpy(&tamDato							,buffer + posicion							,sizeof(int));
	instruccion.nombreEsiOrigen = malloc(tamDato+1);
	memcpy(instruccion.nombreEsiOrigen					,buffer + (posicion+=sizeof(int))						,tamDato);
	instruccion.nombreEsiOrigen[tamDato] = '\0';


	memcpy(&instruccion.operacion			,buffer + (posicion+=tamDato)							,sizeof(int));
	memcpy(&tamClave						,buffer + (posicion+=sizeof(int))						,sizeof(int));
	memcpy(instruccion.key					,buffer + (posicion+=sizeof(int))						,tamClave);
	instruccion.key[tamClave] = '\0';

	if(instruccion.operacion==SET){	//si es un set significa que tengo que seguir leyendo del buffer el valor asociado a la key(el dato)

		memcpy(&tamDato							,buffer + (posicion+=tamClave)							,sizeof(int));
		instruccion.dato = malloc(tamDato+1);
		memcpy(instruccion.dato					,buffer + (posicion+=sizeof(int))						,tamDato);
		instruccion.dato[tamDato] = '\0';

	} else {
		instruccion.dato = NULL;
	}

	return instruccion;
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
//Sacar Siguiente instruccion de la lista
//**************************************************************************//
Instruccion* sacarSiguienteInstruccion(t_list* listaInstruccion) {
	Instruccion* instruccionAux=NULL;
	if(list_size(listaInstruccion)>0){
		instruccionAux =	list_remove(listaInstruccion,0);

	}
	return instruccionAux;
}

//**************************************************************************//
// Se carga un Registro de Instruccion
//**************************************************************************//
Instruccion pasarAEstructura(Instruccion* puntero, char* nombreEsi) {
	Instruccion instruccion;
	instruccion.operacion= (puntero->operacion);
	strcpy(instruccion.key,puntero->key);
	instruccion.dato= puntero->dato;

	instruccion.nombreEsiOrigen=malloc(strlen(nombreEsi)+1);
	strcpy(instruccion.nombreEsiOrigen,nombreEsi);
	instruccion.nombreEsiOrigen[strlen(nombreEsi)] = '\0';

	return instruccion;
}

//**************************************************************************//
// Procesar script de Instrucciones
//**************************************************************************//
bool procesarScript(char* pathScript, t_list* listaInstrucciones){

	// Abro el Script
    FILE *archivo = fopen(pathScript, "r");
    int caracter;
	char *unaInstruccion = string_new();
	int numeroLinea=0;

	// Si se pudo posicionar dentro del archivo
	if(fseek( archivo, 0, SEEK_SET ) == 0){
		//puts("fseek");
	    while ((caracter = fgetc(archivo)) != EOF) {

			// Si leyo una linea completa, agrego la instruccion a la lista
			if(caracter == '\n'){
				//puts("leo una linea");
				numeroLinea +=1;
 //printf("%s\n",unaInstruccion);

				//chequeo error de parseo
				t_esi_operacion parsed= parse(unaInstruccion);
				Instruccion* registroInstruccion = NULL;
				registroInstruccion = malloc(sizeof(Instruccion));

				if(!parsed.valido){
					printf("instruccion %s invalida en posicion %d \n",unaInstruccion,numeroLinea);
				//	log_info(infoLogger,"instruccion %s invalida en posicion %d \n",unaInstruccion,numeroLinea);
					exit(EXIT_FAILURE);
				}
				else{
					switch(parsed.keyword){
					case GET:
						cargarInstruccion(registroInstruccion,GET,parsed.argumentos.GET.clave,NULL);

						break;
					case SET:
						cargarInstruccion(registroInstruccion,SET,parsed.argumentos.SET.clave,parsed.argumentos.SET.valor);

						break;
					case STORE:
						cargarInstruccion(registroInstruccion,STORE,parsed.argumentos.STORE.clave,NULL);

						break;
					default:
						printf("instruccion %s no es interpretable en posicion %d \n",unaInstruccion,numeroLinea);
						//log_info("instruccion %s no es interpretable en posicion %d \n",unaInstruccion,numeroLinea);
						exit(EXIT_FAILURE);
					}
				}

        		// Agrego la instruccion a la lista
				list_add(listaInstrucciones,registroInstruccion);

				// Inicializo el string de la Instruccion
				unaInstruccion = string_new();
			}else{
				string_append_with_format(&unaInstruccion, "%c", caracter);	
			}
	    }
	}

	// Cierro el FD
	fclose(archivo);


	if(list_size(listaInstrucciones) > 0){
		return true;
	}else{		
		return false;
	}
}

//*************************//
//Llenar el registro instruccion al parsear
//*************************//
void cargarInstruccion(Instruccion* registro, int codigo, char key[40], char* valor){

	registro->operacion= codigo;
	strcpy(registro->key, key);
	registro->key[strlen(key)] = '\0';

	if(valor != NULL){
		registro->dato=malloc(strlen(valor)+1);
		strcpy(registro->dato, valor);
		registro->dato[strlen(valor)] = '\0';

	} else {
		registro->dato=NULL;
	}
}

void mostrarInstruccion(Instruccion* registro){
	printf("%d ", registro->operacion);
	printf("%s ",registro->key);
	printf("%s \n",registro->dato);
}



//**************************************************************************//
// Obtener la Proxima Instruccion a Ejecutar de un ESI
//**************************************************************************//
Instruccion* obtenerSiguienteInstruccion(t_list* listaInstrucciones){

	Instruccion* instruccionAux = NULL;
	if(list_size(listaInstrucciones)>0){
		instruccionAux = list_get(listaInstrucciones,0);
	}
	return instruccionAux;
}

//**************************************************************************//
// Elimina la Ultima Instruccion Ejecutada de un ESI
//**************************************************************************//
void eliminarUltimaInstruccion(t_list* listaInstrucciones){

	Instruccion* instruccionAux = NULL;
	if(list_size(listaInstrucciones)>0){
		instruccionAux = list_remove(listaInstrucciones,0);
		
	}
	return;
}

//**************************************************************************//
// Obtener el tamano de la Proxima Instruccion a Ejecutar de un ESI
//**************************************************************************//
int obtenerTamanoProximaInstruccion(t_list* listaInstrucciones){

	return 10;
}

//********************************************************************//


// Dado un Socket de un Proceso, lo cargo en la Lista
void cargarProcesoLista(t_list* listaUtilizar, t_list* listaProcesar, int socketProcesoCargar){

	Proceso* registroProcesoAux2 = NULL;

	bool _find_socket_(Proceso* registroProcesoAux)
	{
		return (registroProcesoAux->socketProceso == socketProcesoCargar);
	}

	registroProcesoAux2 = list_find(listaUtilizar,(void*)_find_socket_);

	// Agrego el elemento en la Lista
	list_add(listaProcesar, registroProcesoAux2);
}


// Dado un Socket de un Proceso, lo elimino de la Lista
void eliminarProcesoLista(t_list* listaProcesar, int socketProcesoEliminar){

	if(list_size(listaProcesar) > 0){

		bool _find_socket_(Proceso* registroProcesoAux)
		{
			return (registroProcesoAux->socketProceso == socketProcesoEliminar);
		}

		list_remove_by_condition(listaProcesar,(void*)_find_socket_);
	
	}
	return;
}

// Dado un Socket de un Proceso, lo elimino de la Cola
void eliminarProcesoCola(t_queue* colaProcesar, int socketProcesoEliminar){

	Proceso* registroProcesoAux;
	int indice;

	if(queue_size(colaProcesar) > 0){

		for (indice = 0; indice  < queue_size(colaProcesar); indice=indice+1 ) {

			// Extraigo un elemento
			registroProcesoAux = queue_pop(colaProcesar);

			if(registroProcesoAux->socketProceso != socketProcesoEliminar){
				// Lo vuelvo a agregar a la cola
				queue_push(colaProcesar, registroProcesoAux);
			}
		}
	}

	return;
}


//**************************************************************************//
// Obtener un Registro Proceso segun su Socket
//**************************************************************************//
Proceso* obtenerRegistroProceso(t_list* listaProcesosConectados, int socketProcesoConsultar){

	Proceso* registroProcesoAux2 = NULL;

	bool _find_socket_(Proceso* registroProcesoAux)
	{
		return (registroProcesoAux->socketProceso == socketProcesoConsultar);
	}

	registroProcesoAux2 = list_find(listaProcesosConectados,(void*)_find_socket_);

	return registroProcesoAux2;
}


//**************************************************************************//
// Obtener el Proceso segun su Socket
//**************************************************************************//
char* obtenerNombreProceso(t_list* listaProcesosConectados, int socketProcesoConsultar){

	// Obtengo el nombre del Proceso segun su Socket
	char* nombreProceso = string_new();

	if(list_size(listaProcesosConectados) > 0){

	    void _each_elemento_(Proceso* registroProcesoAux)
		{
			if(registroProcesoAux->socketProceso == socketProcesoConsultar){
				string_append_with_format(&nombreProceso, "%s", registroProcesoAux->nombreProceso);			
			}
		}
	    list_iterate(listaProcesosConectados, (void*)_each_elemento_);
	}

    return nombreProceso;
}

//**************************************************************************//
// Obtener el Socket dado el Nombre del Proceso
//**************************************************************************//
int obtenerSocketProceso(t_list* listaProcesosConectados, char* nombreProcesoBuscado){

	int socketBuscado = 0;

	if(list_size(listaProcesosConectados) > 0){

	    void _each_elemento_(Proceso* registroProcesoAux)
		{
			if(registroProcesoAux->nombreProceso != NULL && strcmp(registroProcesoAux->nombreProceso, nombreProcesoBuscado) == 0){
				socketBuscado =	registroProcesoAux->socketProceso;
			}
		}
	    list_iterate(listaProcesosConectados, (void*)_each_elemento_);
	}

    return socketBuscado;
}

//**************************************************************************//
// Registrar las Instrucciones en el Log de Operaciones
//**************************************************************************//
void registrarLogOperaciones(t_list* listaProcesosConectados, int operacion, char key[40], char* dato, int socketProcesoConsultar){

	// Obtengo el nombre del Proceso segun su Socket
	char* nombreProceso = string_new();
	string_append_with_format(&nombreProceso, "%s", obtenerNombreProceso(listaProcesosConectados, socketProcesoConsultar));

	char* registroLogOperaciones = string_new();
	char* tipoOperacion = string_new();

	// Defino el archivo de Operaciones
    FILE *logOperaciones = fopen("log/operaciones.log", "a");

	if(logOperaciones == NULL){
		printf("No se pudo crear el Log de Operaciones.\n");
		fclose(logOperaciones); 
		return;
	}else{

		switch(operacion){
			case GET:
				string_append_with_format(&tipoOperacion, "GET");
				break;
			case SET:
				string_append_with_format(&tipoOperacion, "SET");
				break;
			case STORE:
				string_append_with_format(&tipoOperacion, "STORE");
				break;
			default:
				string_append_with_format(&tipoOperacion, "ERROR");
				break;			
		}
		string_append_with_format(&registroLogOperaciones, "%s %s %s %s\n", nombreProceso, tipoOperacion, key, dato);
		fwrite(registroLogOperaciones, strlen(registroLogOperaciones), sizeof(char), logOperaciones);
	}
	fclose(logOperaciones); 
	free(nombreProceso);
	free(tipoOperacion);

	return;
}


//**************************************************************************//
// Carga un Registro en una Cola (el Registro lo extraigo de una Lista) 
//**************************************************************************//
void cargarProcesoCola(t_list* listaUtilizar, t_queue* colaUtilizar, int socketProcesoConsultar){

	Proceso* registroProcesoAux2 = NULL;

	bool _find_socket_(Proceso* registroProcesoAux)
	{
		return (registroProcesoAux->socketProceso == socketProcesoConsultar);
	}

	registroProcesoAux2 = list_find(listaUtilizar,(void*)_find_socket_);

	// Agrego el elemento en la Cola
	queue_push(colaUtilizar, registroProcesoAux2);
}


//**************************************************************************//
// Obtener la Instancia Asignada a un Recurso
//**************************************************************************//
Instancia* obtenerInstanciaAsignada(t_dictionary * dictionario, char* key){ 

	Instancia* instancia = NULL;

	if(dictionary_size(dictionario) > 0){	
		instancia = dictionary_get(dictionario, key);
	}else{

	    // Creo Registro Nuevo pero VACIO
	    instancia = malloc(sizeof(Instancia));
	    instancia->nombreProceso = NULL;
	    instancia->socketProceso = 0;
	    instancia->entradasLibres = 0;
	}

	return instancia;
}

//**************************************************************************//
// El Coordinador procesa la instruccion y elige la Instancia que lo va a ejecutar
//**************************************************************************//
Instancia* obtenerInstanciaNueva(t_list* listaInstanciasConectadas, Instruccion* datosInstruccion, char* algoritmoDistribucion){
	int algoritmo;
	int cantidadElem,dividendo,resto;						//para algoritmo EK
	char key;
	Instancia* instanciaElegida = NULL;

	bool comparacion(Instancia* instancia1, Instancia* instancia2){
		if (instancia1->entradasLibres >= instancia2->entradasLibres){return true;}
		else {return false;}
	}			

	if(strcmp(algoritmoDistribucion,"LSU") == 0) {
		algoritmo=LSU;
	}else{
		if(algoritmoDistribucion[0]=='K'){
			algoritmo=KE;
		}
		else{
			if(algoritmoDistribucion[0]=='E'){
				algoritmo=EL;
			}else{
				puts("no se reconoce algoritmo");
			}
		}
	}

	switch(algoritmo){

		case EL:
			//saco la primer instancia de la lista
			instanciaElegida = list_remove(listaInstanciasConectadas, 0);
			//ahora la pongo al final de la lista
			list_add(listaInstanciasConectadas,instanciaElegida);			
			break;

		case LSU:

			list_sort(listaInstanciasConectadas, (void*)comparacion);
			instanciaElegida = list_remove(listaInstanciasConectadas,0);
			list_add(listaInstanciasConectadas,instanciaElegida);
			// hasta aca elige por espacio de sobra. en caso de que empaten, Tiene en Cuenta el orden original de la lista
			//Se mantiene la hipotesis que en algoritmo EL, cuando empatan se mantiene primero a la menos recientemente usada
			//por eso se quita y vuelve a agregar el elemento a la lista, para marcar que fue usado recientemente y quede despues de otros
			//en caso de empatar

			break;

		case KE:
			//calculo elementos de la lista
			cantidadElem= list_size(listaInstanciasConectadas);
			key= datosInstruccion->key[0];
			key= tolower(key);
			//divido cantidad de letras del abecedario por cantidad de instancias
			// sumo 1 porque si por ejemplo: tuviera abecedario de una sola letra, 'a'-'a' daria cero cuando 
			//enrealidad tengo una letra, asi que sumo uno
			dividendo= ('z'- 'a' + 1)/ cantidadElem;
			resto= ('z'-'a' + 1)%cantidadElem;
			if(resto!=0){dividendo = dividendo+1;}
			instanciaElegida = list_get(listaInstanciasConectadas, (key-'a')/dividendo);

			break;

		default:
			puts("default");
			log_error (infoLogger,"Algoritmo de distribución no reconocido");
	break;
	}


//	puts("\n\n\n");
//	showContenidolistaInstanciasConectadas(listaInstanciasConectadas);
//	puts("\n\n\n");

	return instanciaElegida;
}


//**************************************************************************//
// Mostrar el contenido de la Lista de Procesos Conectados
//**************************************************************************//
void showContenidolistaProcesosConectados(t_list* listaProcesosConectados){

	int indice = 0;

	if(list_size(listaProcesosConectados) > 0){

	    void _each_elemento_(Proceso* registroProcesoAux)
		{
			indice = indice + 1;

			// Muestro el encabezaado
			if(indice == 1) {
				printf("\nLISTA PROCESOS CONECTADOS\n");
				printf("Proceso         \t Socket\n");
				printf("-------------------------------------\n");
			}

			printf("%15s \t %d\n", registroProcesoAux->nombreProceso,registroProcesoAux->socketProceso);

		}
	    list_iterate(listaProcesosConectados, (void*)_each_elemento_);
	}else{
		printf("\nLista ProcesosConectados vacia\n");
	}
}

//**************************************************************************//
// Mostrar el contenido de la Lista de Instancias Conectados
//**************************************************************************//
void showContenidolistaInstanciasConectadas(t_list* listaInstanciasConectadas){

	int indice = 0;

	if(list_size(listaInstanciasConectadas) > 0){

	    void _each_elemento_(Instancia* registroProcesoAux)
		{
			indice = indice + 1;

			// Muestro el encabezaado
			if(indice == 1) {
				printf("\nLISTA INSTANCIAS CONECTADAS\n");
				printf("Proceso \t Socket\n");
				printf("------\n");

			}

			printf("%s\t %d\n", registroProcesoAux->nombreProceso,registroProcesoAux->socketProceso);

		}
	    list_iterate(listaInstanciasConectadas, (void*)_each_elemento_);
	}else{
		printf("\nLista InstanciasConectadas vacia\n");
	}
}

//**************************************************************************//
// Devuelve el Proceso que quiere usar un Recurso
//**************************************************************************//
void listarRecursosBloqueados(t_list* listaClavesBloqueadasRequeridas, t_dictionary* diccionarioClavesBloqueadas, char* key){ 

	int indice = 0;

	// Si el Recurso esta bloqueado por Archivo de Configuracion
    if(dictionary_has_key(diccionarioClavesBloqueadas, key) ){


		KeyBloqueada* registroKeyBloqueada=NULL;
		registroKeyBloqueada = dictionary_get(diccionarioClavesBloqueadas,key);

		if(registroKeyBloqueada->nombreProceso == NULL){
			printf("\nEl Recurso fue bloqueado por archivo de configuración\n");
			return;
		}				
    }

	// Si el Recurso esta bloqueado por otro Proceso
	if(list_size(listaClavesBloqueadasRequeridas) > 0){

	    void _each_elemento_(KeyBloqueada* registroKeyBloqueada)
		{
			indice = indice + 1;

			// Muestro el encabezaado
			if(indice == 1) {
				printf("\nPROCESOS\n");
				printf("------\n");

			}

			if(strcmp(registroKeyBloqueada->key, key) == 0) {
				printf("%s\n", registroKeyBloqueada->nombreProceso);
			}
		}
	    list_iterate(listaClavesBloqueadasRequeridas, (void*)_each_elemento_);

	}else{
		printf("\nEl Recurso %s no se encuentra bloqueado\n", key);
	}
}


//**************************************************************************//
// Mostrar el contenido de un Dirtionary
//**************************************************************************//
void showContenidoDiccionario(t_dictionary * dictionario, char* nombreDiccionario){ 

	int indice = 0;

	if(dictionary_size(dictionario) > 0){

	    void _each_elemento_(char* key, Proceso* registroProcesoAux)
		{
			indice = indice + 1;

			// Muestro el encabezaado
			if(indice == 1) {
				printf("\nDICCIONARIO %s\n", nombreDiccionario);
				printf("------\n");
			}

			printf("[%s]%s\n", key, registroProcesoAux->nombreProceso);

		}
	    dictionary_iterator(dictionario, (void*)_each_elemento_);
	}else{
		printf("\nDiccionario %s vacio\n", nombreDiccionario);
	}
}


//**************************************************************************//
// Mostrar el contenido de la Lista listaClavesBloqueadasRequeridas
//**************************************************************************//
void showContenidolistaClavesBloqueadasRequeridas(t_list* listaClavesBloqueadasRequeridas){ 

	int indice = 0;

	if(list_size(listaClavesBloqueadasRequeridas) > 0){

	    void _each_elemento_(KeyBloqueada* registroKeyBloqueada)
		{
			indice = indice + 1;

			// Muestro el encabezaado
			if(indice == 1) {
				printf("\nLISTA CLAVESBLOQUEADASREQUERIDAS\n");
				printf("------\n");
			}

			printf("[%s]%s\n", registroKeyBloqueada->key, registroKeyBloqueada->nombreProceso);

		}
	    list_iterate(listaClavesBloqueadasRequeridas, (void*)_each_elemento_);
	}else{
		printf("\nLista ClavesBloqueadasRequeridas vacia\n");
	}
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
				printf("------\n");
			}

			printf("%s\n", registroProcesoAux->nombreProceso);

		}
	    list_iterate(listaReady, (void*)_each_elemento_);
	}else{
		printf("\nLista Ready vacia\n");
	}
}

//**************************************************************************//
// Mostrar el contenido de una Cola de Planificacion
//**************************************************************************//
void showContenidoCola(t_queue* cola, char* nombreCola){

	int indice = 0;
	Proceso* registroProcesoAux = NULL;

	if(queue_size(cola) > 0){


		for (indice = 0; indice  < queue_size(cola); indice=indice+1 ) {

			// Obtengo un elemento
			registroProcesoAux = queue_pop(cola);


			// Muestro el encabezaado
			if(indice == 0) {
				printf("\nCOLA %s\n", nombreCola);
				printf("------\n");
			}

			printf("%s\n", registroProcesoAux->nombreProceso);


			// Lo vuelvo a agregar a la cola
			queue_push(cola, registroProcesoAux);
		}
	}else{
		printf("\nCola %s vacia\n", nombreCola);
	}
}

float estimarRafaga (float estimacionAnterior, int rafagaAnterior, int alfa){
		return alfa/100.0 * rafagaAnterior + (1 - alfa/100.0) * estimacionAnterior;
	}

KeyBloqueada* sacarProcesoConClaveBloqueadaDeLaLista(t_list* listaClavesBloqueadasRequeridas, char* key){

	KeyBloqueada* registroKeyBloqueada=NULL;

	bool buscarProcesoConClaveBloqueada(KeyBloqueada* registroKeyBloqueadaAux){
		return (!strcmp(key, registroKeyBloqueadaAux->key));
	}
	registroKeyBloqueada = list_remove_by_condition(listaClavesBloqueadasRequeridas, (void*)buscarProcesoConClaveBloqueada);
	return registroKeyBloqueada;
}

void liberarKeyBloqueada(KeyBloqueada* registroKeyBloqueadaAux){
	if(registroKeyBloqueadaAux->dato!=NULL) free(registroKeyBloqueadaAux->dato);
	free(registroKeyBloqueadaAux->nombreProceso);
	free(registroKeyBloqueadaAux);
}

KeyBloqueada* crearNodoDeUnaKeyBloqueada(KeyBloqueada keyBloqueada){

	KeyBloqueada* registroKeyBloqueada = malloc(sizeof(KeyBloqueada));

	registroKeyBloqueada->nombreProceso = malloc(strlen(keyBloqueada.nombreProceso)+1);
	strcpy(registroKeyBloqueada->nombreProceso, keyBloqueada.nombreProceso);
	registroKeyBloqueada->nombreProceso[strlen(keyBloqueada.nombreProceso)] = '\0';

	registroKeyBloqueada->operacion = keyBloqueada.operacion;

	strcpy(registroKeyBloqueada->key, keyBloqueada.key);
	registroKeyBloqueada->key[strlen(keyBloqueada.key)] = '\0';

	if(keyBloqueada.dato!=NULL){
		registroKeyBloqueada->dato = malloc(strlen(keyBloqueada.dato)+1);
		strcpy(registroKeyBloqueada->dato, keyBloqueada.dato);
		registroKeyBloqueada->dato[strlen(keyBloqueada.dato)] = '\0';
	}
	else registroKeyBloqueada->dato = NULL;

	return registroKeyBloqueada;

}

Proceso* obtenerProximoProcesoPlanificado(t_list* listaReady, t_queue* colaReady,t_dictionary* diccionarioRafagas, char* algoritmoPlanificacion, int alfa){

	Proceso* proximoProcesoPlanificado = NULL;
	//lista auxiliar de procesos con sus respectivas rafagas o tasas de respuesta
	t_list* listaProcesosConRafagas = list_create();
	bool elAlgoritmoEsHRRN = (string_starts_with(algoritmoPlanificacion,"HRRN"));


		//se usa con un list_iterate() de la listaReady
		//lo que hace es recibir cada Proceso y añadirlo junto con su rafaga que se obtiene del diccionario a una lista de tipo ProcesoConRafaga
		void cargarListaAuxiliar(Proceso* registroProcesoAux){

			Rafagas* registroRafagaAux;

			//obtengo la rafaga de un Proceso del diccionario
			registroRafagaAux = dictionary_get(diccionarioRafagas,registroProcesoAux->nombreProceso);

			//cargo el nombre del proceso y su rafaga (o tasa de resp.) una misma variable de tipo ProcesoConRafaga
			ProcesoConRafaga* registroProcesoConRafagaAux = malloc(sizeof(ProcesoConRafaga));

			registroProcesoConRafagaAux->nombreProceso = malloc(strlen(registroProcesoAux->nombreProceso)+1);
			strcpy(registroProcesoConRafagaAux->nombreProceso, registroProcesoAux->nombreProceso);
			registroProcesoConRafagaAux->nombreProceso[strlen(registroProcesoAux->nombreProceso)] = '\0';

			//si es SJF, almaceno la prox. rafaga, y si es HRRN calculo y almaceno la tasa de respuesta
			if(!elAlgoritmoEsHRRN) registroProcesoConRafagaAux->proximaEstimacion = registroRafagaAux->proximaEstimacion;
			else registroProcesoConRafagaAux->proximaEstimacion = (registroRafagaAux->proximaEstimacion + registroRafagaAux->tiempoDeEsperaDeCpu)/registroRafagaAux->proximaEstimacion;

			//agrego el proceso con su rafaga o tasa de resp. en caso de ser HRRN en una lista auxiliar
			list_add(listaProcesosConRafagas ,registroProcesoConRafagaAux);

		}

		//se usa para la funcion list_sort(), para que ordene una lista de ProcesosConRafagas de menor a mayor, por su estimacion
		bool ordenarPorRafagaMasCortaSJF(ProcesoConRafaga* registroProcesoConRafaga1, ProcesoConRafaga* registroProcesoConRafaga2){
			if (registroProcesoConRafaga1->proximaEstimacion < registroProcesoConRafaga2->proximaEstimacion) return true;
			else return false;
		}

		bool ordenarPorTasaDeReapuestaMasLargaHRRN(ProcesoConRafaga* registroProcesoConRafaga1, ProcesoConRafaga* registroProcesoConRafaga2){
			if (registroProcesoConRafaga1->proximaEstimacion > registroProcesoConRafaga2->proximaEstimacion) return true;
			else return false;
		}

		//se usa en un list_iterate() de ProcesosConRafagas, lo que hace es tomar el nombre de un ProcesoConRafagas,
		//buscar ese nombre en la listaReady, y poner ese elemento tipo Proceso en la colaReady
		void ponerProcesoDeLaListaDeReadyEnLaCola(ProcesoConRafaga* registroProcesoConRafagaAux){
				//es para encontrar en la listaReady, el proceso que coincida con el ProcesoConRafaga. Para eso verifica si los dos tienen el mismo nombre
				bool buscarProcesoPorSuNombre(Proceso* registroProcesoAux){
					return (!strcmp(registroProcesoAux->nombreProceso, registroProcesoConRafagaAux->nombreProceso));
				}
				Proceso* registroProcesoAuxDeLaLista = list_find(listaReady, (void*)buscarProcesoPorSuNombre);
				queue_push(colaReady, registroProcesoAuxDeLaLista);
		}

		void liberarProcesoConRafaga(ProcesoConRafaga* registroProcesoConRafagaAux){
			free(registroProcesoConRafagaAux->nombreProceso);
			free(registroProcesoConRafagaAux);
		}


	// Si hay elementos en la ListaReady
	if(list_size(listaReady) > 0){

		//limpio la cola de ready para luego llenarla con los procesos en el orden que lo establezcan los algoritmos
		if (queue_size(colaReady) >0) queue_clean(colaReady);
		//showContenidolistaReady(listaReady);

		// Ordeno la ColaReady segun el Algoritmo de Planificacion
		if(string_starts_with(algoritmoPlanificacion,"SJF-SD") || string_starts_with(algoritmoPlanificacion,"SJF-CD")){

			//actualiza el diccionario con las nuevas estimaciones y carga una lista auxiliar (listaProcesosConRafagas) con todos los procesos y sus estimaciones
			list_iterate(listaReady, (void*)cargarListaAuxiliar);

			//ordena la lista auxiliar por procesos con rafaga mas corta, osea, de menor a mayor
			list_sort(listaProcesosConRafagas, (void*)ordenarPorRafagaMasCortaSJF);

			//carga los procesos en la colaReady
			list_iterate(listaProcesosConRafagas, (void*)ponerProcesoDeLaListaDeReadyEnLaCola);

		}

		if(string_starts_with(algoritmoPlanificacion,"HRRN")){

			//actualiza el diccionario con las nuevas estimaciones y carga una lista auxiliar (listaProcesosConRafagas) con todos los procesos y sus estimaciones
			list_iterate(listaReady, (void*)cargarListaAuxiliar);

			//ordena la lista auxiliar por procesos con rafaga mas corta, osea, de menor a mayor
			list_sort(listaProcesosConRafagas, (void*)ordenarPorTasaDeReapuestaMasLargaHRRN);

			//carga los procesos en la colaReady
			list_iterate(listaProcesosConRafagas, (void*)ponerProcesoDeLaListaDeReadyEnLaCola);

		}

		// Extraigo un elemento de la ColaReady
		proximoProcesoPlanificado = queue_pop(colaReady);

		void mostrarListaDeProcesosConRafagas(ProcesoConRafaga* registroProcesoConRafagaAux){
			printf("%s \t valor por el cual se ordeno: %f\n", registroProcesoConRafagaAux->nombreProceso, registroProcesoConRafagaAux->proximaEstimacion);
		}
		//muestro la lista de procesos y los valores sobre los cuales se planificaron
		//printf("\nLista de Procesos ordenados segun su criterio (por rafaga o tasa de resp.): \n----------------\n");
		//list_iterate(listaProcesosConRafagas, (void*)mostrarListaDeProcesosConRafagas);

		list_clean_and_destroy_elements(listaProcesosConRafagas, (void*)liberarProcesoConRafaga);
		list_destroy(listaProcesosConRafagas);



		return proximoProcesoPlanificado;
	}else{
		return NULL;
	}
}

//Funcion para determinar si un archivo local existe
bool existeArchivo(char *filename){
	puts("quiero abrir");
    FILE *archivo = fopen(filename, "rb");
	puts("abrí");
    if(!archivo){
		puts("falso");
    	return false;
    }else{
		puts("true");
        fclose(archivo);
		puts("cerré");
    	return true;
    }
}

/* ------------------------------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------------------------------- */


// Cargo la Lista de Procesos Conectados en el Coordinador
void cargarListaProcesosConectados(t_list *listaProcesosConectados, Proceso* nuevoProceso){
	list_add(listaProcesosConectados, nuevoProceso);
}

// Cargo la Lista de Instancias Conectadas en el Coordinador
void cargarListaInstanciasConectadas(t_list *listaInstanciasConectadas, Proceso* nuevoProceso){

	Instancia* instancia= malloc(sizeof(Instancia));
	instancia->nombreProceso= malloc(strlen(nuevoProceso->nombreProceso)+1);
	strcpy(instancia->nombreProceso,nuevoProceso->nombreProceso);
	instancia->nombreProceso[strlen(nuevoProceso->nombreProceso)] = '\0';

	instancia->socketProceso=nuevoProceso->socketProceso;
	instancia->entradasLibres=1;

    list_add(listaInstanciasConectadas, instancia);
}

// Devuelve la cantidad de parametros que contiene un ingreso por consola
int countParametrosConsola(char * string){ 
	int cant_elementos = 0, i;

	for(i=0; i<=strlen(string); i=i+1){
		if(string[i] == ' ' ){
			cant_elementos = cant_elementos + 1;
		}
	}

	return cant_elementos;

}

// Libera Todos los Recursos de un determinado Proceso
void liberarRecursosProceso(t_dictionary * dictionario, char* nombreProceso){

	if(dictionary_size(dictionario) > 0){

		void elemento_destroy(Proceso* self){
			//free(self); // Explota!!!
		}

	    void _each_elemento_(char* key, Proceso* registroProcesoAux)
		{
			if(registroProcesoAux->nombreProceso != NULL && strcmp(registroProcesoAux->nombreProceso, nombreProceso) == 0) {
				dictionary_remove_and_destroy(dictionario, key, (void*) elemento_destroy);
			}
		}
	    dictionary_iterator(dictionario, (void*)_each_elemento_);
	}

}

// Cargo las Claves bloqueadas por archivo de configuracion en el Diccionario de Claves Bloqueadas
int cargarClavesInicialmenteBloqueadas(t_dictionary* diccionarioClavesBloqueadas, char** arregloClavesInicialmenteBloqueadas){

	int indice = 0;
	while(arregloClavesInicialmenteBloqueadas[indice] != NULL){

        Proceso* registroKeyProcesoAux = NULL;
        registroKeyProcesoAux = malloc(sizeof(Proceso));

        // Cargo el Registro
        registroKeyProcesoAux->socketProceso = 0;
        registroKeyProcesoAux->tipoProceso = ESI;
        registroKeyProcesoAux->nombreProceso = NULL;
	    
	    // Bloqueo el Recurso y lo cargo en la Lista de Claves Bloqueadas
	    dictionary_put(diccionarioClavesBloqueadas, arregloClavesInicialmenteBloqueadas[indice], registroKeyProcesoAux);

	    indice++;
	}

	return indice;
}

/******************INSTANCIA********************************************/


/******************INSTANCIA********************************************/
int espacioLibre(Almacenamiento almacenamiento){
	puts("ESPACIO LIBRE");
	FILE* vectorBin= fopen(almacenamiento.vector,"r");
	printf("\n QUIERO ABRIR EL VECTOR %s \n",almacenamiento.vector);
	fseek(vectorBin,0,SEEK_SET);
	char valor;
	int contador=0;
	int i;
	for(i=0;i<almacenamiento.cantidadEntradas;i++){
		fread(&valor, sizeof(char),1,vectorBin);
		puts("dentro de while");
		printf("\n EL VALOR ES %c",valor);
		if(valor=='0'){contador += 1 ;}
	}
	fclose(vectorBin);
	puts("SALGO ESPACIOlIBRE");
	return contador;
}
void liberarUnEspacio(Almacenamiento almacenamiento, int* puntero){
	puts("LIBRERAR UN ESPACIO");
	
	bool esAtomica(t_entrada* entradaAux){
		return	(entradaAux->numeroDeEntrada==*puntero 
				&& entradaAux->tamanioValorAlmacenado <= almacenamiento.tamPorEntrada);
	}

	t_entrada* reemplazada=NULL;
	while(reemplazada==NULL){
		reemplazada = list_find(almacenamiento.tablaEntradas,(void*)esAtomica);
		incrementarPuntero(almacenamiento,puntero);
	}
	liberarEntradaEnVector(almacenamiento,reemplazada);
	
	puts("LIBERE UN ESPACIO");
}

void incrementarPuntero(Almacenamiento almacenamiento,int* puntero) {
	if(*puntero == (almacenamiento.cantidadEntradas-1) ){
		*puntero=0;
	}
	else{*puntero= *puntero+1;
	}
}

bool esEntradaAtomica(Almacenamiento almacenamiento, t_entrada* entrada){
	return entrada->tamanioValorAlmacenado <= almacenamiento.tamPorEntrada;
}

void destruirEntradaEnPosicion(Almacenamiento almacenamiento, int posicion){
	t_entrada* destruida= list_remove(almacenamiento.tablaEntradas,posicion);
	liberarPosicionEnVector(almacenamiento,destruida->numeroDeEntrada);
	free(destruida->clave);
	free(destruida);
}

int persistirDatos(Almacenamiento almacenamiento,Instruccion* datosInstruccion, char* algoritmoDistribucion, int* puntero){
	char*valor=datosInstruccion->dato;
printf("\n\n ENTRO A PERSISTIR DATOS\n\n");
	
	int tamanio= string_length(datosInstruccion->dato);
	int i=0;
	int j=0;
	char letra;
	tamanio= tamanio/almacenamiento.tamPorEntrada +1; //calculo cuantas entradas necesita el valor nuevo
	if(tamanio % almacenamiento.tamPorEntrada==0){tamanio -= 1;}
	int espaciosLibres= espacioLibre(almacenamiento);
	t_entrada* entradaAux=NULL;

	bool porTamanio(t_entrada* entrada1, t_entrada* entrada2){
		puts("PORTAMANIO");
		return entrada1->tamanioValorAlmacenado > entrada2->tamanioValorAlmacenado;
	}


puts("IF INBICIAL");
	if(espaciosLibres<tamanio){//aplico algoritmo en caso de tener que sacar un valor del bin
		if(string_starts_with(algoritmoDistribucion, "CIRCULAR")){
				puts("CIRCULAR)");
				
				while(espaciosLibres<tamanio){//aplico algoritmo hasta tener espacio suficiente
				puts("QUIERO LIBERER");
					liberarUnEspacio(almacenamiento, puntero);
					puts("LIBERO");
					espaciosLibres=espacioLibre(almacenamiento);
					puts("CALCULO ESPACIO LIBRE");
				}



		}	else {if(string_starts_with(algoritmoDistribucion, "BSU")){
			puts("BSU");
					list_sort(almacenamiento.tablaEntradas,(void*)porTamanio);
					while(espaciosLibres<tamanio || i< almacenamiento.cantidadEntradas){//aplico algoritmo hasta tener espacio suficiente
						entradaAux=list_get(almacenamiento.tablaEntradas,i);
						if(esEntradaAtomica(almacenamiento,entradaAux)){
							destruirEntradaEnPosicion(almacenamiento,i);
						}
						else{i++;}
						espaciosLibres=espacioLibre(almacenamiento);
					}



		} else {if (string_starts_with(algoritmoDistribucion, "LRU")){
			puts("LRU");
				i=0;
				while(espaciosLibres<tamanio || i<almacenamiento.cantidadEntradas){								//aplico algoritmo hasta tener espacio suficiente
					entradaAux=list_get(almacenamiento.tablaEntradas,i); //saca valores atomicos en el orden de la lista
					if(esEntradaAtomica(almacenamiento, entradaAux)){					//estoy suponiendo que la lista esta ordenada de menos usado a mas recientemente usado, al igual que en el algoritmo de distribucion
						destruirEntradaEnPosicion(almacenamiento,i);	// y tambien asumo que siempre habrá un valor atómico para sacar. Leí en el issue  1097 que no sucederá ese caso
						
					}
					else{i++;}
					espaciosLibres=espacioLibre(almacenamiento);
				}




		}	else {printf("\n no se pudo leer el algoritmo de reemplazo %s \n", algoritmoDistribucion);
		puts("RETORNO -2");
				return -2;}}}
	}		// Dependiendo el algoritmoDistricucion, persistir los datos localmente
			//en este caso tengo espacio suficiente sin reemplazar, analizo si compactar o simplemente guardar
	if(espaciosLibres<tamanio){
		puts("NO HAY ESPACIO");
		log_info(infoLogger,"no se pueden borrar datos atomicos en la instancia para guardar nuevos datos");
		printf("SALGO RETORNANDO %d",i);
		return -1; }//retorno -1 si no se puede encontrar espacio suficiente para guardar el dato
	FILE* vectorBin= fopen(almacenamiento.vector,"r+");
	
	for(i=0;i<almacenamiento.cantidadEntradas;i++){//busco primer 0 en vectorBin
	puts("ENTRO AL FOR");
		fseek(vectorBin,i,SEEK_SET);
		fread(&letra,sizeof(char),1,vectorBin);
		if(letra=='0'){
			j=1;
			while(letra=='0' && j<tamanio){//cuento cuantas posiciones libres consecutivas hay hasta conseguir la cantidad necesaria
				
				fread(&letra,sizeof(char),1,vectorBin);
				if(letra=='0'){j++;}

			}
			fclose(vectorBin);
			if(j==tamanio){
				escribirBinarioEnPosicion(almacenamiento,i,valor);//retorno i que es la posicion donde se guarda el comienzo del valor
				for(j=0;j<tamanio;j++){
					grabarPosicionEnVector(almacenamiento,i+j);
				}
				printf("SALGO RETORNANDO %d",i);
				return i;
			}
			vectorBin= fopen(almacenamiento.vector,"r+");
		}
	}
	fclose(vectorBin);
	realizarCompactacionLocal(almacenamiento);
	vectorBin= fopen(almacenamiento.vector,"r+");
	fseek(vectorBin,0,SEEK_SET);
	i=0;
	fread(&letra,sizeof(char),1,vectorBin);
	while(letra!='0'){
		i++;
		fread(&letra,sizeof(char),1,vectorBin);
	}
	escribirBinarioEnPosicion(almacenamiento,i,valor);
	fclose(vectorBin);
	for(j=0;j<tamanio;j++){
		grabarPosicionEnVector(almacenamiento,i+j);
	}
	printf("SALGO RETORNANDO %d",i);
	return i;
}

bool enviarInstruccionInstancia(Instruccion registroInstruccion, int socketInstancia){

	// Armo el Paquete de Ejecucion de la Proxima Instruccion
	Paquete paquete = srlz_instruccion('C', EJECUTAR_INSTRUCCION,registroInstruccion);

	// Envio el Paquetea a la Instancia
	if(send(socketInstancia,paquete.buffer,paquete.tam_buffer,0) != -1){

	    free(paquete.buffer);
	    return true;
	    log_info(infoLogger, "Se le envio al COORDINADOR la proxima Instruccion a ejecutar");
	}else{
		return false;
	    log_error(infoLogger, "No se pudo enviar al COORDINADOR la proxima Instruccion a ejecutar");
	}
}

// Cuando se inicia una Instancia, se precarga la Tabla de Entradas con la info del Dump
void preCargarTablaEntradas(char* puntoMontaje, Almacenamiento almacenamiento){

    // Con un puntero a DIR abro el directorio 
    DIR *dir;
    // en *ent habrá información sobre el archivo que se está "sacando" a cada momento 
    struct dirent *ent;
	puts("abro dir");
    /* Empezaremos a leer en el directorio entradas */
    dir = opendir(puntoMontaje);
	puts("entro if");
    // Miramos que no haya error 
    if (dir == NULL){
        printf("No se puede abrir el directorio\n" );
    	return;
    }


    // Una vez nos aseguramos de que no hay error... 
    // Leyendo uno a uno todos los archivos que hay 
	puts("entra while");
    while ((ent = readdir (dir)) != NULL)
    {
        // Nos devolverá el directorio actual (.) y el anterior (..), como hace ls //
        if ( (strcmp(ent->d_name, ".")!=0) && (strcmp(ent->d_name, "..")!=0) )
        {
            char* nombreArchivoProcesar = string_new();
            string_append_with_format(&nombreArchivoProcesar, "%s", ent->d_name);
			puts("procesoArchivo");
            // Una vez tenemos el archivo, lo pasamos a una función para procesarlo. //
            procesoArchivo(nombreArchivoProcesar, config_get_string_value(cfg,"PUNTO_MONTAJE"), almacenamiento);  

            free(nombreArchivoProcesar);
        }
    }
    closedir (dir);
}


//funcion para carga de entradas
void cargarTablaEntradas(t_list *tablaEntradas,Instruccion* estructuraInstruccion){
	t_entrada* nuevaEntrada = NULL;

	int tamanioLista = list_size(tablaEntradas);

	nuevaEntrada=malloc(sizeof(t_entrada));


	nuevaEntrada->clave = estructuraInstruccion->key;

	nuevaEntrada->numeroDeEntrada = 999; //para luego verificar si se carga bien el nmEntrada. quedara 999 si se hizo mal
	nuevaEntrada->tamanioValorAlmacenado = strlen(estructuraInstruccion->dato);

	list_add(tablaEntradas,nuevaEntrada);
}





void procesoArchivo(char *archivo, char* punto_montaje, Almacenamiento almacenamiento){

	t_list* tablaEntradas=almacenamiento.tablaEntradas;
	char *carpeta_archivo = string_new();
	string_append_with_format(&carpeta_archivo, "%s%s", punto_montaje, archivo); // para que lea ficheros de la carpeta "entradas"

	// aca se guarda el contenido de cada fichero es decir el valor almacenado en cada key
	char* contenido_fichero = string_new(); 

	//obtengo la key sacando ultimos 4 caracteres ya que es el nombre sin el formato ".txt"
	char* nombre_archivo_sin_extension = string_new();
	nombre_archivo_sin_extension = string_substring(archivo,0,(string_length(archivo)-4)); 


	t_entrada* elementoDeTabla;

	// Obtengo el Tamano del Valor
	FILE *fichero;
	fichero = fopen(carpeta_archivo, "r");
	fseek(fichero,0,SEEK_END);
	int tamanio= ftell(fichero) + 1;
	fseek(fichero,0,SEEK_SET);


	//printf( "Fichero: %s -> ", archivo );
	if( fichero ){
       //printf( "existe (ABIERTO)\n" );
	   }
    else{
       printf( "Error (NO ABIERTO)\n" );
    }
	fgets(contenido_fichero,tamanio,fichero);

	// Cierro el Archivo
	fclose(fichero);

    //printf( "Contenido/clave-key con formato del fichero: %s\n\n", archivo ); //muestro key con formato ".txt"
    //printf( "Valor: %s\n", contenido_fichero ); 
    printf ("Clave/key: %s\n\n",nombre_archivo_sin_extension); //muestro key sin formato ".txt"

		/*
    	//carga de valor/dato
    	nuevaInstruccion->dato = malloc(strlen(contenido_fichero)+1);
    	strcpy(nuevaInstruccion->dato, contenido_fichero);
    	nuevaInstruccion->dato[strlen(contenido_fichero)] = '\0';
    	//carga de clave/key
    	strcpy(nuevaInstruccion->key,nombre_archivo_sin_extension);
    	cargarTablaEntradas(tablaEntradas,nuevaInstruccion); //cargo la tabla de entradas con esta nueva instruccion
    		elementoDeTabla = list_get(tablaEntradas,list_size(tablaEntradas)-1);
    		//muestro entrada "elementoDeTabla" para testear que esté todo correcto
    	  	printf("Clave:%s - Valor:%s - Numero:%d - Tamanio:%d - Posicion en tabla:%d \n",elementoDeTabla->clave,elementoDeTabla->valor,elementoDeTabla->numeroDeEntrada,elementoDeTabla->tamanioValorAlmacenado,list_size(tablaEntradas)); //prueba imprimir por pantalla el elemento obtenido
		*/
//------------------------------------------------------------------------orueba con entrada//

	t_entrada* nuevaEntrada = NULL;
	nuevaEntrada = malloc(sizeof(t_entrada)); //prueba con entrada  FUNCIONA

 	nuevaEntrada->clave = malloc(strlen(nombre_archivo_sin_extension)+1);
	strcpy(nuevaEntrada->clave, nombre_archivo_sin_extension);
	nuevaEntrada->clave[strlen(nombre_archivo_sin_extension)] = '\0';
	nuevaEntrada->tamanioValorAlmacenado = strlen(contenido_fichero);
	nuevaEntrada->numeroDeEntrada = buscarPosicionEnBin(almacenamiento,contenido_fichero);

	// Si se pudo obtener el Numero de Entrada del Binario
	if(nuevaEntrada->numeroDeEntrada>=0){
		grabarEntradaEnVector(almacenamiento,nuevaEntrada->numeroDeEntrada,nuevaEntrada);
	}

	list_add(tablaEntradas,nuevaEntrada);
	//muestro entrada "elementoDeTabla" para testear que esté todo correcto
    
	elementoDeTabla = list_get(tablaEntradas,list_size(tablaEntradas)-1);

	printf("\nClave:%s - Numero:%d - Tamanio:%d - Posicion en tabla:%d \n",elementoDeTabla->clave,elementoDeTabla->numeroDeEntrada,elementoDeTabla->tamanioValorAlmacenado,list_size(tablaEntradas)); //prueba imprimir por pantalla el elemento obtenido

	// Actualizo el Almacenamiento con la Nueva Entrada
	almacenamiento.tablaEntradas=tablaEntradas;
// ------------------------------------------------------------------

    free(carpeta_archivo);
    free(nombre_archivo_sin_extension);

    list_destroy(tablaEntradas);
/*    
    free(nuevaEntrada->clave);
    free(nuevaEntrada);
*/
}


void dump(t_list* tablaEntradas){
	list_iterate(tablaEntradas,(void*)persistirEntrada);
}

// persistir una entrada en disco
void persistirEntrada(t_entrada* unaEntrada){

    // Defino el Nombre del Archivo con el nombre de la entrada
	char *nombre_formato_archivo = string_new();
    string_append_with_format(&nombre_formato_archivo, "entradas/%s.txt", unaEntrada->clave);

    FILE* archivoTexto;
	archivoTexto = fopen(nombre_formato_archivo,"w+");

	//char *valorIdentificado = unaEntrada->valor;

	// TODO
	// Aca hay que obtener el VALOR de la KEY del archivo BINARIO ya que no existe mas en la Tabla de Estados
	char *valorIdentificado = string_new();
	string_append_with_format(&valorIdentificado, "EJEMPLO");

    // Grabo el Valor de la Entrada en el Archivo
	fputs(valorIdentificado, archivoTexto );

	fclose ( archivoTexto );
    free(nombre_formato_archivo);
    free(valorIdentificado);
}

void error(const char *s)
{
  /* perror() devuelve la cadena S y el error (en cadena de caracteres) que tenga errno */
  perror (s);
  exit(EXIT_FAILURE);
}




// Dado un Path, crea todos los directorios del mismo
bool crearEstructuraDirectorios(char* pathArchivo){

	int cantDirectorios, indice,indice2;
	char** arregloDirectorios = NULL;
	//char* pathDestinoCompleto;
	struct stat st = {0};


	// Separo el pathArchivo en todos los subdirectorios posibles
	arregloDirectorios = string_split(pathArchivo, "/");
	cantDirectorios = cantidadDirectoriosPath(pathArchivo);

	// Si el path comienza con /
	if(strcmp(string_substring(pathArchivo, 0, 1),"/") == 0){
		cantDirectorios = cantDirectorios - 1;
	}

	// Recorro cada nodo del path
	for (indice = 0; indice  < (cantDirectorios); indice=indice+1 ) {

		char* pathDestinoCompleto = string_new();

		for (indice2 = 0; indice2  <= indice; indice2=indice2+1 ) {
			if(indice2 != indice){
				string_append_with_format(&pathDestinoCompleto, "%s/", arregloDirectorios[indice2]);
			}else{
				string_append_with_format(&pathDestinoCompleto, "%s", arregloDirectorios[indice2]);
			}
		}

		// Si no existe el directorio, lo creo
		if (stat(pathDestinoCompleto, &st) == -1) {

		
			if(mkdir(pathDestinoCompleto, 0777) == -1){ // Si hubo error		
				return false;
			}
		}

		free(pathDestinoCompleto);
	}

	free(arregloDirectorios);
	return true;
}

//Devuelve la cantidad de directorios que hay en un path
int cantidadDirectoriosPath(char* pathDirectorio){
	char** arregloDirectorio1 = string_split(pathDirectorio, "/");
	int contadorDirectorios = 0;


	void _each_directory_(char* directorio)
	{
		contadorDirectorios = contadorDirectorios + 1;
	}

	string_iterate_lines(arregloDirectorio1, (void*)_each_directory_);

	free(arregloDirectorio1);

	return contadorDirectorios;
}

char* leerBinarioEnPosicion(Almacenamiento almacenamiento, int posicion){
	FILE* binario= fopen(almacenamiento.binario,"rb");
	int tamEntrada= almacenamiento.tamPorEntrada;
	char* buffer = string_new();
	char letra='z';
	fseek(binario,tamEntrada*posicion,SEEK_SET);
	int contador=0;
	//debuggeanding
	while( letra !='\0'){
		fread(&letra,sizeof(char),1,binario);
		buffer[contador]=letra;
		contador +=1;
	} 
	fclose(binario);
	return buffer;
}

void escribirBinarioEnPosicion(Almacenamiento almacenamiento, int posicion, char* valor){
	FILE* binario= fopen(almacenamiento.binario,"r+");
	int tamEntrada= almacenamiento.tamPorEntrada;
	fseek(binario, tamEntrada*posicion,SEEK_SET);
	int tamanio= string_length(valor);
	fwrite(valor,tamanio,1,binario);
	char fin='\0';
	fwrite(&fin,sizeof(char),1,binario);
	fclose(binario);
}

void grabarPosicionEnVector(Almacenamiento almacenamiento, int posicion){
	FILE* vectorBin=fopen(almacenamiento.vector,"r+");
	fseek(vectorBin,posicion,SEEK_SET);
	int uno ='1';
	fwrite(&uno,1,sizeof(char),vectorBin);
	fclose(vectorBin);
}

void liberarPosicionEnVector(Almacenamiento almacenamiento, int posicion){
	FILE* vectorBin=fopen(almacenamiento.vector,"r+");
	fseek(vectorBin,posicion,SEEK_SET);
	int cero ='0';
	fwrite(&cero,1,sizeof(char),vectorBin);
	fclose(vectorBin);
}

void grabarEntradaEnVector(Almacenamiento almacenamiento, int posicion, t_entrada* entrada){
	int espaciosOcupados= entrada->tamanioValorAlmacenado/almacenamiento.tamPorEntrada;
	int i;
	for(i=0;i<=espaciosOcupados;i+=1){
		grabarPosicionEnVector(almacenamiento,posicion + i);
	}
}

void liberarEntradaEnVector(Almacenamiento almacenamiento, t_entrada* entrada){
	int espaciosOcupados= entrada->tamanioValorAlmacenado/almacenamiento.tamPorEntrada;
	int posicion = entrada->numeroDeEntrada; 
	if(entrada->tamanioValorAlmacenado%almacenamiento.tamPorEntrada != 0) {
		espaciosOcupados += 1;}
	int i;
	for(i=0;i<=espaciosOcupados;i+=1){
		liberarPosicionEnVector(almacenamiento,posicion + i);
	}
}




char* valorEntrada(t_entrada* entrada){
	char* valor = string_new();
	char* archivo= string_new();
	string_append_with_format(&archivo, "%s%s.txt"   ,config_get_string_value(cfg,"PUNTO_MONTAJE"),entrada->clave );
	FILE *fichero;
	fichero = fopen(archivo, "r");
	fseek(fichero,0,SEEK_END);
	int tamanio= ftell(fichero) ;
	fseek(fichero,0,SEEK_SET);
	printf("\n sobre el string %s ",valor);
	fread(valor,sizeof(char)*tamanio,1,fichero);
	printf("\nleí el valor    %s \n", valor);
	return valor;



}

//DEVUELVE LA POSICION DEL BINARIO EN LA QUE SE ENCUENTRA EL VALOR BUSCADO. 
//De no encontrarse retorna -1
//De haber error, retorna -2
int buscarPosicionEnBin(Almacenamiento almacenamiento, char* valor){
	FILE* binario= fopen(almacenamiento.binario,"rb");
	int espacioPorEntrada= almacenamiento.tamPorEntrada;
	fseek(binario,0,SEEK_END);
	int tamanio= ftell(binario);
	//printf("\n tamanio archivo es %d          espacio por entrada es  %d 		cantidad de entradas es %d\n",tamanio, espacioPorEntrada, almacenamiento.cantidadEntradas);
	int entradas= tamanio/espacioPorEntrada;

	if(tamanio%espacioPorEntrada != 0){
		printf("\n error al calcular cant.entradas\n");
		fclose(binario);
		return -2;
	}else{
		//printf("\n se calculo bien cant.entradas igual a %d\n",entradas);
	}
	
	int posicion=0;
	char*buffer= string_new();
	int i=0;
	printf("\nse busca %s\n",valor);
	while(i<entradas){
		buffer= leerBinarioEnPosicion(almacenamiento ,i);
		//printf("\n se leyó %s\n",buffer);
		if(strcmp(buffer,valor) == 0){
			fclose(binario);
			return i;}	
		i +=1;

	}
	fclose(binario);
	return -1;
}
/* esto ya no serviría, lo guardo por si sirve la lógica
int buscarPosicionesEnBin(Almacenamiento almacenamietn, t_list* entradas, char* valorEntrada){
	int tamanio= list_size(entradas);
	t_entrada* entrada;
	int i=0;
	for(i=0;i<tamanio;i++){
		entrada = list_get(entradas,i);
		entrada->numeroDeEntrada = buscarPosicionEnBin(binario,espacioPorEntrada, valorEntrada);
	}
	bool se_Encontro_Todo(t_entrada* unaEntrada){
		return unaEntrada->numeroDeEntrada>=0;
	}
	return list_all_satisfy(entradas, (void*)se_Encontro_Todo);
} */
/******************INSTANCIA********************************************/

// Actualizo el Diccionario con la Nueva Instancia Asignada
void actualizarDiccionarioClavesInstancias(t_dictionary* dictionario, char key[40], Instancia* nuevaInstancia){

    void _each_elemento_(char* keyAux, Instancia* registroInstanciaAux)
	{
		// Actualizo el Registro
		if(strcmp(keyAux, key) == 0){

			registroInstanciaAux->socketProceso = nuevaInstancia->socketProceso;
			registroInstanciaAux->entradasLibres = nuevaInstancia->entradasLibres;

			registroInstanciaAux->nombreProceso=malloc(strlen(nuevaInstancia->nombreProceso)+1);
			strcpy(registroInstanciaAux->nombreProceso,nuevaInstancia->nombreProceso);
			registroInstanciaAux->nombreProceso[strlen(nuevaInstancia->nombreProceso)] = '\0';
		}

	}
    dictionary_iterator(dictionario, (void*)_each_elemento_);
}

// Realizar la Compactacion del Archivo Binario de una Instancia
//Devuelve True si se compacto y False si no se tuvo que compactar porque no era necesario
bool realizarCompactacionLocal(Almacenamiento almacenamiento){

	int tamEntrada = almacenamiento.tamPorEntrada;
	int cantEntradas = almacenamiento.cantidadEntradas;
	char contenidoVectortxt[cantEntradas];
	char nuevoContenidoVectortxt[cantEntradas+1];
	char letra='z';
	char letraAnterior = '1';
	char* valoresDelBinario[cantEntradas];
	bool compactar=false;
	//cuenta la cantidad de espacios ocupados por valores en el binario
	int contadorEspaciosOcupados = 0;
	//cuenta la cantidad de valores que hay almacenados en el binario. No es lo mismo que el espacio,
	//ya que un valor puede ocupar mas de un espacio
	int contadorValoresAlmacenados=0;

	FILE* vectorTxt = fopen(almacenamiento.vector,"r+");
	fseek(vectorTxt,0,SEEK_SET);

	//se lee el txt y se carga en un vector lo leido. si se detecta un '0' seguido de un '1' significa que hay que compactar
	for(int i=0; i<cantEntradas; i++){
		fread(&letra,sizeof(char),1,vectorTxt);
		contenidoVectortxt[i]=letra;
		if(letraAnterior=='0' && letra=='1') compactar=true;
		letraAnterior=letra;
	}

	if(compactar){
		//se recorre el vector del txt. Cuando se detecta un 1, se usa esa posicion para ir leyendo los valores del binario
		//y poniendolos en un vector
		for(int i=0; i<cantEntradas; i++){
			if(contenidoVectortxt[i]=='1'){
				valoresDelBinario[contadorValoresAlmacenados] = leerBinarioEnPosicion(almacenamiento,i);
				//cuando se lee un valor del binario, puede que este ocupe mas de una entrada(espacio)
				//por eso hago la cuenta que sigue, para cambiar la posicion siguiente de la cual leer en caso de que haya ocupado
				//mas de una entrada. La cuenta devuelve 0 si ocupa una entrada, 1 si ocupa 2, etc.
				i += string_length(valoresDelBinario[contadorValoresAlmacenados])/tamEntrada;
				contadorEspaciosOcupados += string_length(valoresDelBinario[contadorValoresAlmacenados])/tamEntrada;
				contadorEspaciosOcupados++;
				contadorValoresAlmacenados++;
			}
		}
		//se va armando el nuevo contenido del txt, que seran todos unos seguidos de ceros
		for(int i=0;i<contadorEspaciosOcupados;i++){
			nuevoContenidoVectortxt[i] = '1';
		}
		//se llenan los espacios restantes del txt con ceros
		for(int i=contadorEspaciosOcupados; i<cantEntradas; i++){
			nuevoContenidoVectortxt[i] = '0';
		}
		nuevoContenidoVectortxt[cantEntradas] = '\0';

		contadorValoresAlmacenados=0;
		//se escribe el binario con el vector donde se almacenaron los valores
		for(int i=0; i<contadorEspaciosOcupados; i++){
			escribirBinarioEnPosicion(almacenamiento,i,valoresDelBinario[contadorValoresAlmacenados]);
			//lo mismo que antes, si un valor que escribo ocupa mas de una entrada
			//se debe cambiar la siguiente posicion de escritura
			i += string_length(valoresDelBinario[contadorValoresAlmacenados])/tamEntrada;
			//se libera el valor leido
			free(valoresDelBinario[contadorValoresAlmacenados]);
			contadorValoresAlmacenados++;
		}
		//se escribe el nuevo contenido del txt
		fseek(vectorTxt,0,SEEK_SET);
		fwrite(nuevoContenidoVectortxt,sizeof(char),cantEntradas+1,vectorTxt);
	}
	fclose(vectorTxt);
	return compactar;
}

// no sirve lo guardo por si sirve la lógica
//void reemplazoValor(t_entrada* unaEntrada, char* valor, char key[40]){

//	if(strcmp(unaEntrada->clave,key) == 0){

// 	unaEntrada->clave = malloc(strlen(valor)+1);
//	strcpy(unaEntrada->clave, valor);
//	unaEntrada->clave[strlen(valor)] = '\0';

//	}

//}

//void actualizarTablaDeEntradas(t_list* tablaEntradas,t_entrada* unaEntrada, char* valor){

//	list_map(tablaEntradas,(void*)reemplazoValor);

//}


bool existeEntradaEnTabla(t_list* tablaEntradas, char key[40]){

	bool esIgualAKey(t_entrada* unaEntrada){
		if(strcmp(unaEntrada->clave,key) == 0){;
			printf(" Existe ");
			return true;
		}else{
			printf(" No Existe ");
			return false;
		}
	}

		if(list_any_satisfy(tablaEntradas, (void*)esIgualAKey)){
			printf(" EXISTE ");
			return true;
		}else{printf(" NO EXISTE ");
				return false;
		}
}



void ejecutarCadaXTiempo(void funcionAEjecutar(), int tiempo ){
	time_t tiempoAux, tiempoActual;
	   double diferencia = 0;
	   int minutos;
	   int segundos = 0;
	   int temp;

	   tiempoAux = time(NULL); // se obtiene tiempo auxiliar de comienzo

	   while (diferencia < tiempo ){
	      tiempoActual = time(NULL); //se obtiene tiempo actual
	      diferencia = difftime(tiempoActual, tiempoAux);
	      minutos = diferencia / 60.0;
	      segundos = diferencia - minutos * 60;

	      if (temp != segundos){ //transcurrió un segundo
	         temp = segundos; //guardo valor previo
	         //printf("%02d\n", segundos);
	      }

	      if(diferencia == tiempo){
	    	  funcionAEjecutar();
	    	  //printf("ejecuto");
	      }
	   }
}
