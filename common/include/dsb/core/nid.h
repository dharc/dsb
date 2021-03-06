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

/** @file nid.h */

#ifndef _NID_H_
#define _NID_H_

#include "dsb/types.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @addtogroup Nodes
 * Nodes are the connecting points in the hypergraph and each is given a unique
 * identity so that we may refer to them. There are nodes for all integers,
 * floats, characters and other kinds of entity. These nodes are combined
 * to form hyperarcs and so act as the foundation to the entire DSB system.
 * The functions in this module support the construction, manipulation and
 * comparison of nodes. All operate upon the core NID structure.
 * @{
 */

/**
 * Run-time type classification of nodes. Some nodes need to be treated
 * in a special built-in manner, for example the integers. A type of NID_USER
 * or above is available for allocation to particular machines or other
 * devices.
 */
enum NIDType
{
	NID_TYPE_SPECIAL=0,		///< Special internal nodes such as true and false.
	NID_TYPE_INTEGER,		///< A node corresponding to an integer.
	NID_TYPE_REAL,			///< A node corresponding to a real number.
	NID_TYPE_CHARACTER,		///< A node corresponding to a unicode character.
	NID_TYPE_LABEL,			///< Nodes used as labels.
	NID_TYPE_VMOP,

	NID_TYPE_USER=1000 //!< NID_USER
};

#define NID_COMMON		0x00	//No serial number
#define NID_VOLATILE	0x01
#define NID_PERSISTENT	0x03
#define NID_AGENT		0x02

/**
 * Node IDentifier.
 *
 * A 96bit unique ID for Nodes in the DSB structure. Each node
 * has a type and a 64bit number of that type. To create a NID corresponding to
 * the integer 50, for example, set `type` to `NID_INTEGER` and `ll` to
 * the number 50.
 *
 * Special nodes such as NULL and TRUE or FALSE can be created using the
 * `NID_SPECIAL` type and then setting `ll` to `SPECIAL_NULL`, `SPECIAL_TRUE`
 * or `SPECIAL_FALSE`.
 *
 * @see specials.h
 */
struct NID
{
	unsigned char header;

	union
	{
		//Has MAC = 1
		struct
		{
			unsigned char mac[8];		///< Serial number
			//unsigned long long n : 40;	///< Node number
			unsigned int n;
		};
		//Has MAC = 0
		struct
		{
			unsigned char r2;			///< Reserved
			unsigned short t;			///< Type
			union
			{
				struct
				{
					unsigned int a;
					unsigned int b;
				};
				long long ll;
				unsigned long long ull;
				double dbl;
				char chr;
			};
		};
	};
};


/*struct NID
{
	enum NIDType type;				///< Node type.

	union
	{
		struct
		{
			unsigned int a;			///< 32bit component of ll.
			unsigned int b;			///< 32bit component of ll.
		};
		unsigned long long ll;		///< 64bit value.
		double dbl;					///< Double version of ll.
		unsigned short chr;			///< Unicode version of ll.
	};

	//TODO Add security tag.
};*/

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

void dsb_nid_null(NID_t *n);

int dsb_nid_pack(const NID_t *n, char *buf, int max);
int dsb_nid_unpack(const char *buf, NID_t *n);

int dsb_nid_eq(const NID_t *n1, const NID_t *n2);
int dsb_nid_leq(const NID_t *n1, const NID_t *n2);
int dsb_nid_geq(const NID_t *n1, const NID_t *n2);

int dsb_nid_op(unsigned long long op, NID_t *n);

/**
 * Checks the MAC component against the local MAC. If there is no MAC then it
 * assumes it is local.
 * @param n NID to check.
 * @return 1 for Local, 0 for remote.
 */
int dsb_nid_isLocal(const NID_t *n);

/**
 * Get the local persistent or volatile base NID.
 * @param persistent 0 gets a volatile base, 1 gets a persistent base.
 * @param n NID to fill with local base.
 * @return SUCCESS
 */
int dsb_nid_local(int head, NID_t *n);

/**
 * Convert a NID to a string.
 *
 * The generated string is placed in the `str` buffer with a maximum length
 * of `len`. The basic numeric types such as integers are converted to numeric
 * strings. Booleans are converted to `true` and `false`.
 *
 * Other non basic types take the form:
 *
 *     <[TYPE:VALUE]>
 *
 * String objects (ie. Nodes that correspond to a string structure) are not
 * converted to strings with this function.
 *
 * @see dstring.h
 *
 * @param nid NID to convert.
 * @param str Buffer to put the string into.
 * @param len Max length of string.
 * @return SUCCESS.
 */
int dsb_nid_toStr(const NID_t *nid, char *str, int len);

int dsb_nid_toRawStr(const NID_t *nid, char *str, int len);

int dsb_nid_pretty(const NID_t *nid, char *str, int len);

/**
 * Generate a NID from a string.
 * @param str Input string.
 * @param nid NID structure to fill with result.
 * @return 0 on success.
 */
int dsb_nid_fromStr(const char *str, NID_t *nid);

/**
 * NodeID Constructor.
 * @param[in] type Node type.
 * @param[in] ll Node value.
 * @param[out] nid Structure to populate.
 * @return nid.
 */
struct NID *dsb_nid(enum NIDType type, unsigned long long ll, NID_t *nid);

/**
 * Make a NID from an int.
 * @param[in] i The source integer.
 * @param[out] n The destination NID to populate.
 */
void dsb_iton(int, NID_t*);

/**
 * Extract an int from a NID. If the NID is not of an integer type then 0
 * is returned.
 * @param n Source NID.
 * @return Integer converted from the NID.
 */
int dsb_ntoi(const NID_t*);

void dsb_cton(char chr, NID_t *n);
char dsb_ntoc(const NID_t *n);

/** @} */

#ifdef __cplusplus
}
#endif

#endif //_NID_H_
