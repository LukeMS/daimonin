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

    The author can be reached via e-mail to daimonin@nord-com.net
*/
#include <include.h>

/* this depends on the text win png*/
#define TEXT_WIN_XLEN 265
#define TEXT_WIN_YLEN 190

#define TEXT_WIN_MAX 250

#define TEST_WIN_FLAGS 

_textwin_set textwin_set;

typedef struct _text_buf {
        char buf[128];		/*text*/
		int channel;		/* which channel */
		int flags;			/* some flags */
        int color;			/* color of text */
}_text_buf;


_text_buf text_win_buf[TEXT_WIN_MAX]; /* here we copy in *all* msg */
uint32 win_start=0,win_lenbuf=0;
int win_len = 0;
int text_win_firstOutputLine;
int text_win_soff;

_text_buf text_win_buf_top[TEXT_WIN_MAX]; /* here we copy in only stuff releated to top windows */
uint32 win_start_top=0,win_lenbuf_top=0;
int win_len_top = 0;
int text_win_top_firstOutputLine;
int text_win_soff_top;

_text_buf text_win_buf_split[TEXT_WIN_MAX]; /* and the same for the 2nd part */
uint32 win_start_split=0,win_lenbuf_split=0;
int win_len_split = 0;
int text_win_split_firstOutputLine;
int text_win_soff_split;

/******************************************************************
 returns a pointer to the textLine[pos] of the lower textwin.
******************************************************************/
char *get_textWinRow(int mouseY){
	if(textwin_set.split_flag == FALSE){
		int pos = (mouseY -488) / 10; /* clicked row */
		pos = (text_win_firstOutputLine+pos)%TEXT_WIN_MAX - text_win_soff;
    if (pos <0)
      pos+=TEXT_WIN_MAX;
		return text_win_buf[pos].buf;
	}else{
		int pos = (mouseY-588)/10+textwin_set.split_size; /* clicked row */
		pos = (text_win_split_firstOutputLine+pos)%TEXT_WIN_MAX - text_win_soff_split;
    if (pos <0)
      pos+=TEXT_WIN_MAX;
		return text_win_buf_split[pos].buf;
	}
} 

void clear_textwin(void)
{
        win_lenbuf=win_start=0;
        win_len = 0;
        text_win_soff=0;

        win_lenbuf_split=win_start_split=0;
        win_len_split = 0;
        text_win_soff_split=0;

        win_lenbuf_top=win_start_top=0;
        win_len_top = 0;
        text_win_soff_top=0;
}

