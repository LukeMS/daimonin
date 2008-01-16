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
#if !defined(__INCLUDE_H)
#define __INCLUDE_H

#ifdef WIN32
#include <win32.h>
#endif
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <setjmp.h>
#include <stdlib.h>

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef HAVE_LIBDMALLOC
#include <dmalloc.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#if defined(HAVE_TIME_H) && defined(TIME_WITH_SYS_TIME)
#include <time.h>
#endif

/* stddef is for offsetof */
#ifdef HAVE_STDDEF_H
#include <stddef.h>
#endif


#include <sys/types.h>
#include <sys/stat.h>

#ifdef WIN32
#include <io.h>
#include <sys/locking.h>
#include <share.h>
#else
#include <dirent.h>
#include <sys/param.h>
#include <unistd.h>
#endif

#include <fcntl.h>

#ifndef FALSE
#define FALSE 0
#define TRUE (!FALSE)
#endif


#define BZ_VERBOSE 2

extern int zip_extract(char *zipArchive, char *destDir);

extern int execute_process(char *p_path, char *exe_name, char *parms, char *output, int seconds_to_wait);

#ifndef WIN32
extern int check_tools(char *name);
#define p_popen popen
#define p_pclose pclose
#endif
#endif
