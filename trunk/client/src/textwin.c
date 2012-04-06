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

#include "include.h"

textwin_window_t textwin[TEXTWIN_NROF];

static void LogTextwin(char *message, char *logfile);
static void GrepForStatometer(char *message);
static void ConvertSmileys(char *message);
static void AddLine(textwin_window_t *tw, const uint32 flags, const uint32 colr,
                    const uint8 indent, const uint8 strong, const uint8 emphasis,
                    const uint8 underline, const char *message);
static void ShowWindowResizingBorders(textwin_window_t *tw, _BLTFX *bltfx);
static void ShowWindowText(textwin_window_t *tw, _BLTFX *bltfx);
static void ShowWindowScrollbarH(textwin_window_t *tw, _BLTFX *bltfx);
static void ShowWindowScrollbarV(textwin_window_t *tw, _BLTFX *bltfx);
static void ShowWindowFrame(textwin_window_t *tw, _BLTFX *bltfx);
static void ResizeTextWindow(textwin_window_t *tw);
static void ScrollTextWindow(textwin_window_t *tw);

void textwin_init(void)
{
    textwin_id_t id;

    for (id = 0; id < TEXTWIN_NROF; id++)
    {
        textwin_window_t  *tw = &textwin[id];
        textwin_linebuf_t *linebuf;

        memset(tw, 0, sizeof(textwin_window_t));

        if (id == TEXTWIN_CHAT_ID)
        {
            tw->wid = WIDGET_CHATWIN_ID;
        }
        else if (id == TEXTWIN_MSG_ID)
        {
            tw->wid = WIDGET_MSGWIN_ID;
        }

        tw->linebuf_size = options.textwin_scrollback;
        MALLOC(linebuf, sizeof(textwin_linebuf_t) * tw->linebuf_size);
        tw->linebuf = linebuf;
        textwin_set_font(id);
    }
}

void textwin_deinit(void)
{
    textwin_id_t id;

    for (id = 0; id < TEXTWIN_NROF; id++)
    {
        uint32            i;
        textwin_window_t *tw = &textwin[id];

        for (i = 0; i < tw->linebuf_used; i++)
        {
            FREE((tw->linebuf + i)->buf);
        }

        FREE(tw->linebuf);
    }
}

void textwin_set_font(textwin_id_t id)
{
    textwin_window_t *tw = &textwin[id];

    WIDGET_REDRAW(tw->wid) = 1;

    switch ((id == TEXTWIN_CHAT_ID) ? options.textwin_chat_font :
            options.textwin_msg_font)
    {
        case 0:
            tw->font = &font_tiny;

            break;

        case 1:
            tw->font = &font_small;

            break;

        case 2:
            tw->font = &font_medium;

            break;

        case 3:
            tw->font = &font_large;

            break;

        case 4:
            tw->font = &font_huge;

            break;
    }

    textwin_calculate_dimensions(tw);
}

void textwin_calculate_dimensions(textwin_window_t *tw)
{
    uint8 diff;

    if ((diff = widget_data[tw->wid].wd % tw->font->c['_'].w))
    {
        widget_data[tw->wid].wd += tw->font->c['_'].w - diff;
    }

    tw->contentwd = MAX(TEXTWIN_WIDTH_MIN, MIN(widget_data[tw->wid].wd,
                                               TEXTWIN_WIDTH_MAX)) - 4;

    if (tw->contentwd > tw->maxcontentwd)
    {
        tw->maxcontentwd = tw->contentwd;
    }

    if ((diff = widget_data[tw->wid].ht % tw->font->line_height))
    {
        widget_data[tw->wid].ht += tw->font->line_height - diff;
    }

    tw->contentht = MAX(TEXTWIN_HEIGHT_MIN, MIN(widget_data[tw->wid].ht,
                                               TEXTWIN_HEIGHT_MAX));
    tw->maxcontentht = tw->linebuf_used * tw->font->line_height;
    tw->linebuf_visi = tw->contentht / tw->font->line_height;
}

