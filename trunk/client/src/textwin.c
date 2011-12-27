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

static uint16 OldSliderPos = 0;

textwin_window_t textwin[TEXTWIN_NROF];

static void TextwinLog(char *message);
static void GrepForStatometer(char *message);
static void ConvertSmileys(char *message);
static void AddLine(textwin_window_t *tw, const uint32 flags, const uint32 colr,
                    const uint8 indent, const uint8 strong, const uint8 emphasis,
                    const uint8 underline, const char *message);
static void ShowWindowResizingBorders(textwin_window_t *tw, _BLTFX *bltfx);
static void ShowWindowText(textwin_window_t *tw, _BLTFX *bltfx);
static void ShowWindowScrollbar(textwin_window_t *tw, _BLTFX *bltfx);
static void ShowWindowFrame(textwin_window_t *tw, _BLTFX *bltfx);
static uint8 ScrollWindow(textwin_window_t *tw, const sint16 dist);
static void ResizeWindow(textwin_window_t *tw, const sint16 xrel, const sint16 yrel);

void textwin_init(textwin_id_t id)
{
    textwin_window_t  *tw = &textwin[id];
    textwin_text_t *text;

    if (id == TEXTWIN_CHAT_ID)
    {
        tw->widget = WIDGET_CHATWIN_ID;
    }
    else if (id == TEXTWIN_MSG_ID)
    {
        tw->widget = WIDGET_MSGWIN_ID;
    }
    else
    {
        return;
    }

    tw->scroll_pos = 0;
    tw->scroll_off = 0;
    tw->maxstringlen = widget_data[tw->widget].wd -
                       (skin_sprites[SKIN_SPRITE_SLIDER]->bitmap->w * 2) - 4;
    tw->scroll_size = options.textwin_scrollback;
    tw->scroll_used = 0;
    MALLOC(text, sizeof(textwin_text_t) * tw->scroll_size);
    tw->text = text;
    textwin_set_font(id);
}

