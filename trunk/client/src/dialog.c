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

    The author can be reached via e-mail to info@daimonin.net
*/

#include "include.h"
#define TXT_START_NAME  136
#define TXT_START_LEVEL 370
#define TXT_START_EXP   400
#define TXT_Y_START      82
#define X_COL1 136
#define X_COL2 290
#define X_COL3 430

int dialog_new_char_warn    = 0;
int dialog_login_warning_level = DIALOG_LOGIN_WARNING_NONE;

enum
{
    SEL_BUTTON,
    SEL_CHECKBOX,
    SEL_RANGE,
    SEL_TEXT
}; /* selection types */

enum
{
    OPTION,
    SPELL,
    SKILL
};

int  active_button   = -1;



#define MIN_ATTR_DELTA -2 /* attr. can be sub by MIN_ATTR_DELTA points. */
#define MAX_ATTR_DELTA  1 /* attr. can be add by MIN_ATTR_DELTA points. */

typedef struct _attribute
{
    char   *name;
    char   *info;   /* info text row 1 */
    int     minRange, maxRange;
    int     value;
    int     valDelta;
}
_attribute;

enum
{
    ATT_STR,
    ATT_DEX,
    ATT_CON,
    ATT_INT,
    ATT_WIS,
    ATT_POW,
    ATT_CHA,
    ATT_SUM
};

static char *weapon_skill[] =
    {
        "-","slash","impact","cleave","pierce"
    };

static _attribute   attribute[]                     =
    {
        {"STR", "Strength",     8,15, 4, 0}, {"DEX", "Dextery",      8,15, 4, 0}, {"CON", "Constitution", 8,15, 4, 0},
        {"INT", "Intellect",    8,15, 4, 0}, {"WIS", "Wisdom",       8,15, 4, 0}, {"POW", "Power",        8,15, 4, 0},
        {"CHA", "Charisma",     8,15, 4, 0}
    };
#define SUM_ATTRIBUTE (sizeof(attribute)/sizeof(_attribute))

char               *gender[]                        =
    {
        "male", "female", "hermaphrodite", "neuter"
    };
#define SUM_GENDER (sizeof(gender)/sizeof(char*))

int                 gen                             = 0;

/******************************************************************
 Option Menue
******************************************************************/
char               *opt_tab[]                       =
    {
        "Communication", "Combat", "Inventory", "Map", "Sound", "Client", "Fullscreen flags", "Windowed flags", "Debug", 0
    };

_option             opt[]                           =
    {
        /* Chat & Messages */
        {"Channel format:", "How the channels are shown.","~Prefixed~ prefixes each line with [channel:player], and","~Indented~ only prefixes the first line, indenting subsequent ones.", "Prefixed#Indented",SEL_RANGE,0,1,1,0, &options.channelformat,VAL_INT},
        {"Enable smileys:", "Whether to show smileys graphically.","","", "",SEL_CHECKBOX,0,1,1,0, &options.smileys,VAL_BOOL},
        {"Log messages:", "Whether/which messages are logged.","~None~ turns off logging, ~Only chat~ logs only the chat textwindow, and","~All~ logs everything in both textwindows.", "None#Only chat#All",SEL_RANGE,0,2,1,1, &options.msglog,VAL_INT},
        {"Textwindows use alpha:", "Whether the textwindows are transparent.","","WARNING: Don't check this if you have a VERY slow computer!", "",SEL_CHECKBOX,0,1,1,1, &options.use_TextwinAlpha,VAL_INT},
        {"Textwindows alpha value:", "Transparency value of textwindows.","A higher value means a darker textwindow.","Only has meaning if 'Textwindows use alpha' is checked.", "",SEL_RANGE,0,255,5,110, &options.textwin_alpha,VAL_INT},
        {"Enable chatfilter:", "Whether to filter incoming messages for 'bad' words.","See '/cfilter ?' for more details on this.","", "",SEL_CHECKBOX,0,1,1,0, &options.chatfilter,VAL_BOOL},
        {"#"},
        /* End of Page */

        /* Combat */
        {"Low health warning:", "Whether to show a low health warning above your head.","If hitpoints are lower than the given percentage of maximum a warning","is shown. Set to ~0~ to disable.", "",SEL_RANGE,0,100,5,20, &options.warning_hp,VAL_INT},
        {"Show yourself targeted:", "Whether to show your name in the target area.","","", "",SEL_CHECKBOX,0,1,1,0, &options.show_target_self,VAL_BOOL},
        {"Kill-Stats:", "Whether to keep a count all kills based on mob name.","","", "",SEL_CHECKBOX,0,1,1,1, &options.kerbholz,VAL_BOOL},
#ifdef DEVELOPMENT
        {"Stat-O-Meter:", "How often to update the Stat-O-Meter.","It is updated every X seconds, as given here.","", "",SEL_RANGE,1,15,1,5, &options.statsupdate,VAL_INT},
#else
        {"Stat-O-Meter:", "Whether to show the Stat-O-Meter.","It is updated every X seconds, as given here.","Set to ~0~ to disable.", "",SEL_RANGE,0,15,1,5, &options.statsupdate,VAL_INT},
#endif
        {"Extended exp display:", "How the exp display is shown.","The format key is: ~4nl~ = For next level; ~tnl~ = Till next level;","~LExp~ = Level exp; ~TExp~ = Total exp;", "Level/LExp#LExp\\%#LExp/LExp 4nl#TExp/TExp 4nl#(LExp\\%) LExp tnl",SEL_RANGE,0,4,1,4, &options.iExpDisplay,VAL_INT},
        {"#"},
        /* End of Page */

        /* Inventory */
        {"Overload warning:", "Whether to show an overloaded warning above your head.","If your carry weight is higher than the given percentage of","your carry limit a warning is shown. Set to ~0~ to disable.", "",SEL_RANGE,0,100,5,85, &options.warning_weight,VAL_INT},
        {"Show item qua/con:", "Whether to show an item's qua/con in the inventory/on the playerdoll.","","", "",SEL_CHECKBOX,0,1,1,1, &options.showqc,VAL_BOOL},
        {"Item damage percent 1:", "Whether to show a damaged item in orange.","If the item's con is lower than the given percentage of its qua","the damage bar is shown in orange. Set to ~0~ to disable.", "",SEL_RANGE,0,100,5,90, &options.itemdmg_limit_orange,VAL_INT},
        {"Item damage percent 2:", "Whether to show a severely damaged item in red.","If the item's con is lower than the given percentage of its qua","the damage bar is shown in red. Set to ~0~ to disable.", "",SEL_RANGE,0,100,5,80, &options.itemdmg_limit_red,VAL_INT},
        {"Playerdoll:", "Whether to always show the playerdoll.","If unchecked, the playerdoll is only shown while the inventory is open.","", "",SEL_CHECKBOX,0,1,1,0, &options.playerdoll,VAL_BOOL},
        {"#"},
        /* End of Page */

        /* Map */
        {"Player names:", "Whether/when to show player's names above their heads.","","", "Never#Always#Only others#Only yours",SEL_RANGE,0,3,1,2, &options.player_names,VAL_INT},
        {"Playfield start X:", "The X-position of the playfield.","","", "",SEL_RANGE,-20,1000,10,-10, &options.mapstart_x,VAL_INT},
        {"Playfield start Y:", "The Y-position of the playfield.","","", "",SEL_RANGE,0,700,10,60, &options.mapstart_y,VAL_INT},
        {"Playfield zoom:", "The zoom percentage of the playfield.","EXPERIMENTAL!!!!!","", "",SEL_RANGE,50,200,5,100, &options.zoom,VAL_INT},
        {"Smooth zoom:", "Whether to use smooth zoom on the playfield.","EXPERIMENTAL!!!!!","WARNING: VERY VERY VERY CPU intensive!", "",SEL_CHECKBOX,0,1,1,0, &options.smooth,VAL_BOOL},
        {"#"},
        /* End of Page */

        /* Sound */
        {"Sound volume:", "The sound volume as a percentage of maximum volume.","Set to ~0~ to disable sound effects.","", "",SEL_RANGE,0,100,5,100, &options.sound_volume,VAL_INT},
        {"Music volume:", "The music volume as a percentage of maximum volume.","Set to ~0~ to disable background music.","", "",SEL_RANGE,0,100,5,80, &options.music_volume,VAL_INT},
        {"Audible heartbeat:", "Whether to hear your heart pumping during combat.","If hitpoints are lower than the given percentage of maximum","you will hear your heartbeat. Set to ~0~ to disable.", "",SEL_RANGE,0,100,5,50, &options.heartbeat,VAL_INT},
        {"#"},
        /* End of Page */

        /* Client */
        {"Fullscreen:", "Whether to use fullscreen/window mode.","EXPERIMENTAL!!!!!","WARNING: Toggling this checkbox may crash your client!", "",SEL_CHECKBOX,0,1,1,0, &options.fullscreen,VAL_BOOL},
        {"Resolution:", "The resolution of the screen/window.","If you change to lower resolutions your GUI-windows may be hidden.","WARNING: Some resolutions are NOT for fullscreen!", "800x600#960x600#1024x768#1100x700#1280x720#1280x800#1280x960#1280x1024#1440x900#1400x1050#1600x1200#1680x1050#1920x1080#1920x1200#2048x1536#2560x1600",SEL_RANGE,0,15,1,0, &options.resolution,VAL_INT},
        {"Automatic bpp:", "Whether to use your system's default bits per pixel.","NOTE: You need to restart the client for this option to take effect.","", "",SEL_CHECKBOX,0,1,1,1, &options.auto_bpp_flag,VAL_BOOL},
        {"Colordeep:", "The bpp for fullscreen mode.","This is overruled if 'automatic bpp' is checked.","NOTE: You need to restart the client for this option to take effect.", "8 bpp#16 bpp#32 bpp",SEL_RANGE,0,2,1,2, &options.video_bpp,VAL_INT},
        {"Save CPU time with sleep():", "Whether to use sleep().","The client eats less CPU time when checked.","", "",SEL_CHECKBOX,0,1,1,1, &options.limit_speed,VAL_BOOL},
        {"Sleep time in ms:", "The number of ms for which the client will sleep.","Only has meaning if 'save CPU time with sleep()' is checked.","", "",SEL_RANGE,0,1000,1,15, &options.sleep,VAL_INT},
        {"Show tooltips:", "Whether to show tooltips when hovering the mouse pointer over items.","","", "",SEL_CHECKBOX,0,1,1,1, &options.show_tooltips,VAL_BOOL},
        {"Key-info in dialog menus:", "UNUSED.","","", "",SEL_CHECKBOX,0,1,1,1, &options.show_d_key_infos,VAL_BOOL},
        {"SpeedUp:", "Whether/how much to speed up the client.","The (potential) cost is some graphical glitches. A higher value means","a faster the client but more glitches. Set to ~0~ to disable.", "",SEL_RANGE,0,400,10,0, &options.speedup,VAL_INT},
        {"Menu repeat speed:", "Whether/how fast to repeat a held down key.","This only affects keypresses in menus/the console,","not in gameplay (that rate is fixed).", "Off#Slow#Medium#Fast",SEL_RANGE,0,3,1,2, &options.menu_repeat,VAL_INT},
#ifdef DEVELOPMENT
        {"Widget layout:", "Whether to alter the widget layout.", "Select the action you want to perform.","Unlike other options, this will only take effect when you choose ~D~one.", "No change#Show all#Default layout",SEL_RANGE,0,2,1,0, &options.widget_layout,VAL_INT},
#endif
#ifdef WIDGET_SNAP
        {"Widget snap width:", "The distance at which widgets will snap to each other","Set to zero to disable it.","EXPERIMENTAL!", "",SEL_RANGE,0,32,1,0, &options.widget_snap,VAL_INT},
#endif
        /*{"Collect All Items:", "Don't ask for number of items to get, just get all of them.","","", "",SEL_CHECKBOX,0,1,1,0, &options.collectAll,VAL_BOOL},   */
        {"#"}, /* End of Page */

        /* Fullscreen Flags */
        {"Hardware surface:", "Don't change unless you know what you are doing.","NOTE: You need to restart the client for this option to take effect.","", "",SEL_CHECKBOX,0,1,1,0, &options.Full_HWSURFACE,VAL_BOOL},
        {"Software surface:", "Don't change unless you know what you are doing.","NOTE: You need to restart the client for this option to take effect.","", "",SEL_CHECKBOX,0,1,1,1, &options.Full_SWSURFACE,VAL_BOOL},
        {"Hardware accel:", "Don't change unless you know what you are doing.","NOTE: You need to restart the client for this option to take effect.","", "",SEL_CHECKBOX,0,1,1,1, &options.Full_HWACCEL,VAL_BOOL},
        {"Doublebuffer:", "Don't change unless you know what you are doing.","NOTE: You need to restart the client for this option to take effect.","", "",SEL_CHECKBOX,0,1,1,0, &options.Full_DOUBLEBUF,VAL_BOOL},
        {"Any format:", "Don't change unless you know what you are doing.","NOTE: You need to restart the client for this option to take effect.","", "",SEL_CHECKBOX,0,1,1,0, &options.Full_ANYFORMAT,VAL_BOOL},
        {"Async blit:", "Don't change unless you know what you are doing.","NOTE: You need to restart the client for this option to take effect.","", "",SEL_CHECKBOX,0,1,1,0, &options.Full_ASYNCBLIT,VAL_BOOL},
        {"Hardware palette:", "Don't change unless you know what you are doing.","NOTE: You need to restart the client for this option to take effect.","", "",SEL_CHECKBOX,0,1,1,1, &options.Full_HWPALETTE,VAL_BOOL},
        {"Resizeable:", "Don't change unless you know what you are doing.","NOTE: You need to restart the client for this option to take effect.","", "",SEL_CHECKBOX,0,1,1,0, &options.Full_RESIZABLE,VAL_BOOL},
        {"No frame:", "Don't change unless you know what you are doing.","NOTE: You need to restart the client for this option to take effect.","", "",SEL_CHECKBOX,0,1,1,0, &options.Full_NOFRAME,VAL_BOOL},
        {"RLE accel:", "Don't change unless you know what you are doing.","NOTE: You need to restart the client for this option to take effect.","", "",SEL_CHECKBOX,0,1,1,1, &options.Full_RLEACCEL,VAL_BOOL},
        {"#"},
        /* End of Page */

        /* Windowed flags*/
        {"Win Hardware surface:", "Don't change unless you know what you are doing.","NOTE: You need to restart the client for this option to take effect.","", "",SEL_CHECKBOX,0,1,1,0, &options.Win_HWSURFACE,VAL_BOOL},
        {"Win Software surface:", "Don't change unless you know what you are doing.","NOTE: You need to restart the client for this option to take effect.","", "",SEL_CHECKBOX,0,1,1,1, &options.Win_SWSURFACE,VAL_BOOL},
        {"Win Hardware accel:", "Don't change unless you know what you are doing.","NOTE: You need to restart the client for this option to take effect.","", "",SEL_CHECKBOX,0,1,1,0, &options.Win_HWACCEL,VAL_BOOL},
        {"Win Doublebuffer:", "Don't change unless you know what you are doing.","NOTE: You need to restart the client for this option to take effect.","", "",SEL_CHECKBOX,0,1,1,0, &options.Win_DOUBLEBUF,VAL_BOOL},
        {"Win Any format:", "Don't change unless you know what you are doing.","NOTE: You need to restart the client for this option to take effect.","", "",SEL_CHECKBOX,0,1,1,1, &options.Win_ANYFORMAT,VAL_BOOL},
        {"Win Async blit:", "Don't change unless you know what you are doing.","NOTE: You need to restart the client for this option to take effect.","", "",SEL_CHECKBOX,0,1,1,0, &options.Win_ASYNCBLIT,VAL_BOOL},
        {"Win Hardware Palette:", "Don't change unless you know what you are doing.","NOTE: You need to restart the client for this option to take effect.","", "",SEL_CHECKBOX,0,1,1,1, &options.Win_HWPALETTE,VAL_BOOL},
        {"Win Resizeable:", "Don't change unless you know what you are doing.","NOTE: You need to restart the client for this option to take effect.","", "",SEL_CHECKBOX,0,1,1,0, &options.Win_RESIZABLE,VAL_BOOL},
        {"Win No frame:", "Don't change unless you know what you are doing.","NOTE: You need to restart the client for this option to take effect.","", "",SEL_CHECKBOX,0,1,1,0, &options.Win_NOFRAME,VAL_BOOL},
        {"Win RLE accel:", "Don't change unless you know what you are doing.","NOTE: You need to restart the client for this option to take effect.","", "",SEL_CHECKBOX,0,1,1,1, &options.Win_RLEACCEL,VAL_BOOL},
        {"#"}, /* End of Page */

        /* Debug */
        {"Show framerate:", "Whether to show the fps and other details under the map name.","","", "",SEL_CHECKBOX,0,1,1,0, &options.show_frame,VAL_BOOL},
        {"Force redraw:", "Whether to force the system to redraw EVERY frame.","","", "",SEL_CHECKBOX,0,1,1,0, &options.force_redraw,VAL_BOOL},
        {"Use update rect:", "Whether to use update rect.","","", "",SEL_CHECKBOX,0,1,1,0, &options.use_rect,VAL_BOOL},
        {"#"},
        /* End of Page */

        {0} /* End of Options */
    };

