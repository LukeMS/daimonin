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

/* changed to macros in define.h */
/*
#define SockList_AddChar(_sl_,_c_)    _sl_->buf[_sl_->len++]=_c_;

#define GetInt_String(_data_) ((_data_[0]<<24) + (_data_[1]<<16) + (_data_[2]<<8) + _data_[3])
#define GetShort_String(_data_) ((_data_[0]<<8)+_data_[1])
*/

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

/******************************************************************************
 *
 * Start of read routines.
 *
 ******************************************************************************/


/* TEMP. FUNCTIONS TO SIMULATE A NEW SOCKET - THIS IS JUST A QUICK HACK TO
 * TEST THE read packet function. Code is in the style of education source.
 */

/* When called, we fill sl1 with a command. 
 * If return is 1, we have a valid command.
 * 0 means we got not a valid.
 */
int socket_read_pp(SockList *sl1, SockList *sl, int len)
{
    int toread, ret = 0;

    sl1->buf[0] = 0;
    sl1->len = 0;

    if (sl->len >= 2) /* there is something in our in buffer we had read before */
    {
        toread = 2 + (sl->buf[0] << 8) + sl->buf[1]; /* len of the command */

        if (toread <= sl->len) /* if we have a command, copy it *now* */
        {
            memcpy(sl1->buf, sl->buf, toread);
            sl1->len = toread;
            if (sl->len - toread)
                memcpy(sl->buf, sl->buf + toread, sl->len - toread);
            sl->len -= toread;
            ret = toread;
        }
    }

    return ret;
}


/* New ReadPacket function.
 * This is the only place we read from the TCP/IP socket.
 * We read in 0-x commands at once - as much we can get with one
 * read but not more as our buffer can get.
 * The command processing will take care about incomplete or damaged
 * commands.
 */
int SockList_ReadPacket(NewSocket *ns, int len)
{
    SockList   *sl  = &ns->readbuf;
    int         stat_ret;

#ifdef WIN32
    stat_ret = recv(ns->fd, sl->buf + sl->len, len - sl->len, 0);
#else
    do
    {
        stat_ret = read(ns->fd, sl->buf + sl->len, len - sl->len);
    }
    while (stat_ret < 0 && errno == EINTR);
#endif

    /*LOG(-1,"STAT: %d (%d)\n", stat_ret, sl->len);*/

    if (stat_ret > 0)
    {
        sl->len += stat_ret;
#ifdef CS_LOGSTATS
        cst_tot.ibytes += stat_ret;
        cst_lst.ibytes += stat_ret;
#endif
    }
    else if (stat_ret < 0) /* lets check its a real problem */
    {
#ifdef WIN32
        if (WSAGetLastError() != WSAEWOULDBLOCK)
        {
            if (WSAGetLastError() == WSAECONNRESET)
                LOG(llevDebug, "Connection closed by client\n");
            else
                LOG(llevDebug, "ReadPacket got error %d, returning 0\n", WSAGetLastError());        
            return stat_ret;
        }
#else
        if (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK)
        {
            LOG(llevDebug, "ReadPacket got error %d, returning 0\n", errno);
            return stat_ret;
        }   
#endif
        return 1;
    } 

    return stat_ret;
}

/*******************************************************************************
 *
 * Start of write related routines.
 *
 ******************************************************************************/

/* Adds data to a socket buffer for whatever reason.
 * ns is the socket we are adding the data to, buf is the start of the
 * data, and len is the number of bytes to add.
 */

