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

    The author can be reached via e-mail to info@daimonin.net
*/

#include <include.h>

static int thread_flag = FALSE;
static SDL_Thread *socket_thread;
static SDL_mutex *read_lock;
static SDL_mutex *write_lock;
static SDL_mutex *socket_lock;
static SDL_cond *socket_cond;

static _command_buffer_read *read_cmd_end=NULL;
_command_buffer_read *read_cmd_start=NULL;

#ifdef __LINUX
static char *strerror_local(int errnum)
{
#if defined(HAVE_STRERROR)
    return(strerror(errnum));
#else
    return("strerror not implemented");
#endif
}
#endif

void send_command_binary(int cmd, const char *body, int len)
{
	SDL_LockMutex(write_lock);

	if(csocket.outbuf.len + 3 > MAXSOCKBUF)
	{
		SOCKET_CloseClientSocket(&csocket);
		return;
	}

	/* adjust the buffer */
	if(csocket.outbuf.pos)
		memcpy(csocket.outbuf.buf, csocket.outbuf.buf+csocket.outbuf.pos, csocket.outbuf.len);

	csocket.outbuf.pos=0;

	if(!body)
	{
		len = 0x8001;

		csocket.outbuf.buf[csocket.outbuf.len++] = ((uint32) (len) >> 8) & 0xFF;
		csocket.outbuf.buf[csocket.outbuf.len++] = ((uint32) (len)) & 0xFF;
		csocket.outbuf.buf[csocket.outbuf.len++] = (unsigned char) cmd;;
	}

	SDL_UnlockMutex(write_lock);
}

/* move a command/buffer to the out buffer so it can be written to the socket */
int send_socklist(int fd, SockList  msg)
{
	SDL_LockMutex(write_lock);

	if(csocket.outbuf.len + msg.len > MAXSOCKBUF)
	{
		SOCKET_CloseClientSocket(&csocket);
		return -1;
	}

	/* adjust the buffer */
	if(csocket.outbuf.pos && csocket.outbuf.len)
		memcpy(csocket.outbuf.buf, csocket.outbuf.buf+csocket.outbuf.pos, csocket.outbuf.len);

	csocket.outbuf.pos=0;

	csocket.outbuf.buf[csocket.outbuf.len++] = (uint8) ((msg.len >> 8) & 0xFF);
	csocket.outbuf.buf[csocket.outbuf.len++] = ((uint32) (msg.len)) & 0xFF;

	memcpy(csocket.outbuf.buf+csocket.outbuf.len, msg.buf, msg.len);
	csocket.outbuf.len += msg.len;

	SDL_UnlockMutex(write_lock);

	return 0;
}


/* get a read command from the queue.
 * remove it from queue and return a pointer to it.
 * return NULL if there is no command
 */
_command_buffer_read *get_read_cmd(void)
{
	_command_buffer_read *tmp;

	/*LOG(-1,"GET-READ-CMD: %d %d - %d\n", csocket.inbuf.len, csocket.inbuf.pos,read_cmd_start );*/
	//return NULL;
	SDL_LockMutex(read_lock);

	if(!read_cmd_start)
		return NULL;

	tmp = read_cmd_start;
	read_cmd_start = tmp->next;
	if(read_cmd_end == tmp)
		read_cmd_end = NULL;

	SDL_UnlockMutex(read_lock);

	return tmp;
}

/* free a read cmd buffer struct and its data tail */
void free_read_cmd(_command_buffer_read *cmd)
{
	free(cmd->data);
	free(cmd);
}

/* clear & free the whole read cmd queue */
void clear_read_cmd_queue(void)
{
	SDL_LockMutex(read_lock);

	while(read_cmd_start)
		free_read_cmd(get_read_cmd());

	SDL_UnlockMutex(read_lock);
}