void textwin_set_font(textwin_id_t id)
{
    textwin_window_t *tw = &textwin[id];

    switch ((id == TEXTWIN_CHAT_ID) ? options.textwin_chat_font :
            options.textwin_msg_font)
    {
        case 0:
            tw->font = &font_tiny_out;

            break;

        case 2:
            tw->font = &font_small_out;

            break;

        case 3:
            tw->font = &font_medium;

            break;

        case 4:
            tw->font = &font_medium_out;

            break;

        case 5:
            tw->font = &font_large_out;

            break;

        default:
            tw->font = &font_small;
    }

    WIDGET_REDRAW(tw->widget) = 1;
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
        /* Log before ignores, chatfilters and so on. */
        if (options.textwin_use_logging == 1 ||
            options.textwin_use_logging >= 3)
        {
            TextwinLog(buf);
        }

        /* Unless this is an gmaster communicating in an official capacity, we
         * can ignore it. This is of course easy to work around. Just replace
         * the following expression with 1 and recompile to ignore gmasters too.
         * But officially they must be heard. */
        if (!(flags & NDI_FLAG_GMASTER))
        {
            if (((flags & NDI_FLAG_SAY) &&
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
            chatfilter_filter(buf); /* Filter incoming msg for f*words */
        }

        if (buddy_check(buf)) /* Color messages from buddys */
        {
            flags |= NDI_FLAG_BUDDY;
        }

        /* save last incoming tell player for client sided /reply */
        if ((flags & NDI_FLAG_TELL))
        {
            strcpy(cpl.player_reply, buf);
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
        GrepForStatometer(buf);

        if (options.textwin_use_logging >= 2)
        {
            TextwinLog(buf);
        }
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
                if ((w += tw->font->c[(unsigned char)(*c)].w +
                     tw->font->char_offset) >= tw->maxstringlen)
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
    WIDGET_REDRAW(textwin[id].widget) = 1;
}

static void TextwinLog(char *message)
{
    static PHYSFS_File *handle = NULL;
    time_t              t;
    char                buf[HUGE_BUF];

    if (PHYSFS_isInitialised &&
        PHYSFS_getWriteDir())
    {
        if (!handle)
        {
            char fname[TINY_BUF];

            sprintf(fname, "%s/%s", DIR_LOGS, FILE_TEXTWINLOG);

            /* We only log this stuff when opening the chat log fails. */
            if (!(handle = PHYSFS_openAppend(fname)))
            {
                LOG(LOG_SYSTEM, "Saving '%s'... ", fname);
                LOG(LOG_ERROR, "FAILED (%s)!\n", PHYSFS_getLastError());

                return;
            }
        }

        time(&t);
        strftime(buf, sizeof(buf), "%d-%m-%y %H:%M:%S", localtime(&t));
        sprintf(strchr(buf, '\0'),": %s\n", message);
        PHYSFS_writeString(handle, buf);
    }
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
    unsigned char actChar = 0;
    uint16        i;

    for (i = 0; message[i] != 0; i++)
    {
        uint16 j = i + 1;
        uint8  move = 1;

        if (message[i] == ':')
        {
            if (message[j] == '\'' &&
                message[j + 1] == '(')
            {
                move++;
                actChar = 138;
            }
            else if (message[j] == '-')
            {
                j++;
                move++;
            }

            /* we replace it with the 'ASCII'-code of the smiley in systemfont */
            switch (message[j])
            {
                case ')':
                    actChar = 128;

                    break;

                case '(':
                    actChar = 129;

                    break;

                case 'D':
                    actChar = 130;

                    break;

                case '|':
                    actChar = 131;

                    break;

                case 'o':
                case 'O':
                case '0':
                    actChar = 132;

                    break;

                case 'p':
                case 'P':
                    actChar = 133;

                    break;

                case 's':
                case 'S':
                    actChar = 139;

                    break;

                case 'x':
                case 'X':
                    actChar = 140;

                    break;
            }
        }
        else if (message[i] == ';')
        {
            if (message[j] == '-')
            {
                j++;
                move++;
            }

            switch (message[j])
            {
                case ')':
                    actChar = 134;

                    break;

                case 'p':
                    actChar = 137;

                    break;

                case 'P':
                    actChar = 137;

                    break;
            }
        }
        else if ((message[i] == '8' ||
                  message[i] == 'B') &&
                 message[i + 1] == ')')
        {
            actChar = 135;
        }
        else if ((message[i] == '8' ||
                  message[i] == 'B') &&
                 message[i + 1] == '-' &&
                 message[i + 2] == ')')
        {
            move++;
            actChar = 135;
        }
        else if (message[i] == '^' &&
                 message[i + 2] == '^' &&
                 (message[i + 1] == '_' ||
                  message[i + 1] == '-'))
        {
            move++;
            actChar = 136;
        }
        else if (message[i] == '>' &&
                 message[i + 1] == ':')
        {
            j++;
            move++;

            if (message[j] == '-')
            {
                j++;
                move++;
            }

            if (message[j] == ')')
            {
                actChar = 141;
            }
            else if (message[j] == 'D')
            {
                actChar = 142;
            }
        }

        if (actChar != 0)
        {
            message[i] = actChar;
            memmove(&message[i + 1], &message[i + 1 + move],
                    strlen(&message[i + 1 + move]) + 1);
        }
    }
}

/* Add message (already cut down to a single lines worth) to the textwindow structure. */
static void AddLine(textwin_window_t *tw, const uint32 flags, const uint32 colr,
                    const uint8 indent, const uint8 strong, const uint8 emphasis,
                    const uint8 underline, const char *message)
{
    uint16 line = tw->scroll_pos;
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

    sprintf((tw->text + line)->buf, "%s%s", buf, message);
    (tw->text + line)->flags = flags;

    if (!(flags & NDI_FLAG_PLAYER))
    {
        (tw->text + line)->fg = colr;
        (tw->text + line)->bg = 0;
    }
    else
    {
        if ((flags & NDI_FLAG_GMASTER))
        {
            (tw->text + line)->fg = skin_prefs.chat_gmaster;
        }
        else if ((flags & NDI_FLAG_EMOTE))
        {
            (tw->text + line)->fg = skin_prefs.chat_emote;
        }
        else if ((flags & NDI_FLAG_GSAY))
        {
            (tw->text + line)->fg = skin_prefs.chat_gsay;
        }
        else if ((flags & NDI_FLAG_SAY))
        {
            (tw->text + line)->fg = skin_prefs.chat_say;
        }
        else if ((flags & NDI_FLAG_SHOUT))
        {
            (tw->text + line)->fg = skin_prefs.chat_shout;
        }
        else if ((flags & NDI_FLAG_TELL))
        {
            (tw->text + line)->fg = skin_prefs.chat_tell;
        }
        else
        {
            (tw->text + line)->fg = colr;
        }

        if ((flags & NDI_FLAG_EAVESDROP))
        {
            (tw->text + line)->bg = skin_prefs.chat_eavesdrop;
        }
        else if ((flags & NDI_FLAG_BUDDY))
        {
            (tw->text + line)->bg = skin_prefs.chat_buddy;
        }
        else if ((flags & NDI_FLAG_CHANNEL))
        {
            (tw->text + line)->bg = skin_prefs.chat_channel;
        }
        else
        {
            (tw->text + line)->bg = 0;
        }
    }

    if (tw->scroll_off)
    {
        tw->scroll_off++;
    }

    if (tw->scroll_used < tw->scroll_size)
    {
        tw->scroll_used++;
    }

    tw->scroll_pos = (line + 1) % tw->scroll_size;
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
                     SDL_SRCALPHA | SDL_RLEACCEL, 0);
        bg = SDL_DisplayFormat(skin_sprites[SKIN_SPRITE_ALPHA]->bitmap);
    }

    /* if we don't have a backbuffer, create it */
    if (!widget_surface[tw->widget])
    {
        widget_surface[tw->widget] = SDL_DisplayFormatAlpha(skin_sprites[SKIN_SPRITE_ALPHA]->bitmap);
        SDL_SetColorKey(widget_surface[tw->widget], SDL_SRCCOLORKEY | SDL_RLEACCEL,
                        SDL_MapRGB(widget_surface[tw->widget]->format, 0x00, 0x00,
                                   0x00));
    }

    /* lets draw the widgets in the backbuffer */
    if (WIDGET_REDRAW(tw->widget))
    {
        /* Draw a textwindow in this order:
         *   the background;
         *   the resizing borders, if necessary;
         *   the text, if necessary;
         *   the scrollbar, if necessary; then
         *   the frame. */
        WIDGET_REDRAW(tw->widget) = 0;
        bltfx.surface = widget_surface[tw->widget];
        bltfx.flags = 0;
        bltfx.alpha = 0;
        SDL_FillRect(bltfx.surface, NULL,
                     SDL_MapRGBA(widget_surface[tw->widget]->format, 0x00, 0x00,
                                 0x00, 0));
        tw->x = widget_data[tw->widget].x1;
        tw->y = widget_data[tw->widget].y1;
//widget_data[tw->widget].ht = (widget_data[tw->widget].ht / tw->font->line_height + 1) * tw->font->line_height;
        tw->size = widget_data[tw->widget].ht / tw->font->line_height;
        tw->maxstringlen = widget_data[tw->widget].wd -
                           (skin_sprites[SKIN_SPRITE_SLIDER]->bitmap->w * 2) -
                           4;

        if (tw->resize)
        {
            ShowWindowResizingBorders(tw, &bltfx);
        }

        if (tw->scroll_used)
        {
            ShowWindowText(tw, &bltfx);
        }

        if (tw->scroll_used > tw->size)
        {
            ShowWindowScrollbar(tw, &bltfx);
        }
        else
        {
            tw->slider_h = 0;
        }

        ShowWindowFrame(tw, &bltfx);
    }

    box.x = widget_data[tw->widget].x1;
    box.y = widget_data[tw->widget].y1;
    box.w = widget_data[tw->widget].wd;
    box.h = widget_data[tw->widget].ht;
    SDL_SetClipRect(ScreenSurface, &box);
    SDL_BlitSurface(bg, NULL, ScreenSurface, &box);
    SDL_BlitSurface(widget_surface[tw->widget], NULL, ScreenSurface, &box);
    SDL_SetClipRect(ScreenSurface, NULL);
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
        box.w = widget_data[tw->widget].wd - TEXTWIN_ACTIVE_MIN * 2;
        box.h = TEXTWIN_ACTIVE_MAX - TEXTWIN_ACTIVE_MIN;
        SDL_FillRect(bltfx->surface, &box, rgb);
    }
    else if (tw->resize == TEXTWIN_RESIZE_DOWNRIGHT ||
             tw->resize == TEXTWIN_RESIZE_DOWN ||
             tw->resize == TEXTWIN_RESIZE_DOWNLEFT)
    {
        box.x = TEXTWIN_ACTIVE_MIN;
        box.y = widget_data[tw->widget].ht - TEXTWIN_ACTIVE_MAX - 1;
        box.w = widget_data[tw->widget].wd - TEXTWIN_ACTIVE_MIN * 2;
        box.h = TEXTWIN_ACTIVE_MAX - TEXTWIN_ACTIVE_MIN;
        SDL_FillRect(bltfx->surface, &box, rgb);
    }

    if (tw->resize == TEXTWIN_RESIZE_UPRIGHT ||
        tw->resize == TEXTWIN_RESIZE_RIGHT ||
        tw->resize == TEXTWIN_RESIZE_DOWNRIGHT)
    {
        box.x = widget_data[tw->widget].wd - TEXTWIN_ACTIVE_MAX - 1;
        box.y = TEXTWIN_ACTIVE_MIN;
        box.w = TEXTWIN_ACTIVE_MAX - TEXTWIN_ACTIVE_MIN;
        box.h = widget_data[tw->widget].ht - TEXTWIN_ACTIVE_MIN * 2;
        SDL_FillRect(bltfx->surface, &box, rgb);
    }
    else if (tw->resize == TEXTWIN_RESIZE_DOWNLEFT ||
             tw->resize == TEXTWIN_RESIZE_LEFT ||
             tw->resize == TEXTWIN_RESIZE_UPLEFT)
    {
        box.x = TEXTWIN_ACTIVE_MIN;
        box.y = TEXTWIN_ACTIVE_MIN;
        box.w = TEXTWIN_ACTIVE_MAX - TEXTWIN_ACTIVE_MIN;
        box.h = widget_data[tw->widget].ht - TEXTWIN_ACTIVE_MIN * 2;
        SDL_FillRect(bltfx->surface, &box, rgb);
    }
} 

