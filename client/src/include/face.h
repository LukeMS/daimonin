/*
    Daimonin SDL client, a client program for the Daimonin MMORPG.


  Copyright (C) 2003 Michael Toennies

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

#ifndef __FACE_H
#define __FACE_H

#define FACE_MAX_NROF 20000

#define FACE_FLAG_LOADED      1 << 1 // face loaded in memory
#define FACE_FLAG_REQUESTED   1 << 2 // face requested from server - do it only one time
#define FACE_FLAG_ALTERNATIVE 1 << 3 // this has an alternative image
#define FACE_FLAG_DOUBLE      1 << 4 // this is a double wall type
#define FACE_FLAG_UP          1 << 5 // this is a upper part of something
#define FACE_FLAG_D1          1 << 6 // this is a x1x object (animation or direction)
#define FACE_FLAG_D3          1 << 7 // this is a x3x object (animation or direction)

typedef struct face_t
{
    char           *name;
    int             pos;
    int             len;
    uint32          crc;
    struct _Sprite *sprite;
    uint16          alt_a;    /* index of alternative face */
    uint16          alt_b;    /* index of alternative face */
    uint8           flags;
}
face_t;

/* table of pre definded multi arch objects.
 * mpart_id and mpart_nr in the arches are commited from server
 * to analyze the exaclty tile position inside a mpart object.
 *
 * The way of determinate the starting and shift points is explained
 * in the dev/multi_arch folder of the arches, where the multi arch templates &
 * masks are.
 */

typedef struct face_mpart_nr_t
{
    uint16 xoff;
    uint16 yoff;
}
face_mpart_nr_t;

typedef struct face_mpart_id_t
{
    uint16          xlen;
    uint16          ylen;
    face_mpart_nr_t part[16];
}
face_mpart_id_t;

extern face_t          face_list[FACE_MAX_NROF];
extern uint16          face_nrof;
extern face_mpart_id_t face_mpart_id[16];

extern void   face_saveinfo(uint16 num, uint32 crc, const char *name);
extern void   face_save(uint16 num, uint8 *data, uint32 len);
extern sint32 face_find(const char *name);
extern void   face_get(sint32 num);

#endif /* ifndef __FACE_H */
