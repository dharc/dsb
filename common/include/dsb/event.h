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
#ifndef _EVENT_H_
#define _EVENT_H_

#include "dsb/harc.h"

#define MAX_EVENT_PARAMS	2

enum EventType
{
	EVENT_GET=0,
	EVENT_SET,
	EVENT_DEFINE,
	EVENT_DELETE,
	EVENT_DEP,
	EVENT_INVALID
};

/**
 * DSB Event base structure.
 */
struct Event
{
	enum EventType type;
	struct HARC dest;
	struct NID p[MAX_EVENT_PARAMS];
};


/**
 * Initialise the event subsystem. Must be called before any events are
 * allocated or freed.
 * @return 0 on success.
 */
int dsb_event_init();

/**
 * Cleanup memory upon exit. Should be called to safely close the
 * application.
 * @return 0 on success.
 */
int dsb_event_final();

/**
 * Allocate an event from the event pool.
 * @return Event pointer or NULL if no spare events.
 */
struct Event *dsb_event_allocate();

/**
 * Add the event back to the pool of available events.
 * @param evt The event previously allocated.
 */
void dsb_event_free(struct Event *evt);

/**
 * Number of parameters expected for this event.
 * @param evt
 * @return Number of expected parameters.
 */
int dsb_event_params(const struct Event *evt);

#endif

