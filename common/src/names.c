/*
 * names.c
 *
 *  Created on: 5 Jun 2013
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

#include "dsb/nid.h"
#include "dsb/globals.h"
#include "dsb/specials.h"
#include "dsb/string.h"
#include "dsb/array.h"
#include "dsb/names.h"
#include "dsb/errors.h"
#include "dsb/wrap.h"

#include <string.h>
#include <malloc.h>

#define MAX_NAME_SIZE	50
#define NAME_HASH_SIZE	200

struct NameEntry
{
	char name[MAX_NAME_SIZE];
	NID_t nid;
	struct NameEntry *next;
};

static struct NameEntry *nametable[NAME_HASH_SIZE];
static NID_t namesobj;
static int countnames;

/*
 * RWLOCK for threadsafe access to nametable.
 */
#if defined(UNIX) && !defined(NO_THREADS)
#include <pthread.h>
static pthread_rwlock_t nametable_mtx = PTHREAD_RWLOCK_INITIALIZER;
#endif //LINUX THREADED

int dsb_names_init()
{
	//Builtins
	dsb_names_add("root",&Root);
	dsb_names_add("proot",&PRoot);
	dsb_names_add("null",&Null);
	dsb_names_add("true",&True);
	dsb_names_add("false",&False);
	dsb_names_add("size",&Size);
	dsb_names_add("names",&Names);

	return 0;
}

int dsb_names_final()
{
	struct NameEntry *entry;
	struct NameEntry *tmp;
	int i;

	//Cleanup memory for name entries.
	for (i=0; i<NAME_HASH_SIZE; i++)
	{
		entry = nametable[i];
		while (entry != 0)
		{
			tmp = entry->next;
			free(entry);
			entry = tmp;
		}
	}

	return 0;
}

static int namehash(const char *name)
{
	unsigned int hash = 0;
	int c;

	while ((c = *name++))
		hash = c + (hash << 6) + (hash << 16) - hash;

	return hash % NAME_HASH_SIZE;
}

int dsb_names_add(const char *name, const NID_t *nid)
{
	struct NameEntry *entry;
	int hash = namehash(name);

	entry = malloc(sizeof(struct NameEntry));
	strcpy(entry->name,name);
	entry->nid = *nid;

	//WRITE LOCK
#if defined(UNIX) && !defined(NO_THREADS)
	pthread_rwlock_wrlock(&nametable_mtx);
#endif

	entry->next = nametable[hash];
	nametable[hash] = entry;

#if defined(UNIX) && !defined(NO_THREADS)
	pthread_rwlock_unlock(&nametable_mtx);
#endif

	return 0;
}

/*
 * Note that rebuild is not threadsafe and is assumed to only be called
 * once or very rarely by a single thread.
 */
int dsb_names_rebuild()
{
	dsb_names_update("root",&Root);
	dsb_names_update("proot",&PRoot);

	//Get the new persistent names object.
	dsb_get(&PRoot,&Names,&namesobj);

	//If there is a names object then
	if (dsb_nid_eq(&namesobj,&Null) == 0)
	{
		int i;
		char buf[MAX_NAME_SIZE];
		NID_t *array;

		countnames = dsb_array_readalloc(&namesobj,&array);

		//For each name in the array add to local map.
		for (i=0; i<countnames; i++)
		{
			dsb_string_ntoc(buf,MAX_NAME_SIZE,&array[i]);
			printf("Added name: %s\n",buf);
			dsb_names_update(buf,&array[i]);
		}
	}
	//There is no names object, so make it
	else
	{
		dsb_new(&PRoot,&namesobj);
		dsb_set(&PRoot,&Names,&namesobj);
		dsb_setnni(&namesobj,&Size,0);
	}

	return 0;
}

int dsb_names_update(const char *name, const NID_t *nid)
{
	struct NameEntry *entry;
	int hash = namehash(name);

#if defined(UNIX) && !defined(NO_THREADS)
	pthread_rwlock_wrlock(&nametable_mtx);
#endif

	entry = nametable[hash];

	while (entry != 0)
	{
		if (strcmp(entry->name,name) == 0)
		{
			entry->nid = *nid;

#if defined(UNIX) && !defined(NO_THREADS)
			pthread_rwlock_unlock(&nametable_mtx);
#endif
			return 0;
		}
		entry = entry->next;
	}

#if defined(UNIX) && !defined(NO_THREADS)
	pthread_rwlock_unlock(&nametable_mtx);
#endif

	//Not found so add
	return dsb_names_add(name,nid);
}

const NID_t *dsb_names_llookup(const char *name)
{
	struct NameEntry *entry;
	int hash = namehash(name);

#if defined(UNIX) && !defined(NO_THREADS)
	pthread_rwlock_rdlock(&nametable_mtx);
#endif
	entry = nametable[hash];

	while (entry != 0)
	{
		if (strcmp(entry->name,name) == 0)
		{
#if defined(UNIX) && !defined(NO_THREADS)
			pthread_rwlock_unlock(&nametable_mtx);
#endif
			return &entry->nid;
		}
		entry = entry->next;
	}

#if defined(UNIX) && !defined(NO_THREADS)
	pthread_rwlock_unlock(&nametable_mtx);
#endif

	return 0;
}

const NID_t *dsb_names_plookup(const char *name)
{
	const NID_t *res = dsb_names_llookup(name);

	printf("Lookup nf: %s (%d)\n",name,countnames);

	if (res == 0)
	{
		NID_t tmp;

		//Opps, so make it.
		dsb_new(&PRoot,&tmp);
		//Add to names array structure.
		//TODO: this is not safe across a network!!
		dsb_setnin(&namesobj,countnames++,&tmp);
		dsb_setnni(&namesobj,&Size,countnames);
		//Put string into it.
		dsb_string_cton(&tmp,name);
		dsb_names_add(name,&tmp);

		//Name is now there so find it.
		return dsb_names_llookup(name);
	}
	else
	{
		return res;
	}
}

int dsb_names_lookup(const char *name, NID_t *nid)
{
	*nid = *(dsb_names_plookup(name));
	return 0;
}

int dsb_names_revlookup(const NID_t *nid, char *name, int max)
{
	struct NameEntry *entry;
	int hash = namehash(name);

#if defined(UNIX) && !defined(NO_THREADS)
	pthread_rwlock_rdlock(&nametable_mtx);
#endif
	entry = nametable[hash];

	while (entry != 0)
	{
		if (dsb_nid_eq(&entry->nid,nid) == 1)
		{
			strcpy(name,entry->name);

#if defined(UNIX) && !defined(NO_THREADS)
			pthread_rwlock_unlock(&nametable_mtx);
#endif
			return 0;
		}
		entry = entry->next;
	}

#if defined(UNIX) && !defined(NO_THREADS)
	pthread_rwlock_unlock(&nametable_mtx);
#endif
	return ERR_NONAME;
}
