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
#include <zephyr.h>
#include <stdlib.h>
#include <stdio.h>
#include <device.h>
#include <gpio.h>
#include <uart.h>
#include <shell/shell.h>
#include <posix/time.h>
#include <logging/log.h>
#include "ff_settings.h"
#include "ff_bender.h"
#include "ff_bling.h"
#include "ff_util.h"
#include "ff_time.h"
#include "ff_unlocks.h"
#include "bling/ff_bling_eyes.h"
#include "bling/ff_bling_flappy.h"

LOG_MODULE_REGISTER(ff_bender);
static int error_count = 0;

void __gfx_area_banner(const struct shell *shell, int i);
void __gfx_benchoff(const struct shell *shell);
void __gfx_moon(const struct shell *shell);
void __gfx_floppy_disk(const struct shell *shell);
void __gfx_alcohol_poisoning(const struct shell *shell);
void __gfx_game_over(const struct shell *shell);
void __gfx_believe(const struct shell *shell);
void __gfx_internet(const struct shell *shell);
void __gfx_cyphercon_badge(const struct shell *shell);
void __gfx_game_win(const struct shell *shell);

bender_data_t* ff_bender_ptr_get() {
  return ff_settings_bender_ptr_get();
}

/**
 * @brief Initialize the B.E.N.D.E.R. console
 */
void ff_bender_init() {
  bender_data_t* p_state = ff_bender_ptr_get();
  
  //Game is 0% completed
  p_state->percentage_complete = 0;

  // Start at home
  p_state->location = 0;  

  //Challenge Attempts
  for (int i = 0; i < 5; i++)
    p_state->attempts[i] = 0;

  //Initialze Items & Location Data
  for (int i = 0; i <= 2; i++) {
    for (int j = 0; j <= 4; j++) {
      p_state->l00t[i][j].haz = false;
      p_state->l00t[i][j].booze = false;
      p_state->l00t[i][j].booze_volume = 0;
      p_state->l00t[i][j].location = j;
      if(i<=1) p_state->location_solved[i][j] = false;
    }
  }

  //Initialze b00z3 (only row 2 are drinks)
  for (int i = 0; i <= 4; i++) {
    p_state->l00t[2][i].booze = true;
    p_state->l00t[2][i].booze_volume = 22.0;
  }
  p_state->l00t[2][0].booze_ABV = BEER_0_ABV;
  p_state->l00t[2][1].booze_ABV = BEER_1_ABV;
  p_state->l00t[2][2].booze_ABV = BEER_2_ABV;
  p_state->l00t[2][3].booze_ABV = BEER_3_ABV;
  p_state->l00t[2][4].booze_ABV = BEER_4_ABV;

  //Initialize BAC
  p_state->BAC = 0.0;
  p_state->first_drink = 0; //EPOCH TIME

  //Initialize l00t descriptions
  strcpy(p_state->l00t[0][0].name, HOME_AREA_ITEM);
  strcpy(p_state->l00t[0][1].name, NORTH_AREA_ITEM);
  strcpy(p_state->l00t[0][2].name, SOUTH_AREA_ITEM);  
  strcpy(p_state->l00t[0][3].name, EAST_AREA_ITEM);
  strcpy(p_state->l00t[0][4].name, WEST_AREA_ITEM);
  strcpy(p_state->l00t[1][0].name, HOME_AREA_WINZ);
  strcpy(p_state->l00t[1][1].name, NORTH_AREA_WINZ);
  strcpy(p_state->l00t[1][2].name, SOUTH_AREA_WINZ);
  strcpy(p_state->l00t[1][3].name, EAST_AREA_WINZ);
  strcpy(p_state->l00t[1][4].name, WEST_AREA_WINZ);
  strcpy(p_state->l00t[2][0].name, HOME_AREA_BEER);
  strcpy(p_state->l00t[2][1].name, NORTH_AREA_BEER);
  strcpy(p_state->l00t[2][2].name, SOUTH_AREA_BEER);
  strcpy(p_state->l00t[2][3].name, EAST_AREA_BEER);
  strcpy(p_state->l00t[2][4].name, WEST_AREA_BEER);

  //You are a normal user
  p_state->su = false;

  //Gender not set yet
  p_state->gender = 'N';

  //Weight not set yet
  p_state->weight = 0;
}

/**
 * @brief Generate a BENDER score as a bitmask
 */
uint16_t ff_bender_score_get() {
  uint16_t score = 0;
  bender_data_t data = ff_settings_ptr_get()->bender_state;
  for (uint8_t loc=0; loc<5; loc++) {
    bool mp = data.location_solved[0][loc];
    bool ee = data.location_solved[1][loc];

    //Mask main puzzle
    if (mp) {
      score += (1 << (2*loc));
    }
    //Mask easter egg
    if (ee) {
      score += (1 << ((2*loc)+1));
    }
  }
  return score;
}

/**
 * @brief B.E.N.D.E.R. Function to convert a single hexadecimal charachter to binary
 */
void __htob(char h, uint8_t *b){
  switch(h){
    case '0' : strcpy(b,"0000"); break;
    case '1' : strcpy(b,"0001"); break;
    case '2' : strcpy(b,"0010"); break;
    case '3' : strcpy(b,"0011"); break;
    case '4' : strcpy(b,"0100"); break;
    case '5' : strcpy(b,"0101"); break;
    case '6' : strcpy(b,"0110"); break;
    case '7' : strcpy(b,"0111"); break;
    case '8' : strcpy(b,"1000"); break;
    case '9' : strcpy(b,"1001"); break;
    case 'A' : strcpy(b,"1010"); break;
    case 'B' : strcpy(b,"1011"); break;
    case 'C' : strcpy(b,"1100"); break;
    case 'D' : strcpy(b,"1101"); break;
    case 'E' : strcpy(b,"1110"); break;
    case 'F' : strcpy(b,"1111"); break;
    case 'a' : strcpy(b,"1010"); break;
    case 'b' : strcpy(b,"1011"); break;
    case 'c' : strcpy(b,"1100"); break;
    case 'd' : strcpy(b,"1101"); break;
    case 'e' : strcpy(b,"1110"); break;
    case 'f' : strcpy(b,"1111"); break;
    default : break;
  }
}

/**
 * @brief B.E.N.D.E.R. test command
 * @param shell		Pointer to shell context
 * @param argc		Number of command arguments
 * @param argv		Pointer to array of command arguments
 * @return 0 if successful
 */

static int __cmd_help(const struct shell *shell, size_t argc, char **argv) {
  shell_print(shell,
  "  GOTO R HACKADAI PAEG AT https://hackaday.io/project/164346-andxor-dc27-badge\n"
  "  4 COMMAND LINE INTERFACE EXPLANASHUN AN HIT \"TAB\" BUTTUN AT ANY TIEM 4 LIST OV ALL COMMANDZ\n");
  return 0;
}
// Create a root-level command
SHELL_CMD_REGISTER(HELP, NULL, "Provide general help to the user", __cmd_help);

//Returns the value of your Blood Alchol Content
float get_BAC() {
  bender_data_t* p_state = ff_bender_ptr_get();
  double time_h = 0;
  struct timespec now = ff_time_now_get();
  int difference = now.tv_sec - p_state->first_drink;
  double final_BAC = p_state->BAC;

  if(p_state->first_drink!=0){//Convert seconds to hours
    time_h = (double)difference/3600;
  }
  
  if (difference > 0){
    if(final_BAC - (time_h * 0.015) > 0)
      return final_BAC - (time_h * 0.015); //Normal case for BAC calculation
    else
      return 0; //This prevents your BAC from going into the negatives
  }
  else
    return final_BAC; //odd case where time loss causes you to have drank in the future
}

//Sets the value of your Blood Alchol Content
void set_BAC(float oz, float abv) {
  bender_data_t* p_state = ff_bender_ptr_get();
  struct timespec now = ff_time_now_get();
  float GENDER_CONSTANT;

  if (p_state->gender == 'M')
    GENDER_CONSTANT = 0.73;
  else if (p_state->gender == 'F')
    GENDER_CONSTANT = 0.66;
  else
    GENDER_CONSTANT = 0.695;  // Non-Binary Gender Determination BAC constant, am I the only one who thinks about these things?

  //Initialize Drinking Variables incase you sobered up or its your first time
  if(get_BAC() == 0){
    p_state->BAC = 0; //get_BAC being 0 doesnt imply BAC is actually 0, since get_BAC incorporates time degredation
    p_state->first_drink = now.tv_sec; //reset the drinking time 
  }

  // Update BAC
  p_state->BAC += (oz * abv * 5.14) / (p_state->weight * GENDER_CONSTANT);
}

/**
 * @brief B.E.N.D.E.R. drink command
 * @param shell		Pointer to shell context
 * @param argc		Number of command arguments
 * @param argv		Pointer to array of command arguments
 * @return 0 if successful
 */

static int __cmd_drink(const struct shell *shell, size_t argc, char **argv) {
  bender_data_t* p_state = ff_bender_ptr_get();
  bool error_flag = false;

  if (p_state->gender == 'N') {
    shell_print(shell,"  You haven't set your gender yet...\n");
    return 0;
  }

  if (p_state->weight == 0) {
    shell_print(shell,"  You haven't set your weight yet...\n");
    return 0;
  }

  float booze_vol = strtol(argv[2], NULL,10);

  if (p_state->l00t[0][0].haz == true) { //Do you have a bottle opener
    if (argc == 3) {
      for (int i = 0; i <= 4; i++) {  // You can only drink a beer if you have it, there is enough volume, the volume is positive, and the beer actually exists
        if((strcmp(argv[1], p_state->l00t[2][i].name) == 0) && (p_state->l00t[2][i].booze_volume >= booze_vol) && 
          (booze_vol > 0) && (p_state->l00t[2][i].haz)) {
        
          // Calculate your new BAC
          set_BAC(booze_vol, p_state->l00t[2][i].booze_ABV);

          // Subtract b00z3 you drank from the bottle
          p_state->l00t[2][i].booze_volume -= booze_vol;

          // Display Results
          shell_print(shell,"  You down some delicious craft b33r");
          shell_print(shell,"  Your BAC is now %.4f\n", get_BAC());
          ff_settings_save();
          error_flag = false;
          break;
        } else error_flag = true;
      }
    } else error_flag = true;

    if (error_flag) {
      shell_print(shell, "  That doesn't make sense...you can't drink that \n\n");
    }
    
    // Kill player if they drink too much b00z3
    if (get_BAC() >= 0.1350) {
      shell_print(shell, "  You drank too much b00z3 and die of dysentery...err...alcohol poisoning\n\n");
      shell_print(shell, "  GAME OVER!\n");
      __gfx_alcohol_poisoning(shell);
      ff_bender_init();
      ff_settings_save();
    }
  } 
  else
    shell_print(shell, "  You can't open the bottle without a bottle opener...\n");

  return 0;
}

// Create a root-level command
SHELL_CMD_REGISTER(DRINK, NULL, "usage $drink <BOOZ3>\n", __cmd_drink);

/**
 * @brief B.E.N.D.E.R. bender status sub command command
 * @param shell		Pointer to shell context
 * @param argc		Number of command arguments
 * @param argv		Pointer to array of command arguments
 * @return 0 if successful
 */

static int __cmd_bender_status(const struct shell *shell, size_t argc, char **argv) {
  bender_data_t* p_state = ff_bender_ptr_get();
  settings_t* p_settings = ff_settings_ptr_get();

  shell_print(shell,"  Name: %s",p_settings->name);

  if(p_state->gender == 'N')
    shell_print(shell,"  Gender: Not Set");
  else
    shell_print(shell,"  Gender: %c", p_state->gender);

  if(p_state->weight == 0)
    shell_print(shell,"  Weight: Not Set");
  else
    shell_print(shell,"  Weight: %d lbs", p_state->weight);

  shell_print(shell,"  Blood Alcohol Content: %.4f",get_BAC());
  
  shell_print(shell,"  B.E.N.D.E.R. Completion: %i%%",p_state->percentage_complete);
  
  switch(p_state->location){
    case 0: shell_print(shell,"  Location: Home Village"); break;
    case 1: shell_print(shell,"  Location: North Village"); break;
    case 2: shell_print(shell,"  Location: South Village"); break;
    case 3: shell_print(shell,"  Location: East Village"); break;
    case 4: shell_print(shell,"  Location: West Village"); break;
  }
  
  shell_print(shell,"\n  L00T: LIST OF YER HOARDINGS");
  shell_print(shell,"  ---------------------------");

  //You always have a FINGER
  shell_print(shell,"  FINGER");

  //Print Items You Have Acquired
  for (int i = 0; i <= 2; i++) {
    for (int j = 0; j <= 4; j++) {
      if((p_state->l00t[i][j].haz == true) && (i < 2))
        shell_print(shell, "  %s",p_state->l00t[i][j].name);
      else if ((p_state->l00t[i][j].haz == true) && (i == 2))
        shell_print(shell, "  %s (%.1f oz)",p_state->l00t[i][j].name,p_state->l00t[i][j].booze_volume);
    }
  }
  shell_print(shell,"");
  return 0;
}

static int __cmd_bender_weight(const struct shell *shell, size_t argc, char **argv) {
  bender_data_t* p_state = ff_bender_ptr_get();
 
  // Change the players weight
  if (p_state->weight != 0) {
    shell_print(shell,"  You already changed your weight. Deal with it or restart the game.\n");
  } 
  else if ((argc == 2) && (strtol(argv[1], NULL, 10) >= 100) && (strtol(argv[1], NULL, 10) <= 400)) {
    p_state->weight = (int) strtol(argv[1], NULL, 10);
    shell_print(shell,"  Weight change successful!\n");
    ff_settings_save();
  } else {
    shell_print(shell,"  Enter a valid weight between 100-400 lbs.");  
    shell_print(shell,"  e.g. $ bender weight 200\n");  
  }
  return 0;
}

static int __cmd_bender_gender(const struct shell *shell, size_t argc, char **argv) {
  bender_data_t* p_state = ff_bender_ptr_get();
  bool error_flag = false;

  if (p_state->gender != 'N') {
    shell_print(shell,"  You already changed your gender. Deal with it or restart the game.\n");
  } else if (argc == 2) {
    if ((strcmp(argv[1], "M") == 0) || (strcmp(argv[1], "m") == 0)) {
      p_state->gender = 'M';
      shell_print(shell, "  Sex change successful: Man-Bot!\n");
      ff_settings_save();
    } else if ((strcmp(argv[1], "F") == 0) || (strcmp(argv[1], "f") == 0)) {
      p_state->gender = 'F';
      shell_print(shell,"  Sex change successful: Fem-Bot!\n");
      ff_settings_save();
    } else if ((strcmp(argv[1], "X") == 0) || (strcmp(argv[1], "x") == 0)) {
      p_state->gender = 'X';
      shell_print(shell,"  Sex change successful: NonBinary-Bot!\n");
      ff_settings_save();
    } else
      error_flag = true;
  } else
    error_flag = true;

  if (error_flag) {
    shell_print(shell, "  That doesn't make sense...you must be gender drunk.\n");
  }
  
  return 0;
}

static int __cmd_bender_recap(const struct shell *shell, size_t argc, char **argv) {
  __gfx_alcohol_poisoning(shell);
  shell_print(shell,"\n"
    "  What? Did you get black out drunk and forget about what happened? Shit alright...\n"
    "  Last year you woke up in the middle of the desert, no idea how you got there (big surprise).\n"
    "  Some weird ass server rack was there, robot locked up inside, no power.\n"
    "  You hacked some puzzles, got drunk, ended up in a Radio Shack, got drunk...\n"
    "  Went into a Saloon, got drunk, ended up dancing in a with some creepy WiFi cactus dude.\n"
    "  Messed with a TacoCorp Taco Truck, got drunk, got power up and running to the server rack.\n"
    "  Robot came out and yelled at you, you ate a bunch of Tacos, got super drunk, and blew the robot.\n"
    "  Fell into a hole in the ground, made your way through an underground bunker, then opened a door...\n"
    "  Soundes like a good time. Oh and TacoCorp is probably not happy with you.\n"
  );

  shell_print(shell,
    "  So what happened after that door opened?\n"
    "  You slipped on a taco wrapper and tripped over a pizza box left by some lazy asshole pizza delivery man.\n"
    "  Spinning around backwards, then falling into some windowed freezer thing...fuck it was cold.\n"
    "  The world got dark...you slept...and then awoke. You must have been in some cryogenic sleep for a long time.\n"
    "  Exiting your sleepy freezer bed you stretch and yawn many years of nasty taco morning breath.\n"
    "  You're not in the same place as before either. Someone moved that cold coffin while you were knocked out.\n"
    "  There are stairs going up, you ascend them to a solid metal door and uncrank a door locking mechanism...\n"
    "  (You should look around at this point)\n"
  );
  return 0;
}

static int __cmd_bender_reset(const struct shell *shell, size_t argc, char **argv) {
  shell_print(shell,"  Resetting badge hacking challenge...\n");
  ff_bender_init();
  ff_settings_save();
  return 0;
}

static int __cmd_bender_commands(const struct shell *shell, size_t argc, char **argv) {
  shell_print(shell,"  Commands Specific to the B.E.N.D.E.R. Hacking Challenge:");
  shell_print(shell,"  BENDER, LOOK, HACK, DRINK, STEAL, WALK, N, S, E, W\n");
  return 0;
}

// Create a list of sub commands(for completion) - note each has its own handler
SHELL_STATIC_SUBCMD_SET_CREATE(sub_bender,
    SHELL_CMD(STATUS, NULL, "Status of your progress", __cmd_bender_status),
    SHELL_CMD(WEIGHT, NULL, "Set your weight (100-400lbs)", __cmd_bender_weight),
    SHELL_CMD(GENDER, NULL, "Set your gender (M,F,X)", __cmd_bender_gender),
    SHELL_CMD(RECAP, NULL, "Recap since DC26 - Run This First!", __cmd_bender_recap),
    SHELL_CMD(RESET, NULL, "Reset the challenge (B.E.N.D.E.R. specific save data)", __cmd_bender_reset),
    SHELL_CMD(COMMANDS, NULL, "Commands specific to B.E.N.D.E.R.\n", __cmd_bender_commands), SHELL_SUBCMD_SET_END);
// Create root command to host the sub commands - note sub_walk and it has no handler
SHELL_CMD_REGISTER(BENDER, &sub_bender, "Badge Enabled Non Directive Enigma Routine (B.E.N.D.E.R) 2.0\n", NULL);

/**
 * @brief B.E.N.D.E.R. steal command
 * @param shell		Pointer to shell context
 * @param argc		Number of command arguments
 * @param argv		Pointer to array of command arguments
 * @return 0 if successful
 */

static int __cmd_steal(const struct shell *shell, size_t argc, char **argv) {
  bender_data_t* p_state = ff_bender_ptr_get();
  bool error_flag = false;
 
  if (argc == 2) {
    if ((strcmp(argv[1], p_state->l00t[0][p_state->location].name) == 0) && (p_state->l00t[0][p_state->location].haz == false)) {
      //freebie item && you dont have it 
      p_state->l00t[0][p_state->location].haz = true; // now you have it
      ff_settings_save();
      shell_print(shell, "  You stole a %s\n", p_state->l00t[0][p_state->location].name);
    } 
    else if ((strcmp(argv[1], p_state->l00t[0][p_state->location].name) == 0) && (p_state->l00t[0][p_state->location].haz == true)) {
      //freebie item && you do have it
      shell_print(shell, "  You already stole that...you must be drunk...\n");
    }
    else if ((strcmp(argv[1], p_state->l00t[2][p_state->location].name) == 0) && (p_state->l00t[2][p_state->location].haz == false)) {
      //freebie beer && you dont have it
      p_state->l00t[2][p_state->location].haz = true; // now you have it
      ff_settings_save();
      shell_print(shell, "  You stole a %s\n", p_state->l00t[2][p_state->location].name);
    } 
    else if ((strcmp(argv[1], p_state->l00t[2][p_state->location].name) == 0) && (p_state->l00t[2][p_state->location].haz == true)) {
      //freebie beer && you do have it
      shell_print(shell, "  You already stole that...you must be drunk...\n");
    }
    else if ((strcmp(argv[1], p_state->l00t[1][p_state->location].name) == 0) && (p_state->l00t[1][p_state->location].haz == false) && 
              (p_state->location_solved[0][p_state->location] == true)) {
      //awarded item && you dont have it
      p_state->l00t[1][p_state->location].haz = true; // now you have it
      ff_settings_save();
      shell_print(shell, "  You stole a %s\n", p_state->l00t[1][p_state->location].name);
    }
    else if ((strcmp(argv[1], p_state->l00t[1][p_state->location].name) == 0) && (p_state->l00t[1][p_state->location].haz == true) && 
              (p_state->location_solved[0][p_state->location] == true)) {
      //awarded item && you do have it
      shell_print(shell, "  You already stole that...you must be drunk...\n");
    }
    else error_flag = true;
  }
  else error_flag = true;

  if (error_flag) {shell_print(shell,"  That doesn't make sense...you can't steal that\n");}

  return 0;
}

// Create a root-level command
SHELL_CMD_REGISTER(STEAL, NULL, "usage $steal <ITEM_IN_CAPS>\n", __cmd_steal);

/**
 * @brief B.E.N.D.E.R. flush command
 * @param shell		Pointer to shell context
 * @param argc		Number of command arguments
 * @param argv		Pointer to array of command arguments
 * @return 0 if successful
 */

static int __cmd_flush(const struct shell *shell, size_t argc, char **argv) {
  bender_data_t* p_state = ff_bender_ptr_get();

  char* temp_pw = "CYBERPATHOGENS";

  if(p_state->su == true){
    if(p_state->location == 1){
      shell_print(shell,"  Your super 1337 badge computer executes a flush.");
      shell_print(shell,"  Almost everything goes down the hole, except a few remaining letters: %s", temp_pw);
      shell_print(shell,"  Something must be clogged as everything begins to flow back and fill up the bowl.\n");
    }
    else{
      shell_print(shell,"  Your super 1337 badge computer executes a remote flush.");
      shell_print(shell,"  But you aren't in the bathroom to see what happens...\n");
    }
  }
  else
    shell_print(shell,"  Permission denied\n");

  return 0;
}
// Create a root-level command
SHELL_CMD_REGISTER(FLUSH, NULL, "FLUSH BUFFERZ DOWN DA HOLEZ\n", __cmd_flush);

/**
 * @brief B.E.N.D.E.R. EKOZ command
 * @param shell		Pointer to shell context
 * @param argc		Number of command arguments
 * @param argv		Pointer to array of command arguments
 * @return 0 if successful
 */

