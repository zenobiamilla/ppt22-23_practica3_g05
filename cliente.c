/*******************************************************
 * Protocolos de Transporte
 * Grado en Ingeniería Telemática
 * Dpto. Ingeníería de Telecomunicación
 * Univerisdad de Jaén
 *
 *******************************************************
 * Práctica 1.
 * Fichero: cliente.c
 * Versión: 3.1
 * Fecha: 10/2020
 * Descripción:
 * 	Cliente sencillo TCP para IPv4 e IPv6
 * Autor: Juan Carlos Cuevas Martínez
 *
 ******************************************************
 * Alumno 1:Zenobia Milla Herrero
 * Alumno 2:
 *
 ******************************************************/
#include <stdio.h>		// Biblioteca estándar de entrada y salida
#include <ws2tcpip.h>	// Necesaria para las funciones IPv6
#include <conio.h>		// Biblioteca de entrada salida básica
#include <locale.h>		// Para establecer el idioma de la codificación de texto, números, etc.
#include "protocol.h"	//Declarar constantes y funciones de la práctica

#pragma comment(lib, "Ws2_32.lib")//Inserta en la vinculación (linking) la biblioteca Ws2_32.lib


int main(int* argc, char* argv[])
{
	SOCKET sockfd;
	struct sockaddr* server_in = NULL;
	struct sockaddr_in server_in4;
	struct sockaddr_in6 server_in6;
	int address_size = sizeof(server_in4);
	char buffer_in[1024], buffer_out[1024], input[1024];
	int recibidos = 0, enviados = 0;
	int estado;
	char option;
	int ipversion = AF_INET;//IPv4 por defecto
	char ipdest[256];
	char default_ip4[16] = "127.0.0.1";
	char default_ip6[64] = "::1";

	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	char cmd[5];

	// Variables para la cabecera del mensaje
	char from[256];
	char to[256];
	char subject[512];

	//Variables añadidas ademas de las de cabecera para la practica 3
	int parte_mensaje;
	int control;

	//Inicialización de idioma
	setlocale(LC_ALL, "es-ES");


	//Inicialización Windows sockets - SOLO WINDOWS
	wVersionRequested = MAKEWORD(1, 1);
	err = WSAStartup(wVersionRequested, &wsaData);

	if (err != 0) {
		return(0);
	}

	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1) {
		WSACleanup();
		return(0);
	}
	//Fin: Inicialización Windows sockets

	printf("**************\r\nCLIENTE TCP SENCILLO SOBRE IPv4 o IPv6\r\n*************\r\n");

	do {
		printf("CLIENTE> ¿Qué versión de IP desea usar? 6 para IPv6, 4 para IPv4 [por defecto] ");
		gets_s(ipdest, sizeof(ipdest));

		if (strcmp(ipdest, "6") == 0) {
			//Si se introduce 6 se empleará IPv6
			ipversion = AF_INET6;

		}
		else { //Distinto de 6 se elige la versión IPv4
			ipversion = AF_INET;
		}

		sockfd = socket(ipversion, SOCK_STREAM, 0);
		if (sockfd == INVALID_SOCKET) {
			printf("CLIENTE> ERROR\r\n");
			exit(-1);
		}
		else {
			printf("CLIENTE> Introduzca la IP destino (pulsar enter para IP por defecto): ");
			gets_s(ipdest, sizeof(ipdest));

			//Dirección por defecto según la familia
			if (strcmp(ipdest, "") == 0 && ipversion == AF_INET)
				strcpy_s(ipdest, sizeof(ipdest), default_ip4);

			if (strcmp(ipdest, "") == 0 && ipversion == AF_INET6)
				strcpy_s(ipdest, sizeof(ipdest), default_ip6);

			if (ipversion == AF_INET) {
				server_in4.sin_family = AF_INET;
				server_in4.sin_port = htons(SMTP_SERVICE_PORT);
				inet_pton(ipversion, ipdest, &server_in4.sin_addr.s_addr);
				server_in = (struct sockaddr*) & server_in4;
				address_size = sizeof(server_in4);
			}

			if (ipversion == AF_INET6) {
				memset(&server_in6, 0, sizeof(server_in6));
				server_in6.sin6_family = AF_INET6;
				server_in6.sin6_port = htons(SMTP_SERVICE_PORT);
				inet_pton(ipversion, ipdest, &server_in6.sin6_addr);
				server_in = (struct sockaddr*) & server_in6;
				address_size = sizeof(server_in6);
			}

			//Cada nueva conexión establece el estado incial (BIENVENIDA)
			estado = S_BIENVENIDA;

			if (connect(sockfd, server_in, address_size) == 0) {
				printf("CLIENTE> CONEXION ESTABLECIDA CON %s:%d\r\n", ipdest, SMTP_SERVICE_PORT);

				//Inicio de la máquina de estados: actualizados a los indicados en la figura 1 del guion
				do {
					switch (estado) {

					case S_BIENVENIDA:
						// Se recibe el mensaje de bienvenida
						break;

					case S_HELO: //Identificacion host
						printf("CLIENTE> Introduzca el host: (pulse enter para salir)");
						gets_s(input, sizeof(input));
						//Si pulsamos enter enviamos el comando QUIT y pasamos al estado FIN
						if (strlen(input) == 0) {//Si pulsamos enter enviamos el comando QUIT
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", QUIT, CRLF);
							estado = S_FIN;
						}
						else {//si no enviamos el comando HELO
							sprintf_s(buffer_out, sizeof(buffer_out), "%s %s%s", HELO, input, CRLF);
						}
						break;

					case S_MAIL:
						parte_mensaje = 1; //inicializamos la variable que nos controla la parte del mensaje
						control = 0; //inicializamosla variable que nos controla el final del mensaje
						// Remitente del correo
						printf("CLIENTE> Introduzca el correo del remitente (pulse enter para salir): ");
						gets_s(input, sizeof(input));
						if (strlen(input) == 0) { //si es salir mandamos el comando QUIT
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", QUIT, CRLF);
							estado = S_FIN;
						}
						else {//si no mandamos el comando de aplicacion MAIL con el correo del remitente
							strcpy_s(from, sizeof(from), input); //almacenamos en la variable "from" el remitente
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s%s", MAIL, input, CRLF);
						}
						break;

					case S_RCPT:
						printf("CLIENTE> Introduzca un destinatario (enter para salir): ");
						gets_s(input, sizeof(input));
						if (strlen(input) == 0) {//si es salir mandamos el comando QUIT
							sprintf_s(buffer_out, sizeof(buffer_out), "%s %s", QUIT, CRLF);
							estado = S_FIN;
						}
						else {//si no mandamos el comando de aplicacion RCPT con el correo del remitente
							strcpy_s(to, sizeof(to), input); //almacenamos en la variabre "to" el destinatario
							sprintf_s(buffer_out, sizeof(buffer_out), "%s %s %s", RCPT, input, CRLF);
						}
						break;

					case S_DATA: //enviamos el comando DATA
						sprintf_s(buffer_out, sizeof(buffer_out), "%s %s", DATA, CRLF);
						break;

					case S_MENSAJE:
						/*Dividimos en dos partes el mensajes. Una donde formaremos la cabecera y la enviamos
						 y la parte 2 donde creamos el cuerpo del mensaje a enviar*/
						if (parte_mensaje == 1) {
							printf("CLIENTE> Introduzca el asunto: ");
							gets_s(input, sizeof(input));
							strcpy_s(subject, sizeof(subject), input); //almacenamos en una variable "subject"
							printf("CLIENTE> Introduzca datos (enter para salir): ");
							gets_s(input, sizeof(input));
							//montamos el mensaje completo para enviarlo
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s%s%s%s%s%s%s%s%s%s.%s", "From: ", from, CRLF, "To: ", to, CRLF, "Subject: ", subject, CRLF, input, CRLF, CRLF);
						}

						// ahora montamos el cuerpo del mensaje. Cambiamos el valor de la variable en el controlador de estados
						if (parte_mensaje == 2) {
							do {
								printf("CLIENTE> Introduzca datos (. para finalizar): ");
								gets_s(input, sizeof(input));
								if (strlen(input) > 998) {
									printf("CLIENTE> Ha introducido una linea demasiado larga\r\n");
								}
							} while (strlen(input) > 998);

							// Comprobamos si quiere finalizar el mensaje
							if (strcmp(input, ".") == 0) {
								control = 1;
								sprintf_s(buffer_out, sizeof(buffer_out), ".%s", CRLF);
							}
							else { //si no ha finalizado enviamos la linea introducida
								sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", input, CRLF);
							}
						}
							break;
					}//fin  switch(estado)
						
					if (estado != S_BIENVENIDA) {
						enviados = send(sockfd, buffer_out, (int)strlen(buffer_out), 0);
						if (enviados == SOCKET_ERROR) { //control de errores
								estado = S_FIN;
								continue;// La sentencia continue hace que la ejecución dentro de un
								// bucle salte hasta la comprobación del mismo.
						}
					}

					// Recibimos datos del servidor
					if (estado != S_MENSAJE || (estado == S_MENSAJE && control == 1)) {
						recibidos = recv(sockfd, buffer_in, 512, 0);
						if (recibidos <= 0) {
							DWORD error = GetLastError();
							if (recibidos < 0) {
								printf("CLIENTE> Error %d en la recepcion de datos\r\n", error);
								estado = S_FIN;
							}
							else {
								printf("CLIENTE> Conexion con el servidor cerrada\r\n");
								estado = S_FIN;
							}
						}
						else {
							buffer_in[recibidos] = 0x00;
							printf( "SERVIDOR> %s", buffer_in);
							
						}
					}
					
					// Controlador para el cambio de estado
					switch (estado) {
						case S_BIENVENIDA:
							if (strncmp(buffer_in, "220", 3) == 0) {
								estado++; // Pasamos al estado S_HELO
							}
							else {
									estado = S_FIN; // Si hay error finalizamos
							}
							break;

						case S_HELO:
							if (strncmp(buffer_in, "250", 3) == 0) {
								estado++; // Pasamos al estado S_MAIL
								printf("CLIENTE> Direcc�on de Host correcta\r\n");
							} 
							break;

						case S_MAIL:
							if (strncmp(buffer_in, "250", 3) == 0) {
								estado++; // Pasamos al estado S_RCPT
								printf("CLIENTE> Remitente correcto\r\n");
							} //si da un error u otro codigo volvemos al mismo estado
							break;

						case S_RCPT:
							if (strncmp(buffer_in, "250", 3) == 0) {
								printf("CLIENTE> Destinatario correcto\r\n");
								estado++; // Pasamos al estado S_DATA								
							}
							else {//si no es correcto no pasamos de estado y volvemos al mismo
									printf("CLIENTE> Destinatario incorrecto\r\n");
							}
							break;

						case S_DATA:
							if (strncmp(buffer_in, "354", 3) == 0) {
									estado++;  // pasamos al estado S_MENSAJE
							}
							break;

						case S_MENSAJE:
							//comprobamos en que parte del mensaje estamos
							if (control == 1) {// 
								parte_mensaje = 1;
								if (strncmp(buffer_in, "250", 1) == 0) {
									printf("CLIENTE> Correo enviado con exito\r\n");
									estado++; // Pasamos al estado S_FIN
								}
								else { //Si se produce un error en el envio del mensaje
									printf("CLIENTE> Ha habido un error enviando el correo\r\n");
									estado++; // Pasamos al estado S_FIN
								}
							}
							else { //si la variable control=0, no ha terminado el mensaje, seguimos introduciendo datos
								parte_mensaje = 2;
							}
							break;
					} // fin controlador de estados
				
				} while (estado != S_FIN);

			}
			else {
				int error_code = GetLastError();
				printf("CLIENTE> ERROR AL CONECTAR CON %s:%d\r\n", ipdest, SMTP_SERVICE_PORT);
				
			}
			closesocket(sockfd);
		}
		printf("-----------------------\r\n\r\nCLIENTE> Volver a conectar (S/N)\r\n");
		option = _getche();

	} while (option != 'n' && option != 'N');

	return(0);
}