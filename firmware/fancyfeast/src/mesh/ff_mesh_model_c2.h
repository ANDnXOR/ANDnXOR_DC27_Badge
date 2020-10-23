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
#include <bluetooth/mesh/access.h>

#ifndef FF_MESH_MODEL_C2_H
#define FF_MESH_MODEL_C2_H

#include "ff_mesh.h"

#define FF_MESH_MODEL_C2_ID 0x0004

//Opcode use for spreading C2 across the mesh such that most badges receive it
#define FF_MESH_MODEL_C2_UNACK BT_MESH_MODEL_OP_2(0x80, 0x60)

typedef enum {
    c2_cmd_bling,
    c2_cmd_message,
    c2_cmd_unlock,
    __c2_cmd_counter
} c2_cmd_t;

typedef struct {
  uint8_t command;
  uint8_t data;
} __packed mesh_model_c2_t;

extern const struct bt_mesh_model_op ff_mesh_model_c2_op[];

/**
 * @brief Send a C2 command now to the mesh
 */
extern void ff_mesh_model_c2_publish_now(uint8_t cmd, uint8_t data);

/**
 * @brief Initialize the C2 mesh model. Bind the model and subscribe to
 * other badges.
 * @param address         The local address to use
 * @param element_address The element used to bind the model to. Probably the
 * same as address.
 * @param p_model         Pointer to the mesh model object created in the main
 * mesh
 */
extern void ff_mesh_model_c2_init(uint16_t address,
                                      uint16_t element_address,
                                      struct bt_mesh_model *p_model);

#endif