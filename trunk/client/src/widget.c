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

/************************************************************************/
/*
	To add a new widget:

	1) Add an entry (same index in both cases) to "_widgetdata con_widget[]" (widget.c), and "enum _WidgetID" (widget.h)
	2) If applicable, add handler code for widget movement in widget_event_mousedn()
	3) If applicable, add handler code to get_widget_owner()
	4) Add handler/function to 'process_widget(int nID, int proc)'
*/

#include <include.h>

/* file-scope routines .. */
static void     process_widget(int nID, int proc);
static Boolean  load_interface_file(char *filename);
static void     init_priority_list();
static void     kill_priority_list();
/* ... */

/* Current (working) data-list of all widgets */
_widgetdata cur_widget[TOTAL_WIDGETS];

/* Current (default) data-list of all widgets */
_widgetdata def_widget[TOTAL_WIDGETS];

/* Default (default) data-list of all widgets */
/* {name,priority_index,x1,y1,width,height,moveable?, active?} */
const static _widgetdata con_widget[TOTAL_WIDGETS] =
{
	{"STATS",NULL,227,0,172,102,        TRUE, TRUE, TRUE},
	{"RESIST",NULL,497,0,198,79,        TRUE, TRUE, TRUE},
	{"MAIN_LVL",NULL,399,39,98,62,      TRUE, TRUE, TRUE},
	{"SKILL_EXP",NULL,497,79,198,22,    TRUE, TRUE, TRUE},
	{"REGEN",NULL,399,0,98,39,          TRUE, TRUE, TRUE},
	{"SKILL_LVL",NULL,695,0,52,101,     TRUE, TRUE, TRUE},
	{"MENUBUTTONS",NULL,747,0,47,101,   TRUE, TRUE, TRUE},
	{"QUICKSLOTS",NULL,513,107,282,34,  TRUE, TRUE, TRUE},
	{"CHATWIN",NULL,0,366,261,233,      TRUE, TRUE, TRUE},
	{"MSGWIN",NULL,537,366,261,233,     TRUE, TRUE, TRUE},
	{"MIXWIN",NULL,539,420,261,233,     TRUE, FALSE, TRUE},
	{"GROUP",NULL,658,187,120,31,       TRUE, TRUE, TRUE},
	{"PLAYERDOLL",NULL,0,41,221,224,    TRUE, TRUE, TRUE},
    {"BELOWINV",NULL,262,545,274,55,    TRUE, TRUE, TRUE},
    {"PLAYERINFO",NULL,0,0,219,41,      TRUE, TRUE, TRUE},
    {"RANGEBOX",NULL,6,100,94,60,       TRUE, TRUE, TRUE},
    {"TARGET",NULL,267,514,264,31,      TRUE, TRUE, TRUE},
    {"MAININV",NULL,539,147,239,32,     TRUE, TRUE, TRUE},
    {"MAPNAME",NULL,228,106,36,12,      TRUE, TRUE, TRUE},
    {"CONSOLE",NULL,271,517,256,25,     TRUE, FALSE, TRUE},
    {"NUMBER",NULL,271,465,256,43,      TRUE, FALSE, TRUE},
    {"STATOMETER",NULL,8,50,160,40,     TRUE, TRUE, TRUE},
};

/* default overall priority list.. will change during runtime */
/* widget at the head has highest priority */
/* events go to the head first.. */
/* displaying goes to the foot first.. */
static widget_node *priority_list_head;
static widget_node *priority_list_foot;


/* Determines which widget has mouse focus */
/* This value is determined in the mouse routines for the widgets */
_widgetevent widget_mouse_event =
{
	0,
	0,
	0
};

/* this is used when moving a widget with the mouse */
static _widgetmove widget_event_move =
{
	FALSE,
	0,
	0,
	0
};


SDL_Surface*    widgetSF[TOTAL_WIDGETS] = {NULL};