void textwin_show_string(uint32 flags, uint32 colr, char *format, ...)
{
    const textwin_id_t id = (!(flags & NDI_FLAG_PLAYER)) ? TEXTWIN_MSG_ID : TEXTWIN_CHAT_ID;
    va_list   ap;
    char      buf[HUGE_BUF],
             *start,
             *c,
             *space = NULL;
    textwin_window_t *tw = &textwin[id];
    uint8     strong = 0,
              emphasis = 0,
              underline = 0,
              strong2 = 0,
              emphasis2 = 0,
              underline2 = 0,
              old_strong = 0,
              old_emphasis = 0,
              old_underline = 0,
              indent = 0;
    uint16    w = 0;

    WIDGET_REDRAW(textwin[id].wid) = 1;

    /* Copy the formatted string to a working buffer, buf. */
    va_start(ap, format);
    vsprintf(buf, format, ap);
    va_end(ap);

#ifdef DAI_DEVELOPMENT
    LOG(LOG_MSG, "TEXTWIN: >%s<\n", buf);
#endif

    /* Here we handle chat... */
    if (id == TEXTWIN_CHAT_ID)
    {
        char  cname[TINY_BUF] = "",
              pname[TINY_BUF] = "";

        if ((flags & NDI_FLAG_CHANNEL))
        {
            uint8  i,
                   cn,
                   pn;
            uint16 len;

            /* TODO: should be MAX_CHANNEL_NAME. In 0.11.0 move this from
             * server/include/channel.h to protocol.h. */
            for (i = 0; buf[i]; i++)
            {
                if (i >= 12)
                {
                    i = 0;

                    break;
                }

                if (buf[i] == ' ')
                {
                    break;
                }

                cname[i] = buf[i];
            }

            cname[i] = '\0';
            cn = i + 1;

            if (!cname[0])
            {
                LOG(LOG_ERROR, "Malformed channel chat: >%s<\n", buf);

                return;
            }

            for (i = 0; buf[cn + i]; i++)
            {
                if (i >= MAX_PLAYER_NAME)
                {
                    i = 0;

                    break;
                }

                if (buf[cn + i] == ':')
                {
                    break;
                }

                pname[i] = buf[cn + i];
            }

            pname[i] = '\0';
            pn = i + 1;

            if (!pname[0])
            {
                LOG(LOG_ERROR, "Malformed channel chat: >%s<\n", buf);

                return;
            }

            len = strlen(buf);
            memmove(buf + 1, buf, len++);
            buf[0] = '[';
            buf[cn] = '#';
            buf[cn + pn] = ' ';

            if ((flags & NDI_FLAG_EMOTE))
            {
                buf[len] = ']';
            }
            else
            {
                memmove(buf + cn + pn + 1, buf + cn + pn, len - cn + pn); 
                buf[cn + pn] = ']';
            }

            buf[len + 1] = '\0';
        }

        /* Log before ignores, chatfilters and so on. */
        switch (options.textwin_use_logging)
        {
            case 1: // only chat
            case 3: // both separately
                LogTextwin(buf, FILE_CHATLOG);

                break;

            case 4: // both together
                LogTextwin(buf, FILE_TEXTWINLOG);

                break;
        }

        /* Unless this is a gmaster communicating in an official capacity, we
         * can ignore it. This is of course easy to work around. Just replace
         * the following expression with 1 and recompile to ignore gmasters too.
         * But officially they must be heard. */
        if (!(flags & NDI_FLAG_GMASTER))
        {
            if (((flags & NDI_FLAG_CHANNEL) &&
                 ignore_check(pname, cname)) ||
                ((flags & NDI_FLAG_SAY) &&
                 ignore_check(buf, "say")) ||
                ((flags & NDI_FLAG_SHOUT) &&
                 ignore_check(buf, "shout")) ||
                ((flags & NDI_FLAG_TELL) &&
                 ignore_check(buf, "tell")) ||
                ((flags & NDI_FLAG_EMOTE) &&
                 ignore_check(buf, "emote")))
            {
                return;
            }
        }

        if (options.textwin_use_chatfilter)
        {
            chatfilter_filter(buf);
        }

        if (buddy_check(buf))
        {
            flags |= NDI_FLAG_BUDDY;
        }

        /* save last incoming tell player for client sided /reply */
        if ((flags & NDI_FLAG_TELL))
        {
            uint8 max = strcspn(buf, ", ");

            cpl.reply[0] = '\0';

            if (max <= MAX_PLAYER_NAME)
            {
                strncpy(cpl.reply, buf, max);
            }
        }

        /* If the smiley option is on, convert all the smileys. */
        if (options.textwin_use_smileys)
        {
            ConvertSmileys(buf);
        }
    }
    /* ...And here, messages. */
    else
    {
        /* Log before ignores, chatfilters and so on. */
        switch (options.textwin_use_logging)
        {
            case 2: // only msgs
            case 3: // both separately
                LogTextwin(buf, FILE_MSGLOG);

                break;

            case 4: // both together
                LogTextwin(buf, FILE_TEXTWINLOG);

                break;
        }

        GrepForStatometer(buf);
    }

    for (start = c = buf; *c; c++)
    {
        switch (*c)
        {
            /* toggle strong and do not count width of embedded character
             * code. */
            case ECC_STRONG:
                strong = !strong;

                continue;

            /* toggle emphasis and do not count width of embedded character
             * code. */
            case ECC_EMPHASIS:
                emphasis = !emphasis;

                continue;

            /* toggle underline and do not count width of embedded character
             * code. */
            case ECC_UNDERLINE:
                underline = !underline;

                continue;

            /* force a newline. */
            case '\n':
                *c = '\0';

                break;

            /* change unprintable characters to space and drop through. */
            case 0x01:
            case 0x02:
            case 0x03:
            case 0x04:
            case 0x05:
            case 0x06:
            case 0x07:
            case 0x08:
            case 0x09:
            case 0x0b:
            case 0x0c:
            case 0x0d:
            case 0x0e:
            case 0x0f:
            case 0x10:
            case 0x11:
            case 0x12:
            case 0x13:
            case 0x14:
            case 0x15:
            case 0x16:
            case 0x17:
            case 0x18:
            case 0x19:
            case 0x1a:
            case 0x1b:
            case 0x1c:
            case 0x1d:
            case 0x1e:
            case 0x1f:
                 *c = ' ';

            /* remember this position and drop through. */
            case ' ':
                space = c;
                strong2 = strong;
                emphasis2 = emphasis;
                underline2 = underline;

            /* if the line has got too long, find a nice break point. */
            default:
                if (tw->contentwd &&
                    (w += tw->font->c[(unsigned char)(*c)].w +
                     tw->font->char_offset) >= tw->contentwd)
                {
                    /* If we have found a space above, that's our break point. */
                    if (space)
                    {
                        *(c = space) = '\0';
                        space = NULL;
                        strong = strong2;
                        emphasis = emphasis2;
                        underline = underline2;
                    }
                    /* else we'll need to insert one. */
                    else
                    {
                        char *cc = c - 1;

                        strong2 = strong;
                        emphasis2 = emphasis;
                        underline2 = underline;

                        for (; cc > start; cc--)
                        {
                            if (*cc == ECC_STRONG)
                            {
                                strong = !strong;
                            }
                            else if (*cc == ECC_EMPHASIS)
                            {
                                emphasis = !emphasis;
                            }
                            else if (*cc == ECC_UNDERLINE)
                            {
                                underline = !underline;
                            }
                            else if (ispunct(*cc))
                            {
                                memmove(cc + 2, cc + 1, strlen(cc));
                                *(c = cc + 1) = '\0';

                                break;
                            }
                        }

                        if (cc == start)
                        {
                            strong = strong2;
                            emphasis = emphasis2;
                            underline = underline2;
                            memmove(c + 1, c, strlen(c));
                            *c = '\0';
                        }
                    }
                }
        }

        if (*c == '\0')
        {
            AddLine(tw, flags, colr, indent, old_strong, old_emphasis,
                    old_underline, start);

            if ((flags & NDI_FLAG_PLAYER))
            {
                indent = options.textwin_indentation;
                w = (tw->font->c[' '].w + tw->font->char_offset) * indent;
            }
            else
            {
                w = 0;
            }

            old_strong = strong;
            old_emphasis = emphasis;
            old_underline = underline;
            start = c + 1;
        }
    }

    /* Add a final line for the end of the string. */
    AddLine(tw, flags, colr, indent, old_strong, old_emphasis, old_underline,
            start);
}

static void LogTextwin(char *message, char *logfile)
{
    char         fname[TINY_BUF],
                 buf[HUGE_BUF];
    PHYSFS_File *handle;
    time_t       t;

    if (!PHYSFS_isInitialised ||
        !PHYSFS_getWriteDir())
    {
        return;
    }

    sprintf(fname, "%s/%s", DIR_LOGS, logfile);

    /* We only log this stuff when opening the chat log fails. */
    if (!(handle = PHYSFS_openAppend(fname)))
    {
        LOG(LOG_SYSTEM, "Saving '%s'... ", fname);
        LOG(LOG_ERROR, "FAILED (%s)!\n", PHYSFS_getLastError());

        return;
    }

    time(&t);
    strftime(buf, sizeof(buf), "%d-%m-%y %H:%M:%S", localtime(&t));
    sprintf(strchr(buf, '\0'),": %s\n", message);
    PHYSFS_writeString(handle, buf);
    PHYSFS_close(handle);
}

/* Extracts exp/kill info from particular messages for the 'statometer'. */
/* TODO: This is a horrible way to do this (inefficient and relies on exactly
 * worded server messages) and will be changed in future. But for now, it
 * works...
 * -- Smacky  20100120 */
static void GrepForStatometer(char *message)
{
    if (options.statsupdate)
    {
        int exp;

        if (sscanf(message, "You got %d exp in skill", &exp) == 1)
        {
            statometer.exp += exp;
        }
        else if (sscanf(message, "You got %d (+%*d) exp in skill", &exp) == 1)
        {
            statometer.exp += exp;
        }
    }

    if (options.statsupdate &&
        !strncmp(message, "You killed ", 11))
    {
        statometer.kills++;
    }

    if (options.kerbholz)
    {
        if (!strncmp(message, "You killed ", 11))
        {
            char *enemy1,
                  enemy2[MEDIUM_BUF];
            int   newkill;

            if ((enemy1 = strstr(message, " with ")))
            {
                strncpy(enemy2, message + 11, enemy1 - (message + 11));
                enemy2[enemy1 - (message + 11)] = 0;
            }
            else
            {
                strncpy(enemy2, message + 11, (strlen(message + 11) - 1));
                enemy2[strlen(message + 11) - 1] = 0;
            }

            if ((newkill = addKill(enemy2)) == 1)
            {
                strcat(message, " for the first time!");
            }
            else if (newkill > 0)
            {
                struct kills_list *node = getKillEntry(enemy2);

                if (node)
                {
                    sprintf(strchr(message, '\0'), " %d/%d times.",
                            node->session, node->kills);
                }
            }
        }
    }
}

