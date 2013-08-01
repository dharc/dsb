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
#include "dsb/core/nid.h"
#include "dsb/core/event.h"
#include "dsb/errors.h"
#include "dsb/patterns/array.h"
#include "dsb/globals.h"

//Needs to be implemented elsewhere.
extern int dsb_send(struct Event *,int);

static void get_evt_cb(const Event_t *evt, const NID_t *res)
{
	*(NID_t*)evt->data = *res;
}

int dsb_get(const struct NID *d1, const struct NID *d2, struct NID *r)
{
	Event_t evt;
	int res;
	evt.d1 = *d1;
	evt.d2 = *d2;
	evt.flags = 0;
	evt.type = EVENT_GET;
	evt.data = r;
	evt.cb = get_evt_cb;
	res = dsb_send(&evt,0);
	return res;
}

int dsb_getzzi(const char *d1, const char *d2, int *r)
{
	NID_t n1;
	NID_t n2;
	NID_t r1;
	int ret;

	dsb_nid_fromStr(d1,&n1);
	dsb_nid_fromStr(d2,&n2);
	ret = dsb_get(&n1,&n2,&r1);
	*r = dsb_ntoi(&r1);
	return ret;
}

int dsb_getzii(const char *d1, int d2, int *r)
{
	NID_t n1;
	NID_t n2;
	NID_t r1;
	int ret;

	dsb_nid_fromStr(d1,&n1);
	dsb_iton(d2,&n2);
	ret = dsb_get(&n1,&n2,&r1);
	*r = dsb_ntoi(&r1);
	return ret;
}

int dsb_getnii(const NID_t *d1, int d2, int *r)
{
	NID_t n2;
	NID_t r1;
	int ret;

	dsb_iton(d2,&n2);
	ret = dsb_get(d1,&n2,&r1);
	if (r1.header != 0 || r1.t != NID_TYPE_INTEGER) return ERR_NOTINTEGER;
	*r = dsb_ntoi(&r1);
	return ret;
}

int dsb_getnni(const NID_t *d1, const NID_t *d2, int *r)
{
	NID_t r1;
	int ret;

	ret = dsb_get(d1,d2,&r1);
	if (r1.header != 0 || r1.t != NID_TYPE_INTEGER) return ERR_NOTINTEGER;
	*r = dsb_ntoi(&r1);
	return ret;
}

int dsb_getnzn(const NID_t *d1, const char *d2, NID_t *r)
{
	NID_t n2;

	dsb_nid_fromStr(d2,&n2);
	return dsb_get(d1,&n2,r);
}

int dsb_getzzn(const char *d1, const char *d2, NID_t *r)
{
	NID_t n1;
	NID_t n2;

	dsb_nid_fromStr(d1,&n1);
	dsb_nid_fromStr(d2,&n2);
	return dsb_get(&n1,&n2,r);
}

int dsb_getnin(const NID_t *d1, int d2, NID_t *r)
{
	NID_t n2;
	dsb_iton(d2,&n2);
	return dsb_get(d1,&n2,r);
}

int dsb_getnzi(const NID_t *d1, const char *d2, int *r)
{
	NID_t n2;
	NID_t r1;
	int ret;

	dsb_nid_fromStr(d2,&n2);
	ret = dsb_get(d1,&n2,&r1);
	if (r1.header != 0 || r1.t != NID_TYPE_INTEGER) return ERR_NOTINTEGER;
	*r = dsb_ntoi(&r1);
	return ret;
}

int dsb_getdef(
		const NID_t *d1,
		const NID_t *d2,
		NID_t *def
		)
{
	struct Event evt;
	int res;
	evt.d1 = *d1;
	evt.d2 = *d2;
	evt.flags = 0;
	evt.type = EVENT_GETDEF;
	evt.data = def;
	evt.cb = get_evt_cb;
	res = dsb_send(&evt,0);
	return res;
}

int dsb_getA(const struct NID *d1, const struct NID *d2, struct NID *r)
{
	struct Event *evt = dsb_event_allocate();
	int res;
	evt->d1 = *d1;
	evt->d2 = *d2;
	evt->flags = EFLAG_FREE;
	evt->type = EVENT_GET;
	evt->data = r;
	evt->cb = get_evt_cb;
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
	evt.data = n;
	evt.cb = get_evt_cb;
	res = dsb_send(&evt,0);
	return res;
}

