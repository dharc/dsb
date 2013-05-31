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

typedef struct HARC HARC_t;
typedef struct NID NID_t;

//Op codes
#define VMOP_READ		0x010000	///< Read a,b -> c. Sync GET event.
#define VMOP_WRITE		0x020000	///< Write c,d -> a,b. DEFINE event where c is evaluator.
#define VMOP_JUMP		0x030000	///< Jump a
#define VMOP_COPY		0x040000	///< Copy a into b.
#define VMOP_JEQ		0x050000	///< Jump a when b == c.
#define VMOP_JNEQ		0x060000	///< Jump a when b != c.
#define VMOP_DEP		0x070000	///< Add dependency on a,b. DEP event.
#define VMOP_CONST		0x080000	///< Put a constant NID into a.
#define VMOP_RET		0x090000	///< Return a value as the result.

//Generate op codes with register info
#define VMOP0(O)			(O)
#define VMOP1(O,A)			(VMOP0(O) | ((A) << 12))
#define VMOP2(O,A,B)		(VMOP1(O,A) | (B << 8))
#define VMOP3(O,A,B,C)		(VMOP2(O,A,B) | (C << 4))
#define VMOP4(O,A,B,C,D)	(VMOP3(O,A,B,C) | (D))

//High-level op code generation
#define VM_READ(A,B,C)		VMOP3(VMOP_READ,A,B,C)
#define VM_WRITE(A,B,C,D)	VMOP4(VMOP_WRITE,A,B,C,D)
#define VM_JUMP(A)			(VMOP_JUMP | ((unsigned char)(A) & 0xFF))
#define VM_COPY(A,B)		VMOP2(VMOP_COPY,A,B)
#define VM_JEQ(A,B,C)		(VMOP2(VMOP_JEQ,A,B) | ((unsigned char)(C) & 0xFF))
#define VM_JNEQ(A,B,C)		(VMOP2(VMOP_JNEQ,A,B) | ((unsigned char)(C) & 0xFF))
#define VM_DEP(A,B)			VMOP2(VMOP_DEP,A,B)
#define VM_CONST(A)			VMOP1(VMOP_CONST,A)
#define VM_RET(A)			VMOP1(VMOP_RET,A)

//Extract register values
#define VMREG_A(A)			(((A) >> 12) & 0xF)
#define VMREG_B(A)			(((A) >> 8) & 0xF)
#define VMREG_C(A)			(((A) >> 4) & 0xF)
#define VMREG_D(A)			((A) & 0xF)

//Extract op code without register options.
#define VM_OP(A)			((A) & 0xFF0000)

/**
 * Call a VM function stored at a particular node.
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
 * @param code Array of NIDs containing the code.
 * @param maxip Size of the code array.
 * @param params An array of parameters.
 * @param pn Number of parameters.
 * @param res Location to put the result.
 * @return SUCCESS.
 */
int dsb_vm_interpret(const NID_t *code, int maxip, const NID_t *params, int pn, NID_t *res);

int dsb_vm_interpret_reg(const NID_t *code, int maxip, NID_t *reg, const NID_t *params, int pn, NID_t *res);


#ifdef __cplusplus
}
#endif

#endif /* VM_H_ */
