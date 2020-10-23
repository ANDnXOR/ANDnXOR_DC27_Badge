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

#include "nrf52840.h"
// #include "nrfx_power.h"
#include <gpio.h>
#include <posix/time.h>
#include <settings/settings.h>
#include <zephyr.h>
#define LOG_LEVEL 4
#include <logging/log.h>
LOG_MODULE_REGISTER(ff_main);

#include "ff_badgebus.h"
#include "ff_bender.h"
#include "ff_bling.h"
#include "ff_gfx.h"
#include "ff_post.h"
#include "ff_settings.h"
#include "ff_shell.h"
#include "ff_time.h"
#include "ff_ui.h"
#include "ff_util.h"
#include "mesh/ff_mesh.h"
#include "mesh/ff_mesh_model_status.h"
#include "system.h"

struct device *gpio;

void main() {
  // Grab GPIO device
  gpio = device_get_binding(SW0_GPIO_CONTROLLER);
  if (!gpio) {
    LOG_ERR("Unable to grab GPIO0 Controller");
    return;
  }

  // Set LEDs as output
  gpio_pin_configure(gpio, LED0_GPIO_PIN, GPIO_DIR_OUT);
  // Turn on to indicate booting
  gpio_pin_write(gpio, LED0_GPIO_PIN, 1);

  // Init BENDER before settings
  ff_bender_init();
  ff_settings_init();

  // Do normal stuff
  ff_gfx_init();
  ff_util_mcumgr_init();

  // Print POST status
  ff_post_dump();

  // Init bling
  ff_bling_init();

  // Run startup script
  ff_shell_rc();

  ////////////////////////////// BADGE IS READY //////////////////////////////

  LOG_INF("AND!XOR DC27 Started %s [%s]", VERSION, BUILD_TIMESTAMP);

  // Turn off to indicate ready
  k_sleep(2000);
  gpio_pin_write(gpio, LED0_GPIO_PIN, 0);

  uint8_t counter = 0;
  while (1) {
    if (ff_post_state_get() != FF_POST_DEFAULT) {
      gpio_pin_write(gpio, LED0_GPIO_PIN, counter % 2);
      k_sleep(100);
    } else {
      // Ensure status LED is off
      gpio_pin_write(gpio, LED0_GPIO_PIN, 0);
      k_sleep(1000);
    }

    counter++;

#ifdef CONFIG_ANDNXOR_TEST_FIRMWARE
  switch(counter % 3) {
    case 0:
    ff_gfx_fill(COLOR_RED);
    ff_gfx_push_buffer();
    break;
    case 1:
    ff_gfx_fill(COLOR_GREEN);
    ff_gfx_push_buffer();
    break;
    case 2:
    ff_gfx_fill(COLOR_BLUE);
    ff_gfx_push_buffer();
    break;
  }
#endif
  }
}

/*
 * Default USB SN string descriptor is CONFIG_USB_DEVICE_SN, but sometimes
 * we want use another string as SN descriptor such as the chip's unique
 * ID. So user can implement this function return a string thats will be
 * replaced the default SN.
 * Please note that the new SN descriptor you changed must has same length
 * as CONFIG_USB_DEVICE_SN.
 */
u8_t *usb_update_sn_string_descriptor(void) {
  uint32_t a0 = NRF_FICR->DEVICEID[0];
  uint32_t a1 = NRF_FICR->DEVICEID[1];
  static char sn[22];
  sprintf(sn, "%08x%08xv%04d", a0, a1, VERSION_INT);
  return sn;
}
