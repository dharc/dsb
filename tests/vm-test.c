/*
 * vm-test.c
 *
 *  Created on: 30 May 2013
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
#include "dsb/globals.h"
#include "dsb/xfunc.h"

static NID_t evtd1;
static NID_t evtd2;
static NID_t evtres;

int vm_interpret(HARC_t *harc, NID_t *code, int maxip);

/*
 * DUMMY event send function to intercept internal events of the evaluator.
 * Needs to simulate a hypergraph for the definition to work against.
 */
int dsb_send(struct Event *evt)
{
	if (evt->type == EVENT_GET)
	{
		dsb_iton(666,evt->res);
		evt->flags |= EVTFLAG_DONE;
	}
	else if (evt->type == EVENT_DEFINE)
	{
		evtres = evt->def;
		evtd1 = evt->d1;
		evtd2 = evt->d2;
	}
	return 0;
}

void test_vm_copy()
{
	struct HARC harc;
	NID_t code[100];
	struct VMContext ctx;
	ctx.ip = 0;
	ctx.timeout = 1000;
	ctx.code = code;
	ctx.codesize = 100;
	ctx.result = &harc.h;

	//Actual code.
	dsb_nid_op(VM_CPY(1,0),&code[0]);
	dsb_iton(50,&code[1]);
	dsb_nid_op(VM_RET(1),&code[2]);

	CHECK(dsb_vm_interpret(&ctx) == -1);
	CHECK(harc.h.ll == 50);

	//Actual code.
	dsb_nid_op(VM_CPY(1,0),&code[0]);
	dsb_iton(54,&code[1]);
	dsb_nid_op(VM_CPY(2,1),&code[2]);
	dsb_nid_op(VM_RET(2),&code[3]);

	ctx.ip = 0;
	ctx.timeout = 1000;
	CHECK(dsb_vm_interpret(&ctx) == -1);
	CHECK(harc.h.ll == 54);

	DONE;
}

void test_vm_get()
{
	struct HARC harc;
	NID_t code[100];
	struct VMContext ctx;
	ctx.ip = 0;
	ctx.timeout = 1000;
	ctx.code = code;
	ctx.codesize = 100;
	ctx.result = &harc.h;

	//Actual code.
	dsb_nid_op(VM_CPY(1,0),&code[0]);
	dsb_iton(50,&code[1]);
	dsb_nid_op(VM_GET(2,1,0),&code[2]);
	dsb_iton(51,&code[3]);
	dsb_nid_op(VM_RET(2),&code[4]);

	CHECK(dsb_vm_interpret(&ctx) == -1);
	CHECK(harc.h.ll == 666);

	DONE;
}

void test_vm_def()
{
	struct HARC harc;
	NID_t code[100];
	struct VMContext ctx;
	ctx.ip = 0;
	ctx.timeout = 1000;
	ctx.code = code;
	ctx.codesize = 100;
	ctx.result = &harc.h;

	dsb_nid_op(VM_DEF(0,0,0),&code[0]);
	dsb_iton(50,&code[1]);
	dsb_iton(51,&code[2]);
	dsb_iton(52,&code[3]);
	dsb_nid_op(VM_RET(0),&code[4]);
	dsb_iton(53,&code[5]);

	CHECK(dsb_vm_interpret(&ctx) == -1);
	CHECK(evtd1.ll == 50);
	CHECK(evtd2.ll == 51);
	CHECK(evtres.ll == 52);

	DONE;
}

