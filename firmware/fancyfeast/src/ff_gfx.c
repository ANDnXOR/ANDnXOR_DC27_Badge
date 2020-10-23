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
#include <device.h>
#include <stdlib.h>
#include <zephyr.h>

#define LOG_LEVEL 4
#include <logging/log.h>
LOG_MODULE_REGISTER(ff_gfx);

#include "autoconf.h"
#include "drivers/is31fl3741.h"
#include "ff_gfx.h"
#include "ff_gfx_font.h"

#ifndef SWAP_INT16_T
#define SWAP_INT16_T(a, b)                                                     \
  {                                                                            \
    int16_t t = a;                                                             \
    a = b;                                                                     \
    b = t;                                                                     \
  }
#endif

/**
 * @brief Helper function to map from matrix x,y and colors to IS31FL3741 CS/SW
 */
static void __set(int16_t x, int16_t y, color_rgb_t color) {
  if (x < 0 || y < 0 || x >= WIDTH || y >= WIDTH) {
    return;
  }

  // Rotate the pixels such that (0,0) is at (0,36) and (12,8) is (0,8)
  uint8_t col = x * 3;
  uint8_t row = y;

#ifdef CONFIG_IS31FL3741

  if (y == 0 || y == (HEIGHT - 1)) {
    is31fl3741_set_physical(row, col, color.r);
    is31fl3741_set_physical(row, col + 1, color.g);
    is31fl3741_set_physical(row, col + 2, color.b);
  } else {
    is31fl3741_set_physical(row, col, color.b);
    is31fl3741_set_physical(row, col + 1, color.g);
    is31fl3741_set_physical(row, col + 2, color.r);
  }
#endif
}

/**
 * @brief Draw a bitmap as a single frame
 * @param p_frame   Pointer to raw frame bitmap in memory
 * @param fg        Foreground color
 * @param bg        Background color
 */
void ff_gfx_draw_bitmap(uint8_t *p_frame, color_rgb_t fg, color_rgb_t bg) {
  ff_gfx_fill(bg);
  for (int16_t y = 0; y < HEIGHT; y++) {
    for (int16_t x = 0; x < WIDTH; x++) {
      int16_t byte_offset = (y * WIDTH) + x;
      int16_t byte = p_frame[byte_offset / 8];
      if ((byte & (1 << (byte_offset % 8))) > 0) {
        ff_gfx_draw_pixel(x, y, fg);
      }
    }
  }
}

void ff_gfx_draw_char(int16_t x, int16_t y, char c, color_rgb_t color,
                      color_rgb_t bg) {
  // Clip the character
  if ((x >= WIDTH) || (y >= HEIGHT) || ((x + 4) < 0) || ((y + 6) < 0)) {
    return;
  }

  // To Upper Case
  if (c >= 'a' && c <= 'z') {
    c -= 32;
  }

  // Bound the character in our fontspace
  uint8_t index = c - 32;
  if (index > 95) {
    return;
  }

  for (uint8_t line = 0; line <= 4; line++) {
    uint8_t data = 0;
    switch (line) {
    case 0:
      data = (uint8_t)font4x6[index][0] >> 4;
      break;
    case 1:
      data = (uint8_t)font4x6[index][0] >> 1;
      break;
    case 2:
      // split over two bytes
      data = (uint8_t)((font4x6[index][0] & 0x03) << 2) |
             (font4x6[index][1] & 0x02);
      break;
    case 3:
      data = (uint8_t)font4x6[index][1] >> 4;
      break;
    case 4:
      data = (uint8_t)font4x6[index][1] >> 1;
      break;
    }
    data = (data & 0x0E);

    for (uint8_t col = 0; col < 4; col++) {
      if ((data & (1 << (3 - col))) > 0) {
        ff_gfx_draw_pixel(x + col, y + line, color);
      } else {
        // ff_gfx_draw_pixel(x + col, y + line, bg);
      }
    }
  }

  // // Char bitmap = 5 columns
  // for (int8_t i = 0; i < 5; i++) {
  //   uint8_t line = ff_gfx_font[c * 5 + i];
  //   for (int8_t j = 0; j < 8; j++, line >>= 1) {
  //     if (line & 1) {
  //       ff_gfx_draw_pixel(x + i, y + j, color);
  //     } else if (*(uint32_t*)&bg != *(uint32_t*)&color) {
  //       ff_gfx_draw_pixel(x + i, y + j, bg);
  //     }
  //   }
  // }

  // If opaque, draw vertical line for last column
  if (*(uint32_t *)&bg != *(uint32_t *)&color) {
    ff_gfx_draw_line(x + 5, y, x + 5, y + 8, bg);
  }
}

/**
 * @brief Print a string to the display
 * @param x       : The starting x coordinate
 * @param y       : The starting y coordinate
 * @param string  : The string to print
 * @param color   : The foreground color
 * @param bg      : The background color to draw with
 */
void ff_gfx_print(int16_t x, int16_t y, char *string, color_rgb_t color,
                  color_rgb_t bg) {
  int16_t xx = 0;
  for (uint32_t i = 0; i < strlen(string); i++) {
    xx = (i * FF_GFX_FONT_CHAR_WIDTH) + x;
    if (x > WIDTH) {
      break;
    }
    ff_gfx_draw_char(xx, y, string[i], color, bg);
  }
}

/**
 * Initialize the graphics layer
 */
int ff_gfx_init() { return 0; }

