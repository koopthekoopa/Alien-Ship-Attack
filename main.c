#include <gb/gb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rand.h>
#include "title_data.c"
#include "title_map.c"
#include "bg_data.c"
#include "bg_map.c"
#include "gameover_data.c"
#include "gameover_map.c"
#include "logo_data.c"
#include "meta_sprites.c"
#include "logo_map.c"
#include "player.c"
UINT8 i;
UBYTE hasdinojustmovedy,apressed,running,gameover,splashscreen;
UINT8 dinomanimationframe,obstanimationframe,lastspriteid, currentBeat, skipgeneratingobstacles,speed;
UINT16 lastscreenquadrantrendered,currentscreenquadrant,nextscene,screenpixeloffset, laststarttime, timerCounter;
INT8 jumpindex,lastobstacleindex;

GameCharacter ship;
GameCharacter alien;
UBYTE spritesize = 8;

void performantdelay(UINT8 numloops){
    UINT8 i;
    for(i = 0; i < numloops; i++){
        wait_vbl_done();
    }     
}

UBYTE checkcollisions(GameCharacter* one, GameCharacter* two){
    return (one->x >= two->x && one->x <= two->x + two->width) && (one->y >= two->y && one->y <= two->y + two->height) || (two->x >= one->x && two->x <= one->x + one->width) && (two->y >= one->y && two->y <= one->y + one->height);
}

void movegamecharacter(GameCharacter* character, UINT8 x, UINT8 y){
    move_sprite(character->spritids[0], x, y);
    move_sprite(character->spritids[1], x + spritesize, y);
    move_sprite(character->spritids[2], x, y + spritesize);
    move_sprite(character->spritids[3], x + spritesize, y + spritesize);
}

void setupship(){
    ship.x = 80;
    ship.y = 130;
    ship.width = 16;
    ship.height = 16;

    // load sprites for ship
    set_sprite_tile(0, 0);
    ship.spritids[0] = 0;
    set_sprite_tile(1, 1);
    ship.spritids[1] = 1;
    set_sprite_tile(2, 2);
    ship.spritids[2] = 2;
    set_sprite_tile(3, 3);
    ship.spritids[3] = 3;

    movegamecharacter(&ship, ship.x, ship.y);
}

void playgameover(){
	//Sound : Game Over sound
	NR10_REG = 0x7D; //or 1E or 1D for louder sound / 2E / 3E / 4E... for more "vibe"
	NR11_REG = 0xBF;
	NR12_REG = 0xF6; //B7, C7, D7...F7 for longer sound
	NR13_REG = 0x7C;
	NR14_REG = 0x86;
}

void setupalien(){
    alien.x = 30;
    alien.y = 0;
    alien.width = 16;
    alien.height = 16;    

    // load sprites for bug
    set_sprite_tile(4, 4);
    alien.spritids[0] = 4;
    set_sprite_tile(5, 5);
    alien.spritids[1] = 5;
    set_sprite_tile(6, 6);
    alien.spritids[2] = 6;
    set_sprite_tile(7, 7);
    alien.spritids[3] = 7;

    movegamecharacter(&alien, alien.x, alien.y);
}

void playmusicnext();
void fadeout();
void fadein();

void bgsplashdraw1(){
    DISPLAY_OFF;
    set_bkg_data(0, 114, logo_data);
    set_bkg_tiles(0, 0, 20, 18, logo_map);

    SHOW_BKG;
    DISPLAY_ON;
}

bgsplashdraw2(){
    DISPLAY_OFF;
    set_bkg_data(0, 114, title_data);
    set_bkg_tiles(0, 0, 20, 18, title_map);

    SHOW_BKG;
    DISPLAY_ON;
}

