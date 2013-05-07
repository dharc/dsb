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
#ifndef _NID_H_
#define _NID_H_

enum NIDType
{
	NID_SPECIAL=0,
	NID_INTEGER,
	NID_REAL,
	NID_CHARACTER,
	NID_LABEL,
	//---- Integer Operations -----
	NID_INTADD,
	NID_INTSUB,
	NID_INTDIV,
	NID_INTMUL,
	NID_INTBITAND,
	NID_INTBITNOT,
	NID_INTBITOR,
	NID_INTSHIFTL,
	NID_INTSHIFTR,

	NID_USER=1000
};

/**
 * Node IDentifier. A 96bit unique ID for Nodes in the DSB structure.
 */
struct NID
{
	enum NIDType type;

	union
	{
		struct
		{
			unsigned int a;
			unsigned int b;
		};
		unsigned long long ll;
		double dbl;
		unsigned short chr;
	};
};

/**
 * Initialise the NID allocation system. Must be called before first NID is
 * allocated.
 * @return 0 on success.
 */
int dsb_nid_init();

/**
 * Should be called before exit. Currently does nothing.
 * @return 0
 */
int dsb_nid_final();

/**
 * Compare two NIDs. If both are equal then 0 is returned, otherwise if a < b
 * then a negative number is returned, otherwise a positive number.
 * @param a First NID
 * @param b Second NID
 * @return 0 for equality, negative if a<b, positive if a>b.
 */
int dsb_nid_compare(const struct NID *a, const struct NID *b);

/**
 * Convert a NID to a string.
 * @param nid NID to convert.
 * @param str Buffer to put the string into.
 * @param len Max length of string.
 * @return 0 on success.
 */
int dsb_nid_toStr(const struct NID *nid, char *str, int len);

/**
 * Generate a NID from a string.
 * @param str Input string.
 * @param nid NID structure to fill with result.
 * @return 0 on success.
 */
int dsb_nid_fromStr(const char *str, struct NID *nid);

void dsb_iton(int,struct NID*);
int dsb_ntoi(const struct NID*);

#endif //_NID_H_
