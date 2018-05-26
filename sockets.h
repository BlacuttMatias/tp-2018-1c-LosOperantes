#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <netinet/in.h>
#include <asm/poll.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/wait.h>
#include <signal.h>
#include <commons/config.h>
#include <commons/log.h>
#include "registros.h"
#include "funciones.h"
#define BACKLOG 250

int crearServidor(int puerto);
int conectarseAservidor(char *ip,int puerto);

void enviarString(int socket, char* buf);
void recibirString(int socket, char* buf);

int sendall(int, void *, int);

int aceptarConexionCliente(int socketServerFileDescriptor);

Encabezado recibir_header(int* socket);
Paquete recibir_payload(int* socket,int* tam);