/* write stuff to the socket */
static inline void write_socket_buffer(int fd, SockList *sl)
{
	int amt;


	if (sl->len == 0)
		return;

	SDL_LockMutex(write_lock);
	amt = send(fd, sl->buf + sl->pos, sl->len, MSG_DONTWAIT);
	/*LOG(-1,"WRITE(%d) written: %d bytes\n", sl->len, amt);*/

	/* following this link: http://www-128.ibm.com/developerworks/linux/library/l-sockpit/#N1019D
	* send() with MSG_DONTWAIT under linux can return 0 which means the data 
	* is "queued for transmission". I was not able to find that in the send() man pages...
	* In my testings it never happend, so i put it here in to have it perhaps triggered in
	* some server runs (but we should trust perhaps ibm developer infos...).
	*/
	if(!amt)
		amt = sl->len; /* as i understand, the data is now internal buffered? So remove it from our write buffer */
		
	if (amt > 0)
	{
		sl->pos += amt;
		sl->len -= amt;
	}
	SDL_UnlockMutex(write_lock);

	if (amt < 0) /* error */
	{
#ifdef WIN32 /* ***win32 write_socket_buffer: change error handling */
		if (WSAGetLastError() == WSAEWOULDBLOCK)
			return;

		LOG(LOG_DEBUG, "New socket write failed (wsb) (%d).\n", WSAGetLastError());
		SOCKET_CloseClientSocket(&csocket);
#else
		if (errno == EWOULDBLOCK || errno == EINTR)
			return;
		LOG(LOG_DEBUG, "New socket write failed (wsb %d) (%d: %s).\n", EAGAIN, errno, strerror_local(errno));
		SOCKET_CloseClientSocket(&csocket);
#endif
		return;
	}

}

/* read stuff from socket */
static inline int read_socket_buffer(int fd, SockList *sl)
{
	int         stat_ret, read_bytes, tmp;

	/* calculate how many bytes can be read in one row in our round robin buffer */
	tmp = sl->pos+sl->len;

	/* we have still some bytes until we hit our buffer border ?*/
	if(tmp >= MAXSOCKBUF)
	{
		tmp = tmp-MAXSOCKBUF; /* thats our start offset */
		read_bytes = sl->pos - tmp; /* thats our free buffer until ->pos*/
	}
	else		
		read_bytes = MAXSOCKBUF-tmp; /* tmp is our offset and there is still a bit to read in */

	stat_ret = recv(fd, sl->buf + tmp, read_bytes, 0);

	/*LOG(-1,"READ(%d)(%d): %d\n", ROUND_TAG, fd, stat_ret);*/

	if (stat_ret > 0)
		sl->len += stat_ret;
	else if (stat_ret < 0) /* lets check its a real problem */
	{
#ifdef WIN32
		if (WSAGetLastError() == WSAEWOULDBLOCK)
			return 1;

		if (WSAGetLastError() == WSAECONNRESET)
			LOG(LOG_DEBUG, "Connection closed by server\n");
		else
			LOG(LOG_DEBUG, "ReadPacket got error %d, returning 0\n", WSAGetLastError());
#else
		if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
			return 1;

		LOG(LOG_DEBUG, "ReadPacket got error %d (%s), returning 0\n", errno, strerror_local(errno));
#endif
		SOCKET_CloseClientSocket(&csocket);
	}
	else
		SOCKET_CloseClientSocket(&csocket);

	return stat_ret;
}

