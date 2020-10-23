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
LOG_MODULE_REGISTER(ff_bling_lazers);

#include "../ff_bling.h"
#include "../ff_gfx.h"
#include "../ff_ui.h"
#include "../ff_util.h"
#include "ff_bling_lazers.h"

/**
 * @brief This is the handler that gets called every time a frame needs to be
 * drawn
 * @param p_bling	Pointer to bling object to use to draw the current frame
 */
void ff_bling_handler_lazers(bling_t *p_bling) {
  ff_gfx_fill(COLOR_BLACK);

  // Lazers are stored in user_data in the following format
  //[x][direction][hue][x][direction][hue][x][direction][hue]...[x][direction][hue]

  // Handle each lazer (skipping light pipe rows) by validating and drawing each
  for (uint8_t i = 0; i < (HEIGHT); i++) {
    int8_t x = p_bling->user_data[i * 3];
    int8_t direction = p_bling->user_data[(i * 3) + 1];
    float hue = (float)p_bling->user_data[(i * 3) + 2] / 255.0;

    // Validate direction
    if (direction == 0) {
      direction = 1;
    }

    // Flag that generates a new lazer
    bool new_lazer =
        ((direction < 0) && x < -16) || ((direction > 0) && x > (WIDTH + 16));

    // Generate a new lazer ensuring all are valid
    if (new_lazer) {
      // Generate 32 bits of entropy once then pull bits from there for
      // performance
      uint32_t rand = sys_rand32_get();
      uint8_t rand_h = rand & 0xFF;
      uint8_t rand_x = (rand >> 8) & 0xFF;
      uint8_t rand_d = (rand >> 16) & 0xFF;

      hue = (float)(rand_h % 255) / 255.0;
      if ((rand_d % 2) == 0) {
        direction = -1;
        x = (rand_x % 20) + WIDTH;
      } else {
        direction = 1;
        x = 0 - (rand_x % 20);
      }
    }

    // Now draw the lazer
    color_rgb_t rgb = ff_gfx_color_hsv_to_rgb(hue, 1.0, 1.0);
    if (direction < 0) {
      ff_gfx_draw_line(x, i, x + 8, i + 1, rgb);
      x -= 2;
    } else {
      ff_gfx_draw_line(x, i, x - 8, i + 1, rgb);
      x += 2;
    }

    // Store lazer data back
    p_bling->user_data[i * 3] = x;
    p_bling->user_data[(i * 3) + 1] = direction;
    p_bling->user_data[(i * 3) + 2] = (uint8_t)(hue * 255.0);
  }

  ff_gfx_push_buffer();
}