/* a way to steal the mouse, and to prevent widgets from using mouse events */
/* Ex: prevents widgets from using mouse events during dragging procedure */
Boolean IsMouseExclusive = FALSE;

/* load the defaults and initialize the priority list */
/* create the interface file, if it doesn't exist */
void init_widgets_fromDefault()
{
	int lp;

//	LOG(LOG_MSG,"Entering init_widgets_fromDefault()..\n");

	/* in all cases should reset */
	kill_widgets();

	/* exit, if there're no widget ID's */
	if(!TOTAL_WIDGETS) { return; }

	/* store the constant default widget lookup in the current lookup(s) */
	for(lp=0; lp<TOTAL_WIDGETS; ++lp)
		cur_widget[lp] = def_widget[lp] = con_widget[lp];

	/* allocate the priority list now */
	init_priority_list();

//	LOG(LOG_MSG, "..init_widgets_fromDefault(): Done.\n");
}

/* try to load the main interface file and initialize the priority list */
/* on failure, initialize the widgets w/ init_widgets_fromDefault() */
void init_widgets_fromCurrent()
{
//	LOG(LOG_MSG,"Entering init_widgets_fromCurrent()..\n");

	/* in all cases should reset */
	kill_widgets();

	/* exit, if there're no widgets */
	if(!TOTAL_WIDGETS) { return; }

	/* if cannot open/load the interface file .. */
    /* .. load defaults and create file */
	if(!load_interface_file(INTERFACE_FILE))
    {
		/* inform user.. */
		LOG(LOG_MSG, "..Can't open/load the interface file - %s. Resetting..\n", INTERFACE_FILE);
        /* load the defaults - this also allocates priority list */
        init_widgets_fromDefault();
		/* create the interface file.. */
		save_interface_file();
    }
    /* was able to load the interface file .. */
    else
    {
	    /* clear the priority list if it already exists */
	    if(priority_list_head) { kill_priority_list(); }

	    /* allocate the priority list now */
	    init_priority_list();
    }

//	LOG(LOG_MSG, "..init_widgets_fromCurrent(): Done.\n");
}

/* try to load an interface file and initialize the priority list */
/* on failure, exit without changing anything */
Boolean init_widgets_fromFile(char *filename)
{
//	LOG(LOG_MSG,"Entering init_widgets_fromFile()..\n");

	/* exit, if there're no widgets */
	if(!TOTAL_WIDGETS) { return FALSE; }

	/* exit, if cannot open/find file */
	if(!load_interface_file(filename)) { return FALSE; }

	/* clear the priority list if it already exists */
	if(priority_list_head) { kill_priority_list(); }

	/* allocate the priority list now */
	init_priority_list();

//	LOG(LOG_MSG, "..init_widgets_fromFile(): Done.\n");

	return TRUE;
}

/* used in two places atm, so its in a static-scope function.. */
void init_priority_list()
{
    widget_node *node;
    int lp;

	/* if its already allocated, leave */
	if(priority_list_head) { return; }

//    LOG(LOG_MSG,"Entering init_priority_list()..\n");

	/* allocate the head of the list */
    priority_list_head = node = _malloc (sizeof(widget_node),"init_priority_list: widget_node");
    if (!node) { exit(0); }
	/* set the members and store a 'link' to this pointer */
	priority_list_head->next = NULL;
	priority_list_head->prev = NULL;
	priority_list_head->WidgetID = 0;
	cur_widget[0].priority_index = priority_list_head;

    for (lp=1; lp<TOTAL_WIDGETS; ++lp)
	{
		/* allocate it */
		node->next = _malloc (sizeof(widget_node),"init_priority_list: widget_node");
		if (!node->next) { exit(0); }
		node->next->prev = node;
		/* set the members and store a 'link' to this pointer */
		node = node->next;
		node->next = NULL;
		node->WidgetID = lp;
		cur_widget[lp].priority_index = node;
    }

	/* set the foot of the priority list */
	priority_list_foot = node;

	/* some helpfull information */
//	LOG(LOG_MSG, "..Output of node list:\n");
//	for(lp=0,node=priority_list_head;node;node=node->next,++lp)
//	{
//		LOG(LOG_MSG, "..Node #%d: %d\n", lp,node->WidgetID);
//	}
//	LOG(LOG_MSG, "..Allocated %d/%d nodes!\n", lp, TOTAL_WIDGETS);

//    LOG(LOG_MSG, "..init_priority_list(): Done.\n");
}