static int socket_thread_loop(void *nix)
{
	while(thread_flag)
	{
        /* Want a valid socket for the IO loop */
        SDL_LockMutex(socket_lock);
        if(csocket.fd == SOCKET_NO)
        {
            SDL_CondWait(socket_cond, socket_lock);
            if(!thread_flag)
            {
                SDL_UnlockMutex(socket_lock);
                break;
            }
        }
        
		if(csocket.fd == SOCKET_NO && GameStatus >= GAME_STATUS_STARTCONNECT)
		{
			SDL_Delay(150);
            SDL_UnlockMutex(socket_lock);
			continue;
		}

		if(csocket.fd != SOCKET_NO && GameStatus >= GAME_STATUS_STARTCONNECT)
			read_socket_buffer(csocket.fd, &csocket.inbuf);

		/* lets check we have a valid command */
		while(csocket.inbuf.len >= 2)
		{
			_command_buffer_read *tmp;
			int head_off=2, toread = -1, pos = csocket.inbuf.pos;

			/*LOG(-1,"READ COMMAND-A: %d %d\n", csocket.inbuf.len, csocket.inbuf.pos ); */
			if(csocket.inbuf.buf[pos] & 0x80) /* 3 byte length heasder? */
			{
				if(csocket.inbuf.len > 2)
				{
					head_off = 3;

					toread = ((csocket.inbuf.buf[pos]&0x7f) << 16);
					if(++pos >= MAXSOCKBUF)
						pos -= MAXSOCKBUF;
					toread += (csocket.inbuf.buf[pos] << 8);
					if(++pos >= MAXSOCKBUF)
						pos -= MAXSOCKBUF;
					toread += csocket.inbuf.buf[pos];
				}

			}
			else /* 2 size length header */
			{
				toread = (csocket.inbuf.buf[pos] << 8);
				if(++pos >= MAXSOCKBUF)
					pos -= MAXSOCKBUF;
				toread += csocket.inbuf.buf[pos];
			}

			/* adjust pos to data start */
			if(++pos >= MAXSOCKBUF)
				pos -= MAXSOCKBUF;
			/*LOG(-1,"READ COMMAND-B: %d %d %d %d\n", csocket.inbuf.len, csocket.inbuf.pos,head_off, toread); */

			/* leave collecting commands when we hit an incomplete one */
			if(toread == -1 || csocket.inbuf.len < toread+head_off)
            {
                SDL_UnlockMutex(socket_lock);
				break; 
            }

			tmp = (_command_buffer_read *) malloc(sizeof(_command_buffer_read));
			tmp->data = (uint8 *) malloc(toread + 1);
			tmp->len = toread;
			tmp->next = NULL;

			if(pos + toread > MAXSOCKBUF) /* splitted data tail? */
			{
				int tmp_read, read_part;

				read_part = (pos + toread) - MAXSOCKBUF;
				tmp_read = toread - read_part;
				memcpy(tmp->data, csocket.inbuf.buf+pos, tmp_read);
				memcpy(tmp->data+tmp_read, csocket.inbuf.buf, read_part);
				csocket.inbuf.pos = read_part;
			}
			else
			{
				memcpy(tmp->data, csocket.inbuf.buf+pos, toread);
				csocket.inbuf.pos = pos + toread;
			}
			tmp->data[tmp->len] = 0; /* ensure we have a zero at the end - simple buffer overflow proection */
			csocket.inbuf.len -= toread + head_off;
			/*LOG(-1,"READ COMMAND-C: %d %d\n", csocket.inbuf.len, csocket.inbuf.pos );*/

			SDL_LockMutex(read_lock);
			/* put tmp to the end of our read cmd queue */
			if(!read_cmd_start)
				read_cmd_start = tmp;
			else
				read_cmd_end->next = tmp;
			read_cmd_end = tmp;
			SDL_UnlockMutex(read_lock);
		}

		if(csocket.fd != SOCKET_NO && GameStatus >= GAME_STATUS_STARTCONNECT)
			write_socket_buffer(csocket.fd, &csocket.outbuf);
        
        SDL_UnlockMutex(socket_lock);

		/* LOG(-1,"SOCKET LOOP: %d %d\n", csocket.inbuf.len, csocket.inbuf.pos ); */
	}

	return 0;
}

void socket_thread_start(void)
{
	LOG(-1,"START THREAD\n"); 
	thread_flag = TRUE;

	socket_cond = SDL_CreateCond();
	socket_lock = SDL_CreateMutex();
	read_lock = SDL_CreateMutex();
	write_lock = SDL_CreateMutex();

	socket_thread = SDL_CreateThread(socket_thread_loop, NULL);
	if ( socket_thread == NULL ) 
		LOG(LOG_ERROR, "Unable to start socket thread: %s\n", SDL_GetError());
}


void socket_thread_stop(void)
{
	LOG(-1,"STOP THREAD\n"); 

	if(thread_flag)
	{
		thread_flag = FALSE;
        SDL_CondSignal(socket_cond);
		SDL_WaitThread(socket_thread, NULL);

        SDL_DestroyCond(socket_cond);
		SDL_DestroyMutex(socket_lock);
		SDL_DestroyMutex(read_lock);
		SDL_DestroyMutex(write_lock);
	}
}


int SOCKET_GetError()
{
#ifdef __WIN_32
    return(WSAGetLastError());
#elif __LINUX
    return errno;
#endif
}

Boolean SOCKET_CloseSocket(SOCKET fd)
{
	if (fd == SOCKET_NO)
		return(TRUE);

#ifdef __LINUX
	close(fd);
#else
	closesocket(fd);
#endif
	return(TRUE);
}

Boolean SOCKET_CloseClientSocket(struct ClientSocket *csock)
{
	if (csock->fd == SOCKET_NO)
		return(TRUE);

    LOG(-1, "CloseClientSocket()\n");

    /* No more socket for the IO thread */
    SDL_LockMutex(socket_lock);

	FreeMemory((void *)&csock->inbuf.buf);
	FreeMemory((void *)&csock->outbuf.buf);
    csock->inbuf.buf = csock->outbuf.buf = NULL;
	csock->inbuf.len = 0;
	csock->outbuf.len = 0;
	csock->inbuf.pos = 0;
	csock->outbuf.pos = 0;
	csock->fd = SOCKET_NO;

    SDL_CondSignal(socket_cond);
    SDL_UnlockMutex(socket_lock);

	return(TRUE);
}


