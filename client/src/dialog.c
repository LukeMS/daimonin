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
#define TXT_START_NAME  136
#define TXT_START_LEVEL 370
#define TXT_START_EXP   400
#define TXT_Y_START      82
#define X_COL1 136
#define X_COL2 290
#define X_COL3 430

int dialog_new_char_warn = FALSE;

int add_rangebox(int x, int y, int id, int text_x, int text_, char *text, int text_color);

enum {SEL_BUTTON, SEL_CHECKBOX, SEL_RANGE, SEL_TEXT}; /* selection types */
enum {VAL_BOOL,  VAL_TEXT, VAL_CHAR, VAL_INT, VAL_U32} value_type;
enum {OPTION, SPELL, SKILL};

static int active_button=-1;


typedef struct _option
{
	char *name;
	char *info1;	/* info text row 1 */
	char *info2;	/* info text row 2 */
	char *val_text; /* text-replacement for number values */
	int  sel_type;
	int  minRange, maxRange, deltaRange;
	void *value;
	int value_type;
}_option ;

#define MIN_ATTR_DELTA -2 /* attr. can be sub by MIN_ATTR_DELTA points. */
#define MAX_ATTR_DELTA  1 /* attr. can be add by MIN_ATTR_DELTA points. */

typedef struct _attribute
{
	char *name;
	char *info;	/* info text row 1 */
	int  minRange, maxRange;
	int  value;
	int  valDelta;
}_attribute;

enum {ATT_STR, ATT_DEX, ATT_CON, ATT_INT, ATT_WIS, ATT_POW, ATT_CHA, ATT_SUM};

static _attribute attribute[] ={
	{"STR", "Strength",     8,15, 4, 0},
	{"DEX", "Dextery",      8,15, 4, 0},
	{"CON", "Constitution", 8,15, 4, 0},
	{"INT", "Intellect",    8,15, 4, 0},
	{"WIS", "Wisdom",       8,15, 4, 0},
	{"POW", "Power",        8,15, 4, 0},
	{"CHA", "Charisma",     8,15, 4, 0}
};
#define SUM_ATTRIBUTE (sizeof(attribute)/sizeof(_attribute))

char *gender[]={"male", "female", "hermaphrodite", "neuter"};
#define SUM_GENDER (sizeof(gender)/sizeof(char*))

int gen=0;

/******************************************************************
 Option Menue
******************************************************************/
static char *opt_tab[OPTION_LIST_MAX]={
	"Sound", "Server", "Visual", "Look & Feel", "Debug"
};
#define OPTION_TAB_SIZE (sizeof(opt_tab)/sizeof(char*))

static _option opt[] ={
	{"Sound", "set sound volume","", "",SEL_RANGE, 0,100,5, &options.sound_volume, VAL_INT},
	{"Music", "set music volume","Sub info","", SEL_RANGE, 0,100,5, &options.music_volume, VAL_INT},
	{"#"}, /* End of Page */
	{"dummy", "--","", "",SEL_RANGE, 0,100,5, &options.sound_volume, VAL_INT},
	{"#"}, /* End of Page */
	{"Textwindow splitted ", "","","", SEL_CHECKBOX, 0,0,0, &options.use_TextwinSplit, VAL_BOOL},
	{"Textwindows use alpha", "Don't use this if you have a very slow computer","","", SEL_CHECKBOX, 0,0,0, &options.use_TextwinAlpha, VAL_INT},
	{"Textwindows alpha value", "higher = darker","","", SEL_RANGE, 0,255,5, &options.textwin_alpha, VAL_INT},
	{"#"}, /* End of Page */
	{"Player Names", "Show Names of Players above their Heads",	"",	"show no names#show all names#show only other#show only your", SEL_RANGE, 0,3,1, &options.player_names, VAL_INT},
	{"Low health warning:", "Shows a low health warning above your head","Activatetd if health is less than the given percent value.","", SEL_RANGE, 0,100,5, &options.warning_hp, VAL_INT},
	{"Low food warning:", "Shows a low food warning above your head.","Activatetd if food is less than the given percent value.","", SEL_RANGE, 0,100,5, &options.warning_food, VAL_INT},
	{"Show Tooltips", "","", "",SEL_CHECKBOX, 0,0,0, &options.show_tooltips, VAL_BOOL},
	{"Key-info in Dialog menus", "","", "",SEL_CHECKBOX, 0,0,0, &options.show_d_key_infos, VAL_BOOL},
	{"#"}, /* End of Page */
	{"Show Framerate", "","", "",SEL_CHECKBOX, 0,0,0, &options.show_frame, VAL_BOOL},
	{"#"}, /* End of Page */
};

/******************************************************************
 Skill Menue
******************************************************************/
static char *skill_tab[SKILL_LIST_MAX]={
	"Agility", "Person", "Mental", "Physique", "Magic", "Wisdom", "Misc"
};
#define SKILL_TAB_SIZE (sizeof(skill_tab)/sizeof(char*))

/******************************************************************
 Spell Menue
******************************************************************/
static char *spell_tab[SPELL_LIST_MAX]={ /* form server->include/spellist.h->spellpathnames */
 "Protection", "Fire", "Frost", "Electricity", "Missiles", "Self",
 "Summoning", "Abjuration", "Restoration", "Detonation",
/*
 "Mind",
 "Creation", "Teleportation", "Information", "Transmutation",
 "Transferrence", "Turning", "Wounding", "Death", "Light"
*/
};
#define SPELL_TAB_SIZE (sizeof(spell_tab)/sizeof(char*))

static char *spell_class[SPELL_LIST_CLASS]={
	"Spell", "Prayer"
};

/******************************************************************
 draws a frame.
******************************************************************/
void draw_frame(int x, int y, int w, int h){
	SDL_Rect box;

	box.x = x;  box.y = y;
	box.h = h;  box.w = 1;
	SDL_FillRect(ScreenSurface, &box, sdl_gray4);
	box.x = x+w;
	box.h++;
	SDL_FillRect(ScreenSurface, &box, sdl_gray3); /* right */
	box.x = x; 	box.y+= h;
	box.w = w;  box.h = 1;
	SDL_FillRect(ScreenSurface, &box, sdl_gray4); /* bottom */
	box.x++;    box.y = y;
	SDL_FillRect(ScreenSurface, &box, sdl_gray3); /* top */
}

/******************************************************************
 decode value with given value_type to text.
******************************************************************/
char *get_value(void* value, int type){
	static char txt_value[20];
	switch (type){
		case VAL_INT:
			sprintf(txt_value, "%d",  *((int*)value));
			return txt_value;
		case VAL_U32:
			sprintf(txt_value, "%d",  *((uint32*)value));
			return txt_value;
		case VAL_CHAR:
			sprintf(txt_value, "%d",  *((uint8*)value));
			return txt_value;
		default:
			return NULL;
	}
}

/******************************************************************
 add the close button and handle mouse events on it.
******************************************************************/
void add_close_button(int x, int y, int menu){
 	int mx, my, mb;

	mb = SDL_GetMouseState(&mx, &my)& SDL_BUTTON(SDL_BUTTON_LEFT);
	StringBlt(ScreenSurface, &SystemFont, "X", x+463, y+28, COLOR_BLACK, NULL, NULL);
	if (mx >x+459 && mx < x+469 && my > y+27 && my < y+39){
		StringBlt(ScreenSurface, &SystemFont, "X", x+462, y+27, COLOR_HGOLD, NULL, NULL);
		if (mb && mb_clicked)
	 		check_menu_keys(menu, SDLK_ESCAPE);
	}else
		StringBlt(ScreenSurface, &SystemFont, "X", x+462, y+27, COLOR_WHITE, NULL, NULL);
}

/******************************************************************
 add a button and handle mouse events on it.
 text_h:
 	info for keyboard user. You can give them here a highlighted char
 	to identify the key for this button. if not needed set it to NULL.
******************************************************************/
static int add_button(int x, int y, int id, int gfxNr, char *text, char *text_h){
	char *text_sel;
 	int ret=0;
 	int mx, my, mb;

	mb = SDL_GetMouseState(&mx, &my)& SDL_BUTTON(SDL_BUTTON_LEFT);
	if (text_h)
		text_sel= text_h;
	else
	  text_sel =text;

	sprite_blt(Bitmaps[gfxNr],x, y, NULL, NULL);
	if (mx >x && my >y && mx <x+Bitmaps[gfxNr]->bitmap->w && my <y+Bitmaps[gfxNr]->bitmap->h){
		if (mb && mb_clicked && active_button <0)
  		active_button = id;
		if (active_button== id){
			sprite_blt(Bitmaps[gfxNr+1],x, y++, NULL, NULL);
			if (!mb)
				ret =1;
		}
		StringBlt(ScreenSurface, &SystemFont, text,     x+11, y+2, COLOR_BLACK, NULL, NULL);
		StringBlt(ScreenSurface, &SystemFont, text_sel, x+10, y+1, COLOR_HGOLD, NULL, NULL);
	}else{
		StringBlt(ScreenSurface, &SystemFont, text,     x+11, y+2, COLOR_BLACK, NULL, NULL);
		StringBlt(ScreenSurface, &SystemFont, text_sel, x+10, y+1, COLOR_WHITE, NULL, NULL);
	}
	return ret;
}

