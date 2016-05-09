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

    The author can be reached via e-mail to info@daimonin.org
*/

#include "include.h"

extern char d_ServerName[2048];
extern int  d_ServerPort;
static int  get_action_keycode, drop_action_keycode; /* thats the key for G'et command from keybind */
static int  menuRepeatKey   = -1;
       int  old_mouse_y = 0;
button_status global_buttons;

typedef struct _keys
{
    uint8         pressed; /*true: key is pressed*/
    uint32          time; /*tick time last repeat is initiated*/
}
_keys;
static _keys    keys[MAX_KEYS];

_key_macro      defkey_macro[]          =
    {
        {"?M_SOUTHWEST",    "southwest",        KEYFUNC_MOVE,  1, MENU_NO
        },
        {"?M_SOUTH",          "south",            KEYFUNC_MOVE,  2, MENU_NO},
        {"?M_SOUTHEAST",    "southeast",        KEYFUNC_MOVE,  3, MENU_NO},
        {"?M_WEST",           "west",                 KEYFUNC_MOVE,  4, MENU_NO},
        {"?M_STAY",           "stay",                 KEYFUNC_MOVE,  5, MENU_NO},
        {"?M_EAST",           "east",                 KEYFUNC_MOVE,  6, MENU_NO},
        {"?M_NORTHWEST",    "northwest",        KEYFUNC_MOVE,  7, MENU_NO},
        {"?M_NORTH",          "north",            KEYFUNC_MOVE,  8, MENU_NO},
        {"?M_NORTHEAST",    "northeast",        KEYFUNC_MOVE,  9, MENU_NO},
        {"?M_RUN",            "run",                  KEYFUNC_RUN,   0, MENU_NO},
        {"?M_CONSOLE",      "console",          KEYFUNC_CONSOLE,0, MENU_NO},
        {"?M_UP",               "up",                   KEYFUNC_CURSOR,0, MENU_NO},
        {"?M_DOWN",           "down",                 KEYFUNC_CURSOR,1, MENU_NO},
        {"?M_LEFT",           "left",             KEYFUNC_CURSOR,2, MENU_NO},
        {"?M_RIGHT",          "right",            KEYFUNC_CURSOR,3, MENU_NO},
        {"?M_RANGE",          "cycle fire mode forwards",     KEYFUNC_RANGE,0, MENU_NO},
        {"?M_RANGE_BACK",     "cycle fire mode backwards",    KEYFUNC_RANGE_BACK,0, MENU_NO},
        {"?M_RANGE_BOW",      "fire mode: bow",     KEYFUNC_RANGE_SELECT,FIRE_MODE_BOW, MENU_NO},
        {"?M_RANGE_SPELL",    "fire mode: spell",   KEYFUNC_RANGE_SELECT,FIRE_MODE_SPELL, MENU_NO},
        {"?M_RANGE_SKILL",    "fire mode: skill",   KEYFUNC_RANGE_SELECT,FIRE_MODE_SKILL, MENU_NO},
        {"?M_APPLY",          "apply <tag>",      KEYFUNC_APPLY,0,  MENU_NO},
        {"?M_EXAMINE",      "examine <tag>",    KEYFUNC_EXAMINE,0,  MENU_NO},
        {"?M_DROP",           "drop <tag>",       KEYFUNC_DROP,0, MENU_NO},
        {"?M_GET",            "get <tag>",        KEYFUNC_GET,0,  MENU_NO},
        {"?M_LOCK",           "lock <tag>",       KEYFUNC_LOCK,0, MENU_NO},
        {"?M_MARK",           "mark<tag>",        KEYFUNC_MARK,0, MENU_NO},
        {"?M_STATUS",           "status",           KEYFUNC_STATUS,       0, MENU_ALL},
        {"?M_OPTION",           "option",           KEYFUNC_OPTION,       0, MENU_ALL},
        {"?M_KEYBIND",      "key bind",         KEYFUNC_KEYBIND,      0, MENU_ALL},
        {"?M_SKILL_LIST",   "skill list",       KEYFUNC_SKILL,        0, MENU_ALL},
        {"?M_SPELL_LIST",   "spell list",       KEYFUNC_SPELL,        0, MENU_ALL},
        {"?M_PAGEUP",           "scroll up",        KEYFUNC_PAGEUP,       0, MENU_NO},
        {"?M_PAGEDOWN",     "scroll down",      KEYFUNC_PAGEDOWN,     0, MENU_NO},
        {"?M_LAYER0",           "l0",               KEYFUNC_LAYER0,       0, MENU_NO},
        {"?M_LAYER1",           "l1",               KEYFUNC_LAYER1,       0, MENU_NO},
        {"?M_LAYER2",           "l2",               KEYFUNC_LAYER2,       0, MENU_NO},
        {"?M_LAYER3",           "l3",               KEYFUNC_LAYER3,       0, MENU_NO},
        {"?M_HELP",             "show help",        KEYFUNC_HELP,         0, MENU_NO},
        {"?M_PAGEUP_TOP",     "scroll up",        KEYFUNC_PAGEUP_TOP,   0, MENU_NO},
        {"?M_PAGEDOWN_TOP", "scroll down",      KEYFUNC_PAGEDOWN_TOP, 0, MENU_NO},
        {"?M_TARGET_ENEMY", "/target enemy",    KEYFUNC_TARGET_ENEMY, 0, MENU_NO},
        {"?M_TARGET_FRIEND","/target friend",   KEYFUNC_TARGET_FRIEND,0, MENU_NO},
        {"?M_TARGET_SELF",  "/target self",     KEYFUNC_TARGET_SELF,  0, MENU_NO},
        {"?M_COMBAT_TOGGLE","/combat",          KEYFUNC_COMBAT,       0, MENU_NO},
        {"?M_SCREENTOGGLE","fstoggle",          KEYFUNC_SCREENTOGGLE,       0, MENU_NO},
    };
#define DEFAULT_KEYMAP_MACROS (sizeof(defkey_macro)/sizeof(struct _key_macro))

/* Magic console macro: when this is found at the beginning of a user defined macro, then
 * what follows this macro will be put in the input console ready to be edited
 */
char            macro_magic_console[]   = "?M_MCON";

int             KeyScanFlag; /* for debug/alpha , remove later */
int             cursor_type             = 0;
#define KEY_REPEAT_TIME 35
#define KEY_REPEAT_TIME_INIT 175
static Uint32   menuRepeatTicks = 0, menuRepeatTime = KEY_REPEAT_TIME_INIT;

/* the state of the mouse buttons */
uint32 MouseState = IDLE;
/* reset after each pass through the main loop */
int MouseEvent = 0; /* do not set to IDLE, EVER */
int itemExamined = 0;


/* cmds for fire/move/run - used from move_keys()*/
#ifdef DEBUG_TEXT
static char    *directions_name[10]     =
    {
        "null", "southwest", "south", "southeast", "west", "stay", "east", "northwest", "north", "northeast"
    };
#endif
static char    *directionsrun[10]       =
    {
        "/run 0", "/run 6", "/run 5", "/run 4", "/run 7",\
        "/run 5", "/run 3", "/run 8", "/run 1", "/run 2"
    };

static int      key_event(SDL_KeyboardEvent *key);
static void     key_string_event(SDL_KeyboardEvent *key);
static uint8  check_macro_keys(char *text);
static void     move_keys(int num);
static void     key_repeat(void);
static void     cursor_keys(int num);
int             key_meta_menu(SDL_KeyboardEvent *key);
void            key_connection_event(SDL_KeyboardEvent *key);
static void     check_esc_menu_keys(int key);
void            check_menu_keys(int menu, int key);


void init_keys(void)
{
    register int i;

    for (i = 0; i < MAX_KEYS; i++)
        keys[i].time = 0;
    reset_keys();
}

void reset_keys(void)
{
    register int i;

    SDL_EnableKeyRepeat(0, SDL_DEFAULT_REPEAT_INTERVAL);
    reset_input_mode();
    InputStringFlag = 0;
    InputStringEndFlag = 0;
    InputStringEscFlag = 0;

    for (i = 0; i < MAX_KEYS; i++)
        keys[i].pressed = 0;
}

/******************************************************************
 x: mouse x-pos ; y: mouse y-pos
 ret: 0  if mousepointer is in the game-field.
     -1 if mousepointer is in a menue-field.
******************************************************************/
int mouseInPlayfield(int x, int y)
{
    //we simply realc the mousevalues

    x=(int)(x/(options.zoom/100.0f));
    y=(int)(y/(options.zoom/100.0f));

    x = x - options.mapstart_x -6;
    y = y - options.mapstart_y - 55;

    if (x < 408)
    {
        if ((y < 200) && (y + y + x > 400))
            return -1; /* upper left */
        if ((y >= 200) && (y + y - x < 400))
            return -1; /* lower left */
    }
    else
    {
        x -= 408;
        if ((y < 200) && (y + y > x))
            return -1; /* upper right */
        if ((y >= 200) && (y + y + x < 845))
            return -1; /* lower right */
    }
    return 0;
}

/******************************************************************
    src:  (if != DRAG_GET_STATUS) set actual dragging source.
    item: (if != NULL) set actual dragging item.
    ret:  the actual dragging source.
******************************************************************/
int draggingInvItem(int src)
{
    static int  drag_src    = DRAG_NONE;

    if (src != DRAG_GET_STATUS)
        drag_src = src;
    return drag_src;
}

/******************************************************************
 wait for user to input a numer.
******************************************************************/
static void mouse_InputNumber()
{
    int mx=0, my=0;

    static int  delta   = 0;
    static int  timeVal = 1;
    int         x, y;

    if (!(SDL_GetMouseState(&x, &y) & SDL_BUTTON(SDL_BUTTON_LEFT)))
    {
        timeVal = 1;
        delta = 0;
        return;
    }
    mx = x - cur_widget[IN_NUMBER_ID].x1;
    my = y - cur_widget[IN_NUMBER_ID].y1;

    if (mx <230 || mx> 237 || my < 5 || delta++ & 15)
        return;
    if (my > 13)
    {
        /* + */
        x = atoi(InputString) + timeVal;
        if (x > cpl.nrof)
            x = cpl.nrof;
    }
    else
    {
        /* - */
        x = atoi(InputString) - timeVal;
        if (x < 1)
            x = 1;
    }
    sprintf(InputString, "%d", x);
    InputCount = strlen(InputString);
    timeVal += (timeVal / 8) + 1;
}

/******************************************************************
 move our hero with mouse.
******************************************************************/
static void mouse_moveHero()
{
#define MY_POS 8
    int         x, y, tx, ty;
    static int  delta   = 0;

    if (0)
        return; /* disable until we have smooth moving - people think this IS the real mouse moving */
    if (delta++ & 7)
        return; /* dont move to fast */
    SDL_GetMouseState(&x, &y);
    if (get_widget_owner(x,y)!=-1)
        return;
    if (draggingInvItem(DRAG_GET_STATUS))
        return; /* still dragging an item */
    if (cpl.input_mode == INPUT_MODE_NUMBER)
        return;
    if (cpl.menustatus != MENU_NO)
        return;
    if (textwin_flags & (TW_RESIZE + TW_SCROLL))
        return; /* textwin events */
    if (!(SDL_GetMouseState(&x, &y) & SDL_BUTTON(SDL_BUTTON_LEFT)))
    {
        delta = 0;
        return;
    }
    /* textwin has high priority, so dont move if playfield is overlapping */
    /* widget function handle this... */
//    if ((options.use_TextwinSplit) && x > 538 && y > 560 - (txtwin[TW_MSG].size + txtwin[TW_CHAT].size) * 10)
//        return;

    if (get_tile_position(x, y, &tx, &ty))
        return;
    if (tx == MY_POS)
    {
        if (ty == MY_POS)
            process_macro_keys(KEYFUNC_MOVE, 5);
        else if (ty > MY_POS)
            process_macro_keys(KEYFUNC_MOVE, 2);
        else if (ty < MY_POS)
            process_macro_keys(KEYFUNC_MOVE, 8);
    }
    else if (tx < MY_POS)
    {
        if (ty == MY_POS)
            process_macro_keys(KEYFUNC_MOVE, 4);
        else if (ty > MY_POS)
            process_macro_keys(KEYFUNC_MOVE, 1);
        else if (ty < MY_POS)
            process_macro_keys(KEYFUNC_MOVE, 7);
    }
    else
    {
        /* (x > MY_POS) */
        if (ty == MY_POS)
            process_macro_keys(KEYFUNC_MOVE, 6);
        if (ty < MY_POS)
            process_macro_keys(KEYFUNC_MOVE, 9);
        if (ty > MY_POS)
            process_macro_keys(KEYFUNC_MOVE, 3);
    }
#undef MY_POS
}


