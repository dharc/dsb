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
	NID_SPECIAL=0,		///< Special internal nodes such as true and false.
	NID_INTEGER,		///< A node corresponding to an integer.
	NID_REAL,			///< A node corresponding to a real number.
	NID_CHARACTER,		///< A node corresponding to a unicode character.
	NID_LABEL,			///< Nodes used as labels.
	NID_OPERATOR,		///< Definition operators.
	//---- Integer Operations -----
	NID_INTADD,   //!< NID_INTADD
	NID_INTSUB,   //!< NID_INTSUB
	NID_INTDIV,   //!< NID_INTDIV
	NID_INTMUL,   //!< NID_INTMUL
	NID_INTBITAND,//!< NID_INTBITAND
	NID_INTBITNOT,//!< NID_INTBITNOT
	NID_INTBITOR, //!< NID_INTBITOR
	NID_INTSHIFTL,//!< NID_INTSHIFTL
	NID_INTSHIFTR,//!< NID_INTSHIFTR

	NID_USER=1000 //!< NID_USER
};

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
};

typedef struct NID NID_t;

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

int dsb_nid_pack(const NID_t *n, char *buf, int max);
int dsb_nid_unpack(const char *buf, NID_t *n);

/**
 * Compare two NIDs. If both are equal then 0 is returned, otherwise if a < b
 * then a negative number is returned, otherwise a positive number.
 * @param a First NID
 * @param b Second NID
 * @return 0 for equality, negative if a<b, positive if a>b.
 */
int dsb_nid_compare(const struct NID *a, const struct NID *b);

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
int dsb_nid_toStr(const struct NID *nid, char *str, int len);

/**
 * Generate a NID from a string.
 * @param str Input string.
 * @param nid NID structure to fill with result.
 * @return 0 on success.
 */
int dsb_nid_fromStr(const char *str, struct NID *nid);

/**
 * NodeID Constructor.
 * @param[in] type Node type.
 * @param[in] ll Node value.
 * @param[out] nid Structure to populate.
 * @return nid.
 */
struct NID *dsb_nid(enum NIDType type, unsigned long long ll, struct NID *nid);

/**
 * Make a NID from an int.
 * @param[in] i The source integer.
 * @param[out] n The destination NID to populate.
 */
void dsb_iton(int,struct NID*);

/**
 * Extract an int from a NID. If the NID is not of an integer type then 0
 * is returned.
 * @param n Source NID.
 * @return Integer converted from the NID.
 */
int dsb_ntoi(const struct NID*);

/** @} */

#ifdef __cplusplus
}
#endif

#endif //_NID_H_
