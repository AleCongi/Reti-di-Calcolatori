/*
 * ServerTCP.h
 *
 *  Created on: 21 nov 2021
 *      Author: Alessandro
 */

#ifndef SERVERTCP_H_
#define SERVERTCP_H_

void clearWinSock();
void errorHandler(char*);
void leave(int);

void reverse(char*, int);
int intToStr(int, char* , int);
void ftoa(float, char*, int);

int legitOperator(char);
int legitInput(char*);
int numericCheck(char*, char*);

void populateValues(char*, char*, char*);

char* calculation(int, char*, char*);
char* sum(int, int);
char* sub(int, int);
char* mult(int, int);
char* division(int, int);

void setAddressPort(struct sockaddr_in *, int, char*);
struct sockaddr_in sockBuild(int *, int, char **);

#endif /* SERVERTCP_H_ */
