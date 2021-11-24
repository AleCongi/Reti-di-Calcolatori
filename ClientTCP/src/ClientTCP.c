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
		if (argumentsCheck(argc, argv)) {
			//SOCKET ADDRESSES BUILD
			struct sockaddr_in sad = sockBuild(&check, argc, argv);
			if (check) {
				//TRYING TO REACH SERVER
				if (connect(c_socket, (struct sockaddr*) &sad, sizeof(sad))
						== 0) {
					printf(
							"\n\nConnection established with %s:%d\nTo make an operation,"
									"insert parameter in this order:\n*operator(+,-,*,/)* *integer_value_1*"
									"*integer_value 2*\n ex: + 25 13\nPress = to quit connection\n\n\n", inet_ntoa(sad.sin_addr), sad.sin_port);
					char input[150];
					char resultant[150];
					char *rmvSpace;
					while (1) {
						memset(input, 0, sizeof(input));
						gets(input);
						rmvSpace = removeLeadingSpaces(input);
						//"=" IS THE QUIT COMMAND
						if ((rmvSpace[0] == '=') && (rmvSpace[1] == '\0')) {
							send(c_socket, rmvSpace, sizeof(char[150]), 0);
							closesocket(c_socket);
							clearWinSock();
							return 1;
						} else {
							if (send(c_socket, rmvSpace, sizeof(char[150]), 0)
									!= -1) {
								//CLIENTS WAITS FOR AN ANSWER
								if (recv(c_socket, resultant, sizeof(char[150]),
										0) != -1) {
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
			} else {
				errorHandler("Socket Build failed.\n");
				clearWinSock();
				return -1;
			}
		} else {
			errorHandler("Invalid arguments.\n");
			clearWinSock();
			return -1;
		}
	} else {
		errorHandler("Socket Creation failed.\n");
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

//This function would reduce this kind of input "+      26   15"
//into "+ 26 15"
void removeExtraSpaces(char *str) {
	int i, x;
	for (i = x = 0; str[i]; ++i)
		if (!isspace(str[i]) || ((i > 0) && !isspace(str[i - 1]))) {
			str[x++] = str[i];
		}
	str[x] = '\0';
}

//This function would reduce this kind of input "      + 26 15"
//into "+ 26 15"
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

//Checks if custom IP and PORT are valid
int argumentsCheck(int argc, char **argv) {
	if (argc > 1) {
		int pointNumber = 0;
		int i = 0;
		while (argv[1][i] |= '\0') {
			if (argv[1][i] == '.') {
				if (pointNumber == 1) {
					if (argv[1][i - 1] == '0' && argv[1][i - 2] == '.'
							&& argv[1][i + 1] == '0' && argv[1][i + 3] == '0') {
						return 0;
					}
				}
				pointNumber++;
			}
			i++;
		}
		if (pointNumber == 3) {
			if (argc == 2){
			return 1;
			} else if (argc == 3){
				if (atoi(argv[2]) >= 0 && atoi(argv[2]) <= 65535){
					return 1;
				} else {
					return 0;
				}
			}
		} else {
			return 0;
		}
	}else if (argc == 1) {
		return 1;
	}
	return 0;
}

//Population of socket structure: IP and PORT
void setAddresses(struct sockaddr_in *sad, int port, char *ip) {
	sad->sin_addr.s_addr = inet_addr(ip);
	sad->sin_port = htons(port);
}

//If arguments were passed during Server Run,
//the function inserts argvs in socket structure
struct sockaddr_in sockBuild(int *ok, int argc, char *argv[]) {
	struct sockaddr_in sad;
	memset(&sad, 0, sizeof(sad));
	sad.sin_family = AF_INET;
	if (argc == 1) {
		setAddresses(&sad, PROTOPORT, IP);
	} else if (argc == 2) {
		setAddresses(&sad, PROTOPORT, argv[1]);
	} else if (argc == 3) {
		int port = atoi(argv[2]);
		if (port > 0) {
			setAddresses(&sad, port, argv[1]);
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

