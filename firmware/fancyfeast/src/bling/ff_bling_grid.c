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
LOG_MODULE_REGISTER(ff_bling_grid);

#include "../ff_bling.h"
#include "../ff_gfx.h"
#include "../ff_ui.h"
#include "../ff_util.h"
#include "ff_bling_grid.h"

/**
 * @brief This is the handler that gets called every time a frame needs to be
 * drawn
 * @param p_bling	Pointer to bling object to use to draw the current frame
 */
void ff_bling_handler_grid(bling_t *p_bling) {
  // Skip frames, must be odd number so grid pattern works below
  if ((p_bling->frame % 5) == 0) {
    ff_gfx_fill(COLOR_BLACK);
    color_rgb_t rgb = ff_gfx_color_hsv_to_rgb(p_bling->hue, 1.0, 1.0);
    // x coordinate to start on
    uint8_t start_x = p_bling->frame % 2;
    for (uint8_t y = 0; y < HEIGHT; y++) {
      uint8_t xx = (start_x + (y % 2)) % 2;
      for (uint8_t x = xx; x < WIDTH; x += 2) {
        ff_gfx_draw_pixel(x, y, rgb);
      }
    }
    ff_gfx_push_buffer();
  }

  // We're using the frame counter to determine which frame to show
  p_bling->frame++;

  // Allow right wheel to adjust
  ff_bling_adjust_hue(p_bling);
}