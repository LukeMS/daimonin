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

/* Dynamically request and setup a "global working" socket buffer for a socket/player
 * This functions also returns from the <cmd><length><data> part the pointer to data[0]
 * The buffer can be setup indirect by SockBuf_xxx functions increasing the buffers len
 * counter or direct by using the returned char ptr. In that case we must tell the
 * finish function the length of data we put in the buffer.
 * Note: data_len is a "guessed length" value for the buffer size - not the real package len
 */
char *socket_buffer_request(NewSocket *ns, int data_len)
{
	int 		header_len = 3;
	sockbuf_struct *tmp = ns->sockbuf;

	/* lets adjust data_len by a the header. */
	if(data_len > 0xffff)
		header_len = 5;

	/* lets check we already have a "working buffer" */
	if(!tmp)
		ns->sockbuf = socket_buffer_get(data_len + header_len);
	/* the buffer is already there... lets check there is enough space for data_len */
	else if(tmp->bufsize < data_len + header_len) 
	{
		/* we must request a new buffer - lets throw the old one in our send queue */
		socket_buffer_enqueue(ns, tmp);
		ns->sockbuf = socket_buffer_get(data_len + header_len);
	}	
	tmp = ns->sockbuf;
	tmp->ns = ns; /* tell the sockbuf its direct connected to this socket */

	/* we first save the current buffer position - perhaps there is something already in */
	tmp->request_len = tmp->len;
	*(tmp->buf+tmp->len) = 0x00; /* ensure no flags are set */

	if(data_len > 0xffff) /* increase to 5 byte header and set cmd flag for it */
		*(tmp->buf+tmp->len) = 0x80;
	tmp->len+=header_len;

#ifdef SEND_BUFFER_DEBUG
	LOG(llevDebug,"SOCKBUF: Requested buffer %x for %d + %d bytes\n", tmp, data_len, header_len);
#endif
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
	tmp->instance = sbuf->instance;
	tmp->ns = sbuf->ns;
	tmp->flags = sbuf->flags;

#ifdef SEND_BUFFER_DEBUG
	LOG(llevDebug,"SOCKBUF: Adjusted buffer %x to %x from %d to %d bytes (%d)\n", sbuf, tmp, sbuf->bufsize, tmp->bufsize, len);
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
	LOG(llevDebug,"SOCKBUF: Reseted buffer %x\n", tmp);
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

	/* sanity check: is there a valid buffer? */
	if(!tmp)
	{
		LOG(llevBug,"BUG: Called socket_buffer_add() without valid sockbuf\n");
		return;
	}

	if(len == SOCKBUF_DYNAMIC)
	{
		int header_len;

		/* some sanity checks - this bugs should only happens when testing & debugging */
		if(tmp->request_len == SOCKBUF_DYNAMIC || tmp->request_len >= tmp->len)
		{
			/* the buffer is really messed up... */
			LOG(llevBug,"BUG: Called socket_buffer_add() with invalid request buffer length len:%d rlen:%d\n", 
				tmp->len, tmp->request_len);
			return;
		}

		/* lets calc now len - it was increased by the SockBuf_xx functions */
		header_len = ((*(char *)(tmp->buf+tmp->request_len))&0x80)?SOCKBUF_HEADER_EXTENDED:SOCKBUF_HEADER_DEFAULT;
		len = tmp->len - tmp->request_len - header_len;

		if(len < 0)
		{
			LOG(llevBug,"Debug: Called socket_buffer_add() with invalid length len:%d rlen:%d\n", 
				tmp->len, tmp->request_len);
			tmp->len = tmp->request_len; /* its a try to restore a valid buffer */
			return;
		}

		/* the data is filled up and ready, the ->len value is ok... 
		 * just rewrite now the command tag and the length
		 */
		if(header_len == SOCKBUF_HEADER_DEFAULT)
		{
			*(tmp->buf+tmp->request_len) = cmd;
			*((uint16*)(tmp->buf+tmp->request_len+1)) = (uint16)len;
		}
		else
		{
			*(tmp->buf+tmp->request_len) = cmd|0x80;
			*((uint32*)(tmp->buf+tmp->request_len+1)) = (uint32)len;
		}

#ifdef SEND_BUFFER_DEBUG
		{
			uint8 cc;
			cc= *(tmp->buf+tmp->request_len);
			LOG(llevDebug,"SOCKBUF: Finish buffer %x for cmd %x(%d) with %d bytes (dynamic)\n", tmp, cmd, cmd, len);
		}
#endif
		tmp->request_len = SOCKBUF_DYNAMIC;
	}
	else
	{

		/* lets check our dummy cmd we have a 2 or 4 byte length header */
		if((*(tmp->buf+tmp->request_len) & 0x80))
		{
			*(tmp->buf+tmp->request_len) = cmd|0x80;
			*((uint32*)(tmp->buf+tmp->request_len+1)) = (uint32)len;
		}
		else
		{
			*(tmp->buf+tmp->request_len) = cmd;
			if(len > 0xFFFF)
			{
				/* this should really not happens - our buffer controller should take care about
				* it as we filled the data in. This usually means we had an overflow!
				*/
				LOG(llevBug,"WARNING: Called socket_buffer_add() with short length but len:%d\n", len);
				return;
			}
			*((uint16*)(tmp->buf+tmp->request_len+1)) = (uint16)len;
		}
#ifdef SEND_BUFFER_DEBUG
		{
			uint8 cc;
			cc= *(tmp->buf+tmp->request_len);
			LOG(llevDebug,"SOCKBUF: Finish buffer %x for cmd %x(%d) with %d bytes\n", tmp, cmd, cmd, len);
		}
#endif
		tmp->request_len = SOCKBUF_DYNAMIC;
		tmp->len += len;
	}
}

