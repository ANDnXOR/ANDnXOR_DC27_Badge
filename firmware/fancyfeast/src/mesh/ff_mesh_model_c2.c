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
#include <stdlib.h>

#include <bluetooth/mesh.h>
#include <bluetooth/mesh/access.h>
#include <shell/shell.h>

#define LOG_LEVEL 4
#include <logging/log.h>
LOG_MODULE_REGISTER(FF_MESH_MODEL_C2);

#include "../bling/ff_bling_doom.h"
#include "../bling/ff_bling_grid.h"
#include "../bling/ff_bling_matrix.h"
#include "../bling/ff_bling_rainbow.h"
#include "../bling/ff_bling_scroll.h"
#include "../bling/ff_bling_sin.h"
#include "../bling/ff_bling_tunnel.h"
#include "../ff_bling.h"
#include "../ff_gfx.h"
#include "../ff_unlocks.h"
#include "ff_mesh.h"
#include "ff_mesh_model_c2.h"
#include "ff_mesh_proxy.h"

static struct bt_mesh_model *m_p_model;

const static char *__messages[] = {"AND!XOR",
                                   "BEER TIME",
                                   "3,2,1 Rule",
                                   "DEFCON is Cancelled",
                                   "I read Bloomberg for the security articles",
                                   "Never Gonna Give You Up",
                                   "Never Gonna Let You Down",
                                   "Never Gonna Run Around and Desert You"};

#define MESSAGE_COUNT 8

#define C2_UNLOCK_TUNNEL 0x71

/**
 * @brief Handle C2 commands
 */
static void __c2_handler(uint8_t cmd, uint8_t data) {
  if (cmd > __c2_cmd_counter) {
    LOG_ERR("Invalid C2 command");
    return;
  }

  LOG_DBG("Running CMD %d DATA %d", cmd, data);

  switch (cmd) {
    /**
     * Bling C2 command received
     */
  case c2_cmd_bling:
    switch (data) {
    case ff_bling_mode_rainbow:;
      FF_BLING_DEFAULT_RAINBOW(rainbow);
      ff_bling_mode_push(rainbow);
      break;
    case ff_bling_mode_sin:;
      FF_BLING_DEFAULT_SIN(sin);
      ff_bling_mode_push(sin);
      break;
    case ff_bling_mode_matrix:;
      FF_BLING_DEFAULT_MATRIX(matrix);
      ff_bling_mode_push(matrix);
      break;
    case ff_bling_mode_doom:;
      FF_BLING_DEFAULT_DOOM(doom);
      ff_bling_mode_push(doom);
      break;
    case ff_bling_mode_grid:;
      FF_BLING_DEFAULT_GRID(grid);
      ff_bling_mode_push(grid);
      break;
    case ff_bling_mode_copycat:;
      FF_BLING_DEFAULT_COPYCAT(copy);
      ff_bling_mode_push(copy);
      break;
    default:
      LOG_ERR("Invalid C2 bling data");
      break;
    }
    break;
  /**
   * Display message C2 commmand received
   */
  case c2_cmd_message:
    if (data >= MESSAGE_COUNT) {
      LOG_ERR("Invalid C2 message data");
      return;
    }

    LOG_DBG("Scroll bouncing '%s'", __messages[data]);
    FF_BLING_DEFAULT_SCROLL(msg, __messages[data]);
    ff_bling_mode_push(msg);
    break;

    /**
     * Unlock C2 command received
     */
  case c2_cmd_unlock:
    if (data == C2_UNLOCK_TUNNEL) {
      if (!FF_UNLOCK_VALIDATE(FF_UNLOCK_C2)) {
        FF_UNLOCK_SET(FF_UNLOCK_C2);
        FF_BLING_DEFAULT_TUNNEL(bling);
        ff_bling_mode_register(bling, ff_bling_handler_tunnel);
        ff_bling_mode_push(bling);
        ff_gfx_fill(COLOR_YELLOW);
        ff_gfx_push_buffer();
        k_sleep(3000);
      }
    }
    break;
  }
}

