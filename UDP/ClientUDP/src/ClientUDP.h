/*
 * ClientUDP.h
 *
 *  Created on: 21 nov 2021
 *      Author: AleCongi
 */

#ifndef CLIENTUDP_H_
#define CLIENTUDP_H_

void errorHandler(char*);
void clearWinSock();

void removeExtraSpaces(char*);
char* removeLeadingSpaces(char*);

int argumentsCheck(int, char**);

//Common functions
void setAddresses(struct sockaddr_in *, int, char*);
struct sockaddr_in sockBuild(int *, int, char **);

char* translateIntoString(char *);
char* translateIntoInt(char *);

#endif /* CLIENTUDP_H_ */
