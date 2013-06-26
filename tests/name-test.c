/*
 * name-test.c
 *
 *  Created on: 5 Jun 2013
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
#include "dsb/core/nid.h"
#include "dsb/names.h"
#include "dsb/core/event.h"
#include "dsb/errors.h"

#include <string.h>

static int nidalloc = 0;

int dsb_send(struct Event *evt)
{
	if (evt->type == EVENT_ALLOCATE)
	{
		dsb_nid_local(NID_VOLATILE,evt->res);
		evt->res->n = nidalloc++;
	}
	evt->flags |= EVTFLAG_DONE;
	return 0;
}

void test_names_lookup()
{
	NID_t n1;
	const NID_t *n2;

	dsb_iton(33,&n1);

	CHECK(dsb_names_llookup("test1") == 0);
	CHECK(dsb_names_add("test1",&n1) == 0);
	n2 = dsb_names_llookup("test1");
	CHECK(n2 != 0);
	if (n2 != 0)
	{
		CHECK(dsb_ntoi(n2) == 33);
	}

	//Now another name.
	dsb_iton(44,&n1);
	CHECK(dsb_names_llookup("test2") == 0);
	CHECK(dsb_names_add("test2",&n1) == 0);
	n2 = dsb_names_llookup("test2");
	CHECK(n2 != 0);
	if (n2 != 0)
	{
		CHECK(dsb_ntoi(n2) == 44);
	}

	//Now an excessively long name.
	CHECK(dsb_names_add("this is an excessively long name for a NID that will hopefully break the names system because of an overflow",&n1) == ERR_NAMELEN);
	n2 = dsb_names_llookup("this is an excessively long name for a NID that will hopefully break the names system because of an overflow");
	CHECK(n2 == 0);

	DONE;
}

void test_names_revlookup()
{
	NID_t n;
	char buf[100];

	dsb_iton(66,&n);

	CHECK(dsb_names_add("hello",&n) == 0);
	CHECK(dsb_names_revlookup(&n,buf,100) == 0);
	CHECK(strcmp(buf,"hello") == 0);

	DONE;
}

void test_names_update()
{
	DONE;
}

int main(int argc, char *argv[])
{
	dsb_nid_init();
	dsb_event_init();
	dsb_names_init();

	dsb_test(test_names_lookup);
	dsb_test(test_names_revlookup);

	dsb_names_final();
	dsb_event_final();
	dsb_nid_final();

	return 0;
}

