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

#ifndef FF_ADDON_H
#define FF_ADDON_H

#define FF_ADDON_MAGIC_BYTE_INDEX 0x00
#define FF_ADDON_MAGIC_BYTE_DEFCON 0x1B
#define FF_ADDON_MAGIC_BYTE_MAKER 0x49
#define FF_ADDON_MAGIC_BYTE_MAKER_CR4BF04M 0x05

#define FF_ADDON_MAGIC_BYTE_TYPE_AUDIO 0xAD
#define FF_ADDON_MAGIC_BYTE_TYPE_BENDER 0xBE 
#define FF_ADDON_MAGIC_BYTE_TYPE_DOOM 0x01
#define FF_ADDON_MAGIC_BYTE_TYPE_EEPROM 0x66 

typedef enum {
  addon_gpio_mode_alternating,
  addon_gpio_mode_off,
  addon_gpio_mode_on_1,
  addon_gpio_mode_on_2,
  addon_gpio_mode_both,
  addon_gpio_mode_rager,
  __addon_gpio_mode_counter
} addon_gpio_mode_t;

#endif