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
						// Remitente del correo
						printf("CLIENTE> Introduzca el correo del remitente (pulse enter para salir): ");
						gets_s(input, sizeof(input));
						if (strlen(input) == 0) { //si es salir mandamos el comando QUIT
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", QUIT, CRLF);
							estado = S_FIN;
						}
						else {//si no mandamos el comando de aplicacion MAIL con el correo del remitente
					
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s%s", MAIL, input, CRLF);
						}
						break;
					case S_RCPT:
						printf("CLIENTE> Introduzca el destinatario (enter para salir): ");
						gets_s(input, sizeof(input));
						if (strlen(input) == 0) {//si es salir mandamos el comando QUIT
							sprintf_s(buffer_out, sizeof(buffer_out), "%s %s", QUIT, CRLF);
							estado = S_FIN;
						}
						else {//si no mandamos el comando de aplicacion RCPT con el correo del remitente
							
							sprintf_s(buffer_out, sizeof(buffer_out), "%s %s %s", RCPT, input, CRLF);
							break;
						}
					case S_DATA: //Enviamos el comando DATA
						sprintf_s(buffer_out, sizeof(buffer_out), "%s %s", DATA, CRLF);
						break;
					case S_MENSAJE:
						printf("CLIENTE> Introduzca el asunto: ");
						gets_s(input, sizeof(input));
						strcpy_s(subject, sizeof(subject), input);
						// Enviamos la cabecera: From, To, Subject
						sprintf_s(buffer_out, sizeof(buffer_out), "%s%s%s%s%s%s%s%s%s%s", "From: ", from, CRLF, "To: ", to, CRLF, "Subject: ", subject, CRLF, CRLF);
						//Pedimos que introduzca el mensaje, no puede superar 998bytes + cabeceras = tamaño de buffer_out
						printf("CLIENTE> Introduzca el mensaje, (. en una linea aparte para finalizar): ");
						gets_s(input, sizeof(input));
						do{
							//Comprobamos tamaño
							if (strlen(input) > 998)
								printf("CLIENTE>Tamaño excedido. Recuerde, el maximo son 998 caracteres.%s", CRLF);
						} while (strlen(input) > 998);
					
							// Enviamos de la linea introducida
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", input, CRLF);
						
						break;
					}

					if (estado != S_BIENVENIDA) {
						enviados = send(sockfd, buffer_out, (int)strlen(buffer_out), 0);
						if (enviados == SOCKET_ERROR) { //control de errores
							estado = S_FIN;
							continue;// La sentencia continue hace que la ejecución dentro de un
							// bucle salte hasta la comprobación del mismo.
						}
					}

					recibidos = recv(sockfd, buffer_in, 512, 0);
					//La maquina de estado para pasar de estado, la montamos a continuacion de recibir la respuesta del servidor
					//ya que dependera de ella al estado al que pasemos.
					if (recibidos <= 0) { //control de errores
						DWORD error = GetLastError();
						if (recibidos < 0) {
							printf("CLIENTE> Error %d en la recepción de datos\r\n", error);
							estado = S_FIN;
						}
						else {
							printf("CLIENTE> Conexión con el servidor cerrada\r\n");
							estado = S_FIN;
						}
					}
					else { // segun la respuesta del servidor, avanzaremos o no de estado.
						buffer_in[recibidos] = 0x00;
						printf(buffer_in); //Respuesta del servidor
						//si el codigo de respuesta es 2XX->Peticion exito
						// y si el codigo de respuesta es 3xx-> El comando se ha aceptado pero su proceso se ha retrasado a la espera de más información.
						if ((strncmp(buffer_in, "2", 1) == 0) || strncmp(buffer_in, "3", 1) == 0) {
							//Cambio de estado
							if (estado == S_MENSAJE) {
								estado = S_MAIL;
							}
							else {
								estado++;
							}
						}
					}

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
