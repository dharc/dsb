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

#include "dsb/vm.h"
#include "dsb/harc.h"
#include "dsb/errors.h"
#include "dsb/nid.h"
#include "dsb/specials.h"
#include "dsb/wrap.h"
#include "dsb/array.h"
#include "dsb/assembler.h"
#include <malloc.h>
#include <string.h>

struct AsmToken
{
	char *name;
	int (*opfunc)(struct VMLabel *labels, const char *line, NID_t *output, int *ip);
};

#define ASMOP(A)	{#A, asm_##A}

/*
 * Append a new label to the labels structure. Read the label name from line
 * and uses the value of *ip as the labels location.
 */
static void vm_assemble_addlabel(struct VMLabel *labels, const char *line, int *ip)
{
	int i;
	int len;
	for (len=0; len<9; len++)
	{
		if (line[len] < 'a' || line[len] > 'z') break;
	}

	for (i=0; i<MAX_LABELS; i++)
	{
		if (labels[i].lip == -1)
		{
			strncpy(labels[i].label, line, len);
			labels[i].label[len] = 0;
			labels[i].lip = *ip;
			break;
		}
	}
}

static int vm_assemble_off(struct VMLabel *labels,const char *line, int *off);

/*
 * Search the labels list for a match and return the location.
 */
static int vm_assemble_lookup(struct VMLabel *labels, const char *line)
{
	int i;
	int len;
	int off = 0;

	for (i=0; i<MAX_LABELS; i++)
	{
		len = strlen(labels[i].label);
		if (strncmp(labels[i].label,line,len) == 0)
		{
			off = labels[i].lip;
			if (line[len] == '+' || line[len] == '-')
			{
				vm_assemble_off(labels, &line[len], &len);
				off += len;
			}
			break;
		}
	}
	return off;
}

/*
 * Parse an integer offset/address value or other integer constant
 */
static int vm_assemble_off(struct VMLabel *labels,const char *line, int *off)
{
	int i=0;
	int neg = 0;
	*off = 0;

	//Is it a label?
	if (line[0] == ':')
	{
		*off = vm_assemble_lookup(labels,&line[1]);
		return 1;
	}

	if (line[0] == '-') {
		i++;
		neg = 1;
	}

	if (line[0] == '+') {
		i++;
	}

	while (line[i] >= '0' && line[i] <= '9') {
		*off *= 10;
		*off += line[i] - '0';
		i++;
	}
	if (neg == 1) *off = -(*off);

	if (neg == 1 && i == 1) return 0;

	return i;
}

/*
 * Parse a register number.
 */
static int vm_assemble_reg(const char *line, int *reg, int *i)
{
	if (line[0] == '%')
	{
		//Read a single HEX digit for reg id.
		if (line[1] >= '0' && line[1] <= '9') *reg = line[1]-'0';
		else if (line[1] >= 'a' && line[1] <= 'f') *reg = line[1]-'a';
		else return 0;

		*i += 2;
		return 1;
	}

	DSB_ERROR(ERR_ASMNOTREG,line);
	return 0;
}

static int asm_load(struct VMLabel *labels, const char *line, NID_t *output, int *ip)
{
	int tmp;
	int regA;
	int regB;
	int regC = 0;
	int i = 0;

	//Get a register
	if (vm_assemble_reg(&line[i],&regA, &i) == 0) return ERR_ASMNOTREG;

	++i; //White space

	//Multi-register load
	if (line[i] == '%')
	{
		//Get a register
		if (vm_assemble_reg(&line[i],&regB, &i) == 0) return ERR_ASMNOTREG;

		//With an offset as well
		if (line[i] == '+' || line[i] == '-')
		{
			//Get an offset value
			tmp = vm_assemble_off(labels,&line[i],&regC);
			if (tmp == 0) return DSB_ERROR(ERR_ASMNOTOFF,&line[i]);
			i += tmp;
		}

		//Insert OP into output
		dsb_nid_op(VM_LOADR(regA,regB,regC),&output[*ip]);
		*ip = *ip + 1;
	}
	else
	{
		//Get an offset value
		tmp = vm_assemble_off(labels,&line[i],&regB);
		if (tmp == 0) return DSB_ERROR(ERR_ASMNOTOFF,&line[i]);
		i += tmp;

		//Insert OP into output
		dsb_nid_op(VM_LOAD(regA,regB),&output[*ip]);
		*ip = *ip + 1;
	}

	return i;
}

