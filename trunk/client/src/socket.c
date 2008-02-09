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
#define SOCKET_TIMEOUT_MS 4000

static SDL_Thread *input_thread;
static SDL_mutex *input_buffer_mutex;
static SDL_cond *input_buffer_cond;

static SDL_Thread *output_thread;
static SDL_mutex *output_buffer_mutex;
static SDL_cond *output_buffer_cond;

/** Mutex to protect socket deinitialization */
static SDL_mutex *socket_mutex;

/** All socket threads will exit if they see this flag set */
static int abort_thread = FALSE;

/* start is the first waiting item in queue, end is the most recent enqueued */
static command_buffer *input_queue_start = NULL, *input_queue_end = NULL;
static command_buffer *output_queue_start = NULL, *output_queue_end = NULL;

/*
 * Buffer queue management
 */

/** Create a new command buffer of the given size, copying the data buffer if not NULL.
 * The buffer will always be null-terminated for safety (and one byte larger than requested).
 * @param len requested buffer size in bytes
 * @param data buffer data to copy (len bytes), or NULL
 * @return a new command buffer or NULL in case of an error
 */
static command_buffer *command_buffer_new(unsigned int len, uint8 *data)
{
    command_buffer *buf;

    if(!(buf = (command_buffer *)malloc(sizeof(command_buffer)+len+1)))
		return NULL;

    buf->next = buf->prev = NULL;
    buf->len = len;

    if (data)
        memcpy(buf->data, data, len);
    buf->data[len] = 0; /* Buffer overflow sentinel */

    return buf;
}

/** Free all memory related to a single command buffer */
void command_buffer_free(command_buffer *buf)
{
    free(buf);
}

/** Enqueue a command buffer last in a queue */
static void command_buffer_enqueue(command_buffer *buf, command_buffer **queue_start, command_buffer **queue_end)
{
    buf->next = NULL;
    buf->prev = *queue_end;
    if (*queue_start == NULL)
        *queue_start = buf;
    if (buf->prev)
        buf->prev->next = buf;
    *queue_end = buf;
}

/** Remove the first command buffer from a queue */
static command_buffer *command_buffer_dequeue(command_buffer **queue_start, command_buffer **queue_end)
{
    command_buffer *buf = *queue_start;

    if (buf)
    {
        *queue_start = buf->next;
        if (buf->next)
            buf->next->prev = NULL;
        else
            *queue_end = NULL;
    }

    return buf;
}

/*
 * High-level external command buffer queue interface
 */

/** Add a binary command to the output buffer.
 * If body is NULL, a single-byte command is created from cmd.
 * Otherwise body should include the length and cmd header
 */
int send_command_binary(uint8 cmd, uint8 *body, unsigned int len)
{
    command_buffer *buf;

    if (body)
        buf = command_buffer_new(len, body);
    else
    {
        uint8 tmp[3];
        len = 0x8001;
        /* Packet order is obviously big-endian for length data */
        tmp[0] = (len >> 8) & 0xFF;
        tmp[1] = len & 0xFF;
        tmp[2] = cmd;
        buf = command_buffer_new(3, tmp);
    }

    if (buf == NULL)
    {
        SOCKET_CloseClientSocket(&csocket);
        return -1;
    }

    SDL_LockMutex(output_buffer_mutex);
    command_buffer_enqueue(buf, &output_queue_start, &output_queue_end);
    SDL_CondSignal(output_buffer_cond);
    SDL_UnlockMutex(output_buffer_mutex);

    return 0;
}

/** move a command buffer to the out buffer so it can be written to the socket */
int send_socklist(int fd, SockList  msg)
{
    command_buffer *buf;

    buf = command_buffer_new(msg.len + 2, NULL);
    if (buf == NULL)
    {
        SOCKET_CloseClientSocket(&csocket);
        return -1;
    }

    memcpy(buf->data + 2, msg.buf, msg.len);

    buf->data[0] = (uint8) ((msg.len >> 8) & 0xFF);
    buf->data[1] = ((uint32) (msg.len)) & 0xFF;

    SDL_LockMutex(output_buffer_mutex);
    command_buffer_enqueue(buf, &output_queue_start, &output_queue_end);
    SDL_CondSignal(output_buffer_cond);
    SDL_UnlockMutex(output_buffer_mutex);

    return 0;
}

