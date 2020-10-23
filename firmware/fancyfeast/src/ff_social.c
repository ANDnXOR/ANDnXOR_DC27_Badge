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
#include <crc.h>
#include <fs.h>
#include <init.h>
#include <shell/shell.h>
#include <zephyr.h>
#define LOG_LEVEL 4
#include <logging/log.h>
LOG_MODULE_REGISTER(ff_social);

#include "ff_fs.h"
#include "ff_gfx.h"
#include "ff_social.h"
#include "ff_ui.h"
#include "mesh/ff_mesh.h"
#include "mesh/ff_mesh_model_social.h"
#include "system.h"

#define SOCIAL_INIT_PRIORITY 99
#define SOCIAL_FILE FF_FS_MOUNT_POINT "/STASH"
#define SOCIAL_XOR_SIZE 64
static const uint8_t social_xor[SOCIAL_XOR_SIZE] = {
    0x62, 0x00, 0xa0, 0x47, 0x8b, 0xaf, 0x55, 0x73, 0x8d, 0x56, 0x68,
    0xfe, 0xf7, 0x48, 0x21, 0xf2, 0x94, 0xff, 0xd8, 0x62, 0x3a, 0xf9,
    0xea, 0x4e, 0x32, 0x19, 0x35, 0xea, 0xbd, 0xcb, 0xe9, 0x72, 0xdd,
    0xb7, 0x94, 0xbd, 0xc4, 0xf8, 0x61, 0x67, 0x9f, 0x2b, 0xd2, 0x30,
    0x26, 0x16, 0x4a, 0xf9, 0x3e, 0x6e, 0x2e, 0xc4, 0x1e, 0xad, 0x81,
    0x98, 0x2a, 0xd1, 0xc9, 0x46, 0xfc, 0xae, 0xf5, 0x7c};

typedef enum { social_state_none, social_state_synced } social_state_t;

// Counter that is incremented during touch
static uint8_t m_social_touch_count = 0;

// In memory array of badges we've synced with
static social_state_t m_social_sync_status[NODE_COUNT];

/**
 * @brief Reset the sync status
 */
static void __reset() {
  memset(m_social_sync_status, 0, sizeof(social_state_t) * NODE_COUNT);
  // Ensure synced with self
  if (ff_mesh_addr_get() < NODE_COUNT) {
    m_social_sync_status[ff_mesh_addr_get()] = social_state_synced;
  }
}

/**
 * @brief Load the social sync status in an obscured format from the internal
 * flash
 */
static int __load(struct device *device) {
  size_t byte_count = sizeof(social_state_t) * NODE_COUNT;
  size_t offset = 0;
  uint32_t crc;
  bool valid = true;

  LOG_DBG("Loading social sync from '%s'", SOCIAL_FILE);

  // Clear social sync status as a good starting point
  memset(m_social_sync_status, 0, byte_count);

  // Open the social file
  struct fs_file_t file;
  if (fs_open(&file, SOCIAL_FILE)) {
    LOG_ERR("Cannot open social sync status file");
    return -1;
  }

  // Read 64 bytes a time
  while (offset < byte_count) {
    uint8_t temp[SOCIAL_XOR_SIZE];
    if (fs_read(&file, temp, SOCIAL_XOR_SIZE) != SOCIAL_XOR_SIZE) {
      LOG_ERR("Error reading social state file, aborting");
      valid = false;
      break;
    }

    // De-obfuscate the data
    for (uint8_t i = 0; i < SOCIAL_XOR_SIZE; i++) {
      m_social_sync_status[offset] = temp[i] ^ (social_xor[i] + offset);
      offset++;
    }
  }

  // read, compute, and validate the CRC
  if (fs_read(&file, &crc, sizeof(crc)) == sizeof(crc) && valid) {
    uint32_t temp_crc =
        crc32_ieee((uint8_t *)&m_social_sync_status, byte_count);
    valid = (crc == temp_crc);
    if (!valid) {
      LOG_ERR("Social sync CRC failed, calculated 0x%08x expected 0x%08x",
              temp_crc, crc);
    }
  } else {
    LOG_ERR("Unable to read social sync CRC");
    valid = false;
  }
  fs_close(&file);

  // Make sure social sync status gets cleared in the event something went wrong
  if (!valid) {
    __reset();
  } else {
    // Make sure they're sync with themselves
    if (ff_mesh_addr_get() < NODE_COUNT) {
      m_social_sync_status[ff_mesh_addr_get()] = social_state_synced;
    }
  }

  return 0;
}

/**
 * @brief Save the social sync status in an obscured format to the internal
 * flash
 */
