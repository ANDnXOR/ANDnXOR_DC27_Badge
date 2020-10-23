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

#include <fs.h>
#include <init.h>
#include <misc/printk.h>
#include <shell/shell.h>
#include <shell/shell_uart.h>
#include <version.h>
#include <zephyr.h>
#define LOG_LEVEL 4
#include <logging/log.h>
LOG_MODULE_REGISTER(ff_shell);

#include "autoconf.h"
#include "ff_fs.h"
#include "ff_settings.h"
#include "ff_shell.h"
#include "ff_util.h"
#include "system.h"

#define SHELL_CRON_FILE FF_FS_MOUNT_POINT "/JERBS"
#define SHELL_RC_FILE FF_FS_MOUNT_POINT "/RC"

static ff_shell_perm_level_t m_perm_level = ff_shell_perm_level_user;

static int __run_file(char *path);

/**
 * @brief Custom screen clearing shell command
 */
static int __cmd_clear(const struct shell *shell, size_t argc, char **argv) {
  ARG_UNUSED(argv);

  shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT, "%c%s", FF_SHELL_VT100_ESC,
                FF_SHELL_VT100_CURSOR_HOME);
  shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT, "%c%s", FF_SHELL_VT100_ESC,
                FF_SHELL_VT100_CLEAR_SCREEN);

  return 0;
}

/**
 * @brief Shell command that prints the current mesage of the day
 * @param shell		Pointer to shell context
 * @param argc		Number of command arguments
 * @param argv		Pointer to array of command arguments
 * @return 0 if successful
 */
// static int __cmd_motd(const struct shell *shell, size_t argc, char **argv) {
//   int err;
//   struct fs_file_t file;

//   // Open the file
//   err = fs_open(&file, FF_FS_MOUNT_POINT "/etc/motd");
//   if (err) {
//     shell_print(shell, "NEVER GONNA GIVE YOU UP...");
//     return 0;
//   }

//   char buffer[256];
//   ssize_t read = fs_read(&file, buffer, 255);
//   buffer[read] = 0;

//   shell_print(shell, "%s", buffer);

//   return 0;
// }

/**
 * @brief Shell command that trolls the person by changing their prompt until
 * reboot
 * @param shell		Pointer to shell context
 * @param argc		Number of command arguments
 * @param argv		Pointer to array of command arguments
 * @return 0 if successful
 */
static int __cmd_emacs(const struct shell *shell, size_t argc, char **argv) {
  ARG_UNUSED(argc);
  ARG_UNUSED(argv);
#define newpromptwhodis                                                        \
  "WERENOSTRANGERS2LUVUKNOETEHRULEZANSODOIAFULLCOMMITMENTSWUTIMTHINKINOVUWOUL" \
  "DNTGITDISFRUMANYOTHRGUYIJUSWANNATELLUHOWIMFEELINGOTTAMAKUUNDERSTANDNEVRGON" \
  "NAGIVUUPNEVRGONNALETUDOWNNEVRGONNARUNAROUND $ "
  int check = 0;

  check = shell_prompt_change(shell, newpromptwhodis);

  if (check == 0)
    shell_print(shell, "Shouldn't have done that...EMACZ IS THE SUCK!");
  else
    shell_print(shell, "Prompt too long");

  return 0;
}

/**
 * @brief Shell command that handles su command to elevate user permissions
 * @param shell		Pointer to shell context
 * @param argc		Number of command arguments
 * @param argv		Pointer to array of command arguments
 * @return 0 if successful
 */
// static int __cmd_su(const struct shell *shell, size_t argc, char **argv) {
//   if (argc != 2) {
//     shell_error(shell, "Invalid argument(s)");
//     return -1;
//   }

//   // Check password
//   if (strcmp(argv[1], "beyonce") == 0) {
//     m_perm_level = ff_shell_perm_level_root;
//     shell_prompt_change(shell, CONFIG_ANDNXOR_SHELL_ROOT_PROMPT);
//     shell_info(shell, "User permissions changed to root.");
//   } else {
//     shell_error(shell, "WRONG");
//     m_perm_level = ff_shell_perm_level_user;
//     shell_prompt_change(shell, CONFIG_ANDNXOR_SHELL_PROMPT);
//   }

//   return 0;
// }


/**
 * @brief Run a file
 * @param Full path to file including mountpoint "/NAND:/"
 */
static int __run_file(char *path) {
  int err;
  struct fs_dirent stat;
  err = fs_stat(path, &stat);
  if (err) {
    LOG_ERR("Unable to stat '%s'", path);
    return -1;
  }

  if (stat.size > 10000) {
    LOG_ERR("File is too large, nice try hax0r");
    return -1;
  }

  struct fs_file_t file;
  err = fs_open(&file, path);
  if (err) {
    LOG_ERR("Unable to open file");
    return -1;
  }

  while (stat.size > 0) {
    char line[128];
    memset(line, 0, 128);
    size_t count = 0;
    while (count < 128) {
      stat.size -= fs_read(&file, line + count, 1);
      if (line[count] == '\n') {
        line[count] = 0;
        break;
      }
      count++;
    }

    // LOG_DBG("Executing: '%s'", line);
    ff_shell_execute(line);
  }

  fs_close(&file);

  return 0;
}

/**
 * @brief Shell command that runs a file
 * @param shell		Pointer to shell context
 * @param argc		Number of command arguments
 * @param argv		Pointer to array of command arguments
 * @return 0 if successful
 */