static int key_login_select_menu(SDL_KeyboardEvent *key)
{
    if (key->type == SDL_KEYDOWN)
    {
        switch (key->keysym.sym)
        {
        case SDLK_UP:
            sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0, 100);
            GameStatusSelect == GAME_STATUS_LOGIN_ACCOUNT?(GameStatusSelect=GAME_STATUS_LOGIN_NEW):(GameStatusSelect=GAME_STATUS_LOGIN_ACCOUNT);
            break;

        case SDLK_DOWN:
            sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0, 100);
            GameStatusSelect==GAME_STATUS_LOGIN_ACCOUNT?(GameStatusSelect=GAME_STATUS_LOGIN_NEW):(GameStatusSelect=GAME_STATUS_LOGIN_ACCOUNT);
            break;

        case SDLK_RETURN:
            sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0, 100);
            dialog_login_warning_level = DIALOG_LOGIN_WARNING_NONE;
            open_input_mode(MAX_ACCOUNT_NAME);
            LoginInputStep = LOGIN_STEP_NAME;
            GameStatus = GameStatusSelect; /* create account or direct login */
            break;

        case SDLK_ESCAPE:
            sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0, 100);
            SOCKET_CloseClientSocket(&csocket);
            GameStatus = GAME_STATUS_INIT;
            return(0);

        default:
            break;
        }
    }
    return(0);
}

/* key input for the account screen (character select / create / delete) */
static int key_account_menu(SDL_KeyboardEvent *key)
{
    if (key->type == SDL_KEYDOWN)
    {
        switch (key->keysym.sym)
        {
        case SDLK_UP:
            sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0, 100);
            if(account.count && --account.selected < 0)
                account.selected = account.count-1;
            break;

        case SDLK_DOWN:
            sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0, 100);
            if(account.count && ++account.selected > account.count-1)
                    account.selected = 0;
            break;

        case SDLK_c:
            if(account.count < ACCOUNT_MAX_PLAYER)
            {
                sound_play_effect(SOUNDTYPE_CLIENT, SOUND_PAGE, 0, 0, 100);
                GameStatus = GAME_STATUS_ACCOUNT_CHAR_CREATE;
            }
            break;

        case SDLK_d:
            if(account.count)
            {
                sound_play_effect(SOUNDTYPE_CLIENT, SOUND_PAGE, 0, 0, 100);
                GameStatus = GAME_STATUS_ACCOUNT_CHAR_DEL;
                /* the player must write "delete" , then we allow deletion of the selected char name */
                reset_input_mode();
                InputStringFlag=1;
                InputStringEndFlag=0;
                open_input_mode(MAX_PLAYER_NAME);
                cpl.menustatus = MENU_NO;
            }
            break;

        case SDLK_RETURN:
            sound_play_effect(SOUNDTYPE_CLIENT, SOUND_PAGE, 0, 0, 100);

            /* put client in wait modus... 2 choices:
            * a.) login fails - we will get a BINARY_CMD_ACCOUNT with error msg
            * b.) server is adding <name> - we get a BINARY_CMD_PLAYER and then regular playing data
            * Option a.) will move us back to character selection with an error message in the status
            */
            if(account.count)
            {
                /* tell server that we want play with this char */
                SendAddMe(account.name[account.selected]);
                sprintf(cpl.name, "%s", account.name[account.selected]);
                GameStatus = GAME_STATUS_WAITFORPLAY;

                /* some sanity settings */
                clear_map();
                // This appears to be INsane in fact. We always blank the
                // character name when we log in?
                // -- Smacky 20100616
                //cpl.name[0] = 0;
                map_udate_flag = 2;
                map_transfer_flag = 1;
            }
            break;

        case SDLK_ESCAPE:
            sound_play_effect(SOUNDTYPE_CLIENT, SOUND_PAGE, 0, 0, 100);
            SOCKET_CloseClientSocket(&csocket);
            GameStatus = GAME_STATUS_INIT;
            return(0);

        default:
            break;
        }
    }
    return(0);
}

int Event_PollInputDevice(void)
{
    SDL_Event       event;
    int             x, y, done = 0;
    static int      active_scrollbar    = 0;

    static Uint32   Ticks               = 0;

    itemExamined        = 0; /* only print text once per dnd */

    if ((SDL_GetTicks() - Ticks > 10) || !Ticks)
    {
        Ticks = SDL_GetTicks();
        if (GameStatus >= GAME_STATUS_PLAY)
        {
            if (InputStringFlag && cpl.input_mode == INPUT_MODE_NUMBER)
                mouse_InputNumber();
            else if (!active_scrollbar && !cursor_type)
                mouse_moveHero();
        }
    }


    global_buttons.valid = -1;
    while (SDL_PollEvent(&event))
    {
        x = global_buttons.mx=event.motion.x;
        y = global_buttons.my=event.motion.y;

        mb_clicked = 0;
        switch (event.type)
        {
        case SDL_MOUSEBUTTONUP:

            /* get the mouse state and set an event (event removed at end of main loop) */
            if(event.button.button == SDL_BUTTON_LEFT)
                MouseEvent = LB_UP;
            else if(event.button.button == SDL_BUTTON_MIDDLE)
                MouseEvent = MB_UP;
            else if(event.button.button == SDL_BUTTON_RIGHT)
                MouseEvent = RB_UP;
            else
                MouseEvent = IDLE;

            /* no button is down */
            MouseState = IDLE;

            cursor_type = 0;


            global_buttons.mx_up = x;
            global_buttons.my_up = y;
            global_buttons.down = -1;
            global_buttons.click = 1;
            global_buttons.valid = 0;
            if (GameStatus < GAME_STATUS_PLAY)
                break;
            mb_clicked = 0;

            cursor_type = 0;
            active_scrollbar = 0;

            if (cpl.menustatus == MENU_BOOK)
            {
                /* no button - we can do a lazy check */
                if ( global_buttons.mx_up >= global_book_data.x &&
                        global_buttons.my_up >= global_book_data.y &&
                        global_buttons.mx_up <= global_book_data.x+global_book_data.xlen &&
                        global_buttons.my_up <= global_book_data.y+global_book_data.ylen)
                {
                    global_buttons.click = -1;

                    if (global_buttons.mx_up >= global_book_data.x+(global_book_data.xlen/2))
                        check_menu_keys(MENU_BOOK, SDLK_RIGHT);
                    else
                        check_menu_keys(MENU_BOOK, SDLK_LEFT);
                }
            }

            /* widget has higher prio than anything below, exept menus
             * so break is we had a widget event
             */
            if (widget_event_mouseup(x,y, &event))
            {
                /* NOTE: place here special handlings that have to be done, even if a widget owns it */

                draggingInvItem(DRAG_NONE); /* sanity handling */
                itemExamined = 0; /* ready for next item */

                break;
            };

            if (InputStringFlag && cpl.input_mode == INPUT_MODE_NUMBER)
                break;

            /***********************
              drag and drop events
            ************************/
            /* Only drop to ground has to be handled, the rest do the widget handlers */
            if (draggingInvItem(DRAG_GET_STATUS) > DRAG_IWIN_BELOW)
            {
                /* KEYFUNC_APPLY and KEYFUNC_DROP works only if cpl.inventory_win = IWIN_INV. The tag must
                            be placed in cpl.win_inv_tag. So we do this and after DnD we restore the old values. */
                int   old_inv_win = cpl.inventory_win;
                int   old_inv_tag = cpl.win_inv_tag;
                cpl.inventory_win = IWIN_INV;

                /* drop to ground */
                if (mouseInPlayfield(x, y))
                {
                    if (draggingInvItem(DRAG_GET_STATUS) != DRAG_QUICKSLOT_SPELL)
                        process_macro_keys(KEYFUNC_DROP, 0);
                }

                cpl.inventory_win = old_inv_win;
                cpl.win_inv_tag = old_inv_tag;
            }
            draggingInvItem(DRAG_NONE);
            itemExamined = 0; /* ready for next item */
            break;

        case SDL_MOUSEMOTION:
            mb_clicked = 0;
            if (GameStatus < GAME_STATUS_PLAY)
                break;

            x_custom_cursor = x;
            y_custom_cursor = y;
            if (cpl.menustatus == MENU_NPC)
            {
                if (event.button.button ==0)
                {
                    gui_npc_mousemove(event.motion.x, event.motion.y);
                    break;
                }

            }

//            textwin_event(TW_CHECK_MOVE, &event);

            /* scrollbar-sliders We have to have it before the widgets cause of the menu*/
            if (event.button.button == SDL_BUTTON_LEFT &&
                !draggingInvItem(DRAG_GET_STATUS))
            {
                /* NPC_GUI Slider */
                if (active_scrollbar == 2 ||
                    (cpl.menustatus == MENU_NPC &&
                     y >= 136 + Screensize.yoff &&
                     y <= 474 + Screensize.yoff &&
                     x >= 561 &&
                     x <= 568))
                {
                    active_scrollbar = 2;

                    if (old_mouse_y - y > 0)
                    {
                        gui_npc->yoff += 12;

                        if (gui_npc->yoff > 0)
                        {
                            gui_npc->yoff = 0;
                        }

                        if (gui_npc->yoff < GUI_NPC_HEIGHT - gui_npc->height)
                        {
                            gui_npc->yoff = GUI_NPC_HEIGHT - gui_npc->height;
                        }

                        if (gui_npc->yoff > 0)
                        {
                            gui_npc->yoff = 0;
                        }
                    }
                    else if (old_mouse_y - y < 0)
                    {
                        gui_npc->yoff -= 12;

                        if (gui_npc->yoff < GUI_NPC_HEIGHT - gui_npc->height)
                        {
                            gui_npc->yoff = GUI_NPC_HEIGHT - gui_npc->height;
                        }

                        if (gui_npc->yoff < GUI_NPC_HEIGHT - gui_npc->height)
                        {
                            gui_npc->yoff = GUI_NPC_HEIGHT - gui_npc->height;
                        }

                        if (gui_npc->yoff > 0)
                        {
                            gui_npc->yoff = 0;
                        }
                    }
                    break;
                }
            }

            /* We have to break now when menu is active -> Menu higher prio than any widget! */
            if (cpl.menustatus != MENU_NO)
                break;

            if (widget_event_mousemv(x,y, &event))
            {
                /* NOTE: place here special handlings that have to be done, even if a widget owns it */
                break;
            }
            /*
            {
            char tz[40];
            sprintf(tz,"x: %d , y: %d", x, y);
            textwin_showstring(COLOR_BLUE | NDI_PLAYER, "%s", tz);
            textwin_showstring(COLOR_BLUE, "%s", tz);
            }
            */


            /* examine an item */
            /*
                  if ((cpl.inventory_win == IWIN_INV) && y > 85 && y < 120 && x < 140){
                          if (!itemExamined){
                              check_keys(SDLK_e);
                              itemExamined = 1;
                    }
                          break;
                      }*/
            break;

        case SDL_MOUSEBUTTONDOWN:

            /* get the mouse state and set an event (event removed at end of main loop) */
            if(event.button.button == SDL_BUTTON_LEFT)
                MouseEvent = MouseState = LB_DN;
            else if(event.button.button == SDL_BUTTON_MIDDLE)
                MouseEvent = MouseState = MB_DN;
            else if(event.button.button == SDL_BUTTON_RIGHT)
                MouseEvent = MouseState = RB_DN;
            else
                MouseEvent = MouseState = IDLE;

            global_buttons.mx_down = x;
            global_buttons.my_down = y;
            global_buttons.down = 1;
            global_buttons.click = -1;
            global_buttons.valid = -1;
            global_buttons.mx_up = -1;
            global_buttons.my_up = -1;
            mb_clicked = 1;
            if (GameStatus < GAME_STATUS_PLAY)
                break;

            if (cpl.menustatus == MENU_NPC)
            {
                if (event.button.button ==4 || event.button.button ==5 || event.button.button == SDL_BUTTON_LEFT)
                {
                    gui_npc_mouseclick(&event);
                    break;
                }

            }


            /********************************************************
              beyond here only when no menu is active.
            *********************************************************/
            if (cpl.menustatus != MENU_NO)
                break;



            /********************************
             * Widget System
             ********************************/
            if (widget_event_mousedn(x,y, &event))
            {
                /* NOTE: place here special handlings that have to be done, even if a widget owns it */
                break;
            }


            /***********************
              mouse in Play-field
            ************************/
            if (mouseInPlayfield(event.motion.x, event.motion.y))
            {
                /* Targetting */
                if ((SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_RIGHT)))
                {
                    int  tx, ty;
                    char tbuf[TINY_BUF];

                    cpl.inventory_win = IWIN_BELOW;
                    get_tile_position(x, y, &tx, &ty);
                    sprintf(tbuf, "/target !%d %d", tx - MAP_MAX_SIZE / 2, ty - MAP_MAX_SIZE / 2);
                    send_game_command(tbuf);
                }
                break;
            }
            /***********************
                  * mouse in Menue-field *
                  ************************/


            break; /* SDL_MOUSEBUTTONDOWN */

        case SDL_KEYUP:
            /* end of key-repeat */
            menuRepeatKey = -1;
            menuRepeatTime = (options.menu_repeat > 0) ? 70 / options.menu_repeat + 280 / options.menu_repeat : 0; // delay
            /* fall through (no break;) */

        case SDL_KEYDOWN:
            /* We *only* type a player name now in the creation screen.
             * or "Delete" in char creation.
             */
            if(GameStatus == GAME_STATUS_ACCOUNT_CHAR_NAME && InputStringFlag)
            {
                key_string_event(&event.key);
                break;
            }
            else if (GameStatus == GAME_STATUS_ACCOUNT_CHAR_RECLAIM && InputStringFlag)
            {
                key_string_event(&event.key);
                break;
            }
            else if (cpl.menustatus == MENU_NO && (!InputStringFlag || cpl.input_mode != INPUT_MODE_NUMBER))
            {
#ifdef DAI_DEVELOPMENT
                if (widget_mouse_event.owner > -1 && f_custom_cursor == MSCURSOR_MOVE && (event.key.keysym.sym == SDLK_DELETE || event.key.keysym.sym == SDLK_BACKSPACE))
                {
                    switch (widget_mouse_event.owner)
                    {
                        case 12: // PLAYERDOLL
                            if (!options.playerdoll) // Actually this shouldn't be necessary as the widget should already be hidden, but JIC
                            {
                                sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0, MENU_SOUND_VOL);
                                cur_widget[widget_mouse_event.owner].show = 0;
                                f_custom_cursor = 0;
                                SDL_ShowCursor(1);
                            }
                            else
                                sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
                            break;
                        case 17: // MAININV
                        case 19: // CONSOLE
                        case 20: // NUMBER
                            sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
                            break;
                        case 21: // STATOMETER
                            if (!options.statsupdate) // Actually this shouldn't be necessary as the widget should already be hidden, but JIC
                            {
                                sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0, MENU_SOUND_VOL);
                                cur_widget[widget_mouse_event.owner].show = 0;
                                f_custom_cursor = 0;
                                SDL_ShowCursor(1);
                            }
                            else
                                sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
                            break;
                        default:
                            sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0, MENU_SOUND_VOL);
                            cur_widget[widget_mouse_event.owner].show = 0;
                            f_custom_cursor = 0;
                            SDL_ShowCursor(1);
                    }
                }
