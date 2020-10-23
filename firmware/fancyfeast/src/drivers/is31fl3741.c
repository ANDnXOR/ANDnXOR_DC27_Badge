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
 *  @andnxor
 *  @zappbrandnxor
 *  @hyr0n1
 *  @bender_andnxor
 *  @lacosteaef
 *  @f4nci3
 *  @Cr4bf04m
 *****************************************************************************/

#include <device.h>
#include <gpio.h>
#include <i2c.h>
#include <zephyr.h>

#define LOG_LEVEL 4
#include <logging/log.h>
LOG_MODULE_REGISTER(is31fl3741);

#include "../ff_gfx.h"
#include "../ff_post.h"
#include "autoconf.h"
#include "is31fl3741.h"
#include "is31fl3741_values.h"

#define BYTES_PER_PIXEL 3
#define MAX_CS 39
#define MAX_SW 9

#define PHYSICAL_MAX_CS (17 * 3) /**columns 3 CS per column**/
#define PHYSICAL_MAX_SW (9)      /**rows**/

// Brightness = 50, bling = fade, 2.8VIN
// Divisor = 7, peak = 54mA
// Divisor = 6, peak = 57mA
// Divisor = 5, peak = 60mA
// Divisor = 4, peak = 65mA
#define LED_DIVISOR_REGULAR 5
#define SCALING_FACTOR_REGULAR 0xFF

// Avoid race conditions
K_SEM_DEFINE(lock, 0, 1);

static struct device *m_led_driver;
static bool m_initialized = false;

// Look up table, Page 1 addresses are offset by 0xB4 so frames can be written
// very quickly (Page 1 address = LUT - 0xB4)
// Y value determines color, Red if y%3 = 0, Green 1, and Blue 2
// Addresses below are within the framebuffer and adjusted per the datasheet and
// offset to account for the address bytes the driver needs for auto increment
// mode
static uint16_t LUT[MAX_CS][MAX_SW] = {
    // CS1
    {0x0001, 0x001F, 0x003D, 0x005B, 0x0079, 0x0097, 0x00B6, 0x00D4, 0x00F2},
    // CS2
    {0x0002, 0x0020, 0x003E, 0x005C, 0x007A, 0x0098, 0x00B7, 0x00D5, 0x00F3},
    // CS3
    {0x0003, 0x0021, 0x003F, 0x005D, 0x007B, 0x0099, 0x00B8, 0x00D6, 0x00F4},
    // CS4
    {0x0004, 0x0022, 0x0040, 0x005E, 0x007C, 0x009A, 0x00B9, 0x00D7, 0x00F5},
    // CS5
    {0x0005, 0x0023, 0x0041, 0x005F, 0x007D, 0x009B, 0x00BA, 0x00D8, 0x00F6},
    // CS6
    {0x0006, 0x0024, 0x0042, 0x0060, 0x007E, 0x009C, 0x00BB, 0x00D9, 0x00F7},
    // CS7
    {0x0007, 0x0025, 0x0043, 0x0061, 0x007F, 0x009D, 0x00BC, 0x00DA, 0x00F8},
    // CS8
    {0x0008, 0x0026, 0x0044, 0x0062, 0x0080, 0x009E, 0x00BD, 0x00DB, 0x00F9},
    // CS9
    {0x0009, 0x0027, 0x0045, 0x0063, 0x0081, 0x009F, 0x00BE, 0x00DC, 0x00FA},
    // CS10
    {0x000A, 0x0028, 0x0046, 0x0064, 0x0082, 0x00A0, 0x00BF, 0x00DD, 0x00FB},
    // CS11
    {0x000B, 0x0029, 0x0047, 0x0065, 0x0083, 0x00A1, 0x00C0, 0x00DE, 0x00FC},
    // CS12
    {0x000C, 0x002A, 0x0048, 0x0066, 0x0084, 0x00A2, 0x00C1, 0x00DF, 0x00FD},
    // CS13
    {0x000D, 0x002B, 0x0049, 0x0067, 0x0085, 0x00A3, 0x00C2, 0x00E0, 0x00FE},
    // CS14
    {0x000E, 0x002C, 0x004A, 0x0068, 0x0086, 0x00A4, 0x00C3, 0x00E1, 0x00FF},
    // CS15
    {0x000F, 0x002D, 0x004B, 0x0069, 0x0087, 0x00A5, 0x00C4, 0x00E2, 0x0100},
    // CS16
    {0x0010, 0x002E, 0x004C, 0x006A, 0x0088, 0x00A6, 0x00C5, 0x00E3, 0x0101},
    // CS17
    {0x0011, 0x002F, 0x004D, 0x006B, 0x0089, 0x00A7, 0x00C6, 0x00E4, 0x0102},
    // CS18
    {0x0012, 0x0030, 0x004E, 0x006C, 0x008A, 0x00A8, 0x00C7, 0x00E5, 0x0103},
    // CS19
    {0x0013, 0x0031, 0x004F, 0x006D, 0x008B, 0x00A9, 0x00C8, 0x00E6, 0x0104},
    // CS20
    {0x0014, 0x0032, 0x0050, 0x006E, 0x008C, 0x00AA, 0x00C9, 0x00E7, 0x0105},
    // CS21
    {0x0015, 0x0033, 0x0051, 0x006F, 0x008D, 0x00AB, 0x00CA, 0x00E8, 0x0106},
    // CS22
    {0x0016, 0x0034, 0x0052, 0x0070, 0x008E, 0x00AC, 0x00CB, 0x00E9, 0x0107},
    // CS23
    {0x0017, 0x0035, 0x0053, 0x0071, 0x008F, 0x00AD, 0x00CC, 0x00EA, 0x0108},
    // CS24
    {0x0018, 0x0036, 0x0054, 0x0072, 0x0090, 0x00AE, 0x00CD, 0x00EB, 0x0109},
    // CS25
    {0x0019, 0x0037, 0x0055, 0x0073, 0x0091, 0x00AF, 0x00CE, 0x00EC, 0x010A},
    // CS26
    {0x001A, 0x0038, 0x0056, 0x0074, 0x0092, 0x00B0, 0x00CF, 0x00ED, 0x010B},
    // CS27
    {0x001B, 0x0039, 0x0057, 0x0075, 0x0093, 0x00B1, 0x00D0, 0x00EE, 0x010C},
    // CS28
    {0x001C, 0x003A, 0x0058, 0x0076, 0x0094, 0x00B2, 0x00D1, 0x00EF, 0x010D},
    // CS29
    {0x001D, 0x003B, 0x0059, 0x0077, 0x0095, 0x00B3, 0x00D2, 0x00F0, 0x010E},
    // CS30
    {0x001E, 0x003C, 0x005A, 0x0078, 0x0096, 0x00B4, 0x00D3, 0x00F1, 0x010F},
    // PAGE 1
    // CS31
    {0x0110, 0x0119, 0x0122, 0x012B, 0x0134, 0x013D, 0x0146, 0x014F, 0x0158},
    // CS32
    {0x0111, 0x011A, 0x0123, 0x012C, 0x0135, 0x013E, 0x0147, 0x0150, 0x0159},
    // CS33
    {0x0112, 0x011B, 0x0124, 0x012D, 0x0136, 0x013F, 0x0148, 0x0151, 0x015A},
    // CS34
    {0x0113, 0x011C, 0x0125, 0x012E, 0x0137, 0x0140, 0x0149, 0x0152, 0x015B},
    // CS35
    {0x0114, 0x011D, 0x0126, 0x012F, 0x0138, 0x0141, 0x014A, 0x0153, 0x015C},
    // CS36
    {0x0115, 0x011E, 0x0127, 0x0130, 0x0139, 0x0142, 0x014B, 0x0154, 0x015D},
    // CS37
    {0x0116, 0x011F, 0x0128, 0x0131, 0x013A, 0x0143, 0x014C, 0x0155, 0x015E},
    // CS38
    {0x0117, 0x0120, 0x0129, 0x0132, 0x013B, 0x0144, 0x014D, 0x0156, 0x015F},
    // CS39
    {0x0118, 0x0121, 0x012A, 0x0133, 0x013C, 0x0145, 0x014E, 0x0157, 0x0160},
};

