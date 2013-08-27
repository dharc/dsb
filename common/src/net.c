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
#include <malloc.h>

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

#if defined(UNIX) && !defined(NO_THREADS)
static pthread_mutex_t net_poll_mtx = PTHREAD_MUTEX_INITIALIZER;
#endif //LINUX THREADED

/*
 * Represents a socket connection. Contains the receive buffer and index.
 */
struct DSBNetConnection
{
	int sockfd;
	char buffer[1000];
	int six;
};

static struct DSBNetConnection *connections[MAX_CONNECTIONS];
static fd_set fdread;
static fd_set fderror;
static int ssock = INVALID_SOCKET;

struct NetMessages
{
	//int size;
	int (*cb)(void*,void*);
};

static struct NetMessages messages[DSBNET_TYPE_END];

int dsb_net_init()
{
	//Initialise windows sockets
	#ifdef WIN32
	struct WSAData wsaData;
	if (WSAStartup(MAKEWORD(1,1), &wsaData) != 0) {
		//ERROR
		DSB_ERROR(ERR_NETCONNECT,"Windows Sockets");
		return ERR_NET;
	}
	#endif

	//Now insert default message handlers.
	messages[DSBNET_SENDEVENT].cb = dsb_net_cb_event;
	messages[DSBNET_EVENTRESULT].cb = dsb_net_cb_result;
	messages[DSBNET_ERROR].cb = dsb_net_cb_error;
	messages[DSBNET_BASE].cb = dsb_net_cb_base;
	messages[DSBNET_DEBUGGER].cb = dsb_net_cb_debugger;

	return 0;
}

int dsb_net_final()
{
	if (ssock)
	{
		close(ssock);
	}
	//TODO CLOSE ALL CONNECTIONS.
	return 0;
}

#pragma GCC diagnostic ignored "-Wunused-value"
static int net_accept()
{
	struct sockaddr_in remote;
	int csock;
	int rsize = sizeof(struct sockaddr_in);

	csock = accept(ssock, (struct sockaddr*)&remote, (socklen_t*)&rsize);

	if (csock == INVALID_SOCKET)
	{
		return DSB_ERROR(ERR_NETACCEPT, 0);
	}

	//Non-block from here.
	#ifdef UNIX
	fcntl(csock, F_SETFL, O_NONBLOCK);
	#endif

	DSB_INFO(INFO_NETACCEPT, 0);
	dsb_net_add(csock);
	return SUCCESS;
}

#pragma GCC diagnostic ignored "-Wunused-value"
int dsb_net_listen(int port)
{
	struct sockaddr_in localAddr;
	int rc;

	ssock = socket(AF_INET, SOCK_STREAM, 0);
	if (ssock == INVALID_SOCKET) {
		return DSB_ERROR(ERR_NET,0);
	}

	//Specify listen port and address
	//memset(&s_localAddr, 0, sizeof(s_localAddr));
	localAddr.sin_family = AF_INET;
	localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	localAddr.sin_port = htons(port);

	rc = bind(ssock, (struct sockaddr*)&localAddr, sizeof(localAddr));

	if (rc == SOCKET_ERROR) {
		#ifndef WIN32
		close(ssock);
		#else
		closesocket(ssock);
		#endif
		ssock = INVALID_SOCKET;
		return DSB_ERROR(ERR_NETLISTEN,0);
	}

	//Attempt to start listening for connection requests.
	rc = listen(ssock, 1);

	if (rc == SOCKET_ERROR) {
		#ifndef WIN32
		close(ssock);
		#else
		closesocket(ssock);
		#endif
		ssock = INVALID_SOCKET;
		return DSB_ERROR(ERR_NETLISTEN,0);
	}

	DSB_INFO(INFO_NETLISTEN,0);
	return SUCCESS;
}

