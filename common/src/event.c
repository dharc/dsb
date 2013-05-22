#include "dsb/event.h"
#include "dsb/errors.h"
#include <malloc.h>

#if defined(UNIX) && !defined(NO_THREADS)
#include <pthread.h>
static pthread_mutex_t evt_pool_mtx = PTHREAD_MUTEX_INITIALIZER;
#endif //LINUX THREADED

#define EVENT_POOL_SIZE	10000

static struct Event *event_heap = 0;		//Single block for cache efficiency
static struct Event **event_pool = 0;		//Free events in the heap
static unsigned int event_lastalloc = 0;

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

struct Event *dsb_event(enum EventType type, const struct NID *d1, const struct NID *d2, struct Event *evt)
{
	evt->type = type;
	evt->flags = 0;
	evt->d1 = *d1;
	evt->d2 = *d2;
	return evt;
}


struct Event *dsb_event_allocate()
{
	struct Event *res;

	#if defined(UNIX) && !defined(NO_THREADS)
	pthread_mutex_lock(&evt_pool_mtx);
	#endif

	//NOTE: No safety checks
	res = event_pool[event_lastalloc++];

	#if defined(UNIX) && !defined(NO_THREADS)
	pthread_mutex_unlock(&evt_pool_mtx);
	#endif

	return res;
}

void dsb_event_free(struct Event *evt)
{
	#if defined(UNIX) && !defined(NO_THREADS)
	pthread_mutex_lock(&evt_pool_mtx);
	#endif

	//NOTE: No safety checks
	event_pool[--event_lastalloc] = evt;

	#if defined(UNIX) && !defined(NO_THREADS)
	pthread_mutex_unlock(&evt_pool_mtx);
	#endif
}


int dsb_event_params(const struct Event *evt)
{
	return SUCCESS;
}

