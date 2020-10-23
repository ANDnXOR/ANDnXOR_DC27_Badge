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
LOG_MODULE_REGISTER(ff_bling_eyes);

#include "../ff_bling.h"
#include "../ff_gfx.h"
#include "../ff_ui.h"
#include "../ff_util.h"
#include "eyes.h"
#include "ff_bling_eyes.h"

/**
 * @brief This is the handler that gets called every time a frame needs to be
 * drawn
 * @param p_bling	Pointer to bling object to use to draw the current frame
 */
void ff_bling_handler_eyes(bling_t *p_bling) {
  // Bounce
  if (p_bling->x >= 5) {
    p_bling->x = 4;
    p_bling->direction = -1;
  } else if (p_bling->x < 0) {
    p_bling->x = 0;
    p_bling->direction = 1;
  }

  // Pick a eye frame
  uint8_t index = p_bling->user_data[0];
  if (index >= FF_BLING_EYES_COUNT) {
    index = 0;
  }
  uint8_t *bits[FF_BLING_EYES_COUNT] = {eye1_bits, eye2_bits, eye3_bits,
                                        eye4_bits};
  uint8_t *bmp = bits[index];

  // Pick a random hue
  color_rgb_t rgb = ff_gfx_color_hsv_to_rgb(p_bling->hue, 1.0, 1.0);

  ff_gfx_fill(COLOR_BLACK);

  // Draw left eye
  uint8_t y = 2;
  ff_gfx_draw_xbm(bmp, p_bling->x, y, FF_BLING_EYES_WIDTH, FF_BLING_EYES_HEIGHT,
                  rgb, COLOR_BLACK);

  // Draw right eye
  ff_gfx_draw_xbm(bmp, p_bling->x + (WIDTH / 2) + 1, y, FF_BLING_EYES_WIDTH,
                  FF_BLING_EYES_HEIGHT, rgb, COLOR_BLACK);

  ff_gfx_push_buffer();

  // Only update every n frames
  if (p_bling->frame % 10 == 0) {
    p_bling->x += p_bling->direction;
    // Pick a new eye
    if (sys_rand32_get() % 10 == 0) {
      p_bling->user_data[0] = sys_rand32_get() % FF_BLING_EYES_COUNT;
    }
  }

  p_bling->frame++;

  ff_bling_adjust_hue(p_bling);
}