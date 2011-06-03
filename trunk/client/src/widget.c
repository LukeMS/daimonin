/************************************************************************/
/*
    Daimonin SDL client, a client program for the Daimonin MMORPG.

    Copyright widget.c (C) 2005 - John Swenson
    Modification by Alderan

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

    The author can be reached via e-mail to jonsebox28@hotmail.com
*/

/* A widget is like a simple window on your desktop. Each widget displays
 * certain specific data (eg, your stats in WIDGET_STATS_ID, and your resistances in
 * WIDGET_RESIST_ID). The exact layout of the widgets are partly determined through
 * code and partly through the current skin (in future this balance will be
 * tipped further in the direction of the skin).
 *
 * Currently widgets are of fixed dimensions and (with exceptions) are always
 * shown, but greater control is in development.
 *
 * By right-clicking and dragging a widget, the player can move it around the
 * screen.
 *
 * To add a new widget:
 *   1) Add an entry (same index in both cases) to DefaultData[] and
 *      widget_id_t.
 *   2) If applicable, add handler code for widget movement in
 *      widget_event_mousedn().
 *   3) If applicable, add handler code to widget_get_owner().
 *   4) Add handler code to widget_process().
 * -- Smacky 20110603 */

#include "include.h"

/* Current (working) data-list of all widgets */
widget_data_t widget_data[WIDGET_NROF];

/* Surfaces for all widgets. */
SDL_Surface *widget_surface[WIDGET_NROF] = { NULL };

/* Determines which widget has mouse focus */
/* This value is determined in the mouse routines for the widgets */
widget_event_t widget_mouse_event = { 0, 0, 0, 0, 0, 0 };

/* Default (default) data-list of all widgets */
/* {name,priority_index,x1,y1,width,height,moveable?, active?} */
static const widget_data_t DefaultData[WIDGET_NROF] =
{
    { "STATS",       NULL, 227, 0,   172, 102, 1, 1, 1 },
    { "RESIST",      NULL, 497, 0,   198, 79,  1, 1, 1 },
    { "MAIN_LVL",    NULL, 399, 39,  98,  62,  1, 1, 1 },
    { "SKILL_EXP",   NULL, 497, 79,  198, 22,  1, 1, 1 },
    { "REGEN",       NULL, 399, 0,   98,  39,  1, 1, 1 },
    { "SKILL_LVL",   NULL, 695, 0,   52,  101, 1, 1, 1 },
    { "MENUBUTTONS", NULL, 747, 0,   47,  101, 1, 1, 1 },
    { "QUICKSLOTS",  NULL, 513, 107, 282, 34,  1, 1, 1 },
    { "CHATWIN",     NULL, 0,   366, 261, 233, 1, 1, 1 },
    { "MSGWIN",      NULL, 537, 366, 261, 233, 1, 1, 1 },
    { "MIXWIN",      NULL, 539, 420, 261, 233, 1, 0, 1 },
    { "GROUP",       NULL, 658, 187, 120, 31,  1, 1, 1 },
    { "PLAYERDOLL",  NULL, 0,   41,  221, 224, 1, 1, 1 },
    { "BELOWINV",    NULL, 262, 545, 274, 55,  1, 1, 1 },
    { "PLAYERINFO",  NULL, 0,   0,   219, 41,  1, 1, 1 },
    { "RANGEBOX",    NULL, 6,   100, 94,  60,  1, 1, 1 },
    { "TARGET",      NULL, 267, 514, 264, 31,  1, 1, 1 },
    { "MAININV",     NULL, 539, 147, 239, 32,  1, 1, 1 },
    { "MAPNAME",     NULL, 228, 106, 36,  12,  1, 1, 1 },
    { "CONSOLE",     NULL, 271, 517, 256, 25,  1, 0, 1 },
    { "NUMBER",      NULL, 271, 465, 256, 43,  1, 0, 1 },
    { "STATOMETER",  NULL, 8,   50,  160, 40,  1, 1, 1 },
};

/* default overall priority list.. will change during runtime */
/* widget at the head has highest priority */
/* events go to the head first.. */
/* displaying goes to the foot first.. */
static widget_node_t *PriorityListHead,
                     *PriorityListFoot;

