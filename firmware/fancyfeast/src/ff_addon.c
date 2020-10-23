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
#include <stdlib.h>

#include <device.h>
#include <gpio.h>
#include <shell/shell.h>
#include <zephyr.h>

#define LOG_LEVEL 4
#include <logging/log.h>
LOG_MODULE_REGISTER(ff_addon);

#include "autoconf.h"
#include "bling/ff_bling_bouncing.h"
#include "bling/ff_bling_doom.h"
#include "drivers/at24c02.h"
#include "drivers/is31fl3741.h"
#include "ff_addon.h"
#include "ff_gfx.h"
#include "ff_post.h"
#include "ff_settings.h"
#include "ff_shell.h"
#include "ff_unlocks.h"
#include "system.h"

#define ADDON_DELAY 2000
#define SAO_GPIO_PIN_1 31
#define SAO_GPIO_PIN_2 1

typedef enum {
  addon_state_searching,
  addon_state_init,
  addon_state_running,
  addon_state_done
} addon_state_t;

static addon_state_t m_eeprom_state = addon_state_searching;

// Used for toggling LEDs
static uint8_t m_led_counter = 0;

/**
 * @brief Adjust LED brightness in real time until SAO is removed
 */
static void __addon_audio_adjust_gcc() {
  uint8_t value = 0;
  while (1) {
    if (!at24c02_read_byte(&value, 3)) {

      value = MAX(value, 1);
      value = MIN(value, 16) - 7;
      value *= 10;
      value = MAX(value, 10);

      is31fl3741_gcc_set(value);
      // LOG_DBG("Audio: %d", value);
    } else {
      return;
    }
    k_sleep(200); // Be nice?
  }

  // Restore brightness
  is31fl3741_gcc_set(ff_settings_ptr_get()->brightness);
}

/**
 * @brief Shell command that creates an eeprom
 * @return 0 if successful
 */
static inline int __cmd_addon_provision_eeprom(const struct shell *shell,
                                               size_t argc, char **argv) {
  if (argc == 1) {
    shell_error(shell, "Expected EXTRAS CREATE <command>");
    return -EINVAL;
  }

  uint8_t eeprom_contents[AT24C_SIZE];
  memset(eeprom_contents, 0, AT24C_SIZE);
  eeprom_contents[0] = FF_ADDON_MAGIC_BYTE_DEFCON;
  eeprom_contents[1] = FF_ADDON_MAGIC_BYTE_MAKER;
  eeprom_contents[2] = FF_ADDON_MAGIC_BYTE_TYPE_EEPROM;

  shell_print(shell, "Provisioning with command '%s' len=%d", argv[1],
              strlen(argv[1]));

  // We can store a command using remaining bytes (less null character)
  if (strlen(argv[1]) <= (AT24C_SIZE - 3 - 1)) {
    memcpy(eeprom_contents + 3, argv[1], strlen(argv[1]));
  }

  if (at24c02_write_bytes(0x00, eeprom_contents, AT24C_SIZE)) {
    shell_error(shell, "Unable to write to Addon :(");
    return 0;
  }

  // Read back contents
  memset(eeprom_contents, 0, AT24C_SIZE);
  at24c02_read_all(eeprom_contents);
  for (uint16_t i = 0; i < AT24C_SIZE; i++) {
    shell_fprintf(shell, SHELL_VT100_COLOR_MAGENTA, "0x%02x ",
                  eeprom_contents[i]);
  }
  shell_print(shell, "\nAddon provisioned");

  return 0;
}

/**
 * @brief Shell command that sets the GPIO mode for the SAO
 * FORMAT: EXTRAS MODE <int>
 * @return 0 if successful
 */
static inline int __cmd_addon_mode(const struct shell *shell, size_t argc,
                                   char **argv) {
  if (argc != 2) {
    shell_error(
        shell,
        "Invalid argument(s). Expected EXTRAS MODE <mode> (mode = 0 to 5)");
    return -1;
  }

  uint8_t mode = strtol(argv[1], NULL, 10);
  if (mode >= __addon_gpio_mode_counter) {
    shell_error(
        shell,
        "Invalid argument(s). Expected EXTRAS MODE <mode> (mode = 0 to 5)");
    return -1;
  }

  // Change the mode
  ff_settings_ptr_get()->addon_gpio_mode = mode;
  ff_settings_save();

  shell_print(shell, "GPIO mode set to %d", mode);
  return 0;
}

/**
 * @brief Shell command that dumps contents of a connected eeprom
 * FORMAT: EXTRAS READ
 * @return 0 if successful
 */
