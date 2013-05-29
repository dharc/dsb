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
#include "dsb/nid.h"
#include "dsb/event.h"
#include "dsb/errors.h"
#include "dsb/harc.h"

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
		if (evt->d1.persist == 1)
		{
			handler = localpersist[0];
		}
		else
		{
			handler = localvolatile[0];
		}
	}

	//Call the handler if it exists.
	if (handler != 0)
	{
		return handler(evt);
	}
	else
	{
		return DSB_ERROR(ERR_ROUTE_MISSING,0);
	}

	//Failed to find a matching handler.
	return DSB_ERROR(ERR_NOROUTE,0);
}
