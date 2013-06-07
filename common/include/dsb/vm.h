/*
 * vm.h
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

#ifndef VM_H_
#define VM_H_

/** @file vm.h */

#ifdef __cplusplus
extern "C"
{
#endif

#include "dsb/nid.h"

typedef struct HARC HARC_t;

//Op codes
//Hyperarc
#define VMOP_READ		0x00010000	///< Read a,b -> c. Sync GET event.
#define VMOP_WRITE		0x00020000	///< Write c,d -> a,b. DEFINE event where c is evaluator.
#define VMOP_DEP		0x00030000	///< Add dependency on a,b. DEP event.
#define VMOP_NEW		0x00040000
#define VMOP_DELETE		0x00050000
//Jumps
#define VMOP_JUMP		0x00100000	///< Jump a
#define VMOP_JEQ		0x00110000	///< Jump a when b == c.
#define VMOP_JNEQ		0x00120000	///< Jump a when b != c.
#define VMOP_JLEQ		0x00130000
#define VMOP_JGEQ		0x00140000
#define VMOP_JLT		0x00150000
#define VMOP_JGT		0x00160000
//Data manip
#define VMOP_COPY		0x00200000	///< Copy a into b.
#define VMOP_LOAD		0x00210000	///< Put a constant NID into a.
#define VMOP_RET		0x00220000	///< Return a value as the result.
#define VMOP_STORE		0x00230000
#define VMOP_LOADR		0x00240000
//Arithmetic
#define VMOP_INC		0x00300000	///< Increment a register containing an integer.
#define VMOP_DEC		0x00310000	///< Decrement a register containing an integer.
#define VMOP_ADD		0x00320000
#define VMOP_SUB		0x00330000
#define VMOP_DIV		0x00340000
#define VMOP_MUL		0x00350000
#define VMOP_INT		0x00360000
#define VMOP_FLOAT		0x00370000
//Bit
#define VMOP_AND		0x00400000
#define VMOP_OR			0x00410000
#define VMOP_XOR		0x00420000
#define VMOP_NEG		0x00430000
#define VMOP_SHIFTL		0x00440000
#define VMOP_SHIFTR		0x00450000
#define VMOP_CLEAR		0x00460000

//Generate op codes with register info
#define VMOP0(O)			(O)
#define VMOP1(O,A)			(VMOP0(O) | ((A) << 12))
#define VMOP2(O,A,B)		(VMOP1(O,A) | (B << 8))
#define VMOP3(O,A,B,C)		(VMOP2(O,A,B) | (C << 4))
#define VMOP4(O,A,B,C,D)	(VMOP3(O,A,B,C) | (D))

//High-level op code generation
#define VM_READ(A,B,C)		VMOP3(VMOP_READ,A,B,C)
#define VM_WRITE(A,B,C,D)	VMOP4(VMOP_WRITE,A,B,C,D)
#define VM_DEP(A,B,C,D)		VMOP4(VMOP_DEP,A,B,C,D)

#define VM_JUMP(A)			(VMOP_JUMP | ((unsigned char)(A) & 0xFF))
#define VM_JEQ(A,B,C)		(VMOP2(VMOP_JEQ,A,B) | ((unsigned char)(C) & 0xFF))
#define VM_JNEQ(A,B,C)		(VMOP2(VMOP_JNEQ,A,B) | ((unsigned char)(C) & 0xFF))
#define VM_JLEQ(A,B,C)		(VMOP2(VMOP_JLEQ,A,B) | ((unsigned char)(C) & 0xFF))
#define VM_JGEQ(A,B,C)		(VMOP2(VMOP_JGEQ,A,B) | ((unsigned char)(C) & 0xFF))
#define VM_JLT(A,B,C)		(VMOP2(VMOP_JLT,A,B) | ((unsigned char)(C) & 0xFF))
#define VM_JGT(A,B,C)		(VMOP2(VMOP_JGT,A,B) | ((unsigned char)(C) & 0xFF))

#define VM_COPY(A,B)		VMOP2(VMOP_COPY,A,B)
#define VM_LOAD(A,B)		(VMOP1(VMOP_LOAD,A) | ((unsigned char)(B) & 0xFF))
#define VM_LOADR(A,B,C)		(VMOP2(VMOP_LOADR,A,B) | ((unsigned char)(C) & 0xFF))
#define VM_RET(A)			VMOP1(VMOP_RET,A)
#define VM_STORE(A,B)		(VMOP1(VMOP_STORE,A) | ((unsigned char)(B) & 0xFF))

#define VM_INC(A)			VMOP1(VMOP_INC,A)
#define VM_DEC(A)			VMOP1(VMOP_DEC,A)
#define VM_ADD(A,B,C)		VMOP3(VMOP_ADD,A,B,C)
#define VM_SUB(A,B,C)		VMOP3(VMOP_SUB,A,B,C)
#define VM_DIV(A,B,C)		VMOP3(VMOP_DIV,A,B,C)
#define VM_MUL(A,B,C)		VMOP3(VMOP_MUL,A,B,C)

#define VM_AND(A,B,C)		VMOP3(VMOP_AND,A,B,C)
#define VM_OR(A,B,C)		VMOP3(VMOP_OR,A,B,C)
#define VM_XOR(A,B,C)		VMOP3(VMOP_XOR,A,B,C)
#define VM_NEG(A,B)			VMOP2(VMOP_NEG,A,B)
#define VM_SHIFTL(A,B)		(VMOP1(VMOP_SHIFTL,A) | ((unsigned char)(B) & 0xFF))
#define VM_SHIFTR(A,B)		(VMOP1(VMOP_SHIFTR,A) | ((unsigned char)(B) & 0xFF))

//Extract register values
#define VMREG_A(A)			(((A) >> 12) & 0xF)
#define VMREG_B(A)			(((A) >> 8) & 0xF)
#define VMREG_C(A)			(((A) >> 4) & 0xF)
#define VMREG_D(A)			((A) & 0xF)

//Extract op code without register options.
#define VM_OP(A)			((A) & 0xFF0000)

/**
 * Virtual Machine Context structure. Stores state information for a
 * particular execution block.
 * @see dsb_vm_interpret_ctx
 */
struct VMContext
{
	NID_t *code;		///< NID array containing the code
	int codesize;			///< Size of the code array.
	const NID_t *params[4];	///< Parameters passed to the code.
	int ip;					///< Instruction pointer.
	int timeout;			///< Max instructions to execute before halt.
	NID_t *result;			///< Any return value from script.
	NID_t reg[16];			///< Registers.
};

/**
 * Call a VM function stored at a particular node. Loads the node as an array
 * and then interprets the array as DSB byte code.
 * @param func An array structure in the graph at this node.
 * @param params An array of parameters
 * @param pn Number of parameters
 * @param res Location to put the result.
 * @return SUCCESS.
 */
int dsb_vm_call(const NID_t *func, const NID_t *params, int pn, NID_t *res);

/**
 * Interpret a NID array as VM code. Instead of getting the code from the graph
 * it directly uses an array of the code. This is used by dsb_vm_call.
 * @see dsb_vm_call
 * @param code Array of NIDs containing the code.
 * @param maxip Size of the code array.
 * @param params An array of parameters.
 * @param pn Number of parameters.
 * @param res Location to put the result.
 * @return SUCCESS.
 */
int dsb_vm_interpret(NID_t *code, int maxip, const NID_t *params, int pn, NID_t *res);

int dsb_vm_interpret_ctx(struct VMContext *ctx);


#ifdef __cplusplus
}
#endif

#endif /* VM_H_ */
