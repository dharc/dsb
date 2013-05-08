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

struct RouterEntry
{
	int (*handler)(struct Event *);
	struct NID x1;
	struct NID x2;
	struct NID y1;
	struct NID y2;
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

int dsb_route_map(
		const struct NID *x1,
		const struct NID *x2,
		const struct NID *y1,
		const struct NID *y2,
		int (*handler)(struct Event *))
{
	//TODO Consider making threadsafe.
	//TODO Check for overlapping node regions.
	//TODO optimise using 2D binary search.
	int ff;
	for (ff=0; ff<MAX_HANDLERS; ff++)
	{
		if (route_table[ff].handler == 0) break;
	}

	if (ff == MAX_HANDLERS-1) return ERR_ROUTE_SLOT;

	route_table[ff].x1 = *x1;
	route_table[ff].x2 = *x2;
	route_table[ff].y1 = *y1;
	route_table[ff].y2 = *y2;
	route_table[ff].handler = handler;

	return SUCCESS;
}

/*
 * Return 0 if n is within the range of a and b. 1 otherwise.
 */
inline int nid_withinrange(const struct NID *a, const struct NID *b, const struct NID *n)
{
	//NOTE: Assumption about a being less than b!!!!
	return ((dsb_nid_compare(a,n) <= 0) && (dsb_nid_compare(b,n) >= 0)) ? 0 : 1;
}

int dsb_route(struct Event *evt)
{
	//TODO Use a more efficient search strategy.
	//TODO Properly support regional events.
	int i;
	int t1,t2;
	for (i=0; i<MAX_HANDLERS; i++)
	{
		//Check first order is in the range
		t1 = nid_withinrange(&(route_table[i].x1),&(route_table[i].x2),&(evt->d1));
		t2 = nid_withinrange(&(route_table[i].y1),&(route_table[i].y2),&(evt->d2));
		if (t1 == 0 && t2 == 0)
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

		//For symmetry, check the alternative order.
		t1 = nid_withinrange(&(route_table[i].x1),&(route_table[i].x2),&(evt->d2));
		t2 = nid_withinrange(&(route_table[i].y1),&(route_table[i].y2),&(evt->d1));
		if (t1 == 0 && t2 == 0)
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

	//Failed to find a matching handler.
	return ERR_NOROUTE;
}
