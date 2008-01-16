/*
    Daimonin Updater, a service program for the Daimonin MMORPG.


  Copyright (C) 2002-2005 Michael Toennies

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

#include "include/include.h"

int check_tools(char *name)
{
    struct stat stat_buf;
    int res;
    res = stat(name, &stat_buf);
    if (res < 0)
    {
        fprintf(stderr, "Can't find %s\n", name);
        return FALSE;
    }
        return TRUE;
}

int execute_process(char *p_path, char *exe_name, char *parms, char *output, int seconds_to_wait)
{
    int ret = 0;
    char cmd[BUFSIZ];
    char buf[BUFSIZ];
    FILE *ptr;

    sprintf(cmd, "%s %s", exe_name, parms);
    if(output)
            *output='\0';

    if ((ptr = popen(cmd, "r")) != NULL)
    {
        while (fgets(buf, BUFSIZ, ptr) != NULL)
        {
            if(output)
                strcat(output, buf);
            else
                printf("%s", buf);
        }

        pclose(ptr);
    }

    return ret;
}
