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
 * ADDITIONALLY:  shell_print(shell, "LED Brightness set to: %d",
 m_settings.brightness); is31fl3741_gcc_set(m_settings.brightness);
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


#ifndef FF_SETTINGS_H
#define FF_SETTINGS_H

#define NAME_MAX_LENGTH 8
#define GRAFFITI_COUNT 8
#define GRAFFITI_MAX_LENGTH 32

#include "ff_addon.h"
#include "ff_bender.h"

typedef struct {
  char name[NAME_MAX_LENGTH + 1]; //+1 for null termination
  uint8_t brightness;
  bool proxy;
  uint16_t unlock_state;
  uint32_t time_sec;
  float bling_hue_matrix;
  float bling_hue_sin;
  float bling_hue_scroll;
  bool bling_rager;
  uint8_t scroll_sensitivity;
  uint8_t unlock;
  bender_data_t bender_state;
  addon_gpio_mode_t addon_gpio_mode;
  uint8_t padding[7]; //For future use
  char wall[GRAFFITI_COUNT][GRAFFITI_MAX_LENGTH + 1];
  uint32_t crc32;
} settings_t;

/**
 * Initialize the settings module
 */
extern void ff_settings_init();

/**
 * @brief Get a pointer to the current settings object
 */
extern settings_t *ff_settings_ptr_get();

/**
 * @brief Get a pointer to the current settings object
 */
extern bender_data_t* ff_settings_bender_ptr_get();

/**
 * @brief Factory reset and reboot badge
 */
extern void ff_settings_reset();

/**
 * @brief Save the settings to the filesystem
 */
extern void ff_settings_save();

#endif