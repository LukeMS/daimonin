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

    The author can be reached via e-mail to daimonin@nord-com.net
*/

#include <include.h>

int SOCKET_GetError()
{
#ifdef __WIN_32
	return(WSAGetLastError ());
#elif __LINUX
	return errno;
#endif
};

#ifdef __WIN_32
/* this readsfrom fd and puts the data in sl.  We return true if we think
 * we have a full packet, 0 if we have a partial packet.  The only processing
 * we do is remove the intial size value.  len (As passed) is the size of the
 * buffer allocated in the socklist.  We make the assumption the buffer is
 * at least 2 bytes long.
 */
 int read_socket(int fd, SockList *sl, int len)
{
	int stat,toread,readsome=0;
    extern int errno;

    /* We already have a partial packet */
    if (sl->len<2) 
	{
		stat=recv(fd, sl->buf + sl->len, 2-sl->len,0);

		if (stat<0) 
		{
			if ((stat==-1) && WSAGetLastError() !=WSAEWOULDBLOCK)
			{
				LOG(LOG_ERROR,"ReadPacket got error %d, returning -1\n",WSAGetLastError());
		                draw_info("WARNING: Lost or bad server connection.",COLOR_RED);                
				return -1;
			}
			return 0;
		}
		if (stat==0) 
		{
            		draw_info("WARNING: Server read package error.",COLOR_RED);                
			return -1;
		}
		sl->len += stat;
		if (stat<2) 
			return 0;	/* Still don't have a full packet */
		readsome=1;
    }

    /* Figure out how much more data we need to read.  Add 2 from the
     * end of this - size header information is not included.
     */
    toread = 2+(sl->buf[0] << 8) + sl->buf[1] - sl->len;
    if ((toread + sl->len) > len)
	{
	        draw_info("WARNING: Server read package error.",COLOR_RED);                
		LOG(LOG_ERROR,"SockList_ReadPacket: Want to read more bytes than will fit in buffer.\n");
		/* return error so the socket is closed */
		return -1;
    }
	do 
	{
		stat=recv(fd, sl->buf + sl->len, toread,0);

		if (stat<0) 
		{
			if ((stat==-1) && WSAGetLastError() !=WSAEWOULDBLOCK)
			{
				LOG(LOG_ERROR,"ReadPacket got error %d, returning 0",WSAGetLastError());
		                draw_info("WARNING: Lost or bad server connection.",COLOR_RED);                
				return -1;
			}
			return 0;
		}
		if (stat==0)
		{
	            draw_info("WARNING: Server read package error.",COLOR_RED);                
			return -1;
		}
		sl->len += stat;
		toread -= stat;
		if (toread==0)
			return 1;
		if (toread < 0)
		{
			LOG(LOG_ERROR,"SockList_ReadPacket: Read more bytes than desired.");
            		draw_info("WARNING: Server read package error.",COLOR_RED);                
			return -1;
		}
	} 
	while (toread>0);

    return 0;
}

/* This writes data to the socket.  we precede the len information on the
 * packet.  Len needs to be passed here because buf could be binary
 * data
 */
int write_socket(int fd, unsigned char *buf, int len)
{
    int amt=0;
    unsigned char *pos=buf;

    /* If we manage to write more than we wanted, take it as a bonus */
    while (len>0) {

	amt=send(fd, pos, len,0);

	if (amt == -1 && WSAGetLastError() !=WSAEWOULDBLOCK) 
	{
	    LOG(LOG_ERROR,"New socket write failed (wsb) (%d).\n", WSAGetLastError());
            draw_info("SOCKET ERROR: Server write failed.",COLOR_RED);                
	    return -1;
	}
	if (amt==0) {
	    LOG(LOG_ERROR,"Write_To_Socket: No data written out (%d).\n",WSAGetLastError());
            draw_info("SOCKET ERROR: No data written out",COLOR_RED);                
	    return -1;
	}
	len -= amt;
	pos += amt;
    }
    return 0;
}


Boolean SOCKET_InitSocket(void)
{
	WSADATA w;
	int error;

	csocket.fd=SOCKET_NO;
	csocket.cs_version=0;

	SocketStatusErrorNr = 0;	
	error = WSAStartup (0x0101,&w);
	if (error)
	{
		LOG(LOG_ERROR,"Error:  Error init starting Winsock: %d!\n", error);
		return(FALSE);
	}
	
	if (w.wVersion!=0x0101)
	{
		LOG(LOG_ERROR,"Error:  Wrong WinSock version!\n");
		return(FALSE);
	}

	return(TRUE);
}


