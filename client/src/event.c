/*
    Daimonin SDL client, a client program for the Daimonin MMORPG.


  Copyright (C) 2003 Michael Toennies

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    The author can be reached via e-mail to daimonin@nord-com.net
*/

#include "include.h"
#include "textwin.h"

extern char d_ServerName[2048];
extern int  d_ServerPort;

static int get_action_keycode,drop_action_keycode; /* thats the key for G'et command from keybind */

_keymap keymap[MAX_KEYMAP]; /* thats the one and only key bind table*/
typedef struct _key_macro
{
        char macro[64]; /* the macro*/
        char cmd[64]; /* our cmd string*/
        int internal;/*intern: Use this function to generate the cmd*/
        int value;/* a default value for cmds*/
        int mode;/* the default send mode*/
        int menu_mode;
} _key_macro;


typedef struct _keys
{
        Boolean pressed; /*true: key is pressed*/
        uint32 time; /*tick time last repeat is initiated*/
} _keys;
static _keys keys[MAX_KEYS];

enum {
        KEYFUNC_NO,KEYFUNC_RUN, KEYFUNC_MOVE,
        KEYFUNC_CONSOLE, KEYFUNC_CURSOR, KEYFUNC_RANGE,
        KEYFUNC_APPLY, KEYFUNC_DROP, KEYFUNC_GET,
        KEYFUNC_LOCK,KEYFUNC_MARK, KEYFUNC_EXAMINE,
        KEYFUNC_SLIST, KEYFUNC_KEYBIND,KEYFUNC_STATUS,
        KEYFUNC_PAGEUP, KEYFUNC_PAGEDOWN,
        KEYFUNC_MENU_UP,KEYFUNC_MENU_DOWN,
        KEYFUNC_KB_NEW,KEYFUNC_KB_EDIT,KEYFUNC_KB_DONE,
        KEYFUNC_KB_REPEAT,KEYFUNC_SPELL_CURRIGHT,
        KEYFUNC_SKILL,KEYFUNC_SPELL_CURLEFT,
        KEYFUNC_SKILL_CURLEFT,KEYFUNC_SKILL_CURRIGHT,
        KEYFUNC_SKILL_CURUP,KEYFUNC_SKILL_CURDOWN,
        KEYFUNC_SPELL_CURUP,KEYFUNC_SPELL_CURDOWN,
        KEYFUNC_SPELL_G1,        KEYFUNC_SPELL_G2,        KEYFUNC_SPELL_G3,
        KEYFUNC_SPELL_G4,        KEYFUNC_SPELL_G5,        KEYFUNC_SPELL_G6,   
        KEYFUNC_SPELL_G7,        KEYFUNC_SPELL_G8,        KEYFUNC_SPELL_G9,
        KEYFUNC_SPELL_G0,KEYFUNC_FIREREADY,KEYFUNC_SPELL_RETURN,
        KEYFUNC_SKILL_RETURN,
        KEYFUNC_LAYER0,		KEYFUNC_LAYER1,		KEYFUNC_LAYER2,		KEYFUNC_LAYER3,
        KEYFUNC_HELP,
        KEYFUNC_PAGEUP_TOP, KEYFUNC_PAGEDOWN_TOP,
        KEYFUNC_TARGET_ENEMY, KEYFUNC_TARGET_FRIEND, KEYFUNC_TARGET_SELF,
        KEYFUNC_COMBAT,
};


_key_macro defkey_macro[] =
{
        {"?M_SOUTHWEST","southwest", KEYFUNC_MOVE,  1, SC_NORMAL, MENU_NO},
        {"?M_SOUTH",	"south",	 KEYFUNC_MOVE,  2, SC_NORMAL, MENU_NO},
        {"?M_SOUTHEAST","southeast", KEYFUNC_MOVE,  3, SC_NORMAL, MENU_NO},
        {"?M_WEST",		"west",		 KEYFUNC_MOVE,  4, SC_NORMAL, MENU_NO},
        {"?M_STAY",		"stay",		 KEYFUNC_MOVE,  5, SC_NORMAL, MENU_NO},
        {"?M_EAST",		"east",		 KEYFUNC_MOVE,  6, SC_NORMAL, MENU_NO},
        {"?M_NORTHWEST","northwest", KEYFUNC_MOVE,  7, SC_NORMAL, MENU_NO},
        {"?M_NORTH",	"north",	 KEYFUNC_MOVE,  8, SC_NORMAL, MENU_NO},
        {"?M_NORTHEAST","northeast", KEYFUNC_MOVE,  9, SC_NORMAL, MENU_NO},
        {"?M_RUN",		"run",		 KEYFUNC_RUN,    0, SC_NORMAL, MENU_NO},
        {"?M_CONSOLE",	"console",   KEYFUNC_CONSOLE,0, SC_NORMAL, MENU_NO},
        {"?M_UP",		"up",		 KEYFUNC_CURSOR,0, SC_NORMAL, MENU_NO},
        {"?M_DOWN",		"down",		 KEYFUNC_CURSOR,1, SC_NORMAL, MENU_NO},
        {"?M_LEFT",		"left",      KEYFUNC_CURSOR,2, SC_NORMAL, MENU_NO},
        {"?M_RIGHT",	"right",     KEYFUNC_CURSOR,3, SC_NORMAL, MENU_NO},
        {"?M_RANGE",	"toggle range",     KEYFUNC_RANGE,0, SC_NORMAL, MENU_NO},
        {"?M_APPLY",	"apply <tag>",     KEYFUNC_APPLY,0, SC_NORMAL, MENU_NO},
        {"?M_EXAMINE",	"examine <tag>",   KEYFUNC_EXAMINE,0, SC_NORMAL, MENU_NO},
        {"?M_DROP",		"drop <tag>",      KEYFUNC_DROP,0, SC_NORMAL, MENU_NO},
        {"?M_GET",		"get <tag>",       KEYFUNC_GET,0, SC_NORMAL, MENU_NO},
        {"?M_LOCK",		"lock <tag>",      KEYFUNC_LOCK,0, SC_NORMAL, MENU_NO},
        {"?M_MARK",		"mark<tag>",       KEYFUNC_MARK,0, SC_NORMAL, MENU_NO},
        {"?M_SPELL_LIST",	"spell list",  KEYFUNC_SLIST,0, SC_NORMAL, MENU_NO&MENU_KEYBIND&MENU_SLIST&MENU_STATUS},
        {"?M_KEYBIND",		"key bind",    KEYFUNC_KEYBIND,0, SC_NORMAL, MENU_NO&MENU_KEYBIND&MENU_SLIST&MENU_STATUS},
        {"?M_STATUS",		"status",      KEYFUNC_STATUS,0, SC_NORMAL, MENU_NO&MENU_KEYBIND&MENU_SLIST&MENU_STATUS},
        {"?M_SKILL_LIST",   "skill list",  KEYFUNC_SKILL,0, SC_NORMAL, MENU_NO&MENU_KEYBIND&MENU_SLIST&MENU_STATUS},
        {"?M_PAGEUP",		"scroll up",   KEYFUNC_PAGEUP,0, SC_NORMAL, MENU_NO},
        {"?M_PAGEDOWN",		"scroll down", KEYFUNC_PAGEDOWN,0, SC_NORMAL, MENU_NO},
        {"?M_FIRE_READY",		"fire_ready <tag>", KEYFUNC_FIREREADY,0, SC_NORMAL, MENU_NO},
        {"?M_LAYER0",		"l0", KEYFUNC_LAYER0,0, SC_NORMAL, MENU_NO},
        {"?M_LAYER1",		"l1", KEYFUNC_LAYER1,0, SC_NORMAL, MENU_NO},
        {"?M_LAYER2",		"l2", KEYFUNC_LAYER2,0, SC_NORMAL, MENU_NO},
        {"?M_LAYER3",		"l3", KEYFUNC_LAYER3,0, SC_NORMAL, MENU_NO},
        {"?M_HELP",			"show help", KEYFUNC_HELP,0, SC_NORMAL, MENU_NO},
        {"?M_PAGEUP_TOP",		"scroll up",   KEYFUNC_PAGEUP_TOP,0, SC_NORMAL, MENU_NO},
        {"?M_PAGEDOWN_TOP",		"scroll down", KEYFUNC_PAGEDOWN_TOP,0, SC_NORMAL, MENU_NO},
        {"?M_TARGET_ENEMY",		"/target enemy", KEYFUNC_TARGET_ENEMY,0, SC_NORMAL, MENU_NO},
        {"?M_TARGET_FRIEND",	"/target friend", KEYFUNC_TARGET_FRIEND,0, SC_NORMAL, MENU_NO},
        {"?M_TARGET_SELF",		"/target self", KEYFUNC_TARGET_SELF,0, SC_NORMAL, MENU_NO},
        {"?M_COMBAT_TOGGLE",	"/combat", KEYFUNC_COMBAT,0, SC_NORMAL, MENU_NO},
};

#define DEFAULT_KEYMAP_MACROS (sizeof(defkey_macro)/sizeof(struct _key_macro))

/* this is the macro list for the menus */
_key_macro menu_macro[] =
{
    {"?M_SLIST",	    "spell list",   KEYFUNC_SLIST,0, 0, MENU_NO&MENU_KEYBIND&MENU_SLIST&MENU_STATUS&MENU_SKILL},
    {"?M_KEYBIND",	    "key bind",     KEYFUNC_KEYBIND,0, 0, MENU_NO&MENU_KEYBIND&MENU_SLIST&MENU_STATUS&MENU_SKILL},
    {"?M_STATUS",	    "status",       KEYFUNC_STATUS,0, 0, MENU_NO&MENU_KEYBIND&MENU_SLIST&MENU_STATUS&MENU_SKILL},        
    {"?M_SKILL_LIST",   "skill list",     KEYFUNC_SKILL,0, 0, MENU_NO&MENU_KEYBIND&MENU_SLIST&MENU_STATUS&MENU_SKILL},
    
    {"?M_KB_CURUP",     "cursor up",    KEYFUNC_MENU_UP,  0, 0, MENU_KEYBIND},
    {"?M_KB_CURDOWN",   "cursor up",    KEYFUNC_MENU_DOWN,  0, 0, MENU_KEYBIND},

    {"?M_KB_NEW",	        "new macro",       KEYFUNC_KB_NEW,0, 0, MENU_KEYBIND},        
    {"?M_KB_EDIT",	        "edit macro",      KEYFUNC_KB_EDIT,0, 0, MENU_KEYBIND},        
    {"?M_KB_DONE",	        "keybind done",    KEYFUNC_KB_DONE,0, 0, MENU_KEYBIND},        
    {"?M_KB_REPEAT" ,   	"set repeat",      KEYFUNC_KB_REPEAT,0, 0, MENU_KEYBIND},        

    {"?M_SPELL_CURLEFT" ,   "left cur",      KEYFUNC_SPELL_CURLEFT,0, 0, MENU_SLIST},        
    {"?M_SPELL_CURRIGHT" ,  "right cur",    KEYFUNC_SPELL_CURRIGHT,0, 0, MENU_SLIST},        
    {"?M_SPELL_CURUP" ,     "up cur",      KEYFUNC_SPELL_CURUP,0, 0, MENU_SLIST},        
    {"?M_SPELL_CURDOWN" ,   "down cur",    KEYFUNC_SPELL_CURDOWN,0, 0, MENU_SLIST},        
    
    {"?M_SPELL_G1" ,   "group1",    KEYFUNC_SPELL_G1,0, 0, MENU_SLIST},        
    {"?M_SPELL_G2" ,   "group2",    KEYFUNC_SPELL_G2,0, 0, MENU_SLIST},        
    {"?M_SPELL_G3" ,   "group3",    KEYFUNC_SPELL_G3,0, 0, MENU_SLIST},        
    {"?M_SPELL_G4" ,   "group4",    KEYFUNC_SPELL_G4,0, 0, MENU_SLIST},        
    {"?M_SPELL_G5" ,   "group5",    KEYFUNC_SPELL_G5,0, 0, MENU_SLIST},        
    {"?M_SPELL_G6" ,   "group1",    KEYFUNC_SPELL_G6,0, 0, MENU_SLIST},        
    {"?M_SPELL_G7" ,   "group7",    KEYFUNC_SPELL_G7,0, 0, MENU_SLIST},        
    {"?M_SPELL_G8" ,   "group8",    KEYFUNC_SPELL_G8,0, 0, MENU_SLIST},        
    {"?M_SPELL_G9" ,   "group9",    KEYFUNC_SPELL_G9,0, 0, MENU_SLIST},        
    {"?M_SPELL_G0" ,   "group0",    KEYFUNC_SPELL_G0,0, 0, MENU_SLIST},        
    {"?M_SPELL_RETURN" , "ready <spell>",    KEYFUNC_SPELL_RETURN,0, 0, MENU_SLIST},        
    {"?M_SKILL_RETURN" , "ready <skill>",    KEYFUNC_SKILL_RETURN,0, 0, MENU_SKILL},        
    
    
    {"?M_SKILL_CURLEFT" ,   "left cur",      KEYFUNC_SKILL_CURLEFT,0, 0, MENU_SKILL},        
    {"?M_SKILL_CURRIGHT" ,   "right cur",    KEYFUNC_SKILL_CURRIGHT,0, 0, MENU_SKILL},        
    {"?M_SKILL_CURUP" ,     "up cur",      KEYFUNC_SKILL_CURUP,0, 0, MENU_SKILL},        
    {"?M_SKILL_CURDOWN" ,   "down cur",    KEYFUNC_SKILL_CURDOWN,0, 0, MENU_SKILL},        
};

