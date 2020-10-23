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

#ifndef DRIVERS_IQS333_H
#define DRIVERS_IQS333_H

#define IQS333_VALID_DEVICE_INFO 0x36

#define IQS333_REG_DEVICE_INFO 0x00
#define IQS333_REG_SYSTEM_FLAGS 0x01
#define IQS333_REG_WHEEL 0x02
#define IQS333_REG_TOUCH 0x03
#define IQS333_REG_COUNTS 0x04
#define IQS333_REG_LTA 0x05
#define IQS333_REG_MULTIPLIERS 0x06
#define IQS333_REG_COMPENSATION 0x07
#define IQS333_REG_PROX_SETTINGS 0x08
#define IQS333_REG_THRESHOLDS 0x09
#define IQS333_REG_TIMINGS 0x0A
#define IQS333_REG_TARGETS 0x0B
#define IQS333_REG_PWM_DUTY 0x0C
#define IQS333_REG_PWM_LIMIT 0x0D
#define IQS333_REG_ACTIVE_CHANNELS 0x0E
#define IQS333_REG_BUZZER 0x0F

#define IQS333_RESEED 0b00001000

#define CAPTOUCH_NO_TOUCH -1
#define CAPTOUCH_MAX_VALUE 2048

/**
 * Get the last touch value read
 * @return the last raw touch value read
 */
extern int16_t iqs333_touch_get();

/**
 * @brief Get the last left wheel reading
 * @return CAPTOUCH_NO_TOUCH if there isn't a current touch or a position 0 to
 * 2048 for representing the current finger position on wheel 1
 */
extern int16_t iqs333_wheel_left_get();

/**
 * @brief Get the last right wheel reading
 * @return CAPTOUCH_NO_TOUCH if there isn't a current touch or a position 0 to
 * 2048 for representing the current finger position on wheel 1
 */
extern int16_t iqs333_wheel_right_get();

#endif