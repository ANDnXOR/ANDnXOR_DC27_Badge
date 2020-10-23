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
#include "shell/shell.h"

#define LOG_LEVEL 4
#include <logging/log.h>
LOG_MODULE_REGISTER(ff_bling_snake);

#include "../ff_bling.h"
#include "../ff_gfx.h"
#include "../ff_ui.h"
#include "../ff_unlocks.h"
#include "ff_bling_snake.h"

#define SNAKE_MAX_LEN 16
#define SNAKE_DIR_UP 0
#define SNAKE_DIR_DOWN 1
#define SNAKE_DIR_LEFT 2
#define SNAKE_DIR_RIGHT 3

int16_t snake_x[SNAKE_MAX_LEN] = {4, 4, 0, 0, 4, 4, 0, 0,
                                  4, 4, 0, 0, 4, 4, 0, 0};
int16_t snake_y[SNAKE_MAX_LEN] = {4, 4, 0, 0, 4, 4, 0, 0,
                                  4, 4, 0, 0, 4, 4, 0, 0};

/**
 * @brief Shell command that plays snake mode
 * @return 0 if successful
 */
int ff_bling_cmd_play_snake(const struct shell *shell, size_t argc,
                            char **argv) {
  if (FF_UNLOCK_VALIDATE(FF_UNLOCK_DISTANCE)) {
    FF_BLING_DEFAULT_SNAKE(bling);
    ff_bling_mode_push(bling);
    return 0;
  } else {
    shell_error(shell, "LOL");
    return -1;
  }
}

void ff_bling_handler_snake(bling_t *p_bling) {
  uint8_t len = MIN(SNAKE_MAX_LEN, p_bling->x);
  if (len == 0) {
    len = 1;
  }

  float hue = p_bling->hue;
  ff_gfx_fill(COLOR_BLACK);
  for (uint8_t i = 0; i < len; i++) {
    color_rgb_t rgb = ff_gfx_color_hsv_to_rgb(hue, 1.0, 1.0);
    ff_gfx_draw_pixel(snake_x[i], snake_y[i], rgb);

    if (p_bling->user_data[0] == FF_BLING_SNAKE_MODE_RAINBOW) {
      hue += 0.08;
      if (hue >= 1.0) {
        hue -= 1.0;
      }
    }
  }
  ff_gfx_push_buffer();

  //   LOG_DBG("x=%d y=%d nx=%d ny=%d", snake_x[0], snake_y[0], next_x, next_y);

  // Loop until we find a new direction
  while (1) {
    // Move the snake
    int16_t next_x = snake_x[0];
    int16_t next_y = snake_y[0];
    switch (p_bling->direction) {
    case SNAKE_DIR_UP:
      next_y--;
      break;
    case SNAKE_DIR_DOWN:
      next_y++;
      break;
    case SNAKE_DIR_LEFT:
      next_x--;
      break;
    case SNAKE_DIR_RIGHT:
      next_x++;
      break;
    }

    // Determine if we can move in the direction we want to
    if (next_x < 0 || next_x >= WIDTH || next_y < 1 || next_y >= HEIGHT - 1 ||
        (sys_rand32_get() % 100) < 10) {
      p_bling->direction = sys_rand32_get() % 4;
      // LOG_DBG("New Direction = %d", p_bling->direction);
    }
    // If we can move, then do so
    else {
      // Walk backwards along the snake
      for (uint8_t i = len - 1; i > 0; i--) {
        snake_x[i] = snake_x[i - 1];
        snake_y[i] = snake_y[i - 1];
      }
      snake_x[0] = next_x;
      snake_y[0] = next_y;
      // Since we found a new direction, stop looping
      break;
    }

    // Detect touch on left wheel and adjust length to match it
    if (ff_ui_is_touched_left()) {
      uint8_t touch_position = ff_ui_touch_left();
      len =
          (float)((FF_UI_TOUCH_MAX - touch_position) / (float)FF_UI_TOUCH_MAX) *
          (float)SNAKE_MAX_LEN;
      p_bling->x = len;
    }
  }

  ff_bling_adjust_hue(p_bling);
}