/* Replaces certain character sequences with a single character that is (should
 * be) defined as a 'graphical' smiley. Note that smileys are only defined for
 * font_small ATM, so if another font is used, the wrong character will be
 * displayed. */
/* TODO: This will be done in a different way, using a skin/user-defined list
 * of sequences and replacements, a la chatfilter, and possibly a dedicated
 * smiley 'font'.
 * -- 20100215 Smacky */
static void ConvertSmileys(char *message)
{
    uint16 i;

    for (i = 0; *(message + i); i++)
    {
        uint16 j = i + 1;
        char   smiley = 0;

        if (*(message + i) == ':' &&
            *(message + j) == '\'')
        {
            switch (*(message + ++j))
            {
                case '(':
                    smiley = ECC_SMILEY_CRY;

                    break;
            }
        }
        else if (*(message + i) == ':')
        {
            if (*(message + j) == '-')
            {
                j++;
            }

            switch (*(message + j))
            {
                case ')':
                    smiley = ECC_SMILEY_SMILE;

                    break;

                case '(':
                    smiley = ECC_SMILEY_FROWN;

                    break;

                case 'D':
                    smiley = ECC_SMILEY_GRIN;

                case 'p':
                case 'P':
                    smiley = ECC_SMILEY_RASPBERRY;

                    break;

                case 's':
                case 'S':
                    smiley = ECC_SMILEY_PERPLEXITY;

                    break;

                case 'x':
                case 'X':
                    smiley = ECC_SMILEY_SUCKALEMON;

                    break;
            }
        }
        else if (*(message + i) == ';')
        {
            if (*(message + j) == '-')
            {
                j++;
            }

            switch (*(message + j))
            {
                case ')':
                    smiley = ECC_SMILEY_WINK;

                    break;

                case 'p':
                case 'P':
                    smiley = ECC_SMILEY_WINKRASPBERRY;

                    break;
            }
        }
        else if ((*(message + i) == '8' ||
                  *(message + i) == 'B') &&
                 *(message + j) == ')')
        {
            smiley = ECC_SMILEY_COOL;
        }
        else if ((*(message + i) == '8' ||
                  *(message + i) == 'B') &&
                 *(message + j++) == '-' &&
                 *(message + j) == ')')
        {
            smiley = ECC_SMILEY_COOL;
        }
        else if (*(message + i) == '>' &&
                 *(message + j) == ':')
        {
            if (*(message + ++j) == '-')
            {
                j++;
            }

            switch (*(message + j))
            {
                case ')':
                    smiley = ECC_SMILEY_NAUGHTY;

                    break;

                case 'D':
                    smiley = ECC_SMILEY_WICKED;

                    break;
            }
        }

        if (smiley)
        {
            *(message + i) = smiley;
            memmove(message + i + 1, message + j + 1, strlen(message + j + 1) + 1);
        }
    }
}

/* Add message (already cut down to a single lines worth) to the textwindow structure. */
static void AddLine(textwin_window_t *tw, const uint32 flags, const uint32 colr,
                    const uint8 indent, const uint8 strong, const uint8 emphasis,
                    const uint8 underline, const char *message)
{
    uint16 line = tw->linebuf_next;
    char   buf[TINY_BUF];

    if (indent)
    {
        uint8 i;

        for (i = 0; i < indent; i++)
        {
            buf[i] = ' ';
        }
    }

    buf[indent] = '\0';

    if (strong)
    {
        sprintf(strchr(buf, '\0'), "%c", ECC_STRONG);
    }

    if (emphasis)
    {
        sprintf(strchr(buf, '\0'), "%c", ECC_EMPHASIS);
    }

    if (underline)
    {
        sprintf(strchr(buf, '\0'), "%c", ECC_UNDERLINE);
    }

    if ((tw->linebuf + line)->buf)
    {
        FREE((tw->linebuf + line)->buf);
    }

    MALLOC((tw->linebuf + line)->buf, strlen(buf) + strlen(message) + 1);
    sprintf((tw->linebuf + line)->buf, "%s%s", buf, message);
    (tw->linebuf + line)->flags = flags;

    if (!(flags & NDI_FLAG_PLAYER) ||
         (flags & NDI_FLAG_CHANNEL)) //temporary
    {
        (tw->linebuf + line)->fg = colr;
        (tw->linebuf + line)->bg = 0;
    }
    else
    {
        if ((flags & NDI_FLAG_GMASTER))
        {
            (tw->linebuf + line)->fg = skin_prefs.chat_gmaster;
        }
        else if ((flags & NDI_FLAG_EMOTE))
        {
            (tw->linebuf + line)->fg = skin_prefs.chat_emote;
        }
        else if ((flags & NDI_FLAG_GSAY))
        {
            (tw->linebuf + line)->fg = skin_prefs.chat_gsay;
        }
        else if ((flags & NDI_FLAG_SAY))
        {
            (tw->linebuf + line)->fg = skin_prefs.chat_say;
        }
        else if ((flags & NDI_FLAG_SHOUT))
        {
            (tw->linebuf + line)->fg = skin_prefs.chat_shout;
        }
        else if ((flags & NDI_FLAG_TELL))
        {
            (tw->linebuf + line)->fg = skin_prefs.chat_tell;
        }
        else
        {
            (tw->linebuf + line)->fg = colr;
        }

        if ((flags & NDI_FLAG_EAVESDROP))
        {
            (tw->linebuf + line)->bg = skin_prefs.chat_eavesdrop;
        }
        else if ((flags & NDI_FLAG_BUDDY))
        {
            (tw->linebuf + line)->bg = skin_prefs.chat_buddy;
        }
        else if ((flags & NDI_FLAG_CHANNEL))
        {
            (tw->linebuf + line)->bg = skin_prefs.chat_channel;
        }
        else
        {
            (tw->linebuf + line)->bg = 0;
        }
    }

    if (tw->linebuf_used < tw->linebuf_size)
    {
        tw->linebuf_used++;
        tw->maxcontentht = tw->linebuf_used * tw->font->line_height;
    }

    tw->linebuf_next = (line + 1) % tw->linebuf_size;

//    if (tw->scroll_yoff == tw->maxcontentht - tw->font->line_height - tw->contentht)
//    {
//        tw->scroll_y = -tw->font->line_height;
//        ScrollTextWindow(tw);
//        tw->scroll_y = 0;
//    }
}

