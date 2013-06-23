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
#include <string.h>

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

//x86 byte order
#define EMIT8(A)	((unsigned char*)output)[(*rip)++] = (unsigned char)A
//#define EMIT32(A)	EMIT8((A) & 0xFF); EMIT8(((A) >> 8) & 0xFF); EMIT8(((A) >> 16) & 0xFF); EMIT8((A) >> 24)
#define EMIT32(A)	memcpy(&(((unsigned char*)(output))[(*rip)]), &(A), 4); (*rip) = (*rip) + 4
#define EMIT64(A)	EMIT8((A) & 0xFF); EMIT8(((A) >> 8) & 0xFF); EMIT8(((A) >> 16) & 0xFF); EMIT8(((A) >> 24) &0xFF); EMIT8(((A) >> 32) &0xFF); EMIT8(((A) >> 40) &0xFF); EMIT8(((A) >> 48) &0xFF); EMIT8(((A) >> 56) &0xFF);

//2byte OPS
//R2 is always address register.

//Move register to register
#define MOVQ(R1,R2)			EMIT8(OPSIZE_64); EMIT8(OP_64_W(0x88)); EMIT8(REGADDR(R1,R2))

//Move to mem with 32bit displacement
#define MOVQ_R32(R1,R2)		EMIT8(OPSIZE_64); EMIT8(OP_64_R(0x88)); EMIT8(DISP32(R1,R2)) //+disp
#define MOVQ_W32(R1,R2)		EMIT8(OPSIZE_64); EMIT8(OP_64_W(0x88)); EMIT8(DISP32(R1,R2)) //+disp
#define LEAQ(R1,R2)			EMIT8(OPSIZE_64); EMIT8(OP_64(0x8D)); EMIT8(DISP32(R1,R2)) //+disp

//Move to mem with 8bit displacement.
#define MOVQ_R8(R1,R2)		EMIT8(OPSIZE_64); EMIT8(OP_64_R(0x88)); EMIT8(DISP8(R1,R2)) //+disp
#define MOVQ_W8(R1,R2)		EMIT8(OPSIZE_64); EMIT8(OP_64_W(0x88)); EMIT8(DISP8(R1,R2)) //+disp
#define LEAQ_8(R1,R2)		EMIT8(OPSIZE_64); EMIT8(OP_64(0x8D)); EMIT8(DISP8(R1,R2)) //+disp

//Move to mem with no displacement.
#define MOVQ_R(R1,R2)		EMIT8(OPSIZE_64); EMIT8(OP_64_R(0x88)); EMIT8(INDIRECT(R1,R2))
#define MOVQ_W(R1,R2)		EMIT8(OPSIZE_64); EMIT8(OP_64_W(0x88)); EMIT8(INDIRECT(R1,R2))

//Move constant into register R
#define MOVQ_I32(R)			EMIT8(OPSIZE_64); EMIT8(0xC7); EMIT8(REGADDR(0,R)); // + 32bit immediate.
#define MOVQ_I64(R)			EMIT8(OPSIZE_64); EMIT8(0xB8 | R); // + 64bit

#define ADDQ_R8(R1,R2)		EMIT8(OPSIZE_64); EMIT8(OP_64_R(0x00)); EMIT8(DISP8(R1,R2)) //+disp
#define ADDQ_W8(R1,R2)		EMIT8(OPSIZE_64); EMIT8(OP_64_W(0x00)); EMIT8(DISP8(R1,R2)) //+disp
#define ADDQ(R1,R2)			EMIT8(OPSIZE_64); EMIT8(OP_64_W(0x00)); EMIT8(REGADDR(R1,R2))
#define ADDQ_I8(R)			EMIT8(OPSIZE_64); EMIT8(0x83); EMIT8(REGADDR(0,R)) //+imed
#define ADDQ_I32(R)			EMIT8(OPSIZE_64); EMIT8(0x81); EMIT8(REGADDR(0,R)) //+imed

#define CMPQ(R1,R2)			EMIT8(OPSIZE_64); EMIT8(OP_64_W(0x38)); EMIT8(REGADDR(R1,R2))
#define SUBQ(R1,R2)			EMIT8(OPSIZE_64); EMIT8(OP_64_W(0x28)); EMIT8(REGADDR(R1,R2))