static void __save() {
  size_t byte_count = sizeof(social_state_t) * NODE_COUNT;
  size_t offset = 0;
  uint32_t crc = crc32_ieee((uint8_t *)&m_social_sync_status, byte_count);
  LOG_DBG("Saving social sync to '%s'", SOCIAL_FILE);

  // Open the social file
  struct fs_file_t file;
  if (fs_open(&file, SOCIAL_FILE)) {
    LOG_ERR("Cannot open social sync status file");
    return;
  }

  // Write 64 bytes at a time
  while (offset < byte_count) {
    uint8_t temp[SOCIAL_XOR_SIZE];
    for (uint8_t i = 0; i < SOCIAL_XOR_SIZE; i++) {
      temp[i] = m_social_sync_status[offset] ^ (social_xor[i] + offset);
      offset++;
    }
    fs_write(&file, temp, SOCIAL_XOR_SIZE);
  }

  fs_write(&file, &crc, sizeof(crc));
  fs_close(&file);
}

/**
 * @brief Shell command to list contents of a directory
 * @param shell		Pointer to shell context
 * @param argc		Number of arguments
 * @param argv		Command arguments
 * @return Result of the command
 */
static int __cmd_social(const struct shell *shell, size_t argc, char **argv) {
  char *emoji = ":(";
  uint16_t count = ff_social_count();
  if (count > 300) {
    emoji = ":D";
  } else if (count > 100) {
    emoji = ":)";
  }

  shell_fprintf(shell, SHELL_VT100_COLOR_CYAN, "%d FRENDZ %s\n\n", count, emoji);
  for (uint16_t i = 0; i < NODE_COUNT; i++) {
    if (m_social_sync_status[i] == social_state_synced) {
      shell_fprintf(shell, SHELL_VT100_COLOR_YELLOW, "%d\t", i);
    }
  }
  shell_fprintf(shell, SHELL_VT100_COLOR_YELLOW, "\n\n");
  return 0;
}

// static int __cmd_social_test(const struct shell *shell, size_t argc,
//                              char **argv) {
//   __save();
//   __load();
//   return 0;
// }

/**
 * @brief Get the current count of badges synced with
 */
uint16_t ff_social_count() {
  uint16_t count = 0;
  for (uint16_t i = 0; i < NODE_COUNT; i++) {
    if (m_social_sync_status[i] == social_state_synced) {
      count++;
    }
  }

  return count;
}

/**
 * @brief Handler for social sync events. Animate the display and track which
 * badges have been synced with here
 */
void ff_social_handler() {

  // While button is pushed, handle social features
  if (ff_ui_is_touched_button()) {
    // Clear the current address, we'll check in a few ms if something comes
    // in
    ff_mesh_model_social_last_clear();

    // Broadcast our social beacon
    ff_mesh_model_social_publish_now();

    // An animation
    if (m_social_touch_count % 2 == 0) {
      ff_gfx_draw_line(0, 0, WIDTH, 0, COLOR_BLUE);
      ff_gfx_draw_line(0, HEIGHT - 1, WIDTH, HEIGHT - 1, COLOR_RED);
    } else {
      ff_gfx_draw_line(0, 0, WIDTH, 0, COLOR_RED);
      ff_gfx_draw_line(0, HEIGHT - 1, WIDTH, HEIGHT - 1, COLOR_BLUE);
    }
    ff_gfx_push_buffer();
    k_sleep(100);

    // An address came in?
    int16_t addr = ff_mesh_model_social_last_get();
    if (addr >= 0 && addr <= NODE_COUNT) {
      // New Badge
      if (m_social_sync_status[addr] == social_state_none) {
        m_social_sync_status[addr] = social_state_synced;
        ff_gfx_fill(COLOR_GREEN);
        ff_gfx_push_buffer();

        // Persist
        __save();

        // Wait until user releases
        while (ff_ui_is_touched_button()) {
          k_sleep(10);
        }
      }
      // Already synced badge
      else {
        ff_gfx_fill(COLOR_YELLOW);
        ff_gfx_push_buffer();
        k_sleep(500);
      }
    }

    m_social_touch_count++;
  }
}

/**
 * @brief Reset the social state
 */
void ff_social_reset() {
  int err = fs_unlink(SOCIAL_FILE);
  if (err) {
    LOG_ERR("Unable to delete '%s' [%d]", SOCIAL_FILE, err);
  }
}

SHELL_CMD_REGISTER(FRENDZ, NULL, "WHO R FRENDZ", __cmd_social);
// SHELL_CMD_REGISTER(test_social, NULL, "Test Social", __cmd_social_test);
SYS_INIT(__load, APPLICATION, SOCIAL_INIT_PRIORITY);