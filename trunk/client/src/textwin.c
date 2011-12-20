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

/* this depends on the text win png*/
#define TEXT_WIN_XLEN 265
#define TEXT_WIN_YLEN 190

int             textwin_flags;
int             txtwin_start_size = 0;
_textwin_set    txtwin[TW_SUM];
static int      old_slider_pos  = 0;
SDL_Surface     *txtwinbg = NULL;
int             old_txtwin_alpha = -1;

/* TODO: All this to be removed, please ignore it. Hypertext (keywords) ONLY
 * available in NPC and book GUIs.
 * -- Smacky 20100711 */
/******************************************************************
  definition of keyword:
    a keyword is the text between two '^' chars.
    the size can vary between a single word and a complete sentence.
    the max length of a keyword is 1 row (inclusive '^') because only
    one LF within is allowed.
******************************************************************/

/******************************************************************
 returns the startpos of a keyword(-part).
******************************************************************/
static char * get_keyword_start(int actWin, int mouseX, int *row)
{
    int     pos, pos2 = 538, key_start;
    char   *text;

    if (actWin >= TW_SUM)
        return 0;

    pos = (txtwin[actWin].top_drawLine + (*row)) % TEXT_WIN_MAX - txtwin[actWin].scroll;
    if (pos < 0)
        pos += TEXT_WIN_MAX;
    if (mouseX < 0)
        goto row2; /* check only for 2. half of keyword */
    /* check if keyword starts in the row before */
    if (txtwin[actWin].scroll + 1 != txtwin[actWin].act_bufsize - txtwin[actWin].size /* dont check in first row */
            && txtwin[actWin].text[pos].key_clipped)
    {
        /* was a clipped keyword clicked? */
        int index   = -1;
        text = txtwin[actWin].text[pos].buf;
        while (text[++index] && pos2 <= mouseX && text[index] != '^')
            pos2 += font_small.c[(int) text[index]].w + font_small.char_offset;
        if (text[index] != '^')
        {
            /* clipped keyword was clicked, so we must start one row before */
            /* TODO not start one row before if its the first row in buffer ! */
            (*row)--;
            pos = (txtwin[actWin].top_drawLine + (*row)) % TEXT_WIN_MAX - txtwin[actWin].scroll;
            if (pos < 0)
                pos += TEXT_WIN_MAX;
            mouseX = 800; /* detect last keyword in this row */
        }
    }

row2 : text = txtwin[actWin].text[pos].buf;
    /* find the first char of the keyword */
    if (txtwin[actWin].text[pos].key_clipped)
        key_start = 0;
    else
        key_start = -1;
    pos = 0; pos2 = 538;
    while (text[pos] && pos2 <= mouseX)
    {
        if (text[pos++] == '^')
        {
            if (key_start < 0)
                key_start = pos; /* start of a key */
            else
                key_start = -1; /* end of a key */
            continue;
        }
        pos2 += font_small.c[(int) text[pos]].w + font_small.char_offset;
    }
    if (key_start < 0)
        return NULL; /* no keyword here */
    (*row)++;
    return &text[key_start];
}

/******************************************************************
 check for keyword and send it to server.
******************************************************************/
void say_clickedKeyword(int actWin, int mouseX, int mouseY)
{
    char    cmdBuf[MAX_KEYWORD_LEN + 1]     =
        {
            "/say "
        };
    char    cmdBuf2[MAX_KEYWORD_LEN + 1]    =
        {
            ""
        };
    char   *text;
    int     clicked_row, pos = 5;

    clicked_row = (mouseY - 588) / 10 + txtwin[actWin].size;
    text = get_keyword_start(actWin, mouseX, &clicked_row);
    if (text == NULL)
        return;
    while (*text && *text != '^')
        cmdBuf[pos++] = *text++;
    if (*text != '^')
    {
        text = get_keyword_start(actWin, -1, &clicked_row);
        if (text != NULL)
            while (*text && *text != '^')
                cmdBuf[pos++] = *text++;
    }
    cmdBuf[pos++] = '\0';

    /* Grunt: fix for clickable keywords that
     * can be commands too. Commands will
     * get executed instead of /say'ed
     */

    if (cmdBuf[5] == '/') // get rid of /say
    {
        pos = 5;
        while (cmdBuf[pos] != '\0')
        {
            cmdBuf2[pos - 5] = cmdBuf[pos];
            pos++;
        }
        cmdBuf2[pos - 5] = '\0';
        client_cmd_generic(cmdBuf2);
    }
    else
        client_cmd_generic(cmdBuf);
}

