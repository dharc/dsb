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
#include <stdlib.h>
#include <malloc.h>
#include "dsb/config.h"
#include "dsb/types.h"

#ifdef UNIX
#include <dlfcn.h>
#include <sys/time.h>
#include <unistd.h>
#endif

#ifdef WIN32
#include <windows.h>
#endif

//TODO: ideally don't have hard coded limits.
#define MAX_MODULE_NAME			50
#define MAX_INTERNAL_MODULES	20
#define MAX_LOADED_MODULES		30
#define MAX_SEARCH_PATHS		20

/*
 * Internal module registration data.
 */
struct ModInternal
{
	char name[MAX_MODULE_NAME];
	Module_t mod;
	void *handle;
	long long nextupdate;
};

static struct ModInternal imods[MAX_INTERNAL_MODULES];
static struct ModInternal *lmods[MAX_LOADED_MODULES];
static char *searchpaths[MAX_SEARCH_PATHS];

static unsigned int modix;

int dsb_module_init()
{
	const char *envpaths = getenv("DSB_MODULE_PATH");
	int length;
	const char *tmp;
	int ix = 0;

	if (envpaths != 0)
	{
		do
		{
			//Split on ':'
			tmp = strchr(envpaths,':');

			length = (tmp==0) ? strlen(envpaths) : tmp-envpaths;
			if (length == 0) break;

			searchpaths[ix] = malloc(length+1);
			strncpy(searchpaths[ix],envpaths,length);
			searchpaths[ix][length] = 0;

			envpaths = tmp+1;
			++ix;

			//Safety check
			if (ix >= (MAX_SEARCH_PATHS-4)) break;
		}
		while (tmp != 0);
	}

	searchpaths[ix++] = SHAREDIR;
	searchpaths[ix++] = "/usr/lib/dsb";
	searchpaths[ix] = 0;

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

int dsb_module_register(const char *name, const Module_t *mod)
{
	//Check for problems
	//if (dsb_module_exists(name) == SUCCESS)
	//	return DSB_ERROR(ERR_MODEXISTS,name);
	if (strlen(name) >= MAX_MODULE_NAME)
		return DSB_ERROR(ERR_MODNAME,name);
	if ((mod->init == 0) || (mod->final == 0))
		return DSB_ERROR(ERR_INVALIDMOD,name);

	strcpy(imods[modix].name,name);
	imods[modix].mod = *mod;
	modix++;

	return SUCCESS;
}

bool dsb_module_probe(const char *name, char *lib)
{
	char filename[300];
	int i = 0;

	//For every search path
	while (searchpaths[i])
	{
		//Generate filename and check if it can be read.
		sprintf(filename,"%s/lib%s.so",searchpaths[i],name);
		if (access(filename,R_OK) == 0)
		{
			//Save full path to "lib".
			strcpy(lib,filename);
			return true;
		}

		++i;
	}

	return false;
}

int dsb_module_load(const char *name, const struct NID *base)
{
	int i,j;
	void *handle;
	char fname[300];
	Module_t *(*init)(void);

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
				//TODO Load all required modules.

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
	if (dsb_module_probe(name,fname) == false)
	{
		return DSB_ERROR(ERR_NOMOD,"Not found");
	}

	//TODO Scan for external modules.
	#ifdef UNIX
	handle = dlopen(fname,RTLD_NOW);
	#else
	handle = LoadLibrary(fname);
	#endif

	if (handle == 0)
	{
		#ifdef UNIX
		return DSB_ERROR(ERR_NOMOD,dlerror());
		#endif
	}

	#ifdef UNIX
	init = dlsym(handle,"dsb_module_info");
	#else
	init = (void*)GetProcAddress(handle, "dsb_module_info");
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
