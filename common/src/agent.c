/*
 * agent.c
 *
 *  Created on: 25 Jun 2013
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

#include "dsb/core/agent.h"
#include "dsb/core/vm.h"
#include "dsb/core/nid.h"
#include <stdio.h>
#include <stdarg.h>
#include <malloc.h>

#define MAX_AGENTS	1000

struct AgentEntry
{
	int active;
	struct VMContext ctx;
	NID_t script;
	void (*cfunc)(void *);
	void *data;
};

static struct AgentEntry agents[MAX_AGENTS];

int dsb_agent_start(const NID_t *agent, int pn, ...)
{
	int handle = 0;
	for (handle=0; handle < MAX_AGENTS; handle++)
	{
		if (agents[handle].active == 0)
		{
			va_list args;
			int i;

			agents[handle].active = 1;
			agents[handle].script = *agent;
			agents[handle].cfunc = 0;

			va_start(args,pn);

			//Generate a valid context for this agent
			dsb_vm_context(&agents[handle].ctx, agent);
			agents[handle].ctx.result = 0;

			//Populate parameters.
			for (i=0; i<pn; i++)
			{
				agents[handle].ctx.vars[i] = *va_arg(args,NID_t*);
			}

			//Run the interpreter.
			dsb_vm_interpret(&agents[handle].ctx);

			//free(agents[handle].ctx.code);

			return handle;
		}
	}

	return -1;
}

int dsb_agent_startx(void (*func)(void*),void *data)
{
	int handle = 0;
	for (handle=0; handle < MAX_AGENTS; handle++)
	{
		if (agents[handle].active == 0)
		{
			agents[handle].cfunc = func;
			agents[handle].data = data;
			agents[handle].active = 1;

			//Initial call.
			agents[handle].cfunc(data);

			return handle;
		}
	}

	return -1;
}

int dsb_agent_trigger(unsigned int id)
{
	if (agents[id].active == 1)
	{
		if (agents[id].cfunc == 0)
		{
			//Reset script to start.
			agents[id].ctx.timeout = 10000;
			agents[id].ctx.ip = 0;

			//Run the interpreter.
			dsb_vm_interpret(&agents[id].ctx);
		}
		else
		{
			agents[id].cfunc(agents[id].data);
		}
	}
	return 0;
}