#endif
                if (event.key.keysym.mod & KMOD_SHIFT)
                    cpl.inventory_win = IWIN_INV;
                else
                    cpl.inventory_win = IWIN_BELOW;
                if (event.key.keysym.mod & KMOD_RCTRL
                        || event.key.keysym.mod & KMOD_LCTRL
                        || event.key.keysym.mod & KMOD_CTRL)
                    cpl.fire_on = 1;
                else
                    cpl.fire_on = 0;
            }

            if (InputStringFlag)
            {
                if (cpl.input_mode != INPUT_MODE_NUMBER)
                    cpl.inventory_win = IWIN_BELOW;
                key_string_event(&event.key);
            }
            else if (!InputStringEndFlag)
            {
                if (GameStatus <= GAME_STATUS_WAITLOOP)
                    done = key_meta_menu(&event.key);
                else if (GameStatus == GAME_STATUS_LOGIN_SELECT)
                    done = key_login_select_menu(&event.key);
                else if (GameStatus == GAME_STATUS_ACCOUNT)
                    done = key_account_menu(&event.key);
                else if (GameStatus == GAME_STATUS_PLAY || GameStatus == GAME_STATUS_ACCOUNT_CHAR_CREATE)
                    done = key_event(&event.key);
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
        old_mouse_y = y;
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


void key_connection_event(SDL_KeyboardEvent *key)
{
    if (key->type == SDL_KEYDOWN)
    {
        switch (key->keysym.sym)
        {
        case SDLK_ESCAPE:
            SOCKET_CloseClientSocket(&csocket);
            textwin_showstring(COLOR_RED, "connection closed. select new server.");
            break;

        default:
            break;
        }
    }
}


/* metaserver menu key */
int key_meta_menu(SDL_KeyboardEvent *key)
{
    if (key->type == SDL_KEYDOWN)
    {
        switch (key->keysym.sym)
        {
        case SDLK_UP:
            if (metaserver_sel)
            {
                metaserver_sel--;
                if (metaserver_start > metaserver_sel)
                    metaserver_start = metaserver_sel;
            }
            break;
        case SDLK_DOWN:
            if (metaserver_sel < metaserver_count - 1)
            {
                metaserver_sel++;
                if (metaserver_sel >= MAXMETAWINDOW)
                    metaserver_start = (metaserver_sel + 1) - MAXMETAWINDOW;
            }
            break;
        case SDLK_RETURN:
            get_meta_server_data(metaserver_sel, ServerName, &ServerPort);
            GameStatus = GAME_STATUS_STARTCONNECT;
            break;

        case SDLK_ESCAPE:
            return(1);

        default:
            break;
        }
    }
    return(0);
}

/* we get TEXT from keyboard. This is for console input */
static void key_string_event(SDL_KeyboardEvent *key)
{
    register int i;

    if (key->type == SDL_KEYDOWN)
    {
        switch (key->keysym.sym)
        {
        case SDLK_ESCAPE:
//            SDL_EnableKeyRepeat(0, SDL_DEFAULT_REPEAT_INTERVAL);
            InputStringEscFlag = 1;
            return;
            break;

        case SDLK_KP_ENTER:
            if (cpl.input_mode == INPUT_MODE_NPCDIALOG)
            {
                check_menu_keys(MENU_NPC, SDLK_KP_ENTER);
                break;
            }

        case SDLK_RETURN:
        case SDLK_TAB:
            if (key->keysym.sym != SDLK_TAB || GameStatus < GAME_STATUS_WAITFORPLAY)
            {
//                SDL_EnableKeyRepeat(0, SDL_DEFAULT_REPEAT_INTERVAL);
                InputStringFlag = 0;
                InputStringEndFlag = 1; /* mark that we've got something here */

                /* record this line in input history only if we are in console mode */
                if (cpl.input_mode == INPUT_MODE_CONSOLE)
                    textwin_addhistory(InputString);
            }
            break;

        /* We need to allow these keys to be used in input -- their precise
         * meaning depends on the next keystroke and is handled below.
         * TODO: SHIFT is allowed during login but has no effect. */
        case SDLK_RSHIFT:
        case SDLK_LSHIFT:
        case SDLK_RCTRL:
        case SDLK_LCTRL:
            break;

        case SDLK_BACKSPACE:
            /* erases the previous character or word if CTRL is pressed */
            if (InputCount && CurrentCursorPos)
            {
                register int ii;

                ii = CurrentCursorPos;  /* actual position of the cursor */
                i = ii - 1;               /* where we will end up, by default one character back */
                if (key->keysym.mod & KMOD_CTRL)
                {
                    while (InputString[i] == ' ' && i >= 0)
                        i--; /* jumps eventual whitespaces */
                    while (InputString[i] != ' ' && i >= 0)
                        i--; /* jumps a word */
                    i++; /* we end up at the beginning of the current word */
                }
                /* this loop copies even the terminating \0 of the buffer */
                while (ii <= InputCount)
                    InputString[i++] = InputString[ii++];
                CurrentCursorPos -= (ii - i) ;
                InputCount -= (ii - i);
            }
            break;

        case SDLK_LEFT:
            /* shifts a character or a word if CTRL is pressed */
            if (key->keysym.mod & KMOD_CTRL)
            {
                i = CurrentCursorPos - 1;
                while (InputString[i] == ' ' && i >= 0)
                    i--; /* jumps eventual whitespaces*/
                while (InputString[i] != ' ' && i >= 0)
                    i--; /* jumps a word */
                CurrentCursorPos = i + 1; /* places the cursor on the first letter of this word */
                break;
            }
            else if (CurrentCursorPos > 0)
                CurrentCursorPos--;
            break;

        case SDLK_RIGHT:
            /* shifts a character or a word if CTRL is pressed */
            if (key->keysym.mod & KMOD_CTRL)
            {
                i = CurrentCursorPos;
                while (InputString[i] == ' ' && i < InputCount)
                    i++; /* jumps eventual whitespaces*/
                while (InputString[i] != ' ' && i < InputCount)
                    i++; /* jumps a word */
                CurrentCursorPos = i; /* places the cursor right after the jumped word */
                break;
            }
            else if (CurrentCursorPos < InputCount)
                CurrentCursorPos++;
            break;

        case SDLK_UP:
            /* If we are in CONSOLE mode, let player scroll back the lines in history */
            if (cpl.input_mode == INPUT_MODE_CONSOLE
                    && HistoryPos < MAX_HISTORY_LINES
                    && InputHistory[HistoryPos + 1][0])
            {
                /* First history line is special, it records what we
                 * were writing before scrolling back the history; so,
                 * by returning back to zero, we can continue our
                 * editing where we left it
                 */
                if (HistoryPos == 0)
                    strncpy(InputHistory[0], InputString, InputCount);
                HistoryPos++;
                textwin_putstring(InputHistory[HistoryPos]);
            }

            /* If we are in INTERFACE mode, let player scroll back the
             * lines in history, but only those that start with "/talk"
             */
            if (cpl.input_mode == INPUT_MODE_NPCDIALOG
                    && HistoryPos < MAX_HISTORY_LINES)
            {
                int nextpos;
                SDL_Rect    box;
                if (HistoryPos == 0)
                {
                    strcpy(InputHistory[0], "/talk ");
                    strncpy(InputHistory[0]+6, InputString, InputCount);
                }

                for (nextpos = HistoryPos + 1; nextpos < MAX_HISTORY_LINES; nextpos++)
                {
                    if (strncmp(InputHistory[nextpos], "/talk ", 6) == 0)
                    {
                        HistoryPos = nextpos;
                        break;
                    }
                }

                strcpy(InputString, InputHistory[HistoryPos] + 6);
                InputCount =CurrentCursorPos = strlen(InputString);
                InputStringFlag = 1;

                box.x = gui_npc->startx + 95;
                box.y = gui_npc->starty + 449;
                box.h = 12;
                box.w = 180;

                SDL_FillRect(ScreenSurface, &box, 0);
                string_blt(ScreenSurface, &font_small, show_input_string(InputString, &font_small,box.w-10),box.x+5 ,box.y, COLOR_WHITE, NULL, NULL);
            }
            break;

        case SDLK_DOWN:
            /* If we are in CONSOLE mode, let player scroll forward the lines in history */
            if (cpl.input_mode == INPUT_MODE_CONSOLE && HistoryPos > 0)
            {
                HistoryPos--;
                textwin_putstring(InputHistory[HistoryPos]);
            }

            /* If we are in INTERFACE mode, let player scroll forward the
             * lines in history, but only those that start with "/talk"
             */
            if (cpl.input_mode == INPUT_MODE_NPCDIALOG
                    && HistoryPos > 0)
            {
                int nextpos;
                SDL_Rect    box;
                for (nextpos = HistoryPos-1; nextpos >= 0; nextpos--)
                {
                    if (strncmp(InputHistory[nextpos], "/talk ", 6) == 0)
                    {
                        HistoryPos = nextpos;
                        break;
                    }
                }
                strcpy(InputString, InputHistory[HistoryPos] + 6);
                InputCount =CurrentCursorPos = strlen(InputString);
                InputStringFlag = 1;
                box.x = gui_npc->startx + 95;
                box.y = gui_npc->starty + 449;
                box.h = 12;
                box.w = 180;
                SDL_FillRect(ScreenSurface, &box, 0);
                string_blt(ScreenSurface, &font_small, show_input_string(InputString, &font_small,box.w-10),box.x+5 ,box.y, COLOR_WHITE, NULL, NULL);
            }
            break;

        case SDLK_DELETE:
        {
            register int ii;

            ii = CurrentCursorPos;  /* actual position of the cursor */
            i = ii + 1;               /* where we will end up, by default one character ahead */
            if (ii == InputCount)
                break;
            if (key->keysym.mod & KMOD_CTRL)
            {
                while (InputString[i] == ' ' && i < InputCount)
                    i++; /* jumps eventual whitespaces */
                while (InputString[i] != ' ' && i < InputCount)
                    i++; /* jumps a word */
            }
            /* this loop copies even the terminating \0 of the buffer */
            while (i <= InputCount)
                InputString[ii++] = InputString[i++];

            InputCount -= (i - ii);
        }
        break;

        case SDLK_HOME:
            CurrentCursorPos = 0;
            break;

        case SDLK_END:
            CurrentCursorPos = InputCount;
            break;

        case SDLK_PAGEUP:
        case SDLK_PAGEDOWN:
        case SDLK_KP_DIVIDE:
        case SDLK_KP_MULTIPLY:
        case SDLK_KP_MINUS:
        case SDLK_KP_PLUS:
        case SDLK_KP1:
        case SDLK_KP2:
        case SDLK_KP3:
        case SDLK_KP4:
        case SDLK_KP5:
        case SDLK_KP6:
        case SDLK_KP7:
        case SDLK_KP8:
        case SDLK_KP9:
        case SDLK_KP0:
        case SDLK_KP_PERIOD:
            if (cpl.input_mode == INPUT_MODE_NPCDIALOG)
            {
                check_menu_keys(MENU_NPC, key->keysym.sym);
                break;
            }
            /* else drop through to default behaviour */

        default:
            /* if we are in number console mode, use GET as quick enter
             * mode - this is a very handy shortcut
             */
            if (cpl.input_mode == INPUT_MODE_NUMBER
                    && ((int)key->keysym.sym == get_action_keycode || (int)key->keysym.sym == drop_action_keycode))
            {
//                SDL_EnableKeyRepeat(0, SDL_DEFAULT_REPEAT_INTERVAL);
                InputStringFlag = 0;
                InputStringEndFlag = 1;/* mark that we got some here*/
            }

            /* now keyboard magic - transform a sym (kind of scancode)
             * to a layout code
             */
            if (InputCount < InputMax)
            {
                register char c;

                if ((key->keysym.unicode & 0xFF80) == 0)
                    c = key->keysym.unicode & 0x7F;
                c = key->keysym.unicode & 0xff;

                if (key->keysym.mod & KMOD_SHIFT)
                    c = toupper(c);

                /* These chars are never allowed. */
                if (c < 32 ||
                    c == ECC_STRONG ||
                    c == ECC_EMPHASIS ||
                    c == ECC_UNDERLINE ||
                    c == ECC_HYPERTEXT)
                {
                    c = 0;
                }
                else
                {
                    if (GameStatus >= GAME_STATUS_LOGIN_ACCOUNT && GameStatus <= GAME_STATUS_LOGIN_NEW)
                    {
                        switch (LoginInputStep)
                        {
                            case LOGIN_STEP_NAME:
                                /* The tolower below is purely visual -- the
                                 * server will force the correct case anyway. */
                                if (!account_char_valid(c))
                                {
                                    c = 0;
                                }
                                else
                                {
                                   c = tolower(c);
                                }

                                break;

                            case LOGIN_STEP_PASS1:
                            case LOGIN_STEP_PASS2:
                                if (!password_char_valid(c))
                                {
                                    c = 0;
                                }

                                break;

                            default:
                                c = 0;
                                break;
                        }
                    }
                    else if (GameStatus == GAME_STATUS_ACCOUNT_CHAR_NAME)
                    {
                        /* The toupeper/tolower below is purely visual -- the
                         * server will force the correct case anyway. */
                        if (!player_char_valid(c))
                        {
                            c = 0;
                        }
                        else if (CurrentCursorPos == 0)
                        {
                            c = toupper(c);
                        }
                        else
                        {
                           c = tolower(c);
                        }
                    }
                    else if (cpl.input_mode == INPUT_MODE_NUMBER)
                    {
                        /* Allow only numbers in number mode. */
                        if (c < '0' || c > '9')
                            c = 0;
                    }
                    else if (cpl.input_mode == INPUT_MODE_CONSOLE)
                    {
                        /* Allow full input in the console, including shifted
                         * chars, but note that /talk strings will be lowercased
                         * and normalised before being sent to the server. */
                        /* FIXME: Do that here? */
                    }
                    else if (cpl.input_mode == INPUT_MODE_NPCDIALOG)
                    {
                        /* Allow full input in an interface, including shifted
                         * chars, but note that this will be lowercased and
                         * normalised before being sent to the server. */
                        /* FIXME: Do that here? */
                    }
                }

                if (c == 0)
                    sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
                else
                {
                    for (i = InputCount; i >= CurrentCursorPos; i--)
                        InputString[i + 1] = InputString[i];
                    InputString[CurrentCursorPos++] = c;
                    InputString[++InputCount] = 0;
                }
            }
            break;
        }
        InputFirstKeyPress = 0;
    }
}

/* we have a key event */
int key_event(SDL_KeyboardEvent *key)
{
    if (GameStatus != GAME_STATUS_PLAY && GameStatus != GAME_STATUS_ACCOUNT_CHAR_CREATE)
        return 0;

    if (key->type == SDL_KEYUP)
    {
        if (KeyScanFlag)
        {
            textwin_showstring(COLOR_RED, "Scancode: %d", key->keysym.sym);
        }

        if (cpl.menustatus != MENU_NO)
        {
            keys[key->keysym.sym].pressed = 0;
        }
        else
        {
            keys[key->keysym.sym].pressed = 0;
            switch (key->keysym.sym)
            {
            case SDLK_LSHIFT:
            case SDLK_RSHIFT:
                cpl.inventory_win = IWIN_BELOW;
                break;
            case SDLK_LALT:
            case SDLK_RALT:
                send_game_command("/run_stop");
#ifdef DEBUG_TEXT
                textwin_showstring(COLOR_DGOLD, "run_stop");
#endif
                cpl.run_on = 0;
                break;
            case SDLK_RCTRL:
            case SDLK_LCTRL:
                cpl.fire_on = 0;
                break;

            default:
                break;
            }
        }
    }
    else if (key->type == SDL_KEYDOWN)
    {
        if (cpl.menustatus != MENU_NO)
        {
            /* we catch here the keybind key, when we insert a new macro there */
            if (cpl.menustatus == MENU_KEYBIND)
            {
                if (keybind_status == KEYBIND_STATUS_EDITKEY)
                {
                    if (key->keysym.sym != SDLK_ESCAPE)
                    {
                        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0, 100);
                        strcpy(bindkey_list[bindkey_list_set.group_nr].entry[bindkey_list_set.entry_nr].keyname,
                               SDL_GetKeyName(key->keysym.sym));
                        bindkey_list[bindkey_list_set.group_nr].entry[bindkey_list_set.entry_nr].key = key->keysym.sym;
                    }
                    keybind_status = KEYBIND_STATUS_NO;
                    return(0);
                }
            }
            keys[key->keysym.sym].pressed = 1;
            keys[key->keysym.sym].time = LastTick + KEY_REPEAT_TIME_INIT;
            check_menu_keys(cpl.menustatus, key->keysym.sym);
        }
        else
        {
            /* no menu */
            if (esc_menu_flag != 1)
            {
                keys[key->keysym.sym].pressed = 1;
                keys[key->keysym.sym].time = LastTick + KEY_REPEAT_TIME_INIT;
                check_keys(key->keysym.sym);
            }
            switch ((int) key->keysym.sym)
            {
            case SDLK_F1:
                quickslot_key(key, 0);
                break;
            case SDLK_F2:
                quickslot_key(key, 1);
                break;
            case SDLK_F3:
                quickslot_key(key, 2);
                break;
            case SDLK_F4:
                quickslot_key(key, 3);
                break;
            case SDLK_F5:
                quickslot_key(key, 4);
                break;
            case SDLK_F6:
                quickslot_key(key, 5);
                break;
            case SDLK_F7:
                quickslot_key(key, 6);
                break;
            case SDLK_F8:
                quickslot_key(key, 7);
                break;
            case SDLK_LSHIFT:
            case SDLK_RSHIFT:
                SetPriorityWidget(MAIN_INV_ID);
                if (!options.playerdoll)
                    SetPriorityWidget(PDOLL_ID);
                cpl.inventory_win = IWIN_INV;
                break;
            case SDLK_RALT:
            case SDLK_LALT:
                cpl.run_on = 1;
                break;
            case SDLK_RCTRL:
            case SDLK_LCTRL:
                cpl.fire_on = 1;

                break;
            case SDLK_ESCAPE:
                if (esc_menu_flag == 0)
                {
                    map_udate_flag = 1;
                    esc_menu_flag = 1;
                    esc_menu_index = ESC_MENU_BACK;
                }
                else
                    esc_menu_flag = 0;
                sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0, 100);
                break;

            default:
                if (esc_menu_flag == 1)
                    check_esc_menu_keys((int)key->keysym.sym);
                break;
            };
        }
    }
    return(0);
}


