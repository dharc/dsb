/*
 * vm-perf.c
 *
 *  Created on: 22 Jun 2013
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

#include "dsb/core/nid.h"
#include "dsb/core/event.h"
#include "dsb/core/vm.h"

#include <unistd.h>
#include <sys/time.h>

NID_t code[100];

int dsb_send(struct Event *evt)
{
	return 0;
}

int generate_benchmark1()
{
	int ip = 0;
	int loop;

	dsb_nid_op(VM_CPY(1,0),&code[ip++]);	//cpy $count 100000
	dsb_iton(10000,&code[ip++]);
	dsb_nid_op(VM_CPY(2,0),&code[ip++]);	//cpy $i 0
	dsb_iton(0,&code[ip++]);
	dsb_nid_op(VM_CPY(3,0),&code[ip++]);	//cpy $sum 0
	dsb_iton(0,&code[ip++]);
	loop = ip;
	dsb_nid_op(VM_INC(2),&code[ip++]);		//inc $i
	dsb_nid_op(VM_ADD(3,3,2),&code[ip++]);	//add $sum $sum $i
	dsb_nid_op(VM_JLT(loop,2,1),&code[ip++]);	//jlt :loop $i $count
	dsb_nid_op(VM_RET(3),&code[ip++]);		//ret $sum
	return ip;
}

int generate_benchmark2()
{
	int ip = 0;

	//dsb_nid_op(VM_CPY(1,0),&code[ip++]);	//cpy $count 100000
	//code[ip].header = 0;
	//code[ip].t = 1;
	//code[ip++].ll = 0x4546474849;
	dsb_nid_op(VM_JLT(1,2,3),&code[ip++]);
	dsb_nid_op(VM_RET(1),&code[ip++]);
	//dsb_nid_op(0,&code[ip++]);
	return ip;
}

static long long getTicks()
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


int main(int argc, char *argv[])
{
	HARC_t harc;
	long long ticks;
	double secs;
	int i;
	int (*output)(void *);
	struct VMContext ctx;
	ctx.code = code;
	ctx.codesize = 100;
	ctx.result = &harc.h;

	dsb_nid_init();
	dsb_event_init();

	ctx.codesize = generate_benchmark1();

	ticks = getTicks();

	for (i=0; i<10000; i++)
	{
		if (dsb_vm_interpret(&ctx) != -1)
		{
			printf("Error\n");
		}
	}

	ticks = getTicks() - ticks;

	secs = (double)ticks / 1000000.0;
	printf("Benchmark 1: %fs or %.2f kop/s\n",(float)secs,30004000.0 / (float)secs / 100000.0);

	//dsb_vm_interpret(code,100, 0, &harc.h);
	printf("Interp Res = %llu\n",harc.h.ll);

	//size = generate_benchmark2();
	//Now compile the code
	dsb_vm_arch_compile(code,ctx.codesize,(void**)&output);

	ticks = getTicks();

	for (i=0; i<10000; i++)
	{
		//Now run the code!!!!!!!!!!
		ctx.codesize = output(&ctx.vars);
	}

	ticks = getTicks() - ticks;

	secs = (double)ticks / 1000000.0;
	printf("Benchmark 1 JIT: %fs or %.2f kop/s\n",(float)secs,30004000.0 / (float)secs / 100000.0);

	printf("JIT Result = %llu\n",ctx.vars[ctx.codesize].ll);

	dsb_event_final();
	dsb_nid_final();
	return 0;
}


