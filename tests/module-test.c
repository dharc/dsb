/*
 * module-test.c
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

#include "dsb/core/module.h"
#include "dsb/test.h"
#include "dsb/errors.h"

struct Event;

int mod_hasloaded = 0;
int mod_hasunloaded = 0;

int dsb_send(struct Event *evt)
{
	return 0;
}

int mod_test_init(const struct NID *b)
{
	mod_hasloaded = 1;
	return 0;
}

int mod_test_final()
{
	mod_hasunloaded = 1;
	return 0;
}

void test_module_register()
{
	Module_t mod;
	mod.init = 0;
	mod.final = 0;
	mod.update = 0;

	CHECK(dsb_module_register("testmod", &mod) == ERR_INVALIDMOD);

	mod.init = mod_test_init;
	CHECK(dsb_module_register("testmod", &mod) == ERR_INVALIDMOD);

	mod.final = mod_test_final;
	CHECK(dsb_module_register("testmod", &mod) == SUCCESS);

	DONE;
}

void test_module_load()
{
	Module_t mod;
	mod.init = mod_test_init;
	mod.final = mod_test_final;
	mod.update = 0;

	CHECK(dsb_module_register("testmod2", &mod) == SUCCESS);
	CHECK(dsb_module_load("testmod2",0) == SUCCESS);
	CHECK(mod_hasloaded == 1);
	CHECK(dsb_module_unload("testmod2") == SUCCESS);
	CHECK(mod_hasunloaded == 1);
	DONE;
}

int main(int argc, char *argv[])
{
	dsb_test(test_module_register);
	dsb_test(test_module_load);
	return 0;
}