#define JMP_64		EMIT(0xFF); EMIT8(REGADDR(4,0));
#define CALL_64		EMIT(0xFF);	EMIT8(REGADDR(2,RBP));

//1byte OPS
#define PUSHQ(R)	EMIT8(0x50 | R)
#define POPQ(R)		EMIT8(0x58 | R)
#define RET			EMIT8(0xC3)
#define JL_8		EMIT8(0x7C)					//Relative
#define JB_32		EMIT8(0x0F); EMIT8(0x82)	//Relative
#define JL_32		EMIT8(0x0F); EMIT8(0x8C)	//Relative
#define JLE_32		EMIT8(0x0F); EMIT8(0x8E)	//Relative
#define JG_32		EMIT8(0x0F); EMIT8(0x8F)	//Relative
#define JGE_32		EMIT8(0x0F); EMIT8(0x8D)	//Relative
#define JE_32		EMIT8(0x0F); EMIT8(0x84)	//Relative
#define JNE_32		EMIT8(0x0F); EMIT8(0x85)	//Relative
#define JMP_REL_32	EMIT8(0xE9);				//Relative
#define CALL_REL_32	EMIT8(0xE8);				//Relative
#define INT3		EMIT8(0xCC);

static int gen_init(void *output, int *rip)
{
	//Save stack stuff and get vars parameter into rax.
	PUSHQ(RBP);						// pushq %rbp
	MOVQ(RSP,RBP);					// movq %rsp, %rbp
	MOVQ_W8(RDI,RBP); EMIT8(-8);	// movq %rdi, -8(%rbp)
	MOVQ_R8(RAX,RBP); EMIT8(-8);	// movq -8(%rbp), %rax

	return 0;
}

/*
 * Read a var component into a register. v is an actual memory offset for the var.
 * ix can be 0, 1 or 2 to get 1st, 2nd or 3rd 64bit part of a nid. (ll = 2).
 */
static int gen_readVar(int v, int ix, int reg, void *output, int *rip)
{
	MOVQ_R8(RAX,RBP); EMIT8(-8);				// movq -8(%rbp), %rax
	if (v != 0) { ADDQ_I8(RAX); EMIT8(v); }		// addq $v, %rax
	if (ix == 0) { MOVQ_R(reg,RAX); }			// movq (%rax), %reg
	else { MOVQ_R8(reg,RAX); EMIT8(ix*8); }		// movq ix*8(%rax), %reg

	return 0;
}

/*
 * Write reg back into var v (component ix).
 */
static int gen_writeVar(int v, int ix, int reg, void *output, int *rip)
{
	MOVQ_R8(RAX,RBP); EMIT8(-8);				// movq -8(%rbp), %rax
	if (v != 0) { ADDQ_I8(RAX); EMIT8(v); }		// addq $v, %rax
	if (ix == 0) { MOVQ_W(reg,RAX); }			// movq %reg, (%rax)
	else { MOVQ_W8(reg,RAX); EMIT8(ix*8); }		// movq %reg, ix*8(%rax)

	return 0;
}

/*
 * Add b to c and put into a.
 */
static int gen_add_vvv(int a, int b, int c, void *output, int *rip)
{
	//Actual memory offsets.
	a = a * sizeof(NID_t);
	b = b * sizeof(NID_t);
	c = c * sizeof(NID_t);

	gen_readVar(b,2,RCX,output,rip);
	gen_readVar(c,2,RAX,output,rip);
	ADDQ(RAX,RCX);		//add %rax, %rcx
	gen_writeVar(a,2,RCX,output,rip);

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

	gen_readVar(b,0,RCX,output,rip);
	gen_writeVar(a,0,RCX,output,rip);
	gen_readVar(b,1,RCX,output,rip);
	gen_writeVar(a,1,RCX,output,rip);
	gen_readVar(b,2,RCX,output,rip);
	gen_writeVar(a,2,RCX,output,rip);

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

	MOVQ_I64(RCX); EMIT64(bp[0]);	//movq $bp[0], %rcx
	gen_writeVar(a,0,RCX,output,rip);
	MOVQ_I64(RCX); EMIT64(bp[1]);	//movq $bp[1], %rcx
	gen_writeVar(a,1,RCX,output,rip);
	MOVQ_I64(RCX); EMIT64(bp[2]);	//movq $bp[2], %rcx
	gen_writeVar(a,2,RCX,output,rip);

	return 0;
}