void textwin_show_window(textwin_id_t id)
{
    textwin_window_t   *tw = &textwin[id];
    static SDL_Surface *bg = NULL;
    SDL_Rect            box;
    _BLTFX              bltfx;

    /* Always blit the background extra because of the alpha, for performance
     * reasons we need to do a DisplayFormat and keep track of the old
     * textwin_alpha value. We also need it to separate because drawing and esp
     * blitting of fonts with palettes to a DisplayFormatted (hardware) surface
     * is slow as hell. */
    if (!bg)
    {
        SDL_SetAlpha(skin_sprites[SKIN_SPRITE_ALPHA]->bitmap,
                     SDL_SRCALPHA | SDL_RLEACCEL, 128);
        bg = SDL_DisplayFormat(skin_sprites[SKIN_SPRITE_ALPHA]->bitmap);
    }

    /* if we don't have a backbuffer, create it */
    if (!widget_surface[tw->wid])
    {
        widget_surface[tw->wid] = SDL_DisplayFormatAlpha(skin_sprites[SKIN_SPRITE_ALPHA]->bitmap);
        SDL_SetColorKey(widget_surface[tw->wid], SDL_SRCCOLORKEY | SDL_RLEACCEL,
                        SDL_MapRGB(widget_surface[tw->wid]->format, 0x00, 0x00,
                                   0x00));
    }

    /* lets draw the widgets in the backbuffer */
    if (WIDGET_REDRAW(tw->wid))
    {
        /* Draw a textwindow in this order:
         *   the background;
         *   the resizing borders, if necessary;
         *   the text, if necessary;
         *   the scrollbar, if necessary; then
         *   the frame. */
        WIDGET_REDRAW(tw->wid) = 0;
        bltfx.surface = widget_surface[tw->wid];
        bltfx.flags = 0;
        bltfx.alpha = 0;
        SDL_FillRect(bltfx.surface, NULL,
                     SDL_MapRGBA(widget_surface[tw->wid]->format, 0x00, 0x00,
                                 0x00, 0));
//widget_data[tw->wid].ht = (widget_data[tw->wid].ht / tw->font->line_height + 1) * tw->font->line_height;
#ifdef DEBUG_TEXTWIN
        if (tw->wid == WIDGET_MSGWIN_ID)
        {
            LOG(LOG_MSG,">>>>xo=%d yo=%d wd=%d maxwd=%d ht=%d maxht=%d lu=%d lv=%d ln=%d\n",
                tw->scroll_xoff, tw->scroll_yoff,
                tw->contentwd, tw->maxcontentwd, tw->contentht, tw->maxcontentht,
                tw->linebuf_used, tw->linebuf_visi, tw->linebuf_next);
        }
#endif

        if (tw->resize)
        {
            ShowWindowResizingBorders(tw, &bltfx);
        }

        if (tw->linebuf_used)
        {
            ShowWindowText(tw, &bltfx);
        }

        if (tw->maxcontentwd > tw->contentwd)
        {
            ShowWindowScrollbarH(tw, &bltfx);
        }
        else
        {
            tw->hbarge_len = 0;
        }

        if (tw->maxcontentht > tw->contentht)
        {
            ShowWindowScrollbarV(tw, &bltfx);
        }
        else
        {
            tw->vbarge_len = 0;
        }

        ShowWindowFrame(tw, &bltfx);
    }

    box.x = widget_data[tw->wid].x1;
    box.y = widget_data[tw->wid].y1;
    box.w = widget_data[tw->wid].wd;
    box.h = widget_data[tw->wid].ht;
//LOG(LOG_MSG,">>>>>>>>>%d,%d %d,%d %d,%d %d,%d\n",box.x,widget_data[tw->wid].x1,box.y,widget_data[tw->wid].y1,box.w,widget_data[tw->wid].wd,box.h,widget_data[tw->wid].ht);
    SDL_SetClipRect(ScreenSurface, &box);
    SDL_BlitSurface(bg, NULL, ScreenSurface, &box);
    SDL_BlitSurface(widget_surface[tw->wid], NULL, ScreenSurface, &box);
    SDL_SetClipRect(ScreenSurface, NULL);

    if (tw->mode == TEXTWIN_MODE_SCROLL)
    {
        ScrollTextWindow(tw);
    }

    if (tw->mode == TEXTWIN_MODE_RESIZE)
    {
        ResizeTextWindow(tw);
        textwin_calculate_dimensions(tw);
    }
}

static void ShowWindowResizingBorders(textwin_window_t *tw, _BLTFX *bltfx)
{
    SDL_Rect     box;
    const uint32 rgb = SDL_MapRGB(bltfx->surface->format, 0x00, 0x80, 0x00);

    if (tw->resize == TEXTWIN_RESIZE_UP ||
        tw->resize == TEXTWIN_RESIZE_UPRIGHT ||
        tw->resize == TEXTWIN_RESIZE_UPLEFT)
    {
        box.x = TEXTWIN_ACTIVE_MIN;
        box.y = TEXTWIN_ACTIVE_MIN;
        box.w = widget_data[tw->wid].wd - TEXTWIN_ACTIVE_MIN * 2;
        box.h = TEXTWIN_ACTIVE_MAX - TEXTWIN_ACTIVE_MIN;
        SDL_FillRect(bltfx->surface, &box, rgb);
    }
    else if (tw->resize == TEXTWIN_RESIZE_DOWNRIGHT ||
             tw->resize == TEXTWIN_RESIZE_DOWN ||
             tw->resize == TEXTWIN_RESIZE_DOWNLEFT)
    {
        box.x = TEXTWIN_ACTIVE_MIN;
        box.y = widget_data[tw->wid].ht - TEXTWIN_ACTIVE_MAX - 1;

        if (tw->maxcontentwd > tw->contentwd)
        {
            box.y -= skin_sprites[SKIN_SPRITE_SLIDER_HCANAL]->bitmap->h;
        }

        box.w = widget_data[tw->wid].wd - TEXTWIN_ACTIVE_MIN * 2;
        box.h = TEXTWIN_ACTIVE_MAX - TEXTWIN_ACTIVE_MIN;
        SDL_FillRect(bltfx->surface, &box, rgb);
    }

    if (tw->resize == TEXTWIN_RESIZE_UPRIGHT ||
        tw->resize == TEXTWIN_RESIZE_RIGHT ||
        tw->resize == TEXTWIN_RESIZE_DOWNRIGHT)
    {
        box.x = widget_data[tw->wid].wd - TEXTWIN_ACTIVE_MAX - 1;

        if (tw->maxcontentht > tw->contentht)
        {
            box.x -= skin_sprites[SKIN_SPRITE_SLIDER_VCANAL]->bitmap->w;
        }

        box.y = TEXTWIN_ACTIVE_MIN;
        box.w = TEXTWIN_ACTIVE_MAX - TEXTWIN_ACTIVE_MIN;
        box.h = widget_data[tw->wid].ht - TEXTWIN_ACTIVE_MIN * 2;
        SDL_FillRect(bltfx->surface, &box, rgb);
    }
    else if (tw->resize == TEXTWIN_RESIZE_DOWNLEFT ||
             tw->resize == TEXTWIN_RESIZE_LEFT ||
             tw->resize == TEXTWIN_RESIZE_UPLEFT)
    {
        box.x = TEXTWIN_ACTIVE_MIN;
        box.y = TEXTWIN_ACTIVE_MIN;
        box.w = TEXTWIN_ACTIVE_MAX - TEXTWIN_ACTIVE_MIN;
        box.h = widget_data[tw->wid].ht - TEXTWIN_ACTIVE_MIN * 2;
        SDL_FillRect(bltfx->surface, &box, rgb);
    }
} 

static void ShowWindowText(textwin_window_t *tw, _BLTFX *bltfx)
{
    uint32 botline = ((tw->linebuf_next)
                      ? tw->linebuf_next : tw->linebuf_used) -
                      (tw->scroll_yoff / tw->font->line_height) - 1;
    sint32 topline = (tw->linebuf_visi < tw->linebuf_used)
                     ? (sint32)(botline - tw->linebuf_visi) : -1;
    uint16 i;

    if (topline < 0)
    {
        if (tw->linebuf_used != tw->linebuf_size)
        {
            topline = 0;
        }
        else if (tw->linebuf_visi < tw->linebuf_used)
        {
            topline = tw->linebuf_size + 1 + topline;
        }
        else
        {
            topline = tw->linebuf_next;
        }
    }

#ifdef DEBUG_TEXTWIN
        if (tw->wid == WIDGET_MSGWIN_ID)
        {
            LOG(LOG_MSG,">>>>top=%d bot=%d\n", topline, botline);
        }
#endif

    /* Blit all the visible lines. */
    for (i = 0; i < tw->linebuf_visi &&
                i < tw->linebuf_used; i++)
    {
        uint16             line = (topline + i) % tw->linebuf_used;
        textwin_linebuf_t *linebuf = tw->linebuf + line;

        strout_blt(bltfx->surface, tw->font, linebuf->buf, -tw->scroll_xoff + 2,
                   tw->font->line_height * i, linebuf->fg, /*linebuf->bg,*/
                   NULL);
    }
}