static inline int __cmd_addon_read_eeprom(const struct shell *shell,
                                          size_t argc, char **argv) {
  if (argc != 1) {
    shell_error(shell, "Invalid Argument(s)");
    return -1;
  }

  // Read full eeprom contents, quit if not found
  uint8_t buf[256];
  if (at24c02_read_all(buf)) {
    shell_error(shell, "EEPROM not found :(");
    return -1;
  }

  // Dump eeprom contents to shell
  for (uint16_t i = 0; i < 256; i += 16) {
    shell_fprintf(shell, SHELL_NORMAL, "%08X  ", i);

    for (uint16_t ii = i; ii < i + 16; ii++) {
      shell_fprintf(shell, SHELL_NORMAL, "%02X ", buf[ii]);
    }

    shell_fprintf(shell, SHELL_NORMAL, "  ");

    for (uint16_t ii = i; ii < i + 16; ii++) {
      shell_fprintf(shell, SHELL_NORMAL, "%c",
                    buf[ii] < 32 || buf[ii] > 127 ? '.' : buf[ii]);
    }

    shell_fprintf(shell, SHELL_NORMAL, "\r\n");
  }

  return 0;
}

/**
 * @brief Shell command that writes an arbitrary byte to the eeprom
 * FORMAT: EXTRAS WRITE <address> <byte hex>
 * @return 0 if successful
 */
static inline int __cmd_addon_write_eeprom(const struct shell *shell,
                                           size_t argc, char **argv) {
  if (argc != 3) {
    shell_error(
        shell,
        "Invalid Argument(s), expected EXTRAS WRITE <address hex> <byte hex>");
    return -1;
  }

  uint8_t address = strtol(argv[1], NULL, 16);
  uint8_t byte = strtol(argv[2], NULL, 16);

  int err = at24c02_write_byte(address, byte);
  if (!err) {
    shell_print(shell, "Wrote 0x%02x to 0x%02x in the EEPROM", byte, address);
  } else {
    shell_error(shell, "Unable to write to EEPROM");
  }
  return err;
}

/**
 * @brief Run a single step in the eeprom state machine
 */
