/****************************************************************************
 * mm/iob/iob.h
 *
 *   Copyright (C) 2014, 2017 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

#ifndef __MM_IOB_IOB_H
#define __MM_IOB_IOB_H 1

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <tinyara/config.h>

#include <semaphore.h>
#include <debug.h>

#include <tinyara/bluetooth/iob/iob.h>

#ifdef CONFIG_MM_IOB

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#if defined(CONFIG_DEBUG) && defined(CONFIG_IOB_DEBUG)
#ifdef CONFIG_CPP_HAVE_VARARGS

#define ioberr(format, ...)    _err(format, ##__VA_ARGS__)
#define iobwarn(format, ...)   _warn(format, ##__VA_ARGS__)
#define iobinfo(format, ...)   _info(format, ##__VA_ARGS__)

#else

#define ioberr                 _err
#define iobwarn                _warn
#define iobinfo                _info

#endif
#else
#ifdef CONFIG_CPP_HAVE_VARARGS

#define ioberr(format, ...)
#define iobwarn(format, ...)
#define iobinfo(format, ...)

#else

#define ioberr                 (void)
#define iobwarn                (void)
#define iobinfo                (void)

#endif
#endif							/* CONFIG_DEBUG && CONFIG_IOB_DEBUG */

/****************************************************************************
 * Public Data
 ****************************************************************************/

/* A list of all free, unallocated I/O buffers */

extern FAR struct iob_s *g_iob_freelist;

/* A list of I/O buffers that are committed for allocation */

extern FAR struct iob_s *g_iob_committed;

#if CONFIG_IOB_NCHAINS > 0
/* A list of all free, unallocated I/O buffer queue containers */

extern FAR struct iob_qentry_s *g_iob_freeqlist;

/* A list of I/O buffer queue containers that are committed for allocation */

extern FAR struct iob_qentry_s *g_iob_qcommitted;
#endif

/* Counting semaphores that tracks the number of free IOBs/qentries */

extern sem_t g_iob_sem;			/* Counts free I/O buffers */
#if CONFIG_IOB_THROTTLE > 0
extern sem_t g_throttle_sem;	/* Counts available I/O buffers when throttled */
#endif
#if CONFIG_IOB_NCHAINS > 0
extern sem_t g_qentry_sem;		/* Counts free I/O buffer queue containers */
#endif

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Name: iob_alloc_qentry
 *
 * Description:
 *   Allocate an I/O buffer chain container by taking the buffer at the head
 *   of the free list. This function is intended only for internal use by
 *   the IOB module.
 *
 ****************************************************************************/

FAR struct iob_qentry_s *iob_alloc_qentry(void);

/****************************************************************************
 * Name: iob_tryalloc_qentry
 *
 * Description:
 *   Try to allocate an I/O buffer chain container by taking the buffer at
 *   the head of the free list without waiting for the container to become
 *   free. This function is intended only for internal use by the IOB module.
 *
 ****************************************************************************/

FAR struct iob_qentry_s *iob_tryalloc_qentry(void);

/****************************************************************************
 * Name: iob_free_qentry
 *
 * Description:
 *   Free the I/O buffer chain container by returning it to the  free list.
 *   The link to  the next I/O buffer in the chain is return.
 *
 ****************************************************************************/

FAR struct iob_qentry_s *iob_free_qentry(FAR struct iob_qentry_s *iobq);

#endif							/* CONFIG_MM_IOB */
#endif							/* __MM_IOB_IOB_H */