/******************************************************************
 add a gfx-button and handle mouse events on it.
 text_h:
 	info for keyboard user. You can give them here a highlighted char
 	to identify the key for this button. if not needed set it to NULL.
******************************************************************/
static int add_gfx_button(int x, int y, int id, int gfxNr, int gfx_inner, char *text, char *text_h){
	char *text_sel;
 	int ret=0;
 	int mx, my, mb;
 	int color = COLOR_WHITE;

	mb = SDL_GetMouseState(&mx, &my)& SDL_BUTTON(SDL_BUTTON_LEFT);
	if (text_h)
		text_sel= text_h;
	else
	  text_sel =text;

	sprite_blt(Bitmaps[gfxNr],x, y, NULL, NULL);
	sprite_blt(Bitmaps[gfx_inner],x+5, y+3, NULL, NULL);
	if (mx >x && my >y && mx <x+Bitmaps[gfxNr]->bitmap->w && my <y+Bitmaps[gfxNr]->bitmap->h){
		if (mb && mb_clicked && active_button <0)
			active_button = id;
		if (active_button== id){
			sprite_blt(Bitmaps[gfxNr+1],x, y, NULL, NULL);
			sprite_blt(Bitmaps[gfx_inner],x+6, y+4, NULL, NULL);
			if (!mb)
				ret =1;
		}
		color = COLOR_HGOLD;
	}
	x+=Bitmaps[gfxNr]->bitmap->w+10;
	y+=Bitmaps[gfxNr]->bitmap->h/2-5;
	StringBlt(ScreenSurface, &SystemFont, text, x+1, y+1, COLOR_BLACK, NULL, NULL);
	StringBlt(ScreenSurface, &SystemFont, text_sel, x, y, color, NULL, NULL);
	return ret;
}

/******************************************************************
 add a group-button and handle mouse events on it.
 text_h:
 	info for keyboard user. You can give them here a highlighted char
 	to identify the key for this button. if not needed set it to NULL.
******************************************************************/
static int add_gr_button(int x, int y, int id, int gfxNr, char *text, char *text_h){
	char *text_sel;
 	int ret=0;
 	int mx, my, mb;
 	
 	mb = SDL_GetMouseState(&mx, &my)& SDL_BUTTON(SDL_BUTTON_LEFT);
	/* use text_h (=highlighted char for keyboard user) if available. */
	if (text_h)
		text_sel= text_h;
	else
	  text_sel =text;

	if (id)
		sprite_blt(Bitmaps[++gfxNr],x, y++, NULL, NULL);
	else
		sprite_blt(Bitmaps[gfxNr],x, y, NULL, NULL);

	StringBlt(ScreenSurface, &SystemFont, text, x+11, y+2, COLOR_BLACK, NULL, NULL);
	if (mx >x && my >y && mx <x+Bitmaps[gfxNr]->bitmap->w && my <y+Bitmaps[gfxNr]->bitmap->h){
		if (mb && mb_clicked)
			ret=1;
		StringBlt(ScreenSurface, &SystemFont, text_sel, x+10, y+1, COLOR_HGOLD, NULL, NULL);
	}else
		StringBlt(ScreenSurface, &SystemFont, text_sel, x+10, y+1, COLOR_WHITE, NULL, NULL);
	return ret;
}

/******************************************************************
	text_x: 0 => text left ; !=0 => text right  of rangebox.
	text_w: size of textfield.
******************************************************************/
int add_rangebox(int x, int y, int id, int text_w, int text_x, char *text, int color){
	static int active= -1;
	SDL_Rect box;
 	int mx, my, mb;
	int width = Bitmaps[BITMAP_DIALOG_RANGE_OFF]->bitmap->w;

	mb = SDL_GetMouseState(&mx, &my)& SDL_BUTTON(SDL_BUTTON_LEFT);
	if (text_x){ /* box right of text */
		text_x = x+width+2;
	}else{
		text_x = x;
		x+= text_w;
	}
	/* draw the range gadget */
	sprite_blt(Bitmaps[BITMAP_DIALOG_RANGE_OFF],x, y, NULL, NULL);
	box.x= text_x-2;
	box.y= y+1;
	box.w= text_w+2;
	box.h= 16;
	SDL_FillRect(ScreenSurface, &box, sdl_gray1);
	StringBlt(ScreenSurface, &SystemFont, text, text_x+1, y+3, COLOR_BLACK, NULL, NULL);
	StringBlt(ScreenSurface, &SystemFont, text, text_x,   y+2, color, NULL, NULL);

	/* check for action */
	if (mx >x && my > y && my < y+18){
		if (mx < x+width/2){
			if (mb && active <0)
				active = id;
			if (active== id){
	     	sprite_blt(Bitmaps[BITMAP_DIALOG_RANGE_L],x, y, NULL, NULL);
				if (!mb){
					active= -1;
					return -1;
				}
			}
		}else if (mx < x+width){
			if (mb && active <0)
				active = id+1; /* a rangebox has 2 buttons */
			if (active== id+1){
	     	sprite_blt(Bitmaps[BITMAP_DIALOG_RANGE_R],x+width/2, y, NULL, NULL);
				if (!mb){
					active= -1;
					return 1;
				}
			}
		}
	}
	return 0;
}

/******************************************************************
 add offset to the value.
******************************************************************/
void add_value(void* value, int type, int offset, int min, int max){
	switch (type){
		case VAL_INT:
			*((int*)value)+= offset;
			if (*((int*)value) > max)
				*((int*)value) = max;
			if (*((int*)value) < min)
				*((int*)value) = min;
			break;
		case VAL_U32:
			*((uint32*)value)+= offset;
			if (*((uint32*)value) > (uint32)max)
				*((uint32*)value) = (uint32)max;
			if (*((uint32*)value) < (uint32)min)
				*((uint32*)value) = (uint32)min;
			break;
		default:
			break;
	}
}

