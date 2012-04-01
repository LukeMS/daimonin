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
#include <include.h>

/* This is because PHYSFS_isInit() was introduced in 2.0.0 and some linux
 * distros are still stuck in 1.x.x. So in time we can dump this global and
 * use PHYSFS_isInit(). But for now, it's a simple query we can reproduce
 * 100% across the board this way, so lets do that. */
int PHYSFS_isInitialised = 0;

void LOG(int loglevel, char *format, ...)
{
    static PHYSFS_File *handle = NULL;
    va_list             ap;
    char                buf[HUGE_BUF];
    static uint8        system_end = 0;

    /* Always log fatal errors and system messages. */
    if (loglevel != LOG_FATAL &&
        loglevel != LOG_SYSTEM)
    {
        /* we want log exactly ONE logLevel*/
        if (LOGLEVEL < 0 &&
            LOGLEVEL * (-1) != loglevel)
        {
            return;
        }
        /* we log all loglevel <= LOGLEVEL*/
        else if (LOGLEVEL >= 0 &&
                 loglevel > LOGLEVEL)
        {
            return;
        }
    }

    va_start(ap, format);
    vsprintf(buf, format, ap);
    va_end(ap);
    fprintf(stdout, "%s", buf);

    if (PHYSFS_isInitialised &&
        PHYSFS_getWriteDir())
    {
        if (!handle)
        {
            char fname[TINY_BUF];

            sprintf(fname, "%s/%s", DIR_LOGS, FILE_LOG);

            if (!(handle = PHYSFS_openWrite(fname)))
            {
                fprintf(stderr, "%s\n'%s' will not be saved!\n",
                        PHYSFS_getLastError(), fname);

                return;
            }
        }

        PHYSFS_writeString(handle, buf);
    }

    /* Exit on fatal error. */
    if (!system_end &&
        loglevel == LOG_FATAL)
    {
        system_end = 1;
        SYSTEM_End();
        exit(EXIT_FAILURE);
    }
}

void SYSTEM_Start(void)
{
    char         buf[MEDIUM_BUF];
    SDL_RWops   *rw;
    SDL_Surface *icon;

    sprintf(buf, "%s/%s", DIR_BITMAPS, CLIENT_ICON_NAME);

    if (PHYSFS_exists(buf) &&
        (rw = PHYSFSRWOPS_openRead(buf)) &&
        (icon = IMG_Load_RW(rw, 0)))
    {
        SDL_RWclose(rw);
        SDL_WM_SetIcon(icon, 0);
    }

    sprintf(buf, "Daimonin SDL Client v%d.%d.%d",
            DAI_VERSION_RELEASE, DAI_VERSION_MAJOR, DAI_VERSION_MINOR);
    SDL_WM_SetCaption(buf, buf);
}

void SYSTEM_End(void)
{
    save_user_settings();
    gameserver_init();
    strout_vim_reset();
    strout_tooltip_reset();
    textwin_deinit();
    widget_deinit();
    SOCKET_DeinitSocket();

    if (PHYSFS_isInitialised)
    {
        PHYSFS_deinit();
        PHYSFS_isInitialised = 0;
    }

    face_deinit();
    sound_freeall();
    sound_deinit();
    skin_deinit();
    sprite_deinit();
    locator_clear_players(NULL);
    clear_lists();

    if (options.show_frame)
    {
        LOG(LOG_MSG, "FPS: Best (%u), Worst (%u)\n",
            options.best_fps, options.worst_fps);
    }
}

