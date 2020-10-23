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

#ifndef FF_GFX_H
#define FF_GFX_H

#include <zephyr.h>

#define WIDTH 17
#define HEIGHT 9
#define FRAME_SIZE_BYTES (WIDTH * HEIGHT * 3)

typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} __packed color_rgb_t;

#define COLOR_BLACK                                                            \
  (color_rgb_t) { 0, 0, 0 }
#define COLOR_BLUE                                                             \
  (color_rgb_t) { 0, 0, 255 }
#define COLOR_GREEN                                                            \
  (color_rgb_t) { 0, 255, 0 }
#define COLOR_ORANGE                                                           \
  (color_rgb_t) { 255, 220, 0 }
#define COLOR_RED                                                              \
  (color_rgb_t) { 255, 0, 0 }
#define COLOR_WHITE                                                            \
  (color_rgb_t) { 255, 255, 255 }
#define COLOR_YELLOW                                                           \
  (color_rgb_t) { 255, 255, 0 }

#define HUE_RED 0.0
#define HUE_GREEN 0.37
#define HUE_BLUE 0.6667

/**
 * Initialize the graphics layer
 */
extern int ff_gfx_init();

extern color_rgb_t ff_gfx_color_hsv_to_rgb(float H, float S, float V);

/**
 * @brief Draw a bitmap as a single frame
 * @param p_frame   Pointer to raw frame bitmap in memory
 * @param fg        Foreground color
 * @param bg        Background color
 */
extern void ff_gfx_draw_bitmap(uint8_t *p_frame, color_rgb_t fg,
                               color_rgb_t bg);

extern void ff_gfx_draw_circle(int16_t x, int16_t y, int16_t r,
                               color_rgb_t color);

extern void ff_gfx_draw_char(int16_t x, int16_t y, char c, color_rgb_t color,
                             color_rgb_t bg);

extern void ff_gfx_draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                             color_rgb_t color);

/**
 * @brief Draw a single pixel in any color to the LED matrix
 * NOTE: This draws immediately and is currently unbuffered. This may change in
 * the future
 *
 * @param x		X coordinate of the pixel
 * @param y		Y coordinate of the pixel
 * @param color	RGB (24-bit) color of the pixel to draw
 */
extern void ff_gfx_draw_pixel(int16_t x, int16_t y, color_rgb_t color);

/**
 * @brief Draw an XBM bitmap
 * @param x   Leftmost coordinate of bitmap
 * @param y   Topmost coordinate of bitmap
 * @param w   Width of bitmap
 * @param y   Height of bitmap
 * @param fg  Foreground color to use
 * @param bg  Background color to use
 */
extern void ff_gfx_draw_xbm(uint8_t *p_xbm, int16_t x, int16_t y, uint8_t w,
                            uint8_t h, color_rgb_t fg, color_rgb_t bg);

extern void ff_gfx_fill_rect(int16_t x, int16_t y, int16_t w, int16_t h,
                             color_rgb_t color);
extern void ff_gfx_fill(color_rgb_t color);

/**
 * @brief Print a string to the display
 * @param string  : The string to print
 * @param x       : The starting x coordinate
 * @param y       : The starting y coordinate
 * @param color   : The foreground color
 * @param bg      : The background color to draw with
 */
extern void ff_gfx_print(int16_t x, int16_t y, char *string, color_rgb_t color,
                         color_rgb_t bg);

/**
 * @brief Push the LED buffer. This is used for speed purposes
 */
extern void ff_gfx_push_buffer();

/**
 * @brief Measure the length in pixels of a given string
 * @param string  : Null terminated string to measure
 * @return Width in pixels of the string
 */
extern int16_t ff_gfx_text_width(char *string);
#endif