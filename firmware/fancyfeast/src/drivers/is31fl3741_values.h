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
#ifndef IS31FL3741_VALUES_H
#define IS31FL3741_VALUES_H

#define IS31FL3741_MAX_ADDRESS 0xB4

#define IS31_REG_CONFIG 0x00
#define IS31_REG_GCC 0x01
#define IS31_REG_PULL_UP_DOWN 0x02
#define IS31_REG_PRODUCT_ID 0xfc
#define IS31_REG_UNLOCK 0xfe
#define IS31_REG_PAGE_WRITE 0xfd
#define IS31_VALUE_CONFIG_SSD_NORMAL 0x1
#define IS31_VALUE_CONFIG_OSDE_DISABLE 0x00
#define IS31_VALUE_CONFIG_OSDE_OPEN 0x02
#define IS31_VALUE_CONFIG_OSDE_SHORT 0x04
#define IS31_VALUE_UNLOCK 0xc5
#define IS31_VALUE_PAGE_0 0x00
#define IS31_VALUE_PAGE_1 0x01
#define IS31_VALUE_PAGE_2 0x02
#define IS31_VALUE_PAGE_3 0x03
#define IS31_VALUE_PAGE_4 0x04
#define IS31_VALUE_PRODUCT_ID 0x60


#define IS31_VALUE_OPEN_SHORT_REG_START_ADDR 0x03
#define IS31_VALUE_OPEN_SHORT_REG_COUNT 45

#define IS31_PULL_UP_DOWN_DEFAULT 0x45

#endif