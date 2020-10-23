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
LOG_MODULE_REGISTER(ff_bling_tunnel);

#include "../ff_bling.h"
#include "../ff_gfx.h"
#include "../ff_ui.h"
#include "../ff_unlocks.h"
#include "../ff_util.h"
#include "ff_bling_tunnel.h"

/**
 * @brief Shell command that plays tunnel mode
 * @return 0 if successful
 */
int ff_bling_cmd_play_tunnel(const struct shell *shell, size_t argc,
                             char **argv) {
  if (FF_UNLOCK_VALIDATE(FF_UNLOCK_C2)) {
    FF_BLING_DEFAULT_TUNNEL(bling);
    ff_bling_mode_push(bling);
    return 0;
  } else {
    shell_error(shell, "LOL");
    return -1;
  }
}

/**
 * @brief This is the handler that gets called every time a frame needs to be
 * drawn
 * @param p_bling	Pointer to bling object to use to draw the current frame
 */
void ff_bling_handler_tunnel(bling_t *p_bling) {
  ff_gfx_fill(COLOR_BLACK);
  if (p_bling->x > 4) {
    p_bling->x = 4;
  }

  if (p_bling->frame % 2 == 0) {
    int16_t x1 = 4 - p_bling->x;
    int16_t x2 = 13 + p_bling->x;
    int16_t y1 = 4 - p_bling->x;
    int16_t y2 = 4 + p_bling->x;

    color_rgb_t rgb = ff_gfx_color_hsv_to_rgb(p_bling->hue, 1.0, 1.0);
    ff_gfx_draw_line(x1, y1, x2, y1, rgb);
    ff_gfx_draw_line(x1, y1, x1, y2, rgb);
    ff_gfx_draw_line(x1, y2, x2, y2, rgb);
    ff_gfx_draw_line(x2, y1, x2, y2, rgb);
    ff_gfx_push_buffer();

    p_bling->x = (p_bling->x + 1) % 5;
  }

  p_bling->frame++;

  ff_bling_adjust_hue(p_bling);
}