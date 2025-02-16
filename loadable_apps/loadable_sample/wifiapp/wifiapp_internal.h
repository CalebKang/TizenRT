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
#ifndef __EXAMPLES_ELF_WIFIAPP_INTERNAL_H
#define __EXAMPLES_ELF_WIFIAPP_INTERNAL_H

#ifdef CONFIG_EXAMPLES_RECOVERY_TEST
void recovery_test(void);
#endif
#ifdef CONFIG_EXAMPLES_MESSAGING_TEST
void messaging_test(void);
#endif

#endif
