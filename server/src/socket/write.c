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

	The author can be reached via e-mail to info@daimonin.org
*/

#include <global.h>

static void Dequeue(sockbuf_struct *sb);

/* Dynamically request and setup a "global working" socket buffer for a socket/player
 * This functions also returns from the <cmd><length><data> part the pointer to data[0]
 * The buffer can be setup indirect by SockBuf_xxx functions increasing the buffers len
 * counter or direct by using the returned char ptr. In that case we must tell the
 * finish function the length of data we put in the buffer.
 * Note: data_len is a "guessed length" value for the buffer size - not the real package len
 */
char *socket_buffer_request(NewSocket *ns, int data_len)
{
	int 		header_len = (data_len <= 0xffff)
	                             ? SOCKBUF_HEADER_DEFAULT
	                             : SOCKBUF_HEADER_EXTENDED;
	sockbuf_struct *tmp = ns->sockbuf;

	/* lets check we already have a "working buffer" */
	if(!tmp)
		ns->sockbuf = socket_buffer_get(data_len + header_len);
	/* the buffer is already there... lets check there is enough space for data_len */
	else if(tmp->bufsize - tmp->len < data_len + header_len)
	{
		/* we must request a new buffer - lets throw the old one in our send queue */
        if(tmp->len)/* enqueue if there is anything in the buffer */
    		socket_buffer_enqueue(ns, tmp);
        else if(!tmp->queued && !(tmp->flags & SOCKBUF_FLAG_STATIC)) /* when not, release it! */
            return_poolchunk(tmp, tmp->pool);
        else /* this should not be happen - tell the devs */
            LOG(llevBug,"BUG: Blocked working socket_buffer not released in socket_buffer_request()\n");

		ns->sockbuf = socket_buffer_get(data_len + header_len);
	}
	tmp = ns->sockbuf;
#ifdef SEND_BUFFER_DEBUG
	LOG(llevDebug,"SOCKBUF: Requested buffer %p(%d) for %d + %d bytes\n", tmp, tmp->len, data_len, header_len);
#endif
	tmp->ns = ns; /* tell the sockbuf its direct connected to this socket */

	/* we first save the current buffer position - perhaps there is something already in */
	tmp->request_len = tmp->len;
	*(tmp->buf+tmp->len) = 0x00; /* ensure no flags are set */

	if(data_len > 0xffff) /* increase to 5 byte header and set cmd flag for it */
		*(tmp->buf+tmp->len) = 0x80;
	tmp->len+=header_len;

	return (char *)(tmp->buf+tmp->len); /* for direct access we return the <data> of this buffer part */
}

/* we want increase the len of sbuf but that will break the bufsize.
 * So, we must allocate a bigger one, copy the data and adjust all
 * pointer - as return ptr and for *one* socket where is buffer is
 * perhaps added as working bufffer
 */
sockbuf_struct *socket_buffer_adjust(sockbuf_struct *sbuf, int len)
{
	sockbuf_struct *tmp;
	int addmem = (4*1024); /* WHEN we are here at last then don't be to shy in memory questions */

	/* lets first check HOW big the chunk is we have to add */
	if(len >= (4*1024))
		addmem = len + (4*1024); /* huh... thats a big one?! */

	tmp = socket_buffer_get(sbuf->bufsize + addmem);

	memcpy(tmp->buf, sbuf->buf, sbuf->len);
	tmp->len = sbuf->len;
	tmp->request_len = sbuf->request_len;
	tmp->pos = sbuf->pos;
	tmp->queued = sbuf->queued;
	tmp->ns = sbuf->ns;
	tmp->flags = sbuf->flags;

#ifdef SEND_BUFFER_DEBUG
	LOG(llevDebug,"SOCKBUF: Adjusted buffer %p to %p from %d(%d) to %d bytes (%d)\n", sbuf, tmp, sbuf->bufsize, sbuf->len, tmp->bufsize, len);
#endif

	/* we remove the old buffer now - all is copied
	 * NOTE: Here is a good reason why we never ever should enqueue a socket
	 * or use it elsewhere when its still attached as working buffer
	 */
	return_poolchunk(sbuf, sbuf->pool);

	/* tricky stuff - insert the new buffer in the socket we are perhaps added as working buffer */
	if(tmp->ns)
		((NewSocket *)(tmp->ns))->sockbuf = tmp; /* a void * - i forgot how to make it ADT like... oops */

	return tmp;
}

