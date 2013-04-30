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

struct RouterEntry
{
	int (*handler)(const struct Event *);
	struct NID l;
	struct NID h;
};

#define MAX_HANDLERS	20

struct RouterEntry route_table[MAX_HANDLERS];

int dsb_route_init(void)
{
	int i;
	for (i=0; i<MAX_HANDLERS; i++)
	{
		route_table[i].handler = 0;
	}
	return SUCCESS;
}

int dsb_route_final(void)
{
	return SUCCESS;
}

int dsb_route_map(const struct NID *l, const struct NID *h, int (*handler)(const struct Event *))
{
	//TODO Consider making threadsafe.
	int ff;
	for (ff=0; ff<MAX_HANDLERS; ff++)
	{
		if (route_table[ff].handler == 0) break;
	}

	if (ff == MAX_HANDLERS-1) return ERR_ROUTE_SLOT;

	route_table[ff].l = *l;
	route_table[ff].h = *h;
	route_table[ff].handler = handler;

	return SUCCESS;
}

int dsb_route(const struct Event *evt)
{
	int i;
	for (i=0; i<MAX_HANDLERS; i++)
	{
		//TODO Select correct destination NID from HARC.
		//Compare destination with low and high for each handler.
		if (dsb_nid_compare(&(route_table[i].l),&(evt->dest.a)) <= 0)
		{
			if (dsb_nid_compare(&(route_table[i].h),&(evt->dest.a)) >= 0)
			{
				if (route_table[i].handler != 0)
				{
					return route_table[i].handler(evt);
				}
				else
				{
					return ERR_ROUTE_MISSING;
				}
			}
		}
	}

	//Failed to find a matching handler.
	return ERR_ROUTE_UNKNOWN;
}
