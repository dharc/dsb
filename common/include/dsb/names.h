/*
 * names.h
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

#ifndef NAMES_H_
#define NAMES_H_

typedef struct NID NID_t;

#ifdef __cplusplus
extern "C"
{
#endif

int dsb_names_init();
int dsb_names_final();

int dsb_names_rebuild();

#define DSB_NAME(A) static const NID_t *A
/// Define a NID variable to cache a name lookup.
#define DSB_INIT(A,B) A = dsb_names_plookup(#B)

const NID_t *dsb_names_plookup(const char *name);

/**
 * Add a special hard coded label. This does not add to the persistent
 * hypergraph, instead use dsb_names_lookup.
 * @see dsb_names_lookup
 * @param name
 * @param nid
 * @return SUCCESS
 */
int dsb_names_add(const char *name, const NID_t *nid);

int dsb_names_update(const char *name, const NID_t *nid);

/**
 * Find the NID associated with a name or create a new NID for it.
 * @param name Name to look for.
 * @param nid Structure to be filled with the result.
 * @return SUCCESS.
 */
int dsb_names_lookup(const char *name, NID_t *nid);

int dsb_names_revlookup(const NID_t *nid, char *name, int max);

#ifdef __cplusplus
}
#endif

#endif /* NAMES_H_ */