static void ShowWindowText(textwin_window_t *tw, _BLTFX *bltfx)
{
    sint32    topline = tw->scroll_pos - tw->scroll_off - MIN(tw->size,
                        tw->scroll_used);
    uint16    i;


    if (topline < 0)
    {
        if (tw->scroll_used == tw->scroll_size)
        {
            if (tw->size >= tw->scroll_size)
            {
                topline = tw->scroll_pos;
            }
            else
            {
                topline = tw->scroll_size + topline + 1;
            }
        }
        else
        {
            topline = 0;
        }
    }

    /* TODO: fucking maths */

    /* Blit all the visible lines. */
    for (i = 0; i < tw->size && i < tw->scroll_used; i++)
    {
        textwin_text_t *text = (tw->text + ((topline + i) % tw->scroll_used));

//LOG(LOG_MSG,">>>>>>>>>>>>>>%d,%d,%d,%d,%d,%s\n", tw->scroll_off, tw->scroll_pos, tw->size, topline, i, text->buf);
        string_blt(bltfx->surface, tw->font, text->buf, 2,
                   tw->font->line_height * i, text->fg, /*text->bg,*/ NULL,
                   NULL);
    }
}

/* Draw scrollbar. */
static void ShowWindowScrollbar(textwin_window_t *tw, _BLTFX *bltfx)
{
    SDL_Rect  box;
    uint16    x2 = widget_data[tw->widget].wd -
                   skin_sprites[SKIN_SPRITE_SLIDER]->bitmap->w,
              h = widget_data[tw->widget].ht - 
                  skin_sprites[SKIN_SPRITE_SLIDER_UP]->bitmap->h -
                  skin_sprites[SKIN_SPRITE_SLIDER_DOWN]->bitmap->h,
              sy = ((tw->scroll_used - tw->size - tw->scroll_off) * h) /
                   tw->scroll_used,
              sh = MAX(1, (tw->size * h) / tw->scroll_used); /* between 0.0 <-> 1.0 */
     
    box.x = box.y = 0;
    box.w = skin_sprites[SKIN_SPRITE_SLIDER]->bitmap->w;
    box.h = h;
    sprite_blt(skin_sprites[SKIN_SPRITE_SLIDER_UP], x2, 0, NULL, bltfx);
    sprite_blt(skin_sprites[SKIN_SPRITE_SLIDER], x2,
               skin_sprites[SKIN_SPRITE_SLIDER_UP]->bitmap->h, &box, bltfx);
    sprite_blt(skin_sprites[SKIN_SPRITE_SLIDER_DOWN], x2, widget_data[tw->widget].ht -
               skin_sprites[SKIN_SPRITE_SLIDER_DOWN]->bitmap->h, NULL, bltfx);
 
    if (!tw->scroll_off &&
        sy + sh < h)
    {
        sy++;
    }
 
    box.h = sh;
    sprite_blt(skin_sprites[SKIN_SPRITE_TWIN_SCROLL], x2 + 2,
               skin_sprites[SKIN_SPRITE_SLIDER_UP]->bitmap->h + sy + 1, &box, bltfx);
 
    if (tw->highlight == TEXTWIN_HIGHLIGHT_UP)
    {
        box.x = x2;
        box.y = 0;
        box.h = skin_sprites[SKIN_SPRITE_SLIDER_UP]->bitmap->h;
        box.w = 1;
        SDL_FillRect(bltfx->surface, &box, -1);
        box.x += skin_sprites[SKIN_SPRITE_SLIDER_UP]->bitmap->w - 1;
        SDL_FillRect(bltfx->surface, &box, -1);
        box.w = skin_sprites[SKIN_SPRITE_SLIDER_UP]->bitmap->w - 1;
        box.h = 1;
        box.x = x2;
        SDL_FillRect(bltfx->surface, &box, -1);
        box.y += skin_sprites[SKIN_SPRITE_SLIDER_UP]->bitmap->h - 1;
        SDL_FillRect(bltfx->surface, &box, -1);
    }
    else if (tw->highlight == TEXTWIN_HIGHLIGHT_ABOVE)
    {
        box.x = x2 + 2;
        box.y = skin_sprites[SKIN_SPRITE_SLIDER_UP]->bitmap->h + 2;
        box.h = sy + 1;
        box.w = 5;
        SDL_FillRect(bltfx->surface, &box, 0);
    }
    else if (tw->highlight == TEXTWIN_HIGHLIGHT_SLIDER)
    {
        box.x = x2 + 2;
        box.y = skin_sprites[SKIN_SPRITE_SLIDER_UP]->bitmap->h + 3 + sy;
        box.w = 1;
        SDL_FillRect(bltfx->surface, &box, -1);
        box.x += 4;
        SDL_FillRect(bltfx->surface, &box, -1);
        box.x -= 4;
        box.h = 1;
        box.w = 4;
        SDL_FillRect(bltfx->surface, &box, -1);
        box.y += sh - 1;
        SDL_FillRect(bltfx->surface, &box, -1);
    }
    else if (tw->highlight == TEXTWIN_HIGHLIGHT_UNDER)
    {
        box.x = x2 + 2;
        box.h = tw->size * tw->font->line_height - sy - sh -
                tw->font->line_height;
        box.y = skin_sprites[SKIN_SPRITE_SLIDER_UP]->bitmap->h + 3 + sy + box.h;
        box.w = 5;
        SDL_FillRect(bltfx->surface, &box, 0);
    }
    else if (tw->highlight == TEXTWIN_HIGHLIGHT_DOWN)
    {
        box.x = x2;
        box.y = tw->size * tw->font->line_height + 4;
        box.h = skin_sprites[SKIN_SPRITE_SLIDER_UP]->bitmap->h;
        box.w = 1;
        SDL_FillRect(bltfx->surface, &box, -1);
        box.x += skin_sprites[SKIN_SPRITE_SLIDER_UP]->bitmap->w - 1;
        SDL_FillRect(bltfx->surface, &box, -1);
        box.w = skin_sprites[SKIN_SPRITE_SLIDER_UP]->bitmap->w - 1;
        box.h = 1;
        box.x = x2;
        SDL_FillRect(bltfx->surface, &box, -1);
        box.y += skin_sprites[SKIN_SPRITE_SLIDER_UP]->bitmap->h - 1;
        SDL_FillRect(bltfx->surface, &box, -1);
    }
 
    tw->slider_h = sh;
    tw->slider_y = sy;
}

