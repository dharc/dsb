/*
 * pattern_types.h
 *
 *  Created on: 17 Jun 2013
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

#ifndef PATTERN_TYPES_H_
#define PATTERN_TYPES_H_


/* Graph pattern type IDs. */


//NID Types
#define		DSB_PATTERN_NULL			0x00000000
#define		DSB_PATTERN_BOOLEAN			(DSB_PATTERN_NULL+1)
#define		DSB_PATTERN_OPERATOR		(DSB_PATTERN_NULL+2)
#define		DSB_PATTERN_NUMBER 			0x00000100
#define		DSB_PATTERN_INTEGER			(DSB_PATTERN_NUMBER+1)
#define		DSB_PATTERN_REAL			(DSB_PATTERN_NUMBER+2)
#define		DSB_PATTERN_CHARACTER		(DSB_PATTERN_NUMBER+3)
#define		DSB_PATTERN_OBJECT			0x00000200

//Array relatives.
#define		DSB_PATTERN_ARRAY			0x00010000
#define		DSB_PATTERN_LABEL			(DSB_PATTERN_ARRAY+1)
#define		DSB_PATTERN_STRING			(DSB_PATTERN_ARRAY+2)
#define		DSB_PATTERN_BYTECODE		(DSB_PATTERN_ARRAY+3)

//Display Patterns.
#define		DSB_PATTERN_DISPLAY			0x00020000
#define		DSB_PATTERN_SHAPE_LINE		(DSB_PATTERN_DISPLAY+1)
#define		DSB_PATTERN_SHAPE_RECTANGLE	(DSB_PATTERN_DISPLAY+2)
#define		DSB_PATTERN_SHAPE_IMAGE		(DSB_PATTERN_DISPLAY+3)
#define		DSB_PATTERN_SHAPE_TEXT		(DSB_PATTERN_DISPLAY+4)


#endif /* PATTERN_TYPES_H_ */
