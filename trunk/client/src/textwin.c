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

typedef struct _text_buf {
        char buf[256]; /*text*/
        int color; /*color of text*/
}_text_buf;

_text_buf text_win_buf[TEXT_WIN_MAX];

uint32 win_start=0,win_lenbuf=0;
int win_len = 0;
int text_win_soff;

void clear_textwin(void)
{
        win_lenbuf=win_start=0;
        win_len = 0;
        text_win_soff=0;
}

/* add this to the text windows...*/
void draw_info (char *str, int color )
{
        int i, len,a, media=0;
        Boolean gflag;

        int winlen = 239;
        char buf[4096];

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
            if(str[i] != '&')
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
								|| str[it]=='(' || str[it]==')' || str[it]==';'|| str[it]=='-'
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
                strcpy(text_win_buf[win_start%TEXT_WIN_MAX].buf, buf);
                text_win_buf[win_start%TEXT_WIN_MAX].color = color;
                win_start++;
                if(text_win_soff)
                    text_win_soff++;
                win_lenbuf++;
                if(win_lenbuf >TEXT_WIN_MAX)
                    win_lenbuf=TEXT_WIN_MAX;
                win_start%=TEXT_WIN_MAX;
                a=len = 0;
                if(str[i]==0)
                    break;
            }
            if(str[i] != 0x0a)
                buf[a++] = str[i];
            
        }			
}

#define TEXT_WIN_LINES 9
void show_textwin(int x, int y)
{
        int i, index,temp;

        sprite_blt(Bitmaps[BITMAP_TEXTWIN],x, y+2, NULL, NULL);

        index = win_start-(TEXT_WIN_LINES+1);
        if(index <0 && win_lenbuf == TEXT_WIN_MAX)
                index=TEXT_WIN_MAX+index;
        else if(index <0)
            index = 0;

        if(win_lenbuf>TEXT_WIN_LINES)
        {
            if(text_win_soff >(int)win_lenbuf-(TEXT_WIN_LINES+1))
                text_win_soff = win_lenbuf-(TEXT_WIN_LINES+1);
        }
        else
            text_win_soff=0;

        if(text_win_soff <0)
            text_win_soff=0;

        blt_window_slider(Bitmaps[BITMAP_TWIN_SCROLL], win_lenbuf,(TEXT_WIN_LINES+1),
            (win_lenbuf<(TEXT_WIN_LINES+1)?0:win_lenbuf-(TEXT_WIN_LINES+1))-text_win_soff, x+252, y+14);
            
        for(i=0;i<=TEXT_WIN_LINES && i <(int)win_lenbuf;i++)
        {
            temp = (index+i)%TEXT_WIN_MAX;
            if(win_lenbuf>TEXT_WIN_LINES) /* only use scroll offset, when there is some */
            {
                temp-=text_win_soff;
                if(temp <0)
                    temp=TEXT_WIN_MAX+temp;
            }
            StringBlt(ScreenSurface, &SystemFont, &text_win_buf[temp].buf[0],
                                x+2, y+1+i*10,text_win_buf[temp].color, NULL, NULL);
        }
        StringBlt(ScreenSurface, &SystemFont, ">",
                                x+2, y+1+i*10,1, NULL, NULL);
}
