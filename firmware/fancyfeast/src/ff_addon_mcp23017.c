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
#include "i2c.h"
#include "zephyr.h"
#define LOG_LEVEL 4
#include <logging/log.h>
LOG_MODULE_REGISTER(ff_addon_mcp23017);

#include "drivers/ff_i2c.h"
#include "ff_util.h"
#include "system.h"

#define ADDONS_ANIMATION_TIME (60 * 1000)
#define ADDON_I2C_ADDRESS_7B 0x20

// MCP23017 registers assuming BANK=1
#define MCP23017_REG_IODIRA 0x00
#define MCP23017_REG_IODIRB 0x01
// Ports A and B GPIO latch state (aka output)
#define MCP23017_REG_OLATA 0x14
#define MCP23017_REG_OLATB 0x15

struct device *i2c;

#define TRANSITION                                                             \
  if (ff_util_millis() >= transition_time) {                                   \
    m_state = (m_state + 1) % addon_state_count;                               \
    transition_time = ff_util_millis() + ADDONS_ANIMATION_TIME;                \
  }

/**
 * @brief Helper macro that writes to PORT A of the MCP23017 device
 */
#define WRITE_PORTA(value)                                                     \
  ff_i2c_take();                                                               \
  if (i2c_reg_write_byte(i2c, ADDON_I2C_ADDRESS_7B, MCP23017_REG_OLATA,        \
                         value)) {                                             \
    LOG_ERR("Unable to write to MCP23017 PORTA [%d]", value);                  \
    m_state = addon_state_searching;                                           \
  }                                                                            \
  ff_i2c_give();

/**
 * @brief Helper macro that writes to PORT B of the MCP23017 device
 */
#define WRITE_PORTB(value)                                                     \
  ff_i2c_take();                                                               \
  if (i2c_reg_write_byte(i2c, ADDON_I2C_ADDRESS_7B, MCP23017_REG_OLATB,        \
                         value)) {                                             \
    LOG_ERR("Unable to write to MCP23017 PORTB [%d]", value);                  \
    m_state = addon_state_searching;                                           \
  }                                                                            \
  ff_i2c_give();

// Define states for the addon FSM
typedef enum {
  addon_state_chase,
  addon_state_chase2,
  addon_state_kitt,
  addon_state_random,
  addon_state_count,
  addon_state_searching,
  addon_state_stopped
} addon_state_t;

static addon_state_t m_state = addon_state_searching;

void __addon_task() {
  int8_t direction = 1;
  uint8_t index = 0;
  uint32_t transition_time = 0;

  m_state = addon_state_searching;
  i2c = device_get_binding(CONFIG_AT24C02_I2C_MASTER_DEV_NAME);

  while (1) {

    switch (m_state) {
    // Searching for an addon
    case addon_state_searching:

      k_sleep(2000);

      // This assumes POR which sets BANK to 0

      // Write to DIRA register
      int result = i2c_reg_write_byte(i2c, ADDON_I2C_ADDRESS_7B,
                                      MCP23017_REG_IODIRA, 0x00);
      // Found an addon, transition to chase animation
      if (result == 0) {
        k_sleep(50);
        // Port B DIRB write was a success, let's try Port B DIRB
        if (i2c_reg_write_byte(i2c, ADDON_I2C_ADDRESS_7B, MCP23017_REG_IODIRB,
                               0x00) == 0) {
          // That worked, start animating
          m_state = addon_state_chase2;
          transition_time = ff_util_millis() + ADDONS_ANIMATION_TIME;
        }
      }
      break;

    // Chase animation mode
    case addon_state_chase:
      k_sleep(50);
      WRITE_PORTA((uint8_t)(1 << index));
      WRITE_PORTB((uint8_t)(1 << index));
      index = (index + 1) % 8;
      TRANSITION;
      break;

    // Chase animation mode, but more extreme
    case addon_state_chase2:
      k_sleep(30);
      WRITE_PORTA((uint8_t)(1 << index));
      WRITE_PORTB((uint8_t)(1 << index));

      if (direction < 0) {
        if (index > 0) {
          index--;
        } else {
          index = 7;
        }
        if (sys_rand32_get() % 100 < 4) {
          direction = 1;
        }
      } else {
        index = (index + 1) % 8;
        if (sys_rand32_get() % 100 < 4) {
          direction = -1;
        }
      }

      TRANSITION;
      break;

      // Kit wiping back and forth
    case addon_state_kitt:
      k_sleep(70);

      switch (index) {
      case 0:
        WRITE_PORTA(0x7E);
        WRITE_PORTB(0x00);
        break;
      case 1:
        WRITE_PORTA(0x81);
        WRITE_PORTB(0x00);
        break;
      case 2:
        WRITE_PORTA(0x00);
        WRITE_PORTB(0x81);
        break;
      case 3:
        WRITE_PORTA(0x00);
        WRITE_PORTB(0x42);
        break;
      case 4:
        WRITE_PORTA(0x00);
        WRITE_PORTB(0x24);
        break;
      case 5:
        WRITE_PORTA(0x00);
        WRITE_PORTB(0x08);
        break;
      case 6:
        WRITE_PORTA(0x00);
        WRITE_PORTB(0x10);
        break;
      }
      if (direction > 0) {
        index++;
        if (index >= 7) {
          direction = -1;
          index = 6;
        }
      } else {
        if (index > 0) {
          index--;
        } else {
          index = 0;
          direction = 1;
        }
      }
      TRANSITION;
      break;

    // Random LED mode
    case addon_state_random:
      k_sleep(100);
      uint32_t rand = sys_rand32_get();
      WRITE_PORTA(rand & 0xFF);
      WRITE_PORTB((rand >> 8) & 0xFF);
      TRANSITION;
      break;

    // Do nothing
    case addon_state_stopped:
      k_sleep(100);
      break;

    // Should never get here but if it does go back to searching mode
    case addon_state_count:
      m_state = addon_state_searching;
      break;
    }
  }
}

K_THREAD_DEFINE(mcp23017, 2048, __addon_task, NULL, NULL, NULL,
                FF_THREAD_PRIORITY_MEDIUM, 0, 2000);