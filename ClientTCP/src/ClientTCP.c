/*
 ============================================================================
 Name        : ClientTCP.c
 Author      : AleCongi (Alessandro Congedo), Giorgia Villano
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
#include "ClientTCP.h"
#define PROTOPORT 27015 // default protocol port number
#define IP "127.0.0.1" //default IP

int main(int argc, char *argv[]) {

#if defined WIN32
	// Initialize Winsock
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (result != NO_ERROR) {
		printf("Error at WSAStartup()\n");
		return 0;
	}
#endif
	int c_socket;
	int check = 1;
	c_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (c_socket != -1) {
		// ASSEGNAZIONE DI UN INDIRIZZO ALLA SOCKET
		struct sockaddr_in sad = sockBuild(&check, argc, argv);
		if (check) {
			// CONNESSIONE AL SERVER
			if (connect(c_socket, (struct sockaddr*) &sad, sizeof(sad)) == 0) {
				char input[150];
				char resultant[150];
				char *rmvSpace;
				while (1) {
					memset(input, 0, sizeof(input));
					gets(input);
					rmvSpace = removeLeadingSpaces(input);
					if ((rmvSpace[0] == '=') && (rmvSpace[1] == '\0')) {
						send(c_socket, rmvSpace, sizeof(char[150]), 0);
						closesocket(c_socket);
						clearWinSock();
						return 1;
					} else {
						if (send(c_socket, rmvSpace, sizeof(char[150]), 0)
								!= -1) {
							if (recv(c_socket, resultant, sizeof(char[150]), 0)
									!= -1) {
								printf("\nResult: %s\n", resultant);
							} else {
								errorHandler("Failed to receive.\n");
								closesocket(c_socket);
								clearWinSock();
								return -1;
							}
						} else {
							errorHandler("Failed to send.\n");
							closesocket(c_socket);
							clearWinSock();
							return -1;
						}
					}
					memset(rmvSpace, 0, sizeof(char[150]));
				}
			} else {
				errorHandler("Failed to connect.\n");
				closesocket(c_socket);
				clearWinSock();
				return -1;
			}
		}
		return -1;
	} else {
		errorHandler("socket creation failed.\n");
		clearWinSock();
		return -1;
	}
}

void errorHandler(char *errorMessage) {
	printf("%s", errorMessage);
}

void clearWinSock() {
#if defined WIN32
	WSACleanup();
#endif
}
void removeExtraSpaces(char *str) {
	int i, x;
	for (i = x = 0; str[i]; ++i)
		if (!isspace(str[i]) || ((i > 0) && !isspace(str[i - 1]))) {
			str[x++] = str[i];
		}
	str[x] = '\0';
}

char* removeLeadingSpaces(char *str) {
	static char str1[150];
	int count = 0, j, k;
	while (str[count] == ' ') {
		count++;
	}
	for (j = count, k = 0; str[j] != '\0'; j++, k++) {
		str1[k] = str[j];
	}
	str1[k] = '\0';
	removeExtraSpaces(str1);
	return str1;
}

void setAddressPort(struct sockaddr_in *sad, int port, char *ip) {
	sad->sin_addr.s_addr = inet_addr(ip);
	sad->sin_port = htons(port);
}

struct sockaddr_in sockBuild(int *ok, int argc, char *argv[]) {
	struct sockaddr_in sad;
	memset(&sad, 0, sizeof(sad));
	sad.sin_family = AF_INET;
	if (argc == 1) {
		setAddressPort(&sad, PROTOPORT, IP);
	} else if (argc == 2) {
		setAddressPort(&sad, PROTOPORT, argv[1]);
	} else if (argc == 3) {
		int port = atoi(argv[2]);
		if (port > 0) {
			setAddressPort(&sad, port, argv[1]);
		} else {
			errorHandler("Bad port number.\n");
			*ok = 0;
		}
	} else {
		*ok = 0;
		memset(&sad, 0, sizeof(sad));
	}
	return sad;
}

