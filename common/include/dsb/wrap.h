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

#ifdef __cplusplus
extern "C"
{
#endif

struct NID;
typedef struct NID NID_t;

/**
 * @addtogroup API
 * @{
 */

/**
 * Get the result of following a hyperarc. This function blocks until the
 * result is obtained.
 * @see dsb_getA
 * @param[in] d1 First tail node.
 * @param[in] d2 Second tail node.
 * @param[out] r NID to be filled with result.
 * @return SUCCESS or some other error code.
 */
int dsb_get(const NID_t *d1, const NID_t *d2, NID_t *r);

int dsb_getzzi(const char *d1, const char *d2, int *r);
int dsb_getzii(const char *d1, int d2, int *r);
int dsb_getnii(const NID_t *d1, int d2, int *r);
int dsb_getnin(const NID_t *d1, int d2, NID_t *r);
int dsb_getnni(const NID_t *d1, const NID_t *d2, int *r);
int dsb_getnzn(const NID_t *d1, const char *d2, NID_t *r);
int dsb_getnzi(const NID_t *d1, const char *d2, int *r);

int dsb_getA(const NID_t *d1, const NID_t *d2, NID_t *r);

int dsb_dict(const NID_t *d, const NID_t *n);
int dsb_dictnz(const NID_t *d, const char *n);
int dsb_dictzz(const char *d, const char *n);

/**
 * Set the node a hyperarc points to. This function is synchronous and
 * does guarentee that, on the relevant machine, the set completes before
 * any reads occur. This function does not modify an "objects" dictionary.
 * @param d1 First tail node.
 * @param d2 Second tail node.
 * @param v Node to change the head to.
 * @return SUCCESS or some other error code.
 */
int dsb_set(const NID_t *d1, const NID_t *d2, const NID_t *v);

int dsb_setnzz(const NID_t *d1, const char *d2, const char *v);
int dsb_setzzz(const char *d1, const char *d2, const char *v);
int dsb_setnni(const NID_t *d1, const NID_t *d2, int v);
int dsb_setnin(const NID_t *d1, int d2, const NID_t *v);
int dsb_setnii(const NID_t *d1, int d2, int v);
int dsb_setnzn(const NID_t *d1, const char *d2, const NID_t *v);


int dsb_define(
		const NID_t *d1,
		const NID_t *d2,
		const NID_t *def
		);

int dsb_defineM(
		const NID_t *d1a,
		const NID_t *d1b,
		const NID_t *d2a,
		const NID_t *d2b,
		const NID_t *def
		);

int dsb_getdef(
		const NID_t *d1,
		const NID_t *d2,
		NID_t *def
		);

int dsb_notify(const NID_t *d1, const NID_t *d2);
int dsb_dependency(
		const NID_t *d1,
		const NID_t *d2,
		const NID_t *dep1,
		const NID_t *dep2
		);

int dsb_new(const NID_t *base, NID_t *n);

int dsb_getmeta(const NID_t *d1, const NID_t *d2, NID_t *meta);
int dsb_setmeta(const NID_t *d1, const NID_t *d2, const NID_t *meta);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* WRAP_H_ */
