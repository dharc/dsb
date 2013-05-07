/*
 * harc-test.c
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

#include "dsb/test.h"
#include "dsb/harc.h"
#include "dsb/nid.h"

void test_harc_gen()
{
	struct HARC res;
	struct NID n1;
	struct NID n2;

	n1.type = NID_INTEGER;
	n1.ll = 55;
	n2.type = NID_INTEGER;
	n2.ll = 66;
	CHECK(dsb_harc_gen(&n1,&n2,&res) == 0);
	CHECK(res.a1 == NID_INTEGER);
	CHECK(res.a2 == NID_INTEGER);
	CHECK(res.b == 55);
	CHECK(res.c == 66);

	n1.ll = 77;
	CHECK(dsb_harc_gen(&n1,&n2,&res) == 0);
	CHECK(res.b == 66);
	CHECK(res.c == 77);

	n1.ll = 66;
	CHECK(dsb_harc_gen(&n1,&n2,&res) == 0);
	CHECK(res.b == 66);
	CHECK(res.c == 66);
	DONE;
}

void test_harc_compare()
{
	struct HARC h1;
	struct HARC h2;

	h1.a = 0;
	h1.b = 0;
	h1.c = 0;
	h2.a = 0;
	h2.b = 0;
	h2.c = 0;
	CHECK(dsb_harc_compare(&h1,&h2) == 0);

	h2.a = 0;
	h2.b = 0;
	h2.c = 1;
	CHECK(dsb_harc_compare(&h1,&h2) < 0);

	h2.a = 0;
	h2.b = 1;
	h2.c = 0;
	CHECK(dsb_harc_compare(&h1,&h2) < 0);

	h2.a = 1;
	h2.b = 0;
	h2.c = 0;
	CHECK(dsb_harc_compare(&h1,&h2) < 0);

	h1.a = 2;
	h1.b = 0;
	h1.c = 0;
	CHECK(dsb_harc_compare(&h1,&h2) > 0);

	h1.a = 1;
	h1.b = 1;
	h1.c = 0;
	CHECK(dsb_harc_compare(&h1,&h2) > 0);

	h1.a = 1;
	h1.b = 0;
	h1.c = 1;
	CHECK(dsb_harc_compare(&h1,&h2) > 0);
	DONE;
}

int main(int argc, char *argv[])
{
	dsb_test(test_harc_gen);
	dsb_test(test_harc_compare);
	return 0;
}