void test_vm_jeq()
{
	struct HARC harc;
	NID_t code[100];
	struct VMContext ctx;
	ctx.ip = 0;
	ctx.timeout = 1000;
	ctx.code = code;
	ctx.codesize = 100;
	ctx.result = &harc.h;

	//Params are equal
	dsb_nid_op(VM_JEQ(5,0,0),&code[0]);
	dsb_iton(50,&code[1]);
	dsb_iton(50,&code[2]);
	dsb_nid_op(VM_RET(0),&code[3]);
	dsb_iton(66,&code[4]);
	dsb_nid_op(VM_RET(0),&code[5]);
	dsb_iton(77,&code[6]);

	CHECK(dsb_vm_interpret(&ctx) == -1);
	CHECK(harc.h.ll == 77);

	//Params are not equal
	dsb_nid_op(VM_JEQ(5,0,0),&code[0]);
	dsb_iton(50,&code[1]);
	dsb_iton(51,&code[2]);
	dsb_nid_op(VM_RET(0),&code[3]);
	dsb_iton(66,&code[4]);
	dsb_nid_op(VM_RET(0),&code[5]);
	dsb_iton(77,&code[6]);

	ctx.ip = 0;
	ctx.timeout = 1000;
	CHECK(dsb_vm_interpret(&ctx) == -1);
	CHECK(harc.h.ll == 66);

	//Params are equal with variable
	dsb_nid_op(VM_CPY(1,0),&code[0]);
	dsb_iton(50,&code[1]);
	dsb_nid_op(VM_JEQ(6,1,0),&code[2]);
	dsb_iton(50,&code[3]);
	dsb_nid_op(VM_RET(0),&code[4]);
	dsb_iton(66,&code[5]);
	dsb_nid_op(VM_RET(0),&code[6]);
	dsb_iton(77,&code[7]);

	ctx.ip = 0;
	ctx.timeout = 1000;
	CHECK(dsb_vm_interpret(&ctx) == -1);
	CHECK(harc.h.ll == 77);

	//Params are equal with variable
	dsb_nid_op(VM_CPY(1,0),&code[0]);
	dsb_iton(50,&code[1]);
	dsb_nid_op(VM_JEQ(6,0,1),&code[2]);
	dsb_iton(50,&code[3]);
	dsb_nid_op(VM_RET(0),&code[4]);
	dsb_iton(66,&code[5]);
	dsb_nid_op(VM_RET(0),&code[6]);
	dsb_iton(77,&code[7]);

	ctx.ip = 0;
	ctx.timeout = 1000;
	CHECK(dsb_vm_interpret(&ctx) == -1);
	CHECK(harc.h.ll == 77);

	DONE;
}

void test_vm_jne()
{
	struct HARC harc;
	NID_t code[100];
	struct VMContext ctx;
	ctx.ip = 0;
	ctx.timeout = 1000;
	ctx.code = code;
	ctx.codesize = 100;
	ctx.result = &harc.h;

	//Params are equal
	dsb_nid_op(VM_JNE(5,0,0),&code[0]);
	dsb_iton(50,&code[1]);
	dsb_iton(50,&code[2]);
	dsb_nid_op(VM_RET(0),&code[3]);
	dsb_iton(66,&code[4]);
	dsb_nid_op(VM_RET(0),&code[5]);
	dsb_iton(77,&code[6]);

	CHECK(dsb_vm_interpret(&ctx) == -1);
	CHECK(harc.h.ll == 66);

	//Params are not equal
	dsb_nid_op(VM_JNE(5,0,0),&code[0]);
	dsb_iton(50,&code[1]);
	dsb_iton(51,&code[2]);
	dsb_nid_op(VM_RET(0),&code[3]);
	dsb_iton(66,&code[4]);
	dsb_nid_op(VM_RET(0),&code[5]);
	dsb_iton(77,&code[6]);

	ctx.ip = 0;
	ctx.timeout = 1000;
	CHECK(dsb_vm_interpret(&ctx) == -1);
	CHECK(harc.h.ll == 77);

	//Params are equal with variable
	dsb_nid_op(VM_CPY(1,0),&code[0]);
	dsb_iton(50,&code[1]);
	dsb_nid_op(VM_JNE(6,1,0),&code[2]);
	dsb_iton(50,&code[3]);
	dsb_nid_op(VM_RET(0),&code[4]);
	dsb_iton(66,&code[5]);
	dsb_nid_op(VM_RET(0),&code[6]);
	dsb_iton(77,&code[7]);

	ctx.ip = 0;
	ctx.timeout = 1000;
	CHECK(dsb_vm_interpret(&ctx) == -1);
	CHECK(harc.h.ll == 66);

	//Params are equal with variable
	dsb_nid_op(VM_CPY(1,0),&code[0]);
	dsb_iton(50,&code[1]);
	dsb_nid_op(VM_JNE(6,0,1),&code[2]);
	dsb_iton(50,&code[3]);
	dsb_nid_op(VM_RET(0),&code[4]);
	dsb_iton(66,&code[5]);
	dsb_nid_op(VM_RET(0),&code[6]);
	dsb_iton(77,&code[7]);

	ctx.ip = 0;
	ctx.timeout = 1000;
	CHECK(dsb_vm_interpret(&ctx) == -1);
	CHECK(harc.h.ll == 66);

	DONE;
}