/* had to make this for the circumstances */
void kill_priority_list()
{
    widget_node *tmp_node;
    int lp;

    /* leave if its clear already */
    if(!priority_list_head) { return; }

//    LOG(LOG_MSG,"Entering kill_priority_list()..\n");

//    LOG(LOG_MSG, "..Output of deleted node(s):\n");

    /* walk down the list and free it */
    for(lp=0;priority_list_head;++lp)
    {
//        LOG(LOG_MSG, "..Node #%d: %d\n", lp, priority_list_head->WidgetID);

        tmp_node = priority_list_head->next;
        free(priority_list_head);
        priority_list_head = tmp_node;
    }

//    LOG(LOG_MSG, "..De-Allocated %d/%d nodes!\n", lp, TOTAL_WIDGETS);

    priority_list_head = NULL;
    priority_list_foot = NULL;

//    LOG(LOG_MSG, "..kill_priority_list(): Done.\n");
}

/* perform de-initialization (system-scope) of the widgets.. */
void kill_widgets()
{
    int pos;
	for(pos=0; pos < TOTAL_WIDGETS; ++pos)
	{
        if (widgetSF[pos])
        {
            SDL_FreeSurface(widgetSF[pos]);
            widgetSF[pos]=NULL;
        }
	}
    kill_priority_list();

}