Boolean SOCKET_InitSocket(void)
{
#ifdef WIN32
	WSADATA w;
	WORD	wVersionRequested = MAKEWORD( 2, 2 );
	int     error;

	csocket.fd = SOCKET_NO;
	csocket.cs_version = 0;

	SocketStatusErrorNr = 0;
	error = WSAStartup(wVersionRequested, &w);
	if (error)
	{
		wVersionRequested = MAKEWORD( 2, 0 );
		error = WSAStartup(wVersionRequested, &w);
		if (error)
		{
			wVersionRequested = MAKEWORD( 1, 1 );
			error = WSAStartup(wVersionRequested, &w);
			if (error)
			{
				LOG(LOG_ERROR, "Error:  Error init starting Winsock: %d!\n", error);
				return(FALSE);
			}
		}
	}

	LOG(LOG_MSG, "Using socket version %x!\n", w.wVersion);
#endif
	return(TRUE);
}


Boolean SOCKET_DeinitSocket(void)
{
	if(csocket.fd != SOCKET_NO)
		SOCKET_CloseClientSocket(&csocket);

#ifdef WIN32
	WSACleanup();
#endif

	return(TRUE);
}

Boolean SOCKET_OpenClientSocket(struct ClientSocket *csock, char *host, int port)
{
    int tmp = 1;
    
    /* No more socket for the IO thread */
    SDL_LockMutex(socket_lock);
    
    if(! SOCKET_OpenSocket(&csock->fd, host, port))
       return FALSE;
    
    csock->inbuf.buf = (unsigned char *) malloc(MAXSOCKBUF);
    csock->inbuf.len = 0;
    csock->inbuf.pos = 0;
    csock->outbuf.buf = (unsigned char *) malloc(MAXSOCKBUF);
    csock->outbuf.len = 0;
    csock->outbuf.pos = 0;

    csock->command_sent = 0;
    csock->command_received = 0;
    csock->command_time = 0;

    if(setsockopt(csock->fd, IPPROTO_TCP, TCP_NODELAY, (char *) &tmp, sizeof(tmp)))
    {
        LOG(LOG_ERROR, "ERROR: setsockopt(TCP_NODELAY) failed\n");
    }
    
    /* socket available for socket thread */
    SDL_CondSignal(socket_cond);
    SDL_UnlockMutex(socket_lock);

    return TRUE;
}

#ifdef __WIN_32
Boolean SOCKET_OpenSocket(SOCKET *socket_temp, *csock, char *host, int port)
{
    int             error, tmp=1;
    long            temp;
    struct hostent *hostbn;
    int             oldbufsize;
    int             newbufsize = 65535, buflen = sizeof(int);
    uint32          start_timer;
	struct linger       linger_opt;

	LOG(LOG_DEBUG, "OpenSocket: %s\n", host);
    /* The way to make the sockets work on XP Home - The 'unix' style socket
     * seems to fail inder xp home.
     */
    *socket_temp = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	insock.sin_family = AF_INET;
    insock.sin_port = htons((unsigned short) port);

    if (isdigit(*host))
        insock.sin_addr.s_addr = inet_addr(host);
    else
    {
        hostbn = gethostbyname(host);
        if (hostbn == (struct hostent *) NULL)
        {
            LOG(LOG_ERROR, "Unknown host: %s\n", host);
            *socket_temp = SOCKET_NO;
            return(FALSE);
        }
        memcpy(&insock.sin_addr, hostbn->h_addr, hostbn->h_length);
    }

    temp = 1;   /* non-block */

	if (ioctlsocket(*socket_temp, FIONBIO, &temp) == -1)
	{
		LOG(LOG_ERROR, "ERROR: ioctlsocket(*socket_temp, FIONBIO , &temp)\n");
		*socket_temp = SOCKET_NO;
		return(FALSE);
	}

	linger_opt.l_onoff = 1;
	linger_opt.l_linger = 5;
	if (setsockopt(*socket_temp, SOL_SOCKET, SO_LINGER, (char *) &linger_opt, sizeof(struct linger)))
		LOG(LOG_ERROR, "BUG: Error on setsockopt LINGER\n");

	error = 0;

    start_timer = SDL_GetTicks();
    while (connect(*socket_temp, (struct sockaddr *) &insock, sizeof(insock)) == SOCKET_ERROR)
    {
        SDL_Delay(3);

        /* timeout.... without connect will REALLY hang a long time */
        if (start_timer + SOCKET_TIMEOUT_SEC * 1000 < SDL_GetTicks())
        {
            *socket_temp = SOCKET_NO;
            return(FALSE);
        }

        SocketStatusErrorNr = WSAGetLastError();
        if (SocketStatusErrorNr == WSAEISCONN)  /* we have a connect! */
            break;

        if (SocketStatusErrorNr == WSAEWOULDBLOCK
         || SocketStatusErrorNr == WSAEALREADY
         || (SocketStatusErrorNr == WSAEINVAL && error)) /* loop until we finished */
        {
            error = 1;
            continue;
        }

        LOG(LOG_MSG, "Connect Error:  %d", SocketStatusErrorNr);
        *socket_temp = SOCKET_NO;
        return(FALSE);
    }
    /* we got a connect here! */

	if (getsockopt(*socket_temp, SOL_SOCKET, SO_RCVBUF, (char *) &oldbufsize, &buflen) == -1)
        oldbufsize = 0;

    if (oldbufsize < newbufsize)
    {
        if (setsockopt(*socket_temp, SOL_SOCKET, SO_RCVBUF, (char *) &newbufsize, sizeof(&newbufsize)))
        {
            setsockopt(*socket_temp, SOL_SOCKET, SO_RCVBUF, (char *) &oldbufsize, sizeof(&oldbufsize));
        }
    }

    LOG(LOG_MSG, "Connected to %s:%d\n", host, port);
    return(TRUE);
}

