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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <misc/reboot.h>
#include <shell/shell.h>

#include "nrf52840.h"

#define LOG_LEVEL 4
#include <logging/log.h>
LOG_MODULE_REGISTER(ff_badgebus);

#include "bling/ff_bling_fade.h"
#include "bling/ff_bling_rainbow.h"
#include "bling/ff_bling_rgb.h"
#include "bling/ff_bling_scroll.h"
#include "ff_badgebus.h"
#include "ff_bling.h"

/**
 * @brief Shell command that processes badge bus commands
 * @param shell		Pointer to shell context
 * @param argc		Number of command arguments
 * @param argv		Pointer to array of command arguments
 * @return 0 if successful
 */
static int __cmd_bus(const struct shell *shell, size_t argc, char **argv) {
  // Expected format
  // bus <hex data>
  if (argc != 2) {
    shell_error(shell, "Invalid arguments. Expected bus <data>");
  }

  uint8_t packet[BADGEBUS_MAX_LENGTH];
  uint8_t response[BADGEBUS_MAX_LENGTH];
  const char *pos = argv[1];
  size_t length = 0;

  /* WARNING: no sanitization or error-checking whatsoever */
  while (pos < strchr(argv[1], 0)) {
    char buf[5];
    sprintf(buf, "0x%c%c", pos[0], pos[1]);
    packet[length] = strtol(buf, NULL, 0);
    pos += 2;
    length++;
  }

  int32_t response_size =
      ff_badgebus_handle(packet, length, response, BADGEBUS_MAX_LENGTH);

  // Print raw hex response
  for (int32_t i = 0; i < response_size; i++) {
    shell_fprintf(shell, SHELL_NORMAL, "%02x", response[i]);
  }
  shell_fprintf(shell, SHELL_NORMAL, "\n");

  return 0;
}

// This is a badge bus coordinator in the spirit (maybe even compliant) with
// tymkrs badge bus spec https://github.com/Wireb/badge_bus/wiki

void __init_packet(uint8_t *p_packet, uint32_t length) {
  if (length < BADGEBUS_MIN_LENGTH) {
    return;
  }

  p_packet[0] = BADGEBUS_SYNC_BYTE;
  p_packet[1] = BADGEBUS_SYNC_BYTE;
  p_packet[2] = BADGEBUS_HEADER_BYTE;
  p_packet[3] = BADGEBUS_HEADER_BYTE;
  p_packet[BADGEBUS_OFFSET_STATUS] = 0x00; // no CRC
  p_packet[length - 2] = 0x00;
  p_packet[length - 1] = 0x00;
}

/**
 * @brief Handle a badgebus message as a slave
 * @param p_packet			Pointer to packet data
 * @param length			Length of data in the packet
 * @param p_response 		Buffer for response from slave
 * @param response_buff_size 	Size of buffer to use for response
 * @return Size of response
 */
