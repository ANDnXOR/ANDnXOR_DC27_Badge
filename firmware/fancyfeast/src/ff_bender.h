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
#ifndef FF_BENDER_H
#define FF_BENDER_H

/**
 * @brief Initialize the B.E.N.D.E.R. console
 */
extern void ff_bender_init();

#define LOG_LEVEL 4

#define CONSOLE_PHONE_NUM "805-203-6888"

//Yep, doesn't add to 100. That will be a fun thing to find out.
#define PERCENTAGE_H_HACK 15
#define PERCENTAGE_N_HACK 15
#define PERCENTAGE_S_HACK 15
#define PERCENTAGE_E_HACK 15
#define PERCENTAGE_W_HACK 15
#define PERCENTAGE_H_EGG 4
#define PERCENTAGE_N_EGG 4
#define PERCENTAGE_S_EGG 4
#define PERCENTAGE_E_EGG 4
#define PERCENTAGE_W_EGG 4

#define HOME_AREA_ITEM "BOTTLE_OPENER"						// ITEM 0
#define HOME_AREA_BEER "BALLAST_POINT_SOUR_WENCH"			// BEER 0
#define HOME_AREA_WINZ "NOTHING"							// WINZ 0 - There is no item to pickup once the game is over
#define NORTH_AREA_ITEM "FLOPPY_DISK"						// ITEM 1
#define NORTH_AREA_BEER "KEYSTONE_LIGHT"					// BEER 1
#define NORTH_AREA_WINZ "USB_CONDOM"						// WINZ 1
#define SOUTH_AREA_ITEM "LOCK_PICK"							// ITEM 2
#define SOUTH_AREA_BEER "SOUTHERN_STAR_CONSPIRACY_THEORY"	// BEER 2
#define SOUTH_AREA_WINZ "PAPER_CLIP"						// WINZ 2
#define EAST_AREA_ITEM "TAPE_PUNCH"							// ITEM 3
#define EAST_AREA_BEER "CASCADE_BOURBONIC_PLAGUE_2016"		// BEER 3
#define EAST_AREA_WINZ "WIRE"								// WINZ 3
#define WEST_AREA_ITEM "GRAIN_OF_RICE"						// ITEM 4
#define WEST_AREA_BEER "MODERN_TIMES_FRUITLANDS"			// BEER 4
#define WEST_AREA_WINZ "STARSAN"							// WINZ 4
#define BEER_0_ABV 0.070 
#define BEER_1_ABV 0.041
#define BEER_2_ABV 0.071
#define BEER_3_ABV 0.105
#define BEER_4_ABV 0.048

typedef struct{ 
    char name[33];
    bool haz;
	bool booze;
	uint8_t location;
	float booze_volume;
	float booze_ABV;
}item_t;

typedef struct{
	uint8_t location;
	uint8_t attempts[5];
	int weight;
	float BAC;
	int first_drink;
	char gender;
	bool location_solved[2][5]; //ROW 0=MainPuzzle, 1=EasterEgg, COL = Locations
	item_t l00t[3][5]; //ROW 0=Freebie Item, 1=Awarded Item, 2=B00Z3, COL = Locations
	uint8_t percentage_complete;
	bool su;
}bender_data_t;

/**
 * @brief Generate a BENDER score as a bitmask
 */
extern uint16_t ff_bender_score_get();

#endif