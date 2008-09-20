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

extern char d_ServerName[2048];
extern int  d_ServerPort;
static int  get_action_keycode, drop_action_keycode; /* thats the key for G'et command from keybind */
static int  menuRepeatKey   = -1;
       int  old_mouse_y = 0;
button_status global_buttons;

typedef struct _keys
{
    Boolean         pressed; /*true: key is pressed*/
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
static char    *directions_name[10]     =
    {
        "null", "southwest", "south", "southeast", "west", "stay", "east", "northwest", "north", "northeast"
    };
static char    *directionsrun[10]       =
    {
        "/run 0", "/run 6", "/run 5", "/run 4", "/run 7",\
        "/run 5", "/run 3", "/run 8", "/run 1", "/run 2"
    };

static int      key_event(SDL_KeyboardEvent *key);
static void     key_string_event(SDL_KeyboardEvent *key);
static Boolean  check_macro_keys(char *text);
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
    InputStringFlag = FALSE;
    InputStringEndFlag = FALSE;
    InputStringEscFlag = FALSE;

    for (i = 0; i < MAX_KEYS; i++)
        keys[i].pressed = FALSE;
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
            sound_play_effect(SOUND_SCROLL, 0, 0, 100);
            GameStatusSelect == GAME_STATUS_LOGIN_ACCOUNT?(GameStatusSelect=GAME_STATUS_LOGIN_NEW):(GameStatusSelect=GAME_STATUS_LOGIN_ACCOUNT);
            break;

        case SDLK_DOWN:
            sound_play_effect(SOUND_SCROLL, 0, 0, 100);
            GameStatusSelect==GAME_STATUS_LOGIN_ACCOUNT?(GameStatusSelect=GAME_STATUS_LOGIN_NEW):(GameStatusSelect=GAME_STATUS_LOGIN_ACCOUNT);
            break;

        case SDLK_RETURN:
            sound_play_effect(SOUND_SCROLL, 0, 0, 100);
            dialog_login_warning_level = DIALOG_LOGIN_WARNING_NONE;
            open_input_mode(MAX_ACCOUNT_NAME);
            LoginInputStep = LOGIN_STEP_NAME;
            GameStatus = GameStatusSelect; /* create account or direct login */
            break;

        case SDLK_ESCAPE:
            sound_play_effect(SOUND_SCROLL, 0, 0, 100);
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
            sound_play_effect(SOUND_SCROLL, 0, 0, 100);
            if(account.count && --account.selected < 0)
                account.selected = account.count-1;
            break;

        case SDLK_DOWN:
            sound_play_effect(SOUND_SCROLL, 0, 0, 100);
            if(account.count && ++account.selected > account.count-1)
                    account.selected = 0;
            break;

        case SDLK_c:
            if(account.count < ACCOUNT_MAX_PLAYER)
            {
                sound_play_effect(SOUND_PAGE, 0, 0, 100);
                GameStatus = GAME_STATUS_ACCOUNT_CHAR_CREATE;
            }
            break;

        case SDLK_d:
            if(account.count)
            {
                sound_play_effect(SOUND_PAGE, 0, 0, 100);
                GameStatus = GAME_STATUS_ACCOUNT_CHAR_DEL;
                /* the player must write "delete" , then we allow deletion of the selected char name */
                reset_input_mode();
                InputStringFlag=TRUE;
                InputStringEndFlag=FALSE;
                open_input_mode(MAX_PLAYER_NAME);
                cpl.menustatus = MENU_NO;
            }
            break;

        case SDLK_RETURN:
            sound_play_effect(SOUND_PAGE, 0, 0, 100);

            /* put client in wait modus... 2 choices:
            * a.) login fails - we will get a BINARY_CMD_ACCOUNT with error msg
            * b.) server is adding <name> - we get a BINARY_CMD_PLAYER and then regular playing data
            * Option a.) will move us back to character selection with an error message in the status
            */
            if(account.count)
            {
                /* tell server that we want play with this char */
                SendAddMe(account.name[account.selected]);
                GameStatus = GAME_STATUS_WAITFORPLAY;

                /* some sanity settings */
                clear_map();
                cpl.name[0] = 0;
                map_udate_flag = 2;
                map_transfer_flag = 1;
            }
            break;

        case SDLK_ESCAPE:
            sound_play_effect(SOUND_PAGE, 0, 0, 100);
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
                    gui_interface_mousemove(&event);
                    break;
                }

            }