static uint16_t *LUT_PHYSICAL[PHYSICAL_MAX_SW][PHYSICAL_MAX_CS] = {
    // ROW 0
    {
        NULL,        NULL,        NULL,        // 0
        &LUT[30][8], &LUT[31][8], &LUT[32][8], // 1
        NULL,        NULL,        NULL,        // 2
        NULL,        NULL,        NULL,        // 3
        NULL,        NULL,        NULL,        // 4
        NULL,        NULL,        NULL,        // 5
        NULL,        NULL,        NULL,        // 6
        NULL,        NULL,        NULL,        // 7
        NULL,        NULL,        NULL,        // 8
        NULL,        NULL,        NULL,        // 9
        NULL,        NULL,        NULL,        // 10
        NULL,        NULL,        NULL,        // 11
        NULL,        NULL,        NULL,        // 12
        NULL,        NULL,        NULL,        // 13
        NULL,        NULL,        NULL,        // 14
        &LUT[12][6], &LUT[13][6], &LUT[14][6], // 15
        NULL,        NULL,        NULL         // 16
    },

    // ROW 1
    {
        NULL,        NULL,        NULL,        // 0
        &LUT[21][6], &LUT[22][6], &LUT[23][6], // 1
        &LUT[36][0], &LUT[37][0], &LUT[38][0], // 2
        &LUT[33][0], &LUT[34][0], &LUT[35][0], // 3
        &LUT[30][0], &LUT[31][0], &LUT[32][0], // 4
        &LUT[27][0], &LUT[28][0], &LUT[29][0], // 5
        &LUT[24][0], &LUT[25][0], &LUT[26][0], // 6
        &LUT[21][0], &LUT[22][0], &LUT[23][0], // 7
        &LUT[18][0], &LUT[19][0], &LUT[20][0], // 8
        &LUT[15][0], &LUT[16][0], &LUT[17][0], // 9
        &LUT[12][0], &LUT[13][0], &LUT[14][0], // 10
        &LUT[9][0],  &LUT[10][0], &LUT[11][0], // 11
        &LUT[6][0],  &LUT[7][0],  &LUT[8][0],  // 12
        &LUT[3][0],  &LUT[4][0],  &LUT[5][0],  // 13
        &LUT[0][0],  &LUT[1][0],  &LUT[2][0],  // 14
        &LUT[9][6],  &LUT[10][6], &LUT[11][6], // 15
        NULL,        NULL,        NULL         // 16
    },

    // ROW 2
    {
        &LUT[36][7], &LUT[37][7], &LUT[38][7], // 0
        &LUT[21][7], &LUT[22][7], &LUT[23][7], // 1
        &LUT[36][1], &LUT[37][1], &LUT[38][1], // 2
        &LUT[33][1], &LUT[34][1], &LUT[35][1], // 3
        &LUT[30][1], &LUT[31][1], &LUT[32][1], // 4
        &LUT[27][1], &LUT[28][1], &LUT[29][1], // 5
        &LUT[24][1], &LUT[25][1], &LUT[26][1], // 6
        &LUT[21][1], &LUT[22][1], &LUT[23][1], // 7
        &LUT[18][1], &LUT[19][1], &LUT[20][1], // 8
        &LUT[15][1], &LUT[16][1], &LUT[17][1], // 9
        &LUT[12][1], &LUT[13][1], &LUT[14][1], // 10
        &LUT[9][1],  &LUT[10][1], &LUT[11][1], // 11
        &LUT[6][1],  &LUT[7][1],  &LUT[8][1],  // 12
        &LUT[3][1],  &LUT[4][1],  &LUT[5][1],  // 13
        &LUT[0][1],  &LUT[1][1],  &LUT[2][1],  // 14
        &LUT[9][7],  &LUT[10][7], &LUT[11][7], // 15
        &LUT[6][7],  &LUT[7][7],  &LUT[8][7],  // 16
    },

    // ROW 3
    {
        &LUT[36][8], &LUT[37][8], &LUT[38][8], // 0
        &LUT[21][8], &LUT[22][8], &LUT[23][8], // 1
        &LUT[36][2], &LUT[37][2], &LUT[38][2], // 2
        &LUT[33][2], &LUT[34][2], &LUT[35][2], // 3
        &LUT[30][2], &LUT[31][2], &LUT[32][2], // 4
        &LUT[27][2], &LUT[28][2], &LUT[29][2], // 5
        &LUT[24][2], &LUT[25][2], &LUT[26][2], // 6
        &LUT[21][2], &LUT[22][2], &LUT[23][2], // 7
        &LUT[18][2], &LUT[19][2], &LUT[20][2], // 8
        &LUT[15][2], &LUT[16][2], &LUT[17][2], // 9
        &LUT[12][2], &LUT[13][2], &LUT[14][2], // 10
        &LUT[9][2],  &LUT[10][2], &LUT[11][2], // 11
        &LUT[6][2],  &LUT[7][2],  &LUT[8][2],  // 12
        &LUT[3][2],  &LUT[4][2],  &LUT[5][2],  // 13
        &LUT[0][2],  &LUT[1][2],  &LUT[2][2],  // 14
        &LUT[9][8],  &LUT[10][8], &LUT[11][8], // 15
        &LUT[6][8],  &LUT[7][8],  &LUT[8][8],  // 16
    },

    // ROW 4
    {
        &LUT[27][6], &LUT[28][6], &LUT[29][6], // 0
        &LUT[24][6], &LUT[25][6], &LUT[26][6], // 1
        &LUT[36][3], &LUT[37][3], &LUT[38][3], // 2
        &LUT[33][3], &LUT[34][3], &LUT[35][3], // 3
        &LUT[30][3], &LUT[31][3], &LUT[32][3], // 4
        &LUT[27][3], &LUT[28][3], &LUT[29][3], // 5
        &LUT[24][3], &LUT[25][3], &LUT[26][3], // 6
        &LUT[21][3], &LUT[22][3], &LUT[23][3], // 7
        &LUT[18][3], &LUT[19][3], &LUT[20][3], // 8
        &LUT[15][3], &LUT[16][3], &LUT[17][3], // 9
        &LUT[12][3], &LUT[13][3], &LUT[14][3], // 10
        &LUT[9][3],  &LUT[10][3], &LUT[11][3], // 11
        &LUT[6][3],  &LUT[7][3],  &LUT[8][3],  // 12
        &LUT[3][3],  &LUT[4][3],  &LUT[5][3],  // 13
        &LUT[0][3],  &LUT[1][3],  &LUT[2][3],  // 14
        &LUT[0][6],  &LUT[1][6],  &LUT[2][6],  // 15
        &LUT[3][7],  &LUT[4][7],  &LUT[5][7],  // 16
    },

    // ROW 5
    {
        &LUT[27][7], &LUT[28][7], &LUT[29][7], // 0
        &LUT[24][7], &LUT[25][7], &LUT[26][7], // 1
        &LUT[36][4], &LUT[37][4], &LUT[38][4], // 2
        &LUT[33][4], &LUT[34][4], &LUT[35][4], // 3
        &LUT[30][4], &LUT[31][4], &LUT[32][4], // 4
        &LUT[27][4], &LUT[28][4], &LUT[29][4], // 5
        &LUT[24][4], &LUT[25][4], &LUT[26][4], // 6
        &LUT[21][4], &LUT[22][4], &LUT[23][4], // 7
        &LUT[18][4], &LUT[19][4], &LUT[20][4], // 8
        &LUT[15][4], &LUT[16][4], &LUT[17][4], // 9
        &LUT[12][4], &LUT[13][4], &LUT[14][4], // 10
        &LUT[9][4],  &LUT[10][4], &LUT[11][4], // 11
        &LUT[6][4],  &LUT[7][4],  &LUT[8][4],  // 12
        &LUT[3][4],  &LUT[4][4],  &LUT[5][4],  // 13
        &LUT[0][4],  &LUT[1][4],  &LUT[2][4],  // 14
        &LUT[0][7],  &LUT[1][7],  &LUT[2][7],  // 15
        NULL,        NULL,        NULL         // 16
    },

    // ROW 6
    {
        NULL,        NULL,        NULL,        // 0
        &LUT[24][8], &LUT[25][8], &LUT[26][8], // 1
        &LUT[36][5], &LUT[37][5], &LUT[38][5], // 2
        &LUT[33][5], &LUT[34][5], &LUT[35][5], // 3
        &LUT[30][5], &LUT[31][5], &LUT[32][5], // 4
        &LUT[27][5], &LUT[28][5], &LUT[29][5], // 5
        &LUT[24][5], &LUT[25][5], &LUT[26][5], // 6
        &LUT[21][5], &LUT[22][5], &LUT[23][5], // 7
        NULL,        NULL,        NULL,        // 8
        &LUT[15][5], &LUT[16][5], &LUT[17][5], // 9
        &LUT[12][5], &LUT[13][5], &LUT[14][5], // 10
        &LUT[9][5],  &LUT[10][5], &LUT[11][5], // 11
        &LUT[6][5],  &LUT[7][5],  &LUT[8][5],  // 12
        &LUT[3][5],  &LUT[4][5],  &LUT[5][5],  // 13
        &LUT[0][5],  &LUT[1][5],  &LUT[2][5],  // 14
        &LUT[0][8],  &LUT[1][8],  &LUT[2][8],  // 15
        NULL,        NULL,        NULL         // 16
    },

    // ROW 7
    {
        NULL,        NULL,        NULL,        // 0
        NULL,        NULL,        NULL,        // 1
        &LUT[36][6], &LUT[37][6], &LUT[38][6], // 2
        &LUT[33][6], &LUT[34][6], &LUT[35][6], // 3
        &LUT[30][6], &LUT[31][6], &LUT[32][6], // 4
        NULL,        NULL,        NULL,        // 5
        NULL,        NULL,        NULL,        // 6
        NULL,        NULL,        NULL,        // 7
        NULL,        NULL,        NULL,        // 8
        NULL,        NULL,        NULL,        // 9
        NULL,        NULL,        NULL,        // 10
        NULL,        NULL,        NULL,        // 11
        &LUT[6][6],  &LUT[7][6],  &LUT[8][6],  // 12
        &LUT[3][6],  &LUT[4][6],  &LUT[5][6],  // 13
        NULL,        NULL,        NULL,        // 14
        NULL,        NULL,        NULL,        // 15
        NULL,        NULL,        NULL         // 16
    },

    // ROW 8
    {
        NULL,        NULL,        NULL,        // 0
        &LUT[33][8], &LUT[34][8], &LUT[35][8], // 1
        NULL,        NULL,        NULL,        // 2
        NULL,        NULL,        NULL,        // 3
        NULL,        NULL,        NULL,        // 4
        NULL,        NULL,        NULL,        // 5
        NULL,        NULL,        NULL,        // 6
        NULL,        NULL,        NULL,        // 7
        NULL,        NULL,        NULL,        // 8
        NULL,        NULL,        NULL,        // 9
        NULL,        NULL,        NULL,        // 10
        NULL,        NULL,        NULL,        // 11
        NULL,        NULL,        NULL,        // 12
        NULL,        NULL,        NULL,        // 13
        NULL,        NULL,        NULL,        // 14
        &LUT[12][8], &LUT[13][8], &LUT[14][8], // 15
        NULL,        NULL,        NULL         // 16
    },
};

