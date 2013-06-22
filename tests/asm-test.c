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

	CHECK(dsb_assemble("jeq 55 5 6", code, 50) == 3);
	CHECK(code[0].ll == VM_JEQ(55,0,0));
	CHECK(code[1].ll == 5);
	CHECK(code[2].ll == 6);

	CHECK(dsb_assemble("jeq 55 $x 6", code, 50) == 2);
	CHECK(code[0].ll == VM_JEQ(55,4,0));
	CHECK(code[1].ll == 6);

	CHECK(dsb_assemble("jeq 55 7 $y", code, 50) == 2);
	CHECK(code[0].ll == VM_JEQ(55,0,4));
	CHECK(code[1].ll == 7);

	CHECK(dsb_assemble("jeq 55 $x $y", code, 50) == 1);
	CHECK(code[0].ll == VM_JEQ(55,4,5));

	//Missing parameter checks.
	CHECK(dsb_assemble("jeq 55 $x $y 7", code, 50) == 0);
	CHECK(dsb_assemble("jeq $z $x $y", code, 50) == 0);
	CHECK(dsb_assemble("jeq 55 $x", code, 50) == 0);
	CHECK(dsb_assemble("jeq 55", code, 50) == 0);
	CHECK(dsb_assemble("jeq", code, 50) == 0);

	DONE;
}

void test_asm_jne()
{
	NID_t code[100];

	CHECK(dsb_assemble("jne 55 5 6", code, 50) == 3);
	CHECK(code[0].ll == VM_JNE(55,0,0));
	CHECK(code[1].ll == 5);
	CHECK(code[2].ll == 6);

	CHECK(dsb_assemble("jne 55 $x 6", code, 50) == 2);
	CHECK(code[0].ll == VM_JNE(55,4,0));
	CHECK(code[1].ll == 6);

	CHECK(dsb_assemble("jne 55 7 $y", code, 50) == 2);
	CHECK(code[0].ll == VM_JNE(55,0,4));
	CHECK(code[1].ll == 7);

	CHECK(dsb_assemble("jne 55 $x $y", code, 50) == 1);
	CHECK(code[0].ll == VM_JNE(55,4,5));
	DONE;
}

void test_asm_jle()
{
	NID_t code[100];

	CHECK(dsb_assemble("jle 55 5 6", code, 50) == 3);
	CHECK(code[0].ll == VM_JLE(55,0,0));
	CHECK(code[1].ll == 5);
	CHECK(code[2].ll == 6);

	CHECK(dsb_assemble("jle 55 $x 6", code, 50) == 2);
	CHECK(code[0].ll == VM_JLE(55,4,0));
	CHECK(code[1].ll == 6);

	CHECK(dsb_assemble("jle 55 7 $y", code, 50) == 2);
	CHECK(code[0].ll == VM_JLE(55,0,4));
	CHECK(code[1].ll == 7);

	CHECK(dsb_assemble("jle 55 $x $y", code, 50) == 1);
	CHECK(code[0].ll == VM_JLE(55,4,5));
	DONE;
}

void test_asm_jge()
{
	NID_t code[100];

	CHECK(dsb_assemble("jge 55 5 6", code, 50) == 3);
	CHECK(code[0].ll == VM_JGE(55,0,0));
	CHECK(code[1].ll == 5);
	CHECK(code[2].ll == 6);

	CHECK(dsb_assemble("jge 55 $x 6", code, 50) == 2);
	CHECK(code[0].ll == VM_JGE(55,4,0));
	CHECK(code[1].ll == 6);

	CHECK(dsb_assemble("jge 55 7 $y", code, 50) == 2);
	CHECK(code[0].ll == VM_JGE(55,0,4));
	CHECK(code[1].ll == 7);

	CHECK(dsb_assemble("jge 55 $x $y", code, 50) == 1);
	CHECK(code[0].ll == VM_JGE(55,4,5));
	DONE;
}

void test_asm_jlt()
{
	NID_t code[100];

	CHECK(dsb_assemble("jlt 55 5 6", code, 50) == 3);
	CHECK(code[0].ll == VM_JLT(55,0,0));
	CHECK(code[1].ll == 5);
	CHECK(code[2].ll == 6);

	CHECK(dsb_assemble("jlt 55 $x 6", code, 50) == 2);
	CHECK(code[0].ll == VM_JLT(55,4,0));
	CHECK(code[1].ll == 6);

	CHECK(dsb_assemble("jlt 55 7 $y", code, 50) == 2);
	CHECK(code[0].ll == VM_JLT(55,0,4));
	CHECK(code[1].ll == 7);

	CHECK(dsb_assemble("jlt 55 $x $y", code, 50) == 1);
	CHECK(code[0].ll == VM_JLT(55,4,5));
	DONE;
}