static int __cmd_ekoz(const struct shell *shell, size_t argc, char **argv) {
  /*
  The purpose of this is to fake a buffer overflow for the purposes of puzzle privlege escalation.
  Some of the commands will have protections from being executed unless p_state->su == true.
  Traditional buffer overflows can be used to do fun things, like execute other commands with privelged access.
  This is due to the fact that sometimes programs will have system call commands assigned to adjoining heap variables.

  The logic works as follows: Build a string out of all of the argument variables.
  Chop the string at the buffer maximum (buf_max) in addition to counting how many arguments occour until the max (temp_argc).
  Once we hit max, create a new_argv char pointer array where 0 is the buffermax+1 to the first space.
  Since we tracked the number of arguments, the rest of the new_argv should reference the existing original argv pointers.
  Finally, reference new_argv[0] against the list of all known commands. 
  If new_argv[0] = a valid command, execute it with p_state->su as TRUE, else dump some BS buffer overflow message.
  Once the logic returns, de-escalate privlages of su to FALSE.
  */
  bender_data_t* p_state = ff_bender_ptr_get();
  uint8_t temp_argc = 2; //start at 2 to offset the initial ekoz and pre-overflow argv's
  uint8_t buf_max = 24; //The fake soft limit of ekoz, so over 24 chars causes an overflow
  char temp_str[128] = "";


  //Build a string out of the arguments
  for(int i=1; i < argc; i++){
    //Break this loop if we are going to exceed temp_str buffer length, avoid an ACTUAL buffer overflow
    if(strlen(temp_str)+strlen(argv[i])>=128)
      break;

    strcat(temp_str, argv[i]);
    if(i!=argc-1)
      strcat(temp_str," "); //dont add an extra space at the end
    if(strlen(temp_str)<=buf_max)
      temp_argc++; //save this so we know how many args before we passed buffer limit
    else if(argc == 2)
      temp_argc = 1; //this is the case where there are only 2 args, allign to array reference
  }

  if(strlen(temp_str)<=buf_max) 
    shell_print(shell, "  %s\n", temp_str); //echo like normal...nothing to see here
  else{
    //Now we fake a buffer overflow. Why fake it? Cuz I dont want hackers actually crashing the badge...YOLO
    char temp_sub_str[strlen(temp_str)-buf_max+1];
    memcpy(temp_sub_str, &temp_str[buf_max], strlen(temp_str)-buf_max);
    temp_sub_str[strlen(temp_str)-buf_max] = '\0';

    //Make sure ekoz isn't recursively being called
    if((temp_sub_str[0] == 'E')&&(temp_sub_str[1] == 'K')&&(temp_sub_str[2] == 'O')&&(temp_sub_str[3] == 'Z')&&(strlen(temp_sub_str)>=4)){
      shell_print(shell,"  Recursively call EKOZ...gonna have a bad time. Dont do that.\n");
      return 0;
    }

    //enable SU status temporarily when executing the command
    p_state->su = true;  
    shell_print(shell,"  With great power comes great responsibility.");

    //Call the command
    shell_execute_cmd(shell, temp_sub_str);

    //remove SU status once the use of the function ends
    p_state->su = false;   
  }
  return 0;
}

// Create a root-level command
SHELL_CMD_REGISTER(EKOZ, NULL, "REPEAT WUT U SEZ UMTIL 24 CHARZ\n", __cmd_ekoz);

static void error_msg_look(const struct shell *shell){
  switch(error_count){
    case 0: shell_print(shell, "  That's not something you can look at...drunky Mc Drunk face...\n");error_count++;break;
    case 1: shell_print(shell, "  You can't look at that, because....Aliens...\n");error_count++;break;
    case 2: shell_print(shell, "  There are no stupid look commands, just dumb fuckers at the console.\n");error_count++;break;
    case 3: shell_print(shell, "  You must have salt in your eyes...dont put salt in your eyes...put salt in your eyes!\n");error_count=0;break;
  }
}

/**
 * @brief B.E.N.D.E.R. hack command
 * @param shell		Pointer to shell context
 * @param argc		Number of command arguments
 * @param argv		Pointer to array of command arguments
 * @return 0 if successful
 */

static int __cmd_hack(const struct shell *shell, size_t argc, char **argv) {
  bender_data_t* p_state = ff_bender_ptr_get();

  bool error_flag = false;

  if ((argc == 4) && (strcmp(argv[2], "WITH") == 0)) {
    switch (p_state->location) { 
      case 0: {  // HOME AREA HACKS
        //Audio puzzle unlock
        if ((strcmp(argv[1], "DJ_COMPUTER") == 0) && (strcmp(argv[3], "BAYJFKKL") == 0)){
          if (!FF_UNLOCK_VALIDATE(FF_UNLOCK_AUDIO_PUZZLE)) {
            shell_print(shell,"The computer whirrs and beeps, the Sound Blaster card lets out a puff of smoke then starts playing\n"
              "your favorite tune through a pair of logitech speakers. Never gonna give you up....\n"
              "Audio puzzle complete, please do not share.\n");
            FF_UNLOCK_SET(FF_UNLOCK_AUDIO_PUZZLE);
          } else {
            shell_print(shell, "Herr derr only once my friend.\n");
          }
        } 
        //Unlock flappy bird
        else if ((strcmp(argv[1], "DJ_COMPUTER") == 0) && (strcmp(argv[3], "BUCGLB") == 0)){
          if (!FF_UNLOCK_VALIDATE(FF_UNLOCK_FLAPPY)) {
            shell_print(shell,"The computer whirrs and beeps, the screen lights up with a sweet EGA color palette. A bird and some\n"
            "pipes come on the ancient CRT.\n"
              "Flappy Pixel unlocked, please do not share.\n");
            FF_UNLOCK_SET(FF_UNLOCK_FLAPPY);
            FF_BLING_DEFAULT_FLAPPY(bling);
            ff_bling_mode_register(bling, ff_bling_handler_flappy);
            ff_bling_mode_push(bling);
          } else {
            shell_print(shell, "Herr derr only once my friend.\n");
          }
        } 
        //Unlock eyes bling
        else if ((strcmp(argv[1], "DJ_COMPUTER") == 0) && (strcmp(argv[3], "PCGNKJ") == 0)){
          if (!FF_UNLOCK_VALIDATE(FF_UNLOCK_EYES)) {
            shell_print(shell,"The computer whirrs and beeps, you feel as though something is staring at you.\n"
              "Eyes Bling mode unlocked, please do not share.\n");
            FF_UNLOCK_SET(FF_UNLOCK_EYES);
            FF_BLING_DEFAULT_EYES(bling);
            ff_bling_mode_register(bling, ff_bling_handler_eyes);
            ff_bling_mode_push(bling);
          } else {
            shell_print(shell, "Herr derr only once my friend.\n");
          }
        } 
        else if ((strcmp(argv[1], "BREATHALYZER") == 0) && (strcmp(argv[3], "BREATH") == 0) 
        &&(p_state->l00t[1][1].haz == true)&&(p_state->l00t[1][2].haz == true)&&(p_state->l00t[1][3].haz == true)&&(p_state->l00t[1][4].haz == true)
        &&(p_state->l00t[2][1].haz == true)&&(p_state->l00t[2][2].haz == true)&&(p_state->l00t[2][3].haz == true)&&(p_state->l00t[2][4].haz == true)
        &&(p_state->l00t[2][0].haz == true)&&(p_state->l00t[2][0].haz == true)&&(p_state->location_solved[0][0] == false)){

          // check if all 5 beers were drank
          bool drank_all_the_b00z3 = true;
          for (int i = 0; i <= 4; i++) {
            if (p_state->l00t[2][i].booze_volume == 22.0)
              drank_all_the_b00z3 = false;
          }

          if (drank_all_the_b00z3) {
            // truncate the float to 4 decimal places & save as a string its not 100% elegant but just as messy as any other method to
            // "chop" significant digits on a float
            char temp_BAC[7];
            snprintf(temp_BAC, 7, "%.4f", get_BAC()); 

            if (strcmp("0.1337", temp_BAC) == 0) {
              __gfx_game_win(shell);
              shell_print(shell, "  YOUR BREATH...YOUR STANKY ASS BREATH...HAS A BAC IS 1.337 AND THE FINAL CHALLENGE IS SOLVED."); 
              shell_print(shell, "  A BRIGHT LIGHT BEAMS FROM THE NIGHT SKY, SOMETHING DARTING ACROSS VERY QUICKLY."); 
              shell_print(shell, "  THE RADIO TOWER IS NOW BATHED IN BLINDING LIGHT AND YOU CANT SEE."); 
              shell_print(shell, "  TO BE CONTINUED...\n");
              shell_print(shell, "  YOU HAVE COMPLETED THE AND!XOR DC27 BADGE CHALLENGE. TYPE BENDER STATUS AND SCREENSHOT IT.");
              shell_print(shell, "  CONTACT US ON TWITTER @ANDNXOR && EMAIL HYR0N@ANDNXOR.COM ASAP BEFORE THE CON IS OVER.");
              shell_print(shell, "  YOUR STATE WILL SAVE ON A TASK THAT RUNS EVERY DAY, YOU MAY SAFELY DISCONNECT THE BADGE IN 24 HOURS...\n");
              // Set the completion flag
              shell_print(shell,"  Home Village Hack: %i%% Completion Earned\n",PERCENTAGE_H_HACK);
              p_state->location_solved[0][0]=true;
              p_state->percentage_complete+=PERCENTAGE_H_HACK;
              ff_settings_save();
            } else {
              shell_print(shell,"  Your BAC is %.4f", get_BAC());
              shell_print(shell,"  If its too low, keep drinking...");
              shell_print(shell,"  If its too high, wait it out...");
              shell_print(shell,"  Regardless, you suck at this game!\n");
            }
          } else {
            shell_print(shell,"  You have to drink from EVERY craft beer! Don't suck at this and follow instructions!\n");
          }
        } 
        else
          error_flag = true;
      } break;

      case 1: {  // NORTH AREA HACKS
        if ((strcmp(argv[1], "GLORYHOLE") == 0) && (strcmp(argv[3], "CYBERPATHOGENS") == 0) && (p_state->location_solved[0][1]==false)){
          shell_print(shell,"  After yelling the password you hear a faint reply...");
          shell_print(shell,"  CONDOM...COMBO...IM...A...TEAPOT...\n");

        } 
        else if ((strcmp(argv[1], "GLORYHOLE") == 0) && (strcmp(argv[3], "DICKBUTT") == 0)&& (p_state->location_solved[1][1]==false)){
          shell_print(shell,"  Easter Egg Hack: %i%% Completion Earned\n",PERCENTAGE_N_EGG);
          p_state->location_solved[1][1]=true;
          p_state->percentage_complete+=PERCENTAGE_N_EGG;
          ff_settings_save();
        } 
        else if ((strcmp(argv[1], "COMBO_SPINNER") == 0) && (strcmp(argv[3], "418") == 0) && (p_state->location_solved[0][1]==false)){
          //Area puzzle has been complete, change state variables and increment completion percentage
          shell_print(shell,"  The combination works, the door to the vending panel swings open, and some new l00t drops on the floor.");
          shell_print(shell,"  North Village Hack: %i%% Completion Earned\n",PERCENTAGE_N_HACK);
          p_state->location_solved[0][1]=true;
          p_state->percentage_complete+=PERCENTAGE_N_HACK;
          ff_settings_save();
        }
        else if ((strcmp(argv[1], "COMBO_SPINNER") == 0) && (strcmp(argv[3], "418") != 0) && (p_state->location_solved[0][1]==false)){
          //Fuck people for trying to brute force, let's lie to them and make them think the combo changes
          shell_print(shell,"  The combination does not work.");
          shell_print(shell,"  You hear a rattle inside, the combo must have changed.");
          shell_print(shell,"  Brute force or honest mistakes just won't do the trick.");
          p_state->attempts[p_state->location]++;
          if(p_state->attempts[p_state->location] <= 3){
            shell_print(shell,"  Attempts Remaining: %d\n",3-p_state->attempts[p_state->location]);
            ff_settings_save();
          }
          else{
            __gfx_game_over(shell);
            ff_bender_init();
            ff_settings_save();
          }
        }
        else
          error_flag = true;
      } break;

      case 2: {  // SOUTH AREA HACKS    
        if ((strcmp(argv[1], "PUNCH_TAPE") == 0)&&(strcmp(argv[3], "TAPE_PUNCH") == 0)&&(p_state->l00t[0][3].haz == true)&&(p_state->location_solved[1][2]==false)){
          __gfx_cyphercon_badge(shell);
          shell_print(shell,"  Easter Egg Hack: %i%% Completion Earned\n",PERCENTAGE_S_EGG);
          p_state->location_solved[1][2]=true;
          p_state->percentage_complete+=PERCENTAGE_S_EGG;
          ff_settings_save();
          shell_print(shell,"  Look at you not following directions. Just blindly punching tape for the shit of it.");
          shell_print(shell,"  Marching to the beat of your own drum...That's worth some points.\n");
        }   
        else if ((strcmp(argv[1], "PUNCH_TAPE") == 0)&&(strcmp(argv[3], "TAPE_PUNCH") == 0)&&(p_state->l00t[0][3].haz == true)&&(p_state->location_solved[1][2]==true)){
          break;
        }   
        else if ((strcmp(argv[1], "PUNCH_TAPE") == 0)&&(p_state->l00t[0][3].haz == false)) {
          shell_print(shell,"  You're not going to do much punching without a TAPE_PUNCH\n");
        }
        else if ((strcmp(argv[1], "PUNCH_TAPE") == 0)&&(p_state->l00t[0][3].haz == true)&&(p_state->location_solved[0][2]==false)) { 
          //This is used to abstract the argv reference from when I move code from test to actual hack command
          int p = 3;

          //Validate Input - Check length ensuring 8 bits x 2 precision for hex = length of 16
          if(strlen(argv[p])!=16){
            shell_print(shell, "  Error: Patch is not the required length (16) - Current length (%d)\n",strlen(argv[p]));
            return 0;
          }

          //Validate Input - Check for non hex inputs
          bool not_hex = false;
          for(int i=0; i<16; i++){
            if(((argv[p][i] >= 0)&&(argv[p][i] <= 47))||((argv[p][i] >= 58)&&(argv[p][i] <= 64))||((argv[p][i] >= 71)&&(argv[p][i] <= 96))||((argv[p][i] >= 103))){
              //if value of the character is outside the bounds of (0..9) OR (A..F) OR (a..f)
              not_hex = true;
              shell_print(shell,"  Error: Patch contains non-hexadecimal character: %c",argv[p][i]);
            }
          }
          if (not_hex){shell_print(shell,"");return 0;}

          //Convert each pair of the argv hex patch to a multidimensional array of binary
          char b_patch[8][9];
          char b_temp[5];

          for(int i=0; i<8; i++){
            __htob(argv[p][i*2],b_temp);
            strcpy(b_patch[i],b_temp);
            __htob(argv[p][i*2+1],b_temp);
            strcat(b_patch[i],b_temp);
          }

          //Create the version of the current master tape
          char master_bad[] = "49CE495420B00D8C";
          char m_patch[8][9];
          for(int i=0; i<8; i++){
            __htob(master_bad[i*2],b_temp);
            strcpy(m_patch[i],b_temp);
            __htob(master_bad[i*2+1],b_temp);
            strcat(m_patch[i],b_temp);
          }

          //Print out an ASCII version of the patch tape
          shell_print(shell,"  You carefully punch your patch on a blank tape strip and submit it in to the PNEUMATIC_TUBE_SYSTEM.");
          shell_print(shell,"  SHOOOMF! (Thats the sound pneumatic vacuum tubes make)\n");
          for(int i=0; i<8; i++){
            if(i==0) shell_print(shell,"  -^-^-^-^-^-^-");
            for(int j=0; j<8; j++){
              if(j==0) shell_fprintf(shell,SHELL_NORMAL,"  | "); //Print padding from the tape edge
              if(j==5) shell_fprintf(shell,SHELL_NORMAL,"*"); //Print the clock notch
              if(b_patch[i][j]=='0') shell_fprintf(shell,SHELL_NORMAL,"."); //Print a dash to signify no punch
              else shell_fprintf(shell,SHELL_NORMAL,"O"); //Print the hole punch
              if(j==7) shell_print(shell," |");
            }
            if(i==7) shell_print(shell,"  -^-^-^-^-^-^-\n");
          }

          //Print out an ASCII version of the patched master tape
          shell_print(shell,"  Patiently you wait and a copy of the patched master tape is returned to you...\n");
          
          //Lazy Man's XOR -> cuz why convert to real binary if i have to convert it back to char just for printing? Too much typing...
          for(int i=0; i<8; i++){
            for(int j=0; j<8; j++){
              if(m_patch[i][j] == b_patch[i][j]) m_patch[i][j] = '0';
              else m_patch[i][j] = '1';
            }
          }

          for(int i=0; i<8; i++){
            if(i==0) shell_print(shell,"  -^-^-^-^-^-^-");
            for(int j=0; j<8; j++){
              if(j==0) shell_fprintf(shell,SHELL_NORMAL,"  | "); //Print padding from the tape edge
              if(j==5) shell_fprintf(shell,SHELL_NORMAL,"*"); //Print the clock notch
              if(m_patch[i][j]=='0') shell_fprintf(shell,SHELL_NORMAL,"."); //Print a dash to signify no punch
              else shell_fprintf(shell,SHELL_NORMAL,"O"); //Print the hole punch
              if(j==7) shell_print(shell," |");
            }
            if(i==7) shell_print(shell,"  -^-^-^-^-^-^-\n");
          }

          //Check if it matches the solution - 8080808080858086
          if (strcmp(argv[p], "8080808080858086") == 0){
            //Area puzzle has been complete, change state variables and increment completion percentage
            shell_print(shell,"  The patch works! This machine which is the TACONET comes back to life!.");
            shell_print(shell,"  Also a taxidermied squirrel appears wedged in the tube now...who sent that through?");
            shell_print(shell,"  New l00t awaits you as well, for all your ancient hackery.");
            shell_print(shell,"  South Village Hack: %i%% Completion Earned\n",PERCENTAGE_S_HACK);
            p_state->location_solved[0][2]=true;
            p_state->percentage_complete+=PERCENTAGE_S_HACK;
            ff_settings_save();
          }
          else if (strcmp(argv[p], "8080808080858086") != 0){
            //Fuck people for trying to brute force
            shell_print(shell,"  The patch does not work.");
            shell_print(shell,"  Additionally you hear something inside of the PNEUMATIC_TUBE_SYSTEM...it sounds like Wire giggling?");
            p_state->attempts[p_state->location]++;
            if(p_state->attempts[p_state->location] <= 3){
              shell_print(shell,"  Attempts Remaining: %d\n",3-p_state->attempts[p_state->location]);
              ff_settings_save();
            }
            else{
              __gfx_game_over(shell);
              ff_bender_init();
              ff_settings_save();
            }
          }
        }
        else
          error_flag = true; 
      } break;

      case 3: {  // EAST AREA HACKS
        if ((strcmp(argv[1], "RESET_BUTTON") == 0) && (strcmp(argv[3], "FINGER") == 0) && (p_state->location_solved[0][3]==false)){
          //You press the button and have not solved this puzzle yet
          struct device *tpm_uart = device_get_binding("UART_1"); //Setup the TPM on the back of the badge
          
          if (!tpm_uart) {
              LOG_ERR("Cannot bind to device UART_1 in BENDER implant puzzle.");;
              return 0;
          }

          static const char *msg_01 = "TPM=1_"; //Default Setting TPM=1 means Encryption Enabled
          static const char *msg_02 = "274=1_"; //SPECIMIN_TACO pod LEDS in octal (look at the ANSI art)
          static const char *msg_03 = "AX3$7_"; //Fake encrypted tail of the message traffic
          unsigned char recvChar; 
          char buffer[6];
          int recv_count;
          bool malformed_data_flag = false;

          //Send A Message 1 - The TPM Header
          for (int i = 0; i < strlen(msg_01); i++) {
            uart_poll_out(tpm_uart, msg_01[i]);
          }

          //Recieve A Message - Check TPM header
          recv_count = 0;
          while (1) {
            while (uart_poll_in(tpm_uart, &recvChar) < 0);
            buffer[recv_count]=recvChar;
            recv_count++;
            if (recvChar == '_') {break;}
          }
          
          //Make a substring to get rid of any garbage at the end of the buffer
          char sub_buffer[recv_count+1];
          memcpy(sub_buffer,buffer,recv_count);
          sub_buffer[recv_count]='\0';

          if(strcmp(sub_buffer, "TPM=1_") == 0){ //TPM Encryption Enabled
            //Send "Encrypted" Garbage
            for (int i = 0; i < strlen(msg_03); i++) {
              uart_poll_out(tpm_uart, msg_03[i]);
            }
            //Recieve A Message - Encrypted Garbage
            recv_count = 0;
            while (1) {
              while (uart_poll_in(tpm_uart, &recvChar) < 0);
              buffer[recv_count]=recvChar;
              recv_count++;
              if (recvChar == '_') {break;}
            }
            //Make a substring chop any random memory crap at the end of the buffer
            memcpy(sub_buffer,buffer,recv_count);
            sub_buffer[recv_count]='\0';

            if (strcmp(sub_buffer, "AX3$7_") == 0){
              //Steady state - nothing has changed from the default message
              shell_print(shell, "  The computer resets. Nothing happens.\n");
            }
            else {
              malformed_data_flag = true;
            }
            
          }
          else if (strcmp(sub_buffer, "TPM=0_") == 0){ //TPM Encryption Disabled
            //Send A Message 2
            for (int i = 0; i < strlen(msg_02); i++) {
              uart_poll_out(tpm_uart, msg_02[i]);
            }
            //Recieve A Message
            recv_count = 0;
            while (1) {
              while (uart_poll_in(tpm_uart, &recvChar) < 0);
              buffer[recv_count]=recvChar;
              recv_count++;
              if (recvChar == '_') {break;}
            }

            //Make a substring to get rid of any garbage at the end of the buffer
            memcpy(sub_buffer,buffer,recv_count);
            sub_buffer[recv_count]='\0';

            //Evaluate the decrypted and possibly / hopefully modified TPM message
            if(strcmp(sub_buffer, "274=1_") == 0){ //274=1 Then nothing happens (steady state)
              shell_print(shell, "  The computer resets. Nothing happens.\n");
            }
            else if(strcmp(sub_buffer, "274=0_") == 0){ //274=0 Then Giant Taco defrosts, breaks out, and kills you
              shell_print(shell, "  The computer resets. Something happens...");
              shell_print(shell, "  The LICKWID_SALSA_NYETROJEN begins to spray out the back of the SPECIMIN_TACO pod.");
              shell_print(shell, "  It takes a while, spraying chunky salsa everywhere. You grab chips and eat it while waiting.");
              shell_print(shell, "  You see the giant taco begin to move...the pod opens and the taco leaps out.");
              shell_print(shell, "  Standing there in shock and awe, you are no challenge for the giant taco.");
              shell_print(shell, "  It's giant tasty corn tortilla shell latches on to you...and eats you.\n");
              __gfx_game_over(shell);
              ff_bender_init();
              ff_settings_save();
            }
            else if(strcmp(sub_buffer, "457=1_") == 0){ //457=1 Pod freezes and kills Fitz
              shell_print(shell, "  The computer resets. Something happens...");
              shell_print(shell, "  The LICKWID_SALSA_NYETROJEN begins to fill the HOOMAHN_FITZY pod.");
              shell_print(shell, "  The chill wakes Joe up and he begins slamming the pod window to get out.");
              shell_print(shell, "  He then picks up a NINKASI_SLEIGHR and tries to break the window.");
              shell_print(shell, "  But eventually stops to just drink it...NINKASI...Oregon people...good people.");
              shell_print(shell, "  The freeze eventually wins and Joe stops moving.");
              shell_print(shell, "  He's dead but appears to have died happy, laughing at one of his own jokes.");
              shell_print(shell, "  A tear comes to your eye reflecting on what you just did.");
              shell_print(shell, "  Pressed up against the pod glass, your tear freezes your face to it.");
              shell_print(shell, "  You're stuck and eventually your entire head turns into a HOOMAHN ice cube.\n");
              __gfx_game_over(shell);
              ff_bender_init();
              ff_settings_save();
            }
            else if(strcmp(sub_buffer, "457=0_") == 0){ //457=0 Fitz is free, you win
              shell_print(shell, "  The computer resets. Something happens...");
              shell_print(shell, "  The HOOMAHN_FITZY pod opens and Joe is released.");
              shell_print(shell, "  High fives for celebration but he doesn't want to stick around. Would you?");
              shell_print(shell, "  Before he leaves he tapes up a photo of himself in the pod window to trick his captors...");
              shell_print(shell, "  Why does he keep life size photos of his face? Dunno but it works out.");
              shell_print(shell, "  Joe tosses a care package for your troubles on the ground and then bolts.");
              //Area puzzle has been complete, change state variables and increment completion percentage
              shell_print(shell, "  East Village Hack: %i%% Completion Earned\n",PERCENTAGE_E_HACK);
              p_state->location_solved[0][3]=true;
              p_state->percentage_complete+=PERCENTAGE_E_HACK;
              ff_settings_save();
            }
            else{ //User fucked up and put unexpected data. Punish them.
              malformed_data_flag = true;
            }
          }
          else{
            malformed_data_flag = true;
          }

          if(malformed_data_flag){
            shell_print(shell,"  The computer resets. Something happens...");
            shell_print(shell,"  Whatever you did, it was wrong. A capacitor pops and magic smoke fills the air.");
            shell_print(shell,"  There are a few more redundant components, but not many...");
            p_state->attempts[p_state->location]++;
            if(p_state->attempts[p_state->location] <= 3){
              shell_print(shell,"  Attempts Remaining: %d\n",3-p_state->attempts[p_state->location]);
              ff_settings_save();
            }
            else{
              __gfx_game_over(shell);
              ff_bender_init();
              ff_settings_save();
            }
          }
        }
        else if ((strcmp(argv[1], "RESET_BUTTON") == 0) && (strcmp(argv[3], "FINGER") == 0) && (p_state->location_solved[0][3]==true)){
          //You press the button and have solved this puzzle
          shell_print(shell,"  No point, the Fitz is free. Go home, you're drunk.\n");
        }
        else if ((strcmp(argv[1], "CIRCUIT_BOARD") == 0) && (strcmp(argv[3], "GRAIN_OF_RICE") == 0) && (p_state->location_solved[1][3]==false) &&
          (p_state->l00t[0][4].haz == true)){
          __gfx_believe(shell);
          shell_print(shell,"  Easter Egg Hack: %i%% Completion Earned\n",PERCENTAGE_E_EGG);
          p_state->location_solved[1][3]=true;
          p_state->percentage_complete+=PERCENTAGE_E_EGG;
          ff_settings_save();
          shell_print(shell,"  Slipping the tiny grain of rice on to the circuit board easily looks like a hardware implant.");
          shell_print(shell,"  Functionally...it does nothing...Might as well publish a story about it.\n");
        }
        else
          error_flag = true;
      } break;

      case 4: {  // WEST AREA HACKS
        if ((strcmp(argv[1], "FLOPPY_DRIVE") == 0) && (strcmp(argv[3], "FLOPPY_DISK") == 0) && (p_state->location_solved[1][4]==false)
          && (p_state->l00t[0][1].haz == true)){
          //You have the floppy DISK from the north area [freebie item = 0][location north = 1] && haven't done this yet [1][4] = false
          __gfx_floppy_disk(shell);
          shell_print(shell,"  Easter Egg Hack: %i%% Completion Earned\n",PERCENTAGE_W_EGG);
          p_state->location_solved[1][4]=true;
          p_state->percentage_complete+=PERCENTAGE_W_EGG;
          ff_settings_save();
          shell_print(shell,"  The console begins to boot from the FLOPPY_DISK and MONITOR_1 comes to life.\n");
        } 
        else if ((strcmp(argv[1], "KEYBOARD") == 0) && (p_state->location_solved[1][4]==false) && (p_state->location_solved[0][4]==false)){ //No Floppy Disk Inserted Yet
          shell_print(shell,"  The console is hung. It needs to boot first.\n");
        }
        else if ((strcmp(argv[1], "KEYBOARD") == 0) && (p_state->location_solved[1][4]==true) && (p_state->location_solved[0][4]==false)){ //Floppy Disk Inserted 
          //Reminder to figure out your salted hash....
          //SYSTUM CEREAL
          //AWESUM the output of the above with 0x removed 

          //Get the badge serial
          uint32_t a0 = NRF_FICR->DEVICEID[0];
          uint32_t a1 = NRF_FICR->DEVICEID[1];
          char input_badge_serial[18];
          char output_badge_serial_hash[16];
          char temp_str[2];
          char output_badge_serial_hash_string[32];
          sprintf(input_badge_serial, "%08x%08x", a0, a1); //This means the hash is based on the ID without "0x" prepended to it
                   
          //Get the salted hash of the badge serial id
          ff_util_md5_salted(input_badge_serial, strlen(input_badge_serial), output_badge_serial_hash);

          //Clear the hash string memory space of goblins
          memset(output_badge_serial_hash_string,0,32);

          //Build the hash string from the 16 byte hex array
          for (uint8_t i = 0; i < 16; i++) {
            sprintf(temp_str, "%02x", output_badge_serial_hash[i]);
            strcat(output_badge_serial_hash_string,temp_str);
          }

          if ((strcmp(argv[3], output_badge_serial_hash_string) != 0) && (p_state->location_solved[0][4]==false)){
            //They typed the wrong thing
            shell_print(shell,"  Error Invalid License Key.");
            p_state->attempts[p_state->location]++;
            if(p_state->attempts[p_state->location] <= 3){
              shell_print(shell,"  Attempts Remaining Before Format Initiated: %d\n",3-p_state->attempts[p_state->location]);
              ff_settings_save();
            }
            else{
              __gfx_game_over(shell);
              ff_bender_init();
              ff_settings_save();
            }
          }
          else if ((strcmp(argv[3], output_badge_serial_hash_string) == 0) && (p_state->location_solved[0][4]==false)){
            //They typed the right thing
            shell_print(shell,"  License Accepted!");
            //Area puzzle has been complete, change state variables and increment completion percentage
            shell_print(shell,"  The brewery comes to life and the DRAWER slides open with a little reward as well.");
            shell_print(shell,"  West Village Hack: %i%% Completion Earned\n",PERCENTAGE_W_HACK);
            p_state->location_solved[0][4]=true;
            p_state->percentage_complete+=PERCENTAGE_W_HACK;
            ff_settings_save();
          }
        }
        else if ((strcmp(argv[1], "KEYBOARD") == 0) && (p_state->location_solved[0][4]==true)){
          //You already defeated this area challenge but still try fucking with the keyboard
          shell_print(shell,"  No touch keyboard! Beer is finally brewing!\n");
        }
        else
          error_flag = true;
      } break;
    }
  } else
    error_flag = true;

  if (error_flag) {
    shell_print(shell,"  Am I doing it right? NO! You're not doing it right...\n");
  }

  return 0;
}
// Create a root-level command
SHELL_CMD_REGISTER(HACK, NULL, "all the things\n", __cmd_hack);

