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


#define BOOK_LINE_NORMAL	0
#define BOOK_LINE_TITLE		1
#define BOOK_LINE_ICON		2

/* internal used */
#define BOOK_LINE_PAGE		16

static _gui_book_line *get_page_tag(char *data, int len, int *pos)
{
	char *buf,c;
	static _gui_book_line book_line;

	memset(&book_line, 0 , sizeof(_gui_book_line));
	book_line.mode = BOOK_LINE_PAGE;
	(*pos)++;
	while((c= *(data+*pos)) != '\0' && c  != 0)
	{
		if(c == '>')
			return &book_line;

		(*pos)++;
		if(c<=' ')
			continue;

		/* check inside tags */
		switch(c)
		{
		case 't':
			if(!(buf = get_parameter_string(data, pos)))
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
	while((c= *(data+*pos)) != '\0' && c  != 0)
	{
		if(c == '>')
			return &book_line;

		(*pos)++;
		if(c<=' ')
			continue;

		/* check inside tags */
		switch(c)
		{
			case 't':
				if(!(buf = get_parameter_string(data, pos)))
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

static _gui_book_line *get_icon_tag(char *data, int len, int *pos)
{
	return NULL;
}

static _gui_book_line *check_book_tag(char *data, int len, int *pos)
{
	int c;
	_gui_book_line *book_line;

	for(;len>*pos;(*pos)++)
	{
		c = *(data+*pos);

		if(c<=' ')
			continue;

		if(c=='t') /* title tag */
		{
			book_line = get_title_tag(data, len, pos);
			if(!book_line)
				return NULL;
			return book_line;
		}
		else if(c=='i') /* 'icon' (picture) tag */
		{
		}
		else if(c=='p') /* new page */
		{
			book_line = get_page_tag(data, len, pos);
			if(!book_line)
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

	if(!gui_interface_book)
	{
		gui_interface_book = malloc(sizeof(_gui_book_struct));
		memset(gui_interface_book,0,sizeof(_gui_book_struct));
	}
	if(!gui_interface_book->start)
	{
		gui_interface_book->start=page;
	}
	else
	{
		page_link=gui_interface_book->start;
		for(;page_link->next;page_link=page_link->next)
			;
		page_link->next = page;
	}
}

/* post formating & initializing of a loaded book */
static void format_book(_gui_book_struct *book)
{
	int pc=0;
	_gui_book_page *page;

	if(!gui_interface_book)
		return;

	gui_interface_book->page_show = 0;

	page = gui_interface_book->start;
	while(page)
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

	if(!gui_interface_book)
		return;

	page = gui_interface_book->start;

	while(page)
	{
		page_tmp = page->next;
		for(i=0;i<BOOK_PAGE_LINES;i++)
		{
			if(page->line[i])
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
	int pos=0, lc=0;
	_gui_book_page current_book_page;
	int plc=0, plc_logic=0;
	char c;

	book_clear();
	memset(&current_book_page, 0, sizeof(_gui_book_page));
	memset(&current_book_line, 0, sizeof(_gui_book_line));

	for(pos=0;len>pos;pos++)
	{
		c = *(data+pos);

		if(c == 0x0d)
			continue;

		if(c == '<')
		{
			pos++;
			book_line = check_book_tag(data, len, &pos);

			if(!book_line)
			{
				draw_info(data,COLOR_GREEN);
				draw_info("ERROR in book cmd!", COLOR_RED);
				return NULL;
			}

			if((book_line->mode & BOOK_LINE_TITLE && plc_logic+2 >= BOOK_PAGE_LINES)
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

			if(book_line->mode & BOOK_LINE_TITLE)
			{
				_gui_book_line *b_line = malloc(sizeof(_gui_book_line));
				memcpy(b_line, book_line,sizeof(_gui_book_line));
				b_line->mode = BOOK_LINE_TITLE;
				current_book_page.line[plc++] = b_line;
				plc_logic+=2;
			}
			continue;
		}

		if(c == '>') /* should never happens */
		{
			draw_info(data,COLOR_GREEN);
			draw_info("ERROR in book cmd!", COLOR_RED);
			return NULL;
		}

		/* we have a line */
		if(c== '\0' || c  == 0 || c == 0x0a)
		{
			force_line_jump:
			/* we *can* check the line length here (and wrap it to next line) but atm we don't */
			book_line = malloc(sizeof(_gui_book_line));
			memcpy(book_line, &current_book_line,sizeof(_gui_book_line));
			current_book_page.line[plc++] = book_line;
			plc_logic++;
			memset(&current_book_line, 0, sizeof(_gui_book_line));
			lc=0;

			if(plc_logic >= BOOK_PAGE_LINES)
			{
				_gui_book_page *page = malloc(sizeof(_gui_book_page));

				/* add the page & reset the current one */
				memcpy(page, &current_book_page, sizeof(_gui_book_page));
				book_link_page(page);
				memset(&current_book_page, 0, sizeof(_gui_book_page));
				plc=0;
				plc_logic=0;
			}

			continue;
		}

		current_book_line.line[lc++]=c;
		if(lc>=BOOK_LINES_CHAR-2)
			goto force_line_jump;
	}

	if(plc_logic)
	{
		_gui_book_page *page = malloc(sizeof(_gui_book_page));

		/* add the page & reset the current one */
		memcpy(page, &current_book_page, sizeof(_gui_book_page));
		book_link_page(page);
	}

	format_book(gui_interface_book);
	return gui_interface_book;
}

void show_book(int x, int y)
{
	char buf[128];
    SDL_Rect    box;
	int i, ii, yoff;
	_gui_book_page *page1, *page2;

    sprite_blt(Bitmaps[BITMAP_JOURNAL], x, y, NULL, NULL);

	if(!gui_interface_book)
		return;

	box.x=x+95;
	box.y=y+90;
	box.w=275;
	box.h=340;

	/* get the 2 pages we show */
	page1 = gui_interface_book->start;
	for(i=0;i!=gui_interface_book->page_show && page1;i++,page1=page1->next)
		;
	page2=page1->next;



	if(page1)
	{
		sprintf(buf,"Page %d of %d",gui_interface_book->page_show+1,gui_interface_book->pages);
		StringBlt(ScreenSurface, &Font6x3Out, buf, box.x+120, box.y+355, COLOR_WHITE, NULL, NULL);

		SDL_SetClipRect(ScreenSurface, &box);
		//	SDL_FillRect(ScreenSurface, &box, 0);
		for(yoff=0,i=0, ii=0;ii<BOOK_PAGE_LINES;ii++,yoff+=12)
		{
			if(!page1->line[i])
				break;
			if(page1->line[i]->mode == BOOK_LINE_NORMAL)
			{
				StringBlt(ScreenSurface, &SystemFont, page1->line[i]->line , box.x+2, box.y+2+yoff, COLOR_BLACK, NULL, NULL);
			}
			else if(page1->line[i]->mode == BOOK_LINE_TITLE)
			{
				StringBlt(ScreenSurface, &BigFont, page1->line[i]->line, box.x+2, box.y+2+yoff, COLOR_BLACK, NULL, NULL);
				ii++;
				yoff+=12;
			}
			i++;
		}
		SDL_SetClipRect(ScreenSurface, NULL);
	}


	box.x=x+406;
	box.y=y+90;
	box.w=275;
	box.h=340;

	if(gui_interface_book->pages)
	{
		sprintf(buf,"%c and %c to turn page",ASCII_RIGHT, ASCII_LEFT);
		StringBlt(ScreenSurface, &Font6x3Out, buf, box.x-55, box.y+355, COLOR_GREEN, NULL, NULL);
	}

	if(page2)
	{
		sprintf(buf,"Page %d of %d",gui_interface_book->page_show+2,gui_interface_book->pages);
		StringBlt(ScreenSurface, &Font6x3Out, buf, box.x+120, box.y+355, COLOR_WHITE, NULL, NULL);

		SDL_SetClipRect(ScreenSurface, &box);
		for(yoff=0,i=0, ii=0;ii<BOOK_PAGE_LINES;ii++,yoff+=12)
		{
			if(!page2->line[i])
				break;
			if(page2->line[i]->mode == BOOK_LINE_NORMAL)
			{
				StringBlt(ScreenSurface, &SystemFont, page2->line[i]->line , box.x+2, box.y+2+yoff, COLOR_BLACK, NULL, NULL);
			}
			else if(page2->line[i]->mode == BOOK_LINE_TITLE)
			{
				StringBlt(ScreenSurface, &BigFont, page2->line[i]->line, box.x+2, box.y+2+yoff, COLOR_BLACK, NULL, NULL);
				ii++;
				yoff+=12;
			}
			i++;
		}
		SDL_SetClipRect(ScreenSurface, NULL);
	}
}

