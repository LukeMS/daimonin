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
typedef struct artifactstruct {
	char					*parse_text; /* memory block with artifacts parse commands for loader.c */
	char					*name; /* thats the fake arch name when chained to arch list */
	char					*def_at_name; /* we use it as marker for def_at is valid and quick name access */
	struct artifactstruct	*next;
	linked_char				*allowed;
	archetype				def_at; /* thats the base archtype object - this is chained to arch list */
	int						t_style;
	uint16					chance;
	uint8					difficulty;
} artifact;

typedef struct artifactliststruct {
	struct artifactliststruct	*next;
	struct artifactstruct		*items;
	uint16	total_chance;	/* sum of chance for are artifacts on this list */
	sint16	type;			/* Object type that this list represents. 
							 * -1 are "Allowed none" items. They are called explicit by name
							 */
} artifactlist;