/* add this to the text windows...*/
void draw_info (char *str, int flags )
{
        int i, len,a, media=0, color, mode;
        Boolean gflag;

        int winlen = 239;
        char buf[4096];

		color = flags&0xff;
		mode = flags;
        /*
        * first: we set all white spaces (char<32) to 32 to remove really all odd stuff.
        * except 0x0a - this is EOL for us and will be set to
        * 0 to mark C style end of string
        */
        for(i=0;str[i]!=0;i++)
        {
            if(str[i] < 32 && str[i] != 0x0a && str[i] != '§')
                str[i]= 32;
        }

        /*
         * ok, here we must cut a string to make it fit in window
         * for it we must add the char length
         * we assume this standard font in the windows...
        */

        len = 0;
        gflag = FALSE;
        for(a=i=0;;i++)
        {
            if(str[i] == '§' || gflag)
            {
                if(str[i] == '§')
                {
                    media = i;
                    gflag = TRUE;
                }
                else
                {
                    if(str[i] == 0x0a)
                    {
                        str[i] =0;
                        init_media_tag(&str[media]);
                        gflag = FALSE;
                    }
                }
                continue;
            }
            if(str[i] != '^')
                len += SystemFont.c[(int)(str[i])].w+SystemFont.char_offset;

            if(len>=winlen || str[i] == 0x0a ||str[i]==0)
            {
				/*
                if(str[i]==0 && !a)
                    break;
				*/

				/* now the special part - lets look for a good point to cut */
				if(len>=winlen && a>10)
				{
					int ii=a,it=i,ix=a,tx=i;

						while(ii>=a/2)
						{
							if(str[it]==' ' || str[it]==':' || str[it]=='.' || str[it]==','
								|| str[it]=='(' || str[it]==';'|| str[it]=='-'
								|| str[it]=='+'|| str[it]=='*'|| str[it]=='?'|| str[it]=='/'
								|| str[it]=='='|| str[it]=='.'|| str[it]==0|| str[it]==0x0a)
							{
								tx=it;
								ix=ii;
								break;
							}
							it--;
							ii--;
						};
						i=tx;
						a=ix;
					}

                buf[a]=0;

				/* add message always to mixed textwin (default) */
                strcpy(text_win_buf[win_start%TEXT_WIN_MAX].buf, buf);
                text_win_buf[win_start%TEXT_WIN_MAX].color = color;
                text_win_buf[win_start%TEXT_WIN_MAX].flags = mode;
                win_start++;
                if(text_win_soff)
                    text_win_soff++;
                win_lenbuf++;
                if(win_lenbuf >TEXT_WIN_MAX)
                    win_lenbuf=TEXT_WIN_MAX;
                win_start%=TEXT_WIN_MAX;

				if(mode & NDI_PLAYER)
				{
					strcpy(text_win_buf_top[win_start_top%TEXT_WIN_MAX].buf, buf);
					text_win_buf_top[win_start_top%TEXT_WIN_MAX].color = color;
					text_win_buf_top[win_start_top%TEXT_WIN_MAX].flags = mode;
					win_start_top++;
			       if(text_win_soff_top)
						text_win_soff_top++;
					win_lenbuf_top++;
					if(win_lenbuf_top >TEXT_WIN_MAX)
						win_lenbuf_top=TEXT_WIN_MAX;
					win_start_top%=TEXT_WIN_MAX;
				}
				else
				{
					strcpy(text_win_buf_split[win_start_split%TEXT_WIN_MAX].buf, buf);
					text_win_buf_split[win_start_split%TEXT_WIN_MAX].color = color;
					text_win_buf_split[win_start_split%TEXT_WIN_MAX].flags = mode;
					win_start_split++;
			       if(text_win_soff_split)
						text_win_soff_split++;
					win_lenbuf_split++;
					if(win_lenbuf_split >TEXT_WIN_MAX)
						win_lenbuf_split=TEXT_WIN_MAX;
					win_start_split%=TEXT_WIN_MAX;
				}
                a=len = 0;
                if(str[i]==0)
                    break;
	        }
            if(str[i] != 0x0a)
                buf[a++] = str[i];
            
        }			
}

static void show_window_top(int x, int y, int lines)
{
        int i, temp;

        text_win_top_firstOutputLine = win_start_top-(lines+1);
        if(text_win_top_firstOutputLine <0 && win_lenbuf_top == TEXT_WIN_MAX)
                text_win_top_firstOutputLine+=TEXT_WIN_MAX;
        else if(text_win_top_firstOutputLine <0)
            text_win_top_firstOutputLine = 0;

        if((int)win_lenbuf_top>lines)
        {
            if(text_win_soff_top >(int)win_lenbuf_top-(lines+1))
                text_win_soff_top = win_lenbuf_top-(lines+1);
        }
        else
            text_win_soff_top=0;

        if(text_win_soff_top <0)
            text_win_soff_top=0;

        for(i=0;i<=lines && i <(int)win_lenbuf_top;i++)
        {
            temp = (text_win_top_firstOutputLine+i)%TEXT_WIN_MAX;
            if((int)win_lenbuf_top>lines) /* only use scroll offset, when there is some */
            {
                temp-=text_win_soff_top;
                if(temp <0)
                    temp=TEXT_WIN_MAX+temp;
            }
            StringBlt(ScreenSurface, &SystemFont, &text_win_buf_top[temp].buf[0],
                                x+2, y+1+i*10,text_win_buf_top[temp].color, NULL, NULL);
        }

		sprite_blt(Bitmaps[BITMAP_SLIDER_UP],x+250, y+3, NULL,NULL);
		sprite_blt(Bitmaps[BITMAP_SLIDER_DOWN],x+250, y+4+lines*10, NULL,NULL);

		/* now we need at last a 2 line window - or our up/down slider are bigger as the 
		 * area for slider itself!
		 */
		if(lines >=1)
		{
		    SDL_Rect box;

			temp = (lines*10)-9; 
			box.x = 0;
			box.y = 0;
			box.w = Bitmaps[BITMAP_SLIDER]->bitmap->w;
			box.h = temp;
			sprite_blt(Bitmaps[BITMAP_SLIDER],x+250, y+Bitmaps[BITMAP_SLIDER_UP]->bitmap->h+3, &box,NULL);
				
			if(lines >1)
				blt_window_slider(Bitmaps[BITMAP_TWIN_SCROLL], win_lenbuf_top,(lines+1),
				    ((int)win_lenbuf_top<(lines+1)?0:win_lenbuf_top-(lines+1))-text_win_soff_top, temp-2, x+252, y+Bitmaps[BITMAP_SLIDER_UP]->bitmap->h+4);
		}
}

