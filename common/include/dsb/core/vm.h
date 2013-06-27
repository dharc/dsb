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

#include "dsb/core/nid.h"

typedef struct HARC HARC_t;

//Op codes
//Hyperarc
#define VMOP_GET		0x00010000	///< Read a,b -> c. Sync GET event.
#define VMOP_DEF		0x00020000	///< Write c,d -> a,b. DEFINE event where c is evaluator.
#define VMOP_DEP		0x00030000	///< Add dependency on a,b. DEP event.
#define VMOP_NEW		0x00040000
#define VMOP_DEL		0x00050000
#define VMOP_GETD		0x00060000	///< Get and add dependency
//Jumps
#define VMOP_JMP		0x01010000	///< Jump a
#define VMOP_JEQ		0x01020000	///< Jump a when b == c.
#define VMOP_JNE		0x01030000	///< Jump a when b != c.
#define VMOP_JLE		0x01040000
#define VMOP_JGE		0x01050000
#define VMOP_JLT		0x01060000
#define VMOP_JGT		0x01070000
//Data manip
#define VMOP_CPY		0x02010000	///< Copy a into b.
#define VMOP_RET		0x02020000	///< Return a value as the result.
//Arithmetic
#define VMOP_INC		0x03010000	///< Increment a register containing an integer.
#define VMOP_DEC		0x03020000	///< Decrement a register containing an integer.
#define VMOP_ADD		0x03030000
#define VMOP_SUB		0x03040000
#define VMOP_DIV		0x03050000
#define VMOP_MUL		0x03060000
#define VMOP_INT		0x03070000
#define VMOP_FLT		0x03080000
//Bit
#define VMOP_AND		0x04010000
#define VMOP_OR			0x04020000
#define VMOP_XOR		0x04030000
#define VMOP_NEG		0x04040000
#define VMOP_SHL		0x04050000
#define VMOP_SHR		0x04060000
#define VMOP_CLR		0x04070000
//Highlevel
#define VMOP_PATH		0x05010000
#define VMOP_PATHD		0x05020000
#define VMOP_JISA		0x05030000
#define VMOP_JISNT		0x05040000

//Generate op codes with mem info
#define VMOP0(O)			((unsigned long long)(O) << 32)
#define VMOP1(O,A)			(VMOP0(O) | ((A) << 24))
#define VMOP2(O,A,B)		(VMOP1(O,A) | (B << 16))
#define VMOP3(O,A,B,C)		(VMOP2(O,A,B) | (C << 8))
#define VMOP4(O,A,B,C,D)	(VMOP3(O,A,B,C) | (D))

#define VMOPL0(O,L)			((unsigned long long)(O) << 32)
#define VMOPL1(O,L,A)		(VMOP0(O) | ((unsigned long long)(L) << 32) | ((A) << 24))
#define VMOPL2(O,L,A,B)		(VMOP1(O,A) | ((unsigned long long)(L) << 32) | (B << 16))
#define VMOPL3(O,L,A,B,C)	(VMOP2(O,A,B) | ((unsigned long long)(L) << 32) | (C << 8))
#define VMOPL4(O,L,A,B,C,D)	(VMOP3(O,A,B,C) | ((unsigned long long)(L) << 32) | (D))

//High-level op code generation macros
#define VM_GET(A,B,C)		VMOP3(VMOP_GET,A,B,C)
#define VM_DEF(A,B,C)		VMOP3(VMOP_DEF,A,B,C)
#define VM_DEP(A,B,C,D)		VMOP4(VMOP_DEP,A,B,C,D)
#define VM_NEW(A,B)			VMOP2(VMOP_NEW,A,B)
#define VM_DEL(A,B)			VMOP2(VMOP_DEL,A,B)

