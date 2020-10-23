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
#include <posix/time.h>
#include <shell/shell.h>

#define LOG_LEVEL 4
#include <logging/log.h>
LOG_MODULE_REGISTER(ff_mesh_model_time);

#include "../ff_time.h"
#include "ff_mesh.h"
#include "ff_mesh_model_time.h"
#include "ff_mesh_proxy.h"

static struct bt_mesh_model* m_p_model;

/**
 * @brief Callback for when an UNACK time model is received from the mesh. When
 * received adjust local time
 * @param model		Pointer to mesh model that was received
 * @param ctx		Pointer to context (src dest etc) for the model we
 * received
 * @param buf		Pointer to buffere containing the model data
 */
static void __time_unack(struct bt_mesh_model* p_model,
                         struct bt_mesh_msg_ctx* p_ctx,
                         struct net_buf_simple* p_buf) {
  if (p_buf->len != sizeof(struct timespec)) {
    LOG_ERR("Invalid time receieved from mesh. len=%d expected=%d", p_buf->len,
            sizeof(struct timespec));
    return;
  }

  struct timespec now;
  memcpy(&now, p_buf->data, sizeof(struct timespec));
  ff_time_now_set(&now);
}

// Register opcodes and callbacks
const struct bt_mesh_model_op ff_mesh_model_time_op[] = {
    {FF_MESH_MODEL_TIME_OPCODE_UNACK, sizeof(struct timespec), __time_unack},
    BT_MESH_MODEL_OP_END,
};

/**
 * @brief Initialize the time mesh model. Bind the model and subscribe to
 * other badges.
 * @param address         The local address to use
 * @param element_address The element used to bind the model to. Probably the
 * same as address.
 * @param p_model         Pointer to the mesh model object created in the main
 * mesh
 */
void ff_mesh_model_time_init(uint16_t address,
                             uint16_t element_address,
                             struct bt_mesh_model* p_model) {
  // Bind time model
  LOG_DBG("Binding TIME Model address: 0x%04x element: 0x%04x model_id:0x%04x", address,
          element_address, FF_MESH_MODEL_TIME_ID);
  bt_mesh_cfg_mod_app_bind(MESH_NET_INDEX, address, element_address, MESH_APP_INDEX,
                           FF_MESH_MODEL_TIME_ID, NULL);

  // Subscribe to time so we can potentially proxy it
  LOG_DBG("Subscribing to TIME Model address: 0x%04x model_id:0x%04x", ADDRESS_BROADCAST_ALL,
          FF_MESH_MODEL_TIME_ID);
  m_p_model = p_model;
  bt_mesh_cfg_mod_sub_add(MESH_NET_INDEX, address, element_address, ADDRESS_BROADCAST_ALL,
                          FF_MESH_MODEL_TIME_ID, NULL);
}

/**
 * @brief Publish the current time to the mesh immediately
 */
void ff_mesh_model_time_publish_now() {
  struct timespec now = ff_time_now_get();
  m_p_model->pub->addr = ADDRESS_BROADCAST_ALL;
  m_p_model->pub->ttl = CONFIG_ANDNXOR_TIME_TTL;
  
  bt_mesh_model_msg_init(m_p_model->pub->msg, FF_MESH_MODEL_TIME_OPCODE_UNACK);
  net_buf_simple_add_mem(m_p_model->pub->msg, &now, sizeof(struct timespec));
  bt_mesh_model_publish(m_p_model);
}