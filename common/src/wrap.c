/*
 * wrap.c
 *
 *  Created on: 8 May 2013
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

#include "dsb/wrap.h"
#include "dsb/nid.h"
#include "dsb/event.h"

//Needs to be implemented elsewhere.
extern int dsb_send(struct Event *,int);

int dsb_get(const struct NID *d1, const struct NID *d2, struct NID *r)
{
	struct Event evt;
	int res;
	evt.d1 = *d1;
	evt.d2 = *d2;
	evt.flags = 0;
	evt.type = EVENT_GET;
	evt.res = r;
	res = dsb_send(&evt,0);
	return res;
}

int dsb_getA(const struct NID *d1, const struct NID *d2, struct NID *r)
{
	struct Event *evt = dsb_event_allocate();
	int res;
	evt->d1 = *d1;
	evt->d2 = *d2;
	evt->flags = EVTFLAG_FREE;
	evt->type = EVENT_GET;
	evt->res = r;
	res = dsb_send(evt,1);
	return res;
}

int dsb_new(const NID_t *base, NID_t *n)
{
	struct Event evt;
	int res;
	evt.d1 = *base;
	evt.d2 = *base;
	evt.flags = 0;
	evt.type = EVENT_ALLOCATE;
	evt.res = n;
	res = dsb_send(&evt,0);
	return res;
}

int dsb_set(const struct NID *d1, const struct NID *d2, const struct NID *v)
{
	return dsb_define(d1,d2,v,0);
}

int dsb_setnzz(const NID_t *d1, const char *d2, const char *v)
{
	NID_t n1;
	NID_t n2;

	dsb_nid_fromStr(d2,&n1);
	dsb_nid_fromStr(v,&n2);
	return dsb_set(d1,&n1,&n2);
}

int dsb_setzzz(const char *d1, const char *d2, const char *v)
{
	NID_t n1;
	NID_t n2;
	NID_t n3;

	dsb_nid_fromStr(d1,&n1);
	dsb_nid_fromStr(d2,&n2);
	dsb_nid_fromStr(v,&n3);
	return dsb_set(&n1,&n2,&n3);
}

int dsb_define(
		const struct NID *d1,
		const struct NID *d2,
		const struct NID *def,
		int eval
		)
{
	struct Event *evt = dsb_event_allocate();
	evt->d1 = *d1;
	evt->d2 = *d2;
	evt->def = *def;
	evt->eval = eval;
	evt->flags = EVTFLAG_FREE;
	evt->type = EVENT_DEFINE;
	return dsb_send(evt,1);
}

int dsb_notify(const struct NID *d1, const struct NID *d2)
{
	struct Event *evt = dsb_event_allocate();
	evt->d1 = *d1;
	evt->d2 = *d2;
	evt->flags = EVTFLAG_FREE;
	evt->type = EVENT_NOTIFY;
	return dsb_send(evt,1);
}

int dsb_dependency(
		const NID_t *d1,
		const NID_t *d2,
		const NID_t *dep1,
		const NID_t *dep2
		)
{
	struct Event *evt = dsb_event_allocate();
	evt->d1 = *d1;
	evt->d2 = *d2;
	evt->dep1 = *dep1;
	evt->dep2 = *dep2;
	evt->flags = EVTFLAG_FREE;
	evt->type = EVENT_DEP;
	return dsb_send(evt,1);
}
