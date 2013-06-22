/*
 * module.c
 *
 *  Created on: 7 May 2013
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
#include "dsb/errors.h"
#include "string.h"
#include <stdio.h>
#include <malloc.h>
#include "dsb/config.h"

#ifdef UNIX
#include <dlfcn.h>
#include <sys/time.h>
#endif

#define MAX_MODULE_NAME			50
#define MAX_INTERNAL_MODULES	20
#define MAX_LOADED_MODULES		30

/*
 * Internal module registration data.
 */
struct ModInternal
{
	char name[MAX_MODULE_NAME];
	struct Module mod;
	void *handle;
	long long nextupdate;
};

static struct ModInternal imods[MAX_INTERNAL_MODULES];
static struct ModInternal *lmods[MAX_LOADED_MODULES] = {0};

unsigned int modix = 0;

int dsb_module_init()
{
	return SUCCESS;
}

int dsb_module_final()
{
	//Unload all modules
	int i;
	for (i=0; i<MAX_LOADED_MODULES; i++)
	{
		if (lmods[i] != 0)
		{
			if (lmods[i]->mod.final != 0)
			{
				lmods[i]->mod.final();
			}
		}
	}

	return SUCCESS;
}

int dsb_module_register(const char *name, const struct Module *mod)
{
	//Check for problems
	if (dsb_module_exists(name) == SUCCESS)
		return DSB_ERROR(ERR_MODEXISTS,name);
	if (strlen(name) >= MAX_MODULE_NAME)
		return DSB_ERROR(ERR_MODNAME,name);
	if ((mod->init == 0) || (mod->final == 0))
		return DSB_ERROR(ERR_INVALIDMOD,name);

	strcpy(imods[modix].name,name);
	imods[modix].mod = *mod;
	modix++;

	return SUCCESS;
}

int dsb_module_exists(const char *name)
{
	int i;

	//First scan internal registered modules.
	for (i=0; i<modix; i++)
	{
		//Do we have a name match
		if (strcmp(name,imods[i].name) == 0)
		{
			return SUCCESS;
		}
	}

	//Now scan for external modules...
	//TODO Scan for external modules.
	return ERR_NOMOD;
}

int dsb_module_load(const char *name, const struct NID *base)
{
	int i,j;
	void *handle;
	char fname[200];
	struct Module *(*init)(void);

	//First scan internal registered modules.
	for (i=0; i<modix; i++)
	{
		//Do we have a name match
		if (strcmp(name,imods[i].name) == 0)
		{
			for (j=0; j<MAX_LOADED_MODULES; j++)
			{
				if (lmods[j] == 0) break;
			}
			lmods[j] = &(imods[i]);
			lmods[j]->nextupdate = 0;
			if (imods[i].mod.init != 0)
			{
				//Initialise the module!
				return imods[i].mod.init(base);
			}
			else
			{
				return DSB_ERROR(ERR_INVALIDMOD,name);
			}
		}
	}

	//Now scan for external modules...
	//TODO Scan for external modules.
	#ifdef UNIX
	sprintf(fname,"lib%s.so",name);
	handle = dlopen(fname,RTLD_NOW);
	#endif

	if (handle == 0)
	{
		#ifdef UNIX
		return DSB_ERROR(ERR_NOMOD,dlerror());
		#endif
	}

	#ifdef UNIX
	init = dlsym(handle,"dsb_module_info");
	#endif

	if (init == 0)
	{
		return DSB_ERROR(ERR_INVALIDMOD,name);
	}

	for (j=0; j<MAX_LOADED_MODULES; j++)
	{
		if (lmods[j] == 0) break;
	}

	lmods[j] = malloc(sizeof(struct ModInternal));
	lmods[j]->handle = handle;
	lmods[j]->nextupdate = 0;
	strcpy(lmods[j]->name,name);
	lmods[j]->mod = *(init());
	if (lmods[j]->mod.init != 0)
	{
		//Initialise the module!
		return lmods[j]->mod.init(base);
	}
	else
	{
		return DSB_ERROR(ERR_INVALIDMOD,name);
	}

	return DSB_ERROR(ERR_NOMOD,name);;
}

int dsb_module_unload(const char *name)
{
	int i;

		//First scan internal registered modules.
		for (i=0; i<modix; i++)
		{
			//Do we have a name match
			if (strcmp(name,imods[i].name) == 0)
			{
				if (imods[i].mod.final != 0)
				{
					//Initialise the module!
					return imods[i].mod.final();
				}
				else
				{
					return DSB_ERROR(ERR_INVALIDMOD,name);
				}
			}
		}

		//Now scan for external modules...
		//TODO Scan for external modules.
		return DSB_ERROR(ERR_NOMOD,name);
}

static long long getTicks()
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

int dsb_module_updateall()
{
	int i;
	long long ticks = getTicks();
	for (i=0; i<MAX_LOADED_MODULES; i++)
	{
		if (lmods[i] != 0)
		{
			if (lmods[i]->mod.update != 0)
			{
				if (lmods[i]->nextupdate <= ticks)
				{
					if (lmods[i]->mod.ups > 0)
					{
						lmods[i]->nextupdate = ticks + (1000000 / lmods[i]->mod.ups);
					}
					else{
						lmods[i]->nextupdate = ticks;
					}
					lmods[i]->mod.update();
				}
			}
		}
		else
		{
			break;
		}
	}

	return SUCCESS;
}
