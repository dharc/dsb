/*
 * dsb.c
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

#include "dsb/common.h"
#include <sys/time.h>

int dsb_common_init()
{
	int ret;
	ret = dsb_nid_init();
	if (ret != SUCCESS) return ret;
	ret = dsb_event_init();
	if (ret != SUCCESS) return ret;
	ret = dsb_net_init();
	if (ret != SUCCESS) return ret;
	ret = dsb_module_init();
	if (ret != SUCCESS) return ret;
	ret = dsb_names_init();
	if (ret != SUCCESS) return ret;
	ret = dsb_xfunc_init();
	return ret;
}

int dsb_common_final()
{
	int ret;
	ret = dsb_xfunc_final();
	if (ret != SUCCESS) return ret;
	ret = dsb_names_final();
	if (ret != SUCCESS) return ret;
	ret = dsb_module_final();
	if (ret != SUCCESS) return ret;
	ret = dsb_net_final();
	if (ret != SUCCESS) return ret;
	ret = dsb_event_final();
	if (ret != SUCCESS) return ret;
	ret = dsb_nid_final();
	return ret;
}

/*
 * Get accurate clock time.
 */
long long dsb_getTicks()
{
	#ifdef UNIX
	unsigned long long ticks;
	struct timeval now;
	gettimeofday(&now, NULL);
	ticks = ((unsigned long long)now.tv_sec) * (unsigned long long)1000000 + ((unsigned long long)now.tv_usec);
	return ticks;
	#endif

	#ifdef WIN32
	LARGE_INTEGER tks;
	QueryPerformanceCounter(&tks);
	return (((unsigned long long)tks.HighPart << 32) + (unsigned long long)tks.LowPart);
	#endif
}

