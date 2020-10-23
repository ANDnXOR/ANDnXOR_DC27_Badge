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
#include <stdio.h>
#include <stdlib.h>

#include <fs.h>
#include <posix/time.h>
#include <shell/shell.h>
#include <zephyr.h>
#define LOG_LEVEL 4
#include <logging/log.h>
LOG_MODULE_REGISTER(ff_bling);

#include "autoconf.h"
#include "bling/ff_bling_bouncing.h"
#include "bling/ff_bling_circles.h"
#include "bling/ff_bling_doom.h"
#include "bling/ff_bling_eyes.h"
#include "bling/ff_bling_fade.h"
#include "bling/ff_bling_flappy.h"
#include "bling/ff_bling_flash.h"
#include "bling/ff_bling_grid.h"
#include "bling/ff_bling_kit.h"
#include "bling/ff_bling_lazers.h"
#include "bling/ff_bling_lines.h"
#include "bling/ff_bling_matrix.h"
#include "bling/ff_bling_rainbow.h"
#include "bling/ff_bling_rgb.h"
#include "bling/ff_bling_scroll.h"
#include "bling/ff_bling_sin.h"
#include "bling/ff_bling_snake.h"
#include "bling/ff_bling_tunnel.h"
#include "ff_bling.h"
#include "ff_fs.h"
#include "ff_gfx.h"
#include "ff_intro.h"
#include "ff_social.h"
#include "ff_time.h"
#include "ff_ui.h"
#include "ff_unlocks.h"
#include "ff_util.h"
#include "mesh/ff_mesh_model_bling.h"
#include "mesh/ff_mesh_model_social.h"
#include "system.h"

#define MENU_DELAY 200
#define BLING_MODE_STACK_SIZE 8
#define TOUCH_MAX_DISTANCE 30

// Rudimentary stack of bling modes starting at index 0, statically allocating
// memory
static bling_t m_bling_modes[255];
// Jump table where bling modes and functions in jump table must be at same
// offset
static bling_handler_t m_bling_jmp_table[255];

static volatile int16_t m_curr_bling_index = 0;
static uint8_t m_bling_count = 0;

// Menu variables
static bool m_is_touched = false;
static int32_t m_touch_last = 0;
static int32_t m_touch_distance = 0;
static int32_t m_touch_bling_index_start = 0;

static void __handler_copycat(bling_t *p_bling);

/**
 * @brief Primary bling task that constainly animates the LED matrix
 */
static void __bling_task(void *id, void *unused1, void *unused2) {
  // Play the intro before starting up bling
  ff_intro_run();

  uint32_t start_time;
  while (1) {
    // Record when this frame rendering started
    start_time = ff_util_millis();

    // Execute the bling handler
    bling_t *p_bling = &m_bling_modes[m_curr_bling_index];
    m_bling_jmp_table[m_curr_bling_index](p_bling);

    // // If it's not forever bling and not the default bling, count time the
    // // remaining time
    // if (p_bling->run_time_ms != FF_BLING_RUN_TIME_FOREVER &&
    //     m_curr_bling_index > 0) {
    //   p_bling->run_time_ms -= BLING_PERIOD;

    //   // Out of time, pop bling (if we can)
    //   if (p_bling->run_time_ms < 0) {
    //     LOG_DBG("Times up for bling %d", p_bling->mode);
    //     if (m_curr_bling_index > 0) {
    //       m_curr_bling_index--;
    //     }
    //   }
    // }

    // Every frame check for social events
    ff_social_handler();

    // Handle menu changes
    if (ff_ui_is_touched_left()) {
      int32_t left_pos = ff_ui_touch_left();

      // If it's already being touched
      if (m_is_touched) {

        // Move menu relative to finger movement
        int32_t delta = left_pos - m_touch_last;

        // Correct for very fast movement or crossing 12 o clock
        if (delta < -128) {
          delta += 256;
        } else if (delta > 128) {
          delta -= 256;
        }

        // Calculate the total distance traveled
        m_touch_distance += delta;

        // If they move around the left wheel enough distance, unlock something
        if (abs(m_touch_distance) > FF_UNLOCK_DISTANCE_TRAVEL) {
          if (!FF_UNLOCK_VALIDATE(FF_UNLOCK_DISTANCE)) {
            FF_UNLOCK_SET(FF_UNLOCK_DISTANCE);
            FF_BLING_DEFAULT_SNAKE_RAINBOW(bling);
            ff_bling_mode_register(bling, ff_bling_handler_snake);
            ff_bling_mode_push(bling);
            ff_gfx_fill(COLOR_YELLOW);
            ff_gfx_push_buffer();
            k_sleep(3000);
            m_is_touched = false;
          }
        }

        // Calculate the number of "clicks" the finger moved
        int32_t clicks = (m_touch_distance / 32);
        int16_t last_bling_index = m_curr_bling_index;
        m_curr_bling_index =
            (m_touch_bling_index_start + clicks) % m_bling_count;
        if (m_curr_bling_index < 0) {
          m_curr_bling_index += m_bling_count;
        }

        // If they moved at all through the menu animate the light pipes and reset frame counter
        if (last_bling_index != m_curr_bling_index) {
          m_bling_modes[m_curr_bling_index].frame = 0;

          ff_gfx_draw_line(0, 0, WIDTH, 0, COLOR_WHITE);
          ff_gfx_draw_line(0, HEIGHT - 1, WIDTH, HEIGHT - 1, COLOR_WHITE);
          ff_gfx_push_buffer();
          k_sleep(MENU_DELAY);
          ff_gfx_draw_line(0, 0, WIDTH, 0, COLOR_BLACK);
          ff_gfx_draw_line(0, HEIGHT - 1, WIDTH, HEIGHT - 1, COLOR_BLACK);
          ff_gfx_push_buffer();
        }

        // Remember where we left off last frame
        m_touch_last = left_pos;
      }
      // Filter erroneous touches
      else {
        // Wait 50ms, if still held then set the flag
        k_sleep(50);
        m_is_touched = ff_ui_is_touched_left();
        m_touch_last = left_pos;
        m_touch_bling_index_start = m_curr_bling_index;
        m_touch_distance = 0;
      }
    }
    // Clear state
    else {
      m_is_touched = false;
    }

    if (!ff_settings_ptr_get()->bling_rager) {
      // Try to run at 20fps
      ff_util_sleep_until(start_time + BLING_PERIOD);
    }
  }
}

