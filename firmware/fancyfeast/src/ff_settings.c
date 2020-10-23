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
#include <posix/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <crc.h>
#include <fs.h>
#include <init.h>
#include <misc/reboot.h>
#include <shell/shell.h>
#include <zephyr.h>

#define LOG_LEVEL 4
#include <logging/log.h>
LOG_MODULE_REGISTER(ff_settings);

#include "drivers/is31fl3741.h"
#include "ff_bender.h"
#include "ff_fs.h"
#include "ff_settings.h"
#include "ff_time.h"
#include "system.h"

#define SETTINGS_FILE FF_FS_MOUNT_POINT "/VARZ"
#define SETTINGS_SCROLL_SENSITIVITY_MIN 5
#define SETTINGS_SCROLL_SENSITIVITY_DEFAULT 15
#define SETTINGS_SCROLL_SENSITIVITY_MAX 30

static settings_t m_settings = {.name = "AND!XOR",
                                .brightness = CONFIG_LED_BRIGHTNESS_DEFAULT,
                                .bling_hue_matrix = HUE_GREEN,
                                .bling_rager = false,
                                .proxy = true,
                                .scroll_sensitivity =
                                    SETTINGS_SCROLL_SENSITIVITY_DEFAULT,
                                .unlock = 0};

/**
 * @brief Shell function that prints the value of the name setting
 */
static int __cmd_print_name(const struct shell *shell, size_t argc,
                            char **argv) {
  shell_print(shell, "Name: '%s'", m_settings.name);
  return 0;
}

/**
 * @brief Shell function that set the value of the name setting
 */
static int __cmd_set_name(const struct shell *shell, size_t argc, char **argv) {
  snprintf(m_settings.name, NAME_MAX_LENGTH + 1, "%s", argv[1]);
  ff_settings_save();
  shell_print(shell, "Name: '%s'", m_settings.name);
  return 0;
}

/**
 * @brief Shell function that prints the value of the led brightness setting
 */
static int __cmd_print_led(const struct shell *shell, size_t argc,
                           char **argv) {
  shell_print(shell, "LED Brightness: %d", m_settings.brightness);
  return 0;
}

/**
 * @brief Shell function that set the value of the led brightness setting
 */
static int __cmd_set_led(const struct shell *shell, size_t argc, char **argv) {
  m_settings.brightness = (uint8_t)strtol(argv[1], NULL, 10);
  ff_settings_save();
  shell_print(shell, "LED Brightness set to: %d", m_settings.brightness);
  is31fl3741_gcc_set(m_settings.brightness);
  return 0;
}

/**
 * @brief Shell function that prints the value of the proxy setting
 */
static int __cmd_print_proxy(const struct shell *shell, size_t argc,
                             char **argv) {
  shell_fprintf(shell, SHELL_NORMAL, "Proxy ");
  if (m_settings.proxy) {
    shell_fprintf(shell, SHELL_NORMAL, "Enabled\r\n");
  } else {
    shell_fprintf(shell, SHELL_NORMAL, "Disabled\r\n");
  }
  return 0;
}

/**
 * @brief Shell function that set the value of the proxy setting
 */
static int __cmd_set_proxy(const struct shell *shell, size_t argc,
                           char **argv) {
  m_settings.proxy = (strcmp(argv[1], "ON") == 0);
  ff_settings_save();
  __cmd_print_proxy(shell, argc, argv);
  return 0;
}

/**
 * @brief Shell function that prints the value of the scroll sensitivity setting
 */
// static int __cmd_print_scroll(const struct shell *shell, size_t argc,
//                               char **argv) {
//   shell_print(shell, "Scroll Sensitivity: %d", m_settings.scroll_sensitivity);
//   return 0;
// }

/**
 * @brief Shell function that set the value of the scroll sensitivity setting
 */
// static int __cmd_set_scroll(const struct shell *shell, size_t argc,
//                             char **argv) {
//   uint8_t scroll = strtol(argv[1], NULL, 10);
//   if (scroll < SETTINGS_SCROLL_SENSITIVITY_MIN) {
//     scroll = SETTINGS_SCROLL_SENSITIVITY_MIN;
//   } else if (scroll > SETTINGS_SCROLL_SENSITIVITY_MAX) {
//     scroll = SETTINGS_SCROLL_SENSITIVITY_MAX;
//   }
//   shell_print(shell, "Scroll sensitivity set to %d", scroll);
//   m_settings.scroll_sensitivity = scroll;
//   ff_settings_save();
//   return 0;
// }

/**
 * @brief Shell function that prints all settings values
 */
static int __cmd_print_all(const struct shell *shell, size_t argc,
                           char **argv) {
  __cmd_print_name(shell, argc, argv);
  __cmd_print_led(shell, argc, argv);
  __cmd_print_proxy(shell, argc, argv);
  return 0;
}

/**
 * @brief Get a pointer to the current settings object
 */
settings_t *ff_settings_ptr_get() { return &m_settings; }

/**
 * @brief Get a pointer to the current bender object
 */