/******************************************************************
 clear the screen of a text-window.
******************************************************************/
void textwin_init()
{
    int i;

    for (i = TW_MIX; i < TW_SUM; i++)
    {
        txtwin[i].bot_drawLine = 0;
        txtwin[i].act_bufsize = 0;
        txtwin[i].scroll = 0;
        txtwin[i].size = 9;
    }
}

/******************************************************************
 add string to the text-window (perform auto-clipping).
******************************************************************/
void textwin_showstring(uint32 flags, uint32 colr, char *format, ...)
{
    va_list ap;
    static int  key_start   = 0;
    static int  key_count   = 0;
    int         i, len, a;
    int         winlen      = 244;
    char        buf[HUGE_BUF];
    char       *text;
    int         actWin, z;
    /* Hmm I'm not 100& sure, if this is the best place for that */
    char *buf2;
    char        *enemy1;
    char        enemy2[MEDIUM_BUF];
    int         newkill=0;
    struct kills_list *node=NULL;
    int tempexp;
    int tempexp2;

    va_start(ap, format);
    vsprintf(buf, format, ap);
    va_end(ap);

    if (options.statsupdate)
    {
        tempexp=0;
        tempexp2=0;
        if (sscanf(buf,"You got %d exp in skill",&tempexp)!=EOF)
        {
            statometer.exp+=tempexp;
        }
        else if (sscanf(buf,"You got %d (+%d) exp in skill",&tempexp, &tempexp2)!=EOF)
        {
            statometer.exp+=tempexp;
        }
    }

    if (options.statsupdate && !strncmp(buf,"You killed ",11))
        statometer.kills++;

    if (options.kerbholz)
    {
        if (!strncmp(buf,"You killed ",11))
        {
            enemy1=strstr(buf, " with ");
            if (enemy1!=0)
            {
                strncpy(enemy2,buf+11,enemy1-(buf+11));
                enemy2[enemy1-(buf+11)]=0;
            } else {
                strncpy(enemy2,buf+11,(strlen(buf+11)-1));
                enemy2[strlen(buf+11)-1]=0;
            }
            newkill=addKill(enemy2);

            if (newkill>0)
            {
                if (newkill==1)
                {
                    strcat(buf, " for the first time!");
                    colr = NDI_COLR_LIME;
                }
                else
                {
                    node=getKillEntry(enemy2);

                    if (node)
                    {
                        sprintf(strchr(buf, '\0'), " %d/%d times.",
                                node->session, node->kills);
                    }
                }
            }
        }
    }

    /* Create a modifiable version of buf */
    /* FIXME: Basically unnecessary. */
    MALLOC_STRING(buf2, buf);

    /*
     * first: we set all white spaces (char<32) to 32 to remove really all odd stuff.
     * except 0x0a - this is EOL for us and will be set to
     * 0 to mark C style end of string
     */
    for (i = 0; buf2[i] != 0; i++)
    {
        if ((unsigned char)buf2[i] < 32 && buf2[i] != 0x0a)
        {
            buf2[i] = 32;
        }
    }

#ifdef DAI_DEVELOPMENT
    LOG(LOG_MSG,">%s<\n", buf2);
#endif

    /*
     * ok, here we must cut a string to make it fit in window
     * for it we must add the char length
     * we assume this standard font in the windows...
     */
    len = 0;
    for (a = i = 0; ; i++)
    {

        if (buf2[i] != '^')
            len += font_small.c[(uint8) (buf2[i])].w + font_small.char_offset;

        if (len >= winlen || buf2[i] == 0x0a || buf2[i] == 0)
        {
            /*
                        if(buf2[i]==0 && !a)
                            break;
                     */

            /* now the special part - lets look for a good point to cut */
            if (len >= winlen && a > 10)
            {
                int ii =a, it = i, ix = a, tx = i;

                while (ii >= a / 2)
                {
                    if (buf2[it] == ' '
                            || buf2[it] == ':'
                            || buf2[it] == '.'
                            || buf2[it] == ','
                            || buf2[it] == '('
                            || buf2[it] == ';'
                            || buf2[it] == '-'
                            || buf2[it] == '+'
                            || buf2[it] == '*'
                            || buf2[it] == '?'
                            || buf2[it] == '/'
                            || buf2[it] == '='
                            || buf2[it] == '.'
                            || buf2[it] == 0
                            || buf2[it] == 0x0a)
                    {
                        tx = it;
                        ix = ii;
                        break;
                    }
                    it--;
                    ii--;
                };
                i = tx;
                a = ix;
            }
            buf[a] = 0;

            actWin = TW_MIX;
            for (z = 0; z < 2; z++)
            {
                /* add messages to mixed-textwin and either to msg OR chat-textwin */
                strcpy(txtwin[actWin].text[txtwin[actWin].bot_drawLine % TEXT_WIN_MAX].buf, buf);
                txtwin[actWin].text[txtwin[actWin].bot_drawLine % TEXT_WIN_MAX].colr = colr;
                txtwin[actWin].text[txtwin[actWin].bot_drawLine % TEXT_WIN_MAX].flags = flags;
                txtwin[actWin].text[txtwin[actWin].bot_drawLine % TEXT_WIN_MAX].key_clipped = key_start;
                if (txtwin[actWin].scroll)
                    txtwin[actWin].scroll++;
                if (txtwin[actWin].act_bufsize < TEXT_WIN_MAX)
                    txtwin[actWin].act_bufsize++;
                txtwin[actWin].bot_drawLine++;
                txtwin[actWin].bot_drawLine %= TEXT_WIN_MAX;
                actWin++; /* next window => MSG_WIN */
                if ((flags & NDI_FLAG_PLAYER))
                {
                    actWin++; /* next window => MSG_CHAT */
                    WIDGET_REDRAW(WIDGET_CHATWIN_ID) = 1;
                }
                else
                    WIDGET_REDRAW(WIDGET_MSGWIN_ID) = 1;
            }

            /* hack: because of autoclip we must scan every line again */
            for (text = buf; *text; text++)
                if (*text == '^')
                    key_count = (key_count + 1) & 1;
            if (key_count)
                key_start = 0x1000;
            else
                key_start = 0;

            a = len = 0;
            if (buf2[i] == 0)
                break;
        }
        if (buf2[i] != 0x0a)
            buf[a++] = buf2[i];
    }
    FREE(buf2);
}

