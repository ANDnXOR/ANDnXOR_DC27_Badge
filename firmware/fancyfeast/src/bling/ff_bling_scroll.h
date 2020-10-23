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

#ifndef FF_BLING_SCROLL_H
#define FF_BLING_SCROLL_H

#include "../ff_bling.h"
#include "../ff_settings.h"

/**
 * This is the handler that gets called every time a frame needs to be drawn
 */
extern void ff_bling_handler_scroll(bling_t *p_bling);

/**
 * This is the handler that gets called every time a frame needs to be drawn
 */
extern void ff_bling_handler_scroll_bounce(bling_t *p_bling);

/**
 * This is the handler that gets called every time a frame needs to be drawn
 */
extern void ff_bling_handler_scroll_time(bling_t *p_bling);

/**
 * @brief Macro for defining a default scroll bling mode
 * @param[in] _name		Instance name
 */
#define FF_BLING_DEFAULT_SCROLL(_name, _text)                                  \
  bling_t _name = {.hue = ff_settings_ptr_get()->bling_hue_scroll,             \
                   .mode = ff_bling_mode_scroll,                               \
                   .y = 1};                                                    \
  snprintf(_name.user_data, FF_BLING_USER_DATA_SIZE, "%s", _text)

/**
 * @brief Macro for defining a default scroll bounce bling mode
 * @param[in] _name		Instance name
 */
#define FF_BLING_DEFAULT_SCROLL_BOUNCE(_name, _text)                           \
  bling_t _name = {.hue = 0, .mode = ff_bling_mode_scroll_bounce, .y = 1};     \
  snprintf(_name.user_data, FF_BLING_USER_DATA_SIZE, "%s", _text)

/**
 * @brief Macro for defining a default scroll time bling mode
 * @param[in] _name		Instance name
 */
#define FF_BLING_DEFAULT_SCROLL_TIME(_name)                                    \
  bling_t _name = {.hue = 0, .mode = ff_bling_mode_scroll_time, .y = 1}

#endif