/* setup the socket buffers (using mempool) */
sockbuf_struct *socket_buffer_get(int len)
{
	sockbuf_struct *tmp;

	if(len <=256)
	{
		tmp = get_poolchunk(pool_sockbuf_small);
		tmp->pool = pool_sockbuf_small;
		tmp->bufsize = 256;
	}
	else if(len <=2048)
	{
		tmp = get_poolchunk(pool_sockbuf_medium);
		tmp->pool = pool_sockbuf_medium;
		tmp->bufsize = 2048;
	}
	else if(len <=4096)
	{
		tmp = get_poolchunk(pool_sockbuf_huge);
		tmp->pool = pool_sockbuf_huge;
		tmp->bufsize = 4096;
	}
	else /* get a dynmic buffer - report this so we can adjust the buffers */
	{
		tmp = get_poolchunk(pool_sockbuf_dynamic);
		tmp->pool = pool_sockbuf_dynamic;
		tmp->bufsize = ((len/256)+1)*256;
		tmp->buf = malloc(tmp->bufsize);
		if(!tmp->buf)
			LOG(llevError, "dynamic sockbuf: malloc returned zero for %d(%d) bytes\n", len, tmp->bufsize);
#ifdef SEND_BUFFER_DEBUG
		LOG(llevDebug, "Allocated overzised dynamic sockbuf for: %d(%d) bytes\n", len, tmp->bufsize);
#endif
	}
	tmp->ns = tmp->last = tmp->next = NULL;
	tmp->request_len = tmp->instance = tmp->len = tmp->pos = tmp->flags = 0;

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
* out_buf != NULL: use it as buffer 
* (WARNING: we assume an already allocated ->buf in the right size
* out_buf == NULL: allocate a fitting command buffer
* len >= 0: copy len bytes from cmd_buf to out_buf (binary data or string collection)
* len == -1: cmd_buf is a C string ('\0' terminated), use strlen() to get length
*/
sockbuf_struct *compose_socklist_buffer(int cmd, sockbuf_struct *out_buf, char *cmd_buf, int len, int flags)
{
	sockbuf_struct *tmp_buf = out_buf;

	if(len < 0) /* ensure a valid length value */
		len = strlen(cmd_buf);

	if(!tmp_buf) /* allocate command buffer first */
		tmp_buf = socket_buffer_get(len);

	/* note: ->pos is not used - we never, ever attach something to a sockbuf
	 * which is already partly transfered by the socket func. tmp_buf->pos is always zero here
	 */
	*(tmp_buf->buf+tmp_buf->len) = (uint8)cmd;

	/* setup the <1 byte cmd><2/4 bytes length> header */ 
	if(len > 0xffff)
	{
		*((uint32*)(tmp_buf->buf+tmp_buf->len+1)) = (uint32)len;
		*tmp_buf->buf |= 0x80; /* mark the command its followed by a 4 byte length header */
		tmp_buf->len += 5;
	}
	else
	{
		*((uint16*)(tmp_buf->buf+tmp_buf->len+1)) = (uint16)len;
		tmp_buf->len += 3;
	}

	memcpy(tmp_buf->buf+tmp_buf->len, cmd_buf, len);
	tmp_buf->len += len;
	tmp_buf->flags |= flags;

	return tmp_buf;

}

/* attach socket buffer to outgoing list of this ns socket 
 * Its a FIFO list, start means here "newest"
 */
void socket_buffer_enqueue(NewSocket *ns, sockbuf_struct *sockbufptr)
{
	if(!sockbufptr)
		return;

	if(ns->sockbuf_start)
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

	/* ensure to reset the "working buffer" pointers */
	sockbufptr->ns = NULL;
	ns->sockbuf = NULL;

	sockbufptr->instance++; /* mark the buffer as enqueued */
	/* lets collect some statistic */
	ns->sockbuf_bytes += sockbufptr->len;
	ns->sockbuf_nrof++;
}

/* NOTE: we have a FIFO queue here - we remove from the end 
 * We just check there is something at the end of the queue -
 * if so this must be the one we want remove
 */
void socket_buffer_dequeue(NewSocket *ns)
{
	sockbuf_struct *tmp = ns->sockbuf_end;

	if(tmp)
	{
		ns->sockbuf_end = ns->sockbuf_end->last;

		if(ns->sockbuf_end)

		{
			ns->sockbuf_end->next = NULL;
			/* most important settings so we can send the buffers data */
			ns->sockbuf_pos = ns->sockbuf_end->pos;
			ns->sockbuf_len = ns->sockbuf_end->len;
		}
		else /* the removed buffer is also start */
			ns->sockbuf_start = NULL;

		ns->sockbuf_nrof--;
		ns->sockbuf_bytes -= tmp->len;

		if(--tmp->instance < 0)
			LOG(llevBug,"socket_buffer_dequeue(): instance counter < 0 (%x)\n", tmp->instance);

		if(!tmp->instance && !(tmp->flags & SOCKBUF_FLAG_STATIC))
			return_poolchunk(tmp, tmp->pool);
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
	sockbuf->instance = 0;
}
void initialize_socket_buffer_medium(sockbuf_struct *sockbuf)
{
	sockbuf->buf = malloc(2048);
	sockbuf->instance = 0;
}
void initialize_socket_buffer_huge(sockbuf_struct *sockbuf)
{
	sockbuf->buf = malloc(4096);
	sockbuf->instance = 0;
}
void initialize_socket_buffer_dynamic(sockbuf_struct *sockbuf)
{
	sockbuf->buf = NULL;
	sockbuf->instance = 0;
}

/* destructor called from mempool */
void free_socket_buffer_dynamic(sockbuf_struct* sb)
{
	if(sb->buf)
		FREE_AND_NULL_PTR(sb->buf);
	sb->instance = 0;
}

