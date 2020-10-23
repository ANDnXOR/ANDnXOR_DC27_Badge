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
#include <string.h>

#include <base64.h>
// #include <ff.h>
#include <fs.h>
#include <init.h>
#include <nffs/nffs.h>
#include <shell/shell.h>
#include <zephyr.h>

#define LOG_LEVEL 4
#include <logging/log.h>
LOG_MODULE_REGISTER(ff_fs);

#include "autoconf.h"
#include "ff_fs.h"
#include "ff_post.h"
#include "ff_util.h"

// static FATFS fat_fs;
static struct nffs_flash_desc flash_desc;
struct fs_mount_t mount = {
    .type = FS_NFFS,
    .fs_data = &flash_desc,
};

static char m_cwd[FF_FS_MAX_PATH_LEN] = "/";

/**
 * @brief Generate an absolute path from a relative path and the cwd
 * @param name  String containing the name of the path
 * @param path  The path to copy the string into
 * @param len   The len of the path buffer
 */
static void __abs_path(const char *name, char *path, size_t len) {
  if (name[0] == '/') {
    strncpy(path, name, len - 1);
    path[len - 1] = '\0';
  } else {
    if (strcmp(m_cwd, "/") == 0) {
      snprintf(path, len, "/%s", name);
    } else {
      snprintf(path, len, "%s/%s", m_cwd, name);
    }
  }
}

/**
 * @brief Shell command to change current working directory
 * @param shell		Pointer to shell context
 * @param argc		Number of arguments
 * @param argv		Command arguments
 * @return Result of the command
 */
// static int __cmd_cd(const struct shell *shell, size_t argc, char **argv) {
//   char path[FF_FS_MAX_PATH_LEN];
//   char full_path[FF_FS_MAX_PATH_LEN];
//   struct fs_dirent entry;
//   int err;

//   if (argc < 2) {
//     strcpy(m_cwd, "/");
//     return 0;
//   }

//   if (strcmp(argv[1], "..") == 0) {
//     char *prev = strrchr(m_cwd, '/');

//     if (!prev || prev == m_cwd) {
//       strcpy(m_cwd, "/");
//     } else {
//       *prev = '\0';
//     }

//     /* No need to test that a parent exists */
//     return 0;
//   }

//   // Generate an absolute path relative to the mount point
//   __abs_path(argv[1], path, sizeof(path));

//   // Prepend the mountpoint to hide implementation from the user
//   snprintf(full_path, FF_FS_MAX_PATH_LEN, FF_FS_MOUNT_POINT "%s", path);

//   err = fs_stat(full_path, &entry);
//   if (err) {
//     shell_error(shell, "%s doesn't exist", path);
//     return -ENOEXEC;
//   }

//   if (entry.type != FS_DIR_ENTRY_DIR) {
//     shell_error(shell, "%s is not a directory", path);
//     return -ENOEXEC;
//   }

//   strcpy(m_cwd, path);

//   return 0;
// }

/**
 * @brief Shell command to list contents of a directory
 * @param shell		Pointer to shell context
 * @param argc		Number of arguments
 * @param argv		Command arguments
 * @return Result of the command
 */
