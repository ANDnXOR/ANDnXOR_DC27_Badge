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
#include <posix/time.h>
#include <shell/shell.h>
#include <zephyr.h>

#define LOG_LEVEL 4
#include <logging/log.h>
LOG_MODULE_REGISTER(ff_time);

#include "ff_time.h"

static int64_t m_offset = 0;

/**
 * @brief Shell command that prints the current time in Vegas' timezone
 * @param shell		Pointer to shell context
 * @param argc		Number of command arguments
 * @param argv		Pointer to array of command arguments
 * @return 0 if successful
 */
static int __cmd_date(const struct shell *shell, size_t argc, char **argv) {
  if (argc > 1) {
    shell_print(shell, "LOL\r");
    return -1;
  }

  ff_date_time now_date;
  struct timespec now = ff_time_now_get();
  ff_time_vegas_adjustment(&now);
  ff_time_convert_to_date(now, &now_date);
  shell_print(shell, "%d/%d/%d - %02d:%02d:%02d\r", now_date.month,
              now_date.day, now_date.year, now_date.hours, now_date.minutes,
              now_date.seconds);

  return 0;
}

/**
 * @brief Get the current time
 * @return The current time since epoch
 */
inline struct timespec ff_time_now_get() {
  struct timespec curtime;
  clock_gettime(CLOCK_REALTIME, &curtime);
  // Adjust with our local offset
  curtime.tv_sec += (uint32_t)(m_offset / NSEC_PER_SEC);
  curtime.tv_nsec += (uint32_t)(m_offset % NSEC_PER_SEC);
  return curtime;
}

/**
 * @brief Set the current time since epoch
 * @param time_ms	: The current time since epoch
 */
void ff_time_now_set(const struct timespec *p_new_time) {
  // clock_settime() in zephyr only supports a single set - they have buggy
  // code. Until it's fixed we need to maintain our own offset
  LOG_DBG("Setting time to %d.%d", p_new_time->tv_sec, p_new_time->tv_nsec);
  // Get what we think is our time in nanoseconds
  struct timespec local_time = ff_time_now_get();
  int64_t local_nsec =
      (NSEC_PER_SEC * (int64_t)local_time.tv_sec) + (int64_t)local_time.tv_nsec;
  // Compute new time as nanoseconds
  int64_t new_nsec = (NSEC_PER_SEC * (int64_t)p_new_time->tv_sec) +
                     (int64_t)p_new_time->tv_nsec;
  // Find delta
  int64_t delta = new_nsec - local_nsec;
  // Adjust our local offset
  LOG_DBG("Delta time by %lld nsec", delta);
  if (delta > 0) {
    m_offset += delta;
  }
}

/**
 * @brief Convert POSIX timestamp to our own date struct
 * @param t      Native time structure
 * @param date   Pointer to date struct to hold the results
 **/
void ff_time_convert_to_date(struct timespec ts, ff_date_time *date) {
  uint32_t a;
  uint32_t b;
  uint32_t c;
  uint32_t d;
  uint32_t e;
  uint32_t f;

  // Ensure seconds are positive
  if (ts.tv_sec < 1) {
    ts.tv_sec = 0;
  }

  // Clear milliseconds
  date->milliseconds = 0;

  // Retrieve hours, minutes and seconds
  date->seconds = ts.tv_sec % 60;
  ts.tv_sec /= 60;
  date->minutes = ts.tv_sec % 60;
  ts.tv_sec /= 60;
  date->hours = ts.tv_sec % 24;
  ts.tv_sec /= 24;

  // Convert POSIX time to date
  a = (uint32_t)((4 * ts.tv_sec + 102032) / 146097 + 15);
  b = (uint32_t)(ts.tv_sec + 2442113 + a - (a / 4));
  c = (20 * b - 2442) / 7305;
  d = b - 365 * c - (c / 4);
  e = d * 1000 / 30601;
  f = d - e * 30 - e * 601 / 1000;

  // January and February are counted as months 13 and 14 of the previous year
  if (e <= 13) {
    c -= 4716;
    e -= 1;
  } else {
    c -= 4715;
    e -= 13;
  }

  // Retrieve year, month and day
  date->year = c;
  date->month = e;
  date->day = f;
}

/**
 * @brief Adjust a POSIX timestamp for Vegas during summer
 * @param p_time	Pointer to time structure to adjust
 */
void ff_time_vegas_adjustment(struct timespec *p_time) {
  p_time->tv_sec += FF_TIME_VEGAS_DST_UTC_OFFSET_SEC;
}

SHELL_CMD_REGISTER(WHENRIT, NULL, "WAT TIME IS IT", __cmd_date);