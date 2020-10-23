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

#ifndef AT24C02_H
#define AT24C02_H

#define AT24C_SIZE 256
#define AT24C_PAGE_SIZE 8
#define AT24C_WRITE_CYCLE 5

/**
 * @brief Read a single byte from the eeprom, this can also be used to determine
 * if the eeprom is plugged into the badge
 * @param byte	Pointer to where to store the data
 * @return Negative if failure, zero if successful
 */
extern int at24c02_read(uint8_t *byte);

/**
 * @brief Read the entire EEPROM
 * @param p_buf		Buffer to store the entire eeprom contents in
 */
extern int at24c02_read_all(uint8_t *p_buf);

/**
 * @brief Read arbitrary byte from eeprom
 * @param p_buf		Buffer to put the bytes into
 * @param offset	Offset of the address to start at
 * @return Negative value if error, 0 if okay
 */
extern int at24c02_read_byte(uint8_t *p_buf, uint8_t offset);

/**
 * @brief Write a byte to the EEPROM. This function will block until write is
 * complete
 * @param addr		8-bit address to write to
 * @param value		The value to write to that address
 */
extern int at24c02_write_byte(uint8_t addr, uint8_t value);


/**
 * @brief Write a buffer of bytes to the EEPROM
 * @param addr    Register address to write to
 * @param p_buf   Buffer to write
 * @param len     Size of that buffer
 * @param return  Negative value if error
 */
extern int at24c02_write_bytes(uint8_t addr, uint8_t *p_buf, size_t len);

#endif