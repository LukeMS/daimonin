/*
    Daimonin, the Massive Multiuser Online Role Playing Game
    Server Applicatiom

    Copyright (C) 2001 Michael Toennies

    A split from Crossfire, a Multiplayer game for X-windows.

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

/* newsocket.c contains some base functions that both the client and server
 * can use.  As such, depending what we are being compiled for will
 * determine what we can include.  the client is designed have
 * CFCLIENT defined as part of its compile flags.
 */

#include <global.h>

/***********************************************************************
 *
 * SockList functions/utilities
 *
 **********************************************************************/

/* add a 0 terminated string 
 * len is the string length we copy
 * if len is zero, we copy until we find '\0' as string terminator 
 */
void SockList_AddString(SockList *sl, const char *data, int len)
{
	if(len)
	{
		memcpy(sl->buf+sl->len,data,len);
		sl->buf[len+sl->len++] = 0;
	}
	else
	{
		register char    c;

		while ((c = *data++))
			sl->buf[sl->len++] = c;
		sl->buf[sl->len++] = 0;
	}
}


/*******************************************************************************
 *
 * Start of write related routines.
 *
 ******************************************************************************/


/* Takes a string of data, and writes it out to the socket. A very handy
* shortcut function.
*/
void Write_String_To_Socket(NewSocket *ns, char cmd, char *buf, int len)
{
    SockList    sl;

    sl.len = len;
    sl.buf = (uint8 *) buf;
    *((char *) buf) = cmd;
    Send_With_Handling(ns, &sl);
}

/* Send With Handling - calls Write_To_Socket to send data to the client.
* The only difference in this function is that we take a SockList
* and we prepend the length information.
*/
void Send_With_Handling  (NewSocket *ns, SockList *msg)
{
    unsigned char sbuf[4];

    if (ns->status == Ns_Dead || !msg)
        return;

    /* Almost certainly we've overflowed a buffer, so quite now to make
    * it easier to debug.
    */
    if (msg->len >= MAXSOCKBUF)
        LOG(llevError, "Trying to send a buffer beyond properly size, len =%d\n", msg->len);

    if(msg->len > 32*1024-1) /* if > 32kb use 3 bytes header and set the high bit to show it client */
    {
        sbuf[0] = ((uint32) (msg->len) >> 16) & 0xFF;
        sbuf[0] |= 0x80; /* high bit marker for the client */
        sbuf[1] = ((uint32) (msg->len) >> 8) & 0xFF;
        sbuf[2] = ((uint32) (msg->len)) & 0xFF;
        Write_To_Socket(ns, sbuf, 3);
    }
    else
    {
        sbuf[0] = ((uint32) (msg->len) >> 8) & 0xFF;
        sbuf[1] = ((uint32) (msg->len)) & 0xFF;
        Write_To_Socket(ns, sbuf, 2);
    }
    Write_To_Socket(ns, msg->buf, msg->len);
}

/* This function will write to our socket buffer but not the "real* OS socket */
void    Write_To_Socket (NewSocket *ns, unsigned char *buf, int len)
{
    int avail, end;

    if ((len + ns->outputbuffer.len) > MAXSOCKBUF)
    {
        LOG(llevDebug, "Socket host %s has overrun internal buffer - marking as dead (bl:%d l:%d)\n",
            STRING_SAFE(ns->ip_host), ns->outputbuffer.len, len);
        ns->status = Ns_Dead;
        return;
    }

    /* data + end is where we start putting the new data.  The last byte
    * currently in use is actually data + end -1
    */
    end = ns->outputbuffer.start + ns->outputbuffer.len;
    /* The buffer is already in a wrapped state, so adjust end */
    if (end >= MAXSOCKBUF)
        end -= MAXSOCKBUF;
    avail = MAXSOCKBUF - end;

    /* We can all fit it behind the current data without wrapping */
    if (avail >= len)
        memcpy(ns->outputbuffer.data + end, buf, len);
    else
    {
        memcpy(ns->outputbuffer.data + end, buf, avail);
        memcpy(ns->outputbuffer.data, buf + avail, len - avail);
    }
    ns->outputbuffer.len += len;
}
