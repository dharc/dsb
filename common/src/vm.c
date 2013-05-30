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

int dsb_vm_interpret(HARC_t *harc, NID_t *code, int maxip)
{
	int ip = 0; //Instruction pointer.
	NID_t reg[16]; //Context registers.
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
		case VMOP_CONST:	reg[VMREG_A(op)] = code[++ip]; break;
		case VMOP_RET:		harc->h = reg[VMREG_A(op)]; return 0;
		case VMOP_COPY:		reg[VMREG_B(op)] = reg[VMREG_A(op)]; break;
		case VMOP_JUMP:		ip += (char)(op & 0xFF); continue;
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

		default: break;
		}

		ip++;
	}

	return 0;
}