/* Draw horizontal scrollbar. */
static void ShowWindowScrollbarH(textwin_window_t *tw, _BLTFX *bltfx)
{
    _Sprite  *barge_sprite,
             *canal_sprite,
             *left_sprite,
             *right_sprite;
    uint16    y,
              canal_pos,
              canal_len,
              barge_pos,
              barge_len;
    SDL_Rect  box;

    switch (tw->scroll)
    {
        case TEXTWIN_SCROLL_FREESCROLL:
        case TEXTWIN_SCROLL_HBARGE:
            barge_sprite = skin_sprites[SKIN_SPRITE_SLIDER_HL_HBARGE];
            canal_sprite = skin_sprites[SKIN_SPRITE_SLIDER_HCANAL];
            left_sprite = skin_sprites[SKIN_SPRITE_SLIDER_LEFT];
            right_sprite = skin_sprites[SKIN_SPRITE_SLIDER_RIGHT];

            break;

        case TEXTWIN_SCROLL_HCANALLEFT:
        case TEXTWIN_SCROLL_HCANALRIGHT:
            barge_sprite = skin_sprites[SKIN_SPRITE_SLIDER_HBARGE];
            canal_sprite = skin_sprites[SKIN_SPRITE_SLIDER_HL_HCANAL];
            left_sprite = skin_sprites[SKIN_SPRITE_SLIDER_LEFT];
            right_sprite = skin_sprites[SKIN_SPRITE_SLIDER_RIGHT];

            break;

        case TEXTWIN_SCROLL_LEFT:
            barge_sprite = skin_sprites[SKIN_SPRITE_SLIDER_HBARGE];
            canal_sprite = skin_sprites[SKIN_SPRITE_SLIDER_HCANAL];
            left_sprite = skin_sprites[SKIN_SPRITE_SLIDER_HL_LEFT];
            right_sprite = skin_sprites[SKIN_SPRITE_SLIDER_RIGHT];

            break;

        case TEXTWIN_SCROLL_RIGHT:
            barge_sprite = skin_sprites[SKIN_SPRITE_SLIDER_HBARGE];
            canal_sprite = skin_sprites[SKIN_SPRITE_SLIDER_HCANAL];
            left_sprite = skin_sprites[SKIN_SPRITE_SLIDER_LEFT];
            right_sprite = skin_sprites[SKIN_SPRITE_SLIDER_HL_RIGHT];

            break;

        default:
            barge_sprite = skin_sprites[SKIN_SPRITE_SLIDER_HBARGE];
            canal_sprite = skin_sprites[SKIN_SPRITE_SLIDER_HCANAL];
            left_sprite = skin_sprites[SKIN_SPRITE_SLIDER_LEFT];
            right_sprite = skin_sprites[SKIN_SPRITE_SLIDER_RIGHT];
    }

    y = widget_data[tw->wid].ht - canal_sprite->bitmap->h;

    /* Canal. */
    canal_pos = left_sprite->bitmap->w;
    canal_len = widget_data[tw->wid].wd - canal_pos - right_sprite->bitmap->w -
                skin_sprites[SKIN_SPRITE_SLIDER_VCANAL]->bitmap->w;
    box.x = 0;
    box.y = 0;
    box.w = canal_len;
    box.h = canal_sprite->bitmap->h;
    sprite_blt(canal_sprite, canal_pos, y, &box, bltfx);

    /* Left. */
    sprite_blt(left_sprite, 0, y, NULL, bltfx);

    /* Right. */
    sprite_blt(right_sprite, canal_pos + canal_len, y, NULL, bltfx);

    /* Barge. */
    barge_pos = (float)tw->scroll_xoff / (float)tw->maxcontentwd * canal_len +
                canal_pos;
    barge_len = (float)widget_data[tw->wid].wd / (float)tw->maxcontentwd *
               canal_len;
    box.w = barge_len;
    sprite_blt(barge_sprite, barge_pos,
               y + (box.h - barge_sprite->bitmap->h) / 2, &box,
               bltfx);

    tw->hbarge_pos = barge_pos;
    tw->hbarge_len = barge_len;
    tw->hcanal_pos = canal_pos;
    tw->hcanal_len = canal_len;
}

/* Draw vertical scrollbar. */
static void ShowWindowScrollbarV(textwin_window_t *tw, _BLTFX *bltfx)
{
    _Sprite  *barge_sprite,
             *canal_sprite,
             *down_sprite,
             *up_sprite;
    uint16    x,
              canal_pos,
              canal_len,
              barge_pos,
              barge_len;
    SDL_Rect  box;

    switch (tw->scroll)
    {
        case TEXTWIN_SCROLL_DOWN:
            barge_sprite = skin_sprites[SKIN_SPRITE_SLIDER_VBARGE];
            canal_sprite = skin_sprites[SKIN_SPRITE_SLIDER_VCANAL];
            down_sprite = skin_sprites[SKIN_SPRITE_SLIDER_HL_DOWN];
            up_sprite = skin_sprites[SKIN_SPRITE_SLIDER_UP];

            break;

        case TEXTWIN_SCROLL_UP:
            barge_sprite = skin_sprites[SKIN_SPRITE_SLIDER_VBARGE];
            canal_sprite = skin_sprites[SKIN_SPRITE_SLIDER_VCANAL];
            down_sprite = skin_sprites[SKIN_SPRITE_SLIDER_DOWN];
            up_sprite = skin_sprites[SKIN_SPRITE_SLIDER_HL_UP];

            break;

        case TEXTWIN_SCROLL_FREESCROLL:
        case TEXTWIN_SCROLL_VBARGE:
            barge_sprite = skin_sprites[SKIN_SPRITE_SLIDER_HL_VBARGE];
            canal_sprite = skin_sprites[SKIN_SPRITE_SLIDER_VCANAL];
            down_sprite = skin_sprites[SKIN_SPRITE_SLIDER_DOWN];
            up_sprite = skin_sprites[SKIN_SPRITE_SLIDER_UP];

            break;

        case TEXTWIN_SCROLL_VCANALDOWN:
        case TEXTWIN_SCROLL_VCANALUP:
            barge_sprite = skin_sprites[SKIN_SPRITE_SLIDER_VBARGE];
            canal_sprite = skin_sprites[SKIN_SPRITE_SLIDER_HL_VCANAL];
            down_sprite = skin_sprites[SKIN_SPRITE_SLIDER_DOWN];
            up_sprite = skin_sprites[SKIN_SPRITE_SLIDER_UP];

            break;

        default:
            barge_sprite = skin_sprites[SKIN_SPRITE_SLIDER_VBARGE];
            canal_sprite = skin_sprites[SKIN_SPRITE_SLIDER_VCANAL];
            down_sprite = skin_sprites[SKIN_SPRITE_SLIDER_DOWN];
            up_sprite = skin_sprites[SKIN_SPRITE_SLIDER_UP];
    }

    x = widget_data[tw->wid].wd - canal_sprite->bitmap->w;

    /* Canal. */
    canal_pos = up_sprite->bitmap->h;
    canal_len = widget_data[tw->wid].ht - canal_pos - down_sprite->bitmap->h -
                skin_sprites[SKIN_SPRITE_SLIDER_HCANAL]->bitmap->h;
    box.x = 0;
    box.y = 0;
    box.w = canal_sprite->bitmap->w;
    box.h = canal_len;
    sprite_blt(canal_sprite, x, canal_pos, &box, bltfx);

    /* Up. */
    sprite_blt(up_sprite, x, 0, NULL, bltfx);
 
    /* Down. */
    sprite_blt(down_sprite, x, canal_pos + canal_len, NULL, bltfx);

    /* Barge. */
    barge_pos = (float)(tw->maxcontentht - tw->contentht - tw->scroll_yoff) / (float)tw->maxcontentht * canal_len +
                canal_pos;
    barge_len = (float)widget_data[tw->wid].ht / (float)tw->maxcontentht *
               canal_len;
    box.h = barge_len;
    sprite_blt(barge_sprite, x + (box.w - barge_sprite->bitmap->w) / 2,
               barge_pos, &box, bltfx);

    tw->vbarge_pos = barge_pos;
    tw->vbarge_len = barge_len;
    tw->vcanal_pos = canal_pos;
    tw->vcanal_len = canal_len;
}