/*
 * Increment var a by 1
 */
static int gen_inc_v(int a, void *output, int *rip)
{
	//Actual memory offsets
	a = a * sizeof(NID_t);

	gen_readVar(a,2,RCX,output,rip);
	LEAQ_8(RCX,RCX); EMIT8(1);			//leaq 1(%rcx), %rcx
	gen_writeVar(a,2,RCX,output,rip);

	return 0;
}

/*
 * Return var A index in rax register.
 */
static int gen_ret_v(int a, void *output, int *rip)
{
	MOVQ_I32(RAX); EMIT32(a);	//movq $a, %rax
	POPQ(RBP);					//popq %rbp
	RET;						//ret

	return 0;
}

/*
 * Jump if a less than b.
 */
static int gen_jlt_vv(unsigned int *iptable, int a, int b, int off, void *output, int *rip)
{
	//Actual memory offsets
	a = a * sizeof(NID_t);
	b = b * sizeof(NID_t);

	gen_readVar(b,2,RCX,output,rip);
	gen_readVar(a,2,RAX,output,rip);
	CMPQ(RCX,RAX);						//cmpq %%rcx,%rax
	//INT3;								//int3

	//Do we have the correct instruction pointer.
	if (iptable[off] != 0xFFFFFFFF)
	{
		int roff = iptable[off] - (*rip) - 6;
		JL_32; EMIT32(roff);			//jl d
	}
	else
	{
		//NEED TO PATCH OFFSET LATER!!
	}

	return 0;
}

static void dump_hex(unsigned char *data, int len)
{
	int i;

	for (i=0; i<len; i++)
	{
		if ((i % 10) == 0)
		{
			printf("\n%04x: %02x ",(unsigned int)i, data[i]);
		}
		else
		{
			printf("%02x ",data[i]);
		}
	}
	printf("\n");
}

int dsb_vm_arch_compile(NID_t *code, int size, void **output)
{
	int ip = 0;
	int rip = 0;
	unsigned int op;
	unsigned int off;
	unsigned int varA;
	unsigned int varB;
	unsigned int varC;
	//unsigned int varD;

	//TODO make this same size as "size" param.
	unsigned int iptable[100];
	for (op = 0; op<100; op++)
	{
		iptable[op] = 0xFFFFFFFF;
	}

	//Allocate executable memory. Use munmap(addr,len) to free. Note, allocates pages
	*output = mmap(0,1000,PROT_READ|PROT_WRITE|PROT_EXEC,MAP_PRIVATE|MAP_ANON,-1,0);

	if (output == 0) return DSB_ERROR(ERR_NOEXECMEM,0);

	//Prefix code.
	gen_init(*output,&rip);

	while (ip < size)
	{
		op = VMGET_OP(code[ip].ll);
		off = VMGET_LABEL(code[ip].ll);
		varA = VMGET_A(code[ip].ll);
		varB = VMGET_B(code[ip].ll);
		varC = VMGET_C(code[ip].ll);
		//varD = VMGET_D(code[ip].ll);

		//Map VM instruction pointer to real instruction pointer...
		//Used for jumps.
		iptable[ip] = rip;

		switch (op)
		{
		case VMOP_CPY:	if (varB != 0) { gen_copy_vv(varA-1,varB-1,*output,&rip); }
						else { gen_copy_vc(varA-1,&code[++ip],*output,&rip); }
						break;
		case VMOP_INC:	gen_inc_v(varA-1,*output,&rip);
						break;
		case VMOP_RET:	gen_ret_v(varA-1,*output,&rip);
						break;
		case VMOP_ADD:	if ((varB != 0) && (varC != 0)) { gen_add_vvv(varA-1,varB-1,varC-1, *output, &rip); }
						break;
		case VMOP_JLT:	if ((varA != 0) && (varB != 0)) { gen_jlt_vv(iptable,varA-1,varB-1,off,*output,&rip); }
						break;
		default: break;
		}

		ip++;
	}

	dump_hex(*output, rip);

	return 0;
}
