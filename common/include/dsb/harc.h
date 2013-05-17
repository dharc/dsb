/* 
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

/** @file harc.h */

#ifndef _HARC_H_
#define _HARC_H_

#include "dsb/nid.h"

struct Event;
typedef struct Event Event_t;
struct Dependency;
typedef struct Dependency Dependency_t;

/**
 * @addtogroup Hyperarc
 * @{
 */

#define HARC_OUTOFDATE	1	///< This hyperarc needs a re-evaluation of its def
#define HARC_VOLATILE	2	///< A volatile HARC, destroyed asap
#define HARC_LOCK		4	///< Thread lock.
#define HARC_EXTERNAL	8	///< Observed externally so always evaluate.

/**
 * Hyperarc structure. A hyperarc consists of two tail nodes and one head
 * node. There may be a definition to describe how the head node is to be
 * calculated. The evaluator selects how this definition is to be
 * interpreted and processed to generate the head node.
 */
struct HARC
{
	NID_t t1;		///< Tail 1. Should always be greater than or equal to tail 2.
	NID_t t2;		///< Tail 2. Should always be less than or equal to tail 1.
	NID_t h;		///< Head. Typically a cached value that results from evaluating the definition.
	NID_t def;		///< Definition. Identifies the structure to use as the definition.
	int e;			///< Evaluator. Which definition evaluator should be used for this HARC.

	//The following are volatile and do not need to be saved.
	int flags;		///< Status flags (HARC_ definitions).
	void *data;		///< Internal (optional) state used by evaluator.
	Dependency_t *deps;		///< List of dependencies.

	#ifndef STRIP_HARC_META
	NID_t meta;		///< Meta data for this hyperarc.
	#endif
};

typedef struct HARC HARC_t;

/**
 * Processes an event on a hyperarc.
 * @param harc The hyperarc corresponding to the event destination.
 * @param event The event to be processed on the hyperarc.
 * @return SUCCESS.
 */
int dsb_harc_event(HARC_t *harc, Event_t *event);

int dsb_harc_eval(HARC_t *harc);

/** @} */

#endif