/* draw frame round window. */
static void ShowWindowFrame(textwin_window_t *tw, _BLTFX *bltfx)
{
    SDL_Rect     box;
    const uint32 rgb = SDL_MapRGB(bltfx->surface->format, 0x60, 0x60, 0x60);

    /* top */
    box.x = 0;
    box.y = 0;
    box.w = widget_data[tw->wid].wd;
    box.h = 1;
    SDL_FillRect(bltfx->surface, &box, rgb);
    /* bottom */
    box.y = widget_data[tw->wid].ht - 1;
    SDL_FillRect(bltfx->surface, &box, rgb);
    /* left */
    box.y = 0;
    box.w = 1;
    box.h = widget_data[tw->wid].ht;
    SDL_FillRect(bltfx->surface, &box, rgb);
    /* right */
    box.x = widget_data[tw->wid].wd - 1;
    SDL_FillRect(bltfx->surface, &box, rgb);
}

/* Handles keypresses in either textwindow. */
void textwin_keypress(SDLKey key, textwin_id_t id)
{
    textwin_window_t *tw = &textwin[id];

    switch (key)
    {
        case SDLK_UP:
            tw->scroll_y = 1 * tw->font->line_height;
            ScrollTextWindow(tw);
            tw->scroll_y = 0;

            break;

        case SDLK_DOWN:
            tw->scroll_y = -1 * tw->font->line_height;
            ScrollTextWindow(tw);
            tw->scroll_y = 0;

            break;

        case SDLK_PAGEUP:
            tw->scroll_y = tw->linebuf_visi * tw->font->line_height;
            ScrollTextWindow(tw);
            tw->scroll_y = 0;

            break;

        case SDLK_PAGEDOWN:
            tw->scroll_y = -tw->linebuf_visi * tw->font->line_height;
            ScrollTextWindow(tw);
            tw->scroll_y = 0;

            break;

        default:
            break;
    }
}

