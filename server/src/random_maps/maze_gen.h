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


/*#include <random_map.h> */
char   **maze_gen(int xsize, int ysize, int option);  /* the outside interface routine */
void    fill_maze_full(char **maze, int x, int y, int xsize, int ysize);
void    fill_maze_sparse(char **maze, int x, int y, int xsize, int ysize);
void    make_wall_free_list(int xsize, int ysize);
void    pop_wall_point(int *x, int *y);
int     find_free_point(char **maze, int *x, int *y, int xc, int yc, int xsize, int ysize);
