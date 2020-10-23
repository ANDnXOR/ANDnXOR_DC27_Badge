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
 * ADDITIONALLY:  shell_print(shell, "LED Brightness set to: %d",
 m_settings.brightness); is31fl3741_gcc_set(m_settings.brightness);
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

#ifndef FF_UNLOCKS_H
#define FF_UNLOCKS_H

#include "ff_settings.h"

#define FF_UNLOCK_DISTANCE_TRAVEL (8 * 256)

#define FF_UNLOCK_WHOAMI 0x01
#define FF_UNLOCK_DISTANCE 0x02
#define FF_UNLOCK_AUDIO 0x04
#define FF_UNLOCK_C2 0x08
#define FF_UNLOCK_AUDIO_PUZZLE 0x10
#define FF_UNLOCK_DC619 0x20
#define FF_UNLOCK_FLAPPY 0x40
#define FF_UNLOCK_EYES 0x80

// Macro to set an unlock
#define FF_UNLOCK_SET(_unlock)                                                 \
  LOG_INF("Unlock 0x%04x Found", _unlock);                                      \
  (ff_settings_ptr_get()->unlock = ff_settings_ptr_get()->unlock | _unlock);   \
  ff_settings_save()

// Macro that returns true if an unlock is valid
#define FF_UNLOCK_VALIDATE(_unlock)                                            \
  ((ff_settings_ptr_get()->unlock & _unlock) > 0)

#endif