/* here we look in the user defined keymap and try to get same useful macros */
uint8 check_menu_macros(char *text)
{
    if (!strcmp("?M_SPELL_LIST", text))
    {
        if (cpl.menustatus == MENU_KEYBIND)
            save_keybind_file();
        map_udate_flag = 2;
        if (cpl.menustatus != MENU_SPELL)
        {
            cpl.menustatus = MENU_SPELL;
        }
        else
        {
            cpl.menustatus = MENU_NO;
        }

        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0, 100);
        reset_keys();
        return 1;
    }
    if (!strcmp("?M_SKILL_LIST", text))
    {
        if (cpl.menustatus == MENU_KEYBIND)
            save_keybind_file();

        map_udate_flag = 2;
        if (cpl.menustatus != MENU_SKILL)
        {
            cpl.menustatus = MENU_SKILL;
        }
        else
            cpl.menustatus = MENU_NO;
        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0, 100);
        reset_keys();
        return 1;
    }
    if (!strcmp("?M_KEYBIND", text))
    {
        map_udate_flag = 2;
        if (cpl.menustatus != MENU_KEYBIND)
        {
            keybind_status = KEYBIND_STATUS_NO;
            cpl.menustatus = MENU_KEYBIND;
        }
        else
        {
            save_keybind_file();
            cpl.menustatus = MENU_NO;
        }
        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0, 100);
        reset_keys();
        return 1;
    }
    if (!strcmp("?M_STATUS", text))
    {
        if (cpl.menustatus == MENU_KEYBIND)
            save_keybind_file();

        map_udate_flag = 2;
        if (cpl.menustatus != MENU_STATUS)
        {
            cpl.menustatus = MENU_STATUS;
        }
        else
            cpl.menustatus = MENU_NO;
        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0, 100);
        reset_keys();
        return 1;
    }

    return 0;
}

/* here we handle menu change direct from open menu to
 * open menu, menu close by double press the trigger key
 * and other menu handling stuff - but NOT they keys
 * inside a menu!
 */
static int check_keys_menu_status(int key)
{
    int i, j;

    for (j = 0; j < BINDKEY_LIST_MAX; j++) /* groups */
    {
        for (i = 0; i < OPTWIN_MAX_OPT; i++)
        {
            if (key == bindkey_list[j].entry[i].key)
            {
                if (check_menu_macros(bindkey_list[j].entry[i].text))
                    return 1;
            }
        }
    }
    return  0;
}


void check_keys(int key)
{
    int     i, j;
    char    buf[MEDIUM_BUF];

    for (j = 0; j < BINDKEY_LIST_MAX; j++) /* groups */
    {
        for (i = 0; i < OPTWIN_MAX_OPT; i++)
        {
            if (key == bindkey_list[j].entry[i].key)
            {
                /* if no key macro, submit the text as cmd*/
                if (check_macro_keys(bindkey_list[j].entry[i].text))
                {
                    sprintf(buf, "%s", bindkey_list[j].entry[i].text);
#ifdef DEBUG_TEXT
                    textwin_showstring(COLOR_DGOLD, "%s", buf);
#endif
                    send_game_command(buf);
                }
                return;
            }
        }
    }
}