/**
 * @brief B.E.N.D.E.R. look command
 * @param shell		Pointer to shell context
 * @param argc		Number of command arguments
 * @param argv		Pointer to array of command arguments
 * @return 0 if successful
 */

static int __cmd_look(const struct shell *shell, size_t argc, char **argv) {
  bender_data_t* p_state = ff_bender_ptr_get();
  bool error_flag = false;
   
  switch (argc) {
    case 1: // displays general information about the environment to the user - user typed "look"
      switch (p_state->location) {
        
        case 0:  // HOME - PUZZLE REQUIRING BEER & AUDIO DECODER
          if (p_state->location_solved[0][0] == false) {
            shell_print(shell, "  You enter the doorway and it opens to a dusty radio broadcast room.");
            shell_print(shell, "  Lets call this HOME for now...");
            shell_print(shell, "  A relic of the past, but nostalgia holds a place in your heart.");
            shell_print(shell, "  There's a DJ_COMPUTER on a desk, powered up, with a prompt.");
            shell_print(shell, "  A pile of mixtapes, discs, and a STICKY_NOTE on a piece of media are in the corner.");
            shell_print(shell, "  You peer out the window and to your surprise the world has gone to shit. Like nuclear winter fallout shit.");
            shell_print(shell, "  They did it. They finally did it. DEF CON was cancelled and look what happened.");
            shell_print(shell, "  From this radio tower lookout you can see two intersecting roads going in to the city.");
            shell_print(shell, "  They are perpendicular creating four possible directions: NORTH, SOUTH, EAST, and WEST.");
            shell_print(shell, "  Looking back away from the window and back in the room you notice something SOMETHING_FAMILIAR...\n");
          } 
          else {
            __gfx_game_win(shell);
            shell_print(shell, "  TO BE CONTINUED...\n");
            shell_print(shell, "  YOU HAVE COMPLETED THE AND!XOR DC27 BADGE CHALLENGE. TYPE BENDER STATUS AND SCREENSHOT IT.");
            shell_print(shell, "  CONTACT US ON TWITTER @ANDNXOR && EMAIL HYR0N@ANDNXOR.COM ASAP BEFORE THE CON IS OVER.");
            shell_print(shell, "  YOUR STATE WILL SAVE ON A TASK THAT RUNS EVERY DAY, YOU MAY SAFELY DISCONNECT THE BADGE IN 24 HOURS...\n");
          }
          //Item display - There is no "if you win this main puzzle" item because it's game over at that point
          if(p_state->l00t[0][p_state->location].haz == false){
            shell_print(shell, "  An item catches your eye for the taking: %s", p_state->l00t[0][p_state->location].name);
          }
          if(p_state->l00t[2][p_state->location].haz == false){
            shell_print(shell, "  You smell some booze for the drinking: %s", p_state->l00t[2][p_state->location].name);
          }

          shell_print(shell,"");
        break;

        case 1:  // NORTH - PUZZLE (BUFFER OVERFLOW)
          shell_print(shell, "  You follow the path through empty streets, windows are boarded up, no one in sight.");
          shell_print(shell, "  However in the distance you notice a building, which is still open?");
          shell_print(shell, "  The path dead ends at a public restroom, a silohoute of a shadow burned in the wall.");
          shell_print(shell, "  Inside is a pretty typical restroom, stalls of shitters and pissers...");
          shell_print(shell, "  Someone has conveniently labeled them with a smudgey brown marker...how nice!");
          shell_print(shell, "  Lets call them MR_URINAL, SHITTER_STALL_1, SHITTER_STALL_2, and SHITTER_STALL_3");
          if (p_state->location_solved[0][p_state->location] == false) {  
            // Haven't beat the puzzle yet
            shell_print(shell, "  A CONDOM_VENDING_MACHINE is on the wall, looks like it requires a quarter.\n");
          }
          else {
            // Standard context displayed once the puzzle has been beaten
            shell_print(shell, "  A CONDOM_VENDING_MACHINE is on the wall, it is empty now.\n");
          }
          //Item display
          if(p_state->l00t[0][p_state->location].haz == false){
            shell_print(shell, "  An item catches your eye for the taking: %s", p_state->l00t[0][p_state->location].name);
          }
          if((p_state->l00t[1][p_state->location].haz == false) && (p_state->location_solved[0][p_state->location] == true)){
            shell_print(shell, "  An new item catches your eye for the taking: %s", p_state->l00t[1][p_state->location].name);
          }
          if(p_state->l00t[2][p_state->location].haz == false){
            shell_print(shell, "  You smell some booze for the drinking: %s", p_state->l00t[2][p_state->location].name);
          }
          shell_print(shell,"");
        break;

        case 2:  // SOUTH - PUZZLE REQUIRING KNOWLEGE OF PUNCHTAPES (We Heart The TyMkrs!)
          shell_print(shell, "  You find yourself entering a large utility warehouse...");
          shell_print(shell, "  As you enter you notice...tubes. Like, tubes everywhere.");
          shell_print(shell, "  It appears you found the TACONET. The rumors were true.");
          shell_print(shell, "  However there are different types of tubes, in particular you notice a PNEUMATIC_TUBE_SYSTEM.");
          shell_print(shell, "  You also notice a reel of paper PUNCH_TAPE hanging from one of the tubes.");
          if (p_state->location_solved[0][p_state->location] == false) {  
            // Haven't beat the puzzle yet
            shell_print(shell, "  The PNEUMATIC_TUBE_SYSTEM appears to be working.");
            shell_print(shell, "  However the main control screen of the TACONET shows a seg fault. Fuuuuuuck");
            shell_print(shell, "  On the floor is a binder title RTFM.");
            shell_print(shell, "  Damn, DEF CON being cancelled ruined everything.\n");
          }
          else {
            // Standard context displayed once the puzzle has been beaten
            shell_print(shell, "  The PNEUMATIC_TUBE_SYSTEM appears to be no longer working, jammed with a squirrel.");
            shell_print(shell, "  However the TACONET is now alive and well with cat memes, bot-nets, Rick Rollz, and infosec drama.");
            shell_print(shell, "  You pat yourself on the back for making the world a better place...\n");
          }
          //Item display
          if(p_state->l00t[0][p_state->location].haz == false){
            shell_print(shell, "  An item catches your eye for the taking: %s", p_state->l00t[0][p_state->location].name);
          }
          if((p_state->l00t[1][p_state->location].haz == false) && (p_state->location_solved[0][p_state->location] == true)){
            shell_print(shell, "  An new item catches your eye for the taking: %s", p_state->l00t[1][p_state->location].name);
          }
          if(p_state->l00t[2][p_state->location].haz == false){
            shell_print(shell, "  You smell some booze for the drinking: %s", p_state->l00t[2][p_state->location].name);
          }
          shell_print(shell,"");
        break;

        case 3:  // EAST - PUZZLE REQUIRING HARDWARE IMPLANT (We Heart Fitzy & Grains of Rice)
          shell_print(shell, "  You find yourself entering in to TacoCorp labs...old taco nemesis...");
          shell_print(shell, "  Weird. There is a giant tank of LICKWID_SALSA_NYETROJEN...Future idiots can't spell.");
          shell_print(shell, "  It appears to be hooked to a computer and doing something with two cryogenic pods.");
          shell_print(shell, "  Actually they look like the same sleepy freezer pod you came out of back at HOME.");
          shell_print(shell, "  They are both hooked up to some COMPUTER with an exposed CIRCUIT_BOARD.");
          shell_print(shell, "  One of the pods has a gigantic taco in stasis...SPECIMIN_TACO.");
          if (p_state->location_solved[0][p_state->location] == false) {  
            // Haven't beat the puzzle yet
            shell_print(shell, "  The other pod has some other specimin...HOOMAHN_FITZY.\n");
          }
          else {
            // Standard context displayed once the puzzle has been beaten
            shell_print(shell, "  The other pod has the foto of Fitz in stasis...maybe they won't notice.\n");
          }
          //Item display
          if(p_state->l00t[0][p_state->location].haz == false){
            shell_print(shell, "  An item catches your eye for the taking: %s", p_state->l00t[0][p_state->location].name);
          }
          if((p_state->l00t[1][p_state->location].haz == false) && (p_state->location_solved[0][p_state->location] == true)){
            shell_print(shell, "  An new item catches your eye for the taking: %s", p_state->l00t[1][p_state->location].name);
          }
          if(p_state->l00t[2][p_state->location].haz == false){
            shell_print(shell, "  You smell some booze for the drinking: %s", p_state->l00t[2][p_state->location].name);
          }
          shell_print(shell,"");
        break;

        case 4:  // WEST - PUZZLE REQUIRING REVERSE ENGINEERING OF BINARY (GHIDRA)
          shell_print(shell, "  You find yourself walking up to a working craft brewery! #HBS!");
          shell_print(shell, "  It's good to know b00z3 is alive and well.");
          shell_print(shell, "  Inside you see a few CONICALS, working their fermenty magic.");
          shell_print(shell, "  Pipes and lines run from them, through program logic controllers.");
          shell_print(shell, "  In the middle of the room is some industrial CONTROL_BENCH.");
          shell_print(shell, "  Everything seems integrated to this magic binary device.");
          if (p_state->location_solved[0][p_state->location] == false) {  
            // Haven't beat the puzzle yet
            shell_print(shell, "  However the brew system appears to be offline, how sad!\n");
          }
          else {
            // Standard context displayed once the puzzle has been beaten
            shell_print(shell, "  All is well in the world and chugging along, the CONICALS recieving some sparge love.\n");
          }
          //Item display
          if(p_state->l00t[0][p_state->location].haz == false){
            shell_print(shell, "  An item catches your eye for the taking: %s", p_state->l00t[0][p_state->location].name);
          }
          if((p_state->l00t[1][p_state->location].haz == false) && (p_state->location_solved[0][p_state->location] == true)){
            shell_print(shell, "  An new item catches your eye for the taking: %s", p_state->l00t[1][p_state->location].name);
          }
          if(p_state->l00t[2][p_state->location].haz == false){
            shell_print(shell, "  You smell some booze for the drinking: %s", p_state->l00t[2][p_state->location].name);
          }
          shell_print(shell,"");
        break;
      }
    break; //end arcg(1) check for "look" by itself

    // displays details information about items you are looking at up close,
    // switch filtered by location - user typed "look at THING"
    case 3:
      if (strcmp(argv[1], "AT") == 0) {  // make sure they at least type "at" and not some garbage
        switch (p_state->location) {
          case 0: // Home Items
          if (strcmp(argv[2], "SOMETHING_FAMILIAR") == 0) {
            if((p_state->l00t[1][1].haz == false)||(p_state->l00t[1][2].haz == false)||(p_state->l00t[1][3].haz == false)||(p_state->l00t[1][4].haz == false)
             ||(p_state->l00t[2][1].haz == false)||(p_state->l00t[2][2].haz == false)||(p_state->l00t[2][3].haz == false)||(p_state->l00t[2][4].haz == false)){
              shell_print(shell, "  It looks like that old metal bending robot BREATHALYZER.");
              shell_print(shell, "  However the years have not been kind to him.");
              shell_print(shell, "  There was some kind power upgrade but no one routinely changed his super battery system.");
              shell_print(shell, "  The batteries have corroded over the years and covered him in alkaline chunks.");
              shell_print(shell, "  In general, he needs a lot of work. Come back once you have all the challenge l00t.\n");
            }
            else{//You have all winz items - USB_CONDOM, PAPER_CLIP, LOCK_PICK, STARSAN
              shell_print(shell, "  Intuition takes over, you quickly douse him with STAR_SAN and the alkaline dissolves.");
              shell_print(shell, "  Next you open his locked belly casing with a LOCK_PICK, leveraging the PAPER_CLIP as a tension bar.");
              shell_print(shell, "  You pull out his internal 10ft USB cable and bring it to the DJ_COMPUTER for power...");
              shell_print(shell, "  But you're not that dumb. NO! Why trust DJ Dead's computer?");
              shell_print(shell, "  Always use protection, which is why that USB_CONDOM does just the trick for safe power.");
              shell_print(shell, "  The robot comes back to life, extends his BREATHALYZER, and projects instructions on the wall:\n"); 
              shell_print(shell, "  TO PROCEED YOU MUST ABIDE BY THE FOLLOWING RULES:");
              shell_print(shell, "  * HAVE A BLOOD ALCOHOL LEVEL OF EXACTLY 0.1337, ACCURATE TO 4 DECIMAL PLACES");
              shell_print(shell, "  * DRINK FROM ALL FIVE CRAFT BEERS COLLECTED");
              shell_print(shell, "  * %% BAC = ((A x 5.14) \\ (W x R)) – 0.015 x H");
              shell_print(shell, "  * A = TOTAL LIQUID OUNCES OF ALCOHOL CONSUMED = ABV * VOLUME OF ALCOHOL DRANK");
              shell_print(shell, "  * W = PERSONS WEIGHT IN POUNDS");
              shell_print(shell, "  * R = GENDER CONSTANT OF ALCOHOL DISTRIBUTION (0.73 FOR MALE, 0.66 FOR FEMALE, 0.695 FOR NON-BINARY)");
              shell_print(shell, "  * H = HOURS ELAPSED SINCE YOUR FIRST DRINK (HOPE YOU DIDNT START THE PARTY EARLY)");
              shell_print(shell, "  * CLI = HACK BREATHALYZER WITH BREATH\n");
            }
          } 
          else if (strcmp(argv[2], "DJ_COMPUTER") == 0) {
            shell_print(shell, "  ENTER UNLOCK CODE> \n");
          } 
          else if (strcmp(argv[2], "STICKY_NOTE") == 0) {
            shell_print(shell, "  A reminder from a DJ Dead to throw together a MIXTAPE.\n");
          } 
          else if (strcmp(argv[2], "MIXTAPE") == 0) {
            shell_print(shell, "  It's hard to read, the LABEL is very faded.\n");
          } 
          else if (strcmp(argv[2], "LABEL") == 0) {
            shell_print(shell, "  Almost half peeled off but you can see some ENGRAVING from what was written.\n");
          }           
          else if (strcmp(argv[2], "ENGRAVING") == 0) {
            shell_print(shell, "  Tilted in the light the engraving shows http://bit.ly/2HB1Ggg\n");
            if(p_state->location_solved[1][0]==false){
              shell_print(shell,"  Easter Egg Hack: %i%% Completion Earned\n",PERCENTAGE_H_EGG);
              p_state->location_solved[1][0]=true;
              p_state->percentage_complete+=PERCENTAGE_H_EGG;
              ff_settings_save();
            }
          }           
          else
            error_flag = true;
          break;

          case 1: // North Items
          if (strcmp(argv[2], "MR_URINAL") == 0) {
            __gfx_benchoff(shell);
            shell_print(shell, "  Nothing special here, just a porcelian wall piece with a Brian Benchoff Urinal Cake.");
            shell_print(shell, "  Make sure to aim between the eyes to avoid splashback...\n");
          } 
          else if (strcmp(argv[2], "SHITTER_STALL_1") == 0) {
            shell_print(shell, "  A nuclear winter toilet stall. Looks like someone printed Hackaday Articles to read!");
            shell_print(shell, "  Wait...no...they're all shit post articles by Brian Benchoff to be used as toilet paper.\n");
          }
          else if (strcmp(argv[2], "SHITTER_STALL_2") == 0) {
            shell_print(shell, "  A nuclear winter toilet stall. Nothing here but a bunch of bathroom stall TAGGING.\n");
          }
          else if (strcmp(argv[2], "SHITTER_STALL_3") == 0) {
            shell_print(shell, "  A nuclear winter toilet stall...with a GLORYHOLE in the wall...uhh...");
            shell_print(shell, "  A message on the stall is written in sharpie: SPEEK PAZWORD IN 2 GLORYHOLE.");
            shell_print(shell, "  It looks likes someone has been here recently, and they drank too much and thew up...");
            shell_print(shell, "  Yep. Puked up Keystone Light and alphabet soup in that toilet, you need a buffer from this grossness");
            shell_print(shell, "  There's so many letters swirling around to the brim, that the toilet is about to overflow.");
            shell_print(shell, "  This is super gross, you gag and the sound you make ekoz throughout the restroom.");
            shell_print(shell, "  You need to flush this if you plan to be in here any longer.\n");
          }
          else if (strcmp(argv[2], "GLORYHOLE") == 0) {
            shell_print(shell, "  Bravely...you put your face in front of the hole...and listen...all ambient noises seem to echo inside.\n");
          }
          else if (strcmp(argv[2], "CONDOM_VENDING_MACHINE") == 0) {
            shell_print(shell, "  Always play it safe, even in the apocalpyse! The dispense knob wont turn, it requires a quarter.");
            shell_print(shell, "  However gum is jammed in the coin slot (we hope its gum).");
            shell_print(shell, "  There's also a 3 wheel COMBO_SPINNER for maintenance.\n");
          }
          else if (strcmp(argv[2], "TAGGING") == 0) {
            __gfx_moon(shell);
            shell_print(shell, "  4 GUD TIEM CALL %s",CONSOLE_PHONE_NUM);
            shell_print(shell, "  YELL NAYM IN GLORYHOLE\n");
          } else
            error_flag = true;
          break;

          case 2: // South Items
          if (strcmp(argv[2], "PNEUMATIC_TUBE_SYSTEM") == 0) {
            shell_print(shell, "  Its one of those magical air poweres tubes where you put something in a pill shaped container.");
            shell_print(shell, "  Then put the pill shaped container in the tube. Then it goes into somewhere far far away...");
            shell_print(shell, "  According to the label on the tube, it gets sent to Minnesota?\n");
          }
          else if (strcmp(argv[2], "RTFM") == 0) {
            shell_print(shell, "  It appears to be a cleverly titled manual for making the TACONET work (yes it's bootsrapped on paper tape).");
            if(p_state->location_solved[0][2]==false){
              shell_print(shell, "  Something's wrong though with the current tape, you need to patch the information superhighway...of tacos!");
              shell_print(shell, "  After flipping through the pages you get the gist of it.");
            }
            else shell_print(shell, "  It's purely nostalgic at this point, considering you have already fixed the TACONET...");
            shell_print(shell, "  0) Paper tape instructions are in 8-bits: 7-Bit ASCII format LSB and 1-Bit parity MSB");
            shell_print(shell, "  1) You can only submit patches to an existing MASTER_TAPE located off-site at the Rabbit Hole.");
            shell_print(shell, "  2) Patches must be submitted in hexadecimal and will be XOR'd with the MASTER_TAPE.");
            shell_print(shell, "  3) Patches are submitted via hack PUNCH_TAPE with <YOUR_16_CHARACHTER_PATCH_IN_HEX>.");
            shell_print(shell, "  4) You can always refer to a copy of the original MASTER_TAPE by looking at it.\n");
          }
          else if (strcmp(argv[2], "MASTER_TAPE") == 0) {
            //Print out an ASCII version of the current master tape
            shell_print(shell,"  Here's what your copy of the current master tape looks like...\n");
            char master_bad[] = "49CE495420B00D8C";
            char m_patch[8][9];
            char b_temp[5];
            for(int i=0; i<8; i++){
              __htob(master_bad[i*2],b_temp);
              strcpy(m_patch[i],b_temp);
              __htob(master_bad[i*2+1],b_temp);
              strcat(m_patch[i],b_temp);
            }

            for(int i=0; i<8; i++){
              if(i==0) shell_print(shell,"  -^-^-^-^-^-^-");
              for(int j=0; j<8; j++){
                if(j==0) shell_fprintf(shell,SHELL_NORMAL,"  | "); //Print padding from the tape edge
                if(j==5) shell_fprintf(shell,SHELL_NORMAL,"*"); //Print the clock notch
                if(m_patch[i][j]=='0') shell_fprintf(shell,SHELL_NORMAL,"."); //Print a dash to signify no punch
                else shell_fprintf(shell,SHELL_NORMAL,"O"); //Print the hole punch
                if(j==7) shell_print(shell," |");
              }
              if(i==7) shell_print(shell,"  -^-^-^-^-^-^-\n");
            }
          } 
          else if (strcmp(argv[2], "PUNCH_TAPE") == 0) {
            shell_print(shell, "  It's old skool punch tape. You punch holes in the tape...with a TAPE_PUNCH...and a computer reads it!\n");
          } 
          else if (strcmp(argv[2], "TACONET") == 0) {
            __gfx_internet(shell);
            shell_print(shell, "  Not at all what you expected. Honestly, expected more cats.\n");
          } else
            error_flag = true;
          break;

          case 3:
          if (strcmp(argv[2], "LICKWID_SALSA_NYETROJEN") == 0) {
            shell_print(shell, "  The tank appears to hold some amazing hybrid compound.");
            shell_print(shell, "  Half Salsa. Half Liquid Nitrogen. Half Man-Bear-Pig.");
            shell_print(shell, "  What would that be? Since MBP is one and a half of a thing...");
            shell_print(shell, "  I mean, then the MBP half would actually be 3/4 an actual MBP.");
            shell_print(shell, "  So this tank is holding like, 175%% of something?\n");
          } 
          else if (strcmp(argv[2], "CIRCUIT_BOARD") == 0) {
            shell_print(shell, "  Up close you notice a Trusted Platform Module (TPM) for encryption.");
            shell_print(shell, "  However it also looks like the TPM has debugging breakouts.");
            shell_print(shell, "  Really small too. Almost the size of a grain...of something...");
            shell_print(shell, "  There's a RESET_BUTTON you could hack with your FINGER.\n");
          } 
          else if (strcmp(argv[2], "SPECIMIN_TACO") == 0) {
            shell_print(shell, "  Thats one big ass frozen taco.");
            shell_print(shell, "  Also, you notice some lights on the front of the stasis pod.");
            shell_print(shell, "  Why is a giant taco being kept in frozen stasis?\n");
          }
          else if (strcmp(argv[2], "HOOMAHN_FITZY") == 0) {
            shell_print(shell, "  Brushing the frost off of the pod glass, you see Joe Fitz locked inside.");
            shell_print(shell, "  He seems to be asleep, dreaming of the cyberz, but not completely frozen.");
            shell_print(shell, "  Before he crashed out he drew a figure 8 on the inside of the pod glass...");
            shell_print(shell, "  Regardless, need to figure a way to get him out.\n");
          }
          else if (strcmp(argv[2], "COMPUTER") == 0) {
            shell_print(shell, " There's no keyboard, just a display of a program already running."); 
            shell_print(shell, " It's a control system keeping both pods in stasis."); 
            shell_print(shell, " Additionally it claims all control messages are securely encrypted.\n"); 
          }
          else if (strcmp(argv[2], "RESET_BUTTON") == 0) {
            shell_print(shell, " Shiny...button...must...push...\n"); 
          }
          else
            error_flag = true;
          break;

          case 4: // West Items
          if (strcmp(argv[2], "CONTROL_BENCH") == 0) {
            shell_print(shell, "  You see an old, yet functioning computer running Linux.");
            shell_print(shell, "  A smart choice if you want some uptime on this brewery.");
            shell_print(shell, "  There are two monitors, call them MONITOR_1 and MONITOR_2");
            shell_print(shell, "  They only have a FLOPPY_DRIVE for standard IO, other than the KEYBOARD.");
            shell_print(shell, "  Also, there is a DRAWER at the base of the bench.\n");
          } 
          else if ((strcmp(argv[2], "MONITOR_1") == 0)&&(p_state->location_solved[1][4]==false)) {
             shell_print(shell, "  The screen shows nothing as if the system is waiting to boot.\n");
          } 
          else if ((strcmp(argv[2], "MONITOR_1") == 0)&&(p_state->location_solved[1][4]==true)) {
            shell_print(shell, "  Looking at the first monitor, all you see is the message ACTIVATION FAILED.");
            shell_print(shell, "  You quickly pound out a pwd and check the logs to find: hbs@linux~$ /var/log/brewOS");
            shell_print(shell, "  It was sent off site somewhere, but it has a link: http://bit.ly/2HWwd7z");
            shell_print(shell, "  With that file, you could surely figure out the license key and hack the KEYBOARD with it...right?");
            shell_print(shell, "  All this brewery is making you b00ze hangry. You just want to NOM on delicious craft brew...\n");
          } 
          else if ((strcmp(argv[2], "MONITOR_2") == 0)&&(p_state->location_solved[1][4]==false)) {
            shell_print(shell, "  The output appears to be running a program looking for a disk in the FLOPPY_DRIVE.\n");
          }
          else if ((strcmp(argv[2], "MONITOR_2") == 0)&&(p_state->location_solved[1][4]==true)) {
            shell_print(shell, "  The output appears to show to computer is happy you fed it's FLOPPY_DRIVE a FLOPPY_DISK. NOM NOM NOM.\n");
          }
          else if (strcmp(argv[2], "KEYBOARD") == 0) {
            shell_print(shell, "  Soooo many keys and in dvorak layout too. They are sticky with what you hope is just malt...\n");
          }
          else if ((strcmp(argv[2], "DRAWER") == 0)&&(p_state->location_solved[0][p_state->location] == false)) {
            shell_print(shell, "  It is locked by some pneumatic device controlled by the computer.\n"); 
          }
          else if ((strcmp(argv[2], "DRAWER") == 0)&&(p_state->location_solved[0][p_state->location] == true)) {
            shell_print(shell, "  The l00t DRAWER is now open.\n");
          }
          else if ((strcmp(argv[2], "FLOPPY_DRIVE") == 0)&&(p_state->location_solved[1][4]==false)) {
            shell_print(shell, "  The drive is spinning, waiting for a disk.\n");
          }
          else if ((strcmp(argv[2], "FLOPPY_DRIVE") == 0)&&(p_state->location_solved[1][4]==true)) {
            shell_print(shell, "  The drive is busy NOM-ing away on the disk.\n");
          }
          else if (strcmp(argv[2], "CONICALS") == 0) {
            shell_print(shell, "  Giant...shiny...stainless...steelery...full of pre-natal b00z3.");
            shell_print(shell, "  Something is written on it with smudgey finger oil remnants...");
            shell_print(shell, "  TkVWRVIgR09OTkEgR0lWRSBZT1UgVVAgTkVWRVIgR09OTkEgTEVUIFlPVSBET1dO\n");
          }
          else
            error_flag = true;
          break;
        }
      } else error_flag = true;
      
    break; //end argc(3) check for "look at THING"

    default: // user typed neither 1 nor 3 arguements
      error_flag = true;
    break;

  } //end argc switch statement

  if(error_flag) error_msg_look(shell);

  return 0;
}

