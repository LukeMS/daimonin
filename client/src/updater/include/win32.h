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
/*
 * Structures and types used to implement opendir/readdir/closedir
 * on Windows 95/NT and set the loe level defines
 */

#if !defined(AFX_STDAFX_H__31666CA1_2474_11D5_AE6C_F07569C10000__INCLUDED_)
#define AFX_STDAFX_H__31666CA1_2474_11D5_AE6C_F07569C10000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif /* _MSC_VER > 1000 */

#endif

#define WIN32_LEAN_AND_MEAN
/* includes for VC - plz add other include settings for different compiler
 * when needed and comment it
 */
#include <winsock2.h>
/* sometimes wanted from curl, original in WS2TCPIP.H */
typedef int socklen_t;

#include <time.h>
#include <direct.h>
#include <math.h>
#include <sys/stat.h>   /* odd: but you don't get stat here with __STDC__ */
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <process.h>

#ifndef MINGW
#define __STDC__ 1      /* odd too: CF want this, but don't include it */
/* before the standard includes */
#endif

#ifndef HAVE_SNPRINTF
#define HAVE_SNPRINTF 1
#define snprintf _snprintf
#endif


/* include all needed autoconfig.h defines */
#define HAVE_STRICMP
#define HAVE_STRNICMP

#define CS_LOGSTATS
#define HAVE_SRAND
#ifndef HAVE_FCNTL_H
#define HAVE_FCNTL_H
#endif
#ifndef HAVE_STDDEF_H
#define HAVE_STDDEF_H
#endif
#define GETTIMEOFDAY_TWO_ARGS
#define MAXPATHLEN 256

/* Many defines to redirect unix functions or fake standard unix values */
#define inline __inline
#define unlink(__a) _unlink(__a)
#define mkdir(__a, __b) _mkdir(__a)
#define rmdir(__a) _rmdir(__a)
#define getpid() _getpid()
#define popen(__a, __b) _popen(__a, __b)
#define pclose(__a) _pclose(__a)
#define strdup(__a) _strdup(__a)
#define stricmp(__a, __b) _stricmp(__a, __b)
#define strnicmp(__a, __b, __c) _strnicmp(__a, __b, __c)
#define getcwd(__a, __b) _getcwd(__a, __b)
#define chdir(__a) _chdir(__a)
#define access(__a, __b) _access(__a, __b)
#define chmod(__a, __b) _chmod(__a, __b)
#define hypot(__a, __b) _hypot(__a, __b)
#define umask(__a) _umask(__a)
#define lseek(__a, __b, __c) _lseek(__a, __b, __c)


#define PREFIXDIR ""

#ifndef MINGW
#define fileno(__a) _fileno(__a)
#define S_ISDIR(x) (((x) & S_IFMT) == S_IFDIR)
#define S_ISREG(x) (((x) & S_IFMT) == S_IFREG)
#endif

#ifndef S_ISGID
#define S_ISGID 0002000
#endif
#ifndef S_IWOTH
#define S_IWOTH 0000200
#endif
#ifndef S_IWGRP
#define S_IWGRP 0000020
#endif
#ifndef S_IWUSR
#define S_IWUSR 0000002
#endif
#ifndef S_IROTH
#define S_IROTH 0000400
#endif
#ifndef S_IRGRP
#define S_IRGRP 0000040
#endif
#ifndef S_IRUSR
#define S_IRUSR 0000004
#endif

/* define 64bit handling */
#ifndef int64
#define int64 __int64
#endif
#ifndef atoll
#define atoll _atoi64
#endif

/* struct dirent - same as Unix */
typedef struct dirent
{
    long            d_ino;                  /* inode (always 1 in WIN32) */
    off_t           d_off;              /* offset to this dirent */
    unsigned short  d_reclen;   /* length of d_name */
    char            d_name[_MAX_FNAME + 1]; /* filename (null terminated) */
}
dirent;

#define NAMLEN(dirent) strlen((dirent)->d_name)

/* typedef DIR - not the same as Unix */
typedef struct
{
    long                    handle;             /* _findfirst/_findnext handle */
    short                   offset;             /* offset into directory */
    short                   finished;               /* 1 if there are not more files */
    struct _finddata_t      fileinfo;   /* from _findfirst/_findnext */
    char                   *dir;                    /* the dir we are reading */
    struct dirent           dent;           /* the dirent to return */
}
DIR;

struct timezone
{
    int tz_minuteswest;
    int tz_dsttime;
};

/* Function prototypes */
extern int              gettimeofday(struct timeval *time_Info, struct timezone *timezone_Info);
extern DIR             *opendir(const char *);
extern struct dirent   *readdir(DIR *);
extern int              closedir(DIR *);
extern void             rewinddir(DIR *);
