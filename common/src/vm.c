/*
 * vm.c
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

#include "dsb/vm.h"
#include "dsb/harc.h"
#include "dsb/errors.h"
#include "dsb/nid.h"
#include "dsb/specials.h"
#include "dsb/wrap.h"
#include "dsb/array.h"
#include <malloc.h>

int dsb_vm_call(const NID_t *func, const NID_t *params, int pn, NID_t *res)
{
	int maxip;	//End of instructions.
	NID_t *code = malloc(sizeof(NID_t)*1000);

	//Read in the code
	maxip = dsb_array_read(func, code, 1000);

	//Run the interpreter.
	dsb_vm_interpret(code,maxip,params,pn,res);

	free(code);

	return SUCCESS;
}

int dsb_vm_interpret(const NID_t *code, int maxip, const NID_t *params, int pn, NID_t *res)
{
	NID_t reg[16];
	return dsb_vm_interpret_reg(code,maxip,reg,params,pn,res);
}

int dsb_vm_interpret_reg(const NID_t *code, int maxip, NID_t *reg, const NID_t *params, int pn, NID_t *res)
{
	int ip = 0; //Instruction pointer.
	//NID_t reg[16]; //Context registers.
	unsigned int op;

	//Main VM loop.
	while (ip < maxip)
	{
		//Not a valid instruction.
		if (code[ip].t != NID_VMOP)	return 0;
		op = (unsigned int)code[ip].ll;

		//Switch on operation type.
		switch(VM_OP(op))
		{
		case VMOP_CONST:	reg[VMREG_A(op)] = code[++ip];
							break;
		case VMOP_RET:		*res = reg[VMREG_A(op)];
							return 0;
		case VMOP_COPY:		reg[VMREG_B(op)] = reg[VMREG_A(op)];
							break;
		case VMOP_JUMP:		ip += (char)(op & 0xFF);
							continue;
		case VMOP_JEQ:		if (dsb_nid_eq(&reg[VMREG_A(op)],&reg[VMREG_B(op)]) == 1)
							{
								ip += (char)(op & 0xFF); continue;
							}
							break;
		case VMOP_JNEQ:		if (dsb_nid_eq(&reg[VMREG_A(op)],&reg[VMREG_B(op)]) == 0)
							{
								ip += (char)(op & 0xFF); continue;
							}
							break;
		case VMOP_READ:		dsb_get(&reg[VMREG_A(op)],&reg[VMREG_B(op)],&reg[VMREG_C(op)]);
							break;

		default: break;
		}

		ip++;
	}

	return 0;
}

