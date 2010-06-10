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

    The author can be reached via e-mail to info@daimonin.org
*/
#ifndef __QUEST_H
#define __QUEST_H

/* how much open/pending quests a player can have at once (and which are listed in the quest list) */
#define QUESTS_PENDING_MAX 15

extern void insert_quest_item(struct obj *quest_trigger, struct obj *target);
extern void add_quest_containers(struct obj *op);
extern void add_quest_trigger(struct obj *who, struct obj *trigger);
extern void set_quest_status(struct obj *trigger, int q_status, int q_type);
extern int update_quest(struct obj *trigger, char *text, char *vim);
extern void check_kill_quest_event(struct obj *pl, struct obj *op);
extern void check_cont_quest_event(struct obj *pl, struct obj *op);
extern uint32 get_nrof_quest_item(const struct obj *target, const char *aname, const char *name, const char *title);
extern int quest_count_pending(const struct obj *pl);
extern struct obj *quest_find_name(const struct obj *pl, const char *name);
extern void send_quest_list(struct obj *pl);
extern void quest_list_command(struct obj *pl, char *cmd);

#endif