/******************************************************************
 draws all options for the actual page.
******************************************************************/
inline void optwin_draw_options(int x, int y){
	#define LEN_NAME  111
	int i= -1, pos =0, max =-1;
	int y2 = y+347; /* for info text */
	int mxy_opt = -1;
	int page = option_list_set.group_nr;
	int id =0;
	int mx, my, mb;

	mb = SDL_GetMouseState(&mx, &my)& SDL_BUTTON(SDL_BUTTON_LEFT);
	/* find actual page */
	while (page && ++i < sizeof(opt)/sizeof(_option))
		if (opt[i].name[0] == '#')
  		--page;

	/* draw actual page */
	while (opt[++i].name && opt[i].name[0] != '#')
	{
		max++;
		StringBlt(ScreenSurface, &SystemFont, opt[i].name, x+1, y+3, COLOR_BLACK, NULL, NULL);
		switch (opt[i].sel_type){

			case SEL_CHECKBOX:
				if (mx > x && mx < x+280 && my > y && my < y+20 ){
				StringBlt(ScreenSurface, &SystemFont, opt[i].name, x, y+2, COLOR_HGOLD, NULL, NULL);
					mxy_opt = i; /* remember this tab for later use */
				}
				else
					StringBlt(ScreenSurface, &SystemFont, opt[i].name, x, y+2, COLOR_WHITE, NULL, NULL);

				sprite_blt(Bitmaps[BITMAP_DIALOG_CHECKER],x+LEN_NAME, y, NULL, NULL);
				if (*((Boolean*)opt[i].value)==TRUE){
					StringBlt(ScreenSurface, &SystemFont, "X", x+LEN_NAME+8, y+2, COLOR_BLACK, NULL, NULL);
					StringBlt(ScreenSurface, &SystemFont, "X", x+LEN_NAME+7, y+1, COLOR_WHITE, NULL, NULL);
				}
				if ((pos ==option_list_set.entry_nr && option_list_set.key_change)
  			|| (mb && mb_clicked && active_button <0 && mx > x+LEN_NAME && mx < x+LEN_NAME+20 && my > y && my < y+18)){
					mb_clicked =0;
					option_list_set.key_change =0;
					if (*((Boolean*)opt[i].value)==TRUE)
						*((Boolean*)opt[i].value)=FALSE;
					else
						*((Boolean*)opt[i].value)=TRUE;
				}
				break;

			case SEL_RANGE:
				{
					#define LEN_VALUE 100
					SDL_Rect box;
					box.x = x+LEN_NAME, box.y=y+1;
					box.h =16, box.w=LEN_VALUE;

					if (mx > x && mx < x+280 && my > y && my < y+20){
						StringBlt(ScreenSurface, &SystemFont, opt[i].name, x, y+2, COLOR_HGOLD, NULL, NULL);
						mxy_opt = i; /* remember this tab for later use */
					}
					else
						StringBlt(ScreenSurface, &SystemFont, opt[i].name, x, y+2, COLOR_WHITE, NULL, NULL);

					SDL_FillRect(ScreenSurface, &box, 0);
					if (*opt[i].val_text==0){
						StringBlt(ScreenSurface, &SystemFont,
     					get_value(opt[i].value, opt[i].value_type),
							box.x+2, y+2, COLOR_WHITE, NULL, NULL);
					}else{
						#define MAX_LEN 40
						char text[MAX_LEN+1];
						int o= *((int*)opt[i].value);
						int p=0, q=-1;
						/* find start pos of string */
						while(o && opt[i].val_text[p])
							if (opt[i].val_text[p++]=='#') o--;
						/* find end pos of string */
						while(q++ < MAX_LEN  && opt[i].val_text[p])
							if ( (text[q]=opt[i].val_text[p++])=='#') break;
						text[q]=0;
						StringBlt(ScreenSurface, &SystemFont,text, box.x+2, y+2, COLOR_WHITE, NULL, NULL);
						#undef MAX_LEN
					}
		        	sprite_blt(Bitmaps[BITMAP_DIALOG_RANGE_OFF],x+LEN_NAME+LEN_VALUE, y, NULL, NULL);


					/* keyboard event */
					if (option_list_set.key_change && option_list_set.entry_nr==pos){
						if (option_list_set.key_change==-1)
							add_value(opt[i].value, opt[i].value_type,-opt[i].deltaRange, opt[i].minRange, opt[i].maxRange);
						else if (option_list_set.key_change==1)
							add_value(opt[i].value, opt[i].value_type, opt[i].deltaRange,	opt[i].minRange, opt[i].maxRange);
						option_list_set.key_change =0;
					}

					if (mx >x+LEN_NAME+LEN_VALUE && mx < x+LEN_NAME+LEN_VALUE+14 && my > y && my < y+18){
						if (mb && active_button <0)
      				active_button = id+1; /* 2 buttons per row */
						if (active_button==id+1){
				     	sprite_blt(Bitmaps[BITMAP_DIALOG_RANGE_L],x+LEN_NAME+LEN_VALUE, y, NULL, NULL);
							if (!mb)
								add_value(opt[i].value, opt[i].value_type, -opt[i].deltaRange, opt[i].minRange, opt[i].maxRange);
						}
					}
     			else if (mx >x+LEN_NAME+LEN_VALUE+14 && mx < x+LEN_NAME+LEN_VALUE+28 && my > y && my < y+18){
						if (mb && active_button <0)	active_button = id;
						if (active_button==id){
				     	sprite_blt(Bitmaps[BITMAP_DIALOG_RANGE_R],x+LEN_NAME+LEN_VALUE+14, y, NULL, NULL);
							if (!mb)
								add_value(opt[i].value, opt[i].value_type, opt[i].deltaRange,	opt[i].minRange, opt[i].maxRange);
						}
					}
					#undef LEN_VALUE
				}
   			break;

			case SEL_BUTTON:
				sprite_blt(Bitmaps[BITMAP_DIALOG_BUTTON_UP],x, y, NULL, NULL);
				break;
		}
		y+=20;
		pos++;
		id+=2;
	}
	if (option_list_set.entry_nr> max) option_list_set.entry_nr =max;

	/* print the info text */
	x+=20;
	if (mxy_opt >=0){
		if (*opt[mxy_opt].info2==0)
			y2+=5;
		StringBlt(ScreenSurface, &SystemFont, opt[mxy_opt].info1, x+1, y2+1, COLOR_BLACK, NULL, NULL);
		StringBlt(ScreenSurface, &SystemFont, opt[mxy_opt].info1, x,   y2,   COLOR_WHITE, NULL, NULL);
		StringBlt(ScreenSurface, &SystemFont, opt[mxy_opt].info2, x+1, y2+13,COLOR_BLACK, NULL, NULL);
		StringBlt(ScreenSurface, &SystemFont, opt[mxy_opt].info2, x,   y2+12,COLOR_WHITE, NULL, NULL);
	}
	#undef LEN_NAME
}

/******************************************************************
 draws all tabs on the left side of window.
******************************************************************/
static void draw_tabs(char *tabs[], int sum, int *act_tab, char *head_text, int x, int y){
	int i=-1;
	int mx, my, mb;
	static int active =0;

	mb = SDL_GetMouseState(&mx, &my)& SDL_BUTTON(SDL_BUTTON_LEFT);
	sprite_blt(Bitmaps[BITMAP_DIALOG_TAB_START],x, y-10, NULL, NULL);
	sprite_blt(Bitmaps[BITMAP_DIALOG_TAB    ],x, y, NULL, NULL);
	StringBlt(ScreenSurface, &SystemFont, head_text, x+15, y+4, COLOR_BLACK, NULL, NULL);
	StringBlt(ScreenSurface, &SystemFont, head_text, x+14, y+3, COLOR_WHITE, NULL, NULL);
	y+=17;
	sprite_blt(Bitmaps[BITMAP_DIALOG_TAB    ],x, y, NULL, NULL);
	y+=17;
	while (++i < sum){
		sprite_blt(Bitmaps[BITMAP_DIALOG_TAB    ],x, y, NULL, NULL);
		if (i== *act_tab)
			sprite_blt(Bitmaps[BITMAP_DIALOG_TAB_SEL],x, y, NULL, NULL);
		StringBlt(ScreenSurface, &SystemFont, tabs[i], x+25, y+4, COLOR_BLACK, NULL, NULL);
		if (mx > x && mx < x+100 && my > y && my < y+17){
			StringBlt(ScreenSurface, &SystemFont, tabs[i], x+24, y+3, COLOR_HGOLD, NULL, NULL);
			if (mb && mb_clicked)
				active =1;
			if (active)
		   *act_tab = i;
		}
		else
			StringBlt(ScreenSurface, &SystemFont, tabs[i], x+24, y+3, COLOR_WHITE, NULL, NULL);
		y+=17;
	}
	sprite_blt(Bitmaps[BITMAP_DIALOG_TAB_STOP],x, y, NULL, NULL);
	if (!mb) active =0;
}

