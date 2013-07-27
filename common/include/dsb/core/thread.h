/*
 * thread.h
 *
 *  Created on: 9 Jun 2013
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

#ifndef THREAD_H_
#define THREAD_H_

#include "dsb/config.h"

#if defined(UNIX) && !defined(NO_THREADS)
#include <pthread.h>
#endif

#if defined(UNIX) && !defined(NO_THREADS)
#define R_LOCK(A) pthread_rwlock_rdlock(&A)
#define W_LOCK(A) pthread_rwlock_wrlock(&A)
#define R_UNLOCK(A) pthread_rwlock_unlock(&A)
#define W_UNLOCK(A) pthread_rwlock_unlock(&A)
#define LOCK(A) pthread_mutex_lock(&A)
#define UNLOCK(A) pthread_mutex_unlock(&A)

#define WAIT(A,B) pthread_cond_wait(&A,&B);
#define BROADCAST(A) pthread_cond_broadcast(&A);

#define RWLOCK(A) static pthread_rwlock_t A = PTHREAD_RWLOCK_INITIALIZER
#define MUTEX(A) static pthread_mutex_t A = PTHREAD_MUTEX_INITIALIZER
#define COND(A) static pthread_cond_t A = PTHREAD_COND_INITIALIZER
#else
#define R_LOCK(A)
#define W_LOCK(A)
#define R_UNLOCK(A)
#define W_UNLOCK(A)
#define LOCK(A)
#define UNLOCK(A)
#define RWLOCK(A)
#define MUTEX(A)
#endif


#endif /* THREAD_H_ */
