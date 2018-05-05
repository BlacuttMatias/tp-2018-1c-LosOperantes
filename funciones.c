#include "funciones.h"

//*********************** SERIALIZADO Y DESERIALIZADO ********************************//

Paquete srlz_datosProceso(char proceso, int codigoOperacion, char* nombreProceso, int tipoProceso, int socketProceso){

	int posicion = 0;//int para ir guiando desde donde se copia
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
	int posicion = 0; //int para ir guiando desde donde se copia
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
//Serealizar instrucciones en esi para enviar
//**************************************************************************//

Paquete srlz_datosInstruccion(char proceso, int codigoOperacion, Instruccion instruccion){
	int posicion;
	int sizeBuffer;
	int tamanioPuntero;
		if(instruccion.dato==NULL){
			tamanioPuntero=0;
		}
		else{tamanioPuntero=strlen(instruccion.dato);}

	Paquete paquete;
	//				tam puntero		cod.op			key					contenido data
	sizeBuffer= 	sizeof(int) + 	sizeof(int) + 	sizeof(char[40]) + tamanioPuntero;
	paquete.tam_buffer=sizeBuffer;
	memcpy(paquete.buffer								,&(tamanioPuntero)					,sizeof(int));
	memcpy(paquete.buffer + (posicion = sizeof(int))	,&(instruccion.operacion)			,sizeof(int));
	memcpy(paquete.buffer + (posicion +=sizeof(int))	,&(instruccion.key[0])				,(sizeof(char))*40);
	if(tamanioPuntero != 0){
	memcpy(paquete.buffer + (posicion += (sizeof(char))*40),instruccion.dato			,tamanioPuntero);
	}


	return paquete;

}

//**************************************************************************//
//Sacar Siguiente instruccion de la lista
//**************************************************************************//

//(todavia falta probar)
Instruccion* sacarSiguienteInstruccion(t_list* listaInstruccion) {
	t_list* listaAuxiliar;
	Instruccion* instruccionAux=NULL;
	if(list_size(listaInstruccion)>0){
		instruccionAux =	list_remove(listaInstruccion,0);


	}
	return instruccionAux;
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
	char *unaInstruccion = string_new();
	int numeroLinea=0;

	puts("abro archivo");
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
				//puts("alloco un registroInstruccion");
				registroInstruccion = malloc(sizeof(Instruccion));
				//puts("alloqué el registro");
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


        		// Cargo el Registro de la instruccion
        		//registroInstruccion->texto_instruccion = malloc(strlen(unaInstruccion)+1);
        		//strcpy( registroInstruccion->texto_instruccion ,unaInstruccion);
        		//registroInstruccion->texto_instruccion[strlen(unaInstruccion)] = '\0';

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


	// Muestro por pantalla el contenido de la Lista
	int indice = 0;

	if(list_size(listaInstrucciones) > 0){

	    void _each_elemento_(Instruccion* registroInstruccionAux)
		{
			indice = indice + 1;

			// Muestro el encabezaado
			if(indice == 1) {
				printf("\nLISTA DE INSTRUCCIONES\n");
				printf("--------------\n");
			}

			mostrarInstruccion(registroInstruccionAux);
			//printf("%s\n", registroInstruccionAux->texto_instruccion);

		}
	    list_iterate(listaInstrucciones, (void*)_each_elemento_);
	}else{		
		return false;
	}

	return true;
}

//*************************//
//Llenar el registro instruccion al parsear
//*************************//
void cargarInstruccion(Instruccion* registro, int codigo, char key[40], char* valor){
	//puts("cargando instruccion\n");
	registro->operacion= codigo;
	//puts("cargue operacion");
	strcpy(registro->key, key);
	//puts("cargue key");
	if(valor != NULL){
		registro->dato=malloc(strlen(valor)+1);
		strcpy(registro->dato, valor);}
	else {registro->dato=NULL;}
		//puts("cargue instruccion");
}

void mostrarInstruccion(Instruccion* registro){
	printf("%d ", registro->operacion);
	printf("%s ",registro->key);
	printf("%s \n",registro->dato);
}


//**************************************************************************//
// Obtener la Proxima Instruccion a Ejecutar de un ESI
//**************************************************************************//
char* obtenerProximaInstruccion(t_list* listaInstrucciones){

	char* proximaInstruccion = string_new();
	return proximaInstruccion;
}

//**************************************************************************//
// Obtener el tamano de la Proxima Instruccion a Ejecutar de un ESI
//**************************************************************************//
int obtenerTamanoProximaInstruccion(t_list* listaInstrucciones){

	return 10;
}

//********************************************************************//



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
				printf("Proceso\n");
				printf("------\n");
			}

			printf("%s\n", registroProcesoAux->nombreProceso);

		}
	    list_iterate(listaReady, (void*)_each_elemento_);
	}
}

//**************************************************************************//
// Mostrar el contenido de la Cola Ready
//**************************************************************************//
void showContenidocolaReady(t_queue* colaReady){ 

	int indice = 0;
	Proceso* registroProcesoAux = NULL;

	if(queue_size(colaReady) > 0){


		for (indice = 0; indice  < queue_size(colaReady); indice=indice+1 ) {

			// Obtengo un elemento
			registroProcesoAux = queue_pop(colaReady);


			// Muestro el encabezaado
			if(indice == 0) {
				printf("\nCOLA READY\n");
				printf("Proceso\n");
				printf("------\n");
			}

			printf("%s\n", registroProcesoAux->nombreProceso);


			// Lo vuelvo a agregar a la cola
			queue_push(colaReady, registroProcesoAux);
		}



	}
}


//**************************************************************************//
// Ordena la ColaReady y la devuelve segun el Planificador
//**************************************************************************//
t_queue* planificarReady(t_list* listaReady, char* algoritmoPlanificacion){

	t_queue* colaAux;
	colaAux = queue_create();


	if(string_starts_with(algoritmoPlanificacion,"SJF-SD")){
		// TODO
	}	

	if(string_starts_with(algoritmoPlanificacion,"SJF-CD")){
		// TODO
	}	


	if(string_starts_with(algoritmoPlanificacion,"HRRN")){

		// TODO

		if(list_size(listaReady) > 0){

		    void _each_elemento_(Proceso* registroProcesoAux)
			{

				// Agrego el Registro a la cola
				queue_push(colaAux, registroProcesoAux);
			}
		    list_iterate(listaReady, (void*)_each_elemento_);
		}


	}	

	return colaAux;
}

//Funcion para determinar si un archivo local existe
bool existeArchivo(char *filename){

    FILE *archivo = fopen(filename, "rb");

    if(!archivo){
    	return false;
    }else{
        fclose(archivo);
    	return true;
    }
}
