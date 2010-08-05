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

#ifndef __WRAPPER_H
#define __WRAPPER_H

#include <physfs.h>

/* include here the hardware depend headers */
#ifdef __WIN_32
#include "win32.h"
#elif __LINUX
#include <cflinux.h>
#endif

#if !defined(HAVE_STRICMP)
#include <strings.h>
#define stricmp(_s1_,_s2_) strcasecmp(_s1_,_s2_)
#endif

#if !defined(HAVE_STRNICMP)
#include <strings.h>
#define strnicmp(_s1_,_s2_,_nrof_) strncasecmp(_s1_,_s2_,_nrof_)
#endif

/* mallocs _P_ to size _S_, logging OOM or initialising to 0. */
#undef MALLOC
#if 0 // Seems Visual C++ cannot handle this style.
#define MALLOC(_P_, _S_) \
((!((_P_) = malloc((_S_)))) ? \
LOG(LOG_DEBUG, "%s %d: Out of memory!\n", __FILE__, __LINE__) : \
memset((_P_), 0, (_S_)))
#else
#define MALLOC(_P_, _S_) \
if (!((_P_) = malloc((_S_)))) \
{ \
    LOG(LOG_DEBUG, "%s %d: Out of memory!\n", __FILE__, __LINE__); \
} \
else \
{ \
    memset((_P_), 0, (_S_)); \
}
#endif

/* mallocs _P_ to size strlen(_S_) + 1, logging OOM or initialising to
 * sprintf(_P_, "%s", _S_). */
#undef MALLOC_STRING
#if 0 // Seems Visual C++ cannot handle this style.
#define MALLOC_STRING(_P_, _S_) \
((!((_P_) = malloc(strlen((_S_)) + 1))) ? \
LOG(LOG_DEBUG, "%s %d: Out of memory!\n", __FILE__, __LINE__) : \
sprintf((_P_), "%s", (_S_)))
#else
#define MALLOC_STRING(_P_, _S_) \
if (!((_P_) = malloc(strlen((_S_)) + 1))) \
{ \
    LOG(LOG_DEBUG, "%s %d: Out of memory!\n", __FILE__, __LINE__); \
} \
else \
{ \
    sprintf((_P_), "%s", (_S_)); \
}
#endif

/* frees _P_ and sets it to NULL. */
#undef FREE
#define FREE(_P_) \
do \
{ \
    free((_P_)); \
    (_P_) = NULL; \
} \
while (0)

#define MAX_METASTRING_BUFFER 128*2013

typedef enum _LOGLEVEL
{
    LOG_MSG,
    LOG_ERROR,
    LOG_DEBUG
}    _LOGLEVEL;
#define LOGLEVEL LOG_DEBUG
extern void     LOG(int logLevel, char *format, ...);
extern void     MSGLOG(char *msg);

extern char    *GetCacheDirectory(void);
extern char    *GetGfxUserDirectory(void);
extern char    *GetBitmapDirectory(void);
extern char    *GetSfxDirectory(void);
extern char    *GetMediaDirectory(void);
extern char    *GetIconDirectory(void);

extern Boolean  SYSTEM_Start(void);
extern Boolean  SYSTEM_End(void);
extern int      attempt_fullscreen_toggle(SDL_Surface **surface, uint32 *flags);
extern uint32   get_video_flags(void);
extern int      parse_metaserver_data(char *info);

#if defined( __WIN_32)  || defined(__LINUX)
	FILE *msglog;
#endif

#if defined(HAVE_STRNICMP)
#else
#if !defined(HAVE_STRNCASECMP)
int             strncasecmp(char *s1, char *s2, int n);
#endif
#endif

#if defined(HAVE_STRICMP)
#else
#if !defined(HAVE_STRCASECMP)
int             strcasecmp(char *s1, char *s2);
#endif
#endif

extern PHYSFS_sint64 PHYSFS_readString(PHYSFS_File *handle, char *s, size_t len);
extern PHYSFS_sint64 PHYSFS_writeString(PHYSFS_File *handle, const char *cs);
extern void print_SDL_versions();
extern void print_SDL_version(char* preamble, SDL_version* v);

#endif /* ifndef __WRAPPER_H */
