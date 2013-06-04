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
#include "dsb/nid.h"
#include "dsb/errors.h"
#include "dsb/event.h"
#include "dsb/vm.h"
#include "dsb/assembler.h"

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
	return 0;
}

void test_vm_const()
{
	struct HARC harc;
	NID_t code[100];
	int i;

	//Init code array to VMOP types.
	for (i=0; i<100; i++)
	{
		code[i].header = 0;
		code[i].t = NID_VMOP;
	}

	//Actual code.
	code[0].ll = VM_LOAD(0,2);
	code[1].ll = VM_RET(0);			// Return reg 0.
	dsb_iton(55,&code[2]);			// reg 0 = 55

	CHECK(dsb_vm_interpret(code,100, 0,0, &harc.h) == -1);
	CHECK(harc.h.ll == 55);

	//Actual code.
	code[0].ll = VM_LOAD(15,2);
	code[1].ll = VM_RET(15);			// Return reg 0.
	dsb_iton(66,&code[2]);			// reg 0 = 55

	CHECK(dsb_vm_interpret(code,100, 0,0, &harc.h) == -1);
	CHECK(harc.h.ll == 66);

	DONE;
}

void test_vm_copy()
{
	struct HARC harc;
	NID_t code[100];
	int i;

	//Init code array to VMOP types.
	for (i=0; i<100; i++)
	{
		code[i].header = 0;
		code[i].t = NID_VMOP;
	}

	//Actual code.
	code[0].ll = VM_LOAD(0,3);
	code[1].ll = VM_COPY(0,1);
	code[2].ll = VM_RET(1);			// Return reg 0.
	dsb_iton(55,&code[3]);			// reg 0 = 55

	CHECK(dsb_vm_interpret(code,100, 0,0, &harc.h) == -1);
	CHECK(harc.h.ll == 55);

	DONE;
}

void test_vm_jump()
{
	struct HARC harc;
	NID_t code[100];
	int i;

	//Init code array to VMOP types.
	for (i=0; i<100; i++)
	{
		code[i].header = 0;
		code[i].t = NID_VMOP;
	}

	//Actual code for forward jump
	code[0].ll = VM_LOAD(0,5);
	code[1].ll = VM_LOAD(1,6);
	code[2].ll = VM_JUMP(4);		// Jump to IP 6.
	code[3].ll = VM_RET(0);			// Return reg 0.
	code[4].ll = VM_RET(1);			// Return reg 1.
	dsb_iton(77,&code[5]);			// reg 0 = 77
	dsb_iton(88,&code[6]);			// reg 1 = 88

	CHECK(dsb_vm_interpret(code,100, 0,0, &harc.h) == -1);
	CHECK(harc.h.ll == 88);

	//Backward jump.
	code[0].ll = VM_LOAD(0,6);
	code[1].ll = VM_LOAD(1,7);
	code[2].ll = VM_JUMP(4);		// Jump to IP 6.
	code[3].ll = VM_RET(0);			// Return reg 0.
	code[4].ll = VM_JUMP(3);
	code[5].ll = VM_RET(1);			// Return reg 1.
	dsb_iton(99,&code[6]);			// reg 0 = 99
	dsb_iton(33,&code[7]);			// reg 1 = 33

	CHECK(dsb_vm_interpret(code,100, 0,0, &harc.h) == -1);
	CHECK(harc.h.ll == 99);

	DONE;
}

void test_vm_jeq()
{
	struct HARC harc;
	NID_t code[100];
	int i;

	//Init code array to VMOP types.
	for (i=0; i<100; i++)
	{
		code[i].header = 0;
		code[i].t = NID_VMOP;
	}

	//Actual code for forward jeq
	code[0].ll = VM_LOAD(0,7);
	code[1].ll = VM_LOAD(1,8);
	code[2].ll = VM_LOAD(2,9);
	code[3].ll = VM_LOAD(3,10);
	code[4].ll = VM_JEQ(2,3,6);		// Jump to IP 10 if reg2 == reg3
	code[5].ll = VM_RET(0);			// Return reg 0.
	code[6].ll = VM_RET(1);			// Return reg 1.
	dsb_iton(2,&code[7]);			// reg 0 = 2
	dsb_iton(3,&code[8]);			// reg 1 = 3
	dsb_iton(4,&code[9]);			// reg 2 = 4
	dsb_iton(4,&code[10]);			// reg 3 = 4

	CHECK(dsb_vm_interpret(code,100, 0,0, &harc.h) == -1);
	CHECK(harc.h.ll == 3);

	dsb_iton(5,&code[9]);

	CHECK(dsb_vm_interpret(code,100, 0,0, &harc.h) == -1);
	CHECK(harc.h.ll == 2);

	DONE;
}