/* a way to steal the mouse, and to prevent widgets from using mouse events */
/* Ex: prevents widgets from using mouse events during dragging procedure */
static uint8 IsMouseExclusive = 0;

/* file-scope routines .. */
static void InitPriorityList(void);
static void DeinitPriorityList(void);

/* Initialises the default widget positions and priority list */
void widget_init(void)
{
    widget_id_t id;

    widget_deinit();

    for (id = 0; id < WIDGET_NROF; id++)
    {
        widget_data[id] = DefaultData[id];
    }

    if (PriorityListHead)
    {
        DeinitPriorityList();
    }

    InitPriorityList();
}

/* perform de-initialization (system-scope) of the widgets.. */
void widget_deinit(void)
{
    widget_id_t id;

    for(id = 0; id < WIDGET_NROF; id++)
    {
        if (widget_surface[id])
        {
            SDL_FreeSurface(widget_surface[id]);
            widget_surface[id] = NULL;
        }
    }

    DeinitPriorityList();
}

/* load the widgets/interface from a file */
/* do not perform any dynamic allocation! */
void widget_load(void)
{
    PHYSFS_File *handle;
    char         buf[TINY_BUF];
    widget_id_t  id = WIDGET_NROF;

    widget_init();

    if (!(handle = load_client_file(FILE_WIDGET)))
    {
        return;
    }

    /* Read the settings from the file */
    while (PHYSFS_readString(handle, buf, sizeof(buf)) >= 0)
    {
        char *key = buf,
             *value;

        /* Skip comments and blank bufs. */
        if (buf[0]=='#' ||
            buf[0]=='\0')
        {
            continue;
        }

        /* If no : can be found, set value to the end (\0) of buf. */
        if (!(value = strchr(buf, ':')))
        {
            value = &buf[strlen(buf)];
        }
        /* Otherwise, replace the : with \0 (terminates key) and set value to
         * the next character. */
        else
        {
            *value++ = '\0';

            /* Skip any spaces, so value starts with a real character. */
            while (*value != '\0' &&
                   isspace(*value))
            {
                value++;
            }
        }

        if (id == WIDGET_NROF &&
            !strcmp(key, "Widget"))
        {
            /* find the index of the widget for reference */
            for (id = 0; id < WIDGET_NROF; id++)
            {
                 if (!strcmp(widget_data[id].name, value))
                 {
                     break;
                 }
            }

            /* the widget name couldn't be found? */
            if (id == WIDGET_NROF)
            {
                LOG(LOG_ERROR, "Ignoring unrecognised widget >%s<!\n",
                    value);
            }
        }

        if (id == WIDGET_NROF)
        {
           continue;
        }

        if (!strcmp(key, "....X1"))
        {
            widget_data[id].x1 = atoi(value);
        }
        else if (!strcmp(key, "....Moveable"))
        {
            widget_data[id].moveable = atoi(value);
        }
        else if (!strcmp(key, "....Active"))
        {
            widget_data[id].show = atoi(value);
        }
        else if (!strcmp(key, "....Y1"))
        {
            widget_data[id].y1 = atoi(value);
        }
        else if (!strcmp(key, "....Wd"))
        {
            widget_data[id].wd = atoi(value);
        }
        else if (!strcmp(key, "....Ht"))
        {
            widget_data[id].ht = atoi(value);
        }
        else if (!strcmp(key, "....END"))
        {
            id = WIDGET_NROF;
        }
    }

    PHYSFS_close(handle);
}

/* save the widgets/interface to a file */
void widget_save(void)
{
    PHYSFS_File *handle;
    widget_id_t  id;

    if (!(handle = save_client_file(FILE_WIDGET)))
    {
        return;
    }

    for (id = 0; id < WIDGET_NROF; id++)
    {
        char buf[TINY_BUF];

        sprintf(buf, "\nWidget: %s\n", widget_data[id].name);
        PHYSFS_writeString(handle, buf);
        sprintf(buf, "....Moveable: %d\n", widget_data[id].moveable);
        PHYSFS_writeString(handle, buf);
        sprintf(buf, "....Active: %d\n", widget_data[id].show);
        PHYSFS_writeString(handle, buf);
        sprintf(buf, "....X1: %d\n", widget_data[id].x1);
        PHYSFS_writeString(handle, buf);
        sprintf(buf, "....Y1: %d\n", widget_data[id].y1);
        PHYSFS_writeString(handle, buf);
        sprintf(buf, "....Wd: %d\n", widget_data[id].wd);
        PHYSFS_writeString(handle, buf);
        sprintf(buf, "....Ht: %d\n", widget_data[id].ht);
        PHYSFS_writeString(handle, buf);
        PHYSFS_writeString(handle, "....END\n");
    }

    PHYSFS_close(handle);
}

