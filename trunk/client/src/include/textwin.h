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

#if !defined(__TEXTWIN_H)
#define __TEXTWIN_H

typedef struct _textwin_set {
	int split_flag;	/* if FALSE, normal window - if TRUE: splitted */
	int size;		/* if not split, size of window min:9 max:37 */
	int split_size; /* if split, main part: size min:9, max 36 */
	int top_size;   /* top part if split, size: min: 1, max 27 */
	int use_alpha;	/* true: use alpha - false: no alpha */
	int alpha;		/* alpha value - from 0 to 255 */
	int slider_start; /* start-offset of the slider */
	int slider_h; 
}_textwin_set;

extern _textwin_set textwin_set;

extern int text_win_soff;
extern int text_win_soff_split;
extern int text_win_soff_top;
extern char *get_textWinRow(int pos);
extern void draw_info ( char *str, int color );
extern void show_textwin(int x, int y);
extern uint32 win_lenbuf;
void clear_textwin(void);


#endif
