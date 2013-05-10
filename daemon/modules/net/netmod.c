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

#include "dsb/module.h"
#include "dsb/errors.h"
#include "dsb/nid.h"
#include <zmq.h>

struct Module netmod;

void *ctx;	//zmq context
void *sock; //zmq listening socket.

int net_init(const NID_t *base)
{
	int rc;

	//Set up listening socket if possible.
	//ctx = zmq_ctx_new ();
	ctx = zmq_init(1); //DEPRECATED
	sock = zmq_socket (ctx, ZMQ_REP);
	rc = zmq_bind (sock, "tcp://*:5555");

	if (rc != 0) return ERR_NETBIND;

	return SUCCESS;
}

int net_final()
{
	zmq_close(sock);
	//zmq_ctx_destroy(ctx);
	zmq_term(ctx); //DEPRECATED
	return SUCCESS;
}

int net_update()
{
	zmq_pollitem_t item;
	zmq_msg_t msg;
	int rc;

	while (1)
	{
		//Poll for message.
		item.socket = sock;
		item.events = ZMQ_POLLIN;
		rc = zmq_poll(&item,1,0);

		//Nothing left to process.
		//if (rc >= 0) break;
		if (item.revents != ZMQ_POLLIN) break;

		//Get the message
		zmq_msg_init(&msg);
		rc = zmq_recv(sock,&msg,0);

		//Process the message.

		zmq_msg_close(&msg);
	}

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

