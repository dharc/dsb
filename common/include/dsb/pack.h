/*
 * pack.h
 *
 *  Created on: 1 Aug 2013
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

#ifndef PACK_H_
#define PACK_H_


// A = Buffer, B = Source, C = size
#define PACK(A,B,C) memcpy(A,B,C); A += C;
#define PACK_INT(A,B) PACK(A,B,sizeof(int))
#define PACK_SHORT(A,B) PACK(A,B,sizeof(short))
#define PACK_LL(A,B) PACK(A,B,sizeof(long long));
#define PACK_CHAR(A,B) *A = *B; A++;
#define PACK_NID(A,B) A += dsb_nid_pack(B,A,1000);

// A = Buffer, B = Destination, C = size
#define UNPACK(A,B,C) memcpy(B,A,C); A += C;
#define UNPACK_INT(A,B) UNPACK(A,B,sizeof(int))
#define UNPACK_SHORT(A,B) UNPACK(A,B,sizeof(short))
#define UNPACK_LL(A,B) UNPACK(A,B,sizeof(long long))
#define UNPACK_CHAR(A,B) *B = *A; A++;
#define UNPACK_NID(A,B) A += dsb_nid_unpack(A,B);

#endif /* PACK_H_ */