/******************************************************************
 draw a text-window.
******************************************************************/
static void show_window(int actWin, int x, int y, _BLTFX *bltfx)
{
    int i, temp;

    txtwin[actWin].x = x;
    txtwin[actWin].y = y;

    if (actWin!=TW_MIX)
    {
        x = y = 0;
    }

    txtwin[actWin].top_drawLine = txtwin[actWin].bot_drawLine - (txtwin[actWin].size + 1);
    if (txtwin[actWin].top_drawLine < 0 && txtwin[actWin].act_bufsize == TEXT_WIN_MAX)
        txtwin[actWin].top_drawLine += TEXT_WIN_MAX;
    else if (txtwin[actWin].top_drawLine < 0)
        txtwin[actWin].top_drawLine = 0;
    if (txtwin[actWin].scroll > txtwin[actWin].act_bufsize - (txtwin[actWin].size + 1))
        txtwin[actWin].scroll = txtwin[actWin].act_bufsize - (txtwin[actWin].size + 1);
    if (txtwin[actWin].scroll < 0)
        txtwin[actWin].scroll = 0;

    for (i = 0; i <= txtwin[actWin].size && i < txtwin[actWin].act_bufsize; i++)
    {
        temp = (txtwin[actWin].top_drawLine + i) % TEXT_WIN_MAX;
        if (txtwin[actWin].act_bufsize > txtwin[actWin].size)
        {
            temp -= txtwin[actWin].scroll;
            if (temp < 0)
                temp = TEXT_WIN_MAX + temp;
        }
        string_blt(bltfx->surface, &font_small, &txtwin[actWin].text[temp].buf[0], x + 2,
                  (y + 1 + i * 10) | txtwin[actWin].text[temp].key_clipped, txtwin[actWin].text[temp].colr, NULL, NULL);
    }

    /* only draw scrollbar if needed */
    if (txtwin[actWin].act_bufsize > txtwin[actWin].size)
    {
        SDL_Rect    box;

        box.x = box.y = 0;
        box.w = skin_sprites[SKIN_SPRITE_SLIDER]->bitmap->w;
        box.h = txtwin[actWin].size * 10 + 1;
//        if (actWin == TW_CHAT)
            temp = -9; /* no textinput-line */
//        else
//            temp = 0;
        sprite_blt(skin_sprites[SKIN_SPRITE_SLIDER_UP], x + 250, y + 2, NULL, bltfx);
        sprite_blt(skin_sprites[SKIN_SPRITE_SLIDER_DOWN], x + 250, y + 13 + temp + txtwin[actWin].size * 10, NULL, bltfx);
        sprite_blt(skin_sprites[SKIN_SPRITE_SLIDER], x + 250, y + skin_sprites[SKIN_SPRITE_SLIDER_UP]->bitmap->h + 2 + temp, &box, bltfx);
        box.h += temp - 2;
        box.w -= 2;

        txtwin[actWin].slider_y = ((txtwin[actWin].act_bufsize - (txtwin[actWin].size + 1) - txtwin[actWin].scroll) * box.h)
                                  / txtwin[actWin].act_bufsize;
        txtwin[actWin].slider_h = (box.h * (txtwin[actWin].size + 1)) / txtwin[actWin].act_bufsize; /* between 0.0 <-> 1.0 */
        if (txtwin[actWin].slider_h < 1)
            txtwin[actWin].slider_h = 1;

        if (!txtwin[actWin].scroll && txtwin[actWin].slider_y + txtwin[actWin].slider_h < box.h)
            txtwin[actWin].slider_y++;

        box.h = txtwin[actWin].slider_h;
        sprite_blt(skin_sprites[SKIN_SPRITE_TWIN_SCROLL], x + 252,
                   y + skin_sprites[SKIN_SPRITE_SLIDER_UP]->bitmap->h + 3 + txtwin[actWin].slider_y, &box, bltfx);

        if (txtwin[actWin].highlight == TW_HL_UP)
        {
            box.x = x + 250;
            box.y = y + 2;
            box.h = skin_sprites[SKIN_SPRITE_SLIDER_UP]->bitmap->h;
            box.w = 1;
            SDL_FillRect(bltfx->surface, &box, -1);
            box.x += skin_sprites[SKIN_SPRITE_SLIDER_UP]->bitmap->w - 1;
            SDL_FillRect(bltfx->surface, &box, -1);
            box.w = skin_sprites[SKIN_SPRITE_SLIDER_UP]->bitmap->w - 1;
            box.h = 1;
            box.x = x + 250;
            SDL_FillRect(bltfx->surface, &box, -1);
            box.y += skin_sprites[SKIN_SPRITE_SLIDER_UP]->bitmap->h - 1;
            SDL_FillRect(bltfx->surface, &box, -1);
        }
        else if (txtwin[actWin].highlight == TW_ABOVE)
        {
            box.x = x + 252;
            box.y = y + skin_sprites[SKIN_SPRITE_SLIDER_UP]->bitmap->h + 2;
            box.h = txtwin[actWin].slider_y + 1;
            box.w = 5;
            SDL_FillRect(bltfx->surface, &box, 0);
        }
        else if (txtwin[actWin].highlight == TW_HL_SLIDER)
        {
            box.x = x + 252;
            box.y = y + skin_sprites[SKIN_SPRITE_SLIDER_UP]->bitmap->h + 3 + txtwin[actWin].slider_y;
            box.w = 1;
            SDL_FillRect(bltfx->surface, &box, -1);
            box.x += 4;
            SDL_FillRect(bltfx->surface, &box, -1);
            box.x -= 4;
            box.h = 1;
            box.w = 4;
            SDL_FillRect(bltfx->surface, &box, -1);
            box.y += txtwin[actWin].slider_h - 1;
            SDL_FillRect(bltfx->surface, &box, -1);
        }
        else if (txtwin[actWin].highlight == TW_UNDER)
        {
            box.x = x + 252;
            box.y = y + skin_sprites[SKIN_SPRITE_SLIDER_UP]->bitmap->h + 3 + txtwin[actWin].slider_y + box.h;
            box.h = txtwin[actWin].size * 10 - txtwin[actWin].slider_y - txtwin[actWin].slider_h - 10;
            box.w = 5;
            SDL_FillRect(bltfx->surface, &box, 0);
        }
        else if (txtwin[actWin].highlight == TW_HL_DOWN)
        {
            box.x = x + 250;
            box.y = y + txtwin[actWin].size * 10 + 4;
            box.h = skin_sprites[SKIN_SPRITE_SLIDER_UP]->bitmap->h;
            box.w = 1;
            SDL_FillRect(bltfx->surface, &box, -1);
            box.x += skin_sprites[SKIN_SPRITE_SLIDER_UP]->bitmap->w - 1;
            SDL_FillRect(bltfx->surface, &box, -1);
            box.w = skin_sprites[SKIN_SPRITE_SLIDER_UP]->bitmap->w - 1;
            box.h = 1;
            box.x = x + 250;
            SDL_FillRect(bltfx->surface, &box, -1);
            box.y += skin_sprites[SKIN_SPRITE_SLIDER_UP]->bitmap->h - 1;
            SDL_FillRect(bltfx->surface, &box, -1);
        }
    }
    else
        txtwin[actWin].slider_h = 0;
}