/******************************************************************
 show the skill-window and handle mouse actions.
******************************************************************/
void show_skilllist(void)
{
	SDL_Rect box;
	char buf[256];
	int x, y, i;
	int mx, my, mb;
	static int active=0, dblclk=0;
	static Uint32 Ticks= 0;

	mb = SDL_GetMouseState(&mx, &my);
	/* background */
	x= SCREEN_XLEN/2-Bitmaps[BITMAP_DIALOG_BG]->bitmap->w/2;
	y= SCREEN_YLEN/2-Bitmaps[BITMAP_DIALOG_BG]->bitmap->h/2;
	sprite_blt(Bitmaps[BITMAP_DIALOG_BG],x, y, NULL, NULL);
	sprite_blt(Bitmaps[BITMAP_DIALOG_TITLE_SKILL],x+250-Bitmaps[BITMAP_DIALOG_TITLE_SKILL]->bitmap->w/2, y+20, NULL, NULL);
	add_close_button(x, y, MENU_SKILL);

	/* tabs */
	draw_tabs(skill_tab, SKILL_TAB_SIZE, &skill_list_set.group_nr, "Skill Group", x+8, y+ 70);

	sprintf(buf,"press ~SHIFT~ + ~%c%c~ to select group           ~%c%c~ to select skill            ~RETURN~ for use",ASCII_UP, ASCII_DOWN,ASCII_UP, ASCII_DOWN);
	StringBlt(ScreenSurface,&SystemFont, buf, x+135, y+410, COLOR_WHITE,NULL, NULL);

	/* Headline */
	StringBlt(ScreenSurface, &SystemFont, "Name", x+TXT_START_NAME+1, y+TXT_Y_START-1, COLOR_BLACK, NULL, NULL);
	StringBlt(ScreenSurface, &SystemFont, "Name", x+TXT_START_NAME,   y+TXT_Y_START-2, COLOR_WHITE, NULL, NULL);
	StringBlt(ScreenSurface, &SystemFont, "Level", x+TXT_START_LEVEL+1, y+TXT_Y_START-1, COLOR_BLACK, NULL, NULL);
	StringBlt(ScreenSurface, &SystemFont, "Level", x+TXT_START_LEVEL, y+TXT_Y_START-2, COLOR_WHITE, NULL, NULL);
	StringBlt(ScreenSurface, &SystemFont, "Experience", x+TXT_START_EXP+1, y+TXT_Y_START-1, COLOR_BLACK, NULL, NULL);
	StringBlt(ScreenSurface, &SystemFont, "Experience", x+TXT_START_EXP, y+TXT_Y_START-2, COLOR_WHITE, NULL, NULL);

	box.x= x+133;
	box.y= y+TXT_Y_START+1;
	box.w= 329;
	box.h= 12;

	/* frame for selection field */
	draw_frame(box.x-1, box.y+11, box.w+1, 313);

  /* print skill entries */
	if (!(mb & SDL_BUTTON(SDL_BUTTON_LEFT))) active =0;
	if (mx > x +TXT_START_NAME && mx < x+TXT_START_NAME+327 && my > y+TXT_Y_START
 	&& my < y+12+TXT_Y_START + DIALOG_LIST_ENTRY*12)
  {
   	if (!mb)
    {
			if (dblclk==1) dblclk=2;
			if (dblclk==3){
	   		dblclk=0;
	   		if (skill_list[skill_list_set.group_nr].entry[skill_list_set.entry_nr].flag==LIST_ENTRY_KNOWN)
			 		check_menu_keys(MENU_SKILL, SDLK_RETURN);
			}
		}else{
				if (dblclk==0){
	   			dblclk=1;
	   			Ticks = SDL_GetTicks();
				}
				if (dblclk==2){
					dblclk=3;
					if (SDL_GetTicks() - Ticks > 300)
						dblclk=0;
				}
	  	if (mb_clicked) /* mb was pressed in the selection field */
				active =1;
			if (active && skill_list_set.entry_nr != (my-y-12-TXT_Y_START)/12){
				skill_list_set.entry_nr = (my-y-12-TXT_Y_START)/12;
	   		dblclk=0;
			}
	 	}
 	}
	for(i=0;i<DIALOG_LIST_ENTRY;i++){
		y+=12;
		box.y+=12;
		if (i !=skill_list_set.entry_nr){
			if (i&1)
				SDL_FillRect(ScreenSurface, &box, sdl_gray2);
			else
				SDL_FillRect(ScreenSurface, &box, sdl_gray1);
	  }else
	  	SDL_FillRect(ScreenSurface, &box, sdl_blue1);

 		if(skill_list[skill_list_set.group_nr].entry[i].flag==LIST_ENTRY_KNOWN)
		{
			StringBlt(ScreenSurface, &SystemFont, skill_list[skill_list_set.group_nr].entry[i].name
				,x+TXT_START_NAME, y+TXT_Y_START, COLOR_WHITE, NULL, NULL);
			if(skill_list[skill_list_set.group_nr].entry[i].exp != -1)
				sprintf(buf,"%d",skill_list[skill_list_set.group_nr].entry[i].exp_level);
			else
				strcpy(buf,"--");
			StringBlt(ScreenSurface, &SystemFont,buf,x+TXT_START_LEVEL, y+TXT_Y_START, COLOR_WHITE, NULL, NULL);
			if(skill_list[skill_list_set.group_nr].entry[i].exp != -1)
				sprintf(buf,"%d",skill_list[skill_list_set.group_nr].entry[i].exp);
			StringBlt(ScreenSurface, &SystemFont,buf,x+TXT_START_EXP, y+TXT_Y_START, COLOR_WHITE, NULL, NULL);
		}
	}
	x+=160; y+=120;

  /* print skill description */
	if(skill_list[skill_list_set.group_nr].entry[skill_list_set.entry_nr].flag>=LIST_ENTRY_KNOWN)
	{
		if ((mb & SDL_BUTTON(SDL_BUTTON_LEFT)) && mx >x-40 && mx < x-10 && my > y+10 && my < y+43){ // selected
	 		check_menu_keys(MENU_SKILL, SDLK_RETURN);
  	}
		sprite_blt(skill_list[skill_list_set.group_nr].entry[skill_list_set.entry_nr].icon,x-42,y+10, NULL, NULL);
		/* center ypos of textblock */
		for (i=0; i < 4;i++)
			if (skill_list[skill_list_set.group_nr].entry[skill_list_set.entry_nr].desc[i][0]== ' ')
				y+=6;
		/* print textblock */
		for (i=0; i <= 3;i++){
			StringBlt(ScreenSurface, &SystemFont,
            &skill_list[skill_list_set.group_nr].entry[skill_list_set.entry_nr].desc[i][0]
            ,x-2, y+1, COLOR_BLACK, NULL, NULL);
			StringBlt(ScreenSurface, &SystemFont,
            &skill_list[skill_list_set.group_nr].entry[skill_list_set.entry_nr].desc[i][0]
            ,x-3, y, COLOR_WHITE, NULL, NULL);
			y+=13;
		}
	}
	if (!mb) active_button =-1;
}

/******************************************************************
 show the spell-window and handle mouse actions.
******************************************************************/
void show_spelllist(void)
{
	SDL_Rect box;
	char buf[256];
	int x,y, i;
	int mx, my, mb;
	static int active=0, dblclk=0;
	static Uint32 Ticks= 0;

	mb = SDL_GetMouseState(&mx, &my)& SDL_BUTTON(SDL_BUTTON_LEFT);
	/* background */
	x= SCREEN_XLEN/2-Bitmaps[BITMAP_DIALOG_BG]->bitmap->w/2;
	y= SCREEN_YLEN/2-Bitmaps[BITMAP_DIALOG_BG]->bitmap->h/2;
	sprite_blt(Bitmaps[BITMAP_DIALOG_BG],x, y, NULL, NULL);
	sprite_blt(Bitmaps[BITMAP_DIALOG_TITLE_SPELL],x+250-Bitmaps[BITMAP_DIALOG_TITLE_SPELL]->bitmap->w/2, y+20, NULL, NULL);
	sprite_blt(Bitmaps[BITMAP_PENTAGRAM],x+25, y+430, NULL, NULL);
	add_close_button(x, y, MENU_SPELL);

	/* tabs */
	draw_tabs(spell_tab, SPELL_TAB_SIZE, &spell_list_set.group_nr, "Spell Path" ,x+8, y+ 70);

	sprintf(buf,"press ~SHIFT~ + ~%c%c~ to select path           ~%c%c~ to select spell            ~RETURN~ for use",ASCII_UP, ASCII_DOWN,ASCII_UP, ASCII_DOWN);
	StringBlt(ScreenSurface,&SystemFont, buf, x+135, y+410, COLOR_WHITE,NULL, NULL);

	/* spellClass buttons */
	for (i=0; i <SPELL_LIST_CLASS; i++){
		if (add_gr_button(x+133+i*56, y+75, (spell_list_set.class_nr==i), BITMAP_DIALOG_BUTTON_UP, spell_class[i], NULL))
				spell_list_set.class_nr= i;
	}

	sprintf(buf,"use ~%c%c~ to select spell group",ASCII_RIGHT, ASCII_LEFT);
	StringBlt(ScreenSurface,&SystemFont, buf, x+340, y+80, COLOR_WHITE,NULL, NULL);

	box.x= x+133;
	box.y= y+TXT_Y_START+1;
	box.w= 329;
	box.h= 12;

	/* frame for selection field */
	draw_frame(box.x-1, box.y+11, box.w+1, 313);

  /* print skill entries */
	if (!mb) active =0;
	if (mx > x +TXT_START_NAME && mx < x+TXT_START_NAME+327 && my > y+TXT_Y_START
 	&& my < y+12+TXT_Y_START + DIALOG_LIST_ENTRY*12)
 	{
   	if (!mb){
			if (dblclk==1) dblclk=2;
			if (dblclk==3){
	   		dblclk=0;
					if (spell_list[spell_list_set.group_nr]
     				.entry[spell_list_set.class_nr][spell_list_set.entry_nr].flag==LIST_ENTRY_KNOWN)
		 		check_menu_keys(MENU_SPELL, SDLK_RETURN);
			}
		}else{
				if (dblclk==0){
	   			dblclk=1;
	   			Ticks = SDL_GetTicks();
				}
				if (dblclk==2){
					dblclk=3;
					if (SDL_GetTicks() - Ticks > 300)
						dblclk=0;
				}
	  	if (mb_clicked) /* mb was pressed in the selection field */
				active =1;
			if (active && spell_list_set.entry_nr != (my-y-12-TXT_Y_START)/12){
				spell_list_set.entry_nr = (my-y-12-TXT_Y_START)/12;;
	   		dblclk=0;
			}
	 	}
 	}
	for(i=0;i<DIALOG_LIST_ENTRY;i++){
		y+=12;
		box.y+=12;
		if (i !=spell_list_set.entry_nr){
			if (i&1)
				SDL_FillRect(ScreenSurface, &box, sdl_gray2);
			else
				SDL_FillRect(ScreenSurface, &box, sdl_gray1);
	  }else
	  	SDL_FillRect(ScreenSurface, &box, sdl_blue1);

		if(spell_list[spell_list_set.group_nr].entry[spell_list_set.class_nr][i].flag==LIST_ENTRY_KNOWN)
		{
			StringBlt(ScreenSurface, &SystemFont
   			, spell_list[spell_list_set.group_nr].entry[spell_list_set.class_nr][i].name
				, x+TXT_START_NAME, y+TXT_Y_START, COLOR_WHITE, NULL, NULL);
		}
	}

	x+=160; y+=120;
  /* print spell description */
	if(spell_list[spell_list_set.group_nr].entry[spell_list_set.class_nr][spell_list_set.entry_nr].flag==LIST_ENTRY_KNOWN)
	{
		if (mb && mx >x-40 && mx < x-10 && my > y+10 && my < y+43){ // selected
   		dblclk=0;
	 		check_menu_keys(MENU_SPELL, SDLK_RETURN);
		}
		sprite_blt(spell_list[spell_list_set.group_nr].entry[spell_list_set.class_nr][spell_list_set.entry_nr].icon,x-42,y+10, NULL, NULL);
		/* center ypos of textblock */
		for (i=0; i < 4;i++)
			if (spell_list[spell_list_set.group_nr].entry[spell_list_set.class_nr][spell_list_set.entry_nr].desc[i][0]== ' ')
				y+=6;
		/* print textblock */
		for (i=0; i < 4;i++){
			StringBlt(ScreenSurface, &SystemFont,
            &spell_list[spell_list_set.group_nr].entry[spell_list_set.class_nr][spell_list_set.entry_nr].desc[i][0]
            ,x-2, y+1, COLOR_BLACK, NULL, NULL);
			StringBlt(ScreenSurface, &SystemFont,
            &spell_list[spell_list_set.group_nr].entry[spell_list_set.class_nr][spell_list_set.entry_nr].desc[i][0]
            ,x-3, y, COLOR_WHITE, NULL, NULL);
			y+=13;
		}
	}
	if (!mb) active_button =-1;
}

