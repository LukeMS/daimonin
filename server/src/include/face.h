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

/* New face structure - this enforces the notion that data is face by
 * face only - you can not change the color of an item - you need to instead
 * create a new face with that color.
 */
typedef struct new_face_struct
{
    const char *name;
    uint16      number;     /* This is the image id.  It should be the */
    /* same value as its position in the array */
} New_Face;

typedef struct map_look_struct
{
    New_Face   *face;
    uint8       flags;
} MapLook;


typedef struct
{
    const char *name;       /* Name of the animation sequence */
    Fontindex  *faces;       /* The different animations */
    uint16      num;             /* Where we are in the array */
    uint8       num_animations;   /* How many different faces to animate */
    uint8       facings;          /* How many facings (9 and 25 are allowed only with the new ext anim system ) */
} Animations;