/************************ MOUSE DOWN **####*****************/
/* check for owner of mouse focus                            */
/* setup widget dragging, if enabled                         */
/* TODO - right-click.. select 'move' to move a widget..     */
int widget_event_mousedn(int x, int y, SDL_Event *event)
{
    widget_id_t id = widget_get_owner(x,y);

    /* setup the event structure in response */
    widget_mouse_event.moving = 0;
    widget_mouse_event.id = id;

    /* sanity check */
    if (id < 0)
    {
        return 0;
    }

    /* setup the event structure in response */
    widget_mouse_event.x = x;
    widget_mouse_event.y = y;

    /* set the priority to this widget */
    widget_set_priority(id);

    /* if its moveable, start moving it when the conditions warrant it */
    if (widget_data[id].moveable &&
        MouseEvent == RB_DN)
    {
        /* If widget is MOVEABLE, this defines the hotspot areas for activating */
        switch(id)
        {
            default:
                /* we know this widget owns the mouse.. */
                widget_mouse_event.moving = 1;
        }

        /* start the movement procedures */
        if(widget_mouse_event.moving)
        {
            widget_mouse_event.id = id;
            widget_mouse_event.xoff = x - widget_data[id].x1;
            widget_mouse_event.yoff = y - widget_data[id].y1;
            /* enable the custom cursor */
            f_custom_cursor = MSCURSOR_MOVE;
            /* hide the system cursor */
            SDL_ShowCursor(0);
        }

        return 1;
    }
    /* NORMAL CONDITION - RESPOND TO MOUSEDOWN EVENT */
    else
    {
    /* Place here all the mousedown Handlers */
        switch (id)
        {
            case WIDGET_SKILL_EXP_ID:
                /* Handle the mousedown on the exp area */
                widget_skill_exp_event(x, y, MOUSE_DOWN);

                break;

            case WIDGET_MENU_B_ID:
                /* Handle mousedown on the menu buttons... */
                widget_menubuttons_event(x, y, MOUSE_DOWN);

                break;

            case WIDGET_QUICKSLOT_ID:
                widget_quickslots_mouse_event(x, y, MOUSE_DOWN);

                break;

            case WIDGET_CHATWIN_ID:
                textwin_button_event(TW_CHAT, WIDGET_CHATWIN_ID, event);

                break;

            case WIDGET_MSGWIN_ID:
                textwin_button_event(TW_MSG, WIDGET_MSGWIN_ID, event);

                break;

            case WIDGET_MIXWIN_ID:
                textwin_button_event(TW_MIX, WIDGET_MIXWIN_ID, event);

                break;

            case WIDGET_GROUP_ID:
//                group_event();

                break;

            case WIDGET_RANGE_ID:
                widget_range_event(x,y, *event, MOUSE_DOWN);

                break;

            case WIDGET_BELOW_INV_ID:
                widget_below_window_event(x,y,MOUSE_DOWN);

                break;

            case WIDGET_TARGET_ID:
                widget_event_target(x, y, *event);

                break;

            case WIDGET_MAIN_INV_ID:
                widget_inventory_event(x, y, *event);

                break;

            case WIDGET_PLAYER_INFO_ID:
                widget_player_data_event(x, y);

                break;

            case WIDGET_IN_NUMBER_ID:
                widget_number_event(x, y, *event);

                break;

            case WIDGET_STATS_ID:
            case WIDGET_RESIST_ID:
            case WIDGET_MAIN_LVL_ID:
            case WIDGET_REGEN_ID:
            case WIDGET_SKILL_LVL_ID:
            case WIDGET_PDOLL_ID:
            case WIDGET_MAPNAME_ID:
            case WIDGET_IN_CONSOLE_ID:
            case WIDGET_STATOMETER_ID:
                break;

            case WIDGET_NROF:
                break;
        }

        return 1;
    }
}