#define DEFAULT_MENU_KEYMAP_MACROS (sizeof(menu_macro)/sizeof(struct _key_macro))

_keymap menu_keymap[] =
{
    {"?M_KB_CURUP","xx",         SDLK_UP, TRUE,  0, MENU_KEYBIND},
    {"?M_KB_CURDOWN","xx",       SDLK_DOWN, TRUE,  0, MENU_KEYBIND},
    {"?M_KB_NEW","xx",           SDLK_n, FALSE,  0, MENU_KEYBIND},
    {"?M_KB_EDIT","xx",          SDLK_e, FALSE,  0, MENU_KEYBIND},
    {"?M_KB_DONE","xx",          SDLK_d, FALSE,  0, MENU_KEYBIND},
    {"?M_KB_REPEAT","xx",        SDLK_r, FALSE,  0, MENU_KEYBIND},
       
    {"?M_SPELL_CURLEFT","xx",         SDLK_LEFT, FALSE,  0, MENU_SLIST},
    {"?M_SPELL_CURRIGHT","xx",       SDLK_RIGHT, FALSE,  0, MENU_SLIST},
    {"?M_SPELL_CURUP","xx",         SDLK_UP, TRUE,  0, MENU_SLIST},
    {"?M_SPELL_CURDOWN","xx",       SDLK_DOWN, TRUE,  0, MENU_SLIST},

    {"?M_SPELL_G1","xx",       SDLK_1, FALSE,  0, MENU_SLIST},
    {"?M_SPELL_G2","xx",       SDLK_2, FALSE,  0, MENU_SLIST},
    {"?M_SPELL_G3","xx",       SDLK_3, FALSE,  0, MENU_SLIST},
    {"?M_SPELL_G4","xx",       SDLK_4, FALSE,  0, MENU_SLIST},
    {"?M_SPELL_G5","xx",       SDLK_5, FALSE,  0, MENU_SLIST},
    {"?M_SPELL_G6","xx",       SDLK_6, FALSE,  0, MENU_SLIST},
    {"?M_SPELL_G7","xx",       SDLK_7, FALSE,  0, MENU_SLIST},
    {"?M_SPELL_G8","xx",       SDLK_8, FALSE,  0, MENU_SLIST},
    {"?M_SPELL_G9","xx",       SDLK_9, FALSE,  0, MENU_SLIST},
    {"?M_SPELL_G0","xx",       SDLK_0, FALSE,  0, MENU_SLIST},
    {"?M_SPELL_RETURN","xx",       SDLK_RETURN, FALSE,  0, MENU_SLIST},
    {"?M_SKILL_RETURN","xx",       SDLK_RETURN, FALSE,  0, MENU_SKILL},
    

    {"?M_SKILL_CURLEFT","xx",         SDLK_LEFT, TRUE,  0, MENU_SKILL},
    {"?M_SKILL_CURRIGHT","xx",       SDLK_RIGHT, TRUE,  0, MENU_SKILL},
    {"?M_SKILL_CURUP","xx",         SDLK_UP, TRUE,  0, MENU_SKILL},
    {"?M_SKILL_CURDOWN","xx",       SDLK_DOWN, TRUE,  0, MENU_SKILL},
};

#define DEFAULT_MENU_KEYMAP (sizeof(menu_keymap)/sizeof(struct _keymap))


int KeyScanFlag; /* for debug/alpha , remove later */

int keymap_count=0;	/* how much keys we have in the the keymap...*/
int cursor_type = 0; 
uint32 key_repeat_time=35;
uint32 key_repeat_time_init=175;

/* cmds for fire/move/run - used from move_keys()*/
static char *directions[10] = {"null","/sw", "/s", "/se",
"/w", "/stay", "/e", "/nw", "/n", "/ne"};
static char *directions_name[10] = {"null","southwest", "south", "southeast",
"west", "stay", "east", "northwest", "north", "northeast"};
static char *directionsrun[10] = { "/run 0","/run 6","/run 5","/run 4","/run 7",\
"/run 5", "/run 3", "/run 8", "/run 1","/run 2"};
static char *directionsfire[10] = { "fire 0","fire 6","fire 5","fire 4","fire 7",\
"fire 0", "fire 3", "fire 8", "fire 1","fire 2"};

static int key_event(SDL_KeyboardEvent *key );
static void key_string_event(SDL_KeyboardEvent *key );
static void check_keys(int key);
static Boolean check_macro_keys(char *text);
static void move_keys(int num);
static void key_repeat(void);
static void cursor_keys(int num);
int key_meta_menu(SDL_KeyboardEvent *key );
void key_connection_event(SDL_KeyboardEvent *key );
void key_setstats_event(SDL_KeyboardEvent *key );
void key_setrace_event(SDL_KeyboardEvent *key );
static void check_menu_keys(int key, int scan);
static Boolean check_menu_macro_keys(char *text);
static Boolean process_menu_macro_keys(int id, int value);
static Boolean check_menu_macros(char *text);
static Boolean process_macro_keys(int id, int value);
static void quickslot_key(SDL_KeyboardEvent *key,int slot);


void init_keys(void)
{
        register int i;

        for(i=0;i<MAX_KEYS;i++)
        {
                keys[i].pressed = FALSE;
                keys[i].time = 0;
        }
        reset_keys();
}

void reset_keys(void)
{
	register int i;

	InputStringFlag=FALSE;
	InputStringEndFlag=FALSE;
	InputStringEscFlag=FALSE;

	for(i=0;i<MAX_KEYS;i++)
		keys[i].pressed = FALSE;
}


/******************************************************************
 x: mouse x-pos ; y: mouse y-pos
 ret: 0  if mousepointer is in the game-field.
     -1 if mousepointer is in a menue-field.
******************************************************************/
int mouseInPlayfield(x, y)
{
	x+=  45;
	y-= 127;
	if (x < 445){
		if ((y <  200) && (y+y+x > 400)) return -1; /* upper left */
		if ((y >= 200) && (y+y-x < 400)) return -1; /* lower left */
	}else{
  	x-=445;
		if ((y <  200) && (y+y > x))     return -1; /* upper right */
		if ((y >= 200) && (y+y+x < 845)) return -1; /* lower right */
	}
	return 0;
}

/******************************************************************
 val: -1 dont change.   0 dragging off.   1 dragging on.
 ret: 0 no dragging  1 still dragging
******************************************************************/
int draggingInvItem(int value){
	static int dragInvItem = 0;
	if (value > -1) dragInvItem =value;
	return dragInvItem;
}

/******************************************************************
 wait for user to input a numer.
******************************************************************/
static void mouse_InputNumber(){
	static int delta = 0;
	static int timeVal =1;
	int x,y;

	if (!(SDL_GetMouseState(&x, &y) & SDL_BUTTON(SDL_BUTTON_LEFT))){
		timeVal = 1;
		delta =0;
		return;
	}
	if (x <330 || x > 337 || y < 510 || delta++ & 15) return;
	if ( y > 518){ /* + */
		x = atoi(InputString)+ timeVal;
		if (x > cpl.nrof) x = cpl.nrof;
	}else{ /* - */
		x = atoi(InputString)- timeVal;
		if (x< 1) x =1;
	}
	sprintf(InputString, "%d", x);
	InputCount = strlen(InputString);
	timeVal+= (timeVal/8)+1;
}

/******************************************************************
 move our hero with mouse.
******************************************************************/
static void mouse_moveHero(){
	#define my_pos 8
	int x,y, tx, ty;
	static int delta = 0;
	if (delta++ & 7) return; /* dont move to fast */
	if (draggingInvItem(-1)) return; /* still dragging an item */
	if (cpl.menustatus != MENU_NO) return;
	if (!(SDL_GetMouseState(&x, &y) & SDL_BUTTON(SDL_BUTTON_LEFT))){
		delta =0;
		return;
	}
	/* textwin has high priority, so dont move if playfield is overlapping */
 	if (textwin_set.split_flag == TRUE && x > 538 
 	&&  y > 560-(textwin_set.split_size+textwin_set.top_size)*10 ) return;

	if (get_tile_position(x, y, &tx, &ty)) return;
	if (tx == my_pos){
		     if (ty == my_pos) process_macro_keys(KEYFUNC_MOVE, 5);
		else if (ty >  my_pos) process_macro_keys(KEYFUNC_MOVE, 2);
		else if (ty <  my_pos) process_macro_keys(KEYFUNC_MOVE, 8);
	}else  if (tx <  my_pos){
				 if (ty == my_pos) process_macro_keys(KEYFUNC_MOVE, 4);
		else if (ty >  my_pos) process_macro_keys(KEYFUNC_MOVE, 1);
		else if (ty <  my_pos) process_macro_keys(KEYFUNC_MOVE, 7);
	}else{ /* (x > my_pos) */
		     if (ty == my_pos) process_macro_keys(KEYFUNC_MOVE, 6);
		     if (ty <  my_pos) process_macro_keys(KEYFUNC_MOVE, 9);
		     if (ty >  my_pos) process_macro_keys(KEYFUNC_MOVE, 3);
	}
	#undef my_pos 
}

