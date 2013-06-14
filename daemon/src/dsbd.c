/*
 * dsbd.c
 *
 *  Created on: 7 May 2013
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

#include "dsb/module.h"
#include "dsb/errors.h"
#include "dsb/processor.h"
#include "dsb/router.h"
#include "dsb/names.h"
#include "dsb/common.h"
#include "dsb/evaluator.h"
#include "dsb/globals.h"
#include "dsb/net_protocol.h"
#include "dsb/config.h"
#include <stdio.h>
#include <signal.h>

void sigint(int s)
{
	dsb_proc_stop();
}

void print_help()
{
	printf("Usage: dsbd [options]\n");
	printf("  -l <name>     Load a named module.\n");
	printf("  -h            Print this help message.\n");
	printf("  -v            Display dsbd version information\n");
}

void print_version()
{
	printf("dsbd - Version: %d.%d.%d (%s, %s)\n",
			VERSION_MAJOR,
			VERSION_MINOR,
			VERSION_PATCH,
			TARGET_NAME,
			TARGET_PROCESSOR);
}

int process_args(int argc, char *argv[])
{
	int i;

	for (i=0; i<argc; i++)
	{
		if (argv[i][0] == '-')
		{
			switch (argv[i][1])
			{
			case 'l':
				dsb_module_load(argv[++i],&PRoot);
				break;

			case 'h':
				print_help();
				break;

			case 'v':
				print_version();
				break;

			case 'd':
				dsb_debug(DBG_EVENTS | DBG_VOLATILE);
				break;

			default:
				printf("Invalid Option\n");
				print_help();
				break;
			}
		}
	}

	return 0;
}

void make_bool()
{
	//AND
	dsb_setzzz("true","and","trueand");
	dsb_dictzz("true","and");
	dsb_setzzz("trueand","true","true");
	dsb_dictzz("trueand","true");
	dsb_setzzz("trueand","false","false");
	dsb_dictzz("trueand","false");
	dsb_setzzz("false","and","falseand");
	dsb_dictzz("false","and");
	dsb_setzzz("falseand","true","false");
	dsb_dictzz("falseand","true");
	dsb_setzzz("falseand","false","false");
	dsb_dictzz("falseand","false");
}

void make_system()
{
	NID_t sys;
	dsb_new(&Root,&sys);
	dsb_setnzn(&Root,"system",&sys);
	dsb_dictnz(&Root,"system");

	//Rebuild root graph
	dsb_setnzn(&Root,"persistent",&PRoot);
	dsb_dictnz(&Root,"persistent");

#ifdef UNIX
	dsb_setnzz(&sys,"os","linux");
#endif
	dsb_dictnz(&sys,"os");

#ifdef _DEBUG
	dsb_setnzn(&sys,"debug",&True);
#else
	dsb_setnzn(&sys,"debug",&False);
#endif
	dsb_dictnz(&sys,"debug");
}

//Internally compiled modules.
extern struct Module *dsb_volatile_module();
extern struct Module *dsb_persistent_module();
extern struct Module *dsb_evaluators_module();
extern struct Module *dsb_network_module();

int main(int argc, char *argv[])
{
	int ret;

	//Initialise the common parts
	dsb_common_init();
	dsb_eval_init();
	//dsb_proc_init();
	dsb_route_init();
	dsb_proc_init();

	//Register the internal modules
	dsb_module_register("volatile",dsb_volatile_module());
	dsb_module_register("persistent",dsb_persistent_module());
	dsb_module_register("evaluators",dsb_evaluators_module());
	dsb_module_register("net",dsb_network_module());

	//These must exist or all else fails completely.
	dsb_module_load("volatile",&Root);
	dsb_module_load("persistent",&PRoot);

	//Effectively required so will load
	dsb_module_load("evaluators",&Root);
	dsb_module_load("net",&Root);

	//Make sure names map is up to date.
	dsb_names_rebuild();

	//Ready to process command line args.
	ret = process_args(argc,argv);
	if (ret != 0) return ret;

	//Build volatile system graph
	make_system();
	//Build boolean structure.
	make_bool();

	//Set signal handler
	signal(SIGINT, sigint);

	//Need to call all module update code.
	//Need to process queues until empty.
	dsb_proc_run(10);

	printf("Terminating...\n");

	dsb_proc_final();
	dsb_eval_final();
	dsb_common_final();

	return 0;
}