/** get a command from the queue.
 * remove it from queue and return a pointer to it.
 * return NULL if there is no command
 */
command_buffer *get_next_input_command()
{
    command_buffer *buf;

    SDL_LockMutex(input_buffer_mutex);
    buf = command_buffer_dequeue(&input_queue_start, &input_queue_end);
    SDL_UnlockMutex(input_buffer_mutex);
    return buf;
}

/*
 * Lowlevel socket IO
 */

/** Worker for the reader thread. It continuously reads data
 * from the socket, splits it into commands and enqueues them
 * on the socket queue.
 * If any error is detected, the socket is closed and the thread exits. It is
 * up to them main thread to detect this and join() the worker threads
 */
static int reader_thread_loop(void *nix)
{
    static uint8 *readbuf = NULL;
	static int readbuf_malloc = 256;
    int readbuf_len = 0;
    int header_len = 0;
    int cmd_len = -1;

    LOG(LOG_DEBUG, "Reader thread started\n");
	if(!readbuf)
		readbuf = malloc(readbuf_malloc);

    while (! abort_thread)
    {
        int ret;
        int toread;

        /* First, try to read a command length sequence */
		if (!readbuf_len)
			toread = 1; /* try to read a command from the socket */
		else if (!(readbuf[0] & 0x80) && readbuf_len < 3)
			toread = 3 - readbuf_len; /* read in 2 or 1 more bytes */
		else if ((readbuf[0] & 0x80) && readbuf_len < 5)
			toread = 5 - readbuf_len; /* read in 4 to 1 more bytes */
        else
        {
			if (readbuf_len == 3 && !(readbuf[0] & 0x80))
			{
				header_len = 3;
				cmd_len = adjust_endian_int16(*((uint16 *)(readbuf+1)));
			}
			else
			if (readbuf_len == 5 && (readbuf[0] & 0x80))
			{
				header_len = 5;
				cmd_len = adjust_endian_int32(*((uint32 *)(readbuf+1)));

			}

			toread = cmd_len + header_len - readbuf_len;
			if(cmd_len+16 > readbuf_malloc)
			{
				uint8 *tmp = readbuf;

				readbuf_malloc = cmd_len+16;
				readbuf = (uint8 *) malloc(readbuf_malloc);
				memcpy(readbuf, tmp, readbuf_len); /* save the already read in header part */
				free(tmp);
			}

			LOG(-1,"CMD_LEN: toread:%d len:%d (%x)\n", toread, cmd_len, (*((char *)readbuf))&~0x80);

        }
		if(toread)
		{
			ret = recv(csocket.fd, readbuf + readbuf_len, toread, 0);

	        if (ret == 0)
		    {
			    /* End of file */
				LOG(LOG_DEBUG, "Reader got EOF trying to read %d bytes\n", toread);
				goto out;
			}
			else if (ret == -1)
			{
				/* IO error */
	#ifdef WIN32
		        LOG(LOG_DEBUG, "Reader got error %d\n", WSAGetLastError());
	#else
		        LOG(LOG_DEBUG, "Reader got error %d (%s)\n", errno, strerror(errno));
	#endif
		        goto out;
			}
			else
			{
				readbuf_len += ret;
				/*            LOG(LOG_DEBUG, "Reader got some data (%d bytes total)\n", readbuf_len); */
			}
		}
        /* Finished with a command ? */
        if (readbuf_len == cmd_len + header_len && !abort_thread)
        {
            /*            LOG(LOG_DEBUG, "Reader got a full command\n", readbuf_len); */
            command_buffer *buf;

			LOG(-1," CMD:%x\n", (*((char *)readbuf))&~0x80);
			buf = command_buffer_new(readbuf_len, readbuf);
            if (buf == NULL)
                goto out;

            SDL_LockMutex(input_buffer_mutex);
            command_buffer_enqueue(buf, &input_queue_start, &input_queue_end);
            SDL_CondSignal(input_buffer_cond);
            SDL_UnlockMutex(input_buffer_mutex);

            cmd_len = -1;
            header_len = 0;
            readbuf_len = 0;
        }
    }

out:
    SOCKET_CloseClientSocket(&csocket);
	free(readbuf);
	readbuf = NULL;
    LOG(LOG_DEBUG, "Reader thread stopped\n");
    return -1;
}

