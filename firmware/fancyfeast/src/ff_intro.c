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
#include <zephyr.h>

#define LOG_LEVEL 4
#include <logging/log.h>
LOG_MODULE_REGISTER(ff_intro);

#include "bling/ff_bling_doom.h"
#include "bling/ff_bling_rgb.h"
// #include "bling/lightning2.h"
#include "ff_bling.h"
#include "ff_gfx.h"
#include "ff_intro.h"
#include "ff_util.h"

/**
 * @brief Run the intro animation
 */
void ff_intro_run() {

  FF_BLING_DEFAULT_RGB(red, bling_rgb_mode_red4);
  FF_BLING_DEFAULT_RGB(lightning, bling_rgb_mode_lightning);
  FF_BLING_DEFAULT_DOOM(doom);

  uint32_t start = ff_util_millis();
  while (ff_util_millis() - start < 3000) {
    ff_bling_handler_rgb(&red);
    k_sleep(50);
  }

  start = ff_util_millis();
  while (ff_util_millis() - start < 5000) {
    ff_bling_handler_rgb(&lightning);
    k_sleep(30);
  }

  ff_gfx_fill(COLOR_BLACK);
  ff_gfx_push_buffer();
  k_sleep(400);

  start = ff_util_millis();
  while (ff_util_millis() - start < 7000) {
    ff_bling_handler_doom(&doom);
    ff_gfx_print(1, 1, "DC27", COLOR_WHITE, COLOR_RED);
    ff_gfx_push_buffer();
    k_sleep(40);
  }

  ff_gfx_fill(COLOR_BLACK);
  ff_gfx_push_buffer();
}