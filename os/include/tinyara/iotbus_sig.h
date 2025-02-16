/****************************************************************************
 *
 * Copyright 2019 Samsung Electronics All Rights Reserved.
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

#ifndef __INCLUDE_TINYARA_IOTBUS_SIG_H
#define __INCLUDE_TINYARA_IOTBUS_SIG_H

/* This file will be used to provide definitions to support
 * kernel test case framework
 */

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <tinyara/config.h>
#include <tinyara/fs/ioctl.h>
#include <iotbus/iotbus_common.h>
#include <iotbus/iotbus_gpio.h>
#include <iotbus/iotbus_spi.h>
#include <iotbus/iotbus_pwm.h>
#include <iotbus/iotbus_adc.h>
#include <iotbus/iotbus_i2c.h>
#include <iotbus/iotbus_uart.h>

#ifdef CONFIG_IOTDEV

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define IOTBUS_SIGPATH						"/dev/iotsig"
#define IOTBUS_INT_SIG_TRIG(type)			iotbus_interrupt_trigger(type)

#define IOTBUS_INTR_REGISTER					_IOTBUSIOC(1)
#define IOTBUS_INTR_UNREGISTER				_IOTBUSIOC(2)
#define IOTBUS_INTR_INFO					_IOTBUSIOC(3)

/****************************************************************************
 * Public Data
 ****************************************************************************/

struct iotbus_int_info_s {
	void *handle;
	iotbus_pin_e pin_type;
	int int_type;
	pid_t pid;
};

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#ifdef __cplusplus
#define EXTERN extern "C"
extern "C" {
#else
#define EXTERN extern
#endif

/****************************************************************************
 * Name: iotbus_sig_register
 *
 * Description:
 *   This function creates a device node like "/dev/iotsig" which will be used
 *   by the iotbus to get events from peripheral
 *
 *
 ****************************************************************************/

void iotbus_sig_register(void);

void iotbus_interrupt_sig_trigger(int irq, iotbus_int_type_e int_type);

#undef EXTERN
#ifdef __cplusplus
}
#endif

#else
#define IOTBUS_INT_SIG_TRIG(type)
#endif
#endif /* __INCLUDE_TINYARA_IOTBUS_SIG_H */
