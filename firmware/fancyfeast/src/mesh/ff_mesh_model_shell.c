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
#include <posix/time.h>
#include <shell/shell.h>

#define LOG_LEVEL 4
#include <logging/log.h>
LOG_MODULE_REGISTER(ff_mesh_model_shell);

#include "../ff_bling.h"
#include "../ff_gfx.h"
#include "../ff_shell.h"
#include "../ff_time.h"
#include "../ff_util.h"
#include "../system.h"
#include "ff_mesh.h"
#include "ff_mesh_model_shell.h"
#include "ff_mesh_proxy.h"

#define RETURN_CODE_UNSET 0xFFFFFFFF

static struct bt_mesh_model *m_p_model;
static volatile int m_return_code = RETURN_CODE_UNSET;

/**
 * @brief Shell command that sends a command to another badge
 * @param shell		Pointer to shell context
 * @param argc		Number of command arguments
 * @param argv		Pointer to array of command arguments
 * @return 0 if successful
 */
static int __cmd_send(const struct shell *shell, size_t argc, char **argv) {
  if (argc != 3) {
    shell_error(shell, "Invalid argument(s)");
    return -1;
  }

  // Parse and validate destination address
  uint16_t address = strtol(argv[1], NULL, 10);
  if (address > 0x8000) {
    shell_error(shell, "Invalid address");
    return -1;
  }

  // Validate the command
  const char *cmd = argv[2];
  if (strlen(cmd) > FF_MESH_MODEL_SHELL_CMD_SIZE - 1) {
    shell_error(shell, "Command too long");
    return -1;
  }

  // FIRE!
  m_return_code = RETURN_CODE_UNSET;
  shell_print(shell, "Sending '%s' to Badge %d. YOLO!", cmd, address);
  ff_mesh_model_shell_publish(address, cmd, strlen(cmd));

  // // Now wait for the results, timing out if necessary
  // // remember when we started waiting
  // struct timespec timeout = ff_time_now_get();
  // timeout.tv_sec += 10;

  // // Wait
  // LOG_DBG("Waiting for shell response");
  // while (m_return_code == RETURN_CODE_UNSET) {
  //   // Don't wait too long
  //   struct timespec now = ff_time_now_get();
  //   if (now.tv_sec > timeout.tv_sec) {
  //     if (shell != NULL) {
  //       shell_error(shell, "\tTimeout while waiting for response :(\r");
  //       LOG_ERR("Shell command timeout");
  //     }
  //     return -1;
  //   }

  //   // Sleep a bit
  //   k_sleep(100);
  // }

  // shell_print(shell, "Return: %d", m_return_code);
  return 0;
}

/**
 * @brief Callback for when a GET model is received from the mesh. When
 * received execute the shell command and return the code
 * @param model		Pointer to mesh model that was received
 * @param ctx		Pointer to context (src dest etc) for the model we
 * received
 * @param buf		Pointer to buffere containing the model data
 */
static void __shell_get(struct bt_mesh_model *p_model,
                        struct bt_mesh_msg_ctx *p_ctx,
                        struct net_buf_simple *p_buf) {
  if (p_buf->len > FF_MESH_MODEL_SHELL_CMD_SIZE) {
    LOG_ERR("Invalid shell command received len=%d expected=%d", p_buf->len,
            FF_MESH_MODEL_SHELL_CMD_SIZE);
    return;
  }

  // Ensure null terminated
  p_buf->data[p_buf->len - 1] = 0;

  char cmd[FF_MESH_MODEL_SHELL_CMD_SIZE];
  memcpy(cmd, p_buf->data, FF_MESH_MODEL_SHELL_CMD_SIZE);

  // Execute the command and get the return code
  LOG_DBG("Shell command received '%s'", cmd);

  int ret = 0;

  // Filter out bad commands
  if (strstr(cmd, "SCRATCH") != NULL) {
    ret = -1;
  } else if (strstr(cmd, "MEOW") != NULL) {
    ret = -1;
  }
  // Execute the command if filters allow
  else {
    ret = ff_shell_execute(cmd);
  }

  // Indicate on badge that somebody else ran a command
  // if (ret == 0) {
  //   FF_BLING_DEFAULT_FLASH(flash);
  //   flash.hue = HUE_GREEN;
  //   ff_bling_mode_push(flash);
  // }

  // Print to shell
  shell_print(ff_shell_ctx_get(), "Badge %d executed '%s'", p_ctx->addr, cmd);

  // Send a response back
  // Needed size: opcode (2 bytes) + return_code + MIC
  // NET_BUF_SIMPLE_DEFINE(msg, 2 + sizeof(ret) + 4);
  // bt_mesh_model_msg_init(&msg, FF_MESH_MODEL_SHELL_OPCODE_STATUS);

  // // Put the return code into the buffer
  // LOG_DBG("Responding to shell command at addr: 0x%04x opcode 0x%08x "
  //         "return_code 0x%08x",
  //         p_ctx->addr, p_model->op->opcode, ret);
  // net_buf_simple_add_mem(&msg, &ret, sizeof(ret));

  // // Send back the response
  // if (bt_mesh_model_send(p_model, p_ctx, &msg, NULL, NULL)) {
  //   LOG_ERR("Unable to reply to shell");
  // }
}

