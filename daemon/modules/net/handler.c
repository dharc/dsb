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

#include <stdio.h>

extern int dsb_send(struct Event *,int);

int net_msg_event(int sock, void *data)
{
	int ret;
	Event_t *evt = data;
	evt->flags = 0;

	printf("Net event = %d\n", evt->type);

	//If GET we need to wait and send result.
	if (evt->type == EVENT_GET)
	{
		struct DSBNetEventResult res;
		ret = dsb_send(evt,0);
		printf("Net get event = %d\n",(int)evt->res.ll);
		res.res = evt->res;
		res.id = evt->eval;
		if (ret == SUCCESS)
		{
			dsb_net_send(sock, DSBNET_EVENTRESULT,&res);
		}
		else
		{
			dsb_net_send_error(sock,ret);
		}
		return SUCCESS;
	}
	else
	{
		Event_t *evt2 = dsb_event_allocate();
		*evt2 = *evt;
		evt2->flags |= EVTFLAG_FREE;
		return dsb_send(evt,1);
	}
}
