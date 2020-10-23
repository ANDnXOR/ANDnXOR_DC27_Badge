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

#include <bluetooth/mesh.h>
#include <bluetooth/mesh/access.h>

#define LOG_LEVEL 4
#include <logging/log.h>
LOG_MODULE_REGISTER(ff_mesh_model_status);

#include "../ff_bender.h"
#include "../ff_post.h"
#include "../ff_settings.h"
#include "../ff_social.h"
#include "../ff_time.h"
#include "../ff_unlocks.h"
#include "../ff_util.h"
#include "../system.h"
#include "ff_mesh.h"
#include "ff_mesh_model_status.h"
#include "ff_mesh_proxy.h"

static struct bt_mesh_model* m_p_model;
static mesh_model_status_t m_status;

/**
 * @brief Callback for when an UNACK model is received from the mesh. Right now
 * just log the result, we will need to do things with this later.
 * @param model		Pointer to mesh model that was received
 * @param ctx		Pointer to context (src dest etc) for the model we
 * received
 * @param buf		Pointer to buffere containing the model data
 */
static void __status_unack(struct bt_mesh_model* model,
                           struct bt_mesh_msg_ctx* ctx,
                           struct net_buf_simple* buf) {
  if (buf->len != sizeof(mesh_model_status_t)) {
    LOG_ERR("Invalid Status UNACK packet received.");
    return;
  }

  mesh_model_status_t* p_status = (mesh_model_status_t*)buf->data;

  char name[9];
  snprintf(name, 9, "%s", p_status->name);
  LOG_DBG("Status Received From 0x%04x FW v%d POST 0x%04x NAME %s SCORE %d", p_status->address,
          p_status->fw_version_int, p_status->post, name, p_status->score);

  ff_mesh_proxy_notify(FF_MESH_MODEL_STATUS_OPCODE_UNACK, buf->data, buf->len, ctx->recv_ttl);
}

/**
 * @brief Background task that occasionally publishes badge state
 */
static void __status_task() {
  while (1) {
    k_sleep(CONFIG_MESH_STATUS_PUBLISH_INTERVAL);
    ff_mesh_model_status_publish_now();
  }
}

const struct bt_mesh_model_op ff_mesh_model_status_op[] = {
    // Set with no ACK (broadcast)
    {FF_MESH_MODEL_STATUS_OPCODE_UNACK, sizeof(mesh_model_status_t), __status_unack},
    BT_MESH_MODEL_OP_END,
};

/**
 * @brief Publish the current status to the mesh immediately
 */
void ff_mesh_model_status_publish_now() {
  settings_t *p_settings = ff_settings_ptr_get();

  m_status.address = ff_mesh_addr_get();
  m_status.fw_version_int = VERSION_INT;
  m_status.post = ff_post_state_get();

  //Combine scores from parts of the badge into the badge status score
  //Score is masked as follows [unlocks 12-bit][social 10-bit][bender 10-bit]
  m_status.score = 0;
  m_status.score += ff_bender_score_get() & 0x03FF; 
  m_status.score += (ff_social_count() & 0x3FF) << 10;
  m_status.score += ((uint32_t)ff_settings_ptr_get()->unlock & 0x0FFF) << 20;

  //Clear name buffer before filling it to prevent corruption
  memset(m_status.name, 0, NAME_MAX_LENGTH);
  memcpy(m_status.name, p_settings->name, MIN(NAME_MAX_LENGTH, strlen(p_settings->name)));

  LOG_DBG("Publishing current health and status");
  m_p_model->pub->addr = ADDRESS_BROADCAST_CLOUD;
  bt_mesh_model_msg_init(m_p_model->pub->msg, FF_MESH_MODEL_STATUS_OPCODE_UNACK);
  net_buf_simple_add_mem(m_p_model->pub->msg, &m_status, sizeof(mesh_model_status_t));
  bt_mesh_model_publish(m_p_model);
}

/**
 * @brief Initialize the status mesh model. Bind the model and subscribe to
 * other badges.
 * @param address         The local address to use
 * @param element_address The element used to bind the model to. Probably the
 * same as address.
 * @param p_model         Pointer to the mesh model object created in the main
 * mesh
 */
void ff_mesh_model_status_init(uint16_t address,
                               uint16_t element_address,
                               struct bt_mesh_model* p_model) {
  // Bind status model
  LOG_DBG(
      "Binding status net: 0x%04x address: 0x%04x element: 0x%04x app_idx: "
      "0x%04x",
      MESH_NET_INDEX, address, element_address, MESH_APP_INDEX);
  bt_mesh_cfg_mod_app_bind(MESH_NET_INDEX, address, element_address, MESH_APP_INDEX,
                           FF_MESH_MODEL_STATUS_ID, NULL);

  // Subscribe to status so we can potentially proxy it
  LOG_DBG("Subscribing to status net: 0x%04x address: 0x%04x element: 0x%04x.", MESH_NET_INDEX,
          address, element_address);
  m_p_model = p_model;
  bt_mesh_cfg_mod_sub_add(MESH_NET_INDEX, address, element_address, ADDRESS_BROADCAST_CLOUD,
                          FF_MESH_MODEL_STATUS_ID, NULL);
}

K_THREAD_DEFINE(b_status,
                1024,
                __status_task,
                NULL,
                NULL,
                NULL,
                FF_THREAD_PRIORITY_MEDIUM,
                0,
                5000);