/*
 * router.c
 *
 *  Created on: 29 Apr 2013
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

#include "dsb/router.h"
#include "dsb/core/nid.h"
#include "dsb/core/event.h"
#include "dsb/errors.h"
#include "dsb/core/harc.h"
#include "dsb/core/agent.h"

static int (*localvolatile[16])(struct Event *);
static int (*localpersist[16])(struct Event *);
static int (*remote[16])(struct Event *);

int dsb_route_init(void)
{
	return SUCCESS;
}

int dsb_route_final(void)
{
	return SUCCESS;
}

int dsb_route_map(
		int flags, int num,
		int (*handler)(struct Event *))
{
	if (flags & ROUTE_REMOTE)
	{
		//TODO Use num later.
		remote[0] = handler;
	}
	else
	{
		if (flags & ROUTE_PERSISTENT)
		{
			localpersist[0] = handler;
		}
		else
		{
			localvolatile[0] = handler;
		}
	}

	return SUCCESS;
}

int dsb_route(struct Event *evt)
{
	int (*handler)(struct Event *);

	//Choose the correct handler...
	if (dsb_nid_isLocal(&evt->d1) == 0)
	{
		//TODO Use num later.
		handler = remote[0];
	}
	else
	{
		switch(evt->d1.header)
		{
		case NID_PERSISTENT:	handler = localpersist[0]; break;
		case NID_COMMON:
		case NID_VOLATILE:		handler = localvolatile[0]; break;
		case NID_AGENT:			dsb_agent_trigger((unsigned int)evt->d1.ll); break;

		default: return DSB_ERROR(WARN_NOROUTE,0);
		}
	}

	//Call the handler if it exists.
	if (handler != 0)
	{
		return handler(evt);
	}
	else
	{
		char buf[100];
		sprintf(buf,"serial = %02x:%02x:%02x:%02x:%02x:%02x\n",evt->d1.mac[0],evt->d1.mac[1],evt->d1.mac[2],evt->d1.mac[3],evt->d1.mac[4],evt->d1.mac[5]);
		return DSB_ERROR(ERR_ROUTE_MISSING,buf);
	}

	//Failed to find a matching handler.
	return DSB_ERROR(WARN_NOROUTE,0);
}
