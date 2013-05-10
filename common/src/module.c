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

#include "dsb/module.h"
#include "dsb/errors.h"
#include "string.h"

#define MAX_MODULE_NAME			50
#define MAX_INTERNAL_MODULES	20

struct ModInternal
{
	char name[MAX_MODULE_NAME];
	struct Module mod;
};

struct ModInternal imods[MAX_INTERNAL_MODULES];
unsigned int modix = 0;

int dsb_module_register(const char *name, const struct Module *mod)
{
	//Check for problems
	if (dsb_module_exists(name) == SUCCESS) return ERR_MODEXISTS;
	if (strlen(name) >= MAX_MODULE_NAME) return ERR_MODNAME;
	if ((mod->init == 0) || (mod->final == 0)) return ERR_INVALIDMOD;

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
	int i;

	//First scan internal registered modules.
	for (i=0; i<modix; i++)
	{
		//Do we have a name match
		if (strcmp(name,imods[i].name) == 0)
		{
			if (imods[i].mod.init != 0)
			{
				//Initialise the module!
				return imods[i].mod.init(base);
			}
			else
			{
				return ERR_INVALIDMOD;
			}
		}
	}

	//Now scan for external modules...
	//TODO Scan for external modules.
	return ERR_NOMOD;
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
					return ERR_INVALIDMOD;
				}
			}
		}

		//Now scan for external modules...
		//TODO Scan for external modules.
		return ERR_NOMOD;
}
