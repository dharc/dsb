/*
 * netmod.c
 *
 *  Created on: 10 May 2013
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

#include "dsb/module.h"
#include "dsb/errors.h"
#include "dsb/nid.h"
#include "dsb/net.h"
#include "dsb/net_protocol.h"
#include "dsb/router.h"
#include <stdio.h>

struct Module netmod;
static int ssock = INVALID_SOCKET;
static fd_set fdread;
static fd_set fderror;

int net_msg_event(int sock, void *data);
int net_cb_base(void *sock, void *data);
int net_handler(Event_t *evt);


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

static int net_listen(int port)
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

int net_init(const NID_t *base)
{
	//Init common net code
	dsb_net_init();

	dsb_route_map(ROUTE_VOLATILE | ROUTE_REMOTE,0,net_handler);
	dsb_route_map(ROUTE_PERSISTENT | ROUTE_REMOTE,0,net_handler);
	dsb_net_callback(DSBNET_BASE,net_cb_base);

	//Set up listener socket.
	net_listen(5555);

	return SUCCESS;
}

int net_final()
{
	//Close listener socket.
	close(ssock);

	dsb_net_final();
	return SUCCESS;
}

int net_update()
{
	int selres;
	struct timeval block;

	//Are we listening
	if (ssock != INVALID_SOCKET)
	{
		//Poll listener socket
		FD_ZERO(&fdread);
		FD_ZERO(&fderror);
		FD_SET(ssock, &fdread);
		FD_SET(ssock, &fderror);

		block.tv_sec = 0;
		block.tv_usec = 0;
		selres = select(ssock+1, &fdread, 0, &fderror, &block);

		//Do we have a connection request?
		if (FD_ISSET(ssock, &fdread))
		{
			net_accept();
		}
	}

	//Poll all connection sockets.
	dsb_net_poll(0);

	return SUCCESS;
}

/*
 * Module registration structure.
 */
struct Module *dsb_network_module()
{
	netmod.init = net_init;
	netmod.update = net_update;
	netmod.final = net_final;
	return &netmod;
}

