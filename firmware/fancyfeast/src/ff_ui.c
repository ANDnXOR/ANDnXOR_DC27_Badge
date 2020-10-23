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

#define LOG_LEVEL 4
#include <logging/log.h>
LOG_MODULE_REGISTER(ff_menu);

#include "autoconf.h"
#include "drivers/iqs333.h"
#include "ff_ui.h"

#define MENU_ITEM_MAX 255

/**
 * Determine if a captouch pad is being touched
 */
bool ff_ui_is_touched() {
#ifdef CONFIG_IQS333
  int16_t touch = iqs333_touch_get();
  if (touch > 0x01) {
    LOG_DBG("Touch value = 0x%04x", touch);
  }
  return (touch > 0x01);
#else
  return 0;
#endif
}

/**
 * Determine if button is being touched
 */
bool ff_ui_is_touched_button() {
#ifdef CONFIG_IQS333
  return (iqs333_touch_get() & FF_UI_TOUCH_MASK_BUTTON) > 0;
#else
  return 0;
#endif
}

/**
 * Determine if left wheel is being touched
 */
bool ff_ui_is_touched_left() {
#ifdef CONFIG_IQS333
  return (iqs333_touch_get() & FF_UI_TOUCH_MASK_LEFT) > 0;
#else
  return 0;
#endif
}

/**
 * Determine if right wheel is being touched
 */
bool ff_ui_is_touched_right() {
#ifdef CONFIG_IQS333
  return (iqs333_touch_get() & FF_UI_TOUCH_MASK_RIGHT) > 0;
#else
  return 0;
#endif
}

/**
 * Get value of the right wheel 0 to 255 with 0 at 0 degrees
 */
uint8_t ff_ui_touch_right() {
#ifdef CONFIG_IQS333
  return (uint8_t)(iqs333_wheel_right_get() >> 3);
#else
  return 0;
#endif
}

/**
 * Get value of the left wheel 0 to 255 with 0 at 0 degrees
 */
uint8_t ff_ui_touch_left() {
#ifdef CONFIG_IQS333
  return (uint8_t)(iqs333_wheel_left_get() >> 3);
#else
  return 0;
#endif
}