int dsb_dict(const NID_t *d, const NID_t *n)
{
	NID_t dict;
	NID_t tmp;

	//Don't add integers to the dictionary...
	//These are iterated as arrays already!
	if (n->header == 0 && n->t == NID_TYPE_INTEGER) return 0;

	dsb_get(d,&Keys,&dict);

	//Make dictionary if it doesn't exist.
	if (dsb_nid_eq(&dict,&Null) == 1)
	{
		dsb_new(d,&dict);
		dsb_array_clear(&dict);
		dsb_set(d,&Keys,&dict);
	}

	//Only add if not already in the dictionary.
	dsb_get(&dict,n,&tmp);
	if (dsb_nid_eq(&tmp,&Null) == 1)
	{
		dsb_array_push(&dict,n);
		dsb_set(&dict,n,&dict);
	}
	return SUCCESS;
}

int dsb_dictnz(const NID_t *d, const char *n)
{
	NID_t nn;
	dsb_nid_fromStr(n,&nn);
	return dsb_dict(d,&nn);
}

int dsb_dictzz(const char *d, const char *n)
{
	NID_t nn;
	NID_t nd;
	dsb_nid_fromStr(n,&nn);
	dsb_nid_fromStr(d,&nd);
	return dsb_dict(&nd,&nn);
}

int dsb_set(const struct NID *d1, const struct NID *d2, const struct NID *v)
{
	struct Event *evt = dsb_event_allocate();
	evt->d1 = *d1;
	evt->d2 = *d2;
	evt->value = *v;
	evt->flags = EFLAG_FREE;
	evt->type = EVENT_SET;
	return dsb_send(evt,0);
}

int dsb_setnzz(const NID_t *d1, const char *d2, const char *v)
{
	NID_t n1;
	NID_t n2;

	dsb_nid_fromStr(d2,&n1);
	dsb_nid_fromStr(v,&n2);
	return dsb_set(d1,&n1,&n2);
}

int dsb_setnzn(const NID_t *d1, const char *d2, const NID_t *v)
{
	NID_t n1;

	dsb_nid_fromStr(d2,&n1);
	return dsb_set(d1,&n1,v);
}

int dsb_setnzi(const NID_t *d1, const char *d2, int v)
{
	NID_t n1;
	NID_t nv;

	dsb_iton(v,&nv);
	dsb_nid_fromStr(d2,&n1);
	return dsb_set(d1,&n1,&nv);
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

int dsb_setnni(const NID_t *d1, const NID_t *d2, int v)
{
	NID_t r1;
	dsb_iton(v,&r1);
	return dsb_set(d1,d2,&r1);
}

int dsb_setnin(const NID_t *d1, int d2, const NID_t *v)
{
	NID_t n2;
	dsb_iton(d2,&n2);
	return dsb_set(d1,&n2,v);
}

int dsb_setnii(const NID_t *d1, int d2, int v)
{
	NID_t n2;
	NID_t nv;

	dsb_iton(d2,&n2);
	dsb_iton(v,&nv);
	return dsb_set(d1,&n2,&nv);
}

int dsb_define(
		const struct NID *d1,
		const struct NID *d2,
		const struct NID *def
		)
{
	struct Event *evt = dsb_event_allocate();
	evt->d1 = *d1;
	evt->d2 = *d2;
	evt->value = *def;
	evt->flags = EFLAG_FREE;
	evt->type = EVENT_DEFINE;
	return dsb_send(evt,0);
}

int dsb_notify(const struct NID *d1, const struct NID *d2)
{
	struct Event *evt = dsb_event_allocate();
	evt->d1 = *d1;
	evt->d2 = *d2;
	evt->flags = EFLAG_FREE;
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
	evt->flags = EFLAG_FREE;
	evt->type = EVENT_DEP;
	return dsb_send(evt,1);
}

int dsb_dependencynznn(
		const NID_t *d1,
		const char *d2,
		const NID_t *dep1,
		const NID_t *dep2
		)
{
	NID_t n1;
	dsb_nid_fromStr(d2,&n1);
	return dsb_dependency(d1,&n1,dep1,dep2);
}
