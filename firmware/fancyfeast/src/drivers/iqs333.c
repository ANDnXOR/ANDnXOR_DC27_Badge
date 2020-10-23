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
#include <device.h>
#include <gpio.h>
#include <i2c.h>
#include <zephyr.h>

#define LOG_LEVEL 4
#include <logging/log.h>
LOG_MODULE_REGISTER(iqs333);

#include "../ff_post.h"
#include "../ff_util.h"
#include "../system.h"
#include "autoconf.h"
#include "ff_i2c.h"
#include "iqs333.h"

#ifdef CONFIG_IQS333

struct iqs333_data {
  struct device *i2c;
};

// The settings below will be written during startup
const static uint8_t m_settings[] = {
    0x16, // Byte 0, re-set ATI
    0x00, // default
    0x48, // Byte 2, disable WDT and timeout
    0x00, // Byte 3, enable both wheels
    0x07, // default
    0x7F  // default
};
#define SETTINGS_SIZE 6

static bool m_initialized = false;
static struct iqs333_data m_iqs333_driver_data;
static int16_t m_touch = 0;
static int16_t m_wheel_left = CAPTOUCH_NO_TOUCH;
static int16_t m_wheel_right = CAPTOUCH_NO_TOUCH;

/**
 * @brief Wait until the captouch countroller signals us on the RDY pin that
 * it's ready
 * @return Return true if GPIO went low, otherwise return false if aborted
 */
static bool __block_rdy() {
  struct device *gpio = device_get_binding(CONFIG_IQS333_RDY_GPIO_DEV);
  // gpio_pin_write(gpio, CONFIG_IQS333_RDY_PIN, 0);
  uint32_t value = 1;

  uint32_t start = ff_util_millis();
  gpio_pin_read(gpio, CONFIG_IQS333_RDY_PIN, &value);
  while (value == 1) {
    // Limit blocking to 100 ms
    if ((ff_util_millis() - start) > 100) {
      return false;
    }
    k_sleep(1);
    gpio_pin_read(gpio, CONFIG_IQS333_RDY_PIN, &value);
  }

  return true;
}

/**
 * @brief Read the wheel data. <Wheel 1 Low><Wheel 1 High><Wheel 2 Low><Wheel 2
 * High>
 */
uint32_t __read_wheel() {
  if (!m_initialized) {
    return 0;
  }

  uint32_t wheel = 0;

  ff_i2c_take();
  if (__block_rdy()) {
    if (i2c_burst_read(m_iqs333_driver_data.i2c, CONFIG_IQS333_I2C_ADDRESS,
                       IQS333_REG_WHEEL, (uint8_t *)&wheel, 4)) {
      // ACK failed
      ff_post_failed(FF_POST_IQS333_ACK);
      LOG_ERR("Reading wheel coord failed");
    }
    // If we were successful but POST was previously a failure, change it
    else if ((ff_post_state_get() & FF_POST_IQS333_ACK) == 0) {
      ff_post_success(FF_POST_IQS333_ACK);
      LOG_INF("Recovered captouch driver");
    }
  }
  ff_i2c_give();

  // LOG_DBG("0x%08x", wheel);
  return wheel;
}

/**
 * @brief Read the last touhc value from the controller
 */
uint16_t __read_touch() {
  if (!m_initialized) {
    return 0;
  }

  uint16_t touch = 0;
  ff_i2c_take();
  if (__block_rdy()) {
    if (i2c_burst_read(m_iqs333_driver_data.i2c, CONFIG_IQS333_I2C_ADDRESS,
                       IQS333_REG_TOUCH, (uint8_t *)&touch, 2)) {
      // ACK failed
      ff_post_failed(FF_POST_IQS333_ACK);
      LOG_ERR("Reading button failed");
    }
    // If we were successful but POST was previously a failure, change it
    else if ((ff_post_state_get() & FF_POST_IQS333_ACK) == 0) {
      ff_post_success(FF_POST_IQS333_ACK);
      LOG_INF("Recovered captouch driver");
    }
  }
  ff_i2c_give();

  return touch;
}

/**
 * @brief Background task that polls the status of the touch controller
 */
