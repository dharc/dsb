/*
 * volatile.c
 *
 *  Created on: 8 May 2013
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

#include "dsb/module.h"
#include "dsb/router.h"
#include "dsb/event.h"
#include "dsb/errors.h"
#include "dsb/evaluator.h"
#include <stdio.h>
#include <malloc.h>

struct Module volmod;

#define VOLFLAG_OUTOFDATE	1	//Mark entry as out-of-date.
#define VOLFLAG_VIRTUAL		2	//Mark region as not wanting caching.

/*
 * An entry to record a single HARC in memory.
 */
struct VolHARCEntry
{
	struct HARC harc;			//The core HARC data.
	void *data;					//Used by the definition evaluator.
	int flags;					//Meta details for volatile storage.
	struct VolHARCEntry *next;	//Next entry in hash table.
};

/*
 * A region entry to define a whole collection of HARCs.
 */
struct VolRegionEntry
{
	struct NID x1;		//Low X NID
	struct NID x2;		//High X NID
	struct NID y1;		//Low Y NID
	struct NID y2;		//High Y NID
	struct NID def;		//Definition for this region of HARCs
	void *data;			//Data for the definition evaluator.
	int eval;			//Which evaluator to use for this region.
	int flags;			//Meta details about the region.
};

#define VOL_HASH_SIZE	1000
#define VOL_MAX_REGIONS	100

//Entry and region storage
struct VolHARCEntry *voltable[VOL_HASH_SIZE];
struct VolRegionEntry *volregions[VOL_MAX_REGIONS];

/*
 * HASH two NIDs together.
 */
int vol_hashnid(const struct NID *a, const struct NID *b)
{
	//TODO Make sure a and b are in correct order.
	//TODO Improve hash function
	return (a->ll + (b->ll*100)) % VOL_HASH_SIZE;
}

/*
 * Create a new region entry with everything initialised to 0.
 */
struct VolRegionEntry *vol_createregion(const struct NID *x1, const struct NID *x2, const struct NID *y1, const struct NID *y2)
{
	struct VolRegionEntry *res;
	int i;

	res = malloc(sizeof(struct VolRegionEntry));
	res->x1 = *x1;
	res->x2 = *x2;
	res->y1 = *y1;
	res->y2 = *y2;
	res->flags = 0;
	res->data = 0;
	res->eval = 0;
	res->def.type = 0;
	res->def.ll = 0;

	//Find a space for the region and insert it.
	//TODO Use 2D search structure for better performance.
	for (i=0; i<VOL_MAX_REGIONS; i++)
	{
		if (volregions[i] == 0)
		{
			volregions[i] = res;
			return res;
		}
	}

	return 0;
}

/*
 * Return 0 if n is within the range of a and b. 1 otherwise.
 * Copied from router.
 */
inline int nid_withinrange2(const struct NID *a, const struct NID *b, const struct NID *n)
{
	//NOTE: Assumption about a being less than b!!!!
	return ((dsb_nid_compare(a,n) <= 0) && (dsb_nid_compare(b,n) >= 0)) ? 0 : 1;
}

/*
 * Search for and return a region that matches the HARC tail given as
 * parameters a and b. 0 is returned if no match is found.
 */
struct VolRegionEntry *vol_getregion(const struct NID *a, const struct NID *b)
{
	//TODO Use search algorithm.
	int i;
	int t1,t2;

	for (i=0; i<VOL_MAX_REGIONS; i++)
	{
		if (volregions[i] == 0) return 0;

		//Check first order is in the range
		t1 = nid_withinrange2(&(volregions[i]->x1),&(volregions[i]->x2),a);
		t2 = nid_withinrange2(&(volregions[i]->y1),&(volregions[i]->y2),b);
		if (t1 == 0 && t2 == 0)
		{
			return volregions[i];
		}

		//Check second order is in the range
		t1 = nid_withinrange2(&(volregions[i]->x1),&(volregions[i]->x2),b);
		t2 = nid_withinrange2(&(volregions[i]->y1),&(volregions[i]->y2),a);
		if (t1 == 0 && t2 == 0)
		{
			return volregions[i];
		}
	}

	return 0;
}

/*
 * Construct a new HARC entry in the hash table.
 */
struct VolHARCEntry *vol_createentry(const struct NID *a, const struct NID *b)
{
	int hash = vol_hashnid(a,b);
	struct VolHARCEntry *res;

	res = malloc(sizeof(struct VolHARCEntry));
	res->harc.t1 = *a;
	res->harc.t2 = *b;
	res->harc.h.type = NID_SPECIAL;
	res->harc.h.ll = 0;
	res->harc.def.type = NID_SPECIAL;
	res->harc.def.ll = 0;
	res->harc.e = 0;
	res->data = 0;
	res->flags = 0;

	//TODO Make threadsafe
	res->next = voltable[hash];
	voltable[hash] = res;
	return res;
}

/*
 * Find or create a HARC entry in the hash table.
 */
struct VolHARCEntry *vol_getentry(const struct NID *a, const struct NID *b)
{
	int hash = vol_hashnid(a,b);
	//TODO Make threadsafe.
	struct VolHARCEntry *res = voltable[hash];

	//TODO Check a and b ordering.
	while (res != 0)
	{
		//Does this HARC entry match?
		if ((dsb_nid_compare(a,&(res->harc.t1)) == 0) && (dsb_nid_compare(b,&(res->harc.t2)) == 0))
		{
			return res;
		}
		//Move to next entry.
		res = res->next;
	}

	return 0;
}

/*
 * Process DEFINE events. Updates definition and sets to out-of-date.
 */
