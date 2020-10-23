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

#include <settings/settings.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/buf.h>
#include <bluetooth/mesh.h>
#include <bluetooth/mesh/access.h>
#include <bluetooth/mesh/proxy.h>
#include <gpio.h>
#include <mgmt/smp_bt.h>
#include <nrf52840.h>
#include <posix/time.h>
#include <shell/shell.h>
#include <stdio.h>
#include <stdlib.h>
#include <zephyr.h>

#define LOG_LEVEL 4
#include <logging/log.h>
LOG_MODULE_REGISTER(ff_mesh);

#include "../ff_settings.h"
#include "../system.h"
#include "ff_mesh.h"
#include "ff_mesh_model_bling.h"
#include "ff_mesh_model_c2.h"
#include "ff_mesh_model_ping.h"
#include "ff_mesh_model_shell.h"
#include "ff_mesh_model_social.h"
#include "ff_mesh_model_status.h"
#include "ff_mesh_model_time.h"
#include "ff_mesh_proxy.h"

// Provisioning data
static const u8_t net_key[16] = {0xcc, 0xb4, 0x58, 0xbb, 0x91, 0xdc,
                                 0x55, 0xeb, 0xa2, 0x06, 0xb4, 0x87,
                                 0xf3, 0xca, 0x6d, 0x88};
static const u8_t dev_key[16] = {0xfd, 0xd0, 0x2e, 0xe1, 0x95, 0xbf,
                                 0xcd, 0xfe, 0x52, 0xb6, 0xf7, 0x60,
                                 0x43, 0x47, 0x8f, 0x1b};
static const u8_t app_key[16] = {0x37, 0xcf, 0xd1, 0x4f, 0xb2, 0x94,
                                 0xae, 0x3d, 0xdb, 0x0c, 0xb7, 0x25,
                                 0x0b, 0xf2, 0xef, 0xa8};
static u32_t iv_index;
static u8_t m_flags;
static u16_t m_address = 0x4000;

static void heartbeat(u8_t hops, u16_t feat) {
  LOG_INF("hops=%d feat=%d", hops, feat);
}

static struct bt_mesh_cfg_srv cfg_srv = {
#if defined(CONFIG_BOARD_BBC_MICROBIT)
    .relay = BT_MESH_RELAY_ENABLED,
    .beacon = BT_MESH_BEACON_DISABLED,
#else
    .relay = BT_MESH_RELAY_ENABLED,
    .beacon = BT_MESH_BEACON_ENABLED,
#endif
    .frnd = BT_MESH_FRIEND_NOT_SUPPORTED,
    .default_ttl = 7,

    /* 3 transmissions with 20ms interval */
    .net_transmit = BT_MESH_TRANSMIT(2, 20),
    .relay_retransmit = BT_MESH_TRANSMIT(3, 20),

    .hb_sub.func = heartbeat,
};

static struct bt_mesh_cfg_cli cfg_cli = {};

static void attention_on(struct bt_mesh_model *model) {}

static void attention_off(struct bt_mesh_model *model) {}

static const struct bt_mesh_health_srv_cb health_srv_cb = {
    .attn_on = attention_on,
    .attn_off = attention_off,
};

static struct bt_mesh_health_srv health_srv = {
    .cb = &health_srv_cb,
};

static const u8_t dev_uuid[16] = {0xdd, 0xdd};

static const struct bt_mesh_prov prov = {
    .uuid = dev_uuid,
};

uint8_t time_now = 0xBB;

////////////////////////// BT MESH COMPOSITION ////////////////////////////////
BT_MESH_HEALTH_PUB_DEFINE(health_pub, 0);
BT_MESH_MODEL_PUB_DEFINE(ff_mesh_model_bling_pub, NULL, 2 + sizeof(bling_t));
BT_MESH_MODEL_PUB_DEFINE(ff_mesh_model_c2_pub, NULL,
                         2 + sizeof(mesh_model_c2_t));
BT_MESH_MODEL_PUB_DEFINE(ff_mesh_model_ping_pub, NULL,
                         2 + sizeof(struct timespec));
BT_MESH_MODEL_PUB_DEFINE(ff_mesh_model_shell_pub, NULL,
                         2 + FF_MESH_MODEL_SHELL_CMD_SIZE);