/* load the widgets/interface from a file */
/* do not perform any dynamic allocation! */
Boolean load_interface_file(char *filename)
{
	int i=-1, pos;
	FILE *stream;
	_widgetdata tmp_widget[TOTAL_WIDGETS];
	char line[256], keyword[256], parameter[256];
	int found_widget[TOTAL_WIDGETS] = {0};

//	LOG(LOG_MSG,"Entering load_interface_file()..\n");

	/* transfer the constant lookup to a temp lookup */
	/* .. we'll use it here to load the file */
	for(pos=0; pos < TOTAL_WIDGETS; ++pos)
		tmp_widget[pos] = con_widget[pos];

	/* sanity check - if the file doesn't exist, exit w/ error */
	if (!(stream = fopen_wrapper(filename, "r")))
	{
		/* inform user.. */
		LOG(LOG_MSG, "load_interface_file(): Can't find file %s.\n",filename);
		/* done.. */
		return FALSE;
	}

	/* Read the settings from the file */
	while (fgets( line, 255, stream ))
	{
		if(line[0]=='#' || line[0]=='\n')
			continue;

		i=0;
		while (line[i] && line[i]!= ':') i++;
		line[++i]=0;
		strcpy(keyword, line);
		strcpy(parameter, line + i + 1);
		/* remove the newline character */
		parameter[strcspn(line + i + 1, "\n")] = 0;

		/* 1) if we find a widget (beginning of block) .. */
		/* 2) read until "....END" (end of block) ,, */
		/* beginning of block .. */
		if(!strcmp(keyword, "Widget:"))
		{
			LOG(LOG_MSG, "..Trying to find \"Widget: %s\"", parameter);

			pos = 0;

			/* find the index of the widget for reference */
			while (	pos < TOTAL_WIDGETS &&
					(strcmp(tmp_widget[pos].name,parameter)!=0)) { ++pos; }

			/* the widget name couldn't be found? */
			if(pos >= TOTAL_WIDGETS)
			{
				LOG(LOG_MSG, ".. Widget not found!\n");

				continue;
			}
			/* get the block .. */
			else
			{
				/* if we haven't found this widget, mark it .. */
				if(!found_widget[pos])
				{
					LOG(LOG_MSG, ".. Found! (Index = %d) (%d widgets total)\n", pos, TOTAL_WIDGETS);
					found_widget[pos] = 1;
				}
				/* if we have found it, skip this block .. */
				else
				{
					LOG(LOG_MSG, ".. Widget already found! Please remove duplicate(s)!\n");
					continue;
				}

				while (fgets( line, 255, stream ))
				{
					if(line[0]=='#' || line[0]=='\n')
						continue;
					if(!strncmp(line, "....END", 7))
						break;

					i=0;
					while (line[i] && line[i]!= ':') i++;
					line[++i]=0;
					strcpy(keyword, line);
					strcpy(parameter, line + i + 1);

					if(!strcmp(keyword, "....X1:"))
					{
						tmp_widget[pos].x1 = atoi(parameter);
//						LOG(LOG_MSG, "..Loading: (%s)\n", keyword);
					}
					else if(!strcmp(keyword, "....Moveable:"))
					{
						tmp_widget[pos].moveable = atoi(parameter);
//						LOG(LOG_MSG, "..Loading: (%s)\n", keyword);
					}
					else if(!strcmp(keyword, "....Active:"))
					{
						tmp_widget[pos].show = atoi(parameter);
//						LOG(LOG_MSG, "..Loading: (%s)\n", keyword);
					}
					else if(!strcmp(keyword, "....Y1:"))
					{
						tmp_widget[pos].y1 = atoi(parameter);
//						LOG(LOG_MSG, "..Loading: (%s)\n", keyword);
					}
					else if(!strcmp(keyword, "....Wd:"))
					{
						tmp_widget[pos].wd = atoi(parameter);
//						LOG(LOG_MSG, "..Loading: (%s)\n", keyword);
					}
					else if(!strcmp(keyword, "....Ht:"))
					{
						tmp_widget[pos].ht = atoi(parameter);
//						LOG(LOG_MSG, "..Loading: (%s)\n", keyword);
					}
				}
			}
		}
	}
	fclose(stream);

	/* test to see if all widgets were found */
	for(pos=0; pos < TOTAL_WIDGETS && found_widget[pos]; ++pos) { }

	/* if all were loaded/found, transfer .. */
	if(pos >= TOTAL_WIDGETS)
	{
		for(pos=0; pos < TOTAL_WIDGETS; ++pos)
			cur_widget[pos] = def_widget[pos] = tmp_widget[pos];
	}
	/* some are missing, don't transfer .. */
	else
	{
		LOG(LOG_MSG, "load_interface_file(): Error! Not all widgets included in interface file: %s\n", filename);

		return FALSE;
	}

//	LOG(LOG_MSG, "..load_interface_file(): Done.\n");

	return TRUE;
}

/* save the widgets/interface to a file */
void save_interface_file(void)
{
	char txtBuffer[20];
	int i=-1;
	FILE *stream;

	/* leave, if there's an error opening or creating */
	if(!(stream = fopen_wrapper(INTERFACE_FILE, "w")))
		return;

	fputs("##############################################\n",stream);
	fputs("# This is the daimonin client interface file #\n",stream);
	fputs("##############################################\n",stream);

	while (++i < TOTAL_WIDGETS)
	{
		/* beginning of block */
		fputs("\nWidget: ", stream);
		fputs(cur_widget[i].name, stream);
		fputs("\n", stream);

		fputs("....Moveable:", stream);
		sprintf(txtBuffer, " %d",  cur_widget[i].moveable);
		fputs(txtBuffer,stream); fputs("\n",stream);

		fputs("....Active:", stream);
		sprintf(txtBuffer, " %d",  cur_widget[i].show);
		fputs(txtBuffer,stream); fputs("\n",stream);

		fputs("....X1:", stream);
		sprintf(txtBuffer, " %d",  cur_widget[i].x1);
		fputs(txtBuffer,stream); fputs("\n",stream);

		fputs("....Y1:", stream);
		sprintf(txtBuffer, " %d",  cur_widget[i].y1);
		fputs(txtBuffer,stream); fputs("\n",stream);

		fputs("....Wd:", stream);
		sprintf(txtBuffer, " %d",  cur_widget[i].wd);
		fputs(txtBuffer,stream); fputs("\n",stream);

		fputs("....Ht:", stream);
		sprintf(txtBuffer, " %d",  cur_widget[i].ht);
		fputs(txtBuffer,stream); fputs("\n",stream);

		/* end of block */
		fputs("....END\n", stream);
	}
	fclose(stream);
}

