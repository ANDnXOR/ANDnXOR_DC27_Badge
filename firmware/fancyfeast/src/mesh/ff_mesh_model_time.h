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
#ifndef FF_MESH_MODEL_TIME_H
#define FF_MESH_MODEL_TIME_H

#include <bluetooth/mesh.h>
#include <bluetooth/mesh/access.h>

#define FF_MESH_MODEL_TIME_ID BT_MESH_MODEL_ID_TIME_SRV
#define FF_MESH_MODEL_TIME_OPCODE_UNACK BT_MESH_MODEL_OP_2(0x82, 0x03)

extern const struct bt_mesh_model_op ff_mesh_model_time_op[];

/**
 * @brief Initialize the time mesh model. Bind the model and subscribe to
 * other badges.
 * @param address         The local address to use
 * @param element_address The element used to bind the model to. Probably the
 * same as address.
 * @param p_model         Pointer to the mesh model object created in the main
 * mesh
 */
extern void ff_mesh_model_time_init(uint16_t address, uint16_t element_address,
                                    struct bt_mesh_model *p_model);

/**
 * @brief Publish the current time to the mesh immediately
 */
extern void ff_mesh_model_time_publish_now();

#endif