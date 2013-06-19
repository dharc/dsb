/*
 * pattern.c
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

#include "dsb/pattern.h"
#include "dsb/pattern_types.h"
#include "dsb/nid.h"
#include "dsb/specials.h"
#include "dsb/wrap.h"
#include "dsb/names.h"
#include "dsb/globals.h"

#include <string.h>

int dsb_pattern_isA(const NID_t *n, int t)
{
	NID_t tmp1;
	NID_t tmp2;

	switch (t)
	{
	case DSB_PATTERN_NUMBER:	return (dsb_pattern_isA(n,DSB_PATTERN_INTEGER) || dsb_pattern_isA(n,DSB_PATTERN_REAL));
	case DSB_PATTERN_INTEGER:	return (n->header == 0 && n->t == NID_INTEGER);
	case DSB_PATTERN_REAL:		return (n->header == 0 && n->t == NID_REAL);
	case DSB_PATTERN_CHARACTER:	return (n->header == 0 && n->t == NID_CHARACTER);
	case DSB_PATTERN_OPERATOR:	return (n->header == 0 && n->t == NID_VMOP);
	case DSB_PATTERN_BOOLEAN:	return (n->header == 0 && n->t == NID_SPECIAL && (n->ll == SPECIAL_TRUE || n->ll == SPECIAL_FALSE));
	case DSB_PATTERN_NULL:		return (dsb_nid_eq(n,&Null));
	case DSB_PATTERN_OBJECT:	return (n->header != 0);

	case DSB_PATTERN_ARRAY:		dsb_get(n,&Size,&tmp1);
								dsb_getnin(n,0,&tmp2);
								return (dsb_pattern_isA(&tmp1,DSB_PATTERN_INTEGER) && !dsb_nid_eq(&tmp2,&Null));

	case DSB_PATTERN_STRING:	dsb_get(n,&Size,&tmp1);
								dsb_getnin(n,0,&tmp2);
								return (dsb_pattern_isA(&tmp1,DSB_PATTERN_INTEGER) && dsb_pattern_isA(&tmp2,DSB_PATTERN_CHARACTER));

	case DSB_PATTERN_BYTECODE:	dsb_get(n,&Size,&tmp1);
								dsb_getnin(n,0,&tmp2);
								return (dsb_pattern_isA(&tmp1,DSB_PATTERN_INTEGER) && dsb_pattern_isA(&tmp2,DSB_PATTERN_OPERATOR));

	default: return 0;
	}
}

unsigned int dsb_pattern_what(const NID_t *n)
{
	NID_t type;

	if (dsb_pattern_isA(n,DSB_PATTERN_INTEGER)) return DSB_PATTERN_INTEGER;
	else if (dsb_pattern_isA(n,DSB_PATTERN_REAL)) return DSB_PATTERN_REAL;
	else if (dsb_pattern_isA(n,DSB_PATTERN_CHARACTER)) return DSB_PATTERN_CHARACTER;
	else if (dsb_pattern_isA(n,DSB_PATTERN_BOOLEAN)) return DSB_PATTERN_BOOLEAN;
	else if (dsb_pattern_isA(n,DSB_PATTERN_NULL)) return DSB_PATTERN_NULL;

	dsb_getnzn(n,"type",&type);
	if (dsb_nid_eq(&type,&Null) == 1)
	{
		if (dsb_pattern_isA(n,DSB_PATTERN_STRING)) return DSB_PATTERN_STRING;
		else if (dsb_pattern_isA(n,DSB_PATTERN_BYTECODE)) return DSB_PATTERN_BYTECODE;
		else if (dsb_pattern_isA(n,DSB_PATTERN_ARRAY)) return DSB_PATTERN_ARRAY;

	}
	else
	{
		char buf[100];
		dsb_names_revlookup(&type,buf,100);

		if (strcmp(buf,"display") == 0) return DSB_PATTERN_DISPLAY;
		else if (strcmp(buf,"line") == 0) return DSB_PATTERN_SHAPE_LINE;
		else if (strcmp(buf,"text") == 0) return DSB_PATTERN_SHAPE_TEXT;
	}

	return DSB_PATTERN_OBJECT;
}

int dsb_pattern_typeIsA(int type, int supertype)
{
	return 0;
}