/**
 * @brief Shell command that plays a scroll bounce mode
 * @return 0 if successful
 */
static inline int __cmd_bling_play_bounce(const struct shell *shell,
                                          size_t argc, char **argv) {
  if (argc != 1) {
    shell_error(shell, "Invalid arguments");
    return -1;
  }
  FF_BLING_DEFAULT_SCROLL_BOUNCE(bounce, argv[1]);
  ff_bling_mode_push(bounce);
  return 0;
}

/**
 * @brief Shell command that plays circles bling mode
 * @return 0 if successful
 */
static inline int __cmd_bling_play_circles(const struct shell *shell,
                                           size_t argc, char **argv) {
  FF_BLING_DEFAULT_CIRCLES(bling);
  ff_bling_mode_push(bling);
  return 0;
}

/**
 * @brief Shell command that plays copycat bling mode
 * @return 0 if successful
 */
static inline int __cmd_bling_play_copy(const struct shell *shell, size_t argc,
                                        char **argv) {
  FF_BLING_DEFAULT_COPYCAT(bling);
  ff_bling_mode_push(bling);
  return 0;
}

/**
 * @brief Shell command that plays doom bling mode
 * @return 0 if successful
 */
static inline int __cmd_bling_play_doom(const struct shell *shell, size_t argc,
                                        char **argv) {
  FF_BLING_DEFAULT_DOOM(bling);
  ff_bling_mode_push(bling);
  return 0;
}

/**
 * @brief Shell command that plays fade bling mode
 * @return 0 if successful
 */
static inline int __cmd_bling_play_fade(const struct shell *shell, size_t argc,
                                        char **argv) {
  FF_BLING_DEFAULT_FADE(fade);
  ff_bling_mode_push(fade);
  return 0;
}

/**
 * @brief Shell command that plays flappy bling mode
 * @return 0 if successful
 */
static inline int __cmd_bling_play_flappy(const struct shell *shell,
                                          size_t argc, char **argv) {
  FF_BLING_DEFAULT_FLAPPY(flappy);
  ff_bling_mode_push(flappy);
  return 0;
}

/**
 * @brief Shell command that plays eyes bling mode
 * @return 0 if successful
 */
static inline int __cmd_bling_play_eyes(const struct shell *shell, size_t argc,
                                        char **argv) {
  FF_BLING_DEFAULT_EYES(eyes);
  ff_bling_mode_push(eyes);
  return 0;
}

/**
 * @brief Shell command that plays a flash mode
 * @return 0 if successful
 */