/* is the widget being moved by the user? if so, let me know! */
/* ..used a function to protect internals */
Boolean IsWidgetDragging()
{
	return widget_event_move.active;
}

/************************ MOUSE DOWN **####*****************/
/* check for owner of mouse focus                            */
/* setup widget dragging, if enabled                         */
/* TODO - right-click.. select 'move' to move a widget..     */
int widget_event_mousedn(int x,int y, SDL_Event *event)
{
	int nID = get_widget_owner(x,y);

	/* setup the event structure in response */
	widget_mouse_event.owner = nID;

	/* sanity check */
	if(nID<0) { return FALSE; }

	/* setup the event structure in response */
	widget_mouse_event.x = x;
	widget_mouse_event.y = y;

	/* set the priority to this widget */
	SetPriorityWidget(nID);

	/* if its moveable, start moving it when the conditions warrant it */
	if(cur_widget[nID].moveable && MouseEvent==RB_DN)
	{
		/* If widget is MOVEABLE, this defines the hotspot areas for activating */
		switch(nID)
		{

			default:
				/* we know this widget owns the mouse.. */
				widget_event_move.active = TRUE;
				break;
		}
		/* start the movement procedures */
		if(widget_event_move.active)
		{
			widget_event_move.id = nID;
			widget_event_move.xOffset = x - cur_widget[nID].x1;
			widget_event_move.yOffset = y - cur_widget[nID].y1;

			/* nothing owns the mouse right now */
			widget_mouse_event.owner = -1;

			/* enable the custom cursor */
			f_custom_cursor = MSCURSOR_MOVE;
			/* hide the system cursor */
			SDL_ShowCursor(0);
		}

		return TRUE;
	}
	/* NORMAL CONDITION - RESPOND TO MOUSEDOWN EVENT */
	else
	{
    /* Place here all the mousedown Handlers */
        switch (nID)
        {
            case SKILL_EXP_ID:
                /* Handle the mousedown on the exp area */
                widget_skill_exp_event(x, y, MOUSE_DOWN);
                break;
            case MENU_B_ID:
                /* Handle mousedown on the menu buttons... */
                widget_menubuttons_event(x, y, MOUSE_DOWN);
                break;
            case QUICKSLOT_ID:
                widget_quickslots_mouse_event(x, y, MOUSE_DOWN);
                break;
            case CHATWIN_ID:
            case MSGWIN_ID:
            case MIXWIN_ID:
                textwin_event(TW_CHECK_BUT_DOWN, event, nID);
                break;
            case GROUP_ID:
//                group_event();
                break;
            case RANGE_ID:
                widget_range_event(x,y, *event, MOUSE_DOWN);
                break;
            case BELOW_INV_ID:
                widget_below_window_event(x,y,MOUSE_DOWN);
                break;
            case TARGET_ID:
                widget_event_target(x, y, *event);
                break;
            case MAIN_INV_ID:
                widget_inventory_event(x, y, *event);
                break;
            case PLAYER_INFO_ID:
                widget_player_data_event(x, y);
                break;
            case IN_NUMBER_ID:
                widget_number_event(x, y, *event);
                break;
        }

		return TRUE;
	}
}