/******************************************************************
 Skill Menue
******************************************************************/
static char        *skill_tab[]                     =
    {
        "Agility", "Mental", "Magic", "Person", "Physique", "Wisdom", "Misc", 0
    };
#define SKILL_TAB_SIZE (sizeof(skill_tab)/sizeof(char*))

/******************************************************************
 Spell Menue
******************************************************************/
char        *spell_tab[]                     =
    {
        "Life", "Death", "Elemental", "Energy",
        "Spirit", "Protection", "Light", "Nether",
        "Nature", "Shadow", "Chaos", "Earth",
        "Conjuration", "Abjuration", "Transmutation", "Arcane",0
    };
#define SPELL_TAB_SIZE (sizeof(spell_tab)/sizeof(char*))

static char        *spell_class[SPELL_LIST_CLASS]   =
    {
        "Spell", "Prayer"
    };

/******************************************************************
 draws a frame.
******************************************************************/
void draw_frame(int x, int y, int w, int h)
{
    SDL_Rect    box;

    box.x = x;  box.y = y;
    box.h = h;  box.w = 1;
    SDL_FillRect(ScreenSurface, &box, sdl_gray4);
    box.x = x + w;
    box.h++;
    SDL_FillRect(ScreenSurface, &box, sdl_gray3); /* right */
    box.x = x;  box.y += h;
    box.w = w;  box.h = 1;
    SDL_FillRect(ScreenSurface, &box, sdl_gray4); /* bottom */
    box.x++;    box.y = y;
    SDL_FillRect(ScreenSurface, &box, sdl_gray3); /* top */
}

/******************************************************************
 decode value with given value_type to text.
******************************************************************/
char * get_value(void *value, int type)
{
    static char txt_value[20];
    switch (type)
    {
        case VAL_INT:
            sprintf(txt_value, "%d", *((int *) value));
            return txt_value;
        case VAL_U32:
            sprintf(txt_value, "%d", *((uint32 *) value));
            return txt_value;
        case VAL_CHAR:
            sprintf(txt_value, "%d", *((uint8 *) value));
            return txt_value;
        default:
            return NULL;
    }
}

/******************************************************************
 add the close button and handle mouse events on it.
******************************************************************/
void add_close_button(int x, int y, int menu, Boolean newstyle)
{
    int mx, my, mb;
    mb = SDL_GetMouseState(&mx, &my) & SDL_BUTTON(SDL_BUTTON_LEFT);

    if (newstyle)
    {
        if (mx > x + 460 && mx <x + 474 && my> y + 26 && my < y + 40)
        {
            sprite_blt(Bitmaps[BITMAP_CLOSEBUTTON], x+463, y+28, NULL,NULL);
            if (mb && mb_clicked)
                check_menu_keys(menu, SDLK_ESCAPE);
        }
    }
    else
    {
        StringBlt(ScreenSurface, &SystemFont, "X", x + 463, y + 28, COLOR_BLACK, NULL, NULL);
        if (mx > x + 459 && mx <x + 469 && my> y + 27 && my < y + 39)
        {
            StringBlt(ScreenSurface, &SystemFont, "X", x + 462, y + 27, COLOR_HGOLD, NULL, NULL);
            if (mb && mb_clicked)
                check_menu_keys(menu, SDLK_ESCAPE);
        }
        else
            StringBlt(ScreenSurface, &SystemFont, "X", x + 462, y + 27, COLOR_WHITE, NULL, NULL);
    }
}

/******************************************************************
 add a button and handle mouse events on it.
 text_h:
    info for keyboard user. You can give them here a highlighted char
    to identify the key for this button. if not needed set it to NULL.
******************************************************************/
int add_button(int x, int y, int id, int gfxNr, char *text, char *text_h)
{
    char   *text_sel;
    int     ret = 0;
    int     xoff, yoff;

    if (text_h)
        text_sel = text_h;
    else
        text_sel = text;

    sprite_blt(Bitmaps[gfxNr], x, y, NULL, NULL);

    // Label centering in button
    yoff =  (Bitmaps[gfxNr]->bitmap->h - (SystemFont.c['W'].h+1)) / 2 + 2;
    xoff =  (Bitmaps[gfxNr]->bitmap->w - (StringWidth(&SystemFont, text)+1)) / 2 + 1;

    if (global_buttons.down!=-1 && global_buttons.mx_down > x && global_buttons.my_down > y &&
            global_buttons.mx_down < x + Bitmaps[gfxNr]->bitmap->w &&
            global_buttons.my_down < y + Bitmaps[gfxNr]->bitmap->h)
    {
        sprite_blt(Bitmaps[gfxNr + 1], x, y++, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont, text, x + xoff, y + yoff, COLOR_BLACK, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont, text_sel, x + (xoff-1), y + (yoff-1), COLOR_HGOLD, NULL, NULL);

    }
    else if ( global_buttons.valid != -1 && global_buttons.click != -1 &&
              global_buttons.mx_up > x && global_buttons.my_up > y &&
              global_buttons.mx_up < x + Bitmaps[gfxNr]->bitmap->w &&
              global_buttons.my_up < y + Bitmaps[gfxNr]->bitmap->h)
    {
        global_buttons.valid = -1;
        ret = TRUE;
    }
    else
    {
        StringBlt(ScreenSurface, &SystemFont, text, x + xoff, y + yoff, COLOR_BLACK, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont, text_sel, x + (xoff-1), y + (yoff-1), COLOR_WHITE, NULL, NULL);
    }
    return ret;
}

/******************************************************************
 add a gfx-button and handle mouse events on it.
 text_h:
    info for keyboard user. You can give them here a highlighted char
    to identify the key for this button. if not needed set it to NULL.
******************************************************************/
/*
static int add_gfx_button(int x, int y, int id, int gfxNr, int gfx_inner, char *text, char *text_h)
{
    char   *text_sel;
    int     ret     = 0;
    int     mx, my, mb;
    int     color   = COLOR_WHITE;

    mb = SDL_GetMouseState(&mx, &my) & SDL_BUTTON(SDL_BUTTON_LEFT);
    if (text_h)
        text_sel = text_h;
    else
        text_sel = text;

    sprite_blt(Bitmaps[gfxNr], x, y, NULL, NULL);
    sprite_blt(Bitmaps[gfx_inner], x + 5, y + 3, NULL, NULL);
    if (mx > x && my > y && mx < x + Bitmaps[gfxNr]->bitmap->w && my < y + Bitmaps[gfxNr]->bitmap->h)
    {
        if (mb && mb_clicked && active_button < 0)
            active_button = id;
        if (active_button == id)
        {
            sprite_blt(Bitmaps[gfxNr + 1], x, y, NULL, NULL);
            sprite_blt(Bitmaps[gfx_inner], x + 6, y + 4, NULL, NULL);
            if (!mb)
                ret = 1;
        }
        color = COLOR_HGOLD;
    }
    x += Bitmaps[gfxNr]->bitmap->w + 10;
    y += Bitmaps[gfxNr]->bitmap->h / 2 - 5;
    StringBlt(ScreenSurface, &SystemFont, text, x + 1, y + 1, COLOR_BLACK, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, text_sel, x, y, color, NULL, NULL);
    return ret;
}
*/