static int __cmd_ls(const struct shell *shell, size_t argc, char **argv) {
  char path[FF_FS_MAX_PATH_LEN] = FF_FS_MOUNT_POINT;
  char full_path[FF_FS_MAX_PATH_LEN];
  struct fs_dir_t dir;
  int err;

  if (argc < 2) {
    strcpy(path, m_cwd);
  } else {
    __abs_path(argv[1], path, sizeof(path));
  }

  // Prepend the mountpoint to hide implementation from the user
  snprintf(full_path, FF_FS_MAX_PATH_LEN, FF_FS_MOUNT_POINT "%s", path);

  err = fs_opendir(&dir, full_path);
  if (err) {
    shell_error(shell, "Unable to open %s (err %d)", path, err);
    return -ENOEXEC;
  }

  uint16_t count = 0;
  while (count < 32000) {
    struct fs_dirent entry;

    err = fs_readdir(&dir, &entry);
    if (err) {
      shell_error(shell, "Unable to read directory\n");
      break;
    }

    /* Check for end of directory listing */
    if (entry.name[0] == '\0') {
      break;
    }

    char size[16];
    sprintf(size, "%d", entry.size);
    char line[] = "                                ";
    memcpy(line, entry.name, strlen(entry.name));
    if (entry.type == FS_DIR_ENTRY_DIR) {
      memcpy(line + strlen(entry.name), "/", 1);
    }
    memcpy(line + 13, size, strlen(size));
    // shell_print(shell, "%s%s\t\t%ld", entry.name,
    //             (entry.type == FS_DIR_ENTRY_DIR) ? "/" : "", entry.size);
    shell_print(shell, "%s", line);
    count++;
  }

  shell_print(shell, "total %d\n", count);

  fs_closedir(&dir);

  return 0;
}

/**
 * @brief Shell command to make a directory on the file system
 */
// static int __cmd_mkdir(const struct shell *shell, size_t argc, char **argv) {
//   char path[FF_FS_MAX_PATH_LEN] = FF_FS_MOUNT_POINT;
//   char full_path[FF_FS_MAX_PATH_LEN];
//   int err;

//   if (argc < 2) {
//     strcpy(path, m_cwd);
//   } else {
//     __abs_path(argv[1], path, sizeof(path));
//   }

//   // Prepend the mountpoint to hide implementation from the user
//   snprintf(full_path, FF_FS_MAX_PATH_LEN, FF_FS_MOUNT_POINT "%s", path);

//   err = fs_mkdir(full_path);
//   if (err) {
//     shell_error(shell, "Error creating dir[%d]", err);
//     err = -ENOEXEC;
//   }

//   return err;
// }

/**
 * @brief Shell command to print the current working directory
 * @param shell		Pointer to shell context
 * @param argc		Number of arguments
 * @param argv		Command arguments
 * @return Result of the command
 */
// static int __cmd_pwd(const struct shell *shell, size_t argc, char **argv) {
//   shell_print(shell, "%s", m_cwd);
//   return 0;
// }

/**
 * @brief Shell command that reads the contents of a file
 * @param shell		Pointer to shell context
 * @param argc		Number of arguments
 * @param argv		Command arguments
 * @return Result of the command
 */
static int __cmd_read(const struct shell *shell, size_t argc, char **argv) {
  char path[FF_FS_MAX_PATH_LEN];
  char full_path[FF_FS_MAX_PATH_LEN];
  struct fs_dirent dirent;
  struct fs_file_t file;
  int count;
  off_t offset;
  int err;

  __abs_path(argv[1], path, sizeof(path));
  // Prepend the mountpoint to hide implementation from the user
  snprintf(full_path, FF_FS_MAX_PATH_LEN, FF_FS_MOUNT_POINT "%s", path);
  // shell_print(shell, "Reading file '%s' abs path '%s' full path '%s'",
  // argv[1], path, full_path);

  if (argc < 2) {
    shell_error(shell, "Invalid argument(s)");
    return -1;
  }

  if (argc > 2) {
    count = strtol(argv[2], NULL, 0);
    if (count <= 0) {
      count = INT_MAX;
    }
  } else {
    count = INT_MAX;
  }

  if (argc > 3) {
    offset = strtol(argv[3], NULL, 0);
  } else {
    offset = 0;
  }

  err = fs_stat(full_path, &dirent);
  if (err) {
    shell_error(shell, "Failed to obtain file %s (err: %d)", full_path, err);
    return -ENOEXEC;
  }

  if (dirent.type != FS_DIR_ENTRY_FILE) {
    shell_error(shell, "Note a file %s", full_path);
    return -ENOEXEC;
  }

  shell_print(shell, "File size: %zd", dirent.size);

  err = fs_open(&file, full_path);
  if (err) {
    shell_error(shell, "Failed to open %s (%d)", full_path, err);
    return -ENOEXEC;
  }

  if (offset > 0) {
    fs_seek(&file, offset, FS_SEEK_SET);
  }

  while (count > 0) {
    ssize_t read;
    u8_t buf[16];
    int i;

    read = fs_read(&file, buf, MIN(count, sizeof(buf)));
    if (read <= 0) {
      break;
    }

    shell_fprintf(shell, SHELL_NORMAL, "%08X  ", offset);

    for (i = 0; i < read; i++) {
      shell_fprintf(shell, SHELL_NORMAL, "%02X ", buf[i]);
    }
    for (; i < sizeof(buf); i++) {
      shell_fprintf(shell, SHELL_NORMAL, "   ");
    }
    i = sizeof(buf) - i;
    shell_fprintf(shell, SHELL_NORMAL, "%*c", i * 3, ' ');

    for (i = 0; i < read; i++) {
      shell_fprintf(shell, SHELL_NORMAL, "%c",
                    buf[i] < 32 || buf[i] > 127 ? '.' : buf[i]);
    }

    shell_print(shell, "");

    offset += read;
    count -= read;
  }

  fs_close(&file);

  return 0;
}