static inline int __cmd_bling_play_flash(const struct shell *shell, size_t argc,
                                         char **argv) {
  LOG_DBG("Playing flash ahhh");
  FF_BLING_DEFAULT_FLASH(flash);
  ff_bling_mode_push(flash);
  return 0;
}

/**
 * @brief Shell command that plays a flash pixel mode
 * @return 0 if successful
 */
static inline int __cmd_bling_play_flash_pixel(const struct shell *shell,
                                               size_t argc, char **argv) {
  FF_BLING_DEFAULT_FLASH_PIXEL(flash);
  ff_bling_mode_push(flash);
  return 0;
}

/**
 * @brief Shell command that plays a kit mode
 * @return 0 if successful
 */
static inline int __cmd_bling_play_kit(const struct shell *shell, size_t argc,
                                       char **argv) {
  FF_BLING_DEFAULT_KIT(kit);
  ff_bling_mode_push(kit);
  return 0;
}

/**
 * @brief Shell command that plays lazers mode
 * @return 0 if successful
 */
static inline int __cmd_bling_play_lazers(const struct shell *shell,
                                          size_t argc, char **argv) {
  FF_BLING_DEFAULT_LAZERS(bling);
  ff_bling_mode_push(bling);
  return 0;
}

/**
 * @brief Shell command that plays a matrix mode
 * @return 0 if successful
 */
static inline int __cmd_bling_play_matrix(const struct shell *shell,
                                          size_t argc, char **argv) {
  FF_BLING_DEFAULT_MATRIX(matrix);
  ff_bling_mode_push(matrix);
  return 0;
}

/**
 * @brief Shell command that does nothing
 * @return 0 if successful
 */
// static inline int __cmd_bling_play_none(const struct shell *shell, size_t
// argc,
//                                         char **argv) {
//   FF_BLING_DEFAULT_NONE(none);
//   ff_bling_mode_push(none);
//   return 0;
// }

/**
 * @brief Shell command that plays rainbow mode
 * @return 0 if successful
 */
static inline int __cmd_bling_play_rainbow(const struct shell *shell,
                                           size_t argc, char **argv) {
  FF_BLING_DEFAULT_RAINBOW(rainbow);
  ff_bling_mode_push(rainbow);
  return 0;
}

/**
 * @brief Shell command that plays a RGB data
 * @return 0 if successful
 */
static inline int __cmd_bling_play_rgb(const struct shell *shell, size_t argc,
                                       char **argv) {
  FF_BLING_DEFAULT_RGB(bling, FF_BLING_DEFAULT_RGB_MODE);
  ff_bling_mode_push(bling);
  return 0;
}

/**
 * @brief Shell command that plays a flash mode
 * @return 0 if successful
 */
static inline int __cmd_bling_play_scroll(const struct shell *shell,
                                          size_t argc, char **argv) {
  if (argc != 2) {
    shell_error(shell, "Invalid arguments");
    return -1;
  }
  if (strlen(argv[1]) < FF_BLING_USER_DATA_SIZE) {
    // DC619 unlock
    if (!FF_UNLOCK_VALIDATE(FF_UNLOCK_DC619)) {
      if (strcmp(argv[1], "DC619") == 0) {
        FF_UNLOCK_SET(FF_UNLOCK_DC619);
      }
    }

    FF_BLING_DEFAULT_SCROLL(scroll, argv[1]);
    ff_bling_mode_push(scroll);
  } else {
    shell_error(shell, "String too long");
    return -1;
  }
  return 0;
}

/**
 * @brief Shell command that plays sin bling mode
 * @return 0 if successful
 */
static inline int __cmd_bling_play_sin(const struct shell *shell, size_t argc,
                                       char **argv) {
  FF_BLING_DEFAULT_SIN(sin);
  ff_bling_mode_push(sin);
  return 0;
}

/**
 * @brief Shell command that plays a flash mode
 * @return 0 if successful
 */
static inline int __cmd_bling_play_time(const struct shell *shell, size_t argc,
                                        char **argv) {
  FF_BLING_DEFAULT_SCROLL_TIME(t);
  ff_bling_mode_push(t);
  return 0;
}

/**
 * @brief Shell command that toggles rager mode
 * @param shell		Pointer to shell context
 * @param argc		Number of command arguments
 * @param argv		Pointer to array of command arguments
 * @return 0 if successful
 */