BT_MESH_MODEL_PUB_DEFINE(ff_mesh_model_social_pub, NULL, 2 + 0);
BT_MESH_MODEL_PUB_DEFINE(ff_mesh_model_status_pub, NULL,
                         2 + sizeof(mesh_model_status_t));
BT_MESH_MODEL_PUB_DEFINE(ff_mesh_model_time_pub, NULL, 2 + 1);

static struct bt_mesh_model root_models[] = {
    BT_MESH_MODEL_CFG_SRV(&cfg_srv),       // Config server 0
    BT_MESH_MODEL_CFG_CLI(&cfg_cli),       // Config client 1
    BT_MESH_MODEL_HEALTH_SRV(&health_srv,  // 2
                             &health_pub), // Health server (mandatory)
    BT_MESH_MODEL(FF_MESH_MODEL_TIME_ID,   // custom model 3
                  ff_mesh_model_time_op, &ff_mesh_model_time_pub, &time_now),
    BT_MESH_MODEL(FF_MESH_MODEL_STATUS_ID, // 4
                  ff_mesh_model_status_op, &ff_mesh_model_status_pub, NULL),
    BT_MESH_MODEL(FF_MESH_MODEL_PING_ID, ff_mesh_model_ping_op,
                  &ff_mesh_model_ping_pub, NULL), // 5
    BT_MESH_MODEL(FF_MESH_MODEL_C2_ID, ff_mesh_model_c2_op,
                  &ff_mesh_model_c2_pub, NULL), // 6
    BT_MESH_MODEL(FF_MESH_MODEL_SHELL_ID, ff_mesh_model_shell_op,
                  &ff_mesh_model_shell_pub, NULL), // 7
    BT_MESH_MODEL(FF_MESH_MODEL_BLING_ID, ff_mesh_model_bling_op,
                  &ff_mesh_model_bling_pub, NULL), // 8
    BT_MESH_MODEL(FF_MESH_MODEL_SOCIAL_ID, ff_mesh_model_social_op,
                  &ff_mesh_model_social_pub, NULL), // 9
};

static struct bt_mesh_elem elements[] = {
    BT_MESH_ELEM(0, root_models, BT_MESH_MODEL_NONE),
};

static const struct bt_mesh_comp comp = {
    .cid = BT_COMP_ID_LF,
    .elem = elements,
    .elem_count = ARRAY_SIZE(elements),
};
////////////////////////// BT MESH COMPOSITION ////////////////////////////////

/**
 * @brief Fully provision the node
 */
static void __provision() {
  int err;

  // Try to provision the node, this can be dangerous but does not require a
  // provision in the loop
  err = bt_mesh_provision(net_key, MESH_NET_INDEX, m_flags, iv_index, m_address,
                          dev_key);
  if (err == -EALREADY) {
    LOG_DBG("Provisioning Used stored settings");
  } else if (err) {
    LOG_ERR("Provisioning failed (err %d)", err);
    return;
  } else {
    LOG_INF("Provisioning completed");

    // Add application key
    LOG_DBG("Adding app key");
    bt_mesh_cfg_app_key_add(MESH_NET_INDEX, m_address, MESH_NET_INDEX,
                            MESH_APP_INDEX, app_key, NULL);
  }

  // Bind to time model
  LOG_DBG("Binding to time model");
  bt_mesh_cfg_mod_app_bind(MESH_NET_INDEX, m_address, m_address, MESH_APP_INDEX,
                           FF_MESH_MODEL_TIME_ID, NULL);

  // Subscribe to time
  LOG_DBG("Subscribing to time.");
  bt_mesh_cfg_mod_sub_add(MESH_NET_INDEX, m_address, m_address, GROUP_ADDR,
                          FF_MESH_MODEL_TIME_ID, NULL);

  // Bind to health model
  LOG_DBG("Binding to health model as a server");
  bt_mesh_cfg_mod_app_bind(MESH_NET_INDEX, m_address, m_address, MESH_APP_INDEX,
                           BT_MESH_MODEL_ID_HEALTH_SRV, NULL);
  // Initialize C2 module
  ff_mesh_model_c2_init(m_address, m_address, &root_models[6]);

  // Initialize ping module
  ff_mesh_model_ping_init(m_address, m_address, &root_models[5]);

  // Initialize status module
  ff_mesh_model_status_init(m_address, m_address, &root_models[4]);

  // Initialize time module
  ff_mesh_model_time_init(m_address, m_address, &root_models[3]);

  // Initialize shell module
  ff_mesh_model_shell_init(m_address, m_address, &root_models[7]);

  // Init the bling module
  ff_mesh_model_bling_init(m_address, m_address, &root_models[8]);

  // Init the social module
  ff_mesh_model_social_init(m_address, m_address, &root_models[9]);

  LOG_DBG("Primary Element Address: 0x%04x", ff_mesh_addr_get());
}