/******************************************************************
 add a group-button and handle mouse events on it.
 text_h:
    info for keyboard user. You can give them here a highlighted char
    to identify the key for this button. if not needed set it to NULL.
******************************************************************/
static int add_gr_button(int x, int y, int id, int gfxNr, char *text, char *text_h)
{
    char   *text_sel;
    int     ret = 0;
    int     mx, my, mb;

    mb = SDL_GetMouseState(&mx, &my) & SDL_BUTTON(SDL_BUTTON_LEFT);
    /* use text_h (=highlighted char for keyboard user) if available. */
    if (text_h)
        text_sel = text_h;
    else
        text_sel = text;

    if (id)
        sprite_blt(Bitmaps[++gfxNr], x, y++, NULL, NULL);
    else
        sprite_blt(Bitmaps[gfxNr], x, y, NULL, NULL);

    StringBlt(ScreenSurface, &SystemFont, text, x + 11, y + 2, COLOR_BLACK, NULL, NULL);
    if (mx > x && my > y && mx < x + Bitmaps[gfxNr]->bitmap->w && my < y + Bitmaps[gfxNr]->bitmap->h)
    {
        if (mb && mb_clicked)
            ret = 1;
        StringBlt(ScreenSurface, &SystemFont, text_sel, x + 10, y + 1, COLOR_HGOLD, NULL, NULL);
    }
    else
        StringBlt(ScreenSurface, &SystemFont, text_sel, x + 10, y + 1, COLOR_WHITE, NULL, NULL);
    return ret;
}

