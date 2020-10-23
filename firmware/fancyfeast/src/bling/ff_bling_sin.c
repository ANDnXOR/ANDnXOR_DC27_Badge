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

#define LOG_LEVEL 4
#include <logging/log.h>
LOG_MODULE_REGISTER(ff_bling_sin);

#include "../ff_bling.h"
#include "../ff_gfx.h"
#include "../ff_settings.h"
#include "../ff_ui.h"
#include "../ff_util.h"
#include "ff_bling_sin.h"

/**
 * @brief This is the handler that gets called every time a frame needs to be
 * drawn
 * @param p_bling	Pointer to bling object to use to draw the current frame
 */
void ff_bling_handler_sin(bling_t *p_bling) {
   uint8_t p2 = 8;
  color_rgb_t rgb1 = ff_gfx_color_hsv_to_rgb(p_bling->hue, 1.0, 1.0);
  color_rgb_t rgb2 = ff_gfx_color_hsv_to_rgb(p_bling->hue + 0.5, 1.0, 1.0);

  ff_gfx_fill(COLOR_BLACK);
  for (uint8_t x = 0; x < WIDTH; x++) {
    uint8_t y1 = SIN_LUT[(p_bling->x + x) % WIDTH] + 1;
    uint8_t y2 = SIN_LUT[(p_bling->x + x + p2) % WIDTH] + 1;

    ff_gfx_draw_pixel(x, y1, rgb1);
    ff_gfx_draw_pixel(x, y2, rgb2);
  }
  ff_gfx_push_buffer();

  // Advance x pointer
  p_bling->x = (p_bling->x + 1) % WIDTH;

  ff_bling_adjust_hue(p_bling);
  ff_settings_ptr_get()->bling_hue_sin = p_bling->hue;
}