int Event_PollInputDevice(void)
{
	SDL_Event event;
	int x, y, done = 0;
	static int active_scrollbar = 0;	
	static int itemExamined  = 0; /* only print text once per dnd */
	static Uint32 Ticks= 0;

	if ((SDL_GetTicks() - Ticks > 10) || !Ticks){
		Ticks = SDL_GetTicks();
		if (GameStatus >= GAME_STATUS_PLAY){
			if (InputStringFlag && cpl.input_mode == INPUT_MODE_NUMBER)
				mouse_InputNumber();
			else
				if (!active_scrollbar && !cursor_type) mouse_moveHero();
		}
	}

	while ( SDL_PollEvent(&event) )
	{
		static int old_mouse_y =0;	
		x = event.motion.x;
		y = event.motion.y;
		switch (event.type)
		{
			case SDL_MOUSEBUTTONUP:
				if (GameStatus < GAME_STATUS_PLAY) break;
				active_scrollbar = 0; 
				cursor_type = 0;    				
				if (InputStringFlag && cpl.input_mode == INPUT_MODE_NUMBER) break;
				if (draggingInvItem(-1) ==1 || draggingInvItem(-1) ==3){ /* drag form IWIN_INV */
					if (draggingInvItem(-1) ==1)
					{
						if (mouseInPlayfield(x, y) || (y > 565 && x >265 && x < 529))	
						{
					     sound_play_effect(SOUND_DROP,0,0,100);
							process_macro_keys(KEYFUNC_DROP, 0);
						}
						else if (x < 223 && y < 300)
							process_macro_keys(KEYFUNC_APPLY, 0);
					}
					/* quickslots... */
					if (x >= SKIN_POS_QUICKSLOT_X && x < SKIN_POS_QUICKSLOT_X+282 &&
						y >= SKIN_POS_QUICKSLOT_Y && y<SKIN_POS_QUICKSLOT_Y+42)
					{
						int ind = get_quickslot(x,y);
						if(ind != -1) /* valid slot */
						{
							if (draggingInvItem(-1) ==1)
								cpl.win_quick_tag = cpl.win_inv_tag;
							quick_slots[ind]=cpl.win_quick_tag;
							/* now we do some tests... first, ensure this item can fit */
							update_quickslots(-1);
							/* now: if this is null, item is *not* in the main inventory
							 * of the player - then we can't put it in quickbar!
							 * Server will not allow apply of items in containers!
							 */
							if(!locate_item_from_inv(cpl.ob->inv , cpl.win_quick_tag))
							{
								sound_play_effect(SOUND_CLICKFAIL,0,0,100);
								draw_info("Only items from main inventory allowed in quickbar!", COLOR_WHITE);
							}
							else
								sound_play_effect(SOUND_GET,0,0,100); /* no bug - we 'get' it in quickslots */
						}
					}
				}else if (draggingInvItem(-1) ==2){ /* drag form IWIN_BELOW */
					if (!mouseInPlayfield(x, y) && y <550){
			            sound_play_effect(SOUND_GET,0,0,100);
						process_macro_keys(KEYFUNC_GET, 0);
					}
				}
				draggingInvItem(0);
				itemExamined  = 0; /* ready for next item */
			break;

			case SDL_MOUSEMOTION:
				if (GameStatus < GAME_STATUS_PLAY) break;
/*
{
char tz[40];
sprintf(tz,"x: %d , y: %d", x, y);
draw_info(tz, COLOR_BLUE |NDI_PLAYER);
draw_info(tz, COLOR_BLUE);
}
*/

				/* size change of splitted textWindow */
				if (x > 538 && x < 790 && cpl.menustatus == MENU_NO && 
					(textwin_set.split_flag && y > 564-(textwin_set.split_size+textwin_set.top_size)*10))
					cpl.resize_twin_marker=TRUE;
				else
					cpl.resize_twin_marker=FALSE;
				if(cpl.resize_twin == TRUE)
				{

					if (event.button.button ==SDL_BUTTON_LEFT && cursor_type && cpl.menustatus == MENU_NO)
							active_scrollbar = cursor_type+10;
					else cursor_type = 0;				

					if (textwin_set.split_flag && x > 538 && x < 790 && cpl.menustatus == MENU_NO)
					{
						if (active_scrollbar ==11 || (y > 577-textwin_set.split_size*10 
										&& y < 581-textwin_set.split_size*10 && !active_scrollbar))
							cursor_type = 1;
						else if (active_scrollbar ==12 || (y > 564-(textwin_set.split_size+textwin_set.top_size)*10 
	      							&&  y < 568-(textwin_set.split_size+textwin_set.top_size)*10 && !active_scrollbar))
							cursor_type = 2;
					}
					if (old_mouse_y != y)
					{
						if (active_scrollbar ==11){
							textwin_set.split_size = (580-y)/10;
   						if (textwin_set.split_size <  9) textwin_set.split_size = 9;
     					if (textwin_set.split_size > 39) textwin_set.split_size =39;       					
						}
						else if (active_scrollbar ==12){
							textwin_set.top_size = (580-y)/10-textwin_set.split_size;
   						if (textwin_set.top_size <  1) textwin_set.top_size = 1;
     					if (textwin_set.top_size > 39) textwin_set.top_size =39;       					
						}
						if (textwin_set.split_size+textwin_set.top_size > 56)
							textwin_set.top_size = 56 -textwin_set.split_size;
					}
				}
				/* scrollbar-sliders */	
				/* TODO: make it better */
				if (event.button.button ==SDL_BUTTON_LEFT && !draggingInvItem(-1)){
					/* TextWin Slider */
					if (active_scrollbar == 2 || (x > 790 && (
						(y > 488 && textwin_set.split_flag == FALSE)||
      			(y > 579-textwin_set.split_size*10 && textwin_set.split_flag == TRUE))))
         	{     
     				int repeat = win_lenbuf;
						active_scrollbar = 2;
						repeat = win_lenbuf/textwin_set.size/10 +2;
						if      (old_mouse_y - y > 0)
							while (repeat--) process_macro_keys(KEYFUNC_PAGEUP, 0);
						else if (old_mouse_y - y < 0)
							while (repeat--) process_macro_keys(KEYFUNC_PAGEDOWN, 0);
						break;
					}
					/* IWIN_INV Slider */
					if (active_scrollbar == 1 || (cpl.inventory_win == IWIN_INV && y > 506 && y < 583 && x >230 && x < 238))
   				{
						active_scrollbar = 1;
						if      (old_mouse_y - y > 0)
      				cpl.win_inv_slot-= INVITEMXLEN;
						else if (old_mouse_y - y < 0)
      				cpl.win_inv_slot+= INVITEMXLEN;
						if  (cpl.win_inv_slot > cpl.win_inv_count)
      				cpl.win_inv_slot = cpl.win_inv_count;
    				break;
        	}
				} /* END scrollbar-sliders */

				/* examine an item */
				/*
				if ((cpl.inventory_win == IWIN_INV) && y > 85 && y < 120 && x < 140){
						if (!itemExamined){
							check_keys(SDLK_e);
							itemExamined = 1;
			      }
						break;
    			}*/
				old_mouse_y = y;
			break;

			case SDL_MOUSEBUTTONDOWN:
				if (GameStatus < GAME_STATUS_PLAY) break;

				/* close number input */
				if (InputStringFlag && cpl.input_mode == INPUT_MODE_NUMBER){
					if (x >339 && x < 349 && y > 510 && y < 522){
						SDL_EnableKeyRepeat(0 , SDL_DEFAULT_REPEAT_INTERVAL);
						InputStringFlag = FALSE;
						InputStringEndFlag = TRUE;
					}
					break;
				}
				/* Toggle textwin */
				if(x >=488 && x< 528 &&y < 536 && y > 521){
					textwin_set.split_flag = !textwin_set.split_flag; 
					sound_play_effect(SOUND_SCROLL,0,0,100);
					break;
				}

				/* possible buttons */
				if(x >=748 && x<=790)
				{
					if(show_help_screen)
					{

						if(y>=1 && y<= 49) /* spell list */
						{
							process_macro_keys(KEYFUNC_HELP, 0);
						}
						else if(y>=51 && y<= 74) /* online help */
						{
							sound_play_effect(SOUND_SCROLL,0,0,100);
							show_help_screen=0;
						}
					}
					else if(y>=1 && y<= 24) /* spell list */
						check_menu_macros("?M_SPELL_LIST");
					else if(y>=26 && y<= 49) /* skill list */
						check_menu_macros("?M_SKILL_LIST");
					else if(y>=51 && y<= 74) /* online help */
						process_macro_keys(KEYFUNC_HELP, 0);
				}

				/* spell menu */
				if (cpl.menustatus == MENU_SLIST){ 
					if (y> 100 && y< 115 && x > 240 && x < 555){ /* group 0-9 */
						int nr = (int)((float)(x-240)/16.5f);
						if (!(nr &1))
          		spell_list_set.group_nr= nr/2;
					}
     			else if (y> 62 && y< 75 && x > 650 && x < 661) /* exit */
						cpl.menustatus = MENU_NO;
     			else if (y> 126 && y< 463){
        		if (x > 151 && x < 394){ /* a-z */
	            spell_list_set.class_nr=0;
            	spell_list_set.entry_nr= (y -126)/13;
        		}
        		else if (x > 402 && x < 654){ /* A-Z */
	            spell_list_set.class_nr=1;
            	spell_list_set.entry_nr= (y -126)/13;
        		}
					}
       		else if (x > 146 && x < 175 && y > 479 && y < 507)
						process_menu_macro_keys(KEYFUNC_SPELL_RETURN, 0);
					break;
				}

				/* skill menu */
				if (cpl.menustatus == MENU_SKILL){ 
					if (y> 104 && y< 118 && x > 193 && x < 602){ /* groups */
						int nr = (x-193)/60;
						if (nr*60 -x > -243)							
          		skill_list_set.group_nr= nr;
					}
     			else if (y> 62 && y< 75 && x > 608 && x < 621) /* exit */
						cpl.menustatus = MENU_NO;
     			else if (y> 126 && y< 463 && x > 190 && x < 605){ /* a-z */
            	skill_list_set.entry_nr= (y -126)/13;
					}
       		else if (x > 186 && x < 216 && y > 479 && y < 507)
						process_menu_macro_keys(KEYFUNC_SKILL_RETURN, 0);
					break;
				}

				/* key binding */
				if (cpl.menustatus == MENU_KEYBIND){ 
					if (y> 198 && y< 206 && x > 490 && x < 500) /* repeat */
						process_menu_macro_keys(KEYFUNC_KB_REPEAT, 0);
		 			else if (y> 452 && y< 468){
						if (x > 248 && x < 314) /* new */
							process_menu_macro_keys(KEYFUNC_KB_NEW, 0);
						else if (x > 322 && x < 388) /* edit */
							process_menu_macro_keys(KEYFUNC_KB_EDIT, 0);
						else if (x > 473 && x < 539) /* done */
							process_menu_macro_keys(KEYFUNC_KB_DONE, 0);
					}
		 			else if (y> 127 && y< 139 && x > 551 && x < 563) /* exit */
						cpl.menustatus = MENU_NO;
     			else if (y> 214 && y< 445 && x > 246 && x < 543){ /* keys */
						if (event.button.button ==4)
         			process_menu_macro_keys(KEYFUNC_MENU_UP,0);					
						else if (event.button.button ==5)
      				process_menu_macro_keys(KEYFUNC_MENU_DOWN,0);
    				else if (event.button.button == SDL_BUTTON_LEFT)
							keybind_entry = (y -214) /13 + keybind_startoff;
					}
					break;
				}

				/***********************
				* mouse in Menue-field *
				************************/
				/* lower textwin */
				if (x > 538 && ((y > 488 && textwin_set.split_flag == FALSE)
				||((y > 579-textwin_set.split_size*10) && textwin_set.split_flag == TRUE))){
					/* mousewheel || scrollbar-button_down? */
					if (event.button.button ==4	|| (event.button.button == SDL_BUTTON_LEFT && 
     				((textwin_set.split_flag == FALSE && y < 497)
         	||(textwin_set.split_flag == TRUE  && y < 588-textwin_set.split_size*10))))
      			process_macro_keys(KEYFUNC_PAGEUP,0);
					/* mousewheel || scrollbar-button_down? */      			
					else if (event.button.button ==5 || (event.button.button == SDL_BUTTON_LEFT && y > 590)) 
      			process_macro_keys(KEYFUNC_PAGEDOWN,0); 
					/* clicked on keyword in textwin? */
					else if (x < 788 && y < 588){
						char cmdBuf[256] ={"/say "};
						int pos =0, pos2;
						int x2 = 538;
						char *text = get_textWinRow(y);

						while (text[pos]){
 							if (text[pos++] !='^'){
	 							x2 += SystemFont.c[(int)text[pos]].w+ SystemFont.char_offset;
								continue;
							}
							pos2 =4;
							while (text[pos] && text[pos] != '^'){
       					cmdBuf[++pos2]= text[pos];
	 							x2 += SystemFont.c[(int)text[pos++]].w+ SystemFont.char_offset;         						
     					}
							cmdBuf[++pos2] =0;
							if (x2 <x) continue;
 							if (cmdBuf[5] && cmdBuf[6]) /* dont say nothing to server */
 							{
								send_command(cmdBuf, -1, SC_NORMAL);
							/*	draw_info(cmdBuf, NDI_PLAYER); */
							}	
							break;
						}
					}
					break;
				}
				/* upper textwin*/
	    			/* todo */

	
				if (!mouseInPlayfield(event.motion.x, event.motion.y)){
					/* combat modus */
					if ((cpl.inventory_win == IWIN_BELOW) && y > 498 && y < 521 && x < 27){
						check_keys(SDLK_c);
						break;
    			}

					/* talk button */
					if ((cpl.inventory_win == IWIN_BELOW) && y > 498+27 && y < 521+27 && x > 200+70 && x < 240+70){
							if (cpl.target_code) send_command("/t_tell hello", -1, SC_NORMAL);								
						break;
    			}



					/* inventory (open / close) */
					if (x < 112 && y > 466 && y <496){
						if (cpl.inventory_win == IWIN_INV)
							cpl.inventory_win = IWIN_BELOW;
						else 
							cpl.inventory_win = IWIN_INV;
						break;
					}

					else if (x >= SKIN_POS_QUICKSLOT_X && x < SKIN_POS_QUICKSLOT_X+282 &&
						y >= SKIN_POS_QUICKSLOT_Y && y<SKIN_POS_QUICKSLOT_Y+42)
					{
						int ind = get_quickslot(x,y);
						if(ind != -1 && quick_slots[ind]!=-1) /* valid slot */
						{
							cpl.win_quick_tag= quick_slots[ind];
							if ((SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT)))
							{
								quick_slots[ind]=-1;
								draggingInvItem(3);
							}
							else
							{
								int stemp = cpl.inventory_win, itemp = cpl.win_inv_tag;
								cpl.inventory_win = IWIN_INV;
								cpl.win_inv_tag= quick_slots[ind];
								process_macro_keys(KEYFUNC_APPLY, 0);
								cpl.inventory_win = stemp;
								cpl.win_inv_tag= itemp;
							}
						}
						break;
					}

					/* inventory ( IWIN_INV )  */
					if (y > 497 && y < 593 && x >8 && x < 238){
						if (cpl.inventory_win != IWIN_INV) break;
						if (x > 230)/* scrollbar */
						{
							if (y < 506 && cpl.win_inv_slot >= INVITEMXLEN)
       					cpl.win_inv_slot-= INVITEMXLEN;
							else if (y > 583)
       				{
								cpl.win_inv_slot+= INVITEMXLEN;
								if  (cpl.win_inv_slot > cpl.win_inv_count)
        					cpl.win_inv_slot = cpl.win_inv_count;
							}
						}else{  /* stuff */
							if (event.button.button ==4 && cpl.win_inv_slot >= INVITEMXLEN)
								cpl.win_inv_slot-= INVITEMXLEN;
							else if (event.button.button ==5)
							{
								cpl.win_inv_slot+= INVITEMXLEN;
								if  (cpl.win_inv_slot > cpl.win_inv_count)
        					cpl.win_inv_slot = cpl.win_inv_count;
							}
							else
       				{
								cpl.win_inv_slot = (y - 497)/32 *INVITEMXLEN +  (x-8) /32 + cpl.win_inv_start ;
								cpl.win_inv_tag = get_inventory_data(cpl.ob, &cpl.win_inv_ctag,&cpl.win_inv_slot,
											&cpl.win_inv_start, &cpl.win_inv_count, INVITEMXLEN, INVITEMYLEN);
								if (event.button.button ==SDL_BUTTON_LEFT)
									draggingInvItem(1);
								else if (event.button.button ==SDL_BUTTON_RIGHT)
									process_macro_keys(KEYFUNC_APPLY, 0);
							}	
						}
						break;
     			}


					/* ground ( IWIN_BELOW )  */
					if (y > 565 && x >265 && x < 529){
						item *Item;
						if (cpl.inventory_win == IWIN_INV) cpl.inventory_win =IWIN_BELOW;
						cpl.win_below_slot = (x-265)/32;
						cpl.win_below_tag = get_inventory_data(cpl.below, &cpl.win_below_ctag,&cpl.win_below_slot,
							&cpl.win_below_start, &cpl.win_below_count,	INVITEMBELOWXLEN, INVITEMBELOWYLEN);
						Item = locate_item (cpl.win_below_tag);
						
						if ((SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT)))
							draggingInvItem(2);
						else 
							process_macro_keys(KEYFUNC_APPLY, 0);
						break;
					}

				}
				/***********************
				 mouse in Play-field 
				************************/
				else
				{
					/* Targetting */
						if ((SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_RIGHT)))
						{
							int tx, ty;
							char tbuf[32];

							cpl.inventory_win = IWIN_BELOW;
							get_tile_position( x, y, &tx, &ty );
							sprintf(tbuf,"/target !%d %d", tx-MAP_MAX_SIZE/2, ty-MAP_MAX_SIZE/2);
							send_command(tbuf, -1, SC_NORMAL);
						}
				}
	    	break;


			case SDL_KEYDOWN:
			case SDL_KEYUP:
				if(!InputStringFlag || cpl.input_mode != INPUT_MODE_NUMBER)
				{
					if(event.key.keysym.mod & KMOD_RSHIFT || 
								event.key.keysym.mod & KMOD_LSHIFT || event.key.keysym.mod & KMOD_SHIFT)
						cpl.inventory_win = IWIN_INV;
					else
						cpl.inventory_win = IWIN_BELOW;
					if(event.key.keysym.mod & KMOD_RCTRL || event.key.keysym.mod & KMOD_LCTRL || event.key.keysym.mod & KMOD_CTRL)
						cpl.fire_on = TRUE;                    
					else
						cpl.fire_on = FALSE;                    
				}

				if(InputStringFlag)
				{
					if(cpl.input_mode != INPUT_MODE_NUMBER)
						cpl.inventory_win = IWIN_BELOW;
					key_string_event(&event.key);
				}
				else if(!InputStringEndFlag)
                {
					if(GameStatus <= GAME_STATUS_WAITLOOP)
						done =key_meta_menu(&event.key);
          else if(GameStatus == GAME_STATUS_PLAY)
						done = key_event(&event.key);
          else if(GameStatus == GAME_STATUS_SETSTATS)
						key_setstats_event(&event.key);
          else if(GameStatus == GAME_STATUS_SETRACE)
						key_setrace_event(&event.key);
          else
						key_connection_event(&event.key);
				}
                                
			break;

            case SDL_QUIT:
				done = 1;
            break;
                        
			default:
            break;        
		}
	}

	/* ok, we have processed all real events.
	   now run through the list of keybinds and control repeat time value.
	   is the key is still marked as pressed in our keyboard mirror table,
	   and is the time this is pressed <= keybind press value + repeat value,
	   we assume a repeat if repeat flag is true.
	   Sadly, SDL don't has a tick count inside the event messages, means the
	   tick value when the event really was triggered. So, the client can't simulate 
       the buffered "rythm" of the key pressings when the client lags. */

	key_repeat();

	return(done);
}

