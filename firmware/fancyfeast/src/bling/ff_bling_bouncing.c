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
LOG_MODULE_REGISTER(ff_bling_bouncing);

#include "../ff_bling.h"
#include "../ff_gfx.h"
#include "../ff_ui.h"
#include "../ff_unlocks.h"
#include "../ff_util.h"
#include "ff_bling_bouncing.h"

#define BALL_COUNT 15

/**
 * @brief Shell command that plays bouncing ball mode
 * @return 0 if successful
 */
int ff_bling_cmd_play_bouncing(const struct shell *shell, size_t argc,
                               char **argv) {
  if (FF_UNLOCK_VALIDATE(FF_UNLOCK_AUDIO)) {
    FF_BLING_DEFAULT_BOUNCING(bling);
    ff_bling_mode_push(bling);
    return 0;
  } else {
    shell_error(shell, "LOL");
    return -1;
  }

  return 0;
}

/**color_rgb_t rgb = ff_gfx_color_hsv_to_rgb(p_bling->hue, 1.0, 1.0);
 * @brief This is the handler that gets called every time a frame needs to be
 * drawn
 * @param p_bling	Pointer to bling object to use to draw the current frame
 */
void ff_bling_handler_bouncing(bling_t *p_bling) {
  ff_gfx_fill(COLOR_BLACK);

  // local copy of the hue so we can adjust the subsequent ball colors
  float hue = p_bling->hue;

  // Handle each ball
  for (uint8_t i = 0; i < BALL_COUNT; i++) {
    color_rgb_t rgb = ff_gfx_color_hsv_to_rgb(hue, 1.0, 1.0);

    // If the ball is too far off the right, move it left some random value
    if (p_bling->user_data[i] > WIDTH) {
      p_bling->user_data[i] = (uint8_t)(0 - (int8_t)(sys_rand32_get() % 60));
    }

    // Move to the right
    p_bling->user_data[i]++;

    int8_t x = p_bling->user_data[i];
    if (x >= 0) {
      uint8_t y = SIN_LUT[(x + i + 1) % SIN_LUT_LENGTH];
      ff_gfx_draw_pixel(x, y, rgb);
    }

    //Adjust ball hue
    hue += (0.5 / (float)BALL_COUNT);
    if (hue > 1.0) {
      hue -= 1.0;
    }
  }
  ff_gfx_push_buffer();
  ff_bling_adjust_hue(p_bling);
}