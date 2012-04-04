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
    TEXTWIN_SCROLL_FREESCROLL,
    TEXTWIN_SCROLL_DOWN,
    TEXTWIN_SCROLL_HBARGE,
    TEXTWIN_SCROLL_HCANALLEFT,
    TEXTWIN_SCROLL_HCANALRIGHT,
    TEXTWIN_SCROLL_LEFT,
    TEXTWIN_SCROLL_RIGHT,
    TEXTWIN_SCROLL_UP,
    TEXTWIN_SCROLL_VBARGE,
    TEXTWIN_SCROLL_VCANALDOWN,
    TEXTWIN_SCROLL_VCANALUP
}
textwin_scroll_t;

typedef struct textwin_linebuf_t
{
    char   *buf;   // text
    uint32  flags; // some flags
    uint32  fg;    // colour of text
    uint32  bg;    // colour of bg
}
textwin_linebuf_t;

typedef struct textwin_window_t
{
    widget_id_t        wid;          // widget associated with this window
    _font             *font;         // the font used in this window
    sint16             contentwd,    // width of content in pixels given window and vcanal width
                       maxcontentwd, // max width of content in pixels given max window and vcanal width
                       contentht,    // height of content in pixels given window and hcanal height
                       maxcontentht; // max height of content in pixels given max window and hcanal height
    textwin_mode_t     mode;         // mode
    textwin_resize_t   resize;       // resizing direction
    sint16             resize_x,     // resizing distance on x axis in pixels
                       resize_y;     // resizing distance on y axis in pixels
    textwin_scroll_t   scroll;       // which part to scroll
    sint16             scroll_x,     // scrolling distance on x axis in pixels
                       scroll_xoff,  // scroll offset on x axis in pixels
                       scroll_y,     // scrolling distance on y axis in pixels
                       scroll_yoff;  // scroll offset on y axis in pixels
    uint16             hbarge_pos,   // start pos of the horizontal barge in pixels
                       hbarge_len,   // width of the horizontal barge in pixels
                       hcanal_pos,   // start pos of the horizontal canal in pixels
                       hcanal_len,   // width of the horizontal canal in pixels
                       vbarge_pos,   // start pos of the vertical barge in pixels
                       vbarge_len,   // height of the vertical barge in pixels
                       vcanal_pos,   // start pos of the vertical canal in pixels
                       vcanal_len;   // height of the vertical canal in pixels
    uint32             linebuf_size, // max size of linebuf buffer in lines
                       linebuf_used, // position in linebuf buffer in lines
                       linebuf_visi, // number or visible lines given window and font height
                       linebuf_next; // next line from linebuf buffer
    textwin_linebuf_t *linebuf;      // the linebuf buffer
}
textwin_window_t;

extern textwin_window_t textwin[TEXTWIN_NROF];

extern void textwin_init(void);
extern void textwin_deinit(void);
extern void textwin_set_font(textwin_id_t id);
extern void textwin_calculate_dimensions(textwin_window_t *tw);
extern void textwin_show_string(uint32 flags, uint32 colr, char *format, ...);
extern void textwin_show_window(textwin_id_t id);
extern void textwin_event(uint8 e, SDL_Event *event, textwin_id_t id);
extern void textwin_keypress(SDLKey key, textwin_id_t id);
extern void textwin_add_history(char *text);
extern void textwin_clear_history(void);
extern void textwin_put_string(char *text);

#endif /* ifndef __TEXTWIN_H */