/******************************************************************
 show the option-window and handle mouse actions.
******************************************************************/
void show_optwin()
{
	int x, y;
	int mx, my, mb;
	int numButton =0;

	mb = SDL_GetMouseState(&mx, &my)& SDL_BUTTON(SDL_BUTTON_LEFT);
	x= SCREEN_XLEN/2-Bitmaps[BITMAP_DIALOG_BG]->bitmap->w/2;
	y= SCREEN_YLEN/2-Bitmaps[BITMAP_DIALOG_BG]->bitmap->h/2;
	sprite_blt(Bitmaps[BITMAP_DIALOG_BG],x, y, NULL, NULL);
	sprite_blt(Bitmaps[BITMAP_DIALOG_TITLE_OPTIONS],x+250-Bitmaps[BITMAP_DIALOG_TITLE_OPTIONS]->bitmap->w/2, y+20, NULL, NULL);
	add_close_button(x, y, MENU_OPTION);

	draw_tabs(opt_tab, OPTION_TAB_SIZE, &option_list_set.group_nr,  "Option Group", x+  8, y+ 70);
	optwin_draw_options(x+130, y+ 90);

	/* mark active entry */
	StringBlt(ScreenSurface, &SystemFont, ">", x+TXT_START_NAME-15
 		, y+10+TXT_Y_START+option_list_set.entry_nr*20, COLOR_HGOLD, NULL, NULL);

	/* save button */
	if (add_button(x+25, y+454, numButton++, BITMAP_DIALOG_BUTTON_UP, "Done", "�D�one"))
		check_menu_keys(MENU_OPTION, SDLK_d);

	if (!mb) active_button =-1;
}

/******************************************************************
 show the keybind-window and handle mouse actions.
******************************************************************/
void show_keybind()
{
	SDL_Rect box;
	char buf[256];
	int x, x2, y, y2, i;
	int mx, my, mb;
	int numButton =0;

	mb = SDL_GetMouseState(&mx, &my);
	/* background */
	x= SCREEN_XLEN/2-Bitmaps[BITMAP_DIALOG_BG]->bitmap->w/2;
	y= SCREEN_YLEN/2-Bitmaps[BITMAP_DIALOG_BG]->bitmap->h/2;
	sprite_blt(Bitmaps[BITMAP_DIALOG_BG],x, y, NULL, NULL);
	sprite_blt(Bitmaps[BITMAP_DIALOG_TITLE_KEYBIND],x+250-Bitmaps[BITMAP_DIALOG_TITLE_KEYBIND]->bitmap->w/2, y+20, NULL, NULL);
	add_close_button(x, y, MENU_KEYBIND);

	sprintf(buf,"~SHIFT~ + ~%c%c~ to select group         ~%c%c~ to select macro          ~RETURN~ to change/create",ASCII_UP, ASCII_DOWN,ASCII_UP, ASCII_DOWN);
	StringBlt(ScreenSurface,&SystemFont, buf, x+125, y+410, COLOR_WHITE,NULL, NULL);


	/* draw group tabs */
	i=0;
	x2= x+8; y2= y+70;
	sprite_blt(Bitmaps[BITMAP_DIALOG_TAB_START],x2, y2-10, NULL, NULL);
	sprite_blt(Bitmaps[BITMAP_DIALOG_TAB    ],x2, y2, NULL, NULL);
	StringBlt(ScreenSurface, &SystemFont, "Group", x2+15, y2+4, COLOR_BLACK, NULL, NULL);
	StringBlt(ScreenSurface, &SystemFont, "Group", x2+14, y2+3, COLOR_WHITE, NULL, NULL);
	y2+=17;
	sprite_blt(Bitmaps[BITMAP_DIALOG_TAB    ],x2, y2, NULL, NULL);
	y2+=17;
	while (i < BINDKEY_LIST_MAX && bindkey_list[i].name[0]){
		sprite_blt(Bitmaps[BITMAP_DIALOG_TAB],x2, y2, NULL, NULL);
		if (i== bindkey_list_set.group_nr)
			sprite_blt(Bitmaps[BITMAP_DIALOG_TAB_SEL],x2, y2, NULL, NULL);
		StringBlt(ScreenSurface, &SystemFont, bindkey_list[i].name, x2+25, y2+4, COLOR_BLACK, NULL, NULL);
		if (mx > x2 && mx < x2+100 && my > y2 && my < y2+17){
			StringBlt(ScreenSurface, &SystemFont, bindkey_list[i].name, x2+24, y2+3, COLOR_HGOLD, NULL, NULL);
			if (mb && bindkey_list_set.group_nr != i) bindkey_list_set.group_nr = i;
		}
		else
			StringBlt(ScreenSurface, &SystemFont, bindkey_list[i].name, x2+24, y2+3, COLOR_WHITE, NULL, NULL);
		y2+=17;
		i++;
	}
	sprite_blt(Bitmaps[BITMAP_DIALOG_TAB_STOP],x2, y2, NULL, NULL);

	/* Headline */
	StringBlt(ScreenSurface, &SystemFont, "Macro", x+X_COL1+1, y+TXT_Y_START-1, COLOR_BLACK, NULL, NULL);
	StringBlt(ScreenSurface, &SystemFont, "Macro", x+X_COL1,   y+TXT_Y_START-2, COLOR_WHITE, NULL, NULL);
	StringBlt(ScreenSurface, &SystemFont, "Key",    x+X_COL2+1, y+TXT_Y_START-1, COLOR_BLACK, NULL, NULL);
	StringBlt(ScreenSurface, &SystemFont, "Key",    x+X_COL2, y+TXT_Y_START-2, COLOR_WHITE, NULL, NULL);
	StringBlt(ScreenSurface, &SystemFont, "Repeat", x+X_COL3+1, y+TXT_Y_START-1, COLOR_BLACK, NULL, NULL);
	StringBlt(ScreenSurface, &SystemFont, "~R~epeat", x+X_COL3, y+TXT_Y_START-2, COLOR_WHITE, NULL, NULL);

	/* save button */
	if (add_button(x+25, y+454, numButton++, BITMAP_DIALOG_BUTTON_UP, "Done", "~D~one"))
		check_menu_keys(MENU_KEYBIND, SDLK_d);


	box.x= x+133;
	box.y= y+TXT_Y_START+1;
	box.w= 329;
	box.h= 12;

	/* frame for selection field */
	draw_frame(box.x-1, box.y+11, box.w+1, 313);
	y2=y;

  /* print keybind entries */
	for(i=0;i<OPTWIN_MAX_OPT;i++){
		y+=12;
		box.y+=12;
		if (mb && mx > x +TXT_START_NAME && mx < x+TXT_START_NAME+327 && my > y+TXT_Y_START && my < y+12+TXT_Y_START)
		{
			if (mb & SDL_BUTTON(SDL_BUTTON_LEFT)){ /* cancel edit */
   			 InputStringEscFlag = TRUE;
   			 keybind_status = KEYBIND_STATUS_NO;
			}
 			bindkey_list_set.entry_nr = i;
			if ((mb & SDL_BUTTON(SDL_BUTTON_RIGHT)) && mb_clicked && mx > x+X_COL1){
				if  (mx < x+X_COL3) /* macro name + key value */
			 		check_menu_keys(MENU_KEYBIND, SDLK_RETURN);
				else  /* key repeat */
			 		check_menu_keys(MENU_KEYBIND, SDLK_r);
				mb_clicked =0;
			}
		}
		if (i !=bindkey_list_set.entry_nr){
			if (i&1)
				SDL_FillRect(ScreenSurface, &box, sdl_gray2);
			else
				SDL_FillRect(ScreenSurface, &box, sdl_gray1);
	  }else
	  	SDL_FillRect(ScreenSurface, &box, sdl_blue1);

		if (bindkey_list[bindkey_list_set.group_nr].entry[i].text[0])
		{
			StringBlt(ScreenSurface, &SystemFont, bindkey_list[bindkey_list_set.group_nr].entry[i].text,    x+X_COL1, y+TXT_Y_START, COLOR_WHITE, NULL, NULL);
			StringBlt(ScreenSurface, &SystemFont, bindkey_list[bindkey_list_set.group_nr].entry[i].keyname, x+X_COL2, y+TXT_Y_START, COLOR_WHITE, NULL, NULL);
			if (bindkey_list[bindkey_list_set.group_nr].entry[i].repeatflag)
				StringBlt(ScreenSurface, &SystemFont, "on", x+X_COL3, y+TXT_Y_START, COLOR_WHITE, NULL, NULL);
			else
				StringBlt(ScreenSurface, &SystemFont, "off", x+X_COL3, y+TXT_Y_START, COLOR_WHITE, NULL, NULL);
		}
	}

	/* Edit modus */
	y2+=13+TXT_Y_START+bindkey_list_set.entry_nr*12;
	box.y = y2;
	if(keybind_status == KEYBIND_STATUS_EDIT){
		box.w = X_COL2-X_COL1;
		SDL_FillRect(ScreenSurface, &box, 0);
		StringBlt(ScreenSurface, &SystemFont, show_input_string(InputString,&SystemFont,box.w)
			,x+X_COL1, y2, COLOR_WHITE, NULL, NULL);
	}else	if(keybind_status == KEYBIND_STATUS_EDITKEY){
		box.x+= X_COL2-X_COL1;
		box.w = X_COL3-X_COL2;
		SDL_FillRect(ScreenSurface, &box, 0);
		StringBlt(ScreenSurface, &SystemFont, "Press a Key!", x+X_COL2, y2, COLOR_WHITE, NULL, NULL);
	}

	if (!mb) active_button =-1;
}

