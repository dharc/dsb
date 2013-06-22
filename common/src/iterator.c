/*
 * iterator.c
 *
 *  Created on: 10 Jun 2013
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

#include "dsb/algorithms/iterator.h"
#include "dsb/core/nid.h"
#include "dsb/wrap.h"
#include "dsb/globals.h"
#include "dsb/patterns/array.h"

#include <malloc.h>

int dsb_iterator_begin(struct DSBIterator *it, const NID_t *n)
{
	NID_t keys;

	//Get the dictionary object
	dsb_get(n,&Keys,&keys);

	//Start by iterating over the dictionary...
	it->object = *n;
	it->count = dsb_array_readalloc(&keys,&it->buffer);
	it->current = 0;
	it->mode = 0;
	return 0;
}

//const NID_t *dsb_iterator_first(struct DSBIterator *it)
//{
//	it->current = 0;
//	if (it->count == it->current) return 0;
//	return &it->buffer[it->current++];
//}

const NID_t *dsb_iterator_next(struct DSBIterator *it)
{
	//End of items...
	if (it->count == it->current)
	{
		if (it->mode == 0)
		{
			//Switch to array iteration mode.
			it->mode = 1;
			if (it->buffer != 0) free(it->buffer);
			it->buffer = 0;
			it->count = 0;
			dsb_getnni(&it->object,&Size,&it->count);
			it->current = -1;

			//No array elements.
			if (it->count == 0) return 0;
		}
		else
		{
			return 0;
		}
	}

	if (it->mode == 0)
	{
		return &it->buffer[it->current++];
	}
	else
	{
		if (it->current == -1)
		{
			it->current = 0;
			return &Size;
		}
		else
		{
			dsb_iton(it->current++,&it->temp);
			return &it->temp;
		}
	}
}

int dsb_iterator_end(struct DSBIterator *it)
{
	if (it->buffer != 0) free(it->buffer);
	return 0;
}
