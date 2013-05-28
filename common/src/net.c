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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

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

#include "dsb/errors.h"
#include "dsb/net.h"
#include "dsb/net_protocol.h"

#define MAX_CONNECTIONS		100

static int connections[MAX_CONNECTIONS];
static fd_set fdread;
static fd_set fderror;

struct NetMessages
{
	int size;
	int (*cb)(int,void*);
};

static struct NetMessages messages[DSBNET_TYPE_END];

int dsb_net_init()
{
	int i;

	//Initialise windows sockets
	#ifdef WIN32
	WSAData wsaData;
	if (WSAStartup(MAKEWORD(1,1), &wsaData) != 0) {
		//ERROR
		DSB_ERROR(ERR_NETCONNECT,url);
		return ERR_NET;
	}
	#endif

	for (i=0; i<MAX_CONNECTIONS; i++) connections[i] = INVALID_SOCKET;

	//Clear all message details
	for (i=0; i<DSBNET_TYPE_END; i++)
	{
		messages[i].size = 0;
		messages[i].cb = 0;
	}

	//Now insert message details.
	messages[DSBNET_SENDEVENT].size = sizeof(struct DSBNetEventSend);
	messages[DSBNET_EVENTRESULT].size = sizeof(struct DSBNetEventResult);
	messages[DSBNET_EVENTRESULT].cb = dsb_net_cb_result;

	return 0;
}

int dsb_net_final()
{
	return 0;
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

	if (host == 0)
	{
		DSB_ERROR(ERR_NETADDR,addr);
		return -1;
	}

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

	//Non-block from here.
	#ifdef UNIX
	fcntl(sock, F_SETFL, O_NONBLOCK);
	#endif

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

static int set_descriptors()
{
	int n = 0;
	int i;

	//Reset all file descriptors
	FD_ZERO(&fdread);
	FD_ZERO(&fderror);

	//Set the file descriptors for each client
	for (i=0; i<MAX_CONNECTIONS; i++) {
		#ifdef WIN32
		if (connections[i] != INVALID_SOCKET) {
		#else
		if (connections[i] >= 0) {
		#endif
			if (connections[i] > n) {
				n = connections[i];
			}
			FD_SET(connections[i], &fdread);
			FD_SET(connections[i], &fderror);
		}
	}

	return n;
}

static int read_messages(int sock)
{
	int rc;
	struct DSBNetHeader *header;
	static char buffer[1000];
	static int six = 0;
	int ix = 0;

	//Read into remaining buffer space.
	rc = recv(sock, buffer + six, 1000 - six, 0);

	//No data to process.
	if (rc <= 0)
	{
		DSB_INFO(INFO_NETDISCONNECT,0);
		dsb_net_disconnect(sock);
		return SUCCESS;
	}

	//Add existing buffer contents to rc and reset index.
	rc += six;
	six = 0;
	ix = 0;

	//We have enough for a header... so repeatedly process until we don't
	while (rc >= sizeof(struct DSBNetHeader))
	{
		header = (struct DSBNetHeader*)buffer+ix;
		ix += sizeof(struct DSBNetHeader);

		//Make sure it is a valid message.
		if (header->chck != DSB_NET_CHECK) return DSB_ERROR(ERR_NETMSGCHK,0);
		if (header->type >= DSBNET_TYPE_END) return DSB_ERROR(ERR_NETMSGTYPE,0);

		//Do we have the entire message contents?
		if (rc-sizeof(struct DSBNetHeader) >= messages[header->type].size)
		{
			char temp[10];
			sprintf(temp, "%d",header->type);
			DSB_DEBUG(DEBUG_NETMSG,temp);

			//YES, so look for the callback
			if (messages[header->type].cb != 0)
			{
				messages[header->type].cb(sock, (void*)buffer+ix);
			}
			else
			{
				DSB_ERROR(ERR_NETCB,0);
			}

			rc -= sizeof(struct DSBNetHeader)+messages[header->type].size;
			ix += messages[header->type].size;

			//Completed message so move to next.
			six = ix;
		}
		else
		{
			//NO, not enough data so finish for now.
			break;
		}
	}

	//Do we have left over data
	if (rc != 0)
	{
		//Yes, so move to bottom for processing next time.
		memcpy(buffer, buffer+six, rc-six);
		//Set start index to next free buffer location.
		six = rc-six;
	}
	else
	{
		six = 0;
	}

	return 0;
}

int dsb_net_poll(unsigned int ms)
{
	int n = set_descriptors();
	struct timeval block;
	int selres;
	int i;

	//There are no connections to check.
	if (n == 0) return SUCCESS;

	//Timeout immediately.
	block.tv_sec = 0;
	block.tv_usec = ms * 10;
	selres = select(n+1, &fdread, 0, &fderror, &block);

	//Some kind of error occurred, it is usually possible to recover from this.
	if (selres <= 0) {
		return SUCCESS;
	}

	//Also check each clients socket to see if any messages or errors are waiting
	for (i=0; i<MAX_CONNECTIONS; i++) {
		#ifdef WIN32
		if (connections[i] != INVALID_SOCKET) {
		#else
		if (connections[i] >= 0) {
		#endif
			//If message received from this client then deal with it
			if (FD_ISSET(connections[i], &fdread)) {
				//Loop until no more messages to read.
				read_messages(connections[i]);
			//An error occurred with this client.
			} else if (FD_ISSET(connections[i], &fderror)) {
				//s_conns[i]->error();
				connections[i] = -1;
				DSB_INFO(INFO_NETDISCONNECT,0);
			}
		}
	}
	return 0;
}

int dsb_net_callback(int msgtype, int (*cb)(int,void *))
{
	if (msgtype < 0 || msgtype >= DSBNET_TYPE_END)
	{
		return DSB_ERROR(ERR_NETMSGTYPE,0);
	}

	messages[msgtype].cb = cb;
	return SUCCESS;
}

int dsb_net_send(int sock, int msgtype, void *msg)
{
	struct DSBNetHeader header;

	if (msgtype < 0 || msgtype >= DSBNET_TYPE_END)
	{
		return DSB_ERROR(ERR_NETMSGTYPE,0);
	}

	header.type = msgtype;
	header.chck = DSB_NET_CHECK;
	//TODO check for errors.
	send(sock, &header, sizeof(header),0);
	send(sock, msg, messages[msgtype].size,0);
	return SUCCESS;
}

int dsb_net_add(int sock)
{
	int i;

	for (i=0; i<MAX_CONNECTIONS; i++)
	{
		if (connections[i] == INVALID_SOCKET)
		{
			connections[i] = sock;
			return SUCCESS;
		}
	}

	return DSB_ERROR(ERR_NETMAX,0);
}

int dsb_net_disconnect(int sock)
{
	int i;
	for (i=0; i<MAX_CONNECTIONS; i++)
	{
		if (connections[i] == sock)
		{
			connections[i] = INVALID_SOCKET;
			//TODO MOVE ALL OTHER SOCKETS DOWN
			break;
		}
	}
	close(sock);
	return SUCCESS;
}

