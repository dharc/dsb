#include "dsb/harc.h"
#include "dsb/nid.h"
#include "dsb/errors.h"
#include "dsb/event.h"
#include "dsb/evaluator.h"
#include "dsb/dependency.h"
#include "dsb/wrap.h"
#include <malloc.h>

HARC_t *dsb_harc(const NID_t *t1, const NID_t *t2, HARC_t *harc)
{
	harc->t1 = *t1;
	harc->t2 = *t2;
	harc->deps = 0;
	harc->e = 0;
	harc->def.type = 0;
	harc->def.ll = 0;
	harc->flags = HARC_OUTOFDATE;
	return harc;
}

int dsb_harc_event(HARC_t *harc, Event_t *event)
{
	Dependency_t *dep;
	Dependency_t *temp;

	if (harc == 0)
	{
			event->res.type = 0;
			event->res.ll = 0;
		return SUCCESS;
	}

	switch(event->type)
	{
	//-------------------------------------------------------------------------
	case EVENT_GET:		//Out of date so evaluate definition
						if ((harc->flags & HARC_OUTOFDATE) != 0)
						{
							//Get evaluator and use to get result.
							dsb_eval_call(harc);

							//No longer out-of-date
							harc->flags &= ~HARC_OUTOFDATE;
						}

						event->res = harc->h;
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
