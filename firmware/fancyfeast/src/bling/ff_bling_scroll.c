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

#include <posix/time.h>
#include <stdio.h>

#define LOG_LEVEL 4
#include <logging/log.h>
LOG_MODULE_REGISTER(ff_bling_scroll);

#include "../ff_bling.h"
#include "../ff_gfx.h"
#include "../ff_settings.h"
#include "../ff_time.h"
#include "../ff_ui.h"
#include "../ff_util.h"
#include "ff_bling_scroll.h"

// How long go to between switch phrases on the wall
#define SCROLL_BOUNCE_TIME 20

/**
 * @brief Bling handler for scrolling text effect
 * @param p_bling   Pointer to bling mode data for this effect
 */
void ff_bling_handler_scroll(bling_t *p_bling) {
  // Draw the text
  ff_gfx_fill(COLOR_BLACK);
  ff_gfx_print(p_bling->x, p_bling->y, (char *)p_bling->user_data,
               ff_gfx_color_hsv_to_rgb(p_bling->hue, 1.0, 1.0), COLOR_BLACK);
  ff_gfx_push_buffer();

  // Scroll
  p_bling->x--;

  // Wrap around
  int16_t width = ff_gfx_text_width((char *)p_bling->user_data);
  if (p_bling->x < (0 - (int16_t)width)) {
    p_bling->x = WIDTH;
  }

  // Detect touch and move x position to match it
  // if (ff_ui_is_touched_left()) {
  //   uint8_t touch_position = ff_ui_touch_left();
  //   // Determine total width we have to draw text (width of text + width of
  //   // display)
  //   int16_t max_width = width + WIDTH;
  //   // Determine relative position on 0 to 1 scale (reversed to make it feel
  //   // natural)
  //   float relative =
  //       (float)(FF_UI_TOUCH_MAX - touch_position) / (float)FF_UI_TOUCH_MAX;
  //   p_bling->x = (int16_t)((float)max_width * relative) - width;
  // }

  ff_bling_adjust_hue(p_bling);
  ff_settings_ptr_get()->bling_hue_scroll = p_bling->hue;
}

/**
 * @brief Bling handler for scrolling text effect, bouncing
 * @param p_bling   Pointer to bling mode data for this effect
 */
void ff_bling_handler_scroll_bounce(bling_t *p_bling) {
  uint8_t count = 0;
  for (uint8_t i = GRAFFITI_COUNT - 1; i >= 0; i--) {
    if (strlen(ff_settings_ptr_get()->wall[i]) > 0) {
      count = i + 1;
      break;
    }
  }

  // Make sure counter is okay
  if (count > GRAFFITI_COUNT) {
    count = 0;
  }

  // Make sure current frame wraps around
  if (p_bling->frame > (BLING_HZ * SCROLL_BOUNCE_TIME * count)) {
    p_bling->frame = 0;
  }

  // Get text off the wall
  char *text;
  if (count == 0) {
    text = "AND!XOR DC27";
  } else {
    text = ff_settings_ptr_get()
               ->wall[p_bling->frame / (BLING_HZ * SCROLL_BOUNCE_TIME)];
  }

  // LOG_DBG("Text='%s' count=%d frame=%d index=%d", text, count,
  // p_bling->frame,
  //         p_bling->frame / (BLING_HZ * SCROLL_BOUNCE_TIME));

  // Draw the text
  ff_gfx_fill(COLOR_BLACK);
  ff_gfx_print(p_bling->x, p_bling->y, text,
               ff_gfx_color_hsv_to_rgb(p_bling->hue, 1.0, 1.0), COLOR_BLACK);
  ff_gfx_push_buffer();

  // Bounce
  int16_t width = ff_gfx_text_width(text);
  // Move left
  if (p_bling->direction < 0) {
    p_bling->x--;
    if ((p_bling->x + width) < MIN(WIDTH, width)) {
      p_bling->direction = 1;
    }
  }
  // Move right
  else {
    p_bling->x++;
    if (p_bling->x > MAX(0, WIDTH - width)) {
      p_bling->direction = -1;
    }
  }

  // Fade
  p_bling->hue += 0.01;
  if (p_bling->hue >= 1) {
    p_bling->hue -= 1;
  }

  p_bling->frame++;
}

/**
 * @brief Bling handler for scrolling time effect
 * @param p_bling   Pointer to bling mode data for this effect
 */
void ff_bling_handler_scroll_time(bling_t *p_bling) {
  char time_str[32];
  char colon = ':';

  // Get current time and convert to a date object in Vegas timezone
  struct timespec now = ff_time_now_get();
  ff_date_time now_date;
  ff_time_vegas_adjustment(&now);
  ff_time_convert_to_date(now, &now_date);

  // Blinking colon
  if ((now_date.seconds & 2) == 0) {
    colon = ' ';
  }

  snprintf(time_str, 32, "%02d:%02d%c%02d", now_date.hours, now_date.minutes,
           colon, now_date.seconds);

  // Draw the text
  ff_gfx_fill(COLOR_BLACK);
  ff_gfx_print(p_bling->x, p_bling->y, time_str,
               ff_gfx_color_hsv_to_rgb(p_bling->hue, 1.0, 1.0), COLOR_BLACK);
  ff_gfx_push_buffer();

  // Scroll
  p_bling->x -= 1;

  // Wrap around
  int16_t width = ff_gfx_text_width(time_str);
  if (p_bling->x < (0 - (int16_t)width)) {
    p_bling->x = WIDTH;
  }

  // // Detect touch and move x position to match it
  // if (ff_ui_is_touched()) {
  //   int16_t touch_position = ff_ui_touch_left();
  //   // Determine total width we have to draw text (width of text + width of
  //   // display)
  //   int16_t max_width = width + WIDTH;
  //   // Determine relative position on 0 to 1 scale (reversed to make it feel
  //   // natural)
  //   float relative =
  //       (float)(FF_UI_TOUCH_MAX - touch_position) / (float)FF_UI_TOUCH_MAX;
  //   p_bling->x = (int16_t)((float)max_width * relative) - width;
  // }

  // Fade
  p_bling->hue += 0.01;
  if (p_bling->hue >= 1) {
    p_bling->hue -= 1;
  }
}