void test_vm_jle()
{
	struct HARC harc;
	NID_t code[100];
	struct VMContext ctx;
	ctx.ip = 0;
	ctx.timeout = 1000;
	ctx.code = code;
	ctx.codesize = 100;
	ctx.result = &harc.h;

	//Params are lequal
	dsb_nid_op(VM_JLE(5,0,0),&code[0]);
	dsb_iton(40,&code[1]);
	dsb_iton(50,&code[2]);
	dsb_nid_op(VM_RET(0),&code[3]);
	dsb_iton(66,&code[4]);
	dsb_nid_op(VM_RET(0),&code[5]);
	dsb_iton(77,&code[6]);

	CHECK(dsb_vm_interpret(&ctx) == -1);
	CHECK(harc.h.ll == 77);

	//Params are not lequal
	dsb_nid_op(VM_JLE(5,0,0),&code[0]);
	dsb_iton(50,&code[1]);
	dsb_iton(45,&code[2]);
	dsb_nid_op(VM_RET(0),&code[3]);
	dsb_iton(66,&code[4]);
	dsb_nid_op(VM_RET(0),&code[5]);
	dsb_iton(77,&code[6]);

	ctx.ip = 0;
	ctx.timeout = 1000;
	CHECK(dsb_vm_interpret(&ctx) == -1);
	CHECK(harc.h.ll == 66);

	//Params are lequal with variable
	dsb_nid_op(VM_CPY(1,0),&code[0]);
	dsb_iton(40,&code[1]);
	dsb_nid_op(VM_JLE(6,1,0),&code[2]);
	dsb_iton(50,&code[3]);
	dsb_nid_op(VM_RET(0),&code[4]);
	dsb_iton(66,&code[5]);
	dsb_nid_op(VM_RET(0),&code[6]);
	dsb_iton(77,&code[7]);

	ctx.ip = 0;
	ctx.timeout = 1000;
	CHECK(dsb_vm_interpret(&ctx) == -1);
	CHECK(harc.h.ll == 77);

	//Params are equal with variable
	dsb_nid_op(VM_CPY(1,0),&code[0]);
	dsb_iton(60,&code[1]);
	dsb_nid_op(VM_JLE(6,0,1),&code[2]);
	dsb_iton(50,&code[3]);
	dsb_nid_op(VM_RET(0),&code[4]);
	dsb_iton(66,&code[5]);
	dsb_nid_op(VM_RET(0),&code[6]);
	dsb_iton(77,&code[7]);

	ctx.ip = 0;
	ctx.timeout = 1000;
	CHECK(dsb_vm_interpret(&ctx) == -1);
	CHECK(harc.h.ll == 77);

	DONE;
}

