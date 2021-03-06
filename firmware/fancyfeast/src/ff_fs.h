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

#ifndef FF_FS_H
#define FF_FS_H

#define FF_FS_INIT_PRIORITY 98
#define FF_FS_MOUNT_PATH "nffs"
#define FF_FS_MOUNT_POINT "/" FF_FS_MOUNT_PATH
#define FF_FS_MAX_PATH_LEN 128
#define FF_FS_WRITE_BUFFER_SIZE 64

/**
 * @brief Re-mount a file system after USB is done with it
 */
extern void ff_fs_remount();

/**
 * @brief Ensure filesystem is unmounted locally - this will be used by USB MSC
 * driver to prevent filesystem corruption
 */
extern void ff_fs_unmount();

#endif