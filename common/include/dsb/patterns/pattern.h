/*
 * pattern.h
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

#ifndef PATTERN_H_
#define PATTERN_H_

#include "dsb/patterns/pattern_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct NID NID_t;

/** @file pattern.h */

/**
 * Query whether a NID corresponds to a particular pattern.
 * @param n NID to test
 * @param t Type of pattern to check for.
 * @return 1 for yes, 0 for no.
 */
int dsb_pattern_isA(const NID_t *n, int t);

/**
 * Get the most specific pattern that a NID matches.
 * @param n NID to check
 * @return Pattern number
 */
unsigned int dsb_pattern_what(const NID_t *n);

/**
 * Check if one pattern is a more general form of another.
 * @param type Specific type
 * @param supertype More general type
 * @return 1 for yes, 0 for no.
 */
int dsb_pattern_typeIsA(int type, int supertype);

#ifdef __cplusplus
}
#endif

#endif /* PATTERN_H_ */
