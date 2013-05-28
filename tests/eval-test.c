/*
 * eval-test.c
 *
 *  Created on: 9 May 2013
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
#include "dsb/evaluator.h"
#include "dsb/nid.h"
#include "dsb/module.h"
#include "dsb/errors.h"
#include "dsb/event.h"
#include "dsb/specials.h"

unsigned int hasevaluated = 0;
unsigned int hassent = 0;

struct NID sizes[10];
struct NID defs[10][30];

/*
 * Create test definitions and hypergraphs.
 */
void init_test_defs()
{
	//Simple constant definition
	dsb_nid(NID_INTEGER,89,&(defs[0][0]));
	dsb_iton(1,&(sizes[0]));

	//Single lookup definition. (def 0 index 0 returns 89);
	dsb_nid(NID_INTEGER,0,&(defs[1][0]));
	dsb_nid(NID_INTEGER,0,&(defs[1][1]));
	dsb_iton(2,&(sizes[1]));
}

/*
 * DUMMY event send function to intercept internal events of the evaluator.
 * Needs to simulate a hypergraph for the definition to work against.
 */
int dsb_send(struct Event *evt)
{
	int whichdef;
	hassent = hassent+1;

	//Only consider GET events.
	if (evt->type == EVENT_GET)
	{
		whichdef = evt->d1.ll;

		//Is it asking for SIZE?
		if ((evt->d2.type == NID_SPECIAL) && (evt->d2.ll == SPECIAL_SIZE))
		{
			*(evt->res) = sizes[whichdef];
		}
		//Is it asking for an integer indexed element?
		else if (evt->d2.type == NID_INTEGER)
		{
			//If an invalid number then return NULL.
			if (evt->d2.ll < 0 || evt->d2.ll >= 30)
			{
				if (evt->res != 0)
				{
					evt->res->type = NID_SPECIAL;
					evt->res->ll = SPECIAL_NULL;
				}
			}
			//Otherwise return definition NID element at index.
			else
			{
				*(evt->res) = defs[whichdef][evt->d2.ll];
			}
		}
		else
		{
			evt->res->type = NID_SPECIAL;
			evt->res->ll = SPECIAL_NULL;
		}
	}

	evt->flags |= EVTFLAG_DONE;
	return SUCCESS;
}

int eval_test(struct HARC *harc)
{
	hasevaluated = 1;
	return SUCCESS;
}

void test_eval_regcall()
{
	HARC_t h;
	h.e = EVAL_CUSTOM+1;
	//The valid case
	CHECK(dsb_eval_register(EVAL_CUSTOM+1,eval_test) == SUCCESS);
	CHECK(dsb_eval_call(&h) == SUCCESS);
	CHECK(hasevaluated == 1);

	//Error cases.
	h.e = EVAL_MAX+1;
	CHECK(dsb_eval_register(-1,eval_test) == ERR_EVALID);
	CHECK(dsb_eval_register(EVAL_MAX+1,eval_test) == ERR_EVALID);
	CHECK(dsb_eval_call(0) == ERR_EVALID);
	CHECK(dsb_eval_call(&h) == ERR_EVALID);
	DONE;
}

void test_eval_basic()
{
	struct HARC harc;
	harc.e = EVAL_BASIC;

	//Check the constant definition
	dsb_nid(NID_INTEGER,0,&(harc.def));
	CHECK(dsb_eval_call(&harc) == SUCCESS);
	CHECK(hassent == 2);
	CHECK(harc.h.type == NID_INTEGER);
	CHECK(harc.h.ll == 89);

	//Check the single lookup definition
	dsb_nid(NID_INTEGER,1,&(harc.def));
	CHECK(dsb_eval_call(&harc) == SUCCESS);
	CHECK(hassent == 6);
	CHECK(harc.h.type == NID_INTEGER);
	CHECK(harc.h.ll == 89);

	DONE;
}

extern struct Module *dsb_evaluators_module();

int main(int argc, char *argv[])
{
	dsb_nid_init();
	dsb_event_init();
	dsb_eval_init();

	init_test_defs();

	//Load the evaluators module
	dsb_module_register("evaluators",dsb_evaluators_module());
	dsb_module_load("evaluators",0);

	dsb_test(test_eval_regcall);
	dsb_test(test_eval_basic);

	dsb_eval_final();
	dsb_event_final();
	dsb_nid_final();
	return 0;
}

