/*
 * math-test.c
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
#include "dsb/nid.h"
#include "dsb/event.h"
#include "dsb/router.h"
#include "dsb/module.h"
#include "dsb/specials.h"
#include "dsb/errors.h"

extern struct Module *dsb_math_module();

void test_math_add()
{
	struct Event evt;
	//struct NID a;
	//struct NID b;

	//Generate 55 +
	dsb_iton(55,&evt.d1);
	evt.d2.type = NID_SPECIAL;
	evt.d2.ll = SPECIAL_ADD;
	evt.type = EVENT_GET;
	evt.flags = 0;

	//Send 55 + get event
	CHECK(dsb_route(&evt) == SUCCESS);
	CHECK(evt.flags & EVTFLAG_DONE);
	CHECK(evt.res.type == NID_INTADD);
	CHECK(evt.res.ll == 55);

	//Generate 55+ 55
	evt.d2 = evt.res;
	evt.type = EVENT_GET;
	evt.flags = 0;

	//Send 55+ 55 get event
	CHECK(dsb_route(&evt) == SUCCESS);
	CHECK(evt.flags & EVTFLAG_DONE);
	CHECK(evt.res.type == NID_INTEGER);
	CHECK(evt.res.ll == 110);

	DONE;
}



int main(int argc, char *argv[])
{
	dsb_nid_init();
	dsb_event_init();
	dsb_route_init();

	//Load the math module
	dsb_module_register("math",dsb_math_module());
	dsb_module_load("math",0);

	dsb_test(test_math_add);

	dsb_route_final();
	dsb_event_final();
	dsb_nid_final();

	return 0;
}