void test_vm_jge()
{
	struct HARC harc;
	NID_t code[100];
	struct VMContext ctx;
	ctx.ip = 0;
	ctx.timeout = 1000;
	ctx.code = code;
	ctx.codesize = 100;
	ctx.result = &harc.h;

	//Params are lequal
	dsb_nid_op(VM_JGE(5,0,0),&code[0]);
	dsb_iton(40,&code[1]);
	dsb_iton(50,&code[2]);
	dsb_nid_op(VM_RET(0),&code[3]);
	dsb_iton(66,&code[4]);
	dsb_nid_op(VM_RET(0),&code[5]);
	dsb_iton(77,&code[6]);

	CHECK(dsb_vm_interpret(&ctx) == -1);
	CHECK(harc.h.ll == 66);

	//Params are not lequal
	dsb_nid_op(VM_JGE(5,0,0),&code[0]);
	dsb_iton(50,&code[1]);
	dsb_iton(45,&code[2]);
	dsb_nid_op(VM_RET(0),&code[3]);
	dsb_iton(66,&code[4]);
	dsb_nid_op(VM_RET(0),&code[5]);
	dsb_iton(77,&code[6]);

	ctx.ip = 0;
	ctx.timeout = 1000;
	CHECK(dsb_vm_interpret(&ctx) == -1);
	CHECK(harc.h.ll == 77);

	//Params are lequal with variable
	dsb_nid_op(VM_CPY(1,0),&code[0]);
	dsb_iton(40,&code[1]);
	dsb_nid_op(VM_JGE(6,1,0),&code[2]);
	dsb_iton(50,&code[3]);
	dsb_nid_op(VM_RET(0),&code[4]);
	dsb_iton(66,&code[5]);
	dsb_nid_op(VM_RET(0),&code[6]);
	dsb_iton(77,&code[7]);

	ctx.ip = 0;
	ctx.timeout = 1000;
	CHECK(dsb_vm_interpret(&ctx) == -1);
	CHECK(harc.h.ll == 66);

	//Params are equal with variable
	dsb_nid_op(VM_CPY(1,0),&code[0]);
	dsb_iton(60,&code[1]);
	dsb_nid_op(VM_JGE(6,0,1),&code[2]);
	dsb_iton(50,&code[3]);
	dsb_nid_op(VM_RET(0),&code[4]);
	dsb_iton(66,&code[5]);
	dsb_nid_op(VM_RET(0),&code[6]);
	dsb_iton(77,&code[7]);

	ctx.ip = 0;
	ctx.timeout = 1000;
	CHECK(dsb_vm_interpret(&ctx) == -1);
	CHECK(harc.h.ll == 66);

	DONE;
}

void test_vm_add()
{
	struct HARC harc;
	NID_t code[100];
	struct VMContext ctx;
	ctx.ip = 0;
	ctx.timeout = 1000;
	ctx.code = code;
	ctx.codesize = 100;
	ctx.result = &harc.h;

	dsb_nid_op(VM_ADD(1,0,0),&code[0]);
	dsb_iton(40,&code[1]);
	dsb_iton(50,&code[2]);
	dsb_nid_op(VM_RET(1),&code[3]);

	CHECK(dsb_vm_interpret(&ctx) == -1);
	CHECK(harc.h.ll == 90);

	DONE;
}

void test_vm_sub()
{
	struct HARC harc;
	NID_t code[100];
	struct VMContext ctx;
	ctx.ip = 0;
	ctx.timeout = 1000;
	ctx.code = code;
	ctx.codesize = 100;
	ctx.result = &harc.h;

	dsb_nid_op(VM_SUB(1,0,0),&code[0]);
	dsb_iton(40,&code[1]);
	dsb_iton(50,&code[2]);
	dsb_nid_op(VM_RET(1),&code[3]);

	CHECK(dsb_vm_interpret(&ctx) == -1);
	CHECK(harc.h.ll == -10);

	DONE;
}

void test_vm_div()
{
	struct HARC harc;
	NID_t code[100];
	struct VMContext ctx;
	ctx.ip = 0;
	ctx.timeout = 1000;
	ctx.code = code;
	ctx.codesize = 100;
	ctx.result = &harc.h;

	dsb_nid_op(VM_DIV(1,0,0),&code[0]);
	dsb_iton(40,&code[1]);
	dsb_iton(50,&code[2]);
	dsb_nid_op(VM_RET(1),&code[3]);

	CHECK(dsb_vm_interpret(&ctx) == -1);
	CHECK(harc.h.ll == 40 / 50);

	DONE;
}