/* reset the buffer to the stored ->request values
*/
void socket_buffer_request_reset(NewSocket *ns)
{
	sockbuf_struct *tmp = ns->sockbuf;

#ifdef SEND_BUFFER_DEBUG
	LOG(llevDebug,"SOCKBUF: Reseted buffer %p\n", tmp);
#endif
	tmp->len = tmp->request_len;
	tmp->request_len = SOCKBUF_DYNAMIC;
}

/* must be called after a socket_buffer_request() and will finish the package.
 * The caller had now written len bytes BY HAND to the ns->sockbuf buffer.
 * we adjust and finish the buffer. <len> must be exactly the package size
 */
void socket_buffer_request_finish(NewSocket *ns, int cmd, int len)
{
	sockbuf_struct *tmp = ns->sockbuf;
	int             header_len;

	/* sanity check: is there a valid buffer? */
	if (!tmp)
	{
		LOG(llevBug, "BUG: Called socket_buffer_add() without valid sockbuf\n");

		return;
	}

	/* some sanity checks - this bugs should only happens when testing & debugging */
	if (tmp->request_len == SOCKBUF_DYNAMIC ||
	    tmp->request_len >= tmp->len)
	{
		/* the buffer is really messed up... */
		LOG(llevBug, "BUG: Called socket_buffer_add() with invalid request buffer length len:%d rlen:%d\n",
		    tmp->len, tmp->request_len);

		return;
	}

	if (len == SOCKBUF_DYNAMIC)
	{
		/* lets calc now len - it was increased by the SockBuf_xx functions */
		len = SOCKBUF_REQUEST_BUFSIZE(tmp);

		if(len < 0)
		{
			LOG(llevBug, "Debug: Called socket_buffer_add() with invalid length len:%d rlen:%d\n",
			    tmp->len, tmp->request_len);
			tmp->len = tmp->request_len; /* its a try to restore a valid buffer */

			return;
		}
	}
	else
	{
		tmp->len += len;
	}

	if ((header_len = SOCKBUF_REQUEST_HDRSIZE(tmp)) == SOCKBUF_HEADER_EXTENDED)
	{
		*(tmp->buf + tmp->request_len) = cmd | 0x80;
		*((uint32*)(tmp->buf + tmp->request_len + 1)) = (uint32)len;
	}
	else // if (header_len == SOCKBUF_HEADER_DEFAULT)
	{
		*(tmp->buf + tmp->request_len) = cmd;
		*((uint16*)(tmp->buf + tmp->request_len + 1)) = (uint16)len;
	}

	tmp->request_len = SOCKBUF_DYNAMIC;
#ifdef SEND_BUFFER_DEBUG
	LOG(llevDebug, "SOCKBUF: Finish buffer %p for cmd %d with %d + %d bytes%s\n",
	    tmp, cmd, header_len, len, (len == SOCKBUF_DYNAMIC) ? " (dynamic)" : "");
#endif
}

