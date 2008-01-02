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


#define BOOK_LINE_NORMAL 0
#define BOOK_LINE_TITLE  1
#define BOOK_LINE_ICON  2
#define BOOK_LINE_NAME  4

/* internal used */
#define BOOK_LINE_PAGE  16
_global_book_data global_book_data;

static _gui_book_line *get_page_tag(char *data, int len, int *pos)
{
    char *buf,c;
    static _gui_book_line book_line;

    memset(&book_line, 0 , sizeof(_gui_book_line));
    book_line.mode = BOOK_LINE_PAGE;
    (*pos)++;
    while ((c= *(data+*pos)) != '\0' && c  != 0)
    {
        if (c == '>')
            return &book_line;

        (*pos)++;
        if (c<=' ')
            continue;

        /* check inside tags */
        switch (c)
        {
            case 't':
                if (!(buf = get_parameter_string(data, pos, BOOK_LINES_CHAR+1)))
                    return NULL;
                book_line.mode |= BOOK_LINE_TITLE;
                strncpy(book_line.line, buf,BOOK_LINES_CHAR);
                buf[BOOK_LINES_CHAR]=0;
                break;

            default:
                return NULL;
                break;
        }
    }

    return NULL;
}

static _gui_book_line *get_title_tag(char *data, int len, int *pos)
{
    char *buf,c;
    static _gui_book_line book_line;

    memset(&book_line, 0 , sizeof(_gui_book_line));
    book_line.mode = BOOK_LINE_TITLE;
    (*pos)++;
    while ((c= *(data+*pos)) != '\0' && c  != 0)
    {
        if (c == '>')
            return &book_line;

        (*pos)++;
        if (c<=' ')
            continue;

        /* check inside tags */
        switch (c)
        {
            case 't':
                if (!(buf = get_parameter_string(data, pos, BOOK_LINES_CHAR+1)))
                    return NULL;
                strncpy(book_line.line, buf,BOOK_LINES_CHAR);
                buf[BOOK_LINES_CHAR]=0;
                break;

            default:
                return NULL;
                break;
        }
    }

    return NULL;
}

static _gui_book_line *get_name_tag(char *data, int len, int *pos)
{
    char *buf,c;
    static _gui_book_line book_line;

    memset(&book_line, 0 , sizeof(_gui_book_line));
    book_line.mode = BOOK_LINE_NAME;
    (*pos)++;
    while ((c= *(data+*pos)) != '\0' && c  != 0)
    {
        if (c == '>')
            return &book_line;

        (*pos)++;
        if (c<=' ')
            continue;

        /* check inside tags */
        switch (c)
        {
            case 't':
                if (!(buf = get_parameter_string(data, pos, BOOK_LINES_CHAR+1)))
                    return NULL;
                strncpy(book_line.line, buf,BOOK_LINES_CHAR);
                buf[BOOK_LINES_CHAR]=0;
                break;

            default:
                return NULL;
                break;
        }
    }

    return NULL;
}

//static _gui_book_line *get_icon_tag(char *data, int len, int *pos)
//{
// return NULL;
//}

static _gui_book_line *check_book_tag(char *data, int len, int *pos)
{
    int c;
    _gui_book_line *book_line;

    for (;len>*pos;(*pos)++)
    {
        c = *(data+*pos);

        if (c<=' ')
            continue;

        if (c=='t') /* title tag */
        {
            book_line = get_title_tag(data, len, pos);
            if (!book_line)
                return NULL;
            return book_line;
        }
        else if (c=='i') /* 'icon' (picture) tag */
        {
        }
        else if (c=='p') /* new page */
        {
            book_line = get_page_tag(data, len, pos);
            if (!book_line)
                return NULL;
            return book_line;
        }
        else if (c=='b') /* book name */
        {
            book_line = get_name_tag(data, len, pos);
            if (!book_line)
                return NULL;
            return book_line;
        }
        else
            return NULL;
    }
    return NULL;
}