Boolean SOCKET_DeinitSocket(void)
{
	WSACleanup ();				/* drop socket */
	return(TRUE);
}

Boolean SOCKET_OpenSocket(SOCKET *socket_temp, struct ClientSocket *csock, char *host, int port)
{
        struct protoent *protox;
        int error, temp;
        struct hostent *hostbn;
        int oldbufsize;
        int newbufsize=65535, buflen=sizeof(int);
        uint32 start_timer;

        protox = getprotobyname("tcp");
        if (protox == (struct protoent  *) NULL)
        {
                LOG(LOG_ERROR,"Error:  Can't get protobyname\n");
                return(FALSE);
        }

        *socket_temp = socket(PF_INET, SOCK_STREAM, protox->p_proto);
        csocket.inbuf.buf=(unsigned char *)malloc(MAXSOCKBUF);
        csocket.inbuf.len=0;
        insock.sin_family = AF_INET;
        insock.sin_port = htons((unsigned short)port);

        if(ioctlsocket(*socket_temp, FIONBIO , &temp) == -1)
        {
                LOG(LOG_ERROR,"ERROR!!!: ioctlsocket(*socket_temp, FIONBIO , &temp)\n");
                *socket_temp = SOCKET_NO;
                return(FALSE);
        }

        if (isdigit(*host))
                insock.sin_addr.s_addr = inet_addr(host);
        else
        {
                hostbn = gethostbyname(host);
                if (hostbn == (struct hostent *) NULL)
                {
                        LOG(LOG_ERROR,"Unknown host: %s\n",host);
                        *socket_temp = SOCKET_NO;
                        return(FALSE);
                }
                memcpy(&insock.sin_addr, hostbn->h_addr, hostbn->h_length);
        }

        csock->command_sent=0;
        csock->command_received=0;
        csock->command_time=0;

        temp = 1;	/* non-block */

        if(ioctlsocket(*socket_temp, FIONBIO , &temp) == -1)
        { 
                LOG(LOG_ERROR,"ERROR: ioctlsocket(*socket_temp, FIONBIO , &temp)\n");
                *socket_temp = SOCKET_NO;
                return(FALSE);
        }

        error=0;

        start_timer= SDL_GetTicks();
        while(connect(*socket_temp,(struct sockaddr *)&insock,sizeof(insock))==SOCKET_ERROR)
        {
                Sleep(3);

                /* timeout.... without connect will REALLY hang a long time */
                if(start_timer+SOCKET_TIMEOUT_SEC*1000 < SDL_GetTicks())
                {
                        *socket_temp = SOCKET_NO;
                        return(FALSE);
                }

                SocketStatusErrorNr=WSAGetLastError();
                if(SocketStatusErrorNr == WSAEISCONN )	/* we have a connect! */
                        break;

                if(SocketStatusErrorNr == WSAEWOULDBLOCK || SocketStatusErrorNr==WSAEALREADY||
                        (SocketStatusErrorNr ==WSAEINVAL&&error))	/* loop until we finished */
                {
                        error=1;
                        continue;
                }

                LOG(LOG_MSG, "Connect Error:  %d", SocketStatusErrorNr);
                *socket_temp = SOCKET_NO;
                return(FALSE);
        }
        /* we got a connect here! */

        if (getsockopt(*socket_temp,SOL_SOCKET,SO_RCVBUF, (char*)&oldbufsize, &buflen)==-1)
                oldbufsize=0;

        if (oldbufsize<newbufsize)
        {
                if(setsockopt(*socket_temp,SOL_SOCKET,SO_RCVBUF, (char*)&newbufsize, sizeof(&newbufsize)))
                {
                        setsockopt(*socket_temp,SOL_SOCKET,SO_RCVBUF, (char*)&oldbufsize, sizeof(&oldbufsize));
                }
        }

        LOG(LOG_MSG, "Connected to %s:%d\n", host, port);
        return(TRUE);
}

/* we used our core connect routine to connect to metaserver, this is the special
   read one.*/

void read_metaserver_data(void)
{
	int stat,temp;
	char *ptr, *buf;

	ptr = (char*)malloc(MAX_METASTRING_BUFFER);
	buf = (char*)malloc(MAX_METASTRING_BUFFER);
	temp=0;
	for(;;)
	{
		/* win32 style input */

		stat = recv (csocket.fd, ptr, MAX_METASTRING_BUFFER, 0);
		if ((stat==-1) && WSAGetLastError() !=WSAEWOULDBLOCK)
		{
			LOG(LOG_ERROR,"Error reading metaserver data!: %d\n", WSAGetLastError());
			break;
		}
		else if(stat > 0)
		{
			if(temp + stat >=MAX_METASTRING_BUFFER)
			{
				memcpy(buf+temp, ptr,temp + stat-MAX_METASTRING_BUFFER-1);
                temp+=stat;
				break;
			}
			memcpy(buf+temp, ptr,stat);
			temp+=stat;
		}
		else if(stat == 0)
		{
			/* connect closed by meta */
			break;
		}

	}
	buf[temp]=0;
   LOG(0,"GET: %s\n", buf);
    parse_metaserver_data(buf);
	FreeMemory(&buf);
	FreeMemory(&ptr);
}

