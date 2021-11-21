/*
 ============================================================================
 Name        : ServerTCP.c
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
#include "ServerTCP.h"
#define PROTOPORT 27015 // default protocol port number
#define QLEN 5 // size of request queue

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
		errorHandler("socket creation failed.\n");
		clearWinSock();
		return -1;
	}
	//ADDRESSING SOCKET
	struct sockaddr_in sad;
	memset(&sad, 0, sizeof(sad)); // ensures that extra bytes contain 0
	sad.sin_family = AF_INET;
	sad.sin_addr.s_addr = inet_addr("127.0.0.1");
	sad.sin_port = htons(port); /* converts values between the host and
	 network byte order. Specifically, htons() converts 16-bit quantities
	 from host byte order to network byte order. */
	if (bind(my_socket, (struct sockaddr*) &sad, sizeof(sad)) < 0) {
		errorHandler("bind() failed.\n");
		closesocket(my_socket);
		clearWinSock();
		return -1;
	}
	//SETTING LISTEN
	if (listen(my_socket, QLEN) < 0) {
		errorHandler("listen() failed.\n");
		closesocket(my_socket);
		clearWinSock();
		system("pause");
		return -1;
	}
	//ACCEPTING NEW CONNECTION
	struct sockaddr_in cad; //structure for the client address
	int client_socket; //socket descriptor for the client
	int client_len; //the size of the client address
	client_len = sizeof(cad); //set the size of the client address
	char input[150]; //input string received from client
	int clientHandler = 0; //index to handle leaving will
	char operator; //assumes the character of the operation needed (+, -, *, /)
	char first[75]; //first value
	char second[75]; //second value
	char *finalRes; //final result, variable we want to send back to client

	while (1) {
		printf("Waiting for a client to connect...");
		if ((client_socket = accept(my_socket, (struct sockaddr*) &cad,
				&client_len)) < 0) {
			errorHandler("accept() failed.\n");
			closesocket(client_socket);
			clearWinSock();
			system("pause");
			return 0;
		}
		printf("Handling client %s\n", inet_ntoa(cad.sin_addr));
		clientHandler = 1;
		while (clientHandler == 1) {
			memset(input, 0, sizeof(input));
			memset(first, 0, sizeof(input));
			memset(second, 0, sizeof(second));
			if (recv(client_socket, input, sizeof(char[150]), 0) < 0) {
				errorHandler("Receive failed.\n");
				closesocket(client_socket);
				clearWinSock();
				clientHandler = 0;
			} else {
				if ((input[0] == '=')
						&& ((input[1] == '\0')
								|| (isspace(input[1]) && input[2] == '\0'))) {
					leave(client_socket);
					clientHandler = 0;
				} else {
					operator = input[0];
					if (legitOperator(operator) && legitInput(input)) {
						populateValues(input, first, second);
						if (numericCheck(first, second)) {
							finalRes = calculation(operator, first, second);
							if (send(client_socket, finalRes, sizeof(char[150]),
									0) < 0) {
								errorHandler("Failed to send.\n");
								closesocket(client_socket);
							}
						}
					}
					operator = 0;
				}
			}
		}
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

//reverses a string 'str' of length 'len'
void reverse(char *str, int len) {
	int i = 0, j = len - 1, temp;
	while (i < j) {
		temp = str[i];
		str[i] = str[j];
		str[j] = temp;
		i++;
		j--;
	}
}

/*converts a given integer x to string str[].
 d is the number of digits required in the output.
 If d is more than the number of digits in x,
 then 0s are added at the beginning.*/
int intToStr(int x, char str[], int d) {
	int i = 0;
	while (x) {
		str[i++] = (x % 10) + '0';
		x = x / 10;
	}
	while (i < d)
		str[i++] = '0';

	reverse(str, i);
	str[i] = '\0';
	return i;
}

// Converts a floating-point/double number to a string.
void ftoa(float n, char *res, int afterpoint) {
	int ipart = (int) n;
	float fpart = n - (float) ipart;
	int i = intToStr(ipart, res, 0);
	if (afterpoint != 0) {
		res[i] = '.';
		fpart = fpart * pow(10, afterpoint);
		intToStr((int) fpart, res + i + 1, afterpoint);
	}
}

//closes client socket
void leave(int clientSocket) {
	printf("Leaving request acquired: closing socket.\n");
	closesocket(clientSocket);
}

//INTEGRITY CONTROLS

//checks if the operator is allowed
int legitOperator(char operator) {
	if (operator == '+' || operator == '-' || operator == '/'
			|| operator == '*') {
		return 1;
	} else {
		errorHandler("Unavailable operation.\n");
		return 0;
	}
}

//checks if input string is compatible
int legitInput(char *input) {
	int i;
	/* is there a space after the operator?
	 * does the first value exist?
	 * is the first value a space char?
	 */
	if (isspace(input[1]) && (input[2] != '\0') && !isspace(input[2])) {
		i = 2;
		while (!isspace(input[i])) {
			i++;
		}
		i++;
		//does the second value exist?
		if (input[i] != '\0') {
			while (input[i] != '\0' && !isspace(input[i])
					&& isdigit(input[i]) != 0) {
				i++;
			}
			//are there other unacceptable values?
			if (isspace(input[i]) && input[i + 1] != '\0') {
				errorHandler(
						"Unavailable operation: there are more than two values\n");
				return 0;
			} else {
				return 1;
			}
		} else {
			errorHandler("Unavailable operation: no second values found.\n");
			return 0;
		}
	} else {
		errorHandler("Unavailable operation: no values found.\n");
		return 0;
	}
}

//checks if values are just numbers
int numericCheck(char *first, char *second) {
	int i = 0;
	int checkDigit = 1;
	//is the first value a number?
	while (first[i] != '\0') {
		if (isdigit(first[i]) == 0) {
			checkDigit = 0;
		}
		i++;
	}
	i = 0;
	if (checkDigit == 1) {
		//is the second value a number?
		while (second[i] != '\0') {
			if (isdigit(second[i]) == 0) {
				checkDigit = 0;
			}
			i++;
		}
		if (checkDigit == 0) {
			errorHandler("Unacceptable second value.\n");
		}
	} else {
		errorHandler("Unacceptable first value.\n");
	}
	return checkDigit;
}

//fills "first value" and "second value" strings from input string
void populateValues(char *input, char *first, char *second) {
	int i = 2;
	while (!isspace(input[i])) {
		first[i - 2] = input[i];
		i++;
	}
	i++;
	int j = 0;
	while (input[i] != '\0' && !isspace(input[i]) && isdigit(input[i]) != 0) {
		second[j] = input[i];
		i++;
		j++;
	}
}

//MATH
char* sum(int first, int second) {
	int resultInt = first + second;
	static char result[150];
	itoa(resultInt, result, 10);
	return result;
}
char* sub(int first, int second) {
	int resultInt = first - second;
	static char result[150];
	itoa(resultInt, result, 10);
	return result;
}
char* mult(int first, int second) {
	int resultInt = first * second;
	static char result[150];
	itoa(resultInt, result, 10);
	return result;
}
char* division(int first, int second) {
	static char result[150];
	if (first == 0 && second == 0) {
		strcpy(result, "Indeterminate Calculation\n");
	} else if (second == 0) {
		strcpy(result, "Impossible Calculation\n");
	} else {
		float resultFlt = (float) first / (float) second;
		ftoa(resultFlt, result, 2);
	}
	return result;
}

char* calculation(int operator, char *first, char *second) {
	int firstInt = atoi(first);
	int secondInt = atoi(second);
	char *result;
	if (operator == '+') {
		result = sum(firstInt, secondInt);
	} else if (operator == '-') {
		result = sub(firstInt, secondInt);
	} else if (operator == '*') {
		result = mult(firstInt, secondInt);
	} else if (operator == '/') {
		result = division(firstInt, secondInt);
	}
	return result;
}