bender_data_t *ff_settings_bender_ptr_get() {
  return &(m_settings.bender_state);
}

void ff_settings_load() {
  // Make sure wall buffers are clear first
  for (uint8_t i = 0; i < GRAFFITI_COUNT; i++) {
    memset(m_settings.wall[i], 0, GRAFFITI_MAX_LENGTH + 1);
  }

  // Open the settings file, failing if not found
  struct fs_file_t file;
  if (fs_open(&file, SETTINGS_FILE)) {
    LOG_ERR("Cannot open badge settings file");
    return;
  }

  settings_t temp;
  ssize_t bytes_read = fs_read(&file, &temp, sizeof(settings_t));
  if (bytes_read != sizeof(settings_t)) {
    LOG_ERR("Unable to read settings file");
    fs_close(&file);
    return;
  }

  fs_close(&file);

  // Validate CRC
  uint32_t crc32 =
      crc32_ieee((uint8_t *)&temp, sizeof(settings_t) - sizeof(uint32_t));
  if (temp.crc32 != crc32) {
    LOG_ERR("Settings CRC32 check failed, stored = 0x%08x computed=0x%08x",
            temp.crc32, crc32);
    return;
  }

  // Copy settings data locally
  memcpy(&m_settings, &temp, sizeof(settings_t));

  // Do module-specific updates to settings
  is31fl3741_gcc_set(m_settings.brightness);

  // Set the current badge time to what was stored
  struct timespec new_time;
  new_time.tv_sec = m_settings.time_sec;
  ff_time_now_set(&new_time);

  // Fin
  LOG_INF("Settings loaded.");
}

/**
 * @brief Initialize the settings module
 */
void ff_settings_init() { ff_settings_load(); }

/**
 * @brief Factory reset and reboot badge
 */
void ff_settings_reset() {
  fs_unlink(SETTINGS_FILE);
  sys_reboot(SYS_REBOOT_COLD);
}

/**
 * @brief Save the settings to the filesystem
 */
void ff_settings_save() {
  LOG_DBG("Saving settings to '%s'", SETTINGS_FILE);

  // Get the current time
  struct timespec now = ff_time_now_get();
  m_settings.time_sec = now.tv_sec;

  // Open the state file
  struct fs_file_t file;
  if (fs_open(&file, SETTINGS_FILE)) {
    LOG_ERR("Cannot open badge settings file");
    return;
  }

  m_settings.crc32 =
      crc32_ieee((uint8_t *)&m_settings, sizeof(settings_t) - sizeof(uint32_t));
  fs_write(&file, &m_settings, sizeof(settings_t));
  fs_close(&file);
}

// VISIBLE <x>
SHELL_STATIC_SUBCMD_SET_CREATE(
    sub_print, SHELL_CMD(ALL, NULL, "VARZ", __cmd_print_all),
    SHELL_CMD(NAME, NULL, "TEH USERZ NAEM PLZ", __cmd_print_name),
    SHELL_CMD(LED, NULL, "TEH LITEZ BRITENESZ PLZ", __cmd_print_led),
    SHELL_CMD(PROXY, NULL, "PROXY MODE PLZ", __cmd_print_proxy),
    // SHELL_CMD(SCROLL, NULL, "SCROLL SENZ PLZ", __cmd_print_scroll),
    SHELL_SUBCMD_SET_END);
SHELL_CMD_REGISTER(VISIBLE, &sub_print, "EKO VARZ", NULL);

// LET <x> B <y>
SHELL_STATIC_SUBCMD_SET_CREATE(sub_set_name_b,
                               SHELL_CMD(B, NULL, "", __cmd_set_name),
                               SHELL_SUBCMD_SET_END);
SHELL_STATIC_SUBCMD_SET_CREATE(sub_set_led_b,
                               SHELL_CMD(B, NULL, "", __cmd_set_led),
                               SHELL_SUBCMD_SET_END);
SHELL_STATIC_SUBCMD_SET_CREATE(sub_set_proxy_b,
                               SHELL_CMD(B, NULL, "", __cmd_set_proxy),
                               SHELL_SUBCMD_SET_END);
// SHELL_STATIC_SUBCMD_SET_CREATE(sub_set_scroll_b,
//                                SHELL_CMD(B, NULL, "", __cmd_set_scroll),
//                                SHELL_SUBCMD_SET_END);
SHELL_STATIC_SUBCMD_SET_CREATE(
    sub_set, SHELL_CMD(NAME, &sub_set_name_b, "TEH USERZ NAEM PLZ", NULL),
    SHELL_CMD(LED, &sub_set_led_b, "TEH LITEZ BRITENESZ PLZ", NULL),
    SHELL_CMD(PROXY, &sub_set_proxy_b, "PROXY MODE PLZ", NULL),
    // SHELL_CMD(SCROLL, &sub_set_scroll_b, "SCROLL SENZ PLZ", NULL),
    SHELL_SUBCMD_SET_END);
SHELL_CMD_REGISTER(LET, &sub_set, "VARZ R SUMTHING ELSE", NULL);