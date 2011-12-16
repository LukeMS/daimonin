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

/* Shows InputString and an appropriate caret. */
void show_input_string(_font *font, SDL_Rect *box, char repl)
{
    uint16 i;
    char   buf[MAX_INPUT_STRING];
    int    len;
    sint16 xoff;

    for (i = 0; i < MAX_INPUT_STRING && InputString[i]; i++)
    {
        buf[i] = (repl) ? repl : InputString[i];
    }

    buf[i] = '\0';

    /* Replace the character at CurrentCursorPos with \0. */
    buf[CurrentCursorPos] = '\0';

    /* Get the pixel length of this first part of buf. */
    len = string_width(font, buf);

    /* Calculate any needed offset so the caret and context is always
     * visible. */
    if ((xoff = len - box->w * 0.8) < 0)
    {
        xoff = 0;
    }

    /* Set the clipping rectangle. */
    SDL_SetClipRect(ScreenSurface, box);

    /* Draw buf up to that point. */
    string_blt(ScreenSurface, font, buf, box->x - xoff, box->y,
               NDI_COLR_WHITE, NULL, NULL);

    /* Restore the character at CurrentCursorPos. */
    if (InputString[CurrentCursorPos])
    {
        buf[CurrentCursorPos] = (repl) ? repl : InputString[CurrentCursorPos];
    }

    /* Draw buf from that point. */
    string_blt(ScreenSurface, font, &buf[CurrentCursorPos],
               box->x - xoff + len, box->y, NDI_COLR_WHITE, NULL, NULL);

    /* Draw the caret. */
    if (InputCaretBlinkFlag &&
        LastTick % 500 >= 250)
    {
        SDL_Rect caret;
        uint32   colr;

        caret.y = box->y;
        caret.h = font->line_height;

        if (!InputMode)
        {
            caret.x = MAX(box->x, box->x - xoff + len - font->char_offset);
            caret.w = 1;
        }
        else
        {
            caret.x = box->x - xoff + len;
            caret.w = font->c[(int)buf[CurrentCursorPos]].w;
        }

        colr = SDL_MapRGB(ScreenSurface->format,
                          (skindef.input_caret >> 16) & 0xff,
                          (skindef.input_caret >> 8) & 0xff,
                          skindef.input_caret & 0xff);
        SDL_FillRect(ScreenSurface, &caret, colr);
    }

    InputCaretBlinkFlag = 1;

    /* Remove the clipping rectangle. */
    SDL_SetClipRect(ScreenSurface, NULL);
}

int read_substr_char(char *srcstr, char *desstr, int *sz, char ct)
{
    register unsigned char c;
    register int s = 0;

    desstr[0] = 0;
    for (; s < 1023;)
    {
        c = *(desstr + s++) = *(srcstr + *sz); /* get character*/
        if (c == 0x0d)
            continue;
        if (c == 0)
            return(-1);
        if (c == 0x0a || c == ct)  /* if it END or WHITESPACE..*/
            break; /* have a single word! (or not...)*/
        (*sz)++; /* point to next source char */
    }
    *(desstr + (--s)) = 0; /* terminate all times with 0, */
    /* s: string length*/
    (*sz)++; /*point to next source charakter return(s);*/
    return s;
}


/* this function gets a ="xxxxxxx" string from a
 * line. It removes the =" and the last " and returns
 * the string in a static buffer.
 * maxlen !!inclusive!! terminating \0
 */