/* setup the socket buffers (using mempool) */
sockbuf_struct *socket_buffer_get(int len)
{
	sockbuf_struct *tmp;

	if(len <=248)
	{
		tmp = get_poolchunk(pool_sockbuf_small);
		tmp->pool = pool_sockbuf_small;
		tmp->bufsize = 256;
	}
	else if(len <=2040)
	{
		tmp = get_poolchunk(pool_sockbuf_medium);
		tmp->pool = pool_sockbuf_medium;
		tmp->bufsize = 2048;
	}
	else if(len <=4088)
	{
		tmp = get_poolchunk(pool_sockbuf_huge);
		tmp->pool = pool_sockbuf_huge;
		tmp->bufsize = 4096;
	}
	else /* get a dynmic buffer - report this so we can adjust the buffers */
	{
		tmp = get_poolchunk(pool_sockbuf_dynamic);
		tmp->pool = pool_sockbuf_dynamic;
		tmp->bufsize = (((len/256)+1)*256)+256;
		tmp->buf = malloc(tmp->bufsize);
		if(!tmp->buf)
			LOG(llevError, "dynamic sockbuf: malloc returned zero for %d(%d) bytes\n", len, tmp->bufsize);
#ifdef SEND_BUFFER_DEBUG
		LOG(llevDebug, "Allocated oversized dynamic sockbuf (%p) for: %d(%d) bytes\n", tmp, len, tmp->bufsize);
#endif
	}
	tmp->ns = NULL;
	tmp->last = tmp->next = tmp->broadcast = NULL;
	tmp->request_len = tmp->queued = tmp->len = tmp->pos = tmp->flags = 0;

	return(tmp);
}


/* setup the data in the right way for the client in format
* <cmd><length of data><data>
* We have to take care about the fact that we don't know for most
* buffer how big they will become when we start to create them.
* To avoid double data copying, we collect the data right in our socket buffer
* and write then the length value back.
* if length is >0xffff use 4 bytes instead of 2 for <length of data>
* and set the high bit of cmd
* paramter:
* data_len >= 0: copy data_len bytes from data to sb (binary data or string collection)
* data_len == -1: data is a C string ('\0' terminated), use strlen() to get length
*/
sockbuf_struct *compose_socklist_buffer(int cmd, char *data, int data_len, int flags)
{
	int 		header_len;
	sockbuf_struct *sb = get_poolchunk(pool_sockbuf_broadcast);

	/* Calculate data and header lengths. */
	if (data_len == SOCKBUF_DYNAMIC)
	{
		data_len = strlen(data) + 1;
	}

	header_len = (data_len > 0xffff)
	             ? SOCKBUF_HEADER_EXTENDED : SOCKBUF_HEADER_DEFAULT;

	/* Allocate sockbuf. */
	sb->pool = pool_sockbuf_broadcast;
	MALLOC(sb->buf, header_len + data_len);
#ifdef SEND_BUFFER_DEBUG
	LOG(llevDebug, "SOCKBUF: Composed broadcast sockbuf (%p) of %d + %d bytes\n",
	    sb, header_len, data_len);
#endif
	sb->ns = NULL;
	sb->last = sb->next = sb->broadcast = NULL;
	sb->request_len = sb->queued = sb->pos = 0;
	sb->bufsize = sb->len = header_len + data_len;
	sb->flags = flags;

	/* Setup header. */
	if (header_len == SOCKBUF_HEADER_EXTENDED)
	{
		*sb->buf = (uint8)cmd | 0x80;
		*((uint32*)(sb->buf + 1)) = (uint32)data_len;
	}
	else // if (header_len == SOCKBUF_HEADER_DEFAULT)
	{
		*sb->buf = (uint8)cmd;
		*((uint16*)(sb->buf + 1)) = (uint16)data_len;
	}

	/* Copy data. */
	memcpy(sb->buf + header_len, data, data_len);

	return sb;
}

/* attach socket buffer to outgoing list of this ns socket
 * Its a FIFO list, start means here "newest"
 */