// Thank you Adafruit!
// https://learn.adafruit.com/led-tricks-gamma-correction/the-quick-fix
static const uint8_t GAMMA[] = {
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   1,   1,
    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   2,   2,   2,   2,
    2,   2,   2,   2,   3,   3,   3,   3,   3,   3,   3,   4,   4,   4,   4,
    4,   5,   5,   5,   5,   6,   6,   6,   6,   7,   7,   7,   7,   8,   8,
    8,   9,   9,   9,   10,  10,  10,  11,  11,  11,  12,  12,  13,  13,  13,
    14,  14,  15,  15,  16,  16,  17,  17,  18,  18,  19,  19,  20,  20,  21,
    21,  22,  22,  23,  24,  24,  25,  25,  26,  27,  27,  28,  29,  29,  30,
    31,  32,  32,  33,  34,  35,  35,  36,  37,  38,  39,  39,  40,  41,  42,
    43,  44,  45,  46,  47,  48,  49,  50,  50,  51,  52,  54,  55,  56,  57,
    58,  59,  60,  61,  62,  63,  64,  66,  67,  68,  69,  70,  72,  73,  74,
    75,  77,  78,  79,  81,  82,  83,  85,  86,  87,  89,  90,  92,  93,  95,
    96,  98,  99,  101, 102, 104, 105, 107, 109, 110, 112, 114, 115, 117, 119,
    120, 122, 124, 126, 127, 129, 131, 133, 135, 137, 138, 140, 142, 144, 146,
    148, 150, 152, 154, 156, 158, 160, 162, 164, 167, 169, 171, 173, 175, 177,
    180, 182, 184, 186, 189, 191, 193, 196, 198, 200, 203, 205, 208, 210, 213,
    215, 218, 220, 223, 225, 228, 231, 233, 236, 239, 241, 244, 247, 249, 252,
    255};

