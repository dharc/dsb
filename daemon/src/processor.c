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
#include "dsb/event.h"
#include "dsb/errors.h"
#include "dsb/router.h"
#include "config.h"
#include <malloc.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>

#if defined(UNIX) && !defined(NO_THREADS)
#include <pthread.h>
#endif

#define QUEUE_SIZE		10000

/*
 * A threadsafe event queue. There should be 3 of these.
 */
struct EventQueue
{
	Event_t **q;
	unsigned int rix;
	unsigned int wix;
	#if defined(UNIX) && !defined(NO_THREADS)
	pthread_mutex_t mtx;
	#endif
};

#define WRITE_QUEUE			0
#define READ_QUEUE			1
#define DEPENDENCY_QUEUE	2

struct EventQueue queue[3];
unsigned int curq;	//Current queue being processed.

int queue_insert(int q, Event_t *e)
{
	#if defined(UNIX) && !defined(NO_THREADS)
	pthread_mutex_lock(&(queue[q].mtx));
	#endif

	queue[q].q[queue[q].wix++] = e;

	#if defined(UNIX) && !defined(NO_THREADS)
	pthread_mutex_unlock(&(queue[q].mtx));
	#endif

	return SUCCESS;
}

Event_t *queue_pop(int q)
{
	Event_t *res = 0;

	#if defined(UNIX) && !defined(NO_THREADS)
	pthread_mutex_lock(&(queue[q].mtx));
	#endif

	//Are there any events left to read.
	if (queue[q].rix < queue[q].wix)
	{
		res = queue[q].q[queue[q].rix++];
	}

	#if defined(UNIX) && !defined(NO_THREADS)
	pthread_mutex_unlock(&(queue[q].mtx));
	#endif

	return res;
}

int dsb_proc_init()
{
	int i;
	for (i=0; i<3; i++)
	{
		queue[i].q = malloc(sizeof(Event_t*) * QUEUE_SIZE);
		memset(queue[i].q,0,sizeof(Event_t*)*QUEUE_SIZE);
		queue[i].rix = 0;
		queue[i].wix = 0;
		#if defined(UNIX) && !defined(NO_THREADS)
		pthread_mutex_init(&(queue[i].mtx),0);
		#endif
	}
	return SUCCESS;
}

int dsb_proc_final()
{
	int i;
	for (i=0; i<3; i++)
	{
		free(queue[i].q);
	}
	return SUCCESS;
}

//Map this to local processor send implementation.
int dsb_send(struct Event *evt, int async)
{
	return dsb_proc_send(evt,async);
}

int dsb_proc_send(struct Event *evt, int async)
{
	int ret;
	int q = evt->type >> 8;
	evt->flags |= EVTFLAG_SENT;
	ret = queue_insert(q,evt);

	//Need to block until done.
	if (async == SYNC)
	{
		ret = dsb_proc_wait(evt);
	}

	if (evt->flags & EVTFLAG_FREE)
	{
		dsb_event_free(evt);
	}
	return ret;
}

int dsb_proc_wait(const struct Event *evt)
{
	if (!(evt->flags & EVTFLAG_SENT))
	{
		return ERR_NOTSENT;
	}
	while (!(evt->flags & EVTFLAG_DONE))
	{
		//Process other events etc.
		dsb_proc_single();
	}

	return SUCCESS;
}

int dsb_proc_single()
{
	int q = curq;
	Event_t *e;
	//Choose an event
	e = queue_pop(q);
	if (e == 0)
	{
		return 0;
	}
	else
	{
		//If an event was found then route it.
		dsb_route(e);
		return 1;
	}
}

long long getTicks()
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
}

int dsb_proc_run(unsigned int maxfreq)
{
	long long tick;
	long long maxticks = maxfreq * 1000000;
	long long sleeptime;

	while(1)
	{
		tick = getTicks();
		//Loop through all the queues.
		curq = WRITE_QUEUE;
		while (dsb_proc_single() == 1);
		curq = READ_QUEUE;
		while (dsb_proc_single() == 1);
		curq = DEPENDENCY_QUEUE;
		while (dsb_proc_single() == 1);

		//Work out how much spare time we have.
		sleeptime = (maxticks - (getTicks() - tick)) / 10000;
		if (sleeptime > 0)
		{
			usleep(sleeptime);
		}
	}
	return SUCCESS;
}

int dsb_proc_runthread()
{
	return SUCCESS;
}
