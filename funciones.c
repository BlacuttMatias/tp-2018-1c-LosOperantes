#include "funciones.h"

//*********************** SERIALIZADO Y DESERIALIZADO ********************************//

Paquete srlz_datosInstancia(char proceso, int codigoOperacion, char* nombreProceso, int entradasLibres, int socketProceso){

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

	memcpy(paquete.buffer + (posicion += strlen(nombreProceso))				,&(entradasLibres)				,sizeof(int));
	memcpy(paquete.buffer + (posicion += sizeof(int))						,&(socketProceso)				,sizeof(int));

	return paquete;
}

Instancia dsrlz_datosInstancia(void* buffer)
{
	int posicion = 0; //int para ir guiando desde donde se copia
	int tamString = 0;
	Instancia solicitud;


	memcpy(&(tamString)					 	,buffer+posicion										,sizeof(int));
	solicitud.nombreProceso = malloc(sizeof(char) * tamString+1);
	memcpy(solicitud.nombreProceso			,buffer+(posicion+=sizeof(int))							,sizeof(char)*tamString);
	solicitud.nombreProceso[tamString]='\0';

	memcpy(&solicitud.entradasLibres 			,buffer+(posicion+=sizeof(char) * tamString)			,sizeof(int));
	memcpy(&solicitud.socketProceso 			,buffer+(posicion+=sizeof(int))							,sizeof(int));

	return solicitud;
}

Paquete srlz_datosKeyBloqueada(char proceso, int codigoOperacion, char* nombreProceso, int operacion, char key[40], char* dato){

	int posicion = 0;//int para ir guiando desde donde se copia
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

	int posicion = 0; //int para ir guiando desde donde se copia
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
		memcpy(solicitud.dato					,buffer+(posicion+=sizeof(int))							,sizeof(char)*tamString);	
		solicitud.dato[tamString] = '\0';

	} else {
		solicitud.dato = NULL;
	}

	return solicitud;
}

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

Paquete srlz_instruccion (char proceso, int codigoOperacion,Instruccion instruccion){

	
	int posicion = 0;//int para ir guiando desde donde se copia
	int sizeBuffer = 0;
	int tamClave = 0;
	int tamPayload = 0;
	int tamDato = 0;
	Paquete paquete;

	tamClave = strlen(instruccion.key);

	//dependiendo de si se hace un get/store o un set, el tamaño del buffer sera uno u otro, porque en el set se agrega tambien el valor asociado a la key
	if(instruccion.operacion == GET || instruccion.operacion == STORE){
		sizeBuffer = sizeof(int)*4 + sizeof(char) + tamClave;
	}
	else{
		tamDato = strlen(instruccion.dato);
		sizeBuffer = sizeof(int)*5 + sizeof(char) + tamClave + tamDato;	//agrego el tamaño del valor y el valor(el dato de la instruccion)
	}
	paquete.tam_buffer = sizeBuffer;
	paquete.buffer = malloc( sizeBuffer );
	tamPayload = sizeBuffer - (sizeof(int)*2) - sizeof(char);


	memcpy(paquete.buffer									,&(proceso)                     ,sizeof(char));
	memcpy(paquete.buffer + (posicion=sizeof(char))			,&(codigoOperacion)				,sizeof(int));
	memcpy(paquete.buffer + (posicion += sizeof(int))		,&(tamPayload)					,sizeof(int));

	memcpy(paquete.buffer + (posicion+=sizeof(int))			,&(instruccion.operacion)		,sizeof(int));

	memcpy(paquete.buffer + (posicion += sizeof(int))		,&(tamClave)					,sizeof(int));
	memcpy(paquete.buffer + (posicion += sizeof(int))		,instruccion.key				,tamClave);

	if(instruccion.operacion==SET){	//en caso ser un set, pongo en el buffer el tamaño del dato y el dato, en get y store no hace falta porque el dato no existe
		memcpy(paquete.buffer + (posicion += tamClave)			,&(tamDato)						,sizeof(int));
		memcpy(paquete.buffer + (posicion += sizeof(int))		,instruccion.dato				,tamDato);
	}
	return paquete;

}