/******************************************************************
 display all text-windows - used now only at startup
******************************************************************/
void textwin_show(int x, int y)
{
    int         len;
    SDL_Rect    box;
    _BLTFX      bltfx;
    bltfx.alpha = options.textwin_alpha;
    bltfx.flags = BLTFX_FLAG_SRCALPHA;
    bltfx.surface = NULL;
    box.x = box.y = 0;
    box.w = skin_sprites[SKIN_SPRITE_TEXTWIN]->bitmap->w;
    y = 599; /* to lazy to work with correct calcs */


    box.h = len = txtwin[TW_MIX].size * 10 + 23;
    y -= len;
    if (options.use_TextwinAlpha)
    {
        sprite_blt(skin_sprites[SKIN_SPRITE_TEXTWIN_MASK], x, y, &box, &bltfx);
    }
    else
        sprite_blt(skin_sprites[SKIN_SPRITE_TEXTWIN], x, y, &box, NULL);

    bltfx.alpha=255;
    bltfx.surface = ScreenSurface;
    bltfx.flags=0;
    show_window(TW_MIX, x, y - 1, &bltfx);


}

/******************************************************************
 display widget text-wins
******************************************************************/
void widget_textwin_show(int x, int y, int actWin)
{
    int         len, wID = 0;
    SDL_Rect    box, box2;
    _BLTFX      bltfx;

    if (actWin==TW_CHAT)
        wID=WIDGET_CHATWIN_ID;
    else if (actWin==TW_MSG)
        wID=WIDGET_MSGWIN_ID;

    box.x = box.y = 0;
    box.w = skin_sprites[SKIN_SPRITE_TEXTWIN]->bitmap->w;
    box.h = len = txtwin[actWin].size * 10 + 13;

    /* backbuffering is a bit trickier
     * we always blit the background extra because of the alpha,
     * for performance reasons we need to do a DisplayFormat (its around 4ms !!! faster on my system)
     * and keep track of the old txtwin_alpha value
     * We also need it to seperate because drawing and esp Blitting of Fonts with palettes to a DisplayFormatted
     * (hardware) surface is slow as hell (i got sometimes 10 seconds (!not milli!) for the simple textdrawings
     * (ok that could also my integratet on board graphics with bad hardware blitting support)
     */

    if (options.use_TextwinAlpha)
    {
        if (old_txtwin_alpha!=options.textwin_alpha)
        {
            if (txtwinbg)
                SDL_FreeSurface(txtwinbg);

            SDL_SetAlpha(skin_sprites[SKIN_SPRITE_TEXTWIN_MASK]->bitmap, SDL_SRCALPHA|SDL_RLEACCEL, options.textwin_alpha);
            txtwinbg=SDL_DisplayFormatAlpha(skin_sprites[SKIN_SPRITE_TEXTWIN_MASK]->bitmap);
            SDL_SetAlpha(skin_sprites[SKIN_SPRITE_TEXTWIN_MASK]->bitmap, SDL_SRCALPHA|SDL_RLEACCEL, 255);

            old_txtwin_alpha=options.textwin_alpha;
        }
        box2.x = x;
        box2.y = y;
        SDL_BlitSurface(txtwinbg, &box, ScreenSurface, &box2);
    }
    else
    {
        sprite_blt(skin_sprites[SKIN_SPRITE_TEXTWIN], x, y, &box, NULL);
    }
    /* if we don't have a backbuffer, create it */
    if (!widget_surface[wID])
    {
        widget_surface[wID]=SDL_ConvertSurface(skin_sprites[SKIN_SPRITE_TEXTWIN_MASK]->bitmap,
                                         skin_sprites[SKIN_SPRITE_TEXTWIN_MASK]->bitmap->format,
                                         skin_sprites[SKIN_SPRITE_TEXTWIN_MASK]->bitmap->flags);
        SDL_SetColorKey(widget_surface[wID], SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB(widget_surface[wID]->format, 0, 0, 0));
    }

    /* lets draw the widgets in the backbuffer */
    if (widget_data[wID].redraw)
    {

        widget_data[wID].redraw=0;

        SDL_FillRect(widget_surface[wID],NULL, SDL_MapRGBA(widget_surface[wID]->format, 0, 0, 0, options.textwin_alpha));

        bltfx.surface=widget_surface[wID];
        bltfx.flags = 0;
        bltfx.alpha = 0;
        if (options.use_TextwinAlpha)
        {
            box.x = 0;
            box.y = 0;
            box.h = 1;
            box.w = skin_sprites[SKIN_SPRITE_TEXTWIN]->bitmap->w;
            SDL_FillRect(widget_surface[wID], &box, SDL_MapRGBA(widget_surface[wID]->format, 0x60, 0x60, 0x60, 255));
            box.y = len;
            box.h = 1;
            box.x = 0;
            box.w = skin_sprites[SKIN_SPRITE_TEXTWIN]->bitmap->w;
            SDL_FillRect(widget_surface[wID], &box, SDL_MapRGBA(widget_surface[wID]->format, 0x60, 0x60, 0x60, 255));
            box.w = skin_sprites[SKIN_SPRITE_TEXTWIN]->bitmap->w;
            box.x = box.w-1;
            box.w = 1;
            box.y = 0;
            box.h = len;
            SDL_FillRect(widget_surface[wID], &box, SDL_MapRGBA(widget_surface[wID]->format, 0x60, 0x60, 0x60, 255));
            box.x = 0;
            box.y = 0;
            box.h = len;
            box.w = 1;
            SDL_FillRect(widget_surface[wID], &box, SDL_MapRGBA(widget_surface[wID]->format, 0x60, 0x60, 0x60, 255));
        }
        show_window(actWin, x, y - 2, &bltfx);

    }
    box.x=x;
    box.y=y;
    box2.x=0;
    box2.y=0;
    box2.w = skin_sprites[SKIN_SPRITE_TEXTWIN]->bitmap->w;
    box2.h = len = txtwin[actWin].size * 10 + 14;

    SDL_BlitSurface(widget_surface[wID], &box2, ScreenSurface, &box);
}


