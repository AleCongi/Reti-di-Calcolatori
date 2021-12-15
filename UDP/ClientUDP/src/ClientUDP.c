/*
 ============================================================================
 Name        : ClientUDP.c
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
#include "ClientUDP.h"

#define PROTOPORT 48000 // default protocol port number
#define IP "localhost" //default IP
#define MAXECHO 255 //maximum buffer size

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
	c_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (c_socket != -1) {
		//SOCKET ADDRESSES BUILD
		struct sockaddr_in sad = sockBuild(&check, argc, argv);
		struct sockaddr_in sadCheck;
		int sadCheckLen = sizeof(sadCheck);
		memset(&sadCheck, 0, sizeof(sadCheck));
		if (check) {
			char input[MAXECHO];
			char resultant[MAXECHO];
			char *rmvSpace = malloc(sizeof(char[MAXECHO]));
			char safeString[MAXECHO];
			while (1) {
				memset(input, 0, sizeof(input));
				system("cls");
				printf(
						"To make an operation, insert parameter in this order:\n*operator(+,-,*,/)* "
								"*integer_value_1* *integer_value 2*\nex: + 25 13\nPress = to quit connection\n\n\n");
				printf("Operation: ");
				gets(input);
				rmvSpace = removeLeadingSpaces(input);
				//"=" IS THE QUIT COMMAND
				if ((rmvSpace[0] == '=') && (rmvSpace[1] == '\0')) {
					//send(c_socket, rmvSpace, sizeof(char[MAXECHO]), 0);
					closesocket(c_socket);
					clearWinSock();
					return 1;
				} else {
					// ECHOMAX CHECK
					strcpy(safeString, rmvSpace);
					if (sendto(c_socket, safeString, sizeof(safeString), 0,
							(struct sockaddr*) &sad, sizeof(sad))
							== sizeof(safeString)) {
						//CLIENTS WAITS FOR AN ANSWER
						recvfrom(c_socket, resultant, sizeof(resultant), 0,
								(struct sockaddr*) &sadCheck, &sadCheckLen);
						if (sad.sin_addr.s_addr == sadCheck.sin_addr.s_addr) {
							system("cls");
							printf("\n\n\n\t%s\n\n\n", resultant);
							system("PAUSE");
						} else {
							errorHandler("Failed to receive.\n");
							system("PAUSE");
							closesocket(c_socket);
							clearWinSock();
							return -1;
						}
					} else {
						errorHandler("Failed to send.\n");
						system("PAUSE");

						closesocket(c_socket);
						clearWinSock();
						return -1;
					}
				}
				memset(rmvSpace, 0, sizeof(char[MAXECHO]));
			}
		} else {
			errorHandler("Socket Build failed.\n");
			clearWinSock();
			return -1;
		}
	} else {
		errorHandler("Socket Creation failed.\n");
		clearWinSock();
		return -1;
	}
	system("PAUSE");
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
	static char str1[MAXECHO];
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
	if (argc == 2) {
		char canonical[95];
		char pNumber[5];
		splitString(argv[1], canonical, pNumber);
		char ip[16];
		strcpy(ip, translateIntoInt(canonical));
		int pointNumber = 0;
		int i = 0;
		while (ip[i] != '\0') {
			if (ip[i] == '.') {
				if (pointNumber == 1) {
					if (ip[i - 1] == '0' && ip[i - 2] == '.' && ip[i + 1] == '0'
							&& ip[i + 3] == '0') {
						return 0;
					}
				}
				pointNumber++;
			}
			i++;
		}
		if (pointNumber == 3) {
			int i = 0;
			while (pNumber[i] != '\0') {
				if (!isdigit(pNumber[i]) || pNumber[i] == '.'
						|| pNumber[i] == ',') {
					return 0;
				}
				i++;
			}
			if (atoi(pNumber) >= 0 && atoi(pNumber) <= 65535) {
				return 1;
			} else {
				return 0;
			}

		} else {
			return 0;
		}
	} else if (argc == 1) {
		return 1;
	}
	return 0;
}

//Population of socket structure: IP and PORT
void setAddresses(struct sockaddr_in *sad, int port, char *ip) {
	if (isdigit(ip[0]) && (isdigit(ip[1]) || ip[1] == '.')
			&& (isdigit(ip[2]) || ip[2] == '.')
			&& (isdigit(ip[3]) || ip[3] == '.')) {
		sad->sin_addr.s_addr = inet_addr(ip);
	} else {
		sad->sin_addr.s_addr = inet_addr(translateIntoInt(ip));
	}
	sad->sin_port = htons(port);
}

//If arguments were passed during Server Run,
//the function inserts argvs in socket structure
struct sockaddr_in sockBuild(int *ok, int argc, char *argv[]) {
	struct sockaddr_in cad;
	memset(&cad, 0, sizeof(cad));
	cad.sin_family = AF_INET;
	if (argc == 1) {
		setAddresses(&cad, PROTOPORT, IP);
	} else if (argc == 2) {
		char line[100];
		char canonical[95];
		char pNumber[5];
		strcpy(line, argv[1]);
		if (splitString(line, canonical, pNumber)) {
			int port = atoi(pNumber);
			if (port < 65536 && port > 0) {
				setAddresses(&cad, port, canonical);
			} else {
				*ok = 0;
				memset(&cad, 0, sizeof(cad));
			}
		} else {
			*ok = 0;
			memset(&cad, 0, sizeof(cad));
		}
	} else {
		*ok = 0;
		memset(&cad, 0, sizeof(cad));
	}
	return cad;
}

char* translateIntoInt(char *input) {

	struct hostent *host;
	host = gethostbyname(input);
	if (host == NULL) {
		errorHandler("gethostbyname() failed.\n");
		exit(EXIT_FAILURE);
	} else {
		struct in_addr *ina = (struct in_addr*) host->h_addr_list[0];
		return inet_ntoa(*ina);
	}
}

char* translateIntoString(char *input) {
	struct in_addr addr;
	struct hostent *host;

	addr.s_addr = inet_addr(input);
	host = gethostbyaddr((char*) &addr, 4, AF_INET);
	char *canonical_name = host->h_name;
	return canonical_name;
}

int splitString(char *input, char *first, char *second) {
	int ok = 0;
	int i = 0;
	while (input[i] != '\0') {
		if (input[i] == ':' && (i != 0) && input[i + 1] != '\0') {
			ok = 1;
		}
		i++;
	}
	i = 0;
	if (ok) {
		while (input[i] != ':') {
			first[i] = input[i];
			i++;
		}
		i++;
		int j = 0;
		while (input[i] != '\0') {
			second[j] = input[i];
			i++;
			j++;
		}
	}
	return ok;
}

