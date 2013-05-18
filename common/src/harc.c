#include "dsb/harc.h"
#include "dsb/nid.h"
#include "dsb/errors.h"
#include "dsb/event.h"
#include "dsb/evaluator.h"

int dsb_harc_event(HARC_t *harc, Event_t *event)
{
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

						return SUCCESS;
	//-------------------------------------------------------------------------
	case EVENT_DEP:		//Add to list of dependents.
	//-------------------------------------------------------------------------
	case EVENT_NOTIFY:	harc->flags |= HARC_OUTOFDATE;
						return SUCCESS;
	//-------------------------------------------------------------------------

	default: return DSB_ERROR(ERR_INVALIDEVENT,0);
	}
}
