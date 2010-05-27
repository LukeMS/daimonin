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

#if defined( __WIN_32)  || defined(__LINUX)
FILE   *logstream;

Boolean logFlush;
#endif
void LOG(int logLevel, char *format, ...)
{
#if defined( __WIN_32)  || defined(__LINUX)
    Boolean flag    = FALSE;
    va_list ap;

    if (LOGLEVEL < 0)   /* we want log exactly ONE logLevel*/
    {
        if (LOGLEVEL * (-1) == logLevel)
            flag = TRUE;
    }
    else    /* we log all logLevel < LOGLEVEL*/
    {
        if (logLevel <= LOGLEVEL)
            flag = TRUE;
    }
    if (!logstream)     /* secure: we have no open stream*/
    {
        logstream = fopen_wrapper(LOG_FILE, "w");
        if (!logstream)
            flag = FALSE;
    }
    if (flag)
    {
        va_start(ap, format);
        vfprintf(stdout, format, ap);
        va_end(ap);
        va_start(ap, format);
        vfprintf(logstream, format, ap);
        va_end(ap);
    }
    fflush(logstream);

#endif
}

void MSGLOG (char *msg)
{
#if defined( __WIN_32)  || defined(__LINUX)
        char timestr[20];
        if(msglog)      /* secure: we have no open stream*/
        {
            time_t now;
            time(&now);

            strftime(timestr, sizeof timestr, "%d-%m-%y %H:%M:%S", localtime(&now));
            fprintf(msglog,"%s: %s\n",timestr, msg);
        }
        else
            LOG(LOG_DEBUG,"Error with chatlogfile\n");
        fflush(msglog);
#endif
}

Boolean SYSTEM_Start(void)
{
    char         buf[MEDIUM_BUF];
    SDL_RWops   *rw;
    SDL_Surface *icon;

    sprintf(buf, "%s%s", GetBitmapDirectory(), CLIENT_ICON_NAME);

    if (PHYSFS_exists(buf) &&
        (rw = PHYSFSRWOPS_openRead(buf)) &&
        (icon = IMG_Load_RW(rw, 0)))
    {
        SDL_RWclose(rw);
        SDL_WM_SetIcon(icon, 0);
    }

    SDL_WM_SetCaption(PACKAGE_NAME, PACKAGE_NAME);

#if defined( __WIN_32)  || defined(__LINUX)

    return(TRUE);
#endif
}

Boolean SYSTEM_End(void)
{
#if defined( __WIN_32)  || defined(__LINUX)
    return(TRUE);
#endif
}

char * GetBitmapDirectory(void)
{
#if defined( __WIN_32)  || defined(__LINUX)
    return("bitmaps/");
#endif
}

char * GetIconDirectory(void)
{
#if defined( __WIN_32)  || defined(__LINUX)
    return("icons/");
#endif
}

char * GetSfxDirectory(void)
{
#if defined( __WIN_32)  || defined(__LINUX)
    return("sfx/");
#endif
}

char * GetCacheDirectory(void)
{
#if defined( __WIN_32)  || defined(__LINUX)
    return("cache/");
#endif
}

char * GetGfxUserDirectory(void)
{
#if defined( __WIN_32)  || defined(__LINUX)
    return("gfx_user/");
#endif
}


char * GetMediaDirectory(void)
{
#if defined( __WIN_32)  || defined(__LINUX)
    return("media/");
#endif
}

#define sdldebug(__s_) LOG(LOG_DEBUG,"%s\n",__s_)


/* warning: linux test version - it don't works 100% - disabled */
/**
 * Attempt to flip the video surface to fullscreen or windowed mode.
 *  Attempts to maintain the surface's state, but makes no guarantee
 *  that pointers (i.e., the surface's pixels field) will be the same
 *  after this call.
 *
 * Caveats: Your surface pointers will be changing; if you have any other
 *           copies laying about, they are invalidated.
 *
 *          Do NOT call this from an SDL event filter on Windows. You can
 *           call it based on the return values from SDL_PollEvent, etc, just
 *           not during the function you passed to SDL_SetEventFilter().
 *
 *          Thread safe? Likely not.
 *
 *          This has been tested briefly under Linux/X11 and Win/DirectX. YMMV.
 *
 *          Palette setting is possibly/probably broken. Please fix.
 *
 *   @param surface pointer to surface ptr to toggle. May be different
 *                  pointer on return. MAY BE NULL ON RETURN IF FAILURE!
 *   @param flags   pointer to flags to set on surface. The value pointed
 *                  to will be XOR'd with SDL_FULLSCREEN before use. Actual
 *                  flags set will be filled into pointer. Contents are
 *                  undefined on failure. Can be NULL, in which case the
 *                  surface's current flags are used.
 *  @return non-zero on success, zero on failure.
 */

