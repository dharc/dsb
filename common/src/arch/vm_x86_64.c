/*
 * vm_x86_64.c
 *
 *  Created on: 22 June 2013
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


#include "dsb/core/vm.h"
#include "dsb/core/nid.h"
#include "dsb/errors.h"

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/mman.h>

#define MOD_INDIRECT 0x00
#define MOD_DISP8 0x40
#define MOD_DISP32 0x80
#define MOD_REG 0xC0

#define MODRM_REG(A) ((A) << 3)
#define MODRM_RM(A) (A)
#define MODRM_MOD(A) (A)
#define MODRM(A,B,C) (MODRM_MOD(A) | MODRM_REG(B) | MODRM_RM(C))

#define RAX		0x00
#define RCX		0x01
#define RDX		0x02
#define RBX		0x03
#define RSP		0x04
#define RBP		0x05
#define RSI		0x06
#define RDI		0x07

//Second param is the indirect one.
//eg. INDIRECT(RAX,RBX) == %rax, (%rbx)
//eg. DISP8(RAX,RBX) == %rax, 16(%rbx)
//eg. DIRECT(RAX) == %rax, 123343
#define INDIRECT(A,B)		MODRM(MOD_INDIRECT,A,B)
#define DIRECT(A)			MODRM(MOD_INDIRECT,RBP,A)
#define DISP8(A,B)			MODRM(MOD_DISP8,A,B)
#define DISP32(A,B)			MODRM(MOD_DISP32,A,B)
#define REGADDR(A,B)		MODRM(MOD_REG,A,B)

#define OP_8_R(O)		((O) | 0x02)	//RM is source
#define OP_8_W(O)		((O) | 0x00)
#define OP_64_R(O)		((O) | 0x03)	//RM is dest
#define OP_64_W(O)		((O) | 0x01)
#define OP_8(O)			((O) | 0x00)	//RM is source
#define OP_64(O)		((O) | 0x01)	//RM is source
#define IOP_8(O)		((O) | 0x80)	//Immediate OP 8bit
#define IOP_64(O)		((O) | 0x81)	//Immediate OP 64bit

#define OPMODRW(O,M)	(((O) << 8) | (M))

#define OPSIZE_64	0x48

#define EMIT8(A)	((unsigned char*)output)[(*rip)++] = (unsigned char)A
#define EMIT16(A)	EMIT8((A) >> 8); EMIT8(A & 0xFF)
#define EMIT32(A)	EMIT8((A) & 0xFF); EMIT8(((A) >> 8) & 0xFF); EMIT8(((A) >> 16) & 0xFF); EMIT8((A) >> 24)

//2byte OPS
//R2 is always address register.
#define MOVQ(R1,R2)			EMIT8(OPSIZE_64); EMIT8(OP_64_W(0x88)); EMIT8(REGADDR(R1,R2))
#define MOVQ_R32(R1,R2)		EMIT8(OPSIZE_64); EMIT8(OP_64_R(0x88)); EMIT8(DISP32(R1,R2)) //+disp
#define MOVQ_W32(R1,R2)		EMIT8(OPSIZE_64); EMIT8(OP_64_W(0x88)); EMIT8(DISP32(R1,R2)) //+disp
#define LEAQ(R1,R2)			EMIT8(OPSIZE_64); EMIT8(OP_64(0x8D)); EMIT8(DISP32(R1,R2)) //+disp

#define MOVQ_R8(R1,R2)		EMIT8(OPSIZE_64); EMIT8(OP_64_R(0x88)); EMIT8(DISP8(R1,R2)) //+disp
#define MOVQ_W8(R1,R2)		EMIT8(OPSIZE_64); EMIT8(OP_64_W(0x88)); EMIT8(DISP8(R1,R2)) //+disp
#define LEAQ_8(R1,R2)		EMIT8(OPSIZE_64); EMIT8(OP_64(0x8D)); EMIT8(DISP8(R1,R2)) //+disp

#define MOVQ_R(R1,R2)		EMIT8(OPSIZE_64); EMIT8(OP_64_R(0x88)); EMIT8(INDIRECT(R1,R2))
#define MOVQ_W(R1,R2)		EMIT8(OPSIZE_64); EMIT8(OP_64_W(0x88)); EMIT8(INDIRECT(R1,R2))

#define MOVQ_I32(R)			EMIT8(OPSIZE_64); EMIT8(0xC7); EMIT8(REGADDR(0,R)); // + 32bit immediate.

//1byte OPS
#define PUSHQ(R)	EMIT8(0x50 | R)
#define POPQ(R)		EMIT8(0x58 | R)
#define RET			EMIT8(0xC3)

static int gen_init(void *output, int *rip)
{

	PUSHQ(RBP);						// pushq %rbp
	MOVQ(RSP,RBP);					// movq %rsp, %rbp
	MOVQ_W8(RDI,RBP); EMIT8(-8);	// movq %rdi, -8(%rbp)
	MOVQ_R8(RAX,RBP); EMIT8(-8);	// movq -8(%rbp), %rax

	return 0;
}

/*
 * Copy var b into var a
 */
