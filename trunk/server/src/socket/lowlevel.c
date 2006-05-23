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

    The author can be reached via e-mail to daimonin@nord-com.net
*/
/* newsocket.c contains some base functions that both the client and server
 * can use.  As such, depending what we are being compiled for will
 * determine what we can include.  the client is designed have
 * CFCLIENT defined as part of its compile flags.
 */

#include <global.h>
#ifdef NO_ERRNO_H
extern int  errno;
#else
#   include <errno.h>
#endif

/***********************************************************************
 *
 * SockList functions/utilities
 *
 **********************************************************************/

/* add a 0 terminated string */
void SockList_AddString(SockList *sl, char *data)
{
    char    c;

    while ((c = *data++))
    {
        sl->buf[sl->len] = c;
        sl->len++;
    };
    sl->buf[sl->len] = c;
    sl->len++;
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

	sbuf[0] = ((uint32) (msg->len) >> 8) & 0xFF;
	sbuf[1] = ((uint32) (msg->len)) & 0xFF;
	Write_To_Socket(ns, sbuf, 2);
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

/******************************************************************************
 *
 * statistics logging functions.
 *
 ******************************************************************************/

#ifdef CS_LOGSTATS
/* cst_tot is for the life of the server, cst_last is for the last series of
 * stats
 */

/* Writes out the gathered stats.  We clear cst_lst.
 */
void write_cs_stats()
{
    time_t  now = time(NULL);

    /* If no connections recently, don't both to log anything */
    if (cst_lst.ibytes == 0 && cst_lst.obytes == 0)
        return;

    /* CSSTAT is put in so scripts can easily find the line */
    LOG(llevInfo, "CSSTAT: %.16s tot in:%d out:%d maxc:%d time:%d last block-> in:%d out:%d maxc:%d time:%d\n",
        ctime(&now), cst_tot.ibytes, cst_tot.obytes, cst_tot.max_conn, now - cst_tot.time_start, cst_lst.ibytes,
        cst_lst.obytes, cst_lst.max_conn, now - cst_lst.time_start);

    LOG(llevInfo, "SYSINFO: objs: %d allocated, %d free, arch-srh:%d (%d cmp)\n", pool_object->nrof_allocated,
        pool_object->nrof_free, arch_search, arch_cmp);

    cst_lst.ibytes = 0;
    cst_lst.obytes = 0;
    cst_lst.max_conn = socket_info.nconns;
    cst_lst.time_start = now;
}
#endif
