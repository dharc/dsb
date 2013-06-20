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
#include "dsb/globals.h"
#include <malloc.h>

int dsb_vm_call(const NID_t *func, const HARC_t *harc, NID_t *res)
{
	int maxip;	//End of instructions.
	NID_t *code = malloc(sizeof(NID_t)*1000);

	//Read in the code
	maxip = dsb_array_read(func, code, 1000);

	//Run the interpreter.
	dsb_vm_interpret(code,maxip,harc,res);

	free(code);

	return SUCCESS;
}

int dsb_vm_interpret(NID_t *code, int maxip, const HARC_t *harc, NID_t *res)
{
	struct VMContext ctx;
	ctx.ip = 0;
	ctx.timeout = 10000;
	ctx.code = code;
	ctx.codesize = maxip;
	ctx.result = res;

	if (harc != 0)
	{
		ctx.vars[0] = harc->t1;
		ctx.vars[1] = harc->t2;
		ctx.vars[2] = harc->h;
	}

	return dsb_vm_interpret_ctx(&ctx);
}

int dsb_vm_interpret_ctx(struct VMContext *ctx)
{
	unsigned long long op;
	unsigned int varno;
	unsigned int varno2;
	unsigned int varno3;
	NID_t *n1;
	NID_t *n2;
	NID_t *n3;
	NID_t *n4;
	unsigned int varno4;

	//Main VM loop.
	while ((ctx->ip < ctx->codesize) && (ctx->timeout-- > 0))
	{
		//Not a valid instruction.
		if (ctx->code[ctx->ip].t != NID_VMOP)	return DSB_ERROR(ERR_VMINVALIP,0);
		op = ctx->code[ctx->ip].ll;

		//Switch on operation type.
		switch(VMGET_OP(op))
		{
		case VMOP_RET:		varno = VMGET_A(op);
							*ctx->result = (varno == 0) ? ctx->code[++ctx->ip] : ctx->vars[varno-1];
							return -1;

		case VMOP_CPY:		varno = VMGET_B(op);
							ctx->vars[VMGET_A(op)-1] = (varno == 0) ? ctx->code[++ctx->ip] : ctx->vars[varno-1];
							break;

		case VMOP_JMP:		ctx->ip = (short)(VMGET_LABEL(op));
							continue;

		case VMOP_JEQ:		varno = VMGET_A(op);
							varno2 = VMGET_B(op);
							n1 = (varno == 0) ? &ctx->code[++ctx->ip] : &ctx->vars[varno-1];
							n2 = (varno2 == 0) ? &ctx->code[++ctx->ip] : &ctx->vars[varno2-1];
							if (dsb_nid_eq(n1,n2) == 1)
							{
								ctx->ip = (short)(VMGET_LABEL(op)); continue;
							}
							break;

		case VMOP_JNE:		varno = VMGET_A(op);
							varno2 = VMGET_B(op);
							n1 = (varno == 0) ? &ctx->code[++ctx->ip] : &ctx->vars[varno-1];
							n2 = (varno2 == 0) ? &ctx->code[++ctx->ip] : &ctx->vars[varno2-1];
							if (dsb_nid_eq(n1,n2) == 0)
							{
								ctx->ip = (short)(VMGET_LABEL(op)); continue;
							}
							break;

		case VMOP_JLE:		varno = VMGET_A(op);
							varno2 = VMGET_B(op);
							n1 = (varno == 0) ? &ctx->code[++ctx->ip] : &ctx->vars[varno-1];
							n2 = (varno2 == 0) ? &ctx->code[++ctx->ip] : &ctx->vars[varno2-1];
							if (dsb_ntoi(n1) <= dsb_ntoi(n2))
							{
								ctx->ip = (short)(VMGET_LABEL(op)); continue;
							}
							break;

		case VMOP_JGE:		varno = VMGET_A(op);
							varno2 = VMGET_B(op);
							n1 = (varno == 0) ? &ctx->code[++ctx->ip] : &ctx->vars[varno-1];
							n2 = (varno2 == 0) ? &ctx->code[++ctx->ip] : &ctx->vars[varno2-1];
							if (dsb_ntoi(n1) >= dsb_ntoi(n2))
							{
								ctx->ip = (short)(VMGET_LABEL(op)); continue;
							}
							break;

		case VMOP_JLT:		varno = VMGET_A(op);
							varno2 = VMGET_B(op);
							n1 = (varno == 0) ? &ctx->code[++ctx->ip] : &ctx->vars[varno-1];
							n2 = (varno2 == 0) ? &ctx->code[++ctx->ip] : &ctx->vars[varno2-1];
							if (dsb_ntoi(n1) < dsb_ntoi(n2))
							{
								ctx->ip = (short)(VMGET_LABEL(op)); continue;
							}
							break;

		case VMOP_JGT:		varno = VMGET_A(op);
							varno2 = VMGET_B(op);
							n1 = (varno == 0) ? &ctx->code[++ctx->ip] : &ctx->vars[varno-1];
							n2 = (varno2 == 0) ? &ctx->code[++ctx->ip] : &ctx->vars[varno2-1];
							if (dsb_ntoi(n1) > dsb_ntoi(n2))
							{
								ctx->ip = (short)(VMGET_LABEL(op)); continue;
							}
							break;

		case VMOP_GET:		varno = VMGET_A(op);
							varno2 = VMGET_B(op);
							varno3 = VMGET_C(op);
							n1 = (varno2 == 0) ? &ctx->code[++ctx->ip] : &ctx->vars[varno2-1];
							n2 = (varno3 == 0) ? &ctx->code[++ctx->ip] : &ctx->vars[varno3-1];
							dsb_get(n1,n2,&ctx->vars[varno-1]);
							break;

		case VMOP_DEF:		varno = VMGET_A(op);
							varno2 = VMGET_B(op);
							varno3 = VMGET_C(op);
							n1 = (varno == 0) ? &ctx->code[++ctx->ip] : &ctx->vars[varno-1];
							n2 = (varno2 == 0) ? &ctx->code[++ctx->ip] : &ctx->vars[varno2-1];
							n3 = (varno3 == 0) ? &ctx->code[++ctx->ip] : &ctx->vars[varno3-1];
							dsb_define(n1,n2,n3,0);
							break;

		case VMOP_DEP:		varno = VMGET_A(op);
							varno2 = VMGET_B(op);
							varno3 = VMGET_C(op);
							varno4 = VMGET_D(op);
							n1 = (varno == 0) ? &ctx->code[++ctx->ip] : &ctx->vars[varno-1];
							n2 = (varno2 == 0) ? &ctx->code[++ctx->ip] : &ctx->vars[varno2-1];
							n3 = (varno3 == 0) ? &ctx->code[++ctx->ip] : &ctx->vars[varno3-1];
							n4 = (varno4 == 0) ? &ctx->code[++ctx->ip] : &ctx->vars[varno4-1];
							dsb_dependency(n1,n2,n3,n4);
							break;

		case VMOP_NEW:		varno = VMGET_A(op);
							varno2 = VMGET_B(op);
							n1 = (varno2 == 0) ? &ctx->code[++ctx->ip] : &ctx->vars[varno2-1];
							dsb_new(n1,&ctx->vars[varno-1]);
							break;

		case VMOP_ADD:		varno = VMGET_A(op);
							varno2 = VMGET_B(op);
							varno3 = VMGET_C(op);
							n1 = (varno2 == 0) ? &ctx->code[++ctx->ip] : &ctx->vars[varno2-1];
							n2 = (varno3 == 0) ? &ctx->code[++ctx->ip] : &ctx->vars[varno3-1];
							dsb_iton(dsb_ntoi(n1)+dsb_ntoi(n2),&ctx->vars[varno-1]);
							break;

		case VMOP_SUB:		varno = VMGET_A(op);
							varno2 = VMGET_B(op);
							varno3 = VMGET_C(op);
							n1 = (varno2 == 0) ? &ctx->code[++ctx->ip] : &ctx->vars[varno2-1];
							n2 = (varno3 == 0) ? &ctx->code[++ctx->ip] : &ctx->vars[varno3-1];
							dsb_iton(dsb_ntoi(n1)-dsb_ntoi(n2),&ctx->vars[varno-1]);
							break;

		case VMOP_DIV:		varno = VMGET_A(op);
							varno2 = VMGET_B(op);
							varno3 = VMGET_C(op);
							n1 = (varno2 == 0) ? &ctx->code[++ctx->ip] : &ctx->vars[varno2-1];
							n2 = (varno3 == 0) ? &ctx->code[++ctx->ip] : &ctx->vars[varno3-1];
							dsb_iton(dsb_ntoi(n1) / dsb_ntoi(n2),&ctx->vars[varno-1]);
							break;

		case VMOP_MUL:		varno = VMGET_A(op);
							varno2 = VMGET_B(op);
							varno3 = VMGET_C(op);
							n1 = (varno2 == 0) ? &ctx->code[++ctx->ip] : &ctx->vars[varno2-1];
							n2 = (varno3 == 0) ? &ctx->code[++ctx->ip] : &ctx->vars[varno3-1];
							dsb_iton(dsb_ntoi(n1)*dsb_ntoi(n2),&ctx->vars[varno-1]);
							break;

		case VMOP_AND:		varno = VMGET_A(op);
							varno2 = VMGET_B(op);
							varno3 = VMGET_C(op);
							n1 = (varno2 == 0) ? &ctx->code[++ctx->ip] : &ctx->vars[varno2-1];
							n2 = (varno3 == 0) ? &ctx->code[++ctx->ip] : &ctx->vars[varno3-1];
							dsb_iton(dsb_ntoi(n1) & dsb_ntoi(n2),&ctx->vars[varno-1]);
							break;

		case VMOP_OR:		varno = VMGET_A(op);
							varno2 = VMGET_B(op);
							varno3 = VMGET_C(op);
							n1 = (varno2 == 0) ? &ctx->code[++ctx->ip] : &ctx->vars[varno2-1];
							n2 = (varno3 == 0) ? &ctx->code[++ctx->ip] : &ctx->vars[varno3-1];
							dsb_iton(dsb_ntoi(n1) | dsb_ntoi(n2),&ctx->vars[varno-1]);
							break;

		case VMOP_XOR:		varno = VMGET_A(op);
							varno2 = VMGET_B(op);
							varno3 = VMGET_C(op);
							n1 = (varno2 == 0) ? &ctx->code[++ctx->ip] : &ctx->vars[varno2-1];
							n2 = (varno3 == 0) ? &ctx->code[++ctx->ip] : &ctx->vars[varno3-1];
							dsb_iton(dsb_ntoi(n1) ^ dsb_ntoi(n2),&ctx->vars[varno-1]);
							break;

		case VMOP_SHL:		varno = VMGET_A(op);
							varno2 = VMGET_B(op);
							varno3 = VMGET_C(op);
							n1 = (varno2 == 0) ? &ctx->code[++ctx->ip] : &ctx->vars[varno2-1];
							n2 = (varno3 == 0) ? &ctx->code[++ctx->ip] : &ctx->vars[varno3-1];
							dsb_iton(dsb_ntoi(n1) << dsb_ntoi(n2),&ctx->vars[varno-1]);
							break;

		case VMOP_SHR:		varno = VMGET_A(op);
							varno2 = VMGET_B(op);
							varno3 = VMGET_C(op);
							n1 = (varno2 == 0) ? &ctx->code[++ctx->ip] : &ctx->vars[varno2-1];
							n2 = (varno3 == 0) ? &ctx->code[++ctx->ip] : &ctx->vars[varno3-1];
							dsb_iton(dsb_ntoi(n1) >> dsb_ntoi(n2),&ctx->vars[varno-1]);
							break;

		case VMOP_INC:		varno = VMGET_A(op);
							ctx->vars[varno-1].ll++;
							break;

		case VMOP_DEC:		varno = VMGET_A(op);
							ctx->vars[varno-1].ll--;
							break;

		case VMOP_CLR:		varno = VMGET_A(op);
							ctx->vars[varno-1] = Null;
							break;

		default: 			{
								char buf[100];
								sprintf(buf, "op = %08x",(unsigned int)VMGET_OP(op));
								return DSB_ERROR(ERR_VMINVALIP,buf);
							}
		}

		ctx->ip++;
	}

	return SUCCESS;
}

