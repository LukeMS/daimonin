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

FILE *fopen_wrapper(char *fname, char *mode) {
  char tmp[256];
  sprintf(tmp, "%s%s", SYSPATH, fname);
  return fopen(fname, mode);
}
#endif



#ifndef __WIN_32

/**
 * Create the directory @p path.  If it already exists as a directory
 * we succeed.
 * Recursive mkdir is from the Gal project's e_mkdir_hier() function.
 **/

int mkdir_recurse(const char *path)
{
       char *copy, *p;

       p = copy = strdup(path);
       do {
               p = strchr (p + 1, '/');
               if (p)
                       *p = '\0';
               if (access (copy, F_OK) == -1) {
                       if (mkdir (copy, 0755) == -1) {
                               return -1;
                       }
               }
               if (p)
                       *p = '/';
       } while (p);
       return 0;
}

FILE *fopen_wrapper(char *fname, char *mode) {
  FILE *f;
  char tmp[256];
  char otmp[256];
  char shtmp[517];
  char *stmp;
  char ctmp;

//  printf("fopen_wrapper: %s, %s | ", fname, mode);

  if(strchr(mode, 'w')) { // overwrite
    sprintf(tmp, "%s/.daimonin/%s", getenv("HOME"), fname);
    if(stmp=strrchr(tmp, '/')) {
      ctmp = stmp[0];
      stmp[0] = 0;
      mkdir_recurse(tmp);
      stmp[0] = ctmp;
    }
    f=fopen(tmp,mode);
  }
  else if(strchr(mode, '+') || strchr(mode, 'a')) { // modify
    sprintf(tmp, "%s/.daimonin/%s", getenv("HOME"), fname);
    if(!(f=fopen(tmp, mode))) {
      sprintf(otmp, "%s%s", SYSPATH, fname);
      if(stmp=strrchr(tmp, '/')) {
        ctmp = stmp[0];
        stmp[0] = 0;
        mkdir_recurse(tmp);
        stmp[0] = ctmp;
      }
      sprintf(shtmp, "cp %s %s", otmp, tmp);
      system(shtmp);
      f=fopen(tmp,mode);
    }
  }
  else { // just read
    sprintf(tmp, "%s/.daimonin/%s", getenv("HOME"), fname);
    if(!(f=fopen(tmp, mode))) {
      sprintf(tmp, "%s%s", SYSPATH, fname);
      f=fopen(tmp, mode);
    }
  }

//  printf("final file: %s\n", tmp);
  return f;
}
#endif

