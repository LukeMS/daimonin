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

static int  send_bufsize = SOCKET_BUFSIZE_SEND;
static int  read_bufsize = SOCKET_BUFSIZE_READ;

/* initialize our base TCP socket */
static void setsockopts(int fd)
{
    struct linger       linger_opt;
    int tmp = 1;

#ifdef WIN32 /* ***WIN32 SOCKET: init win32 non blocking socket */
    u_long tmp2 = 1;
    if (ioctlsocket(fd, FIONBIO, &tmp2) == -1)
        LOG(llevDebug, "InitConnection:  Error on ioctlsocket.\n");
#else
    LOG(llevDebug, "setsockets():  fcntl %x.\n", fcntl(fd, F_GETFL));
    if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NDELAY | O_NONBLOCK ) == -1)
        LOG(llevError, "InitConnection:  Error on fcntl %x.\n", fcntl(fd, F_GETFL));
    LOG(llevDebug, "setsockets():  fcntl %x.\n", fcntl(fd, F_GETFL));
#endif /* end win32 */

    /* Turn LINGER off (don't send left data in background if socket get closed) */
    linger_opt.l_onoff = 0;
    linger_opt.l_linger = 0;
    if (setsockopt(fd, SOL_SOCKET, SO_LINGER, (char *) &linger_opt, sizeof(struct linger)))
        LOG(llevError, "BUG: Error on setsockopt LINGER\n");

    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *) &tmp, sizeof(tmp)))
        LOG(llevError, "error on setsockopt TCP_NODELAY\n");

    if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char *) &send_bufsize, sizeof(send_bufsize)))
        LOG(llevError, "error on setsockopt SO_SNDBUF\n");

    if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char *) &read_bufsize, sizeof(read_bufsize)))
        LOG(llevError, "error on setsockopt SO_RCVBUF\n");


    /* Would be nice to have an autoconf check for this.  It appears that
     * these functions are both using the same calling syntax, just one
     * of them needs extra valus passed.
     */
#if !defined(_WEIRD_OS_) /* means is true for most (win32, linux, etc. ) */
    {
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *) &tmp, sizeof(tmp)))
            LOG(llevError, "error on setsockopt REUSEADDR\n");
    }
#else
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *) NULL, 0))
        LOG(llevDebug, "error on setsockopt REUSEADDR\n");

#endif
}

#define QUEUE_LEN 5

#if WIN32 || !HAVE_GETADDRINFO
int create_socket()
{
    int fd;
    struct sockaddr_in  insock;

#ifndef WIN32 /* non win32 */
    struct protoent    *protox;

    protox = getprotobyname("tcp");
    if (protox == NULL)
    {
        LOG(llevBug, "BUG: init_ericserver: Error getting protox\n");
        return -1;
    }
    fd = socket(PF_INET, SOCK_STREAM, protox->p_proto);

#else /* win32 */
    /* there was reported problems under windows using the protox
     * struct - IPPROTO_TCP should fix it.
     */
    fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
#endif

    LOG(llevInfo, "create_socket(): standard.\n");

    if (fd == -1)
        LOG(llevError, "ERROR: create_socket() Error creating socket on port\n");
    insock.sin_family = AF_INET;
    insock.sin_port = htons(settings.csport);
    insock.sin_addr.s_addr = htonl(INADDR_ANY);

    setsockopts(fd);

    if (bind(fd, (struct sockaddr *) &insock, sizeof(insock)) == (-1))
    {
#ifdef WIN32 /* ***win32: close() -> closesocket() */
        shutdown(fd, SD_BOTH);
        closesocket(fd);
#else
        close(fd);
#endif /* win32 */
        LOG(llevError, "error on bind command.\n");
    }

    if (listen(fd, QUEUE_LEN) == (-1))
    {
#ifdef WIN32 /* ***win32: close() -> closesocket() */
        shutdown(fd, SD_BOTH);
        closesocket(fd);
#else
        close(fd);
#endif /* win32 */
        LOG(llevError, "error on listen\n");
    }

    return fd;
}
#else
int create_socket()
{
    /*
     * this function create a socket on the first available protocol. If there
     * is only one available it is no problem at all. But for IPv6 system it
     * requires that IPv6 sockets can handle IPv4 connections too, which is the
     * case on most system. It would be better to try to create sockets for all
     * protocols, but that requires that the server can handle multiple listen
     * sockets (coming soon). Creating a IPv4 socket will fail when an IPv6
     * socket that handles IPv4 already has been create, so failures will have
     * to be ignored as long as at least one socket is created.
     */
    int fd = -1;
    struct addrinfo hints, *res, *ai;
    char portstr[NI_MAXSERV];

    LOG(llevInfo, "create_socket(): check protocol\n");
    memset(&hints, 0, sizeof(struct addrinfo));

    hints.ai_flags    = AI_PASSIVE;
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    sprintf(portstr, "%d", settings.csport);
    if (getaddrinfo(NULL, portstr, &hints, &res) != 0)
        return -1;

    for (ai = res; ai != NULL; ai = ai->ai_next)
    {
       LOG(llevInfo,"checking family:%d socktype:%d protocol:%d\n",ai->ai_family,ai->ai_socktype,ai->ai_protocol);
        fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
        if (fd <= 0)
            continue;

        setsockopts(fd);

        if (bind(fd, ai->ai_addr, ai->ai_addrlen) == 0)
            break;

        close(fd);
        fd = -1;
    }

    freeaddrinfo(res);

    if (listen(fd, QUEUE_LEN) < 0)
    {
        close(fd);
        return -1;
    }

    return fd;
}
#endif