Instruccion dsrlz_instruccion (void* buffer){

	
	int posicion = 0; //int para ir guiando desde donde se copia
	int tamClave = 0;
	Instruccion instruccion;

	memcpy(&instruccion.operacion			,buffer + posicion										,sizeof(int));
	memcpy(&tamClave						,buffer + (posicion+=sizeof(int))						,sizeof(int));
	memcpy(instruccion.key					,buffer + (posicion+=sizeof(int))						,tamClave);
	instruccion.key[tamClave] = '\0';

	if(instruccion.operacion==SET){	//si es un set significa que tengo que seguir leyendo del buffer el valor asociado a la key(el dato)

		int tamDato = 0;
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
Instruccion pasarAEstructura(Instruccion* puntero) {
	Instruccion instruccion;
	instruccion.operacion= (puntero->operacion);
	strcpy(instruccion.key,puntero->key);
	instruccion.dato= puntero->dato;
	return instruccion;
}

//**************************************************************************//
// Persistir Datos en la Instancia
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
			if(strcmp(registroProcesoAux->nombreProceso, nombreProcesoBuscado) == 0){
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
// El Coordinador procesa la instruccion y elige la Instancia que lo va a ejecutar
//**************************************************************************//
Instancia* obtenerInstanciaAsignada(t_list* listaInstanciasConectadas, Instruccion* datosInstruccion, char* algoritmoDistribucion){
	int algoritmo=8;
	int ultimaPosicion;						//para algoritmo EL
	int cantidadElem,dividendo,resto;						//para algoritmo EK
	char key;
	Instancia* instanciaElegida = NULL;

	bool comparacion(Instancia* instancia1, Instancia* instancia2){
		if (instancia1->entradasLibres >= instancia2->entradasLibres){return true;}
		else {return false;}
	}
	if(!strcmp(algoritmoDistribucion,"LSU")){
		algoritmo=LSU;
	}	else{
		if(algoritmoDistribucion[0]=='K'){
			algoritmo=KE;
		}
		else{
			if(algoritmoDistribucion[0]=='E'){
			algoritmo=EL;}
			else{puts("no se reconoce algoritmo");}
			}}

	switch(algoritmo){

		case EL:
			puts("EL");
			//consigo la ultima posicion de la lista (asumo que los primeros elementos en la lista
			//fueron los mas recientemente usados. y al final los menos usados
			ultimaPosicion = list_size(listaInstanciasConectadas);
			ultimaPosicion = ultimaPosicion - 1;
			//saco la instancia en posicion ultima, que es la menos usada o mas recientemente agregada
			instanciaElegida = list_remove(listaInstanciasConectadas, ultimaPosicion);
			//ahora que se acaba de usar, la pongo primera en la lista
			list_add_in_index(listaInstanciasConectadas,0,instanciaElegida);

			break;

		case LSU:
			puts("LSU");
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
		puts("KE");
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
			log_error (infoLogger,"algoritmo de distribucion no reconocido");
	break;
	}


	puts("\n\n\n");
	showContenidolistaProcesosConectados(listaInstanciasConectadas);
	puts("\n\n\n");
// TODO

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
				printf("Proceso \t Tipo de Proceso \t Socket\n");
				printf("------\n");

			}

			printf("%s\t %d\t %d\n", registroProcesoAux->nombreProceso,registroProcesoAux->tipoProceso,registroProcesoAux->socketProceso);

		}
	    list_iterate(listaProcesosConectados, (void*)_each_elemento_);
	}else{
		printf("\nLista ProcesosConectados vacia\n");
	}
}

//**************************************************************************//
// Devuelve el Proceso que quiere usar un Recurso
//**************************************************************************//
void listarRecursosBloqueados(t_list* listaClavesBloqueadasRequeridas, char* key){ 

	int indice = 0;

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
				printf("------\n");
			}

			printf("%s\n", registroProcesoAux->nombreProceso);


			// Lo vuelvo a agregar a la cola
			queue_push(colaReady, registroProcesoAux);
		}
	}else{
		printf("\nCola Ready vacia\n");
	}
}

//**************************************************************************//
// Mostrar el contenido de la Cola Bloqueado
//**************************************************************************//
void showContenidocolaBloqueados(t_queue* colaBloqueados){ 

	int indice = 0;
	Proceso* registroProcesoAux = NULL;

	if(queue_size(colaBloqueados) > 0){


		for (indice = 0; indice  < queue_size(colaBloqueados); indice=indice+1 ) {

			// Obtengo un elemento
			registroProcesoAux = queue_pop(colaBloqueados);


			// Muestro el encabezaado
			if(indice == 0) {
				printf("\nCOLA BLOQUEADOS\n");
				printf("------\n");
			}

			printf("%s\n", registroProcesoAux->nombreProceso);


			// Lo vuelvo a agregar a la cola
			queue_push(colaBloqueados, registroProcesoAux);
		}
	}else{
		printf("\nCola Bloqueados vacia\n");
	}
}

	//TODO
	//falta que estime realmente bien usando el multiplicador alfa
int estimarRafaga (int estimacionAnterior, int rafagaAnterior){
	return (estimacionAnterior + rafagaAnterior)/2;
}

