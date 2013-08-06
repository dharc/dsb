/*
 * persisnte.c
 *
 *  Created on: 4 June 2013
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
#include "dsb/core/thread.h"

Module_t permod;

#define PERFLAG_OUTOFDATE	1	//Mark entry as out-of-date.
#define PERFLAG_VIRTUAL		2	//Mark region as not wanting caching.

RWLOCK(plock);

/*
 * An entry to record a single HARC in memory.
 */
struct PerHARCEntry
{
	HARC_t harc;				//The core HARC data.
	void *data;					//Used by the definition evaluator.
	int flags;					//Meta details for volatile storage.
	struct PerHARCEntry *next;	//Next entry in hash table.
};

#define PER_HASH_SIZE	1000

//Entry and region storage
static struct PerHARCEntry *pertable[PER_HASH_SIZE];

static int lastallocated = 1;

/*
 * HASH two NIDs together.
 */
static int per_hashnid(const struct NID *a, const struct NID *b)
{
	//TODO Make sure a and b are in correct order.
	//TODO Improve hash function
	return (a->n + (b->n*100)) % PER_HASH_SIZE;
}

/*
 * Construct a new HARC entry in the hash table.
 */
static struct PerHARCEntry *per_createentry(const struct NID *a, const struct NID *b)
{
	int hash = per_hashnid(a,b);
	struct PerHARCEntry *res;

	res = malloc(sizeof(struct PerHARCEntry));
	res->harc.t1 = *a;
	res->harc.t2 = *b;
	dsb_nid_null(&res->harc.h);
	dsb_nid_null(&res->harc.def);
	res->harc.deps = 0;
	res->flags = 0;

	W_LOCK(plock);
	res->next = pertable[hash];
	pertable[hash] = res;
	W_UNLOCK(plock);
	return res;
}

/*
 * Find or create a HARC entry in the hash table.
 */
static HARC_t *per_getharc(const NID_t *a, const NID_t *b, int create)
{
	int hash = per_hashnid(a,b);
	struct PerHARCEntry *res;

	R_LOCK(plock);
	res = pertable[hash];

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
	R_UNLOCK(plock);

	if (create != 0)
	{
		return &(per_createentry(a,b)->harc);
	}
	else
	{
		//Check regions.
	}

	return 0;
}

/*
 * Main event handler for volatile storage.
 */
static int per_handler(Event_t *evt)
{
	if (evt->type == EVENT_ALLOCATE)
	{
		NID_t res;
		dsb_nid_local(NID_PERSISTENT,&res);
		//TODO Put a mutex on this
		res.n = lastallocated++;

		if (evt->cb)
		{
			evt->cb(evt,&res);
		}
		evt->flags |= EFLAG_DONE;
		return SUCCESS;
	}

	if ((evt->flags & EFLAG_MULT) == 0)
	{
		HARC_t *harc = per_getharc(&(evt->d1),&(evt->d2),evt->type != EVENT_GET);
		return dsb_harc_event(harc,evt);
	}
	else
	{
		return SUCCESS;
	}
}

static int per_load_file(const char *filename)
{
	HARC_t harc;
	HARC_t *pharc;
	FILE *fd;

	//Open relevant file.
	fd = fopen(filename,"r");
	if (fd == 0)
	{
		return DSB_ERROR(ERR_PERFILELOAD,filename);
	}

	//Read the last allocated NID.
	if (fscanf(fd, "%x\n", &lastallocated) != 1)
	{
		fclose(fd);
		return DSB_ERROR(ERR_PERFILELOAD,filename);
	}

	printf("Loading %s\n",filename);

	while (dsb_harc_deserialize(fd, &harc) == 0)
	{
		pharc = per_getharc(&harc.t1,&harc.t2,1);
		pharc->def = harc.def;
		pharc->h = harc.h;
		pharc->flags = harc.flags;

		if ((harc.flags & HARC_SCRIPT) != 0)
		{
			pharc->flags |= HARC_OUTOFDATE;
		}
	}

	fclose(fd);
	return 0;
}

/*
 * Open a file and dump hash table contents to it in CSV form.
 */
static int per_save_file(const char *filename)
{
	int h=0;
	FILE *fd;
	struct PerHARCEntry *cur;

	//Open relevant file.
	fd = fopen(filename,"w");
	if (fd == 0)
	{
		return DSB_ERROR(ERR_PERFILESAVE,filename);
	}

	//Write the lastallocated NID
	fprintf(fd, "%x\n", lastallocated);

	//For every hash location
	for (h=0; h<PER_HASH_SIZE; h++)
	{
		cur = pertable[h];
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

int per_init(const NID_t *base)
{
	//A local volatile handler
	dsb_route_map(ROUTE_PERSISTENT | ROUTE_LOCAL,0,per_handler);

	per_load_file("store.dsb");

	return SUCCESS;
}

int per_final()
{
	per_save_file("store.dsb");
	//TODO Cleanup all entries.
	//TODO Cleanup all regions.
	return SUCCESS;
}

/*
 * Module registration structure.
 */
Module_t *dsb_persistent_module()
{
	permod.init = per_init;
	permod.update = 0;
	permod.final = per_final;
	return &permod;
}

