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
#include <include.h>

#ifdef __WIN_32

char *file_path(const char *fname, const char *mode)
{
    static char tmp[256];
    sprintf(tmp, "%s%s", SYSPATH, fname);
    return tmp;
}

#else

/**
* Create the directory @p path.  If it already exists as a directory
* we succeed.
* Recursive mkdir is from the Gal project's e_mkdir_hier() function.
**/

static int mkdir_recurse(const char *path)
{
    char *copy, *p;

    p = copy = strdup(path);
    do
    {
        p = strchr (p + 1, '/');
        if (p)
            *p = '\0';
        if (access (copy, F_OK) == -1)
        {
            if (mkdir (copy, 0755) == -1)
            {
                return -1;
            }
        }
        if (p)
            *p = '/';
    }
    while (p);
    return 0;
}

char *file_path(const char *fname, const char *mode)
{
    static char tmp[256];
    char *stmp;
    char ctmp;

    sprintf(tmp, "%s/.daimonin/%s", getenv("HOME"), fname);

    if (strchr(mode, 'w'))
    { // overwrite (always use file in home dir)
        if ((stmp=strrchr(tmp, '/')))
        {
            ctmp = stmp[0];
            stmp[0] = 0;
            mkdir_recurse(tmp);
            stmp[0] = ctmp;
        }
    }
    else if (strchr(mode, '+') || strchr(mode, 'a'))
    { // modify (copy base file to home dir if not exists)
        if (access(tmp, W_OK))
        {
            char otmp[256];
            char shtmp[517];

            sprintf(otmp, "%s%s", SYSPATH, fname);
            if ((stmp=strrchr(tmp, '/')))
            {
                ctmp = stmp[0];
                stmp[0] = 0;
                mkdir_recurse(tmp);
                stmp[0] = ctmp;
            }

            /* Copy base file to home directory */
            sprintf(shtmp, "cp %s %s", otmp, tmp);
            system(shtmp);

        }
    }
    else
    { // just read (check home dir first, then system dir)
        if (access(tmp, R_OK))
            sprintf(tmp, "%s%s", SYSPATH, fname);
    }

    //    printf("file_path: %s (%s) => %s\n", fname, mode, tmp);

    return tmp;
}
#endif

FILE *fopen_wrapper(const char *fname, const char *mode)
{
    return fopen(file_path(fname, mode), mode);
}

SDL_Surface *IMG_Load_wrapper (const char *file)
{
    return IMG_Load(file_path(file, "r"));
}

#ifdef INSTALL_SOUND
Mix_Chunk *Mix_LoadWAV_wrapper(const char *fname)
{
    return Mix_LoadWAV(file_path(fname, "r"));
}

Mix_Music *Mix_LoadMUS_wrapper(const char *file)
{
    return Mix_LoadMUS(file_path(file, "r"));
}
#endif