int vol_define(struct Event *evt)
{
	struct VolHARCEntry *ent = vol_getentry(&(evt->d1),&(evt->d2));

	//If it doesn't exist then create it now.
	if (ent == 0)
	{
		ent = vol_createentry(&(evt->d1),&(evt->d2));
	}

	ent->harc.e = evt->eval;
	ent->harc.def.type = evt->def.type;
	ent->harc.def.ll = evt->def.ll;
	ent->flags |= VOLFLAG_OUTOFDATE;

	//Generate NOTIFY events to mark others as out-of-date.

	return SUCCESS;
}

/*
 * Process DEFINE events for regions.
 */
int vol_defineregion(struct Event *evt)
{
	struct VolRegionEntry *reg = vol_getregion(&(evt->d1),&(evt->d2));

	//If it doesn't exist then create it now.
	if (reg == 0)
	{
		reg = vol_createregion(&(evt->d1),&(evt->d1b),&(evt->d2),&(evt->d2b));
		reg->def = evt->def;
		reg->eval = evt->eval;
		//TODO set region flags
		return SUCCESS;
	}

	/*
	 * Otherwise may need to split existing regions and remove cached entries.
	 * Need to also consider how non region defines interact in areas that
	 * have region definitions.
	 */

	return SUCCESS;
}

/*
 * Process NOTIFY events by marking as out-of-date.
 */
int vol_notify(struct Event *evt)
{
	//Find the relevant HARC entry...
	struct VolHARCEntry *ent = vol_getentry(&(evt->d1),&(evt->d2));
	//Do not need to worry about regions as they are always out-of-date.
	if (ent == 0) return SUCCESS;

	ent->flags |= VOLFLAG_OUTOFDATE;
	return SUCCESS;
}

/*
 * Process GET events by returning the cached value or evaluating the
 * definition. The special case is for a HARC defined as a region where
 * the HARC needs to be created, or if virtual then simulated.
 */
int vol_get(struct Event *evt)
{
	struct VolHARCEntry *ent = vol_getentry(&(evt->d1),&(evt->d2));
	struct VolRegionEntry *reg;
	struct HARC harc;

	//If no entry, check for region
	if (ent == 0)
	{
		//Check if matches a region.
		reg = vol_getregion(&(evt->d1),&(evt->d2));
		if (reg != 0)
		{
			//Matches a region.
			if ((reg->flags & VOLFLAG_VIRTUAL) != 0)
			{
				//Its a virtual region so don't create a real HARC
				harc.t1.type = evt->d1.type;
				harc.t1.ll = evt->d1.ll;
				harc.t2.type = evt->d2.type;
				harc.t2.ll = evt->d2.ll;
				harc.def.type = reg->def.type;
				harc.def.ll = reg->def.ll;
				//Get evaluator and use to get result.
				dsb_eval_call(reg->eval,&harc,&(reg->data));
				evt->res.type = harc.h.type;
				evt->res.ll = harc.h.ll;
				evt->flags |= EVTFLAG_DONE;
				return SUCCESS;
			}
			else
			{
				//Its not virtual, so cache as real entry.
				ent = vol_createentry(&(evt->d1),&(evt->d2));
				ent->harc.e = reg->eval;
				ent->harc.def.type = reg->def.type;
				ent->harc.def.ll = reg->def.ll;
				ent->flags |= VOLFLAG_OUTOFDATE;
			}
		}
		else
		{
			//No such HARC so return Null.
			evt->res.type = 0;
			evt->res.ll = 0;
			evt->flags |= EVTFLAG_DONE;
			return SUCCESS;
		}
	}

	//Out of date so evaluate definition
	if ((ent->flags & VOLFLAG_OUTOFDATE) != 0)
	{
		//Get evaluator and use to get result.
		dsb_eval_call(ent->harc.e,&(ent->harc),&(ent->data));

		//No longer out-of-date
		evt->flags &= ~VOLFLAG_OUTOFDATE;
	}

	evt->res.type = ent->harc.h.type;
	evt->res.ll = ent->harc.h.ll;
	evt->flags |= EVTFLAG_DONE;
	return SUCCESS;
}

/*
 * Main event handler for volatile storage.
 */
int vol_handler(struct Event *evt)
{
	if ((evt->flags & EVTFLAG_MULT) == 0)
	{
		switch(evt->type)
		{
		case EVENT_GET: return vol_get(evt);
		case EVENT_DEFINE: return vol_define(evt);
		case EVENT_NOTIFY: return vol_notify(evt);
		default: return SUCCESS;
		}
	}
	else
	{
		switch(evt->type)
		{
		//No MULTI GET
		case EVENT_DEFINE: return vol_defineregion(evt);
		//case EVENT_NOTIFY: return vol_notify(evt);
		default: return SUCCESS;
		}
	}
}

int vol_init(const struct NID *base)
{
	struct NID x1;
	struct NID x2;
	int i;

	//Clear voltable
	for (i=0; i<VOL_HASH_SIZE; i++)
	{
		voltable[i] = 0;
	}
	//Clear volregions
	for (i=0; i<VOL_MAX_REGIONS; i++)
	{
		volregions[i] = 0;
	}

	//The entire Node space below user nodes.
	x1.type = 0;
	x1.ll = 0;
	x2.type = NID_USER-1;
	x2.ll = 0xFFFFFFFFFFFFFFFF;
	dsb_route_map(&x1,&x2,&x1,&x2,vol_handler);

	return SUCCESS;
}

int vol_final()
{
	//TODO Cleanup all entries.
	//TODO Cleanup all regions.
	return SUCCESS;
}

/*
 * Module registration structure.
 */
struct Module *dsb_volatile_module()
{
	volmod.init = vol_init;
	volmod.update = 0;
	volmod.final = vol_final;
	return &volmod;
}

