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

#ifdef __cplusplus
extern "C"
{
#endif

#define DSB_NET_VERSION		1
#define DSB_NET_CHECK		0x9898

#define NET_MAX_EVENT_SEND	100
#define MAX_READLIST		100

enum
{
	DSBNET_SENDEVENT = 1,
	DSBNET_EVENTRESULT,
	DSBNET_ERROR,
	DSBNET_DEBUGEVENT,
	DSBNET_DEBUGGER,
	DSBNET_LOGIN,
	DSBNET_ROOT,
	DSBNET_TYPE_END
};

struct DSBNetHeader
{
	unsigned short chck;	///< Check valid message.
	unsigned short type;	///< Message type.
	unsigned short size;
} __attribute__((__packed__));

/**
 * Send a single event over the network. This function will not block, if the
 * event expects a return value then it will be updated as it is received from
 * the network, at some later date.
 *
 * @param sock Socket to send to.
 * @param e Event to send.
 * @return SUCCESS, ERR_SOCKET.
 */
int dsb_net_send_event(void *sock, Event_t *evt, int async);

int dsb_net_send_result(void *sock, int id, const NID_t *res);
int dsb_net_send_error(void *sock, int error);
int dsb_net_send_login(void *sock, const char *user, const char *pass);
int dsb_net_send_root(void *sock, const NID_t *root);
int dsb_net_send_dbgevent(void *sock, Event_t *evt);
int dsb_net_send_debugger(void *sock, int flags);

int dsb_net_cb_event(void *sock, void *data);
int dsb_net_cb_result(void *sock, void *data);
int dsb_net_cb_error(void *sock, void *data);
int dsb_net_cb_login(void *sock, void *data);
int dsb_net_cb_root(void *sock, void *data);

int dsb_net_send_events(void *sock, int count, Event_t *es);
int dsb_net_send_info(void *sock);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* NET_PROTOCOL_H_ */