/** Worker for the writer thread. It waits for enqueued outgoing packets
 * and sends them to the server as fast as it can.
 * If any error is detected, the socket is closed and the thread exits. It is
 * up to them main thread to detect this and join() the worker threads
 */
static int writer_thread_loop(void *nix)
{
    command_buffer *buf = NULL;
    LOG(LOG_DEBUG, "Writer thread started\n");
    while (! abort_thread)
    {
        int written = 0;

        SDL_LockMutex(output_buffer_mutex);
        while (output_queue_start == NULL && !abort_thread)
            SDL_CondWait(output_buffer_cond, output_buffer_mutex);
        buf = command_buffer_dequeue(&output_queue_start, &output_queue_end);
        SDL_UnlockMutex(output_buffer_mutex);

        while (buf && written < buf->len && !abort_thread)
        {
            int ret = send(csocket.fd, buf->data + written, buf->len - written, 0);

            if (ret == 0)
            {
                LOG(LOG_DEBUG, "Writer got EOF\n");
                goto out;
            }
            else if (ret == -1)
            {
                /* IO error */
#ifdef WIN32
                LOG(LOG_DEBUG, "Reader got error %d\n", WSAGetLastError());
#else
                LOG(LOG_DEBUG, "Writer got error %d (%s)\n", errno, strerror(errno));
#endif
                goto out;
            }
            else
                written += ret;
        }
        if (buf)
        {
            command_buffer_free(buf);
            buf = NULL;
        }
        /*        LOG(LOG_DEBUG, "Writer wrote a command (%d bytes)\n", written); */
    }

out:
    if (buf)
        command_buffer_free(buf);
    SOCKET_CloseClientSocket(&csocket);
    LOG(LOG_DEBUG, "Writer thread stopped\n");
    return 0;
}

/**
 * Initialize and start up the worker threads
 */
void socket_thread_start(void)
{
    LOG(-1,"START THREADS\n");

    if (input_buffer_cond == NULL)
    {
        input_buffer_cond = SDL_CreateCond();
        input_buffer_mutex = SDL_CreateMutex();
        output_buffer_cond = SDL_CreateCond();
        output_buffer_mutex = SDL_CreateMutex();
        socket_mutex = SDL_CreateMutex();
    }

    abort_thread = FALSE;

    input_thread = SDL_CreateThread(reader_thread_loop, NULL);
    if ( input_thread == NULL )
        LOG(LOG_ERROR, "Unable to start socket thread: %s\n", SDL_GetError());

    output_thread = SDL_CreateThread(writer_thread_loop, NULL);
    if ( output_thread == NULL )
        LOG(LOG_ERROR, "Unable to start socket thread: %s\n", SDL_GetError());
}

/** Wait for the socket threads to finish.
 * Closes the socket first, if it hasn't already been done. */
void socket_thread_stop(void)
{
    LOG(-1,"STOP THREADS\n");

    SOCKET_CloseClientSocket(&csocket);

    SDL_WaitThread(output_thread, NULL);
    SDL_WaitThread(input_thread, NULL);

    input_thread = output_thread = NULL;
}

