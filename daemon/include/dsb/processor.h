/*
 * processor.h
 *
 *  Created on: 30 Apr 2013
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

/** @file processor.h */

#ifndef PROCESSOR_H_
#define PROCESSOR_H_

struct Event;

#define SYNC	0
#define ASYNC	1

int dsb_proc_init();
int dsb_proc_final();

int dsb_proc_debug(void *sock);

/**
 * Send an Event to be queued and processed.
 * @param evt Event to send.
 * @param async 0 blocks and 1 returns immediately.
 * @return SUCCESS, ERR_NOROUTE or ERR_INVALIDEVENT
 */
int dsb_proc_send(struct Event *evt, int async);

/**
 * Wait for an event to complete.
 * @param evt Event to wait for.
 * @return SUCCESS or ERR_NOTSENT.
 */
int dsb_proc_wait(const struct Event *evt);

int dsb_proc_single();

/**
 * Main event scheduler. Only returns when explicitly terminated.
 * @return
 */
int dsb_proc_run();

/**
 * Stops the currently running system. Causes dsb_proc_run to return. Should
 * be called from a signal or from within some definition/module.
 * @return SUCCESS
 */
int dsb_proc_stop();

#endif /* PROCESSOR_H_ */