static void __poll_task() {
  while (1) {
    m_touch = __read_touch();
    if (m_touch > 0) {
      k_sleep(20);
      uint32_t value = __read_wheel();
      m_wheel_left = (int16_t)(value);
      m_wheel_right = (int16_t)(value >> 16);
      // LOG_DBG("Wheel Touch=0x%04x Value=0x%08x Left=%d Right=%d", touch,
      // value, m_wheel_left,
      //         m_wheel_right);
    } else {
      m_wheel_left = CAPTOUCH_NO_TOUCH;
      m_wheel_right = CAPTOUCH_NO_TOUCH;
    }

    k_sleep(50);
  }
}

/**
 * @brief Initialize the iqs333 device
 * @param dev	The device context to use to initialize
 * @return 0 if okay, otherwise an error
 */
static int __init(struct device *dev) {
  LOG_DBG("Initializing IQS333");

  // Get the I2C Device
  m_iqs333_driver_data.i2c =
      device_get_binding(CONFIG_IQS333_I2C_MASTER_DEV_NAME);
  if (m_iqs333_driver_data.i2c == NULL) {
    LOG_ERR("Failed to get I2C device");
    return -EINVAL;
  }

  struct device *gpio = device_get_binding(CONFIG_IQS333_RDY_GPIO_DEV);
  gpio_pin_configure(gpio, CONFIG_IQS333_RDY_PIN, GPIO_DIR_IN);

  // Read device info first
  uint16_t device_info = 0x00;

  ff_i2c_take();
  if (__block_rdy()) {
    if (i2c_reg_read_byte(m_iqs333_driver_data.i2c, CONFIG_IQS333_I2C_ADDRESS,
                          IQS333_REG_DEVICE_INFO, (uint8_t *)&device_info)) {
      // ACK failed
      ff_post_failed(FF_POST_IQS333_ACK);
      LOG_ERR("Reading device info failed");
      ff_i2c_give();
      return -EIO;
    }
    ff_post_success(FF_POST_IQS333_ACK);
  }
  ff_i2c_give();

  if (device_info == IQS333_VALID_DEVICE_INFO) {
    ff_post_success(FF_POST_IQS333_DEVICE_INFO);
  } else {
    ff_post_failed(FF_POST_IQS333_DEVICE_INFO);
  }

  // Setup proximity
  k_sleep(50);

  ff_i2c_take();
  if (__block_rdy()) {
    i2c_burst_write(m_iqs333_driver_data.i2c, CONFIG_IQS333_I2C_ADDRESS,
                    IQS333_REG_PROX_SETTINGS, m_settings, SETTINGS_SIZE);
  }
  ff_i2c_give();

  // Enable channesl 0-7
  k_sleep(50);
  if (__block_rdy()) {
    // Enable only the 7 channels we need, disable proximity channel (bit 0)
    i2c_reg_write_byte(m_iqs333_driver_data.i2c, CONFIG_IQS333_I2C_ADDRESS,
                       IQS333_REG_ACTIVE_CHANNELS, 0xFE);
  }

  m_initialized = true;

  return 0;
}

/**
 * Get the last touch value read
 * @return the last raw touch value read
 */
inline int16_t iqs333_touch_get() { return m_touch; }

/**
 * @brief Get the last left wheel reading
 * @return CAPTOUCH_NO_TOUCH if there isn't a current touch or a position 0 to
 * 2048 for representing the current finger position on wheel 1
 */
inline int16_t iqs333_wheel_left_get() { return m_wheel_left; }

/**
 * @brief Get the last right wheel reading
 * @return CAPTOUCH_NO_TOUCH if there isn't a current touch or a position 0 to
 * 2048 for representing the current finger position on wheel 1
 */
inline int16_t iqs333_wheel_right_get() { return m_wheel_right; }

DEVICE_INIT(iqs333, CONFIG_IQS333_DEV_NAME, __init, &m_iqs333_driver_data, NULL,
            APPLICATION, CONFIG_IQS333_INIT_PRIORITY);

K_THREAD_DEFINE(iqs333, 512, __poll_task, NULL, NULL, NULL,
                FF_THREAD_PRIORITY_MEDIUM, 0, 500);

#endif