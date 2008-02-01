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

/* socket.c mainly deals with initialization and higher level socket
 * maintenance (checking for lost connections and if data has arrived.)
 * The reading of data is handled in ericserver.c
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
#include "../zlib/zlib.h"

static int  send_bufsize = 24*1024;
static int  read_bufsize = 8*1024;


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
    ns->faceset = 0;
    ns->facecache = 0;
    ns->image2 = 0;
    ns->sound = 0;
    ns->ext_title_flag = 1;
    ns->map2cmd = 0;
    ns->darkness = 1;
    ns->mapx = 17;
    ns->mapy = 17;
    ns->mapx_2 = 8;
    ns->mapy_2 = 8;
    ns->version = 0;
    ns->setup = 0;
    ns->rf_settings = 0;
    ns->rf_skills = 0;
    ns->rf_spells = 0;
    ns->rf_anims = 0;
    ns->rf_bmaps = 0;
    ns->write_overflow = 0;

    ns->cmd_start = NULL;
    ns->cmd_end = NULL;
    /* we should really do some checking here - if total clients overflows
     * we need to do something more intelligent, because client id's will start
     * duplicating (not likely in normal cases, but malicous attacks that
     * just open and close connections could get this total up.
     */
    ns->readbuf.len = 0;
    ns->readbuf.pos = 0;
    if(!ns->readbuf.buf)
        ns->readbuf.buf = malloc(MAXSOCKBUF_IN);
    ns->readbuf.buf[0] = 0;

    ns->pwd_try=0;

    memset(&ns->lastmap, 0, sizeof(struct Map));

    ns->outputbuffer.start = 0;
    ns->outputbuffer.len = 0;
    strcpy(ns->ip_host, ip);

    socket_info.nconns++;
}

void setsockopts(int fd)
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
void init_ericserver()
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


#ifdef SERVER_SEND_FACES
    read_client_images();
#endif
    init_srv_files(); /* load all srv_xxx files or generate them */
}


/*******************************************************************************
 *
 * Start of functions dealing with freeing of the data.
 *
 ******************************************************************************/