void key_setrace_event(SDL_KeyboardEvent *key )
{
	char buf[256];
	if( key->type == SDL_KEYDOWN )
	{
		switch(key->keysym.sym)
		{
		case SDLK_ESCAPE:
			sprintf(buf,"connection closed. select new server.");
			draw_info(buf, COLOR_RED);
			GameStatus = GAME_STATUS_START;
			break;
		case SDLK_n:
			sprintf(buf,"new race.");
			draw_info(buf, COLOR_BLUE);
			send_reply("x");
			break;
		case SDLK_g:
			sprintf(buf,"get race.");
			draw_info(buf, COLOR_BLUE);
			send_reply("d");
			GameStatus = GAME_STATUS_WAITFORPLAY;
			break;
                        
        default:
        break;

		}
	}
}
void key_setstats_event(SDL_KeyboardEvent *key )
{
	char buf[256];
	if( key->type == SDL_KEYDOWN )
	{
		switch(key->keysym.sym)
		{
		case SDLK_ESCAPE:
			sprintf(buf,"connection closed. select new server.");
			draw_info(buf, COLOR_RED);
			GameStatus = GAME_STATUS_START;
			break;
		case SDLK_n:
			sprintf(buf,"new stats.");
			draw_info(buf, COLOR_BLUE);
			send_reply("y");
			break;
		case SDLK_g:
			sprintf(buf,"use stats.");
			draw_info(buf, COLOR_BLUE);
			send_reply("n");
			break;
        case SDLK_1:
            sprintf(buf,"exchange stat.");
            draw_info(buf, COLOR_BLUE);
            send_reply("1");
            break;
        case SDLK_2:
            sprintf(buf,"exchange stat.");
            draw_info(buf, COLOR_BLUE);
            send_reply("2");
            break;
        case SDLK_3:
            sprintf(buf,"exchange stat.");
            draw_info(buf, COLOR_BLUE);
            send_reply("3");
            break;
        case SDLK_4:
            sprintf(buf,"exchange stat.");
            draw_info(buf, COLOR_BLUE);
            send_reply("4");
            break;
        case SDLK_5:
            sprintf(buf,"exchange stat.");
            draw_info(buf, COLOR_BLUE);
            send_reply("5");
            break;
        case SDLK_6:
            sprintf(buf,"exchange stat.");
            draw_info(buf, COLOR_BLUE);
            send_reply("6");
            break;
        case SDLK_7:
            sprintf(buf,"exchange stat.");
            draw_info(buf, COLOR_BLUE);
            send_reply("7");
            break;

            default:
            break;
		}
	}
}