void socket_buffer_enqueue(NewSocket *ns, sockbuf_struct *sb)
{
	sockbuf_struct *sockbufptr = sb;

	if (!sb)
	{
		return;
	}

	/* When we enqueue a sockbuf we are going to write values to its next
	 * and last fields in order to create a linked list of sockbufs in an
	 * individual player's socket. This obviously chains the sockbuf to
	 * that particular player/socket. This is contrary to the whole idea of
	 * broadcast sockbufs which are 'free floating' and accessible by
	 * multiple players/sockets. To address this, when we ask to enqueue a
	 * broadcast sockbuf the following creates a 'dummy' which points to
	 * the real broadcast. It is this dummy which is chained, leaving the
	 * real broadcast (where the actual data is) free floating. */
	if (sb->pool == pool_sockbuf_broadcast)
	{
		sockbufptr = get_poolchunk(pool_sockbuf_broadcast);
		sockbufptr->broadcast = sb;
		sockbufptr->pool = pool_sockbuf_broadcast;
		sockbufptr->pos = sb->pos;
		sockbufptr->len = sb->len;

		/* Bump the real broadcast instance. */
		sb->queued++;
	}
	else
	{
		sockbufptr->ns = NULL;
		ns->sockbuf = NULL;
	}

	if (ns->sockbuf_start)
	{
		ns->sockbuf_start->last = sockbufptr;
		sockbufptr->next = ns->sockbuf_start;
		sockbufptr->last = NULL;
		ns->sockbuf_start = sockbufptr;
	}
	else
	{
		ns->sockbuf_end = ns->sockbuf_start = sockbufptr;
		sockbufptr->next = sockbufptr->last = NULL;
		/* this buffer is also the next for the socket to write */
		ns->sockbuf_pos = sockbufptr->pos;
		ns->sockbuf_len = sockbufptr->len;
	}

	ns->sockbuf_bytes += sockbufptr->len;
	ns->sockbuf_nrof++;

	/* Bump the dummy broadcast OR actual working instance. */
	sockbufptr->queued++;
}

/* NOTE: we have a FIFO queue here - we remove from the end
 * We just check there is something at the end of the queue -
 * if so this must be the one we want remove
 */
void socket_buffer_dequeue(NewSocket *ns)
{
	sockbuf_struct *tmp = ns->sockbuf_end;

	if (tmp)
	{
		if ((ns->sockbuf_end = ns->sockbuf_end->last))
		{
			ns->sockbuf_end->next = NULL;
			/* most important settings so we can send the buffers data */
			ns->sockbuf_pos = ns->sockbuf_end->pos;
			ns->sockbuf_len = ns->sockbuf_end->len;
		}
		else /* the removed buffer is also start */
		{
			ns->sockbuf_start = NULL;
		}

		ns->sockbuf_bytes -= tmp->len;
		ns->sockbuf_nrof--;

		/* When we dequeue a dummy broadcast, we first need to do the 
		 * real broadcast. */
		if (tmp->broadcast)
		{
			Dequeue(tmp->broadcast);
		}

		/* Now dequeue the dummy broadcast OR working sockbuf. */
		Dequeue(tmp);
	}
}

static void Dequeue(sockbuf_struct *sb)
{
	if (--sb->queued < 0)
	{
		LOG(llevBug, "BUG:: %s:Dequeue(): %p instance < 0 (%d)!\n",
		    __FILE__, sb, sb->queued);
	}

	if (!sb->queued &&
	    !(sb->flags & SOCKBUF_FLAG_STATIC))
	{
		return_poolchunk(sb, sb->pool);
	}
}

/* clear a socket buffer queue and release all socket buffers to the mempool */
void socket_buffer_queue_clear(NewSocket *ns)
{
	while (ns->sockbuf_end)
		socket_buffer_dequeue(ns);
}

/* init functions called from mempool */
void initialize_socket_buffer_small(sockbuf_struct *sockbuf)
{
	sockbuf->buf = malloc(256);
	sockbuf->queued = 0;
}
void initialize_socket_buffer_medium(sockbuf_struct *sockbuf)
{
	sockbuf->buf = malloc(2048);
	sockbuf->queued = 0;
}
void initialize_socket_buffer_huge(sockbuf_struct *sockbuf)
{
	sockbuf->buf = malloc(4096);
	sockbuf->queued = 0;
}
void initialize_socket_buffer_dynamic(sockbuf_struct *sockbuf)
{
	sockbuf->buf = NULL;
	sockbuf->queued = 0;
}
void initialize_socket_buffer_broadcast(sockbuf_struct *sockbuf)
{
	sockbuf->buf = NULL;
	sockbuf->queued = 0;
}

/* destructor called from mempool */
void free_socket_buffer_dynamic(sockbuf_struct* sb)
{
	FREE(sb->buf);
	sb->queued = 0;
}

void free_socket_buffer_broadcast(sockbuf_struct* sb)
{
	FREE(sb->buf);
	sb->queued = 0;
}