/**
 * @brief Shell command that deletes a file
 * @param shell		Pointer to shell context
 * @param argc		Number of arguments
 * @param argv		Command arguments
 * @return Result of the command
 */
static int __cmd_rm(const struct shell *shell, size_t argc, char **argv) {
  char path[FF_FS_MAX_PATH_LEN];
  char full_path[FF_FS_MAX_PATH_LEN];
  int err;

  if (argc < 2) {
    shell_error(shell, "Invalid arguments");
    return -ENOEXEC;
  }

  __abs_path(argv[1], path, sizeof(path));
  snprintf(full_path, FF_FS_MAX_PATH_LEN, FF_FS_MOUNT_POINT "%s", path);
  // shell_print(shell, "Deleting file file '%s' abs path '%s' full path '%s'",
  //             argv[1], path, full_path);

  err = fs_unlink(full_path);
  if (err) {
    shell_error(shell, "Failed to remove %s (%d)", path, err);
    err = -ENOEXEC;
  }

  return err;
}

/**
 * @brief Shell command that writes to a file
 * @param shell		Pointer to shell context
 * @param argc		Number of arguments
 * @param argv		Command arguments
 * @return Result of the command
 */
static int __cmd_write(const struct shell *shell, size_t argc, char **argv) {
  char path[FF_FS_MAX_PATH_LEN];
  char full_path[FF_FS_MAX_PATH_LEN];
  u8_t buf[FF_FS_WRITE_BUFFER_SIZE];
  u8_t buf_len;
  int arg_offset;
  struct fs_file_t file;
  off_t offset = -1;
  int err;

  if (argc < 2) {
    shell_error(shell, "Invalid arguments");
    return -ENOEXEC;
  }

  __abs_path(argv[1], path, sizeof(path));
  snprintf(full_path, FF_FS_MAX_PATH_LEN, FF_FS_MOUNT_POINT "%s", path);
  // shell_print(shell, "Writing file '%s' abs path '%s' full path '%s'",
  // argv[1], path, full_path);

  if (!strcmp(argv[2], "-o")) {
    if (argc < 4) {
      shell_error(shell, "Missing argument");
      return -ENOEXEC;
    }

    offset = strtol(argv[3], NULL, 0);

    arg_offset = 4;
  } else {
    arg_offset = 2;
  }

  err = fs_open(&file, full_path);
  if (err) {
    shell_error(shell, "Failed to open %s (%d)", full_path, err);
    return -ENOEXEC;
  }

  if (offset < 0) {
    err = fs_seek(&file, 0, FS_SEEK_END);
  } else {
    err = fs_seek(&file, offset, FS_SEEK_SET);
  }
  if (err) {
    shell_error(shell, "Failed to seek %s (%d)", full_path, err);
    fs_close(&file);
    return -ENOEXEC;
  }

  buf_len = 0U;
  while (arg_offset < argc) {
    buf[buf_len++] = strtol(argv[arg_offset++], NULL, 16);

    if ((buf_len == FF_FS_WRITE_BUFFER_SIZE) || (arg_offset == argc)) {
      err = fs_write(&file, buf, buf_len);
      if (err < 0) {
        shell_error(shell, "Failed to write %s (%d)", full_path, err);
        fs_close(&file);
        return -ENOEXEC;
      }

      buf_len = 0U;
    }
  }

  fs_close(&file);

  return 0;
}

