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

#ifndef __LINKS_H
#define __LINKS_H

/* oblink union is used as */
#define OBJLNK_FLAG_OB      (1 << 0)
#define OBJLNK_FLAG_LINK    (1 << 1)
#define OBJLNK_FLAG_TL      (1 << 2)
#define OBJLNK_FLAG_GM      (1 << 3)
#define OBJLNK_FLAG_BAN     (1 << 4)

/* The use of _STATIC and _REF defines how we handle instancing & freeing.
 * Example: When we loading the base arches, we
 * link the treasure lists with static set objectlink_t
 * structures. When we now creating a object as instance
 * from that base arch, the treasure list pointer is instanced.
 * If we free that object we skip freeing the treasure list
 * because its only a pointer to a base arch treasure list.
 * _REF is used when we create a custom treasure list when loading
 * a object (see loader.c). We need to use a ref_count to handle
 * instances from that custom list (to avoid recreating the same
 * treasure list for every copy_object() ).
 * That engine is usef
 * TODO: We don't have a global list of custom designed treasure list,
 * like we do with the hash list for strings. Its possible - perhaps
 * its useful and speed up things but atm we don't have to much of them.
 */
#define OBJLNK_FLAG_STATIC  (1 << 12)
#define OBJLNK_FLAG_REF     (1 << 13)

/* Used to link together several objects */

/* I created this improved link structures using the
 * original objectlink and objectlinkpt structures.
 * It use now one multi structure and the mempool functions.
 * This link object system can easily be extended by adding new
 * entries in the union{}objlink part, It should be used everywhere
 * we want using lists of objects (examples are treasure lists,
 * friendly/enemy list, button link list...). MT-2004
 */
struct objectlink_t
{
    union
    {
        objectlink_t               *link;
        object_t                     *ob;
        treasurelist_t  *tl;
        struct _gmaster_struct       *gm;
        ban_t           *ban;
    } objlink;

    objectlink_t               *prev;
    objectlink_t               *next;

    tag_t                       id;
    int                         value;
    uint32                      flags;
    uint32                      ref_count;
    union
    {                                            /* a local link paramter */
        struct _tlist_tweak        *tl_tweak;
    } parmlink;
};

extern objectlink_t *objectlink_get(int id);
extern void          objectlink_free(objectlink_t *ol);
extern objectlink_t *objectlink_link(objectlink_t **startptr, objectlink_t **endptr, objectlink_t *afterptr, objectlink_t *beforeptr, objectlink_t *objptr);
extern objectlink_t *objectlink_unlink(objectlink_t **startptr, objectlink_t **endptr, objectlink_t *objptr);

#endif /* ifndef __LINKS_H */
