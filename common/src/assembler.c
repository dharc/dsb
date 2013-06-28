/*
 * assember.c
 *
 *  Created on: 31 May 2013
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
#include "dsb/assembler.h"
#include <malloc.h>
#include <string.h>

#define TOKEN_COMMENT ';'

//Auto split ops into functions.
struct AsmToken
{
	char *name;
	//int (*opfunc)(struct VMLabel *labels, const char *line, NID_t *output, int *ip);
	unsigned int op;
	int ls;		//Number of labels (0 or 1)
	int vs;		//Number of required variables
	int nvs;	//Number of required var/nids
	int varia;	//Does it support variadic parameters.
};

#define ASMOP(A,B,C,D,E,F)	{#A, B, C, D, E, F}

//TODO DONT USE GLOBAL VARIABLE FOR THIS
static int varid;

/*
 * Append a new label to the labels structure. Read the label name from line
 * and uses the value of *ip as the labels location.
 */
static int vm_assemble_addlabel(struct VMLabel *labels, const char *line, int *ip, int mode)
{
	int i;
	int len;
	for (len=0; len<9; len++)
	{
		if ((line[len] < 'a' || line[len] > 'z') && (line[len] < '0' || line[len] > '9')) break;
	}

	for (i=0; i<MAX_LABELS; i++)
	{
		if (labels[i].lip == -1)
		{
			strncpy(labels[i].label, line, len);
			labels[i].label[len] = 0;
			labels[i].lip = *ip;
			labels[i].mode = mode;
			break;
		}
		else if (strncmp(labels[i].label,line,len) == 0)
		{
			//Update value
			labels[i].lip = *ip;
			break;
		}
	}

	return len;
}

static int assemble_off(struct AsmContext *ctx,const char *line, int *off, int *ix);

/*
 * Search the labels list for a match and return the location.
 */
static int vm_assemble_lookup(struct AsmContext *ctx, const char *line, int *off)
{
	int i;
	int len;
	int dum = 0;

	*off = 0;

	len = 0;
	while (line[len] != ' ' && line[len] != '\n' && line[len] != 0) len++;

	for (i=0; i<MAX_LABELS; i++)
	{
		//len = strlen(ctx->labels[i].label);
		if (strncmp(ctx->labels[i].label,line,len) == 0)
		{
			if (ctx->labels[i].label[len] == 0)
			{
				*off = ctx->labels[i].lip;
				if (line[len] == '+' || line[len] == '-')
				{
					assemble_off(ctx, &line[len], &len, &dum);
					*off += len;
				}
				break;
			}
		}
	}
	return len;
}

/*
 * Parse an integer offset/address value or other integer constant
 */
static int assemble_off(struct AsmContext *ctx,const char *line, int *off, int *ix)
{
	int i=0;
	int neg = 0;
	*off = 0;

	if (line[0] == '$')
	{
		char errbuf[100];
		sprintf(errbuf,"offset or label expected :%d",ctx->line);
		ctx->error = 1;
		return DSB_ERROR(ERR_ASMNOTOFF,errbuf);
	}

	//Is it a label?
	if (line[0] == ':')
	{
		*ix = *ix + vm_assemble_lookup(ctx,&line[1],off) + 1;
		return 0;
	}

	if (line[0] == '-') {
		i++;
		neg = 1;
	}
	else if (line[0] == '+') {
		i++;
	}

	while (line[i] >= '0' && line[i] <= '9') {
		*off *= 10;
		*off += line[i] - '0';
		i++;
	}
	if (neg == 1) *off = -(*off);

	if (neg == 1 && i == 1) return 0;

	*ix = *ix + i;
	return 0;
}

/*
 * Parse a variable name and lookup value.
 */
static int assemble_var(struct AsmContext *ctx,const char *line, int *ix)
{
	//Is it a label?
	if (line[0] == '$')
	{
		int i;
		int len;

		for (i=0; i<MAX_LABELS; i++)
		{
			if (ctx->labels[i].lip == -1) break;
			len = strlen(ctx->labels[i].label);
			if (len == 0) continue;
			if (strncmp(ctx->labels[i].label,&line[1],len) == 0)
			{
				*ix += len+1;
				return ctx->labels[i].lip;
			}
		}

		//Not found so add the label.
		varid++;
		*ix = *ix + vm_assemble_addlabel(ctx->labels,&line[1],&varid, 1) + 1;

		return varid;
	}

	//DSB_ERROR(WARN_ASMEXPECTVAR,line);
	return 0;
}