static inline void __state_machine_eeprom() {
  uint8_t magic[3];
  uint8_t eeprom_contents[AT24C_SIZE];

  switch (m_eeprom_state) {
    // Search for eeprom
  case addon_state_searching:
    if (at24c02_read(&magic[0])) {
      // LOG_DBG("Unable to find eeprom, still searching");
    } else {
      LOG_DBG("eeprom found!");

      // Read rest of magic data
      k_sleep(1);
      at24c02_read_byte(&magic[1], 1);
      k_sleep(1);
      at24c02_read_byte(&magic[2], 2);

      LOG_HEXDUMP_DBG(magic, 3, "Magic");

      // If we got this far, magic byte header was found
      // This is an AND!XOR SAO
      if (magic[0] == FF_ADDON_MAGIC_BYTE_DEFCON &&
          magic[1] == FF_ADDON_MAGIC_BYTE_MAKER) {
        switch (magic[2]) {
        case FF_ADDON_MAGIC_BYTE_TYPE_EEPROM:
          LOG_INF("EEPROM SAO Found!");
          LOG_DBG("Switching to init state");
          m_eeprom_state = addon_state_init;

          ff_gfx_draw_line(0, 0, WIDTH, 0, COLOR_ORANGE);
          ff_gfx_draw_line(0, HEIGHT - 1, WIDTH, HEIGHT - 1, COLOR_BLUE);
          ff_gfx_push_buffer();
          k_sleep(200);
          break;
        case FF_ADDON_MAGIC_BYTE_TYPE_AUDIO:
          LOG_INF("Audio Reactive SAO Found!");
          ff_gfx_draw_line(0, 0, WIDTH, 0, COLOR_YELLOW);
          ff_gfx_draw_line(0, HEIGHT - 1, WIDTH, HEIGHT - 1, COLOR_YELLOW);
          ff_gfx_push_buffer();
          k_sleep(200);

          if (!FF_UNLOCK_VALIDATE(FF_UNLOCK_AUDIO)) {
            FF_UNLOCK_SET(FF_UNLOCK_AUDIO);
            FF_BLING_DEFAULT_BOUNCING(bling);
            ff_bling_mode_register(bling, ff_bling_handler_bouncing);
            ff_bling_mode_push(bling);
            ff_gfx_fill(COLOR_YELLOW);
            ff_gfx_push_buffer();
            k_sleep(3000);
          }

          // Block addon thread to adjust gcc in near-realtime
          __addon_audio_adjust_gcc();
          break;
        case FF_ADDON_MAGIC_BYTE_TYPE_BENDER:
          break;
        }
      }

      else if (magic[0] == FF_ADDON_MAGIC_BYTE_DEFCON &&
               magic[1] == FF_ADDON_MAGIC_BYTE_MAKER_CR4BF04M) {
        switch (magic[2]) {
        case FF_ADDON_MAGIC_BYTE_TYPE_DOOM:;
          FF_BLING_DEFAULT_DOOM(doom);
          ff_bling_mode_push(doom);
          m_eeprom_state = addon_state_running;
          break;
        }
      }
    }
    break;

  // Initialize the addon
  case addon_state_init:
    ff_gfx_draw_line(0, 0, WIDTH, 0, COLOR_BLACK);
    ff_gfx_draw_line(0, HEIGHT - 1, WIDTH, HEIGHT - 1, COLOR_BLACK);
    ff_gfx_push_buffer();

    if (!at24c02_read_all(eeprom_contents)) {
      // Success
      LOG_DBG("Running '%s'", eeprom_contents + 3);
      ff_shell_execute(eeprom_contents + 3);
      m_eeprom_state = addon_state_running;
    } else {
      LOG_DBG("Going back to searching");
      m_eeprom_state = addon_state_searching;
    }

    // for (uint16_t i = 0; i <= 255; i++) {
    //   at24c02_write_byte(i, i);
    // }
    // if (!at24c02_read_all(eeprom_contents)) {
    //   LOG_HEXDUMP_DBG(eeprom_contents, AT24C02_SIZE, "EEPROM");
    // }
    break;

  case addon_state_running:;
    // Grab GPIO device
    struct device *gpio = device_get_binding(SW0_GPIO_CONTROLLER);
    if (gpio) {
      // Ensure addon gpio mode is valid
      if (ff_settings_ptr_get()->addon_gpio_mode >= __addon_gpio_mode_counter) {
        ff_settings_ptr_get()->addon_gpio_mode = addon_gpio_mode_alternating;
      }

      // Do the GPIO mode
      switch (ff_settings_ptr_get()->addon_gpio_mode) {
      case addon_gpio_mode_alternating:
      case addon_gpio_mode_rager:
        gpio_pin_write(gpio, SAO_GPIO_PIN_1, (m_led_counter + 1) % 2);
        gpio_pin_write(gpio, SAO_GPIO_PIN_2, m_led_counter % 2);
        m_led_counter++;
        break;
      case addon_gpio_mode_both:
        gpio_pin_write(gpio, SAO_GPIO_PIN_1, 1);
        gpio_pin_write(gpio, SAO_GPIO_PIN_2, 1);
        break;
      case addon_gpio_mode_off:
        gpio_pin_write(gpio, SAO_GPIO_PIN_1, 0);
        gpio_pin_write(gpio, SAO_GPIO_PIN_2, 0);
        break;
      case addon_gpio_mode_on_1:
        gpio_pin_write(gpio, SAO_GPIO_PIN_1, 1);
        gpio_pin_write(gpio, SAO_GPIO_PIN_2, 0);
        break;
      case addon_gpio_mode_on_2:
        gpio_pin_write(gpio, SAO_GPIO_PIN_1, 0);
        gpio_pin_write(gpio, SAO_GPIO_PIN_2, 1);
        break;
      case __addon_gpio_mode_counter:
        // NOP
        break;
      }
    }

    // Test if eeprom is still there
    if (at24c02_read(&magic[0])) {
      LOG_DBG("EEPROM Removed returning to searching");
      m_eeprom_state = addon_state_searching;
    }
    break;
  case addon_state_done:
    // TODO:
    break;
  }
}

/**
 * @brief Background task that manages addon states
 */
static void __addon_task() {
  struct device *gpio = device_get_binding(SW0_GPIO_CONTROLLER);
  if (gpio) {
    gpio_pin_configure(gpio, SAO_GPIO_PIN_1, GPIO_DIR_OUT);
    gpio_pin_configure(gpio, SAO_GPIO_PIN_2, GPIO_DIR_OUT);
  }

  while (1) {
    // Run each state machine
    __state_machine_eeprom();
    if (ff_settings_ptr_get()->addon_gpio_mode == addon_gpio_mode_rager) {
      k_sleep(ADDON_DELAY / 40);
    } else {
      k_sleep(ADDON_DELAY);
    }
  }
}

K_THREAD_DEFINE(addon, 6000, __addon_task, NULL, NULL, NULL,
                FF_THREAD_PRIORITY_MEDIUM, 0, 2000);

SHELL_STATIC_SUBCMD_SET_CREATE(
    sub_addon,
    SHELL_CMD(CREATE, NULL, "WRITE SHELL COMMAND 2 EEPROM K",
              __cmd_addon_provision_eeprom),
    SHELL_CMD(MODE, NULL, "READ DA EEPROM K", __cmd_addon_mode),
    SHELL_CMD(READ, NULL, "READ DA EEPROM K", __cmd_addon_read_eeprom),
    SHELL_CMD(WRITE, NULL, "WRITE 2 EEPROM K", __cmd_addon_write_eeprom),
    SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(EXTRAS, &sub_addon, "SAO COMMANDZ", NULL);