void test_vm_mul()
{
	struct HARC harc;
	NID_t code[100];
	struct VMContext ctx;
	ctx.ip = 0;
	ctx.timeout = 1000;
	ctx.code = code;
	ctx.codesize = 100;
	ctx.result = &harc.h;

	dsb_nid_op(VM_MUL(1,0,0),&code[0]);
	dsb_iton(10,&code[1]);
	dsb_iton(3,&code[2]);
	dsb_nid_op(VM_RET(1),&code[3]);

	CHECK(dsb_vm_interpret(&ctx) == -1);
	CHECK(harc.h.ll == 30);

	DONE;
}

void test_vm_and()
{
	struct HARC harc;
	NID_t code[100];
	struct VMContext ctx;
	ctx.ip = 0;
	ctx.timeout = 1000;
	ctx.code = code;
	ctx.codesize = 100;
	ctx.result = &harc.h;

	dsb_nid_op(VM_AND(1,0,0),&code[0]);
	dsb_iton(3,&code[1]);
	dsb_iton(3,&code[2]);
	dsb_nid_op(VM_RET(1),&code[3]);

	CHECK(dsb_vm_interpret(&ctx) == -1);
	CHECK(harc.h.ll == 3);

	DONE;
}

void test_vm_or()
{
	struct HARC harc;
	NID_t code[100];
	struct VMContext ctx;
	ctx.ip = 0;
	ctx.timeout = 1000;
	ctx.code = code;
	ctx.codesize = 100;
	ctx.result = &harc.h;

	dsb_nid_op(VM_OR(1,0,0),&code[0]);
	dsb_iton(1,&code[1]);
	dsb_iton(2,&code[2]);
	dsb_nid_op(VM_RET(1),&code[3]);

	CHECK(dsb_vm_interpret(&ctx) == -1);
	CHECK(harc.h.ll == 3);

	DONE;
}

void test_vm_xor()
{
	struct HARC harc;
	NID_t code[100];
	struct VMContext ctx;
	ctx.ip = 0;
	ctx.timeout = 1000;
	ctx.code = code;
	ctx.codesize = 100;
	ctx.result = &harc.h;

	dsb_nid_op(VM_XOR(1,0,0),&code[0]);
	dsb_iton(3,&code[1]);
	dsb_iton(2,&code[2]);
	dsb_nid_op(VM_RET(1),&code[3]);

	CHECK(dsb_vm_interpret(&ctx) == -1);
	CHECK(harc.h.ll == 1);

	DONE;
}

void test_vm_shl()
{
	struct HARC harc;
	NID_t code[100];
	struct VMContext ctx;
	ctx.ip = 0;
	ctx.timeout = 1000;
	ctx.code = code;
	ctx.codesize = 100;
	ctx.result = &harc.h;

	dsb_nid_op(VM_SHL(1,0,0),&code[0]);
	dsb_iton(1,&code[1]);
	dsb_iton(4,&code[2]);
	dsb_nid_op(VM_RET(1),&code[3]);

	CHECK(dsb_vm_interpret(&ctx) == -1);
	CHECK(harc.h.ll == 16);

	DONE;
}

void test_vm_shr()
{
	struct HARC harc;
	NID_t code[100];
	struct VMContext ctx;
	ctx.ip = 0;
	ctx.timeout = 1000;
	ctx.code = code;
	ctx.codesize = 100;
	ctx.result = &harc.h;

	dsb_nid_op(VM_SHR(1,0,0),&code[0]);
	dsb_iton(16,&code[1]);
	dsb_iton(4,&code[2]);
	dsb_nid_op(VM_RET(1),&code[3]);

	CHECK(dsb_vm_interpret(&ctx) == -1);
	CHECK(harc.h.ll == 1);

	DONE;
}