static int assemble_nidvar(struct AsmContext *ctx, const char *line, NID_t *nid, int *ix)
{
	if (line[0] == '$')
	{
		return assemble_var(ctx,line,ix);
	}
	else
	{
		//Get a NID
		char buf[100];
		int i=0;
		while (line[i] != ' ' && line[i] != '\t' && line[i] != '.' && line[i] != '\n' && line[i] != 0)
		{
			buf[i] = line[i];
			i++;
		}
		buf[i] = 0;
		dsb_nid_fromStr(buf,nid);
		*ix += i;

		//0 means used NID not var.
		return 0;
	}
}

static int assemble_eol(const char *line, int *ix)
{
	int i=0;
	while (line[i] == ' ' || line[i] == '\t') i++;
	*ix = *ix + i;
	if ((line[i] == '\n') || (line[i] == 0) || (line[i] == TOKEN_COMMENT)) return 1;
	return 0;
}


static int assemble_op(struct AsmToken *tok, struct AsmContext *ctx, const char *line)
{
	int i=0;
	int off = 0;
	int varA = 0;
	int varB = 0;
	int varC = 0;
	int varD = 0;
	NID_t n1;
	NID_t n2;
	NID_t n3;
	NID_t n4;
	NID_t variad;
	NID_t varnid[8];
	int varnum[8];

	//Parse required labels
	if (tok->ls > 0)
	{
		if (assemble_eol(&line[i], &i) == 1)
		{
			char errbuf[100];
			sprintf(errbuf,"label expected :%d",ctx->line);
			ctx->error = 1;
			return DSB_ERROR(ERR_ASMMISSING,errbuf);
		}
		if (assemble_off(ctx,&line[i],&off,&i) != 0) return 0;
	}

	if (tok->vs > 0)
	{
		if (assemble_eol(&line[i], &i) == 1)
		{
			char errbuf[100];
			sprintf(errbuf,"variable expected :%d",ctx->line);
			ctx->error = 1;
			return DSB_ERROR(ERR_ASMMISSING,errbuf);
		}
		varA = assemble_var(ctx, &line[i], &i);
		if (varA == 0)
		{
			char errbuf[100];
			sprintf(errbuf,":%d",ctx->line);
			ctx->error = 1;
			return DSB_ERROR(ERR_ASMNOTVAR,errbuf);
		}

		if (tok->nvs > 0)
		{
			if (assemble_eol(&line[i], &i) == 1)
			{
				char errbuf[100];
				sprintf(errbuf,"variable or NID expected :%d",ctx->line);
				ctx->error = 1;
				return DSB_ERROR(ERR_ASMMISSING,errbuf);
			}
			varB = assemble_nidvar(ctx, &line[i], &n2, &i);
		}
		if (tok->nvs > 1)
		{
			if (assemble_eol(&line[i], &i) == 1)
			{
				char errbuf[100];
				sprintf(errbuf,"variable or NID expected :%d",ctx->line);
				ctx->error = 1;
				return DSB_ERROR(ERR_ASMMISSING,errbuf);
			}
			varC = assemble_nidvar(ctx, &line[i], &n3, &i);
		}
		if (tok->nvs > 2)
		{
			if (assemble_eol(&line[i], &i) == 1)
			{
				char errbuf[100];
				sprintf(errbuf,"variable or NID expected :%d",ctx->line);
				ctx->error = 1;
				return DSB_ERROR(ERR_ASMMISSING,errbuf);
			}
			varD = assemble_nidvar(ctx, &line[i], &n4, &i);
		}
	}
	else
	{
		if (tok->nvs > 0)
		{
			if (assemble_eol(&line[i], &i) == 1)
			{
				char errbuf[100];
				sprintf(errbuf,"variable or NID expected :%d",ctx->line);
				ctx->error = 1;
				return DSB_ERROR(ERR_ASMMISSING,errbuf);
			}
			varA = assemble_nidvar(ctx, &line[i], &n1, &i);
		}
		if (tok->nvs > 1)
		{
			if (assemble_eol(&line[i], &i) == 1)
			{
				char errbuf[100];
				sprintf(errbuf,"variable or NID expected :%d",ctx->line);
				ctx->error = 1;
				return DSB_ERROR(ERR_ASMMISSING,errbuf);
			}
			varB = assemble_nidvar(ctx, &line[i], &n2, &i);
		}
		if (tok->nvs > 2)
		{
			if (assemble_eol(&line[i], &i) == 1)
			{
				char errbuf[100];
				sprintf(errbuf,"variable or NID expected :%d",ctx->line);
				ctx->error = 1;
				return DSB_ERROR(ERR_ASMMISSING,errbuf);
			}
			varC = assemble_nidvar(ctx, &line[i], &n3, &i);
		}
		if (tok->nvs > 3)
		{
			if (assemble_eol(&line[i], &i) == 1)
			{
				char errbuf[100];
				sprintf(errbuf,"variable or NID expected :%d",ctx->line);
				ctx->error = 1;
				return DSB_ERROR(ERR_ASMMISSING,errbuf);
			}
			varD = assemble_nidvar(ctx, &line[i], &n4, &i);
		}
	}

	//Now check for variable parameters.
	if (tok->varia == 1)
	{
		int curvar = 0;

		//Init to 0.
		dsb_iton(0,&variad);

		//While not end of line
		while (assemble_eol(&line[i],&i) == 0)
		{
			varnum[curvar] = assemble_nidvar(ctx, &line[i], &varnid[curvar], &i);
			variad.ll |= (long long)varnum[curvar] << ((7-curvar) * 8);
			curvar++;
		}

		//Save count into the primary ops offset bits.
		off = curvar;
	}

	//Should be end of line now.
	if (assemble_eol(&line[i], &i) == 0)
	{
		char errbuf[100];
		sprintf(errbuf,":%d",ctx->line);
		ctx->error = 1;
		return DSB_ERROR(ERR_ASMTOOMANY,errbuf);
	}

	//Insert OP into output
	dsb_nid_op(((unsigned long long)tok->op << 32) | ((unsigned long long)off << 32) | (varA << 24) | (varB << 16) | (varC << 8) | varD,&ctx->output[ctx->ip]);
	ctx->ip = ctx->ip + 1;
	//Insert optional constant NIDs
	if (tok->vs > 0)
	{
		if (varB == 0 && tok->nvs >= 1) ctx->output[ctx->ip++] = n2;
		if (varC == 0 && tok->nvs >= 2) ctx->output[ctx->ip++] = n3;
		if (varD == 0 && tok->nvs >= 3) ctx->output[ctx->ip++] = n4;
	}
	else
	{
		if (varA == 0 && tok->nvs >= 1) ctx->output[ctx->ip++] = n1;
		if (varB == 0 && tok->nvs >= 2) ctx->output[ctx->ip++] = n2;
		if (varC == 0 && tok->nvs >= 3) ctx->output[ctx->ip++] = n3;
		if (varD == 0 && tok->nvs >= 4) ctx->output[ctx->ip++] = n4;
	}

	//If variable params then dump any constant ones into output
	if (tok->varia == 1)
	{
		ctx->output[ctx->ip++] = variad;
		if (off >= 1 && varnum[0] == 0) ctx->output[ctx->ip++] = varnid[0];
		if (off >= 2 && varnum[1] == 0) ctx->output[ctx->ip++] = varnid[1];
		if (off >= 3 && varnum[2] == 0) ctx->output[ctx->ip++] = varnid[2];
		if (off >= 4 && varnum[3] == 0) ctx->output[ctx->ip++] = varnid[3];
		if (off >= 5 && varnum[4] == 0) ctx->output[ctx->ip++] = varnid[4];
		if (off >= 6 && varnum[5] == 0) ctx->output[ctx->ip++] = varnid[5];
		if (off >= 7 && varnum[6] == 0) ctx->output[ctx->ip++] = varnid[6];
		if (off >= 8 && varnum[7] == 0) ctx->output[ctx->ip++] = varnid[7];
	}

	return 0;
}