/**
 * @brief Get the current mesh address
 * @return Current address of the node (address of primary element)
 */
uint16_t ff_mesh_addr_get() {
  if (comp.elem_count > 0) {
    return comp.elem[0].addr;
  }
  return 0;
}

/**
 * @brief Initialize the bluetooth mesh
 * @param err Status of bluetooth initialization. This is passed from from
 * ble_enable()
 */
void ff_mesh_init(int err) {
  LOG_DBG("%s", __func__);
  if (err) {
    LOG_INF("Bluetooth init failed (err %d)", err);
    return;
  }
  LOG_INF("Bluetooth initialized");

  // Load our address from UICR
  m_address = NRF_UICR->CUSTOMER[0];
  if (!m_address || m_address > 0x4000) {
    m_address = (0x4000 | (0xFFF & NRF_FICR->DEVICEID[1]));
    NRF_UICR->CUSTOMER[0] = m_address;
  }

  // Initialize our custom mesh proxy service
  ff_mesh_proxy_init(&ff_mesh_model_time_pub);

// Initialize the Bluetooth mcumgr transport
#ifdef CONFIG_MCUMGR_SMP_BT
  smp_bt_register();
#endif

  LOG_ERR("Calling bt_mesh_init");
  err = bt_mesh_init(&prov, &comp);
  if (err) {
    LOG_INF("Initializing mesh failed (err %d)", err);
    return;
  }
  LOG_ERR("Done bt_mesh_init");

  // Provision the node
  __provision();

  LOG_INF("Mesh initialized");
}

// void ff_mesh_time_test() {
//   LOG_DBG("Testing time publish");
//   struct bt_mesh_model* p_model = &root_models[3];
//   bt_mesh_model_msg_init(p_model->pub->msg, FF_MESH_MODEL_TIME_OPCODE_UNACK);
//   net_buf_simple_add_u8(p_model->pub->msg, 0xCC);
//   bt_mesh_model_publish(p_model);
// }

/**
 * @brief Shell command that sets the mesh address
 */
static int __cmd_addr(const struct shell *shell, size_t argc, char **argv) {
  // One argument, display the mesh address
  if (argc == 1) {
    shell_print(shell, "Badge address: %d", ff_mesh_addr_get());
    return 0;
  }
  // Two arguments, set the mesh address - this has issues setting bits 1 to 0
  // in UICR so removing capability else if (argc == 2) {
  //   uint16_t temp_address = (uint16_t)strtol(argv[1], NULL, 10);

  //   if (temp_address > 0 && temp_address < 0x8000) {
  //     m_address = temp_address;
  //     NRF_UICR->CUSTOMER[0] = m_address;
  //     bt_mesh_reset();
  //     __provision();
  //   } else {
  //     shell_error(shell, "Invalid Address");
  //   }
  shell_error(shell, "Invalid Argument(s)");
  return -1;
}

/**
 * @brief Shell command that resumes the mesh
 */
static int __cmd_resume(const struct shell *shell, size_t argc, char **argv) {
  bt_mesh_resume();
  shell_print(shell, "Mesh resumed.");
  return 0;
}

/**
 * @brief Shell command that suspends the mesh
 */
static int __cmd_suspend(const struct shell *shell, size_t argc, char **argv) {
  bt_mesh_suspend();
  shell_print(shell, "Mesh suspended.");
  return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(
    sub_mesh, SHELL_CMD(R, NULL, "BADGE ADRREZ K", __cmd_addr),
    SHELL_CMD(OFF, NULL, "SUSPEN COMZ", __cmd_suspend),
    SHELL_CMD(ON, NULL, "REZOOM COMZ", __cmd_resume), SHELL_SUBCMD_SET_END);
SHELL_CMD_REGISTER(COMZ, &sub_mesh, "COMZ TOOLZ K", NULL);