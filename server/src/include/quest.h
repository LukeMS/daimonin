/*
    Daimonin, the Massive Multiuser Online Role Playing Game
    Server Applicatiom

    Copyright (C) 2001 Michael Toennies

    A split from Crossfire, a Multiplayer game for X-windows.

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
#ifndef __QUEST_H
#define __QUEST_H

extern void insert_quest_item(struct obj *quest_trigger, struct obj *target);
extern void add_quest_containers(struct obj *op);
extern void add_quest_trigger(struct obj *who, struct obj *trigger);
extern void set_quest_status(struct obj *trigger, int q_status, int q_type);
extern void check_kill_quest_event(struct obj *pl, struct obj *op);
extern void check_cont_quest_event(struct obj *pl, struct obj *op);
extern int get_nrof_quest_item(const struct obj *target, const char *aname, const char *name, const char *title);

#endif
