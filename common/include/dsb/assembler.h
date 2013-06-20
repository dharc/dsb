/*
 * assember.h
 *
 *  Created on: 31 May 2013
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

#ifndef ASSEMBER_H_
#define ASSEMBER_H_

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct NID NID_t;

struct VMLabel
{
	char label[10];
	int lip;
	int mode;		//0 = label, 1 = variable
};

#define MAX_LABELS		100

/**
 * Compile DSB assembly into byte code. The resulting byte code can be
 * interpreted using dsb_vm_interpret.
 * @param source Assembly string.
 * @param output Array of NIDs to put the byte code into.
 * @param max Size of the output array.
 * @return SUCCESS or assembly error.
 */
int dsb_assemble(const char *source, NID_t *output, int max);

int dsb_assemble_line(struct VMLabel *labels, const char *line, NID_t *output, int *ip);

/**
 * Populate the labels array with all labels in the source and their corresponding
 * location in the final compiled code. Should be use to initialise the labels array
 * before attempting to actual assemble the code.
 * @param labels
 * @param source
 * @return SUCCESS.
 */
int dsb_assemble_labels(struct VMLabel *labels, const char *source);

int dsb_disassemble(const NID_t *src, int size, char *output, int max);

#ifdef __cplusplus
}
#endif

#endif /* ASSEMBER_H_ */