Boolean SOCKET_CloseSocket(SOCKET socket_temp)
{
	/* seems differents sockets have different way to shutdown connects??
	 * win32 needs this

	 * hard way, normally you should wait for a read() == 0...
     */
	if (socket_temp == SOCKET_NO)
		return(TRUE);
	
	shutdown(socket_temp, SD_BOTH );
	closesocket(socket_temp);
	FreeMemory(&csocket.inbuf.buf);
	csocket.fd=SOCKET_NO;	
	return(TRUE);
}

#elif __LINUX
Boolean SOCKET_OpenSocket(SOCKET *socket_temp, struct ClientSocket *csock, char *host, int 
port)
{
        struct protoent *protox;
        int oldbufsize, newbufsize=65535, buflen=sizeof(int), temp;
        struct sockaddr_in insock;

        /*LOG(LOG_DEBUG, "Now trying to open connection to %s %i\n", host, port);*/
        printf("Opening to %s %i\n", host, port);
        protox = getprotobyname("tcp");

        if (protox == (struct protoent  *) NULL)
        {
                fprintf(stderr, "Error getting prorobyname (tcp)\n");
                return FALSE;
        }
        *socket_temp = socket(PF_INET, SOCK_STREAM, protox->p_proto);

        if (*socket_temp ==-1)
        {
                perror("init_connection:  Error on socket command.\n");
                *socket_temp = SOCKET_NO;
                return FALSE;
        }
        csocket.inbuf.buf=(unsigned char *)_malloc(MAXSOCKBUF,"SOCKET_OpenSocket(): MAXSOCKBUF");
        csocket.inbuf.len=0;
        insock.sin_family = AF_INET;
        insock.sin_port = htons((unsigned short)port);
        if (isdigit(*host))
                insock.sin_addr.s_addr = inet_addr(host);
        else
        {
                struct hostent *hostbn = gethostbyname(host);
                if (hostbn == (struct hostent *) NULL)
                {
                        fprintf(stderr,"Unknown host: %s\n",host);
                        return FALSE;
                }
                memcpy(&insock.sin_addr, hostbn->h_addr, hostbn->h_length);
        }
        csock->command_sent=0;
        csock->command_received=0;
        csock->command_time=0;

        temp = 1;	/* non-block*/

        if (connect(*socket_temp,(struct sockaddr *)&insock,sizeof(insock)) == (-1))
        {
                perror("Can't connect to server");
                return FALSE;
        }
        if (fcntl(*socket_temp, F_SETFL, O_NDELAY)==-1)
        {
                fprintf(stderr,"InitConnection:  Error on fcntl.\n");
        }
        if (getsockopt(*socket_temp,SOL_SOCKET,SO_RCVBUF, (char*)&oldbufsize, &buflen)==-1)
                oldbufsize=0;

        if (oldbufsize<newbufsize)
        {
                if(setsockopt(*socket_temp,SOL_SOCKET,SO_RCVBUF, (char*)&newbufsize, sizeof(&newbufsize)))
                {
                        LOG(1,"InitConnection: setsockopt unable to set output buf size to %d\n", newbufsize);
                        setsockopt(*socket_temp,SOL_SOCKET,SO_RCVBUF, (char*)&oldbufsize, sizeof(&oldbufsize));
                }
        }
        return TRUE;
}

Boolean SOCKET_InitSocket(void)
{
        return TRUE;
}

Boolean SOCKET_DeinitSocket(void)
{
        return TRUE;
}

Boolean SOCKET_CloseSocket(SOCKET socket_temp)
{
	if (socket_temp == SOCKET_NO)

		return(TRUE);

	
    	close(socket_temp);
	FreeMemory((void*) &csocket.inbuf.buf);
	csocket.fd=SOCKET_NO;	
	return(TRUE);
}