void textwin_event(uint8 e, SDL_Event *event, textwin_id_t id)
{
    textwin_window_t *tw = &textwin[id];
    const uint16  x = event->motion.x,
                  y = event->motion.y,
                  top = widget_data[tw->wid].y1,
                  right = widget_data[tw->wid].x1 +
                          widget_data[tw->wid].wd - 1,
                  bottom = widget_data[tw->wid].y1 +
                           widget_data[tw->wid].ht - 1,
                  left = widget_data[tw->wid].x1;
    const uint8   button = event->button.button;

    if (!(tw->mode == TEXTWIN_MODE_RESIZE &&
          tw->resize))
    {
        tw->resize = TEXTWIN_RESIZE_NONE;
    }

    tw->resize_x = 0;
    tw->resize_y = 0;

    if (!(tw->mode == TEXTWIN_MODE_SCROLL &&
          (tw->scroll == TEXTWIN_SCROLL_FREESCROLL ||
           tw->scroll == TEXTWIN_SCROLL_HBARGE ||
           tw->scroll == TEXTWIN_SCROLL_VBARGE)))
    {
        tw->scroll = TEXTWIN_SCROLL_NONE;
        tw->scroll_x = 0;
        tw->scroll_y = 0;
    }

    /* Resizing. */
    if (tw->mode != TEXTWIN_MODE_SCROLL &&
        tw->scroll == TEXTWIN_SCROLL_NONE)
    {
        /* If the mouse moves (whether or not the LMB is down), check if we're
         * over a resizing border. Remember it in tw->resize if we are. */
        if (e == SDL_MOUSEMOTION ||
            (e == SDL_MOUSEBUTTONDOWN &&
             button == SDL_BUTTON_LEFT))
        {
            const uint16 right_adj = (tw->maxcontentht > tw->contentht)
                                     ? right - skin_sprites[SKIN_SPRITE_SLIDER_VCANAL]->bitmap->w
                                     : right,
                         bottom_adj = (tw->maxcontentwd > tw->contentwd)
                                      ? bottom - skin_sprites[SKIN_SPRITE_SLIDER_HCANAL]->bitmap->h
                                      : bottom;

            if (y >= top + TEXTWIN_ACTIVE_MIN &&
                y <= top + TEXTWIN_ACTIVE_MAX)
            {
                if (x <= right_adj - TEXTWIN_ACTIVE_MIN &&
                    x >= right_adj - TEXTWIN_ACTIVE_MAX)
                {
                    tw->resize = TEXTWIN_RESIZE_UPRIGHT;
                }
                else if (x >= left + TEXTWIN_ACTIVE_MIN &&
                         x <= left + TEXTWIN_ACTIVE_MAX)
                {
                    tw->resize = TEXTWIN_RESIZE_UPLEFT;
                }
                else if (x <= right_adj - TEXTWIN_ACTIVE_MAX &&
                         x >= left + TEXTWIN_ACTIVE_MAX)
                {
                    tw->resize = TEXTWIN_RESIZE_UP;
                }
            }
            else if (y <= bottom_adj - TEXTWIN_ACTIVE_MIN &&
                     y >= bottom_adj - TEXTWIN_ACTIVE_MAX)
            {
                if (x <= right_adj - TEXTWIN_ACTIVE_MIN &&
                    x >= right_adj - TEXTWIN_ACTIVE_MAX)
                {
                    tw->resize = TEXTWIN_RESIZE_DOWNRIGHT;
                }
                else if (x >= left + TEXTWIN_ACTIVE_MIN &&
                         x <= left + TEXTWIN_ACTIVE_MAX)
                {
                    tw->resize = TEXTWIN_RESIZE_DOWNLEFT;
                }
                else if (x <= right_adj - TEXTWIN_ACTIVE_MAX &&
                         x >= left + TEXTWIN_ACTIVE_MAX)
                {
                    tw->resize = TEXTWIN_RESIZE_DOWN;
                }
            }
            else if (y >= top + TEXTWIN_ACTIVE_MAX &&
                     y <= bottom_adj - TEXTWIN_ACTIVE_MAX)
            {
                if (x <= right_adj - TEXTWIN_ACTIVE_MIN &&
                    x >= right_adj - TEXTWIN_ACTIVE_MAX)
                {
                    tw->resize = TEXTWIN_RESIZE_RIGHT;
                }
                else if (x >= left + TEXTWIN_ACTIVE_MIN &&
                         x <= left + TEXTWIN_ACTIVE_MAX)
                {
                    tw->resize = TEXTWIN_RESIZE_LEFT;
                }
            }

            /* If we are over a resizing border, record the amounts to resize
             * by in tw->resize_x and tw->resize_y. */
            if (tw->resize)
            {
                WIDGET_REDRAW(tw->wid) = 1;

                if (event->motion.xrel > 0)
                {
                    tw->resize_x = tw->font->c['_'].w;
                }
                else if (event->motion.xrel < 0)
                {
                    tw->resize_x = -tw->font->c['_'].w;
                }
                else
                {
                    tw->resize_x = 0;
                }

                if (event->motion.yrel > 0)
                {
                    tw->resize_y = tw->font->line_height;
                }
                else if (event->motion.yrel < 0)
                {
                    tw->resize_y = -tw->font->line_height;
                }
                else
                {
                    tw->resize_y = 0;
                }

                /* If the button is clicked, go into resizing mode. */
                if (e == SDL_MOUSEBUTTONDOWN)
                {
                    tw->mode = TEXTWIN_MODE_RESIZE;
                }
            }
        }
        /* If the LMB is released, change to no mode. */
        else if (e == SDL_MOUSEBUTTONUP &&
                 button == SDL_BUTTON_LEFT)
        {
            WIDGET_REDRAW(tw->wid) = 1;
            tw->mode = TEXTWIN_MODE_NONE;
            textwin_calculate_dimensions(tw);
        }
    }

    /* Scrolling. */
    if (tw->mode != TEXTWIN_MODE_RESIZE)
    {
        if (e == SDL_MOUSEMOTION ||
            (e == SDL_MOUSEBUTTONDOWN &&
             button == SDL_BUTTON_LEFT))
        {
            /* Horizontal. */
            if (tw->maxcontentwd > tw->contentwd)
            {
                if (tw->mode == TEXTWIN_MODE_SCROLL &&
                    (tw->scroll == TEXTWIN_SCROLL_FREESCROLL ||
                     tw->scroll == TEXTWIN_SCROLL_HBARGE))
                {
                    tw->scroll_x = event->motion.xrel * tw->font->c['_'].w;
                }
                else if (y >= bottom -
                              skin_sprites[SKIN_SPRITE_SLIDER_HCANAL]->bitmap->h &&
                         y <= bottom)
                {
                    if (x < left + tw->hcanal_pos)
                    {
                        tw->scroll = TEXTWIN_SCROLL_LEFT;
                        tw->scroll_x = -1 * tw->font->c['_'].w;
                    }
                    else if (x < left + tw->hbarge_pos)
                    {
                        tw->scroll = TEXTWIN_SCROLL_HCANALLEFT;
                        tw->scroll_x = -tw->contentwd * tw->font->c['_'].w;
                    }
                    else if (x < left + tw->hbarge_pos + tw->hbarge_len)
                    {
                        tw->scroll = TEXTWIN_SCROLL_HBARGE;
                        tw->scroll_x = event->motion.xrel * tw->font->c['_'].w;
                    }
                    else if (x < left + tw->hcanal_pos + tw->hcanal_len)
                    {
                        tw->scroll = TEXTWIN_SCROLL_HCANALRIGHT;
                        tw->scroll_x = tw->contentwd * tw->font->c['_'].w;
                    }
                    else if (x < left + tw->hcanal_pos + tw->hcanal_len +
                                 skin_sprites[SKIN_SPRITE_SLIDER_RIGHT]->bitmap->w)
                    {
                        tw->scroll = TEXTWIN_SCROLL_RIGHT;
                        tw->scroll_x = 1 * tw->font->c['_'].w;
                    }
                }
                else if (e == SDL_MOUSEBUTTONDOWN)
                {
                    tw->scroll = TEXTWIN_SCROLL_FREESCROLL;
                    tw->scroll_x = event->motion.xrel * tw->font->c['_'].w;
                }
            }

            /* Vertical. */
            if (tw->maxcontentht > tw->contentht)
            {
                if (tw->mode == TEXTWIN_MODE_SCROLL &&
                    (tw->scroll == TEXTWIN_SCROLL_FREESCROLL ||
                     tw->scroll == TEXTWIN_SCROLL_VBARGE))
                {
                    tw->scroll_y = -event->motion.yrel * tw->font->line_height;
                }
                else if (x >= right -
                              skin_sprites[SKIN_SPRITE_SLIDER_VCANAL]->bitmap->w &&
                         x <= right)
                {
                    if (y < top + tw->vcanal_pos)
                    {
                        tw->scroll = TEXTWIN_SCROLL_UP;
                        tw->scroll_y = 1 * tw->font->line_height;
                    }
                    else if (y < top + tw->vbarge_pos)
                    {
                        tw->scroll = TEXTWIN_SCROLL_VCANALUP;
                        tw->scroll_y = tw->linebuf_visi * tw->font->line_height;
                    }
                    else if (y < top + tw->vbarge_pos + tw->vbarge_len)
                    {
                        tw->scroll = TEXTWIN_SCROLL_VBARGE;
                        tw->scroll_y = -event->motion.yrel * tw->font->line_height;
                    }
                    else if (y < top + tw->vcanal_pos + tw->vcanal_len)
                    {
                        tw->scroll = TEXTWIN_SCROLL_VCANALDOWN;
                        tw->scroll_y = -tw->linebuf_visi * tw->font->line_height;
                    }
                    else if (y < top + tw->vcanal_pos + tw->vcanal_len +
                                 skin_sprites[SKIN_SPRITE_SLIDER_DOWN]->bitmap->h)
                    {
                        tw->scroll = TEXTWIN_SCROLL_DOWN;
                        tw->scroll_y = -1 * tw->font->line_height;
                    }
                }
                else if (e == SDL_MOUSEBUTTONDOWN)
                {
                    tw->scroll = TEXTWIN_SCROLL_FREESCROLL;
                    tw->scroll_y = -event->motion.yrel * tw->font->line_height;
                }
            }

            if (tw->scroll &&
                e == SDL_MOUSEBUTTONDOWN)
            {
                tw->mode = TEXTWIN_MODE_SCROLL;
            }
        }
        else if (e == SDL_MOUSEBUTTONDOWN)
        {
            if (button == SDL_BUTTON_WHEELUP)
            {
                tw->scroll_y = 1 * tw->font->line_height;
                ScrollTextWindow(tw);
                tw->scroll_y = 0;
            }
            else if (button == SDL_BUTTON_WHEELDOWN)
            {
                tw->scroll_y = -1 * tw->font->line_height;
                ScrollTextWindow(tw);
                tw->scroll_y = 0;
            }
        }
        else if (e == SDL_MOUSEBUTTONUP &&
                 button != SDL_BUTTON_RIGHT)
        {
            tw->mode = TEXTWIN_MODE_NONE;
            tw->scroll = TEXTWIN_SCROLL_NONE;
            tw->scroll_x = 0;
            tw->scroll_y = 0;
        }
    }
}

