/*
 * asm-test.c
 *
 *  Created on: 20 Jun 2013
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

int dsb_send(struct Event *evt)
{
	return 0;
}

void test_asm_jeq()
{
	NID_t code[100];

	CHECK(dsb_assemble_array("jeq 55 5 6", code, 50) == 3);
	CHECK(code[0].ll == VM_JEQ(55,0,0));
	CHECK(code[1].ll == 5);
	CHECK(code[2].ll == 6);

	CHECK(dsb_assemble_array("jeq 55 $x 6", code, 50) == 2);
	CHECK(code[0].ll == VM_JEQ(55,4,0));
	CHECK(code[1].ll == 6);

	CHECK(dsb_assemble_array("jeq 55 7 $y", code, 50) == 2);
	CHECK(code[0].ll == VM_JEQ(55,0,4));
	CHECK(code[1].ll == 7);

	CHECK(dsb_assemble_array("jeq 55 $x $y", code, 50) == 1);
	CHECK(code[0].ll == VM_JEQ(55,4,5));

	//Missing parameter checks.
	CHECK(dsb_assemble_array("jeq 55 $x $y 7", code, 50) == 0);
	CHECK(dsb_assemble_array("jeq $z $x $y", code, 50) == 0);
	CHECK(dsb_assemble_array("jeq 55 $x", code, 50) == 0);
	CHECK(dsb_assemble_array("jeq 55", code, 50) == 0);
	CHECK(dsb_assemble_array("jeq", code, 50) == 0);

	DONE;
}

void test_asm_jne()
{
	NID_t code[100];

	CHECK(dsb_assemble_array("jne 55 5 6", code, 50) == 3);
	CHECK(code[0].ll == VM_JNE(55,0,0));
	CHECK(code[1].ll == 5);
	CHECK(code[2].ll == 6);

	CHECK(dsb_assemble_array("jne 55 $x 6", code, 50) == 2);
	CHECK(code[0].ll == VM_JNE(55,4,0));
	CHECK(code[1].ll == 6);

	CHECK(dsb_assemble_array("jne 55 7 $y", code, 50) == 2);
	CHECK(code[0].ll == VM_JNE(55,0,4));
	CHECK(code[1].ll == 7);

	CHECK(dsb_assemble_array("jne 55 $x $y", code, 50) == 1);
	CHECK(code[0].ll == VM_JNE(55,4,5));
	DONE;
}

void test_asm_jle()
{
	NID_t code[100];

	CHECK(dsb_assemble_array("jle 55 5 6", code, 50) == 3);
	CHECK(code[0].ll == VM_JLE(55,0,0));
	CHECK(code[1].ll == 5);
	CHECK(code[2].ll == 6);

	CHECK(dsb_assemble_array("jle 55 $x 6", code, 50) == 2);
	CHECK(code[0].ll == VM_JLE(55,4,0));
	CHECK(code[1].ll == 6);

	CHECK(dsb_assemble_array("jle 55 7 $y", code, 50) == 2);
	CHECK(code[0].ll == VM_JLE(55,0,4));
	CHECK(code[1].ll == 7);

	CHECK(dsb_assemble_array("jle 55 $x $y", code, 50) == 1);
	CHECK(code[0].ll == VM_JLE(55,4,5));
	DONE;
}

void test_asm_jge()
{
	NID_t code[100];

	CHECK(dsb_assemble_array("jge 55 5 6", code, 50) == 3);
	CHECK(code[0].ll == VM_JGE(55,0,0));
	CHECK(code[1].ll == 5);
	CHECK(code[2].ll == 6);

	CHECK(dsb_assemble_array("jge 55 $x 6", code, 50) == 2);
	CHECK(code[0].ll == VM_JGE(55,4,0));
	CHECK(code[1].ll == 6);

	CHECK(dsb_assemble_array("jge 55 7 $y", code, 50) == 2);
	CHECK(code[0].ll == VM_JGE(55,0,4));
	CHECK(code[1].ll == 7);

	CHECK(dsb_assemble_array("jge 55 $x $y", code, 50) == 1);
	CHECK(code[0].ll == VM_JGE(55,4,5));
	DONE;
}

void test_asm_jlt()
{
	NID_t code[100];

	CHECK(dsb_assemble_array("jlt 55 5 6", code, 50) == 3);
	CHECK(code[0].ll == VM_JLT(55,0,0));
	CHECK(code[1].ll == 5);
	CHECK(code[2].ll == 6);

	CHECK(dsb_assemble_array("jlt 55 $x 6", code, 50) == 2);
	CHECK(code[0].ll == VM_JLT(55,4,0));
	CHECK(code[1].ll == 6);

	CHECK(dsb_assemble_array("jlt 55 7 $y", code, 50) == 2);
	CHECK(code[0].ll == VM_JLT(55,0,4));
	CHECK(code[1].ll == 7);

	CHECK(dsb_assemble_array("jlt 55 $x $y", code, 50) == 1);
	CHECK(code[0].ll == VM_JLT(55,4,5));
	DONE;
}

void test_asm_jgt()
{
	NID_t code[100];

	CHECK(dsb_assemble_array("jgt 55 5 6", code, 50) == 3);
	CHECK(code[0].ll == VM_JGT(55,0,0));
	CHECK(code[1].ll == 5);
	CHECK(code[2].ll == 6);

	CHECK(dsb_assemble_array("jgt 55 $x 6", code, 50) == 2);
	CHECK(code[0].ll == VM_JGT(55,4,0));
	CHECK(code[1].ll == 6);

	CHECK(dsb_assemble_array("jgt 55 7 $y", code, 50) == 2);
	CHECK(code[0].ll == VM_JGT(55,0,4));
	CHECK(code[1].ll == 7);

	CHECK(dsb_assemble_array("jgt 55 $x $y", code, 50) == 1);
	CHECK(code[0].ll == VM_JGT(55,4,5));
	DONE;
}

void test_asm_get()
{
	NID_t code[100];

	CHECK(dsb_assemble_array("get $x 10 11", code, 50) == 3);
	CHECK(code[0].ll == VM_GET(4,0,0));
	CHECK(code[1].ll == 10);
	CHECK(code[2].ll == 11);

	CHECK(dsb_assemble_array("get $x $y $z", code, 50) == 1);
	CHECK(code[0].ll == VM_GET(4,5,6));

	//Missing variable.
	CHECK(dsb_assemble_array("get 5 $y $z", code, 50) == 0);

	DONE;
}

void test_asm_def()
{
	NID_t code[100];

	CHECK(dsb_assemble_array("def 23 24 25", code, 50) == 4);
	CHECK(code[0].ll == VM_DEF(0,0,0));
	CHECK(code[1].ll == 23);
	CHECK(code[2].ll == 24);
	CHECK(code[3].ll == 25);

	CHECK(dsb_assemble_array("def $x $y $z", code, 50) == 1);
	CHECK(code[0].ll == VM_DEF(4,5,6));

	DONE;
}

void test_asm_dep()
{
	NID_t code[100];

	CHECK(dsb_assemble_array("dep 23 24 25 26", code, 50) == 5);
	CHECK(code[0].ll == VM_DEP(0,0,0,0));
	CHECK(code[1].ll == 23);
	CHECK(code[2].ll == 24);
	CHECK(code[3].ll == 25);
	CHECK(code[4].ll == 26);

	CHECK(dsb_assemble_array("dep 23 24 25 $x", code, 50) == 4);
	CHECK(code[0].ll == VM_DEP(0,0,0,4));
	CHECK(code[1].ll == 23);
	CHECK(code[2].ll == 24);
	CHECK(code[3].ll == 25);

	CHECK(dsb_assemble_array("dep $x $y $z $w", code, 50) == 1);
	CHECK(code[0].ll == VM_DEP(4,5,6,7));

	CHECK(dsb_assemble_array("dep $x $y $z", code, 50) == 0);

	DONE;
}

void test_asm_new()
{
	NID_t code[100];

	CHECK(dsb_assemble_array("new $x 55", code, 50) == 2);
	CHECK(code[0].ll == VM_NEW(4,0));
	CHECK(code[1].ll == 55);

	CHECK(dsb_assemble_array("new $x $y", code, 50) == 1);
	CHECK(code[0].ll == VM_NEW(4,5));

	DONE;
}

void test_asm_del()
{
	NID_t code[100];

	CHECK(dsb_assemble_array("del 66 55", code, 50) == 3);
	CHECK(code[0].ll == VM_DEL(0,0));
	CHECK(code[1].ll == 66);
	CHECK(code[2].ll == 55);

	CHECK(dsb_assemble_array("del $x $y", code, 50) == 1);
	CHECK(code[0].ll == VM_DEL(4,5));

	DONE;
}

void test_asm_cpy()
{
	NID_t code[100];

	CHECK(dsb_assemble_array("cpy $x 55", code, 50) == 2);
	CHECK(code[0].ll == VM_CPY(4,0));
	CHECK(code[1].ll == 55);

	CHECK(dsb_assemble_array("cpy $x $y", code, 50) == 1);
	CHECK(code[0].ll == VM_CPY(4,5));

	CHECK(dsb_assemble_array("cpy 44 $y", code, 50) == 0);

	DONE;
}

void test_asm_ret()
{
	NID_t code[100];

	CHECK(dsb_assemble_array("ret 77", code, 50) == 2);
	CHECK(code[0].ll == VM_RET(0));
	CHECK(code[1].ll == 77);

	CHECK(dsb_assemble_array("ret $x", code, 50) == 1);
	CHECK(code[0].ll == VM_RET(4));

	DONE;
}

void test_asm_add()
{
	NID_t code[100];

	CHECK(dsb_assemble_array("add $x 4 5", code, 50) == 3);
	CHECK(code[0].ll == VM_ADD(4,0,0));
	CHECK(code[1].ll == 4);
	CHECK(code[2].ll == 5);

	CHECK(dsb_assemble_array("add $x $y $z", code, 50) == 1);
	CHECK(code[0].ll == VM_ADD(4,5,6));

	CHECK(dsb_assemble_array("add 3 $y $z", code, 50) == 0);

	DONE;
}

void test_asm_sub()
{
	NID_t code[100];

	CHECK(dsb_assemble_array("sub $x 4 5", code, 50) == 3);
	CHECK(code[0].ll == VM_SUB(4,0,0));
	CHECK(code[1].ll == 4);
	CHECK(code[2].ll == 5);

	CHECK(dsb_assemble_array("sub $x $y $z", code, 50) == 1);
	CHECK(code[0].ll == VM_SUB(4,5,6));

	CHECK(dsb_assemble_array("sub 3 $y $z", code, 50) == 0);

	DONE;
}

void test_asm_div()
{
	NID_t code[100];

	CHECK(dsb_assemble_array("div $x 4 5", code, 50) == 3);
	CHECK(code[0].ll == VM_DIV(4,0,0));
	CHECK(code[1].ll == 4);
	CHECK(code[2].ll == 5);

	CHECK(dsb_assemble_array("div $x $y $z", code, 50) == 1);
	CHECK(code[0].ll == VM_DIV(4,5,6));

	CHECK(dsb_assemble_array("div 3 $y $z", code, 50) == 0);

	DONE;
}

void test_asm_mul()
{
	NID_t code[100];

	CHECK(dsb_assemble_array("mul $x 4 5", code, 50) == 3);
	CHECK(code[0].ll == VM_MUL(4,0,0));
	CHECK(code[1].ll == 4);
	CHECK(code[2].ll == 5);

	CHECK(dsb_assemble_array("mul $x $y $z", code, 50) == 1);
	CHECK(code[0].ll == VM_MUL(4,5,6));

	CHECK(dsb_assemble_array("mul 3 $y $z", code, 50) == 0);

	DONE;
}

void test_asm_shl()
{
	NID_t code[100];

	CHECK(dsb_assemble_array("shl $x 4 5", code, 50) == 3);
	CHECK(code[0].ll == VM_SHL(4,0,0));
	CHECK(code[1].ll == 4);
	CHECK(code[2].ll == 5);

	CHECK(dsb_assemble_array("shl $x $y $z", code, 50) == 1);
	CHECK(code[0].ll == VM_SHL(4,5,6));

	CHECK(dsb_assemble_array("shl 3 $y $z", code, 50) == 0);

	DONE;
}

void test_asm_shr()
{
	NID_t code[100];

	CHECK(dsb_assemble_array("shr $x 4 5", code, 50) == 3);
	CHECK(code[0].ll == VM_SHR(4,0,0));
	CHECK(code[1].ll == 4);
	CHECK(code[2].ll == 5);

	CHECK(dsb_assemble_array("shr $x $y $z", code, 50) == 1);
	CHECK(code[0].ll == VM_SHR(4,5,6));

	CHECK(dsb_assemble_array("shr 3 $y $z", code, 50) == 0);

	DONE;
}

void test_asm_and()
{
	NID_t code[100];

	CHECK(dsb_assemble_array("and $x 4 5", code, 50) == 3);
	CHECK(code[0].ll == VM_AND(4,0,0));
	CHECK(code[1].ll == 4);
	CHECK(code[2].ll == 5);

	CHECK(dsb_assemble_array("and $x $y $z", code, 50) == 1);
	CHECK(code[0].ll == VM_AND(4,5,6));

	CHECK(dsb_assemble_array("and 3 $y $z", code, 50) == 0);

	DONE;
}

void test_asm_or()
{
	NID_t code[100];

	CHECK(dsb_assemble_array("or $x 4 5", code, 50) == 3);
	CHECK(code[0].ll == VM_OR(4,0,0));
	CHECK(code[1].ll == 4);
	CHECK(code[2].ll == 5);

	CHECK(dsb_assemble_array("or $x $y $z", code, 50) == 1);
	CHECK(code[0].ll == VM_OR(4,5,6));

	CHECK(dsb_assemble_array("or 3 $y $z", code, 50) == 0);

	DONE;
}

void test_asm_xor()
{
	NID_t code[100];

	CHECK(dsb_assemble_array("xor $x 4 5", code, 50) == 3);
	CHECK(code[0].ll == VM_XOR(4,0,0));
	CHECK(code[1].ll == 4);
	CHECK(code[2].ll == 5);

	CHECK(dsb_assemble_array("xor $x $y $z", code, 50) == 1);
	CHECK(code[0].ll == VM_XOR(4,5,6));

	//Test for matching of var names
	CHECK(dsb_assemble_array("xor $x $x $x", code, 50) == 1);
	CHECK(code[0].ll == VM_XOR(4,4,4));

	CHECK(dsb_assemble_array("xor 3 $y $z", code, 50) == 0);

	DONE;
}

void test_asm_call()
{
	NID_t code[100];

	CHECK(dsb_assemble_array("call $r $f 0 1 2 3 4 5 6 7", code, 50) == 10);
	CHECK(code[0].ll == VM_CALL(4,5,8));
	CHECK(code[1].ll == 0);
	CHECK(code[2].ll == 0);
	CHECK(code[3].ll == 1);
	CHECK(code[4].ll == 2);
	CHECK(code[5].ll == 3);
	CHECK(code[6].ll == 4);
	CHECK(code[7].ll == 5);
	CHECK(code[8].ll == 6);
	CHECK(code[9].ll == 7);

	CHECK(dsb_assemble_array("call $r $f 0 1 $g", code, 50) == 4);
	CHECK(code[0].ll == VM_CALL(4,5,3));
	CHECK(code[1].ll == 0x0000060000000000);
	CHECK(code[2].ll == 0);
	CHECK(code[3].ll == 1);

	DONE;
}

int main(int argc, char *argv[])
{
	dsb_nid_init();
	dsb_event_init();

	dsb_test(test_asm_jeq);
	dsb_test(test_asm_jne);
	dsb_test(test_asm_jle);
	dsb_test(test_asm_jge);
	dsb_test(test_asm_jlt);
	dsb_test(test_asm_jgt);

	dsb_test(test_asm_get);
	dsb_test(test_asm_def);
	dsb_test(test_asm_dep);
	dsb_test(test_asm_new);
	dsb_test(test_asm_del);

	dsb_test(test_asm_cpy);
	dsb_test(test_asm_ret);

	dsb_test(test_asm_add);
	dsb_test(test_asm_sub);
	dsb_test(test_asm_div);
	dsb_test(test_asm_mul);
	dsb_test(test_asm_shl);
	dsb_test(test_asm_shr);
	dsb_test(test_asm_and);
	dsb_test(test_asm_or);
	dsb_test(test_asm_xor);

	dsb_test(test_asm_call);

	dsb_event_final();
	dsb_nid_final();
	return 0;
}