/************************ MOUSE UP **************************/
/* check for owner of mouse focus                           */
/* stop dragging the widget, if moving                      */
int widget_event_mouseup(int x, int y, SDL_Event *event)
{
    /* widget moving condition */
    if (widget_mouse_event.moving)
    {
        widget_mouse_event.moving = 0;
        widget_mouse_event.x = x;
        widget_mouse_event.y = y;

        /* disable the custom cursor */
        f_custom_cursor = 0;
        /* show the system cursor */
        SDL_ShowCursor(1);

        /* the interface has changed, save it! */
//        if(options.auto_save_interface) { widget_save(); }

        return 1;
    }
    /* NORMAL CONDITION - RESPOND TO MOUSEUP EVENT */
    else
    {
        widget_id_t id = widget_get_owner(x,y);

        /* setup the event structure in response */
        widget_mouse_event.id = id;

        /* handler(s) for miscellanous mouse movement(s) go here */

        /* sanity check.. return if mouse is not in a widget */
        if (id < 0)
        {
            return 0;
        }

        /* setup the event structure in response */
        widget_mouse_event.x = x;
        widget_mouse_event.y = y;

        /* handler(s) for the widgets go here */

        switch (id)
        {
            /* drop to quickslots */
            case WIDGET_QUICKSLOT_ID:
                widget_quickslots_mouse_event(x,y,MOUSE_UP);

                break;

            case WIDGET_CHATWIN_ID:
                textwin_move_event(TW_CHAT, WIDGET_CHATWIN_ID, event);

                break;

            case WIDGET_MSGWIN_ID:
                textwin_move_event(TW_MSG, WIDGET_MSGWIN_ID, event);

                break;

            case WIDGET_MIXWIN_ID:
                textwin_move_event(TW_MIX, WIDGET_MIXWIN_ID, event);

                break;

            case WIDGET_GROUP_ID:
//                group_event();

                break;

            case WIDGET_PDOLL_ID:
                widget_show_player_doll_event(x,y, MOUSE_UP);

                break;

            case WIDGET_RANGE_ID:
                widget_range_event(x,y, *event, MOUSE_UP);

                break;

            case WIDGET_BELOW_INV_ID:
                widget_below_window_event(x, y, MOUSE_UP);

                break;

            case WIDGET_MAIN_INV_ID:
                widget_inventory_event(x, y, *event);

                break;

            case WIDGET_STATS_ID:
            case WIDGET_RESIST_ID:
            case WIDGET_MAIN_LVL_ID:
            case WIDGET_SKILL_EXP_ID:
            case WIDGET_REGEN_ID:
            case WIDGET_SKILL_LVL_ID:
            case WIDGET_MENU_B_ID:
            case WIDGET_PLAYER_INFO_ID:
            case WIDGET_TARGET_ID:
            case WIDGET_MAPNAME_ID:
            case WIDGET_IN_CONSOLE_ID:
            case WIDGET_IN_NUMBER_ID:
            case WIDGET_STATOMETER_ID:
                break;

            case WIDGET_NROF:
                break;
        }

        return 1;
    }
}


