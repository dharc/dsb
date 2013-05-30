/*
 * basic.c
 *
 *  Created on: 9 May 2013
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

#include "dsb/harc.h"
#include "dsb/errors.h"
#include "dsb/nid.h"
#include "dsb/specials.h"
#include "dsb/wrap.h"

/*
 * The most basic DSB definition evaluator. Does not compile the def.
 */
int eval_basic(struct HARC *harc)
{
	struct NID tmp;
	struct NID res;
	int size;
	int i;

	//Get the size of the definition.
	dsb_nid(NID_SPECIAL,SPECIAL_SIZE,&tmp);
	dsb_get(&(harc->def),&tmp,&res);
	size = res.ll;

	if (size <= 0) return SUCCESS;

	//Get the first element.
	dsb_iton(0,&tmp);
	dsb_get(&(harc->def),&tmp,&res);

	//For every other definition element...
	for (i=1; i<size; i++)
	{
		//Get next component of definition.
		dsb_iton(i,&tmp);
		dsb_get(&(harc->def),&tmp,&tmp);

		if (tmp.t == NID_OPERATOR)
		{

		}
		else
		{
			//Add the dependency
			dsb_dependency(&res,&tmp,&(harc->t1),&(harc->t2));
			//Do the actual lookup, apply last element with current.
			dsb_get(&res,&tmp,&res);
		}
	}

	harc->h = res;

	return SUCCESS;
}

