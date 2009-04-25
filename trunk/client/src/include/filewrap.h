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

    The author can be reached via e-mail to info@daimonin.net
*/

#if !defined(__FILEWRAP_H)
#define __FILEWRAP_H

FILE *fopen_wrapper(const char *fname, const char *mode);
SDL_Surface *IMG_Load_wrapper (const char *file);
#ifdef INSTALL_SOUND
Mix_Chunk *Mix_LoadWAV_wrapper(const char *fname);
Mix_Music *Mix_LoadMUS_wrapper(const char *file);
#endif
char *file_path(const char *fname, const char *mode);

#endif
