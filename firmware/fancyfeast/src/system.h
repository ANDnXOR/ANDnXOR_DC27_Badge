/*****************************************************************************
 * Made with beer and late nights in California.
 *
 * (C) Copyright 2017-2019 AND!XOR LLC (https://andnxor.com/).
 *
 * PROPRIETARY AND CONFIDENTIAL UNTIL AUGUST 11th, 2019 then,
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ADDITIONALLY:
 * If you find this source code useful in anyway, use it in another electronic
 * conference badge, or just think it's neat. Consider buying us a beer
 * (or two) and/or a badge (or two). We are just as obsessed with collecting
 * badges as we are in making them.
 *
 * Contributors:
 * 	@andnxor
 * 	@zappbrandnxor
 * 	@hyr0n1
 * 	@bender_andnxor
 * 	@lacosteaef
 *  @f4nci3
 *  @Cr4bf04m
 *****************************************************************************/
#ifndef SS_SYSTEM_H
#define SS_SYSTEM_H

#include "kernel.h"

#define VERSION_INT 23
#define VERSION "v23"
#define BUILD_TIMESTAMP ""__DATE__" "__TIME__" PT"

#define SYSTEM_INIT_PRIORITY_SETTINGS 98
#define SYSTEM_INIT_PRIORITY_BLE 99

//Total possible nodes, we have 10 prototypes (ish) + 600 at Macrofab. Rounded up to a divisible-by-64 number
#define NODE_COUNT 640
#define FF_THREAD_PRIORITY_HIGH K_PRIO_COOP(1)
#define FF_THREAD_PRIORITY_MEDIUM K_PRIO_PREEMPT(5)
#define FF_THREAD_PRIORITY_LOW K_PRIO_PREEMPT(9)

#endif