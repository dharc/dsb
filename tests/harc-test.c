/*
 * harc-test.c
 *
 *  Created on: 7 May 2013
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
#include "dsb/core/harc.h"
#include "dsb/harc_d.h"
#include "dsb/core/nid.h"
#include "dsb/wrap.h"
#include "dsb/core/event.h"
#include "dsb/evaluator.h"
#include "dsb/errors.h"
#include "dsb/core/module.h"
#include "dsb/core/dependency.h"
#include <malloc.h>

Event_t lastevt;

int dsb_send(struct Event *evt)
{
	lastevt = *evt;
	return 0;
}

void test_harc_gen()
{

}

void test_harc_compare()
{

}

void test_harc_get()
{
	Event_t evt;
	HARC_t harc;
	NID_t res;

	//Initialise an event.
	evt.type = EVENT_GET;
	evt.flags = 0;
	evt.res = &res;
	dsb_iton(2,&(evt.d1));
	dsb_iton(3,&(evt.d2));

	//Initialise the hyperarc
	dsb_harc(&(evt.d1),&(evt.d2),&harc);
	dsb_iton(55,&(harc.def));

	CHECK(dsb_harc_event(&harc,&evt) == SUCCESS);
	CHECK((evt.flags & EVTFLAG_DONE) != 0);
	CHECK((harc.flags & HARC_OUTOFDATE) == 0);
	CHECK(harc.h.ll == 55);

	DONE;
}

void test_harc_define()
{
	Event_t evt;
	HARC_t harc;

	//Initialise an event.
	evt.type = EVENT_DEFINE;
	evt.flags = 0;
	dsb_iton(2,&(evt.d1));
	dsb_iton(3,&(evt.d2));
	dsb_iton(66,&(evt.def));

	//Initialise the hyperarc
	dsb_harc(&(evt.d1),&(evt.d2),&harc);
	dsb_iton(55,&(harc.def));
	harc.flags = 0;

	harc.deps = malloc(sizeof(Dependency_t));
	dsb_iton(8,&(harc.deps->a));
	dsb_iton(9,&(harc.deps->b));
	harc.deps->next = 0;

	CHECK(dsb_harc_event(&harc,&evt) == SUCCESS);
	CHECK((evt.flags & EVTFLAG_DONE) != 0);
	CHECK((harc.flags & HARC_OUTOFDATE) != 0);
	CHECK(harc.def.ll == 66);
	CHECK(lastevt.type == EVENT_NOTIFY);
	CHECK(lastevt.d1.ll == 8);
	CHECK(lastevt.d2.ll == 9);
	CHECK(harc.deps == 0);

	DONE;
}

void test_harc_notify()
{
	Event_t evt;
	HARC_t harc;

	//Initialise an event.
	evt.type = EVENT_NOTIFY;
	evt.flags = 0;
	dsb_iton(2,&(evt.d1));
	dsb_iton(3,&(evt.d2));

	//Initialise the hyperarc
	dsb_harc(&(evt.d1),&(evt.d2),&harc);
	harc.flags = 0;

	harc.deps = malloc(sizeof(Dependency_t));
	dsb_iton(10,&(harc.deps->a));
	dsb_iton(11,&(harc.deps->b));
	harc.deps->next = 0;

	CHECK(dsb_harc_event(&harc,&evt) == SUCCESS);
	CHECK((evt.flags & EVTFLAG_DONE) != 0);
	CHECK((harc.flags & HARC_OUTOFDATE) != 0);
	CHECK(lastevt.type == EVENT_NOTIFY);
	CHECK(lastevt.d1.ll == 10);
	CHECK(lastevt.d2.ll == 11);
	CHECK(harc.deps == 0);

	DONE;
}

void test_harc_dep()
{
	Event_t evt;
	HARC_t harc;

	//Initialise an event.
	evt.type = EVENT_DEP;
	evt.flags = 0;
	dsb_iton(2,&(evt.d1));
	dsb_iton(3,&(evt.d2));
	dsb_iton(5,&(evt.dep1));
	dsb_iton(6,&(evt.dep2));

	//Initialise the hyperarc
	dsb_harc(&(evt.d1),&(evt.d2),&harc);
	harc.flags = 0;

	CHECK(dsb_harc_event(&harc,&evt) == SUCCESS);
	CHECK((evt.flags & EVTFLAG_DONE) != 0);
	CHECK(harc.deps != 0);
	if (harc.deps != 0)
	{
		CHECK(harc.deps->a.ll == 5);
		CHECK(harc.deps->b.ll == 6);
		CHECK(harc.deps->next == 0);
	}

	DONE;
}

extern struct Module *dsb_evaluators_module();

int main(int argc, char *argv[])
{
	dsb_event_init();
	dsb_eval_init();

	//Load the evaluators module
	dsb_module_register("evaluators",dsb_evaluators_module());
	dsb_module_load("evaluators",0);

	dsb_test(test_harc_get);
	dsb_test(test_harc_define);
	dsb_test(test_harc_notify);
	dsb_test(test_harc_dep);

	dsb_eval_final();
	dsb_event_final();
	return 0;
}

