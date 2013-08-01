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
#include "dsb/errors.h"
#include <stdio.h>
#include <stdarg.h>
#include <malloc.h>

//-----------------------------------------------------------------------------

#define MAX_AGENTS			1000
#define MAX_AGENT_PARAMS	8

//-----------------------------------------------------------------------------

struct AgentEntry
{
	int active;
	VMCONTEXT_t ctx;
	NID_t script;
};

static struct AgentEntry *agents;

//-----------------------------------------------------------------------------

int dsb_agent_init()
{
	if (agents != 0)
	{
		return DSB_ERROR(ERR_AGENTINIT,0);
	}
	agents = malloc(sizeof(struct AgentEntry) * MAX_AGENTS);
	return 0;
}

int dsb_agent_final()
{
	if (agents)
	{
		free(agents);
		agents = 0;
	}
	return 0;
}

int dsb_agent_start(const NID_t *agent, int pn, ...)
{
	int handle;

	if (agents == 0 || pn > MAX_AGENT_PARAMS)
	{
		return -1;
	}

	//Find a free agent
	for (handle=0; handle < MAX_AGENTS; ++handle)
	{
		if (agents[handle].active == 0)
		{
			va_list args;
			int i;

			agents[handle].active = 1;
			agents[handle].script = *agent;

			va_start(args,pn);

			//Generate a valid context for this agent
			dsb_vm_context(&agents[handle].ctx, agent);
			agents[handle].ctx.result = 0;

			//Populate parameters.
			for (i=0; i<pn; ++i)
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

int dsb_agent_stop(int handle)
{
	if (handle < 0 || handle >= MAX_AGENTS)
	{
		return DSB_ERROR(ERR_AGENTID,0);
	}
	if (agents == 0)
	{
		return DSB_ERROR(ERR_AGENTINIT,0);
	}

	agents[handle].active = 0;
	return 0;
}

int dsb_agent_trigger(unsigned int id)
{
	if (id >= MAX_AGENTS)
	{
		return DSB_ERROR(ERR_AGENTID,0);
	}
	if (agents == 0)
	{
		return DSB_ERROR(ERR_AGENTINIT,0);
	}

	if (agents[id].active == 1)
	{
		//If already running then don't trigger again.
		if (agents[id].ctx.ip != 0)
		{
			return 0;
		}

		//Run the interpreter.
		agents[id].ctx.timeout = 10000;
		dsb_vm_interpret(&agents[id].ctx);
	}
	return 0;
}

