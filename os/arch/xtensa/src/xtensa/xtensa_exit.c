/******************************************************************
 *
 * Copyright 2018 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************/

/****************************************************************************
 *
 *   Copyright (C) 2016-2017 Gregory Nutt. All rights reserved.
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

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <tinyara/config.h>

#include <sched.h>
#include <syscall.h>
#include <assert.h>
#include <debug.h>

#include <tinyara/arch.h>
#include <tinyara/irq.h>
#ifdef CONFIG_DUMP_ON_EXIT
#include <tinyara/fs/fs.h>
#endif

#include "task/task.h"
#include "sched/sched.h"
#include "group/group.h"
#include "xtensa.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifndef CONFIG_DEBUG_SCHED_INFO
#undef CONFIG_DUMP_ON_EXIT
#endif

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: _xtensa_dumponexit
 *
 * Description:
 *   Dump the state of all tasks whenever on task exits.  This is debug
 *   instrumentation that was added to check file-related reference counting
 *   but could be useful again sometime in the future.
 *
 ****************************************************************************/

#ifdef CONFIG_DUMP_ON_EXIT
static void _xtensa_dumponexit(FAR struct tcb_s *tcb, FAR void *arg)
{
#if CONFIG_NFILE_DESCRIPTORS > 0
	FAR struct filelist *filelist;
#if CONFIG_NFILE_STREAMS > 0
	FAR struct streamlist *streamlist;
#endif
	int i;
#endif

	svdbg("  TCB=%p name=%s pid=%d\n", tcb, tcb->argv[0], tcb->pid);
	svdbg("    priority=%d state=%d\n", tcb->sched_priority, tcb->task_state);

#if CONFIG_NFILE_DESCRIPTORS > 0
	filelist = tcb->group->tg_filelist;
	for (i = 0; i < CONFIG_NFILE_DESCRIPTORS; i++) {
		struct inode *inode = filelist->fl_files[i].f_inode;
		if (inode) {
			svdbg("      fd=%d refcount=%d\n", i, inode->i_crefs);
		}
	}
#endif

#if CONFIG_NFILE_STREAMS > 0
	streamlist = tcb->group->tg_streamlist;
	for (i = 0; i < CONFIG_NFILE_STREAMS; i++) {
		struct file_struct *filep = &streamlist->sl_streams[i];
		if (filep->fs_fd >= 0) {
#ifndef CONFIG_STDIO_DISABLE_BUFFERING
			if (filep->fs_bufstart != NULL) {
				svdbg("      fd=%d nbytes=%d\n", filep->fs_fd, filep->fs_bufpos - filep->fs_bufstart);
			} else
#endif
			{
				svdbg("      fd=%d\n", filep->fs_fd);
			}
		}
	}
#endif
}
#endif

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: _exit
 *
 * Description:
 *   This function causes the currently executing task to cease
 *   to exist.  This is a special case of task_delete() where the task to
 *   be deleted is the currently executing task.  It is more complex because
 *   a context switch must be perform to the next ready to run task.
 *
 ****************************************************************************/

void _exit(int status)
{
	struct tcb_s *tcb;

	/* Make sure that we are in a critical section with local interrupts.
	 * The IRQ state will be restored when the next task is started.
	 */

	(void)irqsave();

	sllvdbg("TCB=%p exiting\n", this_task());

#ifdef CONFIG_DUMP_ON_EXIT
	sllvdbg("Other tasks:\n");
	sched_foreach(_xtensa_dumponexit, NULL);
#endif

#if XCHAL_CP_NUM > 0
	/* Disable co-processor support for the task that is exit-ing. */

	tcb = this_task();
	xtensa_coproc_disable(&tcb->xcp.cpstate, XTENSA_CP_ALLSET);
#endif

	/* Destroy the task at the head of the ready to run list. */

	(void)task_exit();

	/* Now, perform the context switch to the new ready-to-run task at the
	 * head of the list.
	 */

	tcb = this_task();

#if XCHAL_CP_NUM > 0
	/* Set up the co-processor state for the newly started thread. */

	xtensa_coproc_restorestate(&tcb->xcp.cpstate);
#endif

#ifdef CONFIG_ARCH_ADDRENV
	/* Make sure that the address environment for the previously running
	 * task is closed down gracefully (data caches dump, MMU flushed) and
	 * set up the address environment for the new thread at the head of
	 * the ready-to-run list.
	 */

	(void)group_addrenv(tcb);
#endif

	/* Then switch contexts */

	xtensa_context_restore(tcb->xcp.regs);

	/* xtensa_full_context_restore() should not return but could if the software
	 * interrupts are disabled.
	 */

	PANIC();
}
