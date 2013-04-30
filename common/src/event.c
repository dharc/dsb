#include "dsb/event.h"
#include "dsb/errors.h"
#include <malloc.h>

#define EVENT_POOL_SIZE	10000

struct Event *event_heap = 0;		//Single block for cache efficiency
struct Event **event_pool = 0;		//Free events in the heap
unsigned int event_lastalloc = 0;

int dsb_event_init()
{
	int i;

	if (event_heap == 0)
	{
		event_heap = malloc(EVENT_POOL_SIZE * sizeof(struct Event));
		event_pool = malloc(EVENT_POOL_SIZE * sizeof(struct Event*));

		//Initialise pool with heap pointers.
		for (i=0; i<EVENT_POOL_SIZE; i++)
		{
			event_pool[i] = &event_heap[i];
		}
		return SUCCESS;
	}
	//Has already been called so error.
	return ERR_REINIT;
}

int dsb_event_final()
{
	if (event_heap != 0)
	{
		free(event_heap);
		free(event_pool);
		return 0;
	}

	//Was not initialised
	return ERR_NOINIT;
}


struct Event *dsb_event_allocate()
{
	//NOTE: No safety checks
	//TODO Must be thread safe
	return event_pool[event_lastalloc++];
}

void dsb_event_free(struct Event *evt)
{
	//NOTE: No safety checks
	//TODO Must be thread safe
	event_pool[--event_lastalloc] = evt;
}


int dsb_event_params(const struct Event *evt)
{
	return SUCCESS;
}