int32_t ff_badgebus_handle(uint8_t *p_packet, size_t length,
                           uint8_t *p_response, size_t response_buff_size) {
  uint8_t status;
  uint8_t command;
  uint8_t data_length;
  uint16_t crc;
  int32_t response_size = BADGEBUS_MIN_LENGTH;

  // Setup response
  __init_packet(p_response, response_buff_size);
  p_response[BADGEBUS_OFFSET_STATUS] = BADGEBUS_STATUS_RESPONSE_MASK;

  LOG_DBG("Length=%d\n", length);
  // LOG_HEXDUMP_DBG(p_packet, length, "Packet");

  if (length < BADGEBUS_MIN_LENGTH) {
    LOG_ERR("Packet too short");
    p_response[BADGEBUS_OFFSET_LENGTH] = 0;
    p_response[BADGEBUS_OFFSET_STATUS] |= BADGEBUS_STATUS_CMD_MASK;
    response_size = BADGEBUS_MIN_LENGTH;
    return response_size;
  }

  // All packets start off with 0x55 0x55
  if (p_packet[0] != BADGEBUS_SYNC_BYTE || p_packet[1] != BADGEBUS_SYNC_BYTE) {
    LOG_ERR("Packet missing sync bytes");
    p_response[BADGEBUS_OFFSET_LENGTH] = 0;
    p_response[BADGEBUS_OFFSET_STATUS] |= BADGEBUS_STATUS_CMD_MASK;
    response_size = BADGEBUS_MIN_LENGTH;
    return response_size;
  }

  // Then packets follow with 0x42 0x42
  if (p_packet[2] != BADGEBUS_HEADER_BYTE &&
      p_packet[3] != BADGEBUS_HEADER_BYTE) {
    LOG_ERR("Packet missing header");
    p_response[BADGEBUS_OFFSET_LENGTH] = 0;
    p_response[BADGEBUS_OFFSET_STATUS] |= BADGEBUS_STATUS_CMD_MASK;
    response_size = BADGEBUS_MIN_LENGTH;
    return response_size;
  }

  // Status byte is next
  status = p_packet[BADGEBUS_OFFSET_STATUS];

  // Command comes after status
  command = p_packet[BADGEBUS_OFFSET_CMD];
  p_response[BADGEBUS_OFFSET_CMD] = command;

  // Length of data payload
  data_length = p_packet[BADGEBUS_OFFSET_LENGTH];

  // Ensure they didn't give us invalid data length
  if ((data_length + BADGEBUS_MIN_LENGTH) != length) {
    LOG_ERR("Packet length does not match data length=%d expected=%d", length,
            (data_length + BADGEBUS_MIN_LENGTH));
    p_response[BADGEBUS_OFFSET_LENGTH] = 0;
    p_response[BADGEBUS_OFFSET_STATUS] |= BADGEBUS_STATUS_LONG_MASK;
    response_size = BADGEBUS_MIN_LENGTH;
    return response_size;
  }

  // Grab the CRC off the end
  crc = (uint16_t)p_packet[BADGEBUS_OFFSET_DATA + data_length];

  LOG_DBG("Packet parsed status=0x%02x command=0x%02x len=0x%02x crc=0x%04x",
          status, command, data_length, crc);
  if (data_length > 0) {
    LOG_HEXDUMP_DBG(p_packet + BADGEBUS_OFFSET_DATA, data_length, "DATA");
  }

  switch (command) {
  case BADGEBUS_CMD_MAKER_ID:
    // Respond to a maker id request
    // Response will be minimum plus 2 byte payload for maker id
    response_size += 2;
    if (response_buff_size < response_size) {
      LOG_ERR("Response buffer too short %d < %d", response_buff_size,
              response_size);
      p_response[BADGEBUS_OFFSET_LENGTH] = 0;
      p_response[BADGEBUS_OFFSET_STATUS] |= BADGEBUS_STATUS_DATA_MASK;
      response_size = BADGEBUS_MIN_LENGTH;
      break;
    }

    p_response[BADGEBUS_OFFSET_LENGTH] = 2;
    *(uint16_t *)(p_response + BADGEBUS_OFFSET_DATA) = BADGEBUS_MID_ANDNXOR;
    break;

  case BADGEBUS_CMD_BADGE_ID:
    // Respond to a badge id request
    // Response will be minimum plus 2 byte payload for badge id
    response_size += 2;
    if (response_buff_size < response_size) {
      LOG_ERR("Response buffer too short %d < %d", response_buff_size,
              response_size);
      p_response[BADGEBUS_OFFSET_LENGTH] = 0;
      p_response[BADGEBUS_OFFSET_STATUS] |= BADGEBUS_STATUS_DATA_MASK;
      response_size = BADGEBUS_MIN_LENGTH;
      break;
    }

    p_response[BADGEBUS_OFFSET_LENGTH] = 2;
    *(uint16_t *)(p_response + BADGEBUS_OFFSET_DATA) = BADGEBUS_BID_DC27;
    break;

  case BADGEBUS_CMD_MAKER_ID_HUMAN_READABLE:
    // Respond to a maker id human readable request
    // Response will be minimum plus n byte payload for badge id
    response_size += strlen(BADGEBUS_MIDH_ANDNXOR);
    if (response_buff_size < response_size) {
      LOG_ERR("Response buffer too short %d < %d", response_buff_size,
              response_size);
      p_response[BADGEBUS_OFFSET_LENGTH] = 0;
      p_response[BADGEBUS_OFFSET_STATUS] |= BADGEBUS_STATUS_DATA_MASK;
      response_size = BADGEBUS_MIN_LENGTH;
      break;
    }

    p_response[BADGEBUS_OFFSET_LENGTH] = strlen(BADGEBUS_MIDH_ANDNXOR);
    memcpy(p_response + BADGEBUS_OFFSET_DATA, BADGEBUS_MIDH_ANDNXOR,
           strlen(BADGEBUS_MIDH_ANDNXOR));
    break;

  case BADGEBUS_CMD_BADGE_ID_HUMAN_READABLE:
    // Respond to a badge id human readable request
    // Response will be minimum plus n byte payload for badge id
    response_size += strlen(BADGEBUS_BIDH_DC27);
    if (response_buff_size < response_size) {
      LOG_ERR("Response buffer too short %d < %d", response_buff_size,
              response_size);
      p_response[BADGEBUS_OFFSET_LENGTH] = 0;
      p_response[BADGEBUS_OFFSET_STATUS] |= BADGEBUS_STATUS_DATA_MASK;
      response_size = BADGEBUS_MIN_LENGTH;
      break;
    }

    p_response[BADGEBUS_OFFSET_LENGTH] = strlen(BADGEBUS_BIDH_DC27);
    memcpy(p_response + BADGEBUS_OFFSET_DATA, BADGEBUS_BIDH_DC27,
           strlen(BADGEBUS_BIDH_DC27));
    break;

  case BADGEBUS_CMD_BADGE_SERIAL_NUMBER:
    // Respond to a serial number request
    response_size += 8;
    if (response_buff_size < response_size) {
      LOG_ERR("Response buffer too short %d < %d", response_buff_size,
              response_size);
      p_response[BADGEBUS_OFFSET_LENGTH] = 0;
      p_response[BADGEBUS_OFFSET_STATUS] |= BADGEBUS_STATUS_DATA_MASK;
      response_size = BADGEBUS_MIN_LENGTH;
      break;
    }

    p_response[BADGEBUS_OFFSET_LENGTH] = 8;
    memcpy(p_response + BADGEBUS_OFFSET_DATA, (void *)NRF_FICR->DEVICEID, 8);
    break;

  // Generic control command
  case BADGEBUS_CMD_GENERIC_CONTROL:;
    uint8_t gcn = p_packet[BADGEBUS_OFFSET_DATA];
    LOG_DBG("GCN command=0x%02x", gcn);

    if (gcn == 0x99) {
      sys_reboot(SYS_REBOOT_COLD);

    } else if (gcn == 0xDC) {
      FF_BLING_DEFAULT_SCROLL(scroll, "Never gonna give you up");
      ff_bling_mode_push(scroll);
    }
    break;

  // Generic status, always return FF
  case BADGEBUS_CMD_GENERIC_STATUS:
    // Allowed 1 byte of status
    response_size += 1;

    if (response_buff_size < response_size) {
      LOG_ERR("Response buffer too short %d < %d", response_buff_size,
              response_size);
      p_response[BADGEBUS_OFFSET_LENGTH] = 0;
      p_response[BADGEBUS_OFFSET_STATUS] |= BADGEBUS_STATUS_DATA_MASK;
      response_size = BADGEBUS_MIN_LENGTH;
      break;
    }

    p_response[BADGEBUS_OFFSET_LENGTH] = 1;
    p_response[BADGEBUS_OFFSET_DATA] = 0xFF;
    break;

    // Handle generic human readable command with fixed data
  case BADGEBUS_CMD_GENERIC_CONTROL_HUMAN_READABLE:
    response_size += strlen(BADBEBUS_GCN_HUMAN);
    if (response_buff_size < response_size) {
      LOG_ERR("Response buffer too short %d < %d", response_buff_size,
              response_size);
      p_response[BADGEBUS_OFFSET_LENGTH] = 0;
      p_response[BADGEBUS_OFFSET_STATUS] |= BADGEBUS_STATUS_DATA_MASK;
      response_size = BADGEBUS_MIN_LENGTH;
      break;
    }
    p_response[BADGEBUS_OFFSET_LENGTH] = strlen(BADBEBUS_GCN_HUMAN);
    memcpy(p_response + BADGEBUS_OFFSET_DATA, BADBEBUS_GCN_HUMAN,
           strlen(BADBEBUS_GCN_HUMAN));
    break;

  // Handle generic human readable status with fixed data
  case BADGEBUS_CMD_GENERIC_STATUS_HUMAN_READABLE:
    response_size += strlen(BADGEBUS_STATUS_HUMAN);
    if (response_buff_size < response_size) {
      LOG_ERR("Response buffer too short %d < %d", response_buff_size,
              response_size);
      p_response[BADGEBUS_OFFSET_LENGTH] = 0;
      p_response[BADGEBUS_OFFSET_STATUS] |= BADGEBUS_STATUS_DATA_MASK;
      response_size = BADGEBUS_MIN_LENGTH;
      break;
    }
    p_response[BADGEBUS_OFFSET_LENGTH] = strlen(BADGEBUS_STATUS_HUMAN);
    memcpy(p_response + BADGEBUS_OFFSET_DATA, BADGEBUS_STATUS_HUMAN,
           strlen(BADGEBUS_STATUS_HUMAN));
    break;

  // Get available baud rate bitmask
  case BADGEBUS_CMD_AVAILABLE_BAUD_RATES:
    // Allowed 1 byte of baud rate
    response_size += 1;

    if (response_buff_size < response_size) {
      LOG_ERR("Response buffer too short %d < %d", response_buff_size,
              response_size);
      p_response[BADGEBUS_OFFSET_LENGTH] = 0;
      p_response[BADGEBUS_OFFSET_STATUS] |= BADGEBUS_STATUS_DATA_MASK;
      response_size = BADGEBUS_MIN_LENGTH;
      break;
    }

    p_response[BADGEBUS_OFFSET_LENGTH] = 1;
    p_response[BADGEBUS_OFFSET_DATA] = BADBEBUS_BAUD_RATE_BITMASK;
    break;

  // Set baud rate, in our case only set 115200
  case BADGEBUS_CMD_SET_BAUD_RATE:
    // Return an invalid command response
    p_response[BADGEBUS_OFFSET_STATUS] =
        BADGEBUS_STATUS_CMD_MASK | BADGEBUS_STATUS_RESPONSE_MASK;
    break;

  // Custom bling command. It expects one byte of data for now indicating the
  // mode
  case BADGEBUS_CMD_CUSTOM_BLING:;
    uint8_t bling_mode = 0;

    if (data_length > 0) {
      bling_mode = p_packet[BADGEBUS_OFFSET_DATA];
      if (bling_mode >= __bling_mode_counter) {
        bling_mode = __bling_mode_counter - 1;
      }
      LOG_DBG("Bling mode parsed %d", bling_mode);

      switch (bling_mode) {
      case ff_bling_mode_rainbow:;
        FF_BLING_DEFAULT_RAINBOW(rainbow);
        ff_bling_mode_push(rainbow);
        break;
      case ff_bling_mode_scroll_time:;
        FF_BLING_DEFAULT_SCROLL_TIME(time);
        ff_bling_mode_push(time);
        break;
      case ff_bling_mode_scroll:;
        FF_BLING_DEFAULT_SCROLL(scroll, "DEFCON 27");
        ff_bling_mode_push(scroll);
        break;
      case ff_bling_mode_fade:;
        FF_BLING_DEFAULT_FADE(fade);
        ff_bling_mode_push(fade);
        break;
      case ff_bling_mode_rgb:;
        FF_BLING_DEFAULT_RGB(rgb, FF_BLING_DEFAULT_RGB_MODE);
        ff_bling_mode_push(rgb);
        break;
      }
      break;
    default:
      p_response[BADGEBUS_OFFSET_LENGTH] = 0;
      p_response[BADGEBUS_OFFSET_STATUS] |= BADGEBUS_STATUS_CMD_MASK;
      response_size = BADGEBUS_MIN_LENGTH;
      break;
    }
  }

  return response_size;
}

