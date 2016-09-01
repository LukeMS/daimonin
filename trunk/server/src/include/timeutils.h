/*
    Daimonin, the Massive Multiuser Online Role Playing Game
    Server Applicatiom

    Copyright (C) 2001-2005 Michael Toennies

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

#ifndef __TIME_H
#define __TIME_H

extern void regenerate_rod(object_t *rod);
extern void remove_force(object_t *op);
extern void poison_more(object_t *op);
extern void move_gate(object_t *op);
extern void move_timed_gate(object_t *op);
extern void move_detector(object_t *op);
extern void move_conn_sensor(object_t *op);
extern void move_environment_sensor(object_t *op);
extern void animate_trigger(object_t *op);
extern void move_pit(object_t *op);
extern void change_object(object_t *op);
extern void move_teleporter(object_t *op);
extern void move_firewall(object_t *op);
extern void move_player_mover(object_t *op);
extern void move_creator(object_t *op);
extern void move_marker(object_t *op);
extern int  process_object(object_t *op);

#endif /* ifndef __TIME_H */