/* Free's all the memory that ericserver allocates. */
void free_all_newserver()
{
    LOG(llevDebug, "Freeing all new client/server information.\n");
#ifdef SERVER_SEND_FACES
    free_socket_images();
#endif
    /* for clean memory remove we must loop init_sockets to free the buffers */
    free(init_sockets);
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

void    free_newsocket  (NewSocket *ns)
{
    unsigned char *tmp_read = ns->readbuf.buf;

    LOG(llevDebug, "Closing socket %d\n", ns->fd);
    close_newsocket(ns);

    /* clearout the socket but don't restore the buffers.
     * no need to malloc them again & again.
     */
    clear_read_buffer_queue(ns); /* give back the blocks to the mempools */
    memset(ns, 0, sizeof(ns));
    ns->readbuf.buf = tmp_read;
}

/* as long the server don't have a autoupdate/login server
 * as frontend we must serve our depending client files self.
 */
static void load_srv_files(char *fname, int id, int cmd)
{
    FILE   *fp;
    unsigned char *file_tmp, *comp_tmp;
    int     flen;
    unsigned long numread;
    struct stat statbuf;

    LOG(llevDebug, "Loading %s...", fname);
    if ((fp = fopen(fname, "rb")) == NULL)
        LOG(llevError, "\nERROR: Can not open file %s\n", fname);
    fstat(fileno(fp), &statbuf);
    flen = (int) statbuf.st_size;
    file_tmp = malloc(flen);
    numread = (unsigned long) fread(file_tmp, sizeof(char), flen, fp);
    /* get a crc from the unpacked file */
    SrvClientFiles[id].crc = crc32(1L, file_tmp, numread);
    SrvClientFiles[id].len_ucomp = numread;
    numread = flen * 2;
    comp_tmp = (unsigned char *) malloc(numread);
    compress2(comp_tmp, &numread, file_tmp, flen, Z_BEST_COMPRESSION);
    /* we prepare the files with the right commands - so we can flush
     * then direct from this buffer to the client.
     */
    if ((int) numread < flen)
    {
        /* copy the compressed file in the right buffer */
        SrvClientFiles[id].file = malloc(numread + 2);
        memcpy(SrvClientFiles[id].file + 2, comp_tmp, numread);
        SrvClientFiles[id].file[1] = (char) DATA_PACKED_CMD;
        SrvClientFiles[id].len = numread;
    }
    else
    {
        /* compress has no positive effect here */
        SrvClientFiles[id].file = malloc(flen + 2);
        memcpy(SrvClientFiles[id].file + 2, file_tmp, flen);
        SrvClientFiles[id].file[1] = 0;
        SrvClientFiles[id].len = -1;
        numread = flen;
    }
    SrvClientFiles[id].file[0] = BINARY_CMD_DATA;
    SrvClientFiles[id].file[1] |= cmd;
    free(file_tmp);
    free(comp_tmp);

    LOG(llevDebug, "(size: %d (%d) (crc uncomp.: %x)\n", SrvClientFiles[id].len_ucomp, numread, SrvClientFiles[id].crc);
    fclose(fp);
}

/* get the /lib/settings default file and create the
 * /data/client_settings with it.
 */
static void create_client_settings(void)
{
    char    buf[MAX_BUF*4];
    int     i;
    FILE   *fset_default, *fset_create;

    LOG(llevDebug, "Creating %s/client_settings...\n", settings.localdir);

    /* open default */
    sprintf(buf, "%s/client_settings", settings.datadir);
    if ((fset_default = fopen(buf, "rb")) == NULL)
        LOG(llevError, "\nERROR: Can not open file %s\n", STRING_SAFE(buf));

    /* delete our target - we create it new now */
    sprintf(buf, "%s/client_settings", settings.localdir);
    unlink(buf);

    /* open target client_settings */
    if ((fset_create = fopen(buf, "wb")) == NULL)
    {
        fclose(fset_default);
        LOG(llevError, "\nERROR: Can not open file %s\n", STRING_SAFE(buf));
    }

    /* copy default to target */
    while (fgets(buf, MAX_BUF, fset_default) != NULL)
        fputs(buf, fset_create);
    fclose(fset_default);

    /* now we add the server specific date
     * first: the exp levels!
    */
    sprintf(buf, "level %d\n", MAXLEVEL); /* param: number of levels we have */
    fputs(buf, fset_create);

    for (i = 0; i <= MAXLEVEL; i++)
    {
        sprintf(buf, "%x\n", new_levels[i]);
        fputs(buf, fset_create);
    }

    fclose(fset_create);
}

/* load all src_files we can send to client... client_bmaps is generated from
 * the server at startup out of the daimonin png file.
 */
void init_srv_files(void)
{
    char    buf[MAX_BUF];

    memset(&SrvClientFiles, 0, sizeof(SrvClientFiles));

    sprintf(buf, "%s/animations", settings.datadir);
    load_srv_files(buf, SRV_CLIENT_ANIMS, DATA_CMD_ANIM_LIST);

    sprintf(buf, "%s/client_bmaps", settings.localdir);
    load_srv_files(buf, SRV_CLIENT_BMAPS, DATA_CMD_BMAP_LIST);

    sprintf(buf, "%s/client_skills", settings.datadir);
    load_srv_files(buf, SRV_CLIENT_SKILLS, DATA_CMD_SKILL_LIST);

    sprintf(buf, "%s/client_spells", settings.datadir);
    load_srv_files(buf, SRV_CLIENT_SPELLS, DATA_CMD_SPELL_LIST);

    create_client_settings();
    sprintf(buf, "%s/client_settings", settings.localdir);
    load_srv_files(buf, SRV_CLIENT_SETTINGS, DATA_CMD_SETTINGS_LIST);
}

/* a connecting client has requested a srv_ file.
 * not that we don't know anything about the player
 * at this point - we got a open socket, a IP a matching
 * version and a usable setup string from the client.
 */
void send_srv_file(NewSocket *ns, int id)
{
    SockList    sl;

    sl.buf = (unsigned char *)SrvClientFiles[id].file;

    if (SrvClientFiles[id].len != -1)
        sl.len = SrvClientFiles[id].len + 2;
    else
        sl.len = SrvClientFiles[id].len_ucomp + 2;

    Send_With_Handling(ns, &sl);
}