static int asm_store(struct VMLabel *labels, const char *line, NID_t *output, int *ip)
{
	return 0;
}

static int asm_jump(struct VMLabel *labels, const char *line, NID_t *output, int *ip)
{
	int i = 0;
	int tmp;
	int regA;

	//Get an offset value
	tmp = vm_assemble_off(labels,&line[i],&regA);
	if (tmp == 0) return DSB_ERROR(ERR_ASMNOTOFF,&line[i]);
	i += tmp;

	//Must be signed char size.
	if (regA < -128 || regA > 127) return DSB_ERROR(ERR_ASMINVALOFF,&line[i]);

	//Insert OP into output
	dsb_nid_op(VM_JUMP(regA),&output[*ip]);
	*ip = *ip + 1;

	return i;
}

static int asm_jeq(struct VMLabel *labels, const char *line, NID_t *output, int *ip)
{
	int i=0;
	int tmp;
	int regA;
	int regB;
	int regC;

	//Get a register
	if (vm_assemble_reg(&line[i],&regA, &i) == 0) return ERR_ASMNOTREG;

	++i; //White Space.

	//Get a register
	if (vm_assemble_reg(&line[i],&regB, &i) == 0) return ERR_ASMNOTREG;

	++i; //White Space.

	//Get an offset value
	tmp = vm_assemble_off(labels,&line[i],&regC);
	if (tmp == 0) return DSB_ERROR(ERR_ASMNOTOFF,&line[i]);
	i += tmp;

	//Must be signed char size.
	if (regC < -128 || regC > 127) return DSB_ERROR(ERR_ASMINVALOFF,&line[i]);

	//Insert OP into output
	dsb_nid_op(VM_JEQ(regA,regB,regC),&output[*ip]);
	*ip = *ip + 1;

	return i;
}

static int asm_jneq(struct VMLabel *labels, const char *line, NID_t *output, int *ip)
{
	int i=0;
	int tmp;
	int regA;
	int regB;
	int regC;

	//Get a register
	if (vm_assemble_reg(&line[i],&regA, &i) == 0) return ERR_ASMNOTREG;

	++i; //White Space.

	//Get a register
	if (vm_assemble_reg(&line[i],&regB, &i) == 0) return ERR_ASMNOTREG;

	++i; //White Space.

	//Get an offset value
	tmp = vm_assemble_off(labels,&line[i],&regC);
	if (tmp == 0) return DSB_ERROR(ERR_ASMNOTOFF,&line[i]);
	i += tmp;

	//Must be signed char size.
	if (regC < -128 || regC > 127) return DSB_ERROR(ERR_ASMINVALOFF,&line[i]);

	//Insert OP into output
	dsb_nid_op(VM_JNEQ(regA,regB,regC),&output[*ip]);
	*ip = *ip + 1;

	return i;
}

static int asm_jlt(struct VMLabel *labels, const char *line, NID_t *output, int *ip)
{
	return 0;
}

static int asm_jgt(struct VMLabel *labels, const char *line, NID_t *output, int *ip)
{
	return 0;
}

static int asm_jleq(struct VMLabel *labels, const char *line, NID_t *output, int *ip)
{
	return 0;
}

static int asm_jgeq(struct VMLabel *labels, const char *line, NID_t *output, int *ip)
{
	return 0;
}

static int asm_read(struct VMLabel *labels, const char *line, NID_t *output, int *ip)
{
	int i=0;
	int regA;
	int regB;
	int regC;

	//Get a register
	if (vm_assemble_reg(&line[i],&regA, &i) == 0) return ERR_ASMNOTREG;

	++i; //White Space.

	//Get a register
	if (vm_assemble_reg(&line[i],&regB, &i) == 0) return ERR_ASMNOTREG;

	++i; //White Space.

	//Get a register
	if (vm_assemble_reg(&line[i],&regC, &i) == 0) return ERR_ASMNOTREG;

	//Insert OP into output
	dsb_nid_op(VM_READ(regA,regB,regC),&output[*ip]);
	*ip = *ip + 1;

	return i;
}

