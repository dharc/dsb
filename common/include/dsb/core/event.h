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

/** @file event.h */

#ifndef _EVENT_H_
#define _EVENT_H_

/**
 * @addtogroup Events
 * @{
 */

#define MAX_EVENT_PARAMS	6

#include "dsb/core/harc.h"
#include "dsb/core/nid.h"
#include "dsb/types.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
	EVENT_DEFINE=0x000,	//!< Define a HARC
	EVENT_SET,			//!< Set a HARC head to a constant.
	EVENT_DELETE,   	//!< Delete a HARC
	EVENT_NOTIFY,		//!< Notify HARC that it is out-of-date.
	EVENT_AGENT,		//!< Mark as being an agent.

	EVENT_GET=0x100,	//!< Get the head of a HARC
	EVENT_ALLOCATE,		//!< Allocate a new NID
	EVENT_GETDEF,

	EVENT_DEP=0x200,	//!< Add a dependency to a HARC
	EVENT_INVALID		//!< EVENT_INVALID
} EVENTTYPE_t;

enum
{
	EVENT_GETTERS=0,
	EVENT_SETTERS,
	EVENT_DEPENDENCIES,
	EVENT_NOTIFIES
};

typedef enum
{
	EFLAG_NONE = 0,
	EFLAG_FREE = 1,		///< Delete event when complete
	EFLAG_DONE = 2,		///< Event has been processed
	EFLAG_SENT = 4,		///< The event has been sent.
	EFLAG_MULT = 8,		///< An event with a destination region.
	EFLAG_VIRT = 16,	///< Virtual, do not use memory.
	EFLAG_ERRO = 32		///< There was an error in processing the event.
} EVENTFLAG_t;

/**
 * DSB Event structure.
 */
struct Event
{
	EVENTTYPE_t type;
	NID_t d1;			///< Destination
	NID_t d2;			///< Destination

	union {
	NID_t value;		///< Value parameter.

	NID_t *res;			///< Returned event result.
	int err;			///< Error number, if error flag set.

	struct {
	NID_t def;			///< Definition
	union {
	//int eval;			///< Evaluator to use.
	int resid;			///< Result ID to match on network
	};
	};
	};

	union {
	struct {
	NID_t d1b;			///< Second destination in multi events.
	NID_t d2b;			///< Second destination in multi events.
	};
	struct {
	NID_t dep1;		///< Dependency
	NID_t dep2;		///< Dependency
	};
	};

	//Not sent over network
	EVENTFLAG_t flags;	///< Event flags.
};

/**
 * Event Constructor.
 * @param[in] type
 * @param[in] d1
 * @param[in] d2
 * @param[out] evt
 * @return evt
 */
Event_t *dsb_event(EVENTTYPE_t type, const NID_t *d1, const NID_t *d2, Event_t *evt);

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
 * Serialise the event structure into a packed byte array. The number of bytes
 * it packs down to depends upon the event type.
 * @param e Source event to be serialised.
 * @param buf Output buffer.
 * @param max Maximum size of output buffer.
 * @return Number of bytes output.
 */
int dsb_event_pack(const Event_t *e, char *buf, int max);

/**
 * De-serialise from a packed byte array into an event structure.
 * @param buf Source buffer.
 * @param e Output event structure to populate.
 * @return Number of bytes read from buffer.
 */
int dsb_event_unpack(const char *buf, Event_t *e);

/**
 * Allocate an event from the event pool.
 * @return Event pointer or NULL if no spare events.
 */
Event_t *dsb_event_allocate();

/**
 * Add the event back to the pool of available events.
 * @param evt The event previously allocated.
 */
void dsb_event_free(Event_t *evt);

/**
 * Number of parameters expected for this event.
 * @param evt
 * @return Number of expected parameters.
 */
int dsb_event_params(const Event_t *evt);

int dsb_event_pretty(const Event_t *evt, char *buf, int len);

/** @} */

#ifdef __cplusplus
}
#endif

#endif

