/*
 * ServerUDP.h
 *
 *  Created on: 21 nov 2021
 *      Author: Alessandro
 */

#ifndef SERVERUDP_H_
#define SERVERUDP_H_

void clearWinSock();
void errorHandler(char*);
void leave(int);

void reverse(char*, int);
int intToStr(int, char* , int);
void ftoa(float, char*, int);

int legitOperator(char);
int legitInput(char*);
int numericCheck(char*, char*);
int argumentsCheck(int, char**);

void populateValues(char*, char*, char*);

char* calculation(char*, char*, char*);
char* sum(int, int);
char* sub(int, int);
char* mult(int, int);
char* division(int, int);

//Common functions
void setAddresses(struct sockaddr_in *, int, char*);
struct sockaddr_in sockBuild(int *, int, char **);

char* translateIntoString(char *);
char* translateIntoInt(char *);

int splitString(char *, char *, char *);

#endif /* SERVERUDP_H_ */