/******************************************************************
    text_x: 0 => text left ; !=0 => text right  of rangebox.
    text_w: size of textfield.
******************************************************************/
int add_rangebox(int x, int y, int id, int text_w, int text_x, char *text, int color)
{
    static int  active  = -1;
    SDL_Rect    box;
    int         mx, my, mb;
    int         width   = Bitmaps[BITMAP_DIALOG_RANGE_OFF]->bitmap->w;

    mb = SDL_GetMouseState(&mx, &my) & SDL_BUTTON(SDL_BUTTON_LEFT);
    if (text_x)
    {
        /* box right of text */
        text_x = x + width + 2;
    }
    else
    {
        text_x = x;
        x += text_w;
    }
    /* draw the range gadget */
    sprite_blt(Bitmaps[BITMAP_DIALOG_RANGE_OFF], x, y, NULL, NULL);
    box.x = text_x - 2;
    box.y = y + 1;
    box.w = text_w + 2;
    box.h = 16;
    SDL_FillRect(ScreenSurface, &box, 0);
    StringBlt(ScreenSurface, &SystemFont, text, text_x + 1, y + 3, COLOR_BLACK, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, text, text_x, y + 2, color, NULL, NULL);

    /* check for action */
    if (mx > x && my > y && my < y + 18)
    {
        if (mx < x + width / 2)
        {
            if (mb)
                active = id;
            if (active == id)
            {
                sprite_blt(Bitmaps[BITMAP_DIALOG_RANGE_L], x, y, NULL, NULL);
                if (!mb)
                {
                    active = -1;
                    return -1;
                }
            }
        }
        else if (mx < x + width)
        {
            if (mb)
                active = id + 1; /* a rangebox has 2 buttons */
            if (active == id + 1)
            {
                sprite_blt(Bitmaps[BITMAP_DIALOG_RANGE_R], x + width / 2, y, NULL, NULL);
                if (!mb)
                {
                    active = -1;
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
void add_value(void *value, int type, int offset, int min, int max)
{
    switch (type)
    {
        case VAL_INT:
            *((int *) value) += offset;
            if (*((int *) value) > max)
                *((int *) value) = max;
            if (*((int *) value) < min)
                *((int *) value) = min;
            break;
        case VAL_U32:
            *((uint32 *) value) += offset;
            if (*((uint32 *) value) > (uint32) max)
                *((uint32 *) value) = (uint32) max;
            if (*((uint32 *) value) < (uint32) min)
                *((uint32 *) value) = (uint32) min;
            break;
        default:
            break;
    }
}

/******************************************************************
 draws all options for the actual page.
******************************************************************/
inline void optwin_draw_options(int x, int y)
{
#define LEN_NAME  111
    int i = -1, pos = 0, max = 0;
    int y2      = y + 347; /* for info text */
    int mxy_opt = -1;
    int page    = option_list_set.group_nr;
    int id      = 0;
    int mx, my, mb, tmp;

    mb = SDL_GetMouseState(&mx, &my) & SDL_BUTTON(SDL_BUTTON_LEFT);
    /* find actual page */
    while (page && opt[++i].name)
        if (opt[i].name[0] == '#')
            --page;

    /* draw actual page */
    while (opt[++i].name && opt[i].name[0] != '#')
    {
        max++;
        StringBlt(ScreenSurface, &SystemFont, opt[i].name, x + 1, y + 3, COLOR_BLACK, NULL, NULL);
        switch (opt[i].sel_type)
        {
            case SEL_CHECKBOX:
                tmp = COLOR_WHITE;
                if (option_list_set.entry_nr == max - 1)
                {
                    tmp = COLOR_HGOLD;
                    if (mxy_opt == -1)
                        mxy_opt = i; /* remember this tab for later use */
                }
                if (mx > x && mx <x + 280 && my> y && my < y + 20)
                {
                    tmp = COLOR_GREEN;
                    mxy_opt = i; /* remember this tab for later use */
                }

                StringBlt(ScreenSurface, &SystemFont, opt[i].name, x, y + 2, tmp, NULL, NULL);

                sprite_blt(Bitmaps[BITMAP_DIALOG_CHECKER], x + LEN_NAME, y, NULL, NULL);
                if (*((Boolean *) opt[i].value) == TRUE)
                {
                    StringBlt(ScreenSurface, &SystemFont, "X", x + LEN_NAME + 8, y + 2, COLOR_BLACK, NULL, NULL);
                    StringBlt(ScreenSurface, &SystemFont, "X", x + LEN_NAME + 7, y + 1, COLOR_WHITE, NULL, NULL);
                }
                if ((pos == option_list_set.entry_nr && option_list_set.key_change)
                        || (mb
                            && mb_clicked
                            && active_button <0
                            && mx> x + LEN_NAME
                            && mx <x + LEN_NAME + 20
                            && my> y
                            && my < y + 18))
                {
                    mb_clicked = 0;
                    option_list_set.key_change = 0;
                    if (*((Boolean *) opt[i].value) == TRUE)
                        *((Boolean *) opt[i].value) = FALSE;
                    else
                        *((Boolean *) opt[i].value) = TRUE;
                }
                break;

            case SEL_RANGE:
            {
#define LEN_VALUE 100
                SDL_Rect      box;
                box.x = x + LEN_NAME, box.y = y + 1;
                box.h = 16, box.w = LEN_VALUE;

                tmp = COLOR_WHITE;
                if (option_list_set.entry_nr == max - 1)
                {
                    tmp = COLOR_HGOLD;
                    if (mxy_opt == -1)
                        mxy_opt = i; /* remember this tab for later use */
                }
                if (mx > x && mx <x + 280 && my> y && my < y + 20)
                {
                    tmp = COLOR_GREEN;
                    mxy_opt = i; /* remember this tab for later use */
                }

                StringBlt(ScreenSurface, &SystemFont, opt[i].name, x, y + 2, tmp, NULL, NULL);
                /*
                          if (option_list_set.entry_nr == max-1 || (mx > x && mx < x+280 && my > y && my < y+20) ){
                              StringBlt(ScreenSurface, &SystemFont, opt[i].name, x, y+2, COLOR_HGOLD, NULL, NULL);
                              mxy_opt = i;
                          }
                          else
                              StringBlt(ScreenSurface, &SystemFont, opt[i].name, x, y+2, COLOR_WHITE, NULL, NULL);
                          */

                SDL_FillRect(ScreenSurface, &box, 0);
                if (*opt[i].val_text == 0)
                {
                    StringBlt(ScreenSurface, &SystemFont, get_value(opt[i].value, opt[i].value_type), box.x + 2,
                              y + 2, COLOR_WHITE, NULL, NULL);
                }
                else
                {
#define MAX_LEN 40
                    char      text[MAX_LEN + 1];
                    int       o   = *((int *) opt[i].value);
                    int       p = 0, q = -1;
                    /* find start pos of string */
                    while (o && opt[i].val_text[p])
                        if (opt[i].val_text[p++] == '#')
                            o--;
                    /* find end pos of string */
                    while (q++ < MAX_LEN && opt[i].val_text[p])
                        if ((text[q] = opt[i].val_text[p++]) == '#')
                            break;
                    text[q] = 0;
                    StringBlt(ScreenSurface, &SystemFont, text, box.x + 2, y + 2, COLOR_WHITE, NULL, NULL);
#undef MAX_LEN
                }
                sprite_blt(Bitmaps[BITMAP_DIALOG_RANGE_OFF], x + LEN_NAME + LEN_VALUE, y, NULL, NULL);


                /* keyboard event */
                if (option_list_set.key_change && option_list_set.entry_nr == pos)
                {
                    if (option_list_set.key_change == -1)
                        add_value(opt[i].value, opt[i].value_type, -opt[i].deltaRange, opt[i].minRange,
                                  opt[i].maxRange);
                    else if (option_list_set.key_change == 1)
                        add_value(opt[i].value, opt[i].value_type, opt[i].deltaRange, opt[i].minRange, opt[i].maxRange);
                    option_list_set.key_change = 0;
                }

                if (mx > x + LEN_NAME + LEN_VALUE && mx <x + LEN_NAME + LEN_VALUE + 14 && my> y && my < y + 18)
                {
                    if (mb && active_button < 0)
                        active_button = id + 1; /* 2 buttons per row */
                    if (active_button == id + 1)
                    {
                        sprite_blt(Bitmaps[BITMAP_DIALOG_RANGE_L], x + LEN_NAME + LEN_VALUE, y, NULL, NULL);
                        if (!mb)
                            add_value(opt[i].value, opt[i].value_type, -opt[i].deltaRange, opt[i].minRange,
                                      opt[i].maxRange);
                    }
                }
                else if (mx > x + LEN_NAME + LEN_VALUE + 14
                         && mx <x + LEN_NAME + LEN_VALUE + 28
                         && my> y
                         && my < y + 18)
                {
                    if (mb && active_button < 0)
                        active_button = id;
                    if (active_button == id)
                    {
                        sprite_blt(Bitmaps[BITMAP_DIALOG_RANGE_R], x + LEN_NAME + LEN_VALUE + 14, y, NULL, NULL);
                        if (!mb)
                            add_value(opt[i].value, opt[i].value_type, opt[i].deltaRange, opt[i].minRange,
                                      opt[i].maxRange);
                    }
                }
#undef LEN_VALUE
            }
            break;

            case SEL_BUTTON:
                sprite_blt(Bitmaps[BITMAP_DIALOG_BUTTON_UP], x, y, NULL, NULL);
                break;
        }
        y += 20;
        pos++;
        id += 2;
    }
    if (option_list_set.entry_nr > max - 1)
        option_list_set.entry_nr = max - 1;

    /* print the info text */
    x += 20;
    if (mxy_opt >= 0)
    {
//        if (*opt[mxy_opt].info2 == 0)
//            y2 += 5;
        StringBlt(ScreenSurface, &SystemFont, opt[mxy_opt].info1, x + 11, y2 + 1, COLOR_BLACK, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont, opt[mxy_opt].info1, x + 10, y2, COLOR_WHITE, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont, opt[mxy_opt].info2, x + 11, y2 + 13, COLOR_BLACK, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont, opt[mxy_opt].info2, x + 10, y2 + 12, COLOR_WHITE, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont, opt[mxy_opt].info3, x + 11, y2 + 25, COLOR_BLACK, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont, opt[mxy_opt].info3, x + 10, y2 + 24, COLOR_WHITE, NULL, NULL);
    }
#undef LEN_NAME
}

/******************************************************************
 draws all tabs on the left side of window.
******************************************************************/
static void draw_tabs(char *tabs[], int *act_tab, char *head_text, int x, int y)
{
    int         i       = -1;
    int         mx, my, mb;
    static int  active  = 0;

    mb = SDL_GetMouseState(&mx, &my) & SDL_BUTTON(SDL_BUTTON_LEFT);
    sprite_blt(Bitmaps[BITMAP_DIALOG_TAB_START], x, y - 10, NULL, NULL);
    sprite_blt(Bitmaps[BITMAP_DIALOG_TAB], x, y, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, head_text, x + 15, y + 4, COLOR_BLACK, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, head_text, x + 14, y + 3, COLOR_WHITE, NULL, NULL);
    y += 17;
    sprite_blt(Bitmaps[BITMAP_DIALOG_TAB], x, y, NULL, NULL);
    y += 17;
    while (tabs[++i])
    {
        sprite_blt(Bitmaps[BITMAP_DIALOG_TAB], x, y, NULL, NULL);
        if (i == *act_tab)
            sprite_blt(Bitmaps[BITMAP_DIALOG_TAB_SEL], x, y, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont, tabs[i], x + 25, y + 4, COLOR_BLACK, NULL, NULL);
        if (mx > x && mx <x + 100 && my> y && my < y + 17)
        {
            StringBlt(ScreenSurface, &SystemFont, tabs[i], x + 24, y + 3, COLOR_HGOLD, NULL, NULL);
            if (mb && mb_clicked)
                active = 1;
            if (active)
                *act_tab = i;
        }
        else
            StringBlt(ScreenSurface, &SystemFont, tabs[i], x + 24, y + 3, COLOR_WHITE, NULL, NULL);
        y += 17;
    }
    sprite_blt(Bitmaps[BITMAP_DIALOG_TAB_STOP], x, y, NULL, NULL);
    if (!mb)
        active = 0;
}

/******************************************************************
 show the skill-window and handle mouse actions.
******************************************************************/
void show_skilllist(void)
{
    SDL_Rect        box;
    char            buf[256];
    int             x, y, i;
    int             mx, my, mb;
    static int      active = 0, dblclk = 0;
    static Uint32   Ticks   = 0;

    mb = SDL_GetMouseState(&mx, &my);
    /* background */
    x = Screensize.x / 2 - Bitmaps[BITMAP_DIALOG_BG]->bitmap->w / 2;
    y = Screensize.y / 2 - Bitmaps[BITMAP_DIALOG_BG]->bitmap->h / 2;
    sprite_blt(Bitmaps[BITMAP_DIALOG_BG], x, y, NULL, NULL);
    sprite_blt(Bitmaps[BITMAP_DIALOG_TITLE_SKILL], x + 250 - Bitmaps[BITMAP_DIALOG_TITLE_SKILL]->bitmap->w / 2, y + 20,
               NULL, NULL);
    add_close_button(x, y, MENU_SKILL, skindef.newclosebutton);

    /* tabs */
    draw_tabs(skill_tab, &skill_list_set.group_nr, "Skill Group", x + 8, y + 70);

    sprintf(buf,
            "~SHIFT~ + ~%c%c~ to select group                  ~%c%c~ to select skill                    ~RETURN~ for use",
            ASCII_UP, ASCII_DOWN, ASCII_UP, ASCII_DOWN);
    StringBlt(ScreenSurface, &SystemFont, buf, x + 135, y + 410, COLOR_WHITE, NULL, NULL);

    /* Headline */
    StringBlt(ScreenSurface, &SystemFont, "Name", x + TXT_START_NAME + 1, y + TXT_Y_START - 1, COLOR_BLACK, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, "Name", x + TXT_START_NAME, y + TXT_Y_START - 2, COLOR_WHITE, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, "Level", x + TXT_START_LEVEL + 1, y + TXT_Y_START - 1, COLOR_BLACK, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, "Level", x + TXT_START_LEVEL, y + TXT_Y_START - 2, COLOR_WHITE, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, "Experience", x + TXT_START_EXP + 1, y + TXT_Y_START - 1, COLOR_BLACK, NULL,
              NULL);
    StringBlt(ScreenSurface, &SystemFont, "Experience", x + TXT_START_EXP, y + TXT_Y_START - 2, COLOR_WHITE, NULL, NULL);

    box.x = x + 133;
    box.y = y + TXT_Y_START + 1;
    box.w = 329;
    box.h = 12;

    /* frame for selection field */
    draw_frame(box.x - 1, box.y + 11, box.w + 1, 313);

    /* print skill entries */
    if (!(mb & SDL_BUTTON(SDL_BUTTON_LEFT)))
        active = 0;
    if (mx > x + TXT_START_NAME
            && mx <x + TXT_START_NAME + 327
            && my> y + TXT_Y_START
            && my < y + 12 + TXT_Y_START + DIALOG_LIST_ENTRY * 12)
    {
        if (!mb)
        {
            if (dblclk == 1)
                dblclk = 2;
            if (dblclk == 3)
            {
                dblclk = 0;
                if (skill_list[skill_list_set.group_nr].entry[skill_list_set.entry_nr].flag == LIST_ENTRY_KNOWN)
                    check_menu_keys(MENU_SKILL, SDLK_RETURN);
            }
        }
        else
        {
            if (dblclk == 0)
            {
                dblclk = 1;
                Ticks = SDL_GetTicks();
            }
            if (dblclk == 2)
            {
                dblclk = 3;
                if (SDL_GetTicks() - Ticks > 300)
                    dblclk = 0;
            }
            if (mb_clicked) /* mb was pressed in the selection field */
                active = 1;
            if (active && skill_list_set.entry_nr != (my - y - 12 - TXT_Y_START) / 12)
            {
                skill_list_set.entry_nr = (my - y - 12 - TXT_Y_START) / 12;
                dblclk = 0;
            }
        }
    }
    for (i = 0; i < DIALOG_LIST_ENTRY; i++)
    {
        y += 12;
        box.y += 12;
        if (i != skill_list_set.entry_nr)
        {
            if (i & 1)
                SDL_FillRect(ScreenSurface, &box, skindef.rowcolor[0]);
            else
                SDL_FillRect(ScreenSurface, &box, skindef.rowcolor[1]);
        }
        else
            SDL_FillRect(ScreenSurface, &box, sdl_blue1);

        if (skill_list[skill_list_set.group_nr].entry[i].flag == LIST_ENTRY_KNOWN)
        {
            StringBlt(ScreenSurface, &SystemFont, skill_list[skill_list_set.group_nr].entry[i].name, x + TXT_START_NAME,
                      y + TXT_Y_START, COLOR_WHITE, NULL, NULL);
            if (skill_list[skill_list_set.group_nr].entry[i].exp == -1)
                strcpy(buf, "**");
            else
                sprintf(buf, "%d", skill_list[skill_list_set.group_nr].entry[i].exp_level);

            StringBlt(ScreenSurface, &SystemFont, buf, x + TXT_START_LEVEL, y + TXT_Y_START, COLOR_WHITE, NULL, NULL);

            if (skill_list[skill_list_set.group_nr].entry[i].exp == -1)
                strcpy(buf, "**");
            else if (skill_list[skill_list_set.group_nr].entry[i].exp == -2)
                strcpy(buf, "**");
            else
                sprintf(buf, "%d", skill_list[skill_list_set.group_nr].entry[i].exp);
            StringBlt(ScreenSurface, &SystemFont, buf, x + TXT_START_EXP, y + TXT_Y_START, COLOR_WHITE, NULL, NULL);
        }
    }
    x += 160; y += 120;

    /* print skill description */
    if (skill_list[skill_list_set.group_nr].entry[skill_list_set.entry_nr].flag >= LIST_ENTRY_KNOWN)
    {
        if ((mb & SDL_BUTTON(SDL_BUTTON_LEFT)) && mx > x - 40 && mx <x - 10 && my> y + 10 && my < y + 43)
        {
            /* selected */
            check_menu_keys(MENU_SKILL, SDLK_RETURN);
        }
        sprite_blt(skill_list[skill_list_set.group_nr].entry[skill_list_set.entry_nr].icon, x - 42, y + 10, NULL, NULL);
        /* center ypos of textblock */
        for (i = 0; i < 4; i++)
            if (skill_list[skill_list_set.group_nr].entry[skill_list_set.entry_nr].desc[i][0] == ' ')
                y += 6;
        /* print textblock */
        for (i = 0; i <= 3; i++)
        {
            StringBlt(ScreenSurface, &SystemFont,
                      &skill_list[skill_list_set.group_nr].entry[skill_list_set.entry_nr].desc[i][0], x - 2, y + 1,
                      COLOR_BLACK, NULL, NULL);
            StringBlt(ScreenSurface, &SystemFont,
                      &skill_list[skill_list_set.group_nr].entry[skill_list_set.entry_nr].desc[i][0], x - 3, y,
                      COLOR_WHITE, NULL, NULL);
            y += 13;
        }
    }
    if (!mb)
        active_button = -1;
}

/******************************************************************
 show the spell-window and handle mouse actions.
******************************************************************/
void show_spelllist(void)
{
    SDL_Rect        box;
    char            buf[256];
    int             x, y, i;
    int             mx, my, mb;
    static int      active = 0, dblclk = 0;
    static Uint32   Ticks   = 0;

    mb = SDL_GetMouseState(&mx, &my) & SDL_BUTTON(SDL_BUTTON_LEFT);
    /* background */
    x = Screensize.x / 2 - Bitmaps[BITMAP_DIALOG_BG]->bitmap->w / 2;
    y = Screensize.y / 2 - Bitmaps[BITMAP_DIALOG_BG]->bitmap->h / 2;
    sprite_blt(Bitmaps[BITMAP_DIALOG_BG], x, y, NULL, NULL);
    sprite_blt(Bitmaps[BITMAP_DIALOG_TITLE_SPELL], x + 250 - Bitmaps[BITMAP_DIALOG_TITLE_SPELL]->bitmap->w / 2, y + 20,
               NULL, NULL);
    sprite_blt(Bitmaps[BITMAP_PENTAGRAM], x + 25, y + 430, NULL, NULL);
    add_close_button(x, y, MENU_SPELL, skindef.newclosebutton);

    /* tabs */
    draw_tabs(spell_tab, &spell_list_set.group_nr, "Spell Path", x + 8, y + 70);

    sprintf(buf,
            "~SHIFT~ + ~%c%c~ to select path                   ~%c%c~ to select spell                    ~RETURN~ for use",
            ASCII_UP, ASCII_DOWN, ASCII_UP, ASCII_DOWN);
    StringBlt(ScreenSurface, &SystemFont, buf, x + 135, y + 410, COLOR_WHITE, NULL, NULL);

    /* spellClass buttons */
    for (i = 0; i < SPELL_LIST_CLASS; i++)
    {
        if (add_gr_button(x + 133 + i * 56, y + 75, (spell_list_set.class_nr == i), BITMAP_DIALOG_BUTTON_UP,
                          spell_class[i], NULL))
            spell_list_set.class_nr = i;
    }

    sprintf(buf, "use ~F1-F8~ for spell to quickbar");
    StringBlt(ScreenSurface, &SystemFont, buf, x + 340, y + 69, COLOR_WHITE, NULL, NULL);
    sprintf(buf, "use ~%c%c~ to select spell group", ASCII_RIGHT, ASCII_LEFT);
    StringBlt(ScreenSurface, &SystemFont, buf, x + 340, y + 80, COLOR_WHITE, NULL, NULL);

    box.x = x + 133;
    box.y = y + TXT_Y_START + 1;
    box.w = 329;
    box.h = 12;

    /* frame for selection field */
    draw_frame(box.x - 1, box.y + 11, box.w + 1, 313);

    /* print skill entries */
    if (!mb)
        active = 0;
    if (mx > x + TXT_START_NAME
            && mx <x + TXT_START_NAME + 327
            && my> y + TXT_Y_START
            && my < y + 12 + TXT_Y_START + DIALOG_LIST_ENTRY * 12)
    {
        if (!mb)
        {
            if (dblclk == 1)
                dblclk = 2;
            if (dblclk == 3)
            {
                dblclk = 0;
                if (spell_list[spell_list_set.group_nr]
                        .entry[spell_list_set.class_nr][spell_list_set.entry_nr].flag == LIST_ENTRY_KNOWN)
                    check_menu_keys(MENU_SPELL, SDLK_RETURN);
            }
        }
        else
        {
            if (dblclk == 0)
            {
                dblclk = 1;
                Ticks = SDL_GetTicks();
            }
            if (dblclk == 2)
            {
                dblclk = 3;
                if (SDL_GetTicks() - Ticks > 300)
                    dblclk = 0;
            }
            if (mb_clicked) /* mb was pressed in the selection field */
                active = 1;
            if (active && spell_list_set.entry_nr != (my - y - 12 - TXT_Y_START) / 12)
            {
                spell_list_set.entry_nr = (my - y - 12 - TXT_Y_START) / 12;;
                dblclk = 0;
            }
        }
    }
    for (i = 0; i < DIALOG_LIST_ENTRY; i++)
    {
        y += 12;
        box.y += 12;
        if (i != spell_list_set.entry_nr)
        {
            if (i & 1)
                SDL_FillRect(ScreenSurface, &box, skindef.rowcolor[0]);
            else
                SDL_FillRect(ScreenSurface, &box, skindef.rowcolor[1]);
        }
        else
            SDL_FillRect(ScreenSurface, &box, sdl_blue1);

        if (spell_list[spell_list_set.group_nr].entry[spell_list_set.class_nr][i].flag == LIST_ENTRY_KNOWN)
        {
            StringBlt(ScreenSurface, &SystemFont,
                      spell_list[spell_list_set.group_nr].entry[spell_list_set.class_nr][i].name, x + TXT_START_NAME,
                      y + TXT_Y_START, COLOR_WHITE, NULL, NULL);
        }
    }

    x += 160; y += 120;
    /* print spell description */
    if (spell_list[spell_list_set.group_nr].entry[spell_list_set.class_nr][spell_list_set.entry_nr].flag
            == LIST_ENTRY_KNOWN)
    {
        if (mb && mx > x - 40 && mx <x - 10 && my> y + 10 && my < y + 43)
        {
            /* selected */
            dblclk = 0;
            check_menu_keys(MENU_SPELL, SDLK_RETURN);
        }
        sprite_blt(spell_list[spell_list_set.group_nr].entry[spell_list_set.class_nr][spell_list_set.entry_nr].icon,
                   x - 42, y + 10, NULL, NULL);
        /* center ypos of textblock */
        for (i = 0; i < 4; i++)
            if (spell_list[spell_list_set.group_nr].entry[spell_list_set.class_nr][spell_list_set.entry_nr].desc[i][0]
                    == ' ')
                y += 6;
        /* print textblock */
        for (i = 0; i < 4; i++)
        {
            StringBlt(ScreenSurface, &SystemFont,
                      &spell_list[spell_list_set.group_nr].entry[spell_list_set.class_nr][spell_list_set.entry_nr].desc[i][0],
                      x
                      - 2, y + 1, COLOR_BLACK, NULL, NULL);
            StringBlt(ScreenSurface, &SystemFont,
                      &spell_list[spell_list_set.group_nr].entry[spell_list_set.class_nr][spell_list_set.entry_nr].desc[i][0],
                      x
                      - 3, y, COLOR_WHITE, NULL, NULL);
            y += 13;
        }
    }
    if (!mb)
        active_button = -1;
}

/******************************************************************
 show the option-window and handle mouse actions.
******************************************************************/
void show_optwin()
{
    char    buf[128];
    int     x, y;
    int     mx, my, mb;
    int     numButton   = 0;

    mb = SDL_GetMouseState(&mx, &my) & SDL_BUTTON(SDL_BUTTON_LEFT);
    x = Screensize.x / 2 - Bitmaps[BITMAP_DIALOG_BG]->bitmap->w / 2;
    y = Screensize.y / 2 - Bitmaps[BITMAP_DIALOG_BG]->bitmap->h / 2;
    sprite_blt(Bitmaps[BITMAP_DIALOG_BG], x, y, NULL, NULL);
    sprite_blt(Bitmaps[BITMAP_DIALOG_TITLE_OPTIONS], x + 250 - Bitmaps[BITMAP_DIALOG_TITLE_OPTIONS]->bitmap->w / 2,
               y + 20, NULL, NULL);
    add_close_button(x, y, MENU_OPTION, skindef.newclosebutton);

    draw_tabs(opt_tab, &option_list_set.group_nr, "Option Group", x + 8, y + 70);
    optwin_draw_options(x + 130, y + 90);

    sprintf(buf, "~SHIFT~ + ~%c%c~ to select group            ~%c%c~ to select option          ~%c%c~ to change option",
            ASCII_UP, ASCII_DOWN, ASCII_UP, ASCII_DOWN, ASCII_RIGHT, ASCII_LEFT);
    StringBlt(ScreenSurface, &SystemFont, buf, x + 135, y + 410, COLOR_WHITE, NULL, NULL);
    /* mark active entry */
    StringBlt(ScreenSurface, &SystemFont, ">", x + TXT_START_NAME - 15,
              y + 10 + TXT_Y_START + option_list_set.entry_nr * 20, COLOR_HGOLD, NULL, NULL);

    /* save button */
    if (add_button(x + 25, y + 454, numButton++, BITMAP_DIALOG_BUTTON_UP, "Done", "~D~one"))
        check_menu_keys(MENU_OPTION, SDLK_d);

    if (!mb)
        active_button = -1;
}

/******************************************************************
 show the keybind-window and handle mouse actions.
******************************************************************/
void show_keybind()
{
    SDL_Rect    box;
    char        buf[256];
    int         x, x2, y, y2, i;
    int         mx, my, mb;
    int         numButton   = 0;

    mb = SDL_GetMouseState(&mx, &my);
    /* background */
    x = Screensize.x / 2 - Bitmaps[BITMAP_DIALOG_BG]->bitmap->w / 2;
    y = Screensize.y / 2 - Bitmaps[BITMAP_DIALOG_BG]->bitmap->h / 2;
    sprite_blt(Bitmaps[BITMAP_DIALOG_BG], x, y, NULL, NULL);
    sprite_blt(Bitmaps[BITMAP_DIALOG_TITLE_KEYBIND], x + 250 - Bitmaps[BITMAP_DIALOG_TITLE_KEYBIND]->bitmap->w / 2,
               y + 20, NULL, NULL);
    add_close_button(x, y, MENU_KEYBIND, skindef.newclosebutton);

    sprintf(buf, "~SHIFT~ + ~%c%c~ to select group         ~%c%c~ to select macro          ~RETURN~ to change/create",
            ASCII_UP, ASCII_DOWN, ASCII_UP, ASCII_DOWN);
    StringBlt(ScreenSurface, &SystemFont, buf, x + 125, y + 410, COLOR_WHITE, NULL, NULL);


    /* draw group tabs */
    i = 0;
    x2 = x + 8; y2 = y + 70;
    sprite_blt(Bitmaps[BITMAP_DIALOG_TAB_START], x2, y2 - 10, NULL, NULL);
    sprite_blt(Bitmaps[BITMAP_DIALOG_TAB], x2, y2, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, "Group", x2 + 15, y2 + 4, COLOR_BLACK, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, "Group", x2 + 14, y2 + 3, COLOR_WHITE, NULL, NULL);
    y2 += 17;
    sprite_blt(Bitmaps[BITMAP_DIALOG_TAB], x2, y2, NULL, NULL);
    y2 += 17;
    while (i < BINDKEY_LIST_MAX && bindkey_list[i].name[0])
    {
        sprite_blt(Bitmaps[BITMAP_DIALOG_TAB], x2, y2, NULL, NULL);
        if (i == bindkey_list_set.group_nr)
            sprite_blt(Bitmaps[BITMAP_DIALOG_TAB_SEL], x2, y2, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont, bindkey_list[i].name, x2 + 25, y2 + 4, COLOR_BLACK, NULL, NULL);
        if (mx > x2 && mx <x2 + 100 && my> y2 && my < y2 + 17)
        {
            StringBlt(ScreenSurface, &SystemFont, bindkey_list[i].name, x2 + 24, y2 + 3, COLOR_HGOLD, NULL, NULL);
            if (mb && bindkey_list_set.group_nr != i)
                bindkey_list_set.group_nr = i;
        }
        else
            StringBlt(ScreenSurface, &SystemFont, bindkey_list[i].name, x2 + 24, y2 + 3, COLOR_WHITE, NULL, NULL);
        y2 += 17;
        i++;
    }
    sprite_blt(Bitmaps[BITMAP_DIALOG_TAB_STOP], x2, y2, NULL, NULL);

    /* Headline */
    StringBlt(ScreenSurface, &SystemFont, "Macro", x + X_COL1 + 1, y + TXT_Y_START - 1, COLOR_BLACK, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, "Macro", x + X_COL1, y + TXT_Y_START - 2, COLOR_WHITE, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, "Key", x + X_COL2 + 1, y + TXT_Y_START - 1, COLOR_BLACK, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, "Key", x + X_COL2, y + TXT_Y_START - 2, COLOR_WHITE, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, "Repeat", x + X_COL3 + 1, y + TXT_Y_START - 1, COLOR_BLACK, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, "~R~epeat", x + X_COL3, y + TXT_Y_START - 2, COLOR_WHITE, NULL, NULL);

    /* save button */
    if (add_button(x + 25, y + 454, numButton++, BITMAP_DIALOG_BUTTON_UP, "Done", "~D~one"))
        check_menu_keys(MENU_KEYBIND, SDLK_d);


    box.x = x + 133;
    box.y = y + TXT_Y_START + 1;
    box.w = 329;
    box.h = 12;

    /* frame for selection field */
    draw_frame(box.x - 1, box.y + 11, box.w + 1, 313);
    y2 = y;

    /* print keybind entries */
    for (i = 0; i < OPTWIN_MAX_OPT; i++)
    {
        y += 12;
        box.y += 12;
        if (mb
                && mx > x + TXT_START_NAME
                && mx <x + TXT_START_NAME + 327
                && my> y + TXT_Y_START
                && my < y + 12 + TXT_Y_START)
        {
            if (mb & SDL_BUTTON(SDL_BUTTON_LEFT))
            {
                /* cancel edit */
                InputStringEscFlag = TRUE;
                keybind_status = KEYBIND_STATUS_NO;
            }
            bindkey_list_set.entry_nr = i;
            if ((mb & SDL_BUTTON(SDL_BUTTON_RIGHT)) && mb_clicked && mx > x + X_COL1)
            {
                if (mx < x + X_COL3) /* macro name + key value */
                    check_menu_keys(MENU_KEYBIND, SDLK_RETURN);
                else  /* key repeat */
                    check_menu_keys(MENU_KEYBIND, SDLK_r);
                mb_clicked = 0;
            }
        }
        if (i != bindkey_list_set.entry_nr)
        {
            if (i & 1)
                SDL_FillRect(ScreenSurface, &box, skindef.rowcolor[0]);
            else
                SDL_FillRect(ScreenSurface, &box, skindef.rowcolor[1]);
        }
        else
            SDL_FillRect(ScreenSurface, &box, sdl_blue1);

        if (bindkey_list[bindkey_list_set.group_nr].entry[i].text[0])
        {
            StringBlt(ScreenSurface, &SystemFont, bindkey_list[bindkey_list_set.group_nr].entry[i].text, x + X_COL1,
                      y + TXT_Y_START, COLOR_WHITE, NULL, NULL);
            StringBlt(ScreenSurface, &SystemFont, bindkey_list[bindkey_list_set.group_nr].entry[i].keyname, x + X_COL2,
                      y + TXT_Y_START, COLOR_WHITE, NULL, NULL);
            if (bindkey_list[bindkey_list_set.group_nr].entry[i].repeatflag)
                StringBlt(ScreenSurface, &SystemFont, "on", x + X_COL3, y + TXT_Y_START, COLOR_WHITE, NULL, NULL);
            else
                StringBlt(ScreenSurface, &SystemFont, "off", x + X_COL3, y + TXT_Y_START, COLOR_WHITE, NULL, NULL);
        }
    }

    /* Edit modus */
    y2 += 13 + TXT_Y_START + bindkey_list_set.entry_nr * 12;
    box.y = y2;
    if (keybind_status == KEYBIND_STATUS_EDIT)
    {
        box.w = X_COL2 - X_COL1;
        SDL_FillRect(ScreenSurface, &box, 0);
        StringBlt(ScreenSurface, &SystemFont, show_input_string(InputString, &SystemFont, box.w), x + X_COL1, y2,
                  COLOR_WHITE, NULL, NULL);
    }
    else if (keybind_status == KEYBIND_STATUS_EDITKEY)
    {
        box.x += X_COL2 - X_COL1;
        box.w = X_COL3 - X_COL2;
        SDL_FillRect(ScreenSurface, &box, 0);
        StringBlt(ScreenSurface, &SystemFont, "Press a Key!", x + X_COL2, y2, COLOR_WHITE, NULL, NULL);
    }

    if (!mb)
        active_button = -1;
}

static void blit_face(int id, int x, int y)
{
    if (id == -1 || !FaceList[id].sprite)
        return;

    if (FaceList[id].sprite->status != SPRITE_STATUS_LOADED)
        return;

    sprite_blt(FaceList[id].sprite, x, y, NULL, NULL);
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
    int             id  = 0;
    int             x, y, i;
    char            buf[256];
    int             mx, my, mb;
    int             delta;
    _server_char   *tmpc;

    mb = SDL_GetMouseState(&mx, &my) & SDL_BUTTON(SDL_BUTTON_LEFT);
    x = 25;
    /*x= Screensize.x/2-Bitmaps[BITMAP_DIALOG_BG]->bitmap->w/2;*/
    y = Screensize.y / 2 - Bitmaps[BITMAP_DIALOG_BG]->bitmap->h / 2;
    sprite_blt(Bitmaps[BITMAP_DIALOG_BG], x, y, NULL, NULL);
    sprite_blt(Bitmaps[BITMAP_DIALOG_TITLE_CREATION], x + 250 - Bitmaps[BITMAP_DIALOG_TITLE_CREATION]->bitmap->w / 2,
               y + 20, NULL, NULL);
    sprite_blt(Bitmaps[BITMAP_PENTAGRAM], x + 25, y + 430, NULL, NULL);
    add_close_button(x, y, MENU_CREATE, skindef.newclosebutton);

    /* print all attributes */
    StringBlt(ScreenSurface, &SystemFont, "Welcome!", x + 131, y + 64, COLOR_BLACK, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, "Welcome!", x + 130, y + 63, COLOR_WHITE, NULL, NULL);
    sprintf(buf, "Use ~%c%c~ and ~%c%c~ cursor keys to setup your stats.", ASCII_UP, ASCII_DOWN, ASCII_RIGHT, ASCII_LEFT);
    StringBlt(ScreenSurface, &SystemFont, buf, x + 131, y + 76, COLOR_BLACK, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, buf, x + 130, y + 75, COLOR_WHITE, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, "Adjust the stats using *all* your points.", x + 131, y + 89, COLOR_BLACK,
              NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, "Adjust the stats using *all* your points.", x + 130, y + 88, COLOR_WHITE,
              NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, "Press ~C~ to create your new character", x + 131, y + 101, COLOR_BLACK, NULL,
              NULL);
    StringBlt(ScreenSurface, &SystemFont, "Press ~C~ to create your new character.", x + 130, y + 100, COLOR_WHITE,
              NULL, NULL);


    /* create button */
    if (add_button(x + 30, y + 397, id++, BITMAP_DIALOG_BUTTON_UP, "Create", "~C~reate"))
        check_menu_keys(MENU_CREATE, SDLK_c);

    if (dialog_new_char_warn == 1)
    {
        StringBlt(ScreenSurface, &SystemFont, "  ** ASSIGN ALL **", x + 21, y + 368, COLOR_BLACK, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont, "  ** ASSIGN ALL **", x + 20, y + 367, COLOR_HGOLD, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont, "** POINTS FIRST **", x + 21, y + 380, COLOR_BLACK, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont, "** POINTS FIRST **", x + 20, y + 379, COLOR_HGOLD, NULL, NULL);
    }
    else if (dialog_new_char_warn == 2)
    {
        StringBlt(ScreenSurface, &SystemFont, "  ** ASSIGN YOUR **", x + 21, y + 368, COLOR_BLACK, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont, "  ** ASSIGN YOUR **", x + 20, y + 367, COLOR_HGOLD, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont, "** W-SKILL FIRST **", x + 21, y + 380, COLOR_BLACK, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont, "** W-SKILL FIRST **", x + 20, y + 379, COLOR_HGOLD, NULL, NULL);
    }

    /* draw attributes */
    StringBlt(ScreenSurface, &SystemFont, "Points:", x + 130, y + CREATE_Y0 + 3 * 17, COLOR_WHITE, NULL, NULL);
    if (new_character.stat_points)
        sprintf(buf, "%.2d  LEFT", new_character.stat_points);
    else
        sprintf(buf, "%.2d", new_character.stat_points);
    StringBlt(ScreenSurface, &SystemFont, buf, x + 171, y + CREATE_Y0 + 3 * 17 + 1, COLOR_BLACK, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, buf, x + 170, y + CREATE_Y0 + 3 * 17, COLOR_HGOLD, NULL, NULL);

//    if (create_list_set.entry_nr > 8)
//        create_list_set.entry_nr = 8;

    for (i = 0; i < ATT_SUM; i++)
    {
        id += 2;
        sprintf(buf, "%s:", attribute[i].name);

        if (create_list_set.entry_nr == i + 2)
            StringBlt(ScreenSurface, &SystemFont, buf, x + 130, y + CREATE_Y0 + (i + 4) * 17, COLOR_GREEN, NULL, NULL);
        else
            StringBlt(ScreenSurface, &SystemFont, buf, x + 130, y + CREATE_Y0 + (i + 4) * 17, COLOR_WHITE, NULL, NULL);
        sprintf(buf, "%.2d", new_character.stats[i]);

        if (create_list_set.entry_nr == i + 2)
            delta = add_rangebox(x + 170, y + CREATE_Y0 + (i + 4) * 17, id, 20, 0, buf, COLOR_GREEN);
        else
            delta = add_rangebox(x + 170, y + CREATE_Y0 + (i + 4) * 17, id, 20, 0, buf, COLOR_WHITE);
        /* keyboard event */

        if (create_list_set.key_change && create_list_set.entry_nr == i + 2)
        {
            delta = create_list_set.key_change;
            create_list_set.key_change = 0;
        }

        if (delta)
        {

            dialog_new_char_warn = 0;
            if (delta > 0)
            {
                if (new_character.stats[i] + 1 <= new_character.stats_max[i] && new_character.stat_points)
                {
                    new_character.stats[i]++;
                    new_character.stat_points--;
                }
            }
            else
            {
                if (new_character.stats[i] - 1 >= new_character.stats_min[i])
                {
                    new_character.stats[i]--;
                    new_character.stat_points++;
                }
            }
        }
    }

    if (create_list_set.entry_nr == 0)
        StringBlt(ScreenSurface, &SystemFont, "Race:", x + 130, y + CREATE_Y0 + 0 * 17 + 2, COLOR_GREEN, NULL, NULL);
    else
        StringBlt(ScreenSurface, &SystemFont, "Race:", x + 130, y + CREATE_Y0 + 0 * 17 + 2, COLOR_WHITE, NULL, NULL);

    sprintf(buf,"%s %s", gender[new_character.gender_selected], new_character.name);
    if (create_list_set.entry_nr == 0)
        delta = add_rangebox(x + 170, y + CREATE_Y0 + 0 * 17, ++id, 80, 0, buf, COLOR_GREEN);
    else
        delta = add_rangebox(x + 170, y + CREATE_Y0 + 0 * 17, ++id, 80, 0, buf, COLOR_WHITE);
    if (create_list_set.key_change && create_list_set.entry_nr == 0)
    {
        delta = create_list_set.key_change;
        create_list_set.key_change = 0;
    }

    if (delta) /* init new race */
    {
        int g;

        new_character.skill_selected = -1;
        if (delta >0)
        {
            /* try to get a new valid gender */
            for (;;)
            {
                ++new_character.gender_selected;
                if (new_character.gender_selected > 3)
                {
                    g = -1;
                    break;
                }

                if (new_character.gender[new_character.gender_selected])
                {
                    g = new_character.gender_selected;
                    break;
                }
            }
        }
        else
        {
            /* try to get a new valid gender */
            for (;;)
            {
                --new_character.gender_selected;
                if (new_character.gender_selected < 0)
                {
                    g = -1;
                    break;
                }

                if (new_character.gender[new_character.gender_selected])
                {
                    g = new_character.gender_selected;
                    break;
                }
            }
        }

        if (g == -1)
        {
            for (tmpc = first_server_char; tmpc; tmpc = tmpc->next)
            {
                /* get our current template */
                if (!strcmp(tmpc->name, new_character.name))
                {
                    /* get next template */
                    if (delta > 0)
                    {
                        tmpc = tmpc->next;
                        if (!tmpc)
                            tmpc = first_server_char;
                        memcpy(&new_character, tmpc, sizeof(_server_char));

                        for (new_character.gender_selected = 0;;++new_character.gender_selected)
                        {
                            if (new_character.gender[new_character.gender_selected])
                                break;
                        }
                        break;
                    }
                    else
                    {
                        tmpc = tmpc->prev;
                        if (!tmpc)
                        {
                            /* get last node */
                            for (tmpc = first_server_char; tmpc->next; tmpc = tmpc->next)
                                ;
                        }
                        memcpy(&new_character, tmpc, sizeof(_server_char));

                        for (new_character.gender_selected = 3;;--new_character.gender_selected)
                        {
                            if (new_character.gender[new_character.gender_selected])
                                break;
                        }
                        break;
                    }
                }
            }
        }
    }

    if (create_list_set.entry_nr == 1)
        StringBlt(ScreenSurface, &SystemFont, "W-Skill:", x + 130, y + CREATE_Y0 + 1 * 17 + 2, COLOR_GREEN, NULL, NULL);
    else
        StringBlt(ScreenSurface, &SystemFont, "W-Skill:", x + 130, y + CREATE_Y0 + 1 * 17 + 2, COLOR_WHITE, NULL, NULL);

    if (create_list_set.entry_nr == 1)
        delta = add_rangebox(x + 170, y + CREATE_Y0 + 1 * 17, ++id, 80, 0, weapon_skill[new_character.skill_selected+1], COLOR_GREEN);
    else
        delta = add_rangebox(x + 170, y + CREATE_Y0 + 1 * 17, ++id, 80, 0, weapon_skill[new_character.skill_selected+1], COLOR_WHITE);

    if (create_list_set.key_change && create_list_set.entry_nr == 1)
    {
        delta = create_list_set.key_change;
        create_list_set.key_change = 0;
    }
    if (delta)
    {
        if (delta > 0) /* +1 */
        {
            if (++new_character.skill_selected>3)
                new_character.skill_selected = 0;
        }
        else
        {
            if (--new_character.skill_selected<0)
                new_character.skill_selected = 3;
        }
    }

    /* draw player image */
    StringBlt(ScreenSurface, &SystemFont, cpl.name, x + 40, y + 85, COLOR_WHITE, NULL, NULL);

    blit_face(new_character.face_id[new_character.gender_selected], x + 35, y + 100);

    sprintf(buf, "HP: ~%d~", new_character.bar[0] * 4 + new_character.bar_add[0]);
    StringBlt(ScreenSurface, &SystemFont, buf, x + 36, y + 146, COLOR_BLACK, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, buf, x + 35, y + 145, COLOR_WHITE, NULL, NULL);
    sprintf(buf, "SP: ~%d~", new_character.bar[1] * 2 + new_character.bar_add[1]);
    StringBlt(ScreenSurface, &SystemFont, buf, x + 36, y + 157, COLOR_BLACK, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, buf, x + 35, y + 156, COLOR_WHITE, NULL, NULL);
    sprintf(buf, "GR: ~%d~", new_character.bar[2] * 2 + new_character.bar_add[2]);
    StringBlt(ScreenSurface, &SystemFont, buf, x + 36, y + 168, COLOR_BLACK, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, buf, x + 35, y + 167, COLOR_WHITE, NULL, NULL);

    StringBlt(ScreenSurface, &SystemFont, "W-Skill is your 'weapon skill' - the kind of weapon you are able to use." , x + 135, y + 324, COLOR_BLACK, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, "W-Skill is your 'weapon skill' - the kind of weapon you are able to use.", x + 134, y + 323, COLOR_WHITE, NULL, NULL);

    StringBlt(ScreenSurface, &SystemFont, "There are 4 base weapon skills:" , x + 135, y + 346, COLOR_BLACK, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, "There are 4 base weapon skills:", x + 134, y + 345, COLOR_WHITE, NULL, NULL);


    StringBlt(ScreenSurface, &SystemFont, "* SLASH allows use of bladed weapons like swords" , x + 135, y + 358, COLOR_BLACK, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, "* SLASH allows use of bladed weapons like swords", x + 134, y + 357, COLOR_WHITE, NULL, NULL);

    StringBlt(ScreenSurface, &SystemFont, "* IMPACT is for mace, morningstars, clubs and hammers" , x + 135, y + 370, COLOR_BLACK, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, "* IMPACT is for mace, morningstars, clubs and hammers", x + 134, y + 369, COLOR_WHITE, NULL, NULL);

    StringBlt(ScreenSurface, &SystemFont, "* CLEAVE allows use of any sort of axes" , x + 135, y + 382, COLOR_BLACK, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, "* CLEAVE allows use of any sort of axes", x + 134, y + 381, COLOR_WHITE, NULL, NULL);

    StringBlt(ScreenSurface, &SystemFont, "* PIERCE is for daggers, degen or rapiers" , x + 135, y + 394, COLOR_BLACK, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, "* PIERCE is for daggers, degen or rapiers", x + 134, y + 393, COLOR_WHITE, NULL, NULL);


    StringBlt(ScreenSurface, &SystemFont, new_character.desc[0], x + 160, y + 434, COLOR_BLACK, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, new_character.desc[0], x + 159, y + 433, COLOR_WHITE, NULL, NULL);

    StringBlt(ScreenSurface, &SystemFont, new_character.desc[1], x + 160, y + 446, COLOR_BLACK, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, new_character.desc[1], x + 159, y + 445, COLOR_WHITE, NULL, NULL);

    StringBlt(ScreenSurface, &SystemFont, new_character.desc[2], x + 160, y + 458, COLOR_BLACK, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, new_character.desc[2], x + 159, y + 457, COLOR_WHITE, NULL, NULL);

    StringBlt(ScreenSurface, &SystemFont, new_character.desc[3], x + 160, y + 470, COLOR_BLACK, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, new_character.desc[3], x + 159, y + 469, COLOR_WHITE, NULL, NULL);

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

    if (!mb)
        active_button = -1;
}

/******************************************************************
 show login: status & identify part.
******************************************************************/
void show_login_server(void)
{
    SDL_Rect    box;
    char        buf[256];
    int         x, y, i;
    int         mx, my, mb, t;

    mb = SDL_GetMouseState(&mx, &my);
    /* background */
    /*x= Screensize.x/2-Bitmaps[BITMAP_DIALOG_BG]->bitmap->w/2;*/
    x = 25;
    y = Screensize.y / 2 - Bitmaps[BITMAP_DIALOG_BG]->bitmap->h / 2;
    sprite_blt(Bitmaps[BITMAP_DIALOG_BG], x, y, NULL, NULL);
    sprite_blt(Bitmaps[BITMAP_LOGO270], x + 20, y + 85, NULL, NULL);
    sprite_blt(Bitmaps[BITMAP_DIALOG_TITLE_LOGIN], x + 250 - Bitmaps[BITMAP_DIALOG_TITLE_LOGIN]->bitmap->w / 2, y + 20,
               NULL, NULL);
    /*  add_close_button(x, y, MENU_LOGIN); */

    t = x + 275;
    x += 170;
    y += 100;
    draw_frame(x - 3, y - 3, 211, 168);
    box.x = x - 2;
    box.y = y - 2;
    box.w = 210;
    box.h = 17;

    StringBlt(ScreenSurface, &SystemFont, "Server", t + 1 - 21, y - 35, COLOR_BLACK, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, "Server", t - 21, y - 36, COLOR_WHITE, NULL, NULL);

    if (!strcmp(ServerName, "127.0.0.1"))
        strcpy(buf, "local host");
    else
        sprintf(buf, "%s", ServerName);

    t -= get_string_pixel_length(buf, &BigFont) / 2;
    StringBlt(ScreenSurface, &BigFont, buf, t + 1, y - 21, COLOR_BLACK, NULL, NULL);
    StringBlt(ScreenSurface, &BigFont, buf, t, y - 22, COLOR_HGOLD, NULL, NULL);

    SDL_FillRect(ScreenSurface, &box, skindef.rowcolor[0]);
    box.y = y + 15;
    box.h = 150;
    SDL_FillRect(ScreenSurface, &box, skindef.rowcolor[1]);
    StringBlt(ScreenSurface, &SystemFont, "- UPDATING FILES- ", x + 58, y + 1, COLOR_BLACK, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, "- UPDATING FILES -", x + 57, y, COLOR_WHITE, NULL, NULL);
    if (request_file_chain >= 0)
        StringBlt(ScreenSurface, &SystemFont, "Updating settings file from server....", x + 2, y + 20, COLOR_WHITE,
                  NULL, NULL);
    if (request_file_chain > 1)
        StringBlt(ScreenSurface, &SystemFont, "Updating skills file from server....", x + 2, y + 32, COLOR_WHITE, NULL,
                  NULL);
    if (request_file_chain > 3)
        StringBlt(ScreenSurface, &SystemFont, "Updating spells file from server....", x + 2, y + 44, COLOR_WHITE, NULL,
                  NULL);
    if (request_file_chain > 5)
        StringBlt(ScreenSurface, &SystemFont, "Updating bmaps file from server....", x + 2, y + 56, COLOR_WHITE, NULL,
                  NULL);
    if (request_file_chain > 7)
        StringBlt(ScreenSurface, &SystemFont, "Updating anims file from server....", x + 2, y + 68, COLOR_WHITE, NULL,
                  NULL);
    if (request_file_chain > 9)
        StringBlt(ScreenSurface, &SystemFont, "Sync files...", x + 2, y + 80, COLOR_WHITE, NULL, NULL);

    /* if set, we have requested something and the stuff in the socket buffer is our file! */
    if (request_file_chain == 1
            || request_file_chain == 3
            || request_file_chain == 5
            || request_file_chain == 7
            || request_file_chain == 9)
    {
        sprintf(buf, "received ~%d~ bytes", csocket.inbuf.len);
        StringBlt(ScreenSurface, &SystemFont, buf, x + 1, y + 150, COLOR_WHITE, NULL, NULL);
    }

    /* login user part */
    if (GameStatus == GAME_STATUS_REQUEST_FILES)
        return;
    StringBlt(ScreenSurface, &SystemFont, "done.", x + 2, y + 92, COLOR_WHITE, NULL, NULL);
    y += 180;
    if (GameStatus <= GAME_STATUS_LOGIN)
    {
        StringBlt(ScreenSurface, &SystemFont, "Query for Login. Waiting...", x, y, COLOR_HGOLD, NULL, NULL);
        return;
    }
    else if (GameStatus == GAME_STATUS_LOGIN_SELECT)
    {
        if (GameStatusLogin)
        {
            StringBlt(ScreenSurface, &BigFont, ">> Login <<", x+51, y+52, COLOR_BLACK, NULL, NULL);
            StringBlt(ScreenSurface, &BigFont, ">> Login <<", x+49, y+50, COLOR_GREEN, NULL, NULL);
            StringBlt(ScreenSurface, &BigFont, "Create Character", x+18, y+32, COLOR_BLACK, NULL, NULL);
            StringBlt(ScreenSurface, &BigFont, "Create Character", x+16, y+30, COLOR_WHITE, NULL, NULL);
        }
        else
        {
            StringBlt(ScreenSurface, &BigFont, "Login", x+72, y+52, COLOR_BLACK, NULL, NULL);
            StringBlt(ScreenSurface, &BigFont, "Login", x+70, y+50, COLOR_WHITE, NULL, NULL);
            StringBlt(ScreenSurface, &BigFont, ">> Create Character <<", x-3, y+32, COLOR_BLACK, NULL, NULL);
            StringBlt(ScreenSurface, &BigFont, ">> Create Character <<", x-5, y+30, COLOR_GREEN, NULL, NULL);
        }
        y += 160;
        StringBlt(ScreenSurface, &SystemFont,
                  "Select ~Create Character~ for a new or ~Login~ for a saved character." ,
                  x-10, y+1, COLOR_BLACK, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont, "Select ~Create Character~ for a new or ~Login~ for a saved character.",
                  x-11, y  , COLOR_WHITE, NULL, NULL);
        y+=12;
        sprintf(buf,"Use ~%c,%c~ to select and press then ~Return~", ASCII_UP, ASCII_DOWN);
        StringBlt(ScreenSurface, &SystemFont, buf ,
                  x-10, y+1, COLOR_BLACK, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont, buf,
                  x-11  , y  , COLOR_WHITE, NULL, NULL);

        return;
    }

    if (GameStatusLogin)
    {
        StringBlt(ScreenSurface, &BigFont, "Login", x+72, y+2, COLOR_BLACK, NULL, NULL);
        StringBlt(ScreenSurface, &BigFont, "Login", x+70, y+0, COLOR_WHITE, NULL, NULL);
    }
    else
    {
        StringBlt(ScreenSurface, &BigFont, "Create Character", x+18, y+2, COLOR_BLACK, NULL, NULL);
        StringBlt(ScreenSurface, &BigFont, "Create Character", x+16, y+0, COLOR_WHITE, NULL, NULL);
    }

    if (GameStatusLogin)
        StringBlt(ScreenSurface, &SystemFont, "Enter your Name", x, y+20, COLOR_HGOLD, NULL, NULL);
    else
        StringBlt(ScreenSurface, &SystemFont, "Select a Name", x, y+20, COLOR_HGOLD, NULL, NULL);

    sprite_blt(Bitmaps[BITMAP_LOGIN_INP], x - 2, y + 35, NULL, NULL);
    if (GameStatus == GAME_STATUS_NAME)
        StringBlt(ScreenSurface, &SystemFont,
                  show_input_string(InputString, &SystemFont, Bitmaps[BITMAP_LOGIN_INP]->bitmap->w - 16), x + 2, y + 37,
                  COLOR_WHITE, NULL, NULL);
    else
        StringBlt(ScreenSurface, &SystemFont, cpl.name, x + 2, y + 37, COLOR_WHITE, NULL, NULL);

    if (GameStatus < GAME_STATUS_PSWD)
    {
        if (GameStatus == GAME_STATUS_PSWD_WAIT)
            StringBlt(ScreenSurface, &SystemFont, "sending name... waiting...", x + 2, y + 60, COLOR_WHITE, NULL, NULL);
        else
        {
            if (!GameStatusLogin)
            {
                StringBlt(ScreenSurface, &SystemFont, "*REMEMBER*: ~Your website account name~",x+1, y+61, COLOR_BLACK, NULL, NULL);
                StringBlt(ScreenSurface, &SystemFont, "*REMEMBER*: ~Your website account name~",x+2, y+60, COLOR_WHITE, NULL, NULL);
                StringBlt(ScreenSurface, &SystemFont, "                             ~is NOT your character name !~",x+1, y+71, COLOR_BLACK, NULL, NULL);
                StringBlt(ScreenSurface, &SystemFont, "                             ~is NOT your character name !~",x+2, y+70, COLOR_WHITE, NULL, NULL);
                StringBlt(ScreenSurface, &SystemFont, "                             ~Select a different one WITHOUT~",x+1, y+81, COLOR_BLACK, NULL, NULL);
                StringBlt(ScreenSurface, &SystemFont, "                             ~Select a different one WITHOUT~",x+2, y+80, COLOR_WHITE, NULL, NULL);
                StringBlt(ScreenSurface, &SystemFont, "                             ~numbers and special chars in it.~",x+1, y+91, COLOR_BLACK, NULL, NULL);
                StringBlt(ScreenSurface, &SystemFont, "                             ~numbers and special chars in it.~",x+2, y+90, COLOR_WHITE, NULL, NULL);
            }
        }
        goto login_jmp;
    }
    StringBlt(ScreenSurface, &SystemFont, "Enter your Password", x + 2, y + 60, COLOR_HGOLD, NULL, NULL);
    sprite_blt(Bitmaps[BITMAP_LOGIN_INP], x - 2, y + 75, NULL, NULL);

    if (GameStatus == GAME_STATUS_PSWD)
    {
        strcpy(buf, show_input_string(InputString, &SystemFont, Bitmaps[BITMAP_LOGIN_INP]->bitmap->w - 16));
        for (i = 0; i < CurrentCursorPos; i++)
            buf[i] = '*';
        for (i = CurrentCursorPos + 1; i < (int) strlen(InputString) + 1; i++)
            buf[i] = '*';
        buf[i] = 0;
        StringBlt(ScreenSurface, &SystemFont, buf, x + 2, y + 77, COLOR_WHITE, NULL, NULL);
    }
    else
    {
        for (i = 0; i < (int) strlen(cpl.password); i++)
            buf[i] = '*';buf[i] = 0;
        StringBlt(ScreenSurface, &SystemFont, buf, x + 2, y + 77, COLOR_WHITE, NULL, NULL);
    }
    if (GameStatus < GAME_STATUS_VERIFYPSWD)
    {
        if (GameStatus == GAME_STATUS_VERIFYPSWD_WAIT)
            StringBlt(ScreenSurface, &SystemFont, "sending password... waiting...", x + 2, y + 100, COLOR_WHITE, NULL, NULL);
        goto login_jmp;
    }

    if (GameStatus == GAME_STATUS_VERIFYPSWD)
    {
        StringBlt(ScreenSurface, &SystemFont, "New Character: Verify Password", x + 2, y + 100, COLOR_HGOLD, NULL, NULL);
        sprite_blt(Bitmaps[BITMAP_LOGIN_INP], x - 2, y + 115, NULL, NULL);
        strcpy(buf, show_input_string(InputString, &SystemFont, Bitmaps[BITMAP_LOGIN_INP]->bitmap->w - 16));
        for (i = 0; i < (int) strlen(InputString); i++)
            buf[i] = '*';
        StringBlt(ScreenSurface, &SystemFont, buf, x + 2, y + 117, COLOR_WHITE, NULL, NULL);
    }

login_jmp:
    switch (dialog_login_warning_level)
    {
        case DIALOG_LOGIN_WARNING_NONE:
            break;

        case DIALOG_LOGIN_WARNING_NAME_NO:
            StringBlt(ScreenSurface, &SystemFont, "There is no character with that name!",
                      x+1, y+111  , COLOR_BLACK, NULL, NULL);
            StringBlt(ScreenSurface, &SystemFont, "There is no character with that name!",
                      x+2, y+110  , COLOR_ORANGE, NULL, NULL);
            break;
        case DIALOG_LOGIN_WARNING_NAME_BLOCKED:
            StringBlt(ScreenSurface, &SystemFont, "Name or character is in creating process or blocked!",
                      x+1, y+111  , COLOR_BLACK, NULL, NULL);
            StringBlt(ScreenSurface, &SystemFont, "Name or character is in creating process or blocked!",
                      x+2, y+110  , COLOR_ORANGE, NULL, NULL);
            break;
        case DIALOG_LOGIN_WARNING_NAME_PLAYING:
            StringBlt(ScreenSurface, &SystemFont, "Name is taken - choose a different one!",
                      x+1, y+111  , COLOR_BLACK, NULL, NULL);
            StringBlt(ScreenSurface, &SystemFont, "Name is taken - choose a different one!",
                      x+2, y+110  , COLOR_ORANGE, NULL, NULL);
            break;
        case DIALOG_LOGIN_WARNING_NAME_TAKEN:
            StringBlt(ScreenSurface, &SystemFont, "Name is taken - choose a different one!",
                      x+1, y+111  , COLOR_BLACK, NULL, NULL);
            StringBlt(ScreenSurface, &SystemFont, "Name is taken - choose a different one!",
                      x+2, y+110  , COLOR_ORANGE, NULL, NULL);
            break;
        case DIALOG_LOGIN_WARNING_NAME_BANNED:
            StringBlt(ScreenSurface, &SystemFont, "Name is banned - choose a different one!",
                      x+1, y+111  , COLOR_BLACK, NULL, NULL);
            StringBlt(ScreenSurface, &SystemFont, "Name is banned - choose a different one!",
                      x+2, y+110  , COLOR_ORANGE, NULL, NULL);
            break;
        case DIALOG_LOGIN_WARNING_NAME_WRONG:
            StringBlt(ScreenSurface, &SystemFont, "Name is too short - it must be two chars or longer!",
                      x+1, y+111  , COLOR_BLACK, NULL, NULL);
            StringBlt(ScreenSurface, &SystemFont, "Name is too short - it must be two chars or longer!",
                      x+2, y+110  , COLOR_ORANGE, NULL, NULL);
            break;
        case DIALOG_LOGIN_WARNING_PWD_WRONG:
            StringBlt(ScreenSurface, &SystemFont, "Password is illegal or does not match!",
                      x+1, y+111  , COLOR_BLACK, NULL, NULL);
            StringBlt(ScreenSurface, &SystemFont, "Password is illegal or does not match!",
                      x+2, y+110  , COLOR_ORANGE, NULL, NULL);
            break;
        case DIALOG_LOGIN_WARNING_PWD_SHORT:
            StringBlt(ScreenSurface, &SystemFont, "Password is too short - it must be six chars or longer!",
                      x+1, y+111  , COLOR_BLACK, NULL, NULL);
            StringBlt(ScreenSurface, &SystemFont, "Password is too short - it must be six chars or longer!",
                      x+2, y+110  , COLOR_ORANGE, NULL, NULL);
            break;
        case DIALOG_LOGIN_WARNING_PWD_NAME:
            StringBlt(ScreenSurface, &SystemFont, "Password can't be same as character name!!!",
                      x+1, y+111  , COLOR_BLACK, NULL, NULL);
            StringBlt(ScreenSurface, &SystemFont, "Password can't be same as character name!!!",
                      x+2, y+110  , COLOR_ORANGE, NULL, NULL);
            break;
    }

    y += 157;
    if (GameStatusLogin) /* Login */
    {
        StringBlt(ScreenSurface, &SystemFont, "REMEMBER: You must have first ~created a character~ to login!",
                  x-10, y+1  , COLOR_BLACK, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont, "REMEMBER: You must have first ~created a character~ to login!",
                  x-11, y  , COLOR_WHITE, NULL, NULL);
        y+=12;
        StringBlt(ScreenSurface, &SystemFont, "1.) Enter the ~name~ of your character and press RETURN.",
                  x-10, y+1  , COLOR_BLACK, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont, "1.) Enter the ~name~ of your character and press RETURN.",
                  x-11, y  , COLOR_WHITE, NULL, NULL);
        y+=12;
        StringBlt(ScreenSurface, &SystemFont, "2.) Then enter the ~password~ of your character.",
                  x-10, y+1  , COLOR_BLACK, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont, "2.) Then enter the ~password~ of your character.",
                  x-11, y  , COLOR_WHITE, NULL, NULL);
    }
    else /* Create Char */
    {
        StringBlt(ScreenSurface, &SystemFont, "1.) Enter a ~name~ WITHOUT any numbers and special chars. Press RETURN.",
                  x-10, y+1  , COLOR_BLACK, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont, "1.) Enter a ~name~ WITHOUT any numbers and special chars. Press RETURN.",
                  x-11, y  , COLOR_WHITE, NULL, NULL);
        y+=12;
        StringBlt(ScreenSurface, &SystemFont, "2.) Enter a ~password~ WITH at least one special chars. Press RETURN." ,
                  x-10  , y+1  , COLOR_BLACK, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont, "2.) Enter a ~password~ WITH at least one special chars. Press RETURN." ,
                  x-11  , y  , COLOR_WHITE, NULL, NULL);
        y+=12;
        StringBlt(ScreenSurface, &SystemFont, "3.) Enter the ~password~ again to ~verify~ it. Press RETURN." ,
                  x-10  , y+1  , COLOR_BLACK, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont, "3.) Enter the ~password~ again to ~verify~ it. Press RETURN." ,
                  x-11  , y  , COLOR_WHITE, NULL, NULL);
    }

}


/******************************************************************
 show login: select-server part.
******************************************************************/
void show_meta_server(_server *node, int metaserver_start, int metaserver_sel)
{
    int         x, y, i;
    char        buf[1024];
    SDL_Rect    rec_name;
    SDL_Rect    rec_desc;
    SDL_Rect    box;
    int         mx, my, mb;
    Boolean     mainserverdown=TRUE;

    mb = SDL_GetMouseState(&mx, &my);
    /* background */
    /*  x= Screensize.x/2-Bitmaps[BITMAP_DIALOG_BG]->bitmap->w/2;*/
    x = 25;
    y = Screensize.y / 2 - Bitmaps[BITMAP_DIALOG_BG]->bitmap->h / 2;
    sprite_blt(Bitmaps[BITMAP_DIALOG_BG], x, y, NULL, NULL);
    sprite_blt(Bitmaps[BITMAP_LOGO270], x + 20, y + 85, NULL, NULL);
    sprite_blt(Bitmaps[BITMAP_DIALOG_TITLE_LOGIN], x + 250 - Bitmaps[BITMAP_DIALOG_TITLE_LOGIN]->bitmap->w / 2, y + 20,
               NULL, NULL);
    /*  add_close_button(x, y, MENU_LOGIN); */

    rec_name.w = 272;
    rec_desc.w = 325;

    box.x = x + 133;
    box.y = y + TXT_Y_START + 1;
    box.w = 329;
    box.h = 12;

    /* frame for selection field */
    draw_frame(box.x - 1, box.y + 11, box.w + 1, 313);
    /* we should prepare for this the SystemFontOut */
    StringBlt(ScreenSurface, &SystemFont, "Servers", x + TXT_START_NAME + 1, y + TXT_Y_START - 1, COLOR_BLACK, NULL,
              NULL);
    StringBlt(ScreenSurface, &SystemFont, "Servers", x + TXT_START_NAME, y + TXT_Y_START - 2, COLOR_WHITE, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, "Players", x + 416, y + TXT_Y_START - 1, COLOR_BLACK, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, "Players", x + 415, y + TXT_Y_START - 2, COLOR_WHITE, NULL, NULL);

    sprintf(buf, "use cursors ~%c%c~ to select server                                  press ~RETURN~ to connect",
            ASCII_UP, ASCII_DOWN);
    StringBlt(ScreenSurface, &SystemFont, buf, x + 140, y + 410, COLOR_WHITE, NULL, NULL);

    for (i = 0; i < OPTWIN_MAX_OPT; i++)
    {
        box.y += 12;
        if (i & 1)
            SDL_FillRect(ScreenSurface, &box, skindef.rowcolor[0]);
        else
            SDL_FillRect(ScreenSurface, &box, skindef.rowcolor[1]);
    }

    for (i = 0; node && i < metaserver_start; i++)
        node = node->next;

    for (i = 0; node && i < MAXMETAWINDOW; i++)
    {
        if (i == metaserver_sel - metaserver_start)
        {
            int o   = 0;
            if (!node->desc4[0])
                o += 5; if (!node->desc3[0])
                o += 5;
            if (!node->desc2[0])
                o += 5; if (!node->desc1[0])
                o += 5;
            sprintf(buf, "version %s", node->version);
            StringBlt(ScreenSurface, &SystemFont, buf, x + 160, y + 418 + o, COLOR_BLACK, NULL, NULL);
            StringBlt(ScreenSurface, &SystemFont, buf, x + 159, y + 417 + o, COLOR_WHITE, NULL, NULL);
            StringBlt(ScreenSurface, &SystemFont, node->desc1, x + 160, y + 431 + o, COLOR_BLACK, &rec_desc, NULL);
            StringBlt(ScreenSurface, &SystemFont, node->desc1, x + 159, y + 430 + o, COLOR_HGOLD, &rec_desc, NULL);
            StringBlt(ScreenSurface, &SystemFont, node->desc2, x + 160, y + 442 + o, COLOR_BLACK, &rec_desc, NULL);
            StringBlt(ScreenSurface, &SystemFont, node->desc2, x + 159, y + 441 + o, COLOR_HGOLD, &rec_desc, NULL);
            StringBlt(ScreenSurface, &SystemFont, node->desc3, x + 160, y + 453 + o, COLOR_BLACK, &rec_desc, NULL);
            StringBlt(ScreenSurface, &SystemFont, node->desc3, x + 159, y + 452 + o, COLOR_HGOLD, &rec_desc, NULL);
            /* hm, 3 descr. are far enough */
            /*StringBlt(ScreenSurface,&SystemFont, node->desc1, x+150, y+464+o, COLOR_HGOLD, &rec_desc,NULL);*/
            box.y = y + TXT_Y_START + 13 + i * 12;
            SDL_FillRect(ScreenSurface, &box, sdl_blue1);
        }
        StringBlt(ScreenSurface, &SystemFont, node->nameip, x + 137, y + 94 + i * 12, COLOR_WHITE, &rec_name, NULL);
        if (!strcmp(node->nameip,"daimonin.game-server.cc"))
            mainserverdown=FALSE;
        if (node->player >= 0)
            sprintf(buf, "%d", node->player);
        else
            sprintf(buf, "-");
        StringBlt(ScreenSurface, &SystemFont, buf, x + 416, y + 94 + i * 12, COLOR_WHITE, NULL, NULL);
        node = node->next;
    }
    if (mainserverdown && (i>0))
    {
        StringBlt(ScreenSurface, &SystemFont, "Main Server is temporarily down!", x + 137, y + 94 + ((i+1) * 12), COLOR_RED, &rec_name, NULL);
    }

}