static uint8 check_macro_keys(char *text)
{
    register int i;
    int magic_len;

    magic_len = strlen(macro_magic_console);
    if (!strncmp(macro_magic_console, text, magic_len) && (int) strlen(text) > magic_len)
    {
        process_macro_keys(KEYFUNC_CONSOLE, 0);
        textwin_putstring(&text[magic_len]);
        return(0);
    }
    for (i = 0; i < (int)DEFAULT_KEYMAP_MACROS; i++)
    {
        if (!strcmp(defkey_macro[i].macro, text))
        {
            if (!process_macro_keys(defkey_macro[i].internal, defkey_macro[i].value))
                return(0);
            return(1);
        }
    }
    return(1);
}


uint8 process_macro_keys(int id, int value)
{
    int     nrof, tag = 0, loc = 0;
    char    buf[MEDIUM_BUF];
    item   *it, *tmp;

    switch (id)
    {
    case KEYFUNC_PAGEUP:
        if (options.use_TextwinSplit)
        {
            txtwin[TW_CHAT].scroll++;
            WIDGET_REDRAW(CHATWIN_ID);
        }
        else
            txtwin[TW_MIX].scroll++;
        break;
    case KEYFUNC_PAGEDOWN:
        if (options.use_TextwinSplit)
        {
            txtwin[TW_CHAT].scroll--;
            WIDGET_REDRAW(CHATWIN_ID);
        }
        else
            txtwin[TW_MIX].scroll--;
        break;
    case KEYFUNC_PAGEUP_TOP:
        txtwin[TW_MSG].scroll++;
        WIDGET_REDRAW(MSGWIN_ID);
        break;
    case KEYFUNC_PAGEDOWN_TOP:
        txtwin[TW_MSG].scroll--;
        WIDGET_REDRAW(MSGWIN_ID);
        break;

    case KEYFUNC_TARGET_ENEMY:
        send_game_command("/target 0");
        break;
    case KEYFUNC_TARGET_FRIEND:
        send_game_command("/target 1");
        break;
    case KEYFUNC_TARGET_SELF:
        send_game_command("/target 2");
        break;
    case KEYFUNC_COMBAT:
        send_game_command("/combat");
        break;

    case KEYFUNC_SPELL:
        map_udate_flag = 2;
        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0, 100);
        if (cpl.menustatus == MENU_KEYBIND)
            save_keybind_file();

        if (cpl.menustatus != MENU_SPELL)
        {
            cpl.menustatus = MENU_SPELL;
        }
        else
        {
            cpl.menustatus = MENU_NO;
        }
        reset_keys();
        break;
    case KEYFUNC_SKILL:
        map_udate_flag = 2;
        if (cpl.menustatus == MENU_KEYBIND)
            save_keybind_file();

        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0, 100);
        if (cpl.menustatus != MENU_SKILL)
            cpl.menustatus = MENU_SKILL;
        else
            cpl.menustatus = MENU_NO;
        reset_keys();
        break;
    case KEYFUNC_STATUS:
        map_udate_flag = 2;
        if (cpl.menustatus == MENU_KEYBIND)
            save_keybind_file();

        if (cpl.menustatus != MENU_STATUS)
            cpl.menustatus = MENU_STATUS;
        else
            cpl.menustatus = MENU_NO;
        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0, 0);
        reset_keys();
        break;
    case KEYFUNC_KEYBIND:
        map_udate_flag = 2;
        if (cpl.menustatus != MENU_KEYBIND)
        {
            keybind_status = KEYBIND_STATUS_NO;
            cpl.menustatus = MENU_KEYBIND;
        }
        else
        {
            save_keybind_file();
            cpl.menustatus = MENU_NO;
        }
        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0, 100);
        reset_keys();
        break;

    case KEYFUNC_CONSOLE:
        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CONSOLE, 0, 0, 100);
        reset_keys();
        if (cpl.input_mode == INPUT_MODE_NO)
        {
            cpl.input_mode = INPUT_MODE_CONSOLE;
            SetPriorityWidget(IN_CONSOLE_ID);
            open_input_mode(253);
        }
        else if (cpl.input_mode == INPUT_MODE_CONSOLE)
            cpl.input_mode = INPUT_MODE_NO;
        map_udate_flag = 2;
        break;

    case KEYFUNC_RUN:
        if (!(cpl.runkey_on = cpl.runkey_on ? 0 : 1))
            send_game_command("/run_stop");
#ifdef DEBUG_TEXT
        textwin_showstring(COLOR_DGOLD, "runmode %s",
                           (cpl.runkey_on) ? "on" : "off");
#endif
        break;
    case KEYFUNC_MOVE:
        move_keys(value);
        break;
    case KEYFUNC_CURSOR:
        cursor_keys(value);
        break;

        /* Alter fire mode selection */
    case KEYFUNC_RANGE:
    case KEYFUNC_RANGE_BACK:
        if (SDL_GetModState() & KMOD_ALT) break;
    case KEYFUNC_RANGE_SELECT:
        switch (id)
        {
        case KEYFUNC_RANGE:
            if (++RangeFireMode > FIRE_MODE_INIT - 1)
                RangeFireMode = 0;
            break;
        case KEYFUNC_RANGE_BACK:
            if (--RangeFireMode < 0)
                RangeFireMode = FIRE_MODE_INIT - 1;
            break;
        case KEYFUNC_RANGE_SELECT:
            RangeFireMode = value;
        }
        map_udate_flag = 2;
        return 0;
        break;

    case KEYFUNC_APPLY:
        if (cpl.inventory_win == IWIN_BELOW)
            tag = cpl.win_below_tag;
        else
            tag = cpl.win_inv_tag;

        if (tag == -1 || !locate_item(tag))
            return 0;
#ifdef DEBUG_TEXT
        textwin_showstring(COLOR_DGOLD, "apply %s", locate_item(tag)->s_name);
#endif
        client_send_apply(tag);
        return 0;
        break;
    case KEYFUNC_EXAMINE:
        if (cpl.inventory_win == IWIN_BELOW)
            tag = cpl.win_below_tag;
        else
            tag = cpl.win_inv_tag;
        if (tag == -1 || !locate_item(tag))
            return 0;
        client_send_examine(tag);
#ifdef DEBUG_TEXT
        textwin_showstring(COLOR_DGOLD, "examine %s", locate_item(tag)->s_name);
#endif
        return 0;
        break;
    case KEYFUNC_MARK:
        if (cpl.inventory_win == IWIN_BELOW)
            tag = cpl.win_below_tag;
        else
            tag = cpl.win_inv_tag;
        if (tag == -1 || !locate_item(tag))
            return 0;
        send_mark_obj((it = locate_item(tag)));
        if (it)
        {
            textwin_showstring(COLOR_DGOLD, "%s %s",
                               (cpl.mark_count == (int)it->tag) ? "unmark" :
                               "mark", it->s_name);
        }
        return 0;
        break;
    case KEYFUNC_LOCK:
        if (cpl.inventory_win == IWIN_BELOW)
            tag = cpl.win_below_tag;
        else
            tag = cpl.win_inv_tag;
        if (tag == -1 || !locate_item(tag))
            return 0;
        toggle_locked((it = locate_item(tag)));
        if (!it)
            return 0;
        textwin_showstring(COLOR_DGOLD, "%s %s",
                           (it->locked) ? "unlock" : "lock", it->s_name);
        return 0;
        break;
    case KEYFUNC_GET:
        nrof = 1; /* number of Items */
        if (cpl.inventory_win == IWIN_BELOW) /* from below to inv*/
        {
            tag = cpl.win_below_tag;
            if (cpl.container)
            {
                /* container, aber nicht der gleiche */
                if ((int)cpl.container->tag != cpl.win_below_ctag)
                    loc = cpl.container->tag;
                else
                    loc = cpl.ob->tag;
            }
            else
                loc = cpl.ob->tag;
        }
        else /* inventory */
        {
            if (cpl.container)
            {
                if ((int)cpl.container->tag == cpl.win_inv_ctag)
                {
                    tag = cpl.win_inv_tag;
                    loc = cpl.ob->tag;
                }
                else /* from inventory to container - if the container is in inv */
                {
                    tag = -1;

                    if (cpl.ob)
                    {
                        for (tmp = cpl.ob->inv; tmp; tmp = tmp->next)
                        {
                            if (tmp->tag == cpl.container->tag)
                            {
                                tag = cpl.win_inv_tag;
                                loc = cpl.container->tag;
                                break;
                            }
                        }
                        if (tag == -1)
                            textwin_showstring(COLOR_DGOLD, "You already have it.");
                    }
                }
            }
            else
            {
                textwin_showstring(COLOR_DGOLD, "You have no open container to put it in.");
                /*
                tag = cpl.win_inv_tag;
                loc = cpl.ob->tag;
                */
                tag = -1;
            }
        }

        if (tag == -1 || !locate_item(tag))
            return 0;
        if ((it = locate_item(tag)))
            nrof = it->nrof;
        else
            return 0;
        if (nrof == 1)
            nrof = 0;
        else
        {
            /* seems be problems with this option
                     if( options.collectAll==1)
                       {
                        nrof = cpl.nrof;
                        goto collectAll;
                       }
                  */


            reset_keys();
            cpl.input_mode = INPUT_MODE_NUMBER;
            SetPriorityWidget(IN_NUMBER_ID);
            open_input_mode(22);
            cpl.loc = loc;
            cpl.tag = tag;
            cpl.nrof = nrof;
            cpl.nummode = NUM_MODE_GET;
            sprintf(buf, "%d", nrof);
            textwin_putstring(buf);
            strncpy(cpl.num_text, it->s_name, 250);
            cpl.num_text[250] = 0;
            return 0;
        }
        /*collectAll:*/
        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_GET, 0, 0, 100);
#ifdef DEBUG_TEXT
        textwin_showstring(COLOR_DGOLD, "get %s", it->s_name);
#endif
        send_inv_move(loc, tag, nrof);
        return 0;

        break;

    case KEYFUNC_LAYER0:
        if (debug_layer[0])
            debug_layer[0] = 0;
        else
            debug_layer[0] = 1;
        textwin_showstring(COLOR_DGOLD, "debug: map layer 0 %s.",
                           (debug_layer[0]) ? "activated" : "deactivated");
        return 0;
        break;
    case KEYFUNC_LAYER1:
        if (debug_layer[1])
            debug_layer[1] = 0;
        else
            debug_layer[1] = 1;
        textwin_showstring(COLOR_DGOLD, "debug: map layer 1 %s.",
                           (debug_layer[1]) ? "activated" : "deactivated");
        return 0;
        break;
    case KEYFUNC_LAYER2:
        if (debug_layer[2])
            debug_layer[2] = 0;
        else
            debug_layer[2] = 1;
        textwin_showstring(COLOR_DGOLD, "debug: map layer 2 %s.",
                           (debug_layer[2]) ? "activated" : "deactivated");
        return 0;
        break;
    case KEYFUNC_LAYER3:
        if (debug_layer[3])
            debug_layer[3] = 0;
        else
            debug_layer[3] = 1;
        textwin_showstring(COLOR_DGOLD, "debug: map layer 3 %s.",
                           (debug_layer[3]) ? "activated" : "deactivated");
        return 0;
        break;

    case KEYFUNC_HELP:
        if (cpl.menustatus == MENU_KEYBIND)
            save_keybind_file();

        cpl.menustatus = MENU_NO;
        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0, 100);

        /*
                cpl.menustatus = MENU_NO;
                sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0, 100);
        */
        return 0;
        break;

    case KEYFUNC_DROP:
        nrof = 1;
        if (cpl.inventory_win == IWIN_INV) /* drop from inventory */
        {
            tag = cpl.win_inv_tag;
            loc = cpl.below->tag;
            if (cpl.win_inv_ctag == -1 && cpl.container && cpl.below)
            {
                for (tmp = cpl.below->inv; tmp; tmp = tmp->next)
                {
                    if (tmp->tag == cpl.container->tag)
                    {
                        tag = cpl.win_inv_tag;
                        loc = cpl.container->tag;
                        break;
                    }
                }
            }
        }
        else
        {
            textwin_showstring(COLOR_DGOLD, "Select something that can be dropped first!");
            return 0;
        }
        if (tag == -1 || !locate_item(tag))
            return 0;
        if ((it = locate_item(tag)))
            nrof = it->nrof;
        else
            return 0;

        if (it->locked)
        {
            sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICKFAIL, 0, 0, 100);
            textwin_showstring(COLOR_DGOLD, "Unlock the item first!");
            return 0;
        }

        if (nrof == 1)
            nrof = 0;
        else
        {
            reset_keys();
            cpl.input_mode = INPUT_MODE_NUMBER;
            SetPriorityWidget(IN_NUMBER_ID);
            open_input_mode(22);
            cpl.loc = loc;
            cpl.tag = tag;
            cpl.nrof = nrof;
            cpl.nummode = NUM_MODE_DROP;
            sprintf(buf, "%d", nrof);
            textwin_putstring(buf);
            strncpy(cpl.num_text, it->s_name, 250);
            cpl.num_text[250] = 0;
            return 0;
        }
        sound_play_effect(SOUNDTYPE_NORMAL, SOUND_DROP, 0, 0, 100);
