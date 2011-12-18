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

#ifndef __MISC_H
#define __MISC_H

#define MAX_INPUT_STR 256

extern void         show_input_string(_font *font, SDL_Rect *box, char repl);
extern int          read_substr_char(char *srcstr, char *desstr, int *sz, char ct);
extern char        *get_parameter_string(char *data, int *pos, int maxlen);
extern int          isqrt(int n);
extern void         smiley_convert(char *msg);
extern void         markdmbuster();
extern char        *normalize_string(const char *string);
extern int          setup_endian_sync(const char *const buf);
extern uint32       adjust_endian_int32(const uint32 buf);
extern uint16       adjust_endian_int16(const uint16 buf);
extern char        *adjust_string(char *buf);
extern PHYSFS_File *load_client_file(const char *fname);
extern PHYSFS_File *save_client_file(const char *fname);
extern uint32       percentage_colr(sint8 percentage);

#endif /* ifndef __MISC_H */