static void show_window_split(int x, int y, int lines)
{
        int i, temp;

        text_win_split_firstOutputLine = win_start_split-(lines+1);
        if(text_win_split_firstOutputLine <0 && win_lenbuf_split == TEXT_WIN_MAX)
                text_win_split_firstOutputLine+=TEXT_WIN_MAX;
        else if(text_win_split_firstOutputLine <0)
            text_win_split_firstOutputLine = 0;

        if((int)win_lenbuf_split>lines)
        {
            if(text_win_soff_split >(int)win_lenbuf_split-(lines+1))
                text_win_soff_split = win_lenbuf_split-(lines+1);
        }
        else
            text_win_soff_split=0;

        if(text_win_soff_split <0)
            text_win_soff_split=0;

        for(i=0;i<=lines && i <(int)win_lenbuf_split;i++)
        {
            temp = (text_win_split_firstOutputLine+i)%TEXT_WIN_MAX;
            if((int)win_lenbuf_split>lines) /* only use scroll offset, when there is some */
            {
                temp-=text_win_soff_split;
                if(temp <0)
                    temp=TEXT_WIN_MAX+temp;
            }
            StringBlt(ScreenSurface, &SystemFont, &text_win_buf_split[temp].buf[0],
                                x+2, y+1+i*10,text_win_buf_split[temp].color, NULL, NULL);
        }
		lines++;
		sprite_blt(Bitmaps[BITMAP_SLIDER_UP],x+250, y+3, NULL,NULL);
		sprite_blt(Bitmaps[BITMAP_SLIDER_DOWN],x+250, y+3+lines*10, NULL,NULL);
		/* now we need at last a 2 line window - or our up/down slider are bigger as the 
		 * area for slider itself!
		 */
		if(lines >=1)
		{
		    SDL_Rect box;

			temp = (lines*10)-10; 
			box.x = 0;
			box.y = 0;
			box.w = Bitmaps[BITMAP_SLIDER]->bitmap->w;
			box.h = temp;
			sprite_blt(Bitmaps[BITMAP_SLIDER],x+250, y+Bitmaps[BITMAP_SLIDER_UP]->bitmap->h+3, &box,NULL);
				
			if(lines >1)
				blt_window_slider(Bitmaps[BITMAP_TWIN_SCROLL], win_lenbuf_split,lines,
				   ((int)win_lenbuf_split<lines?0:win_lenbuf_split-lines)-text_win_soff_split, temp-2, x+252, y+Bitmaps[BITMAP_SLIDER_UP]->bitmap->h+4);
		}
}

