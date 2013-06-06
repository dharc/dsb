/*
 * array.c
 *
 *  Created on: 29 May 2013
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

#include "dsb/array.h"
#include "dsb/nid.h"
#include "dsb/specials.h"
#include "dsb/wrap.h"
#include "dsb/globals.h"
#include <malloc.h>

int dsb_array_write(const NID_t *src, int size, const NID_t *dest)
{
	NID_t attr;
	NID_t val;
	int i;

	//Set the size attribute
	dsb_nid(NID_SPECIAL,SPECIAL_SIZE, &attr);
	dsb_iton(size,&val);
	dsb_set(dest, &attr,&val);

	//Set each element.
	for (i=0; i<size; i++)
	{
		dsb_iton(i,&attr);
		dsb_set(dest, &attr,&(src[i]));
	}

	return size;
}

int dsb_array_read(const NID_t *src, NID_t *dest, int max)
{
	NID_t attr;
	NID_t val;
	int i;
	int len2;

	dsb_nid(NID_SPECIAL,SPECIAL_SIZE, &attr);
	dsb_get(src,&attr,&val);
	len2 = dsb_ntoi(&val);

	//TODO make sure size exists.

	for (i=0; i<len2; i++)
	{
		dsb_iton(i,&attr);

		//If we are not the last then use async read
		if (i < len2-1)
		{
			dsb_getA(src,&attr,&(dest[i]));
		}
		else
		{
			dsb_get(src,&attr,&(dest[i]));
		}
	}

	return len2;
}

int dsb_array_readalloc(const NID_t *src, NID_t **dest)
{
	NID_t attr;
	NID_t val;
	int i;
	int len;

	dsb_get(src,&Size,&val);
	len = dsb_ntoi(&val);

	if (len <= 0) return 0;

	*dest = malloc(sizeof(NID_t)*len);

	for (i=0; i<len; i++)
	{
		dsb_iton(i,&attr);

		//If we are not the last then use async read
		if (i < len-1)
		{
			dsb_getA(src,&attr,&((*dest)[i]));
		}
		else
		{
			dsb_get(src,&attr,&((*dest)[i]));
		}
	}

	return len;
}

