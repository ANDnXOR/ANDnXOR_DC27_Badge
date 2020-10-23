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
#include <stdlib.h>

#include <bluetooth/mesh.h>
#include <bluetooth/mesh/access.h>
#include <posix/time.h>
#include <shell/shell.h>

#define LOG_LEVEL 4
#include <logging/log.h>
LOG_MODULE_REGISTER(ff_mesh_model_ping);

#include "../ff_shell.h"
#include "../ff_time.h"
#include "ff_mesh.h"
#include "ff_mesh_model_ping.h"

// Local pointer to ping model
static struct bt_mesh_model *m_p_model;
static volatile struct timespec m_timestamp;

/**
 * @brief Shell command that pings another badge
 * @param shell		Pointer to shell context
 * @param argc		Number of command arguments
 * @param argv		Pointer to array of command arguments
 * @return 0 if successful
 */
static int __cmd_ping(const struct shell *shell, size_t argc, char **argv) {
  uint16_t address = 0;
  if (argc != 2) {
    shell_error(shell, "Invalid arguments. SCRATCH <address>");
    return -1;
  }

  address = strtol(argv[1], NULL, 10);
  shell_print(shell, "Scratching %d...\r", address);

  ff_mesh_model_ping(address);
  return 0;
}

/**
 * Callback for when a GET ping model is received from the mesh. Response to GET
 * with an ACK. Nothing crazy.
 */
static void __ping_get(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
                       struct net_buf_simple *buf) {

  // TODO: Check address with BT_MESH_ADDR_IS_UNICAST and only respond if
  // unicast

  // Needed size: opcode (2 bytes) + msg + MIC
  NET_BUF_SIMPLE_DEFINE(msg, 2 + sizeof(struct timespec) + 4);
  bt_mesh_model_msg_init(&msg, FF_MESH_MODEL_PING_OPCODE_STATUS);

  // Parrot back the time they sent us
  LOG_DBG("Responding to ping at addr: 0x%04x opcode 0x%08x", ctx->addr,
          model->op->opcode);
  net_buf_simple_add_mem(&msg, model->pub->msg->data, sizeof(struct timespec));

  if (bt_mesh_model_send(model, ctx, &msg, NULL, NULL)) {
    LOG_ERR("Unable to reply to ping");
  }
}

/**
 * Callback for when a STATUS ping model is received from the mesh. This should
 * have been a response to a previous ping get
 */
static void __ping_status(struct bt_mesh_model *model,
                          struct bt_mesh_msg_ctx *ctx,
                          struct net_buf_simple *buf) {

  m_timestamp = ff_time_now_get();

  LOG_DBG("PING Response from 0x%02x %d.%d", ctx->addr, m_timestamp.tv_sec,
          m_timestamp.tv_nsec);
}

const struct bt_mesh_model_op ff_mesh_model_ping_op[] = {
    {FF_MESH_MODEL_PING_OPCODE_GET, 0, __ping_get},       // Outbound ping
    {FF_MESH_MODEL_PING_OPCODE_STATUS, 0, __ping_status}, // Ping response
    BT_MESH_MODEL_OP_END,
};

/**
 * @brief Initialize the ping mesh model. Bind the model and subscribe to
 * other badges.
 * @param address         The local address to use
 * @param element_address The element used to bind the model to. Probably the
 * same as address.
 * @param p_model         Pointer to the mesh model object created in the main
 * mesh
 */
void ff_mesh_model_ping_init(uint16_t address, uint16_t element_address,
                             struct bt_mesh_model *p_model) {
  // Bind Ping model
  LOG_DBG(
      "Binding ping model net: 0x%04x address: 0x%04x element: 0x%04x app_idx: "
      "0x%04x",
      MESH_NET_INDEX, address, element_address, MESH_APP_INDEX);
  bt_mesh_cfg_mod_app_bind(MESH_NET_INDEX, address, element_address,
                           MESH_APP_INDEX, FF_MESH_MODEL_PING_ID, NULL);

  // Subscribe to ping so we can reply to it
  LOG_DBG(
      "Subscribing to ping model. net: 0x%04x address: 0x%04x element: 0x%04x.",
      MESH_NET_INDEX, address, element_address);
  m_p_model = p_model;
  // Subscribe only to our own address
  bt_mesh_cfg_mod_sub_add(MESH_NET_INDEX, address, element_address, address,
                          FF_MESH_MODEL_PING_ID, NULL);
}

/**
 * @brief Ping another node on the mesh
 * @param address	16-bit address of the peer
 */
void ff_mesh_model_ping(uint16_t address) {
  m_p_model->pub->addr = address;
  bt_mesh_model_msg_init(m_p_model->pub->msg, FF_MESH_MODEL_PING_OPCODE_GET);

  // Ping with our current time
  struct timespec ping_time = ff_time_now_get();

  m_timestamp = ping_time;
  net_buf_simple_add_mem(m_p_model->pub->msg, &ping_time,
                         sizeof(struct timespec));

  LOG_DBG("Pinging 0x%02x Len=%d", address, m_p_model->pub->msg->len);

  // Fire!
  bt_mesh_model_publish(m_p_model);

  // remember when we started waiting
  struct timespec timeout = ff_time_now_get();
  timeout.tv_sec += 10;

  // Wait
  while (m_timestamp.tv_nsec == ping_time.tv_nsec &&
         m_timestamp.tv_sec == ping_time.tv_sec) {

    // Don't wait too long
    struct timespec now = ff_time_now_get();
    if (now.tv_sec > timeout.tv_sec) {
      const struct shell *shell = ff_shell_ctx_get();
      if (shell != NULL) {
        shell_error(shell, "Timeout while waiting for response :(\r");
      }
      return;
    }

    // Sleep a bit
    k_sleep(100);
  }

  LOG_ERR("DONE WAITING FOR PING");

  // Now print the result
  struct timespec delta;
  delta.tv_sec = m_timestamp.tv_sec - ping_time.tv_sec;
  delta.tv_nsec = m_timestamp.tv_nsec - ping_time.tv_nsec;

  // Carry the 1
  if (delta.tv_nsec < 0) {
    delta.tv_nsec += UINT32_MAX;
    delta.tv_sec--;
  }

  // Output result of ping
  const struct shell *shell = ff_shell_ctx_get();
  if (shell != NULL) {
    shell_print(shell, "Response from %d in %d.%d second(s)", address,
                delta.tv_sec, (delta.tv_nsec / 1000000));
  }

  m_timestamp = ping_time;
}

SHELL_CMD_REGISTER(SCRATCH, NULL, "SCRATCH ANOTHR BADGE", __cmd_ping);