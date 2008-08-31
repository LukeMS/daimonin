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
#if !defined(__SDLSOCKET_H)
#define __SDLSOCKET_H

#define SOCKET_NO -1

typedef struct _command_buffer
{
    struct _command_buffer *next; /* Next in queue */
    struct _command_buffer *prev; /* Previous in queue */
    int len;
    uint8 data[0];
}
command_buffer;

extern void command_buffer_free(command_buffer *buf);
/* extern _command_buffer_read *read_cmd_start; */
extern int send_command_binary(uint8 cmd, uint8 *body, unsigned int len);
extern int send_socklist(int fd, SockList msg);
extern command_buffer *get_next_input_command(void);
extern void socket_thread_start(void);
extern void socket_thread_stop(void);
extern int handle_socket_shutdown();
extern Boolean  SOCKET_InitSocket(void);
extern Boolean  SOCKET_DeinitSocket(void);
extern Boolean  SOCKET_OpenSocket(SOCKET *socket_temp, char *host, int port);
extern Boolean  SOCKET_OpenClientSocket(struct ClientSocket *csock, char *host, int port);
extern Boolean  SOCKET_CloseSocket(SOCKET socket);
extern Boolean  SOCKET_CloseClientSocket(struct ClientSocket *csock);
extern int      SOCKET_GetError(void);  /* returns socket error */
extern int      read_metaserver_data(SOCKET fd);

#endif
