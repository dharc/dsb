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

struct NID;

/**
 * @addtogroup Modules
 * Search, load and register modules. Modules can provide new handlers, agents,
 * evaluators or other functionality to be incorporated with DSB.
 * @{
 */

/**
 * Module registration structure that each module must provide.
 */
struct Module
{
	int (*init)(const struct NID *);
	int (*update)();
	int (*final)();
};

/**
 * Used to register internal compiled modules. Will return an error if the
 * module structure is missing required parts or if the module has already
 * been registered. Not to be used for external modules.
 * @param name The name of the internal module.
 * @param Structure containing module init, update and final functions.
 * @return SUCCESS, ERR_MODEXISTS, ERR_INVALIDMOD.
 */
int dsb_module_register(const char *name, const struct Module *);

/**
 * Search for and then load a named module. It will search first for internal
 * registered modules and then the file system for the module library (.so)
 * file.
 * @param name Name of the module to load.
 * @param base An option base node to module configuration.
 * @return SUCCESS, ERR_NOMOD, ERR_INVALIDMOD.
 */
int dsb_module_load(const char *name, const struct NID *base);

int dsb_module_unload(const char *name);

/**
 * Search for a named module and validate but do not load it.
 * @param name Name of module.
 * @return SUCCESS means module was found, ERR_NOMOD means it wasn't.
 */
int dsb_module_exists(const char *name);

int dsb_module_updateall();

int dsb_module_init();

int dsb_module_final();

/** @} */

#endif /* MODULE_H_ */
