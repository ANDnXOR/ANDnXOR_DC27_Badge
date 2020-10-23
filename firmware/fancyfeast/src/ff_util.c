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

#include <device.h>
#include <dfu/flash_img.h>
#include <dfu/mcuboot.h>
#include <fs.h>
// #ifdef CONFIG_MCUMGR_CMD_FS_MGMT
// #include <fs_mgmt/fs_mgmt.h>
// #endif
#include <img_mgmt/img_mgmt.h>
#include <mbedtls/md5.h>
#include <mgmt/smp.h>
#include <misc/reboot.h>
#include <os_mgmt/os_mgmt.h>
#include <shell/shell.h>
#include <stat_mgmt/stat_mgmt.h>
#include <zephyr.h>

#define LOG_LEVEL 4
#include <logging/log.h>
LOG_MODULE_REGISTER(ff_util);

#include "autoconf.h"
#include "ff_fs.h"
#include "ff_unlocks.h"
#include "ff_util.h"

#define AWESUM_SALT "PEPPER"
#define AWESUM_SALT_LEN 6

uint8_t m_whoami_count = 0;

/**
 * @brief Shell command performs a DFU
 * NOTE: THIS COMMAND HAS BEEN REMOVED AND WILL NOT BE COMPILED IN, CODE IS KEPT
 * HERE FOR FUTURE ZAPP IN CASE HE NEEDS IT
 * @param shell		Pointer to shell context
 * @param argc		Number of command arguments
 * @param argv		Pointer to array of command arguments
 * @return 0 if successful
 */
// static int __cmd_dfu(const struct shell *shell, size_t argc, char **argv) {
//   int err;
//   if (argc != 2) {
//     shell_error(shell, "Invalid arguments");
//     return -1;
//   }

//   char path[FF_FS_MAX_PATH_LEN];
//   snprintf(path, FF_FS_MAX_PATH_LEN, FF_FS_MOUNT_POINT "/%s", argv[1]);

//   shell_print(shell, "Performing DFU with %s.", path);

//   struct fs_dirent dirent;
//   err = fs_stat(path, &dirent);
//   if (err) {
//     shell_error(shell, "Unable to stat '%s'", argv[1]);
//     return -1;
//   }
//   size_t bytes_left = dirent.size;

//   shell_print(shell, "Firmware size: %d bytes", dirent.size);

//   struct fs_file_t file;
//   err = fs_open(&file, path);
//   if (err) {
//     shell_error(shell, "Unable to open '%s' [%d]", path, err);
//     return -1;
//   }

//   // Setup flash context
//   struct flash_img_context image_context;
//   flash_img_init(&image_context);

//   // Erase the slot
//   shell_print(shell, "Erasing slot 1, this may take a minute...");
//   boot_erase_img_bank(DT_FLASH_AREA_IMAGE_0_ID);

//   uint8_t buffer[CONFIG_IMG_BLOCK_BUF_SIZE];
//   while (bytes_left > 0) {
//     ssize_t bytes_read = fs_read(&file, buffer, CONFIG_IMG_BLOCK_BUF_SIZE);
//     if (bytes_read < 0) {
//       shell_error(shell, "Error while reading file. [%d]", bytes_read);
//       fs_close(&file);
//       return -1;
//     }
//     bytes_left -= bytes_read;

//     // Write to internal flash slot 1, flush if this was the last of the data
//     err = flash_img_buffered_write(&image_context, buffer, bytes_read,
//                                    (bytes_left == 0));
//     if (err) {
//       shell_error(shell, "Error writing to slot 1 [%d]", err);
//       fs_close(&file);
//       return -1;
//     }

//     // Output some status
//     shell_print(shell, "DFU %d bytes, %d bytes remaining", bytes_read,
//                 bytes_left);
//   }

//   fs_close(&file);

//   // Validate image

//   // Swap
//   boot_request_upgrade(true);

//   // Reboot
//   sys_reboot(SYS_REBOOT_COLD);

//   return 0;
// }

/**
 * @brief Shell runs an md5sum on a string
 * @param shell		Pointer to shell context
 * @param argc		Number of command arguments
 * @param argv		Pointer to array of command arguments
 * @return 0 if successful
 */