// Create a root-level command
SHELL_CMD_REGISTER(LOOK, NULL, "B.E.N.D.E.R. look at an object", __cmd_look);


/**
 * @brief B.E.N.D.E.R. walk home
 * @param shell		Pointer to shell context
 * @param argc		Number of command arguments
 * @param argv		Pointer to array of command arguments
 * @return 0 if successful
 */
static int __cmd_walk_home(const struct shell *shell, size_t argc, char **argv) {
  bender_data_t* p_state = ff_bender_ptr_get();
  if (p_state->location != 0) {
    p_state->location = 0;
    __gfx_area_banner(shell, p_state->location);
    ff_settings_save();
  } else
    shell_print(shell, "  You are already at good old ::1\n");
  return 0;
}

/**
 * @brief B.E.N.D.E.R. walk north
 * @param shell		Pointer to shell context
 * @param argc		Number of command arguments
 * @param argv		Pointer to array of command arguments
 * @return 0 if successful
 */
static int __cmd_walk_north(const struct shell *shell, size_t argc, char **argv) {
  bender_data_t* p_state = ff_bender_ptr_get();

  if(p_state->location_solved[0][0]==false){
    // You can only leave home(0) to go north(1), or leave south(2) to go home(0)
    if (p_state->location == 0) {
      p_state->location = 1;
      __gfx_area_banner(shell, p_state->location);
      ff_settings_save();
    } else if (p_state->location == 2) {
      p_state->location = 0;
      __gfx_area_banner(shell, p_state->location);
      ff_settings_save();
    } else
      shell_print(shell, "  You better stay on the trail...\n");
    }
  else shell_print(shell, "  You can't walk anywhere, you're stuck in this room...until next year...\n");
  return 0;
}

/**
 * @brief B.E.N.D.E.R. walk south
 * @param shell		Pointer to shell context
 * @param argc		Number of command arguments
 * @param argv		Pointer to array of command arguments
 * @return 0 if successful
 */
static int __cmd_walk_south(const struct shell *shell, size_t argc, char **argv) {
  bender_data_t* p_state = ff_bender_ptr_get();

  if(p_state->location_solved[0][0]==false){
    // You can only leave home(0) to go south(2), or leave north(1) to go home(0)
    if (p_state->location == 0) {
      p_state->location = 2;
      __gfx_area_banner(shell, p_state->location);
      ff_settings_save();
    } else if (p_state->location == 1) {
      p_state->location = 0;
      __gfx_area_banner(shell, p_state->location);
      ff_settings_save();
    } else
      shell_print(shell, "  You better stay on the trail...\n");      
  }
  else shell_print(shell, "  You can't walk anywhere, you're stuck in this room...until next year...\n");
  return 0;
}

/**
 * @brief B.E.N.D.E.R. walk east
 * @param shell		Pointer to shell context
 * @param argc		Number of command arguments
 * @param argv		Pointer to array of command arguments
 * @return 0 if successful
 */
static int __cmd_walk_east(const struct shell *shell, size_t argc, char **argv) {
  bender_data_t* p_state = ff_bender_ptr_get();

  if(p_state->location_solved[0][0]==false){
    // You can only leave home(0) to go east(3), or leave west(4) to go home(0)
    if (p_state->location == 0) {
      p_state->location = 3;
      __gfx_area_banner(shell, p_state->location);
      ff_settings_save();
    } else if (p_state->location == 4) {
      p_state->location = 0;
      __gfx_area_banner(shell, p_state->location);
      ff_settings_save();
    } else
      shell_print(shell, "  You better stay on the trail...\n");      
  }
  else shell_print(shell, "  You can't walk anywhere, you're stuck in this room...until next year...\n");
  return 0;
}

/**
 * @brief B.E.N.D.E.R. walk west
 * @param shell		Pointer to shell context
 * @param argc		Number of command arguments
 * @param argv		Pointer to array of command arguments
 * @return 0 if successful
 */
static int __cmd_walk_west(const struct shell *shell, size_t argc, char **argv) {
  bender_data_t* p_state = ff_bender_ptr_get();

  if(p_state->location_solved[0][0]==false){
    // You can only leave home(0) to go west(4), or leave east(3) to go home(0)
    if (p_state->location == 0) {
      p_state->location = 4;
      __gfx_area_banner(shell, p_state->location);
      ff_settings_save();
    } else if (p_state->location == 3) {
      p_state->location = 0;
      __gfx_area_banner(shell, p_state->location);
      ff_settings_save();
    } else
      shell_print(shell, "  You better stay on the trail...\n");       
  }
  else shell_print(shell, "  You can't walk anywhere, you're stuck in this room...until next year...\n");
  return 0;
}

// Create a list of sub commands(for completion) - note each has its own handler
SHELL_STATIC_SUBCMD_SET_CREATE(sub_walk,
    SHELL_CMD(HOME, NULL, "Walk Home", __cmd_walk_home),
    SHELL_CMD(NORTH, NULL, "Walk North", __cmd_walk_north),
    SHELL_CMD(SOUTH, NULL, "Walk South", __cmd_walk_south),
    SHELL_CMD(EAST, NULL, "Walk East", __cmd_walk_east),
    SHELL_CMD(WEST, NULL, "Walk West", __cmd_walk_west), SHELL_SUBCMD_SET_END);
// Create root command to host the sub commands - note sub_walk and it has no handler
SHELL_CMD_REGISTER(WALK, &sub_walk, "B.E.N.D.E.R. walk", NULL);

// Shorthand Zork style directional commands for walking
SHELL_CMD_REGISTER(N, NULL, "B.E.N.D.E.R. Walk North (shorthand command)", __cmd_walk_north);
SHELL_CMD_REGISTER(S, NULL, "B.E.N.D.E.R. Walk South (shorthand command)", __cmd_walk_south);
SHELL_CMD_REGISTER(E, NULL, "B.E.N.D.E.R. Walk East (shorthand command)", __cmd_walk_east);
SHELL_CMD_REGISTER(W, NULL, "B.E.N.D.E.R. Walk West (shorthand command)", __cmd_walk_west);