#ifdef DEBUG_TEXT
        textwin_showstring(COLOR_DGOLD, "drop %s", it->s_name);
#endif
        send_inv_move(loc, tag, nrof);
        return 0;
        break;
    case KEYFUNC_SCREENTOGGLE:
        if (!ToggleScreenFlag)
            ToggleScreenFlag = 1;
        return 0;
        break;


    default:
        return 1;
        break;
    }
    return 0;
}

static void cursor_keys(int num)
{
    switch (num)
    {
    case 0:
        if (cpl.inventory_win == IWIN_BELOW)
        {
            if (cpl.win_below_slot - INVITEMBELOWXLEN >= 0)
                cpl.win_below_slot -= INVITEMBELOWXLEN;
            cpl.win_below_tag = get_inventory_data(cpl.below, &cpl.win_below_ctag, &cpl.win_below_slot,
                                                   &cpl.win_below_start, &cpl.win_below_count, INVITEMBELOWXLEN,
                                                   INVITEMBELOWYLEN);
        }
        else
        {
            if (cpl.win_inv_slot - INVITEMXLEN >= 0)
                cpl.win_inv_slot -= INVITEMXLEN;
            cpl.win_inv_tag = get_inventory_data(cpl.ob, &cpl.win_inv_ctag, &cpl.win_inv_slot, &cpl.win_inv_start,
                                                 &cpl.win_inv_count, INVITEMXLEN, INVITEMYLEN);
        }
        break;

    case 1:
        if (cpl.inventory_win == IWIN_BELOW)
        {
            if (cpl.win_below_slot + INVITEMXLEN < cpl.win_below_count)
                cpl.win_below_slot += INVITEMXLEN;
            cpl.win_below_tag = get_inventory_data(cpl.below, &cpl.win_below_ctag, &cpl.win_below_slot,
                                                   &cpl.win_below_start, &cpl.win_below_count, INVITEMBELOWXLEN,
                                                   INVITEMBELOWYLEN);
        }
        else
        {
            if (cpl.win_inv_slot + INVITEMXLEN < cpl.win_inv_count)
                cpl.win_inv_slot += INVITEMXLEN;
            cpl.win_inv_tag = get_inventory_data(cpl.ob, &cpl.win_inv_ctag, &cpl.win_inv_slot, &cpl.win_inv_start,
                                                 &cpl.win_inv_count, INVITEMXLEN, INVITEMYLEN);
        }
        break;

    case 2:
        if (cpl.inventory_win == IWIN_BELOW)
        {
            cpl.win_below_slot--;
            cpl.win_below_tag = get_inventory_data(cpl.below, &cpl.win_below_ctag, &cpl.win_below_slot,
                                                   &cpl.win_below_start, &cpl.win_below_count, INVITEMBELOWXLEN,
                                                   INVITEMBELOWYLEN);
        }
        else
        {
            cpl.win_inv_slot--;
            cpl.win_inv_tag = get_inventory_data(cpl.ob, &cpl.win_inv_ctag, &cpl.win_inv_slot, &cpl.win_inv_start,
                                                 &cpl.win_inv_count, INVITEMXLEN, INVITEMYLEN);
        }
        break;

    case 3:
        if (cpl.inventory_win == IWIN_BELOW)
        {
            cpl.win_below_slot++;
            cpl.win_below_tag = get_inventory_data(cpl.below, &cpl.win_below_ctag, &cpl.win_below_slot,
                                                   &cpl.win_below_start, &cpl.win_below_count, INVITEMBELOWXLEN,
                                                   INVITEMBELOWYLEN);
        }
        else
        {
            cpl.win_inv_slot++;
            cpl.win_inv_tag = get_inventory_data(cpl.ob, &cpl.win_inv_ctag, &cpl.win_inv_slot, &cpl.win_inv_start,
                                                 &cpl.win_inv_count, INVITEMXLEN, INVITEMYLEN);
        }
        break;
    }
}

/******************************************************************
 Handle quickslot key event.
******************************************************************/
void quickslot_key(SDL_KeyboardEvent *key, int slot)
{
    int     tag;

    if (slot < 0 || slot > MAX_QUICK_SLOTS)
    {
        return;
    }

    /* put spell into quickslot */
    if (!key && cpl.menustatus == MENU_SPELL)
    {
        if (spell_list[spell_list_set.group_nr].entry[spell_list_set.class_nr][spell_list_set.entry_nr].flag
                == LIST_ENTRY_KNOWN)
        {
            if (new_quickslots[slot].type == 1 && new_quickslots[slot].group == spell_list_set.group_nr &&
                new_quickslots[slot].path == spell_list_set.class_nr && new_quickslots[slot].tag == spell_list_set.entry_nr)
            {
                quickslot_unset(slot);
                textwin_showstring(COLOR_DGOLD, "unset F%d.", slot + 1);
            }
            else
            {
                quickslot_set(slot, 1, spell_list_set.group_nr, spell_list_set.class_nr,
                              spell_list_set.entry_nr, -1, -1);
                textwin_showstring(COLOR_DGOLD, "set F%d to %s",
                                   slot + 1,
                                   spell_list[spell_list_set.group_nr].entry[spell_list_set.class_nr][spell_list_set.entry_nr].name);
            }
        }
    }
    /* put item into quickslot */
    else if (key && key->keysym.mod & (KMOD_SHIFT | KMOD_ALT))
    {
        if (cpl.inventory_win == IWIN_BELOW)
            return;
        tag = cpl.win_inv_tag;

        if (tag == -1 || !locate_item(tag))
            return;
        new_quickslots[slot].type = 0;
        if (new_quickslots[slot].tag == tag)
            quickslot_unset(slot);
        else
        {
            quickslot_set(slot, 2, -1, -1, tag, cpl.win_inv_slot, -1);
            textwin_showstring(COLOR_DGOLD, "set F%d to %s",
                               slot + 1, locate_item(tag)->s_name);
        }
    }
    /* apply item or ready spell */
    else
    {
        if (new_quickslots[slot].type <= 0)
        {
            return;
        }
        if (new_quickslots[slot].tag != -1)
        {
            if (new_quickslots[slot].type == 1)
            {
                fire_mode_tab[FIRE_MODE_SPELL].spell = &spell_list[new_quickslots[slot].group].entry[new_quickslots[slot].path][new_quickslots[slot].tag];
                RangeFireMode = FIRE_MODE_SPELL;
                spell_list_set.group_nr = new_quickslots[slot].group;
                spell_list_set.class_nr = new_quickslots[slot].path;
                spell_list_set.entry_nr = new_quickslots[slot].tag;
                return;
            }
            else if (locate_item(new_quickslots[slot].tag))
            {
                client_send_apply(new_quickslots[slot].tag);
                return;
            }
        }
    }
}

static void move_keys(int num)
{
#ifdef DEBUG_TEXT
    char buf[MEDIUM_BUF];
#endif

    if (cpl.menustatus != MENU_NO)
        reset_menu_status();

    if (num == 5)
    {
        send_move_command(num, 0);
        return;
    }

    /* move will overruled from fire */
    /* because real toggle mode don't work, this works a bit different */
    /* pressing alt will not set move mode until unpressed when firemode is on */
    /* but it stops running when released */
    if ((cpl.runkey_on || cpl.run_on) && (!cpl.firekey_on && !cpl.fire_on)) /* runmode on, or ALT key trigger */
    {
        send_game_command(directionsrun[num]);
#ifdef DEBUG_TEXT
        sprintf(buf, "run ");
#endif
    }
    /* thats the range menu - we handle it messages unique */
    else if (cpl.firekey_on || cpl.fire_on)
    {
        char *tmp_name = NULL;

        if (RangeFireMode == FIRE_MODE_SKILL)
        {
            if (!fire_mode_tab[FIRE_MODE_SKILL].skill || fire_mode_tab[FIRE_MODE_SKILL].skill->flag == -1)
            {
                textwin_showstring(COLOR_WHITE, "no skill selected.");
                return;
            }
            tmp_name = fire_mode_tab[RangeFireMode].skill->name;

/*            sprintf(buf, "/%s %d %d %s", directionsfire[num], RangeFireMode, -1,
                    fire_mode_tab[RangeFireMode].skill->name);
*/
        }
        else if (RangeFireMode == FIRE_MODE_SPELL)
        {
            if (!fire_mode_tab[FIRE_MODE_SPELL].spell || fire_mode_tab[FIRE_MODE_SPELL].spell->flag == -1)
            {
                textwin_showstring(COLOR_WHITE, "no spell selected.");
                return;
            }
            tmp_name = fire_mode_tab[RangeFireMode].spell->name;
/*            sprintf(buf, "/%s %d %d %s", directionsfire[num], RangeFireMode, -1,
                    fire_mode_tab[RangeFireMode].spell->name);
*/
        }
        else
        {

/*            sprintf(buf, "/%s %d %d %d", directionsfire[num], RangeFireMode, fire_mode_tab[RangeFireMode].item,
                    fire_mode_tab[RangeFireMode].amun);
*/
            if (RangeFireMode == FIRE_MODE_BOW)
            {
                if (fire_mode_tab[FIRE_MODE_BOW].item == FIRE_ITEM_NO)
                {
                    textwin_showstring(COLOR_WHITE, "no range weapon selected.");
                    return;
                }
            }
        }

        /* atm we only use direction, mode and the skill/spell name */
        send_fire_command(num, RangeFireMode, tmp_name);

#ifdef DEBUG_TEXT
        if (RangeFireMode == FIRE_MODE_SKILL)
            sprintf(buf, "use %s %s", fire_mode_tab[RangeFireMode].skill->name, directions_name[num]);
        else if (RangeFireMode == FIRE_MODE_SPELL)
            sprintf(buf, "cast %s %s", fire_mode_tab[RangeFireMode].spell->name, directions_name[num]);
        else if (RangeFireMode == FIRE_MODE_BOW)
            sprintf(buf, "fire %s", directions_name[num]);

        textwin_showstring(COLOR_DGOLD, "%s", buf);
#endif
        return;
    }
    else
    {
        send_move_command(num, 0);
#ifdef DEBUG_TEXT
        buf[0] = '\0';
    }
    textwin_showstring(COLOR_DGOLD, "%s", directions_name[num]);
#else
    }
#endif
}




/******************************************************************
 Handle key repeating.
******************************************************************/
static void key_repeat(void)
{
    register int i,j;

    /* A 'real' menu/dialog or the escape menu: */
    if (cpl.menustatus != MENU_NO || esc_menu_flag == 1)
    {
        /* check menu-keys for repeat */
        if (options.menu_repeat > 0 && (SDL_GetTicks() - menuRepeatTicks > menuRepeatTime || !menuRepeatTicks || menuRepeatKey < 0))
        {
            menuRepeatTicks = SDL_GetTicks();
            if (menuRepeatKey >= 0)
            {
                if (esc_menu_flag == 1)
                    check_esc_menu_keys(menuRepeatKey);
                else
                    check_menu_keys(cpl.menustatus, menuRepeatKey);
                menuRepeatTime = 70 / options.menu_repeat; // interval
            }
        }
    }
    /* Normal gameplay: */
    else
    {
        /* TODO: optimize this one, too */
        for (j = 0; j < BINDKEY_LIST_MAX; j++)
        {
            /* groups */
            for (i = 0; i < OPTWIN_MAX_OPT; i++)
            {
                if (keys[bindkey_list[j].entry[i].key].pressed && bindkey_list[j].entry[i].repeatflag) /* key for this keymap is pressed*/
                {
                    if (keys[bindkey_list[j].entry[i].key].time + KEY_REPEAT_TIME - 5 < LastTick) /* if time to repeat */
                    {
                        /* repeat x times*/
                        while ((keys[bindkey_list[j].entry[i].key].time += KEY_REPEAT_TIME - 5) < LastTick)
                        {
                            if (check_macro_keys(bindkey_list[j].entry[i].text)) /* if no key macro, submit the text as cmd*/
                            {
                                char buf[MEDIUM_BUF];

                                sprintf(buf, "%s", bindkey_list[j].entry[i].text);
#ifdef DEBUG_TEXT
                                textwin_showstring(COLOR_DGOLD, "%s", buf);
#endif
                                send_game_command(buf);
                            }
                        }
                    }
                }
            }
        }
    }
}