static void book_link_page(_gui_book_page *page)
{
    _gui_book_page *page_link;

    if (!gui_interface_book)
    {
        gui_interface_book = malloc(sizeof(_gui_book_struct));
        memset(gui_interface_book,0,sizeof(_gui_book_struct));
    }
    if (!gui_interface_book->start)
    {
        gui_interface_book->start=page;
    }
    else
    {
        page_link=gui_interface_book->start;
        for (;page_link->next;page_link=page_link->next)
            ;
        page_link->next = page;
    }
}

/* post formating & initializing of a loaded book */
static void format_book(_gui_book_struct *book, char *name)
{
    int pc=0;
    _gui_book_page *page;

    if (!gui_interface_book)
        return;

    gui_interface_book->page_show = 0;
    strcpy(gui_interface_book->name, name);
    page = gui_interface_book->start;
    while (page)
    {
        pc++;
        page = page->next;
    }

    gui_interface_book->pages = pc;
}


/* free & clear the book gui */
void book_clear(void)
{
    int i;
    _gui_book_page *page_tmp, *page;

    if (!gui_interface_book)
        return;

    page = gui_interface_book->start;

    while (page)
    {
        page_tmp = page->next;
        for (i=0;i<BOOK_PAGE_LINES;i++)
        {
            if (page->line[i])
                free(page->line[i]);
        }
        free(page);
        page=page_tmp;
    }

    free(gui_interface_book);
    gui_interface_book = NULL;
}