/************************ MOUSE UP **************************/
/* check for owner of mouse focus                           */
/* stop dragging the widget, if active                      */
int widget_event_mouseup(int x, int y, SDL_Event *event)
{

	/* widget moving condition */
	if(widget_event_move.active)
	{
		widget_event_move.active = FALSE;
		widget_mouse_event.owner = widget_event_move.id;
		widget_mouse_event.x = x;
		widget_mouse_event.y = y;

		/* disable the custom cursor */
		f_custom_cursor = 0;
		/* show the system cursor */
		SDL_ShowCursor(1);

		/* the interface has changed, save it! */
//		if(options.auto_save_interface) { save_interface_file(); }

		return TRUE;
	}
	/* NORMAL CONDITION - RESPOND TO MOUSEUP EVENT */
	else
	{
		int nID = get_widget_owner(x,y);

		/* setup the event structure in response */
		widget_mouse_event.owner = nID;

        /* handler(s) for miscellanous mouse movement(s) go here */

		/* sanity check.. return if mouse is not in a widget */
		if(nID<0) { return FALSE; }
        else
        {
		    /* setup the event structure in response */
		    widget_mouse_event.x = x;
		    widget_mouse_event.y = y;
        }
		/* handler(s) for the widgets go here */

        switch (nID)
        {
            /* drop to quickslots */
            case QUICKSLOT_ID:
                widget_quickslots_mouse_event(x,y,MOUSE_UP);
                break;
            case CHATWIN_ID:
            case MSGWIN_ID:
            case MIXWIN_ID:
                textwin_event(TW_CHECK_BUT_UP, event, nID);
                break;
            case GROUP_ID:
//                group_event();
                break;
            case PDOLL_ID:
                    widget_show_player_doll_event(x,y, MOUSE_UP);
                break;
            case RANGE_ID:
                widget_range_event(x,y, *event, MOUSE_UP);
                break;
            case MAIN_INV_ID:
                widget_inventory_event(x, y, *event);
                break;
        }
		return TRUE;
	}
}


/******************************* MOUSE MOVE **************************/
/* check for owner of mouse focus                                    */
/* drag the widget, if active                                        */
int widget_event_mousemv(int x,int y, SDL_Event *event)
{
    cursor_type = 0; /* with widgets we have to clear every loop the txtwin cursor */

	/* widget moving condition */
	if(widget_event_move.active)
	{
		cur_widget[widget_event_move.id].x1 = x - widget_event_move.xOffset;
		cur_widget[widget_event_move.id].y1 = y - widget_event_move.yOffset;

        map_udate_flag = 2;
		return TRUE;
	}
	/* NORMAL CONDITION - RESPOND TO MOUSEMOVE EVENT */
	else
	{
		int nID = get_widget_owner(x,y);

		/* setup the event structure in response */
		widget_mouse_event.owner = nID;

        /* handler(s) for miscellanous mouse movement(s) go here */


        /* textwin special handling */
        if (txtwin[TW_CHAT].highlight != TW_HL_NONE)
        {
            txtwin[TW_CHAT].highlight = TW_HL_NONE;
            WIDGET_REDRAW(CHATWIN_ID);
        }
        if (txtwin[TW_MSG].highlight != TW_HL_NONE)
        {
            txtwin[TW_MSG].highlight = TW_HL_NONE;
            WIDGET_REDRAW(MSGWIN_ID);
        }
		/* sanity check.. return if mouse is not in a widget */
		if(nID<0) { return FALSE; }
        else
        {
		    /* setup the event structure in response */
		    widget_mouse_event.x = x;
		    widget_mouse_event.y = y;
        }

		/* handler(s) for the widgets move go here */

        switch (nID)
        {
            case CHATWIN_ID:
            case MSGWIN_ID:
            case MIXWIN_ID:
                textwin_event(TW_CHECK_MOVE, event, nID);
                break;
            case MAIN_INV_ID:
                widget_inventory_event(x, y, *event);
                break;
        }

		return TRUE;
	}
}

