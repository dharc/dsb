/*
 * iter-test.c
 *
 *  Created on: 13 Jun 2013
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

#include "dsb/test.h"
#include "dsb/algorithms/iterator.h"
#include "dsb/wrap.h"
#include "dsb/core/event.h"
#include "dsb/core/specials.h"

int dsb_send(struct Event *evt)
{
	if (evt->type == EVENT_GET)
	{
		NID_t res;

		//The object with a dictionary.
		if (evt->d1.ll == 0)
		{
			if (evt->d2.ll == SPECIAL_KEYS)
			{
				dsb_iton(44,&res);
				evt->flags |= EFLAG_DONE;
			}
		}
		//The dictionary itself.
		else if (evt->d1.ll == 44)
		{
			if (evt->d2.t == NID_TYPE_SPECIAL)
			{
				if (evt->d2.ll == SPECIAL_SIZE)
				{
					dsb_iton(3, &res);
					evt->flags |= EFLAG_DONE;
				}
			}
			else
			{
				switch(evt->d2.ll)
				{
				case 0:		dsb_iton(55,&res); break;
				case 1:		dsb_iton(66,&res); break;
				case 2:		dsb_iton(77,&res); break;
				default:	dsb_nid_null(&res); break;
				}
				evt->flags |= EFLAG_DONE;
			}
		}

		if (evt->cb)
		{
			evt->cb(evt,&res);
		}
	}
	return 0;
}

void test_iterator_iterator()
{
	struct DSBIterator it;
	NID_t obj;
	const NID_t *p;
	int i = 0;

	dsb_iton(0,&obj);

	CHECK(dsb_iterator_begin(&it,&obj) == 0);
	p = dsb_iterator_next(&it);
	CHECK(it.count == 3);
	CHECK(p != 0);
	while (p != 0)
	{
		if (i == 0) { CHECK(p->ll == 55); }
		else if (i == 1) { CHECK(p->ll == 66); }
		else if (i == 2) { CHECK(p->ll == 77); }
		i++;
		p = dsb_iterator_next(&it);
	}
	CHECK(i == 3);
	CHECK(dsb_iterator_end(&it) == 0);
	DONE;
}

int main(int argc, char *argv[])
{
	dsb_nid_init();
	dsb_event_init();
	dsb_test(test_iterator_iterator);

	dsb_event_final();
	dsb_nid_final();
	return 0;
}
