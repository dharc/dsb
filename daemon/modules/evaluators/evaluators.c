/*
 * evaluators.c
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

#include "dsb/evaluator.h"
#include "dsb/module.h"
#include "dsb/errors.h"
#include "dsb/globals.h"
#include "dsb/wrap.h"

struct Module evalmod;
struct HARC;

int eval_basic(HARC_t *harc);
int eval_vm(HARC_t *harc);

int eval_init()
{
	dsb_eval_register(EVAL_BASIC,eval_basic);
	dsb_eval_register(EVAL_DSBVM,eval_vm);
	return SUCCESS;
}

int eval_final()
{
	dsb_eval_register(EVAL_BASIC,0);
	dsb_eval_register(EVAL_DSBVM,0);
	return SUCCESS;
}

int eval_update()
{
	int x1,x2,y1,y2 = 0;
		int ret;
		ret =  dsb_getnii(&PRoot,0,&x1);
		ret += dsb_getnii(&PRoot,1,&x2);
		ret += dsb_getnii(&PRoot,2,&y1);
		ret += dsb_getnii(&PRoot,3,&y2);
		return 0;
}

/*
 * Module registration structure.
 */
struct Module *dsb_evaluators_module()
{
	evalmod.init = eval_init;
	evalmod.update = eval_update;
	evalmod.final = eval_final;
	return &evalmod;
}
