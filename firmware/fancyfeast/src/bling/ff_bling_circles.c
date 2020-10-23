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
LOG_MODULE_REGISTER(ff_bling_circles);

#include "../ff_bling.h"
#include "../ff_gfx.h"
#include "../ff_ui.h"
#include "../ff_util.h"
#include "ff_bling_circles.h"

/**
 * @brief This is the handler that gets called every time a frame needs to be
 * drawn
 * @param p_bling	Pointer to bling object to use to draw the current frame
 */
void ff_bling_handler_circles(bling_t *p_bling) {
  ff_gfx_fill(COLOR_BLACK);

  // Angle is degrees but to fit into 8 bits is divided by 2 so 0-179 are valid
  // Unpack the data
  uint16_t angle = p_bling->user_data[0];
  int8_t x, y;     // center coord
  int8_t xx, yy;   // One extreme
  color_rgb_t rgb = ff_gfx_color_hsv_to_rgb(p_bling->hue, 1.0, 1.0);

  // Left eye
  x = 3;
  y = 4;
  xx = x + SIN_LUT_LARGE[angle / 5] - SIN_LUT_LARGE_AMPLITUDE;
  yy = y + SIN_LUT_LARGE[((angle + 45) % 180) / 5] - SIN_LUT_LARGE_AMPLITUDE;
  ff_gfx_draw_line(x, y, xx, yy, rgb);

  // Right Eye
  x = 13;
  y = 4;
  xx = x + SIN_LUT_LARGE[angle / 5] - SIN_LUT_LARGE_AMPLITUDE;
  yy = y + SIN_LUT_LARGE[((angle + 45) % 180) / 5] - SIN_LUT_LARGE_AMPLITUDE;
  ff_gfx_draw_line(x, y, xx, yy, rgb);

  // Increment and wrap around
  angle = (angle + 5) % 180;
  // Pack the data back into the user data
  p_bling->user_data[0] = angle;

  p_bling->hue += 0.01;
  if (p_bling->hue >= 1.0) {
    p_bling->hue -= 1.0;
  }

  ff_gfx_push_buffer();
}