/*
 * array.h
 *
 *  Created on: 28 May 2013
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

#ifndef ARRAY_H_
#define ARRAY_H_

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct NID NID_t;

/**
 * Write a local C NID array into the DSB graph.
 * @param src A C array of NIDs.
 * @param size The size of the source array.
 * @param dest Destination node in the graph on which to build the array.
 * @return Number of elements added.
 */
int dsb_array_write(const NID_t *src, int size, const NID_t *dest);

/**
 * Read an array from the graph to a local C NID array.
 * @param src Graph node containing an array structure.
 * @param dest Local C array to fill.
 * @param max Maximum size of local C array.
 * @return Actual size of read array.
 */
int dsb_array_read(const NID_t *src, NID_t *dest, int max);

#ifdef __cplusplus
}
#endif

#endif /* ARRAY_H_ */
