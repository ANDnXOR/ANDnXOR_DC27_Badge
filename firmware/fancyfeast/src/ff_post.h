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
#ifndef FF_POST_H
#define FF_POST_H

#define FF_POST_ADDON_DEVICE 0x0001
#define FF_POST_IS31FL3741_ACK 0x0040
#define FF_POST_IS31FL3741_PRODUCT_ID 0x0080
#define FF_POST_IS31FL3741_OPEN 0x0100
#define FF_POST_IS31FL3741_SHORT 0x0200
#define FF_POST_FS_MOUNT 0x0400
#define FF_POST_IQS333_ACK 0x0800
#define FF_POST_IQS333_DEVICE_INFO 0x1000

//Default POST state is to assume all tests failed
#define FF_POST_DEFAULT                                                        \
  (FF_POST_ADDON_DEVICE | FF_POST_IS31FL3741_ACK |                             \
   FF_POST_IS31FL3741_PRODUCT_ID | FF_POST_IS31FL3741_OPEN |                  \
   FF_POST_IS31FL3741_SHORT | FF_POST_FS_MOUNT | FF_POST_IQS333_ACK |          \
   FF_POST_IQS333_DEVICE_INFO)

extern uint16_t ff_post_state_get();

/**
 * @brief Dump POST status to log
 */
extern void ff_post_dump();

extern void ff_post_failed(uint16_t mask);
extern void ff_post_success(uint16_t mask);

#endif
