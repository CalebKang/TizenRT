/****************************************************************************
 *
 * Copyright 2018 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 *
 ****************************************************************************/
/************************************************************************************
 * arch/arm/src/stm32l4/stm32l4_ltdc.h
 *
 *   Copyright (C) 2013-2014 Ken Pettit. All rights reserved.
 *   Authors: Ken Pettit <pettitd@gmail.com>
 *            Marco Krahl <ocram.lhark@gmail.com>
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
 ************************************************************************************/

#ifndef __ARCH_ARM_SRC_STM32L4_STM32L4_LTDC_H
#define __ARCH_ARM_SRC_STM32L4_STM32L4_LTDC_H

/************************************************************************************
 * Included Files
 ************************************************************************************/

#include <tinyara/config.h>

#include <stdbool.h>
#include <semaphore.h>

#include <tinyara/video/fb.h>

#include <arch/chip/ltdc.h>

#ifdef CONFIG_STM32L4_LTDC

/************************************************************************************
 * Pre-processor Definitions
 ************************************************************************************/

/************************************************************************************
 * Public Types
 ************************************************************************************/

/* Common layer state structure for the LTDC and DMA2D controller */

struct stm32_ltdc_s {
	/* Fixed settings */

	int lid;					/* Layer identifier */
	struct fb_videoinfo_s vinfo;	/* Layer videoinfo */
	struct fb_planeinfo_s pinfo;	/* Layer planeinfo */

	/* Positioning */

	struct ltdc_area_s area;	/* Active layer area */
	fb_coord_t xpos;			/* Reference x position */
	fb_coord_t ypos;			/* Reference y position */

	/* Coloring */

	uint32_t color;				/* Layer color definition */
#ifdef CONFIG_FB_CMAP
	struct fb_cmap_s *cmap;		/* Reference to the valid color lookup table */
#endif

	/* Blending */

	uint8_t alpha;				/* Layer constant alpha value */
	uint32_t colorkey;			/* Layer colorkey */
	uint32_t blendmode;			/* Layer blend factor */

	/* Operation */

	sem_t *lock;				/* Ensure mutually exclusive access */
};

/************************************************************************************
 * Public Data
 ************************************************************************************/

/************************************************************************************
 * Public Functions
 ************************************************************************************/
/* The STM32 LTDC driver uses the common framebuffer interfaces declared in
 * include/tinyara/video/fb.h.
 */

int stm32l4_ltdcinitialize(void);
FAR struct fb_vtable_s *stm32l4_ltdcgetvplane(int vplane);
void stm32l4_ltdcuninitialize(void);

/************************************************************************************
 * Name: stm32_ltdcgetlayer
 *
 * Description:
 *   Get the ltdc layer structure to perform hardware layer operation
 *
 * Parameter:
 *   lid - Layer identifier
 *
 * Return:
 *   Reference to the layer control structure on success or Null if parameter
 *   invalid.
 *
 ************************************************************************************/

FAR struct ltdc_layer_s *stm32_ltdcgetlayer(int lid);

/************************************************************************************
 * Name: stm32_lcdclear
 *
 * Description:
 *   This is a non-standard LCD interface just for the STM32 LTDC. Clearing the
 *   display in the normal way by writing a sequences of runs that covers the
 *   entire display can be slow.  Here the display is cleared by simply setting
 *   all video memory to the specified color.
 *
 ************************************************************************************/

void stm32l4_lcdclear(uint16_t color);

/************************************************************************************
 * Name: stm32_lcd_backlight
 *
 * Description:
 *   If CONFIG_STM32L4_LCD_BACKLIGHT is defined, then the board-specific logic must
 *   provide this interface to turn the backlight on and off.
 *
 ************************************************************************************/

#ifdef CONFIG_STM32L4_LCD_BACKLIGHT
void stm32l4_backlight(bool blon);
#endif

#endif							/* CONFIG_STM32L4_LTDC */
#endif							/* __ARCH_ARM_SRC_STM32L4_STM32L4_LTDC_H */