void test_vm_jneq()
{
	struct HARC harc;
	NID_t code[100];
	int i;

	//Init code array to VMOP types.
	for (i=0; i<100; i++)
	{
		code[i].header = 0;
		code[i].t = NID_VMOP;
	}

	//Actual code for forward jeq
	code[0].ll = VM_LOAD(0,7);
	code[1].ll = VM_LOAD(1,8);
	code[2].ll = VM_LOAD(2,9);
	code[3].ll = VM_LOAD(3,10);
	code[4].ll = VM_JNEQ(2,3,6);		// Jump to IP 10 if reg2 == reg3
	code[5].ll = VM_RET(0);			// Return reg 0.
	code[6].ll = VM_RET(1);			// Return reg 1.
	dsb_iton(2,&code[7]);			// reg 0 = 2
	dsb_iton(3,&code[8]);			// reg 1 = 3
	dsb_iton(4,&code[9]);			// reg 2 = 4
	dsb_iton(4,&code[10]);			// reg 3 = 4

	CHECK(dsb_vm_interpret(code,100, 0,0, &harc.h) == -1);
	CHECK(harc.h.ll == 2);

	dsb_iton(5,&code[9]);

	CHECK(dsb_vm_interpret(code,100, 0,0, &harc.h) == -1);
	CHECK(harc.h.ll == 3);

	DONE;
}

void test_vm_read()
{
	struct HARC harc;
	NID_t code[100];
	int i;

	//Init code array to VMOP types.
	for (i=0; i<100; i++)
	{
		code[i].header = 0;
		code[i].t = NID_VMOP;
	}

	//Actual code.
	code[0].ll = VM_LOAD(0,4);
	code[1].ll = VM_LOAD(1,5);
	code[2].ll = VM_READ(0,1,0);	// reg 0 = GET(reg 0, reg 1)
	code[3].ll = VM_RET(0);			// Return reg 0.
	dsb_iton(55,&code[4]);			// reg 0 = 55
	dsb_iton(0,&code[5]);			// reg 1 = 0

	CHECK(dsb_vm_interpret(code,100, 0,0, &harc.h) == -1);
	CHECK(harc.h.ll == 666);

	DONE;
}

void test_vm_asmconst()
{
	NID_t code[50];

	CHECK(dsb_assemble("load %2 1", code, 50) == 1);
	CHECK(VMREG_A(code[0].ll) == 2);
	CHECK((char)(code[0].ll & 0xFF) == 1);
	CHECK(VM_OP(code[0].ll) == VMOP_LOAD);

	CHECK(dsb_assemble("load %4 66\n", code, 50) == 1);
	CHECK((char)(code[0].ll & 0xFF) == 66);
	CHECK(VMREG_A(code[0].ll) == 4);
	CHECK(VM_OP(code[0].ll) == VMOP_LOAD);

	DONE;
}

void test_vm_asmjump()
{
	NID_t code[10];

	CHECK(dsb_assemble("jump 55", code, 50) == 1);
	CHECK((char)(code[0].ll & 0xFF) == 55);
	CHECK(VM_OP(code[0].ll) == VMOP_JUMP);

	CHECK(dsb_assemble("jump -55", code, 50) == 1);
	CHECK((char)(code[0].ll & 0xFF) == -55);
	CHECK(VM_OP(code[0].ll) == VMOP_JUMP);

	CHECK(dsb_assemble("jump -128", code, 50) == 1);
	CHECK((char)(code[0].ll & 0xFF) == -128);
	CHECK(VM_OP(code[0].ll) == VMOP_JUMP);

	DONE;
}

void test_vm_asmcopy()
{
	NID_t code[10];

	CHECK(dsb_assemble("copy %3 %4", code, 50) == 1);
	CHECK(VMREG_A(code[0].ll) == 3);
	CHECK(VMREG_B(code[0].ll) == 4);
	CHECK(VM_OP(code[0].ll) == VMOP_COPY);

	DONE;
}

void test_vm_asmcomments()
{
	NID_t code[50];

	CHECK(dsb_assemble("\n\n#fhfthfhf\n\nload %4 4\n",code,50) == 1);

	DONE;
}

void test_asm_label()
{
	NID_t code[10];

	CHECK(dsb_assemble(":mylabel", code, 10) == 0);
	CHECK(dsb_assemble("data 0\ndata 1\n:mylabel\njump :mylabel", code, 10) == 3);
	CHECK((char)(code[2].ll & 0xFF) == 2);

	CHECK(dsb_assemble("data 0\ndata 1\n    :mylabel\njump :mylabel", code, 10) == 3);
	CHECK((char)(code[2].ll & 0xFF) == 2);

	DONE;
}

int main(int argc, char *argv[])
{
	dsb_nid_init();
	dsb_event_init();

	dsb_test(test_vm_const);
	dsb_test(test_vm_copy);
	dsb_test(test_vm_jump);
	dsb_test(test_vm_jeq);
	dsb_test(test_vm_jneq);
	dsb_test(test_vm_read);

	dsb_test(test_vm_asmconst);
	dsb_test(test_vm_asmjump);
	dsb_test(test_vm_asmcomments);
	dsb_test(test_vm_asmcopy);

	dsb_test(test_asm_label);

	dsb_event_final();
	dsb_nid_final();
	return 0;
}