static int asm_write(struct VMLabel *labels, const char *line, NID_t *output, int *ip)
{
	int i=0;
	int tmp;
	int regA;
	int regB;
	int regC;
	int regD;

	//Get a register
	if (vm_assemble_reg(&line[i],&regA, &i) == 0) return ERR_ASMNOTREG;

	++i; //White Space.

	//Get a register
	if (vm_assemble_reg(&line[i],&regB, &i) == 0) return ERR_ASMNOTREG;

	++i; //White Space.

	//Get a register
	if (vm_assemble_reg(&line[i],&regC, &i) == 0) return ERR_ASMNOTREG;

	++i;

	//Get an evaluator value
	tmp = vm_assemble_off(labels,&line[i],&regD);
	if (tmp == 0) return DSB_ERROR(ERR_ASMNOTOFF,&line[i]);
	i += tmp;

	//Insert OP into output
	dsb_nid_op(VM_WRITE(regA,regB,regC,regD),&output[*ip]);
	*ip = *ip + 1;

	return i;
}

static int asm_dep(struct VMLabel *labels, const char *line, NID_t *output, int *ip)
{
	int i=0;
	int regA;
	int regB;
	int regC;
	int regD;

	//Get a register
	if (vm_assemble_reg(&line[i],&regA, &i) == 0) return ERR_ASMNOTREG;

	++i; //White Space.

	//Get a register
	if (vm_assemble_reg(&line[i],&regB, &i) == 0) return ERR_ASMNOTREG;

	++i; //White Space.

	//Get a register
	if (vm_assemble_reg(&line[i],&regC, &i) == 0) return ERR_ASMNOTREG;

	++i;

	//Get a register
	if (vm_assemble_reg(&line[i],&regD, &i) == 0) return ERR_ASMNOTREG;

	//Insert OP into output
	dsb_nid_op(VM_DEP(regA,regB,regC,regD),&output[*ip]);
	*ip = *ip + 1;

	return i;
}

static int asm_new(struct VMLabel *labels, const char *line, NID_t *output, int *ip)
{
	return 0;
}

static int asm_delete(struct VMLabel *labels, const char *line, NID_t *output, int *ip)
{
	return 0;
}

static int asm_copy(struct VMLabel *labels, const char *line, NID_t *output, int *ip)
{
	int i = 0;
	int regA, regB;

	//Get a register
	if (vm_assemble_reg(&line[i],&regA, &i) == 0) return ERR_ASMNOTREG;

	++i; //White space.

	//Get a register
	if (vm_assemble_reg(&line[i],&regB, &i) == 0) return ERR_ASMNOTREG;

	//Insert OP into output
	dsb_nid_op(VM_COPY(regA,regB),&output[*ip]);
	*ip = *ip + 1;

	return i;
}

static int asm_ret(struct VMLabel *labels, const char *line, NID_t *output, int *ip)
{
	int i=0;
	int regA;

	//Get a register
	if (vm_assemble_reg(&line[i],&regA, &i) == 0) return ERR_ASMNOTREG;

	//Insert OP into output
	dsb_nid_op(VM_RET(regA),&output[*ip]);
	*ip = *ip + 1;

	return i;
}

static int asm_data(struct VMLabel *labels, const char *line, NID_t *output, int *ip)
{
	int i=0;

	//Insert OP into output
	dsb_nid_fromStr(&line[i],&output[*ip]);
	*ip = *ip + 1;

	return 0;
}

static int asm_inc(struct VMLabel *labels, const char *line, NID_t *output, int *ip)
{
	int i=0;
	int regA;

	//Get a register
	if (vm_assemble_reg(&line[i],&regA, &i) == 0) return ERR_ASMNOTREG;

	//Insert OP into output
	dsb_nid_op(VM_INC(regA),&output[*ip]);
	*ip = *ip + 1;

	return i;
}

static int asm_dec(struct VMLabel *labels, const char *line, NID_t *output, int *ip)
{
	int i=0;
	int regA;

	//Get a register
	if (vm_assemble_reg(&line[i],&regA, &i) == 0) return ERR_ASMNOTREG;

	//Insert OP into output
	dsb_nid_op(VM_DEC(regA),&output[*ip]);
	*ip = *ip + 1;

	return i;
}

