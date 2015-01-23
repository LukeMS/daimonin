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

/* Quest status codes. */
#define QSTAT_UNKNOWN  0
#define QSTAT_NO       1
#define QSTAT_ACTIVE   2
#define QSTAT_SOLVED   3
#define QSTAT_DONE     4
#define QSTAT_DISALLOW 5

extern void insert_quest_item(object_t *quest_trigger, object_t *target);
extern void add_quest_containers(object_t *op);
extern void add_quest_trigger(object_t *who, object_t *trigger);
extern void set_quest_status(object_t *trigger, int q_status, int q_type);
extern int quest_get_active_status(player_t *pl, object_t *trigger);
extern int update_quest(object_t *trigger, uint8 subtype, object_t *info, char *text, char *vim);
extern void check_kill_quest_event(object_t *pl, object_t *op);
extern void check_cont_quest_event(object_t *pl, object_t *op);
extern uint32 get_nrof_quest_item(const object_t *target, const char *aname, const char *name, const char *title);
extern int quest_count_pending(const object_t *pl);
extern object_t *quest_find_name(const object_t *pl, const char *name);
extern void send_quest_list(object_t *pl);
extern void quest_list_command(object_t *pl, char *cmd);

#endif /* ifndef __QUEST_H */
