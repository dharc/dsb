/*
 * math.c
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

#include "dsb/nid.h"
#include "dsb/errors.h"
#include "dsb/module.h"
#include "dsb/event.h"
#include "dsb/harc.h"
#include "dsb/specials.h"
#include "dsb/router.h"

struct Module mathmod;

/*
 * Generate an intermediate INTADD NID
 */
int math_arith_add1(struct Event *evt)
{
	signed long long num = evt->dest.c;

	if (evt->type == EVENT_GET)
	{
		evt->res.type = NID_INTADD;
		evt->res.ll = num;
		//TODO Make threadsafe
		evt->flags |= EVTFLAG_DONE;
	}

	return SUCCESS;
}

int math_arith_add2(struct Event *evt)
{
	signed long long num1 = evt->dest.b;
	signed long long num2 = evt->dest.c;

	if (evt->type == EVENT_GET)
	{
		evt->res.type = NID_INTEGER;
		evt->res.ll = num1 + num2;
		//TODO Make threadsafe
		evt->flags |= EVTFLAG_DONE;
	}

	return SUCCESS;
}

int math_init(const struct NID *base)
{
	struct HARC low;
	struct HARC high;

	//Route for the ADD operator.
	dsb_harc_C(NID_SPECIAL, SPECIAL_ADD, NID_INTEGER, 0, &low);
	dsb_harc_C(NID_SPECIAL, SPECIAL_ADD, NID_INTEGER, 0xFFFFFFFFFFFFFFFF, &high);
	dsb_route_map(&low,&high,math_arith_add1);
	dsb_harc_C(NID_INTADD, 0, NID_INTEGER, 0, &low);
	dsb_harc_C(NID_INTADD, 0xFFFFFFFFFFFFFFFF, NID_INTEGER, 0xFFFFFFFFFFFFFFFF, &high);
	dsb_route_map(&low,&high,math_arith_add2);

	return SUCCESS;
}

int math_final()
{
	return SUCCESS;
}

struct Module *dsb_math_module()
{
	mathmod.init = math_init;
	mathmod.update = 0;
	mathmod.final = math_final;
	return &mathmod;
}