#define FRAME_BUFFER_SIZE                                                      \
  ((CONFIG_LED_RGB_WIDTH * CONFIG_LED_RGB_HEIGHT * BYTES_PER_PIXEL) + 2)
struct is31fl3741_data {
  struct device *i2c;
  // Frame buffer stores all PWM values for the LED matrix
  // In addition, the first byte of each page (at 0x0000 and 0x00B4) is 0x00 so
  // that the frame buffer can be blasted at once to the LED driver
  uint8_t frame_buffer[FRAME_BUFFER_SIZE];
};

static struct is31fl3741_data is31fl3741_driver_data;

/**
 * @brief Helper function to switch register pages in the LED Driver
 */
static int __switch_page(struct device *i2c, uint8_t page) {
  if (i2c == NULL) {
    LOG_ERR("%s Cannot switch page, invalid device", __func__);
    return -EIO;
  }

  if (i2c->driver_api == NULL) {
    LOG_ERR("%s Cannot switch page, invalid device driver API", __func__);
    return -EIO;
  }

  // Unlock the page write register
  if (i2c_reg_write_byte(i2c, CONFIG_IS31FL3741_I2C_ADDRESS, IS31_REG_UNLOCK,
                         IS31_VALUE_UNLOCK)) {
    LOG_ERR("Unlocking IS31FL3741 failed.");
    return -EIO;
  }

  // Switch to page 2
  if (i2c_reg_write_byte(i2c, CONFIG_IS31FL3741_I2C_ADDRESS,
                         IS31_REG_PAGE_WRITE, page)) {
    LOG_ERR("Switching to page %d failed.", page);
    return -EIO;
  }

  return 0;
}