/** Detect and handle socket system shutdowns. Also reset the socket system
 * for a restart.
 * The main thread should poll this function which
 * detects connection shutdowns and removes the
 * threads if it happens */
int handle_socket_shutdown()
{
    if (abort_thread)
    {
        socket_thread_stop();
        abort_thread = FALSE;

        /* Empty all queues */
        while (input_queue_start)
            command_buffer_free(command_buffer_dequeue(&input_queue_start, &input_queue_end));
        while (output_queue_start)
            command_buffer_free(command_buffer_dequeue(&output_queue_start, &output_queue_end));

        LOG(LOG_DEBUG, "Connection lost\n");
        return TRUE;
    }
    return FALSE;
}

/*
 * Low-level portable socket functions
 */

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
    if (shutdown(fd, SHUT_RDWR))
        perror("shutdown");
    if (close(fd))
        perror("close");
#else
    shutdown(fd, 2);
    closesocket(fd);
#endif
    return(TRUE);
}

Boolean SOCKET_CloseClientSocket(struct ClientSocket *csock)
{
    SDL_LockMutex(socket_mutex);

    if (csock->fd == SOCKET_NO)
    {
        SDL_UnlockMutex(socket_mutex);
        return(TRUE);
    }

    LOG(-1, "CloseClientSocket()\n");

    SOCKET_CloseSocket(csock->fd);

    FreeMemory((void *)&csock->inbuf.buf);
    FreeMemory((void *)&csock->outbuf.buf);
    csock->inbuf.buf = csock->outbuf.buf = NULL;
    csock->inbuf.len = 0;
    csock->outbuf.len = 0;
    csock->inbuf.pos = 0;
    csock->outbuf.pos = 0;
    csock->fd = SOCKET_NO;

    abort_thread = TRUE;

    /* Poke anyone waiting at a cond */
    SDL_CondSignal(input_buffer_cond);
    SDL_CondSignal(output_buffer_cond);

    SDL_UnlockMutex(socket_mutex);

    return(TRUE);
}


Boolean SOCKET_InitSocket(void)
{
#ifdef WIN32
    WSADATA w;
    WORD    wVersionRequested = MAKEWORD( 2, 2 );
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
    if (csocket.fd != SOCKET_NO)
        SOCKET_CloseClientSocket(&csocket);

#ifdef WIN32
    WSACleanup();
#endif

    return(TRUE);
}

#define MAXSOCKBUF 128*1024
Boolean SOCKET_OpenClientSocket(struct ClientSocket *csock, char *host, int port)
{
    int tmp = 1;

    if (! SOCKET_OpenSocket(&csock->fd, host, port))
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

    if (setsockopt(csock->fd, IPPROTO_TCP, TCP_NODELAY, (char *) &tmp, sizeof(tmp)))
    {
        LOG(LOG_ERROR, "ERROR: setsockopt(TCP_NODELAY) failed\n");
    }

    return TRUE;
}

#ifdef __WIN_32
Boolean SOCKET_OpenSocket(SOCKET *socket_temp, char *host, int port)
{
    int             error;
    long            temp;
    struct hostent *hostbn;
    int             oldbufsize;
    int             newbufsize = 65535, buflen = sizeof(int);
    uint32          start_timer;
    struct linger   linger_opt;

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
        if (start_timer + SOCKET_TIMEOUT_MS < SDL_GetTicks())
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

        LOG(LOG_MSG, "Connect Error:  %d\n", SocketStatusErrorNr);
        *socket_temp = SOCKET_NO;
        return(FALSE);
    }
    /* we got a connect here! */

    /* Clear nonblock flag */
    temp = 0;
    if (ioctlsocket(*socket_temp, FIONBIO, &temp) == -1)
    {
        LOG(LOG_ERROR, "ERROR: ioctlsocket(*socket_temp, FIONBIO , &temp == 0)\n");
        *socket_temp = SOCKET_NO;
        return(FALSE);
    }

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