/* draw frame round window. */
static void ShowWindowFrame(textwin_window_t *tw, _BLTFX *bltfx)
{
    SDL_Rect     box;
    const uint32 rgb = SDL_MapRGB(bltfx->surface->format, 0x60, 0x60, 0x60);

    /* top */
    box.x = 0;
    box.y = 0;
    box.w = widget_data[tw->widget].wd;
    box.h = 1;
    SDL_FillRect(bltfx->surface, &box, rgb);
    /* right */
    box.x = widget_data[tw->widget].wd - 1;
    box.y = 0;
    box.w = 1;
    box.h = widget_data[tw->widget].ht;
    SDL_FillRect(bltfx->surface, &box, rgb);
    /* bottom */
    box.x = 0;
    box.y = widget_data[tw->widget].ht - 1;
    box.w = widget_data[tw->widget].wd;
    box.h = 1;
    SDL_FillRect(bltfx->surface, &box, rgb);
    /* left */
    box.x = 0;
    box.y = 0;
    box.w = 1;
    box.h = widget_data[tw->widget].ht;
    SDL_FillRect(bltfx->surface, &box, rgb);
}

/* Handles keypresses in either textwindow. */
void textwin_keypress(SDLKey key, textwin_id_t id)
{
    textwin_window_t *tw = &textwin[id];

    switch (key)
    {
        case SDLK_UP:
            ScrollWindow(tw, 1);

            break;

        case SDLK_DOWN:
            ScrollWindow(tw, -1);

            break;

        case SDLK_PAGEUP:
            ScrollWindow(tw, tw->size);

            break;

        case SDLK_PAGEDOWN:
            ScrollWindow(tw, -tw->size);

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
                  top = widget_data[tw->widget].y1,
                  right = widget_data[tw->widget].x1 +
                          widget_data[tw->widget].wd - 1,
                  bottom = widget_data[tw->widget].y1 +
                           widget_data[tw->widget].ht - 1,
                  left = widget_data[tw->widget].x1;
    const uint8   button = event->button.button;

    tw->highlight = TEXTWIN_HIGHLIGHT_NONE;

    if (!(tw->flags & TEXTWIN_FLAG_RESIZE))
    {
        if (e == SDL_MOUSEMOTION)
        {
            if (x >= right - skin_sprites[SKIN_SPRITE_SLIDER]->bitmap->w &&
                x <= right &&
                y >= top &&
                button != SDL_BUTTON_LEFT)
            {
                WIDGET_REDRAW(tw->widget) = 1;

#define OFFSET (tw->y + skin_sprites[SKIN_SPRITE_SLIDER_UP]->bitmap->h)
                if (y < OFFSET)
                {
                    tw->highlight = TEXTWIN_HIGHLIGHT_UP;
                }
                else if (y < OFFSET + tw->slider_y)
                {
                    tw->highlight = TEXTWIN_HIGHLIGHT_ABOVE;
                }
                else if (y < OFFSET + tw->slider_y + tw->slider_h + 3)
                {
                    tw->highlight = TEXTWIN_HIGHLIGHT_SLIDER;
                }
                else if (y < widget_data[tw->widget].y1 + tw->size *
                             tw->font->line_height + 4)
                {
                    tw->highlight = TEXTWIN_HIGHLIGHT_UNDER;
                }
                else if (y < widget_data[tw->widget].y1 +
                             widget_data[tw->widget].ht)
                {
                    tw->highlight = TEXTWIN_HIGHLIGHT_DOWN;
                }
#undef OFFSET
            }
        }
        else if (e == SDL_MOUSEBUTTONDOWN)
        {
            WIDGET_REDRAW(tw->widget) = 1;

            if (button == SDL_BUTTON_WHEELUP)
            {
                ScrollWindow(tw, 1);
            }
            else if (button == SDL_BUTTON_WHEELDOWN)
            {
                ScrollWindow(tw, -1);
            }
            else if (button == SDL_BUTTON_LEFT)
            {
                if (tw->highlight == TEXTWIN_HIGHLIGHT_UP) /* clicked scroller-button up */
                {
                    ScrollWindow(tw, 1);
                }
                else if (tw->highlight == TEXTWIN_HIGHLIGHT_ABOVE) /* clicked above the slider */
                {
                    ScrollWindow(tw, tw->size);
                }
                else if (tw->highlight == TEXTWIN_HIGHLIGHT_SLIDER)
                {
                    /* clicked on the slider */
                    tw->flags |= TEXTWIN_FLAG_SCROLL;
                    OldSliderPos = y - tw->slider_y;
                }
                else if (tw->highlight == TEXTWIN_HIGHLIGHT_UNDER) /* clicked under the slider */
                {
                    ScrollWindow(tw, -tw->size);
                }
                else if (tw->highlight == TEXTWIN_HIGHLIGHT_DOWN) /* clicked scroller-button down */
                {
                    ScrollWindow(tw, -1);
                }
            }
        }
        else if (e == SDL_MOUSEBUTTONUP)
        {
            tw->flags &= ~TEXTWIN_FLAG_SCROLL;
        }
    }

    if (!(tw->flags & TEXTWIN_FLAG_SCROLL))
    {
        if (e == SDL_MOUSEMOTION ||
            (e == SDL_MOUSEBUTTONDOWN &&
             button == SDL_BUTTON_LEFT))
        {
            if (y >= top + TEXTWIN_ACTIVE_MIN &&
                y <= top + TEXTWIN_ACTIVE_MAX)
            {
                if (x <= right - TEXTWIN_ACTIVE_MIN &&
                    x >= right - TEXTWIN_ACTIVE_MAX)
                {
                    tw->resize = TEXTWIN_RESIZE_UPRIGHT;
                }
                else if (x >= left + TEXTWIN_ACTIVE_MIN &&
                         x <= left + TEXTWIN_ACTIVE_MAX)
                {
                    tw->resize = TEXTWIN_RESIZE_UPLEFT;
                }
                else if (x <= right - TEXTWIN_ACTIVE_MAX &&
                         x >= left + TEXTWIN_ACTIVE_MAX)
                {
                    tw->resize = TEXTWIN_RESIZE_UP;
                }
            }
            else if (y <= bottom - TEXTWIN_ACTIVE_MIN &&
                     y >= bottom - TEXTWIN_ACTIVE_MAX)
            {
                if (x <= right - TEXTWIN_ACTIVE_MIN &&
                    x >= right - TEXTWIN_ACTIVE_MAX)
                {
                    tw->resize = TEXTWIN_RESIZE_DOWNRIGHT;
                }
                else if (x >= left + TEXTWIN_ACTIVE_MIN &&
                         x <= left + TEXTWIN_ACTIVE_MAX)
                {
                    tw->resize = TEXTWIN_RESIZE_DOWNLEFT;
                }
                else if (x <= right - TEXTWIN_ACTIVE_MAX &&
                         x >= left + TEXTWIN_ACTIVE_MAX)
                {
                    tw->resize = TEXTWIN_RESIZE_DOWN;
                }
            }
            else if (y >= top + TEXTWIN_ACTIVE_MAX &&
                     y <= bottom - TEXTWIN_ACTIVE_MAX)
            {
                if (x <= right - TEXTWIN_ACTIVE_MIN &&
                    x >= right - TEXTWIN_ACTIVE_MAX)
                {
                    tw->resize = TEXTWIN_RESIZE_RIGHT;
                }
                else if (x >= left + TEXTWIN_ACTIVE_MIN &&
                         x <= left + TEXTWIN_ACTIVE_MAX)
                {
                    tw->resize = TEXTWIN_RESIZE_LEFT;
                }
            }

            if (e == SDL_MOUSEBUTTONDOWN &&
                tw->resize)
            {
                tw->flags |= TEXTWIN_FLAG_RESIZE;
            }

            WIDGET_REDRAW(tw->widget) = 1;
        }
        else if (e == SDL_MOUSEBUTTONUP &&
                 button == SDL_BUTTON_LEFT)
        {
            tw->flags &= ~TEXTWIN_FLAG_RESIZE;
            tw->resize = TEXTWIN_RESIZE_NONE;
            WIDGET_REDRAW(tw->widget) = 1;
        }
    }

//    if ((tw->flags & TEXTWIN_FLAG_SCROLL))
//    {
//        ScrollWindow(tw, x, y);
//    }

    if ((tw->flags & TEXTWIN_FLAG_RESIZE))
    {
        ResizeWindow(tw, event->motion.xrel, event->motion.yrel);
    }
}

/* Scroll the visible window by dist lines -- positive for up, negative for
 * down. 'Scroll' is not really right -- we don't scroll, we reposition. Maybe
 * in future we can do real scrolling? */
static uint8 ScrollWindow(textwin_window_t *tw, const sint16 dist)
{
    /* No scrolling small windows. */
    if (tw->scroll_used < tw->size)
    {
        return 0;
    }

    if (dist > 0 &&
        tw->scroll_off < tw->scroll_used - tw->size)
    {
        tw->scroll_off += MIN(dist, tw->scroll_used - tw->size - tw->scroll_off);
        WIDGET_REDRAW(tw->widget) = 1;

        return 1;
    }
    else if (dist < 0 &&
             tw->scroll_off > 0)
    {
        tw->scroll_off += MAX(dist, -tw->scroll_off);
        WIDGET_REDRAW(tw->widget) = 1;

        return 1;
    }

    return 0;
//    tw->slider_y = y - OldSliderPos;
//    tw->scroll_off = tw->scroll_used - tw->size - (tw->scroll_used *
//                 tw->slider_y) / (tw->size * tw->font->line_height - 1);
}

static void ResizeWindow(textwin_window_t *tw, const sint16 xrel, const sint16 yrel)
{
    const sint32  postwidth = widget_data[tw->widget].wd + xrel,
                  postheight = widget_data[tw->widget].ht + yrel;

    if (tw->resize == TEXTWIN_RESIZE_UP ||
        tw->resize == TEXTWIN_RESIZE_UPRIGHT ||
        tw->resize == TEXTWIN_RESIZE_UPLEFT)
    {
        if (postheight <= TEXTWIN_HEIGHT_MIN &&
            yrel > 0)
        {
            tw->resize = TEXTWIN_RESIZE_NONE;
            widget_data[tw->widget].ht = TEXTWIN_HEIGHT_MIN;
        }
        else if (postheight >= TEXTWIN_HEIGHT_MAX &&
                 yrel < 0)
        {
            tw->resize = TEXTWIN_RESIZE_NONE;
            widget_data[tw->widget].ht = TEXTWIN_HEIGHT_MAX;
        }
        else if (yrel)
        {
            widget_data[tw->widget].y1 += yrel;
            widget_data[tw->widget].ht -= yrel;
        }
    }
    else if (tw->resize == TEXTWIN_RESIZE_DOWNRIGHT ||
             tw->resize == TEXTWIN_RESIZE_DOWN ||
             tw->resize == TEXTWIN_RESIZE_DOWNLEFT)
    {
        if (postheight <= TEXTWIN_HEIGHT_MIN &&
            yrel < 0)
        {
            tw->resize = TEXTWIN_RESIZE_NONE;
            widget_data[tw->widget].ht = TEXTWIN_HEIGHT_MIN;
        }
        else if (postheight >= TEXTWIN_HEIGHT_MAX &&
                 yrel > 0)
        {
            tw->resize = TEXTWIN_RESIZE_NONE;
            widget_data[tw->widget].ht = TEXTWIN_HEIGHT_MAX;
        }
        else if (yrel)
        {
            widget_data[tw->widget].ht += yrel;
        }
    }

    if (tw->resize == TEXTWIN_RESIZE_UPRIGHT ||
        tw->resize == TEXTWIN_RESIZE_RIGHT ||
        tw->resize == TEXTWIN_RESIZE_DOWNRIGHT)
    {
        if (postwidth <= TEXTWIN_WIDTH_MIN &&
            xrel < 0)
        {
            tw->resize = TEXTWIN_RESIZE_NONE;
            widget_data[tw->widget].wd = TEXTWIN_WIDTH_MIN;
        }
        else if (postwidth >= TEXTWIN_WIDTH_MAX &&
                 xrel > 0)
        {
            tw->resize = TEXTWIN_RESIZE_NONE;
            widget_data[tw->widget].wd = TEXTWIN_WIDTH_MAX;
        }
        else if (xrel)
        {
            widget_data[tw->widget].wd += xrel;
        }
    }
    else if (tw->resize == TEXTWIN_RESIZE_DOWNLEFT ||
             tw->resize == TEXTWIN_RESIZE_LEFT ||
             tw->resize == TEXTWIN_RESIZE_UPLEFT)
    {
        if (postwidth <= TEXTWIN_WIDTH_MIN &&
            xrel > 0)
        {
            tw->resize = TEXTWIN_RESIZE_NONE;
            widget_data[tw->widget].wd = TEXTWIN_WIDTH_MIN;
        }
        else if (postwidth >= TEXTWIN_WIDTH_MAX &&
                 xrel < 0)
        {
            tw->resize = TEXTWIN_RESIZE_NONE;
            widget_data[tw->widget].wd = TEXTWIN_WIDTH_MAX;
        }
        else if (xrel)
        {
            widget_data[tw->widget].x1 += xrel;
            widget_data[tw->widget].wd -= xrel;
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