static int __cmd_bling_rager(const struct shell *shell, size_t argc,
                             char **argv) {

  if (FF_UNLOCK_VALIDATE(FF_UNLOCK_WHOAMI)) {
    ff_settings_ptr_get()->bling_rager = !ff_settings_ptr_get()->bling_rager;
    if (ff_settings_ptr_get()->bling_rager) {
      shell_print(shell, "Rager mode enabled");
    } else {
      shell_print(shell, "Rager mode disabled");
    }
    ff_settings_save();
  } else {
    shell_error(shell, "Rager mode locked");
  }
  return 0;
}

/**
 * @brief Shell command that prints the current bling stack
 * @param shell		Pointer to shell context
 * @param argc		Number of command arguments
 * @param argv		Pointer to array of command arguments
 * @return 0 if successful
 */
// static int __cmd_bling_stack(const struct shell *shell, size_t argc,
//                              char **argv) {
//   for (uint8_t i = 0; i <= m_curr_bling_index; i++) {
//     shell_print(shell, "%d: %s", i, ff_bling_name(m_bling_modes[i].mode));
//   }
//   return 0;
// }

/**
 * @brief Shell command that stops a bling mode
 * @param shell		Pointer to shell context
 * @param argc		Number of command arguments
 * @param argv		Pointer to array of command arguments
 * @return 0 if successful
 */
// static int __cmd_bling_stop(const struct shell *shell, size_t argc,
//                             char **argv) {
//   ff_bling_mode_pop();
//   shell_print(shell, "Stopping bling mode.");
//   return 0;
// }

/**
 * @brief Bling handler that copies nearby badges
 * @param p_bling   Pointer to bling mode data for this effect
 */
static void __handler_copycat(bling_t *p_bling) {

  //As long as it's being touched indicate
  if (ff_ui_is_touched_left() || ff_ui_is_touched_right()) {
    ff_gfx_fill(COLOR_BLACK);
    ff_gfx_print(1,1,"COPY",COLOR_YELLOW,COLOR_BLACK);
    ff_gfx_push_buffer();
    return;
  }
 
  // get a pointer to the last received bling mode
  bling_t *p_mode = ff_mesh_model_bling_state_get();

  // Search for, and jump to the bling mode copied
  for (uint8_t i = 0; i < m_bling_count; i++) {
    if (m_bling_modes[i].mode == p_mode->mode) {
      m_bling_jmp_table[i](p_mode);
      return;
    }
  }

  // If we got this far, the copycat bling mdoe was not registered, fallback
  m_bling_jmp_table[0](p_mode);
}

/**
 * @brief Get a pointer to the current bling mode
 * @return A pointer to the current bling mode
 */
bling_t *ff_bling_current_mode_get() {
  return &m_bling_modes[m_curr_bling_index];
}

/**
 * @brief Adjust the hue based on the current touch of the right wheel
 * @param p_bling   Pointer to the bling mode to adjust
 */
void ff_bling_adjust_hue(bling_t *p_bling) {
  // Detect touch on right wheel and adjust hue to match it
  if (ff_ui_is_touched_right()) {
    uint8_t touch_position = ff_ui_touch_right();
    p_bling->hue =
        (float)(FF_UI_TOUCH_MAX - touch_position) / (float)FF_UI_TOUCH_MAX;
  }
}

/**
 * @brief Pop the current bling mode off the stack
 */
void ff_bling_mode_pop() {
  // if (m_curr_bling_index >= 1) {
  //   // Clear the top item on the stack then pop
  //   memset(&m_bling_stack[m_curr_bling_index], 0, sizeof(bling_t));
  //   m_curr_bling_index--;
  // } else {
  //   LOG_ERR("Cannot pop last bling mode");
  // }
}

/**
 * @brief Set the current bling mode. This will interrupt the current bling mode
 * at the next opportunity and not return to the previous. If stack is full, the
 * new mode will not be pushed or run
 * @param mode	Bling mode to switch to, pushing current running bling
 * mode onto stack
 */
void ff_bling_mode_push(bling_t bling) {
  // Search through registered bling modes, if mode is found switch to it
  for (uint8_t i = 0; i < m_bling_count; i++) {
    if (m_bling_modes[i].mode == bling.mode) {
      LOG_DBG("Found bling mode %d at index %d", bling.mode, i);

      memcpy(&m_bling_modes[i], &bling, sizeof(bling_t));
      m_curr_bling_index = i;
      return;
    }
  }

  LOG_ERR("Unable to push bling mode '%s'", ff_bling_name(bling.mode));
}

/**
 * @brief Register a bling mode with the badge
 * @param bling   Initial state for the bling mode
 * @param handler Bling handler function to call every frame
 */
