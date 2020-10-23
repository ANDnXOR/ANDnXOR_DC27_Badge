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

#ifndef FF_BLING_TUNNEL_H
#define FF_BLING_TUNNEL_H

#include "../ff_bling.h"

extern int ff_bling_cmd_play_tunnel(const struct shell *shell, size_t argc,
                                    char **argv);

/**
 * This is the handler that gets called every time a frame needs to be drawn
 */
extern void ff_bling_handler_tunnel(bling_t *p_bling);

/**
 * @brief Macro for defining a default tunnel bling mode
 * @param[in] _name		Instance name
 */
#define FF_BLING_DEFAULT_TUNNEL(_name)                                         \
  bling_t _name = {.mode = ff_bling_mode_tunnel, .x = 0, .hue = 0.5};

#endif