void test_asm_jgt()
{
	NID_t code[100];

	CHECK(dsb_assemble("jgt 55 5 6", code, 50) == 3);
	CHECK(code[0].ll == VM_JGT(55,0,0));
	CHECK(code[1].ll == 5);
	CHECK(code[2].ll == 6);

	CHECK(dsb_assemble("jgt 55 $x 6", code, 50) == 2);
	CHECK(code[0].ll == VM_JGT(55,4,0));
	CHECK(code[1].ll == 6);

	CHECK(dsb_assemble("jgt 55 7 $y", code, 50) == 2);
	CHECK(code[0].ll == VM_JGT(55,0,4));
	CHECK(code[1].ll == 7);

	CHECK(dsb_assemble("jgt 55 $x $y", code, 50) == 1);
	CHECK(code[0].ll == VM_JGT(55,4,5));
	DONE;
}

void test_asm_get()
{
	NID_t code[100];

	CHECK(dsb_assemble("get $x 10 11", code, 50) == 3);
	CHECK(code[0].ll == VM_GET(4,0,0));
	CHECK(code[1].ll == 10);
	CHECK(code[2].ll == 11);

	CHECK(dsb_assemble("get $x $y $z", code, 50) == 1);
	CHECK(code[0].ll == VM_GET(4,5,6));

	//Missing variable.
	CHECK(dsb_assemble("get 5 $y $z", code, 50) == 0);

	DONE;
}

void test_asm_def()
{
	NID_t code[100];

	CHECK(dsb_assemble("def 23 24 25", code, 50) == 4);
	CHECK(code[0].ll == VM_DEF(0,0,0));
	CHECK(code[1].ll == 23);
	CHECK(code[2].ll == 24);
	CHECK(code[3].ll == 25);

	CHECK(dsb_assemble("def $x $y $z", code, 50) == 1);
	CHECK(code[0].ll == VM_DEF(4,5,6));

	DONE;
}

void test_asm_dep()
{
	NID_t code[100];

	CHECK(dsb_assemble("dep 23 24 25 26", code, 50) == 5);
	CHECK(code[0].ll == VM_DEP(0,0,0,0));
	CHECK(code[1].ll == 23);
	CHECK(code[2].ll == 24);
	CHECK(code[3].ll == 25);
	CHECK(code[4].ll == 26);

	CHECK(dsb_assemble("dep 23 24 25 $x", code, 50) == 4);
	CHECK(code[0].ll == VM_DEP(0,0,0,4));
	CHECK(code[1].ll == 23);
	CHECK(code[2].ll == 24);
	CHECK(code[3].ll == 25);

	CHECK(dsb_assemble("dep $x $y $z $w", code, 50) == 1);
	CHECK(code[0].ll == VM_DEP(4,5,6,7));

	CHECK(dsb_assemble("dep $x $y $z", code, 50) == 0);

	DONE;
}

void test_asm_new()
{
	NID_t code[100];

	CHECK(dsb_assemble("new $x 55", code, 50) == 2);
	CHECK(code[0].ll == VM_NEW(4,0));
	CHECK(code[1].ll == 55);

	CHECK(dsb_assemble("new $x $y", code, 50) == 1);
	CHECK(code[0].ll == VM_NEW(4,5));

	DONE;
}

void test_asm_del()
{
	NID_t code[100];

	CHECK(dsb_assemble("del 66 55", code, 50) == 3);
	CHECK(code[0].ll == VM_DEL(0,0));
	CHECK(code[1].ll == 66);
	CHECK(code[2].ll == 55);

	CHECK(dsb_assemble("del $x $y", code, 50) == 1);
	CHECK(code[0].ll == VM_DEL(4,5));

	DONE;
}

void test_asm_cpy()
{
	NID_t code[100];

	CHECK(dsb_assemble("cpy $x 55", code, 50) == 2);
	CHECK(code[0].ll == VM_CPY(4,0));
	CHECK(code[1].ll == 55);

	CHECK(dsb_assemble("cpy $x $y", code, 50) == 1);
	CHECK(code[0].ll == VM_CPY(4,5));

	CHECK(dsb_assemble("cpy 44 $y", code, 50) == 0);

	DONE;
}

void test_asm_ret()
{
	NID_t code[100];

	CHECK(dsb_assemble("ret 77", code, 50) == 2);
	CHECK(code[0].ll == VM_RET(0));
	CHECK(code[1].ll == 77);

	CHECK(dsb_assemble("ret $x", code, 50) == 1);
	CHECK(code[0].ll == VM_RET(4));

	DONE;
}

void test_asm_add()
{
	NID_t code[100];

	CHECK(dsb_assemble("add $x 4 5", code, 50) == 3);
	CHECK(code[0].ll == VM_ADD(4,0,0));
	CHECK(code[1].ll == 4);
	CHECK(code[2].ll == 5);

	CHECK(dsb_assemble("add $x $y $z", code, 50) == 1);
	CHECK(code[0].ll == VM_ADD(4,5,6));

	CHECK(dsb_assemble("add 3 $y $z", code, 50) == 0);

	DONE;
}