static void blit_face(int id, int x, int y)
{
    if(id == -1 || !FaceList[id].sprite)
        return;
    
    if(FaceList[id].sprite->status != SPRITE_STATUS_LOADED)
        return;

	sprite_blt(FaceList[id].sprite ,x, y, NULL, NULL);
}

#define CREATE_Y0 120
/******************************************************************
 show hero creation dialog.
******************************************************************/
/* as nice this menu is there is a technical problem here:
 * in this function is the input, setting and display mixed to one
 * function of source. There are plenty of books and tutorials out
 * explaining why it is bad - if this application becomes more complex
 * we need to migrate some stuff in own functions.
 * MT-2004
 */
void show_newplayer_server(void)
{
	int id=0;
	int x,y, i;
	char buf[256];
	int mx, my, mb;
	int delta;
	_server_char *tmpc;

	mb = SDL_GetMouseState(&mx, &my)& SDL_BUTTON(SDL_BUTTON_LEFT);
	x= SCREEN_XLEN/2-Bitmaps[BITMAP_DIALOG_BG]->bitmap->w/2;
	y= SCREEN_YLEN/2-Bitmaps[BITMAP_DIALOG_BG]->bitmap->h/2;
	sprite_blt(Bitmaps[BITMAP_DIALOG_BG],x, y, NULL, NULL);
	sprite_blt(Bitmaps[BITMAP_DIALOG_TITLE_CREATION],x+250-Bitmaps[BITMAP_DIALOG_TITLE_CREATION]->bitmap->w/2, y+20, NULL, NULL);
	sprite_blt(Bitmaps[BITMAP_PENTAGRAM],x+25, y+430, NULL, NULL);
	add_close_button(x, y, MENU_CREATE);

	/* print all attributes */
  StringBlt(ScreenSurface, &SystemFont, "Welcome!", x+131, y+64, COLOR_BLACK, NULL, NULL);
  StringBlt(ScreenSurface, &SystemFont, "Welcome!", x+130, y+63, COLOR_WHITE, NULL, NULL);
  sprintf(buf,"Use ~%c%c~ and ~%c%c~ cursor keys to setup your stats.", ASCII_UP, ASCII_DOWN, ASCII_RIGHT, ASCII_LEFT);
  StringBlt(ScreenSurface, &SystemFont, buf, x+131, y+76, COLOR_BLACK, NULL, NULL);
  StringBlt(ScreenSurface, &SystemFont, buf, x+130, y+75, COLOR_WHITE, NULL, NULL);
  StringBlt(ScreenSurface, &SystemFont, "Adjust the stats using *all* your points.", x+131, y+89, COLOR_BLACK, NULL, NULL);
  StringBlt(ScreenSurface, &SystemFont, "Adjust the stats using *all* your points.", x+130, y+88, COLOR_WHITE, NULL, NULL);
  StringBlt(ScreenSurface, &SystemFont, "Press ~C~ to create your new character", x+131, y+101, COLOR_BLACK, NULL, NULL);
  StringBlt(ScreenSurface, &SystemFont, "Press ~C~ to create your new character.", x+130, y+100, COLOR_WHITE, NULL, NULL);


	/* create button */
	if (add_button(x+30, y+397, id++, BITMAP_DIALOG_BUTTON_UP, "Create", "~C~reate"))
		check_menu_keys(MENU_CREATE, SDLK_c);

	if(dialog_new_char_warn == TRUE)
	{
		StringBlt(ScreenSurface, &SystemFont, "  ** ASSIGN ALL **", x+21, y+368, COLOR_BLACK, NULL, NULL);
		StringBlt(ScreenSurface, &SystemFont, "  ** ASSIGN ALL **", x+20, y+367, COLOR_HGOLD, NULL, NULL);
		StringBlt(ScreenSurface, &SystemFont, "** POINTS FIRST **", x+21, y+380, COLOR_BLACK, NULL, NULL);
		StringBlt(ScreenSurface, &SystemFont, "** POINTS FIRST **", x+20, y+379, COLOR_HGOLD, NULL, NULL);
	}

	/* draw attributes */
	StringBlt(ScreenSurface, &SystemFont, "Points:", x+130, y+CREATE_Y0+3*17, COLOR_WHITE, NULL, NULL);
	if(new_character.stat_points)
		sprintf(buf, "%.2d  LEFT", new_character.stat_points);
	else
		sprintf(buf, "%.2d", new_character.stat_points);
	StringBlt(ScreenSurface, &SystemFont, buf , x+171, y+CREATE_Y0+3*17+1, COLOR_BLACK, NULL, NULL);
	StringBlt(ScreenSurface, &SystemFont, buf , x+170, y+CREATE_Y0+3*17, COLOR_HGOLD, NULL, NULL);
	
	if(create_list_set.entry_nr > 8)
		create_list_set.entry_nr =8;

	for (i=0; i< ATT_SUM; i++)
	{
		id+=2;
		sprintf(buf, "%s:", attribute[i].name);

		if(create_list_set.entry_nr == i+2)
		    StringBlt(ScreenSurface, &SystemFont, buf, x+130, y+CREATE_Y0+(i+4)*17, COLOR_GREEN, NULL, NULL);
		else
		    StringBlt(ScreenSurface, &SystemFont, buf, x+130, y+CREATE_Y0+(i+4)*17, COLOR_WHITE, NULL, NULL);
		sprintf(buf, "%.2d", new_character.stats[i]);

		if(create_list_set.entry_nr == i+2)
			delta = add_rangebox(x+170, y+CREATE_Y0+(i+4)*17, id, 20, 0, buf, COLOR_GREEN);
		else
			delta = add_rangebox(x+170, y+CREATE_Y0+(i+4)*17, id, 20, 0, buf, COLOR_WHITE);
		/* keyboard event */

		if (create_list_set.key_change && create_list_set.entry_nr == i+2)
		{
				delta=create_list_set.key_change;
				create_list_set.key_change =0;
		}

		if(delta)
		{
			dialog_new_char_warn = FALSE;
			if(delta>0)
			{
				if(new_character.stats[i]+1 <= new_character.stats_max[i] && new_character.stat_points)
				{
					new_character.stats[i]++;
					new_character.stat_points--;
				}
			}
			else
			{
				if(new_character.stats[i]-1 >= new_character.stats_min[i])
				{
					new_character.stats[i]--;
					new_character.stat_points++;
				}
			}
		}
	}


	for(i=0,tmpc=first_server_char;tmpc;tmpc=tmpc->next,i++)
	{
		SDL_Rect box;
		box.h = 55;
		box.w = 55;
		box.x = x+125+i*60;
		box.y = y+320;

		SDL_FillRect(ScreenSurface, &box, sdl_gray2);
		blit_face(tmpc->pic_id ,box.x+5, box.y+5);
		StringBlt(ScreenSurface, &Font6x3Out, tmpc->name, box.x+10, box.y+40, COLOR_WHITE, NULL, NULL);

		if(!strcmp(tmpc->name, new_character.name))
			sprite_blt(Bitmaps[BITMAP_NCHAR_MARKER] ,box.x,box.y, NULL, NULL);

	}


	if(create_list_set.entry_nr == 0)
		StringBlt(ScreenSurface, &SystemFont, "Race:",x+130, y+CREATE_Y0+0*17+2, COLOR_GREEN, NULL, NULL);
	else
		StringBlt(ScreenSurface, &SystemFont, "Race:",x+130, y+CREATE_Y0+0*17+2, COLOR_WHITE, NULL, NULL);
	if(create_list_set.entry_nr == 0)
		delta= add_rangebox(x+170, y+CREATE_Y0+0*17, ++id, 80, 0, new_character.name , COLOR_GREEN);
	else
		delta= add_rangebox(x+170, y+CREATE_Y0+0*17, ++id, 80, 0, new_character.name, COLOR_WHITE);
	if (create_list_set.key_change && create_list_set.entry_nr == 0){
		delta=create_list_set.key_change;
		create_list_set.key_change =0;
	}

	if(delta) /* init new race */
	{
		int g;

		for(tmpc=first_server_char;tmpc;tmpc=tmpc->next)
		{
			/* get our current template */
			if(!strcmp(tmpc->name, new_character.name))
			{
				/* get next template */
				if(delta>0)
				{
					tmpc = tmpc->next;
					if(!tmpc)
						tmpc = first_server_char;
				}
				else
				{
					tmpc = tmpc->prev;
					if(!tmpc)
					{
						/* get last node */
						for(tmpc=first_server_char;tmpc->next;tmpc=tmpc->next)
							;
					}

				}
				memcpy(&new_character, tmpc, sizeof(_server_char));

				/* adjust gender */
				for(g=0;g<4;g++)
				{
					if(new_character.gender[g])
					{
						new_character.gender_selected = g;
						break;
					}
				}
				break;
			}
		}
	}

	if(create_list_set.entry_nr == 1)
		StringBlt(ScreenSurface, &SystemFont, "Gender:",x+130, y+CREATE_Y0+1*17+2, COLOR_GREEN, NULL, NULL);
	else
		StringBlt(ScreenSurface, &SystemFont, "Gender:",x+130, y+CREATE_Y0+1*17+2, COLOR_WHITE, NULL, NULL);
	if(create_list_set.entry_nr == 1)
		delta= add_rangebox(x+170, y+CREATE_Y0+1*17, ++id, 80, 0, gender[new_character.gender_selected], COLOR_GREEN);
	else
		delta= add_rangebox(x+170, y+CREATE_Y0+1*17, ++id, 80, 0, gender[new_character.gender_selected], COLOR_WHITE);
	if (create_list_set.key_change && create_list_set.entry_nr == 1){
		delta=create_list_set.key_change;
		create_list_set.key_change =0;
	}
	if(delta)
	{
		int g, tmp_g;

		if(delta>0) /* +1 */
		{
			for(g=0;g<4;g++)
			{
				tmp_g = ++new_character.gender_selected;
				if(tmp_g >3)
					tmp_g-=4;
				if(new_character.gender[tmp_g])
				{
					new_character.gender_selected=tmp_g;
					break;
				}
			}

		}
		else
		{
			for(g=3;g>=0;g--)
			{
				tmp_g = new_character.gender_selected-g;
				if(tmp_g <0)
					tmp_g+=4;

				if(new_character.gender[tmp_g])
				{
					new_character.gender_selected=tmp_g;
					break;
				}
			}
		}
	}

	/* draw player image */
	StringBlt(ScreenSurface, &SystemFont, cpl.name, x+40, y+85, COLOR_WHITE, NULL, NULL);

	blit_face(new_character.face_id[new_character.gender_selected] ,x+35, y+100);
	sprintf(buf, "HP: ~%d~", new_character.bar[0]*4+new_character.bar_add[0]);
	StringBlt(ScreenSurface, &SystemFont, buf, x+36, y+146, COLOR_BLACK, NULL, NULL);
	StringBlt(ScreenSurface, &SystemFont, buf, x+35, y+145, COLOR_WHITE, NULL, NULL);
	sprintf(buf, "SP: ~%d~", new_character.bar[1]*2+new_character.bar_add[1]);
	StringBlt(ScreenSurface, &SystemFont, buf, x+36, y+157, COLOR_BLACK, NULL, NULL);
	StringBlt(ScreenSurface, &SystemFont, buf, x+35, y+156, COLOR_WHITE, NULL, NULL);
	sprintf(buf, "GR: ~%d~", new_character.bar[2]*2+new_character.bar_add[2]);
	StringBlt(ScreenSurface, &SystemFont, buf, x+36, y+168, COLOR_BLACK, NULL, NULL);
	StringBlt(ScreenSurface, &SystemFont, buf, x+35, y+167, COLOR_WHITE, NULL, NULL);

	StringBlt(ScreenSurface, &SystemFont, new_character.desc[0] , x+160, y+434, COLOR_BLACK, NULL, NULL);
	StringBlt(ScreenSurface, &SystemFont, new_character.desc[0], x+159, y+433, COLOR_WHITE, NULL, NULL);
	
	StringBlt(ScreenSurface, &SystemFont, new_character.desc[1] , x+160, y+446, COLOR_BLACK, NULL, NULL);
	StringBlt(ScreenSurface, &SystemFont, new_character.desc[1], x+159, y+445, COLOR_WHITE, NULL, NULL);

	StringBlt(ScreenSurface, &SystemFont, new_character.desc[2] , x+160, y+458, COLOR_BLACK, NULL, NULL);
	StringBlt(ScreenSurface, &SystemFont, new_character.desc[2], x+159, y+457, COLOR_WHITE, NULL, NULL);

	StringBlt(ScreenSurface, &SystemFont, new_character.desc[3] , x+160, y+470, COLOR_BLACK, NULL, NULL);
	StringBlt(ScreenSurface, &SystemFont, new_character.desc[3], x+159, y+469, COLOR_WHITE, NULL, NULL);

	/* draw portrait */
	/*
	box.x= x+350;
	box.y= y+CREATE_Y0;
	box.w= 100;
	box.h= 100;
	SDL_FillRect(ScreenSurface, &box, sdl_gray1);
	StringBlt(ScreenSurface, &SystemFont, "Portrait:",  x+350, y+CREATE_Y0-13, COLOR_WHITE, NULL, NULL);
	add_rangebox(x+350, y+CREATE_Y0+101, ++id, 70, 1, "todo ;-)", COLOR_GREEN);
*/

	if (!mb) active_button =-1;
}