static int asm_add(struct VMLabel *labels, const char *line, NID_t *output, int *ip)
{
	int i=0;
	int regA;
	int regB;
	int regC;

	//Get a register
	if (vm_assemble_reg(&line[i],&regA, &i) == 0) return ERR_ASMNOTREG;

	++i; //White Space.

	//Get a register
	if (vm_assemble_reg(&line[i],&regB, &i) == 0) return ERR_ASMNOTREG;

	++i; //White Space.

	//Get a register
	if (vm_assemble_reg(&line[i],&regC, &i) == 0) return ERR_ASMNOTREG;

	//Insert OP into output
	dsb_nid_op(VM_ADD(regA,regB,regC),&output[*ip]);
	*ip = *ip + 1;

	return i;
}

static int asm_sub(struct VMLabel *labels, const char *line, NID_t *output, int *ip)
{
	int i=0;
	int regA;
	int regB;
	int regC;

	//Get a register
	if (vm_assemble_reg(&line[i],&regA, &i) == 0) return ERR_ASMNOTREG;

	++i; //White Space.

	//Get a register
	if (vm_assemble_reg(&line[i],&regB, &i) == 0) return ERR_ASMNOTREG;

	++i; //White Space.

	//Get a register
	if (vm_assemble_reg(&line[i],&regC, &i) == 0) return ERR_ASMNOTREG;

	//Insert OP into output
	dsb_nid_op(VM_SUB(regA,regB,regC),&output[*ip]);
	*ip = *ip + 1;

	return i;
}

static int asm_mul(struct VMLabel *labels, const char *line, NID_t *output, int *ip)
{
	int i=0;
	int regA;
	int regB;
	int regC;

	//Get a register
	if (vm_assemble_reg(&line[i],&regA, &i) == 0) return ERR_ASMNOTREG;

	++i; //White Space.

	//Get a register
	if (vm_assemble_reg(&line[i],&regB, &i) == 0) return ERR_ASMNOTREG;

	++i; //White Space.

	//Get a register
	if (vm_assemble_reg(&line[i],&regC, &i) == 0) return ERR_ASMNOTREG;

	//Insert OP into output
	dsb_nid_op(VM_MUL(regA,regB,regC),&output[*ip]);
	*ip = *ip + 1;

	return i;
}

static int asm_div(struct VMLabel *labels, const char *line, NID_t *output, int *ip)
{
	int i=0;
	int regA;
	int regB;
	int regC;

	//Get a register
	if (vm_assemble_reg(&line[i],&regA, &i) == 0) return ERR_ASMNOTREG;

	++i; //White Space.

	//Get a register
	if (vm_assemble_reg(&line[i],&regB, &i) == 0) return ERR_ASMNOTREG;

	++i; //White Space.

	//Get a register
	if (vm_assemble_reg(&line[i],&regC, &i) == 0) return ERR_ASMNOTREG;

	//Insert OP into output
	dsb_nid_op(VM_DIV(regA,regB,regC),&output[*ip]);
	*ip = *ip + 1;

	return i;
}

static int asm_and(struct VMLabel *labels, const char *line, NID_t *output, int *ip)
{
	int i=0;
	int regA;
	int regB;
	int regC;

	//Get a register
	if (vm_assemble_reg(&line[i],&regA, &i) == 0) return ERR_ASMNOTREG;

	++i; //White Space.

	//Get a register
	if (vm_assemble_reg(&line[i],&regB, &i) == 0) return ERR_ASMNOTREG;

	++i; //White Space.

	//Get a register
	if (vm_assemble_reg(&line[i],&regC, &i) == 0) return ERR_ASMNOTREG;

	//Insert OP into output
	dsb_nid_op(VM_AND(regA,regB,regC),&output[*ip]);
	*ip = *ip + 1;

	return i;
}

