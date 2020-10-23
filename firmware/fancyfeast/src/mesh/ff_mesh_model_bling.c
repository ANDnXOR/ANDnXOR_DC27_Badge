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
#include <bluetooth/mesh.h>
#include <bluetooth/mesh/access.h>
#include <fs.h>
#include <shell/shell.h>

#define LOG_LEVEL 4
#include <logging/log.h>
LOG_MODULE_REGISTER(ff_mesh_model_bling);

#include "../bling/ff_bling_lazers.h"
#include "../ff_bling.h"
#include "../system.h"
#include "ff_mesh.h"
#include "ff_mesh_model_bling.h"

static struct bt_mesh_model *m_p_model;

// Local storage for the last bling mode we received
// Initialize it with lazers
FF_BLING_DEFAULT_LAZERS(m_bling_received);

/**
 * @brief Background task that publishes the badge's current bling mode (top on
 * the bling stack) to the world
 */
static void __bling_task() {
  while (1) {
    k_sleep(CONFIG_MESH_BLING_PUBLISH_INTERVAL);
    ff_mesh_model_bling_publish_now();
  }
}

/**
 * @brief Callback for when an UNACK bling model is received from the mesh. When
 * received, store the bling state data locally.
 * @param model		Pointer to mesh model that was received
 * @param ctx		Pointer to context (src dest etc) for the model we
 * received
 * @param buf		Pointer to buffere containing the model data
 */
static void __bling_unack(struct bt_mesh_model *p_model,
                          struct bt_mesh_msg_ctx *p_ctx,
                          struct net_buf_simple *p_buf) {
  // Ensure received frame is correct size
  if (p_buf->len != sizeof(bling_t)) {
    LOG_ERR("Invalid bling data received len=%d expected=%d", p_buf->len,
            sizeof(bling_t));
    return;
  }

  // Make a local copy for validation
  bling_t temp;
  memcpy(&temp, p_buf->data, sizeof(bling_t));

  // Validate the mode
  if (temp.mode >= __bling_mode_counter) {
    LOG_DBG("Invalid bling mode received from 0x%04x", p_ctx->addr);
    return;
  }

  // // Ensure run time is forever
  // if (temp.run_time_ms != FF_BLING_RUN_TIME_FOREVER) {
  //   LOG_DBG("Temporary bling mode received from 0x%04x", p_ctx->addr);
  //   return;
  // }

  // Ensure scroll text is something
  if (temp.mode == ff_bling_mode_scroll ||
      temp.mode == ff_bling_mode_scroll_bounce) {
    size_t len = strlen(temp.user_data);
    if (len == 0 || len > (FF_BLING_USER_DATA_SIZE - 1)) {
      LOG_DBG("Scroll text is invalid length from 0x%04x", p_ctx->addr);
      return;
    }
  }

  // If we got this far it's considered valid
  LOG_DBG("Received bling from 0x%04x", p_ctx->addr);
  if (p_ctx->addr == ff_mesh_addr_get()) {
    LOG_DBG("Ignoring bling from self");
    return;
  }

  if (p_ctx->addr >= NODE_COUNT) {
    LOG_DBG("Invalid address");
    return;
  }

  // Copy the bling data locally for copycat mode
  LOG_DBG("Storing bling mode from 0x%04x", p_ctx->addr);
  memcpy((void *)&m_bling_received, &temp, sizeof(bling_t));
}

// Register the opcodes with callbacks
const struct bt_mesh_model_op ff_mesh_model_bling_op[] = {
    // Set with no ACK (broadcast)
    {FF_MESH_MODEL_BLING_OPCODE_UNACK, sizeof(bling_t), __bling_unack},
    BT_MESH_MODEL_OP_END,
};

/**
 * @brief Initialize the bling mesh model. Bind the model and subscribe to
 * other badges.
 * @param address         The local address to use
 * @param element_address The element used to bind the model to. Probably the
 * same as address.
 * @param p_model         Pointer to the mesh model object created in the main
 * mesh
 */
void ff_mesh_model_bling_init(uint16_t address, uint16_t element_address,
                              struct bt_mesh_model *p_model) {
  // Bind bling model
  LOG_DBG("Binding BLING Model address: 0x%04x element: 0x%04x model_id:0x%04x",
          address, element_address, FF_MESH_MODEL_BLING_ID);
  bt_mesh_cfg_mod_app_bind(MESH_NET_INDEX, address, element_address,
                           MESH_APP_INDEX, FF_MESH_MODEL_BLING_ID, NULL);

  // Subscribe to bling so we can potentially proxy it
  LOG_DBG("Subscribing to BLING Model address: 0x%04x model_id:0x%04x",
          ADDRESS_BROADCAST_ALL, FF_MESH_MODEL_BLING_ID);
  m_p_model = p_model;
  bt_mesh_cfg_mod_sub_add(MESH_NET_INDEX, address, element_address,
                          ADDRESS_BROADCAST_ALL, FF_MESH_MODEL_BLING_ID, NULL);
}

/**
 * @brief Publish the current bling to the mesh immediately
 */
void ff_mesh_model_bling_publish_now() {
  LOG_DBG("Publishing current bling state, size=%d", sizeof(bling_t));

  bling_t *p_mode = ff_bling_current_mode_get();
  // if (p_mode->mode == ff_bling_mode_copycat ||
  //     p_mode->run_time_ms != FF_BLING_RUN_TIME_FOREVER) {
  //   LOG_DBG("Not publishing copycat");
  //   return;
  // }

  m_p_model->pub->addr = ADDRESS_BROADCAST_ALL;
  m_p_model->pub->ttl = CONFIG_ANDNXOR_BLING_TTL;
  bt_mesh_model_msg_init(m_p_model->pub->msg, FF_MESH_MODEL_BLING_OPCODE_UNACK);
  net_buf_simple_add_mem(m_p_model->pub->msg, p_mode, sizeof(bling_t));
  bt_mesh_model_publish(m_p_model);
}

/**
 * @brief Get a pointer to the bling data received
 * @return pointer to the last bling data received
 */
bling_t *ff_mesh_model_bling_state_get() { return &m_bling_received; }

K_THREAD_DEFINE(b_bling, 2048, __bling_task, NULL, NULL, NULL,
                FF_THREAD_PRIORITY_MEDIUM, 0, 10000);