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
#ifndef FF_SHELL_H
#define FF_SHELL_H

#define FF_SHELL_VT100_ESC 0x1B
#define FF_SHELL_VT100_CURSOR_HOME "[H"
#define FF_SHELL_VT100_CLEAR_SCREEN "[2J"

typedef enum { ff_shell_perm_level_user, ff_shell_perm_level_root } ff_shell_perm_level_t;

/**
 * @brief Get a reference to the current shell
 * @return Pointer to the current shell
 */
extern const struct shell* ff_shell_ctx_get();

/**
 * @brief Execute a command on our shell
 * @param cmd Char array containing the command to execute
 * @return Return code from running the command
 */
extern int ff_shell_execute(const char* cmd);

/**
 * @brief Initialize the fancy feast shell
 */
extern int ff_shell_init();

/**
 * @brief Get current perm level of the user
 */
extern ff_shell_perm_level_t ff_shell_perm_level_get();

/**
 * @brief Run the shell startup script
 */
extern void ff_shell_rc();

/**
 * @brief Reset all RC and Jerbs tasks
 */
extern void ff_shell_rc_jerbs_reset();

#endif