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
LOG_MODULE_REGISTER(ff_bling_rainbow);

#include "../ff_bling.h"
#include "../ff_gfx.h"
#include "../ff_ui.h"
#include "../ff_util.h"
#include "ff_bling_rainbow.h"

#define RESOLUTION 5.0

// Look up table to re-map bad angles that cause pixel issues
static const uint8_t ANGLE_LUT[] = {
    0,  1,  1,  4,  4,  5,  5,  8,  8,  9,  10, 10, 13, 13, 14, 14, 14, 18,
    18, 19, 19, 22, 22, 23, 23, 26, 26, 27, 28, 28, 28, 31, 32, 32, 32, 0};

/**
 * @brief This is the handler that gets called every time a frame needs to be
 * drawn
 * @param p_bling	Pointer to bling object to use to draw the current frame
 */
void ff_bling_handler_rainbow(bling_t *p_bling) {
  while (p_bling->hue >= 1) {
    p_bling->hue--;
  }

  // Angle is stored as angle (degrees) / 10
  uint8_t angle = p_bling->user_data[0];
  if (angle > 35) {
    angle = FF_BLING_RAINBOW_DEFAULT_ANGLE;
  }

  // Compute a line to draw the center points along
  float sx1 = (WIDTH / 2) + (float)SIN_LUT_VERY_LARGE[(angle + 9) % 36] -
              SIN_LUT_VERY_LARGE_AMPLITUDE - 1;
  float sy1 = (HEIGHT / 2) + (float)SIN_LUT_VERY_LARGE[(angle) % 36] -
              SIN_LUT_VERY_LARGE_AMPLITUDE - 1;
  float sx2 = (WIDTH / 2) + (float)SIN_LUT_VERY_LARGE[(angle + 27) % 36] -
              SIN_LUT_VERY_LARGE_AMPLITUDE - 1;
  float sy2 = (HEIGHT / 2) + (float)SIN_LUT_VERY_LARGE[(angle + 18) % 36] -
              SIN_LUT_VERY_LARGE_AMPLITUDE - 1;

  float cxstep = (sx2 - sx1) / (17.0 * RESOLUTION);
  float cystep = (sy2 - sy1) / (17.0 * RESOLUTION);
  float cx = sx1;
  float cy = sy1;

  ff_gfx_fill(COLOR_BLACK);
  // Move along the angle through the center of the screen
  for (float i = 0; i < 17; i += (1.0 / RESOLUTION)) {

    color_rgb_t rgb = ff_gfx_color_hsv_to_rgb(p_bling->hue, 1.0, 1.0);
    float x1 =
        cx + (float)(SIN_LUT_VERY_LARGE[angle] - SIN_LUT_VERY_LARGE_AMPLITUDE);
    float y1 = cy + (float)(SIN_LUT_VERY_LARGE[(angle + 27) % 36] -
                            SIN_LUT_VERY_LARGE_AMPLITUDE);
    float x2 = cx + (float)(SIN_LUT_VERY_LARGE[(angle + 18) % 36] -
                            SIN_LUT_VERY_LARGE_AMPLITUDE);
    float y2 = cy + (float)(SIN_LUT_VERY_LARGE[(angle + 9) % 36] -
                            SIN_LUT_VERY_LARGE_AMPLITUDE);

    ff_gfx_draw_line(x1, y1, x2, y2, rgb);

    // Dividing by resolution ensures it's phase locked with drawing.
    //.0003 controls the speed at which it changes
    p_bling->hue += (1.0 / (17.0 * RESOLUTION)) + .0003;
    cx += cxstep;
    cy += cystep;
  }

  ff_gfx_push_buffer();

  // Modify angle of rainbow
  if (ff_ui_is_touched_right()) {
    angle = (uint8_t)(36.0 * (255.0 - (float)ff_ui_touch_right()) / 255.0) * 1;
    p_bling->user_data[0] = ANGLE_LUT[angle];
  }
}