static struct AsmToken asmops[] = {
		//Name, OP, LS, VS, NVS
		ASMOP(jmp,VMOP_JMP,1,0,0,0),
		ASMOP(jeq,VMOP_JEQ,1,0,2,0),
		ASMOP(jne,VMOP_JNE,1,0,2,0),
		ASMOP(jlt,VMOP_JLT,1,0,2,0),
		ASMOP(jgt,VMOP_JGT,1,0,2,0),
		ASMOP(jle,VMOP_JLE,1,0,2,0),
		ASMOP(jge,VMOP_JGE,1,0,2,0),

		ASMOP(get,VMOP_GET,0,1,2,0),
		ASMOP(getd,VMOP_GETD,0,1,2,0),
		ASMOP(def,VMOP_DEF,0,0,3,0),
		ASMOP(dep,VMOP_DEP,0,0,4,0),
		ASMOP(new,VMOP_NEW,0,1,1,0),
		ASMOP(del,VMOP_DEL,0,0,2,0),

		ASMOP(cpy,VMOP_CPY,0,1,1,0),
		ASMOP(ret,VMOP_RET,0,0,1,0),
		//ASMOP(const),

		ASMOP(inc,VMOP_INC,0,1,0,0),
		ASMOP(dec,VMOP_DEC,0,1,0,0),
		ASMOP(add,VMOP_ADD,0,1,2,0),
		ASMOP(sub,VMOP_SUB,0,1,2,0),
		ASMOP(mul,VMOP_MUL,0,1,2,0),
		ASMOP(div,VMOP_DIV,0,1,2,0),
		ASMOP(and,VMOP_AND,0,1,2,0),
		ASMOP(or,VMOP_OR,0,1,2,0),
		ASMOP(xor,VMOP_XOR,0,1,2,0),
		ASMOP(neg,VMOP_NEG,0,1,2,0),
		ASMOP(shl,VMOP_SHL,0,1,2,0),
		ASMOP(shr,VMOP_SHR,0,1,2,0),
		ASMOP(clr,VMOP_CLR,0,1,0,0),
		ASMOP(int,VMOP_INT,0,1,1,0),
		ASMOP(flt,VMOP_FLT,0,1,1,0),