/* find the widget with mouse focus on a mouse-hit-test basis */
int get_widget_owner(int x,int y )
{
	widget_node *node;
	int nID;

    /* Priority overide function, we have to have that here for resizing.... */
    if (textwin_flags & TW_RESIZE)
    {
        if (textwin_flags & TW_CHAT)
            return CHATWIN_ID;
        else if (textwin_flags & TW_MSG)
            return MSGWIN_ID;
        else if (textwin_flags & TW_MIX)
            return MIXWIN_ID;
    }

	/* mouse cannot be used by widgets */
	if(IsMouseExclusive)
	{
	    return -1;
    }

	/* priority list doesn't exist */
	if(!priority_list_head) { return -1; }

	/* loop through the list & perform custom or default hit-test */
	for(node=priority_list_head;node;node=node->next)
	{
		nID = node->WidgetID;

        if (!cur_widget[nID].show) continue;

		switch(nID)
		{

            case PDOLL_ID: /*Playerdoll widget is NOT a rectangle, handle special... */
                if (x>cur_widget[nID].x1+111)
                {
                    if (    x <= cur_widget[nID].x1 + cur_widget[nID].wd
                        &&  y >= cur_widget[nID].y1
                        &&  y <= ((x-(cur_widget[nID].x1+111))/-2)+215+cur_widget[nID].y1)
                            return nID;
                            break;
                }
                else
                {
                    if (    x >= cur_widget[nID].x1
                        &&  y >= cur_widget[nID].y1
                        &&  y <= ((x-cur_widget[nID].x1)/2)+160+cur_widget[nID].y1)
                            return nID;
                            break;
                }

			default:
				if(	x >= cur_widget[nID].x1 &&
					x <= (cur_widget[nID].x1 + cur_widget[nID].wd) &&
					y >= cur_widget[nID].y1 &&
					y <= (cur_widget[nID].y1 + cur_widget[nID].ht) )
				{
					return nID;
				}
				break;
		}
	}

	return -1;
}

/* function list for each widget - calls the widget with the process type */
void process_widget(int nID, int proc)
{
	switch(nID) /* doesn't matter which order the case statements follow */
	{
		case STATS_ID:
				widget_player_stats(cur_widget[nID].x1,cur_widget[nID].y1);
			break;
		case RESIST_ID:
				widget_show_resist(cur_widget[nID].x1,cur_widget[nID].y1);
			break;
		case MAIN_LVL_ID:
				widget_show_main_lvl(cur_widget[nID].x1,cur_widget[nID].y1);
			break;
		case SKILL_EXP_ID:
				widget_show_skill_exp(cur_widget[nID].x1,cur_widget[nID].y1);
			break;
		case REGEN_ID:
				widget_show_regeneration(cur_widget[nID].x1,cur_widget[nID].y1);
			break;
		case SKILL_LVL_ID:
				widget_skillgroups(cur_widget[nID].x1,cur_widget[nID].y1);
			break;
		case MENU_B_ID:
				widget_menubuttons(cur_widget[nID].x1,cur_widget[nID].y1);
			break;
		case QUICKSLOT_ID:
				widget_quickslots(cur_widget[nID].x1,cur_widget[nID].y1);
			break;
		case CHATWIN_ID:
				widget_textwin_show(cur_widget[nID].x1,cur_widget[nID].y1, TW_CHAT);
			break;
		case MSGWIN_ID:
				widget_textwin_show(cur_widget[nID].x1,cur_widget[nID].y1, TW_MSG);
			break;
		case MIXWIN_ID:
				widget_textwin_show(cur_widget[nID].x1,cur_widget[nID].y1, TW_MIX);
			break;
		case GROUP_ID:
				widget_show_group(cur_widget[nID].x1,cur_widget[nID].y1);
			break;
		case PDOLL_ID:
				widget_show_player_doll(cur_widget[nID].x1,cur_widget[nID].y1);
			break;
		case BELOW_INV_ID:
				widget_show_below_window(cpl.below, cur_widget[nID].x1,cur_widget[nID].y1);
			break;
		case PLAYER_INFO_ID:
				widget_show_player_data(cur_widget[nID].x1,cur_widget[nID].y1);
			break;
		case RANGE_ID:
				widget_show_range(cur_widget[nID].x1,cur_widget[nID].y1);
			break;
		case TARGET_ID:
				widget_show_target(cur_widget[nID].x1,cur_widget[nID].y1);
			break;
        case MAIN_INV_ID:
                widget_show_inventory_window(cur_widget[nID].x1,cur_widget[nID].y1);
            break;
        case MAPNAME_ID:
                widget_show_mapname(cur_widget[nID].x1,cur_widget[nID].y1);
            break;
        case IN_CONSOLE_ID:
                widget_show_console(cur_widget[nID].x1,cur_widget[nID].y1);
            break;
        case IN_NUMBER_ID:
                widget_show_number(cur_widget[nID].x1,cur_widget[nID].y1);
            break;
        case STATOMETER_ID:
                widget_show_statometer(cur_widget[nID].x1, cur_widget[nID].y1);
            break;

	}
}