/**
 * @brief Initialize the LED Matrix Driver
 */
static int is31fl3741_init(struct device *dev) {
  int err;

  // Init the semaphore
  k_sem_init(&lock, 1, UINT_MAX);

  m_led_driver = dev;
  struct is31fl3741_data *data = m_led_driver->driver_data;

  // Wait for semaphore (probably unnecessary)
  k_sem_take(&lock, K_FOREVER);

  struct device *gpio = device_get_binding(SW0_GPIO_CONTROLLER);
  gpio_pin_configure(gpio, CONFIG_IS31FL3741_SDB_PIN, GPIO_DIR_OUT);
  gpio_pin_write(gpio, CONFIG_IS31FL3741_SDB_PIN, 1);
  k_sleep(50);

  data->i2c = device_get_binding(CONFIG_IS31FL3741_I2C_MASTER_DEV_NAME);
  if (data->i2c == NULL) {
    LOG_ERR("Failed to get I2C device");
    k_sem_give(&lock);
    return -EINVAL;
  }

  uint32_t i2c_cfg = I2C_SPEED_SET(I2C_SPEED_FAST) | I2C_MODE_MASTER;
  err = i2c_configure(data->i2c, i2c_cfg);
  if (err) {
    k_sem_give(&lock);
    return err;
  }

  // Read the product id
  uint8_t product_id = 0x00;
  if (i2c_reg_read_byte(data->i2c, CONFIG_IS31FL3741_I2C_ADDRESS,
                        IS31_REG_PRODUCT_ID, &product_id)) {
    // ACK failed
    ff_post_failed(FF_POST_IS31FL3741_ACK);
    LOG_ERR("Reading product ID failed");
    k_sem_give(&lock);
    return -EIO;
  }

  // Obviously we're talking to the IC if we got this far
  ff_post_success(FF_POST_IS31FL3741_ACK);

  // Validate product ID
  if (product_id == IS31_VALUE_PRODUCT_ID) {
    ff_post_success(FF_POST_IS31FL3741_PRODUCT_ID);
  } else {
    ff_post_failed(FF_POST_IS31FL3741_PRODUCT_ID);
  }

  // Max scale 1/2 LEDs
  err = __switch_page(data->i2c, IS31_VALUE_PAGE_2);
  if (err) {
    k_sem_give(&lock);
    return err;
  }
  for (uint8_t i = 0; i < IS31FL3741_MAX_ADDRESS; i++) {
    i2c_reg_write_byte(data->i2c, CONFIG_IS31FL3741_I2C_ADDRESS, i,
                       SCALING_FACTOR_REGULAR);
  }
  // Max scale 1/2 LEDs
  err = __switch_page(data->i2c, IS31_VALUE_PAGE_3);
  if (err) {
    k_sem_give(&lock);
    return err;
  }
  for (uint8_t i = 0; i < IS31FL3741_MAX_ADDRESS; i++) {
    // Manually scale light pipe LEDs
    i2c_reg_write_byte(data->i2c, CONFIG_IS31FL3741_I2C_ADDRESS, i,
                       SCALING_FACTOR_REGULAR);
  }

  // PWM 1/2 LEDs to 0
  err = __switch_page(data->i2c, IS31_VALUE_PAGE_0);
  if (err) {
    k_sem_give(&lock);
    return err;
  }
  for (uint8_t i = 0; i < IS31FL3741_MAX_ADDRESS; i++) {
    if (i2c_reg_write_byte(data->i2c, CONFIG_IS31FL3741_I2C_ADDRESS, i, 0x00)) {
      LOG_ERR("PWM LED values failed");
      k_sem_give(&lock);
      return -EIO;
    }
  }
  // PWM 1/2 LEDs to 0
  err = __switch_page(data->i2c, IS31_VALUE_PAGE_1);
  if (err) {
    k_sem_give(&lock);
    return err;
  }
  for (uint8_t i = 0; i < IS31FL3741_MAX_ADDRESS; i++) {
    if (i2c_reg_write_byte(data->i2c, CONFIG_IS31FL3741_I2C_ADDRESS, i, 0x00)) {
      LOG_ERR("PWM LED values failed");
      k_sem_give(&lock);
      return -EIO;
    }
  }

  // Do some config
  err = __switch_page(data->i2c, IS31_VALUE_PAGE_4);
  if (err) {
    k_sem_give(&lock);
    return err;
  }

  // Set GCC value
  if (i2c_reg_write_byte(data->i2c, CONFIG_IS31FL3741_I2C_ADDRESS, IS31_REG_GCC,
                         CONFIG_LED_BRIGHTNESS_DEFAULT)) {
    LOG_ERR("Global current control failed");
    k_sem_give(&lock);
    return -EIO;
  }

  // Set Pull up/down register to default
  if (i2c_reg_write_byte(data->i2c, CONFIG_IS31FL3741_I2C_ADDRESS,
                         IS31_REG_PULL_UP_DOWN, IS31_PULL_UP_DOWN_DEFAULT)) {
    LOG_ERR("Pull up/down resistor control failed");
    k_sem_give(&lock);
    return -EIO;
  }

  // Switch to normal operations
  if (i2c_reg_write_byte(data->i2c, CONFIG_IS31FL3741_I2C_ADDRESS,
                         IS31_REG_CONFIG, IS31_VALUE_CONFIG_SSD_NORMAL)) {
    LOG_ERR("Global current control failed");
    k_sem_give(&lock);
    return -EIO;
  }

  // Ensure frame buffer is clear
  if (data->frame_buffer == NULL) {
    LOG_ERR("Frame buffer is null while trying to clear it");
    k_sem_give(&lock);
    return -EIO;
  }
  memset(data->frame_buffer, 0, FRAME_BUFFER_SIZE);

  // Fill frame buffer with custom scaling factors for gamma
  uint16_t address = 0x00;
  for (uint16_t y = 0; y < 39; y++) {
    for (uint16_t x = 0; x < 9; x++) {
      address = LUT[y][x];
      switch (y % 3) {
      // Red
      case 0:
        data->frame_buffer[address] = 0x6F;
        break;
      // Green
      case 1:
        data->frame_buffer[address] = 0x4F;
        break;
      // Blue
      case 2:
        data->frame_buffer[address] = 0xFF;
        break;
      }
    }
  }

  // Write the frame buffer to the scaling factor pages
  __switch_page(data->i2c, 2);
  i2c_write(data->i2c, data->frame_buffer, 0xB4 + 1,
            CONFIG_IS31FL3741_I2C_ADDRESS);

  __switch_page(data->i2c, 3);
  i2c_write(data->i2c, data->frame_buffer + 0xB5, 0xAA + 2,
            CONFIG_IS31FL3741_I2C_ADDRESS);

  // Clear frame buffer
  memset(data->frame_buffer, 0, FRAME_BUFFER_SIZE);

  m_initialized = true;

  k_sem_give(&lock);

  // Do a quick test on the LEDs
  is31fl3741_open_short_detection();
  return 0;
}

