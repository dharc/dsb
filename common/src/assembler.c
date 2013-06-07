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
static int vm_assemble_reg(const char *line, int *reg)
{
	if (line[0] == '%')
	{
		//Read a single HEX digit for reg id.
		if (line[1] >= '0' && line[1] <= '9') *reg = line[1]-'0';
		else if (line[1] >= 'a' && line[1] <= 'f') *reg = line[1]-'a';
		else return 0;
		return 2;
	}
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
	tmp = vm_assemble_reg(&line[i],&regA);
	if (tmp == 0) return DSB_ERROR(ERR_ASMNOTREG,&line[i]);
	i += tmp;

	++i; //White space

	//Multi-register load
	if (line[i] == '%')
	{
		//Get a register
		tmp = vm_assemble_reg(&line[i],&regB);
		if (tmp == 0) return DSB_ERROR(ERR_ASMNOTREG,&line[i]);
		i += tmp;

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
	tmp = vm_assemble_reg(&line[i],&regA);
	if (tmp == 0) return DSB_ERROR(ERR_ASMNOTREG,&line[i]);
	i += tmp;

	++i; //White Space.

	//Get a register
	tmp = vm_assemble_reg(&line[i],&regB);
	if (tmp == 0) return DSB_ERROR(ERR_ASMNOTREG,&line[i]);
	i += tmp;

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
	tmp = vm_assemble_reg(&line[i],&regA);
	if (tmp == 0) return DSB_ERROR(ERR_ASMNOTREG,&line[i]);
	i += tmp;

	++i; //White Space.

	//Get a register
	tmp = vm_assemble_reg(&line[i],&regB);
	if (tmp == 0) return DSB_ERROR(ERR_ASMNOTREG,&line[i]);
	i += tmp;

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
	int tmp;
	int regA;
	int regB;
	int regC;

	//Get a register
	tmp = vm_assemble_reg(&line[i],&regA);
	if (tmp == 0) return DSB_ERROR(ERR_ASMNOTREG,&line[i]);
	i += tmp;

	++i; //White Space.

	//Get a register
	tmp = vm_assemble_reg(&line[i],&regB);
	if (tmp == 0) return DSB_ERROR(ERR_ASMNOTREG,&line[i]);
	i += tmp;

	++i; //White Space.

	//Get a register
	tmp = vm_assemble_reg(&line[i],&regC);
	if (tmp == 0) return DSB_ERROR(ERR_ASMNOTREG,&line[i]);
	i += tmp;

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
	tmp = vm_assemble_reg(&line[i],&regA);
	if (tmp == 0) return DSB_ERROR(ERR_ASMNOTREG,&line[i]);
	i += tmp;

	++i; //White Space.

	//Get a register
	tmp = vm_assemble_reg(&line[i],&regB);
	if (tmp == 0) return DSB_ERROR(ERR_ASMNOTREG,&line[i]);
	i += tmp;

	++i; //White Space.

	//Get a register
	tmp = vm_assemble_reg(&line[i],&regC);
	if (tmp == 0) return DSB_ERROR(ERR_ASMNOTREG,&line[i]);
	i += tmp;

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
	int tmp;
	int regA;
	int regB;
	int regC;
	int regD;

	//Get a register
	tmp = vm_assemble_reg(&line[i],&regA);
	if (tmp == 0) return DSB_ERROR(ERR_ASMNOTREG,&line[i]);
	i += tmp;

	++i; //White Space.

	//Get a register
	tmp = vm_assemble_reg(&line[i],&regB);
	if (tmp == 0) return DSB_ERROR(ERR_ASMNOTREG,&line[i]);
	i += tmp;

	++i; //White Space.

	//Get a register
	tmp = vm_assemble_reg(&line[i],&regC);
	if (tmp == 0) return DSB_ERROR(ERR_ASMNOTREG,&line[i]);
	i += tmp;

	++i;

	//Get a register
	tmp = vm_assemble_reg(&line[i],&regD);
	if (tmp == 0) return DSB_ERROR(ERR_ASMNOTREG,&line[i]);
	i += tmp;

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
	int tmp;
	int regA, regB;

	//Get a register
	tmp = vm_assemble_reg(&line[i],&regA);
	if (tmp == 0) return DSB_ERROR(ERR_ASMNOTREG,&line[i]);
	i += tmp;

	++i; //White space.

	//Get a register
	tmp = vm_assemble_reg(&line[i],&regB);
	if (tmp == 0) return DSB_ERROR(ERR_ASMNOTREG,&line[i]);
	i += tmp;

	//Insert OP into output
	dsb_nid_op(VM_COPY(regA,regB),&output[*ip]);
	*ip = *ip + 1;

	return i;
}

static int asm_ret(struct VMLabel *labels, const char *line, NID_t *output, int *ip)
{
	int i=0;
	int tmp;
	int regA;

	//Get a register
	tmp = vm_assemble_reg(&line[i],&regA);
	if (tmp == 0) return DSB_ERROR(ERR_ASMNOTREG,&line[i]);
	i += tmp;

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
	int tmp;
	int regA;

	//Get a register
	tmp = vm_assemble_reg(&line[i],&regA);
	if (tmp == 0) return DSB_ERROR(ERR_ASMNOTREG,&line[i]);
	i += tmp;

	//Insert OP into output
	dsb_nid_op(VM_INC(regA),&output[*ip]);
	*ip = *ip + 1;

	return i;
}

static int asm_dec(struct VMLabel *labels, const char *line, NID_t *output, int *ip)
{
	int i=0;
	int tmp;
	int regA;

	//Get a register
	tmp = vm_assemble_reg(&line[i],&regA);
	if (tmp == 0) return DSB_ERROR(ERR_ASMNOTREG,&line[i]);
	i += tmp;

	//Insert OP into output
	dsb_nid_op(VM_DEC(regA),&output[*ip]);
	*ip = *ip + 1;

	return i;
}

static int asm_add(struct VMLabel *labels, const char *line, NID_t *output, int *ip)
{
	int i=0;
	int tmp;
	int regA;
	int regB;
	int regC;

	//Get a register
	tmp = vm_assemble_reg(&line[i],&regA);
	if (tmp == 0) return DSB_ERROR(ERR_ASMNOTREG,&line[i]);
	i += tmp;

	++i; //White Space.

	//Get a register
	tmp = vm_assemble_reg(&line[i],&regB);
	if (tmp == 0) return DSB_ERROR(ERR_ASMNOTREG,&line[i]);
	i += tmp;

	++i; //White Space.

	//Get a register
	tmp = vm_assemble_reg(&line[i],&regC);
	if (tmp == 0) return DSB_ERROR(ERR_ASMNOTREG,&line[i]);
	i += tmp;

	//Insert OP into output
	dsb_nid_op(VM_ADD(regA,regB,regC),&output[*ip]);
	*ip = *ip + 1;

	return i;
}

static int asm_sub(struct VMLabel *labels, const char *line, NID_t *output, int *ip)
{
	int i=0;
	int tmp;
	int regA;
	int regB;
	int regC;

	//Get a register
	tmp = vm_assemble_reg(&line[i],&regA);
	if (tmp == 0) return DSB_ERROR(ERR_ASMNOTREG,&line[i]);
	i += tmp;

	++i; //White Space.

	//Get a register
	tmp = vm_assemble_reg(&line[i],&regB);
	if (tmp == 0) return DSB_ERROR(ERR_ASMNOTREG,&line[i]);
	i += tmp;

	++i; //White Space.

	//Get a register
	tmp = vm_assemble_reg(&line[i],&regC);
	if (tmp == 0) return DSB_ERROR(ERR_ASMNOTREG,&line[i]);
	i += tmp;

	//Insert OP into output
	dsb_nid_op(VM_SUB(regA,regB,regC),&output[*ip]);
	*ip = *ip + 1;

	return i;
}

static int asm_mul(struct VMLabel *labels, const char *line, NID_t *output, int *ip)
{
	int i=0;
	int tmp;
	int regA;
	int regB;
	int regC;

	//Get a register
	tmp = vm_assemble_reg(&line[i],&regA);
	if (tmp == 0) return DSB_ERROR(ERR_ASMNOTREG,&line[i]);
	i += tmp;

	++i; //White Space.

	//Get a register
	tmp = vm_assemble_reg(&line[i],&regB);
	if (tmp == 0) return DSB_ERROR(ERR_ASMNOTREG,&line[i]);
	i += tmp;

	++i; //White Space.

	//Get a register
	tmp = vm_assemble_reg(&line[i],&regC);
	if (tmp == 0) return DSB_ERROR(ERR_ASMNOTREG,&line[i]);
	i += tmp;

	//Insert OP into output
	dsb_nid_op(VM_MUL(regA,regB,regC),&output[*ip]);
	*ip = *ip + 1;

	return i;
}

static int asm_div(struct VMLabel *labels, const char *line, NID_t *output, int *ip)
{
	int i=0;
	int tmp;
	int regA;
	int regB;
	int regC;

	//Get a register
	tmp = vm_assemble_reg(&line[i],&regA);
	if (tmp == 0) return DSB_ERROR(ERR_ASMNOTREG,&line[i]);
	i += tmp;

	++i; //White Space.

	//Get a register
	tmp = vm_assemble_reg(&line[i],&regB);
	if (tmp == 0) return DSB_ERROR(ERR_ASMNOTREG,&line[i]);
	i += tmp;

	++i; //White Space.

	//Get a register
	tmp = vm_assemble_reg(&line[i],&regC);
	if (tmp == 0) return DSB_ERROR(ERR_ASMNOTREG,&line[i]);
	i += tmp;

	//Insert OP into output
	dsb_nid_op(VM_DIV(regA,regB,regC),&output[*ip]);
	*ip = *ip + 1;

	return i;
}

static int asm_and(struct VMLabel *labels, const char *line, NID_t *output, int *ip)
{
	int i=0;
	int tmp;
	int regA;
	int regB;
	int regC;

	//Get a register
	tmp = vm_assemble_reg(&line[i],&regA);
	if (tmp == 0) return DSB_ERROR(ERR_ASMNOTREG,&line[i]);
	i += tmp;

	++i; //White Space.

	//Get a register
	tmp = vm_assemble_reg(&line[i],&regB);
	if (tmp == 0) return DSB_ERROR(ERR_ASMNOTREG,&line[i]);
	i += tmp;

	++i; //White Space.

	//Get a register
	tmp = vm_assemble_reg(&line[i],&regC);
	if (tmp == 0) return DSB_ERROR(ERR_ASMNOTREG,&line[i]);
	i += tmp;

	//Insert OP into output
	dsb_nid_op(VM_AND(regA,regB,regC),&output[*ip]);
	*ip = *ip + 1;

	return i;
}

static int asm_or(struct VMLabel *labels, const char *line, NID_t *output, int *ip)
{
	int i=0;
	int tmp;
	int regA;
	int regB;
	int regC;

	//Get a register
	tmp = vm_assemble_reg(&line[i],&regA);
	if (tmp == 0) return DSB_ERROR(ERR_ASMNOTREG,&line[i]);
	i += tmp;

	++i; //White Space.

	//Get a register
	tmp = vm_assemble_reg(&line[i],&regB);
	if (tmp == 0) return DSB_ERROR(ERR_ASMNOTREG,&line[i]);
	i += tmp;

	++i; //White Space.

	//Get a register
	tmp = vm_assemble_reg(&line[i],&regC);
	if (tmp == 0) return DSB_ERROR(ERR_ASMNOTREG,&line[i]);
	i += tmp;

	//Insert OP into output
	dsb_nid_op(VM_OR(regA,regB,regC),&output[*ip]);
	*ip = *ip + 1;

	return i;
}

static int asm_xor(struct VMLabel *labels, const char *line, NID_t *output, int *ip)
{
	int i=0;
	int tmp;
	int regA;
	int regB;
	int regC;

	//Get a register
	tmp = vm_assemble_reg(&line[i],&regA);
	if (tmp == 0) return DSB_ERROR(ERR_ASMNOTREG,&line[i]);
	i += tmp;

	++i; //White Space.

	//Get a register
	tmp = vm_assemble_reg(&line[i],&regB);
	if (tmp == 0) return DSB_ERROR(ERR_ASMNOTREG,&line[i]);
	i += tmp;

	++i; //White Space.

	//Get a register
	tmp = vm_assemble_reg(&line[i],&regC);
	if (tmp == 0) return DSB_ERROR(ERR_ASMNOTREG,&line[i]);
	i += tmp;

	//Insert OP into output
	dsb_nid_op(VM_XOR(regA,regB,regC),&output[*ip]);
	*ip = *ip + 1;

	return i;
}

static int asm_neg(struct VMLabel *labels, const char *line, NID_t *output, int *ip)
{
	int i=0;
	int tmp;
	int regA;
	int regB;

	//Get a register
	tmp = vm_assemble_reg(&line[i],&regA);
	if (tmp == 0) return DSB_ERROR(ERR_ASMNOTREG,&line[i]);
	i += tmp;

	++i; //White Space.

	//Get a register
	tmp = vm_assemble_reg(&line[i],&regB);
	if (tmp == 0) return DSB_ERROR(ERR_ASMNOTREG,&line[i]);
	i += tmp;

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
	tmp = vm_assemble_reg(&line[i],&regA);
	if (tmp == 0) return DSB_ERROR(ERR_ASMNOTREG,&line[i]);
	i += tmp;

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
	tmp = vm_assemble_reg(&line[i],&regA);
	if (tmp == 0) return DSB_ERROR(ERR_ASMNOTREG,&line[i]);
	i += tmp;

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

/*
 * Assemble a single line, fill output and return number of elements
 * written to the output.
 */
int dsb_assemble_line(struct VMLabel *labels, const char *line, NID_t *output, int *ip)
{
	int i = 0;
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

	//Now check which command is given.
	if (strncmp(&line[i],"load",4) == 0)
	{
		i += 5;
		i += asm_load(labels, &line[i], output,ip);
	}
	//-------------------------------------------------------------------------
	else if (strncmp(&line[i],"copy",4) == 0)
	{
		i+=5; //COPY + space.
		i += asm_copy(labels,&line[i],output,ip);
	}
	//-------------------------------------------------------------------------
	else if (strncmp(&line[i],"jump",4) == 0)
	{
		i +=5; //JUMP + space.
		i += asm_jump(labels,&line[i],output,ip);
	}
	//-------------------------------------------------------------------------
	else if (strncmp(&line[i],"jeq",3) == 0)
	{
		i+=4; //JEQ + space.
		i += asm_jeq(labels,&line[i],output,ip);
	}
	//-------------------------------------------------------------------------
	else if (strncmp(&line[i],"jneq",4) == 0)
	{
		i+=5; //JNEQ + space.
		i += asm_jneq(labels,&line[i],output,ip);
	}
	//-------------------------------------------------------------------------
	else if (strncmp(&line[i],"ret",3) == 0)
	{
		i+=4; //RET + space.
		i += asm_ret(labels,&line[i],output,ip);
	}
	//-------------------------------------------------------------------------
	else if (strncmp(&line[i],"inc",3) == 0)
	{
		i+=4; //INC + space.
		i += asm_inc(labels,&line[i],output,ip);
	}
	//-------------------------------------------------------------------------
	else if (strncmp(&line[i],"dec",3) == 0)
	{
		i+=4; //DEC + space.
		i += asm_dec(labels,&line[i],output,ip);
	}
	//-------------------------------------------------------------------------
	else if (strncmp(&line[i],"data",4) == 0)
	{
		i+=5; //DATA + space.
		i += asm_data(labels,&line[i],output,ip);
	}
	//-------------------------------------------------------------------------
	else if (strncmp(&line[i],"read",4) == 0)
	{
		i+=5; //READ + space.
		i += asm_read(labels,&line[i],output,ip);
	}
	//-------------------------------------------------------------------------
	else if (strncmp(&line[i],"write",5) == 0)
	{
		i+=6; //WRITE + space.
		i += asm_write(labels,&line[i],output,ip);
	}
	//-------------------------------------------------------------------------
	else if (strncmp(&line[i],"dep",3) == 0)
	{
		i+=4; //DEP + space.
		i += asm_dep(labels,&line[i],output,ip);
	}
	//-------------------------------------------------------------------------
	else if (strncmp(&line[i],"new",3) == 0)
	{
		i+=4; //NEW + space.
		i += asm_new(labels,&line[i],output,ip);
	}
	//-------------------------------------------------------------------------
	else if (strncmp(&line[i],"delete",6) == 0)
	{
		i+=7; //DELETE + space.
		i += asm_delete(labels,&line[i],output,ip);
	}
	//-------------------------------------------------------------------------
	else if (strncmp(&line[i],"add",3) == 0)
	{
		i+=4; //ADD + space.
		i += asm_add(labels,&line[i],output,ip);
	}
	//-------------------------------------------------------------------------
	else if (strncmp(&line[i],"sub",3) == 0)
	{
		i+=4; //SUB + space.
		i += asm_sub(labels,&line[i],output,ip);
	}
	//-------------------------------------------------------------------------
	else if (strncmp(&line[i],"div",3) == 0)
	{
		i+=4; //DIV + space.
		i += asm_div(labels,&line[i],output,ip);
	}
	//-------------------------------------------------------------------------
	else if (strncmp(&line[i],"mul",3) == 0)
	{
		i+=4; //MUL + space.
		i += asm_mul(labels,&line[i],output,ip);
	}
	//-------------------------------------------------------------------------
	else if (strncmp(&line[i],"and",3) == 0)
	{
		i+=4; //AND + space.
		i += asm_and(labels,&line[i],output,ip);
	}
	//-------------------------------------------------------------------------
	else if (strncmp(&line[i],"or",2) == 0)
	{
		i+=3; //OR + space.
		i += asm_or(labels,&line[i],output,ip);
	}
	//-------------------------------------------------------------------------
	else if (strncmp(&line[i],"xor",3) == 0)
	{
		i+=4; //XOR + space.
		i += asm_xor(labels,&line[i],output,ip);
	}
	//-------------------------------------------------------------------------
	else if (strncmp(&line[i],"neg",3) == 0)
	{
		i+=4; //NEG + space.
		i += asm_neg(labels,&line[i],output,ip);
	}
	//-------------------------------------------------------------------------
	else if (strncmp(&line[i],"shiftl",6) == 0)
	{
		i+=7; //SHIFTL + space.
		i += asm_shiftl(labels,&line[i],output,ip);
	}
	//-------------------------------------------------------------------------
	else if (strncmp(&line[i],"shiftr",6) == 0)
	{
		i+=7; //SHIFTR + space.
		i += asm_shiftr(labels,&line[i],output,ip);
	}
	//-------------------------------------------------------------------------
	else if (strncmp(&line[i],"clear",5) == 0)
	{
		i+=6; //CLEAR + space.
		i += asm_clear(labels,&line[i],output,ip);
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