/* we used our core connect routine to connect to metaserver, this is the special
   read one.*/

void read_metaserver_data(SOCKET fd)
{
    int     stat, temp;
    char   *ptr, *buf;
    void   *tmp_free;

    ptr = (char *) malloc(MAX_METASTRING_BUFFER);
    buf = (char *) malloc(MAX_METASTRING_BUFFER);
    temp = 0;
    for (; ;)
    {
        /* win32 style input */

        stat = recv(fd, ptr, MAX_METASTRING_BUFFER, 0);
        if ((stat == -1) && WSAGetLastError() != WSAEWOULDBLOCK)
        {
            LOG(LOG_ERROR, "Error reading metaserver data!: %d\n", WSAGetLastError());
            break;
        }
        else if (stat > 0)
        {
            if (temp + stat >= MAX_METASTRING_BUFFER)
            {
                memcpy(buf + temp, ptr, temp + stat - MAX_METASTRING_BUFFER - 1);
                temp += stat;
                break;
            }
            memcpy(buf + temp, ptr, stat);
            temp += stat;
        }
        else if (stat == 0)
        {
            /* connect closed by meta */
            break;
        }
    }
    buf[temp] = 0;
    LOG(0, "GET: %s\n", buf);
    parse_metaserver_data(buf);
    tmp_free = &buf;
    FreeMemory(tmp_free);
    tmp_free = &ptr;
    FreeMemory(tmp_free);
}