/**
 * Test open / short status of all LEDs. POST state is updated after each test.
 * @return 0 if successful, negative value if there was a failure
 */
int is31fl3741_open_short_detection() {
  if (!m_initialized) {
    return -EIO;
  }

  int err;
  struct is31fl3741_data *data = m_led_driver->driver_data;
  uint8_t osde_open[IS31_VALUE_OPEN_SHORT_REG_COUNT];
  uint8_t osde_short[IS31_VALUE_OPEN_SHORT_REG_COUNT];

  // Wait for semaphore
  k_sem_take(&lock, K_FOREVER);

  // Configuration register is in page 4
  __switch_page(data->i2c, 4);

  // Set GCC value to 0x01 per the datasheet
  if (i2c_reg_write_byte(data->i2c, CONFIG_IS31FL3741_I2C_ADDRESS, IS31_REG_GCC,
                         0x01)) {
    LOG_ERR("Global current control failed");
    k_sem_give(&lock);
    return -EIO;
  }

  // Now disable the pull up/down registers per the datasheet
  if (i2c_reg_write_byte(data->i2c, CONFIG_IS31FL3741_I2C_ADDRESS,
                         IS31_REG_PULL_UP_DOWN, 0x00)) {
    LOG_ERR("Unable to disable pull up/down registers");
    k_sem_give(&lock);
    return -EIO;
  }

  // Enable OSDE Open check
  err = i2c_reg_write_byte(
      data->i2c, CONFIG_IS31FL3741_I2C_ADDRESS, IS31_REG_CONFIG,
      IS31_VALUE_CONFIG_OSDE_OPEN | IS31_VALUE_CONFIG_SSD_NORMAL);
  if (err) {
    LOG_ERR("Unable to enable OSDE[%d]", err);
  }

  k_sleep(50);

  // Read open status from registers
  err = i2c_burst_read(data->i2c, CONFIG_IS31FL3741_I2C_ADDRESS,
                       IS31_VALUE_OPEN_SHORT_REG_START_ADDR, osde_open,
                       IS31_VALUE_OPEN_SHORT_REG_COUNT);
  if (err) {
    LOG_ERR("Unable to read OSDE open status [%d]", err);
  }

  // Enable OSDE Short check
  err = i2c_reg_write_byte(
      data->i2c, CONFIG_IS31FL3741_I2C_ADDRESS, IS31_REG_CONFIG,
      IS31_VALUE_CONFIG_OSDE_SHORT | IS31_VALUE_CONFIG_SSD_NORMAL);
  if (err) {
    LOG_ERR("Unable to enable OSDE[%d]", err);
  }

  k_sleep(50);

  // Read short status from registers
  err = i2c_burst_read(data->i2c, CONFIG_IS31FL3741_I2C_ADDRESS,
                       IS31_VALUE_OPEN_SHORT_REG_START_ADDR, osde_short,
                       IS31_VALUE_OPEN_SHORT_REG_COUNT);
  if (err) {
    LOG_ERR("Unable to read OSDE open status [%d]", err);
  }

  // Print the results
  uint8_t count_short = 0;
  uint8_t count_open = 0;
  for (uint8_t i = 0; i < IS31_VALUE_OPEN_SHORT_REG_COUNT; i++) {
    for (uint8_t j = 0; j < 8; j++) {
      if ((osde_open[i] & (1 << j)) > 0) {
        count_open++;
      }
      if ((osde_short[i] & (1 << j)) > 0) {
        count_short++;
      }
    }
  }

  // POST
  if (count_open == 0) {
    LOG_INF("No open LEDs detected!");
    ff_post_success(FF_POST_IS31FL3741_OPEN);
  } else {
    LOG_ERR("%d open LEDs detected", count_open);
    ff_post_failed(FF_POST_IS31FL3741_OPEN);
  }
  if (count_short == 0) {
    LOG_INF("No LED shorts detected!");
    ff_post_success(FF_POST_IS31FL3741_SHORT);
  } else {
    LOG_ERR("%d LED shorts detected", count_short);
    ff_post_failed(FF_POST_IS31FL3741_SHORT);
  }

  // Return GCC to default
  __switch_page(data->i2c, 4);
  if (i2c_reg_write_byte(data->i2c, CONFIG_IS31FL3741_I2C_ADDRESS, IS31_REG_GCC,
                         CONFIG_LED_BRIGHTNESS_DEFAULT)) {
    LOG_ERR("Global current control failed");
    k_sem_give(&lock);
    return -EIO;
  }

  // Set GCC value to 0x01 per the datasheet
  // if (i2c_reg_write_byte(data->i2c, CONFIG_IS31FL3741_I2C_ADDRESS,
  // IS31_REG_GCC,
  //                        0x01)) {
  //   LOG_ERR("Global current control failed");
  //   k_sem_give(&lock);
  //   return -EIO;
  // }

  // Return Pull up/down register to default
  if (i2c_reg_write_byte(data->i2c, CONFIG_IS31FL3741_I2C_ADDRESS,
                         IS31_REG_PULL_UP_DOWN, IS31_PULL_UP_DOWN_DEFAULT)) {
    LOG_ERR("Pull up/down resistor control failed");
    k_sem_give(&lock);
    return -EIO;
  }

  k_sem_give(&lock);

  return 0;
}

