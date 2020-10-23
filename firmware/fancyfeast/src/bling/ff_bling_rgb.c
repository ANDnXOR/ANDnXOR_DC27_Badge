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
LOG_MODULE_REGISTER(ff_bling_rgb);

#include "../ff_bling.h"
#include "../ff_gfx.h"
#include "../ff_ui.h"
#include "../ff_unlocks.h"
#include "ff_bling_rgb.h"

// Bling data
#include "bw1.h"
#include "colors4.h"
#include "colors9.h"
#include "lightning2.h"
#include "psycho2.h"
#include "red4.h"

/**
 * @brief Bling handler that plays an RGB animation
 * @param p_bling   Pointer to bling mode data for this effect
 */
void ff_bling_handler_rgb(bling_t *p_bling) {
  if (p_bling->user_data == NULL) {
    return;
  }

  // Remove last two rgb unless they unlocked something
  uint8_t rgb_count = __bling_rgb_mode_counter;
  if (!FF_UNLOCK_VALIDATE(FF_UNLOCK_AUDIO_PUZZLE)) {
    rgb_count -= 2;
  }

  bling_rgb_mode_t rgb_mode = p_bling->user_data[0];
  if (rgb_mode >= rgb_count) {
    rgb_mode = 0;
  }

  color_rgb_t *rgb_data = NULL;
  uint32_t byte_count = 0;
  switch (rgb_mode) {
  case bling_rgb_mode_bw1:
    byte_count = image_bw1_rgb_len;
    rgb_data = (color_rgb_t *)image_bw1_rgb;
    break;
  case bling_rgb_mode_colors4:
    byte_count = image_colors4_rgb_len;
    rgb_data = (color_rgb_t *)image_colors4_rgb;
    break;
  case bling_rgb_mode_colors9:
    byte_count = image_colors9_rgb_len;
    rgb_data = (color_rgb_t *)image_colors9_rgb;
    break;
  case bling_rgb_mode_psycho2:
    byte_count = image_psycho2_rgb_len;
    rgb_data = (color_rgb_t *)image_psycho2_rgb;
    break;
  case bling_rgb_mode_red4:
    byte_count = image_red4_rgb_len;
    rgb_data = (color_rgb_t *)image_red4_rgb;
    break;
  case bling_rgb_mode_lightning:
    byte_count = image_lightning2_rgb_len;
    rgb_data = (color_rgb_t *)image_lightning2_rgb;
    break;
  default:
    break;
  }

  // If frame counter is 0, calculate frames from size
  if (p_bling->frames == 0) {
    p_bling->frames = byte_count / FRAME_SIZE_BYTES;
  }

  // Make sure we don't overrun the frame counter
  if (p_bling->frame >= p_bling->frames) {
    p_bling->frame = 0;
  }

  // Seek to first pixel
  color_rgb_t *pixel = rgb_data + (WIDTH * HEIGHT * p_bling->frame);

  p_bling->frame++;

  // This is slow!
  for (int16_t x = 0; x < WIDTH; x++) {
    for (int16_t y = 0; y < HEIGHT; y++) {
      ff_gfx_draw_pixel(x, y, pixel[(y * WIDTH) + x]);
    }
  }

  // Detect touch on right wheel and change RGB mode to match it
  if (ff_ui_is_touched_right()) {
    uint8_t touch_position = ff_ui_touch_right();
    uint8_t step = FF_UI_TOUCH_MAX / rgb_count;
    rgb_mode = touch_position / step;
    p_bling->frames = 0;
  }

  // Save the current bling mode
  p_bling->user_data[0] = rgb_mode;

  // Update the LED matrix
  ff_gfx_push_buffer();
}