/******************************* MOUSE MOVE **************************/
/* check for owner of mouse focus                                    */
/* drag the widget, if moving                                        */
int widget_event_mousemv(int x,int y, SDL_Event *event)
{
    cursor_type = 0; /* with widgets we have to clear every loop the txtwin cursor */

    /* widget moving condition */
    if (widget_mouse_event.moving)
    {
//        int adjx = x - widget_mouse_event.xoff,
//            adjy = y - widget_mouse_event.yoff;
#ifdef WIDGET_SNAP
#define LEFT(_ID_)   (widget_data[(_ID_)].x1)
#define RIGHT(_ID_)  (widget_data[(_ID_)].x1 + widget_data[(_ID_)].wd)
#define TOP(_ID_)    (widget_data[(_ID_)].y1)
#define BOTTOM(_ID_) (widget_data[(_ID_)].y1 + widget_data[(_ID_)].ht)
        if (options.widget_snap>0)
        {
            if (event->motion.xrel != 0 &&
                event->motion.yrel != 0)
            {
                widget_id_t    id_e = widget_mouse_event.id;
                widget_node_t *node;

                for (node = PriorityListHead; node; node = node->next)
                {
                    widget_id_t id = node->id;
                    uint8       done = 0;

                    if (id == id_e ||
                        !widget_data[id].show)
                    {
                        continue;
                    }

                    if ((TOP(id_e) >= TOP(id) &&
                         TOP(id_e) <= BOTTOM(id)) ||
                        (BOTTOM(id_e) >= TOP(id) &&
                         BOTTOM(id_e) <= BOTTOM(id)))
                    {
                        if (event->motion.xrel < 0 &&
                            LEFT(id_e) <= RIGHT(id) + options.widget_snap &&
                            LEFT(id_e) > RIGHT(id))
                        {
    //                        adjx = RIGHT(id);
                            event->motion.x = RIGHT(id) + widget_mouse_event.xoff;
                            done = 1;
                        }
                        else if (event->motion.xrel > 0 &&
                                 RIGHT(id_e) >= LEFT(id) - options.widget_snap &&
                                 RIGHT(id_e) < LEFT(id))
                        {
    //                        adjx = LEFT(id) - widget_data[id_e].wd;
                            event->motion.x = LEFT(id) - widget_data[id_e].wd + widget_mouse_event.xoff;
                            done = 1;
                        }
                    }
                    if ((LEFT(id_e) >= LEFT(id) &&
                         LEFT(id_e) <= RIGHT(id)) ||
                        (RIGHT(id_e) >= LEFT(id) &&
                         RIGHT(id_e) <= RIGHT(id)))
                    {
                        if (event->motion.yrel < 0 &&
                            TOP(id_e) <= BOTTOM(id) + options.widget_snap &&
                            TOP(id_e) > BOTTOM(id))
                        {
    //                        adjy = BOTTOM(id);
                            event->motion.y = BOTTOM(id) + widget_mouse_event.yoff;
                            done = 1;
                        }
                        else if (event->motion.yrel > 0 &&
                                 BOTTOM(id_e) >= TOP(id) - options.widget_snap &&
                                 BOTTOM(id_e) < TOP(id))
                        {
    //                        adjy = TOP(id) - widget_data[id_e].ht;
                            event->motion.y = TOP(id) - widget_data[id_e].ht + widget_mouse_event.yoff;
                            done = 1;
                        }
                    }
                    if (done)
                    {
    //                    textwin_showstring(COLOR_RED, "%s l=%d r=%d t=%d b=%d", widget_data[id].name, LEFT(id), RIGHT(id), TOP(id), BOTTOM(id));
                        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0, 10);
                        event->motion.xrel = event->motion.yrel = 0; // acts as a brake, preventing id_e from 'skipping' through a stack of nodes
                        SDL_PushEvent(event);
                        break;
                    }
                }
            }
        }
#undef LEFT
#undef RIGHT
#undef TOP
#undef BOTTOM
#endif
        widget_data[widget_mouse_event.id].x1 = x - widget_mouse_event.xoff; // adjx;
        widget_data[widget_mouse_event.id].y1 = y - widget_mouse_event.yoff; // adjy;
        map_udate_flag = 2;
        return 1;
    }
    /* NORMAL CONDITION - RESPOND TO MOUSEMOVE EVENT */
    else
    {
        widget_id_t id = widget_get_owner(x,y);

        /* setup the event structure in response */
        widget_mouse_event.moving = 0;
        widget_mouse_event.id = id;

        /* handler(s) for miscellanous mouse movement(s) go here */

        /* textwin special handling */
        if (txtwin[TW_CHAT].highlight != TW_HL_NONE)
        {
            txtwin[TW_CHAT].highlight = TW_HL_NONE;
            WIDGET_REDRAW(WIDGET_CHATWIN_ID) = 1;
        }
        if (txtwin[TW_MSG].highlight != TW_HL_NONE)
        {
            txtwin[TW_MSG].highlight = TW_HL_NONE;
            WIDGET_REDRAW(WIDGET_MSGWIN_ID) = 1;
        }
        /* sanity check.. return if mouse is not in a widget */
        if (id < 0)
        {
            return 0;
        }

        /* setup the event structure in response */
        widget_mouse_event.x = x;
        widget_mouse_event.y = y;

        /* handler(s) for the widgets move go here */

        switch (id)
        {
            case WIDGET_CHATWIN_ID:
                textwin_move_event(TW_CHAT, WIDGET_CHATWIN_ID, event);

                break;

            case WIDGET_MSGWIN_ID:
                textwin_move_event(TW_MSG, WIDGET_MSGWIN_ID, event);

                break;

            case WIDGET_MIXWIN_ID:
                textwin_move_event(TW_MIX, WIDGET_MIXWIN_ID, event);

                break;

            case WIDGET_MAIN_INV_ID:
                widget_inventory_event(x, y, *event);

                break;

            case WIDGET_STATS_ID:
            case WIDGET_RESIST_ID:
            case WIDGET_MAIN_LVL_ID:
            case WIDGET_SKILL_EXP_ID:
            case WIDGET_REGEN_ID:
            case WIDGET_SKILL_LVL_ID:
            case WIDGET_MENU_B_ID:
            case WIDGET_QUICKSLOT_ID:
            case WIDGET_GROUP_ID:
            case WIDGET_PDOLL_ID:
            case WIDGET_BELOW_INV_ID:
            case WIDGET_PLAYER_INFO_ID:
            case WIDGET_RANGE_ID:
            case WIDGET_TARGET_ID:
            case WIDGET_MAPNAME_ID:
            case WIDGET_IN_CONSOLE_ID:
            case WIDGET_IN_NUMBER_ID:
            case WIDGET_STATOMETER_ID:
                break;

            case WIDGET_NROF:
                break;
        }

        return 1;
    }
}