void key_connection_event(SDL_KeyboardEvent *key )
{
	char buf[256];
	if( key->type == SDL_KEYDOWN )
	{
		switch(key->keysym.sym)
		{
			case SDLK_ESCAPE:
				sprintf(buf,"connection closed. select new server.");
				draw_info(buf, COLOR_RED);
				GameStatus = GAME_STATUS_START;
			break;
                        
			default:                    
			break;
		}
	}
}

/* metaserver menu key */
int key_meta_menu(SDL_KeyboardEvent *key )
{

	if( key->type == SDL_KEYDOWN )
	{
		switch(key->keysym.sym)
		{
			case SDLK_UP:
			if(metaserver_sel)
			{
				metaserver_sel--;
				if(metaserver_start>metaserver_sel)
					metaserver_start=metaserver_sel;
			}
				break;
			case SDLK_DOWN:
				if(metaserver_sel<metaserver_count-1)
				{
					metaserver_sel++;
					if(metaserver_sel>=MAXMETAWINDOW)
						metaserver_start=(metaserver_sel+1)-MAXMETAWINDOW;
				}
				break;
			case SDLK_RETURN:
				get_meta_server_data(metaserver_sel, ServerName, &ServerPort);
				GameStatus = GAME_STATUS_STARTCONNECT;
			break;

			case SDLK_ESCAPE:
				return(1);
			break;
                        default:
                                break;
		}
	}
	return(0);
}

/* we get TEXT from keyboard. This is for console input */
static void key_string_event(SDL_KeyboardEvent *key )
{
	register char c;

	if( key->type == SDL_KEYDOWN )
	{
        switch(key->keysym.sym)
		{
			case SDLK_ESCAPE:
				SDL_EnableKeyRepeat(0 , SDL_DEFAULT_REPEAT_INTERVAL);
				InputStringEscFlag=TRUE;
				return;
				break;
			case SDLK_KP_ENTER:
			case SDLK_RETURN:
			case SDLK_TAB:
				if (key->keysym.sym != SDLK_TAB || GameStatus < GAME_STATUS_WAITFORPLAY)
				{
					SDL_EnableKeyRepeat(0 , SDL_DEFAULT_REPEAT_INTERVAL);
					InputStringFlag=FALSE;
					InputStringEndFlag = TRUE;/* mark that we got some here*/
				}
			break;

			case SDLK_BACKSPACE:
				if(InputCount)
					InputString[--InputCount]=0;
				break;

			default:
				

				/* if we are in number console mode, use GET as quick enter
				 * mode - this is a very handy shortcut
				 */
				if(cpl.input_mode == INPUT_MODE_NUMBER && 
					(key->keysym.sym == get_action_keycode ||key->keysym.sym == drop_action_keycode))
				{
					SDL_EnableKeyRepeat(0 , SDL_DEFAULT_REPEAT_INTERVAL);
					InputStringFlag=FALSE;
					InputStringEndFlag = TRUE;/* mark that we got some here*/
				}
				/* now keyboard magic - transform a sym (kind of scancode)
				 * to a layout code
				 */
				if(InputCount <InputMax)
				{  
					c=0;
					/* we want only numbers in number mode - even when shift is hold */
					if(cpl.input_mode == INPUT_MODE_NUMBER)
					{
						switch(key->keysym.sym)
						{
							case SDLK_0:
								c='0';
							break;
							case SDLK_1:
								c='1';
							break;
							case SDLK_2:
								c='2';
							break;
							case SDLK_3:
								c='3';
							break;
							case SDLK_4:
								c='4';
							break;
							case SDLK_5:
								c='5';
							break;
							case SDLK_6:
								c='6';
							break;
							case SDLK_7:
								c='7';
							break;
							case SDLK_8:
								c='8';
							break;
							case SDLK_9:
								c='9';
							break;
							default:
							    c=0;
							break;
						}
						if(c)
						{
							InputString[InputCount++]=c;
							InputString[InputCount]=0;
						}
					}
					else
					{
						if ( (key->keysym.unicode & 0xFF80) == 0 )
							c = key->keysym.unicode & 0x7F;
						c = key->keysym.unicode & 0xff;
						if(c>=32)
						{
							if(key->keysym.mod & KMOD_RSHIFT ||
												key->keysym.mod & KMOD_LSHIFT ||
											key->keysym.mod & KMOD_SHIFT)
								c = toupper( c);
	
							InputString[InputCount++]=c;
							InputString[InputCount]=0;
						}
					}
				}
			break;
		}
	}
    
}
/* we have a key event */
int key_event(SDL_KeyboardEvent *key )
{

	if( key->type == SDL_KEYUP )
	{
		if(KeyScanFlag)
		{
			char buf[256];
			sprintf(buf,"Scancode: %d", key->keysym.sym);
			draw_info(buf,COLOR_RED);
		}

		if(cpl.menustatus != MENU_NO)
		{
			keys[key->keysym.sym].pressed = FALSE;
		}
		else
		{
			keys[key->keysym.sym].pressed = FALSE;
			switch(key->keysym.sym)
			{
			case SDLK_LSHIFT:
				case SDLK_RSHIFT:
					cpl.inventory_win = IWIN_BELOW;
					break;
				case SDLK_LALT:
				case SDLK_RALT:
					send_command("/run_stop", -1, SC_FIRERUN);
					/*draw_info("run_stop",COLOR_DGOLD);*/
					cpl.run_on = FALSE;
					cpl.resize_twin =FALSE;
				break;
				case SDLK_RCTRL:
				case SDLK_LCTRL:
					cpl.fire_on = FALSE;                    
				break;
                             
                default:                        
                break;
			}
		}
	}
	else if( key->type == SDL_KEYDOWN )
	{
		if(cpl.menustatus != MENU_NO)
		{
            /* we catch here the keybind key, when we insert a new macro there */
            if(cpl.menustatus == MENU_KEYBIND)
            {
                if(keybind_status == KEYBIND_STATUS_EDITKEY || keybind_status == KEYBIND_STATUS_NEWKEY)
                {
										if (key->keysym.sym != SDLK_ESCAPE)
										{
	                    sound_play_effect(SOUND_SCROLL,0,0,100);
  	                  strcpy(keybind_key.keyname, SDL_GetKeyName(key->keysym.sym));
    	                keybind_key.key = key->keysym.sym;
      	              add_keybind_macro(&keybind_key);
										}
        	          keybind_status = KEYBIND_STATUS_NO;
          	        return(0);
                }
            }

            keys[key->keysym.sym].pressed = TRUE;
			keys[key->keysym.sym].time = LastTick+key_repeat_time_init;
			check_menu_keys(key->keysym.sym,key->keysym.sym);
		}
		else
		{
			if(esc_menu_flag != TRUE)
			{
				keys[key->keysym.sym].pressed = TRUE;
				keys[key->keysym.sym].time = LastTick+key_repeat_time_init;
	            		check_keys(key->keysym.sym);
			}
			switch((int)key->keysym.sym)
			{
				case SDLK_F1:
					quickslot_key(key,0);
				break;
				case SDLK_F2:
					quickslot_key(key,1);
				break;
				case SDLK_F3:
					quickslot_key(key,2);
				break;
				case SDLK_F4:
					quickslot_key(key,3);
				break;
				case SDLK_F5:
					quickslot_key(key,4);
				break;
				case SDLK_F6:
					quickslot_key(key,5);
				break;
				case SDLK_F7:
					quickslot_key(key,6);
				break;
				case SDLK_F8:
					quickslot_key(key,7);
				break;
				case SDLK_LSHIFT:
				case SDLK_RSHIFT:
					cpl.inventory_win = IWIN_INV;
					break;
				case SDLK_RALT:
				case SDLK_LALT:

                    /*draw_info("run",COLOR_DGOLD);*/
					/* thats the tricky part!
					 * only WHEN we have the mouse cursor 
					 * inside the textwin BEFORE we hit ALT
					 * - only then set resize_twin.
					 * This avoid the problem we hit ALT
					 * for running on the map and then moving the
					 * mouse inside the textwin - thas action
					 * has a different context.
					 */
					if(cpl.resize_twin_marker)
						cpl.resize_twin =TRUE;
					cpl.run_on = TRUE;
					break;
				case SDLK_RCTRL:
				case SDLK_LCTRL:
					cpl.fire_on = TRUE;
                  
				break;
				case SDLK_ESCAPE:
					if(show_help_screen)
					{
						show_help_screen=0;
						break;
					}
					if(esc_menu_flag == FALSE)
					{
						map_udate_flag=1;
						esc_menu_flag = TRUE;
						esc_menu_index = ESC_MENU_BACK;
					}
					else
						esc_menu_flag = FALSE;
				        sound_play_effect(SOUND_SCROLL,0,0,100);
				break;
                                
		                default:
					if(esc_menu_flag == TRUE)
					{
						switch((int)key->keysym.sym)
						{
							case SDLK_RETURN:
								if(esc_menu_index == ESC_MENU_KEYS)
								{
									show_help_screen=0;
									keybind_status = KEYBIND_STATUS_NO;
									cpl.menustatus = MENU_KEYBIND;
								}
								else if(esc_menu_index == ESC_MENU_LOGOUT)
								{
									SOCKET_CloseSocket(csocket.fd);
									GameStatus = GAME_STATUS_INIT;
								}
						        sound_play_effect(SOUND_SCROLL,0,0,100);
								esc_menu_flag = FALSE;
							break;

							case SDLK_UP:
								esc_menu_index--;
								if(esc_menu_index<0)
									esc_menu_index=ESC_MENU_INDEX-1;
							break;
							case SDLK_DOWN:
								esc_menu_index++;
								if(esc_menu_index>=ESC_MENU_INDEX)
									esc_menu_index=0;
							break;
						};
					}
				break;

			};
		}
	}

	return(0);
}



static void check_menu_keys(int key, int scan)
{
        register int i;

        /* here we catch hard defined keys first */
        /* for the spelllist for example, we don't want define all keys as macros */
        if(key == SDLK_ESCAPE)
        {
                if(cpl.menustatus == MENU_KEYBIND)
                        save_keybind_file(KEYBIND_FILE);
                cpl.menustatus = MENU_NO;
                map_udate_flag=2;
                reset_keys();
                return;
        }

        /* first, we have menu keys who can be set by the user - we ask for them first */
        for(i=0;i<keymap_count;i++)
        {
            /* our key*/
            if(scan == keymap[i].key)
            {
                if(check_menu_macros(keymap[i].text))
                    return;
            }
        }

        /* now we has checked hard coded & user defined key */
        /* now we control menu macros */
        
        for(i=0;i<DEFAULT_MENU_KEYMAP;i++)
        {
                if(key == menu_keymap[i].key && 
                            menu_keymap[i].menu_mode & cpl.menustatus)
                {
                    check_menu_macro_keys(menu_keymap[i].text);
                    return;
                }
        }
}

static Boolean check_menu_macro_keys(char *text)
{
    register int i;


    for(i=0;i<DEFAULT_MENU_KEYMAP_MACROS;i++) 
    {
        if(!strcmp(menu_macro[i].macro, text) && menu_macro[i].menu_mode&cpl.menustatus)
        {                    
            if(process_menu_macro_keys(menu_macro[i].internal,menu_macro[i].value))
                   return(TRUE);
        }
    }
        
    return(FALSE);
}