static int __cmd_md5(const struct shell *shell, size_t argc, char **argv) {
  if (argc != 2) {
    shell_error(shell, "Invalid argument(s)");
    return -1;
  }

  char md5sum[16];
  ff_util_md5_salted(argv[1], MIN(strlen(argv[1]), 64), md5sum);

  for (uint8_t i = 0; i < 16; i++) {
    shell_fprintf(shell, SHELL_NORMAL, "%02x", md5sum[i]);
  }
  shell_fprintf(shell, SHELL_NORMAL, "\n");

  return 0;
}

/**
 * @brief Shell command that doesn't do anything other than old school AND!XOR
 * whoami
 * @param shell		Pointer to shell context
 * @param argc		Number of command arguments
 * @param argv		Pointer to array of command arguments
 * @return 0 if successful
 */
static int __cmd_whoami(const struct shell *shell, size_t argc, char **argv) {
  switch (m_whoami_count) {
  case 0:
    shell_print(shell, "First, you must ask who do you think you are...");
    break;
  case 1:
    shell_print(shell, "...Only then can you find peace...");
    break;
  case 2:
    shell_print(
        shell, "Look deep into your mind and search for your true identity...");
    break;
  case 3:
    shell_print(shell,
                "As the Dalai Lama once....wait....why do you keep asking? ");
    break;
  case 4:
    shell_print(shell,
                "Okay, I admit.  I don't know... just stop, that's all I ask");
    break;
  case 5:
    shell_print(shell, "STOP, for the love of god...");
    break;
  case 6:
    shell_print(shell, "First, you must ask who do you think you are...");
    break;
  case 7:
    shell_print(shell,
                "Ah, you thought I started over didn't you, but you have to "
                "be so damn persistent");
    break;
  case 8:
    shell_print(shell, "Really have nothing better to do?");
    break;
  case 9:
    shell_print(shell,
                "FINE, you win, SKYNET initialization sequence unlocked");
    FF_UNLOCK_SET(FF_UNLOCK_WHOAMI);
    break;
  }
  m_whoami_count = (m_whoami_count + 1) % 10;
  return 0;
}

/**
 * @brief Initialize the mcumgr modules
 */
void ff_util_mcumgr_init() {
// #ifdef CONFIG_MCUMGR_CMD_FS_MGMT
//   fs_mgmt_register_group();
// #endif
#ifdef CONFIG_MCUMGR_CMD_OS_MGMT
  os_mgmt_register_group();
#endif
#ifdef CONFIG_MCUMGR_CMD_IMG_MGMT
  img_mgmt_register_group();
#endif
#ifdef CONFIG_MCUMGR_CMD_STAT_MGMT
  stat_mgmt_register_group();
#endif
}

/**
 * @brief perform and MD5 salted hash unique to AND!XOR
 * @param input   Pointer to buffer containing data to salt and hash
 * @param len     Length of data in the input buffer
 * @param output  Pointer to buffer where hash should be stored (16 bytes)
 */
void ff_util_md5_salted(uint8_t *input, size_t len, uint8_t *output) {
  mbedtls_md5_context ctx;
  mbedtls_md5_init(&ctx);
  mbedtls_md5_starts_ret(&ctx);
  mbedtls_md5_update_ret(&ctx, input, len);
  mbedtls_md5_update_ret(&ctx, AWESUM_SALT, AWESUM_SALT_LEN);
  mbedtls_md5_finish_ret(&ctx, output);
}

/**
 * @brief Get the current system time in milliseconds
 */
inline uint32_t ff_util_millis() { return z_tick_get() / K_MSEC(1); }

/**
 * @brief Sleep until a specific end time
 * @param end_time_ms	The time in ms to end at
 */
inline void ff_util_sleep_until(uint32_t end_time_ms) {
  uint32_t now = ff_util_millis();
  if (end_time_ms > now) {
    // Sleep only the delta time
    k_sleep(end_time_ms - now);
  } else {
    // At least be a nice thread
    k_yield();
  }
}

/**
 * @brief Look for suffix in end of string
 * @param str     Pointer to string array
 * @param suffix  Pointer to string array of suffix to look for
 * @return 0 if str ends with suffix
 */
int ff_util_string_ends_with(const char *str, const char *suffix) {
  if (!str || !suffix)
    return 0;

  size_t lenstr = strlen(str);
  size_t lensuffix = strlen(suffix);
  if (lensuffix > lenstr)
    return 0;

  return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

SHELL_CMD_REGISTER(AWESUM, NULL, "AWESUM HASH", __cmd_md5);
SHELL_CMD_REGISTER(WHOAMI, NULL, "WHO R U", __cmd_whoami);