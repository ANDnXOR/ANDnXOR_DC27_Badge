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

#include <gpio.h>
#include <init.h>
#include <kernel.h>
#include "autoconf.h"

#define LOG_LEVEL 4
#include <logging/log.h>
LOG_MODULE_REGISTER(spi_nor_reset);

#ifdef CONFIG_ANDNXOR_SPI_NOR

/**
 * @brief This gets called before W25Q flash driver is initialized. It simply forces a reset of the
 * flash by toggling GPIO.
 */
static int __init(struct device* dev) {
  // Grab GPIO device
  struct device* gpio = device_get_binding(SW0_GPIO_CONTROLLER);
  if (!gpio) {
    LOG_ERR("Unable to grab GPIO0 Controller");
    return -1;
  }

  LOG_DBG("Reseting SPI NOR device");
  
  // Set LEDs as output
  gpio_pin_configure(gpio, CONFIG_ANDNXOR_SPI_NOR_RESET_PIN, GPIO_DIR_OUT);
  // Turn off
  gpio_pin_write(gpio, CONFIG_ANDNXOR_SPI_NOR_RESET_PIN, 0);
  k_sleep(10);
  gpio_pin_write(gpio, CONFIG_ANDNXOR_SPI_NOR_RESET_PIN, 1);
  LOG_DBG("Flash reset");
  return 0;
}

SYS_INIT(__init, POST_KERNEL, CONFIG_ANDNXOR_SPI_NOR_RESET_INIT_PRIORITY);

#endif