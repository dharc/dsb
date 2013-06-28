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

#include "dsb/core/vm.h"
#include "dsb/core/harc.h"
#include "dsb/errors.h"
#include "dsb/core/nid.h"
#include "dsb/core/specials.h"
#include "dsb/wrap.h"
#include "dsb/patterns/array.h"
#include "dsb/globals.h"
#include <malloc.h>
#include <stdarg.h>

//#ifdef X86_64
#include "arch/vm_x86_64.c"
//#endif

#define MAX_XFUNCS	1000

static XFUNC_t xfuncs[MAX_XFUNCS];

int dsb_vm_xfunc(unsigned int id, const char *name, XFUNC_t func)
{
	if (id >= MAX_XFUNCS)
	{
		return DSB_ERROR(ERR_VMXFUNCID,name);
	}

	if (xfuncs[id] != 0)
	{
		return DSB_ERROR(ERR_VMXFUNCEXIST,name);
	}

	xfuncs[id] = func;
	return SUCCESS;
}

int dsb_vm_context(struct VMContext *ctx, const NID_t *func)
{
	ctx->timeout = 10000;
	ctx->ip = 0;
	//TODO DO A JIT LOOKUP HERE
	ctx->code = malloc(sizeof(NID_t)*1000);
	//Read in the code
	ctx->codesize = dsb_array_read(func, ctx->code, 1000);
	//TODO DO A JIT COMPILE HERE

	if (ctx->codesize == 0)
	{
		return DSB_ERROR(ERR_VMNOCODE,0);
	}

	return 0;
}

int dsb_vm_call(NID_t *res, const NID_t *func, int pn, ...)
{
	va_list args;
	struct VMContext ctx;
	int i;

	va_start(args,pn);

	dsb_vm_context(&ctx, func);
	ctx.result = res;

	for (i=0; i<pn; i++)
	{
		ctx.vars[i] = *va_arg(args,NID_t*);
	}

	va_end(args);

	//Run the interpreter.
	dsb_vm_interpret(&ctx);

	free(ctx.code);

	return SUCCESS;
}

int dsb_vm_interpret(struct VMContext *ctx)
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
		if (ctx->code[ctx->ip].t != NID_TYPE_VMOP)	return DSB_ERROR(ERR_VMINVALIP,0);
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

		case VMOP_GETD:		varno = VMGET_A(op);
							varno2 = VMGET_B(op);
							varno3 = VMGET_C(op);
							n1 = (varno2 == 0) ? &ctx->code[++ctx->ip] : &ctx->vars[varno2-1];
							n2 = (varno3 == 0) ? &ctx->code[++ctx->ip] : &ctx->vars[varno3-1];
							dsb_get(n1,n2,&ctx->vars[varno-1]);
							dsb_dependency(n1,n2,&ctx->vars[0],&ctx->vars[1]);
							break;

		case VMOP_DEF:		varno = VMGET_A(op);
							varno2 = VMGET_B(op);
							varno3 = VMGET_C(op);
							n1 = (varno == 0) ? &ctx->code[++ctx->ip] : &ctx->vars[varno-1];
							n2 = (varno2 == 0) ? &ctx->code[++ctx->ip] : &ctx->vars[varno2-1];
							n3 = (varno3 == 0) ? &ctx->code[++ctx->ip] : &ctx->vars[varno3-1];
							dsb_define(n1,n2,n3);
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

		case VMOP_CALLX:	varno = VMGET_A(op);
							varno2 = VMGET_B(op);
							n1 = (varno2 == 0) ? &ctx->code[++ctx->ip] : &ctx->vars[varno2-1];
							{
								int xfuncid = dsb_ntoi(n1);
								XFUNC_t func;
								NID_t vars[8];
								int parno;
								int i;

								if (xfuncid < 0 || xfuncid > MAX_XFUNCS)
								{
									DSB_ERROR(ERR_VMXFUNCID,0);
									break;
								}

								func = xfuncs[xfuncid];
								if (func == 0)
								{
									printf("Xfuncid: %d", xfuncid);
									DSB_ERROR(ERR_VMXFUNCNONE,0);
									break;
								}

								if (VMGET_LABEL(op) > 0)
								{
									n2 = &ctx->code[++ctx->ip];
								}

								//Extract and set parameters from vars or constants.
								for (i=0; i<VMGET_LABEL(op); i++)
								{
									parno = (n2->ll >> ((7-i) * 8)) & 0xFF;
									vars[i] = (parno == 0) ? ctx->code[++ctx->ip] : ctx->vars[parno-1];
								}

								func(&ctx->vars[varno-1],i,vars);
							}
							break;

		case VMOP_CALL:		varno = VMGET_A(op);
							varno2 = VMGET_B(op);
							n1 = (varno2 == 0) ? &ctx->code[++ctx->ip] : &ctx->vars[varno2-1];
							{
								int i;
								int parno;
								struct VMContext nctx;
								dsb_vm_context(&nctx, n1);
								nctx.result = &ctx->vars[varno-1];

								if (VMGET_LABEL(op) > 0)
								{
									n2 = &ctx->code[++ctx->ip];
								}

								//Extract and set parameters from vars or constants.
								for (i=0; i<VMGET_LABEL(op); i++)
								{
									parno = (n2->ll >> ((7-i) * 8)) & 0xFF;
									nctx.vars[i] = (parno == 0) ? ctx->code[++ctx->ip] : ctx->vars[parno-1];
								}

								//Run the interpreter.
								dsb_vm_interpret(&nctx);
								free(nctx.code);
							}
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

