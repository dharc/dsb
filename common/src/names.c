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

int dsb_names_init()
{
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

	//TODO make thread safe.
	entry->next = nametable[hash];
	nametable[hash] = entry;

	return SUCCESS;
}

int dsb_names_rebuild()
{
	dsb_names_update("root",&Root);
	dsb_names_update("proot",&PRoot);

	//Need to check for names object and rebuild index...

	return 0;
}

int dsb_names_update(const char *name, const NID_t *nid)
{
	struct NameEntry *entry;
	int hash = namehash(name);

	entry = nametable[hash];

	while (entry != 0)
	{
		if (strcmp(entry->name,name) == 0)
		{
			entry->nid = *nid;
			return SUCCESS;
		}
		entry = entry->next;
	}

	return dsb_names_add(name,nid);
}

int dsb_names_lookup(const char *name, NID_t *nid)
{
	struct NameEntry *entry;
	int hash = namehash(name);

	entry = nametable[hash];

	while (entry != 0)
	{
		if (strcmp(entry->name,name) == 0)
		{
			*nid = entry->nid;
			return SUCCESS;
		}
		entry = entry->next;
	}

	//Opps, so make it.
	dsb_new(&PRoot,nid);
	dsb_string_cton(nid,name);
	return dsb_names_add(name,nid);
}

int dsb_names_revlookup(const NID_t *nid, char *name, int max)
{
	struct NameEntry *entry;
	int hash = namehash(name);

	entry = nametable[hash];

	while (entry != 0)
	{
		if (dsb_nid_eq(&entry->nid,nid) == 1)
		{
			strcpy(name,entry->name);
			return SUCCESS;
		}
		entry = entry->next;
	}

	return ERR_NONAME;
}