char *get_parameter_string(char *data, int *pos, int maxlen)
{
    char *start_ptr, *end_ptr;
    static char buf[4096];
    int done=0;
    int offset=0, cpysize=0;

    /* we assume a " after the =... don't be to shy, we search for a '"' */
    start_ptr = strchr(data+*pos,'"');
    if (!start_ptr)
        return NULL; /* error */

    buf[0]='\0'; // sanity 0
    while (!done)
    {
        end_ptr = strchr(++start_ptr,'"');
        if (!end_ptr)
            return NULL; /* error */

        if ((*(end_ptr-1))=='\\') //We have a escaped " which is NOT the end-"
        {
            /* sanity checks against buffer overflow */
            cpysize=(end_ptr-start_ptr-1)+offset;
            if ((strlen(buf)+cpysize)>4095)
                cpysize=4095-strlen(buf);

            strncat(buf,start_ptr-offset,cpysize);
            start_ptr = end_ptr;
            offset=1; //after the first loop we have to catch the " which is message not endtag
        }
        else //we have the end "-tag
        {
            /* sanity checks against buffer overflow */
            cpysize=(end_ptr-start_ptr)+offset;
            if ((strlen(buf)+cpysize)>4095)
                cpysize=4095-strlen(buf);

            strncat(buf, start_ptr-offset, cpysize);
            done=1;
        }
    }

    /* ahh... ptr arithmetic... eat that, high level language fans ;) */
    *pos += ++end_ptr-(data+*pos);

    /* sanity truncate string to maxlen */
    if (maxlen>0)
    {
#ifdef DAI_DEVELOPMENT
        if ((int)strlen(buf)>maxlen)
            textwin_showstring(0, NDI_COLR_RED,"FixMe: Interface parameter string out of bounds!");
#endif
        buf[maxlen-1]='\0';
    }

    return buf;
}

/*
 * Based on (n+1)^2 = n^2 + 2n + 1
 * given that   1^2 = 1, then
 *      2^2 = 1 + (2 + 1) = 1 + 3 = 4
 *      3^2 = 4 + (4 + 1) = 4 + 5 = 1 + 3 + 5 = 9
 *      4^2 = 9 + (6 + 1) = 9 + 7 = 1 + 3 + 5 + 7 = 16
 *      ...
 * In other words, a square number can be express as the sum of the
 * series n^2 = 1 + 3 + ... + (2n-1)
 */
int isqrt(int n)
{
    int result, sum, prev;
    result = 0;
    prev = sum = 1;
    while (sum <= n)
    {
        prev += 2;
        sum += prev;
        ++result;
    }
    return result;
}

/* We HAVE to replace the smileys codes with the char-value of the smiley,
 * before ANY stringbreaking, stringwidth-calcs or drawings are done!
 */

void smiley_convert(char *msg)
{
    unsigned char   actChar;
    int             i, j, move;


    for (i = 0; msg[i] != 0; i++)
    {
        actChar = 0;
        move = 1;
        if (msg[i] == (unsigned char)':')
        {
            j = i + 1;
            if (msg[j] == '\'' && msg[j+1] == '(')
            {
                actChar = 138;
                move=2;
            }
            else if (msg[j] == '-')
            {
                j++;
                move++;
            }
            switch (msg[j])
            {
                case ')': actChar=128; break;  /* we replace it with the 'ASCII'-code of the smiley in systemfont */
                case '(': actChar=129; break;
                case 'D': actChar=130; break;
                case '|': actChar=131; break;
                case 'o':
                case 'O':
                case '0': actChar=132; break;
                case 'p':
                case 'P': actChar=133; break;

                case 's':
                case 'S': actChar=139; break;
                case 'x':
                case 'X': actChar=140; break;
            }

        }
        else if (msg[i] == (unsigned char)';')
        {
            j = i + 1;
            if (msg[j] == '-')
            {
                j++;
                move++;
            }

            if (msg[j] == ')') actChar = 134;
            else if (msg[j] == 'p') actChar = 137;
            else if (msg[j] == 'P') actChar = 137;
        }
        else if (((msg[i] == (unsigned char)'8') || (msg[i] == (unsigned char)'B')) && (msg[i+1] == ')'))
        {
            actChar=135;
        }
        else if (((msg[i] == (unsigned char)'8') || (msg[i] == (unsigned char)'B')) && (msg[i+2] == ')') && (msg[i+1] == '-'))
        {
            actChar=135;
            move=2;
        }
        else if ((msg[i] == (unsigned char)'^' && msg[i+2] == '^') && ((msg[i+1] == '_' ) || (msg[i+1] == '-')))
        {
            actChar = 136;
            move=2;
        }
        else if ((msg[i] == (unsigned char)'>') && (msg[i+1] == (unsigned char)':'))
        {
            j=i+2;
            move=2;
            if (msg[j] == '-')
            {
                move++;
                j++;
            }
            if (msg[j]==')')
                actChar=141;
            else if (msg[j]=='D')
                actChar=142;
        }

        if (actChar!=0)
        {
            msg[i]=actChar;
            memmove(&msg[i+1],&msg[i+1+move],strlen(&msg[i+1+move])+1);
        }
    }

}