/**
 * @brief Initialize the filesystem module
 * @param dev	The device for the module, not sure why we need this here
 * @return Result to the init
 */
int ff_fs_init(struct device *dev) {
  int err;
  struct device *flash_dev;

  // Ensure mount point is set
  flash_dev = device_get_binding(CONFIG_FS_NFFS_FLASH_DEV_NAME);
  if (!flash_dev) {
    LOG_ERR("Error getting NFFS flash device binding");
  } else {
    mount.mnt_point = FF_FS_MOUNT_POINT;
    mount.storage_dev = flash_dev;

    err = fs_mount(&mount);
    if (err < 0) {
      LOG_ERR("Error mounting nffs [%d]", err);
    } else {
      LOG_INF("File system mounted at %s\n", mount.mnt_point);
      ff_post_success(FF_POST_FS_MOUNT);
    }
  }

  return 0;
}

/**
 * @brief Re-mount a file system after USB is done with it
 */
// void ff_fs_remount() {
//   // This does a low-level re-mount in fatfs, not through the zephyr API.
//   Zephyr
//   // still thinks it's mounted
//   FATFS *ff = (FATFS *)mount.fs_data;
//   memset(ff, 0, sizeof(FATFS));
//   FRESULT res = f_mount((FATFS *)mount.fs_data, &mount.mnt_point[1], 1);
//   if (res != FR_OK) {
//     LOG_ERR("Unable to mount filesystem [%d]", res);
//   } else {
//     LOG_INF("Filesystem mounted");
//   }
// }

/**
 * @brief Ensure filesystem is unmounted locally - this will be used by USB MSC
 * driver to prevent filesystem corruption
 */
// void ff_fs_unmount() {
//   // This does a low-level unmount in fatfs. Zephyr does not expose an umount
//   // method :(
//   // FATFS *ff = (FATFS *)mount.fs_data;
//   FRESULT res = f_mount(0, &mount.mnt_point[1], 0);
//   if (res != FR_OK) {
//     LOG_ERR("Unable to unmount filesystem [%d]", res);
//   } else {
//     // mount.mnt_point = NULL;
//     LOG_INF("Filesystem unmounted");
//   }
// }

SHELL_CMD_REGISTER(BURN, NULL, "DELETE FILE K", __cmd_rm);
// SHELL_CMD_REGISTER(DAB, NULL, "CHANGE WERKING DIRECTORY", __cmd_cd);
SHELL_CMD_REGISTER(FLOSS, NULL, "WATZ IN DA DIRECTORY?", __cmd_ls);
// SHELL_CMD_REGISTER(MAEK_DIR, NULL, "Create a directory", __cmd_mkdir);
// SHELL_CMD_REGISTER(WHERAMI, NULL, "PRINT WERKING DIRECTORY", __cmd_pwd);
SHELL_CMD_REGISTER(NOM, NULL, "READ FILE K", __cmd_read);
SHELL_CMD_REGISTER(PLAEC, NULL, "WRITE TO FILE K", __cmd_write);
SYS_INIT(ff_fs_init, APPLICATION, FF_FS_INIT_PRIORITY);