/*
 * cli.c
 *
 *  Created on: 29 Jul 2013
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

#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include "dsb/net.h"
#include "dsb/wrap.h"
#include "dsb/core/nid.h"

void sigint(int s);
extern void *hostsock;

struct CommandHandler
{
	char *cmd;
	void (*h)(char **word, int c);
};

/*
 * Connect to dsbd server
 */
static void lite_cmd_connect(char **word, int c)
{
	if (c != 2)
	{
		printf("Missing host name\n");
		return;
	}
	hostsock = dsb_net_connect(word[1]);
}

static void lite_cmd_exit(char **word, int c)
{
	sigint(0);
}

/*
 * Send a SET event.
 */
static void lite_cmd_set(char **word, int c)
{
	if (c != 4)
	{
		printf("Set requires exactly 3 arguments.\n");
		return;
	}

	dsb_setzzz(word[1],word[2],word[3]);
}

/*
 * Send a GET event.
 */
static void lite_cmd_get(char **word, int c)
{
	char buf[100];
	NID_t res;

	if (c != 3)
	{
		printf("Get requires exactly 2 arguments.\n");
		return;
	}

	dsb_getzzn(word[1],word[2],&res);
	dsb_nid_toStr(&res,buf,100);
	printf(" = %s\n",buf);
}

/*
 * Mapping of string command names to functions.
 */
static struct CommandHandler commands[] = {
		{"connect", lite_cmd_connect},
		{"exit", lite_cmd_exit},
		{"set", lite_cmd_set},
		{"get", lite_cmd_get},
		{0,0}
};

/*
 * Find command in commands array and call associated function.
 */
static void call_command(const char *cmd, char **words, int count)
{
	int ix = 0;

	while (commands[ix].cmd != 0)
	{
		if (strcmp(commands[ix].cmd,cmd) == 0)
		{
			commands[ix].h(words,count);
			return;
		}
		++ix;
	}

	printf("Unknown command \"%s\"\n",cmd);
}

static int split_words(const char *in, char **word)
{
	int ix = 0;
	const char *tmp;

	do
	{
		tmp = strchr(in,' ');
		if (tmp != 0)
		{
			strncpy(word[ix], in, tmp-in);
			word[ix][tmp-in] = 0;
			in += tmp-in + 1;
		}
		else
		{
			strcpy(word[ix],in);
			//Remove newline char.
			word[ix][strlen(word[ix])-1] = 0;
		}

		++ix;
	} while (tmp != 0);

	return ix;
}

/*
 * Command-line-interface thread.
 */
static void *lite_cli(void *arg)
{
	char inbuf[200];
	char *words[20];
	int count;

	for (count = 0; count < 20; count++)
	{
		words[count] = malloc(200);
	}

	while (1)
	{
		//Print prompt and get line.
		printf("> ");
		fflush(stdout);
		fgets(inbuf,200,stdin);

		//Split into array of words.
		count = split_words(inbuf,words);

		//Process commands.
		if (count > 0)
		{
			call_command(words[0],words,count);
		}
	}
	return 0;
}

void dsb_lite_startcli()
{
	pthread_t th;
	pthread_create(&th, 0, lite_cli, 0);
}