extern void     markdmbuster()
{
    int tag=-1;
    item *it=NULL;
    char    buf[TINY_BUF];

    sprintf(buf,"%s's DMBuster",cpl.name);
    tag=locate_item_tag_from_name(buf);

    if (tag == -1 || !locate_item(tag))
        return;

    send_mark_obj((it = locate_item(tag)));
    if (it)
    {
        textwin_showstring(0, NDI_COLR_OLIVE, "%s %s",
                           (cpl.mark_count == (int)it->tag) ? "unmark" : "mark",
                           it->s_name);
    }


}

static char ScratchSpace[MAX_BUF];

/* Strips excess whitespace from string, writing the normalized string to ScratchSpace.
 */
char *normalize_string(const char *string)
{
    char  buf[MAX_BUF], /* this will be a wc of string */
         *token = NULL;
    strcpy(buf, string);
    /* Wipe ScratchSpace clean every time: */
    *ScratchSpace = '\0';
    /* Get the next non-whitespace token from buf and concatenate it and one
     * trailing whitespace to ScratchSpace:
     */
    for (token = strtok(buf, " \t"); token != NULL; token = strtok(NULL, " \t"))
    {
        strcat(ScratchSpace, token);
        strcat(ScratchSpace, " ");
    }
    /* There will be one trailing whitespace left. Get rid of it: */
    ScratchSpace[strlen(ScratchSpace) - 1] = '\0';
    return ScratchSpace;
}


/* we get endian templates from the server.
* setup the shift values
*/
int setup_endian_sync(const char *const buf)
{
    /* we have 6 bytes here showing the server endian */
    endian_int16 = *((uint16 *)buf);
    endian_int32 = *((uint32 *)(buf+2));

    /* DEBUG: only for testing */
    // endian_int32 = 0x02010403;
    // endian_int16 = 0x0102;

    LOG(LOG_MSG, "Endian:: we got short16:%x int32:%x\n", endian_int16, endian_int32);

    /* lets first check the simplest case: which means we don't must shift anything! */
    endian_do16 = 0; /* easy going! */
    if(endian_int16 != 0x0201)
    {
        uint16 test16 = 0x0201;

        /* well, its easy: if we don't have 0x0201 then we have 0x0102... */
        if(endian_int16 == 0x0201) /* some stupid sanity check */
            return 0;

        endian_do16 = 1;

        LOG(LOG_MSG, "CHECK Endian 16bit:: we got %x we created read:%x send:%x\n", endian_int16, adjust_endian_int16(endian_int16), adjust_endian_int16(test16));
        if(endian_int16 != adjust_endian_int16(test16) || test16 != adjust_endian_int16(endian_int16))
            return 0; /* should NEVER happens */
    }

    /* 32 bit is a bit more complex */
    if(endian_int32 == 0x04030201)
        endian_do32 = 0;
    else /* ok, we have a bit work to do */
    {
        uint32 test32 = 0x04030201;

        endian_do32 = 1;
        /* to lazy to do this smart with a loop */
        if((endian_int32 & 0x000000ff) == 0x01)
            endian_shift32[0] = 0;
        else if((endian_int32 & 0x0000ff00) == 0x0100)
            endian_shift32[0] = 8;
        else if((endian_int32 & 0x00ff0000) == 0x010000)
            endian_shift32[0] = 16;
        else
            endian_shift32[0] = 24;

        if((endian_int32 & 0x000000ff) == 0x02)
            endian_shift32[1] = 0;
        else if((endian_int32 & 0x0000ff00) == 0x0200)
            endian_shift32[1] = 8;
        else if((endian_int32 & 0x00ff0000) == 0x020000)
            endian_shift32[1] = 16;
        else
            endian_shift32[1] = 24;

        if((endian_int32 & 0x000000ff) == 0x03)
            endian_shift32[2] = 0;
        else if((endian_int32 & 0x0000ff00) == 0x0300)
            endian_shift32[2] = 8;
        else if((endian_int32 & 0x00ff0000) == 0x030000)
            endian_shift32[2] = 16;
        else
            endian_shift32[2] = 24;

        if((endian_int32 & 0x000000ff) == 0x04)
            endian_shift32[3] = 0;
        else if((endian_int32 & 0x0000ff00) == 0x0400)
            endian_shift32[3] = 8;
        else if((endian_int32 & 0x00ff0000) == 0x040000)
            endian_shift32[3] = 16;
        else
            endian_shift32[3] = 24;

        /* ok... new we test what we configured by shifting 0x04030201 to the
        * server endian - it MUST match our server template
        */
        LOG(LOG_MSG, "CHECK Endian 32bit:: we got %x we created read:%x send:%x\n", endian_int32, adjust_endian_int32(endian_int32), adjust_endian_int32(test32));
        if(endian_int32 != adjust_endian_int32(test32) || test32 != adjust_endian_int32(endian_int32))
            return 0; /* should NEVER happens */
    }
    return 1;
}


