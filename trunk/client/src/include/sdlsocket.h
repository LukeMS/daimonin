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
#if !defined(__SDLSOCKET_H)
#define __SDLSOCKET_H

#define SOCKET_NO -1

extern Boolean  SOCKET_InitSocket(void);
extern Boolean  SOCKET_DeinitSocket(void);
extern Boolean  SOCKET_OpenSocket(SOCKET *socket_temp, struct ClientSocket *csock, char *host, int port);
extern Boolean  SOCKET_CloseSocket(SOCKET socket);
extern int      SOCKET_GetError(void);  /* returns socket error */

extern int      write_socket(int fd, unsigned char *buf, int len);
extern int      read_socket(int fd, struct SockList *sl, int len);

void            read_metaserver_data(void);

#endif