void test_vm_inc()
{
	struct HARC harc;
	NID_t code[100];
	struct VMContext ctx;
	ctx.ip = 0;
	ctx.timeout = 1000;
	ctx.code = code;
	ctx.codesize = 100;
	ctx.result = &harc.h;

	dsb_nid_op(VM_CPY(1,0),&code[0]);
	dsb_iton(16,&code[1]);
	dsb_nid_op(VM_INC(1),&code[2]);
	dsb_nid_op(VM_RET(1),&code[3]);

	CHECK(dsb_vm_interpret(&ctx) == -1);
	CHECK(harc.h.ll == 17);

	DONE;
}

void test_vm_dec()
{
	struct HARC harc;
	NID_t code[100];
	struct VMContext ctx;
	ctx.ip = 0;
	ctx.timeout = 1000;
	ctx.code = code;
	ctx.codesize = 100;
	ctx.result = &harc.h;

	dsb_nid_op(VM_CPY(1,0),&code[0]);
	dsb_iton(16,&code[1]);
	dsb_nid_op(VM_DEC(1),&code[2]);
	dsb_nid_op(VM_RET(1),&code[3]);

	CHECK(dsb_vm_interpret(&ctx) == -1);
	CHECK(harc.h.ll == 15);

	DONE;
}

void test_vm_clr()
{
	struct HARC harc;
	NID_t code[100];
	struct VMContext ctx;
	ctx.ip = 0;
	ctx.timeout = 1000;
	ctx.code = code;
	ctx.codesize = 100;
	ctx.result = &harc.h;

	dsb_nid_op(VM_CLR(1),&code[0]);
	dsb_nid_op(VM_RET(1),&code[1]);

	CHECK(dsb_vm_interpret(&ctx) == -1);
	CHECK(dsb_nid_eq(&harc.h,&Null) == 1);

	DONE;
}

static int xfunctest;

static int test_xfuncdummy(NID_t *res, int pcount, const NID_t *params)
{
	int i;

	xfunctest = 0;
	for (i=0; i<pcount; i++)
	{
		xfunctest += (int)params[i].ll;
	}

	return 0;
}

void test_vm_callx()
{
	struct HARC harc;
	NID_t code[100];
	struct VMContext ctx;
	ctx.ip = 0;
	ctx.timeout = 1000;
	ctx.code = code;
	ctx.codesize = 100;
	ctx.result = &harc.h;

	XFUNC(1,test_xfuncdummy);

	dsb_nid_op(VM_CALLX(1,0,2),&code[0]);
	dsb_iton(1,&code[1]);
	dsb_iton(0,&code[2]);
	dsb_iton(55,&code[3]);
	dsb_iton(66,&code[4]);
	dsb_nid_op(VM_RET(1),&code[5]);

	CHECK(dsb_vm_interpret(&ctx) == -1);
	CHECK(xfunctest == (55+66));

	DONE;
}

int main(int argc, char *argv[])
{
	dsb_nid_init();
	dsb_event_init();

	dsb_test(test_vm_copy);
	dsb_test(test_vm_get);
	dsb_test(test_vm_def);
	dsb_test(test_vm_jeq);
	dsb_test(test_vm_jne);
	dsb_test(test_vm_jle);
	dsb_test(test_vm_jge);

	dsb_test(test_vm_add);
	dsb_test(test_vm_sub);
	dsb_test(test_vm_div);
	dsb_test(test_vm_mul);
	dsb_test(test_vm_and);
	dsb_test(test_vm_or);
	dsb_test(test_vm_xor);
	dsb_test(test_vm_shl);
	dsb_test(test_vm_shr);
	dsb_test(test_vm_inc);
	dsb_test(test_vm_dec);
	dsb_test(test_vm_clr);

	dsb_test(test_vm_callx);

	dsb_event_final();
	dsb_nid_final();
	return 0;
}

