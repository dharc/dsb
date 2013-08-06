/*
 * processor.c
 *
 *  Created on: 30 Apr 2013
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

#include "dsb/processor.h"
#include "dsb/core/event.h"
#include "dsb/errors.h"
#include "dsb/router.h"
#include "dsb/core/module.h"
#include "dsb/net_protocol.h"
#include "dsb/config.h"
#include "dsb/core/thread.h"
#include <malloc.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>

#define QUEUE_SIZE		10000
#define NUM_THREADS		4

//TODO Need to properly support rix and wix.

/*
 * A threadsafe event queue. There should be 3 of these.
 */
struct EventQueue
{
	Event_t **q;
	unsigned int rix;	//< Current read index
	unsigned int wix;	//< Current write index
};

MUTEX(qmtx);
RWLOCK(qlock);
COND(qcond);

typedef enum
{
	QUEUE_AGENT=0,
	QUEUE_ACTIVE,
	QUEUE_END
} QUEUE_t;

static struct EventQueue queue[QUEUE_END];
static int procisrunning;
static void *debugsock;

//Forward declarations of static functions
static void *dsb_proc_runthread(void *arg);

/*
 * Thread-safe event queue insertion.
 */
static int queue_insert(QUEUE_t q, Event_t *e)
{
	LOCK(qmtx);

	queue[q].q[queue[q].wix] = e;
	queue[q].wix = (queue[q].wix + 1) % QUEUE_SIZE;

	UNLOCK(qmtx);

	return SUCCESS;
}

/*
 * Thread-safe event queue removal.
 */
static Event_t *queue_pop(QUEUE_t q)
{
	Event_t *res = 0;

	LOCK(qmtx);

	//Are there any events left to read.
	res = queue[q].q[queue[q].rix];

	if (res != 0)
	{
		queue[q].q[queue[q].rix] = 0;
		queue[q].rix = (queue[q].rix + 1) % QUEUE_SIZE;
	}

	UNLOCK(qmtx);

	return res;
}

int dsb_proc_init()
{
	queue[QUEUE_AGENT].q = malloc(sizeof(Event_t*) * QUEUE_SIZE);
	memset(queue[QUEUE_AGENT].q,0,sizeof(Event_t*)*QUEUE_SIZE);
	queue[QUEUE_AGENT].rix = 0;
	queue[QUEUE_AGENT].wix = 0;

	queue[QUEUE_ACTIVE].q = malloc(sizeof(Event_t*) * QUEUE_SIZE);
	memset(queue[QUEUE_ACTIVE].q,0,sizeof(Event_t*)*QUEUE_SIZE);
	queue[QUEUE_ACTIVE].rix = 0;
	queue[QUEUE_ACTIVE].wix = 0;
	return SUCCESS;
}

int dsb_proc_final()
{
	free(queue[QUEUE_AGENT].q);
	free(queue[QUEUE_ACTIVE].q);
	return SUCCESS;
}

int dsb_proc_debug(void *sock)
{
	debugsock = sock;
	return SUCCESS;
}

int dsb_proc_stop()
{
	procisrunning = 0;
	BROADCAST(qcond);
	return SUCCESS;
}

extern unsigned int dbgflags;

static int dsb_proc_single()
{
	Event_t *e;

	//Choose an event
	e = queue_pop(QUEUE_ACTIVE);
	if (e == 0)
	{
		return 0;
	}
	else
	{
		//If an event was found then route it.
		dsb_route(e);

		//If we have a debugger connected.
		#ifdef _DEBUG
		if (dbgflags & DBG_EVENTS)
		{
			char buf[200];
			dsb_event_pretty(e,buf,200);
			dsb_log(DEBUG_EVENTS,buf);
		}
		if (debugsock != 0)
		{
			dsb_net_send_dbgevent(debugsock,e);
		}
		#endif

		//Mark as DONE
		e->flags |= EFLAG_DONE;

		if (e->flags & EFLAG_FREE)
		{
			dsb_event_free(e);
		}
		return 1;
	}
}

/**
 * Wait for an event to complete.
 * @param evt Event to wait for.
 * @return SUCCESS or ERR_NOTSENT.
 */