/* here we look in the user defined keymap and try to get same useful macros */
static Boolean check_menu_macros(char *text)
{
    if(!strcmp("?M_SPELL_LIST",text) )
    {
        
        map_udate_flag=2;
        if(cpl.menustatus != MENU_SLIST)
		{
			show_help_screen=0;
            cpl.menustatus = MENU_SLIST;
		}
        else
            cpl.menustatus = MENU_NO;       
            
        sound_play_effect(SOUND_SCROLL,0,0,100);
        reset_keys();
        return TRUE;
    }
    if(!strcmp("?M_SKILL_LIST",text) )
    {
        map_udate_flag=2;
        if(cpl.menustatus != MENU_SKILL)
		{
			show_help_screen=0;
            cpl.menustatus = MENU_SKILL;
		}
        else
            cpl.menustatus = MENU_NO;            
        sound_play_effect(SOUND_SCROLL,0,0,100);
        reset_keys();
        return TRUE;
    }
    if(!strcmp("?M_KEYBIND",text) )
    {
        map_udate_flag=2;
        if(cpl.menustatus != MENU_KEYBIND)
        {
			show_help_screen=0;
            keybind_status = KEYBIND_STATUS_NO;
            cpl.menustatus = MENU_KEYBIND;
        }
        else
        {
            save_keybind_file(KEYBIND_FILE);
            cpl.menustatus = MENU_NO;           
        }
        sound_play_effect(SOUND_SCROLL,0,0,100);
        reset_keys();
        return TRUE;
    }
    if(!strcmp("?M_STATUS",text) )
    {
        map_udate_flag=2;
        if(cpl.menustatus != MENU_STATUS)
		{
			show_help_screen=0;
            cpl.menustatus = MENU_STATUS;
		}
        else
            cpl.menustatus = MENU_NO;            
        sound_play_effect(SOUND_SCROLL,0,0,100);
        reset_keys();
        return TRUE;
    }
    return FALSE;
}

static void check_keys(int key)
{
        register int i;
		char buf[512];

        for(i=0;i<keymap_count;i++)
        {
                /* our key*/
                if(key == keymap[i].key)
                {
                    
                        if(check_macro_keys(keymap[i].text))
                        /* if no key macro, submit the text as cmd*/
                        {
							draw_info(keymap[i].text,COLOR_DGOLD);
							strcpy(buf,keymap[i].text);
							if(!client_command_check(buf))
	                            send_command(buf, -1,keymap[i].mode);
                        }
                        return;
                }
        }
}

static Boolean check_macro_keys(char *text)
{
	register int i;

	for(i=0;i<DEFAULT_KEYMAP_MACROS;i++)
	{
		if(!strcmp(defkey_macro[i].macro, text) )
		{
			if(!process_macro_keys(defkey_macro[i].internal,defkey_macro[i].value))
                return(FALSE);
            return(TRUE);
        }
	}
	return(TRUE);
}

static Boolean process_macro_keys(int id, int value)
{
	int nrof, tag=0, loc=0;
	char buf[256];
	item *it, *tmp;

	switch(id)
	{
        case KEYFUNC_FIREREADY:
            if(cpl.inventory_win == IWIN_BELOW)
                tag = cpl.win_below_tag;
            else
                tag = cpl.win_inv_tag;
            examine_range_marks(tag);
        break;
        case KEYFUNC_PAGEUP:
		if(textwin_set.split_flag == TRUE)
	        	text_win_soff_split++;
		else
            		text_win_soff++;
        break;
        case KEYFUNC_PAGEDOWN:
		if(textwin_set.split_flag == TRUE)
		{
			if(text_win_soff_split--<0)
				text_win_soff_split=0;
		}
		else
		{
			if(text_win_soff--<0)
			    text_win_soff=0;
		}
            break;
        case KEYFUNC_TARGET_ENEMY:
			send_command("/target 0", -1, SC_NORMAL);
        break;
        case KEYFUNC_TARGET_FRIEND:
			send_command("/target 1", -1, SC_NORMAL);
        break;
        case KEYFUNC_TARGET_SELF:
			send_command("/target 2", -1, SC_NORMAL);
        break;
        case KEYFUNC_COMBAT:
			send_command("/combat", -1, SC_NORMAL);
        break;
        case KEYFUNC_PAGEUP_TOP:
	            text_win_soff_top++;
        break;

        case KEYFUNC_PAGEDOWN_TOP:
				if(text_win_soff_top--<0)
				    text_win_soff_top=0;

            break;
        case KEYFUNC_SLIST:
            map_udate_flag=2;
            sound_play_effect(SOUND_SCROLL,0,0,100);
            if(cpl.menustatus != MENU_SLIST)
                cpl.menustatus = MENU_SLIST;
            else
                cpl.menustatus = MENU_NO;            
            reset_keys();
            break;
        case KEYFUNC_SKILL:
            map_udate_flag=2;
            sound_play_effect(SOUND_SCROLL,0,0,100);
            if(cpl.menustatus != MENU_SKILL)
                cpl.menustatus = MENU_SKILL;
            else
                cpl.menustatus = MENU_NO;            
            reset_keys();
            break;
        case KEYFUNC_STATUS:
            map_udate_flag=2;
            if(cpl.menustatus != MENU_STATUS)
                cpl.menustatus = MENU_STATUS;
            else
                cpl.menustatus = MENU_NO;            
            sound_play_effect(SOUND_SCROLL,0,0,0);
            reset_keys();
            break;
        case KEYFUNC_KEYBIND:
            map_udate_flag=2;
            if(cpl.menustatus != MENU_KEYBIND)
            {
                keybind_status = KEYBIND_STATUS_NO;
                cpl.menustatus = MENU_KEYBIND;
            }
            else
            {
                save_keybind_file(KEYBIND_FILE);
                cpl.menustatus = MENU_NO;            
            }
            sound_play_effect(SOUND_SCROLL,0,0,100);
            reset_keys();
            break;
        case KEYFUNC_CONSOLE:
			show_help_screen=0;
            sound_play_effect(SOUND_CONSOLE,0,0,100);
            reset_keys();
            if(cpl.input_mode == INPUT_MODE_NO)
			{
				cpl.input_mode = INPUT_MODE_CONSOLE;
                open_input_mode(253);
			}
			else  if(cpl.input_mode == INPUT_MODE_CONSOLE)
				cpl.input_mode = INPUT_MODE_NO;
			map_udate_flag=2;
			break;

        case KEYFUNC_RUN:
			if(!(cpl.runkey_on=cpl.runkey_on?FALSE:TRUE))
				send_command("/run_stop", -1, SC_FIRERUN);
			sprintf(buf,"runmode %s", cpl.runkey_on?"on":"off");
			/*draw_info(buf,COLOR_DGOLD);*/
			break;
		case KEYFUNC_MOVE:
			move_keys(value);
		break;
		case KEYFUNC_CURSOR:
			cursor_keys(value);
		break;

        case KEYFUNC_RANGE:
        if (RangeFireMode++ == FIRE_MODE_INIT-1)
           RangeFireMode = 0; 
        map_udate_flag=2;
        return FALSE;
        break;

		case KEYFUNC_APPLY:
			if(cpl.inventory_win == IWIN_BELOW)
				tag = cpl.win_below_tag;
			else
				tag = cpl.win_inv_tag;

			if(tag == -1)
				return FALSE;
				sprintf(buf,"apply %s", locate_item (tag)->s_name);
			draw_info(buf,COLOR_DGOLD);
			client_send_apply (tag);
            return FALSE;
            break;
		case KEYFUNC_EXAMINE:
			if(cpl.inventory_win == IWIN_BELOW)
				tag = cpl.win_below_tag;
			else
			    tag = cpl.win_inv_tag;
			if(tag == -1)
				return FALSE;
				client_send_examine (tag);
				sprintf(buf,"examine %s", locate_item (tag)->s_name);
				draw_info(buf,COLOR_DGOLD);
                return FALSE;
                break;
		case KEYFUNC_MARK:
			if(cpl.inventory_win == IWIN_BELOW)
				tag = cpl.win_below_tag;
			else
			    tag = cpl.win_inv_tag;                
			if(tag == -1)
				return FALSE;
			send_mark_obj ((it=locate_item (tag)));
			sprintf(buf,"mark %s", it->s_name);
			draw_info(buf,COLOR_DGOLD);
            return FALSE;
            break;
		case KEYFUNC_LOCK:
            if(cpl.inventory_win == IWIN_BELOW)
				tag = cpl.win_below_tag;
			else
			    tag = cpl.win_inv_tag;
			if(tag == -1)
				return FALSE;
			toggle_locked ((it=locate_item (tag)));
			if(it->locked)
				sprintf(buf,"unlock %s", it->s_name);
			else
				sprintf(buf,"lock %s", it->s_name);
			draw_info(buf,COLOR_DGOLD);
            return FALSE;
		break;
		case KEYFUNC_GET:
			nrof = 1; /* number of Items */
			if(cpl.inventory_win == IWIN_BELOW) /* from below to inv*/
			{
        tag = cpl.win_below_tag;
				if (cpl.container)
        {
					/* container, aber nicht der gleiche */
          if(cpl.container->tag != cpl.win_below_ctag) loc = cpl.container->tag;
          else loc = cpl.ob->tag;
        }
       	else loc = cpl.ob->tag;
			}
			else /* inventory */
			{
                if(cpl.container)
                {
                    if(cpl.container->tag == cpl.win_inv_ctag)
                    {

                        tag = cpl.win_inv_tag;
                        loc = cpl.ob->tag;
                    }
                    else /* from inventory to container - if the container is in inv */
                    {
                        tag = -1;

                        if(cpl.ob)
                        {
                            for (tmp = cpl.ob->inv; tmp; tmp=tmp->next)
                            {
                                if(tmp->tag == cpl.container->tag)
                                {
                                    tag = cpl.win_inv_tag;
                                    loc = cpl.container->tag;
                                    break;   
                                }
                            }
                            if(tag==-1)
                                draw_info("you already have it.",COLOR_DGOLD);                            
                        }
                    }
                }
                else
                {
                    draw_info("you have no open container to put it in.", COLOR_DGOLD);
                    /*
                    tag = cpl.win_inv_tag;
                    loc = cpl.ob->tag;
                    */
                    tag = -1;
                }

			}

			if(tag == -1)
				return FALSE;
			if((it = locate_item (tag)))
				nrof = it->nrof;
            else
                return FALSE;
           if(nrof == 1)
				nrof = 0;
            else
            {
								
                reset_keys();
                cpl.input_mode = INPUT_MODE_NUMBER;
                open_input_mode(22);
                cpl.loc = loc;
                cpl.tag =tag;
		cpl.nrof = nrof;
		cpl.nummode = NUM_MODE_GET;
		sprintf(InputString,"%d",nrof);
                InputCount =strlen(InputString);
		strncpy(cpl.num_text,it->s_name,250);
		cpl.num_text[250]=0;
                return FALSE;

            }
			sound_play_effect(SOUND_GET,0,0,100);
            sprintf(buf,"get %s", it->s_name);
			draw_info(buf,COLOR_DGOLD);
			client_send_move (loc, tag, nrof);
            return FALSE;

			break;

			case KEYFUNC_LAYER0:
			if(debug_layer[0])
				debug_layer[0]=FALSE;
			else
				debug_layer[0]=TRUE;
			sprintf(buf,"debug: map layer 0 %s.\n", debug_layer[0]?"activated":"deactivated");
			draw_info(buf,COLOR_DGOLD);
			return FALSE;	
		break;
			case KEYFUNC_LAYER1:
			if(debug_layer[1])
				debug_layer[1]=FALSE;
			else
				debug_layer[1]=TRUE;
			sprintf(buf,"debug: map layer 1 %s.\n", debug_layer[1]?"activated":"deactivated");
			draw_info(buf,COLOR_DGOLD);
			return FALSE;	
		break;
			case KEYFUNC_LAYER2:
			if(debug_layer[2])
				debug_layer[2]=FALSE;
			else
				debug_layer[2]=TRUE;
			sprintf(buf,"debug: map layer 2 %s.\n", debug_layer[2]?"activated":"deactivated");
			draw_info(buf,COLOR_DGOLD);
			return FALSE;	
		break;
			case KEYFUNC_LAYER3:
			if(debug_layer[3])
				debug_layer[3]=FALSE;
			else
				debug_layer[3]=TRUE;
			sprintf(buf,"debug: map layer 3 %s.\n", debug_layer[3]?"activated":"deactivated");
			draw_info(buf,COLOR_DGOLD);
			return FALSE;	
		break;
		case KEYFUNC_HELP:
			cpl.menustatus = MENU_NO;       
			sound_play_effect(SOUND_SCROLL,0,0,100);
			if(show_help_screen)
			{
				if(++show_help_screen>MAX_HELP_SCREEN)
					show_help_screen=1;
			}
			else
				show_help_screen=1;

			return FALSE;	
		break;

		case KEYFUNC_DROP:
			nrof = 1;
            if(cpl.inventory_win == IWIN_INV) /* from below to inv*/
			{
                if(cpl.win_inv_ctag != -1)
                {
                    tag = cpl.win_inv_tag;
                    loc = cpl.below->tag;
                }
                else
                {
                    tag = cpl.win_inv_tag;
                    loc = cpl.below->tag;

                    if(cpl.container)
                    {
                        if(cpl.below)
                        {
                            for (tmp = cpl.below->inv; tmp; tmp=tmp->next)
                            {
                                if(tmp->tag == cpl.container->tag)
                                {
                                    tag = cpl.win_inv_tag;
                                    loc = cpl.container->tag;                   
                                    break;
                                }
                            }
                        }
                    }
                }
			}
            else
            {
                sprintf(buf,"The item is already on the floor.");
                draw_info(buf,COLOR_DGOLD);
                return FALSE;
            }
            if(tag == -1)
				return FALSE;
			if((it = locate_item (tag)))
				nrof = it->nrof;
            else
                return FALSE;
                
           if(it->locked)
	   {
           	sound_play_effect(SOUND_CLICKFAIL,0,0,100);
	   	draw_info("unlock item first!",COLOR_DGOLD);
		return FALSE;
	   }

	    if(nrof == 1)
		nrof = 0;
            else
            {
                reset_keys();
                cpl.input_mode = INPUT_MODE_NUMBER;
                open_input_mode(22);
                cpl.loc = loc;
                cpl.tag =tag;
                cpl.nrof = nrof;
				cpl.nummode = NUM_MODE_DROP;
				sprintf(InputString,"%d",nrof);
                InputCount =strlen(InputString); 
                strncpy(cpl.num_text,it->s_name,250);
				cpl.num_text[250]=0;
                return FALSE;




            }
			sound_play_effect(SOUND_DROP,0,0,100);
			sprintf(buf,"drop %s", it->s_name);
   			draw_info(buf,COLOR_DGOLD);
			client_send_move (loc, tag, nrof);
            return FALSE;
		break;
                
        default:
            return TRUE;
        break;
	}
    return FALSE;
}