// Coordina la Planificacion de Todos los Procesos
Proceso* obtenerProximoProcesoPlanificado(t_list* listaReady, t_queue* colaReady,t_dictionary* diccionarioRafagas, char* algoritmoPlanificacion){

	Proceso* proximoProcesoPlanificado = NULL;
	//ejemplo para conseguir rafaga
	/*Proceso* unProceso;
	Rafagas* unasRafagas;
	unProceso= list_get(listaReady,0);
	unasRafagas= dictionary_get(diccionarioRafagas, unProceso->nombreProceso);
	int estimacionRafaga=unasRafagas->proximaEstimacion;
*/


	


	// Si hay elementos en la ListaReady
	if(list_size(listaReady) > 0){

		// TODO Aplicar los Algoritmos de Planificacion. Ahora solo agarra la ListaReady y ejecuta una clase de FIFO

		// Ordeno la ColaReady segun el Algoritmo de Planificacion
		if(string_starts_with(algoritmoPlanificacion,"SJF-SD")){
			// TODO Aca se deberia cargar la ColaReady siguiendo el Algoritmo
		}	

		if(string_starts_with(algoritmoPlanificacion,"SJF-CD")){
			// TODO Aca se deberia cargar la ColaReady siguiendo el Algoritmo
		}	


		if(string_starts_with(algoritmoPlanificacion,"HRRN")){
			// TODO Aca se deberia cargar la ColaReady siguiendo el Algoritmo
		}	


		// Cargo la ColaReady FIFO para TESTEAR
	    void _each_elemento_(Proceso* registroProcesoAux)
		{
			// Agrego el Registro a la cola
			queue_push(colaReady, registroProcesoAux);
		}
	    list_iterate(listaReady, (void*)_each_elemento_);

		// Extraigo un elemento de la ColaReady
		proximoProcesoPlanificado = queue_pop(colaReady);

		return proximoProcesoPlanificado;		
	}else{
		return NULL;		
	}
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

/* ------------------------------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------------------------------- */

/* ---------------------------------------- */
/*  Funciones de Instancia 					*/
/* ---------------------------------------- */

//funcion para carga de entradas
void cargarTablaEntradas(t_list *tablaEntradas,Instruccion* estructuraInstruccion){
	t_entrada* nuevaEntrada = NULL;

	int tamanioLista = list_size(tablaEntradas);

	nuevaEntrada=malloc(sizeof(t_entrada));


	nuevaEntrada->clave = estructuraInstruccion->key;

	nuevaEntrada->valor = malloc(strlen(estructuraInstruccion->dato)+1);
	strcpy(nuevaEntrada->valor,estructuraInstruccion->dato);
	nuevaEntrada->valor[strlen(estructuraInstruccion->dato)] = '\0';

	nuevaEntrada->numeroDeEntrada = tamanioLista+1;
	nuevaEntrada->tamanioValorAlmacenado = strlen(estructuraInstruccion->dato);

	list_add(tablaEntradas,nuevaEntrada);
}

// persistir una entrada en disco
void persistirEntrada(t_entrada* unaEntrada){

    // Defino el Nombre del Archivo con el nombre de la entrada
	char *nombre_formato_archivo = string_new();
    string_append_with_format(&nombre_formato_archivo, "entradas/%s.txt", unaEntrada->clave);

    FILE* archivoTexto;
	archivoTexto = fopen(nombre_formato_archivo,"w+");

	char *valorIdentificado = unaEntrada->valor;

    // Grabo el Valor de la Entrada en el Archivo
	fputs(valorIdentificado, archivoTexto );

	fclose ( archivoTexto );
    free(nombre_formato_archivo);
}

void error(const char *s)
{
  /* perror() devuelve la cadena S y el error (en cadena de caracteres) que tenga errno */
  perror (s);
  exit(EXIT_FAILURE);
}

void procesoArchivo(char *archivo)
{
  /* Para "procesar", o al menos, hacer algo con el archivo, vamos a decir su tamaño en bytes */
  FILE *fich;
  int ftam;

  fich=fopen(archivo, "r");
  if (fich)
    {
      fseek(fich, 0L, SEEK_END);
      fclose(fich);
      /* Si todo va bien, decimos el tamaño */
      printf ("%30s (%ld bytes)\n", archivo, ftell(fich));
    }
  else
    /* Si ha pasado algo, sólo decimos el nombre */
    printf ("%30s (No info.)\n", archivo);
}

void dump(t_list* tablaEntradas){


		list_iterate(tablaEntradas,(void*)persistirEntrada);

}

/* ------------------------------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------------------------------- */


// Cargo la Lista de Procesos Conectados en el Coordinador
void cargarListaProcesosConectados(t_list *listaProcesosConectados, Proceso* nuevoProceso){

	list_add(listaProcesosConectados, nuevoProceso);
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

	void elemento_destroy(Proceso* self){
		//free(self); // Explota!!!
	}

    void _each_elemento_(char* key, Proceso* registroProcesoAux)
	{
		if(strcmp(registroProcesoAux->nombreProceso, nombreProceso) == 0) {
			dictionary_remove_and_destroy(dictionario, key, (void*) elemento_destroy);
		}
	}
    dictionary_iterator(dictionario, (void*)_each_elemento_);
}