static int dsb_proc_wait(const Event_t *evt)
{
	if (!(evt->flags & EFLAG_SENT))
	{
		return ERR_NOTSENT;
	}
	while (!(evt->flags & EFLAG_DONE))
	{
		//Process active queue
		R_LOCK(qlock);
		while (dsb_proc_single() == 1);
		R_UNLOCK(qlock);

		printf("PROCWAIT waiting...\n");

		//Wait on condition
		LOCK(qmtx);
		while (!(evt->flags & EFLAG_DONE) && queue[QUEUE_ACTIVE].rix == queue[QUEUE_ACTIVE].wix)
		{
			WAIT(qcond,qmtx);
		}
		UNLOCK(qmtx);

		printf("PROCWAIT restart...\n");
	}

	return SUCCESS;
}

int dsb_send(Event_t *evt, bool async)
{
	int ret;

	evt->flags |= EFLAG_SENT;
	ret = queue_insert(QUEUE_AGENT, evt);

	//Need to block until done.
	if (async == false)
	{
		//BROADCAST(qcond);
		ret = dsb_proc_wait(evt);
	}
	else
	{
		//Wake up other threads...
		printf("Broadcasting...\n");
		BROADCAST(qcond);
	}

	return ret;
}

int dsb_sendACTIVE(Event_t *evt)
{
	int ret;

	evt->flags |= EFLAG_SENT;
	ret = queue_insert(QUEUE_ACTIVE, evt);

	BROADCAST(qcond);

	return ret;
}

static void move_active()
{
	EVENTTYPE_t type;
	Event_t *e;

	e = queue[QUEUE_AGENT].q[queue[QUEUE_AGENT].rix];
	if (e)
	{
		type = e->type;
	}
	while (queue[QUEUE_AGENT].rix != queue[QUEUE_AGENT].wix)
	{
		if (e->type == type)
		{
			queue_insert(QUEUE_ACTIVE,e);
			queue[QUEUE_AGENT].q[queue[QUEUE_AGENT].rix] = 0;
			queue[QUEUE_AGENT].rix = (queue[QUEUE_AGENT].rix + 1) % QUEUE_SIZE;
			e = queue[QUEUE_AGENT].q[queue[QUEUE_AGENT].rix];
		}
		else
		{
			break;
		}
	}
}

/*
 * Get accurate clock time.
 */
/*static long long getTicks()
{
	#ifdef UNIX
	unsigned long long ticks;
	struct timeval now;
	gettimeofday(&now, NULL);
	ticks = ((unsigned long long)now.tv_sec) * (unsigned long long)1000000 + ((unsigned long long)now.tv_usec);
	return ticks;
	#endif

	#ifdef WIN32
	LARGE_INTEGER tks;
	QueryPerformanceCounter(&tks);
	return (((unsigned long long)tks.HighPart << 32) + (unsigned long long)tks.LowPart);
	#endif
}*/

static void *dsb_proc_runthread(void *arg)
{
	printf("Thread active!\n");

	while (procisrunning)
	{
		//Process active queue
		R_LOCK(qlock);
		while (dsb_proc_single() == 1);
		R_UNLOCK(qlock);

		printf("Thread waiting...\n");
		BROADCAST(qcond); //TODO MAKE AGENT CONDITION

		//Wait on condition
		LOCK(qmtx);
		while (procisrunning && (queue[QUEUE_ACTIVE].rix == queue[QUEUE_ACTIVE].wix))
		{
			WAIT(qcond,qmtx);
		}
		UNLOCK(qmtx);

		printf("Thread restart...\n");
	}
	return SUCCESS;
}

int dsb_proc_run(unsigned int maxfreq)
{
	pthread_t threads[NUM_THREADS];
	int i;

	procisrunning = 1;
	printf("Creating threads\n");

	//Make some threads now
	for (i=0; i<NUM_THREADS; i++)
	{
		if (pthread_create(&threads[i],0,dsb_proc_runthread,0) != 0)
		{
			printf("Could not create threads!\n");
		}
	}

	while(procisrunning == 1)
	{
		//Wait if no agent events.
		LOCK(qmtx);
		while (procisrunning && (queue[QUEUE_AGENT].rix == queue[QUEUE_AGENT].wix))
		{
			printf("Waiting for agent queue...\n");
			WAIT(qcond,qmtx);
		}
		UNLOCK(qmtx);

		//Must have write lock to move from agent to active queue
		W_LOCK(qlock);
		printf("MOVE ACTIVE\n");
		move_active();
		W_UNLOCK(qlock);

		BROADCAST(qcond);
	}
	return SUCCESS;
}