/**
 * @brief Callback for when a STATUS model is received from the mesh. When
 * received store the return code so the executing command knows the result
 * @param model		Pointer to mesh model that was received
 * @param ctx		Pointer to context (src dest etc) for the model we
 * received
 * @param buf		Pointer to buffere containing the model data
 */
static void __shell_status(struct bt_mesh_model *p_model,
                           struct bt_mesh_msg_ctx *p_ctx,
                           struct net_buf_simple *p_buf) {
  m_return_code = *(int *)p_buf->data;
  LOG_DBG("Return code received: 0x%08x", m_return_code);
}

// Register the opcodes with callbacks
const struct bt_mesh_model_op ff_mesh_model_shell_op[] = {
    // Set with no ACK (broadcast)
    {FF_MESH_MODEL_SHELL_OPCODE_GET, FF_MESH_MODEL_SHELL_CMD_SIZE,
     __shell_get},                                          // Shell send
    {FF_MESH_MODEL_SHELL_OPCODE_STATUS, 0, __shell_status}, // Shell response
    BT_MESH_MODEL_OP_END,
};

/**
 * @brief Initialize the shell mesh model. Bind the model and subscribe to
 * other badges.
 * @param address         The local address to use
 * @param element_address The element used to bind the model to. Probably the
 * same as address.
 * @param p_model         Pointer to the mesh model object created in the main
 * mesh
 */
void ff_mesh_model_shell_init(uint16_t address, uint16_t element_address,
                              struct bt_mesh_model *p_model) {
  // Bind shell model
  LOG_DBG("Binding SHELL Model address: 0x%04x element: 0x%04x model_id:0x%04x",
          address, element_address, FF_MESH_MODEL_SHELL_ID);
  bt_mesh_cfg_mod_app_bind(MESH_NET_INDEX, address, element_address,
                           MESH_APP_INDEX, FF_MESH_MODEL_SHELL_ID, NULL);

  // Subscribe to shell so we can execute commands
  LOG_DBG("Subscribing to SHELL Model address: 0x%04x model_id:0x%04x", address,
          FF_MESH_MODEL_SHELL_ID);
  m_p_model = p_model;
  bt_mesh_cfg_mod_sub_add(MESH_NET_INDEX, address, element_address, address,
                          FF_MESH_MODEL_SHELL_ID, NULL);
}

/**
 * @brief Publish a shell command to the mesh immediately
 * @param dest		Destination address to send to
 * @param cmd		Command string to send
 * @param length	Length of command string
 */
void ff_mesh_model_shell_publish(uint16_t dest, const char *cmd,
                                 size_t length) {
  uint8_t *data[FF_MESH_MODEL_SHELL_CMD_SIZE];
  memset(data, 0, FF_MESH_MODEL_SHELL_CMD_SIZE);
  snprintf((char *)data, FF_MESH_MODEL_SHELL_CMD_SIZE, "%s", cmd);

  LOG_DBG("Publishing shell cmd: '%s'", cmd);
  m_p_model->pub->addr = dest;
  m_p_model->pub->ttl = CONFIG_ANDNXOR_SHELL_TTL;
  bt_mesh_model_msg_init(m_p_model->pub->msg, FF_MESH_MODEL_SHELL_OPCODE_GET);
  net_buf_simple_add_mem(m_p_model->pub->msg, data,
                         FF_MESH_MODEL_SHELL_CMD_SIZE);
  bt_mesh_model_publish(m_p_model);
}

SHELL_CMD_REGISTER(MEOW, NULL, "EXECUET SUMTHING ON ANOTHR BADGE", __cmd_send);