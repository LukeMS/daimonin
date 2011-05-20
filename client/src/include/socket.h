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

    The author can be reached via e-mail to info@daimonin.org
*/

#ifndef __SOCKET_H
#define __SOCKET_H

#define SOCKET_NO -1

typedef struct _command_buffer
{
    struct _command_buffer *next; /* Next in queue */
    struct _command_buffer *prev; /* Previous in queue */
    int len;
    uint8 data[0];
}
command_buffer;

/* ClientSocket could probably hold more of the global values - it could
* probably hold most all socket/communication related values instead
* of globals.
*/
/* with the binary client protocol, ClientSocket only hold ATM the socket identifier.
* we can safely add here more features like latency counters, pings or statistics.
* MT/2008
*/
typedef struct ClientSocket
{
    SOCKET      fd;
    const char *host;
    uint16      port;
    uint32      ping;
}
ClientSocket;

extern ClientSocket csocket;

/* flags for send_command_binary() */
#define SEND_CMD_FLAG_DYNAMIC   0 /* data tail length can vary, add 2 length bytes */
#define SEND_CMD_FLAG_STRING    1 /* add a '\0' to the outbuffer string as sanity set */
#define SEND_CMD_FLAG_FIXED     2 /* the the command as fixed, without length tag (server knows length) */

extern int             send_command_binary(int cmd, char *body, int len,
                                           int flags);
extern void            command_buffer_free(command_buffer *buf);
extern command_buffer *get_next_input_command(void);
extern void            socket_thread_start(void);
extern void            socket_thread_stop(void);
extern int             handle_socket_shutdown();
extern uint8           SOCKET_InitSocket(void);
extern uint8           SOCKET_DeinitSocket(void);
extern uint8           SOCKET_OpenSocket(SOCKET *socket_temp, char *host,
                                         int port);
extern uint8           SOCKET_OpenClientSocket(struct ClientSocket *csock,
                                               char *host, int port);
extern uint8           SOCKET_CloseSocket(SOCKET socket);
extern uint8           SOCKET_CloseClientSocket(struct ClientSocket *csock);
extern int             SOCKET_GetError(void);  /* returns socket error */
extern int             read_metaserver_data(SOCKET fd);
extern char           *get_ip_from_hostname(char *host, char *ip_buf);

#endif /* ifndef __SOCKET_H */
