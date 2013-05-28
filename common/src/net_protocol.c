/*
 * net_protocol.c
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

#include "dsb/net.h"
#include "dsb/net_protocol.h"
#include "dsb/errors.h"

static Event_t *readlist[MAX_READLIST];

int dsb_net_send_event(int sock, Event_t *e, int async)
{
	int ix = 0;
	int count = 0;
	//msg.evt = *evt;

	if (e->type == EVENT_GET)
	{
		//Find a spare slot
		for (ix=0; ix<(MAX_READLIST-1); ix++)
		{
			if (readlist[ix] == 0)
			{
				readlist[ix] = e;
				e->resid = ix;
				break;
			}
		}

		//WARNING: Run out of space for async reads so turn this
		//one into a sync read to clear.
		if (ix == MAX_READLIST-1)
		{
			async = 0;
			readlist[MAX_READLIST-1] = e;
			e->resid = MAX_READLIST-1;
		}
	}

	//Actually send the event
	dsb_net_send(sock, DSBNET_SENDEVENT, e);

	//Block if we need a response.
	if ((async == 0) && (e->type == EVENT_GET))
	{
		while (((e->flags & EVTFLAG_DONE) == 0) && count < 100)
		{
			dsb_net_poll(1);
			count++;
		}

		if ((e->flags & EVTFLAG_DONE) == 0)
		{
			return DSB_ERROR(ERR_NETTIMEOUT,0);
		}
	}
	return SUCCESS;
}

int dsb_net_cb_event(int sock, void *data)
{
	return SUCCESS;
}

int dsb_net_send_error(int sock, int err)
{
	struct DSBNetError e;
	e.err = err;
	dsb_net_send(sock, DSBNET_ERROR, &e);
	return SUCCESS;
}

int dsb_net_cb_error(int sock, void *data)
{
	struct DSBNetError *err = (struct DSBNetError*)data;
	DSB_ERROR(err->err,0);
	return SUCCESS;
}

int dsb_net_cb_result(int sock, void *data)
{
	struct DSBNetEventResult *res = (struct DSBNetEventResult*)data;
	Event_t *evt;

	//Is the ID valid
	if (res->id < 0 || res->id >= MAX_READLIST)
	{
		return DSB_ERROR(ERR_NETRESULT,0);
	}

	//Find event associated with ID.
	evt = readlist[res->id];
	if (evt == 0)
	{
		return DSB_ERROR(ERR_NETRESULT,0);
	}
	readlist[res->id] = 0;

	//Actually update value and mark event as complete.
	*(evt->res) = res->res;
	evt->flags |= EVTFLAG_DONE;

	return SUCCESS;
}
