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
	struct VMContext ctx;
	ctx.ip = 0;
	ctx.timeout = 10000;
	ctx.code = code;
	ctx.codesize = maxip;
	ctx.result = res;
	return dsb_vm_interpret_ctx(&ctx);
}

int dsb_vm_interpret_ctx(struct VMContext *ctx)
{
	unsigned int op;

	//Main VM loop.
	while ((ctx->ip < ctx->codesize) && (ctx->timeout-- > 0))
	{
		//Not a valid instruction.
		if (ctx->code[ctx->ip].t != NID_VMOP)	return -1;
		op = (unsigned int)ctx->code[ctx->ip].ll;

		//Switch on operation type.
		switch(VM_OP(op))
		{
		case VMOP_LOAD:		ctx->reg[VMREG_A(op)] = ctx->code[op & 0xFF];
							break;
		case VMOP_RET:		*ctx->result = ctx->reg[VMREG_A(op)];
							return -1;
		case VMOP_COPY:		ctx->reg[VMREG_B(op)] = ctx->reg[VMREG_A(op)];
							break;
		case VMOP_JUMP:		ctx->ip += (char)(op & 0xFF) + 1;
							continue;
		case VMOP_JEQ:		if (dsb_nid_eq(&ctx->reg[VMREG_A(op)],&ctx->reg[VMREG_B(op)]) == 1)
							{
								ctx->ip += (char)(op & 0xFF); continue;
							}
							break;
		case VMOP_JNEQ:		if (dsb_nid_eq(&ctx->reg[VMREG_A(op)],&ctx->reg[VMREG_B(op)]) == 0)
							{
								ctx->ip += (char)(op & 0xFF); continue;
							}
							break;
		case VMOP_READ:		dsb_get(&ctx->reg[VMREG_A(op)],&ctx->reg[VMREG_B(op)],&ctx->reg[VMREG_C(op)]);
							break;

		default: break;
		}

		ctx->ip++;
	}

	return SUCCESS;
}