/******************************************************************
 Import the key-binding file.
******************************************************************/
void read_keybind_file(void)
{
    char           buf[MEDIUM_BUF];
    PHYSFS_File   *handle;
    PHYSFS_sint64  len;
    int            i, pos;

    LOG(LOG_MSG, "Trying to load keybindings... ");
    sprintf(buf, "%s/%s", DIR_SETTINGS, FILE_KEYBIND);

    if (!(handle = PHYSFS_openRead(buf)))
    {
        LOG(LOG_ERROR, "FAILED: %s!\n", PHYSFS_getLastError());
        bindkey_list_set.group_nr = 0;
        bindkey_list_set.entry_nr = 0;

        return;
    }

    bindkey_list_set.group_nr = -1;
    bindkey_list_set.entry_nr = 0;

    while ((len = PHYSFS_readString(handle, buf, sizeof(buf))) >= 0)
    {
        /* Skip comments, blank lines, and incomplete lines. */
        if (buf[0] == '#' ||
            buf[0] == '\0' ||
            len < 4)
        {
            continue;
        }

        i = 1;

        /* found key group */
        if (buf[0] == '+')
        {
            if (++bindkey_list_set.group_nr == BINDKEY_LIST_MAX)
            {
                break;
            }

            while (buf[++i] && buf[i] != '"' && i - 2 < OPTWIN_MAX_TABLEN - 1)
            {
                bindkey_list[bindkey_list_set.group_nr].name[i - 2] = buf[i];
            }

            bindkey_list[bindkey_list_set.group_nr].name[i - 2] = 0;
            bindkey_list_set.entry_nr = 0;

            continue;
        }

        if (bindkey_list_set.group_nr < 0)
        {
            break; /* something is wrong with the file */
        }

        /* found a key entry */
        sscanf(buf, " %d %d", &bindkey_list[bindkey_list_set.group_nr].entry[bindkey_list_set.entry_nr].key,
               &bindkey_list[bindkey_list_set.group_nr].entry[bindkey_list_set.entry_nr].repeatflag);
        pos = 0;

        while (buf[++i] && buf[i] != '"') /* start of 1. string */
        {
            ;
        }

        while (buf[++i] && buf[i] != '"')
        {
            bindkey_list[bindkey_list_set.group_nr].entry[bindkey_list_set.entry_nr].keyname[pos++] = buf[i];
        }

        bindkey_list[bindkey_list_set.group_nr].entry[bindkey_list_set.entry_nr].keyname[pos] = 0;
        pos = 0;

        while (buf[++i] && buf[i] != '"') /* start of 2. string */
        {
            ;
        }

        while (buf[++i] && buf[i] != '"')
        {
            bindkey_list[bindkey_list_set.group_nr].entry[bindkey_list_set.entry_nr].text[pos++] = buf[i];
        }

        bindkey_list[bindkey_list_set.group_nr].entry[bindkey_list_set.entry_nr].text[pos] = 0;

        if (!strcmp(bindkey_list[bindkey_list_set.group_nr].entry[bindkey_list_set.entry_nr].text, "?M_GET"))
        {
            get_action_keycode = bindkey_list[bindkey_list_set.group_nr].entry[bindkey_list_set.entry_nr].key;
        }
        else if (!strcmp(bindkey_list[bindkey_list_set.group_nr].entry[bindkey_list_set.entry_nr].text, "?M_DROP"))
        {
            drop_action_keycode = bindkey_list[bindkey_list_set.group_nr].entry[bindkey_list_set.entry_nr].key;
        }

        if (++bindkey_list_set.entry_nr == OPTWIN_MAX_OPT)
        {
            break;
        }
    }

    PHYSFS_close(handle);

    if (bindkey_list_set.group_nr <= 0)
    {
        sprintf(bindkey_list[0].entry[0].keyname, "keybind file is corrupt!");
        strcpy(bindkey_list[0].entry[0].text, "|ERROR!|");
        LOG(LOG_ERROR, "FAILED: keybind file is corrupt!\n");
    }
    else
    {
        LOG(LOG_MSG, "OK!\n");
    }

    bindkey_list_set.group_nr = 0;
    bindkey_list_set.entry_nr = 0;
}

/******************************************************************
 Export the key-binding file.
******************************************************************/
void save_keybind_file(void)
{
    int          entry, group;
    char         buf[MEDIUM_BUF];
    PHYSFS_File *handle;

    LOG(LOG_MSG, "Trying to save keybindings... ");
    sprintf(buf, "%s/%s", DIR_SETTINGS, FILE_KEYBIND);

    if (!(handle = PHYSFS_openWrite(buf)))
    {
        LOG(LOG_ERROR, "FAILED: %s!\n", PHYSFS_getLastError());

        return;
    }

    for (group = 0; group < BINDKEY_LIST_MAX; group++)
    {
        if (!bindkey_list[group].name[0])
        {
            continue; /* this group is empty, what about the next one? */
        }

        if (group)
        {
            PHYSFS_writeString(handle, "\n");
        }

        sprintf(buf, "+\"%s\"\n", bindkey_list[group].name);
        PHYSFS_writeString(handle, buf);

        for (entry = 0; entry < OPTWIN_MAX_OPT; entry++)
        {
            if (!bindkey_list[group].entry[entry].text[0])
            {
                continue; /* this entry is empty, what about the next one? */
            }

            /* we need to know for INPUT_MODE_NUMBER "quick get" this key */
            if (!strcmp(bindkey_list[group].entry[entry].text, "?M_GET"))
            {
                get_action_keycode = bindkey_list[group].entry[entry].key;
            }
            else if (!strcmp(bindkey_list[group].entry[entry].text, "?M_DROP"))
            {
                drop_action_keycode = bindkey_list[group].entry[entry].key;
            }

            /* save key entry */
            sprintf(buf, "%.3d %d \"%s\" \"%s\"\n", bindkey_list[group].entry[entry].key,
                    bindkey_list[group].entry[entry].repeatflag, bindkey_list[group].entry[entry].keyname,
                    bindkey_list[group].entry[entry].text);
            PHYSFS_writeString(handle, buf);
        }
    }

    PHYSFS_close(handle);
    LOG(LOG_MSG, "OK!\n");
}

/******************************************************************
 Handle keystrokes in the escape menu.
******************************************************************/
static void check_esc_menu_keys(int key)
{
    reset_keys();
    switch (key)
    {
    case SDLK_RETURN:
        if (esc_menu_index == ESC_MENU_KEYS)
        {
            keybind_status = KEYBIND_STATUS_NO;
            cpl.menustatus = MENU_KEYBIND;
        }
        else if (esc_menu_index == ESC_MENU_SETTINGS)
        {
            keybind_status = KEYBIND_STATUS_NO;
            if (cpl.menustatus == MENU_KEYBIND)
                save_keybind_file();
            cpl.menustatus = MENU_OPTION;
        }
        else if (esc_menu_index == ESC_MENU_LOGOUT)
        {
            save_quickslots_entrys();
            SOCKET_CloseClientSocket(&csocket);
        }
        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0, 100);
        esc_menu_flag = 0;
        break;

    case SDLK_UP:
        esc_menu_index--;
        if (esc_menu_index < 0)
            esc_menu_index = ESC_MENU_INDEX - 1;
        menuRepeatKey = SDLK_UP;
        break;
    case SDLK_DOWN:
        esc_menu_index++;
        if (esc_menu_index >= ESC_MENU_INDEX)
            esc_menu_index = 0;
        menuRepeatKey = SDLK_DOWN;
        break;
    };
}

