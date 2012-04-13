/*
    Daimonin SDL client, a client program for the Daimonin MMORPG.


  Copyright (C) 2012 Michael Toennies

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

#ifndef __WDH_H
#define __WDH_H

/* Process handlers. */

extern void wdh_process_pdoll(widget_id_t id);
extern void wdh_process_pinfo(widget_id_t id);
extern void wdh_process_stats(widget_id_t id);
extern void wdh_process_main_lvl(widget_id_t id);
extern void wdh_process_skill_exp(widget_id_t id);
extern void wdh_process_regen(widget_id_t id);
extern void wdh_process_skill_lvl(widget_id_t id);
extern void wdh_process_menu_b(widget_id_t id);
extern void wdh_process_statometer(widget_id_t id);
extern void wdh_process_chatwin(widget_id_t id);
extern void wdh_process_msgwin(widget_id_t id);
extern void wdh_process_number(widget_id_t id);
extern void wdh_process_console(widget_id_t id);
extern void wdh_process_resist(widget_id_t id);
extern void wdh_process_mapname(widget_id_t id);
extern void wdh_process_range(widget_id_t id);
extern void wdh_process_quickslots(widget_id_t id);
extern void wdh_process_target(widget_id_t id);
extern void wdh_process_main_inv(widget_id_t id);
extern void wdh_process_below_inv(widget_id_t id);
extern void wdh_process_group(widget_id_t id);

/* Event handlers. */

extern void wdh_event_pdoll(widget_id_t id, SDL_Event *e);
extern void wdh_event_pinfo(widget_id_t id, SDL_Event *e);
extern void wdh_event_menu_b(widget_id_t id, SDL_Event *e);
extern void wdh_event_skill_exp(widget_id_t id, SDL_Event *e);
extern void wdh_event_chatwin(widget_id_t id, SDL_Event *e);
extern void wdh_event_msgwin(widget_id_t id, SDL_Event *e);
extern void wdh_event_quickslots(widget_id_t id, SDL_Event *e);
extern void wdh_event_main_inv(widget_id_t id, SDL_Event *e);
extern void wdh_event_below_inv(widget_id_t id, SDL_Event *e);
extern void wdh_event_range(widget_id_t id, SDL_Event *e);
extern void wdh_event_target(widget_id_t id, SDL_Event *e);
extern void wdh_event_number(widget_id_t id, SDL_Event *e);

#endif /* ifndef __WDH_H */
