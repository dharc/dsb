/*
 * specials.h
 *
 *  Created on: 7 May 2013
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

/** @file specials.h */

#ifndef SPECIALS_H_
#define SPECIALS_H_

enum
{
	SPECIAL_NULL=0,
	SPECIAL_TRUE,
	SPECIAL_FALSE,
	SPECIAL_NAMES,
	SPECIAL_PROOT,
	//---- Integer Ops ----
	SPECIAL_ADD,
	SPECIAL_SUB,
	SPECIAL_DIV,
	SPECIAL_MUL,
	SPECIAL_SHIFTL,
	SPECIAL_SHIFTR,
	SPECIAL_BITAND,
	SPECIAL_BITOR,
	SPECIAL_BITNOT,
	//---- Definitions and Strings
	SPECIAL_SIZE,
	SPECIAL_KEYS,

	SPECIAL_END
};

enum
{
	OP_BEGINSUB,
	OP_ENDSUB,
	OP_SET
};



#endif /* SPECIALS_H_ */
