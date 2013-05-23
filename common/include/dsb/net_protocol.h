/*
 * net_protocol.h
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

#ifndef NET_PROTOCOL_H_
#define NET_PROTOCOL_H_

#include "dsb/event.h"

/**
 * @file net_protocol.h
 * Structures for the DSB network protocol.
 */

/**
 * @addtogroup Net
 * @{
 */

#define DSB_NET_VERSION		1
#define DSB_NET_CHECK		0x9898

#define NET_MAX_EVENT_SEND	100

enum
{
	DSBNET_SENDEVENT = 1,
	DSBNET_EVENTRESULT,
	DSBNET_TYPE_END
};

struct DSBNetHeader
{
	unsigned short chck;	///< Check valid message.
	unsigned short type;	///< Message type.
};

struct DSBNetInfoReq
{
	//struct DSBNetHeader h;
	unsigned short proto_version;
	unsigned short major;
	unsigned short minor;
	unsigned short patch;
	//Other fields to be added later.
};

struct DSBNetInfoRep
{
	//struct DSBNetHeader h;
	unsigned short proto_version;
	unsigned short major;
	unsigned short minor;
	unsigned short patch;
	//Other fields to be added later.
};

struct DSBNetAuthReq
{
	struct DSBNetHeader h;
	char username[20];
	char password[20];
};

struct DSBNetAuthRep
{
	//struct DSBNetHeader h;
	unsigned int sig;	//New client signature.
};

struct DSBNetEventSend
{
	//struct DSBNetHeader h;
	Event_t evt;
};

struct DSBNetEventResult
{
	int id;
	NID_t res;
};

/**
 * Send a single event over the network. This function will not block, if the
 * event expects a return value then it will be updated as it is received from
 * the network, at some later date.
 *
 * @param sock Socket to send to.
 * @param e Event to send.
 * @return SUCCESS, ERR_SOCKET.
 */
int dsb_net_send_event(int sock, Event_t *e);

int dsb_net_cb_event(int sock, void *data);

int dsb_net_send_events(int sock, int count, Event_t *es);
int dsb_net_send_info(int sock);

/** @} */

#endif /* NET_PROTOCOL_H_ */