/**
 * @brief Change the LED global current control
 */
void is31fl3741_gcc_set(uint8_t brightness) {
  int err;
  struct is31fl3741_data *data = m_led_driver->driver_data;

  k_sem_take(&lock, K_FOREVER);
  // Do some config
  err = __switch_page(data->i2c, IS31_VALUE_PAGE_4);
  if (err) {
    k_sem_give(&lock);
    return;
  }

  // Set GCC value
  if (i2c_reg_write_byte(data->i2c, CONFIG_IS31FL3741_I2C_ADDRESS, IS31_REG_GCC,
                         brightness)) {
    LOG_ERR("Global current control failed");
    k_sem_give(&lock);
    return;
  }
  k_sem_give(&lock);
}

/**
 * @brief Push the current frame buffer to the LED Matrix
 */
int is31fl3741_push_buffer() {
  if (!m_initialized) {
    return -EIO;
  }

  struct is31fl3741_data *data = m_led_driver->driver_data;

  // Wait for semaphore
  k_sem_take(&lock, K_FOREVER);
  k_sleep(1);

  // Push the buffer
  if (__switch_page(data->i2c, 0)) {
    LOG_ERR("Unable to switch to page 0");
    k_sem_give(&lock);
    return -EIO;
  }
  if (i2c_write(data->i2c, data->frame_buffer, 0xB4 + 1,
                CONFIG_IS31FL3741_I2C_ADDRESS)) {
    LOG_ERR("Unable to write to page 0 frame buffer");
    k_sem_give(&lock);
    return -EIO;
  }

  if (__switch_page(data->i2c, 1)) {
    LOG_ERR("Unable to switch to page 1");
    k_sem_give(&lock);
    return -EIO;
  }
  if (i2c_write(data->i2c, data->frame_buffer + 0xB5, 0xAA + 2,
                CONFIG_IS31FL3741_I2C_ADDRESS)) {
    LOG_ERR("Unable to write to page 1 frame buffer");
    k_sem_give(&lock);
    return -EIO;
  }

  k_sem_give(&lock);

  return 0;
}

