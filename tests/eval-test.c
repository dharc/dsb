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

unsigned int hasevaluated = 0;

int eval_test(struct HARC *harc, void **data)
{
	hasevaluated = 1;
	return SUCCESS;
}

void test_eval_regcall()
{
	//The valid case
	CHECK(dsb_eval_register(EVAL_CUSTOM+1,eval_test) == SUCCESS);
	CHECK(dsb_eval_call(EVAL_CUSTOM+1,0,0) == SUCCESS);
	CHECK(hasevaluated == 1);

	//Error cases.
	CHECK(dsb_eval_register(-1,eval_test) == ERR_EVALID);
	CHECK(dsb_eval_register(EVAL_MAX+1,eval_test) == ERR_EVALID);
	CHECK(dsb_eval_call(-1,0,0) == ERR_EVALID);
	CHECK(dsb_eval_call(50,0,0) == ERR_NOEVAL);
	DONE;
}

void test_eval_basic()
{
	DONE;
}

extern struct Module *dsb_evaluators_module();

int main(int argc, char *argv[])
{
	dsb_nid_init();

	//Load the evaluators module
	dsb_module_register("evaluators",dsb_evaluators_module());
	dsb_module_load("evaluators",0);

	dsb_test(test_eval_regcall);
	dsb_test(test_eval_basic);

	dsb_nid_final();
	return 0;
}

