/*
 * evaluator.c
 *
 *  Created on: 8 May 2013
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
#include "dsb/harc.h"
#include "dsb/errors.h"

typedef int (*eval_t)(HARC_t *harc);

eval_t evaluators[MAX_EVALUATORS] = {0};

int dsb_eval_init()
{
	return SUCCESS;
}

int dsb_eval_final()
{
	return SUCCESS;
}

int dsb_eval_register(int id, int (*e)(HARC_t *harc))
{
	if (id <= 0 || id >= MAX_EVALUATORS) return DSB_ERROR(ERR_EVALID,0);
	if (evaluators[id] != 0) return DSB_ERROR(ERR_EVALEXISTS,0);
	evaluators[id] = e;
	return SUCCESS;
}

int dsb_eval_call(HARC_t *harc)
{
	if (harc == 0) return DSB_ERROR(ERR_EVALID,0);
	//Null evaluator just copies definition to head.
	if (harc->e == 0)
	{
		harc->h = harc->def;
		return SUCCESS;
	}

	//Sanity check the ID.
	if (harc->e < 0 || harc->e >= MAX_EVALUATORS) return DSB_ERROR(ERR_EVALID,0);
	if (evaluators[harc->e] == 0) return DSB_ERROR(ERR_NOEVAL,0);

	return evaluators[harc->e](harc);
}

