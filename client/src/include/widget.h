/*
    Daimonin SDL client, a client program for the Daimonin MMORPG.


  Copyright (C) 2005 John Swenson

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

    The author can be reached via e-mail to jonsebox28@teleport.com
*/

#ifndef __WIDGET_H
#define __WIDGET_H

/* add the widget id here */
typedef enum widget_id_t
{
    WIDGET_STATS_ID,
    WIDGET_RESIST_ID,
    WIDGET_MALVL_ID,
    WIDGET_SKEXP_ID,
    WIDGET_REGEN_ID,
    WIDGET_SKLVL_ID,
    WIDGET_MENUB_ID,
    WIDGET_QSLOTS_ID,
    WIDGET_CHATWIN_ID,
    WIDGET_MSGWIN_ID,
    WIDGET_GROUP_ID,
    WIDGET_PDOLL_ID,
    WIDGET_BELOW_ID,
    WIDGET_PINFO_ID,
    WIDGET_RANGE_ID,
    WIDGET_TARGET_ID,
    WIDGET_INV_ID,
    WIDGET_MAPNAME_ID,
    WIDGET_CONSOLE_ID,
    WIDGET_NUMBER_ID,
    WIDGET_SMETER_ID,

    WIDGET_NROF // must be last element
}
widget_id_t;

/* used in the priority list (to order widgets) */
typedef struct widget_node_t
{
    struct widget_node_t *next,
                         *prev;

    widget_id_t id;
}
widget_node_t;

/* information about a widget - used for current/default list */
typedef struct widget_data_t
{
    /* These are saved to FILE_WIDGET. */
    char          *name;     // name
    uint8          moveable; // draggable?
    uint8          show;     // active?
    sint16         x1;       // x screen posiition in pixels
    sint16         y1;       // y screen posiition in pixels
    uint16         wd;       // width in pixels
    uint16         ht;       // height in pixels

    /* These are not saved. */
    void          (*process)(widget_id_t);            // redraw handler
    void          (*event)(widget_id_t, SDL_Event *); // mouse event handler
    SDL_Surface   *surface;                           // backbuffer
    widget_node_t *priority_index;                    // internal use only
    uint8          redraw;                            // redraw?
}
widget_data_t;

/* used for mouse button/move events */
typedef struct widget_event_t
{
    uint8       moving;
    widget_id_t id;
    sint16      x;
    sint16      y;
    sint16      xoff;
    sint16      yoff;
}
widget_event_t;

/* helpermakros */
#define WIDGET_SHOW(_ID_) widget_data[(_ID_)].show
#define WIDGET_REDRAW(_ID_) widget_data[(_ID_)].redraw

extern widget_data_t   widget_data[WIDGET_NROF];
extern widget_event_t  widget_mouse_event;

extern void        widget_init(void);
extern void        widget_deinit(void);
extern void        widget_load(void);
extern void        widget_save(void);
extern int         widget_event_mousedn(int x, int y, SDL_Event *event);
extern int         widget_event_mouseup(int x, int y, SDL_Event *event);
extern int         widget_event_mousemv(int x, int y, SDL_Event *event);
extern uint32      widget_get_mouse_state(int *mx, int *my, widget_id_t id);
extern void        widget_set_priority(widget_id_t id);
extern widget_id_t widget_get_owner(int x, int y);
extern void        widget_process(void);

#endif /* ifndef __WIDGET_H */
