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
 *   Copyright (C) 2016 Gregory Nutt. All rights reserved.
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

#include <sys/types.h>
#include <stdint.h>
#include <sched.h>
#include <debug.h>

#include <tinyara/kmalloc.h>
#include <tinyara/arch.h>
#include <tinyara/board.h>

#include <arch/xtensa/xtensa_coproc.h>
#include <arch/chip/tie.h>
#include <arch/board/board.h>

#include "xtensa.h"

/****************************************************************************
 * Pre-processor Macros
 ****************************************************************************/

/* XTENSA requires at least a 4-byte stack alignment.  For floating point use,
 * however, the stack must be aligned to 8-byte addresses.
 *
 * REVIST: Is this true?  Comes from ARM EABI
 */

#define STACK_ALIGNMENT     8

/* Stack alignment macros */

#define STACK_ALIGN_MASK    (STACK_ALIGNMENT-1)
#define STACK_ALIGN_DOWN(a) ((a) & ~STACK_ALIGN_MASK)
#define STACK_ALIGN_UP(a)   (((a) + STACK_ALIGN_MASK) & ~STACK_ALIGN_MASK)

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: up_create_stack
 *
 * Description:
 *   Allocate a stack for a new thread and setup up stack-related information
 *   in the TCB.
 *
 *   The following TCB fields must be initialized by this function:
 *
 *   - adj_stack_size: Stack size after adjustment for hardware, processor,
 *     etc.  This value is retained only for debug purposes.
 *   - stack_alloc_ptr: Pointer to allocated stack
 *   - adj_stack_ptr: Adjusted stack_alloc_ptr for HW.  The initial value of
 *     the stack pointer.
 *
 * Input Parameters:
 *   - tcb: The TCB of new task
 *   - stack_size:  The requested stack size.  At least this much
 *     must be allocated.
 *   - ttype:  The thread type.  This may be one of following (defined in
 *     include/tinyara/sched.h):
 *
 *       TCB_FLAG_TTYPE_TASK     Normal user task
 *       TCB_FLAG_TTYPE_PTHREAD  User pthread
 *       TCB_FLAG_TTYPE_KERNEL   Kernel thread
 *
 *     This thread type is normally available in the flags field of the TCB,
 *     however, there are certain contexts where the TCB may not be fully
 *     initialized when up_create_stack is called.
 *
 *     If CONFIG_BUILD_KERNEL is defined, then this thread type may affect
 *     how the stack is allocated.  For example, kernel thread stacks should
 *     be allocated from protected kernel memory.  Stacks for user tasks and
 *     threads must come from memory that is accessible to user code.
 *
 ****************************************************************************/

int up_create_stack(FAR struct tcb_s *tcb, size_t stack_size, uint8_t ttype)
{
#if XCHAL_CP_NUM > 0
	struct xcptcontext *xcp;
	uintptr_t cpstart;
#endif

	/* Is there already a stack allocated of a different size?  Because of
	 * alignment issues, stack_size might erroneously appear to be of a
	 * different size.  Fortunately, this is not a critical operation.
	 */

	if (tcb->stack_alloc_ptr && tcb->adj_stack_size != stack_size) {
		/* Yes.. Release the old stack */

		up_release_stack(tcb, ttype);
	}
#if XCHAL_CP_NUM > 0
	/* Add the size of the co-processor save area to the stack allocation.
	 * REVISIT:  This may waste memory.  Increasing the caller's requested
	 * stack size should only be necessary if the requested size could not
	 * hold the co-processor save area.
	 */

	stack_size += XTENSA_CP_SA_SIZE;
#endif

	/* Do we need to allocate a new stack? */

	if (!tcb->stack_alloc_ptr) {
		/* Allocate the stack.  If DEBUG is enabled (but not stack debug),
		 * then create a zeroed stack to make stack dumps easier to trace.
		 */

#if defined(CONFIG_BUILD_KERNEL) && defined(CONFIG_MM_KERNEL_HEAP)
		/* Use the kernel allocator if this is a kernel thread */

		if (ttype == TCB_FLAG_TTYPE_KERNEL) {
			tcb->stack_alloc_ptr = (uint32_t *)kmm_malloc(stack_size);
		} else
#endif
		{
			/* Use the user-space allocator if this is a task or pthread */

			tcb->stack_alloc_ptr = (uint32_t *)kumm_malloc(stack_size);
		}

#ifdef CONFIG_DEBUG
		/* Was the allocation successful? */

		if (!tcb->stack_alloc_ptr) {
			serr("ERROR: Failed to allocate stack, size %d\n", stack_size);
		}
#endif
	}

	/* Did we successfully allocate a stack? */

	if (tcb->stack_alloc_ptr) {
		uintptr_t top_of_stack;
		size_t size_of_stack;

#ifdef CONFIG_STACK_COLORATION
		uint32_t *ptr;
		int i;

		/* Yes.. If stack debug is enabled, then fill the stack with a
		 * recognizable value that we can use later to test for high
		 * water marks.
		 */

		for (i = 0, ptr = (uint32_t *)tcb->stack_alloc_ptr; i < stack_size; i += sizeof(uint32_t)) {
			*ptr++ = STACK_COLOR;
		}
#endif

		/* XTENSA uses a push-down stack:  the stack grows toward lower
		 * addresses in memory.  The stack pointer register points to the
		 * lowest, valid working address (the "top" of the stack).  Items on
		 * the stack are referenced as positive word offsets from sp.
		 */

		top_of_stack = (uintptr_t)tcb->stack_alloc_ptr + stack_size - 4;

#if XCHAL_CP_NUM > 0
		/* Allocate the co-processor save area at the top of the (push down)
		 * stack.
		 *
		 * REVISIT:  This is not secure.  In secure built configurations it
		 * be more appropriate to use kmm_memalign() to allocte protected
		 * memory rather than using the stack.
		 */

		cpstart = (uintptr_t)_CP_ALIGNDOWN(XCHAL_CP0_SA_ALIGN, top_of_stack - XCHAL_CP1_SA_ALIGN);
		top_of_stack = cpstart;

		/* Initialize the coprocessor save area (see xtensa_coproc.h) */

		xcp = &tcb->xcp;
		xcp->cpstate.cpenable = xtensa_get_cpenable();	/* Update coprocessors active status for this thread */
		xcp->cpstate.cpstored = 0;	/* No coprocessors saved for this thread */
		xcp->cpstate.cpasa = (uint32_t *)cpstart;	/* Start of aligned save area */
#endif

		/* The XTENSA stack must be aligned.  If necessary top_of_stack must be
		 * rounded down to the next boundary to meet this alignment requirement.
		 *
		 * NOTE: Co-processor save area not included in the size of the stack.
		 */

		top_of_stack = STACK_ALIGN_DOWN(top_of_stack);
		size_of_stack = top_of_stack - (uint32_t)tcb->stack_alloc_ptr + 4;

		/* Save the adjusted stack values in the struct tcb_s */

		tcb->adj_stack_ptr = (FAR uint32_t *)top_of_stack;
		tcb->adj_stack_size = size_of_stack;

		board_autoled_on(LED_STACKCREATED);
		return OK;
	}

	return ERROR;
}
