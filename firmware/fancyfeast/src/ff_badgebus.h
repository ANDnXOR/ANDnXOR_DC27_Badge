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
#ifndef FF_BADGEBUS_H
#define FF_BADGEBUS_H

#define BADGEBUS_MID_ANDNXOR 0x049E
#define BADGEBUS_MIDH_ANDNXOR "AND!XOR"
#define BADGEBUS_BID_DC27 0x2700
#define BADGEBUS_BIDH_DC27 "DC27 Human"
#define BADBEBUS_GCN_HUMAN "LOL!"
#define BADGEBUS_STATUS_HUMAN "Never giving up"
/*
Available baud rates. This command returns a 1 byte packet that is a bit field
of supported baud rates of the slave. The bit values MSB to LSB are: N/A,
256000, 128000, 115200, 57600, 38400, 19200, 14400. The master badge uses this
list to find the highest common denominator to for baud rate switching. If all
bits are clear only 9600 baud is supported.
*/
#define BADBEBUS_BAUD_RATE_BITMASK 0b00010000

// #define BADGEBUS_OK 0
// #define BADGEBUS_ERROR_UNKNOWN -1
// #define BADGEBUS_ERROR_INVALID -2
// #define BADGEBUS_ERROR_INVALID_SYNC -3
// #define BADGEBUS_ERROR_INVALID_HEADER -4
// #define BADGEBUS_ERROR_INVALID_CMD -5

#define BADGEBUS_SYNC_BYTE 0x55
#define BADGEBUS_HEADER_BYTE 0x42

// Minimum length of a badgebus packet
// Sync = 2B, Header = 2B, status = 1B, command = 1B, length = 1B, payload = nB,
// CRC = 2B
#define BADGEBUS_MIN_LENGTH 9
#define BADGEBUS_MAX_LENGTH (BADGEBUS_MIN_LENGTH + 255)

#define BADGEBUS_OFFSET_STATUS 4
#define BADGEBUS_OFFSET_CMD 5
#define BADGEBUS_OFFSET_LENGTH 6
#define BADGEBUS_OFFSET_DATA 7

#define BADGEBUS_CMD_MAKER_ID 0x00
#define BADGEBUS_CMD_BADGE_ID 0x01
#define BADGEBUS_CMD_MAKER_ID_HUMAN_READABLE 0x03
#define BADGEBUS_CMD_BADGE_ID_HUMAN_READABLE 0x04
#define BADGEBUS_CMD_BADGE_SERIAL_NUMBER 0x05
#define BADGEBUS_CMD_GENERIC_CONTROL 0x06
#define BADGEBUS_CMD_GENERIC_STATUS 0x07
#define BADGEBUS_CMD_GENERIC_CONTROL_HUMAN_READABLE 0x08
#define BADGEBUS_CMD_GENERIC_STATUS_HUMAN_READABLE 0x09
#define BADGEBUS_CMD_AVAILABLE_BAUD_RATES 0x0A
#define BADGEBUS_CMD_SET_BAUD_RATE 0x0B

// Custom badgebus commands
#define BADGEBUS_CMD_CUSTOM_BLING 0x80

#define BADGEBUS_STATUS_LONG_MASK 0b00000001
#define BADGEBUS_STATUS_LONG2_MASK 0b00000010
#define BADGEBUS_STATUS_CHECKSUM_MASK 0b00000100
#define BADGEBUS_STATUS_CMD_MASK 0b00001000
#define BADGEBUS_STATUS_DATA_MASK 0b00010000
#define BADGEBUS_STATUS_RESPONSE_MASK 0b01000000
#define BADGEBUS_STATUS_CRC_MASK 0b10000000

/**
 * @brief Handle a badgebus message as a slave
 * @param p_packet				Pointer to packet data
 * @param length				Length of data in the packet
 * @param p_response 			Buffer for response from slave
 * @param response_buff_size 	Length of buffer to use for response
 * @return Size of response, negative value if error
 */
extern int32_t ff_badgebus_handle(uint8_t *p_packet, size_t length,
                                  uint8_t *p_response,
                                  size_t response_buff_size);

extern void ff_badgebus_test();

#endif