/* find the widget with mouse focus on a mouse-hit-test basis */
widget_id_t widget_get_owner(int x, int y)
{
    widget_node_t *node;
    widget_id_t id;

    /* Priority overide function, we have to have that here for resizing.... */
    if (textwin_flags & TW_RESIZE)
    {
        if ((textwin_flags & TW_CHAT))
        {
            return WIDGET_CHATWIN_ID;
        }
        else if ((textwin_flags & TW_MSG))
        {
            return WIDGET_MSGWIN_ID;
        }
        else if ((textwin_flags & TW_MIX))
        {
            return WIDGET_MIXWIN_ID;
        }
    }

   
    if (IsMouseExclusive || // mouse cannot be used by widgets
        !PriorityListHead)  // priority list doesn't exist
    {
        return WIDGET_NROF;
    }

    /* loop through the list & perform custom or default hit-test */
    for (node = PriorityListHead; node; node = node->next)
    {
        id = node->id;

        if (!widget_data[id].show)
        {
            continue;
        }

        switch (id)
        {
            case WIDGET_PDOLL_ID: /*Playerdoll widget is NOT a rectangle, handle special... */
                if (x > widget_data[id].x1 + 111)
                {
                    if (x <= widget_data[id].x1 + widget_data[id].wd &&
                        y >= widget_data[id].y1 &&
                        y <= ((x-(widget_data[id].x1 + 111)) / -2) + 215 +
                              widget_data[id].y1)
                    {
                            return id;
                    }
                }
                else
                {
                    if (x >= widget_data[id].x1 &&
                        y >= widget_data[id].y1 &&
                        y <= ((x - widget_data[id].x1) / 2) + 160 +
                              widget_data[id].y1)
                    {
                            return id;
                    }
                }

                break;

            default:
                if (x >= widget_data[id].x1 &&
                    x <= (widget_data[id].x1 + widget_data[id].wd) &&
                    y >= widget_data[id].y1 &&
                    y <= (widget_data[id].y1 + widget_data[id].ht))
                {
                    return id;
                }
        }
    }

    return WIDGET_NROF;
}

