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
LOG_MODULE_REGISTER(ff_bling_doom);

#include "../ff_bling.h"
#include "../ff_gfx.h"
#include "../ff_ui.h"
#include "../ff_util.h"
#include "ff_bling_doom.h"

color_rgb_t doom_shades[] = {(color_rgb_t){0x07, 0x07, 0x07}, (color_rgb_t){0x57, 0x17, 0x07},
                             (color_rgb_t){0x9F, 0x2F, 0x07}, (color_rgb_t){0xDF, 0x4F, 0x07},
                             (color_rgb_t){0xCF, 0x7F, 0x0F}, (color_rgb_t){0xC7, 0x97, 0x1F},
                             (color_rgb_t){0xBF, 0xA7, 0x27}, (color_rgb_t){0xB7, 0xB7, 0x37},
                             (color_rgb_t){0xFF, 0xFF, 0xFF}};

/**
 * @brief This is the handler that gets called every time a frame needs to be drawn
 * @param p_bling	Pointer to bling object to use to draw the current frame
 */
void ff_bling_handler_doom(bling_t* p_bling) {
  if ((p_bling->frame % 2) == 0) {
    // Generate a single random number then mask the bits to determine height of each flame, this
    // avoids 31 sys_rand32_get() calls
    uint32_t r = sys_rand32_get();
    for (int16_t x = 0; x < WIDTH; x++) {
      for (int16_t y = 0; y < HEIGHT; y++) {
        // Height is determined by the bit at that x location in the rand number
        int16_t h = ((r & (1 << x)) >> x) & 1;
        ff_gfx_draw_pixel(x, y, doom_shades[y - h]);
      }
    }
    ff_gfx_push_buffer();
  }
  p_bling->frame++;
}