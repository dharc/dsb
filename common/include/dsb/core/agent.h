/*
 * agent.h
 *
 *  Created on: 24 Jun 2013
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

#ifndef AGENT_H_
#define AGENT_H_

#include "dsb/types.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Initialises agent table. Must be called before any other dsb_agent_*
 * functions, which will fail with ERR_AGENTINIT if not.
 * @see dsb_agent_final
 * @return 0
 */
int dsb_agent_init();

/**
 * Cleanup agent table. Should be called on exit. Calls to any
 * dsb_agent_* functions after this will fail with ERR_AGENTINIT.
 * @return 0
 */
int dsb_agent_final();

/**
 * Add a script as an agent. This will cause the script to be executed
 * immediately and then whenever any of its dependencies change.
 * Errors can be if dsb_agent_init has not been called, if there are no
 * free agent slots in the agent table or if the number of parameters
 * passed is too large ( > 8 ). All extra parameters are NID_t pointers.
 *
 * @param agent The DSB object containing the VM code.
 * @param pn Number of parameters.
 * @param ... Variable number of NID_t* parameters (pn).
 * @return Handle to the agent or -1 on error.
 */
int dsb_agent_start(const NID_t *agent, int pn, ...);

/**
 * Terminate the agent. Will no longer respond to triggers.
 * @param handle Handle as returned by dsb_agent_start.
 * @return 0 or ERR_AGENTINT or ERR_AGENTID.
 */
int dsb_agent_stop(int handle);

/**
 * Cause an agent to be called. Called whenever a notify event is
 * received destined for this agent.
 * @param id Agent handle.
 * @return 0 or ERR_AGENTINIT, ERR_AGENTID.
 */
int dsb_agent_trigger(unsigned int id);

#ifdef __cplusplus
}
#endif

#endif /* AGENT_H_ */