static void cursor_keys(int num)
{
	switch(num)
	{
		case 0:
			if(cpl.inventory_win==IWIN_BELOW)
			{
				if(cpl.win_below_slot-INVITEMBELOWXLEN>=0)
					cpl.win_below_slot-=INVITEMBELOWXLEN;
				cpl.win_below_tag = get_inventory_data(cpl.below, &cpl.win_below_ctag,&cpl.win_below_slot,
							&cpl.win_below_start, &cpl.win_below_count,
							INVITEMBELOWXLEN, INVITEMBELOWYLEN);
			}
			else
			{
			    if(cpl.win_inv_slot-INVITEMXLEN>=0)
				    cpl.win_inv_slot-=INVITEMXLEN;
				cpl.win_inv_tag = 
                    get_inventory_data(cpl.ob, &cpl.win_inv_ctag,&cpl.win_inv_slot,
					&cpl.win_inv_start, &cpl.win_inv_count, INVITEMXLEN, INVITEMYLEN);
			}
			break;

		case 1:
			if(cpl.inventory_win==IWIN_BELOW)
			{
				if(cpl.win_below_slot+INVITEMXLEN<cpl.win_below_count)
					cpl.win_below_slot+=INVITEMXLEN;
				cpl.win_below_tag = get_inventory_data(cpl.below, &cpl.win_below_ctag,&cpl.win_below_slot,
					&cpl.win_below_start, &cpl.win_below_count,
					INVITEMBELOWXLEN, INVITEMBELOWYLEN);
			}
			else
			{
		        if(cpl.win_inv_slot+INVITEMXLEN<cpl.win_inv_count)
					cpl.win_inv_slot+=INVITEMXLEN;
				cpl.win_inv_tag = 
                    get_inventory_data(cpl.ob,&cpl.win_inv_ctag, &cpl.win_inv_slot,
					&cpl.win_inv_start, &cpl.win_inv_count, INVITEMXLEN, INVITEMYLEN);
			}
		break;

		case 2:
			if(cpl.inventory_win==IWIN_BELOW)
			{
				cpl.win_below_slot--;
				cpl.win_below_tag = get_inventory_data(cpl.below, &cpl.win_below_ctag,&cpl.win_below_slot,
					&cpl.win_below_start, &cpl.win_below_count,
					INVITEMBELOWXLEN, INVITEMBELOWYLEN);
			}
			else
			{
				cpl.win_inv_slot--;
				cpl.win_inv_tag = 
                    get_inventory_data(cpl.ob, &cpl.win_inv_ctag,&cpl.win_inv_slot,
					&cpl.win_inv_start, &cpl.win_inv_count, INVITEMXLEN, INVITEMYLEN);
			}
		break;

		case 3:
			if(cpl.inventory_win==IWIN_BELOW)
			{
				cpl.win_below_slot++;
				cpl.win_below_tag = get_inventory_data(cpl.below,&cpl.win_below_ctag, &cpl.win_below_slot,
					&cpl.win_below_start, &cpl.win_below_count,
					INVITEMBELOWXLEN, INVITEMBELOWYLEN);
			}
			else
			{
					cpl.win_inv_slot++;
					cpl.win_inv_tag = 
                        get_inventory_data(cpl.ob, &cpl.win_inv_ctag,&cpl.win_inv_slot,
					&cpl.win_inv_start, &cpl.win_inv_count, INVITEMXLEN, INVITEMYLEN);
			}
		break;
	}
}

static void move_keys(int num)
{
	char buf[256];
	char msg[256];

	/* move will overruled from fire */
	/* because real toggle mode don't work, this works a bit different */
	/* pressing alt will not set move mode until unpressed when firemode is on */
	/* but it stops running when released */
	if((cpl.runkey_on || cpl.run_on) && (!cpl.firekey_on && !cpl.fire_on)) /* runmode on, or ALT key trigger */
	{
		send_command(directionsrun[num],-1, SC_FIRERUN);
		strcpy(buf, "run ");
	}
	/* thats the range menu - we handle it messages unique */
	else if(cpl.firekey_on || cpl.fire_on)
	{

        if(RangeFireMode == FIRE_MODE_SKILL)
        {
            if(!fire_mode_tab[FIRE_MODE_SKILL].skill || fire_mode_tab[FIRE_MODE_SKILL].skill->flag == -1)
			{
				draw_info("no skill selected.",COLOR_WHITE);
                return;
			}
            sprintf(buf,"/%s %d %d %s",directionsfire[num], RangeFireMode, -1,fire_mode_tab[RangeFireMode].skill->name);
            sprintf(msg,"use %s %s",fire_mode_tab[RangeFireMode].skill->name, directions_name[num]);
			
        }
        else if(RangeFireMode == FIRE_MODE_SPELL)
        {
            if(!fire_mode_tab[FIRE_MODE_SPELL].spell || fire_mode_tab[FIRE_MODE_SPELL].spell->flag == -1)
			{
				draw_info("no spell selected.",COLOR_WHITE);
                return;
			}
            sprintf(buf,"/%s %d %d %s",directionsfire[num], RangeFireMode, -1,fire_mode_tab[RangeFireMode].spell->name);
            sprintf(msg,"cast %s %s",fire_mode_tab[RangeFireMode].spell->name, directions_name[num]);
        }
        else
            sprintf(buf,"/%s %d %d %d",directionsfire[num], RangeFireMode, fire_mode_tab[RangeFireMode].item,fire_mode_tab[RangeFireMode].amun);


        if(RangeFireMode == FIRE_MODE_BOW)
		{
            if(fire_mode_tab[FIRE_MODE_BOW].item == FIRE_ITEM_NO)
			{
				draw_info("no range weapon selected.",COLOR_WHITE);
                return;
			}
            if(fire_mode_tab[FIRE_MODE_BOW].amun == FIRE_ITEM_NO)
			{
				draw_info("no ammo selected.",COLOR_WHITE);
                return;
			}
            sprintf(msg,"fire %s", directions_name[num]);
		}
        else if(RangeFireMode == FIRE_MODE_THROW)
		{
            if(fire_mode_tab[FIRE_MODE_THROW].item == FIRE_ITEM_NO)
			{
				draw_info("no item selected.",COLOR_WHITE);
                return;
			}
            sprintf(msg,"throw %s",directions_name[num]);
		}
        else if(RangeFireMode == FIRE_MODE_WAND)
		{
            if(fire_mode_tab[FIRE_MODE_WAND].item == FIRE_ITEM_NO)
			{
				draw_info("no device selected.",COLOR_WHITE);
                return;
			}
            sprintf(msg,"fire device %s",directions_name[num]);
		}
        else if(RangeFireMode == FIRE_MODE_SUMMON)
		{
            sprintf(msg,"cmd golem %s", directions_name[num]);
		}

		fire_command (buf);
		/*draw_info(msg,COLOR_DGOLD);*/
		return;	
	}
	else
	{
		send_command(directions[num],-1, SC_FIRERUN);
		buf[0]=0;
	}
	strcat(buf, directions_name[num]);
	/*draw_info(buf,COLOR_DGOLD);*/
}


