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
#if !defined(__WIDGET_H)
#define __WIDGET_H

/* used in the priority list (to order widgets) */
struct _widget_node {
    struct _widget_node *next;
    struct _widget_node *prev;
	int WidgetID;
};
typedef struct _widget_node widget_node;

/* information about a widget - used for current/default list */
typedef struct _widgetdata
{
	char *name;				/* what is its name? */

	/* internal use only */
	widget_node *priority_index;

	/* position values */
	int x1;
	int y1;
	int wd;
	int ht;

	Boolean moveable;		/* can you drag it? */
	Boolean show;           /* hidden and inactive or shown */
	Boolean redraw;         /* widget must be redrawn */

}_widgetdata;

/* events that are passed to the widget handler */
typedef enum _proc_type
{
    PROCESS,
    EVENT,
    KILL,
} _proc_type;

enum _MEvent
{
    MOUSE_UP = 1,
    MOUSE_DOWN,
    MOUSE_MOVE,
};

/* add the widget id here */
typedef enum _WidgetID
{
    STATS_ID,
    RESIST_ID,
    MAIN_LVL_ID,
    SKILL_EXP_ID,
    REGEN_ID,
    SKILL_LVL_ID,
    MENU_B_ID,
    QUICKSLOT_ID,
    CHATWIN_ID,
    MSGWIN_ID,
    MIXWIN_ID,
    GROUP_ID,
    PDOLL_ID,
    BELOW_INV_ID,
    PLAYER_INFO_ID,
    RANGE_ID,
    TARGET_ID,
    MAIN_INV_ID,
    MAPNAME_ID,
    IN_CONSOLE_ID,
    IN_NUMBER_ID,
    STATOMETER_ID,

	TOTAL_WIDGETS, /* Must be last element */
}_WidgetID;

/* used for mouse button/move events */
typedef struct _widgetevent
{
	int owner;
	int x;
	int y;
}_widgetevent;

/* this is used when moving a widget with the mouse */
typedef struct _widgetmove
{
	Boolean active;
	int id;
	int xOffset;
	int yOffset;
}_widgetmove;

extern SDL_Surface* widgetSF[TOTAL_WIDGETS];

extern _widgetdata  cur_widget[TOTAL_WIDGETS];
extern _widgetevent widget_mouse_event;
extern Boolean      IsMouseExclusive;

/* init and kill */
extern void	    init_widgets_fromDefault();
extern void	    init_widgets_fromCurrent();
extern Boolean  init_widgets_fromFile(char *filename);
extern void     kill_widgets();

/* file */
extern void     save_interface_file(void);

/* events */
extern int      widget_event_mousedn(int x,int y, SDL_Event *event);
extern int      widget_event_mouseup(int x,int y, SDL_Event *event);
extern int      widget_event_mousemv(int x,int y, SDL_Event *event);

/* misc */
extern void     process_widgets();
extern uint32   GetMouseState(int *mx, int *my,int nWidgetID);
extern Boolean  IsWidgetDragging();
extern void     SetPriorityWidget(int nWidgetID);

extern int      get_widget_owner(int x,int y);

/* helpermakros */
#define WIDGET_REDRAW(__a) cur_widget[__a].redraw=TRUE;

#endif