/**
 * @brief Callback for when an UNACK model is received from the mesh. Parse
 * and execute the C2 command if valid
 * @param model		Pointer to mesh model that was received
 * @param ctx		Pointer to context (src dest etc) for the model
 * we received
 * @param buf		Pointer to buffere containing the model data
 */
static void __c2_unack(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
                       struct net_buf_simple *buf) {
  if (buf->len != sizeof(mesh_model_c2_t)) {
    LOG_ERR("Invalid C2 UNACK packet received.");
    return;
  }

  mesh_model_c2_t *p_c2 = (mesh_model_c2_t *)buf->data;
  LOG_DBG("C2 Received From 0x%04x CMD 0x%02x DATA 0x%02x", ctx->addr,
          p_c2->command, p_c2->data);
  __c2_handler(p_c2->command, p_c2->data);
}

const struct bt_mesh_model_op ff_mesh_model_c2_op[] = {
    // Set with no ACK (broadcast)
    {FF_MESH_MODEL_C2_UNACK, sizeof(mesh_model_c2_t), __c2_unack},
    BT_MESH_MODEL_OP_END,
};

/**
 * @brief Send a C2 command now to the mesh
 */
void ff_mesh_model_c2_publish_now(uint8_t cmd, uint8_t data) {
  mesh_model_c2_t c2;
  c2.command = cmd;
  c2.data = data;

  LOG_DBG("Publishing C2 CMD: 0x%02x DATA: 0x%02x", cmd, data);
  m_p_model->pub->addr = ADDRESS_BROADCAST_ALL;
  m_p_model->pub->ttl = CONFIG_ANDNXOR_C2_TTL;
  bt_mesh_model_msg_init(m_p_model->pub->msg, FF_MESH_MODEL_C2_UNACK);
  net_buf_simple_add_mem(m_p_model->pub->msg, &c2, sizeof(mesh_model_c2_t));
  bt_mesh_model_publish(m_p_model);
}

/**
 * @brief Initialize the C2 mesh model. Bind the model and subscribe to
 * other badges.
 * @param address         The local address to use
 * @param element_address The element used to bind the model to. Probably
 * the same as address.
 * @param p_model         Pointer to the mesh model object created in the
 * main mesh
 */
void ff_mesh_model_c2_init(uint16_t address, uint16_t element_address,
                           struct bt_mesh_model *p_model) {
  // Bind c2 model
  LOG_DBG("Binding C2 Model address: 0x%04x element: 0x%04x model_id:0x%04x",
          address, element_address, FF_MESH_MODEL_C2_ID);
  bt_mesh_cfg_mod_app_bind(MESH_NET_INDEX, address, element_address,
                           MESH_APP_INDEX, FF_MESH_MODEL_C2_ID, NULL);

  // Subscribe to C2 so we can execute its commands
  LOG_DBG("Subscribing to C2 Model address: 0x%04x model_id:0x%04x",
          ADDRESS_BROADCAST_ALL, FF_MESH_MODEL_C2_ID);
  m_p_model = p_model;
  bt_mesh_cfg_mod_sub_add(MESH_NET_INDEX, address, element_address,
                          ADDRESS_BROADCAST_ALL, FF_MESH_MODEL_C2_ID, NULL);
}

// static int __cmd_c2(const struct shell *shell, size_t argc, char **argv) {
//   if (argc != 3) {
//     shell_error(shell, "Incorrect argument count. Expected cc <cmd> <data>");
//     return -1;
//   }

//   uint8_t cmd = strtol(argv[1], NULL, 10);
//   uint8_t data = strtol(argv[2], NULL, 10);

//   shell_print(shell, "Sending command %d with data %d.", cmd, data);
//   ff_mesh_model_c2_publish_now(cmd, data);

//   return 0;
// }

// SHELL_CMD_REGISTER(cc, NULL, "C2", __cmd_c2);