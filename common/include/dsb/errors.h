/*
 * errors.h
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

/** @file errors.h */

#ifndef DSB_ERRORS_H_
#define DSB_ERRORS_H_

#include "dsb/config.h"

/**
 * Error enums used for function return values.
 */
enum
{
	SUCCESS=0,			//!< SUCCESS

	ERR_ERROR=1000,
	ERR_NIDSTR,			///< Invalid NID String
	ERR_REINIT,			///< Multiple init
	ERR_NOINIT,			///< Not initialised
	ERR_ROUTE_SLOT,		///< No spare slots
	ERR_NOROUTE,		///< No handler for event
	ERR_ROUTE_MISSING,	///< Missing handler for event
	ERR_NID_FREE,		///< Can't free NID.
	ERR_NOTSENT,		///< Event hasn't been sent.
	ERR_INVALIDEVENT,	///< Event type is unknown.
	ERR_INVALIDMOD,		///< Module structure is missing something.
	ERR_NOMOD,			///< Cannot find module.
	ERR_MODEXISTS,		///< Module already registered.
	ERR_MODNAME,		///< Invalid module name.
	ERR_EVALID,			///< Invalid evaluator ID.
	ERR_NOEVAL,			///< No evaluator for given ID.
	ERR_EVALEXISTS,		///< Evaluator already registered to ID.
	ERR_NETBIND,
	ERR_NETCONNECT,		///< Could not connect to address.
	ERR_NET,
	ERR_NETMSGCHK,
	ERR_NETMSGDATA,
	ERR_NETMSGTYPE,
	ERR_NETLISTEN,
	ERR_NETMAX,
	ERR_NETACCEPT,
	ERR_NETTIMEOUT,
	ERR_NETADDR,
	ERR_NETRESULT,
	ERR_NETCB,

	WARN_START=0x2000,

	INFO_START=0x3000,
	INFO_NETACCEPT,
	INFO_NETLISTEN,
	INFO_NETDISCONNECT,

	DEBUG_START=0x4000,
	DEBUG_NETMSG,

	ERR_END   			//!< ERR_END
};

/**
 * Convert DSB error number to a string.
 * @param err Error number.
 * @return String for the error.
 */
const char *dsb_log_str(int err);

/**
 * Log and print error messages, depending upon log and debug settings.
 * @param errno
 * @param data Optional node containing additional error details.
 * @return errno, as passed in the parameter.
 */
int dsb_log(int msg, const char *str);

#define DSB_ERROR(A,B) dsb_log(A,B)

#ifdef _DEBUG
#define DSB_INFO(A,B) dsb_log(A,B)
#else
#define DSB_INFO(A,B) A
#endif

#ifdef _DEBUG
#define DSB_DEBUG(A,B) dsb_log(A,B)
#else
#define DSB_DEBUG(A,B) A
#endif


#endif /* ERRORS_H_ */