/**
 * @brief Set the pwm value of a specific LED in the matrix
 * @param cs    : Row number
 * @param sw    : Column number
 * @param value	: PWM value (0 to 255) for the LED
 * @return Error code if anything happened
 */
int is31fl3741_set(uint8_t sw, uint8_t cs, uint8_t value) {
  // Bounds checking. Since we're signed invalid values will always be greater
  // than width or height
  if (cs >= MAX_CS || sw >= MAX_SW) {
    return -EACCES;
  }

  if (m_led_driver == NULL) {
    return -ENODEV;
  }

  struct is31fl3741_data *data = m_led_driver->driver_data;

  k_sem_take(&lock, K_FOREVER);
  data->frame_buffer[LUT[cs][sw]] = GAMMA[value];
  k_sem_give(&lock);

  return 0;
}

/**
 * @brief Set the pwm value of a specific LED in the matrix based on physical
 * layout
 * @param value	: PWM value (0 to 255) for the LED
 * @return Error code if anything happened
 */
int is31fl3741_set_physical(uint8_t row, uint8_t col, uint8_t value) {
  // Bounds checking. Since we're signed invalid values will always be greater
  // than width or height
  if (col >= PHYSICAL_MAX_CS || row >= PHYSICAL_MAX_SW) {
    return -EACCES;
  }

  if (m_led_driver == NULL) {
    return -ENODEV;
  }

  struct is31fl3741_data *data = m_led_driver->driver_data;
  uint16_t *p_lut = LUT_PHYSICAL[row][col];

  // Scale LEDs on the matrix
  if (row > 0 && row < 8) {
    value = value / LED_DIVISOR_REGULAR;
  }

  if (p_lut != NULL) {
    k_sem_take(&lock, K_FOREVER);
    data->frame_buffer[*p_lut] = GAMMA[value];
    k_sem_give(&lock);
  }

  return 0;
}

DEVICE_INIT(is31fl3741, CONFIG_IS31FL3741_DEV_NAME, is31fl3741_init,
            &is31fl3741_driver_data, NULL, APPLICATION,
            CONFIG_IS31FL3741_INIT_PRIORITY);