// void ff_badgebus_test() {
//   LOG_DBG("Testing badgebus");
//   int ret;
//   uint8_t packet[BADGEBUS_MIN_LENGTH];
//   uint8_t response[BADGEBUS_MIN_LENGTH + 2];

//   __init_packet(packet, BADGEBUS_MIN_LENGTH);
//   packet[BADGEBUS_OFFSET_CMD] = BADGEBUS_CMD_MAKER_ID;
//   packet[BADGEBUS_OFFSET_LENGTH] = 0;
//   ret = ff_badgebus_handle(packet, BADGEBUS_MIN_LENGTH, response,
//   BADGEBUS_MIN_LENGTH + 2);

//   LOG_DBG("Badge bus handler returned %d", ret);
//   LOG_HEXDUMP_DBG(response, BADGEBUS_MIN_LENGTH + 2, "Response");
//   if (ret == BADGEBUS_OK) {
//     uint16_t mid = *(uint16_t*)(response + BADGEBUS_OFFSET_DATA);
//     LOG_DBG("MID=0x%04x", mid);
//   }
// }

SHELL_STATIC_SUBCMD_SET_CREATE(sub_bus,
                               SHELL_CMD(BUS, NULL, "PROCEZ PACKET K",
                                         __cmd_bus),
                               SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(BADGE, &sub_bus, "TYMKRS PROTOKOLZ", NULL);