//            textwin_event(TW_CHECK_MOVE, &event);

            /* scrollbar-sliders We have to have it before the widgets cause of the menu*/
            if (event.button.button == SDL_BUTTON_LEFT && !draggingInvItem(DRAG_GET_STATUS))
            {
                /* NPC_GUI Slider */
                if (active_scrollbar == 2 || (cpl.menustatus == MENU_NPC && y >= 136+Screensize.yoff && y <=474+Screensize.yoff && x>=561 && x <= 568))
                {
                    active_scrollbar = 2;

                    if (old_mouse_y - y > 0)
                    {
                        gui_interface_npc->yoff +=12;
                        if (gui_interface_npc->yoff >0)
                            gui_interface_npc->yoff=0;
                        if (gui_interface_npc->yoff < INTERFACE_WINLEN_NPC-gui_interface_npc->win_length)
                            gui_interface_npc->yoff = INTERFACE_WINLEN_NPC-gui_interface_npc->win_length;
                        if (gui_interface_npc->yoff >0)
                            gui_interface_npc->yoff=0;
                    }
                    else if (old_mouse_y - y < 0)
                    {
                        gui_interface_npc->yoff -= 12;

                        if (gui_interface_npc->yoff < INTERFACE_WINLEN_NPC-gui_interface_npc->win_length)
                            gui_interface_npc->yoff = INTERFACE_WINLEN_NPC-gui_interface_npc->win_length;
                        if (gui_interface_npc->yoff < INTERFACE_WINLEN_NPC-gui_interface_npc->win_length)
                            gui_interface_npc->yoff = INTERFACE_WINLEN_NPC-gui_interface_npc->win_length;
                        if (gui_interface_npc->yoff >0)
                            gui_interface_npc->yoff=0;
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
            draw_info(tz, COLOR_BLUE |NDI_PLAYER);
            draw_info(tz, COLOR_BLUE);
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
                if (y >= 51 && y <= 74) /* quest list */
                    cpl.menustatus = MENU_NO;
                if (event.button.button ==4 || event.button.button ==5 || event.button.button == SDL_BUTTON_LEFT)
                {
                    gui_interface_mouse(&event);
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
                    int       tx, ty;
                    char      tbuf[32];

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
            else if (cpl.menustatus == MENU_NO && (!InputStringFlag || cpl.input_mode != INPUT_MODE_NUMBER))
            {
#ifdef DEVELOPMENT
                if (widget_mouse_event.owner > -1 && f_custom_cursor == MSCURSOR_MOVE && (event.key.keysym.sym == SDLK_DELETE || event.key.keysym.sym == SDLK_BACKSPACE))
                {
                    switch (widget_mouse_event.owner)
                    {
                        case 12: // PLAYERDOLL
                            if (!options.playerdoll) // Actually this shouldn't be necessary as the widget should already be hidden, but JIC
                            {
                                sound_play_effect(SOUND_SCROLL, 0, 0, MENU_SOUND_VOL);
                                cur_widget[widget_mouse_event.owner].show = FALSE;
                                f_custom_cursor = 0;
                                SDL_ShowCursor(1);
                            }
                            else
                                sound_play_effect(SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
                            break;
                        case 17: // MAININV
                        case 19: // CONSOLE
                        case 20: // NUMBER
                            sound_play_effect(SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
                            break;
                        case 21: // STATOMETER
                            if (!options.statsupdate) // Actually this shouldn't be necessary as the widget should already be hidden, but JIC
                            {
                                sound_play_effect(SOUND_SCROLL, 0, 0, MENU_SOUND_VOL);
                                cur_widget[widget_mouse_event.owner].show = FALSE;
                                f_custom_cursor = 0;
                                SDL_ShowCursor(1);
                            }
                            else
                                sound_play_effect(SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
                            break;
                        default:
                            sound_play_effect(SOUND_SCROLL, 0, 0, MENU_SOUND_VOL);
                            cur_widget[widget_mouse_event.owner].show = FALSE;
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
                    cpl.fire_on = TRUE;
                else
                    cpl.fire_on = FALSE;
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
    char    buf[256];
    if (key->type == SDL_KEYDOWN)
    {
        switch (key->keysym.sym)
        {
        case SDLK_ESCAPE:
            sprintf(buf, "connection closed. select new server.");
            SOCKET_CloseClientSocket(&csocket);
            draw_info(buf, COLOR_RED);
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
            InputStringEscFlag = TRUE;
            return;
            break;

        case SDLK_KP_ENTER:
        case SDLK_RETURN:
        case SDLK_TAB:
            if (key->keysym.sym != SDLK_TAB || GameStatus < GAME_STATUS_WAITFORPLAY)
            {
//                SDL_EnableKeyRepeat(0, SDL_DEFAULT_REPEAT_INTERVAL);
                InputStringFlag = FALSE;
                InputStringEndFlag = TRUE; /* mark that we've got something here */

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
                InputStringFlag = TRUE;

                box.x = gui_interface_npc->startx + 95;
                box.y = gui_interface_npc->starty + 449;
                box.h = 12;
                box.w = 180;

                SDL_FillRect(ScreenSurface, &box, 0);
                StringBlt(ScreenSurface, &SystemFont, show_input_string(InputString, &SystemFont,box.w-10),box.x+5 ,box.y, COLOR_WHITE, NULL, NULL);
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
                InputStringFlag = TRUE;
                box.x = gui_interface_npc->startx + 95;
                box.y = gui_interface_npc->starty + 449;
                box.h = 12;
                box.w = 180;
                SDL_FillRect(ScreenSurface, &box, 0);
                StringBlt(ScreenSurface, &SystemFont, show_input_string(InputString, &SystemFont,box.w-10),box.x+5 ,box.y, COLOR_WHITE, NULL, NULL);
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

        default:
            /* if we are in number console mode, use GET as quick enter
             * mode - this is a very handy shortcut
             */
            if (cpl.input_mode == INPUT_MODE_NUMBER
                    && ((int)key->keysym.sym == get_action_keycode || (int)key->keysym.sym == drop_action_keycode))
            {
//                SDL_EnableKeyRepeat(0, SDL_DEFAULT_REPEAT_INTERVAL);
                InputStringFlag = FALSE;
                InputStringEndFlag = TRUE;/* mark that we got some here*/
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
                if (c < 32 || c == '^' || c == '~' || c == '°' || c == '|' || c == '§')
                    c = 0;
                else
                {
                    if (GameStatus >= GAME_STATUS_LOGIN_ACCOUNT && GameStatus <= GAME_STATUS_LOGIN_NEW)
                    {
                        switch (LoginInputStep)
                        {
                            case LOGIN_STEP_NAME:
                                /* Allow only these chars for account names. */
                                if ((c < 'a' || c > 'z') &&
                                    (c < 'A' || c > 'Z') &&
                                    (c < '0' || c > '9') && 
                                    (c != '-' && c != '_'))
                                    c = 0;
                                break;

                            case LOGIN_STEP_PASS1:
                            case LOGIN_STEP_PASS2:
                                /* Allow full input for passwords, including
                                 * shifted chars. */
                                break;

                            default:
                                c = 0;
                                break;
                        }
                    }
                    else if (GameStatus == GAME_STATUS_ACCOUNT_CHAR_NAME)
                    {
                        /* Allow only these chars for player names. */
                        if ((c < 'a' || c > 'z') &&
                            (c < 'A' || c > 'Z') &&
                            (c != '-' && c != '_'))
                            c = 0;
                        else if (CurrentCursorPos == 0 && (c >= 'a' && c <= 'z'))
                            c = toupper(c);
                        else if (c >= 'A' && c <= 'Z')
                            c = tolower(c);
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
                    sound_play_effect(SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
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
        InputFirstKeyPress = FALSE;
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
            char    buf[256];
            sprintf(buf, "Scancode: %d", key->keysym.sym);
            draw_info(buf, COLOR_RED);
        }

        if (cpl.menustatus != MENU_NO)
        {
            keys[key->keysym.sym].pressed = FALSE;
        }
        else
        {
            keys[key->keysym.sym].pressed = FALSE;
            switch (key->keysym.sym)
            {
            case SDLK_LSHIFT:
            case SDLK_RSHIFT:
                cpl.inventory_win = IWIN_BELOW;
                break;
            case SDLK_LALT:
            case SDLK_RALT:
                send_game_command("/run_stop");
                /*draw_info("run_stop",COLOR_DGOLD);*/
                cpl.run_on = FALSE;
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
                        sound_play_effect(SOUND_SCROLL, 0, 0, 100);
                        strcpy(bindkey_list[bindkey_list_set.group_nr].entry[bindkey_list_set.entry_nr].keyname,
                               SDL_GetKeyName(key->keysym.sym));
                        bindkey_list[bindkey_list_set.group_nr].entry[bindkey_list_set.entry_nr].key = key->keysym.sym;
                    }
                    keybind_status = KEYBIND_STATUS_NO;
                    return(0);
                }
            }
            keys[key->keysym.sym].pressed = TRUE;
            keys[key->keysym.sym].time = LastTick + KEY_REPEAT_TIME_INIT;
            check_menu_keys(cpl.menustatus, key->keysym.sym);
        }
        else
        {
            /* no menu */
            if (esc_menu_flag != TRUE)
            {
                keys[key->keysym.sym].pressed = TRUE;
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
                cpl.run_on = TRUE;
                break;
            case SDLK_RCTRL:
            case SDLK_LCTRL:
                cpl.fire_on = TRUE;

                break;
            case SDLK_ESCAPE:
                if (show_help_screen)
                    show_help_screen = 0;
                if (show_help_screen_new)
                {
                    show_help_screen_new = FALSE;
                    draw_info("QUICK HELP REMINDER", COLOR_GREEN);
                    draw_info("- move with the NUMPAD keys", COLOR_GREEN);
                    draw_info("- target NPC's with the 'S' key", COLOR_GREEN);
                    draw_info("- speak to NPC's with the 'T' key", COLOR_GREEN);
                    draw_info("- hold SHIFT pressed for your INVENTORY", COLOR_GREEN);
                    draw_info("- select an item with use cursor keys", COLOR_GREEN);
                    draw_info("- apply (use) an item with the 'A' key", COLOR_GREEN);
                    draw_info("- examine an item with the 'E' key", COLOR_GREEN);
                    draw_info("- visit the daimonin website: |www.daimonin.net|", COLOR_GREEN);
                    break;
                }
                if (esc_menu_flag == FALSE)
                {
                    map_udate_flag = 1;
                    esc_menu_flag = TRUE;
                    esc_menu_index = ESC_MENU_BACK;
                }
                else
                    esc_menu_flag = FALSE;
                sound_play_effect(SOUND_SCROLL, 0, 0, 100);
                break;

            default:
                if (esc_menu_flag == TRUE)
                    check_esc_menu_keys((int)key->keysym.sym);
                break;
            };
        }
    }
    return(0);
}


/* here we look in the user defined keymap and try to get same useful macros */
Boolean check_menu_macros(char *text)
{
    if (!strcmp("?M_SPELL_LIST", text))
    {
        if (cpl.menustatus == MENU_KEYBIND)
            save_keybind_file(KEYBIND_FILE);
        map_udate_flag = 2;
        if (cpl.menustatus != MENU_SPELL)
        {
            show_help_screen = 0;
            cpl.menustatus = MENU_SPELL;
        }
        else
        {
            cpl.menustatus = MENU_NO;
        }

        sound_play_effect(SOUND_SCROLL, 0, 0, 100);
        reset_keys();
        return TRUE;
    }
    if (!strcmp("?M_SKILL_LIST", text))
    {
        if (cpl.menustatus == MENU_KEYBIND)
            save_keybind_file(KEYBIND_FILE);

        map_udate_flag = 2;
        if (cpl.menustatus != MENU_SKILL)
        {
            show_help_screen = 0;
            cpl.menustatus = MENU_SKILL;
        }
        else
            cpl.menustatus = MENU_NO;
        sound_play_effect(SOUND_SCROLL, 0, 0, 100);
        reset_keys();
        return TRUE;
    }
    if (!strcmp("?M_KEYBIND", text))
    {
        map_udate_flag = 2;
        if (cpl.menustatus != MENU_KEYBIND)
        {
            show_help_screen = 0;
            keybind_status = KEYBIND_STATUS_NO;
            cpl.menustatus = MENU_KEYBIND;
        }
        else
        {
            save_keybind_file(KEYBIND_FILE);
            cpl.menustatus = MENU_NO;
        }
        sound_play_effect(SOUND_SCROLL, 0, 0, 100);
        reset_keys();
        return TRUE;
    }
    if (!strcmp("?M_STATUS", text))
    {
        if (cpl.menustatus == MENU_KEYBIND)
            save_keybind_file(KEYBIND_FILE);

        map_udate_flag = 2;
        if (cpl.menustatus != MENU_STATUS)
        {
            show_help_screen = 0;
            cpl.menustatus = MENU_STATUS;
        }
        else
            cpl.menustatus = MENU_NO;
        sound_play_effect(SOUND_SCROLL, 0, 0, 100);
        reset_keys();
        return TRUE;
    }

    return FALSE;
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
                    return TRUE;
            }
        }
    }
    return  FALSE;
}


void check_keys(int key)
{
    int     i, j;
    char    buf[512];

    for (j = 0; j < BINDKEY_LIST_MAX; j++) /* groups */
    {
        for (i = 0; i < OPTWIN_MAX_OPT; i++)
        {
            if (key == bindkey_list[j].entry[i].key)
            {
                /* if no key macro, submit the text as cmd*/
                if (check_macro_keys(bindkey_list[j].entry[i].text))
                {
                    draw_info(bindkey_list[j].entry[i].text, COLOR_DGOLD);
                    strcpy(buf, bindkey_list[j].entry[i].text);
                    if (!client_command_check(buf))
                        send_game_command(buf);
                }
                return;
            }
        }
    }
}


static Boolean check_macro_keys(char *text)
{
    register int i;
    int magic_len;

    magic_len = strlen(macro_magic_console);
    if (!strncmp(macro_magic_console, text, magic_len) && (int) strlen(text) > magic_len)
    {
        process_macro_keys(KEYFUNC_CONSOLE, 0);
        textwin_putstring(&text[magic_len]);
        return(FALSE);
    }
    for (i = 0; i < (int)DEFAULT_KEYMAP_MACROS; i++)
    {
        if (!strcmp(defkey_macro[i].macro, text))
        {
            if (!process_macro_keys(defkey_macro[i].internal, defkey_macro[i].value))
                return(FALSE);
            return(TRUE);
        }
    }
    return(TRUE);
}


Boolean process_macro_keys(int id, int value)
{
    int     nrof, tag = 0, loc = 0;
    char    buf[256];
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
        sound_play_effect(SOUND_SCROLL, 0, 0, 100);
        if (cpl.menustatus == MENU_KEYBIND)
            save_keybind_file(KEYBIND_FILE);

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
            save_keybind_file(KEYBIND_FILE);

        sound_play_effect(SOUND_SCROLL, 0, 0, 100);
        if (cpl.menustatus != MENU_SKILL)
            cpl.menustatus = MENU_SKILL;
        else
            cpl.menustatus = MENU_NO;
        reset_keys();
        break;
    case KEYFUNC_STATUS:
        map_udate_flag = 2;
        if (cpl.menustatus == MENU_KEYBIND)
            save_keybind_file(KEYBIND_FILE);

        if (cpl.menustatus != MENU_STATUS)
            cpl.menustatus = MENU_STATUS;
        else
            cpl.menustatus = MENU_NO;
        sound_play_effect(SOUND_SCROLL, 0, 0, 0);
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
            save_keybind_file(KEYBIND_FILE);
            cpl.menustatus = MENU_NO;
        }
        sound_play_effect(SOUND_SCROLL, 0, 0, 100);
        reset_keys();
        break;

    case KEYFUNC_CONSOLE:
        show_help_screen = 0;
        sound_play_effect(SOUND_CONSOLE, 0, 0, 100);
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
        if (!(cpl.runkey_on = cpl.runkey_on ? FALSE : TRUE))
            send_game_command("/run_stop");
        sprintf(buf, "runmode %s", cpl.runkey_on ? "on" : "off");
        /*draw_info(buf,COLOR_DGOLD);*/
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
        return FALSE;
        break;

    case KEYFUNC_APPLY:
        if (cpl.inventory_win == IWIN_BELOW)
            tag = cpl.win_below_tag;
        else
            tag = cpl.win_inv_tag;

        if (tag == -1 || !locate_item(tag))
            return FALSE;
        sprintf(buf, "apply %s", locate_item(tag)->s_name);
        draw_info(buf, COLOR_DGOLD);
        client_send_apply(tag);
        return FALSE;
        break;
    case KEYFUNC_EXAMINE:
        if (cpl.inventory_win == IWIN_BELOW)
            tag = cpl.win_below_tag;
        else
            tag = cpl.win_inv_tag;
        if (tag == -1 || !locate_item(tag))
            return FALSE;
        client_send_examine(tag);
        sprintf(buf, "examine %s", locate_item(tag)->s_name);
        draw_info(buf, COLOR_DGOLD);
        return FALSE;
        break;
    case KEYFUNC_MARK:
        if (cpl.inventory_win == IWIN_BELOW)
            tag = cpl.win_below_tag;
        else
            tag = cpl.win_inv_tag;
        if (tag == -1 || !locate_item(tag))
            return FALSE;
        send_mark_obj((it = locate_item(tag)));
        if (it)
        {
            if (cpl.mark_count == (int) it->tag)
                sprintf(buf, "unmark %s", it->s_name);
            else
                sprintf(buf, "mark %s", it->s_name);
            draw_info(buf, COLOR_DGOLD);
        }
        return FALSE;
        break;
    case KEYFUNC_LOCK:
        if (cpl.inventory_win == IWIN_BELOW)
            tag = cpl.win_below_tag;
        else
            tag = cpl.win_inv_tag;
        if (tag == -1 || !locate_item(tag))
            return FALSE;
        toggle_locked((it = locate_item(tag)));
        if (!it)
            return FALSE;
        if (it->locked)
            sprintf(buf, "unlock %s", it->s_name);
        else
            sprintf(buf, "lock %s", it->s_name);
        draw_info(buf, COLOR_DGOLD);
        return FALSE;
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
                            draw_info("you already have it.", COLOR_DGOLD);
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

        if (tag == -1 || !locate_item(tag))
            return FALSE;
        if ((it = locate_item(tag)))
            nrof = it->nrof;
        else
            return FALSE;
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
            return FALSE;
        }
        /*collectAll:*/
        sound_play_effect(SOUND_GET, 0, 0, 100);
        sprintf(buf, "get %s", it->s_name);
        draw_info(buf, COLOR_DGOLD);
        send_inv_move(loc, tag, nrof);
        return FALSE;

        break;

    case KEYFUNC_LAYER0:
        if (debug_layer[0])
            debug_layer[0] = FALSE;
        else
            debug_layer[0] = TRUE;
        sprintf(buf, "debug: map layer 0 %s.\n", debug_layer[0] ? "activated" : "deactivated");
        draw_info(buf, COLOR_DGOLD);
        return FALSE;
        break;
    case KEYFUNC_LAYER1:
        if (debug_layer[1])
            debug_layer[1] = FALSE;
        else
            debug_layer[1] = TRUE;
        sprintf(buf, "debug: map layer 1 %s.\n", debug_layer[1] ? "activated" : "deactivated");
        draw_info(buf, COLOR_DGOLD);
        return FALSE;
        break;
    case KEYFUNC_LAYER2:
        if (debug_layer[2])
            debug_layer[2] = FALSE;
        else
            debug_layer[2] = TRUE;
        sprintf(buf, "debug: map layer 2 %s.\n", debug_layer[2] ? "activated" : "deactivated");
        draw_info(buf, COLOR_DGOLD);
        return FALSE;
        break;
    case KEYFUNC_LAYER3:
        if (debug_layer[3])
            debug_layer[3] = FALSE;
        else
            debug_layer[3] = TRUE;
        sprintf(buf, "debug: map layer 3 %s.\n", debug_layer[3] ? "activated" : "deactivated");
        draw_info(buf, COLOR_DGOLD);
        return FALSE;
        break;

    case KEYFUNC_HELP:
        if (cpl.menustatus == MENU_KEYBIND)
            save_keybind_file(KEYBIND_FILE);

        cpl.menustatus = MENU_NO;
        sound_play_effect(SOUND_SCROLL, 0, 0, 100);
        if (show_help_screen_new)
            show_help_screen_new = FALSE;
        else
            show_help_screen_new = TRUE;


        /*
                cpl.menustatus = MENU_NO;
                sound_play_effect(SOUND_SCROLL, 0, 0, 100);
                if (show_help_screen)
                {
                    if (++show_help_screen > MAX_HELP_SCREEN)
                        show_help_screen = 1;
                }
                else
                    show_help_screen = 1;
        */
        return FALSE;
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
            sprintf(buf, "The item is already on the floor.");
            draw_info(buf, COLOR_DGOLD);
            return FALSE;
        }
        if (tag == -1 || !locate_item(tag))
            return FALSE;
        if ((it = locate_item(tag)))
            nrof = it->nrof;
        else
            return FALSE;

        if (it->locked)
        {
            sound_play_effect(SOUND_CLICKFAIL, 0, 0, 100);
            draw_info("unlock item first!", COLOR_DGOLD);
            return FALSE;
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
            return FALSE;
        }
        sound_play_effect(SOUND_DROP, 0, 0, 100);
        sprintf(buf, "drop %s", it->s_name);
        draw_info(buf, COLOR_DGOLD);
        send_inv_move(loc, tag, nrof);
        return FALSE;
        break;
    case KEYFUNC_SCREENTOGGLE:
        if (!ToggleScreenFlag)
            ToggleScreenFlag = TRUE;
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
    char    buf[256];

    /* put spell into quickslot */
    if (!key && cpl.menustatus == MENU_SPELL)
    {
        if (spell_list[spell_list_set.group_nr].entry[spell_list_set.class_nr][spell_list_set.entry_nr].flag
                == LIST_ENTRY_KNOWN)
        {
            if (quick_slots[slot].shared.is_spell == TRUE && quick_slots[slot].shared.tag == spell_list_set.entry_nr)
            {
                quick_slots[slot].shared.is_spell = FALSE;
                quick_slots[slot].shared.tag = -1;
                sprintf(buf, "unset F%d.", slot + 1);
                draw_info(buf, COLOR_DGOLD);
            }
            else
            {
                quick_slots[slot].shared.is_spell = TRUE;
                quick_slots[slot].spell.groupNr = spell_list_set.group_nr;
                quick_slots[slot].spell.classNr = spell_list_set.class_nr;
                quick_slots[slot].shared.tag = spell_list_set.entry_nr;
                sprintf(buf, "set F%d to %s", slot + 1,
                        spell_list[spell_list_set.group_nr].entry[spell_list_set.class_nr][spell_list_set.entry_nr].name);
                draw_info(buf, COLOR_DGOLD);
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
        quick_slots[slot].shared.is_spell = FALSE;
        if (quick_slots[slot].shared.tag == tag)
            quick_slots[slot].shared.tag = -1;
        else
        {
            quick_slots[slot].shared.tag = tag;
            quick_slots[slot].item.invSlot = cpl.win_inv_slot;
            sprintf(buf, "set F%d to %s", slot + 1, locate_item(tag)->s_name);
            draw_info(buf, COLOR_DGOLD);
        }
    }
    /* apply item or ready spell */
    else
    {
        if (quick_slots[slot].shared.tag != -1)
        {
            if (quick_slots[slot].shared.is_spell == TRUE)
            {
                fire_mode_tab[FIRE_MODE_SPELL].spell = &spell_list[quick_slots[slot].spell.groupNr].entry[quick_slots[slot].spell.classNr][quick_slots[slot].shared.tag];
                RangeFireMode = FIRE_MODE_SPELL;
                spell_list_set.group_nr = quick_slots[slot].spell.groupNr;
                spell_list_set.class_nr = quick_slots[slot].spell.classNr;
                spell_list_set.entry_nr = quick_slots[slot].shared.tag;
                return;
            }
            if (locate_item(quick_slots[slot].shared.tag))
            {
                sprintf(buf, "F%d quick apply %s", slot + 1, locate_item(quick_slots[slot].shared.tag)->s_name);
                draw_info(buf, COLOR_DGOLD);
                client_send_apply(quick_slots[slot].shared.tag);
                return;
            }
        }
        sprintf(buf, "F%d quick slot is empty", slot + 1);
        draw_info(buf, COLOR_DGOLD);
    }
}

static void move_keys(int num)
{
    char    buf[256];
    char    msg[256];

    if (show_help_screen_new)
    {
        draw_info("First close the QUICK HELP with ESC key.", COLOR_WHITE);
        return;
    }

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
        strcpy(buf, "run ");
    }
    /* thats the range menu - we handle it messages unique */
    else if (cpl.firekey_on || cpl.fire_on)
    {
        int itemid = -1;
        char *tmp_name = NULL;

        if (RangeFireMode == FIRE_MODE_SKILL)
        {
            if (!fire_mode_tab[FIRE_MODE_SKILL].skill || fire_mode_tab[FIRE_MODE_SKILL].skill->flag == -1)
            {
                draw_info("no skill selected.", COLOR_WHITE);
                return;
            }
            tmp_name = fire_mode_tab[RangeFireMode].skill->name;

/*            sprintf(buf, "/%s %d %d %s", directionsfire[num], RangeFireMode, -1,
                    fire_mode_tab[RangeFireMode].skill->name);
*/
            sprintf(msg, "use %s %s", fire_mode_tab[RangeFireMode].skill->name, directions_name[num]);
        }
        else if (RangeFireMode == FIRE_MODE_SPELL)
        {
            if (!fire_mode_tab[FIRE_MODE_SPELL].spell || fire_mode_tab[FIRE_MODE_SPELL].spell->flag == -1)
            {
                draw_info("no spell selected.", COLOR_WHITE);
                return;
            }
            tmp_name = fire_mode_tab[RangeFireMode].spell->name;
/*            sprintf(buf, "/%s %d %d %s", directionsfire[num], RangeFireMode, -1,
                    fire_mode_tab[RangeFireMode].spell->name);
*/
            sprintf(msg, "cast %s %s", fire_mode_tab[RangeFireMode].spell->name, directions_name[num]);
        }
        else
        {
            itemid = fire_mode_tab[RangeFireMode].item;
/*
            sprintf(buf, "/%s %d %d %d", directionsfire[num], RangeFireMode, fire_mode_tab[RangeFireMode].item,
                    fire_mode_tab[RangeFireMode].amun);
*/
        }


        if (RangeFireMode == FIRE_MODE_BOW)
        {
            if (fire_mode_tab[FIRE_MODE_BOW].item == FIRE_ITEM_NO)
            {
                draw_info("no range weapon selected.", COLOR_WHITE);
                return;
            }
            sprintf(msg, "fire %s", directions_name[num]);
        }

        /* atm we only use direction, mode and the skill/spell name */
        send_fire_command(num, RangeFireMode, tmp_name);
        draw_info(msg,COLOR_DGOLD);
        return;
    }
    else
    {
        send_move_command(num, 0);
        buf[0] = 0;
    }
    strcat(buf, directions_name[num]);
    /*draw_info(buf,COLOR_DGOLD);*/
}




/******************************************************************
 Handle key repeating.
******************************************************************/
static void key_repeat(void)
{
    register int i,j;
    char    buf[512];

    /* A 'real' menu/dialog or the escape menu: */
    if (cpl.menustatus != MENU_NO || esc_menu_flag == TRUE)
    {
        /* check menu-keys for repeat */
        if (options.menu_repeat > 0 && (SDL_GetTicks() - menuRepeatTicks > menuRepeatTime || !menuRepeatTicks || menuRepeatKey < 0))
        {
            menuRepeatTicks = SDL_GetTicks();
            if (menuRepeatKey >= 0)
            {
                if (esc_menu_flag == TRUE)
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
                                strcpy(buf, bindkey_list[j].entry[i].text);
                                if (!client_command_check(buf))
                                    send_game_command(buf);
                                draw_info(bindkey_list[j].entry[i].text, COLOR_DGOLD);
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
void read_keybind_file(char *fname)
{
    FILE   *stream;
    char    line[256];
    int     i, pos;

    if ((stream = fopen_wrapper(fname, "r")))
    {
        bindkey_list_set.group_nr = -1;
        bindkey_list_set.entry_nr = 0;
        while (fgets(line, 255, stream))
        {
            if (strlen(line) < 4)
                continue; /* skip empty/incomplete lines */
            i = 1;
            /* found key group */
            if (line[0] == '+')
            {
                if (++bindkey_list_set.group_nr == BINDKEY_LIST_MAX)
                    break;
                while (line[++i] && line[i] != '"' && i - 2 < OPTWIN_MAX_TABLEN - 1)
                    bindkey_list[bindkey_list_set.group_nr].name[i - 2] = line[i];
                bindkey_list[bindkey_list_set.group_nr].name[i - 2] = 0;
                bindkey_list_set.entry_nr = 0;
                continue;
            }
            if (bindkey_list_set.group_nr < 0)
                break; /* something is wrong with the file */
            /* found a key entry */
            sscanf(line, " %d %d", &bindkey_list[bindkey_list_set.group_nr].entry[bindkey_list_set.entry_nr].key,
                   &bindkey_list[bindkey_list_set.group_nr].entry[bindkey_list_set.entry_nr].repeatflag);
            pos = 0;
            while (line[++i] && line[i] != '"'); /* start of 1. string */
            while (line[++i] && line[i] != '"')
                bindkey_list[bindkey_list_set.group_nr].entry[bindkey_list_set.entry_nr].keyname[pos++] = line[i];
            bindkey_list[bindkey_list_set.group_nr].entry[bindkey_list_set.entry_nr].keyname[pos] = 0;
            pos = 0;
            while (line[++i] && line[i] != '"'); /* start of 2. string */
            while (line[++i] && line[i] != '"')
                bindkey_list[bindkey_list_set.group_nr].entry[bindkey_list_set.entry_nr].text[pos++] = line[i];
            bindkey_list[bindkey_list_set.group_nr].entry[bindkey_list_set.entry_nr].text[pos] = 0;

            if (!strcmp(bindkey_list[bindkey_list_set.group_nr].entry[bindkey_list_set.entry_nr].text, "?M_GET"))
                get_action_keycode = bindkey_list[bindkey_list_set.group_nr].entry[bindkey_list_set.entry_nr].key;
            if (!strcmp(bindkey_list[bindkey_list_set.group_nr].entry[bindkey_list_set.entry_nr].text, "?M_DROP"))
                drop_action_keycode = bindkey_list[bindkey_list_set.group_nr].entry[bindkey_list_set.entry_nr].key;

            if (++bindkey_list_set.entry_nr == OPTWIN_MAX_OPT)
                break;
        }
        fclose(stream);
    }
    if (bindkey_list_set.group_nr <= 0)
    {
        sprintf(bindkey_list[0].entry[0].keyname, "file %s is corrupt!", fname);
        strcpy(bindkey_list[0].entry[0].text, "°ERROR!°");
        LOG(LOG_ERROR, "ERROR: key-binding file %s is corrupt.\n", fname);
    }
    bindkey_list_set.group_nr = 0;
    bindkey_list_set.entry_nr = 0;
}

/******************************************************************
 Export the key-binding file.
******************************************************************/
void save_keybind_file(char *fname)
{
    FILE   *stream;
    int     entry, group;
    char    buf[256];

    if (!(stream = fopen_wrapper(fname, "w+")))
        return;
    for (group = 0; group < BINDKEY_LIST_MAX; group++)
    {
        if (!bindkey_list[group].name[0])
            continue; /* this group is empty, what about the next one? */
        if (group)
            fputs("\n", stream);
        sprintf(buf, "+\"%s\"\n", bindkey_list[group].name);
        fputs(buf, stream);
        for (entry = 0; entry < OPTWIN_MAX_OPT; entry++)
        {
            if (!bindkey_list[group].entry[entry].text[0])
                continue; /* this entry is empty, what about the next one? */
            /* we need to know for INPUT_MODE_NUMBER "quick get" this key */
            if (!strcmp(bindkey_list[group].entry[entry].text, "?M_GET"))
                get_action_keycode = bindkey_list[group].entry[entry].key;
            if (!strcmp(bindkey_list[group].entry[entry].text, "?M_DROP"))
                drop_action_keycode = bindkey_list[group].entry[entry].key;
            /* save key entry */
            sprintf(buf, "%.3d %d \"%s\" \"%s\"\n", bindkey_list[group].entry[entry].key,
                    bindkey_list[group].entry[entry].repeatflag, bindkey_list[group].entry[entry].keyname,
                    bindkey_list[group].entry[entry].text);
            fputs(buf, stream);
        }
    }
    fclose(stream);
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
            show_help_screen = 0;
            keybind_status = KEYBIND_STATUS_NO;
            cpl.menustatus = MENU_KEYBIND;
        }
        else if (esc_menu_index == ESC_MENU_SETTINGS)
        {
            show_help_screen = 0;
            keybind_status = KEYBIND_STATUS_NO;
            if (cpl.menustatus == MENU_KEYBIND)
                save_keybind_file(KEYBIND_FILE);
            cpl.menustatus = MENU_OPTION;
        }
        else if (esc_menu_index == ESC_MENU_LOGOUT)
        {
            save_quickslots_entrys();
            SOCKET_CloseClientSocket(&csocket);
        }
        sound_play_effect(SOUND_SCROLL, 0, 0, 100);
        esc_menu_flag = FALSE;
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
    int gui_npc_decline = FALSE, gui_npc_accept = FALSE;
    int shiftPressed    = SDL_GetModState() & KMOD_SHIFT;

    if (cpl.menustatus == MENU_NO)
        return;

    /* close menue */
    if (key == SDLK_ESCAPE)
    {
        if (GameStatus == GAME_STATUS_ACCOUNT_CHAR_CREATE || GameStatus == GAME_STATUS_ACCOUNT_CHAR_DEL)
        {
            dialog_new_char_warn = 0;
            reset_input_mode();
            GameStatus = GAME_STATUS_ACCOUNT;
            cpl.menustatus = MENU_NO;
            return;
        }

        if (cpl.menustatus == MENU_KEYBIND)
            save_keybind_file(KEYBIND_FILE);

        cpl.menustatus = MENU_NO;
        map_udate_flag = 2;
        reset_keys();
        return;
    }

    if (check_keys_menu_status(key))
        return;

    switch (menu)
    {
    case MENU_BOOK:
        if (!gui_interface_book || gui_interface_book->pages == 0)
            return;
        switch (key)
        {
        case SDLK_LEFT:
            gui_interface_book->page_show-=2;
            if (gui_interface_book->page_show<0)
            {
                gui_interface_book->page_show=0;
                sound_play_effect(SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
            }
            else
            {
                sound_play_effect(SOUND_PAGE, 0, 0, MENU_SOUND_VOL);
            }
            break;
        case SDLK_RIGHT:
            gui_interface_book->page_show+=2;
            if (gui_interface_book->page_show>(gui_interface_book->pages-1))
            {
                gui_interface_book->page_show=(gui_interface_book->pages-1)&~1;
                sound_play_effect(SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
            }
            else
            {
                sound_play_effect(SOUND_PAGE, 0, 0, MENU_SOUND_VOL);
            }
            break;
        }
        break;

    case MENU_NPC:

        if (gui_interface_npc->status == GUI_INTERFACE_STATUS_WAIT)
            return;

        switch (key)
        {
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            if (gui_interface_npc->link_selected)
            {
                gui_interface_send_command(1, gui_interface_npc->link[gui_interface_npc->link_selected-1].cmd);
                /*
                send_command(gui_interface_npc->link[gui_interface_npc->link_selected-1].cmd);
                draw_info_format(COLOR_WHITE, "Talking about: %s", gui_interface_npc->link[gui_interface_npc->link_selected-1].link);
                */
                gui_interface_npc->link_selected=0;
                sound_play_effect(SOUND_SCROLL, 0, 0, MENU_SOUND_VOL);
                reset_input_mode();
                break;
            }
            if (gui_interface_npc->keyword_selected)
            {
                gui_interface_send_command(0, gui_interface_npc->keywords[gui_interface_npc->keyword_selected-1]);
                /*
                send_command(gui_interface_npc->link[gui_interface_npc->link_selected-1].cmd);
                draw_info_format(COLOR_WHITE, "Talking about: %s", gui_interface_npc->link[gui_interface_npc->link_selected-1].link);
                */
                gui_interface_npc->keyword_selected=0;
                sound_play_effect(SOUND_SCROLL, 0, 0, MENU_SOUND_VOL);
                reset_input_mode();
                break;
            }
            /* disable quest tag (ugly code...) */
            if (gui_interface_npc->who.body[0] == 'Q')
                break;

            reset_keys();
            reset_input_mode();
            open_input_mode(240);
            textwin_putstring("");
            cpl.input_mode = INPUT_MODE_NPCDIALOG;
            gui_interface_npc->input_flag = TRUE;
            sound_play_effect(SOUND_SCROLL, 0, 0, MENU_SOUND_VOL);
            HistoryPos = 0;
            break;

        case SDLK_a:
            if (gui_interface_npc->used_flag&GUI_INTERFACE_DECLINE && gui_interface_npc->decline.title[0]=='A')
                gui_npc_decline = TRUE;
            else if (gui_interface_npc->used_flag&GUI_INTERFACE_ACCEPT && gui_interface_npc->accept.title[0]=='A')
                gui_npc_accept = TRUE;
            break;
        case SDLK_b:
            if (gui_interface_npc->used_flag&GUI_INTERFACE_DECLINE && gui_interface_npc->decline.title[0]=='B')
                gui_npc_decline = TRUE;
            else if (gui_interface_npc->used_flag&GUI_INTERFACE_ACCEPT && gui_interface_npc->accept.title[0]=='B')
                gui_npc_accept = TRUE;
            break;
        case SDLK_c:
            if (gui_interface_npc->used_flag&GUI_INTERFACE_DECLINE && gui_interface_npc->decline.title[0]=='C')
                gui_npc_decline = TRUE;
            else if (gui_interface_npc->used_flag&GUI_INTERFACE_ACCEPT && gui_interface_npc->accept.title[0]=='C')
                gui_npc_accept = TRUE;
            break;
        case SDLK_d:
            if (gui_interface_npc->used_flag&GUI_INTERFACE_DECLINE && gui_interface_npc->decline.title[0]=='D')
                gui_npc_decline = TRUE;
            else if (gui_interface_npc->used_flag&GUI_INTERFACE_ACCEPT && gui_interface_npc->accept.title[0]=='D')
                gui_npc_accept = TRUE;
            break;
        case SDLK_e:
            if (gui_interface_npc->used_flag&GUI_INTERFACE_DECLINE && gui_interface_npc->decline.title[0]=='E')
                gui_npc_decline = TRUE;
            else if (gui_interface_npc->used_flag&GUI_INTERFACE_ACCEPT && gui_interface_npc->accept.title[0]=='E')
                gui_npc_accept = TRUE;
            break;
        case SDLK_f:
            if (gui_interface_npc->used_flag&GUI_INTERFACE_DECLINE && gui_interface_npc->decline.title[0]=='F')
                gui_npc_decline = TRUE;
            else if (gui_interface_npc->used_flag&GUI_INTERFACE_ACCEPT && gui_interface_npc->accept.title[0]=='F')
                gui_npc_accept = TRUE;
            break;
        case SDLK_g:
            if (gui_interface_npc->used_flag&GUI_INTERFACE_DECLINE && gui_interface_npc->decline.title[0]=='G')
                gui_npc_decline = TRUE;
            else if (gui_interface_npc->used_flag&GUI_INTERFACE_ACCEPT && gui_interface_npc->accept.title[0]=='G')
                gui_npc_accept = TRUE;
            break;
        case SDLK_h:
            if (gui_interface_npc->used_flag&GUI_INTERFACE_DECLINE && gui_interface_npc->decline.title[0]=='H')
                gui_npc_decline = TRUE;
            else if (gui_interface_npc->used_flag&GUI_INTERFACE_ACCEPT && gui_interface_npc->accept.title[0]=='H')
                gui_npc_accept = TRUE;
            break;
        case SDLK_i:
            if (gui_interface_npc->used_flag&GUI_INTERFACE_DECLINE && gui_interface_npc->decline.title[0]=='I')
                gui_npc_decline = TRUE;
            else if (gui_interface_npc->used_flag&GUI_INTERFACE_ACCEPT && gui_interface_npc->accept.title[0]=='I')
                gui_npc_accept = TRUE;
            break;
        case SDLK_j:
            if (gui_interface_npc->used_flag&GUI_INTERFACE_DECLINE && gui_interface_npc->decline.title[0]=='J')
                gui_npc_decline = TRUE;
            else if (gui_interface_npc->used_flag&GUI_INTERFACE_ACCEPT && gui_interface_npc->accept.title[0]=='J')
                gui_npc_accept = TRUE;
            break;
        case SDLK_k:
            if (gui_interface_npc->used_flag&GUI_INTERFACE_DECLINE && gui_interface_npc->decline.title[0]=='K')
                gui_npc_decline = TRUE;
            else if (gui_interface_npc->used_flag&GUI_INTERFACE_ACCEPT && gui_interface_npc->accept.title[0]=='K')
                gui_npc_accept = TRUE;
            break;
        case SDLK_l:
            if (gui_interface_npc->used_flag&GUI_INTERFACE_DECLINE && gui_interface_npc->decline.title[0]=='L')
                gui_npc_decline = TRUE;
            else if (gui_interface_npc->used_flag&GUI_INTERFACE_ACCEPT && gui_interface_npc->accept.title[0]=='L')
                gui_npc_accept = TRUE;
            break;
        case SDLK_m:
            if (gui_interface_npc->used_flag&GUI_INTERFACE_DECLINE && gui_interface_npc->decline.title[0]=='M')
                gui_npc_decline = TRUE;
            else if (gui_interface_npc->used_flag&GUI_INTERFACE_ACCEPT && gui_interface_npc->accept.title[0]=='M')
                gui_npc_accept = TRUE;
            break;
        case SDLK_n:
            if (gui_interface_npc->used_flag&GUI_INTERFACE_DECLINE && gui_interface_npc->decline.title[0]=='N')
                gui_npc_decline = TRUE;
            else if (gui_interface_npc->used_flag&GUI_INTERFACE_ACCEPT && gui_interface_npc->accept.title[0]=='N')
                gui_npc_accept = TRUE;
            break;
        case SDLK_o:
            if (gui_interface_npc->used_flag&GUI_INTERFACE_DECLINE && gui_interface_npc->decline.title[0]=='O')
                gui_npc_decline = TRUE;
            else if (gui_interface_npc->used_flag&GUI_INTERFACE_ACCEPT && gui_interface_npc->accept.title[0]=='O')
                gui_npc_accept = TRUE;
            break;
        case SDLK_p:
            if (gui_interface_npc->used_flag&GUI_INTERFACE_DECLINE && gui_interface_npc->decline.title[0]=='P')
                gui_npc_decline = TRUE;
            else if (gui_interface_npc->used_flag&GUI_INTERFACE_ACCEPT && gui_interface_npc->accept.title[0]=='P')
                gui_npc_accept = TRUE;
            break;
        case SDLK_q:
            if (gui_interface_npc->used_flag&GUI_INTERFACE_DECLINE && gui_interface_npc->decline.title[0]=='Q')
                gui_npc_decline = TRUE;
            else if (gui_interface_npc->used_flag&GUI_INTERFACE_ACCEPT && gui_interface_npc->accept.title[0]=='Q')
                gui_npc_accept = TRUE;
            break;
        case SDLK_r:
            if (gui_interface_npc->used_flag&GUI_INTERFACE_DECLINE && gui_interface_npc->decline.title[0]=='R')
                gui_npc_decline = TRUE;
            else if (gui_interface_npc->used_flag&GUI_INTERFACE_ACCEPT && gui_interface_npc->accept.title[0]=='R')
                gui_npc_accept = TRUE;
            break;
        case SDLK_s:
            if (gui_interface_npc->used_flag&GUI_INTERFACE_DECLINE && gui_interface_npc->decline.title[0]=='S')
                gui_npc_decline = TRUE;
            else if (gui_interface_npc->used_flag&GUI_INTERFACE_ACCEPT && gui_interface_npc->accept.title[0]=='S')
                gui_npc_accept = TRUE;
            break;
        case SDLK_t:
            if (gui_interface_npc->used_flag&GUI_INTERFACE_DECLINE && gui_interface_npc->decline.title[0]=='T')
                gui_npc_decline = TRUE;
            else if (gui_interface_npc->used_flag&GUI_INTERFACE_ACCEPT && gui_interface_npc->accept.title[0]=='T')
                gui_npc_accept = TRUE;
            break;
        case SDLK_u:
            if (gui_interface_npc->used_flag&GUI_INTERFACE_DECLINE && gui_interface_npc->decline.title[0]=='U')
                gui_npc_decline = TRUE;
            else if (gui_interface_npc->used_flag&GUI_INTERFACE_ACCEPT && gui_interface_npc->accept.title[0]=='U')
                gui_npc_accept = TRUE;
            break;
        case SDLK_v:
            if (gui_interface_npc->used_flag&GUI_INTERFACE_DECLINE && gui_interface_npc->decline.title[0]=='V')
                gui_npc_decline = TRUE;
            else if (gui_interface_npc->used_flag&GUI_INTERFACE_ACCEPT && gui_interface_npc->accept.title[0]=='V')
                gui_npc_accept = TRUE;
            break;
        case SDLK_w:
            if (gui_interface_npc->used_flag&GUI_INTERFACE_DECLINE && gui_interface_npc->decline.title[0]=='W')
                gui_npc_decline = TRUE;
            else if (gui_interface_npc->used_flag&GUI_INTERFACE_ACCEPT && gui_interface_npc->accept.title[0]=='W')
                gui_npc_accept = TRUE;
            break;
        case SDLK_x:
            if (gui_interface_npc->used_flag&GUI_INTERFACE_DECLINE && gui_interface_npc->decline.title[0]=='X')
                gui_npc_decline = TRUE;
            else if (gui_interface_npc->used_flag&GUI_INTERFACE_ACCEPT && gui_interface_npc->accept.title[0]=='X')
                gui_npc_accept = TRUE;
            break;
        case SDLK_y:
            if (gui_interface_npc->used_flag&GUI_INTERFACE_DECLINE && gui_interface_npc->decline.title[0]=='Y')
                gui_npc_decline = TRUE;
            else if (gui_interface_npc->used_flag&GUI_INTERFACE_ACCEPT && gui_interface_npc->accept.title[0]=='Y')
                gui_npc_accept = TRUE;
            break;
        case SDLK_z:
            if (gui_interface_npc->used_flag&GUI_INTERFACE_DECLINE && gui_interface_npc->decline.title[0]=='Z')
                gui_npc_decline = TRUE;
            else if (gui_interface_npc->used_flag&GUI_INTERFACE_ACCEPT && gui_interface_npc->accept.title[0]=='Z')
                gui_npc_accept = TRUE;
            break;

        case SDLK_TAB:
            if ((gui_interface_npc->link_count) || (gui_interface_npc->keyword_count))
            {
                if (shiftPressed)
                {
                    if (!gui_interface_npc->link_selected && !gui_interface_npc->keyword_selected)
                    {
                        if (!(gui_interface_npc->link_selected=gui_interface_npc->link_count))
                            gui_interface_npc->keyword_selected=gui_interface_npc->keyword_count;
                    }
                    else if(gui_interface_npc->link_selected)
                    {
                        if (!(--gui_interface_npc->link_selected))
                            gui_interface_npc->keyword_selected=gui_interface_npc->keyword_count;
                    }
                    else
                    {
                        if (--gui_interface_npc->keyword_selected<0)
                            gui_interface_npc->link_selected=gui_interface_npc->link_count;
                    }
                }
                else
                {
                    if (!gui_interface_npc->link_selected && !gui_interface_npc->keyword_selected)
                    {
                        if (!(gui_interface_npc->keyword_count))
                            gui_interface_npc->link_selected++;
                        else
                            gui_interface_npc->keyword_selected++;
                    }
                    else if(gui_interface_npc->keyword_selected)
                    {
                        if(++gui_interface_npc->keyword_selected>gui_interface_npc->keyword_count)
                        {
                            gui_interface_npc->keyword_selected=0;
                            if (gui_interface_npc->link_count)
                                gui_interface_npc->link_selected=1;
                        }
                    }
                    else
                    {
                        if (++gui_interface_npc->link_selected>gui_interface_npc->link_count)
                            gui_interface_npc->link_selected=0;
                    }
                }
                sound_play_effect(SOUND_GET, 0, 0, 100);
            }
            else
                sound_play_effect(SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
            break;
            /* select prize */
        case SDLK_DOWN:
            if (gui_interface_npc->icon_count > 1)
            {
                int i = gui_interface_npc->selected;
                do
                {
                    if (++i >gui_interface_npc->icon_count)
                    {
                        i = 1;
                    }
                    if (gui_interface_npc->icon[i-1].mode == 'S' )
                    {
                        gui_interface_npc->selected = i;
                        sound_play_effect(SOUND_GET, 0, 0, 100);
                        break;
                    }
                }
                while ((i!=gui_interface_npc->selected) && (i!=gui_interface_npc->icon_count));
            }
            break;

        case SDLK_UP:
            if (gui_interface_npc->icon_count > 1)
            {
                int i = gui_interface_npc->selected;
                do
                {
                    if (--i < 1)
                        i = gui_interface_npc->icon_count;
                    if (gui_interface_npc->icon[i-1].mode == 'S' )
                    {
                        gui_interface_npc->selected = i;
                        sound_play_effect(SOUND_GET, 0, 0, MENU_SOUND_VOL);
                        break;
                    }
                }
                while ((i!=gui_interface_npc->selected) && (i!=gui_interface_npc->icon_count));
            }
            break;

            /* key scroll up/down */
        case SDLK_PAGEUP:
            gui_interface_npc->yoff +=12;
            if (gui_interface_npc->yoff >0)
            {
                gui_interface_npc->yoff=0;
                if (menuRepeatKey != SDLK_PAGEUP)
                    sound_play_effect(SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
            }
            menuRepeatKey = SDLK_PAGEUP;
            if (gui_interface_npc->yoff < INTERFACE_WINLEN_NPC-gui_interface_npc->win_length)
            {
                gui_interface_npc->yoff = INTERFACE_WINLEN_NPC-gui_interface_npc->win_length;
            }
            if (gui_interface_npc->yoff >0)
            {
                gui_interface_npc->yoff=0;
            }
            break;
        case SDLK_PAGEDOWN:
            gui_interface_npc->yoff -= 12;

            if (gui_interface_npc->yoff < INTERFACE_WINLEN_NPC-gui_interface_npc->win_length)
            {
                gui_interface_npc->yoff = INTERFACE_WINLEN_NPC-gui_interface_npc->win_length;
                if (menuRepeatKey != SDLK_PAGEDOWN)
                    sound_play_effect(SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
            }
            menuRepeatKey = SDLK_PAGEDOWN;
            if (gui_interface_npc->yoff < INTERFACE_WINLEN_NPC-gui_interface_npc->win_length)
            {
                gui_interface_npc->yoff = INTERFACE_WINLEN_NPC-gui_interface_npc->win_length;
            }
            if (gui_interface_npc->yoff >0)
            {
                gui_interface_npc->yoff=0;
            }
            break;

        }

        /* lets check we have a named button pressed or clicked inside the npc gui */
        if (gui_npc_accept)
        {
            sound_play_effect(SOUND_SCROLL, 0, 0, 100);
            if (gui_interface_npc->accept.command[0]!='\0')
            {
                char cmd[1024];
                /* check selected for possible slot selection */
                if (gui_interface_npc->icon_select)
                {
                    if (gui_interface_npc->accept.command[6] == '#')
                        sprintf(cmd, "/talk %s #%d", gui_interface_npc->accept.command + 7, gui_interface_npc->selected);
                    else
                        sprintf(cmd, "%s #%d", gui_interface_npc->accept.command, gui_interface_npc->selected);
                }
                else
                {
                    strcpy(cmd, gui_interface_npc->accept.command);
                    if (gui_interface_npc->accept.command[6] == '#')
                        sprintf(cmd, "/talk %s", gui_interface_npc->accept.command + 7);
                    else
                        sprintf(cmd, "%s", gui_interface_npc->accept.command);
                }
                gui_interface_send_command(1, cmd);
            }
            else
                reset_gui_interface();
        }
        else if (gui_npc_decline)
        {
            sound_play_effect(SOUND_SCROLL, 0, 0, 100);
            if (gui_interface_npc->decline.command[0]!='\0')
            {
                char cmd[1024];
                /* check selected for possible slot selection */
                if (gui_interface_npc->icon_select && gui_interface_npc->decline.command[6] == '#')
                    sprintf(cmd, "/talk %s #%d", gui_interface_npc->decline.command + 7, gui_interface_npc->selected);
                else
                    strcpy(cmd, gui_interface_npc->decline.command);
                gui_interface_send_command(1, cmd);
            }
            else
                reset_gui_interface();
        }
    break;

    case MENU_OPTION:
        switch (key)
        {
        case SDLK_LEFT:
            option_list_set.key_change = -1;
            /*sound_play_effect(SOUND_SCROLL,0,0,MENU_SOUND_VOL);*/
            menuRepeatKey = SDLK_LEFT;
            break;
        case SDLK_RIGHT:
            option_list_set.key_change = 1;
            /*sound_play_effect(SOUND_SCROLL,0,0,MENU_SOUND_VOL);*/
            menuRepeatKey = SDLK_RIGHT;
            break;
        case SDLK_UP:
            if (!shiftPressed)
            {
                if (option_list_set.entry_nr > 0)
                    option_list_set.entry_nr--;
                else
                    sound_play_effect(SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
            }
            else
            {
                if (option_list_set.group_nr > 0)
                {
                    option_list_set.group_nr--;
                    option_list_set.entry_nr = 0;
                }
                else
                    sound_play_effect(SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
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
                    sound_play_effect(SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
            }
            else
            {
                if (opt_tab[option_list_set.group_nr + 1])
                {
                    option_list_set.group_nr++;
                    option_list_set.entry_nr = 0;
                }
                else
                    sound_play_effect(SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
            }
            menuRepeatKey = SDLK_DOWN;
            break;
        case SDLK_d:
            sound_play_effect(SOUND_SCROLL, 0, 0, MENU_SOUND_VOL);
            map_udate_flag = 2;
            if (cpl.menustatus == MENU_KEYBIND)
                save_keybind_file(KEYBIND_FILE);
            if (cpl.menustatus == MENU_OPTION)
            {
                Boolean res_change = FALSE;
                save_options_dat();
                Mix_VolumeMusic(options.music_volume);
                if (options.playerdoll)
                    cur_widget[PDOLL_ID].show = TRUE;

                /* ToggleScreenFlag sets changes this option in the main loop
                 * so we revert this setting, also if a resolution change occurs
                 * we first change the resolution, without toggling, and after that
                 * succeeds we use the specialized attempt_fullscreentoggle
                 */
                if (options.fullscreen_flag != options.fullscreen)
                {
                    if (options.fullscreen)
                        options.fullscreen = FALSE;
                    else
                        options.fullscreen = TRUE;
                    ToggleScreenFlag = TRUE;
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
                        draw_info_format(COLOR_RED, "Couldn't set %dx%dx%d video mode: %s\n", Screensize.x, Screensize.y, options.used_video_bpp, SDL_GetError());
                        LOG(LOG_ERROR, "Couldn't set %dx%dx%d video mode: %s\n", Screensize.x, Screensize.y, options.used_video_bpp, SDL_GetError());
                        Screensize=sz_tmp;
                        for (i=0;i<16;i++)
                            if (Screensize.x==Screendefs[i].x && Screensize.y == Screendefs[i].y)
                            {
                                options.resolution = i;
                                break;
                            }
                        draw_info_format(COLOR_RED, "Try to switch back to old setting...");
                        LOG(LOG_ERROR, "Try to switch back to old setting...\n");

                        if ((ScreenSurface = SDL_SetVideoMode(Screensize.x, Screensize.y, options.used_video_bpp, videoflags)) == NULL)
                        {
                            draw_info_format(COLOR_RED, "Couldn't set %dx%dx%d video mode: %s\n", Screensize.x, Screensize.y, options.used_video_bpp, SDL_GetError());
                            LOG(LOG_ERROR, "Couldn't set %dx%dx%d video mode: %s\n", Screensize.x, Screensize.y, options.used_video_bpp, SDL_GetError());
                            Screensize=Screendefs[0];
                            options.resolution = 0;
                            draw_info_format(COLOR_RED, "Try to switch back to 800x600...");
                            LOG(LOG_ERROR, "Try to switch back to 800x600...\n");
                            if ((ScreenSurface = SDL_SetVideoMode(Screensize.x, Screensize.y, options.used_video_bpp, videoflags)) == NULL)
                            {
                                /* now we have a problem */
                                draw_info_format(COLOR_RED, "Couldn't set %dx%dx%d video mode: %s\nFATAL ERROR - exit", Screensize.x, Screensize.y, options.used_video_bpp, SDL_GetError());
                                LOG(LOG_ERROR, "Couldn't set %dx%dx%d video mode: %s\nFATAL ERROR - exit", Screensize.x, Screensize.y, options.used_video_bpp, SDL_GetError());
                                Screensize=sz_tmp;
                                exit(2);
                            }
                            else
                                res_change = TRUE;
                        }
                        else
                            res_change = TRUE;
                    }
                    else
                        res_change = TRUE;
                }
                if (res_change)
                {
                    const SDL_VideoInfo    *info    = NULL;
                    info = SDL_GetVideoInfo();
                    options.real_video_bpp = info->vfmt->BitsPerPixel;
//                    SDL_FreeSurface(ScreenSurfaceMap);
//                    ScreenSurfaceMap=SDL_CreateRGBSurface(ScreenSurface->flags, Screensize.x, Screensize.y, options.used_video_bpp, 0,0,0,0);

                }
            }
            cpl.menustatus = MENU_NO;
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
                sound_play_effect(SOUND_SCROLL, 0, 0, MENU_SOUND_VOL);
            }
            else
                sound_play_effect(SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
            map_udate_flag = 2;
            cpl.menustatus = MENU_NO;
            reset_keys();
            break;
        case SDLK_UP:
            if (!shiftPressed)
            {
                if (skill_list_set.entry_nr > 0)
                {
                    if (skill_list[skill_list_set.group_nr].entry[--skill_list_set.entry_nr].flag == LIST_ENTRY_KNOWN)
                        sound_play_effect(SOUND_SCROLL, 0, 0, MENU_SOUND_VOL);
                }
                else
                    sound_play_effect(SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
            }
            else
            {
                if (skill_list_set.group_nr > 0)
                {
                    skill_list_set.group_nr--;
                    skill_list_set.entry_nr = 0;
                }
                else
                    sound_play_effect(SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
            }
            menuRepeatKey = SDLK_UP;
            break;
        case SDLK_DOWN:
            if (!shiftPressed)
            {
                if (skill_list_set.entry_nr < DIALOG_LIST_ENTRY - 1)
                {
                    if (skill_list[skill_list_set.group_nr].entry[++skill_list_set.entry_nr].flag == LIST_ENTRY_KNOWN)
                        sound_play_effect(SOUND_SCROLL, 0, 0, MENU_SOUND_VOL);
                }
                else
                    sound_play_effect(SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
            }
            else
            {
                if (skill_list_set.group_nr < SKILL_LIST_MAX - 1)
                {
                    skill_list_set.group_nr++;
                    skill_list_set.entry_nr = 0;
                }
                else
                    sound_play_effect(SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
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
                sound_play_effect(SOUND_SCROLL, 0, 0, MENU_SOUND_VOL);
            }
            else
                sound_play_effect(SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
            map_udate_flag = 2;
            cpl.menustatus = MENU_NO;
            reset_keys();
            break;
        case SDLK_LEFT:
            if (spell_list_set.class_nr > 0)
            {
                spell_list_set.class_nr--;
                sound_play_effect(SOUND_SCROLL, 0, 0, MENU_SOUND_VOL);
            }
            else
                sound_play_effect(SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
            break;
        case SDLK_RIGHT:
            if (spell_list_set.class_nr < SPELL_LIST_CLASS - 1)
            {
                spell_list_set.class_nr++;
                sound_play_effect(SOUND_SCROLL, 0, 0, MENU_SOUND_VOL);
            }
            else
                sound_play_effect(SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
            break;
        case SDLK_UP:
            if (!shiftPressed)
            {
                if (spell_list_set.entry_nr > 0)
                {
                    if (spell_list[spell_list_set.group_nr].entry[spell_list_set.class_nr][--spell_list_set.entry_nr].flag == LIST_ENTRY_KNOWN)
                        sound_play_effect(SOUND_SCROLL, 0, 0, MENU_SOUND_VOL);
                }
                else
                    sound_play_effect(SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
            }
            else
            {
                if (spell_list_set.group_nr > 0)
                {
                    spell_list_set.group_nr--;
                    spell_list_set.entry_nr = 0;
                }
                else
                    sound_play_effect(SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
            }
            menuRepeatKey = SDLK_UP;
            break;
        case SDLK_DOWN:
            if (!shiftPressed)
            {
                if (spell_list_set.entry_nr < DIALOG_LIST_ENTRY - 1)
                {
                    if (spell_list[spell_list_set.group_nr].entry[spell_list_set.class_nr][++spell_list_set.entry_nr].flag == LIST_ENTRY_KNOWN)
                        sound_play_effect(SOUND_SCROLL, 0, 0, MENU_SOUND_VOL);
                }
                else
                    sound_play_effect(SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
            }
            else
            {
                if (spell_list_set.group_nr < SPELL_LIST_MAX - 1)
                {
                    spell_list_set.group_nr++;
                    spell_list_set.entry_nr = 0;
                }
                else
                    sound_play_effect(SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
            }
            menuRepeatKey = SDLK_DOWN;
            break;
        }
        break;

    case MENU_KEYBIND:
        switch (key)
        {
        case SDLK_UP:
            if (!shiftPressed)
            {
                if (bindkey_list_set.entry_nr > 0)
                {
                    if (bindkey_list[bindkey_list_set.group_nr].entry[--bindkey_list_set.entry_nr].text[0])
                        sound_play_effect(SOUND_SCROLL, 0, 0, MENU_SOUND_VOL);
                }
                else
                    sound_play_effect(SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
            }
            else
            {
                if (bindkey_list_set.group_nr > 0)
                {
                    bindkey_list_set.group_nr--;
                    bindkey_list_set.entry_nr = 0;
                }
                else
                    sound_play_effect(SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
            }
            menuRepeatKey = SDLK_UP;
            break;
        case SDLK_DOWN:
            if (!shiftPressed)
            {
                if (bindkey_list_set.entry_nr < OPTWIN_MAX_OPT - 1)
                {
                    if (bindkey_list[bindkey_list_set.group_nr].entry[++bindkey_list_set.entry_nr].text[0])
                        sound_play_effect(SOUND_SCROLL, 0, 0, MENU_SOUND_VOL);
                }
                else
                    sound_play_effect(SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
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
                    sound_play_effect(SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);
            }
            menuRepeatKey = SDLK_DOWN;
            break;
        case SDLK_d:
            save_keybind_file(KEYBIND_FILE);
            sound_play_effect(SOUND_SCROLL, 0, 0, MENU_SOUND_VOL);
            map_udate_flag = 2;
            cpl.menustatus = MENU_NO;
            reset_keys();
            sound_play_effect(SOUND_SCROLL, 0, 0, MENU_SOUND_VOL);
            break;
        case SDLK_RETURN:
            sound_play_effect(SOUND_SCROLL, 0, 0, MENU_SOUND_VOL);
            keybind_status = KEYBIND_STATUS_EDIT;
            reset_keys();
            open_input_mode(240);
            textwin_putstring(bindkey_list[bindkey_list_set.group_nr].entry[bindkey_list_set.entry_nr].text);
            cpl.input_mode = INPUT_MODE_GETKEY;
            sound_play_effect(SOUND_SCROLL, 0, 0, MENU_SOUND_VOL);
            break;
        case SDLK_r:
            sound_play_effect(SOUND_SCROLL, 0, 0, MENU_SOUND_VOL);
            bindkey_list[bindkey_list_set.group_nr].entry[bindkey_list_set.entry_nr].repeatflag = bindkey_list[bindkey_list_set.group_nr].entry[bindkey_list_set.entry_nr].repeatflag
                    ? FALSE
                    : TRUE;
            sound_play_effect(SOUND_SCROLL, 0, 0, MENU_SOUND_VOL);
            break;
        }
        break;

    case MENU_CREATE:
        switch (key)
        {
        case SDLK_RETURN:
            break;
        case SDLK_ESCAPE:
            /* is handled in main menu handler */
            break;
        case SDLK_n:
            if (new_character.skill_selected == -1)
            {
                dialog_new_char_warn = 2;
                sound_play_effect(SOUND_CLICKFAIL, 0, 0, 100);
                break;
            }
            dialog_new_char_warn = 0;
            reset_input_mode();
            cpl.name[0] = 0;  
            InputStringFlag=TRUE;
            InputStringEndFlag=FALSE;
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
            sound_play_effect(SOUND_SCROLL, 0, 0, MENU_SOUND_VOL);
            menuRepeatKey = -1;
            break;
        case SDLK_DOWN:
            if (++create_list_set.entry_nr >= 2)
                create_list_set.entry_nr = 0;
            sound_play_effect(SOUND_SCROLL, 0, 0, MENU_SOUND_VOL);
            menuRepeatKey = -1;
            break;
        }
        break;
    }
}

