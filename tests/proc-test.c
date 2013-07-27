/*
 * proc-test.c
 *
 *  Created on: 22 May 2013
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
#include "dsb/processor.h"
#include "dsb/core/event.h"
#include "dsb/errors.h"
#include "dsb/types.h"

#include "../daemon/src/processor.c"

void test_proc_send()
{
	Event_t evt;
	Event_t *t;

	evt.type = EVENT_GET;
	evt.err = 555;
	CHECK(dsb_proc_send(&evt,true) == SUCCESS);
	t = queue_pop(1);
	CHECK(t != 0);
	if (t != 0)
	{
		CHECK(t->err == 555);
	}

	evt.type = EVENT_DEFINE;
	evt.err = 777;
	CHECK(dsb_proc_send(&evt,true) == SUCCESS);
	t = queue_pop(0);
	CHECK(t != 0);
	if (t != 0)
	{
		CHECK(t->err == 777);
	}

	evt.type = EVENT_DEP;
	evt.err = 888;
	CHECK(dsb_proc_send(&evt,true) == SUCCESS);
	t = queue_pop(2);
	CHECK(t != 0);
	if (t != 0)
	{
		CHECK(t->err == 888);
	}

	DONE;
}

int main(int argc, char *argv[])
{
	dsb_nid_init();
	dsb_event_init();
	dsb_proc_init();

	dsb_test(test_proc_send);

	dsb_proc_final();
	dsb_event_final();
	dsb_nid_final();
	return 0;
}