uint16 adjust_endian_int16(const uint16 buf)
{
	if(endian_do16)
	{
		static uint16 serv_int16;

		/* well, we just must reverse the nibbles */
		serv_int16 = ((buf>>8)&0x00ff) | ((buf<<8)&0xff00);
		return serv_int16;
	}

	return buf;
}

uint32 adjust_endian_int32(const uint32 buf)
{
	if(endian_do32)
	{
		static uint32 serv_int32;
		uint32 temp;
		int shift;


		temp = buf & 0x000000ff;
		temp <<= endian_shift32[0];
		serv_int32 = temp;

		temp = buf & 0x0000ff00;
		shift = endian_shift32[1] - 8;
		if(shift > 0)
			temp <<= shift;
		else
			temp >>= shift;
		serv_int32 |= temp;

		temp = buf & 0x00ff0000;
		shift = endian_shift32[2] - 16;
		if(shift > 0)
			temp <<= shift;
		else
			temp >>= shift;
		serv_int32 |= temp;

		temp = buf & 0xff000000;
		shift = endian_shift32[3] - 24;
		if(shift > 0)
			temp <<= shift;
		else
			temp >>= shift;
		serv_int32 |= temp;

		return serv_int32;
	}
	return buf;
}

/* removes whitespace from right side */
char * adjust_string(char *buf)
{
    int i, len = strlen(buf);

    for (i = len - 1; i >= 0; i--)
    {
        if (!isspace(buf[i]))
            return buf;

        buf[i] = 0;
    }
    return buf;
}

PHYSFS_File *load_client_file(const char *fname)
{
    PHYSFS_File *handle;

    /* Log what we're doing. */
    LOG(LOG_SYSTEM, "Loading '%s'... ", fname);

    /* If the file doesn't exist, that's OK. */
    if (!PHYSFS_exists(fname))
    {
        LOG(LOG_SYSTEM, "OK (But file does not exist)!\n");

        return NULL;
    }

    /* Open the file for reading.*/
    if (!(handle = PHYSFS_openRead(fname)))
    {
        LOG(LOG_ERROR, "FAILED (%s)!\n", PHYSFS_getLastError());
    }
    else
    {
        LOG(LOG_SYSTEM, "OK!\n");
    }

    return handle;
}

PHYSFS_File *save_client_file(const char *fname)
{
    PHYSFS_File *handle;

    /* Log what we're doing. */
    LOG(LOG_SYSTEM, "Saving '%s'... ", fname);

    /* Open the file for writing.*/
    if (!(handle = PHYSFS_openWrite(fname)))
    {
        LOG(LOG_ERROR, "FAILED (%s)!\n", PHYSFS_getLastError());
    }
    else
    {
#if 0 // TODO: Enable in 0.11.0
        PHYSFS_writeString(handle, HDR_CLIENT);
#endif
        LOG(LOG_SYSTEM, "OK!\n");
    }

    return handle;
}