/******************************************************************
 mouse-button event in the textwindow.
*****************************************************************/
void textwin_button_event(int actWin, widget_id_t id, SDL_Event *event)
{
    if (event->motion.x < widget_data[id].x1 || (textwin_flags & (TW_SCROLL | TW_RESIZE) && (textwin_flags & actWin))) /* scrolling || resising */
        return;

    WIDGET_REDRAW(id) = 1;

    if (event->button.button == 4) /* mousewheel up */
        txtwin[actWin].scroll++;
    else if (event->button.button == 5) /* mousewheel down */
        txtwin[actWin].scroll--;
    else if (event->button.button == SDL_BUTTON_LEFT)
    {
        if (txtwin[actWin].highlight == TW_HL_UP) /* clicked scroller-button up */
            txtwin[actWin].scroll++;
        else if (txtwin[actWin].highlight == TW_ABOVE) /* clicked above the slider */
            txtwin[actWin].scroll += txtwin[actWin].size;
        else if (txtwin[actWin].highlight == TW_HL_SLIDER)
        {
            /* clicked on the slider */
            textwin_flags |= (actWin | TW_SCROLL);
            old_slider_pos = event->motion.y - txtwin[actWin].slider_y;
        }
        else if (txtwin[actWin].highlight == TW_UNDER) /* clicked under the slider */
            txtwin[actWin].scroll -= txtwin[actWin].size;
        else if (txtwin[actWin].highlight == TW_HL_DOWN) /* clicked scroller-button down */
            txtwin[actWin].scroll--;
        else if (event->motion.x<widget_data[id].x1+246
                 && event->motion.y>widget_data[id].y1 + 2
                 && event->motion.y < widget_data[id].y1 + 7
                 && cursor_type == 1)
            textwin_flags |= (actWin | TW_RESIZE);      /* size-change */
        else if (event->motion.x < widget_data[id].x1 + 250)
            say_clickedKeyword(actWin, event->motion.x, event->motion.y);
    }
}