color_rgb_t ff_gfx_color_hsv_to_rgb(float H, float S, float V) {
  while (H >= 1.0) {
    H -= 1.0;
  }

  float h = H * 6;
  uint8_t i = (uint8_t)h;
  float a = V * (1 - S);
  float b = V * (1 - S * (h - i));
  float c = V * (1 - (S * (1 - (h - i))));
  float rf, gf, bf;

  switch (i) {
  case 0:
    rf = V * 255;
    gf = c * 255;
    bf = a * 255;
    break;
  case 1:
    rf = b * 255;
    gf = V * 255;
    bf = a * 255;
    break;
  case 2:
    rf = a * 255;
    gf = V * 255;
    bf = c * 255;
    break;
  case 3:
    rf = a * 255;
    gf = b * 255;
    bf = V * 255;
    break;
  case 4:
    rf = c * 255;
    gf = a * 255;
    bf = V * 255;
    break;
  case 5:
  default:
    rf = V * 255;
    gf = a * 255;
    bf = b * 255;
    break;
  }

  uint8_t R = rf;
  uint8_t G = gf;
  uint8_t B = bf;

  color_rgb_t RGB = {R, G, B};
  return RGB;
}

void ff_gfx_draw_circle(int16_t x, int16_t y, int16_t r, color_rgb_t color) {
  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t xx = 0;
  int16_t yy = r;

  __set(x, y + r, color);
  __set(x, y - r, color);
  __set(x + r, y, color);
  __set(x - r, y, color);

  while (xx < yy) {
    if (f >= 0) {
      yy--;
      ddF_y += 2;
      f += ddF_y;
    }
    xx++;
    ddF_x += 2;
    f += ddF_x;

    __set(x + xx, y + yy, color);
    __set(x - xx, y + yy, color);
    __set(x + xx, y - yy, color);
    __set(x - xx, y - yy, color);
    __set(x + yy, y + xx, color);
    __set(x - yy, y + xx, color);
    __set(x + yy, y - xx, color);
    __set(x - yy, y - xx, color);
  }
}

void ff_gfx_draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                      color_rgb_t color) {
  int16_t steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    SWAP_INT16_T(x0, y0);
    SWAP_INT16_T(x1, y1);
  }

  // Simplify direction we're drawing the line in
  if (x0 > x1) {
    SWAP_INT16_T(x0, x1);
    SWAP_INT16_T(y0, y1);
  }

  int16_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int16_t err = dx / 2;
  int16_t ystep;

  if (y0 < y1) {
    ystep = 1;
  } else {
    ystep = -1;
  }

  for (; x0 <= x1; x0++) {
    if (steep) {
      __set(y0, x0, color);
    } else {
      __set(x0, y0, color);
    }
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
}

/**
 * @brief Draw a single pixel in any color to the LED matrix
 * NOTE: This draws immediately and is currently unbuffered. This may change in
 * the future
 *
 * @param x		X coordinate of the pixel
 * @param y		Y coordinate of the pixel
 * @param color	RGB (24-bit) color of the pixel to draw
 */
void ff_gfx_draw_pixel(int16_t x, int16_t y, color_rgb_t color) {
  // Don't draw off screen
  if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) {
    return;
  }

  __set(x, y, color);
}

/**
 * @brief Draw an XBM bitmap
 * @param x   Leftmost coordinate of bitmap
 * @param y   Topmost coordinate of bitmap
 * @param w   Width of bitmap
 * @param y   Height of bitmap
 * @param fg  Foreground color to use
 * @param bg  Background color to use
 */
void ff_gfx_draw_xbm(uint8_t *p_xbm, int16_t x, int16_t y, uint8_t w, uint8_t h,
                     color_rgb_t fg, color_rgb_t bg) {
  uint8_t bytes_per_row = (w / 8) + 1;
  for (int16_t yy = 0; yy < h; yy++) {
    for (int16_t xx = 0; xx < w; xx++) {
      // Each row is some multiple of bytes
      int16_t byte = p_xbm[(yy * bytes_per_row) + (xx / 8)];
      // xbm format, black is 1, white is 0, we consider black to be background
      if ((byte & (1 << (xx % 8))) == 0) {
        ff_gfx_draw_pixel(x + xx, y + yy, fg);
      } else {
        ff_gfx_draw_pixel(x + xx, y + yy, bg);
      }
    }
  }
}

void ff_gfx_fill_rect(int16_t x, int16_t y, int16_t w, int16_t h,
                      color_rgb_t color) {
  for (int16_t xx = x; xx < x + w; xx++) {
    for (int16_t yy = y; yy < y + h; yy++) {
      if (xx > 0 && xx < WIDTH && yy > 0 && yy < HEIGHT) {
        __set(xx, yy, color);
      }
    }
  }
}

/**
 * @brief Fill the display with a specific color
 */
void ff_gfx_fill(color_rgb_t color) {
  for (uint8_t y = 0; y < HEIGHT; y++) {
    for (uint8_t x = 0; x < WIDTH; x++) {
      __set(x, y, color);
    }
  }
}

/**
 * @brief Push the LED buffer. This is used for speed purposes
 */
void ff_gfx_push_buffer() {
#ifdef CONFIG_IS31FL3741
  is31fl3741_push_buffer();
#endif
}

/**
 * @brief Measure the length in pixels of a given string
 * @param string  : Null terminated string to measure
 * @return Width in pixels of the string
 */
inline int16_t ff_gfx_text_width(char *string) {
  return strlen(string) * FF_GFX_FONT_CHAR_WIDTH;
}