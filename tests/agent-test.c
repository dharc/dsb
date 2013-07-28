/*
 * agent-test.c
 *
 *  Created on: 26 Jun 2013
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
#include "dsb/errors.h"
#include "dsb/core/event.h"
#include "dsb/core/vm.h"
#include "dsb/assembler.h"
#include "dsb/core/module.h"
#include "dsb/router.h"
#include "dsb/globals.h"
#include "dsb/core/agent.h"
#include "dsb/wrap.h"
#include "dsb/common.h"
#include <stdlib.h>


int dsb_send(struct Event *evt)
{
	dsb_route(evt);
	return 0;
}

void test_agent_start()
{
	NID_t myagent;
	int res;

	dsb_new(&Root,&myagent);

	dsb_assemble(""
			"def root 0 5656\n"
			"dep root 1 [02:00:00:00:00:00:01:0000000000] [02:00:00:00:00:00:01:0000000000]\n"
			//"ret %0\n"
			, &myagent);

	dsb_agent_start(&myagent, 0);

	dsb_getnii(&Root,0,&res);
	CHECK(res == 5656);

	dsb_setnii(&Root,0,7676);
	dsb_getnii(&Root,0,&res);
	CHECK(res != 5656);

	dsb_setnii(&Root,1,50);
	dsb_getnii(&Root,0,&res);
	CHECK(res == 5656);

	DONE;
}

static int startxvar;

void myagent1(const NID_t *me, void *data)
{
	NID_t tmpint;
	int *myint = (int*)data;
	*myint = 55;

	dsb_iton(1,&tmpint);

	dsb_dependency(&Root,&tmpint,me,me);
}

void test_agent_startx()
{
	CHECK(dsb_agent_startx(myagent1,&startxvar) != -1);
	CHECK(startxvar == 55);

	startxvar = 77;

	dsb_setnii(&Root,1,56);

	CHECK(startxvar == 55);

	DONE;
}

extern Module_t *dsb_volatile_module();

int main(int argc, char *argv[])
{
	setenv("DSB_SERIAL","00:00:00:00:00:01",1);
	dsb_common_init();
	dsb_route_init();

	//Load the volatile module
	dsb_module_register("volatile",dsb_volatile_module());
	dsb_module_load("volatile",0);

	dsb_test(test_agent_start);
	dsb_test(test_agent_startx);

	dsb_route_final();
	dsb_common_final();

	return 0;
}