int attempt_fullscreen_toggle(SDL_Surface **surface, uint32 *flags)
{
    long        framesize   = 0;
    void       *pixels      = NULL;
    SDL_Rect    clip;
    uint32      tmpflags    = 0;
    int         w           = 0;
    int         h           = 0;
    int         bpp         = 0;
    int         grabmouse   = (SDL_WM_GrabInput(SDL_GRAB_QUERY) == SDL_GRAB_ON);
    int         showmouse   = SDL_ShowCursor(-1);

#ifdef BROKEN
    SDL_Color  *palette     = NULL;
    int         ncolors     = 0;
#endif

    sdldebug("attempting to toggle fullscreen flag...");

    if ((!surface) || (!(*surface)))  /* don't try if there's no surface. */
    {
        sdldebug("Null surface (?!). Not toggling fullscreen flag.");
        return(0);
    } /* if */

    if (SDL_WM_ToggleFullScreen(*surface))
    {
        sdldebug("SDL_WM_ToggleFullScreen() seems to work on this system.");
        if (flags)
            *flags ^= SDL_FULLSCREEN;
        return(1);
    } /* if */
    sdldebug("SDL_WM_ToggleFullScreen() don't work on this system.");

    if (!(SDL_GetVideoInfo()->wm_available))
    {
        sdldebug("No window manager. Not toggling fullscreen flag.");
        return(0);
    } /* if */

    sdldebug("toggling fullscreen flag The not so hard Hard Way...");
    /* 2007-02-18 Alderan:
     * we have to get this values, for compatibility with old sdl libs,
     * only version >=1.2.10 can use SDL_SetVideoMode(NULL,NULL,NULL,flags)
     * to use the old values
     */
    tmpflags = (*surface)->flags;
    w = (*surface)->w;
    h = (*surface)->h;
    bpp = (*surface)->format->BitsPerPixel;

    if (flags == NULL)  /* use the surface's flags. */
        flags = &tmpflags;

    if ((*surface = SDL_SetVideoMode(w, h, bpp, *flags)) == NULL)
    {
        textwin_showstring(COLOR_RED, "Couldn't toggle fullscreen: %s\n", SDL_GetError());
        sdldebug("Set it back...");
        *surface = SDL_SetVideoMode(w, h, bpp, tmpflags);
        if (*surface == NULL)
            sdldebug("Now we have a REALLY BIG problem: coudn't set back... this will likely crash the client...");
    }
    else
    {
        const SDL_VideoInfo    *info    = NULL;
        info = SDL_GetVideoInfo();
        options.real_video_bpp = info->vfmt->BitsPerPixel;
        return 1;
    }

    sdldebug("toggling fullscreen flag The REALLY Hard Way...");

    if (flags == NULL)  /* use the surface's flags. */
        flags = &tmpflags;

    SDL_GetClipRect(*surface, &clip);

    /* save the contents of the screen. */
    if ((!(tmpflags & SDL_OPENGL)) && (!(tmpflags & SDL_OPENGLBLIT)))
    {
        framesize = (w * h) * ((*surface)->format->BytesPerPixel);
        pixels = malloc(framesize);
        if (pixels == NULL)
            return(0);
        memcpy(pixels, (*surface)->pixels, framesize);
    } /* if */

#ifdef BROKEN
    if ((*surface)->format->palette != NULL)
    {
        ncolors = (*surface)->format->palette->ncolors;
        palette = malloc(ncolors * sizeof(SDL_Color));
        if (palette == NULL)
        {
            free(pixels);
            return(0);
        } /* if */
        memcpy(palette, (*surface)->format->palette->colors, ncolors * sizeof(SDL_Color));
    } /* if */
#endif

    if (grabmouse)
        SDL_WM_GrabInput(SDL_GRAB_OFF);

    SDL_ShowCursor(1);

    *surface = SDL_SetVideoMode(w, h, bpp, *flags);
    /* why xor the fullscreen flag??? we get the oppsoite of that what we want! */
    /* *surface = SDL_SetVideoMode(w, h, bpp, (*flags) ^ SDL_FULLSCREEN); */

    if (*surface != NULL)
        *flags ^= SDL_FULLSCREEN;
    else  /* yikes! Try to put it back as it was... */
    {
        sdldebug("Set it back...");
        *surface = SDL_SetVideoMode(w, h, bpp, tmpflags);
        if (*surface == NULL)  /* completely screwed. */
        {
            if (pixels != NULL)
                free(pixels);
#ifdef BROKEN
            if (palette != NULL)
                free(palette);
#endif
            return(0);
        } /* if */
    } /* if */

    /* Unfortunately, you lose your OpenGL image until the next frame... */

    if (pixels != NULL)
    {
        memcpy((*surface)->pixels, pixels, framesize);
        free(pixels);
    } /* if */

#ifdef BROKEN
    if (palette != NULL)
    {
        /* !!! FIXME : No idea if that flags param is right. */
        SDL_SetPalette(*surface, SDL_LOGPAL, palette, 0, ncolors);
        free(palette);
    } /* if */
#endif

    SDL_SetClipRect(*surface, &clip);

    if (grabmouse)
        SDL_WM_GrabInput(SDL_GRAB_ON);

    SDL_ShowCursor(showmouse);

    return(1);
} /* attempt_fullscreen_toggle */


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
        options.fullscreen_flag = TRUE;
        options.doublebuf_flag = FALSE;
        options.rleaccel_flag = FALSE;
        if (options.Full_RLEACCEL)
            options.rleaccel_flag = TRUE;
        if (options.videoflags_full & SDL_DOUBLEBUF)
            options.doublebuf_flag = TRUE;

        return videoflags_full;
    }
    else
    {
        options.fullscreen_flag = FALSE;
        options.doublebuf_flag = FALSE;
        options.rleaccel_flag = FALSE;
        if (options.Win_RLEACCEL)
            options.rleaccel_flag = TRUE;
        if (options.videoflags_win & SDL_DOUBLEBUF)
            options.doublebuf_flag = TRUE;

        return videoflags_win;
    }
}

