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
#ifndef IS31FL3741_H
#define IS31FL3741_H

#include "../ff_gfx.h"

/**
 * @brief Change the LED global current control
 */
extern void is31fl3741_gcc_set(uint8_t brightness);

/**
 * Test open / short status of all LEDs
 * @return 0 if successful, negative value if there was a failure
 */
extern int is31fl3741_open_short_detection();

/**
 * @brief Push the current frame buffer to the LED Matrix
 */
extern int is31fl3741_push_buffer();

/**
 * @brief Set the pwm value of a specific LED in the matrix
 * @param sw 	: Column number
 * @param cs 	: Row number
 * @param value	: PWM value (0 to 255) for the LED
 * @return Error code if anything happened
 */
extern int is31fl3741_set(uint8_t sw, uint8_t cs, uint8_t value);


/**
 * @brief Set the pwm value of a specific LED in the matrix based on physical layout
 * @param value	: PWM value (0 to 255) for the LED
 * @return Error code if anything happened
 */
extern int is31fl3741_set_physical(uint8_t row, uint8_t col, uint8_t value);
#endif