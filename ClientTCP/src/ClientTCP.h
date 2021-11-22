/*
 * ClientTCP.h
 *
 *  Created on: 21 nov 2021
 *      Author: AleCongi
 */

#ifndef CLIENTTCP_H_
#define CLIENTTCP_H_

void errorHandler(char*);
void clearWinSock();

void removeExtraSpaces(char*);
char* removeLeadingSpaces(char*);

void setAddressPort(struct sockaddr_in *, int, char*);
struct sockaddr_in sockBuild(int *, int, char **);

#endif /* CLIENTTCP_H_ */