/* Resize tw according to tw->resize_x and tw->resize_y. */
static void ResizeTextWindow(textwin_window_t *tw)
{
    const sint16 xrel = tw->resize_x,
                 yrel = tw->resize_y;

    /* We may need to scroll the window if we're resizing left and/or up. */
    tw->scroll_x = (xrel < 0) ? xrel : 0;
    tw->scroll_y = (yrel > 0) ? yrel : 0;
    ScrollTextWindow(tw);
    tw->scroll_x = 0;
    tw->scroll_y = 0;

    if (tw->resize == TEXTWIN_RESIZE_UP ||
        tw->resize == TEXTWIN_RESIZE_UPRIGHT ||
        tw->resize == TEXTWIN_RESIZE_UPLEFT)
    {
        const sint32 posty = widget_data[tw->wid].y1 + yrel,
                     posth = widget_data[tw->wid].ht - yrel;

        /* Down. */
        if (yrel > 0)
        {
            if (posty > global_buttons.my)
            {
                return;
            }
            else if (posth <= TEXTWIN_HEIGHT_MIN)
            {
                widget_data[tw->wid].ht = TEXTWIN_HEIGHT_MIN;

                return;
            }
        }
        /* Up. */
        else if (yrel < 0)
        {
            if (posty < global_buttons.my)
            {
                return;
            }
            else if (posth >= TEXTWIN_HEIGHT_MAX)
            {
                widget_data[tw->wid].ht = TEXTWIN_HEIGHT_MAX;

                return;
            }
        }

        /* Down or up. */
        if (yrel)
        {
            widget_data[tw->wid].y1 = posty;
            widget_data[tw->wid].ht = posth;
        }
    }
    else if (tw->resize == TEXTWIN_RESIZE_DOWNRIGHT ||
             tw->resize == TEXTWIN_RESIZE_DOWN ||
             tw->resize == TEXTWIN_RESIZE_DOWNLEFT)
    {
        const sint32 posth = widget_data[tw->wid].ht + yrel;

        /* Down. */
        if (yrel > 0)
        {
            if (posth > global_buttons.my)
            {
                return;
            }
            else if (posth >= TEXTWIN_HEIGHT_MAX)
            {
                widget_data[tw->wid].ht = TEXTWIN_HEIGHT_MAX;

                return;
            }
        }
        /* Up. */
        else if (yrel < 0)
        {
            if (posth < global_buttons.my)
            {
                return;
            }
            else if (posth <= TEXTWIN_HEIGHT_MIN)
            {
                widget_data[tw->wid].ht = TEXTWIN_HEIGHT_MIN;

                return;
            }
        }

        /* Down or up. */
        if (yrel)
        {
            widget_data[tw->wid].ht = posth;
        }
    }

    if (tw->resize == TEXTWIN_RESIZE_UPRIGHT ||
        tw->resize == TEXTWIN_RESIZE_RIGHT ||
        tw->resize == TEXTWIN_RESIZE_DOWNRIGHT)
    {
        const sint32 postw = widget_data[tw->wid].wd + xrel;

        /* Right. */
        if (xrel > 0)
        {
            if (postw > global_buttons.mx)
            {
                return;
            }
            else if (postw >= TEXTWIN_WIDTH_MAX)
            {
                widget_data[tw->wid].wd = TEXTWIN_WIDTH_MAX;

                return;
            }
        }
        /* Left. */
        else if (xrel < 0)
        {
            if (postw < global_buttons.mx)
            {
                return;
            }
            else if (postw <= TEXTWIN_WIDTH_MIN)
            {
		widget_data[tw->wid].wd = TEXTWIN_WIDTH_MIN;

                return;
            }
        }

        /* Right or left. */
        if (xrel)
        {
            widget_data[tw->wid].wd = postw;
        }
    }
    else if (tw->resize == TEXTWIN_RESIZE_DOWNLEFT ||
             tw->resize == TEXTWIN_RESIZE_LEFT ||
             tw->resize == TEXTWIN_RESIZE_UPLEFT)
    {
        const sint32 postx = widget_data[tw->wid].x1 + xrel,
                     postw = widget_data[tw->wid].wd - xrel;

        /* Right. */
        if (xrel > 0)
        {
            if (postx > global_buttons.mx)
            {
                return;
            }
            else if (postw <= TEXTWIN_WIDTH_MIN)
            {
                widget_data[tw->wid].wd = TEXTWIN_WIDTH_MIN;

                return;
            }
        }
        /* Left. */
        else if (xrel < 0)
        {
            if (postx < global_buttons.mx)
            {
                return;
            }
            else if (postw >= TEXTWIN_WIDTH_MAX)
            {
                widget_data[tw->wid].wd = TEXTWIN_WIDTH_MAX;

                return;
            }
        }

        /* Right or left. */
        if (xrel)
        {
            widget_data[tw->wid].x1 = postx;
            widget_data[tw->wid].wd = postw;
        }
    }
}

/* Scroll tw according to tw->scroll_x and tw->scroll_y. */
static void ScrollTextWindow(textwin_window_t *tw)
{
    /* Horizontal. */
    if (tw->scroll_x &&
        tw->maxcontentwd > tw->contentwd)
    {
        /* Right. */
        if (tw->scroll_x > 0 &&
            tw->scroll_xoff < tw->maxcontentwd - tw->contentwd)
        {
            WIDGET_REDRAW(tw->wid) = 1;
            tw->scroll_xoff = MIN(tw->scroll_xoff + tw->scroll_x,
                                  tw->maxcontentwd - tw->contentwd);
        }
        /* Left. */
        else if (tw->scroll_x < 0 &&
                 tw->scroll_xoff > 0)
        {
            WIDGET_REDRAW(tw->wid) = 1;
            tw->scroll_xoff = MAX(0, tw->scroll_xoff + tw->scroll_x);
        }
    }

    /* Vertical. */
    if (tw->scroll_y &&
        tw->maxcontentht > tw->contentht)
    {
        /* Up. */
        if (tw->scroll_y > 0 &&
            tw->scroll_yoff < tw->maxcontentht - tw->contentht)
        {
            WIDGET_REDRAW(tw->wid) = 1;
            tw->scroll_yoff = MIN(tw->scroll_yoff + tw->scroll_y,
                                  tw->maxcontentht - tw->contentht);
        }
        /* Down. */
        else if (tw->scroll_y < 0 &&
                 tw->scroll_yoff > 0)
        {
            WIDGET_REDRAW(tw->wid) = 1;
            tw->scroll_yoff = MAX(0, tw->scroll_yoff + tw->scroll_y);
        }
    }
}

/* TODO: the stuff below has nothing to do with the textwindows so needs to be
 * renamed and moved to another/an own file. */
void textwin_add_history(char *text)
{
    register int i;

    /* If new line is empty or identical to last inserted one, skip it */
    if (!text[0] ||
        strcmp(InputHistory[1], text) == 0)
    {
        return;
    }

    for (i = MAX_HISTORY_LINES - 1; i > 1; i--) /* shift history lines */
    {
        strncpy(InputHistory[i], InputHistory[i - 1], MAX_INPUT_STRING);
    }

    strncpy(InputHistory[1], text, MAX_INPUT_STRING); /* insert new one */
    *InputHistory[0] = 0; /* clear tmp editing line */
    HistoryPos = 0;
}

void textwin_clear_history()
{
    register int i;

    for (i = 0; i < MAX_HISTORY_LINES; i++)
    {
        InputHistory[i][0] = 0; /* it's enough to clear only the first byte of each history line */
    }

    HistoryPos = 0;
}

void textwin_put_string(char *text)
{
    int len;

    len = strlen(text);
    strncpy(InputString, text, MAX_INPUT_STRING); /* copy buf to input buffer */
    CurrentCursorPos = InputCount = len;           /* set cursor after inserted text */
}
