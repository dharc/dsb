/*
 * handler.c
 *
 *  Created on: 23 May 2013
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

#include "dsb/event.h"
#include "dsb/net.h"
#include "dsb/net_protocol.h"
#include "dsb/errors.h"
#include "dsb/processor.h"

#include <stdio.h>
#include <malloc.h>
#include <string.h>

#define MAX_NET_TABLE	1000

struct NetRouteEntry
{
	NID_t base;
	void *sock;
	struct NetRouteEntry *next;
};

static struct NetRouteEntry *routetable[MAX_NET_TABLE];

/*
 * Hash the 6 byte serial number into an int between 0 and MAX_NET_TABLE.
 */
static int hashserial(const char *serial)
{
	int res = *((int*)&serial[2]);
	return res % MAX_NET_TABLE;
}

/*
 * Register the socket with a particular serial number for routing.
 */
int net_cb_base(void *sock, void *data)
{
	struct NetRouteEntry *ent;
	int count = 0;
	int hash;

	ent = malloc(sizeof(struct NetRouteEntry));
	ent->sock = sock;

	count += dsb_nid_unpack(data, &ent->base);
	//Currently ignore difference between volatile and persistent.
	//count += dsb_nid_unpack(data+count,&pent->base);

	hash = hashserial((const char*)ent->base.mac);
	ent->next = routetable[hash];
	routetable[hash] = ent;

	printf("Route: %x:%x:%x:%x:%x:%x\n",ent->base.mac[0], ent->base.mac[1], ent->base.mac[2], ent->base.mac[3], ent->base.mac[4], ent->base.mac[5]);

	return SUCCESS;
}

int net_debug_request(void *sock, void *data)
{
	int flags = *((int*)data);

	if (flags & NET_DEBUG_QUEUES)
	{
		dsb_proc_debug(sock);
	}

	return 0;
}

/*
 * Route the event to the correct machine.
 */
int net_handler(Event_t *evt)
{
	struct NetRouteEntry *ent;
	int hash;

	printf("Remote event routing to: %x:%x:%x:%x:%x:%x\n",evt->d1.mac[0], evt->d1.mac[1], evt->d1.mac[2], evt->d1.mac[3], evt->d1.mac[4], evt->d1.mac[5]);

	hash = hashserial((const char*)evt->d1.mac);
	ent = routetable[hash];
	while (ent != 0)
	{
		if (memcmp(evt->d1.mac,ent->base.mac,6) == 0)
		{
			dsb_net_send_event(ent->sock,evt,1);
			return 0;
		}

		ent = ent->next;
	}

	return DSB_ERROR(ERR_NOROUTE,0);
}