/* loop through all the widgets and call the corresponding handlers */
/* this is called everytime in main.c.. in the main loop */
void process_widgets()
{
	widget_node *node;
	int nID;

	/* sanity checks */
	if(!priority_list_head) { return; }
	if(!priority_list_foot) { return; }

	for(node=priority_list_foot;node;node=node->prev)
	{
		nID = node->WidgetID;
        if (cur_widget[nID].show)
		    process_widget(nID, PROCESS);
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
uint32 GetMouseState(int *mx, int *my, int widget_id)
{
	if(widget_mouse_event.owner == widget_id && !IsMouseExclusive)
	{
		/********************************************************
		* continue only when no menu is active.
		*********************************************************/
		if (cpl.menustatus != MENU_NO || esc_menu_flag) return 0;

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
void SetPriorityWidget(int nWidgetID)
{
	widget_node *node;

	LOG(LOG_MSG,"Entering SetPriorityWidget(nWidgetID=%d)..\n",nWidgetID);

	/* sanity check */
	if(nWidgetID < 0 || nWidgetID >= TOTAL_WIDGETS) { return; }

	/* exit, if already highest priority */
	if(priority_list_head->WidgetID == nWidgetID) { return; }


	/* move the current highest to second highest priority */
	node = (widget_node *) _malloc (sizeof(widget_node),"SetPriorityWidget: widget_node");
	if(!node) { exit(0); } /* memory error */
	*node = *priority_list_head;
	node->prev = priority_list_head;
	node->next->prev = node;
	priority_list_head->next = node;
	cur_widget[node->WidgetID].priority_index = node;

	/* make this widget have highest priority */
	priority_list_head->WidgetID = nWidgetID;

	/* remove it from its previous priority */
	node = cur_widget[nWidgetID].priority_index;

	LOG(LOG_MSG, "..node: %d\n", node);
	LOG(LOG_MSG, "..cur_widget[nWidgetID].priority_index: %d\n", cur_widget[nWidgetID].priority_index);
	LOG(LOG_MSG, "..node->prev: %d, node->next: %d\n", node->prev, node->next);

	if(node->next)
	{
		node->next->prev = node->prev;
		node->prev->next = node->next;
	}
	else /* foot of list */
	{
		/* update the foot of priority list */
		priority_list_foot = node->prev;
		node->prev->next = NULL;
	}

	free(node);

	/* re-link the widget lookup */
	cur_widget[nWidgetID].priority_index = priority_list_head;

	LOG(LOG_MSG, "..SetPriorityWidget(): Done.\n");
}

