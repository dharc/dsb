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

void test_vm_copy()
{
	struct HARC harc;
	NID_t code[100];

	//Actual code.
	dsb_nid_op(VM_CPY(1,0),&code[0]);
	dsb_iton(50,&code[1]);
	dsb_nid_op(VM_RET(1),&code[2]);

	CHECK(dsb_vm_interpret(code,100, 0,0, &harc.h) == -1);
	CHECK(harc.h.ll == 50);

	//Actual code.
	dsb_nid_op(VM_CPY(1,0),&code[0]);
	dsb_iton(54,&code[1]);
	dsb_nid_op(VM_CPY(2,1),&code[2]);
	dsb_nid_op(VM_RET(2),&code[3]);

	CHECK(dsb_vm_interpret(code,100, 0,0, &harc.h) == -1);
	CHECK(harc.h.ll == 54);

	DONE;
}

void test_vm_get()
{
	struct HARC harc;
	NID_t code[100];

	//Actual code.
	dsb_nid_op(VM_CPY(1,0),&code[0]);
	dsb_iton(50,&code[1]);
	dsb_nid_op(VM_GET(2,1,0),&code[2]);
	dsb_iton(51,&code[3]);
	dsb_nid_op(VM_RET(2),&code[4]);

	CHECK(dsb_vm_interpret(code,100, 0,0, &harc.h) == -1);
	CHECK(harc.h.ll == 666);

	DONE;
}


int main(int argc, char *argv[])
{
	dsb_nid_init();
	dsb_event_init();

	dsb_test(test_vm_copy);
	dsb_test(test_vm_get);

	dsb_event_final();
	dsb_nid_final();
	return 0;
}

