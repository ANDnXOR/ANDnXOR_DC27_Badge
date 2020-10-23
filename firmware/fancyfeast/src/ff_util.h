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

#ifndef FF_UTIL_H
#define FF_UTIL_H

#define ABS(x) ((x) > 0 ? (x) : -(x))
// #define MAX(a, b) ((a) > (b) ? (a) : (b))
// #define MIN(a, b) ((a) < (b) ? (a) : (b))

// Precalculated SIN table, amplitude 6, 17 data points
static const uint8_t SIN_LUT[] = {
    0x3, 0x4, 0x5, 0x6, 0x6, 0x6, 0x5, 0x5, 0x4,
    0x2, 0x1, 0x1, 0x0, 0x0, 0x0, 0x1, 0x2, 0x3,
};
// Size of SIN look up table (intentionally -1 due to overlap)
#define SIN_LUT_LENGTH 17

// Precalculated SIN table, amplitude 8, 36 data points
static const uint8_t SIN_LUT_LARGE[] = {
    0x5, 0x6, 0x7, 0x8, 0x8, 0x9, 0x9, 0xa, 0xa, 0xa, 0xa, 0xa, 0x9,
    0x9, 0x8, 0x8, 0x7, 0x6, 0x5, 0x4, 0x3, 0x3, 0x2, 0x1, 0x1, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x1, 0x1, 0x2, 0x3, 0x3, 0x4, 0x5};
#define SIN_LUT_LARGE_AMPLITUDE 5

// Precalculated SIN table, amplitude 17, 36 data points
static const uint8_t SIN_LUT_VERY_LARGE[] = {
    0x9, 0xa, 0xb, 0xd, 0xe, 0xf, 0x10, 0x10, 0x11, 0x11, 0x11, 0x10, 0x10,
    0xf, 0xe, 0xd, 0xb, 0xa, 0x9, 0x7,  0x6,  0x4,  0x3,  0x2,  0x1,  0x1,
    0x0, 0x0, 0x0, 0x1, 0x1, 0x2, 0x3,  0x4,  0x6,  0x7,  0x9};
#define SIN_LUT_VERY_LARGE_LENGTH 36
#define SIN_LUT_VERY_LARGE_AMPLITUDE 8

/**
 * @brief Initialize the mcumgr modules
 */
extern void ff_util_mcumgr_init();

/**
 * @brief perform and MD5 salted hash unique to AND!XOR
 * @param input   Pointer to buffer containing data to salt and hash
 * @param len     Length of data in the input buffer
 * @param output  Pointer to buffer where hash should be stored (16 bytes)
 */
extern void ff_util_md5_salted(uint8_t *input, size_t len, uint8_t *output);

/**
 * @brief Get the current system time in milliseconds
 */
extern uint32_t ff_util_millis();

/**
 * @brief Sleep until a specific end time
 * @param end_time_ms	The time in ms to end at
 */
extern void ff_util_sleep_until(uint32_t end_time_ms);

/**
 * @brief Look for suffix in end of string
 * @param str     Pointer to string array
 * @param suffix  Pointer to string array of suffix to look for
 * @return 0 if str ends with suffix
 */
extern int ff_util_string_ends_with(const char *str, const char *suffix);

#endif