void *dsb_net_connect(const char *url)
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
		return 0;
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
		return 0;
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
		return 0;
	}

	//Non-block from here.
	#ifdef UNIX
	fcntl(sock, F_SETFL, O_NONBLOCK);
	#endif

	//Now need to store connection socket.
	for (i=0; i<MAX_CONNECTIONS; i++)
	{
		if (connections[i] == 0)
		{
			connections[i] = malloc(sizeof(struct DSBNetConnection));
			connections[i]->sockfd = sock;
			connections[i]->six = 0;

			//Register my serial number with the remote machine
			dsb_net_send_base(connections[i]);

			return connections[i];
		}
	}

	#ifndef WIN32
	close(sock);
	#else
	closesocket(sock);
	#endif
	DSB_ERROR(ERR_NETCONNECT,url);
	return 0;
}

static int set_descriptors()
{
	int n = 0;
	int i;

	//Reset all file descriptors
	FD_ZERO(&fdread);
	FD_ZERO(&fderror);

	if (ssock)
	{
		FD_SET(ssock, &fdread);
		FD_SET(ssock, &fderror);
		n = ssock;
	}

	//Set the file descriptors for each client
	for (i=0; i<MAX_CONNECTIONS; i++) {
		#ifdef WIN32
		if ((connections[i] != 0) && (connections[i]->sockfd != INVALID_SOCKET)) {
		#else
		if ((connections[i] != 0) && (connections[i]->sockfd >= 0)) {
		#endif
			if (connections[i]->sockfd > n) {
				n = connections[i]->sockfd;
			}
			FD_SET(connections[i]->sockfd, &fdread);
			FD_SET(connections[i]->sockfd, &fderror);
		}
	}

	return n;
}

#pragma GCC diagnostic ignored "-Wunused-value"
static int read_messages(void *s)
{
	struct DSBNetConnection *sock = s;
	int rc;
	struct DSBNetHeader *header;
	int ix = 0;
	int repeat = 0;

	if (sock == 0) return SUCCESS;

	//Read into remaining buffer space.
	rc = recv(sock->sockfd, sock->buffer + sock->six, 1000 - sock->six, 0);
	if (rc == (1000-sock->six))
	{
		repeat = 1;
	}

	//No data to process.
	if (rc <= 0)
	{
		DSB_INFO(INFO_NETDISCONNECT,0);
		dsb_net_disconnect(sock);
		return SUCCESS;
	}

	//Add existing buffer contents to rc and reset index.
	rc += sock->six;
	sock->six = 0;
	ix = 0;

	//We have enough for a header... so repeatedly process until we don't
	while (rc >= sizeof(struct DSBNetHeader))
	{
		header = (struct DSBNetHeader*)(sock->buffer+ix);
		ix += sizeof(struct DSBNetHeader);

		//Make sure it is a valid message.
		if (header->chck != DSB_NET_CHECK)
		{
			return DSB_ERROR(ERR_NETMSGCHK,0);
		}
		if (header->type >= DSBNET_TYPE_END)
		{
			return DSB_ERROR(ERR_NETMSGTYPE,0);
		}

		//Do we have the entire message contents?
		if (rc-sizeof(struct DSBNetHeader) >= header->size)
		{
			//YES, so look for the callback
			if (messages[header->type].cb != 0)
			{
				messages[header->type].cb(sock, (void*)sock->buffer+ix);
			}
			else
			{
				DSB_ERROR(ERR_NETCB,0);
			}

			rc -= sizeof(struct DSBNetHeader)+header->size;
			ix += header->size;

			//Completed message so move to next.
			sock->six = ix;
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
		memcpy(sock->buffer, sock->buffer+sock->six, rc);
		//Set start index to next free buffer location.
		sock->six = rc;
		printf("Incomplete message: %d\n",rc);
	}
	else
	{
		sock->six = 0;
	}

	//Filled buffer on read so do another read?
	if (repeat == 1)
	{
		printf("Repeat read...\n");
		return read_messages(s);
	}
	else
	{
		return 0;
	}
}
//#pragma GCC diagnostic pop

#pragma GCC diagnostic ignored "-Wunused-value"
int dsb_net_poll(unsigned int ms)
{
	int n;
	struct timeval block;
	int selres;
	int i;

	#if defined(UNIX) && !defined(NO_THREADS)
	pthread_mutex_lock(&net_poll_mtx);
	#endif

	n = set_descriptors();
	//There are no connections to check.
	if (n == 0)
	{
		#if defined(UNIX) && !defined(NO_THREADS)
		pthread_mutex_unlock(&net_poll_mtx);
		#endif

		usleep(ms*1000);

		return SUCCESS;
	}

	//Timeout immediately.
	block.tv_sec = 0;
	block.tv_usec = ms * 1000;
	selres = select(n+1, &fdread, 0, &fderror, &block);

	//Some kind of error occurred, it is usually possible to recover from this.
	if (selres <= 0) {
		#if defined(UNIX) && !defined(NO_THREADS)
		pthread_mutex_unlock(&net_poll_mtx);
		#endif
		return SUCCESS;
	}

	if (ssock)
	{
		//Do we have a connection request?
		if (FD_ISSET(ssock, &fdread))
		{
			net_accept();
		}
	}

	//Also check each clients socket to see if any messages or errors are waiting
	for (i=0; i<MAX_CONNECTIONS; i++) {
		#ifdef WIN32
		if ((connections[i] != 0) && (connections[i]->sockfd >= 0)) {
		#else
		if ((connections[i] != 0) && (connections[i]->sockfd >= 0)) {
		#endif

			//If message received from this client then deal with it
			if (FD_ISSET(connections[i]->sockfd, &fdread)) {
				//Loop until no more messages to read.
				read_messages(connections[i]);
			//An error occurred with this client.
			} else if (FD_ISSET(connections[i]->sockfd, &fderror)) {
				//s_conns[i]->error();
				free(connections[i]);
				connections[i] = 0;
				DSB_INFO(INFO_NETDISCONNECT,0);
			}
		}
	}

	#if defined(UNIX) && !defined(NO_THREADS)
	pthread_mutex_unlock(&net_poll_mtx);
	#endif

	return 0;
}
//#pragma GCC diagnostic pop

int dsb_net_callback(int msgtype, int (*cb)(void*,void *))
{
	if (msgtype < 0 || msgtype >= DSBNET_TYPE_END)
	{
		return DSB_ERROR(ERR_NETMSGTYPE,0);
	}

	messages[msgtype].cb = cb;
	return SUCCESS;
}

int dsb_net_send(void *s, int msgtype, void *msg, int size)
{
	//Space for header should exist before msg buffer.
	//MAKE SURE TO HAVE USED dsb_net_buffer!!
	struct DSBNetHeader *header = msg-sizeof(struct DSBNetHeader);
	struct DSBNetConnection *sock = s;

	if (s == 0) return SUCCESS;
	if (msgtype < 0 || msgtype >= DSBNET_TYPE_END)
	{
		return DSB_ERROR(ERR_NETMSGTYPE,0);
	}

	header->type = msgtype;
	header->chck = DSB_NET_CHECK;
	header->size = size;

	//TODO check for errors.
	send(sock->sockfd, (const char*)header, size+sizeof(struct DSBNetHeader),0);
	free(header); //Release original buffer (see dsb_net_buffer).
	return SUCCESS;
}

void *dsb_net_add(int sock)
{
	int i;

	for (i=0; i<MAX_CONNECTIONS; i++)
	{
		if (connections[i] == 0)
		{
			connections[i] = malloc(sizeof(struct DSBNetConnection));
			connections[i]->sockfd = sock;
			connections[i]->six = 0;

			//Register my serial number with the remote machine
			dsb_net_send_base(connections[i]);

			return connections[i];
		}
	}

	return 0;
}

int dsb_net_disconnect(void *sock)
{
	int i;
	for (i=0; i<MAX_CONNECTIONS; i++)
	{
		if (connections[i] == sock)
		{
			#ifdef WIN32
			closesocket(connections[i]->sockfd);
			#else
			close(connections[i]->sockfd);
			#endif
			free(connections[i]);
			connections[i] = 0;
			break;
		}
	}
	return SUCCESS;
}

char *dsb_net_buffer(int size)
{
	return malloc(size+sizeof(struct DSBNetHeader)) + sizeof(struct DSBNetHeader);
}

