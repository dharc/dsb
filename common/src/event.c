#include "dsb/core/event.h"
#include "dsb/errors.h"
#include <malloc.h>
#include <string.h>

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

int dsb_event_pack(const Event_t *e, char *buf, int max)
{
	char *oldbuf = buf;
	//Pack the type and destination NIDs.
	//*((int*)buf) = e->type;
	memcpy(buf,&e->type,sizeof(int));
	buf += sizeof(int);
	buf += dsb_nid_pack(&(e->d1),buf,max);
	buf += dsb_nid_pack(&(e->d2),buf,max);

	//Pack type specific details.
	switch (e->type)
	{
	//---------------------------------------------------------------------
	case EVENT_GET:
	case EVENT_GETDEF:
		*((int*)buf) = e->resid;
		memcpy(buf,&e->resid,sizeof(int));
		buf += sizeof(int);
		break;
	//---------------------------------------------------------------------
	case EVENT_ALLOCATE:
		//*((int*)buf) = e->resid;
		memcpy(buf,&e->resid,sizeof(int));
		buf += sizeof(int);
		break;
	//---------------------------------------------------------------------
	case EVENT_DEFINE:
		//*((int*)buf) = e->eval;
		buf += dsb_nid_pack(&(e->def),buf,max);
		break;
	//---------------------------------------------------------------------
	case EVENT_SET:
		//*((int*)buf) = e->eval;
		buf += dsb_nid_pack(&(e->def),buf,max);
		break;
	//---------------------------------------------------------------------
	case EVENT_DEP:
		buf += dsb_nid_pack(&(e->dep1),buf,max);
		buf += dsb_nid_pack(&(e->dep2),buf,max);
		break;
	//---------------------------------------------------------------------
	case EVENT_NOTIFY:
		break;
	//---------------------------------------------------------------------
	default: break;
	}

	return (int)(buf-oldbuf);
}

int dsb_event_unpack(const char *buf, Event_t *e)
{
	const char *oldbuf = buf;
	//Unpack the type and destination NIDs.
	//e->type = *((int*)buf);
	memcpy(&e->type,buf,sizeof(int));
	buf += sizeof(int);
	buf += dsb_nid_unpack(buf,&(e->d1));
	buf += dsb_nid_unpack(buf,&(e->d2));

	//Unpack type specific details.
	switch (e->type)
	{
	//---------------------------------------------------------------------
	case EVENT_GET:
	case EVENT_GETDEF:
		//e->resid = *((int*)buf);
		memcpy(&e->resid,buf,sizeof(int));
		buf += sizeof(int);
		break;
	//---------------------------------------------------------------------
	case EVENT_ALLOCATE:
		//e->resid = *((int*)buf);
		memcpy(&e->resid,buf,sizeof(int));
		buf += sizeof(int);
		break;
	//---------------------------------------------------------------------
	case EVENT_DEFINE:
		//e->eval = *((int*)buf);
		buf += dsb_nid_unpack(buf,&(e->def));
		break;
	//---------------------------------------------------------------------
	case EVENT_SET:
		//e->eval = *((int*)buf);
		buf += dsb_nid_unpack(buf,&(e->def));
		break;
	//---------------------------------------------------------------------
	case EVENT_DEP:
		buf += dsb_nid_unpack(buf,&(e->dep1));
		buf += dsb_nid_unpack(buf,&(e->dep2));
		break;
	//---------------------------------------------------------------------
	case EVENT_NOTIFY:
		break;
	//---------------------------------------------------------------------
	default: break;
	}

	return (int)(buf-oldbuf);
}

Event_t *dsb_event(EVENTTYPE_t type, const NID_t *d1, const NID_t *d2, Event_t *evt)
{
	evt->type = type;
	evt->flags = 0;
	evt->d1 = *d1;
	evt->d2 = *d2;
	return evt;
}


Event_t *dsb_event_allocate()
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

int dsb_event_pretty(const Event_t *evt, char *buf, int len)
{
	char d1[100];
	char d2[100];
	char def[100];
	char dep1[100];
	char dep2[100];

	dsb_nid_toStr(&evt->d1,d1,100);
	dsb_nid_toStr(&evt->d2,d2,100);

	switch(evt->type)
	{
	case EVENT_GET:			//dsb_nid_toStr(evt->res,dep1,100);
							sprintf(buf,"GET - %s,%s",d1,d2);
							break;
	case EVENT_GETDEF:			//dsb_nid_toStr(evt->res,dep1,100);
							sprintf(buf,"GETDEF - %s,%s",d1,d2);
							break;
	case EVENT_DEFINE:		dsb_nid_toStr(&evt->def,def,100);
							sprintf(buf,"DEFINE - %s,%s = %s",d1,d2,def);
							break;
	case EVENT_SET:			dsb_nid_toStr(&evt->def,def,100);
							sprintf(buf,"SET - %s,%s = %s",d1,d2,def);
							break;
	case EVENT_DEP:			dsb_nid_toStr(&evt->dep1,dep1,100);
							dsb_nid_toStr(&evt->dep2,dep2,100);
							sprintf(buf,"DEP - %s,%s -> %s,%s",d1,d2,dep1,dep2);
							break;
	case EVENT_NOTIFY:		sprintf(buf,"NOTIFY - %s,%s",d1,d2);
							break;
	case EVENT_ALLOCATE:	sprintf(buf,"NEW - %s,%s",d1,d2);
							break;
	default: break;
	}
	return 0;
}