static int gen_copy_vv(int a, int b, void *output, int *rip)
{
	//Actual memory offsets
	a = a * sizeof(NID_t);
	b = b * sizeof(NID_t);

	//TODO use larger displacement for values above 127

	//If 0 then no need for displacement byte.
	if (b == 0) { MOVQ_R(RCX,RAX); }
	else { MOVQ_R8(RCX,RAX); EMIT8(b+0); }

	if (a == 0) { MOVQ_W(RCX,RAX); }
	else { MOVQ_W8(RCX,RAX); EMIT8(a+0); }

	MOVQ_R8(RCX,RAX); EMIT8(b+8);
	MOVQ_W8(RCX,RAX); EMIT8(a+8);
	MOVQ_R8(RCX,RAX); EMIT8(b+16);
	MOVQ_W8(RCX,RAX); EMIT8(a+16);

	printf("movq %d(%%rax), %%rcx\n", b+0);
	printf("movq %%rcx, %d(%%rax)\n", a+0);
	printf("movq %d(%%rax), %%rcx\n", b+8);
	printf("movq %%rcx, %d(%%rax)\n", a+8);
	printf("movq %d(%%rax), %%rcx\n", b+16);
	printf("movq %%rcx, %d(%%rax)\n", a+16);

	return 0;
}

/*
 * Copy const b into var a
 */
static int gen_copy_vc(int a, NID_t *b, void *output, int *rip)
{
	unsigned long long *bp = (unsigned long long*)b;
	//Actual memory offsets
	a = a * sizeof(NID_t);

	printf("movq $%llu, %%rcx\n", bp[0]);
	printf("movq %%rcx, %d(%%rax)\n", a+0);
	printf("movq $%llu, %%rcx\n", bp[1]);
	printf("movq %%rcx, %d(%%rax)\n", a+8);
	printf("movq $%llu, %%rcx\n", bp[2]);
	printf("movq %%rcx, %d(%%rax)\n", a+16);

	return 0;
}

/*
 * Increment var a
 */
static int gen_inc_v(int a, void *output, int *rip)
{
	//Actual memory offsets
	a = a * sizeof(NID_t);

	MOVQ_R8(RCX,RAX); EMIT8(a+16);
	LEAQ_8(RCX,RCX); EMIT8(1);
	MOVQ_W8(RCX,RAX); EMIT8(a+16);

	printf("movq %d(%%rax), %%rcx\n", a+16);
	printf("leaq 1(%%rcx), %%rcx\n");
	printf("movq %%rcx, %d(%%rax)\n", a+16);

	return 0;
}

/*
 * Increment var a
 */
static int gen_ret_v(int a, void *output, int *rip)
{
	MOVQ_I32(RAX); EMIT32(a);
	POPQ(RBP);
	RET;

	printf("movq $%d, %%rax\n", a);
	printf("popq %%rbp\n");
	printf("ret\n");

	return 0;
}

static void dump_hex(unsigned char *data, int len)
{
	int i;

	for (i=0; i<len; i++)
	{
		printf("%02x ",data[i]);
	}
	printf("\n");
}

int dsb_vm_arch_compile(NID_t *code, int size, void **output)
{
	int ip = 0;
	int rip = 0;
	unsigned int op;
	//unsigned int off;
	unsigned int varA;
	unsigned int varB;
	//unsigned int varC;
	//unsigned int varD;

	//*output = malloc(1000);
	*output = mmap(0,1000,PROT_READ|PROT_WRITE|PROT_EXEC,MAP_PRIVATE|MAP_ANON,-1,0);

	if (output == 0) return DSB_ERROR(ERR_NOEXECMEM,0);

	gen_init(*output,&rip);

	while (ip < size)
	{
		op = VMGET_OP(code[ip].ll);
		//off = VMGET_LABEL(code[ip].ll);
		varA = VMGET_A(code[ip].ll);
		varB = VMGET_B(code[ip].ll);
		//varC = VMGET_C(code[ip].ll);
		//varD = VMGET_D(code[ip].ll);

		switch (op)
		{
		case VMOP_CPY:	if (varB != 0) { gen_copy_vv(varA-1,varB-1,*output,&rip); }
						else { gen_copy_vc(varA-1,&code[++ip],output,&rip); }
						break;
		case VMOP_INC:	gen_inc_v(varA-1,*output,&rip);
						break;
		case VMOP_RET:	gen_ret_v(varA-1,*output,&rip);
						break;
		default: break;
		}

		ip++;
	}

	dump_hex(*output, rip);

	return 0;
}
