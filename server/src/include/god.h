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

#ifndef __GOD_H
#define __GOD_H

struct godlink_t
{
    /* Used to link together the gods */
    const char     *name;           /* name of this god */
    archetype_t   *arch;       /* pointer to the archetype of this god */
    int             id;                 /* id of the god */
    char           *pantheon;           /* the name of the group this god belongs to */
    godlink_t    *next;
};

extern void        init_gods(void);
extern void        add_god_to_list(archetype_t *god_arch);
extern int         baptize_altar(object_t *op);
extern godlink_t  *get_rand_god(void);
extern object_t   *pntr_to_god_obj(godlink_t *godlnk);
extern void        free_all_god(void);
extern void        dump_gods(void);
extern int         lookup_god_by_name(const char *name);
extern const char *determine_god(object_t *op);
extern object_t   *find_god(const char *name);
extern void        pray_at_altar(object_t *pl, object_t *altar);
extern void        become_follower(object_t *op, object_t *new_god);

#endif /* ifndef __GOD_H */