static int asm_or(struct VMLabel *labels, const char *line, NID_t *output, int *ip)
{
	int i=0;
	int regA;
	int regB;
	int regC;

	//Get a register
	if (vm_assemble_reg(&line[i],&regA, &i) == 0) return ERR_ASMNOTREG;

	++i; //White Space.

	//Get a register
	if (vm_assemble_reg(&line[i],&regB, &i) == 0) return ERR_ASMNOTREG;

	++i; //White Space.

	//Get a register
	if (vm_assemble_reg(&line[i],&regC, &i) == 0) return ERR_ASMNOTREG;

	//Insert OP into output
	dsb_nid_op(VM_OR(regA,regB,regC),&output[*ip]);
	*ip = *ip + 1;

	return i;
}

static int asm_xor(struct VMLabel *labels, const char *line, NID_t *output, int *ip)
{
	int i=0;
	int regA;
	int regB;
	int regC;

	//Get a register
	if (vm_assemble_reg(&line[i],&regA, &i) == 0) return ERR_ASMNOTREG;

	++i; //White Space.

	//Get a register
	if (vm_assemble_reg(&line[i],&regB, &i) == 0) return ERR_ASMNOTREG;

	++i; //White Space.

	//Get a register
	if (vm_assemble_reg(&line[i],&regC, &i) == 0) return ERR_ASMNOTREG;

	//Insert OP into output
	dsb_nid_op(VM_XOR(regA,regB,regC),&output[*ip]);
	*ip = *ip + 1;

	return i;
}

static int asm_neg(struct VMLabel *labels, const char *line, NID_t *output, int *ip)
{
	int i=0;
	int regA;
	int regB;

	//Get a register
	if (vm_assemble_reg(&line[i],&regA, &i) == 0) return ERR_ASMNOTREG;

	++i; //White Space.

	//Get a register
	if (vm_assemble_reg(&line[i],&regB, &i) == 0) return ERR_ASMNOTREG;

	//Insert OP into output
	dsb_nid_op(VM_NEG(regA,regB),&output[*ip]);
	*ip = *ip + 1;

	return i;
}

static int asm_shiftl(struct VMLabel *labels, const char *line, NID_t *output, int *ip)
{
	int tmp;
	int regA;
	int regB;
	int i = 0;

	//Get a register
	if (vm_assemble_reg(&line[i],&regA, &i) == 0) return ERR_ASMNOTREG;

	++i; //White space

	//Get an offset value
	tmp = vm_assemble_off(labels,&line[i],&regB);
	if (tmp == 0) return DSB_ERROR(ERR_ASMNOTOFF,&line[i]);
	i += tmp;

	//Insert OP into output
	dsb_nid_op(VM_SHIFTL(regA,regB),&output[*ip]);
	*ip = *ip + 1;

	return i;
}

static int asm_shiftr(struct VMLabel *labels, const char *line, NID_t *output, int *ip)
{
	int tmp;
	int regA;
	int regB;
	int i = 0;

	//Get a register
	if (vm_assemble_reg(&line[i],&regA, &i) == 0) return ERR_ASMNOTREG;

	++i; //White space

	//Get an offset value
	tmp = vm_assemble_off(labels,&line[i],&regB);
	if (tmp == 0) return DSB_ERROR(ERR_ASMNOTOFF,&line[i]);
	i += tmp;

	//Insert OP into output
	dsb_nid_op(VM_SHIFTR(regA,regB),&output[*ip]);
	*ip = *ip + 1;

	return i;
}

static int asm_clear(struct VMLabel *labels, const char *line, NID_t *output, int *ip)
{
	return 0;
}

static int asm_int(struct VMLabel *labels, const char *line, NID_t *output, int *ip)
{
	return 0;
}

static int asm_float(struct VMLabel *labels, const char *line, NID_t *output, int *ip)
{
	return 0;
}

static struct AsmToken asmops[] = {
		ASMOP(load),
		ASMOP(store),
		ASMOP(jump),
		ASMOP(jeq),
		ASMOP(jneq),
		ASMOP(jlt),
		ASMOP(jgt),
		ASMOP(jleq),
		ASMOP(jgeq),
		ASMOP(read),
		ASMOP(write),
		ASMOP(dep),
		ASMOP(new),
		ASMOP(delete),
		ASMOP(copy),
		ASMOP(ret),
		ASMOP(data),
		ASMOP(inc),
		ASMOP(dec),
		ASMOP(add),
		ASMOP(sub),
		ASMOP(mul),
		ASMOP(div),
		ASMOP(and),
		ASMOP(or),
		ASMOP(xor),
		ASMOP(neg),
		ASMOP(shiftl),
		ASMOP(shiftr),
		ASMOP(clear),
		ASMOP(int),
		ASMOP(float),
		{"null",0}
};