#elif __LINUX
Boolean SOCKET_OpenSocket(SOCKET *socket_temp, char *host, int port)
{
    unsigned int  oldbufsize, newbufsize = 65535, buflen = sizeof(int);
    struct linger linger_opt;
    int flags;
    uint32 start_timer;

    /* Use new (getaddrinfo()) or old (gethostbyname()) socket API */
#ifndef HAVE_GETADDRINFO
    /* This method is preferable unless IPv6 is required, due to buggy distros. See mantis 0000425 */
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

    // Set non-blocking.
    flags = fcntl(*socket_temp, F_GETFL);
    if (fcntl(*socket_temp, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        LOG(LOG_ERROR, "socket: Error on switching to non-blocking.fcntl %x.\n", fcntl(*socket_temp, F_GETFL));
        *socket_temp = SOCKET_NO;
        return(FALSE);
    }
    // Try to connect.
    start_timer = SDL_GetTicks();
    while (connect(*socket_temp, (struct sockaddr *) &insock, sizeof(insock)) == -1)
    {
        SDL_Delay(3);
        /* timeout.... without connect will REALLY hang a long time */
        if (start_timer + SOCKET_TIMEOUT_MS < SDL_GetTicks())
        {
            perror("Can't connect to server");
            *socket_temp = SOCKET_NO;
            return(FALSE);
        }
    }
    // Set back to blocking.
    if (fcntl(*socket_temp, F_SETFL, flags) == -1)
    {
        LOG(LOG_ERROR, "socket: Error on switching to blocking.fcntl %x.\n", fcntl(*socket_temp, F_GETFL));
        *socket_temp = SOCKET_NO;
        return(FALSE);
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
/* Try to work around for issue #425 on OSs with broken NIS+ like FC5.
* This should disable any service lookup */
hints.ai_flags = AI_NUMERICSERV;

if (getaddrinfo(host, port_str, &hints, &res) != 0)
    return FALSE;

for (ai = res; ai != NULL; ai = ai->ai_next)
{
    getnameinfo(ai->ai_addr, ai->ai_addrlen, hostaddr, sizeof(hostaddr), NULL, 0, NI_NUMERICHOST);
    printf("  trying %s\n", hostaddr);

    *socket_temp = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
    if (*socket_temp == -1)
    {
        *socket_temp = SOCKET_NO;
        continue;
    }
    // Set non-blocking.
    flags = fcntl(*socket_temp, F_GETFL);
    if (fcntl(*socket_temp, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        LOG(LOG_ERROR, "socket: Error on switching to non-blocking.fcntl %x.\n", fcntl(*socket_temp, F_GETFL));
        *socket_temp = SOCKET_NO;
        return(FALSE);
    }
    // Try to connect.
    start_timer = SDL_GetTicks();
    while (connect(*socket_temp, ai->ai_addr, ai->ai_addrlen) != 0)
    {
        SDL_Delay(3);
        /* timeout.... without connect will REALLY hang a long time */
        if (start_timer + SOCKET_TIMEOUT_MS < SDL_GetTicks())
        {
            close(*socket_temp);
            *socket_temp = SOCKET_NO;
            goto next_try;
        }
    }
    // Set back to blocking.
    if (fcntl(*socket_temp, F_SETFL, flags) == -1)
    {
        LOG(LOG_ERROR, "socket: Error on switching to blocking.fcntl %x.\n", fcntl(*socket_temp, F_GETFL));
        *socket_temp = SOCKET_NO;
        return(FALSE);
    }
    break;
next_try:
    ;
}

freeaddrinfo(res);
if (*socket_temp == SOCKET_NO)
{
    perror("Can't connect to server");
    return FALSE;
}
#endif

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

#endif

/*
 *  Metaserver related functions
 */

/* we used our core connect routine to connect to metaserver, this is the special
   read one.*/

#ifdef __WIN_32
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
            /* FIXME: should select on fd instead of this never-ending (in case of error) busy-loop */
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
