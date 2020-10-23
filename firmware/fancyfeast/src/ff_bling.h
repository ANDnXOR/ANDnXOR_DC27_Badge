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
#ifndef FF_BLING_H
#define FF_BLING_H

#define BLING_HZ 20
#define BLING_PERIOD (1000 / BLING_HZ)
#define FF_BLING_USER_DATA_SIZE 64

typedef enum {
  ff_bling_mode_none,
  ff_bling_mode_bouncing,
  ff_bling_mode_circles,
  ff_bling_mode_copycat,
  ff_bling_mode_doom, // 4
  ff_bling_mode_eyes,
  ff_bling_mode_fade,
  ff_bling_mode_flappy,
  ff_bling_mode_flash,
  ff_bling_mode_flash_pixel,
  ff_bling_mode_grid, // 10
  ff_bling_mode_kit,
  ff_bling_mode_lazers,
  ff_bling_mode_lines,
  ff_bling_mode_matrix,
  ff_bling_mode_rainbow, // 15
  ff_bling_mode_rgb,
  ff_bling_mode_scroll,
  ff_bling_mode_scroll_bounce,
  ff_bling_mode_scroll_time,
  ff_bling_mode_sin, // 20
  ff_bling_mode_snake,
  ff_bling_mode_tunnel,
  __bling_mode_counter
} bling_mode_t;

/**
 * Portable struct to hold all data necessary for a bling mode. This should be
 * able to be sent to another badge and that badge can continue its state
 */
typedef struct {
  float hue;
  int8_t direction;
  int16_t x, y;
  uint32_t frame;  // Current frame number
  uint32_t frames; // Counter of number of frames left in animation (loop when
                   // it reaches 0)
  char user_data[FF_BLING_USER_DATA_SIZE];
  bling_mode_t mode;
} __packed bling_t;

/**
 * @brief Callback prototype for each step of a bling mode
 */
typedef void (*bling_handler_t)(bling_t *p_bling);

/**
 * @brief Adjust the hue based on the current touch of the right wheel
 * @param p_bling   Pointer to the bling mode to adjust
 */
extern void ff_bling_adjust_hue(bling_t *p_bling);

/**
 * @brief Get a pointer to the current bling mode
 * @return A pointer to the current bling mode
 */
extern bling_t *ff_bling_current_mode_get();

/**
 * @brief Initialize the bling module
 */
extern void ff_bling_init();

/**
 * @brief Pop the current bling mode off the stack
 */
extern void ff_bling_mode_pop();

/**
 * @brief Set the current bling mode. This will interrupt the current bling mode
 * at the next opportunity and not return to the previous. If stack is full, the
 * new mode will not be pushed or run
 * @param mode	Bling mode to switch to, pushing current running bling mode onto
 * stack
 */
extern void ff_bling_mode_push(bling_t bling);

/**
 * @brief Register a bling mode with the badge
 * @param bling   Initial state for the bling mode
 * @param handler Bling handler function to call every frame
 */
extern void ff_bling_mode_register(bling_t bling, bling_handler_t handler);

/**
 * @brief Get the name of the current bling mode
 */
extern char *ff_bling_name(bling_mode_t mode);

/**
 * @brief Macro for defining a default mode for copycat bling mode
 * user_data[0] is used as a flag to know if this is the first time running it
 * @param[in] _name		Instance name
 */
#define FF_BLING_DEFAULT_COPYCAT(_name)                                        \
  bling_t _name = {.mode = ff_bling_mode_copycat, .frame = 0};

#endif