/*
 * errors.c
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

#include "dsb/errors.h"
#include <stdio.h>
#include "dsb/config.h"

const char *dsb_log_str(int err)
{
	switch(err)
	{
	case SUCCESS:				return "Success";
	case ERR_NIDSTR:			return "Invalid NID string";
	case ERR_NOINIT:			return "Not Initialised";
	case ERR_REINIT:			return "Multiple Initialisation";
	case ERR_ROUTE_SLOT:		return "No spare router slots";
	case ERR_NOROUTE:			return "No known route for event";
	case ERR_ROUTE_MISSING:		return "Missing handler in route table";
	case ERR_INVALIDEVENT:		return "Invalid event type";
	case ERR_INVALIDMOD:		return "Module is invalid";
	case ERR_NOMOD:				return "Cannot find module";
	case ERR_MODEXISTS:			return "Module already exists";
	case ERR_MODNAME:			return "Invalid module name";
	case ERR_EVALID:			return "Invalid evaluator ID";
	case ERR_NOEVAL:			return "No evaluator for given ID";
	case ERR_EVALEXISTS:		return "Evaluator already exists";
	case ERR_NETBIND:			return "Unable to bind socket";
	case ERR_NETCONNECT:		return "Could not connect to host";
	case ERR_NET:				return "Generic Network Error";
	case ERR_NETMSGCHK:			return "Corrupted message";
	case ERR_NETMSGDATA:		return "Incorrect amount of message data";
	case ERR_NETMSGTYPE:		return "Invalid message type";
	case ERR_NETLISTEN:			return "Could not listen on port";
	case ERR_NETMAX:			return "Max net connections reached";
	case ERR_NETACCEPT:			return "Could not accept connection";
	case ERR_NETTIMEOUT:		return "Expected net response timed out";
	case ERR_NETADDR:			return "Could not find host";
	case ERR_NETRESULT:			return "Unexpected event result";
	case ERR_NETCB:				return "Net message callback error";

	case INFO_NETACCEPT:		return "New connection.";
	case INFO_NETLISTEN:		return "Net Listening";
	case INFO_NETDISCONNECT:	return "Client disconnected";

	case DEBUG_NETMSG:			return "Net Message";
	case DEBUG_NETEVENT:		return "Net Event";
	case DEBUG_RESETNAMES:		return "Reset Names";
	default:					return "Unknown Error";
	}
}

int dsb_log(int msg, const char *str)
{
	if (msg == SUCCESS) return SUCCESS;

	//Is this an error message
	if (msg >> 12 == 0)
	{
		if (str == 0)
		{
			printf("Error: %s\n",dsb_log_str(msg));
		}
		else
		{
			printf("Error: %s - %s\n",dsb_log_str(msg),str);
		}
	}
	//Is this an information message
	else if (msg >> 12 == 3)
	{
		if (str == 0)
		{
			printf("Info: %s\n",dsb_log_str(msg));
		}
		else
		{
			printf("Info: %s - %s\n",dsb_log_str(msg),str);
		}
	}
	//Is this an information message
	else if (msg >> 12 == 4)
	{
		if (str == 0)
		{
			printf("Debug: %s\n",dsb_log_str(msg));
		}
		else
		{
			printf("Debug: %s - %s\n",dsb_log_str(msg),str);
		}
	}

	return msg;
}
