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

static int vm_assemble_off(const char *line, int *off)
{
	int i=0;
	int neg = 0;
	*off = 0;

	if (line[0] == '-') {
		i++;
		neg = 1;
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

/*
 * Assemble a single line, fill output and return number of elements
 * written to the output.
 */
int dsb_assemble_line(struct VMLabel *labels, const char *line, NID_t *output, int *ip)
{
	int tmp;
	int regA, regB, regC, regD;
	int i = 0;
	//Remove leading white space.
	while (line[i] == ' ' || line[i] == '\t') ++i;

	//Check for blank line or end of input.
	if (line[i] == 0 || line[i] == '\n') return 0;
	//Check for a comment line.
	if (line[i] == '#') return 0;

	//Now check which command is given.
	if (strncmp(&line[i],"load",4) == 0)
	{
		i+=5; //LOAD + space.

		//Get a register
		tmp = vm_assemble_reg(&line[i],&regA);
		if (tmp == 0) return DSB_ERROR(ERR_ASMNOTREG,&line[i]);
		i += tmp;

		++i; //White space

		//Get an offset value
		tmp = vm_assemble_off(&line[i],&regB);
		if (tmp == 0) return DSB_ERROR(ERR_ASMNOTOFF,&line[i]);
		i += tmp;

		//Insert OP into output
		dsb_nid_op(VM_LOAD(regA,regB),&output[*ip]);
		*ip = *ip + 1;
	}
	//-------------------------------------------------------------------------
	else if (strncmp(&line[i],"jump",4) == 0)
	{
		i+=5; //JUMP + space.

		//Get an offset value
		tmp = vm_assemble_off(&line[i],&regA);
		if (tmp == 0) return DSB_ERROR(ERR_ASMNOTOFF,&line[i]);
		i += tmp;

		//Must be signed char size.
		if (regA < -128 || regA > 127) return DSB_ERROR(ERR_ASMINVALOFF,&line[i]);

		//Insert OP into output
		dsb_nid_op(VM_JUMP(regA),&output[*ip]);
		*ip = *ip + 1;
	}
	//-------------------------------------------------------------------------
	else if (strncmp(&line[i],"copy",4) == 0)
	{
		i+=5; //COPY + space.

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
	}
	//-------------------------------------------------------------------------
	else if (strncmp(&line[i],"jeq",3) == 0)
	{
		i+=4; //JEQ + space.

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
		tmp = vm_assemble_off(&line[i],&regC);
		if (tmp == 0) return DSB_ERROR(ERR_ASMNOTOFF,&line[i]);
		i += tmp;

		//Must be signed char size.
		if (regC < -128 || regC > 127) return DSB_ERROR(ERR_ASMINVALOFF,&line[i]);

		//Insert OP into output
		dsb_nid_op(VM_JEQ(regA,regB,regC),&output[*ip]);
		*ip = *ip + 1;
	}
	//-------------------------------------------------------------------------
	else if (strncmp(&line[i],"jneq",4) == 0)
	{
		i+=5; //JNEQ + space.

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
		tmp = vm_assemble_off(&line[i],&regC);
		if (tmp == 0) return DSB_ERROR(ERR_ASMNOTOFF,&line[i]);
		i += tmp;

		//Must be signed char size.
		if (regC < -128 || regC > 127) return DSB_ERROR(ERR_ASMINVALOFF,&line[i]);

		//Insert OP into output
		dsb_nid_op(VM_JNEQ(regA,regB,regC),&output[*ip]);
		*ip = *ip + 1;
	}
	//-------------------------------------------------------------------------
	else if (strncmp(&line[i],"ret",3) == 0)
	{
		i+=4; //RET + space.

		//Get a register
		tmp = vm_assemble_reg(&line[i],&regA);
		if (tmp == 0) return DSB_ERROR(ERR_ASMNOTREG,&line[i]);
		i += tmp;

		//Insert OP into output
		dsb_nid_op(VM_RET(regA),&output[*ip]);
		*ip = *ip + 1;
	}
	//-------------------------------------------------------------------------
	else if (strncmp(&line[i],"inc",3) == 0)
	{
		i+=4; //INC + space.

		//Get a register
		tmp = vm_assemble_reg(&line[i],&regA);
		if (tmp == 0) return DSB_ERROR(ERR_ASMNOTREG,&line[i]);
		i += tmp;

		//Insert OP into output
		dsb_nid_op(VM_INC(regA),&output[*ip]);
		*ip = *ip + 1;
	}
	//-------------------------------------------------------------------------
	else if (strncmp(&line[i],"data",4) == 0)
	{
		i+=5; //DATA + space.

		//Insert OP into output
		dsb_nid_fromStr(&line[i],&output[*ip]);
		*ip = *ip + 1;
	}
	//-------------------------------------------------------------------------
	else if (strncmp(&line[i],"read",4) == 0)
	{
		i+=5; //READ + space.

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

		//Must be signed char size.
		if (regC < -128 || regC > 127) return DSB_ERROR(ERR_ASMINVALOFF,&line[i]);

		//Insert OP into output
		dsb_nid_op(VM_READ(regA,regB,regC),&output[*ip]);
		*ip = *ip + 1;
	}

	return 1;
}

int dsb_assemble(const char *source, NID_t *output, int max)
{
	int ip = 0;
	struct VMLabel *labels = malloc(sizeof(struct VMLabel)*MAX_LABELS);

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

