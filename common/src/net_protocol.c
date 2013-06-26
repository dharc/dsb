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
#include "dsb/globals.h"
#include <string.h>
#include <stdio.h>

static Event_t *readlist[MAX_READLIST];

extern int dsb_send(struct Event *,int);


int dsb_net_send_dbgevent(void *sock, Event_t *evt)
{
	char *buffer = dsb_net_buffer(200);
	int count;
	//Pack the event.
	count = dsb_event_pack(evt, buffer, 100);
	//Actually send the event
	dsb_net_send(sock, DSBNET_DEBUGEVENT, buffer, count);
	return SUCCESS;
}

int dsb_net_send_event(void *sock, Event_t *e, int async)
{
	int ix = 0;
	int count = 0;
	char *buffer = dsb_net_buffer(200);
	//msg.evt = *evt;

	if ((e->type >> 8) == 0x1)
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

	//Pack the event to a buffer.
	count = dsb_event_pack(e, buffer, 100);

	//Actually send the event
	dsb_net_send(sock, DSBNET_SENDEVENT, buffer, count);

	//Block if we need a response.
	count = 0;
	if ((async == 0) && ((e->type >> 8) == 0x1))
	{
		while (((e->flags & EVTFLAG_DONE) == 0) && count < 100)
		{
			dsb_net_poll(10);
			count++;
		}

		if ((e->flags & EVTFLAG_DONE) == 0)
		{
			readlist[e->resid] = 0;
			*(e->res) = Null;
			e->flags |= EVTFLAG_DONE;
			return DSB_ERROR(ERR_NETTIMEOUT,0);
		}
	}
	return SUCCESS;
}

int dsb_net_cb_event(void *sock, void *data)
{
	int ret;
	Event_t *evt;
	int count;

	evt = dsb_event_allocate();
	count = dsb_event_unpack(data, evt);
	evt->flags = 0;

	//If READ we need to wait and send result.
	if ((evt->type >> 8) == 0x1)
	{
		NID_t res;
		evt->res = &res;
		ret = dsb_send(evt,0);
		if ((evt->flags & EVTFLAG_ERRO) == 0)
		{
			if (ret == SUCCESS)
			{
				dsb_net_send_result(sock, evt->resid, &res);
			}
			//else
			//{
			//	dsb_net_send_error(sock,ret,"");
			//}
			dsb_event_free(evt);
			return count;
		}
		else
		{
			//dsb_net_send_error(sock,evt->err,"");
			dsb_event_free(evt);
			return count;
		}
	}
	else
	{
		evt->flags |= EVTFLAG_FREE;
		dsb_send(evt,1);
		return count;
	}
}

/*int dsb_net_send_login(void *sock, const char *user, const char *pass)
{
	char buf[60];
	strcpy(buf,user);
	strcpy(&buf[30],pass);
	dsb_net_send(sock, DSBNET_LOGIN, buf, 60);
	return SUCCESS;
}*/

int dsb_net_send_base(void *sock)
{
	char *buf = dsb_net_buffer(200);
	int count = 0;
	NID_t root;
	NID_t proot;

	dsb_nid_local(NID_VOLATILE,&root);
	dsb_nid_local(NID_PERSISTENT,&proot);

	count = dsb_nid_pack(&root,buf,100);
	count += dsb_nid_pack(&proot,&buf[count],100);
	dsb_net_send(sock, DSBNET_BASE, buf, count);
	return SUCCESS;
}

int dsb_net_send_error(void *sock, int err, const char *msg)
{
	int len = strlen(msg);
	char *buffer = dsb_net_buffer(8+len+2);
	//*((int*)buffer) = err;
	memcpy(buffer,&err,sizeof(int));
	memcpy(buffer+sizeof(int),msg,len+1);
	dsb_net_send(sock, DSBNET_ERROR, buffer, sizeof(int)+len+1);
	return SUCCESS;
}

int dsb_net_send_debugger(void *sock, int flags)
{
	char *buffer = dsb_net_buffer(8);
	//*((int*)buffer) = flags;
	memcpy(buffer,&flags,sizeof(int));
	dsb_net_send(sock, DSBNET_DEBUGGER, buffer, sizeof(int));
	return SUCCESS;
}

void *dbgsocket=0;

static void log_handler(int err, const char *msg)
{
	if (msg == 0) msg = "";
	dsb_net_send_error(dbgsocket,err,msg);
}

int dsb_net_cb_debugger(void *sock, void *data)
{
	int flags;
	memcpy(&flags,data,sizeof(int));
	dbgsocket = sock;
	dsb_log_handler(log_handler);
	//dsb_debug(flags);
	return 0;
}

/*int dsb_net_cb_login(void *sock, void *data)
{
	//TODO check username and password.
	NID_t root;
	dsb_nid_local(1,&root);
	dsb_net_send_root(sock,&root);
	return SUCCESS;
}*/

int dsb_net_cb_base(void *sock, void *data)
{
	//TODO
	return SUCCESS;
}

int dsb_net_cb_error(void *sock, void *data)
{
	int errnum;
	int len;

	memcpy(&errnum,data,sizeof(int));
	data += sizeof(int);
	len = strlen(data);
	DSB_ERROR(errnum,(const char*)data);
	return sizeof(int)+len;
}

int dsb_net_send_result(void *sock, int id, const NID_t *res)
{
	char *buf = dsb_net_buffer(200);
	int count = sizeof(int);

	//(*(int*)buf) = id;
	memcpy(buf,&id,sizeof(int));
	count += dsb_nid_pack(res, buf+sizeof(int),100);
	dsb_net_send(sock, DSBNET_EVENTRESULT, buf, count);
	return SUCCESS;
}

int dsb_net_cb_result(void *sock, void *data)
{
	int resid; //= *((int*)data);
	NID_t res;
	Event_t *evt;
	int count = sizeof(int);

	memcpy(&resid,data,sizeof(int));

	count += dsb_nid_unpack(data+sizeof(int), &res);

	//Is the ID valid
	if (resid < 0 || resid >= MAX_READLIST)
	{
		return DSB_ERROR(ERR_NETRESULT,0);
	}

	//Find event associated with ID.
	evt = readlist[resid];
	if (evt == 0)
	{
		return DSB_ERROR(ERR_NETRESULT,0);
	}
	readlist[resid] = 0;

	//Actually update value and mark event as complete.
	*(evt->res) = res;
	evt->flags |= EVTFLAG_DONE;

	if (evt->flags & EVTFLAG_FREE)
	{
		dsb_event_free(evt);
	}

	return count;
}
