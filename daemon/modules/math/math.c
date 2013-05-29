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

//-----------------------------------------------------------------------------
//  Arithmetic handlers.
//-----------------------------------------------------------------------------

/*
 * Generate an intermediate INTADD NID from "number +"
 */
int math_arith_add1(struct Event *evt)
{
	signed long long num = evt->d2.ll;

	if (evt->type == EVENT_GET)
	{
		dsb_nid(NID_INTADD,num,evt->res);
		//TODO Make threadsafe
		evt->flags |= EVTFLAG_DONE;
	}

	return SUCCESS;
}

/*
 * Do addition with INTADD and INT nodes.
 */
int math_arith_add2(struct Event *evt)
{
	signed long long num1 = evt->d1.ll;
	signed long long num2 = evt->d2.ll;

	if (evt->type == EVENT_GET)
	{
		dsb_iton(num1+num2,evt->res);
		//TODO Make threadsafe
		evt->flags |= EVTFLAG_DONE;
	}

	return SUCCESS;
}

/*
 * Generate an intermediate INTSUB NID from "number -"
 */
int math_arith_sub1(struct Event *evt)
{
	signed long long num = evt->d2.ll;

	if (evt->type == EVENT_GET)
	{
		dsb_nid(NID_INTSUB,num,evt->res);
		//TODO Make threadsafe
		evt->flags |= EVTFLAG_DONE;
	}

	return SUCCESS;
}

/*
 * Do addition with INTSUB and INT nodes.
 */
int math_arith_sub2(struct Event *evt)
{
	signed long long num1 = evt->d1.ll;
	signed long long num2 = evt->d2.ll;

	if (evt->type == EVENT_GET)
	{
		dsb_iton(num2-num1,evt->res);
		//TODO Make threadsafe
		evt->flags |= EVTFLAG_DONE;
	}

	return SUCCESS;
}

//-----------------------------------------------------------------------------

/*
 * Initialise the math module by setting up the router to route events
 * for arithmetic operators on numbers to our handlers.
 */
int math_init(const struct NID *base)
{
	struct NID x1;
	struct NID x2;
	struct NID y1;
	struct NID y2;

	//Route for the first part of ADD operator.
	dsb_nid(NID_SPECIAL,SPECIAL_ADD,&x1);
	x2 = x1;
	dsb_iton(0,&y1);
	y2.header = 0;
	y2.t = NID_INTEGER;
	y2.ll = 0xFFFFFFFFFFFFFFFF;
	dsb_route_map(&x1,&x2,&y1,&y2,math_arith_add1);
	//Route for the second part of ADD operator.
	x1.header = 0;
	x1.t = NID_INTADD;
	x1.ll = 0;
	x2.header = 0;
	x2.t = NID_INTADD;
	x2.ll = 0xFFFFFFFFFFFFFFFF;
	dsb_route_map(&x1,&x2,&y1,&y2,math_arith_add2);

	//Route for the first part of SUB operator.
	//dsb_harc_C(NID_SPECIAL, SPECIAL_SUB, NID_INTEGER, 0, &low);
	//dsb_harc_C(NID_SPECIAL, SPECIAL_SUB, NID_INTEGER, 0xFFFFFFFFFFFFFFFF, &high);
	//dsb_route_map(&low,&high,math_arith_sub1);
	//Route for the second part of SUB operator.
	//dsb_harc_C(NID_INTSUB, 0, NID_INTEGER, 0, &low);
	//dsb_harc_C(NID_INTSUB, 0xFFFFFFFFFFFFFFFF, NID_INTEGER, 0xFFFFFFFFFFFFFFFF, &high);
	//dsb_route_map(&low,&high,math_arith_sub2);

	return SUCCESS;
}

int math_final()
{
	return SUCCESS;
}

/*
 * Module registration structure.
 */
struct Module *dsb_math_module()
{
	mathmod.init = math_init;
	mathmod.update = 0;
	mathmod.final = math_final;
	return &mathmod;
}

