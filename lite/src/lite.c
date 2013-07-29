/*
 * lite.c
 *
 *  Created on: 11 May 2013
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
#include "dsb/errors.h"
#include "dsb/core/event.h"
#include "dsb/types.h"
#include "dsb/net.h"
#include "dsb/net_protocol.h"
#include "dsb/globals.h"
#include "dsb/core/agent.h"
#include <signal.h>
#include <string.h>

void *hostsock;
static bool running;
static bool interactive;

void dsb_lite_startcli();

/*
 * Oops.
 */
void sigint(int s)
{
	running = false;
}

/*
 * Send events to network.
 */
int dsb_send(Event_t * evt, bool async)
{
	int res;
	char buf[200];

	//Record the event.
	dsb_event_pack(evt,buf,100);
	res = dsb_net_send_event(hostsock, evt, async);

	if (((evt->type >> 8) != 0x1) && ((evt->flags & EVTFLAG_FREE) != 0))
	{
		dsb_event_free(evt);
	}

	return res;
}

static void log_handler(int msg, const char *str)
{
	printf("INFO: %d: %s\n",msg,str);
}

/*
 * Error notification from the server, print/handle the error message.
 */
static int net_cb_error(void *sock, void *data)
{
	int errnum;
	const char *data2;

	memcpy(&errnum,data,sizeof(int));
	data2 = (const char*)data + sizeof(int);

	log_handler(errnum,data2);

	return 0;
}

static int net_cb_base(void *sock, void *data)
{
	int count;

	//Update roots
	count = dsb_nid_unpack((const char*)data,&Root);
	dsb_nid_unpack(&(((const char*)data)[count]),&PRoot);

	//dsb_names_rebuild();
	//ide->messageLogger()->delayedNamesRebuild();

	return SUCCESS;
}

static int net_cb_debugevent(void *sock, void *data)
{

	return 0;
}

/*
 * Received an event from the network. Check whether we need to do
 * anything with it, such as trigger an agent. Otherwise discard.
 */
static int net_cb_event(void *sock, void *data)
{
	Event_t evt;
	dsb_event_unpack((const char*)data,&evt);

	//If event is for a local agent then trigger it.
	if (evt.d1.header == NID_AGENT)
	{
		if (dsb_nid_isLocal(&evt.d1) == 1)
		{
			dsb_agent_trigger((unsigned int)evt.d1.ll);
		}
	}

	return 0;
}

static void print_help()
{
	printf("Usage: dsb [options]\n");
	printf("  -c <host>     Connect to host.\n");
	printf("  -h            Print this help message.\n");
	printf("  -v            Display dsb version information\n");
	printf("  -i            Interactive mode.\n");
	printf("  -l <module>   Load module.\n");
}

static void print_version()
{
	printf("dsb - Version: %d.%d.%d (%s, %s)\n",
			VERSION_MAJOR,
			VERSION_MINOR,
			VERSION_PATCH,
			TARGET_NAME,
			TARGET_PROCESSOR);
}

/*
 * Loop over all command line arguments. See print_usage.
 */
static int process_args(int argc, char *argv[])
{
	int i;

	for (i=0; i<argc; i++)
	{
		if (argv[i][0] == '-')
		{
			switch (argv[i][1])
			{
			case 'h':
				print_help();
				return 1;

			case 'v':
				print_version();
				return 1;

			case 'c':
				hostsock = dsb_net_connect(argv[++i]);
				break;

			case 'i':
				interactive = true;
				break;

			case 'l':
			{
				char modfile[300];
				if (dsb_module_probe(argv[++i],modfile))
				{
					dsb_module_load(modfile,&Root);
				}
				else
				{
					DSB_ERROR(ERR_NOMOD,argv[i]);
				}
			}; break;

			default:
				printf("Invalid Option\n");
				print_help();
				break;
			}
		}
	}

	return 0;
}

int main(int argc, char *argv[])
{
	int ret;

	//Initialise the common parts
	dsb_common_init();
	hostsock = 0;

	//Register required network handlers...
	dsb_net_callback(DSBNET_ERROR,net_cb_error);
	dsb_net_callback(DSBNET_BASE,net_cb_base);
	dsb_net_callback(DSBNET_SENDEVENT,net_cb_event);
	dsb_net_callback(DSBNET_DEBUGEVENT,net_cb_debugevent);

	//Make sure names map is up to date.
	//dsb_names_rebuild();

	//Process conf file

	//Ready to process command line args.
	ret = process_args(argc,argv);
	if (ret != 0) return ret;
	running = true;

	//Set signal handler
	signal(SIGINT, sigint);

	if (interactive) dsb_lite_startcli();

	while (running)
	{
		dsb_net_poll(100);
	}

	printf("Terminating...\n");

	dsb_common_final();

	return 0;
}