/******************************************************************
 mouse-move event in the textwindow.
*****************************************************************/
int textwin_move_event(int actWin, widget_id_t id, SDL_Event *event)
{
    txtwin[actWin].highlight = TW_HL_NONE;
    WIDGET_REDRAW(id) = 1;

    /* show resize-cursor */
    if ((event->motion.y > widget_data[id].y1 + 2 && event->motion.y < widget_data[id].y1 + 7 && (event->motion.x < widget_data[id].x1+246))
            || (event->button.button == SDL_BUTTON_LEFT && (textwin_flags & (TW_SCROLL | TW_RESIZE))))
    {
        if (!(textwin_flags & TW_SCROLL) && event->motion.x > widget_data[id].x1)
            cursor_type = 1;
    }
    else
    {
        cursor_type = 0;
        textwin_flags &= ~(TW_ACTWIN | TW_SCROLL | TW_RESIZE);
    }

    /* mouse out of window */
    /* we have to leave this here!!! for sanity, also the widgetstuff does some area checking */
    if (event->motion.y < widget_data[id].y1
            || event->motion.x > widget_data[id].x1 + skin_sprites[SKIN_SPRITE_TEXTWIN]->bitmap->w
            || event->motion.y > widget_data[id].y1 + txtwin[actWin].size * 10 + 13 + skin_sprites[SKIN_SPRITE_SLIDER_UP]->bitmap->h)
    {
        if (!(textwin_flags & TW_RESIZE))
            return 1;
    }

    /* highlighting */
    if (event->motion.x > widget_data[id].x1 + 250
            && event->motion.y > widget_data[id].y1
            && event->button.button != SDL_BUTTON_LEFT
            && event->motion.x < widget_data[id].x1 + skin_sprites[SKIN_SPRITE_TEXTWIN]->bitmap->w)
    {
#define OFFSET (txtwin[actWin].y + skin_sprites[SKIN_SPRITE_SLIDER_UP]->bitmap->h)
        if (event->motion.y < OFFSET)
            txtwin[actWin].highlight = TW_HL_UP;
        else if (event->motion.y < OFFSET + txtwin[actWin].slider_y)
            txtwin[actWin].highlight = TW_ABOVE;
        else if (event->motion.y < OFFSET + txtwin[actWin].slider_y + txtwin[actWin].slider_h + 3)
            txtwin[actWin].highlight = TW_HL_SLIDER;
        else if (event->motion.y < widget_data[id].y1 + txtwin[actWin].size * 10 + 4)
            txtwin[actWin].highlight = TW_UNDER;
        else if (event->motion.y < widget_data[id].y1 + txtwin[actWin].size * 10 + 13)
            txtwin[actWin].highlight = TW_HL_DOWN;
#undef OFFSET
        return 0;
    }

    /* slider scrolling */
    if (textwin_flags & TW_SCROLL)
    {
        actWin = textwin_flags & TW_ACTWIN;
        txtwin[actWin].slider_y = event->motion.y - old_slider_pos;
        txtwin[actWin].scroll = txtwin[actWin].act_bufsize
                                - (txtwin[actWin].size + 1)
                                - (txtwin[actWin].act_bufsize * txtwin[actWin].slider_y)
                                / (txtwin[actWin].size * 10 - 1);
        WIDGET_REDRAW(id) = 1;
        return 0;
    }

    /* resizing */
    if (textwin_flags & TW_RESIZE)
    {
        actWin = textwin_flags & TW_ACTWIN;
        if (actWin == TW_CHAT)
        {
            int newsize;
            newsize = ((widget_data[WIDGET_CHATWIN_ID].y1+widget_data[WIDGET_CHATWIN_ID].ht)-event->motion.y) / 10;
            if (newsize < 3)
                newsize = 3;
            /* we need to calc thenew x for the widget, and set the new size */
            widget_data[WIDGET_CHATWIN_ID].y1+=(txtwin[actWin].size-newsize)*10;
            widget_data[WIDGET_CHATWIN_ID].ht=newsize*10+13;
            txtwin[actWin].size=newsize;
        }
        else if (actWin == TW_MSG)
        {
            int newsize;
            newsize = ((widget_data[WIDGET_MSGWIN_ID].y1+widget_data[WIDGET_MSGWIN_ID].ht)-event->motion.y) / 10;
            if (newsize < 9)
                newsize = 9;
            /* we need to calc thenew x for the widget, and set the new size */
            widget_data[WIDGET_MSGWIN_ID].y1+=(txtwin[actWin].size-newsize)*10;
            widget_data[WIDGET_MSGWIN_ID].ht=newsize*10+13;
            txtwin[actWin].size=newsize;
        }
        else
        {
            int newsize;
            newsize = ((widget_data[WIDGET_MIXWIN_ID].y1+widget_data[WIDGET_MIXWIN_ID].ht)-event->motion.y) / 10;
            if (newsize < 9)
                newsize = 9;
            /* we need to calc thenew x for the widget, and set the new size */
            widget_data[WIDGET_MIXWIN_ID].y1+=(txtwin[actWin].size-newsize)*10;
            widget_data[WIDGET_MIXWIN_ID].ht=newsize*10+13;
            txtwin[actWin].size=newsize;
        }
    }
    return 0;
}

void textwin_addhistory(char *text)
{
    register int i;

    /* If new line is empty or identical to last inserted one, skip it */
    if (!text[0] || strcmp(InputHistory[1], text) == 0)
        return;

    for (i = MAX_HISTORY_LINES - 1; i > 1; i--) /* shift history lines */
    {
        strncpy(InputHistory[i], InputHistory[i - 1], MAX_INPUT_STRING);
    }

    strncpy(InputHistory[1], text, MAX_INPUT_STRING); /* insert new one */
    *InputHistory[0] = 0; /* clear tmp editing line */
    HistoryPos = 0;
}

void textwin_clearhistory()
{
    register int i;
    for (i = 0; i < MAX_HISTORY_LINES; i++)
    {
        InputHistory[i][0] = 0; /* it's enough to clear only the first byte of each history line */
    }
    HistoryPos = 0;
}

void textwin_putstring(char *text)
{
    int len;

    len = strlen(text);
    strncpy(InputString, text, MAX_INPUT_STRING); /* copy buf to input buffer */
    CurrentCursorPos = InputCount = len;           /* set cursor after inserted text */
}
