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
#include <i2c.h>
#include <zephyr.h>

#define LOG_LEVEL 4
#include <logging/log.h>
LOG_MODULE_REGISTER(at24c02);

#include "../ff_post.h"
#include "at24c02.h"
#include "autoconf.h"
#include "ff_i2c.h"

struct at24c02_data {
  struct device *i2c;
};
static struct at24c02_data m_at24c02_driver_data;

/**
 * @brief Initialize the iqs333 device
 * @param dev	The device context to use to initialize
 * @return 0 if okay, otherwise an error
 */
static int __init(struct device *dev) {
  // Get the I2C Device
  m_at24c02_driver_data.i2c =
      device_get_binding(CONFIG_AT24C02_I2C_MASTER_DEV_NAME);
  if (m_at24c02_driver_data.i2c == NULL) {
    LOG_ERR("Failed to get I2C device");
    ff_post_failed(FF_POST_ADDON_DEVICE);
    return -EINVAL;
  }
  ff_post_success(FF_POST_ADDON_DEVICE);

  LOG_DBG("Initialized AT24C02");
  return 0;
}

/**
 * @brief Read a single byte from the eeprom, this can also be used to determine
 * if the eeprom is plugged into the badge
 * @param byte	Pointer to where to store the data
 * @return Negative if failure, zero if successful
 */
int at24c02_read(uint8_t *byte) {
  if (!m_at24c02_driver_data.i2c) {
    return -EIO;
  }
  int ret = 0;
  ff_i2c_take();
  // LOG_DBG("Reading byte");
  ret = i2c_reg_read_byte(m_at24c02_driver_data.i2c, CONFIG_AT24C02_I2C_ADDRESS,
                          0x00, byte);
  // LOG_DBG("read byte %d", *byte);
  ff_i2c_give();
  return ret;
}

/**
 * @brief Read arbitrary byte from eeprom
 * @param p_buf		Buffer to put the bytes into
 * @param offset	Offset of the address to start at
 * @return Negative value if error, 0 if okay
 */
int at24c02_read_byte(uint8_t *p_buf, uint8_t offset) {
  if (!m_at24c02_driver_data.i2c) {
    return -EIO;
  }

  int ret = 0;
  ff_i2c_take();
  ret = i2c_reg_read_byte(m_at24c02_driver_data.i2c, CONFIG_AT24C02_I2C_ADDRESS,
                          offset, p_buf);
  ff_i2c_give();
  return ret;
}

/**
 * @brief Read the entire EEPROM
 * @param p_buf		Buffer to store the entire eeprom contents in
 */
int at24c02_read_all(uint8_t *p_buf) {
  if (!m_at24c02_driver_data.i2c) {
    return -EIO;
  }

  int ret = 0;
  ff_i2c_take();
  for (uint16_t addr = 0; addr < MIN(AT24C_SIZE, 256); addr++) {
    ret = i2c_reg_read_byte(m_at24c02_driver_data.i2c,
                            CONFIG_AT24C02_I2C_ADDRESS, addr, &p_buf[addr]);
    if (ret < 0) {
      return ret;
    }
  }
  // ret=  i2c_write_read(m_at24c02_driver_data.i2c, CONFIG_AT24C02_I2C_ADDRESS,
  //                        &reg, 1, p_buf, AT24C_SIZE);
  ff_i2c_give();
  return ret;
}

/**
 * @brief Write a byte to the EEPROM. This function will block until write is
 * complete
 * @param addr		8-bit address to write to
 * @param value		The value to write to that address
 */
int at24c02_write_byte(uint8_t addr, uint8_t value) {
  if (!m_at24c02_driver_data.i2c) {
    return -EIO;
  }

  int err = 0;

  ff_i2c_take();
  err = i2c_reg_write_byte(m_at24c02_driver_data.i2c,
                           CONFIG_AT24C02_I2C_ADDRESS, addr, value);
  ff_i2c_give();
  k_sleep(10);
  return err;
}

/**
 * @brief Write a buffer of bytes to the EEPROM
 * @param addr    Register address to write to
 * @param p_buf   Buffer to write
 * @param len     Size of that buffer
 * @param return  Negative value if error
 */
int at24c02_write_bytes(uint8_t addr, uint8_t *p_buf, size_t len) {
  if (!m_at24c02_driver_data.i2c) {
    return -EIO;
  }

  int err = 0;
  uint8_t tx_buf[1 + AT24C_PAGE_SIZE];
  size_t bytes_written = 0;

  while (len > 0) {
    uint8_t bytes_to_write = MIN(len, AT24C_PAGE_SIZE);
    tx_buf[0] = addr;
    memcpy(tx_buf + 1, p_buf + bytes_written, bytes_to_write);

    ff_i2c_take();
    err = i2c_write(m_at24c02_driver_data.i2c, tx_buf, 1 + bytes_to_write,
                    CONFIG_AT24C02_I2C_ADDRESS);

    ff_i2c_give();
    k_sleep(AT24C_WRITE_CYCLE + 1); // Wait for write cycle to complete

    // Manage where we're writing next
    len -= bytes_to_write;
    addr += bytes_to_write;
    bytes_written += bytes_to_write;

    if (err) {
      LOG_ERR("Unable to write to EEPROM [%d]", err);
      return err;
    }

    LOG_DBG("Wrote %d bytes to EEPROM, %d left", bytes_to_write, len);
  }

  return 0;
}

DEVICE_INIT(at24c02, CONFIG_AT24C02_DEV_NAME, __init, &m_at24c02_driver_data,
            NULL, APPLICATION, CONFIG_AT24C02_INIT_PRIORITY);