_gui_book_struct *load_book_interface(int mode, char *data, int len)
{
    _gui_book_line current_book_line, *book_line;
    int pos=0, lc=0, force_line;
    _gui_book_page current_book_page;
    int plc=0, plc_logic=0;
    char c, name[256]="";

    strcpy(name, "Book");
    book_clear();
    memset(&current_book_page, 0, sizeof(_gui_book_page));
    memset(&current_book_line, 0, sizeof(_gui_book_line));

    for (pos=0;len>pos;pos++)
    {
        c = *(data+pos);

        if (c == 0x0d)
            continue;

        if (c == '<')
        {
            pos++;
            book_line = check_book_tag(data, len, &pos);

            if (!book_line)
            {
                draw_info(data,COLOR_GREEN);
                draw_info("ERROR in book cmd!", COLOR_RED);
                return NULL;
            }

            if (book_line->mode & BOOK_LINE_NAME)
            {
                strcpy(name, book_line->line);
                memset(&current_book_line, 0, sizeof(_gui_book_line));
                continue;
            }

title_repeat_jump:
            if ((book_line->mode & BOOK_LINE_TITLE && plc_logic+2 >= BOOK_PAGE_LINES)
                    || book_line->mode & BOOK_LINE_PAGE)
            {
                _gui_book_page *page = malloc(sizeof(_gui_book_page));

                /* add the page & reset the current one */
                memcpy(page, &current_book_page, sizeof(_gui_book_page));
                book_link_page(page);
                memset(&current_book_page, 0, sizeof(_gui_book_page));
                plc=0;
                plc_logic=0;
            }

            if (book_line->mode & BOOK_LINE_TITLE)
            {
                int l_len;
                _gui_book_line *b_line = malloc(sizeof(_gui_book_line));

                memcpy(b_line, book_line,sizeof(_gui_book_line));
                b_line->mode = BOOK_LINE_TITLE;
                current_book_page.line[plc++] = b_line;
                plc_logic+=1;

                /* lets check we need to break the title line (because its to big) */
                if (StringWidthOffset(&BigFont, b_line->line, &l_len, 186))
                {
                    int i = l_len;
                    b_line->line[l_len]=0;

                    /* now lets go back to a ' ' if we don't find one, we cut the line hard */
                    for (i=l_len;i>=0;i--)
                    {
                        if (b_line->line[i] == ' ')
                        {
                            b_line->line[i]=0; /* thats our real eof */
                            break;
                        }
                    }

                    /* lets see where our real eol is ... */
                    if (i<0) /* is it at i?`*/
                        i = l_len; /* nope, its our "physcial" eol */

                    /* now lets remove all whitespaces.. if we hit EOL, jump back */
                    for (;;i++)
                    {
                        if (book_line->line[i] != ' ')
                            break;
                    }

                    if (strlen(&book_line->line[i]))
                    {
                        memcpy(book_line->line, &book_line->line[i],strlen(&book_line->line[i])+1);
                        goto title_repeat_jump;
                    }
                }
            }
            continue;
        }

        if (c == '>') /* should never happens */
        {
            draw_info(data,COLOR_GREEN);
            draw_info("ERROR in book cmd!", COLOR_RED);
            return NULL;
        }

        /* we have a line */
        if (c== '\0' || c  == 0 || c == 0x0a)
        {
            int l_len;
            _gui_book_line *tmp_line;

            force_line = FALSE;
force_line_jump:
            current_book_line.line[lc]=0;

            book_line = malloc(sizeof(_gui_book_line));
            memcpy(book_line, &current_book_line,sizeof(_gui_book_line));
            current_book_page.line[plc] = book_line;
            tmp_line = current_book_page.line[plc++];
            plc_logic++;
            lc=0;

            if (plc_logic >= BOOK_PAGE_LINES)
            {
                _gui_book_page *page = malloc(sizeof(_gui_book_page));

                /* add the page & reset the current one */
                memcpy(page, &current_book_page, sizeof(_gui_book_page));
                book_link_page(page);
                memset(&current_book_page, 0, sizeof(_gui_book_page));
                plc=0;
                plc_logic=0;
            }

            /* now lets check the last line - if the line is to long, lets adjust it */
            if (StringWidthOffset((tmp_line->mode == BOOK_LINE_TITLE) ?&BigFont:&MediumFont, tmp_line->line, &l_len, 186))
            {
                int i, wspace_flag = TRUE;


                tmp_line->line[l_len]=0; /* bigger can't be the string - current_book_line.line is our backbuffer */
                /* now lets go back to a ' ' if we don't find one, we cut the line hard */
                for (i=l_len;i>=0;i--)
                {
                    if (tmp_line->line[i] == ' ')
                    {
                        tmp_line->line[i]=0; /* thats our real eof */
                        break;
                    }
                    else if (i>0)
                    {
                        if (tmp_line->line[i] == '(' && tmp_line->line[i-1] == ')')
                        {
                            tmp_line->line[i]=0;
                            wspace_flag = FALSE;
                            break;
                        }
                    }
                }
                /* lets see where our real eol is ... */
                if (i<0) /* is it at i?`*/
                    i = l_len; /* nope, its our "physcial" eol */

                /* now lets remove all whitespaces.. if we hit EOL, jump back */
                if (wspace_flag)
                {
                    for (;;i++)
                    {
                        if (current_book_line.line[i] == 0)
                        {
                            if (!force_line) /* thats a real eol */
                            {
                                /* clear input line setting */
                                memset(&current_book_line, 0, sizeof(_gui_book_line));
                            }
                            goto force_line_jump_out;
                        }

                        if (current_book_line.line[i] != ' ')
                            break;
                    }
                }

                memcpy(current_book_line.line, &current_book_line.line[i], strlen(&current_book_line.line[i])+1);
                /* we have a forced linebreak, we go back and load more chars */
                lc = strlen(current_book_line.line);
                if (force_line)
                {
                    //if(StringWidthOffset((tmp_line->mode == BOOK_LINE_TITLE) ?&BigFont:&MediumFont, tmp_line->line, &l_len, 186))
                    if (StringWidth((tmp_line->mode == BOOK_LINE_TITLE) ?& BigFont:&MediumFont, current_book_line.line) < 186)
                    {
                        goto force_line_jump_out;
                    }
                }
                goto force_line_jump;
            }
            memset(&current_book_line, 0, sizeof(_gui_book_line));
force_line_jump_out:
            continue;
        }

        current_book_line.line[lc++]=c;
        if (lc>=BOOK_LINES_CHAR-2)
        {
            force_line = TRUE;
            goto force_line_jump;
        }
    }

    if (plc_logic)
    {
        _gui_book_page *page = malloc(sizeof(_gui_book_page));

        /* add the page & reset the current one */
        memcpy(page, &current_book_page, sizeof(_gui_book_page));
        book_link_page(page);
    }

    format_book(gui_interface_book, name);
    return gui_interface_book;
}

