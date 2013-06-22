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

#include <stdio.h>

#define MOVQ_OR_R(O,RA,RB)
#define MOVQ_R_OR(RA,O,RB)

static int gen_init(void *output, int *rip)
{
	printf("pushq %%rbp\n");
	printf("movq %%rsp, %%rbp\n");
	printf("movq %%rdi, -8(%%rbp)\n");
	printf("movq -8(%%rbp), %%rax\n");

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
	printf("movq $%d, %%rax\n", a);
	//Gen_fini;
	printf("ret\n");

	return 0;
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

	return 0;
}