/* Parse server host into address and port
 * Allows URL style syntax with address in brackets
 * for addresses which include colons
 */
int parse_serverhost(char *tmp, char *server, int *port)
{
    char *servstr, *portstr;

    servstr = tmp;
    portstr = servstr;
    if (servstr[0] == '[')
    {
        char *tmp;

        tmp = strchr(++servstr, ']');
        if (tmp == NULL)
            return -1;
        *tmp = '\0';
        portstr = tmp+1;
    }
    portstr = strchr(portstr, ':');
    if (portstr != NULL)
    {
        *port = atoi(portstr + 1);
        *portstr = '\0';
    }
    else
    {
        *port = DEFAULT_SERVER_PORT;
    }
    strcpy(server, servstr);

    return 0;
}

/* Helper function for parse_metaserver_data */
/* Replaces all occurrences of specified char in string with space */
static void char_to_space(char *str, char ch)
{
    int i;

    for(i = 0; str[i] != 0; i++)
        if(str[i] == ch)
            str[i]=' ';
}

/* we have one big string holding all servers from the metaserver
* we do simple castings of 2 placeholders ( | and _ ) to ' ' whitespace
* and use then sscanf to get the info
*/
int parse_metaserver_data(char *info)
{
    struct entry
    {
        int     count;
        char    name[128];
        char    server[128];
        int     port;
        char    version[128];
        int     player;
        char    desc[1024];
    };

    char    *tmp;
    int     i, j, k;
    int     server_count, index;
    struct entry *entries = NULL;
    int     *tag = NULL;
    size_t  size;

    // Count number of strings and allocate that number of entries
    for (i = 0, server_count = 0; info[i] != 0; i++)
    {
        if (info[i] == '\n')
            server_count++;
    }
    if (server_count == 0)
        return FALSE;

    // Allocate entries and tag array
    size = server_count * sizeof(struct entry);
    entries = _malloc(size , "parse_metaserver_data: entries");
    memset(entries, 0, size);
    tag = _malloc(server_count * sizeof(int), "parse_metaserver_data: tag");

    // Initialize tag array
    for (i = 0; i < server_count; i++)
        tag[i] = i;

    // set all '|' to ' ' whitespace so we can use sscanf
    char_to_space(info, '|');

    for(index = 0; index < server_count; index++)
    {
        sscanf(info,"%d %s %s %d %s %d %s", &entries[index].count, entries[index].name,
            entries[index].server, &entries[index].port, entries[index].version,
            &entries[index].player, entries[index].desc);

        // set all '_' to ' ' whitespace
        char_to_space(entries[index].name, '_');
        char_to_space(entries[index].desc, '_');
        char_to_space(entries[index].version, '_');

        LOG(LOG_DEBUG, "SERVER: {%d} %s %s %d %s %d {%s}\n", entries[index].count, entries[index].name,
            entries[index].server, entries[index].port, entries[index].version,
            entries[index].player, entries[index].desc);

        // go to next row
        if(!(tmp = strchr(info, 0x0a)))
            break;
        info = tmp+1;
    }

    // Do tag sort
    for (i = server_count - 1; i >= 0; i--)
    {
        for (j = 0; j < i; j++)
        {
            if (strcmp(entries[tag[j]].name, entries[tag[j+1]].name) > 0)
            {
                k = tag[j];
                tag[j] = tag[j+1];
                tag[j+1] = k;
            }
        }
    }

    // Add to list in sorted tag order
    for (index = 0; index < server_count; index++)
    {
        i = tag[index];
        add_metaserver_data(entries[i].name, entries[i].server, entries[i].port,
            entries[i].player, entries[i].version, entries[i].desc);
    }

    /* Free memory */
    free(entries);
    free(tag);

    return (server_count > 0) ? TRUE : FALSE;
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

/* little helper function to have fgets behavior with physfs */
char * PHYSFS_fgets(char * const str, const int size, PHYSFS_File *const fp)
{
    int i = 0;
    char c;
    do
    {
        if (i == size-1)
          break;

        if (PHYSFS_read(fp, &c, 1, 1) != 1)
            break;

        str[i++] = c;
    }
    while (c != '\0' && c != -1 && c != '\n');

    str[i] = '\0';

    if (i == 0)
        return NULL;

    return str;
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