void __gfx_area_banner(const struct shell *shell, int i){
  bender_data_t* p_state = ff_bender_ptr_get();
  switch(i){
    case 0 : shell_print(shell,
      "\n"
      "           ██░ ██  ▒█████   ███▄ ▄███▓▓█████     ██▒   █▓ ██▓ ██▓     ██▓    ▄▄▄        ▄████ ▓█████ \n"
      "          ▓██░ ██▒▒██▒  ██▒▓██▒▀█▀ ██▒▓█   ▀    ▓██░   █▒▓██▒▓██▒    ▓██▒   ▒████▄     ██▒ ▀█▒▓█   ▀ \n"
      "          ▒██▀▀██░▒██░  ██▒▓██    ▓██░▒███       ▓██  █▒░▒██▒▒██░    ▒██░   ▒██  ▀█▄  ▒██░▄▄▄░▒███   \n"
      "          ░▓█ ░██ ▒██   ██░▒██    ▒██ ▒▓█  ▄      ▒██ █░░░██░▒██░    ▒██░   ░██▄▄▄▄██ ░▓█  ██▓▒▓█  ▄ \n"
      "          ░▓█▒░██▓░ ████▓▒░▒██▒   ░██▒░▒████▒      ▒▀█░  ░██░░██████▒░██████▒▓█   ▓██▒░▒▓███▀▒░▒████▒\n"
      "           ▒ ░░▒░▒░ ▒░▒░▒░ ░ ▒░   ░  ░░░ ▒░ ░      ░ ▐░  ░▓  ░ ▒░▓  ░░ ▒░▓  ░▒▒   ▓▒█░ ░▒   ▒ ░░ ▒░ ░\n"
      "           ▒ ░▒░ ░  ░ ▒ ▒░ ░  ░      ░ ░ ░  ░      ░ ░░   ▒ ░░ ░ ▒  ░░ ░ ▒  ░ ▒   ▒▒ ░  ░   ░  ░ ░  ░\n"
      "           ░  ░░ ░░ ░ ░ ▒  ░      ░      ░           ░░   ▒ ░  ░ ░     ░ ░    ░   ▒   ░ ░   ░    ░   \n"
      "           ░  ░  ░    ░ ░         ░      ░  ░         ░   ░      ░  ░    ░  ░     ░  ░      ░    ░  ░\n"
      "                                                                                                     \n"
      "      ░░░░░░░░░░░░░██░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
      "      ▒▒▒▒▒░░░░░░░░██░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
      "      ▒▒▒▒▒▒▒▒▒▒▒▒░██░░▒▒▒▒▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
      "      ▒▒▒▒▒▒▒▒▒▒▒▒░█▓░▒▒▒▒▒▒▒▒▓▒▒▒▒▒▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
      "      ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
      "      ▒▒▒▒▒▒▒▒▒▒▒▒▒▓█▒▒▒▒▒▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
      "      ▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒█▒▒▒▒▒▒▒▒▒▒▒▒▒▓▒▒▒▒▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░▒█▒▓▒░░░░░░░░░\n"
      "      ▒▓▒▒▒▒▒▒▒▒▒▒▓▒▓▒▒▒▒▒▒▒▒▒▓▓▓▒▒▒▒██▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▒▒▒▒▒▒░░░░░░░░░░░░░░░░░░░░░▒▒█▒░▒░░░░░░░░░\n"
      "      ▒▒▒▒▒▒▒▒▒▒▒▒░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▒▒▒▓▓▒▓▓▓▓▓████▒▒▒▒▒░░░░░░░░░░░░░░░░░░░░░▒█▒▒▒▒▒▓▓▒▓▒▒▒\n"
      "      ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓████████▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒█▒▒▒▒▒▓▒▒▒▒██\n"
      "      ▒▒▒▒▒▒▒▒▒▒▒▒▒▒█▓▒▒▒▒▒▒▒▒▒▒▒▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓████████████▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒█▓▒▒▒▒▒▒▒▒▒▒▒\n"
      "      ▒▒▒▒▒▒▒▒▒▒▒▒▓▒▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓██▓▒▒▒▒▒▒▒▒▒▓████████████████▓▓▒▒▒▓▒▓▒▒▒▒▒▒▒▒▒▒▒▒▓█▒▒▒▒▒▒▒▒▒▒▒▒\n"
      "      ▒▒▒▒▒▒▒▒▒▒▒▓█▒▓█▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓████████████████████▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒█▒▒▒▒▒▒▒▒▒▒▒▒\n"
      "      ▒▒▒▒▒▒▒▒▒▒▒▒▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒████████████████████████▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒█▓▒▒▒▒▒▒▒▒▒▒▒\n"
      "      ▒▒▒▒▒▒▒▒▒▒▒░▓▓█▒░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒█████████████████████████████▒▒▒▒▒▒▒▒▒▒▒▒▒▒█▒▒▒▒▒▒▒▒▒▒▒▒\n"
      "      ▒▒▒▒▒▒▒▒▒▒░▒█░▒▓█▒▒▒▒▒▒▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓█████████████████████████████████▒▒▒▒▒▒▒▒▒▒▒▒█▓▒▒▒▒▒▒▒▒▒▒▒\n"
      "      ▒▒▒▒▒▒▒▒▒▒░░░░░░▓░▒▒▒▒▒▒▒▓▒▓▓▓▒▒▒▒▒▒▒▓█████████████████████████████████████▒▒▒▒▒▒▒▒▒▒▒█▒▒▒▒▒▒▒▒▒▒▒▒\n"
      "      ▒▒▒▒▒▒▒▒▒▒▓▓▓▒░▓▒▒▒▒▒▒▒▒▒▒████████████████████████████████████████████████▒▒▒▒▒▒▒▒▒▒▒▒█▒▒▒▒▒▒▒▒▒▒▒▒\n"
      "      ▒▒▒▒▒▒▒▒▒▒▓▒▓▓█▒▓▒▒▒▒▒▒▒▒▒▓███████████████████████████████████████████████▒▒▒▒▒▒▒▒▒▒▒▓██▒▒▒▒▒▒▒▒▒▒▒\n"
      "      ▒▒▒▒▒▒▒▒▒▒█▓░▒░▓░▒▒▒▒▒▒▒▒▒▒████████████████████████▓████████████████████▒▒▒▒▒▒▒▒▒▒▒▒▒▒█▒▒▒▒▒▒▒▒▒▒▒▒\n"
      "      ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▒▓▒▒▒▒▒▒▒▒▒▒▓██████████████████████▓▓████████████████████▒▒▒▒▒▒▒▒▒▒▒▒▒▒█▒▒▒▒▒▒▒▒▒▒▒▒\n"
      "      ▒▒▒▒▒▒▒▒▒▒█▒▓▓▓██▓▒▒▒▒▒▒▒▒▒████████████████████▓▓▓▓█████████████████████▒▒▒▒▒▒▒▒▒▒▒▒▒▒█▒▒▒▒▒▒▒▒▒▒▒▒\n"
      "      ▒▒▒▒▒▒▒▒▒▓▒██▒▒█▓▓▒▒▒▒▒▒▒▒▒██████████████████▓█▓▓▓██████████████████████▒▒▒▒▒▒▒▒▒▒▒▒▒▒█▒▒▒▒▒▒▒▒▒▒▒▒\n"
      "      ▒▒▒▒▒▒▒▒▒█▓▓█▒▒▓▒██▒▒▒▒▒▒▒▒████████████████▓▓▓▓▒▒▓▓▓▓▓▓▓▓█████▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒▓▒▓█▓▒▒▒▒▒▒▒▒▒▒▒\n"
      "      ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓███████████▓█▓▓▒▓▒▓▒▓▓▓▓▓▓▓▓▓▓██████████████████████████████▓▒▒▒▒▒▒▒▒▒▒▒\n"
      "\n");  
      break;  
      case 1 : shell_print(shell,
      "\n"
      "     ███▄    █  ▒█████   ██▀███  ▄▄▄█████▓ ██░ ██     ██▒   █▓ ██▓ ██▓     ██▓    ▄▄▄        ▄████ ▓█████ \n"
      "     ██ ▀█   █ ▒██▒  ██▒▓██ ▒ ██▒▓  ██▒ ▓▒▓██░ ██▒   ▓██░   █▒▓██▒▓██▒    ▓██▒   ▒████▄     ██▒ ▀█▒▓█   ▀ \n"
      "    ▓██  ▀█ ██▒▒██░  ██▒▓██ ░▄█ ▒▒ ▓██░ ▒░▒██▀▀██░    ▓██  █▒░▒██▒▒██░    ▒██░   ▒██  ▀█▄  ▒██░▄▄▄░▒███   \n"
      "    ▓██▒  ▐▌██▒▒██   ██░▒██▀▀█▄  ░ ▓██▓ ░ ░▓█ ░██      ▒██ █░░░██░▒██░    ▒██░   ░██▄▄▄▄██ ░▓█  ██▓▒▓█  ▄ \n"
      "    ▒██░   ▓██░░ ████▓▒░░██▓ ▒██▒  ▒██▒ ░ ░▓█▒░██▓      ▒▀█░  ░██░░██████▒░██████▒▓█   ▓██▒░▒▓███▀▒░▒████▒\n"
      "    ░ ▒░   ▒ ▒ ░ ▒░▒░▒░ ░ ▒▓ ░▒▓░  ▒ ░░    ▒ ░░▒░▒      ░ ▐░  ░▓  ░ ▒░▓  ░░ ▒░▓  ░▒▒   ▓▒█░ ░▒   ▒ ░░ ▒░ ░\n"
      "    ░ ░░   ░ ▒░  ░ ▒ ▒░   ░▒ ░ ▒░    ░     ▒ ░▒░ ░      ░ ░░   ▒ ░░ ░ ▒  ░░ ░ ▒  ░ ▒   ▒▒ ░  ░   ░  ░ ░  ░\n"
      "       ░   ░ ░ ░ ░ ░ ▒    ░░   ░   ░       ░  ░░ ░        ░░   ▒ ░  ░ ░     ░ ░    ░   ▒   ░ ░   ░    ░   \n"
      "             ░     ░ ░     ░               ░  ░  ░         ░   ░      ░  ░    ░  ░     ░  ░      ░    ░  ░\n"
      "                                                          ░                                               \n"
      "                       ╔╦╦╗                     ╔╦╦╗                       ╔╦╦╗                           \n"
      "                       ╚╬╬╝                     ╚╬╬╝                       ╚╬╬╝                           \n"
      "                        ╠╣                       ╠╣                         ╠╣           ░░░              \n"
      "         ╞══╦╦╗         ║║                       ║║                         ║║          ░   ░             \n"
      "            ╚╬╝         ║║              ╔════╗   ║║       ╔═══╗             ║║         ░   ░░░            \n"
      "             ║          ║║    ╔═════════╝░▒▒▒║   ║║  ╔════╝▒░░║             ║║            ░  ░            \n"
      "             ║          ║║════╝░▒▒▒▒▒▒▒░░░▒▒▒║   ║║══╝░▒▒▒▒▒░░║             ║║               ░            \n"
      "            ▓▓▓         ║║░░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░║   ║║░▒▒▒▒▒▒▒▒▒░║             ║║              ░░            \n"
      "          ▓▓▓▓▓▓▓       ║╝░▒▒▒▒▒▒▒▒░▒▒▒▒▒▒▒░░║   ║╝▒▒▒▒▒░▒▒▒▒▒║             ║║           ░░░       │      \n"
      "          ▓▓░░░▓▓       ║╡▒▒▒▒▒▒▒▒░░▒▒▒▒▒▒░░░║   ║╡▒▒▒░░▒░▒▒▒▒║             ║╡                     └─┐ ┌─ \n"
      "         ▓▓░░░░░▓▓      ║╗▒▒░▒▒▒▒░▒░▒▒▒▒▒▒▒░▒║   ║╗▒▒░▒▒▒░▒░▒▒║             ║║                       ├─┤  \n"
      "        ▓▓░░░░░░░▓▓     ║║▒▒░░▒▒▒▒▒░▒▒▒▒▒▒▒▒▒║   ║║▒▒▒▒▒░▒▒▒▒▒║▒▒▒▒▒        ║║   ▒▒▒▒▒▒▒▒▒▒▒▒       ┌┘░└┬ \n"
      "        ▓░░░░░░░░░▓     ║║▒▒▒▒▒▒▒▒▒░▒▒░▒▒▒▒▒▒║   ║║▒▒▒▒▒░▒▒▒▒▒║▓▓▓▓▒        ║║╡  ▒▓▓▓▓▓▓▓▓▓▓▒      ─┤░░░│ \n"
      "        ▓░░░░░░░░░▓     ║║▒▒▒▒▒▒▒▒▒░▒▒▒▒▒▒▒▒▒║   ║║▒▒▒▒░▒▒▒▒▒▒║▓▓▓▓         ║║╡   ▓⌐¬▓▓▓▓▓▓▓        │░░░│ \n"
      "        ▓░░░░░░░░░▓     ║║▒▒▒▒▒▒▒▒▒░▒▒▒▒▒▒▒▒▒║   ║║▒▒▒░▒▒░░▒▒▒║▓▓▓▓   _     ║║╡   ▓▓▓▓▓▓▓▓▓▓   _   ┌┴┐░┌┴ \n"
      "        ▓░░░░░░░░░▓     ║║▒▒▒▒▒▒▒▒▒░▒▒▒▒▒▒▒▒▒║   ║║▒▒▒░░░▒▒▒▒▒║▓▓▓▓ =(_(=   ║║    ▓▓▓▓▓▓▓▓▓▓ =(_(=   └─┤  \n"
      "        ▓░░░░░░░░░▓     ║║▒▒▒▒▒▒▒▒▒░▒▒▒▒▒▒▒▒▒║   ║║▒▒░░▒▒▒▒▒▒▒║▓▓▓▓  )_)    ║║    ▓▓▓▓▓▓▓▓▓▓  )_)      │  \n"
      "        ▓▓░░░░░░░▓▓     ║║▒▒▒▒▒▒▒▒▒░▒▒▒▒▒▒▒▒▒║   ║║▒▒▒▒▒▒▒▒▒▒▒║▒▒▒   (_(    ║║     ▒▒▒▒▒▒▒▒   (_(     ─┴┐ \n"
      "         ▓▓░░░░░▓▓      ║╝▒▒▒▒▒░▒▒▒▒▒▒▒▒▒▒░▒░║   ║╝▒▒▒▒▒▒▒▒▒░▒║░░▒▒  )_)    ║║    ▒▒░░░░░░▒▒  )_)       │ \n"
      "          ▓░░☻░░▓       ║╡░░░▒▒▒▒▒▒▒▒▒▒▒▒░░░░║   ║╡▒▒▒▒▒▒▒▒▒▒▒║░░▒▒         ║╡    ▒▒░░░░░░▒▒              \n"
      "          ▓▓▓▓▓▓▓       ║╗░░▒▒▒▒▒▒▒▒▒▒▒░╔════╝   ║╗▒▒▒▒░▒▒╔═══╝░▒▒          ║║     ▒▒░░░░▒▒               \n"
      "            ▓▓▓         ║║░░░░╔═════════╝        ║║░▒╔════╝▒▒▒▒▒▒           ║║      ▒▒▒▒▒▒                \n"
      "                        ║║════╝  ▓▓▓▓            ║║══╝      ▓▓▓▓            ║║       ▓▓▓▓                 \n"
      "                        ║║       ▓▓▓▓            ║║         ▓▓▓▓            ║║       ▓▓▓▓                 \n"
      "                        ║║     ▓▓▓▓▓▓▓▓          ║║       ▓▓▓▓▓▓▓▓          ║║     ▓▓▓▓▓▓▓▓               \n"
      "    ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
      "\n");  
      break; 
      case 2 : 
      if (p_state->location_solved[0][2]==false){
      shell_print(shell,
      "\n"
      "      ██████  ▒█████   █    ██ ▄▄▄█████▓ ██░ ██     ██▒   █▓ ██▓ ██▓     ██▓    ▄▄▄        ▄████ ▓█████ \n"
      "    ▒██    ▒ ▒██▒  ██▒ ██  ▓██▒▓  ██▒ ▓▒▓██░ ██▒   ▓██░   █▒▓██▒▓██▒    ▓██▒   ▒████▄     ██▒ ▀█▒▓█   ▀ \n"
      "    ░ ▓██▄   ▒██░  ██▒▓██  ▒██░▒ ▓██░ ▒░▒██▀▀██░    ▓██  █▒░▒██▒▒██░    ▒██░   ▒██  ▀█▄  ▒██░▄▄▄░▒███   \n"
      "      ▒   ██▒▒██   ██░▓▓█  ░██░░ ▓██▓ ░ ░▓█ ░██      ▒██ █░░░██░▒██░    ▒██░   ░██▄▄▄▄██ ░▓█  ██▓▒▓█  ▄ \n"
      "    ▒██████▒▒░ ████▓▒░▒▒█████▓   ▒██▒ ░ ░▓█▒░██▓      ▒▀█░  ░██░░██████▒░██████▒▓█   ▓██▒░▒▓███▀▒░▒████▒\n"
      "    ▒ ▒▓▒ ▒ ░░ ▒░▒░▒░ ░▒▓▒ ▒ ▒   ▒ ░░    ▒ ░░▒░▒      ░ ▐░  ░▓  ░ ▒░▓  ░░ ▒░▓  ░▒▒   ▓▒█░ ░▒   ▒ ░░ ▒░ ░\n"
      "    ░ ░▒  ░ ░  ░ ▒ ▒░ ░░▒░ ░ ░     ░     ▒ ░▒░ ░      ░ ░░   ▒ ░░ ░ ▒  ░░ ░ ▒  ░ ▒   ▒▒ ░  ░   ░  ░ ░  ░\n"
      "    ░  ░  ░  ░ ░ ░ ▒   ░░░ ░ ░   ░       ░  ░░ ░        ░░   ▒ ░  ░ ░     ░ ░    ░   ▒   ░ ░   ░    ░   \n"
      "          ░      ░ ░     ░               ░  ░  ░         ░   ░      ░  ░    ░  ░     ░  ░      ░    ░  ░\n"
      "                                                        ░                                               \n"
      "    ▓       ▓      ║        ║         ╔═══════╝░║       ╔═══════════╗  ║░╚═══════╝░║  ╔══════╗              \n"
      "    ▓       ▓      ║        ║         ║░░░░░░░░░║       ║░░░░░░░░░░░║  ║░░░░░░░░░░░║  ║░░░░░░║              \n"
      "    ▓       ▓      ║        ║         ║░╔═══════╝       ║░╔═══════╗░║  ╚═══════════╝  ║░╔══╗░║              \n"
      "    ▓       ▓      ║        ║         ║░║               ║░║       ║░║                 ║░║  ║░║              \n"
      "  ══▓•••••••▓══════║══════════════════════════════════════════════║░║═════════════════║░║════════════║══════\n"
      "  ░░▓       ▓░░░░░░║░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░║░║░░░░░░░░░░░░░░░░░║░║░░░░░░░░░░░░║░░░░░░\n"
      "  ══▓•••••••▓══════║══════════════════════════════════════════════║░║═════════════════║░║════════════║══════\n"
      "    ▓       ▓      ║        ║         ║░║               ║░║       ║░║                 ║░║  ║░║       ║      \n"
      "    ▓  ░░░  ▓      ║        ║     ╔═════════╗ ╔══════╗  ║░║       ║░║  ╔═══════════╗  ║░║  ║░║       ║      \n"
      "    ▓ ░   ░ ▓      ║        ║     ║░░░░░░░░░║ ║░░░░░░║  ║░║       ║░║  ║░░░░░░░░░░░║  ║░║  ║░║     ☼☼║☼☼    \n"
      "    ▓ ░ - ░ ▓   ◙──╧────────╧──◙  ║░╔═════╗░║ ║░╔══╗░║  ║░║       ║░║  ║░╔═══════╗░║  ║░║  ║░║    ☼  ║  ☼   \n"
      "    ▓ ░ - ░ ▓   │              │  ║░║ ║░║ ║░║ ║░║  ║░║  ║░╚════════════║░║════════════║░║══╝░║   ☼  ☼║☼  ☼  \n"
      "    ▓ ░ - ░ ▓   │ ERROR!!!!!!! │  ║░║ ║░║ ║░║ ║░║  ║░║  ║░░░░░░░░░░░░░░║░║░░░░░░░░░░░░║░║░░░░║  ☼  ☼ ║ ☼  ☼ \n"
      "    ▓ ░ - ░ ▓   │              │  ║░║ ║░║ ║░║ ║░║  ║░║  ╚══════════════║░║════════════║░║════╝  ☼ ☼  ║  ☼ ☼ \n"
      "  ══▓ ░ - ░ ▓═══│ SEGMENTATION │══════║░║═╝░║ ║░║  ║░║            ║░║  ║░║       ║░║  ║░║       ☼ ☼ ☼☼☼ ☼ ☼ \n"
      "  ░░▓ ░ - ░ ▓░░░│              │░░░░░░║░║░░░║ ║░║  ║░║            ║░║  ║░║       ║░║  ║░║       ☼ ☼  ☼  ☼ ☼ \n"
      "  ══▓ ░   ░ ▓═══│ FAULT! FUUU! │══════║░║═══╝ ║░║  ║░╚════════════║░║══════════╗ ║░╚══╝░║       ☼  ☼   ☼  ☼ \n"
      "    ▓  ░░░  ▓   │              │  ║░║ ║░║     ║░║  ║░░░░░░░░░░░░░░║░║░░░░░░░░░░║ ║░░░░░░║       ☼☼  ☼☼☼  ☼  \n"
      "    ▓       ▓   ◙──╤────────╤──◙  ║░╚═╝░║     ║░║  ╚══════════════║░║════════╗░║ ╚══════╝       ☼ ☼     ☼   \n"
      "    ▓       ▓      ║        ║     ║░░░░░║     ║░║                 ║░║  ║░║   ║░║                ☼  ☼☼☼☼☼    \n"
      "    ▓       ▓      ║        ║     ╚═════╝     ║░║                 ║░║  ║░║   ║░║               ☼            \n"
      "    ▓       ▓      ║        ║                 ║░║                 ║░║  ║░║   ║░╚═══╗          ☼             \n"
      "  ══▓•••••••▓═══════════════║═════════════════║░║═════════════════╝░║  ║░║   ║░░░░░║          ☼             \n"
      "  ░░▓       ▓░░░░░░░░░░░░░░░║░░░░░░░░░░░░░░░░░║░║░░░░░░░░░░░░░░░░░░░║  ║░║   ╚═══╗░║                        \n"
      "  ══▓•••••••▓═══════════════║═════════════════║░║═══════════════════╝  ║░║       ║░║                        \n"
      "    ▓       ▓      ║        ║                 ║░║                      ║░║       ║░║                        \n"
      "\n");
      }
      else{
       shell_print(shell,
      "\n"
      "      ██████  ▒█████   █    ██ ▄▄▄█████▓ ██░ ██     ██▒   █▓ ██▓ ██▓     ██▓    ▄▄▄        ▄████ ▓█████ \n"
      "    ▒██    ▒ ▒██▒  ██▒ ██  ▓██▒▓  ██▒ ▓▒▓██░ ██▒   ▓██░   █▒▓██▒▓██▒    ▓██▒   ▒████▄     ██▒ ▀█▒▓█   ▀ \n"
      "    ░ ▓██▄   ▒██░  ██▒▓██  ▒██░▒ ▓██░ ▒░▒██▀▀██░    ▓██  █▒░▒██▒▒██░    ▒██░   ▒██  ▀█▄  ▒██░▄▄▄░▒███   \n"
      "      ▒   ██▒▒██   ██░▓▓█  ░██░░ ▓██▓ ░ ░▓█ ░██      ▒██ █░░░██░▒██░    ▒██░   ░██▄▄▄▄██ ░▓█  ██▓▒▓█  ▄ \n"
      "    ▒██████▒▒░ ████▓▒░▒▒█████▓   ▒██▒ ░ ░▓█▒░██▓      ▒▀█░  ░██░░██████▒░██████▒▓█   ▓██▒░▒▓███▀▒░▒████▒\n"
      "    ▒ ▒▓▒ ▒ ░░ ▒░▒░▒░ ░▒▓▒ ▒ ▒   ▒ ░░    ▒ ░░▒░▒      ░ ▐░  ░▓  ░ ▒░▓  ░░ ▒░▓  ░▒▒   ▓▒█░ ░▒   ▒ ░░ ▒░ ░\n"
      "    ░ ░▒  ░ ░  ░ ▒ ▒░ ░░▒░ ░ ░     ░     ▒ ░▒░ ░      ░ ░░   ▒ ░░ ░ ▒  ░░ ░ ▒  ░ ▒   ▒▒ ░  ░   ░  ░ ░  ░\n"
      "    ░  ░  ░  ░ ░ ░ ▒   ░░░ ░ ░   ░       ░  ░░ ░        ░░   ▒ ░  ░ ░     ░ ░    ░   ▒   ░ ░   ░    ░   \n"
      "          ░      ░ ░     ░               ░  ░  ░         ░   ░      ░  ░    ░  ░     ░  ░      ░    ░  ░\n"
      "                                                        ░                                               \n"
      "    ▓       ▓      ║        ║         ╔═══════╝░║       ╔═══════════╗  ║░╚═══════╝░║  ╔══════╗              \n"
      "    ▓       ▓      ║        ║         ║░░░░░░░░░║       ║░░░░░░░░░░░║  ║░░░░░░░░░░░║  ║░░░░░░║              \n"
      "    ▓       ▓      ║        ║         ║░╔═══════╝       ║░╔═══════╗░║  ╚═══════════╝  ║░╔══╗░║              \n"
      "    ▓       ▓      ║        ║         ║░║               ║░║       ║░║                 ║░║  ║░║              \n"
      "  ══▓•••••••▓══════║══════════════════════════════════════════════║░║═════════════════║░║════════════║══════\n"
      "  ░░▓       ▓░░░░░░║░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░║░║░░░░░░░░░░░░░░░░░║░║░░░░░░░░░░░░║░░░░░░\n"
      "  ══▓•••••••▓══════║══════════════════════════════════════════════║░║═════════════════║░║════════════║══════\n"
      "    ▓       ▓      ║        ║         ║░║               ║░║       ║░║                 ║░║  ║░║       ║      \n"
      "    ▓  ░░░  ▓      ║        ║     ╔═════════╗ ╔══════╗  ║░║       ║░║  ╔═══════════╗  ║░║  ║░║       ║      \n"
      "    ▓ ░   ░ ▓      ║        ║     ║░░░░░░░░░║ ║░░░░░░║  ║░║       ║░║  ║░░░░░░░░░░░║  ║░║  ║░║     ☼☼║☼☼    \n"
      "    ▓ ░ - ░ ▓   ◙──╧────────╧──◙  ║░╔═════╗░║ ║░╔══╗░║  ║░║       ║░║  ║░╔═══════╗░║  ║░║  ║░║    ☼  ║  ☼   \n"
      "    ▓ ░ - ░ ▓   │              │  ║░║ ║░║ ║░║ ║░║  ║░║  ║░╚════════════║░║════════════║░║══╝░║   ☼  ☼║☼  ☼  \n"
      "    ▓ ░ - ░ ▓   │  TACONET UP! │  ║░║ ║░║ ║░║ ║░║  ║░║  ║░░░░░░░░░░░░░░║░║░░░░░░░░░░░░║░║░░░░║  ☼  ☼ ║ ☼  ☼ \n"
      "    ▓ ░ - ░ ▓   │              │  ║░║ ║░║ ║░║ ║░║  ║░║  ╚══════════════║░║════════════║░║════╝  ☼ ☼  ║  ☼ ☼ \n"
      "  ══▓ ░ - ░ ▓═══│  RESUME RAM  │══════║░║═╝░║ ║░║  ║░║            ║░║  ║░║       ║░║  ║░║       ☼ ☼ ☼☼☼ ☼ ☼ \n"
      "  ░░▓ ░ - ░ ▓░░░│              │░░░░░░║░║░░░║ ║░║  ║░║            ║░║  ║░║       ║░║  ║░║       ☼ ☼  ☼  ☼ ☼ \n"
      "  ══▓ ░   ░ ▓═══│   DOWNLOAD   │══════║░║═══╝ ║░║  ║░╚════════════║░║══════════╗ ║░╚══╝░║       ☼  ☼   ☼  ☼ \n"
      "    ▓  ░░░  ▓   │              │  ║░║ ║░║     ║░║  ║░░░░░░░░░░░░░░║░║░░░░░░░░░░║ ║░░░░░░║       ☼☼  ☼☼☼  ☼  \n"
      "    ▓       ▓   ◙──╤────────╤──◙  ║░╚═╝░║     ║░║  ╚══════════════║░║════════╗░║ ╚══════╝       ☼ ☼     ☼   \n"
      "    ▓       ▓      ║        ║     ║░░░░░║     ║░║                 ║░║  ║░║   ║░║                ☼  ☼☼☼☼☼    \n"
      "    ▓       ▓      ║        ║     ╚═════╝     ║░║                 ║░║  ║░║   ║░║               ☼            \n"
      "    ▓       ▓      ║        ║                 ║░║                 ║░║  ║░║   ║░╚═══╗          ☼             \n"
      "  ══▓•••••••▓═══════════════║═════════════════║░║═════════════════╝░║  ║░║   ║░░░░░║          ☼             \n"
      "  ░░▓       ▓░░░░░░░░░░░░░░░║░░░░░░░░░░░░░░░░░║░║░░░░░░░░░░░░░░░░░░░║  ║░║   ╚═══╗░║                        \n"
      "  ══▓•••••••▓═══════════════║═════════════════║░║═══════════════════╝  ║░║       ║░║                        \n"
      "    ▓       ▓      ║        ║                 ║░║                      ║░║       ║░║                        \n"
      "\n");       
      }
      break;
      case 3 : shell_print(shell,
      "\n"
      "         ▓█████ ▄▄▄        ██████ ▄▄▄█████▓    ██▒   █▓ ██▓ ██▓     ██▓    ▄▄▄        ▄████ ▓█████ \n"
      "         ▓█   ▀▒████▄    ▒██    ▒ ▓  ██▒ ▓▒   ▓██░   █▒▓██▒▓██▒    ▓██▒   ▒████▄     ██▒ ▀█▒▓█   ▀ \n"
      "         ▒███  ▒██  ▀█▄  ░ ▓██▄   ▒ ▓██░ ▒░    ▓██  █▒░▒██▒▒██░    ▒██░   ▒██  ▀█▄  ▒██░▄▄▄░▒███   \n"
      "         ▒▓█  ▄░██▄▄▄▄██   ▒   ██▒░ ▓██▓ ░      ▒██ █░░░██░▒██░    ▒██░   ░██▄▄▄▄██ ░▓█  ██▓▒▓█  ▄ \n"
      "         ░▒████▒▓█   ▓██▒▒██████▒▒  ▒██▒ ░       ▒▀█░  ░██░░██████▒░██████▒▓█   ▓██▒░▒▓███▀▒░▒████▒\n"
      "         ░░ ▒░ ░▒▒   ▓▒█░▒ ▒▓▒ ▒ ░  ▒ ░░         ░ ▐░  ░▓  ░ ▒░▓  ░░ ▒░▓  ░▒▒   ▓▒█░ ░▒   ▒ ░░ ▒░ ░\n"
      "          ░ ░  ░ ▒   ▒▒ ░░ ░▒  ░ ░    ░          ░ ░░   ▒ ░░ ░ ▒  ░░ ░ ▒  ░ ▒   ▒▒ ░  ░   ░  ░ ░  ░\n"
      "            ░    ░   ▒   ░  ░  ░    ░              ░░   ▒ ░  ░ ░     ░ ░    ░   ▒   ░ ░   ░    ░   \n"
      "            ░  ░     ░  ░      ░                    ░   ░      ░  ░    ░  ░     ░  ░      ░    ░  ░\n"
      "                                                                                                   \n"
      "                       ╔════════════════════════════╗  ╔══════════════════════════════╗                     \n"
      "                     ┌───┐                          ║  ║                            ┌───┐                   \n"
      "                     │▓░▓│              ╔═══════╗   ┌──┐                            │▓░▓│                   \n"
      "    ┌─────────────────────┐           ▓▓▓▓▓     ║   │∞∞│                           ┌─────────────────────┐  \n"
      "    │▓░░░░░░░░░░░░░░░░░░░▓│         ▓▓▓▓▓▓▓░▓   ╚═══│∞∞│═══════════════╗           │▓░░░░░░░░░░░░░░░░░░░▓│  \n"
      "    │░┌─────────────────┐░│        ▓▓▓▓▓▓▓▓░▓▓      └──┘               ║           │░┌─────────────────┐░│  \n"
      "    │░│                 │░│       ▓▓▓▓░░▓▓▓░▓▓▓                        ║           │░│                 │░│  \n"
      "    │░│                 │░│       ▓▓░░░▓▓▓▓▓░▓▓                        ║           │░│     ░░░░░░░     │░│  \n"
      "    │░│     ▓▓▓▓▓▓▓     │░│      ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓                       ║           │░│    ░░▓▓▓▓▓░░    │░│  \n"
      "    │░│    ▓▓▓▓▓▓▓▓▓    │░│      ═══════════════                       ║           │░│    ░░▓•▓•▓░░    │░│  \n"
      "    │░│   ▓▓▓▓▓▓▓▓▓▓▓   │░│     ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓     ┌──────────┐     ║           │░│   ░░ ░▓▓▓░ ░░   │░│  \n"
      "    │░│  ▓▓▓▓▓▓▓▓▓▓▓▓▓  │░│     ▓▓▓▓ LICKWID ▓░▓▓     │>         │     ║           │░│    ░ ░░░░░ ░    │░│  \n"
      "    │░│ ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓ │░│     ▓▓░▓▓ SALSA ▓▓░▓▓     │          │    ▓░░░░░░░▓    │░│       ░░░       │░│  \n"
      "    │░│                 │░│     ▓░░▓▓▓▓▓▓▓▓▓▓▓▓░▓     │          │    ░░░░░░░░░    │░│        ▓        │░│  \n"
      "    │░└─────────────────┘░│     ▓░▓ NYETROJEN ▓▓▓     │          │≈≈≈≈░░░░░░░░░    │░└─────────────────┘░│  \n"
      "    │░░┌───────────────┐░░│     ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓     └──────────┘    ▓░░░░░░░▓    │░░┌───────────────┐░░│  \n"
      "    │░░│*SPECIMIN_TACO*│░░│     ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓          └┘         ≡       ≡    │░░│*HOOMAHN_FITZY*│░░│  \n"
      "    │░░└───────────────┘░░│      ═══════════════     ┌──────────────────────────┐  │░░└───────────────┘░░│  \n"
      "    │░░░░░░░░░░░░░░░░░░░░░│      ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓     └──────────────────────────┘  │░░░░░░░░░░░░░░░░░░░░░│  \n"
      "    │░░○░•░○░•░•░•░•░○░○░░│      ═▓▓░░▓▓▓▓▓▓▓▓▓═       ┌┐                    ┌┐    │░░•░○░○░•░○░•░•░•░•░░│  \n"
      "    │░░░░░░░░░░░░░░░░░░░░░│      ═▓▓░▓▓▓▓▓░▓▓▓▓═       ││                    ││    │░░░░░░░░░░░░░░░░░░░░░│  \n"
      "    │▓░░░░░░░░░░░░░░░░░░░▓│     ═  ▓▓▓▓▓▓░░▓▓▓  ═      ││                    ││    │▓░░░░░░░░░░░░░░░░░░░▓│  \n"
      "    └──┬┬─────────────┬┬──┘     ═   ▓▓▓▓░▓▓▓▓   ═      ││                    ││    └──┬┬─────────────┬┬──┘  \n"
      "       ├┤             ├┤       ═    ═ ▓▓▓▓▓ ═    ═     ││                    ││       ├┤             ├┤     \n"
      "       └┘             └┘       ═   ═         ═   ═     └┘                    └┘       └┘             └┘     \n"
      "  ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
      "\n");  
      break;
      case 4 : shell_print(shell,
      "\n"
      "         █     █░▓█████   ██████ ▄▄▄█████▓    ██▒   █▓ ██▓ ██▓     ██▓    ▄▄▄        ▄████ ▓█████ \n"
      "        ▓█░ █ ░█░▓█   ▀ ▒██    ▒ ▓  ██▒ ▓▒   ▓██░   █▒▓██▒▓██▒    ▓██▒   ▒████▄     ██▒ ▀█▒▓█   ▀ \n"
      "        ▒█░ █ ░█ ▒███   ░ ▓██▄   ▒ ▓██░ ▒░    ▓██  █▒░▒██▒▒██░    ▒██░   ▒██  ▀█▄  ▒██░▄▄▄░▒███   \n"
      "        ░█░ █ ░█ ▒▓█  ▄   ▒   ██▒░ ▓██▓ ░      ▒██ █░░░██░▒██░    ▒██░   ░██▄▄▄▄██ ░▓█  ██▓▒▓█  ▄ \n"
      "        ░░██▒██▓ ░▒████▒▒██████▒▒  ▒██▒ ░       ▒▀█░  ░██░░██████▒░██████▒▓█   ▓██▒░▒▓███▀▒░▒████▒\n"
      "        ░ ▓░▒ ▒  ░░ ▒░ ░▒ ▒▓▒ ▒ ░  ▒ ░░         ░ ▐░  ░▓  ░ ▒░▓  ░░ ▒░▓  ░▒▒   ▓▒█░ ░▒   ▒ ░░ ▒░ ░\n"
      "          ▒ ░ ░   ░ ░  ░░ ░▒  ░ ░    ░          ░ ░░   ▒ ░░ ░ ▒  ░░ ░ ▒  ░ ▒   ▒▒ ░  ░   ░  ░ ░  ░\n"
      "          ░   ░     ░   ░  ░  ░    ░              ░░   ▒ ░  ░ ░     ░ ░    ░   ▒   ░ ░   ░    ░   \n"
      "            ░       ░  ░      ░                    ░   ░      ░  ░    ░  ░     ░  ░      ░    ░  ░\n"
      "\n"
      "            ╔════════════════════════════╦═══════════════════════════════════╦═════════════════╗           \n"
      "            ║                            ║                                   ║                 ║           \n"
      "          ┌───┐                        ┌───┐                                 ║   ┌──┐        ┌───┐         \n"
      "          │▓▓▓│   ╤                    │▓▓▓│   ╤            ┌─────────────────┐  │∞∞│        │▓▓▓│   ╤     \n"
      "     ┌────┴───┴────┐              ┌────┴───┴────┐       ┌──◄│░░░░░░░░░░░░░░░░░│══│∞∞│   ┌────┴───┴────┐    \n"
      "     │▓▓▓▓▓▓▓▓▓▓▓▓▓│              │▓▓▓▓▓▓▓▓▓▓▓▓▓│       │   └─────────────────┘  └──┘   │▓▓▓▓▓▓▓▓▓▓▓▓▓│    \n"
      "    ┌───────────────┐            ┌───────────────┐      ▲                              ┌───────────────┐   \n"
      "    │▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓│            │▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓│     ┌──────────────────────────┐    │▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓│   \n"
      "    │▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓│  ╤         │▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓│ ╤  ▒│                          │    │▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓│ ╤ \n"
      "    │▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓│  ║         │▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓│ ║  ▒│ ┌──────────┐┌──────────┐ │    │▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓│ ║ \n"
      "    │▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓╠══╩═╗       │▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓╠═╝  ▒│ │>         ││>         │ │    │▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓╠═╝ \n"
      "    │▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓│    ☼       │▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓│    ▒│ │          ││          │ │    │▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓│   \n"
      "    │▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓│    ☼       │▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓│    ▒│ │          ││          │ │    │▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓│   \n"
      "    │▓▓▓▓▓▓┌─┐▓▓▓▓▓▓│    ☼       │▓▓▓▓▓▓┌─┐▓▓▓▓▓▓│    ▒│ │          ││          │ │    │▓▓▓▓▓▓┌─┐▓▓▓▓▓▓│   \n"
      "    │▓▓▓▓▓┌┘║└┐▓▓▓▓▓│    ☼       │▓▓▓▓▓┌┘║└┐▓▓▓▓▓│    ▒│ └──────────┘└──────────┘ │    │▓▓▓▓▓┌┘║└┐▓▓▓▓▓│   \n"
      "    │▓▓▓▓┌┘▓║▓└┐▓▓▓▓│    ☼       │▓▓▓▓┌┘▓║▓└┐▓▓▓▓│    ▒│                          │    │▓▓▓▓┌┘▓║▓└┐▓▓▓▓│   \n"
      "    └┐▓▓▓│══○══│▓▓▓┌┘    ☼       └┐▓▓▓│══○══│▓▓▓┌┘   ┌──────────────────────────────┐  └┐▓▓▓│══○══│▓▓▓┌┘   \n"
      "     └┐▓▓└┐▓║▓┌┘▓▓┌┘     ☼        └┐▓▓└┐▓║▓┌┘▓▓┌┘   ▒│ ∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙ ∙∙∙  √────∙ │   └┐▓▓└┐▓║▓┌┘▓▓┌┘    \n"
      "     ║└┐▓▓└┐║┌┘▓▓┌┘║     ☼        ║└┐▓▓└┐║┌┘▓▓┌┘║   ▒│ ∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙ ∙∙∙         │   ║└┐▓▓└┐║┌┘▓▓┌┘║    \n"
      "     ║ └┐▓▓└─┘▓▓┌┘ ║  ┌┬──────┬┐  ║ └┐▓▓└─┘▓▓┌┘ ║   ▒│  ∙∙∙∙∙∙∙∙∙∙∙∙∙∙  ∙∙∙  √────∙ │   ║ └┐▓▓└─┘▓▓┌┘ ║    \n"
      "     ║  └┐▓▓▓▓▓┌┘  ║  └┤██████├┘  ║  └┐▓▓▓▓▓┌┘  ║   ▒│   ∙∙∙───────∙∙   ∙∙∙         │   ║  └┐▓▓▓▓▓┌┘  ║    \n"
      "     ║  ║└┐▓▓▓┌┘║  ║   │██████│   ║  ║└┐▓▓▓┌┘║  ║   ▒├──────────────────────────────┤   ║  ║└┐▓▓▓┌┘║  ║    \n"
      "     ║  ║ └───┘ ║  ║   │██████│   ║  ║ └───┘ ║  ║   ▒│┌─────────────────┐┌─────────┐│   ║  ║ └───┘ ║  ║    \n"
      "     ║  ║   ○   ║  ║   │██████│   ║  ║   ○   ║  ║   ▒│└─────────────────┘└─────────┘│   ║  ║   ○   ║  ║    \n"
      "    ═╩══╩═     ═╩══╩═  └──────┘  ═╩══╩═     ═╩══╩═  ▒└──────────────────────────────┘  ═╩══╩═     ═╩══╩═   \n"
      " ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
      "\n");  
      break; 
  }
}

