/*
 ============================================================================
 Name        : ServerUDP.c
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
#include <math.h>
#include "ServerUDP.h"

#define PROTOPORT 48000 // default protocol port number
#define IP "localhost" // default IP number
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
	int my_socket;
	int check = 1;
	char ERROR_PRINT[MAXECHO] = { "Unable to calculate." };
	my_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (my_socket != -1) {
		if (argumentsCheck(argc, argv)) {
			//ADDRESSING SOCKET
			struct sockaddr_in sad = sockBuild(&check, argc, argv);
			if (check) {
				if (bind(my_socket, (struct sockaddr*) &sad, sizeof(sad))
						== 0) {
					//ACCEPTING NEW CONNECTION
					struct sockaddr_in cad; //structure for the client address
					int client_len; //the size of the client address
					int package = 0;
					client_len = sizeof(cad); //set the size of the client address

					char input[MAXECHO]; //input string received from client
					char operator[2]; //assumes the character of the operation needed (+, -, *, /)
					char first[127]; //first value
					char second[127]; //second value
					char *finalRes = malloc(sizeof(char[MAXECHO])); //final result, variable we want to send back to client

					while (1) {
						printf("Waiting for a client to connect...\n");
						memset(input, 0, sizeof(char[MAXECHO]));
						memset(first, 0, sizeof(char[127]));
						memset(second, 0, sizeof(char[127]));
						package = 0;
						package = recvfrom(my_socket, input, sizeof(input), 0,
								(struct sockaddr*) &cad, &client_len);
						printf(
								"Requested operation \'%s\' from client %s, ip %s\n",
								input,
								translateIntoString(inet_ntoa(cad.sin_addr)),
								inet_ntoa(cad.sin_addr));

						//QUIT REQUEST
						if ((input[0] == '=')
								&& ((input[1] == '\0')
										|| (isspace(input[1])
												&& input[2] == '\0'))) {
						} else {
							operator[0] = input[0];
							operator[1] = '\0';
							//INPUT INTEGRITY CONTROL
							if (legitOperator(operator[0]) && legitInput(input)) {
								//INPUT TOKENIZATION
								populateValues(input, first, second);
								//NUMERIC CONTROL
								if (numericCheck(first, second)) {
									finalRes = calculation(operator, first,
											second);
									strcpy(input, finalRes);
									//MODIFICARE FINEL RES COME SEGUE: VAL1 OP VAL2 = RES

									//SENDING RESULT
									if (sendto(my_socket, input, sizeof(input),
											0, (struct sockaddr*) &cad,
											client_len) != package) {
										errorHandler("Failed to send.\n");
									}
								} else {
									//SENDING ERROR BACK TO CLIENT
									if (sendto(my_socket, ERROR_PRINT,
											sizeof(ERROR_PRINT), 0,
											(struct sockaddr*) &cad, client_len)
											!= package) {
										errorHandler("Failed to send.\n");
									}
								}
							} else {
								//SENDING ERROR BACK TO CLIENT
								if (sendto(my_socket, ERROR_PRINT,
										sizeof(ERROR_PRINT), 0,
										(struct sockaddr*) &cad, client_len)
										!= package) {
									errorHandler("Failed to send.\n");
								}
							}
							operator[0] = 0;
						}
					}
				} else {
					errorHandler("bind() failed.\n");
					closesocket(my_socket);
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

//Closes client socket
void leave(int clientSocket) {
	printf("Leaving request acquired: closing socket.\n");
	closesocket(clientSocket);
}

//INTEGRITY CONTROLS

//Checks if the operator is allowed
int legitOperator(char operator) {
	if (operator == '+' || operator == '-' || operator == '/'
			|| operator == '*') {
		return 1;
	} else {
		return 0;
	}
}

//Checks if input string is compatible
int legitInput(char *input) {
	int i;
	/* Is there a space after the operator?
	 * does the first value exist?
	 * is the first value a space char?
	 */
	if (isspace(input[1]) && (input[2] != '\0') && !isspace(input[2])) {
		i = 2;
		while (!isspace(input[i])) {
			i++;
		}
		i++;
		//Does the second value exist?
		if (input[i] != '\0') {
			while (input[i] != '\0' && !isspace(input[i])
					&& isdigit(input[i]) != 0) {
				i++;
			}
			//Are there other unacceptable values?
			if (isspace(input[i]) && input[i + 1] != '\0') {
				//errorHandler("Unavailable operation: there are more than two values\n");
				return 0;
			} else {
				return 1;
			}
		} else {
			//errorHandler("Unavailable operation: no second values found.\n");
			return 0;
		}
	} else {
		//errorHandler("Unavailable operation: no values found.\n");
		return 0;
	}
}