#define VM_JMP(L)			VMOPL0(VMOP_JMP,L)
#define VM_JEQ(L,A,B)		VMOPL2(VMOP_JEQ,L,A,B)
#define VM_JNE(L,A,B)		VMOPL2(VMOP_JNE,L,A,B)
#define VM_JLE(L,A,B)		VMOPL2(VMOP_JLE,L,A,B)
#define VM_JGE(L,A,B)		VMOPL2(VMOP_JGE,L,A,B)
#define VM_JLT(L,A,B)		VMOPL2(VMOP_JLT,L,A,B)
#define VM_JGT(L,A,B)		VMOPL2(VMOP_JGT,L,A,B)

#define VM_CPY(A,B)			VMOP2(VMOP_CPY,A,B)
#define VM_RET(A)			VMOP1(VMOP_RET,A)

#define VM_INC(A)			VMOP1(VMOP_INC,A)
#define VM_DEC(A)			VMOP1(VMOP_DEC,A)
#define VM_ADD(A,B,C)		VMOP3(VMOP_ADD,A,B,C)
#define VM_SUB(A,B,C)		VMOP3(VMOP_SUB,A,B,C)
#define VM_DIV(A,B,C)		VMOP3(VMOP_DIV,A,B,C)
#define VM_MUL(A,B,C)		VMOP3(VMOP_MUL,A,B,C)
#define VM_INT(A,B)			VMOP2(VMOP_INT,A,B)
#define VM_FLT(A,B)			VMOP2(VMOP_FLT,A,B)

#define VM_AND(A,B,C)		VMOP3(VMOP_AND,A,B,C)
#define VM_OR(A,B,C)		VMOP3(VMOP_OR,A,B,C)
#define VM_XOR(A,B,C)		VMOP3(VMOP_XOR,A,B,C)
#define VM_NEG(A,B)			VMOP2(VMOP_NEG,A,B)
#define VM_SHL(A,B,C)		VMOP3(VMOP_SHL,A,B,C)
#define VM_SHR(A,B,C)		VMOP3(VMOP_SHR,A,B,C)
#define VM_CLR(A)			VMOP1(VMOP_CLR,A)

//Extract variable numbers
#define VMGET_A(A)			(((A) >> 24) & 0xFF)
#define VMGET_B(A)			(((A) >> 16) & 0xFF)
#define VMGET_C(A)			(((A) >> 8) & 0xFF)
#define VMGET_D(A)			((A) & 0xFF)

//Extract op code without var numbers or address.
#define VMGET_OP(A)			(((A) & 0xFFFF000000000000) >> 32)

#define VMGET_LABEL(A)		(((A) >> 32) & 0xFFFF)

/**
 * Virtual Machine Context structure. Stores state information for a
 * particular execution block.
 * @see dsb_vm_interpret_ctx
 */
struct VMContext
{
	NID_t *code;		///< NID array containing the code
	int codesize;			///< Size of the code array.
	int ip;					///< Instruction pointer.
	int timeout;			///< Max instructions to execute before halt.
	NID_t *result;			///< Any return value from script.
	NID_t vars[16];			///< Registers.
};

int dsb_vm_context(struct VMContext *ctx, const NID_t *func);

/**
 * Call a VM function stored at a particular node. Loads the node as an array
 * and then interprets the array as DSB byte code. Can take a variable number
 * of parameters.
 * @param res Location to put result.
 * @param func VM Bytecode object.
 * @param pn Number of parameters.
 * @return SUCCESS.
 */
int dsb_vm_call(NID_t *res, const NID_t *func, int pn, ...);

int dsb_vm_interpret(struct VMContext *ctx);

typedef int (*XFUNC_t)(NID_t *Res, const NID_t *params);

int dsb_vm_xfunc(const char *name, XFUNC_t func, int params);

/**
 * JIT Compile bytecode into native machine code. The output pointer is of type
 * int (*)(NID_t *) and should be called as res = output(vars);
 * @param code Source bytecode
 * @param size Size of source bytecode
 * @param output Location to store pointer to compiled function.
 * @return Size of output.
 */
int dsb_vm_arch_compile(NID_t *code, int size, void **output);


#ifdef __cplusplus
}
#endif

#endif /* VM_H_ */