void enablesound(){
	// turn on sound
	NR52_REG = 0x80; // turn on sound registers

	// there are 8 buts in this order, a 1 enables that chanel a 0 disables
	// 7	Channel 4 to Main SO2 output level control (Left)
	// 6	Channel 3 to Main SO2 output level control (Left)
	// 5	Channel 2 to Main SO2 output level control (Left)
	// 4	Channel 1 to Main SO2 output level control (Left)
	// 3	Channel 4 to Main SO1 output level control (Right)
	// 2	Channel 3 to Main SO1 output level control (Right)
	// 1	Channel 2 to Main SO1 output level control (Right)
	// 0	Channel 1 to Main SO1 output level control (Right)
	// so if you construct chanel 1 on as 0001 0001 (left and right chanel) which ix x11 in hex


	NR51_REG = 0xFF; // set all chanels


	// NR50 controls volume
	// again 8 bytes
	// 7	Output Vin to Main SO2 output level control (1: on; 0: off) LEAVE ALONE
	// 6-4	SO2 (Left) Main Output level (volume)
}

void startgame(){
    fadeout();

    game();
}

void game(){
    DISPLAY_OFF;
    set_sprite_data(0, 8, Sprites);
    setupship();
    setupalien();
    set_bkg_data(0, 114, bg_data);
    set_bkg_tiles(0, 0, 20, 18, bg_map);

    SHOW_BKG;
    SHOW_SPRITES;
    DISPLAY_ON;
    fadein();
    while(!checkcollisions(&ship, &alien)){
       if(joypad() & J_LEFT){
           movesound();
           ship.x -= 4;
           movegamecharacter(&ship, ship.x, ship.y);
       }
       if(joypad() & J_RIGHT){
           movesound();
           ship.x += 4;
           movegamecharacter(&ship, ship.x, ship.y);
       }
           alienactivate();

       movegamecharacter(&alien,alien.x,alien.y);

       performantdelay(5);      
    }

    gameoverscreen();
}

void spawnalien(){
	NR10_REG = 0x16; 
	NR11_REG = 0x10;
	NR12_REG = 0x73;
	NR13_REG = 0x00;
	NR14_REG = 0x83;
	NR13_REG = 0x00;
	NR12_REG = 0x73;
}

void alienactivate(){

       alien.y += 12;

       if(alien.y >= 144){
           alien.y=0;
           alien.x = ship.x;
           spawnalien();
       }
}


void movesound(){
	NR41_REG = 0x01;
	NR42_REG = 0x11;
	NR43_REG = 0x00;
	NR44_REG = 0xC0;
}

void gameoverscreen(){
    HIDE_SPRITES;
    playgameover();
    delay(500);
    fadeout();
    drawgameover();
    fadein();

    waitpad(J_A|J_B|J_SELECT|J_START);
    DISPLAY_ON;
    SHOW_SPRITES;
    startgame();
}

void drawgameover(){
    set_bkg_data(0, 114, gameover_data);
    set_bkg_tiles(0, 0, 20, 18, gameover_map);
}

void load(){
	enablesound();
	wait_vbl_done();
        bgsplashdraw1();
	delay(2800);
        fadeout();
        bgsplashdraw2();
        fadein();

	// set music playing in bg
 	disable_interrupts();
    add_TIM(playmusicnext);
    enable_interrupts();
    TAC_REG = 0x06; // Not sure what this actually does but it overrides a default for the timer I think
    set_interrupts(TIM_IFLAG|VBL_IFLAG);

	// wait for any of these buttons to be pressed
	waitpad(J_A|J_B|J_SELECT|J_START);
	
	
	// remove music time interupt handler
	disable_interrupts();
	remove_TIM(playmusicnext);
        startgame();		
}

void main(){
    load();
}

void fadeout(){
	for(i=0;i<4;i++){
		switch(i){
			case 0:
				BGP_REG = 0xE4;
				break;
			case 1:
				BGP_REG = 0xF9;
				break;
			case 2:
				BGP_REG = 0xFE;
				break;
			case 3:
				BGP_REG = 0xFF;	
				break;						
		}
		delay(100);
	}
}

void fadein(){
	for(i=0;i<3;i++){
		switch(i){
			case 0:
				BGP_REG = 0xFE;
				break;
			case 1:
				BGP_REG = 0xF9;
				break;
			case 2:
				BGP_REG = 0xE4;
				break;														
		}
		delay(100);
	}
}