static void key_repeat(void)
{
	register int i;
	char buf[512];

    if(cpl.menustatus == MENU_NO)
    {
        for(i=0;i<keymap_count;i++)
    	{
	    	if(keys[keymap[i].key].pressed && keymap[i].repeatflag) /* key for this keymap is pressed*/ 		
            {
		    	if(keys[keymap[i].key].time+key_repeat_time-5 < LastTick) /* if time to repeat */
                {
			    	/* repeat x times*/
    				while((keys[keymap[i].key].time+=key_repeat_time-5) <LastTick)
	    			{
		    			if(check_macro_keys(keymap[i].text)) /* if no key macro, submit the text as cmd*/
						{
							strcpy(buf,keymap[i].text);
							if(!client_command_check(buf))
	                            send_command(buf, -1, keymap[i].mode);
	 				        draw_info(keymap[i].text,COLOR_DGOLD);
						}
			    	}
    			}
    		}
    	}
    }
    else
    {
        for(i=0;i<DEFAULT_MENU_KEYMAP;i++)
	    {
            if(menu_keymap[i].menu_mode & cpl.menustatus)
            {
		        if(keys[menu_keymap[i].key].pressed && menu_keymap[i].repeatflag) /* key for this keymap is pressed*/ 
                {
			        if(keys[menu_keymap[i].key].time+key_repeat_time-5 < LastTick) /* if time to repeat*/
                    {
				        /* repeat x times*/
			    	    while((keys[menu_keymap[i].key].time+=key_repeat_time-5) <LastTick)
				            check_menu_macro_keys(menu_keymap[i].text);
    			    }
	    	    }
            }
    	}
    }
}

void add_keybind_macro(struct _keybind_key *macro)
{
    /* new macro */
    if(macro->entry == -1)
    {
        strcpy(keymap[keymap_count].text,macro->macro);        
        strcpy(keymap[keymap_count].keyname,macro->keyname);        
        keymap[keymap_count].key = macro->key;
        keymap[keymap_count].repeatflag = macro->repeat_flag;
        keymap[keymap_count].mode = SC_NORMAL;
        keybind_entry = ++keymap_count;
    }
    else
    {
        register int i;

        if(macro->macro[0]==0)
        {
            for(i=macro->entry+1;i<keymap_count;i++)
            {
                strcpy(keymap[i-1].text,keymap[i].text);        
                strcpy(keymap[i-1].keyname,keymap[i].keyname);        
                keymap[i-1].key = keymap[i].key;
                keymap[i-1].repeatflag = keymap[i].repeatflag;
                keymap[i-1].mode = keymap[i].mode ;
            }
            --keymap_count;
        }
        else
        {
            strcpy(keymap[macro->entry].text,macro->macro);        
            strcpy(keymap[macro->entry].keyname,macro->keyname);        
            keymap[macro->entry].key = macro->key;
            keymap[macro->entry].repeatflag = macro->repeat_flag;
            keymap[macro->entry].mode = SC_NORMAL;
        }
    }
}

void read_keybind_file(char *fname)
{
	FILE *stream;
	int key, rep, len,len2;
	char *tmp, *tmp2,*tmp_e, *tmp2_e;
	char line[255];

	if( (stream = fopen( fname, "r" )) != NULL )
	{
		for(keymap_count=0;;keymap_count++)
		{
			if( fgets( line, 255, stream ) == NULL)
				break;
			sscanf(line," %d %d", &key, &rep);

			if((tmp = strchr( line, '"' )) ==NULL)
				break;
            if((tmp_e = strchr( tmp+1, '"' )) ==NULL)
                break;
            if((tmp2 = strchr( tmp_e+1, '"' )) ==NULL)
                break;
            if((tmp2_e = strrchr( line, '"' )) ==NULL)
				break;
            if((len =tmp_e-tmp-1) <=0)
                break;
            if((len2 =tmp2_e-tmp2-1) <=0)
                break;
            strncpy(keymap[keymap_count].keyname ,tmp+1,len);
            strncpy(keymap[keymap_count].text,tmp2+1,len2);
            keymap[keymap_count].keyname[len]=0;
            keymap[keymap_count].text[len2]=0;

			/* we need to know for INPUT_MODE_NUMBER "quick get" this key */
			if(!strcmp(keymap[keymap_count].text,"?M_GET"))
				get_action_keycode = key;
			if(!strcmp(keymap[keymap_count].text,"?M_DROP"))
				drop_action_keycode = key;
            
			keymap[keymap_count].key = key;
			keymap[keymap_count].repeatflag = rep;
			keymap[keymap_count].mode = SC_NORMAL;
		}
		fclose( stream );
	}
}

void save_keybind_file(char *fname)
{
    int i;
    FILE *stream;
    char buf[256];
    
    if( (stream = fopen( fname, "w+" )) != NULL )
    {
        for(i=0;i<keymap_count;i++)
        {
			/* we need to know for INPUT_MODE_NUMBER "quick get" this key */
			if(!strcmp(keymap[i].text,"?M_GET"))
				get_action_keycode = keymap[i].key;
			if(!strcmp(keymap[i].text,"?M_DROP"))
				drop_action_keycode = keymap[i].key;

            sprintf(buf, "%d %d \"%s\" \"%s\"\n",keymap[i].key,keymap[i].repeatflag, keymap[i].keyname ,keymap[i].text);
            fputs(buf, stream);
        }
        fclose( stream );        
    }
}

static Boolean process_menu_macro_keys(int id, int value)
{
    switch(id)
    {
        case KEYFUNC_SPELL_RETURN:
            
            if(spell_list[spell_list_set.group_nr].entry[spell_list_set.class_nr][spell_list_set.entry_nr].flag==LIST_ENTRY_KNOWN)
            {
                fire_mode_tab[FIRE_MODE_SPELL].spell = 
                &spell_list[spell_list_set.group_nr].entry[spell_list_set.class_nr][spell_list_set.entry_nr];
                sound_play_effect(SOUND_SCROLL,0,0,100);
		        RangeFireMode = FIRE_MODE_SPELL;
            }
            else
                sound_play_effect(SOUND_CLICKFAIL,0,0,100);
            
            map_udate_flag=2;
            cpl.menustatus = MENU_NO;            
            reset_keys();
            break;
        case KEYFUNC_SPELL_G1:
            spell_list_set.group_nr=0;
            break;
        case KEYFUNC_SPELL_G2:
            spell_list_set.group_nr=1;
            break;
        case KEYFUNC_SPELL_G3:
            spell_list_set.group_nr=2;
            break;
        case KEYFUNC_SPELL_G4:
            spell_list_set.group_nr=3;
            break;
        case KEYFUNC_SPELL_G5:
            spell_list_set.group_nr=4;
            break;
        case KEYFUNC_SPELL_G6:
            spell_list_set.group_nr=5;
            break;
        case KEYFUNC_SPELL_G7:
            spell_list_set.group_nr=6;
            break;
        case KEYFUNC_SPELL_G8:
            spell_list_set.group_nr=7;
            break;
        case KEYFUNC_SPELL_G9:
            spell_list_set.group_nr=8;
            break;
        case KEYFUNC_SPELL_G0:
            spell_list_set.group_nr=9;
            break;
        case KEYFUNC_SPELL_CURLEFT:
        spell_list_set.class_nr=0;
        break;
        case KEYFUNC_SPELL_CURRIGHT:
            spell_list_set.class_nr=1;
            break;
        case KEYFUNC_SPELL_CURUP:
            if(--spell_list_set.entry_nr<0)
                spell_list_set.entry_nr=0;
            break;
        case KEYFUNC_SPELL_CURDOWN:
            if(++spell_list_set.entry_nr>=SPELL_LIST_ENTRY)
                spell_list_set.entry_nr=SPELL_LIST_ENTRY-1;
            break;

        case KEYFUNC_SKILL_RETURN:
            
            if(skill_list[skill_list_set.group_nr].entry[skill_list_set.entry_nr].flag==LIST_ENTRY_KNOWN)
            {
                fire_mode_tab[FIRE_MODE_SKILL].skill = 
                    &skill_list[skill_list_set.group_nr].entry[skill_list_set.entry_nr];
                sound_play_effect(SOUND_SCROLL,0,0,100);
		       RangeFireMode = FIRE_MODE_SKILL;
            }
            else
                sound_play_effect(SOUND_CLICKFAIL,0,0,100);
           
            map_udate_flag=2;
            cpl.menustatus = MENU_NO;            
            reset_keys();
            break;
        case KEYFUNC_SKILL_CURLEFT:
            if(--skill_list_set.group_nr<0)
                skill_list_set.group_nr=0;
            break;    
        case KEYFUNC_SKILL_CURRIGHT:
            if(++skill_list_set.group_nr>=SKILL_LIST_MAX)
                skill_list_set.group_nr=SKILL_LIST_MAX-1;
            break;
        case KEYFUNC_SKILL_CURUP:
            if(--skill_list_set.entry_nr<0)
                skill_list_set.entry_nr=0;
            break;
        case KEYFUNC_SKILL_CURDOWN:
            if(++skill_list_set.entry_nr>=SKILL_LIST_ENTRY)
                skill_list_set.entry_nr=SKILL_LIST_ENTRY-1;
            break;
            

        case KEYFUNC_MENU_UP:
            keybind_entry--;            
            break;
        
    case KEYFUNC_MENU_DOWN:
        keybind_entry++;
        break;
        
    case KEYFUNC_KB_DONE:
        save_keybind_file(KEYBIND_FILE);
        sound_play_effect(SOUND_SCROLL,0,0,100);
        map_udate_flag=2;
        cpl.menustatus = MENU_NO;            
        reset_keys();
        break;
        
    case KEYFUNC_KB_NEW:
        sound_play_effect(SOUND_SCROLL,0,0,100);
        keybind_status = KEYBIND_STATUS_NEW;
        reset_keys();
        open_input_mode(240);
        cpl.input_mode = INPUT_MODE_GETKEY;                
        break;
    case KEYFUNC_KB_EDIT:
        sound_play_effect(SOUND_SCROLL,0,0,100);
        keybind_status = KEYBIND_STATUS_EDIT;
        reset_keys();
        open_input_mode(240);
        strcpy(InputString,keymap[keybind_entry].text);
        InputCount = strlen(keymap[keybind_entry].text);
        cpl.input_mode = INPUT_MODE_GETKEY;                
        break;
    case KEYFUNC_KB_REPEAT:
        sound_play_effect(SOUND_SCROLL,0,0,100);
        keybind_repeat=keybind_repeat?FALSE:TRUE;
        break;
        
    default:
        return FALSE;
        break;
    }
    return TRUE;
}

static void quickslot_key(SDL_KeyboardEvent *key,int slot)
{
	int tag;
	char buf[256];

	/* set or apply */
	if(key->keysym.mod & (KMOD_SHIFT|KMOD_CTRL|KMOD_ALT))
	{
		if(cpl.inventory_win == IWIN_BELOW)
			return;
		tag = cpl.win_inv_tag;
		
		if(tag == -1)
			return;
		quick_slots[slot]=tag;
		sprintf(buf,"set F%d to %s", slot+1,locate_item (tag)->s_name);
		draw_info(buf,COLOR_DGOLD);
	}
	else
	{
		if(quick_slots[slot]!=-1)
		{
			sprintf(buf,"F%d quick apply %s", slot+1,locate_item (quick_slots[slot])->s_name);
			draw_info(buf,COLOR_DGOLD);
			client_send_apply (quick_slots[slot]);
			return;
		}
		sprintf(buf,"F%d quick slot is empty", slot+1);
		draw_info(buf,COLOR_DGOLD);
	}

}

