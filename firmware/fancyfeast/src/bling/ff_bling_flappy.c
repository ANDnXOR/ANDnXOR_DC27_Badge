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
LOG_MODULE_REGISTER(ff_bling_flappy);

#include "../ff_bling.h"
#include "../ff_gfx.h"
#include "../ff_ui.h"
#include "../ff_util.h"
#include "ff_bling_flappy.h"

#define FLAPPY_PLAYER_X 2
#define FLAPPY_GRAVITY 0.018
#define FLAPPY_PIPE_GAP 4
#define FLAPPY_PIPE_SPEED -0.2
#define FLAPPY_JUMP_VELOCITY -0.12

// Intentionally using static data here rather than bling state, this is a game
// and not well-suited for copycat
static float player_y = 2.0;
static float velocity = 0.0;

// Pipes start in static positions
static float pipe_x[4] = {10, 15, 20, 25};
static uint8_t pipe_y[4] = {3, 4, 3, 2};

/**
 * @brief This is the handler that gets called every time a frame needs to be
 * drawn
 * @param p_bling	Pointer to bling object to use to draw the current frame
 */
void ff_bling_handler_flappy(bling_t *p_bling) {
  bool collision = false;
  ff_gfx_fill(COLOR_BLACK);

  color_rgb_t rgb = ff_gfx_color_hsv_to_rgb(velocity, 1.0, 1.0);
  ff_gfx_draw_pixel(FLAPPY_PLAYER_X, player_y, rgb);

  // Draw pipes
  for (uint8_t i = 0; i < 4; i++) {
    color_rgb_t pipe_rgb = COLOR_GREEN;
    // Collision check
    if ((int8_t)pipe_x[i] == FLAPPY_PLAYER_X) {
      if ((player_y <= pipe_y[i]) ||
          (player_y >= pipe_y[i] + FLAPPY_PIPE_GAP)) {
        collision = true;
        pipe_rgb = rgb;
      }
    }

    ff_gfx_draw_line(pipe_x[i], 0, pipe_x[i], pipe_y[i], pipe_rgb);
    ff_gfx_draw_line(pipe_x[i], HEIGHT, pipe_x[i], pipe_y[i] + FLAPPY_PIPE_GAP,
                     pipe_rgb);

    // Move the pipe
    if (!collision) {
      pipe_x[i] += FLAPPY_PIPE_SPEED;
    }

    if (pipe_x[i] < 0) {
      pipe_x[i] = WIDTH + 3;
      pipe_y[i] = sys_rand32_get() % 5;
    }
  }

  // Make sure player did not fall through floor
  collision |= (player_y >= HEIGHT);

  // Apply physics?
  if (!collision) {
    player_y += velocity;
    velocity += FLAPPY_GRAVITY;
  }

  if (ff_ui_is_touched_right()) {
    velocity = FLAPPY_JUMP_VELOCITY;
  }

  // Animate and redraw
  if (collision) {
    for (uint8_t y = 0; y < HEIGHT; y++) {
      ff_gfx_draw_line(0, y, WIDTH, y, COLOR_RED);
      ff_gfx_push_buffer();
      k_sleep(100);
    }

    pipe_x[0] = 10;
    pipe_x[1] = 15;
    pipe_x[2] = 20;
    pipe_x[3] = 25;
    pipe_y[0] = 3;
    pipe_y[1] = 4;
    pipe_y[2] = 3;
    pipe_y[3] = 2;
    player_y = 2.0;
    velocity = 0;
  }

  ff_gfx_push_buffer();
}