		ASMOP(call,VMOP_CALL,0,1,1,1),	//Variable params...
		ASMOP(callx,VMOP_CALLX,0,1,1,1),

		//TODO high level ops.
		{"null",0,0,0,0}
};

/*
 * Assemble a single line, fill output and return number of elements
 * written to the output.
 */
int dsb_assemble_line(struct AsmContext *ctx, const char *line)
{
	int i = 0;
	char opbuf[20];
	char *tmp;

	//Remove leading white space.
	while (line[i] == ' ' || line[i] == '\t') ++i;

	//Check for blank line or end of input.
	if (line[i] == 0 || line[i] == '\n') return 0;
	//Check for a comment line.
	if (line[i] == TOKEN_COMMENT) return 0;

	//Do we have a label?
	if (line[i] == ':')
	{
		vm_assemble_addlabel(ctx->labels,&line[i+1],&ctx->ip, 0);
		return 0;
	}

	//Parse operation into buffer (space ends op)
	tmp = strchr(&line[i],' ');
	if (tmp == 0) return 0;
	strncpy(opbuf,&line[i],tmp-(&line[i]));
	opbuf[tmp-(&line[i])] = 0;

	//Search for operation parser function
	i = 0;
	while (asmops[i].op != 0)
	{
		if (strcmp(opbuf,asmops[i].name) == 0)
		{
			//Parse operands and generate opcode.
			assemble_op(&asmops[i],ctx,tmp+1);
			break;
		}
		i++;
	}

	if (asmops[i].op == 0)
	{
		char errbuf[100];
		sprintf(errbuf,"%s :%d",opbuf,ctx->line);
		ctx->error = 1;
		DSB_ERROR(ERR_ASMUNKOP,errbuf);
	}

	return 1;
}

int dsb_assemble_compile(struct AsmContext *ctx, const char *source)
{
	ctx->ip = 0;
	ctx->error = 0;
	ctx->line = 1;

	//For every line
	while(1)
	{
		//Do we have a control command
		if (source[0] == '.')
		{
			if (strncmp(source,".param",6) == 0)
			{
				//Increment varid to reserve space
				varid++;
				sprintf(ctx->labels[varid-1].label,"%d",varid-1);
				ctx->labels[varid-1].lip = varid;
				ctx->labels[varid-1].mode = 1;
			}
			else if (strncmp(source,".const",6) == 0)
			{
				//Add a constant to context for future substitution.
			}
		}
		else
		{
			dsb_assemble_line(ctx,source);
		}

		//Move to next line if there is one.
		source = strchr(source,'\n');
		if (source == 0) break;
		source++;
		ctx->line++;
	}

	return 0;
}

int dsb_assemble_array(const char *source, NID_t *output, int max)
{
	struct AsmContext ctx;
	ctx.output = output;
	ctx.maxout = max;
	ctx.labels = malloc(sizeof(struct VMLabel)*MAX_LABELS);

	dsb_assemble_labels(ctx.labels,source);

	//Compile once without valid labels.
	dsb_assemble_compile(&ctx, source);
	//Compile again to patch labels.
	//TODO find way not to do a double compile.
	dsb_assemble_compile(&ctx, source);

	free(ctx.labels);
	return ctx.ip;
}

