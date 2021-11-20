/*
 ============================================================================
 Name        : ServerTCP.c
 Author      : AleCongi
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#if defined WIN32
#include <winsock.h>
#else
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#define closesocket close
#endif
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#define PROTOPORT 27015 // default protocol port number
#define QLEN 6 // size of request queue

void errorhandler(char *errorMessage) {
	printf("%s", errorMessage);
}

void clearwinsock() {
#if defined WIN32
	WSACleanup();
#endif
}

int main(int argc, char *argv[]) {
	int port;
	if (argc > 1) {
		port = atoi(argv[1]); // if argument specified convert argument to binary
	} else
		port = PROTOPORT; // use default port number
	if (port < 0) {
		printf("bad port number %s \n", argv[1]);
		return 0;
	}

#if defined WIN32
	// Initialize Winsock
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (result != NO_ERROR) {
		printf("Error at WSAStartup()\n");
		return 0;
	}
#endif
	int my_socket;
	my_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (my_socket < 0) {
		errorhandler("socket creation failed.\n");
		clearwinsock();
		return -1;
	}
	// ASSEGNAZIONE DI UN INDIRIZZO ALLA SOCKET
	struct sockaddr_in sad;
	memset(&sad, 0, sizeof(sad)); // ensures that extra bytes contain 0
	sad.sin_family = AF_INET;
	sad.sin_addr.s_addr = inet_addr("127.0.0.1");
	sad.sin_port = htons(port); /* converts values between the host and
	 network byte order. Specifically, htons() converts 16-bit quantities
	 from host byte order to network byte order. */
	if (bind(my_socket, (struct sockaddr*) &sad, sizeof(sad)) < 0) {
		errorhandler("bind() failed.\n");
		closesocket(my_socket);
		clearwinsock();
		return -1;
	}
	// SETTAGGIO DELLA SOCKET ALL'ASCOLTO
	if (listen(my_socket, QLEN) < 0) {
		errorhandler("listen() failed.\n");
		closesocket(my_socket);
		clearwinsock();
		system("pause");
		return -1;
	}
	// ACCETTARE UNA NUOVA CONNESSIONE
	struct sockaddr_in cad; // structure for the client address
	int client_socket; // socket descriptor for the client
	int client_len; // the size of the client address
	client_len = sizeof(cad); // set the size of the client address
	char input[150];
	int clientHandler = 0;
	char operator;
	char first[75];
	char second[75];
	int i = 0;
	int j = 0;

	while (1) {
		printf("Waiting for a client to connect...");
		if ((client_socket = accept(my_socket, (struct sockaddr*) &cad,
				&client_len)) < 0) {
			errorhandler("accept() failed.\n");
			// CHIUSURA DELLA CONNESSIONE
			closesocket(client_socket);
			clearwinsock();
			system("pause");
			return 0;
		}
		printf("Handling client %s\n", inet_ntoa(cad.sin_addr));
		clientHandler = 1;
		while (clientHandler == 1) {
			memset(input, 0, sizeof(input));
			if (recv(client_socket, input, sizeof(char[150]), 0) < 0) {
				errorhandler("receive failed.\n");
				// CHIUSURA DELLA CONNESSIONE
				closesocket(client_socket);
				clearwinsock();
				clientHandler = 0;
			} else {
				if ((input[0] == '=') && (input[1] == '\0')) {
					printf("Ricevuta richiesta abbandono. Chiusura socket.");
					system("pause");
					closesocket(client_socket);
					clientHandler = 0;
				} else {
					operator = input[0];
					if (operator == '+' || operator == '-' || operator == '/'
							|| operator == '*') {
						if (isspace(input[1]) && (input[2] != '\0')
								&& !isspace(input[2])) {
							i = 2;
							while (!isspace(input[i])) {
								first[i - 2] = input[i];
								i++;
							}
							i++;
							if (input[i] != '\0') {
								j = 0;
								while (input[i] != '\0' && !isspace(input[i])) {
									second[j] = input[i];
									i++;
									j++;
								}
								if (isspace(input[i]) && input[i + 1] != '\0') {
									errorhandler("Unavailable operation.");
								} else {


									//OPERAZIONI





								}
							} else {
								errorhandler("Unavailable operation.");
							}
							memset(first, 0, sizeof(input));
							memset(second, 0, sizeof(input));
						} else {
							errorhandler("Unavailable operation.");
						}
					} else {
						errorhandler("Unavailable operation.");

					}
					operator = 0;
					printf("%s", input);
				}
			}
		}
	} // end-while
} // end-main