//Define note names
typedef enum {
  C3, Cd3, D3, Dd3, E3, F3, Fd3, G3, Gd3, A3, Ad3, B3,
  C4, Cd4, D4, Dd4, E4, F4, Fd4, G4, Gd4, A4, Ad4, B4,
  C5, Cd5, D5, Dd5, E5, F5, Fd5, G5, Gd5, A5, Ad5, B5,
  C6, Cd6, D6, Dd6, E6, F6, Fd6, G6, Gd6, A6, Ad6, B6,
  C7, Cd7, D7, Dd7, E7, F7, Fd7, G7, Gd7, A7, Ad7, B7,
  C8, Cd8, D8, Dd8, E8, F8, Fd8, G8, Gd8, A8, Ad8, B8,
  SILENCE
} pitch;

const UWORD frequencies[] = { //values based on a formula used by the GB processor
  44, 156, 262, 363, 457, 547, 631, 710, 786, 854, 923, 986,
  1046, 1102, 1155, 1205, 1253, 1297, 1339, 1379, 1417, 1452, 1486, 1517,
  1546, 1575, 1602, 1627, 1650, 1673, 1694, 1714, 1732, 1750, 1767, 1783,
  1798, 1812, 1825, 1837, 1849, 1860, 1871, 1881, 1890, 1899, 1907, 1915,
  1923, 1930, 1936, 1943, 1949, 1954, 1959, 1964, 1969, 1974, 1978, 1982,
  1985, 1988, 1992, 1995, 1998, 2001, 2004, 2006, 2009, 2011, 2013, 2015,
  0
};

//Define Instrument names
//Instruments should be confined to one channel
//due to different registers used between ch1, 2, 3, 4
typedef enum {
	NONE,
	MELODY,  //channel 1
	HARMONY, //channel 1
	SNARE,   //channel 4
	CYMBAL   //channel 4
} instrument;

//Define a note as having a pitch, instrument, and volume envelope
typedef struct {
	instrument i;
	pitch p;
	UBYTE env;
} note;


//function to set sound registers based on notes chosen
void setNote(note *n){
	switch((*n).i){
		case MELODY:
			NR10_REG = 0x00U; //pitch sweep
			NR11_REG = 0x84U; //wave duty
			NR12_REG = (*n).env; //envelope
			NR13_REG = (UBYTE)frequencies[(*n).p]; //low bits of frequency
			NR14_REG = 0x80U | ((UWORD)frequencies[(*n).p]>>8); //high bits of frequency (and sound reset)
		break;
	}
}

note song_intro[40] = { //notes to be played on channel 1
	{NONE, SILENCE, 0x00U},
        {MELODY, D5, 0x83U},
	{MELODY, E5, 0x83U},
	{MELODY, G5, 0x83U},
	{NONE, SILENCE, 0x00U},
        {MELODY, G5, 0x83U},
	{MELODY, E5, 0x83U},
	{MELODY, D5, 0x83U},
	{NONE, SILENCE, 0x00U},
	{MELODY, B6, 0x83U},
	{MELODY, B6, 0x83U},
	{MELODY, A6, 0x83U},
	{MELODY, G6, 0x83U},
	{NONE, SILENCE, 0x00U},
    {MELODY, G5, 0x83U},
	{MELODY, E5, 0x83U},
	{MELODY, D5, 0x83U},
	{NONE, SILENCE, 0x00U},
	{MELODY, D6, 0x83U},	
	{MELODY, Dd6, 0x83U},
	{MELODY, E6, 0x83U},
	{NONE, SILENCE, 0x00U},
	{MELODY, E4, 0x77U},
	{NONE, SILENCE, 0x00U},
	{MELODY, E4, 0x77U},
	{NONE, SILENCE, 0x00U},
	{MELODY, E4, 0x77U},
	{NONE, SILENCE, 0x00U},
        {MELODY, D5, 0x83U},
	{MELODY, E5, 0x83U},
	{MELODY, G5, 0x83U},
	{NONE, SILENCE, 0x00U},
	{MELODY, B6, 0x83U},
};

void playChannel1(){
	setNote(&song_intro[currentBeat]);
	NR51_REG |= 0x11U; //enable sound on channel 1
}

void playmusicnext(){
	if (timerCounter == 52){ // as on splashscreen and CPU looping fast only play every 350 cycles
		timerCounter=0;
		currentBeat = currentBeat == 40 ? 0 : currentBeat+1;
		playChannel1(); //every beat, play the sound for that beat
	}
	timerCounter++;
}