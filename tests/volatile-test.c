/*
 * volatile-test.c
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

#include "dsb/test.h"
#include "dsb/nid.h"
#include "dsb/event.h"
#include "dsb/router.h"
#include "dsb/module.h"
#include "dsb/specials.h"

int dsb_send(struct Event *evt)
{
	return 0;
}

void test_vol_getset()
{
	struct Event evt;
	struct NID a;
	struct NID b;
	NID_t res;

	//Create tail NIDs
	dsb_nid(NID_SPECIAL,SPECIAL_TRUE,&a);
	dsb_nid(NID_INTEGER,1,&b);

	//Generate DEFINE event.
	dsb_event(EVENT_DEFINE,&a,&b,&evt);
	dsb_nid(NID_INTEGER,55,&(evt.def));
	evt.eval = 0; //No evaluator

	//Send DEFINE event.
	CHECK(dsb_route(&evt) == 0);

	//Generate GET event;
	dsb_event(EVENT_GET,&a,&b,&evt);
	evt.res = &res;

	//Send GET event.
	CHECK(dsb_route(&evt) == 0);

	//Check result of GET.
	CHECK((evt.flags & EVTFLAG_DONE) != 0);
	CHECK(res.t == NID_INTEGER);
	CHECK(res.ll == 55);

	DONE;
}

void test_vol_region()
{
	struct Event evt;
	struct NID a;
	struct NID b;
	NID_t res;

	//Create tail NIDs
	dsb_nid(NID_SPECIAL,SPECIAL_TRUE,&a);
	dsb_nid(NID_INTEGER,2,&b);

	//Generate DEFINE event.
	dsb_event(EVENT_DEFINE,&a,&b,&evt);
	dsb_nid(NID_INTEGER,66,&(evt.def));
	dsb_nid(NID_SPECIAL,SPECIAL_TRUE,&(evt.d1b));
	dsb_nid(NID_INTEGER,50,&(evt.d2b));
	evt.eval = 0; //No evaluator
	evt.flags |= EVTFLAG_MULT;

	//Send DEFINE event.
	CHECK(dsb_route(&evt) == 0);

	//Generate GET event;
	dsb_event(EVENT_GET,&a,&b,&evt);
	evt.res = &res;

	//Send GET event.
	CHECK(dsb_route(&evt) == 0);

	//Check result of GET.
	CHECK((evt.flags & EVTFLAG_DONE) != 0);
	CHECK(res.t == NID_INTEGER);
	CHECK(res.ll == 66);

	//Generate GET event;
	b.ll = 20;
	dsb_event(EVENT_GET,&a,&b,&evt);
	evt.res = &res;

	//Send GET event.
	CHECK(dsb_route(&evt) == 0);

	//Check result of GET.
	CHECK((evt.flags & EVTFLAG_DONE) != 0);
	CHECK(res.t == NID_INTEGER);
	CHECK(res.ll == 66);
	DONE;
}

extern struct Module *dsb_volatile_module();

int main(int argc, char *argv[])
{
	dsb_nid_init();
	dsb_event_init();
	dsb_route_init();

	//Load the volatile module
	dsb_module_register("volatile",dsb_volatile_module());
	dsb_module_load("volatile",0);

	dsb_test(test_vol_getset);
	dsb_test(test_vol_region);

	dsb_route_final();
	dsb_event_final();
	dsb_nid_final();

	return 0;
}

