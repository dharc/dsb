/*
 * net.c
 *
 *  Created on: 22 May 2013
 *      Author: nick

Copyright (c) 2013, dharc ltd.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met: 

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer. 
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies, 
either expressed or implied, of the FreeBSD Project.
 */

#include "dsb/errors.h"
#include <string.h>
#include <stdlib.h>

#ifndef WIN32
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>
#endif

#ifdef WIN32
#include <windows.h>
#include <winsock.h>
typedef int socklen_t;
#define MSG_WAITALL 0
#endif

#ifndef WIN32
#include <errno.h>
#include <fcntl.h>
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#endif

#define MAX_CONNECTIONS		100

int connections[MAX_CONNECTIONS] = {-1};

int dsb_net_init()
{
	//Initialise windows sockets
	#ifdef WIN32
	WSAData wsaData;
	if (WSAStartup(MAKEWORD(1,1), &wsaData) != 0) {
		//ERROR
		DSB_ERROR(ERR_NETCONNECT,url);
		return ERR_NET;
	}
	#endif

	return SUCCESS;
}

int dsb_net_final()
{
	return SUCCESS;
}

int dsb_net_connect(const char *url)
{
	int rc;
	struct sockaddr_in destAddr;
	int sock;
	char addr[100];
	int port = 5555;
	char *t;
	int i;
	#ifdef WIN32
	HOSTENT *host;
	#else
	struct hostent *host;
	#endif

	//Split address and port (if there is a port).
	strcpy(addr,url);
	t = strchr(addr,':');
	if (t != 0)
	{
		*t = 0;
		t++;
		port = atoi(t);
	}

	//We want a TCP socket
	sock = socket(AF_INET, SOCK_STREAM, 0);

	if (sock == INVALID_SOCKET) {
		DSB_ERROR(ERR_NETCONNECT,url);
		return -1;
	}

	//DNS lookup the IP.
	#ifdef WIN32
	host = gethostbyname(addr);
	#else
	host = gethostbyname(addr);
	#endif

	destAddr.sin_family = AF_INET;
	destAddr.sin_addr.s_addr = ((struct in_addr *)(host->h_addr))->s_addr;
	destAddr.sin_port = htons(port);

	//Actually attempt to connect
	rc = connect(sock, (struct sockaddr*)&destAddr, sizeof(destAddr));

	if (rc == SOCKET_ERROR) {
		#ifndef WIN32
		close(sock);
		#else
		closesocket(sock);
		#endif

		DSB_ERROR(ERR_NETCONNECT,url);
		return -1;
	}

	//Now need to store connection socket.
	for (i=0; i<MAX_CONNECTIONS; i++)
	{
		if (connections[i] == -1)
		{
			connections[i] = sock;
			return sock;
		}
	}

	#ifndef WIN32
	close(sock);
	#else
	closesocket(sock);
	#endif
	DSB_ERROR(ERR_NETCONNECT,url);
	return -1;
}

int dsb_net_poll()
{
	//Select on each connection and pass to correct msg handler.
	return SUCCESS;
}