int dsb_assemble(const char *source, const NID_t *output)
{
	NID_t *array = malloc(sizeof(NID_t)*1000);
	int ret;

	ret = dsb_assemble_array(source,array,1000);
	dsb_array_write(array,ret,output);
	return ret;
}

int dsb_assemble_labels(struct VMLabel *labels, const char *source)
{
	int i;
	varid = 0;

	for (i=0; i<MAX_LABELS; i++)
	{
		labels[i].lip = -1;
		labels[i].mode = 0;
	}

	return SUCCESS;
}

int dsb_disassemble(const NID_t *src, int size, char *output, int max)
{
	int i;
	int j;
	int pc;
	char bufa[100];
	char bufb[100];
	char bufc[100];
	char bufd[100];
	int op;

	for (i=0; i<size; i++)
	{
		if (src[i].header == 0 && src[i].t == NID_TYPE_VMOP)
		{
			op = VMGET_OP(src[i].ll);

			j = 0;
			while (asmops[j].op != 0)
			{
				if (op == asmops[j].op)
				{
					int a = (int)VMGET_A(src[i].ll);
					int b = (int)VMGET_B(src[i].ll);
					int c = (int)VMGET_C(src[i].ll);
					int d = (int)VMGET_D(src[i].ll);
					int off = (int)VMGET_LABEL(src[i].ll);

					pc = asmops[j].nvs + asmops[j].vs;
					if (pc > 0)
					{
						if (a == 0)
						{
							dsb_nid_toStr(&src[++i],bufa,100);
						}
						else
						{
							sprintf(bufa,"$%d",a-1);
						}
					}
					if (pc > 1)
					{
						if (b == 0)
						{
							dsb_nid_toStr(&src[++i],bufb,100);
						}
						else
						{
							sprintf(bufb,"$%d",b-1);
						}
					}
					if (pc > 2)
					{
						if (c == 0)
						{
							dsb_nid_toStr(&src[++i],bufc,100);
						}
						else
						{
							sprintf(bufc,"$%d",c-1);
						}
					}
					if (pc > 3)
					{
						if (d == 0)
						{
							dsb_nid_toStr(&src[++i],bufd,100);
						}
						else
						{
							sprintf(bufd,"$%d",d-1);
						}
					}

					if (asmops[j].ls > 0)
					{
						switch(pc)
						{
						case 0: sprintf(output,"%s %d\n",asmops[j].name,off); break;
						case 1: sprintf(output,"%s %d %s\n",asmops[j].name,off,bufa); break;
						case 2: sprintf(output,"%s %d %s %s\n",asmops[j].name,off,bufa,bufb); break;
						case 3: sprintf(output,"%s %d %s %s %s\n",asmops[j].name,off,bufa,bufb,bufc); break;
						case 4: sprintf(output,"%s %d %s %s %s %s\n",asmops[j].name,off,bufa,bufb,bufc,bufd); break;
						}
					}
					else
					{
						switch(pc)
						{
						case 0: sprintf(output,"%s",asmops[j].name); break;
						case 1: sprintf(output,"%s %s",asmops[j].name,bufa); break;
						case 2: sprintf(output,"%s %s %s",asmops[j].name,bufa,bufb); break;
						case 3: sprintf(output,"%s %s %s %s",asmops[j].name,bufa,bufb,bufc); break;
						case 4: sprintf(output,"%s %s %s %s %s",asmops[j].name,bufa,bufb,bufc,bufd); break;
						}
					}

					//Deal with variable parameters.
					if (asmops[j].varia == 1)
					{
						int count = off;
						NID_t varbyte = src[++i];
						int k;

						for (k=0; k<count; k++)
						{
							a = (varbyte.ll >> ((7-k) * 8)) & 0xFF;
							if (a == 0)
							{
								dsb_nid_toStr(&src[++i],bufa,100);
							}
							else
							{
								sprintf(bufa,"$%d",a-1);
							}
							sprintf(output,"%s %s",output,bufa);
						}
					}

					sprintf(output,"%s\n",output);
					break;
				}
				j++;
			}

			output += strlen(output);
		}
		else
		{
			//ERROR
		}

	}
	return 0;
}

