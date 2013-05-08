/*
 * wrap.h
 *
 *  Created on: 30 Apr 2013
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

/*
 * This file contains API wrapper functions to help generate events to
 * manipulate the hypergraph.
 */

#ifndef WRAP_H_
#define WRAP_H_

struct NID;

/**
 * Get the result of following a hyperarc. This function blocks until the
 * result is obtained.
 * @see dsb_getA
 * @param[in] d1 First tail node.
 * @param[in] d2 Second tail node.
 * @param[out] r NID to be filled with result.
 * @return SUCCESS or some other error code.
 */
int dsb_get(const struct NID *d1, const struct NID *d2, struct NID *r);


int dsb_set(const struct NID *d1, const struct NID *d2, const struct NID *v);
int dsb_define(
			const struct NID *d1,
			const struct NID *d2,
			const struct NID *def,
			int eval
			);
int dsb_new(struct NID *n);
int dsb_delete(struct NID *n);

#endif /* WRAP_H_ */