//Checks if values are just numbers
int numericCheck(char *first, char *second) {
	int i = 0;
	int checkDigit = 1;
	//is the first value a number?
	while (first[i] != '\0') {
		if (isdigit(first[i]) == 0 || first[i] == '.' || first[i] == ',') {
			checkDigit = 0;
		}
		i++;
	}
	i = 0;
	if (checkDigit == 1) {
		//is the second value a number?
		while (second[i] != '\0') {
			if (isdigit(second[i]) == 0 || second[i] == '.'
					|| second[i] == ',') {
				checkDigit = 0;
				printf("%c", second[i]);
			}
			i++;
		}
		if (checkDigit == 0) {
			//errorHandler("Unacceptable second value.\n");
		}
	} else {
		//errorHandler("Unacceptable first value.\n");
	}
	return checkDigit;
}

//Checks if custom IP are valid
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
			if (argc == 2) {
				return 1;
			} else if (argc == 3) {
				int i = 0;
				while (argv[2][i] != '\0') {
					if (!isdigit(argv[2][i]) || argv[2][i] == '.'
							|| argv[2][i] == ',') {
						return 0;
					}
					i++;
				}
				if (atoi(argv[2]) >= 0 && atoi(argv[2]) <= 65535) {
					return 1;
				} else {
					return 0;
				}
			}
		} else {
			return 0;
		}
	} else if (argc == 1) {
		return 1;
	}
	return 0;
}

//Fills "first value" and "second value" strings from input string
void populateValues(char *input, char *first, char *second) {
	int i = 2;
	while (!isspace(input[i])) {
		first[i - 2] = input[i];
		i++;
	}
	i++;
	int j = 0;
	while (input[i] != '\0' && !isspace(input[i])) {
		second[j] = input[i];
		i++;
		j++;
	}
}

//MATH
char* sum(int first, int second) {
	int resultInt = first + second;
	static char result[MAXECHO];
	itoa(resultInt, result, 10);
	return result;
}
char* sub(int first, int second) {
	int resultInt = first - second;
	static char result[MAXECHO];
	itoa(resultInt, result, 10);
	return result;
}
char* mult(int first, int second) {
	int resultInt = first * second;
	static char result[MAXECHO];
	itoa(resultInt, result, 10);
	return result;
}
char* division(int first, int second) {
	static char result[MAXECHO];
	// "0/0"
	if (first == 0 && second == 0) {
		strcpy(result, "Indeterminate Calculation\n");
	} else if (second == 0) { // "n/0" with n integer
		strcpy(result, "Impossible Calculation\n");
	} else {
		float resultFlt = (float) first / (float) second;
		gcvt(resultFlt, 3, result);
		int i = 0;
		while (result[i + 1] != '\0') {
			i++;
		}
		if (result[i] == '.' || result[i] == ',') {
			result[i] = ' ';
		}
	}
	return result;
}

char* calculation(char *operator, char *first, char *second) {
	int firstInt = atoi(first);
	int secondInt = atoi(second);
	char *result = malloc(sizeof(char[MAXECHO]));
	strcpy(result, first);
	strcat(result, " ");
	strcat(result, operator);
	strcat(result, " ");
	strcat(result, second);
	strcat(result, " ");
	strcat(result, "=");
	strcat(result, " ");
	if (operator[0] == '+') {
		strcat(result, sum(firstInt, secondInt));
	} else if (operator[0] == '-') {
		strcat(result, sub(firstInt, secondInt));
	} else if (operator[0] == '*') {
		strcat(result, mult(firstInt, secondInt));
	} else if (operator[0] == '/') {
		strcat(result, division(firstInt, secondInt));
	}
	return result;
}

//Population of socket structure: IP and PORT
void setAddresses(struct sockaddr_in *sad, int port, char *ip) {
	sad->sin_addr.s_addr = inet_addr(translateIntoInt(ip));
	sad->sin_port = htons(port);
}

//If arguments were passed during Server Run,
//the function inserts argvs in socket structure
struct sockaddr_in sockBuild(int *ok, int argc, char *argv[]) {
	struct sockaddr_in sad;
	memset(&sad, 0, sizeof(sad));
	sad.sin_family = AF_INET;
	if (argc == 1) {
		//TRANSLATE PROTOIP
		setAddresses(&sad, PROTOPORT, IP);
	} else if (argc == 2) {
		//STRIP AND TRANSLATE
		setAddresses(&sad, PROTOPORT, argv[1]);
	} else if (argc == 3) {
		//DELETE
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

