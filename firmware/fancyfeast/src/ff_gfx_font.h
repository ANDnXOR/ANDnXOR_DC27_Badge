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
#ifndef FF_GFX_FONT_H
#define FF_GFX_FONT_H

#define FF_GFX_FONT_CHAR_WIDTH 4

const static uint8_t font4x6 [96][2] = {
 {  0x00  ,  0x00  },   /*SPACE*/
 {  0x49  ,  0x08  },   /*'!'*/
 {  0xb4  ,  0x00  },   /*'"'*/
 {  0xbe  ,  0xf6  },   /*'#'*/
 {  0x7b  ,  0x7a  },   /*'$'*/
 {  0xa5  ,  0x94  },   /*'%'*/
 {  0x55  ,  0xb8  },   /*'&'*/
 {  0x48  ,  0x00  },   /*'''*/
 {  0x29  ,  0x44  },   /*'('*/
 {  0x44  ,  0x2a  },   /*')'*/
 {  0x15  ,  0xa0  },   /*'*'*/
 {  0x0b  ,  0x42  },   /*'+'*/
 {  0x00  ,  0x50  },   /*','*/
 {  0x03  ,  0x02  },   /*'-'*/
 {  0x00  ,  0x08  },   /*'.'*/
 {  0x25  ,  0x90  },   /*'/'*/
 {  0x76  ,  0xba  },   /*'0'*/
 {  0x59  ,  0x5c  },   /*'1'*/
 {  0xc5  ,  0x9e  },   /*'2'*/
 {  0xc5  ,  0x38  },   /*'3'*/
 {  0x92  ,  0xe6  },   /*'4'*/
 {  0xf3  ,  0x3a  },   /*'5'*/
 {  0x73  ,  0xba  },   /*'6'*/
 {  0xe5  ,  0x90  },   /*'7'*/
 {  0x77  ,  0xba  },   /*'8'*/
 {  0x77  ,  0x3a  },   /*'9'*/
 {  0x08  ,  0x40  },   /*':'*/
 {  0x08  ,  0x50  },   /*';'*/
 {  0x2a  ,  0x44  },   /*'<'*/
 {  0x1c  ,  0xe0  },   /*'='*/
 {  0x88  ,  0x52  },   /*'>'*/
 {  0xe5  ,  0x08  },   /*'?'*/
 {  0x56  ,  0x8e  },   /*'@'*/
 {  0x77  ,  0xb6  },   /*'A'*/
 {  0x77  ,  0xb8  },   /*'B'*/
 {  0x72  ,  0x8c  },   /*'C'*/
 {  0xd6  ,  0xba  },   /*'D'*/
 {  0x73  ,  0x9e  },   /*'E'*/
 {  0x73  ,  0x92  },   /*'F'*/
 {  0x72  ,  0xae  },   /*'G'*/
 {  0xb7  ,  0xb6  },   /*'H'*/
 {  0xe9  ,  0x5c  },   /*'I'*/
 {  0x64  ,  0xaa  },   /*'J'*/
 {  0xb7  ,  0xb4  },   /*'K'*/
 {  0x92  ,  0x9c  },   /*'L'*/
 {  0xbe  ,  0xb6  },   /*'M'*/
 {  0xd6  ,  0xb6  },   /*'N'*/
 {  0x56  ,  0xaa  },   /*'O'*/
 {  0xd7  ,  0x92  },   /*'P'*/
 {  0x76  ,  0xee  },   /*'Q'*/
 {  0x77  ,  0xb4  },   /*'R'*/
 {  0x71  ,  0x38  },   /*'S'*/
 {  0xe9  ,  0x48  },   /*'T'*/
 {  0xb6  ,  0xae  },   /*'U'*/
 {  0xb6  ,  0xaa  },   /*'V'*/
 {  0xb6  ,  0xf6  },   /*'W'*/
 {  0xb5  ,  0xb4  },   /*'X'*/
 {  0xb5  ,  0x48  },   /*'Y'*/
 {  0xe5  ,  0x9c  },   /*'Z'*/
 {  0x69  ,  0x4c  },   /*'['*/
 {  0x91  ,  0x24  },   /*'\'*/
 {  0x64  ,  0x2e  },   /*']'*/
 {  0x54  ,  0x00  },   /*'^'*/
 {  0x00  ,  0x1c  },   /*'_'*/
 {  0x44  ,  0x00  },   /*'`'*/
 {  0x0e  ,  0xae  },   /*'a'*/
 {  0x9a  ,  0xba  },   /*'b'*/
 {  0x0e  ,  0x8c  },   /*'c'*/
 {  0x2e  ,  0xae  },   /*'d'*/
 {  0x0e  ,  0xce  },   /*'e'*/
 {  0x56  ,  0xd0  },   /*'f'*/
 {  0x55  ,  0x3B  },   /*'g'*/
 {  0x93  ,  0xb4  },   /*'h'*/
 {  0x41  ,  0x44  },   /*'i'*/
 {  0x41  ,  0x51  },   /*'j'*/
 {  0x97  ,  0xb4  },   /*'k'*/
 {  0x49  ,  0x44  },   /*'l'*/
 {  0x17  ,  0xb6  },   /*'m'*/
 {  0x1a  ,  0xb6  },   /*'n'*/
 {  0x0a  ,  0xaa  },   /*'o'*/
 {  0xd6  ,  0xd3  },   /*'p'*/
 {  0x76  ,  0x67  },   /*'q'*/
 {  0x17  ,  0x90  },   /*'r'*/
 {  0x0f  ,  0x38  },   /*'s'*/
 {  0x9a  ,  0x8c  },   /*'t'*/
 {  0x16  ,  0xae  },   /*'u'*/
 {  0x16  ,  0xba  },   /*'v'*/
 {  0x16  ,  0xf6  },   /*'w'*/
 {  0x15  ,  0xb4  },   /*'x'*/
 {  0xb5  ,  0x2b  },   /*'y'*/
 {  0x1c  ,  0x5e  },   /*'z'*/
 {  0x6b  ,  0x4c  },   /*'{'*/
 {  0x49  ,  0x48  },   /*'|'*/
 {  0xc9  ,  0x5a  },   /*'}'*/
 {  0x54  ,  0x00  },   /*'~'*/
 {  0x56  ,  0xe2  }    /*''*/
};

#endif