/* calc the videoflags from the settings */
/* when settings are changed at runtime, this MUST called again */
uint32 get_video_flags(void)
{
    uint32  videoflags_full, videoflags_win;

    videoflags_full = SDL_FULLSCREEN;

    if (options.Full_DOUBLEBUF)
        videoflags_full |= SDL_DOUBLEBUF;
    if (options.Full_HWSURFACE)
        videoflags_full |= SDL_HWSURFACE;
    if (options.Full_SWSURFACE)
        videoflags_full |= SDL_SWSURFACE;
    if (options.Full_HWACCEL)
        videoflags_full |= SDL_HWACCEL;
    if (options.Full_ANYFORMAT)
        videoflags_full |= SDL_ANYFORMAT;
    if (options.Full_ASYNCBLIT)
        videoflags_full |= SDL_ASYNCBLIT;
    if (options.Full_HWPALETTE)
        videoflags_full |= SDL_HWPALETTE;
    if (options.Full_RESIZABLE)
        videoflags_full |= SDL_RESIZABLE;
    if (options.Full_NOFRAME)
        videoflags_full |= SDL_NOFRAME;

    videoflags_win = 0;
    if (options.Win_DOUBLEBUF)
        videoflags_win |= SDL_DOUBLEBUF;
    if (options.Win_HWSURFACE)
        videoflags_win |= SDL_HWSURFACE;
    if (options.Win_SWSURFACE)
        videoflags_win |= SDL_SWSURFACE;
    if (options.Win_HWACCEL)
        videoflags_win |= SDL_HWACCEL;
    if (options.Win_ANYFORMAT)
        videoflags_win |= SDL_ANYFORMAT;
    if (options.Win_ASYNCBLIT)
        videoflags_win |= SDL_ASYNCBLIT;
    if (options.Win_HWPALETTE)
        videoflags_win |= SDL_HWPALETTE;
    if (options.Win_RESIZABLE)
        videoflags_win |= SDL_RESIZABLE;
    if (options.Win_NOFRAME)
        videoflags_win |= SDL_NOFRAME;

    options.videoflags_win = videoflags_win;
    options.videoflags_full = videoflags_full;


    if (options.fullscreen)
    {
        options.doublebuf_flag = 0;
        options.rleaccel_flag = 0;
        if (options.Full_RLEACCEL)
            options.rleaccel_flag = 1;
        if (options.videoflags_full & SDL_DOUBLEBUF)
            options.doublebuf_flag = 1;

        return videoflags_full;
    }
    else
    {
        options.doublebuf_flag = 0;
        options.rleaccel_flag = 0;
        if (options.Win_RLEACCEL)
            options.rleaccel_flag = 1;
        if (options.videoflags_win & SDL_DOUBLEBUF)
            options.doublebuf_flag = 1;

        return videoflags_win;
    }
}

/* This seems to be lacking on some system */
#if defined(HAVE_STRNICMP)
#else
#if !defined(HAVE_STRNCASECMP)
int strncasecmp(char *s1, char *s2, int n)
{
    register int c1, c2;

    while (*s1 && *s2 && n)
    {
        c1 = tolower(*s1);
        c2 = tolower(*s2);
        if (c1 != c2)
            return (c1 - c2);
        s1++;
        s2++;
        n--;
    }
    if (!n)
        return(0);
    return (int) (*s1 - *s2);
}
#endif
#endif

#if defined(HAVE_STRICMP)
#else
#if !defined(HAVE_STRCASECMP)
int strcasecmp(char *s1, char *s2)
{
    register int c1, c2;

    while (*s1 && *s2)
    {
        c1 = tolower(*s1);
        c2 = tolower(*s2);
        if (c1 != c2)
            return (c1 - c2);
        s1++;
        s2++;
    }
    if (*s1 == '\0' && *s2 == '\0')
        return 0;
    return (int) (*s1 - *s2);
}
#endif
#endif

/* Similar to fgets(). Reads the next line from handle into s (at most len - 1
 * characters), stopping when it reads \0, \n, \r, or EOF. This terminating
 * character is replaced in s with \0. The return is the number of characters
 * read excluuding the terminator (so (PHYSFS_sint64)strlen(s)) or -1 on
 * error. */
PHYSFS_sint64 PHYSFS_readString(PHYSFS_File *handle, char *s, size_t len)
{
    size_t        i = 0;
    char          c;
    PHYSFS_sint64 objCount = -1;

    /* Sanity. */
    if (handle)
    {
        for (; i < len; i++)
        {
            if (!handle ||
                PHYSFS_eof(handle))
            {
                break;
            }

            if (PHYSFS_read(handle, &c, 1, 1) < 1)
            {
                LOG(LOG_ERROR, "%s\n", PHYSFS_getLastError());
                objCount = -1;

                break;
            }

            objCount++;

            if (c == '\0' ||
                c == '\n' ||
                c == '\r')
            {
                break;
            }

            *(s + i) = c;
        }
    }

    *(s + i) = '\0';

    return objCount;
}

/* Write cs to handle. Returns number of characters written (or less if there
 * was an error, which is logged, or -1 for a total failure. */
PHYSFS_sint64 PHYSFS_writeString(PHYSFS_File *handle, const char *cs)
{
    PHYSFS_sint64 objCount = -1;;

    /* Sanity. */
    if (handle)
    {
        size_t len = strlen(cs);

        if ((objCount = PHYSFS_write(handle, cs, 1, (PHYSFS_uint32)len)) < (PHYSFS_sint64)len)
        {
            LOG(LOG_ERROR, "%s\n", PHYSFS_getLastError());
        }
    }

    return objCount;
}

/* Flush the SDL version to the client log */
void print_SDL_version(char* preamble, SDL_version* v)
{
	LOG(LOG_MSG, "%s %u.%u.%u\n", preamble, v->major, v->minor, v->patch);
}

void print_SDL_versions()
{
	SDL_version ver;
	SDL_VERSION(&ver);
	print_SDL_version("SDL compile-time version", &ver);
	ver = *SDL_Linked_Version();
	print_SDL_version("SDL runtime version", &ver);
}