void ff_bling_mode_register(bling_t bling, bling_handler_t handler) {
  LOG_DBG("Registering %d: %s", m_bling_count, ff_bling_name(bling.mode));
  m_bling_modes[m_bling_count] = bling;
  m_bling_jmp_table[m_bling_count] = handler;
  m_bling_count++;
}

/**
 * @brief Init the bling thread that will handle all bling related tasks
 */
void ff_bling_init() {
  char text[128];
  snprintf(text, 128, "DEFCON 27 * AND!XOR * %s", ff_settings_ptr_get()->name);

  FF_BLING_DEFAULT_BOUNCING(bouncing);
  FF_BLING_DEFAULT_CIRCLES(circles);
  FF_BLING_DEFAULT_COPYCAT(copy);
  FF_BLING_DEFAULT_DOOM(doom);
  FF_BLING_DEFAULT_EYES(eyes);
  FF_BLING_DEFAULT_FADE(fade);
  FF_BLING_DEFAULT_FLAPPY(flappy);
  FF_BLING_DEFAULT_FLASH(flash);
  FF_BLING_DEFAULT_FLASH_PIXEL(flash_pixel);
  FF_BLING_DEFAULT_GRID(grid);
  FF_BLING_DEFAULT_KIT(kit);
  FF_BLING_DEFAULT_LAZERS(lazers);
  FF_BLING_DEFAULT_LINES(lines);
  FF_BLING_DEFAULT_MATRIX(matrix);
  FF_BLING_DEFAULT_RAINBOW(rainbow);
  FF_BLING_DEFAULT_RGB(rgb, bling_rgb_mode_colors9);
  FF_BLING_DEFAULT_SCROLL(scroll, text);
  FF_BLING_DEFAULT_SCROLL_BOUNCE(bounce, text);
  FF_BLING_DEFAULT_SCROLL_TIME(time);
  FF_BLING_DEFAULT_SIN(sin);
  FF_BLING_DEFAULT_SNAKE(snake);
  FF_BLING_DEFAULT_SNAKE_RAINBOW(snake_rainbow);
  FF_BLING_DEFAULT_TUNNEL(tunnel);

  ff_bling_mode_register(lazers, ff_bling_handler_lazers);
  ff_bling_mode_register(circles, ff_bling_handler_circles);
  ff_bling_mode_register(copy, __handler_copycat);
  ff_bling_mode_register(doom, ff_bling_handler_doom);
  ff_bling_mode_register(fade, ff_bling_handler_fade);
  ff_bling_mode_register(flash, ff_bling_handler_flash);
  ff_bling_mode_register(flash_pixel, ff_bling_handler_flash_pixel);
  ff_bling_mode_register(grid, ff_bling_handler_grid);
  ff_bling_mode_register(kit, ff_bling_handler_kit);
  ff_bling_mode_register(lines, ff_bling_handler_lines);
  ff_bling_mode_register(matrix, ff_bling_handler_matrix);
  ff_bling_mode_register(rainbow, ff_bling_handler_rainbow);
  ff_bling_mode_register(rgb, ff_bling_handler_rgb);
  ff_bling_mode_register(scroll, ff_bling_handler_scroll);
  ff_bling_mode_register(bounce, ff_bling_handler_scroll_bounce);
  ff_bling_mode_register(time, ff_bling_handler_scroll_time);
  ff_bling_mode_register(sin, ff_bling_handler_sin);
  ff_bling_mode_register(snake, ff_bling_handler_snake);

  if (FF_UNLOCK_VALIDATE(FF_UNLOCK_DISTANCE)) {
    ff_bling_mode_register(snake_rainbow, ff_bling_handler_snake);
  }

  if (FF_UNLOCK_VALIDATE(FF_UNLOCK_AUDIO)) {
    ff_bling_mode_register(bouncing, ff_bling_handler_bouncing);
  }

  if (FF_UNLOCK_VALIDATE(FF_UNLOCK_C2)) {
    ff_bling_mode_register(tunnel, ff_bling_handler_tunnel);
  }

  if (FF_UNLOCK_VALIDATE(FF_UNLOCK_EYES)) {
    ff_bling_mode_register(eyes, ff_bling_handler_eyes);
  }

  if (FF_UNLOCK_VALIDATE(FF_UNLOCK_FLAPPY)) {
    ff_bling_mode_register(flappy, ff_bling_handler_flappy);
  }
}

/**
 * @brief Get the name of the current bling mode
 */