void show_book(int x, int y)
{
    char buf[128];
    SDL_Rect    box;
    int i, ii, yoff;
    _gui_book_page *page1, *page2;

    sprite_blt(Bitmaps[BITMAP_JOURNAL], x, y, NULL, NULL);
    global_book_data.x = x;
    global_book_data.y = y;
    global_book_data.xlen = Bitmaps[BITMAP_JOURNAL]->bitmap->w;
    global_book_data.ylen = Bitmaps[BITMAP_JOURNAL]->bitmap->h;

    add_close_button(x+27, y+2, MENU_BOOK, FALSE);

    if (!gui_interface_book)
        return;

    if (gui_interface_book->name)
        StringBlt( ScreenSurface, &BigFont, gui_interface_book->name , x+global_book_data.xlen/2-
                   get_string_pixel_length(gui_interface_book->name, &BigFont)/2,y+9, COLOR_WHITE, NULL, NULL);

    box.x=x+47;
    box.y=y+72;
    box.w=200;
    box.h=300;

    /* get the 2 pages we show */
    page1 = gui_interface_book->start;
    for (i=0;i!=gui_interface_book->page_show && page1;i++,page1=page1->next)
        ;
    page2=page1->next;

    if (page1)
    {
        sprintf(buf,"Page %d of %d",gui_interface_book->page_show+1,gui_interface_book->pages);
        StringBlt(ScreenSurface, &Font6x3Out, buf, box.x+70, box.y+295, COLOR_WHITE, NULL, NULL);

        SDL_SetClipRect(ScreenSurface, &box);
        /*SDL_FillRect(ScreenSurface, &box, 35325);*/
        for (yoff=0,i=0, ii=0;ii<BOOK_PAGE_LINES;ii++,yoff+=16)
        {
            if (!page1->line[i])
                break;
            if (page1->line[i]->mode == BOOK_LINE_NORMAL)
            {
                StringBlt(ScreenSurface, &MediumFont, page1->line[i]->line , box.x+2, box.y+2+yoff, COLOR_BLACK, NULL, NULL);
            }
            else if (page1->line[i]->mode == BOOK_LINE_TITLE)
            {
                StringBlt(ScreenSurface, &BigFont, page1->line[i]->line, box.x+2, box.y+2+yoff, COLOR_DBROWN, NULL, NULL);
            }
            i++;
        }
        SDL_SetClipRect(ScreenSurface, NULL);
    }


    box.x=x+280;
    box.y=y+72;
    box.w=200;
    box.h=300;

    if (gui_interface_book->pages)
    {
        sprintf(buf,"%c and %c to turn page",ASCII_RIGHT, ASCII_LEFT);
        StringBlt(ScreenSurface, &Font6x3Out, buf, box.x-59, box.y+300, COLOR_GREEN, NULL, NULL);
    }

    if (page2)
    {
        sprintf(buf,"Page %d of %d",gui_interface_book->page_show+2,gui_interface_book->pages);
        StringBlt(ScreenSurface, &Font6x3Out, buf, box.x+76, box.y+295, COLOR_WHITE, NULL, NULL);

        SDL_SetClipRect(ScreenSurface, &box);
        /*SDL_FillRect(ScreenSurface, &box, 35325);*/
        for (yoff=0,i=0, ii=0;ii<BOOK_PAGE_LINES;ii++,yoff+=16)
        {
            if (!page2->line[i])
                break;
            if (page2->line[i]->mode == BOOK_LINE_NORMAL)
            {
                StringBlt(ScreenSurface, &MediumFont, page2->line[i]->line , box.x+2, box.y+2+yoff, COLOR_BLACK, NULL, NULL);
            }
            else if (page2->line[i]->mode == BOOK_LINE_TITLE)
            {
                StringBlt(ScreenSurface, &BigFont, page2->line[i]->line, box.x+2, box.y+2+yoff, COLOR_DBROWN, NULL, NULL);
            }
            i++;
        }
        SDL_SetClipRect(ScreenSurface, NULL);
    }
}

