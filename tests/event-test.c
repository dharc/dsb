/*
 * event-test.c
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

#include "dsb/test.h"
#include "dsb/core/event.h"

int dsb_send(struct Event *evt)
{
	return 0;
}

void test_event_allocate()
{
	struct Event *evt;
	struct Event *evt2;

	CHECK(dsb_event_init() == 0);
	evt = dsb_event_allocate();
	CHECK(evt != 0);
	evt2 = dsb_event_allocate();
	CHECK(evt != evt2);
	//CHECK((evt2 - evt) == sizeof(struct Event));
	CHECK(dsb_event_final() == 0);

	DONE;
}

void test_event_packunpack()
{
	Event_t source;
	Event_t result;
	char buffer[200];

	//----- GET -----
	source.type = EVENT_GET;
	dsb_iton(44,&source.d1);
	dsb_iton(55,&source.d2);
	source.cb = 0;

	CHECK(dsb_event_pack(&source,buffer,200) == 28);
	CHECK(dsb_event_unpack(buffer, &result) == 28);

	CHECK(result.type == EVENT_GET);
	CHECK(result.d1.ll == 44);
	CHECK(result.d2.ll == 55);
	CHECK(result.cb == 0);

	//----- DEFINE -----
	source.type = EVENT_DEFINE;
	dsb_iton(66,&source.d1);
	dsb_iton(77,&source.d2);
	dsb_iton(88,&source.value);

	CHECK(dsb_event_pack(&source,buffer,200) == 40);
	CHECK(dsb_event_unpack(buffer, &result) == 40);

	CHECK(result.type == EVENT_DEFINE);
	CHECK(result.value.ll == 88);

	//----- DEP -----
	source.type = EVENT_DEP;
	dsb_iton(99,&source.d1);
	dsb_iton(111,&source.d2);
	dsb_iton(222,&source.dep1);
	dsb_iton(333,&source.dep2);

	CHECK(dsb_event_pack(&source,buffer,200) == 52);
	CHECK(dsb_event_unpack(buffer, &result) == 52);

	CHECK(result.type == EVENT_DEP);
	CHECK(result.dep1.ll == 222);
	CHECK(result.dep2.ll == 333);

	DONE;
}

void test_event_params()
{
	DONE;
}

int main(int argc, char *argv[])
{
	//dsb_event_init();
	dsb_test(test_event_allocate);
	dsb_test(test_event_params);
	dsb_test(test_event_packunpack);
	//dsb_event_final();
	return 0;
}