void read_metaserver_data(void)
{
        int stat,temp;
        char *ptr, *buf;

        ptr = (char*)_malloc(MAX_METASTRING_BUFFER,"read_metaserver_data(): metastring buffer1");
        buf = (char*)_malloc(MAX_METASTRING_BUFFER,"read_metaserver_data(): metastring buffer2");
        temp=0;
        for(;;)
        {
                do
                {
                        stat = recv (csocket.fd, ptr, MAX_METASTRING_BUFFER, 0);
                } while (stat == -1);

                /*if ((stat==-1) && WSAGetLastError() !=WSAEWOULDBLOCK)*/
                if (stat==-1)
                {
                        LOG(LOG_ERROR,"Error reading metaserver data!\n");
                        break;
                }
                else if(stat > 0)
                {
                        if(temp + stat >=MAX_METASTRING_BUFFER)
                        {
                                memcpy(buf+temp, ptr,temp + stat-MAX_METASTRING_BUFFER-1);
                                temp+=stat;
                                break;
                        }
                        memcpy(buf+temp, ptr,stat);
                        temp+=stat;
                }
                else if(stat == 0)
                {
                        /* connect closed by meta */
                       break;
                }
        }
        buf[temp]=0;
        parse_metaserver_data(buf);
        FreeMemory((void **)(&buf));
        FreeMemory((void **)(&ptr));
}


int read_socket(int fd, SockList *sl, int len)
{
        int stat,toread,readsome=0;
        extern int errno;

        /* We already have a partial packet */
        if (sl->len<2)
        {
                do
                {
                        stat=read(fd, sl->buf + sl->len, 2-sl->len);
                } while ((stat==-1) && (errno==EINTR));
                if (stat<0)
                {
                        /* In non blocking mode, EAGAIN is set when there is no
                        * data available.
                        */
                        if (errno!=EAGAIN && errno!=EWOULDBLOCK)
                        {
                                LOG(LOG_DEBUG,"ReadPacket got error %d, returning 0",errno);
		                draw_info("WARNING: Lost or bad server connection.",COLOR_RED);
				return -1;                
                        }
                        return 0;
                }
                if (stat==0) 
		{
            		draw_info("WARNING: Server read package error.",COLOR_RED);                
			return -1;
		}
                sl->len += stat;
                if (stat<2) 
			return 0;	/* Still don't have a full packet */
                readsome=1;
        }
        /* Figure out how much more data we need to read.  Add 2 from the
        * end of this - size header information is not included.
        */
        toread = 2+(sl->buf[0] << 8) + sl->buf[1] - sl->len;
        if ((toread + sl->len) > len)
        {
	        draw_info("WARNING: Server read package error.",COLOR_RED);                
                LOG(LOG_ERROR,"SockList_ReadPacket: Want to read more bytes than will fit in buffer.\n");
                /* return error so the socket is closed */
                return -1;
        }
        do
        {
                do
                {
                        stat = read(fd, sl->buf+ sl->len, toread);
                } while ((stat<0) && (errno==EINTR));
                if (stat<0)
                {
                        if (errno!=EAGAIN && errno!=EWOULDBLOCK)
                        {
                                LOG(LOG_DEBUG,"ReadPacket got error %d, returning 0",errno);
		                draw_info("WARNING: Lost or bad server connection.",COLOR_RED);                
				return -1;
                        }
                        return 0;
                }
                if (stat==0)
		{
		        draw_info("WARNING: Server read package error.",COLOR_RED);                
			return -1;
		}
               sl->len += stat;
                toread -= stat;
                if (toread==0) return 1;
                if (toread < 0)
                {
			LOG(LOG_ERROR,"SockList_ReadPacket: Read more bytes than desired.");
            		draw_info("WARNING: Server read package error.",COLOR_RED);                
			return -1;
                }
        } while (toread>0);
        return 0;
}

/* This writes data to the socket.  we precede the len information on the
 * packet.  Len needs to be passed here because buf could be binary
 * data
 */
int write_socket(int fd, unsigned char *buf, int len)
{
        int amt=0;
        unsigned char *pos=buf;

        /* If we manage to write more than we wanted, take it as a bonus */
        while (len>0)
        {
                do
                {
                        amt=write(fd, pos, len);
                } while ((amt<0) && (errno==EINTR));

                if (amt < 0)
                { /* We got an error */
                        LOG(LOG_ERROR,"New socket (fd=%d) write failed.\n", fd);
	                draw_info("SOCKET ERROR: Server write failed.",COLOR_RED);                
                        return -1;
                }
                if (amt==0)
                {
                        LOG(LOG_ERROR,"Write_To_Socket: No data written out.\n");
		        draw_info("SOCKET ERROR: No data written out",COLOR_RED);                
	    		return -1;
                }
                len -= amt;
                pos += amt;
        }
        return 0;
}

#endif