static int __cmd_troll(const struct shell *shell, size_t argc, char **argv) {
  if (argc != 2) {
    shell_error(shell, "Invalid argument(s)");
    return -1;
  }

  char path[128];
  snprintf(path, 128, FF_FS_MOUNT_POINT "/%s", argv[1]);

  if (__run_file(path)) {
    shell_error(shell, "Cannot run file");
    return -1;
  }

  return 0;
}

/**
 * @brief Print all wall messages to shell
 */
static void __cmd_wall_print_all(const struct shell *shell) {
  // Print each message in the graffiti
  for (uint8_t i = 0; i < GRAFFITI_COUNT; i++) {
    shell_print(shell, "%d: %s", i + 1, ff_settings_ptr_get()->wall[i]);
  }
}

/**
 * @brief Shell command that has a wall for people to graffiti
 * @param shell		Pointer to shell context
 * @param argc		Number of command arguments
 * @param argv		Pointer to array of command arguments
 * @return 0 if successful
 */
static int __cmd_wall(const struct shell *shell, size_t argc, char **argv) {
  // Print usage if no input
  if (argc == 1) {
    shell_print(shell, "Incorrect Usage GRAFFITI \"FOO BAR\" or GRAFFITI SHOW");
  } else {
    if (strcmp(argv[1], "SHOW") == 0) {
      __cmd_wall_print_all(shell);
    }
    // Otherwise take the first message and push it onto the wall
    else {
      // Push messages down the wall starting from the bottom
      for (uint8_t i = GRAFFITI_COUNT - 1; i > 0; i--) {
        memcpy(ff_settings_ptr_get()->wall[i],
               ff_settings_ptr_get()->wall[i - 1], GRAFFITI_MAX_LENGTH + 1);
      }
      snprintf(ff_settings_ptr_get()->wall[0], GRAFFITI_MAX_LENGTH, "%s",
               argv[1]);
      __cmd_wall_print_all(shell);

      ff_settings_save();
    }
  }
  return 0;
}

/**
 * @brief Background task that occasionally runs cron jerbs
 */
static void __jerb_task() {
  struct fs_dirent dirent;
  if (fs_stat(SHELL_CRON_FILE, &dirent)) {
    LOG_DBG("Unable to stat JERBS file, creating an empty one");
    struct fs_file_t file;
    fs_open(&file, SHELL_CRON_FILE);
    fs_truncate(&file, 0);
    fs_close(&file);
  }

  while (1) {
    k_sleep(CONFIG_CRON_JERB_INTERVAL);
    __run_file(SHELL_CRON_FILE);
  }
}

/**
 * @brief Execute a command on our shell
 * @param cmd Char array containing the command to execute
 * @return Return code from running the command
 */
int ff_shell_execute(const char *cmd) {
  const struct shell *p_shell = ff_shell_ctx_get();
  int ret = shell_execute_cmd(p_shell, cmd);

  // Cleanup command buffer
  p_shell->ctx->cmd_buff[0] = 0;

  return ret;
}

/**
 * @brief Get a reference to the current shell
 * @return Pointer to the current shell
 */
const struct shell *ff_shell_ctx_get() { return shell_backend_uart_get_ptr(); }

/**
 * @brief Initialize the fancy feast shell
 */
int ff_shell_init(struct device *device) {
  LOG_DBG("Shell Init");
  shell_prompt_change(ff_shell_ctx_get(), CONFIG_ANDNXOR_SHELL_PROMPT);
  __cmd_clear(ff_shell_ctx_get(), 1, NULL);
  // shell_info(ff_shell_ctx_get(), "\033[2JWelcome to AND!XOR DC27\n\n");
  return 0;
}

/**
 * @brief Run the shell startup script
 */
inline void ff_shell_rc() {
  struct fs_dirent dirent;
  if (fs_stat(SHELL_RC_FILE, &dirent)) {
    LOG_DBG("Unable to stat RC file, creating an empty one");
    struct fs_file_t file;
    fs_open(&file, SHELL_RC_FILE);
    fs_truncate(&file, 0);
    fs_close(&file);
  }

  __run_file(SHELL_RC_FILE);
}

/**
 * @brief Reset all RC and Jerbs tasks
 */
void ff_shell_rc_jerbs_reset() {
  int err = fs_unlink(SHELL_RC_FILE);
  if (err) {
    LOG_ERR("Unable to delete '%s' [%d]", SHELL_RC_FILE, err);
  }

  err = fs_unlink(SHELL_CRON_FILE);
  if (err) {
    LOG_ERR("Unable to delete '%s' [%d]", SHELL_CRON_FILE, err);
  }
}

/**
 * @brief Get current perm level of the user
 */
ff_shell_perm_level_t ff_shell_perm_level_get() { return m_perm_level; }

SHELL_CMD_REGISTER(EMACZ, NULL, "DAH GRAETESS", __cmd_emacs);
SHELL_CMD_REGISTER(WIPE, NULL, "CLER TEH SCRIN", __cmd_clear);
// SHELL_CMD_REGISTER(MOTD, NULL, "MESAGE FUR TEH DAI K", __cmd_motd);
SHELL_CMD_REGISTER(GRAFFITI, NULL, "WRIT ON WEL", __cmd_wall);
SHELL_CMD_REGISTER(TROLL, NULL, "RUN FIL K", __cmd_troll);
// SHELL_CMD_REGISTER(WIT_MEOW_MIX, NULL, "ELIVAET PERMISHUNZ K", __cmd_su);

K_THREAD_DEFINE(cronjerbs, 2048, __jerb_task, NULL, NULL, NULL,
                FF_THREAD_PRIORITY_LOW, 0, 5000);

#ifdef CONFIG_SHELL
SYS_INIT(ff_shell_init, APPLICATION, 99);
#endif