/******************************************************************
 show login: status & identify part.
******************************************************************/
void show_login_server(void)
{
	SDL_Rect box;
	char buf[256];
	int x, y, i;
	int mx, my, mb;

	mb = SDL_GetMouseState(&mx, &my);
	/* background */
	x= SCREEN_XLEN/2-Bitmaps[BITMAP_DIALOG_BG]->bitmap->w/2;
	y= SCREEN_YLEN/2-Bitmaps[BITMAP_DIALOG_BG]->bitmap->h/2;
	sprite_blt(Bitmaps[BITMAP_DIALOG_BG],x, y, NULL, NULL);
	sprite_blt(Bitmaps[BITMAP_LOGO270],x+20, y+85, NULL, NULL);
	sprite_blt(Bitmaps[BITMAP_DIALOG_TITLE_LOGIN],x+250-Bitmaps[BITMAP_DIALOG_TITLE_LOGIN]->bitmap->w/2, y+20, NULL, NULL);
/*	add_close_button(x, y, MENU_LOGIN); */

	x+=170;
	y+=100;
	draw_frame(x-3, y-3, 211, 168);
	box.x= x-2;
	box.y= y-2;
	box.w= 210;
	box.h= 17;

	if(!strcmp(ServerName,"127.0.0.1"))
		strcpy(buf,"Server local host");
	else
		sprintf(buf,"Server %s", ServerName);

	StringBlt(ScreenSurface, &BigFont,buf , x+1, y-28, COLOR_BLACK, NULL, NULL);
	StringBlt(ScreenSurface, &BigFont,buf , x , y-29, COLOR_HGOLD, NULL, NULL);

	SDL_FillRect(ScreenSurface, &box, sdl_gray3);
	box.y= y+15;
	box.h= 150;
	SDL_FillRect(ScreenSurface, &box, sdl_gray4);
	StringBlt(ScreenSurface, &SystemFont,"- UPDATING FILES- " , x+58, y+1, COLOR_BLACK, NULL, NULL);
	StringBlt(ScreenSurface, &SystemFont,"- UPDATING FILES -" , x+57 , y, COLOR_WHITE, NULL, NULL);
    if(request_file_chain >=0)
		StringBlt(ScreenSurface, &SystemFont,"Updating settings file from server...." , x+2, y+20, COLOR_WHITE, NULL, NULL);
	if(request_file_chain >1)
		StringBlt(ScreenSurface, &SystemFont,"Updating skills file from server...." , x+2, y+32, COLOR_WHITE, NULL, NULL);
	if(request_file_chain >3)
		StringBlt(ScreenSurface, &SystemFont,"Updating spells file from server...." , x+2, y+44, COLOR_WHITE, NULL, NULL);
	if(request_file_chain >5)
		StringBlt(ScreenSurface, &SystemFont,"Updating bmaps file from server...." , x+2, y+56, COLOR_WHITE, NULL, NULL);
	if(request_file_chain >7)
		StringBlt(ScreenSurface, &SystemFont,"Updating anims file from server...." , x+2, y+68, COLOR_WHITE, NULL, NULL);
	if(request_file_chain >9)
		StringBlt(ScreenSurface, &SystemFont,"Sync files..." , x+2, y+80, COLOR_WHITE, NULL, NULL);

	/* if set, we have requested something and the stuff in the socket buffer is our file! */
	if(request_file_chain == 1 || request_file_chain == 3 || 
					request_file_chain ==  5 || request_file_chain == 7 || request_file_chain == 9)
	{
		sprintf(buf,"received ~%d~ bytes", csocket.inbuf.len);
		StringBlt(ScreenSurface, &SystemFont,buf , x+1, y+150, COLOR_WHITE, NULL, NULL);
	}

	/* login user part */
	if (GameStatus == GAME_STATUS_REQUEST_FILES)
		return;
	StringBlt(ScreenSurface, &SystemFont,"done." , x+2, y+92, COLOR_WHITE, NULL, NULL);
	y+=180;
	StringBlt(ScreenSurface, &SystemFont,"Enter your Name" , x, y, COLOR_HGOLD, NULL, NULL);
	sprite_blt(Bitmaps[BITMAP_LOGIN_INP],x-2, y+15, NULL, NULL);
	if(GameStatus == GAME_STATUS_NAME)
		StringBlt(ScreenSurface, &SystemFont,show_input_string(InputString,&SystemFont,Bitmaps[BITMAP_LOGIN_INP]->bitmap->w-16) , x+2, y+17, COLOR_WHITE, NULL, NULL);
	else
		StringBlt(ScreenSurface, &SystemFont,cpl.name , x+2, y+17, COLOR_WHITE, NULL, NULL);

	StringBlt(ScreenSurface, &SystemFont,"Enter your Password" , x+2, y+40, COLOR_HGOLD, NULL, NULL);
	sprite_blt(Bitmaps[BITMAP_LOGIN_INP],x-2, y+55, NULL, NULL);
	if(GameStatus == GAME_STATUS_PSWD){
		strcpy(buf,show_input_string(InputString,&SystemFont,Bitmaps[BITMAP_LOGIN_INP]->bitmap->w-16));
		for(i=0;i<(int)strlen(InputString);i++)buf[i]='*';
		StringBlt(ScreenSurface, &SystemFont,buf , x+2, y+57, COLOR_WHITE, NULL, NULL);
	}else{
		for(i=0;i<(int)strlen(cpl.password);i++)buf[i]='*';buf[i]=0;
		StringBlt(ScreenSurface, &SystemFont,buf , x+2, y+57, COLOR_WHITE, NULL, NULL);
	}
	if(GameStatus == GAME_STATUS_VERIFYPSWD)
	{
		StringBlt(ScreenSurface, &SystemFont,"New Character: Verify Password" , x+2, y+80, COLOR_HGOLD, NULL, NULL);
		sprite_blt(Bitmaps[BITMAP_LOGIN_INP],x-2, y+95, NULL, NULL);
		strcpy(buf,show_input_string(InputString,&SystemFont,Bitmaps[BITMAP_LOGIN_INP]->bitmap->w-16));
		for(i=0;i<(int)strlen(InputString);i++)buf[i]='*';
		StringBlt(ScreenSurface, &SystemFont, buf, x+2, y+97, COLOR_WHITE, NULL, NULL);
	}

	y+=160;
	StringBlt(ScreenSurface, &SystemFont, "To start playing enter your character ~name~ and ~password~." ,x-10, y+1, COLOR_BLACK, NULL, NULL);
	StringBlt(ScreenSurface, &SystemFont, "To start playing enter your character ~name~ and ~password~." ,x-11  , y  , COLOR_WHITE, NULL, NULL);
	y+=12;
	StringBlt(ScreenSurface, &SystemFont, "You will be asked to ~verify~ the password for new characters." ,x-10, y+1, COLOR_BLACK, NULL, NULL);
	StringBlt(ScreenSurface, &SystemFont, "You will be asked to ~verify~ the password for new characters." ,x-11  , y  , COLOR_WHITE, NULL, NULL);
}


