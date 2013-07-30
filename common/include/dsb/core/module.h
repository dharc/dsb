/*
 * module.h
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

/** @file module.h */

#ifndef MODULE_H_
#define MODULE_H_

#include "dsb/types.h"


#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @addtogroup Modules
 * Search, load and register modules. Modules can provide new handlers, agents,
 * evaluators or other functionality to be incorporated with DSB.
 * @{
 */

/**
 * Module registration structure that each module must provide.
 */
typedef struct
{
	int (*init)(const NID_t *);
	int (*update)();
	int (*final)();
	int ups;					///< Updates per second.
	const char **depends;		///< Null terminated array of required modules.
	bool daemon;				///< Only load module in full daemon.
} Module_t;

/**
 * Used to register internal compiled modules. Will return an error if the
 * module structure is missing required parts or if the module has already
 * been registered. Not to be used for external modules.
 * @param name The name of the internal module.
 * @param Structure containing module init, update and final functions.
 * @return SUCCESS, ERR_MODEXISTS, ERR_INVALIDMOD.
 */
int dsb_module_register(const char *name, const Module_t *);

/**
 * Search for and then load a named module. It will search first for internal
 * registered modules and then the file system for the module library (.so)
 * file.
 * @see dsb_module_probe
 * @param name Name of the module to load.
 * @param base An option base node to module configuration.
 * @return SUCCESS, ERR_NOMOD, ERR_INVALIDMOD.
 */
int dsb_module_load(const char *name, const NID_t *base);

int dsb_module_unload(const char *name);

/**
 * Find the .so or .dll library based upon module name.
 * @param name Module name
 * @param lib Filled with lib name if found.
 * @return true if found, false if not.
 */
bool dsb_module_probe(const char *name, char *lib);

int dsb_module_updateall();

int dsb_module_init();

int dsb_module_final();

#ifdef __cplusplus
}
#endif

/** @} */

#endif /* MODULE_H_ */
