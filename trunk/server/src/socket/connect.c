/*
    Daimonin, the Massive Multiuser Online Role Playing Game
    Server Applicatiom

    Copyright (C) 2001-2007 Michael Toennies

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

#include <global.h>
#ifndef WIN32 /* ---win32 exclude include files */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <sys/stat.h>
#include <stdio.h>
#endif /* !win32 */

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#include <newserver.h>

/* Initializes a connection - really, it just sets up the data structure,
 * socket setup is handled elsewhere.  We do send a version to the
 * client.
 */
void InitConnection(NewSocket *ns, char *ip)
{
    int oldbufsize;
    unsigned int buflen  = sizeof(int);

#ifdef WIN32 /* ***WIN32 SOCKET: init win32 non blocking socket */
    u_long temp = 1;

    if (ioctlsocket(ns->fd, FIONBIO, &temp) == -1)
        LOG(llevDebug, "InitConnection:  Error on ioctlsocket.\n");
#else
    LOG(llevDebug, "InitConnection:  fcntl(%x %x) %x.\n", O_NDELAY, O_NONBLOCK, fcntl(ns->fd, F_GETFL));
    if (fcntl(ns->fd, F_SETFL, fcntl(ns->fd, F_GETFL) | O_NDELAY | O_NONBLOCK ) == -1)
        LOG(llevError, "InitConnection:  Error on fcntl %x.\n", fcntl(ns->fd, F_GETFL));
    LOG(llevDebug, "InitConnection:  fcntl %x.\n", fcntl(ns->fd, F_GETFL));
#endif /* end win32 */

#ifdef ESRV_DEBUG
    getsockopt(ns->fd, SOL_SOCKET, SO_SNDBUF, (char *) &oldbufsize, &buflen);
    LOG(llevDebug, "InitConnection: Socket send buffer size is %d bytes\n", oldbufsize);
    getsockopt(ns->fd, SOL_SOCKET, SO_RCVBUF, (char *) &oldbufsize, &buflen);
    LOG(llevDebug, "InitConnection: Socket read buffer size is %d bytes\n", oldbufsize);
#endif

    ns->login_count = ROUND_TAG + pticks_socket_idle;
    ns->idle_flag = 0;
    ns->addme = 0;
    ns->image2 = 0;
    ns->sound = 0;
    ns->ext_title_flag = 1;
    ns->mapx = 17;
    ns->mapy = 17;
    ns->mapx_2 = 8;
    ns->mapy_2 = 8;
    ns->setup = 0;
    ns->rf_settings = 0;
    ns->rf_skills = 0;
    ns->rf_spells = 0;
    ns->rf_anims = 0;
    ns->rf_bmaps = 0;

	ns->cmd_start = ns->cmd_end = NULL;
	ns->sockbuf_start = ns->sockbuf_end = ns->sockbuf = NULL;
	ns->sockbuf_nrof = ns->sockbuf_bytes = ns->sockbuf_pos =ns->sockbuf_len = 0;

	/* we should really do some checking here - if total clients overflows
     * we need to do something more intelligent, because client id's will start
     * duplicating (not likely in normal cases, but malicous attacks that
     * just open and close connections could get this total up.
     */
    ns->readbuf.len = 0;
    ns->readbuf.pos = 0;
    ns->readbuf.toread = 0; /* imnportant, marks readbuf.cmd as invalid! */
    if(!ns->readbuf.buf)
        ns->readbuf.buf = malloc(MAXSOCKBUF_IN);
    ns->readbuf.buf[0] = 0;

    ns->pwd_try=0;

    memset(&ns->lastmap, 0, sizeof(struct Map));

    strcpy(ns->ip_host, ip);

    socket_info.nconns++;
}


/* basically, all we need to do here is free all data structures that
 * might be associated with the socket.  It is up to the caller to
 * update the list
 */

void close_newsocket(NewSocket *ns)
{
#ifdef WIN32
    WSAAsyncSelect(ns->fd, NULL, 0, FD_CLOSE);
    shutdown(ns->fd, SD_SEND);
    if (closesocket(ns->fd))
#else
    if (close(ns->fd))
#endif
    {
#ifdef ESRV_DEBUG
        LOG(llevDebug, "Error closing socket %d\n", ns->fd);
#endif
    }
}

void free_newsocket(NewSocket *ns)
{
    unsigned char *tmp_read = ns->readbuf.buf;

    LOG(llevDebug, "Closing socket %d\n", ns->fd);
    close_newsocket(ns);

    /* clearout the socket but don't restore the buffers.
     * no need to malloc them again & again.
     */
    command_buffer_queue_clear(ns); /* give back the blocks to the mempools */
	/* flush the write buffers and free them */
    if(ns->sockbuf)
    {
        ns->sockbuf->len = ns->sockbuf->pos = 1; /* avoid clear buffer warning */
        socket_buffer_enqueue(ns, ns->sockbuf);
    }
    socket_buffer_queue_clear(ns);
    memset(ns, 0, sizeof(ns));
    ns->readbuf.buf = tmp_read;
}

