/*
 * evaluator.h
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

/** @file evaluator.h */

#ifndef EVALUATOR_H_
#define EVALUATOR_H_

struct HARC;

/**
 * @addtogroup Evaluators
 * Evaluators are responsible for evaluating hyperarc definitions when they
 * are observed and out-of-date. Definitions can be in any form as long as
 * they return an NID that acts as the head of the hyperarc.
 *
 * @{
 */

#define MAX_EVALUATORS 1000		///< Maximum number of evaluators.

/**
 * Common evaluator IDs. For custom evaluators use a number between
 * EVAL_CUSTOM and EVAL_MAX, but be sure that it is unique.
 */
enum
{
	EVAL_CONSTANT=0,		//!< Constant definitions (same as =).
	EVAL_BASIC,				//!< Basic DASM style definition evaluator
	EVAL_LUA,				//!< LUA Evaluator
	EVAL_CUSTOM=100,		//!< EVAL_CUSTOM
	EVAL_MAX=MAX_EVALUATORS	//!< EVAL_MAX
};

/**
 * Initialise the evaluation system.
 * @return SUCCESS
 */
int dsb_eval_init();

/**
 * Finalise and cleanup the evaluation system.
 * @return SUCCESS
 */
int dsb_eval_final();

/**
 * Register a definition evaluator to a particular id. The id must be greater
 * than 0 and not already be in use.
 * @param id Unique ID for evaluator.
 * @param e The evaluator function.
 * @return SUCCESS or ERR_EVALID.
 */
int dsb_eval_register(int id, int (*e)(struct HARC *harc, void **data));

/**
 * Call a definition evaluator to evaluate a given definition.
 * @param id ID of evaluator as previously registered.
 * @param harc Hyperarc structure to evaluate.
 * @param data Custom data used by evaluator.
 * @return SUCCESS, ERR_EVALID or another error code.
 */
int dsb_eval_call(int id, struct HARC *harc, void **data);

/** @} */

#endif /* EVALUATOR_H_ */