/******************************************************************
 show login: select-server part.
******************************************************************/
void show_meta_server(_server *node, int metaserver_start, int metaserver_sel)
{
	int x,y,i;
	char buf[1024];
	SDL_Rect rec_name;
	SDL_Rect rec_desc;
	SDL_Rect box;
	int mx, my, mb;

	mb = SDL_GetMouseState(&mx, &my);
	/* background */
	x= SCREEN_XLEN/2-Bitmaps[BITMAP_DIALOG_BG]->bitmap->w/2;
	y= SCREEN_YLEN/2-Bitmaps[BITMAP_DIALOG_BG]->bitmap->h/2;
	sprite_blt(Bitmaps[BITMAP_DIALOG_BG],x, y, NULL, NULL);
	sprite_blt(Bitmaps[BITMAP_LOGO270],x+20, y+85, NULL, NULL);
	sprite_blt(Bitmaps[BITMAP_DIALOG_TITLE_LOGIN],x+250-Bitmaps[BITMAP_DIALOG_TITLE_LOGIN]->bitmap->w/2, y+20, NULL, NULL);
/*	add_close_button(x, y, MENU_LOGIN); */

	rec_name.w = 272;
	rec_desc.w = 325;

	box.x= x+133;
	box.y= y+TXT_Y_START+1;
	box.w= 329;
	box.h= 12;

	/* frame for selection field */
	draw_frame(box.x-1, box.y+11, box.w+1, 313);
	/* we should prepare for this the SystemFontOut */
	StringBlt(ScreenSurface,&SystemFont, "Servers", x+TXT_START_NAME+1, y+TXT_Y_START-1, COLOR_BLACK,NULL, NULL);
	StringBlt(ScreenSurface,&SystemFont, "Servers", x+TXT_START_NAME,   y+TXT_Y_START-2, COLOR_WHITE,NULL, NULL);
	StringBlt(ScreenSurface,&SystemFont, "Players", x+416, y+TXT_Y_START-1, COLOR_BLACK,NULL, NULL);
	StringBlt(ScreenSurface,&SystemFont, "Players", x+415, y+TXT_Y_START-2, COLOR_WHITE,NULL, NULL);

	sprintf(buf,"use cursors ~%c%c~ to select server                                  press ~RETURN~ to connect",ASCII_UP, ASCII_DOWN);
	StringBlt(ScreenSurface,&SystemFont, buf, x+140, y+410, COLOR_WHITE,NULL, NULL);

	for(i=0;i<OPTWIN_MAX_OPT;i++){
		box.y+=12;
		if (i&1) SDL_FillRect(ScreenSurface, &box, sdl_gray2);
		else     SDL_FillRect(ScreenSurface, &box, sdl_gray1);
	}

	for(i=0;node && i<metaserver_start;i++)
		node=node->next;

	for(i=0;node && i<MAXMETAWINDOW;i++)
	{
		if(i== metaserver_sel-metaserver_start)
		{
			int o=0;
			if (!node->desc4[0]) o+=5; if (!node->desc3[0]) o+=5;
			if (!node->desc2[0]) o+=5; if (!node->desc1[0]) o+=5;
	  		sprintf(buf,"version %s",node->version);
			StringBlt(ScreenSurface,&SystemFont, buf,         x+160, y+418+o, COLOR_BLACK, NULL, NULL);
			StringBlt(ScreenSurface,&SystemFont, buf,         x+159, y+417+o, COLOR_WHITE, NULL, NULL);
			StringBlt(ScreenSurface,&SystemFont, node->desc1, x+160, y+431+o, COLOR_BLACK, &rec_desc,NULL);
			StringBlt(ScreenSurface,&SystemFont, node->desc1, x+159, y+430+o, COLOR_HGOLD, &rec_desc,NULL);
			StringBlt(ScreenSurface,&SystemFont, node->desc2, x+160, y+442+o, COLOR_BLACK,&rec_desc,NULL);
			StringBlt(ScreenSurface,&SystemFont, node->desc2, x+159, y+441+o, COLOR_HGOLD,&rec_desc,NULL);
			StringBlt(ScreenSurface,&SystemFont, node->desc3, x+160, y+453+o, COLOR_BLACK, &rec_desc,NULL);
			StringBlt(ScreenSurface,&SystemFont, node->desc3, x+159, y+452+o, COLOR_HGOLD, &rec_desc,NULL);
			/* hm, 3 descr. are far enough */
			/*StringBlt(ScreenSurface,&SystemFont, node->desc1, x+150, y+464+o, COLOR_HGOLD, &rec_desc,NULL);*/
			box.y = y+TXT_Y_START+13+i*12;
	  	SDL_FillRect(ScreenSurface, &box, sdl_blue1);
		}
		StringBlt(ScreenSurface,&SystemFont, node->nameip , x+137, y+94+i*12, COLOR_WHITE, &rec_name, NULL);
		if(node->player >=0)
			sprintf(buf,"%d",node->player);
		else
			sprintf(buf,"-");
		StringBlt(ScreenSurface,&SystemFont, buf , x+416, y+94+i*12, COLOR_WHITE, NULL, NULL);
		node = node->next;
	}
}

