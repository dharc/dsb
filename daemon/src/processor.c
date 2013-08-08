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

#define QUEUE_SIZE					10000
#define NUM_THREADS					2
#define ACTIVE_RESTART_SCALER		100

//TODO Need to properly support rix and wix.

/*
 * A threadsafe event queue. There should be 3 of these.
 */
struct EventQueue
{
	Event_t **q;
	unsigned int rix;	//< Current read index
	unsigned int wix;	//< Current write index
	pthread_mutex_t mtx;
};

MUTEX(qwaitmtx);
RWLOCK(qlock);
COND(qcond);
COND(qagentcond);
COND(qwaitcond);

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
	LOCK(queue[q].mtx);

	//Oops, queue is full.
	if (queue[q].q[queue[q].wix] != 0)
	{
		UNLOCK(queue[q].mtx);
		return ERR_PROC_FULL;
	}
	queue[q].q[queue[q].wix] = e;
	queue[q].wix = (queue[q].wix + 1) % QUEUE_SIZE;

	UNLOCK(queue[q].mtx);

	return SUCCESS;
}

/*
 * Thread-safe event queue removal.
 */
static Event_t *queue_pop(QUEUE_t q)
{
	Event_t *res = 0;

	LOCK(queue[q].mtx);

	//Are there any events left to read.
	res = queue[q].q[queue[q].rix];

	if (res != 0)
	{
		queue[q].q[queue[q].rix] = 0;
		queue[q].rix = (queue[q].rix + 1) % QUEUE_SIZE;
	}

	UNLOCK(queue[q].mtx);

	return res;
}

int dsb_proc_init()
{
	queue[QUEUE_AGENT].q = malloc(sizeof(Event_t*) * QUEUE_SIZE);
	memset(queue[QUEUE_AGENT].q,0,sizeof(Event_t*)*QUEUE_SIZE);
	queue[QUEUE_AGENT].rix = 0;
	queue[QUEUE_AGENT].wix = 0;
	pthread_mutex_init(&queue[QUEUE_AGENT].mtx,0);

	queue[QUEUE_ACTIVE].q = malloc(sizeof(Event_t*) * QUEUE_SIZE);
	memset(queue[QUEUE_ACTIVE].q,0,sizeof(Event_t*)*QUEUE_SIZE);
	queue[QUEUE_ACTIVE].rix = 0;
	queue[QUEUE_ACTIVE].wix = 0;
	pthread_mutex_init(&queue[QUEUE_ACTIVE].mtx,0);
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
	BROADCAST(qagentcond);
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

	LOCK(qwaitmtx);
	while (procisrunning && !(evt->flags & EFLAG_DONE))
		WAIT(qwaitcond,qwaitmtx);
	UNLOCK(qwaitmtx);

	return SUCCESS;
}

int dsb_send(Event_t *evt, bool async)
{
	int ret = 0;

	evt->flags |= EFLAG_SENT;
	//Insert and if queue full process and try again
	while (procisrunning && queue_insert(QUEUE_AGENT, evt))
	{
		//printf("QUEUE FULL... processing\n");
		//dsb_proc_active();

		LOCK(queue[QUEUE_ACTIVE].mtx);
		WAIT(qcond,queue[QUEUE_ACTIVE].mtx);
		UNLOCK(queue[QUEUE_ACTIVE].mtx);

		//LOCK(queue[QUEUE_AGENT].mtx);
		//while (procisrunning && queue[QUEUE_AGENT].q[queue[QUEUE_AGENT].wix] != 0)
		//	WAIT(qagentcond,queue[QUEUE_AGENT].mtx);
		//UNLOCK(queue[QUEUE_AGENT].mtx);
	}

	//Check if agent thread has write lock
	//if (TRY_R_LOCK(qlock))
	//{
		//If yes then signal the agent.
		LOCK(queue[QUEUE_AGENT].mtx);
		SIGNAL(qagentcond);
		UNLOCK(queue[QUEUE_AGENT].mtx);
	//}
	//else
	//{
	//	R_UNLOCK(qlock);
	//}

	//Need to block until done.
	if (async == false)
	{
		ret = dsb_proc_wait(evt);
	}

	return ret;
}

int dsb_sendACTIVE(Event_t *evt)
{
	int ret;

	evt->flags |= EFLAG_SENT;
	ret = queue_insert(QUEUE_ACTIVE, evt);

	//TODO CHECK RETURN VALUE FOR FULL QUEUES.

	//LOCK(queue[QUEUE_ACTIVE].mtx);
	//BROADCAST(qcond);
	//UNLOCK(queue[QUEUE_ACTIVE].mtx);

	return ret;
}

static void move_active(EVENTTYPE_t type, int num)
{
	Event_t *e;

	e = queue[QUEUE_AGENT].q[queue[QUEUE_AGENT].rix];

	while (e)
	{
		if (e->type == type)
		{
			//If full then stop and go back to processing active queue
			if (queue_insert(QUEUE_ACTIVE,e))
			{
				return;
			}
			//Inserted into active queue so remove from agent.
			queue[QUEUE_AGENT].q[queue[QUEUE_AGENT].rix] = 0;
			queue[QUEUE_AGENT].rix = (queue[QUEUE_AGENT].rix + 1) % QUEUE_SIZE;

			//Next...
			e = queue[QUEUE_AGENT].q[queue[QUEUE_AGENT].rix];
			--num;
		}
		else
		{
			break;
		}

		if (num <= 0) return;
	}
}

static void *dsb_proc_runthread(void *arg)
{
	Event_t *e;
	EVENTTYPE_t type = EVENT_INVALID;

	while (procisrunning)
	{
		//Process active queue
		R_LOCK(qlock);
		while (dsb_proc_single() == 1);
		R_UNLOCK(qlock);

		if (TRY_W_LOCK(qlock) == 0)
		{
			//Wake any blocking sends to check if they are done yet
			LOCK(qwaitmtx);
			BROADCAST(qwaitcond);
			UNLOCK(qwaitmtx);

			//Wait for agent events......
			LOCK(queue[QUEUE_AGENT].mtx);
			while (procisrunning
					&& ((queue[QUEUE_AGENT].q[queue[QUEUE_AGENT].rix] == 0)))
				WAIT(qagentcond,queue[QUEUE_AGENT].mtx);
			UNLOCK(queue[QUEUE_AGENT].mtx);

			//Select next type of agent event
			e = queue[QUEUE_AGENT].q[queue[QUEUE_AGENT].rix];
			if (e)
			{
				type = e->type;
			}
			//Remove some events before restarting threads
			move_active(type,NUM_THREADS*ACTIVE_RESTART_SCALER);
			W_UNLOCK(qlock);
			move_active(type,QUEUE_SIZE);

			LOCK(queue[QUEUE_ACTIVE].mtx);
			BROADCAST(qcond);
			UNLOCK(queue[QUEUE_ACTIVE].mtx);
		}
		else
		{
			YIELD;
		}
	}
	return SUCCESS;
}

int dsb_proc_run(unsigned int maxfreq)
{
	pthread_t threads[NUM_THREADS];
	int i;

	procisrunning = 1;
	//printf("Creating threads\n");

	//Make some threads now
	for (i=0; i<NUM_THREADS; i++)
	{
		if (pthread_create(&threads[i],0,dsb_proc_runthread,0) != 0)
		{
			printf("Could not create threads!\n");
		}
	}

	dsb_proc_runthread(0);
	return SUCCESS;
}