/******************************************************************
 Handle keystrokes in menue-dialog.
******************************************************************/
void check_menu_keys(int menu, int key)
{
    int   shiftPressed = (SDL_GetModState() & KMOD_SHIFT),
          ctrlPressed = (SDL_GetModState() & KMOD_CTRL);
    sint8 n;
    uint8 res_change = 0;

    if (cpl.menustatus == MENU_NO ||
        check_keys_menu_status(key))
    {
        return;
    }

    switch (menu)
    {
    case MENU_BOOK:
        if (!gui_interface_book || gui_interface_book->pages == 0)
            return;
        switch (key)
        {
        case SDLK_ESCAPE:
            cpl.menustatus = MENU_NO;
            map_udate_flag = 2;
            reset_keys();
            break;
        case SDLK_LEFT:
            gui_interface_book->page_show-=2;
            if (gui_interface_book->page_show<0)
            {
                gui_interface_book->page_show=0;
                sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
            }
            else
            {
                sound_play_effect(SOUNDTYPE_CLIENT, SOUND_PAGE, 0, 0, MENU_SOUND_VOL);
            }
            break;
        case SDLK_RIGHT:
            gui_interface_book->page_show+=2;
            if (gui_interface_book->page_show>(gui_interface_book->pages-1))
            {
                gui_interface_book->page_show=(gui_interface_book->pages-1)&~1;
                sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
            }
            else
            {
                sound_play_effect(SOUNDTYPE_CLIENT, SOUND_PAGE, 0, 0, MENU_SOUND_VOL);
            }
            break;
        }
        break;

    case MENU_NPC:
        gui_npc_keypress(key);
        menuRepeatKey = key;

        break;

    case MENU_OPTION:
        switch (key)
        {
        case SDLK_LEFT:
            n = -1;

            if (shiftPressed)
            {
                n *= 2;
            }

            if (ctrlPressed)
            {
                n *= 4;
            }

            option_list_set.key_change = n;
            /*sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK,0,0,MENU_SOUND_VOL);*/
            menuRepeatKey = SDLK_LEFT;
            break;
        case SDLK_RIGHT:
            n = 1;

            if (shiftPressed)
            {
                n *= 2;
            }

            if (ctrlPressed)
            {
                n *= 4;
            }

            option_list_set.key_change = n;
            /*sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK,0,0,MENU_SOUND_VOL);*/
            menuRepeatKey = SDLK_RIGHT;
            break;
        case SDLK_UP:
            if (!shiftPressed)
            {
                if (option_list_set.entry_nr > 0)
                    option_list_set.entry_nr--;
                else
                    sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
            }
            else
            {
                if (option_list_set.group_nr > 0)
                {
                    option_list_set.group_nr--;
                    option_list_set.entry_nr = 0;
                }
                else
                    sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
            }
            menuRepeatKey = SDLK_UP;
            break;
        case SDLK_DOWN:
            if (!shiftPressed)
            {
                int i    = -1,
                    page = option_list_set.group_nr;
                while (page && opt[++i].name)
                    if (opt[i].name[0] == '#')
                        --page;
                i += 1 + option_list_set.entry_nr + 1;
                if (opt[i].name[0] && opt[i].name[0] != '#')
                    option_list_set.entry_nr++;
                else
                    sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
            }
            else
            {
                if (opt_tab[option_list_set.group_nr + 1])
                {
                    option_list_set.group_nr++;
                    option_list_set.entry_nr = 0;
                }
                else
                    sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
            }
            menuRepeatKey = SDLK_DOWN;
            break;
        case SDLK_d:
        case SDLK_ESCAPE:
            sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0, MENU_SOUND_VOL);
            save_options_dat();
            Mix_VolumeMusic(options.music_volume);
            if (options.playerdoll)
                cur_widget[PDOLL_ID].show = 1;

            /* ToggleScreenFlag sets changes this option in the main loop
             * so we revert this setting, also if a resolution change occurs
             * we first change the resolution, without toggling, and after that
             * succeeds we use the specialized attempt_fullscreentoggle
             */
            if (options.fullscreen_flag != options.fullscreen)
            {
                if (options.fullscreen)
                    options.fullscreen = 0;
                else
                    options.fullscreen = 1;
                ToggleScreenFlag = 1;
            }
            /* lets add on the fly resolution change for testing */
            /* i know this part is heavy, but we want to make sure we try everything
             * before we shut down the client
             */
            if (Screensize.x!=Screendefs[options.resolution].x || Screensize.y!=Screendefs[options.resolution].y)
            {
                Uint32 videoflags = get_video_flags();
                _screensize sz_tmp = Screensize;
                Screensize=Screendefs[options.resolution];

                if ((ScreenSurface = SDL_SetVideoMode(Screensize.x, Screensize.y, options.used_video_bpp, videoflags)) == NULL)
                {
                    int i;
                    textwin_showstring(COLOR_RED, "Couldn't set %dx%dx%d video mode: %s\n", Screensize.x, Screensize.y, options.used_video_bpp, SDL_GetError());
                    LOG(LOG_ERROR, "Couldn't set %dx%dx%d video mode: %s\n", Screensize.x, Screensize.y, options.used_video_bpp, SDL_GetError());
                    Screensize=sz_tmp;
                    for (i=0;i<16;i++)
                        if (Screensize.x==Screendefs[i].x && Screensize.y == Screendefs[i].y)
                        {
                            options.resolution = i;
                            break;
                        }
                    textwin_showstring(COLOR_RED, "Try to switch back to old setting...");
                    LOG(LOG_ERROR, "Try to switch back to old setting...\n");

                    if ((ScreenSurface = SDL_SetVideoMode(Screensize.x, Screensize.y, options.used_video_bpp, videoflags)) == NULL)
                    {
                        textwin_showstring(COLOR_RED, "Couldn't set %dx%dx%d video mode: %s\n", Screensize.x, Screensize.y, options.used_video_bpp, SDL_GetError());
                        LOG(LOG_ERROR, "Couldn't set %dx%dx%d video mode: %s\n", Screensize.x, Screensize.y, options.used_video_bpp, SDL_GetError());
                        Screensize=Screendefs[0];
                        options.resolution = 0;
                        textwin_showstring(COLOR_RED, "Try to switch back to 800x600...");
                        LOG(LOG_ERROR, "Try to switch back to 800x600...\n");
                        if ((ScreenSurface = SDL_SetVideoMode(Screensize.x, Screensize.y, options.used_video_bpp, videoflags)) == NULL)
                        {
                            /* now we have a problem */
                            textwin_showstring(COLOR_RED, "Couldn't set %dx%dx%d video mode: %s\nFATAL ERROR - exit", Screensize.x, Screensize.y, options.used_video_bpp, SDL_GetError());
                            LOG(LOG_ERROR, "Couldn't set %dx%dx%d video mode: %s\nFATAL ERROR - exit", Screensize.x, Screensize.y, options.used_video_bpp, SDL_GetError());
                            Screensize=sz_tmp;
                            exit(2);
                        }
                        else
                            res_change = 1;
                    }
                    else
                        res_change = 1;
                }
                else
                    res_change = 1;
            }
            if (res_change)
            {
                const SDL_VideoInfo    *info    = NULL;
                info = SDL_GetVideoInfo();
                options.real_video_bpp = info->vfmt->BitsPerPixel;
//                    SDL_FreeSurface(ScreenSurfaceMap);
//                    ScreenSurfaceMap=SDL_CreateRGBSurface(ScreenSurface->flags, Screensize.x, Screensize.y, options.used_video_bpp, 0,0,0,0);
            }
            cpl.menustatus = MENU_NO;
            map_udate_flag = 2;
            reset_keys();
            break;
        }
        break;

    case MENU_SKILL:
        switch (key)
        {
        case SDLK_RETURN:
            if (skill_list[skill_list_set.group_nr].entry[skill_list_set.entry_nr].flag == LIST_ENTRY_KNOWN)
            {
                fire_mode_tab[FIRE_MODE_SKILL].skill = &skill_list[skill_list_set.group_nr].entry[skill_list_set.entry_nr];
                RangeFireMode = FIRE_MODE_SKILL;
                sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0, MENU_SOUND_VOL);
            }
            else
                sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
        case SDLK_ESCAPE:
            cpl.menustatus = MENU_NO;
            map_udate_flag = 2;
            reset_keys();
            break;
        case SDLK_UP:
            if (!shiftPressed)
            {
                if (skill_list_set.entry_nr > 0)
                {
                    if (skill_list[skill_list_set.group_nr].entry[--skill_list_set.entry_nr].flag == LIST_ENTRY_KNOWN)
                        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0, MENU_SOUND_VOL);
                }
                else
                    sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
            }
            else
            {
                if (skill_list_set.group_nr > 0)
                {
                    skill_list_set.group_nr--;
                    skill_list_set.entry_nr = 0;
                }
                else
                    sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
            }
            menuRepeatKey = SDLK_UP;
            break;
        case SDLK_DOWN:
            if (!shiftPressed)
            {
                if (skill_list_set.entry_nr < DIALOG_LIST_ENTRY - 1)
                {
                    if (skill_list[skill_list_set.group_nr].entry[++skill_list_set.entry_nr].flag == LIST_ENTRY_KNOWN)
                        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0, MENU_SOUND_VOL);
                }
                else
                    sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
            }
            else
            {
                if (skill_list_set.group_nr < SKILL_LIST_MAX - 1)
                {
                    skill_list_set.group_nr++;
                    skill_list_set.entry_nr = 0;
                }
                else
                    sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
            }
            menuRepeatKey = SDLK_DOWN;
            break;
        }
        break;

    case MENU_SPELL:
        switch (key)
        {
        case SDLK_F1:
            quickslot_key(NULL, 0);
            break;
        case SDLK_F2:
            quickslot_key(NULL, 1);
            break;
        case SDLK_F3:
            quickslot_key(NULL, 2);
            break;
        case SDLK_F4:
            quickslot_key(NULL, 3);
            break;
        case SDLK_F5:
            quickslot_key(NULL, 4);
            break;
        case SDLK_F6:
            quickslot_key(NULL, 5);
            break;
        case SDLK_F7:
            quickslot_key(NULL, 6);
            break;
        case SDLK_F8:
            quickslot_key(NULL, 7);
            break;

        case SDLK_RETURN:
            if (spell_list[spell_list_set.group_nr].entry[spell_list_set.class_nr][spell_list_set.entry_nr].flag
                    == LIST_ENTRY_KNOWN)
            {
                fire_mode_tab[FIRE_MODE_SPELL].spell = &spell_list[spell_list_set.group_nr].entry[spell_list_set.class_nr][spell_list_set.entry_nr];
                RangeFireMode = FIRE_MODE_SPELL;
                sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0, MENU_SOUND_VOL);
            }
            else
                sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
        case SDLK_ESCAPE:
            cpl.menustatus = MENU_NO;
            map_udate_flag = 2;
            reset_keys();
            break;
        case SDLK_LEFT:
            if (spell_list_set.class_nr > 0)
            {
                spell_list_set.class_nr--;
                sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0, MENU_SOUND_VOL);
            }
            else
                sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
            break;
        case SDLK_RIGHT:
            if (spell_list_set.class_nr < SPELL_LIST_CLASS - 1)
            {
                spell_list_set.class_nr++;
                sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0, MENU_SOUND_VOL);
            }
            else
                sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
            break;
        case SDLK_UP:
            if (!shiftPressed)
            {
                if (spell_list_set.entry_nr > 0)
                {
                    if (spell_list[spell_list_set.group_nr].entry[spell_list_set.class_nr][--spell_list_set.entry_nr].flag == LIST_ENTRY_KNOWN)
                        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0, MENU_SOUND_VOL);
                }
                else
                    sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
            }
            else
            {
                if (spell_list_set.group_nr > 0)
                {
                    spell_list_set.group_nr--;
                    spell_list_set.entry_nr = 0;
                }
                else
                    sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
            }
            menuRepeatKey = SDLK_UP;
            break;
        case SDLK_DOWN:
            if (!shiftPressed)
            {
                if (spell_list_set.entry_nr < DIALOG_LIST_ENTRY - 1)
                {
                    if (spell_list[spell_list_set.group_nr].entry[spell_list_set.class_nr][++spell_list_set.entry_nr].flag == LIST_ENTRY_KNOWN)
                        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0, MENU_SOUND_VOL);
                }
                else
                    sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
            }
            else
            {
                if (spell_list_set.group_nr < SPELL_LIST_MAX - 1)
                {
                    spell_list_set.group_nr++;
                    spell_list_set.entry_nr = 0;
                }
                else
                    sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
            }
            menuRepeatKey = SDLK_DOWN;
            break;
        }
        break;

    case MENU_KEYBIND:
        switch (key)
        {
        case SDLK_DELETE:
            if (bindkey_list[bindkey_list_set.group_nr].entry[bindkey_list_set.entry_nr].text[0])
            {
                sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICKFAIL, 0, 0,
                                  MENU_SOUND_VOL);
                memset(&bindkey_list[bindkey_list_set.group_nr].entry[bindkey_list_set.entry_nr],
                       0, sizeof(_keymap));
            }
            break;
        case SDLK_UP:
            if (!shiftPressed)
            {
                if (bindkey_list_set.entry_nr > 0)
                {
                    if (bindkey_list[bindkey_list_set.group_nr].entry[--bindkey_list_set.entry_nr].text[0])
                        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0, MENU_SOUND_VOL);
                }
                else
                    sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
            }
            else
            {
                if (bindkey_list_set.group_nr > 0)
                {
                    bindkey_list_set.group_nr--;
                    bindkey_list_set.entry_nr = 0;
                }
                else
                    sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
            }
            menuRepeatKey = SDLK_UP;
            break;
        case SDLK_DOWN:
            if (!shiftPressed)
            {
                if (bindkey_list_set.entry_nr < OPTWIN_MAX_OPT - 1)
                {
                    if (bindkey_list[bindkey_list_set.group_nr].entry[++bindkey_list_set.entry_nr].text[0])
                        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0, MENU_SOUND_VOL);
                }
                else
                    sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
            }
            else
            {
                if (bindkey_list_set.group_nr < BINDKEY_LIST_MAX - 1
                        && bindkey_list[bindkey_list_set.group_nr + 1].name[0])
                {
                    bindkey_list_set.group_nr++;
                    bindkey_list_set.entry_nr = 0;
                }
                else
                    sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
            }
            menuRepeatKey = SDLK_DOWN;
            break;
        case SDLK_d:
        case SDLK_ESCAPE:
            save_keybind_file();
            sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0, MENU_SOUND_VOL);
            cpl.menustatus = MENU_NO;
            map_udate_flag = 2;
            reset_keys();
            break;
        case SDLK_RETURN:
            sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0, MENU_SOUND_VOL);
            keybind_status = KEYBIND_STATUS_EDIT;
            reset_keys();
            open_input_mode(240);
            textwin_putstring(bindkey_list[bindkey_list_set.group_nr].entry[bindkey_list_set.entry_nr].text);
            cpl.input_mode = INPUT_MODE_GETKEY;
            sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0, MENU_SOUND_VOL);
            break;
        case SDLK_r:
            sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0, MENU_SOUND_VOL);
            bindkey_list[bindkey_list_set.group_nr].entry[bindkey_list_set.entry_nr].repeatflag = bindkey_list[bindkey_list_set.group_nr].entry[bindkey_list_set.entry_nr].repeatflag
                    ? 0
                    : 1;
            sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0, MENU_SOUND_VOL);
            break;
        }
        break;

    case MENU_CREATE:
        switch (key)
        {
        case SDLK_RETURN:
            break;
        case SDLK_ESCAPE:
            dialog_new_char_warn = 0;
            reset_input_mode();
            GameStatus = GAME_STATUS_ACCOUNT;
            cpl.menustatus = MENU_NO;
            map_udate_flag = 2;
            reset_keys();
            break;
        case SDLK_n:
            if (new_character.skill_selected == -1)
            {
                dialog_new_char_warn = 2;
                sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICKFAIL, 0, 0, 100);
                break;
            }
            dialog_new_char_warn = 0;
            reset_input_mode();
            cpl.name[0] = 0;
            InputStringFlag=1;
            InputStringEndFlag=0;
            open_input_mode(MAX_PLAYER_NAME);
            GameStatus = GAME_STATUS_ACCOUNT_CHAR_NAME;
            cpl.menustatus = MENU_NO;
            break;
        case SDLK_LEFT:
            create_list_set.key_change = -1;
            menuRepeatKey = -1;
            break;
        case SDLK_RIGHT:
            create_list_set.key_change = 1;
            menuRepeatKey = -1;
            break;
        case SDLK_UP:
            if (--create_list_set.entry_nr < 0)
                create_list_set.entry_nr = 1;
            sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0, MENU_SOUND_VOL);
            menuRepeatKey = -1;
            break;
        case SDLK_DOWN:
            if (++create_list_set.entry_nr >= 2)
                create_list_set.entry_nr = 0;
            sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0, MENU_SOUND_VOL);
            menuRepeatKey = -1;
            break;
        }
        break;
    }
}
