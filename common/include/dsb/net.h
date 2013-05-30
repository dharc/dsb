/*
 * net.h
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

#ifndef NET_H_
#define NET_H_

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @addtogroup Net
 * @{
 */

/**
 * Initialise common network code. Must be called before any network
 * functions are used.
 * @return SUCCESS
 */
int dsb_net_init();

/**
 * Finalise common network code.
 * @return
 */
int dsb_net_final();

/**
 * Connect to a dsbd server. If successful the connection is added to
 * available connections and set as default.
 * @param url address[:port], may be ip or host name.
 * @return The socket object, or 0 on error.
 */
void *dsb_net_connect(const char *url);

/**
 * Manually add socket to available connections. Used by dsbd module which
 * receives connection requests.
 * @param sock Socket handle.
 * @return DSB Socket object.
 */
void *dsb_net_add(int sock);

int dsb_net_disconnect(void *sock);

/**
 * Send a message to a socket. Will determine message size automatically.
 * @param sock Socket.
 * @param msg Must contain a struct DSBNetHeader.
 * @param size Size of the message data.
 * @return SUCCESS.
 */
int dsb_net_send(void *sock, int msgtype, void *msg, int size);
int dsb_net_poll(unsigned int ms);
int dsb_net_callback(int msgtype, int (*cb)(void*,void *));

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* NET_H_ */
