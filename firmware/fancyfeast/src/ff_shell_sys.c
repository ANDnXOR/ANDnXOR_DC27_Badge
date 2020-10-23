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

#include "nrf52840.h"
#include <fs.h>
#include <misc/reboot.h>
#include <misc/stack.h>
#include <shell/shell.h>
#include <shell/shell_history.h>
#include <version.h>
#include <zephyr.h>
#define LOG_LEVEL 4
#include <logging/log.h>
LOG_MODULE_REGISTER(ff_shell_sys);

#include "ff_fs.h"
#include "ff_settings.h"
#include "ff_shell.h"
#include "ff_social.h"
#include "ff_unlocks.h"
#include "system.h"

#define BANNER                                                                 \
  "\r\n"                                                                       \
  "                              ==\r\n"                                       \
  "                             ==\r\n"                                        \
  "                            ==\r\n"                                         \
  "            ``````````      ==\r\n"                                         \
  "        ```==========```     ==\r\n"                                        \
  " /\\```/\\================`````==\r\n"                                      \
  "(  O O  )=====================\r\n"                                         \
  "=== ^ === ================\r\n"                                             \
  "  \\ O / ==================\r\n"                                            \
  "   ''' ======        =====\r\n"                                             \
  "        ====         ====\r\n"                                              \
  "        ===          ===\r\n"                                               \
  "         ==          ==\r\n"                                                \
  "         ==          ==\r\n"                                                \
  "         ==          ==\r\n"                                                \
  "         WW          WW\r\n"                                                \
  "        ''          ''\r\n"

/**
 * @brief Shell command that inserts a delay
 * @param shell		Pointer to shell context
 * @param argc		Number of command arguments
 * @param argv		Pointer to array of command arguments
 * @return 0 if successful
 */
static int __cmd_paws(const struct shell *shell, size_t argc, char **argv) {
  if (argc != 2) {
    shell_error(shell, "Invalid argument(s)");
    return -1;
  }

  uint32_t ms = (uint32_t)strtol(argv[1], NULL, 10);
  if (ms > (60 * 60 * 1000)) {
    shell_error(shell,
                "Invalid argument(s), cannot PAWS for more than an hour.");
    return -1;
  }

  k_sleep(ms);
  return 0;
}

/**
 * @brief Reboot the badge
 */
static int __cmd_reboot(const struct shell *shell, size_t argc, char **argv) {
  ARG_UNUSED(argc);
  ARG_UNUSED(argv);
  sys_reboot(SYS_REBOOT_COLD);
  return 0;
}

/**
 * @brief Simple shell function that factory resets badge
 * device serial number
 */
static int __cmd_sys(const struct shell *shell, size_t argc, char **argv) {
  if (argc == 2) {

    // Factory reset
    if (strcmp(argv[1], "RESET_ALL") == 0) {
      ff_shell_rc_jerbs_reset();
      ff_social_reset();

      // This will delete settings and restart, needs to happen last
      ff_settings_reset();
    }

    // clear history
    else if (strcmp(argv[1], "CLEAR_HISTORY") == 0) {
      shell_history_purge(shell->history);
      shell_print(shell, "History cleared");
    }
  }
  return -1;
}

/**
 * @brief Simple shell function that returns a hex formatted string representing
 * device serial number
 */
static int __cmd_sys_serial(const struct shell *shell, size_t argc,
                            char **argv) {
  ARG_UNUSED(argc);
  ARG_UNUSED(argv);

  uint32_t a0 = NRF_FICR->DEVICEID[0];
  uint32_t a1 = NRF_FICR->DEVICEID[1];
  shell_fprintf(shell, SHELL_NORMAL, "0x%08x%08x\r\n", a0, a1);

  return 0;
}

/**
 * @brief Shell command that prints the badge version information
 * @param shell		Pointer to shell context
 * @param argc		Number of command arguments
 * @param argv		Pointer to array of command arguments
 * @return 0 if successful
 */
static int __cmd_sys_unlocks(const struct shell *shell, size_t argc, char **argv) {
  ARG_UNUSED(argc);
  ARG_UNUSED(argv);

  uint8_t count = 0;
  shell_print(shell, "Unlock status:");

  if (FF_UNLOCK_VALIDATE(FF_UNLOCK_WHOAMI)) {
    count++;
    shell_print(shell, "\t%d: WHOAMI", count);
  }
  if (FF_UNLOCK_VALIDATE(FF_UNLOCK_DISTANCE)) {
    count++;
    shell_print(shell, "\t%d: Capacitive Touch", count);
  }
  if (FF_UNLOCK_VALIDATE(FF_UNLOCK_AUDIO)) {
    count++;
    shell_print(shell, "\t%d: Audio SAO", count);
  }
  if (FF_UNLOCK_VALIDATE(FF_UNLOCK_C2)) {
    count++;
    shell_print(shell, "\t%d: AND!XOR C2", count);
  }
  if (FF_UNLOCK_VALIDATE(FF_UNLOCK_AUDIO_PUZZLE)) {
    count++;
    shell_print(shell, "\t%d: Spectograph", count);
  }
  if (FF_UNLOCK_VALIDATE(FF_UNLOCK_DC619)) {
    count++;
    shell_print(shell, "\t%d: San Diego", count);
  }
  if (FF_UNLOCK_VALIDATE(FF_UNLOCK_FLAPPY)) {
    count++;
    shell_print(shell, "\t%d: Flapp Bird", count);
  }
  if (FF_UNLOCK_VALIDATE(FF_UNLOCK_EYES)) {
    count++;
    shell_print(shell, "\t%d: Eyes Bling", count);
  }

  shell_print(shell, "Total Unlocks Found: %d", count);

  return 0;
}

/**
 * @brief Shell command that prints the badge version information
 * @param shell		Pointer to shell context
 * @param argc		Number of command arguments
 * @param argv		Pointer to array of command arguments
 * @return 0 if successful
 */
static int __cmd_version(const struct shell *shell, size_t argc, char **argv) {
  ARG_UNUSED(argc);
  ARG_UNUSED(argv);

  shell_fprintf(shell, SHELL_NORMAL, "AND!XOR DC27 %s [%s], kernel %s\r\n",
                VERSION, BUILD_TIMESTAMP, KERNEL_VERSION_STRING);
  shell_fprintf(shell, SHELL_NORMAL, BANNER);

  return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(
    sub_sys, SHELL_CMD(CEREAL, NULL, "BADGE CEREAL NUMBAR", __cmd_sys_serial),
    SHELL_CMD(UNLOCKZ, NULL, "UNLOCKZ R", __cmd_sys_unlocks),
    SHELL_CMD(VERSHUN, NULL, "BADGE VERSHUN INFURMASHUN", __cmd_version),
    SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(SYSTUM, &sub_sys, "SYSTUM STUFFZ", __cmd_sys);

SHELL_STATIC_SUBCMD_SET_CREATE(sub_kernel,
                               SHELL_CMD(FLIP, NULL, "REBOOT K", __cmd_reboot),
                               SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(TABLE, &sub_kernel, "KERNEL STUFFZ", NULL);

SHELL_CMD_REGISTER(PAWS, NULL, "PAWS FUR N MILLI SEKONDZ", __cmd_paws);