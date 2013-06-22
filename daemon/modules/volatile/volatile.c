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

#include "dsb/core/module.h"
#include "dsb/router.h"
#include "dsb/core/event.h"
#include "dsb/errors.h"
#include "dsb/evaluator.h"
#include "dsb/harc_d.h"
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
struct VolHARCEntry *voltable[VOL_HASH_SIZE] = {0};
struct VolRegionEntry *volregions[VOL_MAX_REGIONS] = {0};

static int lastallocated = 1;

/*
 * HASH two NIDs together.
 */
int vol_hashnid(const struct NID *a, const struct NID *b)
{
	//TODO Make sure a and b are in correct order.
	//TODO Improve hash function
	return (a->n + (b->n*100)) % VOL_HASH_SIZE;
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
	dsb_nid_null(&(res->def));

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
/*inline int nid_withinrange2(const struct NID *a, const struct NID *b, const struct NID *n)
{
	//NOTE: Assumption about a being less than b!!!!
	return ((dsb_nid_compare(a,n) <= 0) && (dsb_nid_compare(b,n) >= 0)) ? 0 : 1;
}*/

/*
 * Search for and return a region that matches the HARC tail given as
 * parameters a and b. 0 is returned if no match is found.
 */
/*struct VolRegionEntry *vol_getregion(const struct NID *a, const struct NID *b)
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
}*/

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
	dsb_nid_null(&res->harc.h);
	dsb_nid_null(&res->harc.def);
	res->harc.e = 0;
	res->harc.deps = 0;
	res->flags = 0;

	//TODO Make threadsafe
	res->next = voltable[hash];
	voltable[hash] = res;
	return res;
}

/*
 * Find or create a HARC entry in the hash table.
 */
HARC_t *vol_getharc(const NID_t *a, const NID_t *b, int create)
{
	int hash = vol_hashnid(a,b);
	//TODO Make threadsafe.
	struct VolHARCEntry *res = voltable[hash];

	//TODO Check a and b ordering.
	while (res != 0)
	{
		//Does this HARC entry match?
		if ((dsb_nid_eq(a,&(res->harc.t1)) == 1) && (dsb_nid_eq(b, &(res->harc.t2)) == 1))
		{
			return &(res->harc);
		}
		//Move to next entry.
		res = res->next;
	}

	if (create != 0)
	{
		return &(vol_createentry(a,b)->harc);
	}
	else
	{
		//Check regions.
	}

	return 0;
}


/*
 * Process DEFINE events for regions.
 */
/*int vol_defineregion(struct Event *evt)
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


	return SUCCESS;
}*/


/*
 * Process GET events by returning the cached value or evaluating the
 * definition. The special case is for a HARC defined as a region where
 * the HARC needs to be created, or if virtual then simulated.
 */
/*int vol_get(struct Event *evt)
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
				harc.t1 = evt->d1;
				harc.t2 = evt->d2;
				harc.def = reg->def;
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

	return dsb_harc_handler(&(ent->harc),evt);
}*/

/*
 * Main event handler for volatile storage.
 */
int vol_handler(Event_t *evt)
{
	if (evt->type == EVENT_ALLOCATE)
	{
		dsb_nid_local(0,evt->res);
		//TODO Put a mutex on this
		evt->res->n = lastallocated++;
		evt->flags |= EVTFLAG_DONE;
		return SUCCESS;
	}

	if ((evt->flags & EVTFLAG_MULT) == 0)
	{
		HARC_t *harc = vol_getharc(&(evt->d1),&(evt->d2),evt->type != EVENT_GET);
		return dsb_harc_event(harc,evt);
	}
	else
	{
		return SUCCESS;
	}
}

int vol_init(const NID_t *base)
{
	//A local volatile handler
	dsb_route_map(0,0,vol_handler);

	return SUCCESS;
}

#ifdef _DEBUG

/*
 * Open a file and dump hash table contents to it in CSV form.
 */
static int vol_save_file(const char *filename)
{
	int h=0;
	FILE *fd;
	struct VolHARCEntry *cur;

	//Open relevant file.
	fd = fopen(filename,"w");
	if (fd == 0)
	{
		return DSB_ERROR(ERR_PERFILESAVE,filename);
	}

	//Write the lastallocated NID
	fprintf(fd, "%x\n", lastallocated);

	//For every hash location
	for (h=0; h<VOL_HASH_SIZE; h++)
	{
		cur = voltable[h];
		//For every element at this hash location
		while (cur != 0)
		{
			//Save it!!
			dsb_harc_serialize(fd, &cur->harc);
			cur = cur->next;
		}
	}

	fclose(fd);

	return 0;
}

extern unsigned int dbgflags;

#endif

int vol_final()
{
	//TODO Cleanup all entries.
	//TODO Cleanup all regions.

#ifdef _DEBUG
	if (dbgflags & DBG_VOLATILE)
	{
		vol_save_file("./volatile.log");
	}
#endif

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

