#include <arpa/inet.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>




#include "CUnit/Basic.h"
#include "funciones.h"
#include "registros.h"
#include "sockets.h"

//***** Variables globales//


/*
int inicializoInstruccion () {
	instruccion.operacion=1;
	strcpy(instruccion.key,"keyCorta");
	instruccion.dato="alfaData";
	return 0;
}*/

void test1() {
	puts("aguante viejas locas");
	CU_ASSERT_EQUAL(1+1,2);
}

void test2(){
	Instruccion instruccion;
	Paquete paquete;
	Encabezado encabezado;

	instruccion.operacion=1;
	strcpy(instruccion.key,"keyCorta");
	instruccion.dato="alfaData";

	puts("algo    ");
	paquete=srlz_instruccion('E',1,instruccion);
	puts("algo2    ");
	Instruccion aux;
	puts("algo3    ");
	aux = dsrlz_instruccion( ((paquete.buffer)+(sizeof(int)*3)) );
	puts("algo4    ");
	CU_ASSERT_EQUAL(instruccion.operacion,aux.operacion);
	//CU_ASSERT_EQUAL(instruccion.operacion,aux.operacion);
	//CU_ASSERT_EQUAL(instruccion.operacion,aux.operacion);

}
void test3(){

}



int main() {

	CU_initialize_registry();
	CU_pSuite prueba1 = CU_add_suite("prueba1",NULL,NULL);
	CU_add_test(prueba1,"test1",test1);
	CU_add_test(prueba1,"serealizacion",test2);

	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();
	CU_cleanup_registry();



	return CU_get_error();
}