/*
 * Assemble a single line, fill output and return number of elements
 * written to the output.
 */
int dsb_assemble_line(struct VMLabel *labels, const char *line, NID_t *output, int *ip)
{
	int i = 0;
	char opbuf[20];
	char *tmp;

	//Remove leading white space.
	while (line[i] == ' ' || line[i] == '\t') ++i;

	//Check for blank line or end of input.
	if (line[i] == 0 || line[i] == '\n') return 0;
	//Check for a comment line.
	if (line[i] == '#') return 0;

	//Do we have a label?
	if (line[i] == ':')
	{
		++i;
		//Skip label then white space
		while (line[i] != 0 && line[i] != ' ' && line[i] != '\t' && line[i] != '\n') ++i;
		while (line[i] == ' ' || line[i] == '\t') ++i;
	}

	//Parse operation into buffer (space ends op)
	tmp = strchr(&line[i],' ');
	if (tmp == 0) return 0;
	strncpy(opbuf,&line[i],tmp-(&line[i]));
	opbuf[tmp-(&line[i])] = 0;

	//Search for operation parser function
	i = 0;
	while (asmops[i].opfunc != 0)
	{
		if (strcmp(opbuf,asmops[i].name) == 0)
		{
			//Parse operands and generate opcode.
			asmops[i].opfunc(labels,tmp+1,output,ip);
			break;
		}
		i++;
	}

	return 1;
}

int dsb_assemble(const char *source, NID_t *output, int max)
{
	int ip = 0;
	struct VMLabel *labels = malloc(sizeof(struct VMLabel)*MAX_LABELS);

	dsb_assemble_labels(labels,source);

	//For every line
	while(1)
	{
		dsb_assemble_line(labels,source,output,&ip);
		//Move to next line if there is one.
		source = strchr(source,'\n');
		if (source == 0) break;
		source++;
	}

	free(labels);
	return ip;
}

int dsb_assemble_labels(struct VMLabel *labels, const char *source)
{
	int i;
	int ip = 0;

	for (i=0; i<MAX_LABELS; i++)
	{
		labels[i].lip = -1;
	}

	//For every line
	while(1)
	{
		i = 0;

		//Remove white space
		while (source[i] == ' ' || source[i] == '\t') i++;

		//Blank line
		if (source[i] == 0) break;

		//Is the line a label
		if (source[i] == ':')
		{
			//Add the label
			vm_assemble_addlabel(labels,&source[i+1],&ip);
			++i;
			//Skip label then white space
			while (source[i] != 0 && source[i] != ' ' && source[i] != '\t' && source[i] != '\n') ++i;
			while (source[i] == ' ' || source[i] == '\t') ++i;
		}

		//Is the line an instruction
		if (source[i] != '\n' && source[i] != '#' && source[i] != 0)
		{
			ip++;
		}

		//Move to next line if there is one.
		source = strchr(source,'\n');
		if (source == 0) break;
		source++;
	}

	return SUCCESS;
}

int dsb_disassemble(const NID_t *src, int size, char *output, int max)
{
	int i;
	char buf[100];

	for (i=0; i<size; i++)
	{
		if (src[i].header == 0 && src[i].t == NID_VMOP)
		{
			switch((unsigned int)VM_OP(src[i].ll))
			{
			case VMOP_RET:	sprintf(output,"ret %%%d\n",(unsigned int)VMREG_A(src[i].ll)); break;
			case VMOP_LOAD:	sprintf(output,"load %%%d %d\n",(unsigned int)VMREG_A(src[i].ll),(unsigned int)VMREG_D(src[i].ll)); break;
			default: break;
			}
		}
		else
		{
			dsb_nid_toStr(&src[i],buf,100);
			sprintf(output,"data %s\n",buf);
		}
		output += strlen(output);
	}
	return 0;
}

