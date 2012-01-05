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

#ifndef __TEXTWIN_H
#define __TEXTWIN_H

#define TEXTWIN_BUFSIZE 250

/* The size of the resizing borders. */
#define TEXTWIN_ACTIVE_MIN 2
#define TEXTWIN_ACTIVE_MAX 16

/* Resizing limits. Min values are arbitrary, max must be <= the size of the
 * bitmaps. */
#define TEXTWIN_WIDTH_MIN  100
#define TEXTWIN_HEIGHT_MIN 50
#define TEXTWIN_WIDTH_MAX  1280
#define TEXTWIN_HEIGHT_MAX 1280

typedef enum textwin_id_t
{
    TEXTWIN_CHAT_ID,
    TEXTWIN_MSG_ID,

    TEXTWIN_NROF
}
textwin_id_t;

typedef enum textwin_mode_t
{
    TEXTWIN_MODE_NONE,
    TEXTWIN_MODE_RESIZE,
    TEXTWIN_MODE_SCROLL
}
textwin_mode_t;

typedef enum textwin_resize_t
{
    TEXTWIN_RESIZE_NONE,
    TEXTWIN_RESIZE_UP,
    TEXTWIN_RESIZE_UPRIGHT,
    TEXTWIN_RESIZE_RIGHT,
    TEXTWIN_RESIZE_DOWNRIGHT,
    TEXTWIN_RESIZE_DOWN,
    TEXTWIN_RESIZE_DOWNLEFT,
    TEXTWIN_RESIZE_LEFT,
    TEXTWIN_RESIZE_UPLEFT
}
textwin_resize_t;

typedef enum textwin_scroll_t
{
    TEXTWIN_SCROLL_NONE,
    TEXTWIN_SCROLL_UP,
    TEXTWIN_SCROLL_UPPAGE,
    TEXTWIN_SCROLL_VERTICAL,
    TEXTWIN_SCROLL_DOWNPAGE,
    TEXTWIN_SCROLL_DOWN,
}
textwin_scroll_t;

typedef struct textwin_text_t
{
    char   buf[MEDIUM_BUF]; // text
    int    channel;         // which channel
    uint32 flags;           // some flags
    uint32 fg;              // color of text
    uint32 bg;              // color of bg
}
textwin_text_t;

typedef struct textwin_window_t
{
    uint16              x,            // startpos of the window on the x axis in pixels
                        y;            // startpos of the window on the y axis in pixels
    textwin_mode_t      mode;         // mode
    widget_id_t         widget;       // widget assocoiated with this window
    uint16              maxstringlen; // max length of string in pixels
    int                 slider_h;     // height of the scrollbar-slider
    int                 slider_y;     // start pos of the scrollbar-slider
    textwin_resize_t    resize;       // resizing direction
    sint16              resize_x,     // resizing distance on x axis in pixels
                        resize_y;     // resizing distance on y axis in pixels
    textwin_scroll_t    scroll;       // which part to scroll
    sint16              scroll_x,     // scrolling distance on x axis in pixels
                        scroll_y;     // scrolling distance on y axis in pixels
    uint32              scroll_off;   // scroll offset
    uint32              scroll_size;  // max size of scroll buffer
    uint32              scroll_used;  // position in scroll buffer
    uint32              scroll_pos;   // last printed textline
    uint32              size;         // number or printed textlines
    _font              *font;         // the font used in this window
    textwin_text_t     *text;
}
textwin_window_t;

extern textwin_window_t textwin[TEXTWIN_NROF];

extern void textwin_init(textwin_id_t id);
extern void textwin_set_font(textwin_id_t id);
extern void textwin_show_string(uint32 flags, uint32 colr, char *format, ...);
extern void textwin_show_window(textwin_id_t id);
extern void textwin_event(uint8 e, SDL_Event *event, textwin_id_t id);
extern void textwin_keypress(SDLKey key, textwin_id_t id);
extern void textwin_add_history(char *text);
extern void textwin_clear_history(void);
extern void textwin_put_string(char *text);

#endif /* ifndef __TEXTWIN_H */
