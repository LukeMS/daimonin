/*
    Daimonin, the Massive Multiuser Online Role Playing Game
    Server Applicatiom

    Copyright (C) 2001-2008 Michael Toennies

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

#ifndef WIN32 /* ---win32 exclude unix headers */
#include <sys/socket.h>
#endif /* end win32 */


/* low level read from socket. This function don't knows about packages.
* It handles streams.
*/
int read_socket_buffer(NewSocket *ns)
{
	ReadList   *sl  = &ns->readbuf;
	int         stat_ret, read_bytes, tmp;

	if(ns->status == Ns_Zombie) /* zombie clients don't read anything */
		return 0;

	/* calculate how many bytes can be read in one row in our round robin buffer */
	tmp = sl->pos+sl->len;

	/* we have still some bytes until we hit our buffer border ?*/
	if(tmp >= MAXSOCKBUF_IN)
	{
		tmp = tmp-MAXSOCKBUF_IN; /* thats our start offset */
		read_bytes = sl->pos - tmp; /* thats our free buffer until ->pos*/
	}
	else
		read_bytes = MAXSOCKBUF_IN-tmp; /* tmp is our offset and there is still a bit to read in */

	/* with this settings we can adjust in a hard way the maximum bytes read per round per socket */
	/*
	if(read_bytes >256)
	read_bytes = 256;
	*/

#ifdef WIN32
	stat_ret = recv(ns->fd, sl->buf + tmp, read_bytes, 0);
#else
	stat_ret = read(ns->fd, sl->buf + tmp, read_bytes);
#endif

	LOG(-1,"READ(%d)(%d): %d\n", ROUND_TAG, ns->fd, stat_ret);

	if (stat_ret > 0)
		sl->len += stat_ret;
	else if (stat_ret < 0) /* lets check its a real problem */
	{
#ifdef WIN32
		if (WSAGetLastError() == WSAEWOULDBLOCK)
			return 1;

		if (WSAGetLastError() == WSAECONNRESET)
			LOG(llevDebug, "Connection closed by client\n");
		else
			LOG(llevDebug, "ReadPacket got error %d, returning 0\n", WSAGetLastError());
#else
		if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
			return 1;

		LOG(llevDebug, "ReadPacket got error %d (%s), returning 0\n", errno, strerror_local(errno));
#endif
	}
	else
		return -1; /* return value of zero means closed socket */

	return stat_ret;
}

/* When the socket is clear to write, and we have backlogged data, this
* is called to write it out.
*/
void write_socket_buffer(NewSocket *ns)
{
	int amt, max = -1;

	/* sanity check: lets see we have have something to send */
	while(ns->sockbuf_end)
	{
		max = ns->sockbuf_len - ns->sockbuf_pos;
		if(max > 0) /* break if we have something here */
			break;
        LOG(llevDebug,"WriteSockbuf with no data. skipped. fd:%d (%d :: %d)\n", ns->fd, ns->sockbuf_len, ns->sockbuf_pos);
		socket_buffer_dequeue(ns);
	}

	if(max <= 0) /* nothing in the queue - lets check we have working buffer we can request! */
	{
		/* is there something in our working buffer? */
		if(!ns->sockbuf || !ns->sockbuf->len)
        {
          //  LOG(llevDebug,"WriteSockbuf - nothing\n"); 
            return; /* there is really nothing to do! */
        }
//        LOG(llevDebug,"write:  using workbuffer\n"); 
		socket_buffer_enqueue(ns, ns->sockbuf);
		ns->sockbuf = NULL;
		max = ns->sockbuf_len - ns->sockbuf_pos; /* its a fresh buffer, pos MUST be zero */
	}

	/* with this settings we can adjust in a hard way the maximum bytes written per round per socket */
	/*
	if(max >256)
	max = 256;
	*/

	amt = send(ns->fd, ns->sockbuf_end->buf + ns->sockbuf_pos, max, MSG_DONTWAIT);
//	LOG(-1,"WRITE(%d)(%d): %d (%d)\n", ROUND_TAG, ns->fd, amt, max);

	/* following this link: http://www-128.ibm.com/developerworks/linux/library/l-sockpit/#N1019D
	* send() with MSG_DONTWAIT under linux can return 0 which means the data
	* is "queued for transmission". I was not able to find that in the send() man pages...
	* In my testings it never happend, so i put it here in to have it perhaps triggered in
	* some server runs (but we should trust perhaps ibm developer infos...).
	*/
#ifndef WIN32 /* linux only ATM */
	if(!amt)
	{
		LOG(llevDebug,"IMPORTANT: send() in write_socket_buffer() returned ZERO! check the comment text in loop.c around line 200. (max: %d)\n", max);
		amt = max; /* as i understand, the data is now internal buffered? So remove it from our write buffer */
	}
	else
#endif

		if (amt < 0) /* error */
		{
#ifdef WIN32 /* ***win32 write_socket_buffer: change error handling */
			if (WSAGetLastError() == WSAEWOULDBLOCK)
				return;

			LOG(llevDebug, "New socket write failed (wsb) (%d).\n", WSAGetLastError());
#else
			if (errno == EWOULDBLOCK || errno == EINTR)
				return;
			LOG(llevDebug, "New socket write failed (wsb %d) (%d: %s).\n", EAGAIN, errno, strerror_local(errno));
#endif
			ns->status = Ns_Dead;
			return;
		}
		ns->sockbuf_pos += amt;
		if(ns->sockbuf_len - ns->sockbuf_pos == 0) /* nothing left? release then this buffer */
			socket_buffer_dequeue(ns);
}