static void add_to_buffer(NewSocket *ns, unsigned char *buf, int len)
{
    int avail, end;

    /*LOG(llevDebug,"l: %d : b: %d\n", len, ns->outputbuffer.len);*/

    if ((len + ns->outputbuffer.len) > MAXSOCKBUF)
    {
        LOG(llevDebug, "Socket host %s has overrun internal buffer - marking as dead\n", STRING_SAFE(ns->host));
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
#if 0
    LOG(llevDebug,"Added %d to output buffer, total length now %d, start=%d\n", len,
    ns->outputbuffer.len, ns->outputbuffer.start);
#endif
}

/* When the socket is clear to write, and we have backlogged data, this
 * is called to write it out.
 */
void write_socket_buffer(NewSocket *ns)
{
    int amt, max;

    if (ns->outputbuffer.len == 0)
        return;

    do
    {
        max = MAXSOCKBUF - ns->outputbuffer.start;
        if (ns->outputbuffer.len < max)
            max = ns->outputbuffer.len;

#ifdef WIN32
        amt = send(ns->fd, ns->outputbuffer.data + ns->outputbuffer.start, max, 0);
#else
        do
        {
            amt = write(ns->fd, ns->outputbuffer.data + ns->outputbuffer.start, max);
        }
        while ((amt < 0) && (errno == EINTR));
#endif

        if (amt < 0) /* error */
        {
#ifdef WIN32 /* ***win32 write_socket_buffer: change error handling */
            if (WSAGetLastError() != WSAEWOULDBLOCK)
            {
                LOG(llevDebug, "New socket write failed (wsb) (%d).\n", WSAGetLastError());
#else
                if (errno != EWOULDBLOCK)
                {
                    LOG(llevDebug, "New socket write failed (wsb) (%d: %s).\n", errno, strerror_local(errno));
#endif
                    ns->status = Ns_Dead;
                    return;
                }
                else /* EWOULDBLOCK  - we can't write because socket is busy */
                {
                    ns->can_write = 0;
                    return;
                }
            }
            ns->outputbuffer.start += amt;
            /* wrap back to start of buffer */
            if (ns->outputbuffer.start == MAXSOCKBUF)
                ns->outputbuffer.start = 0;
            ns->outputbuffer.len -= amt;
#ifdef CS_LOGSTATS
            cst_tot.obytes += amt;
            cst_lst.obytes += amt;
#endif
        } 
        while (ns->outputbuffer.len > 0);
    }

    /* This writes data to the socket. - It is very low level -
     * all we try and do is write out the data to the socket
     * provided (ns).  buf is the data to write, len is the number
     * of bytes to write.  IT doesn't return anything - rather, it
     * updates the ns structure if we get an  error.
     */
    void    Write_To_Socket (NewSocket *ns, unsigned char *buf, int len)
    {
        int amt = 0;
    unsigned char * pos = buf;

    if (ns->status == Ns_Dead || !buf)
    {
        LOG(llevDebug, "Write_To_Socket called with dead socket\n");
        return;
    }
    if (!ns->can_write)
    {
        add_to_buffer(ns, buf, len);
        return;
    }
    /* If we manage to write more than we wanted, take it as a bonus */
    while (len > 0)
    {
#ifdef WIN32 /* ***win32 Write_To_Socket: change write() to send() */
        amt = send(ns->fd, pos, len, 0);
#else
        do
        {
            amt = write(ns->fd, pos, len);
        }
        while ((amt < 0) && (errno == EINTR));
#endif

        if (amt < 0)
        {
            /* We got an error */
#ifdef WIN32 /* ***win32 Write_To_Socket: change error handling */
            if (amt == -1 && WSAGetLastError() != WSAEWOULDBLOCK)
            {
                LOG(llevDebug, "New socket write failed WTS (%d).\n", WSAGetLastError());
#else
                if (errno != EWOULDBLOCK)
                {
                    LOG(llevDebug, "New socket write failed WTS (%d: %s).\n", /* ---WIN32 */
            errno, strerror_local(errno));
#endif
                    ns->status = Ns_Dead;
                    return;
                }
                else
                {
                    /* EWOULDBLOCK */
                    /* can't write it, so store it away. */
                    add_to_buffer(ns, pos, len);
                    ns->can_write = 0;
                    return;
                }
            }
            /* amt gets set to 0 above in blocking code, so we do this as
             * an else if to make sure we don't reprocess it.
             */
            else if (amt == 0)
            {
                LOG(llevDebug, "Write_To_Socket: No data written out.\n");
            }
            len -= amt;
            pos += amt;
#ifdef CS_LOGSTATS
            cst_tot.obytes += amt;
            cst_lst.obytes += amt;
#endif
        }
    }

    /* Send With Handling - calls Write_To_Socket to send data to the client.
     * The only difference in this function is that we take a SockList
     *, and we prepend the length information.
     */
    void    Send_With_Handling  (NewSocket *ns, SockList *msg)
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