void test_asm_sub()
{
	NID_t code[100];

	CHECK(dsb_assemble("sub $x 4 5", code, 50) == 3);
	CHECK(code[0].ll == VM_SUB(4,0,0));
	CHECK(code[1].ll == 4);
	CHECK(code[2].ll == 5);

	CHECK(dsb_assemble("sub $x $y $z", code, 50) == 1);
	CHECK(code[0].ll == VM_SUB(4,5,6));

	CHECK(dsb_assemble("sub 3 $y $z", code, 50) == 0);

	DONE;
}

void test_asm_div()
{
	NID_t code[100];

	CHECK(dsb_assemble("div $x 4 5", code, 50) == 3);
	CHECK(code[0].ll == VM_DIV(4,0,0));
	CHECK(code[1].ll == 4);
	CHECK(code[2].ll == 5);

	CHECK(dsb_assemble("div $x $y $z", code, 50) == 1);
	CHECK(code[0].ll == VM_DIV(4,5,6));

	CHECK(dsb_assemble("div 3 $y $z", code, 50) == 0);

	DONE;
}

void test_asm_mul()
{
	NID_t code[100];

	CHECK(dsb_assemble("mul $x 4 5", code, 50) == 3);
	CHECK(code[0].ll == VM_MUL(4,0,0));
	CHECK(code[1].ll == 4);
	CHECK(code[2].ll == 5);

	CHECK(dsb_assemble("mul $x $y $z", code, 50) == 1);
	CHECK(code[0].ll == VM_MUL(4,5,6));

	CHECK(dsb_assemble("mul 3 $y $z", code, 50) == 0);

	DONE;
}

void test_asm_shl()
{
	NID_t code[100];

	CHECK(dsb_assemble("shl $x 4 5", code, 50) == 3);
	CHECK(code[0].ll == VM_SHL(4,0,0));
	CHECK(code[1].ll == 4);
	CHECK(code[2].ll == 5);

	CHECK(dsb_assemble("shl $x $y $z", code, 50) == 1);
	CHECK(code[0].ll == VM_SHL(4,5,6));

	CHECK(dsb_assemble("shl 3 $y $z", code, 50) == 0);

	DONE;
}

void test_asm_shr()
{
	NID_t code[100];

	CHECK(dsb_assemble("shr $x 4 5", code, 50) == 3);
	CHECK(code[0].ll == VM_SHR(4,0,0));
	CHECK(code[1].ll == 4);
	CHECK(code[2].ll == 5);

	CHECK(dsb_assemble("shr $x $y $z", code, 50) == 1);
	CHECK(code[0].ll == VM_SHR(4,5,6));

	CHECK(dsb_assemble("shr 3 $y $z", code, 50) == 0);

	DONE;
}

void test_asm_and()
{
	NID_t code[100];

	CHECK(dsb_assemble("and $x 4 5", code, 50) == 3);
	CHECK(code[0].ll == VM_AND(4,0,0));
	CHECK(code[1].ll == 4);
	CHECK(code[2].ll == 5);

	CHECK(dsb_assemble("and $x $y $z", code, 50) == 1);
	CHECK(code[0].ll == VM_AND(4,5,6));

	CHECK(dsb_assemble("and 3 $y $z", code, 50) == 0);

	DONE;
}

void test_asm_or()
{
	NID_t code[100];

	CHECK(dsb_assemble("or $x 4 5", code, 50) == 3);
	CHECK(code[0].ll == VM_OR(4,0,0));
	CHECK(code[1].ll == 4);
	CHECK(code[2].ll == 5);

	CHECK(dsb_assemble("or $x $y $z", code, 50) == 1);
	CHECK(code[0].ll == VM_OR(4,5,6));

	CHECK(dsb_assemble("or 3 $y $z", code, 50) == 0);

	DONE;
}

void test_asm_xor()
{
	NID_t code[100];

	CHECK(dsb_assemble("xor $x 4 5", code, 50) == 3);
	CHECK(code[0].ll == VM_XOR(4,0,0));
	CHECK(code[1].ll == 4);
	CHECK(code[2].ll == 5);

	CHECK(dsb_assemble("xor $x $y $z", code, 50) == 1);
	CHECK(code[0].ll == VM_XOR(4,5,6));

	//Test for matching of var names
	CHECK(dsb_assemble("xor $x $x $x", code, 50) == 1);
	CHECK(code[0].ll == VM_XOR(4,4,4));

	CHECK(dsb_assemble("xor 3 $y $z", code, 50) == 0);

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

	dsb_event_final();
	dsb_nid_final();
	return 0;
}

