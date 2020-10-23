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
#include <bluetooth/bluetooth.h>
#include <init.h>
#include <zephyr.h>

#define LOG_LEVEL 4
#include <logging/log.h>
LOG_MODULE_REGISTER(ff_ble);

#include "ff_ble.h"
#include "system.h"
#include "mesh/ff_mesh.h"

/**
 * @brief Initialize all things BLE for the badge
 * @param dev	The device for the module, not sure why we need this here
 * @return Result to the init
 */
static int __ble_init(struct device *dev) {
  int err = bt_enable(ff_mesh_init);
  if (err) {
    LOG_ERR("Bluetooth init failed [%d]", err);
    return -1;
  }

  return 0;
}

SYS_INIT(__ble_init, APPLICATION, SYSTEM_INIT_PRIORITY_BLE);