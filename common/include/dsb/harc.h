/* 
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
#ifndef _HARC_H_
#define _HARC_H_

struct NID;

/**
 * The tail of a hyperarc can be used to identify that hyperarc. A tail is
 * generated from two Node identifiers.
 */
struct HARC
{
	union {
	unsigned long long a;
	struct {
		unsigned int a1;
		unsigned int a2;
	};
	};
	unsigned long long b;
	unsigned long long c;
};

/**
 * Generate a hyperarc tail structure from two nodes. The order of the two
 * tail nodes is not significant.
 * @param First tail node.
 * @param Second tail node.
 * @param HARC structure to populate.
 * @return SUCCESS.
 */
int dsb_harc_gen(const struct NID *, const struct NID *, struct HARC *);

/**
 * Compare two hyperarcs for equality. If the first is less than the second
 * -1 is returned. If they are equal 0 is returned and if the first is greater
 * then 1 is returned.
 * @param First hyperarc tail.
 * @param Second hyperarc tail.
 * @return Result of comparison: -1, 0 or 1.
 */
int dsb_harc_compare(const struct HARC *, const struct HARC *);

#endif
