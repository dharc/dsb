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
	NID_USER=1000
};

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

int dsb_nid_init();
int dsb_nid_final();
int dsb_nid_allocate(struct NID *nid);
int dsb_nid_free(const struct NID *nid);
int dsb_nid_compare(const struct NID *a, const struct NID *b);
int dsb_nid_toStr(const struct NID *nid, char *str, int len);
int dsb_nid_fromStr(const char *str, struct NID *nid);

#endif //_NID_H_