void __gfx_alcohol_poisoning(const struct shell *shell){
	shell_print(shell, "\n"
		"      ░▒░░░░░▒▒░░░░░░░░░░░░░▒░░░░░░░░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒▒▒▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓███████████▓\n"
		"      ░░░░▓▓▓▓▓░░░░▒░░░░░░░░░░░░░▒░░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒▓▒▒▒▒▒▒▓▓▓▒▒▒▓▓▓▓▓▓▓▓▓▓▓▓▓█▓███████▓▓▓▓██\n"
		"      ░░░░▓▓▓▓▒░░░░░░░░░░░░░░░░░░▒▒░▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒▒▒▒▒▓▓▓▒▒▒▒▒▒▒▒▓▓▓▓▓▓▓▓█▓▒▒▓░▒▒▒▓▓▓▓███████▓▓▓██▓█▓▓\n"
		"      ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▓▓▓▓▓▓▓▓▓▓▓▓▓▒▓▒░░▓▓▒▒▒▒▒▒▓▓██████████▓▓█████\n"
		"      ░░░░░░░░░░░░░░░░░░░░░░░░░░░░▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒▒▓▓▓▓▓▓▓▓▓▓▓▓▒▒▒▒▒▓▒▒░▒▓▓▒▒▒▒▒▒▓███████▓███▓████\n"
		"      ░░░░░░░░░░░░░░░░░░░░░░░░░░░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▓█▓▓▒▒▒▒▒▒▒████████▓█▓▓▓▓\n"
		"      ░░░░░░░░░░░░░░░░░░░░░░░░░░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░░░▓▓▓▒▒▒▒▒▒▒▒▒▒░▓▓▓▓█▓▒▒▒▒▓▓▒███████▓▓██▓█\n"
		"      ░░░░░░░░░░░░░░░░░░░░░░░░░░▒▓▓▓▓▓▓▓▓▓▓▓████▓▓▓▓▓░░░░░░░░░░░░░░░░▒▓▓▓▒▒▒▒▒▓▓▓▓▒▒▒▓▓▓▓▓▓▒█▓▓▓▓▓▓▓▓▓█▓▓\n"
		"      ░░░░░░░░░░░░░░░░░░░░░░░░░▒▓▓▓▓▓▓▓████████▓▓█▓▓▓▓▓░░░░░░░░░░░░░░░░░░░░░░░░▒▓▓▓▓▓▓▓▓▓▓██▓▓█████▓▓▓▓▓▓\n"
		"      ░░░░░░░░░░░░░░░░░░░░░░░░░▓▓███▓███████████▓▓▓▓▓▓▒░░░░░░░░░░░░░░░░░░░░░░░░▓▓▓▓▓▓▓▒▓██▓██████████▓▓█▓\n"
		"      ░░░░░░░░░░░░░░░░░░░░░░░░▓██▓▓▓▓▓▓▓▓▓▓████████▓░░░░░░░░░░░░░░░░░░░░░░░░░░░░░▓▓▓▓██▓███████▓███▓█▓▓▓▓\n"
		"      ░░░░░░░░░░░░░░░░░░░░░░░▓▓██████▓▓██████████░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░▓▓▓▓▓███████▓██████▓▓███\n"
		"      ░░░░░░░░░░░░░░░░░░░░░░▓▓██████████▒▒░▒▒▒▒▒▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░▓▓▓█████████████▓████████▓▓\n"
		"      ░░░░░░░░░░░░░░░░░░░░░░▓█████████▒▒▒▒▒░▒▒░░░▒▒▒▒░░░░░░░░░░░░░░░░░░░░░░░▓▓██████▓██████████████▓▓████\n"
		"      ░░░░░░░░▒░▒░░░░░░░░░░██████████▓▒▒░▒░░░░▒▒▓▓▓▓▓▒▒▒░░░░░░░░░░░░░░░░░▒▓▓█████████████████████████▓███\n"
		"      ░░░░░░░░░░░░░░░░░░░░████████████▒░▒░░░░▒▒▒▒▒▒▓▓▒▒▒▓▓▓░░░░░░░░░░░░░▓██████████████▓▓▓██▓▓█████████▓█\n"
		"      ░░░░░░░░░░░░░░░░░░░██████████████▒░░░░░░░░░░░░░░░▓▓▒▒▒▒░░░░░░░░░░▓▓▓█▓███████████████████▓▓████████\n"
		"      ░░░░░░░░░░░░░░░░░░▒█████████████▒▒▒▒▒▒▒▒▒░▒░▒░░░░▒░░░░▒▒▒░░░░░░▓██▓▓█████████████████████████████▓█\n"
		"      ░░░░░░░░░░░░░░░░░▒▓███████████▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░░░░░░░░░░░▒░░░░█▓███████████████████████████████████\n"
		"      ░░░░░░░░░░░░░░░░░███████████▒░░▒░░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░▒▒▒▒▒░░░░░▓██████████████████████████████████████\n"
		"      ░░░░░▒▒▒▒▒▒▒░░░░██████████▓▒▒▒▒▒░▒▒▒▒▒▒▒▒▒▒▒▒░▒▒▒▒▒░▒▒░░░▒▒█▓▓█████████████████████████████████████\n"
		"      ░░░░░░░░░░▒▒░░░██████████▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░░░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒█▓██████████████████████████████████████\n"
		"      ░░░░░░░░░░░░░░█████████▒░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░░░▒▒▒▒▒▒▒▒▒▒▒▒░▒▒▒████████████████████████████████████████\n"
		"      ░░░░░░░░░░░░██████████▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░░░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒████████████████████████████████████████\n"
		"      ░░░░░░░░░░▒█████████▒░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒█▓███████████████████████████████████████\n"
		"      ░░░░░░░░░░▓████████▒░░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░░▓▒░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒█████████████████████████████████████████\n"
		"      ░░░░░░░░░██▓▓█████▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░░░░░█▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒█████████████████████████████████████████\n"
		"      ░░░░░░░░▓▓███████▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░▒█▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒██████████████████████████████████████████\n"
		"      ░░░░░░░▒▓██████▓▓▓▓▓▒▒▒▒▒▒▒░░░░▒▒▒▒░▒█▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒█████████████████▓▒▒░██████████████████████\n"
		"      ░░░░░░░▒▓█████▒▓▓▓▓▓▓▓▓▓░░▒▒▒▒▒▒▒▒▓██▒▒▒▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒█████████████████░▒▓░░█████████████▓▒▒██████\n"
		"      ░░░░░░░░▒██▓▒▒▓▓▓▓▓▓▓▓▓▓▒░░▒▒▒▒▒▒████░▒▓▓▓▓▓▓▓▓▒▓▒░▒▒▒████████████████▒▓▒▒▒████████████▓▓▒▒▒▒▒▒████\n"
		"      ░░░░░░░░░░▒▒▓▓▓▓▓▓▓▓▓▒▓██████▓▒▒▓█████▓▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒████████████████▒░░░██████████▓▓▒▒▒▒▒▒▒▒▒▒▒▒█\n"
		"      ░░░░░░░░▒▓▓▓▓▓▓▓▓▓▓▓▒▓█████████▓█▓████▓▓▓▓▓▓▓▓▒▒█████████████████████░░░▒▓████████▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
		"      ░░░░░░▒▓▓▓▓▓▓▓▓▓▓▓▒▒░░░░░░░░▓▓▓░░░███▓▓▓▓▓▓▓▓▓▒▓███████████████████░░░▒▒▒▒█████▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  );
}

void __gfx_benchoff(const struct shell *shell){
  shell_print(shell, "\n"
    "                                ░░░░░░░░░░░░▒▓▒░░░░░░░░░░░░░░░░░░░░░░░░  ░░░                            \n"       
    "                            ░░░░░░▒▒▒░░▓▓▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒░░░░░░░░░                                   \n"   
    "                              ░░░▓▒░▒▓▒▒▓▓▓▓▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒▓▓▓▓▓▓▓░░░░ ░                               \n"     
    "                      ░░░   ░░░▒▒▒▒▓▓▓▓▓▓▓▓▓▓▓▒▒░▒▒▒▒░░░░░▒▒▓▓▓▓▓▓▓▓▒░░░░░░░░░░░                        \n"    
    "                      ░░░  ░░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒░░░░░▒▓▒▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░                        \n"    
    "                          ░░░░░▓▓▓▓▓▓▓▓▓▓▓▓▓▒░░░░░▒░░░░░░▒▓▓▓▓▒▓▓▓▓▓▓▓▓▓▓▓▒░░░░░░                       \n"     
    "                      ░░░░░░▒▓▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒░░▒▒░░░▒▓▓▓▓▒▓▒▓▓▓▓▓▓▓▓▓▒░░░░                        \n"    
    "                      ░░░░▒▓▓▒▓▓▓▓▓▒░░░░░░▒▓▓▓▓▓▓▓▓▓▒░▒▓▒░░▒▓▓▒▓▒▓▒▓▓▓▓▓▓▓▓▓▓░░░                        \n"    
    "                      ░░░▒▒░▓▓▓▓▓▒░░░░░░░░░░▒▒▓▓▓▓▓▓▓▓▒▒▓▓░░▒▓▒░▓▒▓▓▓▓▓▓▓▓▓▓▓▓▒░                        \n"    
    "                      ░░░░░░▓▓▓▓▒░░░░░░░░░░░░░░░▒▒▒▒▓▓▓▓▓▓▓▒▒▓▒▒▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░                    \n"    
    "                      ░░░░░░▓▓▓▒░░░░░░░░░░░░░░░░░░░░▒▓▓▓▓▓▓▓▓▓▓▓▓▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░░                 \n"    
    "                      ░░░░▒▒▒▓▒░░░░░░░░░░░░░░░░░░░░░░▒▓▓▓▓▓▓▓▓▓▓▒▓▒▓▓▓▓▓▓▓▓▓▓▓▓▓▒░░░░░░                 \n"    
    "                      ░░░░░░▓▒░░░░░░░░░░░░░░░░░░░░▒▒░░░░▒▓▓▓▓▓▓▓▓▓▓▒▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░                 \n"    
    "                        ░░░░░▓▒░░░░░░▒▒▒░░░▒▒▒▒░░▒▒▒░▒░░░░░░░▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░                \n"     
    "                      ░░░░░▓▓▒▒▓▓▓▓▒▒▒░░░░░▒▒▒▒▓▓▓▓▓▒░░░░░░░░▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░                 \n"    
    "                      ░░░▓░░░░▒▓▓▓▒▒░░▒░░░░░▒▒░░▒▓▓▓▓▓░░░░░░░░▒▓▓▓▓▒▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░                 \n"    
    "                      ░░▒▒▒░░▒▒▒▓▓▒▒▒▒▒▒▒▒▓▓▒▒▒░▒▒▓▓▓▓▓▓▒▒▒▒▒▒▒▓▓▓▓▓▒▒▓▓▓▓▓▓▓▓▓▓▓░░░░░░                 \n"    
    "                      ░░▒▒▓▓▓▓▒▒▓▓▒▒▒░░▒▓▒▓▓░░░░▒▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒▓▓▒▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░                 \n"    
    "                      ░░▓▒▒▒▓▒▓▓▒▓▓▒▒▒▒▓▒▒▒▓▓▓▓▒▒▓▓▓▒░░░░░░░░░▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░                   \n"    
    "                      ░░▒▒▓▓▓▓▓░░▒▓▓▓▓▓▓▓▓▓▓▓▓▓▒▓▓▓▓▒░░░░░░░░░▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░                   \n"    
    "                      ░░░▒▒▓▓▒░░░░▒▓▓▓▒▓▓▓▓▓▓▓▓▒▓▓▓▒░░░░░░░░░▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓░▒░░░                   \n"    
    "                      ░░░░▒▓▒░░░░▒▓▓▓▓▓▓▓▒▒▒▒▓▓▓▓▒░░░░░░░░░░▒▓▒▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒░░░░░                 \n"    
    "                      ░░ ░░▓░░░░░░░░▒▓▓▓▒░▒▒░░░░░░░░░░░░░░░░░░░▒▓▓▓▓▓▓▓▓▓▓▒▓▓▓▓▓░░░░ ░░                 \n"    
    "                        ░░░░▓▒▒▒▒▓▒▒▒▒▓▓▓▓░░░░░░░░░░░░░░░░░░░░░░▓▓▓▓▓▓▓▓▓▒▓▒▓▓▓▓▓░░░░ ░░                \n"     
    "                      ░░░░░░▒▒▓▓▓▓▓▓▓▓▓░░▒░░░░░░░░░░░░░░░░░░░░▒▓▓▓▓▓▒▒▒▒▓▓▓▓▓▓▓▓▒░░░░░░                 \n"    
    "                        ░░░░▒▒▓▓▓▓▓▓▒▒░░░░░░░░░░░░░░░░░░░░░░░░▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░░                 \n"    
    "                      ░░░░░░▓▒▒▓▓▒░░░░░░░░░░░░░▒░░░░░░░░░░░░░▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒░░░░░                 \n"    
    "                      ░░░░░░░▓░▒▓▓▓▓▒▒░░░░░░░░░░▓▒░░░░░░░░░░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░░                 \n"    
    "                      ░░░░░░░░▓▓▒▒▒▒▓▓▓▓▓▒░░░░░░▒▒░░░░░░░░░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒▒▒░░░                    \n"    
    "                              ░░▓▒░░░░░░░░░▒▒░░░░░░░░░░░░░░▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒▒▒░░░░░░░                   \n"     
    "                              ░░▓▓▓▓▓▓▓▓▓▓▓▒░░░░░░░░░▒▒░░░▒▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░░                       \n"     
    "                              ░░▒▒░▒▒▒▒▒▒░░░░░░░░░░░▒▒▒▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░                           \n"     
    "                              ░░░▓░░░░░░░░░░░░░░░▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░░                       \n"     
    "                              ░░░▓▒░░░░░░░░░░░▒▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░░                       \n"     
    "                              ░░░░▓▓▓▒▒▒▒▒▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░                       \n"     
    "                              ░░░░▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░░░░░░             \n"    
    "                                ░░░░░░░░░▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒░░░░░░░░ ░░          \n"    
    "                                ░░░░░░░▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒░░░░░░░░░░      \n"    
    "                              ░░░░░░░▒▓▓▓▓▓▓▓▓░▒▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒░░░░░     \n"     
    "                ░░░░░░░░░░░░░░░░░▓▓▓▓▒▒▓▓▓▓▓▓▒░░░▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒░░░░░░░▒▓▓▓▓▓▓▓▓▒░░░  \n"     
    "                ░░░░░░░░░▒▓▓▓▓▓▓▒░▓▒▒▓▓▓▓▓▓▓▒░░░░▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒▓▓▓▓▒░░░░░░▒▒▒▒▒▒▒▒▓▓▓▓▓▓░░░   \n"    
    "                ░░░▓▓▓▓▓▓▓▓▓▓▒▒▒░▒▓▓░▒▓▓▓▓▓▓▓▒░░░░░▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒▒▒▓▓▒▒░░▒▒▒▒░▒▒▒▒▒▒▒▓▓▒▓▓▓░░░░░  \n"     
    "                ░░░░▒▒▒▒░░░░░░▒▒▒▓▓░▒▓▓▓▓▓▓▓▓▓▒░░░░▓▓▓▒░░░▒▒▒▓▓▓▓▒▓▓▓░▒▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▒▓▓▓░░░░░░░  \n"     
    "                ░░░░░░▒▒▒▒▒▒▒▒▒▒▒▓▓▒▓▓▓▓▓▓▒░░▓▒░░▒▓▓▒░░░░░░░▒▓▓▓▒▒▓▓▒▒▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▒▒▓▓░░░        \n"     
    "                  ░░░░░░▒▒▒▒▒▒▒▒▓▓▒▓▓▓▓▓▒▒░░░░░░░░░▒▒▒▒▒▒░▓▓▓▓▒▓▓▓▒▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▒▒░░░░          \n"   
    "                ░░░    ░░░░░▒▒▒▒▒▓▓▓▓▓▓▓▒▒▒░░░░░░░░▒▒░░░░▒▒▓▓▓▓▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▒░░░░░░░░        \n"     
    "                        ░░░░░░▒▒▓▓▓▓▓▓▓▒▒░░░░░░░░░░░░▒▒▒▒░▒▓▓▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▓▓▒▒░░░░░   ░░          \n"    
    "                          ░░░░░░░▒▒▓▓▓▓▒▒░░░░▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▓▓▓▓▓▒▒▒▒▒▒▓▓▓▓▓▒░░░░░░░░                \n"    
    "                              ░░░░░░░░░░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒░░░░░░░░░░░░░                \n"    
    );
}

void __gfx_moon(const struct shell *shell){
  shell_print(shell, "\n"
    "   ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░▓█▓▒▒░▒░▒▒░▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "   ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░▓███████████████████████▒▒▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "   ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░▒████▒▓█████████████████████████████▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "   ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░▒██████████████████████████████████████████░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "   ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░████████████████▓▓▓▓█████████████████████████████▒░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "   ░░░░░░▒░░░░░░░░░░░░░░░░░░░░▒████████████████▓▓▓▓███▓█▓████████████████████████████░░░░░░░░░░░░░░░░░░░░░░\n"
    "   ░░░░░░░░░░░░░░░░░░░░░░░░░████████████████████▓██████▓█████████████████████████▓▓▓▓███░░░░░░░░░░░░░░░░░░░\n"
    "   ░░░░░░░░░░░░░░░░░░░░░░▒████▓███▓███████████████████████████████████████████████████████▒░░░░░░░░░░░░░░░░\n"
    "   ░░░░░░░░░▒░▒░▓▒░░░░░░█████████████████████▓██████████████████████████████████████████████▒░░░░░░░░░░░░░░\n"
    "   ░░░░░░░▓░░▒▒▒▓░▓░▒░███████▓▒░▒███▓█▓▓███▓███████████████████████████▓▓▓████████████████████░░░░░░░░░░░░░\n"
    "   ░░░░░▒░░▒▒▒▒▒░▓░▓▓▓░▓███████▓████████████████████████████████████████████████████████████████░░░░░░░░░░░\n"
    "   ░░░░▒░▒▒▓▒▒▒▒░▓░█▓▒▒▒▒▒▒█████████▓█▓█▓█████████████████████████████████████████████████▓▒▒████░░░░░░░░░░\n"
    "   ░░░▒░░░░▓░▒▒▒▒▓░█▒░▒▒▓▓██▓▒▒█████▓██████▓███████████████████████████████████▒█████████▓████████▒░░░░░░░░\n"
    "   ░░░▒░▒░▓░▒▒░▒▓▒░▓▒░░░░░▒▒▒▓▒▓▒░▒▓█████▓▓█████████████████████████▓▓▓████████████████████████████▒░░░░░░░\n"
    "   ░░▒░▒▒▒░▒▒▓▒▒█░▓░▒▓░░░░░░░░░░▒▓▓▓░░░▓████████████████████████████████████████████████████████████░░░░░░░\n"
    "   ░░▓▒▒░▓░▒▓░▒▓▒▓▒▒░░░░░░░░░░▒░▒░▒▒▒▒▓▒░░▒████████████████████████████████████████████▓█████████████░░░░░░\n"
    "   ░▒▒░░░▒▓▓░░▓░▓░▓░░░░░░░░░░░░░░░░▒▒░░▒▒▓▒░▒█▓████████████████████████▓▓████████████████▓███▓▓▒▓▓▓███░░░░░\n"
    "   ░▓░░░▒▒░▒░▓░░▒░░░░░░░░░░░░░░░░░░░░░░▒░▓▒▓▓▓▓▓▒▒█████▓████████▓▓█▒▒▒▒▒▒▒▒████████████████▓░▒▓▓█▓▓███░░░░░\n"
    "   ░░░░░▒░▒▒▓░░▒░░░░░░░░░░░░░░░░░░░░░░░▒░░▒▒▓▒▓▓█▓██▓▓▓█▓▓███▓▓█▓▒▒▒▒███████▓█████████████▒▒▓█░░▒██▓███░░░░\n"
    "   ░░░░░░▒▓░░▒░░░░░░░░░░░░░░░░░░░░░░▒░░░░░▒░▒▒▒▒▓▓██▓█▓▓▓▓▓█▓▓▓▒░▒▒▒▒▓▓▓▓▓████████████████▓▒▓███▓▓█████░░░░\n"
    "   ░░░░░░░░▒▒░░░░░░░░░░░░░░░░░░░░░░░░▓▓▓▒░░▒░▒▓▓▓▓░▒█▓▓▓▓▓▓▓█░░░▒▒▒▓▓█████▓▓████▓███▓██████▓▓▓▓█████████░░░\n"
    "   ░░░░░░░▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░▒▓▓▓███▓███████▒▒▒▓▓▒▓▓▒▒▒▓▒▓▓▓██████████████████████████▓░░░\n"
    "   ░░░░░░░░██▓▓█░▒░░░░░░░░░░░░░░░░░░░░░░░░░░░▒█▓▓▓█▓████████▓█▒░░░░▒▒▓░░░░▒▒▓█▓████▓▓▒▒▓████████████▓▓█▓░░░\n"
    "   ░░░░░░░░▓▓█▓▓░░░░░░▒░░░░░░░░░░░░░░░░░░░██▓▓▓▓███████████████▓▓▓▓▓▒░░░░▒▒▒▓▓███▓▓█▒▒▒▒▓▓██████████▓██▓░░░\n"
    "   ░░░░░░░░██░░░░░▒░░░░░░▒▓▓░░░░░░░░░░░░░▓▓▓███▓███████████████████▓▒░▒▒▒▓▓▓▓██████▓▓████▓▓▒███████▓███▓░░░\n"
    "   ░░░░░░░░██▓░░░░▓▓░▒▒░▒▓▒▓░░░▒░░░░░░░░░▒▓████████████████████████████████████████████████▓▓█▓▓▓▒▓▓▓██▒░░░\n"
    "   ░░░░░░░░█▓▓░░░▓▓▓▒▒▒▓▓▓▒░░░░░░▒▒░░▒▓▓▒▓▓▓▒▓▒▓▓▓░███████████▓█▓▓██████████████████████████▓█▓▓▓██████▒░░░\n"
    "   ░░░░░░░░███▒▒▒▒▓▓▓▓▓▓▓▒░▓░░░▒░▓▓▒░▒▓▓▓▓▓▓▓▓█▓▓████████████▓▓███▓████████████████████████▓▓▒▒▒▒▒▒████░░░░\n"
    "   ░░░░░░░░▒██▓████████▓▓▓▒░▒▒▓▓▓▓▒▓▒▒▒▒▓▓███████▓▓█▓▒▓█▓▓▓▓▒░░░▓▓▓▒▓█████████████▓████████▒█▓████████▓░░░░\n"
    "   ░░░░░░░░░██████▓▓▓▒▒▓░▒▒▒▓▓█▓▓▓▓▒▒▒▓▓█▓█▓▓██▓▓███▒▒▓█▒▒▒▒▒▒▒▒▓█▓▓▒▒▓███████████████████▓▒▒▓███████▓░░░░░\n"
    "   ░░░░░░░░░▓▓▓▓▒▒▒▒░░░▒▓▒▒▓▓▒▓▓▓▒▓▒▒▓▓▒█▓▓▓▓▓█▓▓███████▓▓▓▓███▓█████▓▒▒██▓████▓██▓█████████▓▓▓█████▓▓░░░░░\n"
    "   ░░░░░░░░░░█████▓▒▒▒▓▓▒▓▓▓▒▓▓▓▓▓▓▒▓▓██▓█▓█▒▓▓████████████████▓██████▓▓▓▓████████████▓███████▓███▓▓▓░░░░░░\n"
    "   ░░░░░░░░░░▒████▓▓▒▒▒░▒▒▓▓▓▓▓██▓▒▓▓▓██▓█▓▓█████████▓▒▒▒▒▓▒▒░▒▒▒▓▓▓▓▓███▓▓█████████▓▓█▓▓▓████████▓▓▒░░░░░░\n"
    "   ░░░░░░░░░░░▓██▓█▓▓▓▓▓▓▓▓█▓▓▓▓██▓▓█████▓████████▓▒█░░░░░░░▒▒▒░░░░▒▒▓▓█▓█▓████████▓█████████▒▓▓▓▓▓▓░░░░░░░\n"
    "   ░░░░░░░░░░░░█▓██████████████████████▓██▓█████▓▒░▒▓▒▒▒░░░▒▒▒▒▒▒▒▒░░░░░▓█▓█████████████▓▓███████▓█░░░░░░░░\n"
    "   ░░░░░░░░░░░░░▓▓▓█▓▒▒▓█▓█▓▓███████████▓▓██▓▓▒▒▒▒▓█▓▒▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒████████████████▓█▓██████▓█░░░░░░░░░\n"
    "   ░░░░░░░░░░░░░░▒▒▒▒▓▓▒▒▒▓▒▒▓▒████▓█████████▓▓▓█▓▓▓▓▒▒▒▒▒░░░░░░░░░▒▓▓▒▓▓███████▓█████▓▓███████▓█░░░░░░░░░░\n"
    "   ░░░░░░░░░░░░░░░░██▓▓▓▓▓▓▓▓▓▒▓██████▓▓█████▓▓▓█▓▓▒▓▒▓▓▓▒▒▒▒▒░░▒▒▓▓▓█▓▓████████▓██████████▒▒▒▓▓░░░░░░░░░░░\n"
    "   ░░░░░░░░░░░░░░░░░▓▓▓██▓█▒▓▓█▓▒▓▓██████▓▓██▓▓▓▓▒███▓██▓███▓██▓▓▓██▓█████████▒▒▒▒▒▓▓████▓▒▓▓▓░░░░░░░░░░░░░\n"
    "   ░░░░░░░░░░░░░░░░░░▒████▓██▓▓▓▓▒███████████████████▓▒█▓█▓██████████████████▒█▓██▓▒████▓▓▓▓░░░░░░░░░░░░░░░\n"
    "   ░░░░░░░░░░░░░░░░░░░░▓███████████████▓▓█▒██████▓▓▒▒▓▓▓▓▓▒▓▒█▓▓██████▓██████▓██▓▒▒▓██▓▓▒▓░░░░░░░░░░░░░░░░░\n"
    "   ░░░░░░░░░░░░░░░░░░░░░░▒███████▓███▓██████▒▓▓▓███▓██▓▓▒▒▒▒▓▒▓▓██▓█████████▓██▓▓█████▒▒▒░░░░░░░░░░░░░░░░░░\n"
    "   ░░░░░░░░░░░░░░░░░░░░░░░░▒▓▓████████▓▓▒▓██▓████▓▓░████▓███████▒▒▓▓▓█████▓░░▓▓▓▓▒▓▓▓▒▒░░░░░░░░░░░░░░░░░░░░\n"
    "   ░░░░░░░░░░░░░░░░░░░░░░░░░░░▒████▓█▓▓▓▓█▓▓███████▓██████████▓████████▓█▓▒▓▒▒░▒▒▒▓░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "   ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░▒████▒▒▒▓▒▒▓▒░▒▒▓█████▓▒▒▓█▓███▓▓██▓██▒▓▒░▒░▓▓░▒░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "   ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░▓▓██▓▓██▓▓▓███▓▒██▓██▓▓▓█▓▒▒█▓█▓░░░░░░▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "   ░░░░░▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░▒▓▓█▓▒▒▓█▓█▓▒▒▓▓▓█▓░░░░░░░░░▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "   ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░▒░▒░▒▒▓░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
  );
}

void __gfx_floppy_disk(const struct shell *shell){
  shell_print(shell, "\n"
    "   ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "   ░░░▓████████████████████████████████████████████████████████████████████████░░░\n"
    "   ░░░█░▒▒▒▒▒▒▒▒▒▒░▓▓▓▒▒▒▒▒▓▓▒▒▒▓░█▒█▓▒▒▓▒▒▒░▓▓▒█░░█▒█░▒▓▒▒▓▓▒▓░░▓▒██▓▓▓███████▒░░\n"
    "   ░░░█░█▒▓▒▒▓▒▒▒▒▒▓████████████████▓██▓██▓████▓█▓█████████████████████████████▒░░\n"
    "   ░░░█░███████▓▓█▒▓███████████████████████████████████████████████████████████▒░░\n"
    "   ░░░█░█▓████████▒▓█▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓██▒░░\n"
    "   ░░░█░░░░░░░░░░░░▒███████████████████████████████████████████████████████████▒░░\n"
    "   ░░░█████████████████████████████████████████████████████████████████████████▒░░\n"
    "   ░░░█████████████████████████████████████████████████████████████████████████▒░░\n"
    "   ░░░█████████████████████████████████████████████████████████████████████████▒░░\n"
    "   ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "   ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "   ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "   ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░▒▒▒▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "   ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░▒▒▒▒▒▒▒▒▒▒▒▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "   ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░▒▒▒▒▒░░░░░░░░▒▒▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "   ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░▒▒▒▒▒░░░░░░░░░░░░▒▒▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "   ░░░░░░░░░░░░░░░░░░░░░░░░░░░░▒▒▒▒▒░░░░░░░░░░░░░░▒▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "   ░░░░░░░░░░░░░░░░░░░░░░░░░░░░▒▒▒▒░░░░░░░░░░░░░░░░▒▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "   ░░░░░░░░░░░░░░░░░░░░░░░░░░░░▒▒▒▒░░░░░░░░░░░░░░░░▒▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "   ░░░░░░░░░░░░░░░░░░░░░░░░░░░░▒▒▒▒▒░░░░░░░░░░░░░░▒▒▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "   ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░▒▒▒▒▒░░░░░░░░░░░░▒▒▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "   ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░▒▒▒▒▒▒▒▒░░░░▒▒▒▒▒▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "   ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "   ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░▒▒▒▒▒▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "   ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "   ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "   ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "   ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░▒▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "   ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░▒▒▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "   ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░▒▒▒▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "   ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░▒▒▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "   ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░▒▒▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "   ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░▒▒▒▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "   ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░▒▒▒▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "   ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░▒▒▒▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "   ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░▒▒▒▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "   ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░▒▒▒▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
  );
}

void __gfx_internet(const struct shell *shell){
  shell_print(shell, "\n"
  " ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  " ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  " ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  " ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓███▓█████▓▓▓▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  " ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓████████▓▓▓████▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  " ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▓██████████████▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  " ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▓████████▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  " ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  " ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  " ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  " ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒▒▓▓▒▒▒▒▒▒▒▒▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  " ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒▒▒▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  " ▒▒▒▒▒▒▒▒▒░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒▓▓▓▓▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  " ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒▒▒▓▓▓▓▓▓▒▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  " ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▒▒▒▒▓▓▓▓▓▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  " ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▒▒▒▓▓▓▓▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  " ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒▒▒▒▒▒▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  " ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒▒▒▒▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  " ░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒▒▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  " ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▓▓▓▓▓▓▓▓██▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  " ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  " ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  " ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▓▓▓▓▓▓▓▓▒▒▒▒▓▒▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  " ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▓▓▓▓▓▓▒▒▒▒▓▒▒▒▒▓█▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  " ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▒▒▒▒▒▒▒▓▓▓▓▓▒▒▒▒▒▒▒▓███▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  " ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒▓▓▒▒▒▓██████▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  " ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▓▓▒▒▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒▒▒▒▓█████▓▓▓██▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  " ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓█████▓▓▒▒▒▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒▒▒▒▒▓█████▓▓▓██████████▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  " ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓█████████▓▒▒▒▒▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒▓▓████▓▓▓▓███████████████▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒\n"
  " ▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓█████████████▓▒▒▒▒▒▓▓▓▓▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒▓▓▓▒▒▒▓▓▓▓▓▓▓███████████████████▓▒▒▒▒▒▒▒▒▒▒\n"
  " ▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓████████████████▓▒▒▒▒▒▓▓▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒▓▓▓▓▓▓▓▒▒▒▒▓▓▓███████████████████▓▓▒▒▒▒▒▒▒▒▒\n"
  " ▒▒▒▒▒▒▒▒▒▓▓████████████████████▓▒▓▒▒▒▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▒▒▒▒▓▓▓▓▓▓▓▓████████████████████▓▒▒▒▒▒▒▒▒▒\n"
  " ▒▒▒▒▒▒▒▒▒▓█████████████████████▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒▒▒▓▓▓█▓▓▓▓▓▒▒▒▒▓▓▓▓▓███████████████████▓▒▒▒▒▒▒▒▒▒\n"
  " ▒▒▒▒▒▒▒▒▒▓█████████████████████████▓▓▓▓▒▒▒▒▓▓▓▓▓▓▓████████▓▓▓▓▓▓▓▓▒▓▓▓▓██████████████████▓▒▒▒▒▒▒▒▒▒\n"
  " ▒▒▒▒▒▒▒▒▓██████████████████████████▓▓▓▓▒▒▒▒▓▓▓▓██▓▓▓▓▓████▓▓▓▓▓▓▓▓▓▓▒▓▓██████████████████▓▒▒▒▒▒▒▒▒▒\n"
  " ▒▒▒▒▒▒▒▒▓██████████████████████████████▒▒▒▒▓▓▓▓▓▓▓▓▓▓▓██▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓██████████████████▓▒▒▒▒▒▒▒▒▒\n"
  " ▒▒▒▒▒▒▒▒▓██████████████████████████▓▓▓▓▒▒▒▒▓▓▓▓▓█▓██████████▓▓▓▓▓▓▓▓▒▓▓█████████████████▓▒▒▒▒▒▒▒▒▒▒\n"
  " ▒▒▒▒▒▒▒▒▒███████████████████████████▓▓▓▒▒▒▒▓███████▓▓███████▓▓▓▓▓▓▓▓▓▓▓████████████████▓▒▒▒▒▒▒▒▒▒▒▒\n"
  " ▒▒▒▒▒▒▒▒▒▓████████████████████████████▓▓▓▒▒▓▓▓▓▓▓▓▓▓▓▓████████▓▓▓▓▓▓▓▓▓▓████████████████▒▒▒▒▒▒▒▒▒▒▒\n"
  " ▒▒▒▒▒▒▒▒▓▓██████████████████████████▓▓▓▓▓▓▓▓▓▓▓████████████████▓▓▓▓▓▓▓▓▓▓███████████████▓▒▒▒▒▒▒▒▒▒▒\n"
  " ▒▒▒▒▒▒▒▒▓▓█████████████████████████████▓▓▓█████▓▓▓▓▓▓▓██████████▓▓▓▓▓▓▓▓▓▓██████████████▓▒▒▒▒▒▒▒▒▒▒\n"
  " ▒▒▒▒▒▒▒▒▓█████████████████████████████▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓████████████▓▓▓▓▓▓████████████████▒▒▒▒▒▒▒▒▒▒▒\n"
  " ▒▒▒▒▒▒▒▓▓████████████████████████████▓▓▓▓▓▓▓▓▓███████████████████████████████████████████▓▒▒▒▒▒▒▒▒▒\n"
  " ▒▒▒▒▒▒▒▓███████████████████████████████▓▓▓█▓▓▓▓▓▓▓▓▓▓▓▓████████████████▓██████████████████▓▒▒▒▒▒▒▒▒\n"
  " ▒▒▒▒▒▒▒▓█████████████████████████████▓▓▓▓▓▓▓▓▓▓▓▓███████████████████████▓███████████████████▓▒▒▒▒▒▒\n"
  " ▒▒▒▒▒▒▒▓███████████████████████████████▓▓████████▓▓▓▓████████████████████████████████████████▓▒▒▒▒▒\n"
  " ▒▒▒▒▒▒▒▓███████████████████████████████▓▓▓▓▓▓▓▓▓▓▓▓▓██████████████████████████████████████████▒▒▒▒▒\n"
  " ▒▒▒▒▒▒▒▒▓█████████████████████████████▓▓▓█▓▓▓▓▓███████████████████████████████▓██████████████▓▒▒▒▒▒\n"
  " ▒▒▒▒▒▒▒▒▒▓█████████████████████████████▓▓█████████▓▓▓████████████████████████████████████████▓▒▒▒▒▒\n"
  " ▒▒▒▒▒▒▒▒▒▒▓████████████████████████████▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓███████████████████████████████████▓▒▒▒▒▒\n"
  " ▒▒▒▒▒▒▒▒▓▓▓███████████████████████████▓▓▓█▓▓▓▓▓██████████████████████████████████████████████▓▒▒▒▒▒\n"
  " ▒▒▒▒▒▒▓████████████████████████████████▓▓███████████████████████████████████████████████████▓▒▒▒▒▒▒\n"
  " ▒▒▒▒▒▓█████████████████████████████████▓▓█▓▓▓▓▓▓▓▓▓▓▓▓▓█████████████████████████████████████▓▒▒▒▒▒▒\n"
    "\n");
}

void __gfx_game_over(const struct shell *shell){
  shell_print(shell, "\n"
    "  ███████ YOU ARE DEAD ██████\n"
    "  ███████████████████████████\n"
    "  ███████▀▀▀░░░░░░░▀▀▀███████\n"
    "  ████▀░░░░░░░░░░░░░░░░░▀████\n"
    "  ███│░░░░░░░░░░░░░░░░░░░│███\n"
    "  ██▌│░░░░░░░░░░░░░░░░░░░│▐██\n"
    "  ██░└┐░░░░░░░░░░░░░░░░░┌┘░██\n"
    "  ██░░└┐░░░░░░░░░░░░░░░┌┘░░██\n"
    "  ██░░┌┘▄▄▄▄▄░░░░░▄▄▄▄▄└┐░░██\n"
    "  ██▌░│██████▌░░░▐██████│░▐██\n"
    "  ███░│▐███▀▀░░▄░░▀▀███▌│░███\n"
    "  ██▀─┘░░░░░░░▐█▌░░░░░░░└─▀██\n"
    "  ██▄░░░▄▄▄▓░░▀█▀░░▓▄▄▄░░░▄██\n"
    "  ████▄─┘██▌░░░░░░░▐██└─▄████\n"
    "  █████░░▐█─┬┬┬┬┬┬┬─█▌░░█████\n"
    "  ████▌░░░▀┬┼┼┼┼┼┼┼┬▀░░░▐████\n"
    "  █████▄░░░└┴┴┴┴┴┴┴┘░░░▄█████\n"
    "  ███████▄░░░░░░░░░░░▄███████\n"
    "  ██████████▄▄▄▄▄▄▄██████████\n"
    "  ████████ GAME OVER ████████\n"
    "\n");
}

void __gfx_believe(const struct shell *shell){
  shell_print(shell, "\n"
    "  ▒▒▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "  ░▒▒▒▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░▒▓▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "  ▒▒▒▒▒▒░░░░░░░░░░░░░░░░░░░░░▒░░░░░░▒▓▓▓▓▓▓▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "  ▒▒▒▒▒▒▒▒▒▒▒▒░░░░░░░░░░▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▒▒▒▒▒░▒▒▒▒▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "  ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "  ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "  ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░░░░░░░░░░░░░░░░░░░░░░░\n"
    "  ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░░░░░░░░░░░░░░░░░░░░░░░\n"
    "  ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░░░░░░░░░░░░░░░░░░░░░░\n"
    "  ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "  ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▒▒▒▒▒▒▒▒▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "  ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓██▓▒▒▒▒▒▒▒▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "  ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓███▒▒▒▒▒▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "  ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓███▓▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "  ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓█▓██▓▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "  ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒██▓███▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "  ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓████▓▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "  ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▓▓▓▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "  ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓██▓▓▓▓▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "  ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓████▓▓▓▓▓▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "  ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓████▓▓▓▓▓▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "  ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓██████▓▓▓▓▓▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "  ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓████▓█▓▓▓▓▓▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "  ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓█████▓█▓▓▓▓▓▓▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "  ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓███████▓▓▓▓▓▓▓▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "  ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓███████▓▓▓▓▓▓▓▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "  ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓██████▓█▓▓▓▓▓▓▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "  ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓███████▓█▓▓▓▓▓▓▓▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "  ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓█████████▓▓▓▓▓▓▓▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "  ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓████████▓▓▓▓▓▓▓▓▓▓▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "  ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓███████▓▓▓▓▓▓▓▓▓▓▓▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
    "  ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓███████▓▓▓▓▓▓▓▓▓▓▓▓▒░░░░░░░░░░░░░░▒▒▒▒░░░░░░░░░░░\n"
    "  ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓█████▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░░░░░▒▒▒▓▓▓▒▒▒░░░░░░░░\n"
    "  ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓██████▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒░░░░░░░░░▒▒▒▓▓▓▓▓▓▓▒▒░░░░░░░\n"
    "  ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓██████▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒░░░░░░░░▒▒▓▓▓▓▓▓▓▓▓▓▓▒▒░░░░\n"
    "  ▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓███▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒▒░░░░░░░▒▒▒▓▓▓▓▓▓▓▓▓▓▓▒▒▒▒▒\n"
    "  ▓▓▓▒▓▓▒▒▒▒▒▒▓▓▓▓▒▓▒▒▒▒▒▒▒▓████▓█▓▓▓▓█▓▓▓▓▓▓▓▓▓▓██▓▓▓▒▒▒▒▒▒▒▒▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒▒▒▒\n"
    "  ▓▓▓▓▓▓▓▓▓▓▓▓▓█▓██▓▓▓▓▓▓▓▓▓▓▓▓▓███▓▓█▓▓█▓▓▓▓▓▓▓▓███▓▓▒▒▒▒▒▒▒▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒▒\n"
    "  ▓█▓███████████████▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓█████▓▓▓▓█▓▓▓███▓▓▓▒▒▒▒▒▓▓▓▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒▒\n"
    "  ████████████████████▓▓▓▓▓▓▓▓▓▓▓▓█████████████▓█████▓▓▓▓▒▒▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒\n"
    "  █████████████████████▓▓▓██▓▓▓▓██████████▓█████▓██▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒▒\n"
    "  ████████████▓▓███▓▓▓███▓▓███▓▓▓▓████▓▓▓███▓▓▓█▒▒▒▒▒▒▒▓█▓▓▓▓▒▒░░░░░▓▓▓▒░░░░▓▓▓▓▓\n"
    "  ███▒▒▒██████▒▒▒██░░░▓▓▓░░█▓▓░░▒░████░░░░▓█▒░▒█░░░░░░░▓▓▓▓▓▓▒░░░░▒▒▓▒░░░▓▒░░░▓▓▓\n"
    "  ███▒▒▒███████▒░▒▓░░░░█░░▓█▓░░▓░░▒███░░▒░░▒▒░▒█▓█▒░░▓▓▓▓▓▓▓▓▓▓▒░░▓▓▓░░░▒▓▓▒░░▒▓▓\n"
    "  ███▒▒▒███████▓░░▒░█░░░░▒█▓░░░░░░░▒██░░▒█░░░░▒█▓█▒░░▓▓▓▓▓▓▓▓▓▓▒░░▓▓▓▒░░░▓▓░░░▒▓▓\n"
    "  ███▒▒▒████████░▒▒▒█▓░▒░██▓░░▓▓▓▒░░▓█░░▒██▒░░▒█▓█▒░░▓█▓▓▓▓▓▓▓▓▒░░▓▓▓▓▒░░░░░░▓▓▓▓\n"
    "  ███▓█████████████████████████████████████████████████▓▓▓▓▓▓█████▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓\n"
    "  ███████████████████████████████████████████████▓▓▓██▓▓▓██▓▓▓▓█▓▒▒▒▒▒▒▒▓▓▓▓▓▓▓▓▓\n"
    "  ██████████▒▒▒▒▒▒▒░██▒░░░░░░░█▓░░▒█████░░░▓█░░░░░░░▒█▒░░▒▓▓░░▒█░░░░░░░▒█▓▓▓▓▓▓▓▓\n"
    "  ██████████▒▒▒██▒░░▓█▒░░▓▓▓▓██▓░░▒█████░░░▓█▒░░▒▒▒▒█▓▓░░░▓▒░░█▓░░░░░░░████▓▓▓▓▓▓\n"
    "  ██████████▒▒▒░░░░░▓█▒░░░░░▒██▓░░░█████░░░▓█▒░░▒▒▒▓█▓█▓░░░░░▓▓▓░░░▓█████▓▓▓▓▓▓▓▓\n"
    "  ██████████▒▒▒█▓▒░░▒█▒░░▒▒▒▒▒█▓░░░▒▒░▒█░░░▓█▒░░░░░░▒███▒░░░▒█▓▓░░░░░░░▒███▓▓▓███\n"
    "  ██████████▒▒▒▒▒▒▒▓██▒▒▒▒▒▒▒▒██▒▒▒▒▒▒▒█▓▓▓▓█▓▓▓▓▓▓▓█████████████████████▓▓▓▓████\n"
    "  ██████████████████████████████████████████████████████▓▓██▓▓▓▓▓▓█▓█▓██▓████████\n"
		"\n");
}

void __gfx_cyphercon_badge(const struct shell *shell){
  shell_print(shell, "\n"
  "  ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░░░░░░░░▓███▓▓▒▒▒▒▒\n"
  "  ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▒▒▒▒▓░░░░░░░▒▓█▓▓▒▒▒▒▒▒▒▒\n"
  "  ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░▒█▓▓███▓█▒░▓▓██▓▓▓▒▒▒▒▒▒▒▒\n"
  "  ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░░░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▒▓█▓███▓▓▓▓▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒\n"
  "  ▒▒▒░░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒\n"
  "  ▒▒▒▒▒▒▒░▒▒▒▒▒▒▒▒▒░░░░░░▒░░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▒▒▒▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  "  ▒▒▒▒▒▒▒▒▒▒▒▒░░░░░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░░▒▒▒▒▒▒░░░▒▒▒▓▒▒▒██▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  "  ▒▒▒▒▒▒▒▒▒▒▒▒░░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░▒█▒░▒▒▓░░░░░░░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  "  ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░▓█▓░░░░░░░░░░░░░░░░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  "  ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░▒█▓░░░▓░░░░░░░░░░░░░▒▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  "  ▒▒▒▒▒░░░▒░▒▒▒▒▒░░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░▒██▒░░░░▓▓▓░▒▓░░░░░░░░▒▓▒▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  "  ▒▒▒▒▒▒░▒▒▒▒▒▒▒▒▒▒▒▒▒░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░░██████░▒▓░░░░░▒░░▓░░░░░░░░░░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  "  ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░▒▒▒▒▒▒▒▒▒▒▒▒░███████████▒▒▓░░░░░░█░░▓░░░░░░░░░░░░░░░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  "  ░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░▓███████████████▒█▓░░░░▓▓░░█▒░░░░░░░░░░░░░░░▒▒▒▒▒▒▒▒▒▒▒▒\n"
  "  ▒▒▒▒▒░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░█████▒█████████████████▓░░░░▒▒░█▒░░░░░░░░░░░░░░░▒▓▒▒▒▒▒▒▒▒\n"
  "  ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░███▒░▓████▒██████████████████░░░░░░▓░░█░░░░░░░░█▓▓▒▒▒▒▒▒▒▒▒▒\n"
  "  ░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░█▓▒░▒██████▓█▓█▒██████████████████░░░░░░░▒▓░░░░░░▓█▒▒▒▒▓▓▓▒▒▒▒\n"
  "  ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░░▒▓█▒▒░░░░███▓██░█▓█▓███████████████▒▓▓░░░░▒▒░▓▒░░░▓█▒░▓▓▓▓▒▓▒▒▒\n"
  "  █▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░░░░░░░▒▒█░░░░░░█▓▓░▓▓█▒▓▓▓███████████████▒█▓░░░░░░░▒█▓▓▒███▓▒▒▒▓▓▓\n"
  "  ████▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░░░░░░░░░░░░▓▒█░░░░▒▒██▓▓█▓▒█▓█▓██████████████████▒░░██▓▒▓██▓▓▓████▓▒\n"
  "  ██████▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░░░░░░░░░░░░░░░░░▓▓▒▒░▒░▒▓▓▒█▓█▓█▒█▒████████████████████▒▒███▓▓▓▓▓▓▒▓▓▓\n"
  "  █████████▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░░░░░░░░░░░░░░░░░░░░░░▓▓▒▒░░░▒▓▓▓▒▓█▒▓██▓██████████████▓▓▒███▓▓▓▓▒▒▒▒▒▒▒▒\n"
  "  ████▓▓██████▒▒▒▒▒▒▒▒▒▒▒▒░░░░░░░░░░░░▒░░░░░░░░░░░░░▒▓█▒▒░░░▒▓▓▓▒▓██░▓▓▒███████████▒▓██▓▓▓▓▓▓▒▒▒▒▒▒▒▒\n"
  "  ███████████████▒▒▒▒▒▒▒░░▒░░░░░░░▒░░▒▒▓▓░░░░░░░░░░░░░░▒▒█▒░░░░░█▓▓▓▓█▒███▒██████▓▒███▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒\n"
  "  █▓███████████████▓▓▒▒░░░░░░░░░░░░░░▓▓▓▒▒▒▓░░░░░░░░░░░░░░▓▒▓▒▒░░▒░█▓▓████████▓▓▒███▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  "  █████████████████▓█▓░░░▒░░░░░░░░░░░░░░▓▓▓░▓▓░░░░░░░░░░░░░░░▓▒▓▒░░░▒░████████▒▒███▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  "  █████████████████▓░░░░░░░░▒░░░░░░░░░░░░░░▓░▓▒░▓░░░░░░░░░░░░░░▒▓▓░▒░░░▒▒███▓▒███▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  "  ████████████████░░░░░░░░░░░░▒░░░░░░░░░░░░░░▒▓▓▓▓▓▓░░░░░░░░░░░░░░▒▒░░▒▓███▒▒██▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  "  ██████████████░░░░░░░░█▒░░░░░░░░░░░░░░░░░░░░░░▓▓▓▒▒▓▓░░░░░░░░░░░░░░▒▓██▓▒███▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  "  ▓██████████▓░░░░░░░░░█░░░░░░░░░░░░░░░░░░░░░░░░░░░▓▓▓▓▓▓▓░░░░░░░░░░░░░▓▒███▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  "  ▒▒▓███████░░░░░░░░▒░░░░░░░░░░░░░░░░░▒░░░░░░░░░░░░░░▓▓▓▓░░░░░░░░░░░▓▒▓▒███▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  "  ▒▒▒▒▒███░░░░░░░░░█░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░▒▒░░░░░░░░░░░▓▒███▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  "  ▒▒▒▒▒░░░░░░░░░░▒▒░░░░░▒▓░░░░░░░░░░░░░░░░░▒░░░░░░░░░░░░░░░░░░░░░░█▒▒██▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  "  ▒▒▒▒▒░░░░░░░░█░░░░░░░░░░░█░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░▒▓▒███▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  "  ▒▒▒▒▒▒▒░░░░░░░░░░░░░░░░░░░░▒▓░░░░░░░░░░▒░░░░░░░░░░░░░░░░░░░░░█▒███▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  "  ░░▒▒▒▒▒▒▒▒░░░░░░░░░░░░░░░░░░░░█░░░░░░░▒░░░░░░░░░░▒░░░░░░░░░▓▒▒███▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  "  ▒▒▒░▒▒░▒░░▒▒░░░░░░░░░░░░░░░░░░░░▒█░░░░▓░░░░░░░▒▒░░░░░░░░░▒▓▒███▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  "  ░░░░▒▒░▒▒░▒▒▒▒░░░░░░░░░░░░░░░░░░░░░█░░▓░░░░░▒░▒▓▒░░░░░▒░█▒▓███▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  "  ▒▒░▒░░░░░▒▒▒▒▒▒▒▒░░░░░░░░░░░░░░░░░░░░▓▒░░░░░░░▒▒░░░▒░░▒▓▒███▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  "  ░▒░░░░░░░░░▒▒░▒▒▒▒▒░░░░░░░░░░░▒▓▓░░░░░░░░▒▒▒▒▒▒░░░▓░░█████▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  "  ▒░░▒░░░░░░▒▒░▒▒▒▒▒▒▒▒░░░░░░░░░░░░░░░░░░▒░▓▓▒░░░░░▓░████████▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  "  ░░░░░░░░▒░░░▒▒▒▒▒▒▒▒▒▒▒▒░░░░░░░░░░░░░░░░▒▒▓░░░░░▒▒████████████▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  "  ░░░░░░░░░▒▒░▒▒▒▒▒▒▒▒▒▒▒▒▒▒░░░░░░░░░▒░░░▒▒▒░░▓▒░░████████████████▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  "  ░░▒░░░░░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░░░░░░░░▒▒▒░░░▓▒░▒▓███████████████████▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  "  ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░░░░░░░░░▒░▒▒░█████████████████████████▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  "  ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░░░░░░░▒░█████████████████████████████▓▓▓▓▒▓▒▒▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  "  ░▒░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░░░░░▓█████████████████████████████████▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  "  ░░░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░░░░████████████████████████████████████▓▓▓▓▓▒▒▓▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
  "\n");
}

void __gfx_game_win(const struct shell *shell){
  shell_print(shell, "\n"
	"                                    ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░                                    \n"
	"                                    ░░░░░░░▒▒▓▓███████▓▓▒▒░░░░░░░                                    \n"
	"                            ░░░░░░░░▒▓█████████████████████████▓▒░░░░░░░░                            \n"
	"                            ░░░▒▓███████████████████████████████████▓░░░░                            \n"
	"                       ░░░░░▒███████████████████████████████████████████▒░░░░░                       \n"
	"                       ░░▒█████████████████████████████████████████████████░░░                       \n"
	"                   ░░░░██████████████████████▓▒░░░░░░░░▒▒▓████████████████████░░░░                   \n"
	"                   ░░████████████████████▒░░░░░░░░░░░░░░░░░░░▓██████████████████░░                   \n"
	"               ░░░░▓██████████████████▓░░░░░░░░░░░░░░░░░░░░░░░░▒█████████████████▓░░░░               \n"
	"               ░░░███████████████████▒░░░░░░░░░░░░░░░░░░░░░░░░░░░▓█████████████████░░░               \n"
	"           ░░░░░▒██████████████████▓░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░▒█████████████████▒░░░░░           \n"
	"           ░░░░▓██████████████████▓░░   ░░▓█████▒░░░░░░▓█████▓░░░░░░█████████████████▓░░░░           \n"
	"           ░░░▓███████▒▒▓█████████░░░   ░▒███████░░░░░▒███████▒░░░░░▒█████████████████▓░░░           \n"
	"           ░░▓█████▓░░░░░░░██████▒░░░   ░░██████▓░░░░░░▓█████▓░░░░  ░▓█████████▓▒▓█████▓░░           \n"
	"       ░░░░░▒██████▒░░░░░░░▒█████░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░▒███████░░░░░░▒████▒░░░░░       \n"
	"       ░░░░░███████▓░░░░░░░██████░░░░░░▒░░░░░░░░░░░░░░░░░░░░░░░▒▒░░░░▒██████▓░   ░░░█████░░░░░       \n"
	"       ░░░░▓█████▒░░░░░░░░░▒█████░░░░▒██▓░░░░░░░░░░░░░░░░░░░░░▒██▒░░░▒███████░░░░░░░█████▓░░░░       \n"
	"       ░░░░█████░░░░   ░░░░░░░░▓█░░░░░▒█▒░░░░░░░░░░░░░░░░░░░░░░█▓░░░░▓█████▓▒░░░░░░░░░▓███░░░░       \n"
	"       ░░░▒████▓░░░░░░░░░░░░░░░░░░░░░░░▓█░░░░░░░░░░░░░░░░░░░░░██░░░░▒██▓▒░░░ ░░░░░░░░░░▒██▒░░░       \n"
	"        ░░▓█████▒░░░░░▒█▓▒░░░       ░░░░▒█▓░░░░░░░░░░░░░░░░░▒█▓░░░░░░░░░░    ░░░░░   ░░░██▓░░░       \n"
	"        ░░█████████▓████████▓░░░░░░░░░░░░░▓█▓░░░░░░░░░░░░░▓█▓░░░░░░░░░░░░░░░░▒███░░░░░░▓███░░░       \n"
	"        ░░█████████████████████▓░░░░░   ░░░░▓██▓▒░░░░░▒▓██▓░░░░░░   ░░░░░░▓███████▓░░▒█████░░░       \n"
	"       ░░░█████████████████████████▒░░░░░░░░░░░░░▒▒▒▒▒░░░░░░░░░░░░░░░░▓████████████████████░░░       \n"
	"        ░░▓███████████████████████████▓░░░░░░       ░░░░░░░░░░░░░░▓███████████████████████▓░░        \n"
	"       ░░░▒███████████████████████████████▒░░░░░░░░░░░░░▒▓░░░░▓███████████████████████████▒░░░       \n"
	"        ░░░██████████████████████████████████▓░░░░░░░░░░░░▒▓██████████████████████████████░░░░       \n"
	"       ░░░░▓████████████████████████████▓░░░░░░▓▓▒░░░░░░░░░░░░▒██████████████████████████▓░░░░       \n"
	"       ░░░░░████████████████████████▓▒░░░   ░░░░░▒▓██▒░░░░░░░░░░░░▓██████████████████████░░░░░       \n"
	"       ░░░░░▒████████▓▒▒▒▓██████▓▒░░░░░░░░░░░░▓█████████▓░░░░░░░░░░░░▒███████▓▓█████████▒░░░░░       \n"
	"           ░░▓██████░░░░░░░▓▓▒░░░   ░░░░░░▓█████████████████▒░░░░   ░░░░░▓▓░░░░░░▓█████▓░░           \n"
	"           ░░░▓████▒░░░░░░░░░░░░░░░░░▒▓████████████████████████▓░░░░░░░░░░░░░░░░░░████▓░░░           \n"
	"           ░░░░▓████░░░░    ░░░░░░▓████████████████████████████████▒░░░░░░░░░░░░░▓███▓░░░░           \n"
	"           ░░░░░▒████▓▒░░░░░░░▓███████████████████████████████████████▓░░░░░░▓██████▒░░░░░           \n"
	"               ░░░█████▓░░░░░░░████████████████████████████████████████░░    ░█████░░░               \n"
	"               ░░░░████▒░░░░░░░▓███████████████████████████████████████░░    ░████░░░░               \n"
	"                   ░▒██▓░░░░░░░████████████████████████████████████████▒░░░░░▓██░░                   \n"
	"                   ░░░▒███▒▒▒▓███████████████████████████████████████████▓▓███▒░░░                   \n"
	"                       ░░▒█████████████████████████████████████████████████▒░░                       \n"
	"                       ░░░░░▓███████████████████████████████████████████▓░░░░░                       \n"
	"                            ░░░▒█████████████████████████████████████▒░░░                            \n"
	"                            ░░░░░░░▒▓███████████████████████████▓░░░░░░░░                            \n"
	"                                    ░░░░░░▒▒▓▓█████████▓▓▒▒░░░░░░                                    \n"
	"                                    ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░                                    \n"
	"\n");
}