static void show_window(int x, int y, int lines)
{
	int i, temp;
        
	text_win_firstOutputLine = win_start-(lines+1);
	if(text_win_firstOutputLine <0 && win_lenbuf == TEXT_WIN_MAX)
		text_win_firstOutputLine+=TEXT_WIN_MAX;
	else if(text_win_firstOutputLine <0)
		text_win_firstOutputLine = 0;

	if((int)win_lenbuf>lines)
    {
		if(text_win_soff >(int)win_lenbuf-(lines+1))
			text_win_soff = win_lenbuf-(lines+1);
	}
	else
		text_win_soff=0;

	if(text_win_soff <0)
		text_win_soff=0;

	for(i=0;i<=lines && i <(int)win_lenbuf;i++)
	{
		temp = (text_win_firstOutputLine+i)%TEXT_WIN_MAX;
		if((int)win_lenbuf>lines)
        {
			temp-=text_win_soff;
            if(temp <0)
				temp=TEXT_WIN_MAX+temp;
		}
		StringBlt(ScreenSurface, &SystemFont, &text_win_buf[temp].buf[0],
                                x+2, y+1+i*10,text_win_buf[temp].color, NULL, NULL);
	}
		
		lines++;
		sprite_blt(Bitmaps[BITMAP_SLIDER_UP],x+250, y+2, NULL,NULL);
		sprite_blt(Bitmaps[BITMAP_SLIDER_DOWN],x+250, y+3+lines*10, NULL,NULL);
		/* now we need at last a 2 line window - or our up/down slider are bigger as the 
		 * area for slider itself!
		 */
		if(lines >=1)
		{
		    SDL_Rect box;

			temp = (lines*10)-9; 
			box.x = 0;
			box.y = 0;
			box.w = Bitmaps[BITMAP_SLIDER]->bitmap->w;
			box.h = temp;
			sprite_blt(Bitmaps[BITMAP_SLIDER],x+250, y+Bitmaps[BITMAP_SLIDER_UP]->bitmap->h+2, &box,NULL);
				
			if(lines >1)
				blt_window_slider(Bitmaps[BITMAP_TWIN_SCROLL], win_lenbuf,lines,
				    ((int)win_lenbuf<lines?0:win_lenbuf-lines)-text_win_soff, temp-2, x+252, y+Bitmaps[BITMAP_SLIDER_UP]->bitmap->h+3);
		}
}

void show_textwin(int x, int y)
{
	int len, tmp;
	SDL_Rect box;
	_BLTFX bltfx;

	bltfx.alpha = textwin_set.alpha;
	bltfx.flags = BLTFX_FLAG_SRCALPHA;
	box.x =	box.y = 0;
	box.w = Bitmaps[BITMAP_TEXTWIN]->bitmap->w;
	sprite_blt(Bitmaps[BITMAP_TEXTWIN_BLANK],x-1, y+2, NULL, NULL);
	y=599; /* to lazy to work with correct calcs */
	
	if(textwin_set.split_flag == TRUE)
	{
		box.h = len =(textwin_set.split_size+textwin_set.top_size)*5+18;
		y-=len*2;
		if(textwin_set.use_alpha == TRUE){
			sprite_blt(Bitmaps[BITMAP_TEXTWIN_MASK],x-1, y,     &box, &bltfx);
			sprite_blt(Bitmaps[BITMAP_TEXTWIN_MASK],x-1, y+len, &box, &bltfx);
		}else{
			sprite_blt(Bitmaps[BITMAP_TEXTWIN],x-1, y,     &box,NULL);
			sprite_blt(Bitmaps[BITMAP_TEXTWIN],x-1, y+len, &box, NULL);			
		}
		show_window_top(x, y-2,textwin_set.top_size);
		tmp = textwin_set.top_size*10+12;
		show_window_split(x, y+tmp,textwin_set.split_size);
		sprite_blt(Bitmaps[BITMAP_TEXTWIN_SPLIT],x-1, y+(tmp+1), NULL, NULL); 
		/*
		box.x = x-1;
		box.h =1;		
		box.y = y+tmp+1;
		SDL_FillRect(ScreenSurface, &box, -1);
		box.y = y-1;
		SDL_FillRect(ScreenSurface, &box, -1);
		box.w =1;
		box.h = 599- y;
		*/
/*		SDL_FillRect(ScreenSurface, &box, -1);	*/
	}
	else
	{
		box.h =len= textwin_set.size*10 +23;
		y-=len;
		if(textwin_set.use_alpha == TRUE)
			sprite_blt(Bitmaps[BITMAP_TEXTWIN_MASK],x-1, y, &box, &bltfx);
		else
			sprite_blt(Bitmaps[BITMAP_TEXTWIN],x-1, y, &box,NULL);
		show_window(x, y-1,textwin_set.size);
	}
	StringBlt(ScreenSurface, &SystemFont, ">", x+2, 586, 1,NULL, NULL);
}