#elif __LINUX
Boolean SOCKET_OpenSocket(SOCKET *socket_temp, char *host, int port)
{
    unsigned int  oldbufsize, newbufsize = 65535, buflen = sizeof(int);
    struct linger       linger_opt;

	/* Use new (getaddrinfo()) or old (gethostbyname()) socket API */
#ifndef HAVE_GETADDRINFO
    struct protoent *protox;
    struct sockaddr_in  insock;

	printf("Opening to %s %i\n", host, port);
    protox = getprotobyname("tcp");

    if (protox == (struct protoent *) NULL)
    {
        fprintf(stderr, "Error getting prorobyname (tcp)\n");
        return FALSE;
    }
    *socket_temp = socket(PF_INET, SOCK_STREAM, protox->p_proto);

    if (*socket_temp == -1)
    {
        perror("init_connection:  Error on socket command.\n");
        *socket_temp = SOCKET_NO;
        return FALSE;
    }
    insock.sin_family = AF_INET;
    insock.sin_port = htons((unsigned short) port);
    if (isdigit(*host))
        insock.sin_addr.s_addr = inet_addr(host);
    else
    {
        struct hostent *hostbn  = gethostbyname(host);
        if (hostbn == (struct hostent *) NULL)
        {
            fprintf(stderr, "Unknown host: %s\n", host);
            return FALSE;
        }
        memcpy(&insock.sin_addr, hostbn->h_addr, hostbn->h_length);
    }

    if (connect(*socket_temp, (struct sockaddr *) &insock, sizeof(insock)) == (-1))
    {
        perror("Can't connect to server");
        return FALSE;
    }
#else
    struct addrinfo hints;
    struct addrinfo *res = NULL, *ai;
    char port_str[6], hostaddr[40];

    printf("Opening to %s %i\n", host, port);

    snprintf(port_str, sizeof(port_str), "%d", port);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    /* Workaround for issue #425 on OSs with broken NIS+ like FC5. 
     * This should disable any service lookup */
    hints.ai_flags = AI_NUMERICSERV; 

    if (getaddrinfo(host, port_str, &hints, &res) != 0)
        return FALSE;

    for (ai = res; ai != NULL; ai = ai->ai_next) {
        getnameinfo(ai->ai_addr, ai->ai_addrlen, hostaddr, sizeof(hostaddr), NULL, 0, NI_NUMERICHOST);
        printf("  trying %s\n", hostaddr);

        *socket_temp = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
        if (*socket_temp == -1) {
            *socket_temp = SOCKET_NO;
            continue;
        }

        if (connect(*socket_temp, ai->ai_addr, ai->ai_addrlen) != 0) {
            close(*socket_temp);
            *socket_temp = SOCKET_NO;
            continue;
        }

        break;
    }

    freeaddrinfo(res);
    if (*socket_temp == SOCKET_NO) {
        perror("Can't connect to server");
        return FALSE;
    }
#endif

	LOG(LOG_DEBUG, "socket: fcntl(%x %x) %x.\n", O_NDELAY, O_NONBLOCK, fcntl(*socket_temp, F_GETFL));
	if (fcntl(*socket_temp, F_SETFL, fcntl(*socket_temp, F_GETFL) | O_NONBLOCK ) == -1)
	{
		LOG(LOG_ERROR, "socket:  Error on fcntl %x.\n", fcntl(*socket_temp, F_GETFL));
		*socket_temp = SOCKET_NO;
		return(FALSE);
	}
	LOG(LOG_DEBUG, "socket:  fcntl %x.\n", fcntl(*socket_temp, F_GETFL));

	linger_opt.l_onoff = 1;
	linger_opt.l_linger = 5;
	if (setsockopt(*socket_temp, SOL_SOCKET, SO_LINGER, (char *) &linger_opt, sizeof(struct linger)))
		LOG(LOG_ERROR, "BUG: Error on setsockopt LINGER\n");

	if (getsockopt(*socket_temp, SOL_SOCKET, SO_RCVBUF, (char *) &oldbufsize, &buflen) == -1)
        oldbufsize = 0;

    if (oldbufsize < newbufsize)
    {
        if (setsockopt(*socket_temp, SOL_SOCKET, SO_RCVBUF, (char *) &newbufsize, sizeof(&newbufsize)))
        {
            LOG(LOG_DEBUG, "socket: setsockopt unable to set output buf size to %d\n", newbufsize);
            setsockopt(*socket_temp, SOL_SOCKET, SO_RCVBUF, (char *) &oldbufsize, sizeof(&oldbufsize));
        }
    }
    return TRUE;
}


void read_metaserver_data(SOCKET fd)
{
    int     stat, temp;
    char   *ptr, *buf;
    void   *tmp_free;

    ptr = (char *) _malloc(MAX_METASTRING_BUFFER, "read_metaserver_data(): metastring buffer1");
    buf = (char *) _malloc(MAX_METASTRING_BUFFER, "read_metaserver_data(): metastring buffer2");
    temp = 0;
    for (; ;)
    {
        do
        {
            stat = recv(fd, ptr, MAX_METASTRING_BUFFER, 0);
        }
        while (stat == -1);

        if (stat == -1)
        {
            LOG(LOG_ERROR, "Error reading metaserver data!\n");
            break;
        }
        else if (stat > 0)
        {
            if (temp + stat >= MAX_METASTRING_BUFFER)
            {
                memcpy(buf + temp, ptr, temp + stat - MAX_METASTRING_BUFFER - 1);
                temp += stat;
                break;
            }
            memcpy(buf + temp, ptr, stat);
            temp += stat;
        }
        else if (stat == 0)
        {
            /* connect closed by meta */
            break;
        }
    }

    buf[temp] = 0;
    parse_metaserver_data(buf);
    tmp_free = &buf;
    FreeMemory(tmp_free);
    tmp_free = &ptr;
    FreeMemory(tmp_free);
}

#endif
