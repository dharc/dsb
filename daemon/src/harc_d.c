/*
 * harc_d.c
 *
 *  Created on: 28 May 2013
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
#include "dsb/nid.h"
#include "dsb/errors.h"
#include "dsb/event.h"
#include "dsb/evaluator.h"
#include "dsb/dependency.h"
#include "dsb/wrap.h"
#include <malloc.h>

int dsb_harc_event(HARC_t *harc, Event_t *event)
{
	Dependency_t *dep;
	Dependency_t *temp;
	int ret;

	if (harc == 0)
	{
			if (event->type == EVENT_GET)
			{
				dsb_nid_null(event->res);
			}
		return SUCCESS;
	}

	switch(event->type)
	{
	//-------------------------------------------------------------------------
	case EVENT_GET:		//Out of date so evaluate definition
						if ((harc->flags & HARC_OUTOFDATE) != 0)
						{
							//Get evaluator and use to get result.
							ret = dsb_eval_call(harc);
							if (ret != SUCCESS)
							{
								return ret;
							}

							//No longer out-of-date
							harc->flags &= ~HARC_OUTOFDATE;
						}

						*(event->res) = harc->h;
						event->flags |= EVTFLAG_DONE;
						return SUCCESS;
	//-------------------------------------------------------------------------
	case EVENT_GETDEF:
						*(event->res) = harc->def;
						//event->eval = harc->e;
						event->flags |= EVTFLAG_DONE;
						return SUCCESS;
	//-------------------------------------------------------------------------
	case EVENT_DEFINE:	harc->e = event->eval;
						harc->def = event->def;
						harc->flags |= HARC_OUTOFDATE;

						//Generate NOTIFY events to mark others as out-of-date.
						dep = harc->deps;
						harc->deps = 0;
						while (dep != 0)
						{
							dsb_notify(&(dep->a),&(dep->b));
							temp = dep->next;
							free(dep);
							dep = temp;
						}
						event->flags |= EVTFLAG_DONE;
						return SUCCESS;
	//-------------------------------------------------------------------------
	case EVENT_DEP:		//Add to list of dependents.
						dep = malloc(sizeof(Dependency_t));
						dep->next = harc->deps;
						dep->a = event->dep1;
						dep->b = event->dep2;
						harc->deps = dep;
						event->flags |= EVTFLAG_DONE;
						return SUCCESS;
	//-------------------------------------------------------------------------
	case EVENT_NOTIFY:	harc->flags |= HARC_OUTOFDATE;

						//Generate NOTIFY events to mark others as out-of-date.
						dep = harc->deps;
						harc->deps = 0;
						while (dep != 0)
						{
							dsb_notify(&(dep->a),&(dep->b));
							temp = dep->next;
							free(dep);
							dep = temp;
						}
						event->flags |= EVTFLAG_DONE;
						return SUCCESS;
	//-------------------------------------------------------------------------

	default: return DSB_ERROR(ERR_INVALIDEVENT,0);
	}
}

