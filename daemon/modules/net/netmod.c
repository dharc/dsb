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

#include "dsb/core/module.h"
#include "dsb/errors.h"
#include "dsb/core/nid.h"
#include "dsb/net.h"
#include "dsb/net_protocol.h"
#include "dsb/router.h"
#include <stdio.h>

Module_t netmod;

int net_msg_event(int sock, void *data);
int net_cb_base(void *sock, void *data);
//int net_debug_request(void *sock, void *data);
int net_handler(Event_t *evt);

static void *net_update(void *arg)
{
	while (1)
	{
		//Poll all connection sockets.
		dsb_net_poll(1000);
	}

	return 0;
}

int net_init(const NID_t *base)
{
	//Init common net code
	dsb_net_init();

	dsb_route_map(ROUTE_VOLATILE | ROUTE_REMOTE,0,net_handler);
	dsb_route_map(ROUTE_PERSISTENT | ROUTE_REMOTE,0,net_handler);
	dsb_net_callback(DSBNET_BASE,net_cb_base);
	//dsb_net_callback(DSBNET_DEBUGGER,net_debug_request);

	//Set up listener socket.
	dsb_net_listen(5555);

	pthread_t nett;
	pthread_create(&nett,0,net_update,0);

	return SUCCESS;
}

int net_final()
{
	dsb_net_final();
	return SUCCESS;
}

/*
 * Module registration structure.
 */
Module_t *dsb_network_module()
{
	netmod.init = net_init;
	netmod.update = 0;
	netmod.final = net_final;
	netmod.ups = 1000;
	return &netmod;
}