/* loop through all the widgets and call the corresponding handlers */
/* this is called everytime in main.c.. in the main loop */
void widget_process(void)
{
    widget_node_t *node;

    /* sanity checks */
    if (!PriorityListHead ||
        !PriorityListFoot)
    {
        return;
    }

    for (node = PriorityListFoot; node; node = node->prev)
    {
        widget_id_t    id = node->id;
#ifdef PROFILING_WIDGETS
        uint32         ts = SDL_GetTicks();

        LOG(LOG_MSG, "[Prof] widget_draw %16s shown: %d, redraw: %d, ms: ",
            widget_data[id].name, widget_data[id].show,
            widget_data[id].redraw);
#endif

        if (widget_data[id].show)
        {
            /* Doesn't matter which order the case statements follow */
            switch (id)
            {
                case WIDGET_STATS_ID:
                    widget_player_stats(widget_data[id].x1,widget_data[id].y1);

                    break;

                case WIDGET_RESIST_ID:
                    widget_show_resist(widget_data[id].x1,widget_data[id].y1);

                    break;

                case WIDGET_MAIN_LVL_ID:
                    widget_show_main_lvl(widget_data[id].x1,widget_data[id].y1);

                    break;

                case WIDGET_SKILL_EXP_ID:
                    widget_show_skill_exp(widget_data[id].x1,widget_data[id].y1);

                    break;

                case WIDGET_REGEN_ID:
                    widget_show_regeneration(widget_data[id].x1,widget_data[id].y1);

                    break;

                case WIDGET_SKILL_LVL_ID:
                    widget_skillgroups(widget_data[id].x1,widget_data[id].y1);

                    break;

                case WIDGET_MENU_B_ID:
                    widget_menubuttons(widget_data[id].x1,widget_data[id].y1);

                    break;

                case WIDGET_QUICKSLOT_ID:
                    widget_quickslots(widget_data[id].x1,widget_data[id].y1);

                    break;

                case WIDGET_CHATWIN_ID:
                    widget_textwin_show(widget_data[id].x1,widget_data[id].y1, TW_CHAT);

                    break;

                case WIDGET_MSGWIN_ID:
                    widget_textwin_show(widget_data[id].x1,widget_data[id].y1, TW_MSG);

                    break;

                case WIDGET_MIXWIN_ID:
                    widget_textwin_show(widget_data[id].x1,widget_data[id].y1, TW_MIX);

                    break;

                case WIDGET_GROUP_ID:
                    widget_show_group(widget_data[id].x1,widget_data[id].y1);

                    break;

                case WIDGET_PDOLL_ID:
                    widget_show_player_doll(widget_data[id].x1,widget_data[id].y1);

                    break;

                case WIDGET_BELOW_INV_ID:
                    widget_show_below_window(cpl.below, widget_data[id].x1,widget_data[id].y1);

                    break;

                case WIDGET_PLAYER_INFO_ID:
                    widget_show_player_data(widget_data[id].x1,widget_data[id].y1);

                    break;

                case WIDGET_RANGE_ID:
                    widget_show_range(widget_data[id].x1,widget_data[id].y1);

                    break;

                case WIDGET_TARGET_ID:
                    widget_show_target(widget_data[id].x1,widget_data[id].y1);

                    break;

                case WIDGET_MAIN_INV_ID:
                    widget_show_inventory_window(widget_data[id].x1,widget_data[id].y1);

                    break;

                case WIDGET_MAPNAME_ID:
                    widget_show_mapname(widget_data[id].x1,widget_data[id].y1);

                    break;

                case WIDGET_IN_CONSOLE_ID:
                    widget_show_console(widget_data[id].x1,widget_data[id].y1);

                    break;

                case WIDGET_IN_NUMBER_ID:
                    widget_show_number(widget_data[id].x1,widget_data[id].y1);

                    break;

                case WIDGET_STATOMETER_ID:
                    widget_show_statometer(widget_data[id].x1, widget_data[id].y1);

                    break;

                default:
                    LOG(LOG_ERROR, "Widget %d unhandled in widget_process()!\n",
                        id);
            }
        }

#ifdef PROFILING_WIDGETS
        LOG(LOG_MSG, "%d\n", SDL_GetTicks() - ts);
#endif
    }
}

/* This is used by widgets when they use the mouse */
/* if it returns 0, don't use the mouse! */
/* if it doesn't return 0, expect values such as... */
/*
    IDLE,
    LB_DN,
    LB_UP,
    RB_DN,
    RB_UP,
    MB_UP,
    MB_DN,
*//* use the variable 'MouseEvent' to determine unique mouse events */
uint32 widget_get_mouse_state(int *mx, int *my, widget_id_t id)
{
    if (widget_mouse_event.id == id &&
        !IsMouseExclusive &&
        (cpl.menustatus != MENU_NO ||
         esc_menu_flag))
    {
        *mx = widget_mouse_event.x;
        *my = widget_mouse_event.y;

        return MouseState;
    }

    return 0; /* don't use the mouse in the calling widget! */
}