/* This sets up the socket and reads all the image information into memory. */
void init_ericserver(void)
{
    int oldbufsize;
    unsigned int buflen  = sizeof(int);
#ifndef WIN32 /* non windows */

#ifdef HAVE_SYSCONF
    socket_info.max_filedescriptor = sysconf(_SC_OPEN_MAX);
#else
#  ifdef HAVE_GETDTABLESIZE
    socket_info.max_filedescriptor = getdtablesize();
#  else
    "Unable to find usable function to get max filedescriptors";
#  endif
#endif

#else /* ***win32  -  we init a windows socket */
    WSADATA w;

    socket_info.max_filedescriptor = 1; /* used in select, ignored in winsockets */
    WSAStartup(0x0101, &w);             /* this setup all socket stuff */
    /* ill include no error tests here, winsocket 1.1 should always work */
    /* except some old win95 versions without tcp/ip stack */

#endif /* win32 */

    socket_info.timeout.tv_sec = 0;
    socket_info.timeout.tv_usec = 0;
    socket_info.nconns = 0;

    LOG(llevDebug, "Initialize new client/server data\n");
    init_sockets = malloc(sizeof(NewSocket));
    memset(init_sockets,0,sizeof(NewSocket));
    socket_info.allocated_sockets = 1;

    init_sockets[0].fd = create_socket();
    if (init_sockets[0].fd == -1)
        LOG(llevError, "ERROR: init_ericserver(): Error creating socket on port\n");
    init_sockets[0].status = Ns_Wait;

    /* under some OS the send and read buffer size will 2 times our default size. */
    getsockopt(init_sockets[0].fd, SOL_SOCKET, SO_SNDBUF, (char *) &oldbufsize, &buflen);
    if(oldbufsize > send_bufsize)
    {
        send_bufsize = (int)((float)send_bufsize*((float)send_bufsize/(float)oldbufsize));

        if (setsockopt(init_sockets[0].fd, SOL_SOCKET, SO_SNDBUF, (char *) &send_bufsize, sizeof(send_bufsize)))
            LOG(llevError, "error on setsockopt SO_SNDBUF\n");
        LOG(llevDebug, "InitSocket: send buffer adjusted to %d bytes! (old value: %d bytes)\n", send_bufsize, oldbufsize);
    }

    getsockopt(init_sockets[0].fd, SOL_SOCKET, SO_RCVBUF, (char *) &oldbufsize, &buflen);
    if(oldbufsize > read_bufsize)
    {
        read_bufsize = (int)((float)read_bufsize*((float)read_bufsize/(float)oldbufsize));

        if (setsockopt(init_sockets[0].fd, SOL_SOCKET, SO_RCVBUF, (char *) &read_bufsize, sizeof(read_bufsize)))
            LOG(llevError, "error on setsockopt SO_RCVBUF\n");

        LOG(llevDebug, "InitSocket: read buffer adjusted to %d bytes! (old value: %d bytes)\n", read_bufsize, oldbufsize);
    }
}

NewSocket *socket_get_available(void)
{
	int newsocknum = 0;

	/* If this is the case, all sockets currently in used */
	if (socket_info.allocated_sockets <= socket_info.nconns + 1)
	{
		init_sockets = realloc(init_sockets, sizeof(NewSocket) * (socket_info.nconns + 2));
		LOG(llevDebug, "(new sockets: %d (old# %d)) ", (socket_info.nconns - socket_info.allocated_sockets) + 2,
			socket_info.allocated_sockets);
		if (!init_sockets)
			LOG(llevError, "\nERROR: doeric_server(): out of memory\n");

		do
		{
			newsocknum = socket_info.allocated_sockets;
			socket_info.allocated_sockets++;
			memset(&init_sockets[newsocknum],0, sizeof(NewSocket));
			init_sockets[newsocknum].status = Ns_Avail;
		}
		while (socket_info.allocated_sockets <= socket_info.nconns + 1);
	}
	else
	{
		int j;

		for (j = 1; j < socket_info.allocated_sockets; j++)
		{
			if (init_sockets[j].status == Ns_Avail)
			{
				newsocknum = j;
				break;
			}
		}
	}

	return &init_sockets[newsocknum];
}

/* Free's all the memory that ericserver allocates. */
void free_all_newserver()
{
    LOG(llevDebug, "Freeing all new client/server information.\n");
    /* for clean memory remove we must loop init_sockets to free the buffers */
    free(init_sockets);
}
