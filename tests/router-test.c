/*
 * router-test.c
 *
 *  Created on: 29 Apr 2013
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
#include "dsb/router.h"
#include "dsb/event.h"
#include "dsb/nid.h"

int math_handler1_called = 0;

int math_handler1(struct Event *evt)
{
	CHECK(evt->dest.b == 55);
	math_handler1_called = 1;
	return 0;
}

void test_router_simple()
{
	struct Event evt;
	struct HARC low;
	struct HARC high;

	evt.type = EVENT_GET;
	evt.dest.a1 = NID_INTEGER;
	evt.dest.b = 55;
	evt.dest.a2 = 0;
	evt.dest.c = 0;

	low.a1 = NID_INTEGER;
	low.a2 = 0;
	low.b = 0;
	low.c = 0;
	high.a1 = NID_INTEGER;
	high.a2 = 0;
	high.b = 0x7FFFFFFFFFFFFFFF;
	high.c = 0;

	CHECK(dsb_route_init() == 0);

	CHECK(dsb_route_map(&low,&high,math_handler1) == 0);
	CHECK(dsb_route(&evt) == 0);
	CHECK(math_handler1_called == 1);

	evt.dest.a1 = NID_REAL;
	CHECK(dsb_route(&evt) != 0);

	CHECK(dsb_route_final() == 0);

	DONE;
}

int main(int argc, char *argv[])
{
	dsb_test(test_router_simple);
	return 0;
}

