/*
 * string.c
 *
 *  Created on: 28 May 2013
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

#include "dsb/string.h"
#include "dsb/nid.h"
#include "dsb/specials.h"
#include "dsb/errors.h"
#include "dsb/wrap.h"
#include <string.h>
#include <malloc.h>

int dsb_string_cton(const NID_t *dest, const char *str)
{
	NID_t attr;
	NID_t val;
	int i;
	int len = strlen(str);

	dsb_nid(NID_SPECIAL,SPECIAL_SIZE, &attr);
	dsb_iton(len,&val);
	dsb_set(dest, &attr,&val);

	for (i=0; i<len; i++)
	{
		dsb_iton(i,&attr);
		dsb_cton(str[i],&val);
		dsb_set(dest, &attr,&val);
	}

	return SUCCESS;
}

int dsb_string_ntoc(char *dest, int len, const NID_t *str)
{
	NID_t attr;
	NID_t val;
	NID_t *vals;
	int i;
	int len2;

	dsb_nid(NID_SPECIAL,SPECIAL_SIZE, &attr);
	dsb_get(str,&attr,&val);
	len2 = dsb_ntoi(&val);

	if (len2 == 0)
	{
		dest[0] = 0;
		return SUCCESS;
	}

	vals = malloc(sizeof(NID_t)*len2);

	//TODO make sure size exists.

	for (i=0; i<len2; i++)
	{
		dsb_iton(i,&attr);

		//If we are not the last then use async read
		if (i < len2-1)
		{
			dsb_getA(str,&attr,&(vals[i]));
		}
		else
		{
			dsb_get(str,&attr,&(vals[i]));
		}
	}

	//Now convert NIDS to characters.
	for (i=0; i<len2; i++)
	{
		if (i >= len)
		{
			i--;
			break;
		}
		dest[i] = vals[i].chr;
	}

	dest[i] = 0;
	free(vals);

	return SUCCESS;
}

