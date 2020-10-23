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
#define FF_UI_TOUCH_MAX 255
#define FF_UI_TOUCH_MASK_BUTTON 0x0080
#define FF_UI_TOUCH_MASK_RIGHT 0x0070
#define FF_UI_TOUCH_MASK_LEFT 0x000E

/**
 * Determine if button is being touched
 */
extern bool ff_ui_is_touched_button();

/**
 * Determine if left wheel is being touched
 */
extern bool ff_ui_is_touched_left();

/**
 * Determine if right wheel is being touched
 */
extern bool ff_ui_is_touched_right();

/**
 * Determine if a captouch pad is being touched
 */
extern bool ff_ui_is_touched();

/**
 * Get value of the right wheel 0 to 255 with 0 at 0 degrees
 */
extern uint8_t ff_ui_touch_right();

/**
 * Get value of the left wheel 0 to 255 with 0 at 0 degrees
 */
extern uint8_t ff_ui_touch_left();

/**
 * @brief Initialize the menu
 */
void ff_ui_menu_init();