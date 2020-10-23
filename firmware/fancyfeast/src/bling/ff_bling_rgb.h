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

#ifndef FF_BLING_RGB_H
#define FF_BLING_RGB_H

#include "../ff_bling.h"

extern void ff_bling_handler_rgb(bling_t *p_bling);

typedef enum {
  bling_rgb_mode_colors9,
  bling_rgb_mode_lightning,
  bling_rgb_mode_colors4,
  bling_rgb_mode_red4,
  bling_rgb_mode_bw1,     // Unlocked with DC619
  bling_rgb_mode_psycho2, // Unlocked with DC619
  __bling_rgb_mode_counter
} bling_rgb_mode_t;

#define FF_BLING_DEFAULT_RGB_MODE bling_rgb_mode_bw1

/**
 * @brief Macro for defining a default RGB bling mode
 * @param[in] _name		Instance name
 */
#define FF_BLING_DEFAULT_RGB(_name, _rgb)                                      \
  bling_t _name = {.frames = 0, .mode = ff_bling_mode_rgb};                    \
  _name.user_data[0] = _rgb;

#endif