/* Sets this widget to have highest priority */
/* 1) Transfer head to a new node below head */
/* 2) Transfer this widget to the head */
/* 3) Remove this widget from its previous priority */
void widget_set_priority(widget_id_t id)
{
    widget_node_t *node;

//    LOG(LOG_MSG,"Entering widget_set_priority(id=%d)..\n",id);

    /* sanity check */
    if (id < 0 ||
        id >= WIDGET_NROF)
    {
        return;
    }

    /* exit, if already highest priority */
    if (PriorityListHead->id == id)
    {
        return;
    }


    /* move the current highest to second highest priority */
    MALLOC(node, sizeof(widget_node_t));
    *node = *PriorityListHead;
    node->prev = PriorityListHead;
    node->next->prev = node;
    PriorityListHead->next = node;
    widget_data[node->id].priority_index = node;

    /* make this widget have highest priority */
    PriorityListHead->id = id;

    /* remove it from its previous priority */
    node = widget_data[id].priority_index;

//    LOG(LOG_MSG, "..node: %d\n", node);
//    LOG(LOG_MSG, "..widget_data[id].priority_index: %d\n", widget_data[id].priority_index);
//    LOG(LOG_MSG, "..node->prev: %d, node->next: %d\n", node->prev, node->next);

    if(node->next)
    {
        node->next->prev = node->prev;
        node->prev->next = node->next;
    }
    else /* foot of list */
    {
        /* update the foot of priority list */
        PriorityListFoot = node->prev;
        node->prev->next = NULL;
    }

    FREE(node);

    /* re-link the widget lookup */
    widget_data[id].priority_index = PriorityListHead;

//    LOG(LOG_MSG, "..widget_set_priority(): Done.\n");
}

/* used in two places atm, so its in a static-scope function.. */
static void InitPriorityList(void)
{
    widget_node_t *node;
    widget_id_t    id;

    /* if its already allocated, leave */
    if(PriorityListHead) { return; }

//    LOG(LOG_MSG,"Entering InitPriorityList()..\n");

    /* allocate the head of the list */
    MALLOC(node, sizeof(widget_node_t));
    PriorityListHead = node;
    /* set the members and store a 'link' to this pointer */
    PriorityListHead->next = NULL;
    PriorityListHead->prev = NULL;
    PriorityListHead->id = 0;
    widget_data[0].priority_index = PriorityListHead;

    for (id=1; id<WIDGET_NROF; ++id)
    {
        /* allocate it */
        MALLOC(node->next, sizeof(widget_node_t));
        node->next->prev = node;
        /* set the members and store a 'link' to this pointer */
        node = node->next;
        node->next = NULL;
        node->id = id;
        widget_data[id].priority_index = node;
    }

    /* set the foot of the priority list */
    PriorityListFoot = node;

    /* some helpfull information */
//    LOG(LOG_MSG, "..Output of node list:\n");
//    for(id=0,node=PriorityListHead;node;node=node->next,++id)
//    {
//        LOG(LOG_MSG, "..Node #%d: %d\n", id,node->id);
//    }
//    LOG(LOG_MSG, "..Allocated %d/%d nodes!\n", id, WIDGET_NROF);

//    LOG(LOG_MSG, "..InitPriorityList(): Done.\n");
}

/* had to make this for the circumstances */
static void DeinitPriorityList(void)
{
    widget_id_t id;

    /* leave if its clear already */
    if(!PriorityListHead) { return; }

//    LOG(LOG_MSG,"Entering DeinitPriorityList()..\n");

//    LOG(LOG_MSG, "..Output of deleted node(s):\n");

    /* walk down the list and free it */
    for(id=0;PriorityListHead;++id)
    {
//        LOG(LOG_MSG, "..Node #%d: %d\n", id, PriorityListHead->id);

        widget_node_t *node = PriorityListHead->next;

        FREE(PriorityListHead);
        PriorityListHead = node;
    }

//    LOG(LOG_MSG, "..De-Allocated %d/%d nodes!\n", id, WIDGET_NROF);

    PriorityListHead = NULL;
    PriorityListFoot = NULL;

//    LOG(LOG_MSG, "..DeinitPriorityList(): Done.\n");
}
