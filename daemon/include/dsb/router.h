/*
 * router.h
 *
 *  Created on: 29 Apr 2013
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

/** @file router.h */

#ifndef ROUTER_H_
#define ROUTER_H_

struct Event;
struct NID;

#define ROUTE_VOLATILE		0x0
#define ROUTE_PERSISTENT	0x1
#define ROUTE_REMOTE		0x2
#define ROUTE_LOCAL			0x0

/**
 * @addtogroup Router
 * The router will route events to event handlers as described in its internal
 * routing table. The routing table consists of non overlapping 2D regions in
 * Node space that are associated with a handler. Events should not be routed
 * directly, but go via the processor to be scheduled. The only function to
 * be used externally, therefore, is dsb_route_map.
 * @see dsb_route_map
 * @see Processor
 * @{
 */

int dsb_route_init(void);
int dsb_route_final(void);

/**
 * Map a region of node space to an event handler. All events destined
 * for that region will be routed to the handler. A region is
 * 2-dimensional and so has an x range and y range but it does not matter
 * which range is x or y and they can be flipped.
 * @param flags		Kind of handler.
 * @param num		Number of that kind (should be 0 at present).
 * @param handler	Event handler for the region.
 * @return SUCCESS, ERR_ROUTE_SLOT.
 * @author Nick Pope
 */
int dsb_route_map(
		int flags, int num,
		int (*handler)(struct Event *));

/**
 * Route event to correct handler. Should not be called manually.
 * @param evt The event to route.
 * @return SUCCESS, ERR_NOROUTE or ERR_ROUTE_MISSING.
 */
int dsb_route(struct Event *evt);

/** @} */

#endif /* ROUTER_H_ */
