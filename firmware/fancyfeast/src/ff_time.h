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

#ifndef FF_TIME_H
#define FF_TIME_H

#include <posix/time.h>

//Vegas during summer is 7 hours behind UTC
#define FF_TIME_VEGAS_DST_UTC_OFFSET_SEC (-7 * 60 * 60)

typedef struct {
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hours;
  uint8_t minutes;
  uint8_t seconds;
  uint16_t milliseconds;
} ff_date_time;

/**
 * @brief Convert Unix timestamp to our own date struct
 * @param t      Unix timestamp
 * @param date   Pointer to date struct to hold the results
 **/
extern void ff_time_convert_to_date(struct timespec ts, ff_date_time *date);

/**
 * @brief Get the current time
 * @return The current time since epoce
 */
extern struct timespec ff_time_now_get();

/**
 * @brief Set the current time since epoch
 * @param time_ms	: The current time since epoch
 */
extern void ff_time_now_set(const struct timespec *p_new_time);

/**
 * @brief Adjust a POSIX timestamp for Vegas during summer
 * @param p_time	Pointer to time structure to adjust
 */
extern void ff_time_vegas_adjustment(struct timespec *p_time);

#endif