char *ff_bling_name(bling_mode_t mode) {
  switch (mode) {
  case ff_bling_mode_none:
    return "None";
  case ff_bling_mode_bouncing:
    return "Bouncing Balls";
  case ff_bling_mode_circles:
    return "Circles";
  case ff_bling_mode_copycat:
    return "Copycat";
  case ff_bling_mode_doom:
    return "Doom";
  case ff_bling_mode_eyes:
    return "Eyes";
  case ff_bling_mode_fade:
    return "Fade";
  case ff_bling_mode_flappy:
    return "Flappy";
  case ff_bling_mode_flash:
    return "Flash";
  case ff_bling_mode_grid:
    return "Grid";
  case ff_bling_mode_lines:
    return "Lines";
  case ff_bling_mode_flash_pixel:
    return "Pixels";
  case ff_bling_mode_kit:
    return "Kit";
  case ff_bling_mode_lazers:
    return "Lazers";
  case ff_bling_mode_matrix:
    return "Matrix";
  case ff_bling_mode_rainbow:
    return "Rainbow";
  case ff_bling_mode_rgb:
    return "RGB";
  case ff_bling_mode_scroll:
    return "Scroll Text";
  case ff_bling_mode_scroll_bounce:
    return "Scroll Text Bounce";
  case ff_bling_mode_scroll_time:
    return "Scroll Time";
  case ff_bling_mode_snake:
    return "Snake";
  case ff_bling_mode_tunnel:
    return "Tunnel";
  default:
    return "Unknown";
  }
}

SHELL_STATIC_SUBCMD_SET_CREATE(
    sub_bling_play, SHELL_CMD(BOUNCING, NULL, "", ff_bling_cmd_play_bouncing),
    SHELL_CMD(BOUNCY, NULL, "", __cmd_bling_play_bounce),
    SHELL_CMD(ROUND, NULL, "", __cmd_bling_play_circles),
    SHELL_CMD(COPYCAT, NULL, "", __cmd_bling_play_copy),
    SHELL_CMD(DOOM, NULL, "", __cmd_bling_play_doom),
    SHELL_CMD(EYEZ, NULL, "", __cmd_bling_play_eyes),
    SHELL_CMD(FADE, NULL, "", __cmd_bling_play_fade),
    SHELL_CMD(FLAPPY, NULL, "", __cmd_bling_play_flappy),
    SHELL_CMD(FLASHAHHHHH, NULL, "", __cmd_bling_play_flash),
    SHELL_CMD(DOTZ, NULL, "", __cmd_bling_play_flash_pixel),
    SHELL_CMD(KIT, NULL, "", __cmd_bling_play_kit),
    SHELL_CMD(LAZERS, NULL, "", __cmd_bling_play_lazers),
    SHELL_CMD(LINES, NULL, "", ff_bling_cmd_play_lines),
    SHELL_CMD(MATRIX, NULL, "", __cmd_bling_play_matrix),
    // SHELL_CMD(NADA, NULL, "", __cmd_bling_play_none),
    SHELL_CMD(RAINBOWZ, NULL, "", __cmd_bling_play_rainbow),
    SHELL_CMD(PURTY, NULL, "", __cmd_bling_play_rgb),
    SHELL_CMD(SNAKE, NULL, "", ff_bling_cmd_play_snake),
    SHELL_CMD(WERDZ, NULL, "", __cmd_bling_play_scroll),
    SHELL_CMD(CLOCK, NULL, "", __cmd_bling_play_time),
    SHELL_CMD(TUNNELZ, NULL, "", ff_bling_cmd_play_tunnel),
    SHELL_CMD(WAVE, NULL, "", __cmd_bling_play_sin), SHELL_SUBCMD_SET_END);

SHELL_STATIC_SUBCMD_SET_CREATE(
    sub_bling, SHELL_CMD(WIT, &sub_bling_play, "PLAI BLING", NULL),
    SHELL_CMD(WITWITWIT, NULL, "ENABLE RAGER MOED", __cmd_bling_rager),
    // SHELL_CMD(WITS, NULL, "Print current bling stack", __cmd_bling_stack),
    // SHELL_CMD(WITOUT, NULL, "Stop a bling mode", __cmd_bling_stop),
    SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(RICKROLL, &sub_bling, "BLINGZ", NULL);

#ifndef CONFIG_ANDNXOR_TEST_FIRMWARE
K_THREAD_DEFINE(bling, 1400, __bling_task, NULL, NULL, NULL,
                FF_THREAD_PRIORITY_MEDIUM, 0, 1000);
#endif