/*
 * system-perf.c
 *
 *  Created on: 7 Aug 2013
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

#include "dsb/core/module.h"
#include "dsb/errors.h"
#include "dsb/processor.h"
#include "dsb/router.h"
#include "dsb/names.h"
#include "dsb/common.h"
#include "dsb/globals.h"
#include "dsb/net_protocol.h"
#include "dsb/config.h"
#include <stdio.h>
#include <signal.h>
#include "dsb/core/thread.h"
#include "dsb/test.h"

//Internally compiled modules.
extern Module_t *dsb_volatile_module();
extern Module_t *dsb_persistent_module();

extern int dsb_send(Event_t *, bool);

#define AVS_COUNT_SMALL		10000
#define AVS_COUNT_LARGE		1000000

void sigint(int s)
{
	dsb_proc_stop();
}

static void test_system_avs()
{
	int i;
	long long startticks;
	long long diffticks;
	NID_t a,b;

	//Dummy sync read to make sure system is ready.
	dsb_getnni(&True,&True,&i);

	startticks = dsb_getTicks();

	for (i=0; i<AVS_COUNT_SMALL; i++)
	{
		Event_t *e;
		dsb_iton(i,&a);
		e = dsb_event(EVENT_SET,&a,&a,0);
		dsb_iton(i*2,&e->value);
		dsb_send(e,true);
	}

	dsb_iton(0,&a);
	dsb_get(&a,&a,&b);
	CHECK(b.ll == 0);

	diffticks = dsb_getTicks() - startticks;
	printf(" small count -- %f avs/s\n", (float)AVS_COUNT_SMALL / ((float)diffticks / (float)TICKS_PER_SECOND));

	for (i=100; i<200; i++)
	{
		dsb_iton(i,&a);
		dsb_get(&a,&a,&b);
		CHECK(b.ll == 2*i);
	}

	//----- NOW DO A LARGE NUMBER
	startticks = dsb_getTicks();

	for (i=0; i<AVS_COUNT_LARGE; i++)
	{
		Event_t *e;
		dsb_iton(i,&a);
		e = dsb_event(EVENT_SET,&a,&a,0);
		dsb_iton(i*2,&e->value);
		dsb_send(e,true);
	}

	dsb_iton(0,&a);
	dsb_get(&a,&a,&b);
	CHECK(b.ll == 0);

	diffticks = dsb_getTicks() - startticks;
	printf(" large count -- %f avs/s\n", (float)AVS_COUNT_LARGE / ((float)diffticks / (float)TICKS_PER_SECOND));

	for (i=100; i<200; i++)
	{
		dsb_iton(i,&a);
		dsb_get(&a,&a,&b);
		CHECK(b.ll == 2*i);
	}

	DONE;
}

static void *test_thread(void *arg)
{
	dsb_test(test_system_avs);

	dsb_proc_stop();

	return 0;
}

int main(int argc, char *argv[])
{
	pthread_t testt;

	//Initialise the common parts
	dsb_common_init();
	//dsb_proc_init();
	dsb_route_init();
	dsb_proc_init();

	//Register the internal modules
	dsb_module_register("volatile",dsb_volatile_module());
	dsb_module_register("persistent",dsb_persistent_module());
	//dsb_module_register("net",dsb_network_module());

	//These must exist or all else fails completely.
	dsb_module_load("volatile",&Root);
	dsb_module_load("persistent",&PRoot);

	//Set signal handler
	signal(SIGINT, sigint);

	pthread_create(&testt, 0, test_thread, 0);

	//Need to call all module update code.
	//Need to process queues until empty.
	dsb_proc_run(10);

	printf("Terminating...\n");

	pthread_exit(0);

	dsb_proc_final();
	dsb_common_final();
	return 0;
}
