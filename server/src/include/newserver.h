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
/*
    newserver.h defines various structures and values that are use for the
    new client server communication method.  Values defined here are only
    used on the server side code.  For shared client/server values, see
    newclient.h
*/

#ifndef NEWSERVER_H
#define NEWSERVER_H

#define NUM_LOOK_OBJECTS 15

/* True max is 16383 given current map compaction method */
#define MAXFACENUM  5000
#define MAXANIMNUM  2000

#define FACE_TYPES  1
#define PNG_FACE_INDEX  0

/* This contains basic information on the socket structure.  status is its
 * current state.  we set up our on buffers for sending/receiving, so we can
 * handle some higher level functions.  fd is the actual file descriptor we
 * are using.
 */
enum Sock_Status
{
    Ns_Avail,
    Ns_Wait,
    Ns_Add,
    Ns_Login,
    Ns_Playing,
    Ns_Zombie,
    Ns_Dead
};

/* temp. use for read */
typedef struct ReadList_struct
{
    int             len;
    int             pos;
    int             cmd;    /* used when we already have cmd tag and wait for data tail */
    int             toread; /* length of data tail */
    unsigned char  *buf;
} ReadList ;

/* 3 standard buffers for allocations with SOCKET_BUFFER_REQUEST using without the header size
 * so it fits clean in a buffer /256 bytes
 */
#define SOCKBUF_HEADER_DEFAULT 3 /* thats the standard 3 byte header <cmd><length> */
#define SOCKBUF_HEADER_EXTENDED 5

#define SOCKET_SIZE_SMALL (256-SOCKBUF_HEADER_DEFAULT)
#define SOCKET_SIZE_MEDIUM (2048-SOCKBUF_HEADER_DEFAULT)
#define SOCKET_SIZE_HUGE (4096-SOCKBUF_HEADER_DEFAULT)

/* when we assemble and broadcast a non static sockbuf, SOCKBUF_ASSEMBLE_FREE() must be called
 * in the case the sockbuf is not inserted in any queue - or we never free it!
 */
#define SOCKBUF_COMPOSE(_c_,_ob_,_cb_,_l_,_f_) compose_socklist_buffer((_c_),(_ob_),(_cb_),(_l_),(_f_));
#define SOCKBUF_COMPOSE_FREE(_sb) \
	{if(!((_sb)->instance) && !((_sb)->flags & SOCKBUF_FLAG_STATIC)) \
	return_poolchunk((_sb), (_sb)->pool);}

/* we define this as macros so we can easily add stuff like for example a multiplexer */
#define SOCKBUF_REQUEST_BUFFER(_ns_,_len_) socket_buffer_request((_ns_),(_len_))
#define SOCKBUF_REQUEST_FINISH(_ns_,_cmd_,_len_) socket_buffer_request_finish((_ns_),(_cmd_),(_len_))
#define SOCKBUF_REQUEST_RESET(_ns_) socket_buffer_request_reset((_ns_))
/* sometimes we want know how much there is in <data> */
#define SOCKBUF_REQUEST_BUFSIZE(_sb_) ((_sb_)->len -(_sb_)->request_len-((*((char *)((_sb_)->buf+(_sb_)->request_len)))&0x80?5:3))

/* helper function for SockBuf_xxx to get the right sockbuf */
#define ACTIVE_SOCKBUF(_ns_) ((_ns_)->sockbuf)

/* help define to hide the exactly handling inside the write module */
#define SOCKBUF_ADD_TO_SOCKET(_ns_, _sbuf_) socket_buffer_enqueue((_ns_),(_sbuf_))

/* flag to tell sockbuf functions that len is not set or used */
#define SOCKBUF_DYNAMIC (-1)

/* some flags */
#define SOCKBUF_FLAG_STATIC		0x01 /* the will be not given back to mempool after dequeueing */

/* Contains the base information we use to make up a packet we want to send. */
typedef struct _sockbuf_struct
{
	void            *next;
	void            *last;
	struct mempool  *pool;		/* intern: memory pool maker for mempool */
	void			*ns;
	int				request_len; /* the len/start position after request BEFORE header & cms part */
	int				instance;	/* counter for mempool for multi enqueued buffers */
	int				bufsize;	/* size of buf */
	int				len;		/* length of data i buf */
	int             pos;		/* start point of unsend data in buf */
	int				flags;		/* status flags */
	unsigned char	*buf;
} sockbuf_struct;

/* help functions to write in requested socket buffers */
#define SockBuf_AddChar(_sl_,_c_) \
	{if((int)((_sl_)->len+1)>=(_sl_)->bufsize) \
	(_sl_)=socket_buffer_adjust((_sl_),1); \
	( *((uint8 *)((_sl_)->buf+(_sl_)->len++)) =(char)(_c_) );}
#define SockBuf_AddShort(_sl_,_c_) \
	{if((int)((_sl_)->len+2)>=(_sl_)->bufsize) \
	(_sl_)=socket_buffer_adjust((_sl_),2); \
	*((uint16 *)((_sl_)->buf+(_sl_)->len))=(uint16)(_c_);(_sl_)->len+=2;}
#define SockBuf_AddInt(_sl_,_c_) \
	{if((int)((_sl_)->len+4)>=(_sl_)->bufsize) \
	(_sl_)=socket_buffer_adjust((_sl_),4); \
	*((uint32 *)((_sl_)->buf+(_sl_)->len))=(uint32)(_c_);(_sl_)->len+=4;}
#define SockBuf_AddString(_sl,_data,_len) \
	{if((int)((_sl)->len+(_len)+1)>=(_sl)->bufsize) \
	(_sl)=socket_buffer_adjust((_sl),(_len)+1); \
	memcpy((_sl)->buf+(_sl)->len,(_data),(_len)); \
	(_sl)->len+=(_len); \
	(_sl)->buf[(_sl)->len++] = (char) 0;}


/* helper macro to add a single command to a socket */
#define Write_Command_To_Socket(_ns_,_cmd_) \
	{SOCKBUF_REQUEST_BUFFER((_ns_), 0); \
	SOCKBUF_REQUEST_FINISH((_ns_), (_cmd_), SOCKBUF_DYNAMIC);}

/* helper macro to add a command with string data block */
#define Write_String_To_Socket(_ns_,_cmd_,_buf_,_len_) \
	{SOCKBUF_REQUEST_BUFFER((_ns_),(_len_)+1); \
	if(_buf_) \
	SockBuf_AddString(ACTIVE_SOCKBUF(_ns_),(_buf_),(_len_)); \
	SOCKBUF_REQUEST_FINISH((_ns_), (_cmd_), SOCKBUF_DYNAMIC);}

/* different fixed sized command nodes */
typedef struct _command_struct
{
        void            *next;
        void            *last;
        struct mempool  *pool;
        int              cmd;       /* thats our binary command tag */
        int              len;       /* the length of the data inside buf[] */
        char            *buf;       /* the data tail when len > 0 */
} command_struct;

typedef struct NewSocket_struct
{
        int                 fd;
        struct pl_player    *pl;                /* if != NULL this socket is part of a player struct */
        command_struct      *cmd_start;          /* pointer to the list of incoming commands in process */
        command_struct      *cmd_end;
		sockbuf_struct      *sockbuf_start;		/* pointer to the list of prepared outgoing packages in process */
		sockbuf_struct      *sockbuf_end;
		sockbuf_struct      *sockbuf;			/* thats or CURRENT buffer we working on - this can be filled up */
		int					sockbuf_pos;		/* read pos in sockbuf (needed for multi-enqueued sockbufs */
		int					sockbuf_len;		/* sic */

		int					sockbuf_nrof;		/* number of the queued socket buffers */
		int					sockbuf_bytes;		/* number of bytes in all queued socket buffers we must transfer */
		struct Map          lastmap;            /* Thats the VISIBLE map area of the player, used to send to client */
        uint32              login_count;        /* if someone is to long idle in the login, we kick him here! */
        int                 mapx, mapy;         /* How large a map the client wants */
        int                 mapx_2, mapy_2;     /* same like above but /2 */
        uint32              cs_version;         /*client/server versions */
        uint32              sc_version;
        int                 pwd_try;            /* simple password guessing security */
        uint32              update_tile;        /* marker to see we must update the below windows of the tile the player is */
        char                ip_host[40];            /* IP as string */
        enum Sock_Status    status;
        ReadList            readbuf;            /* Raw data read in from the socket  */

        uint32              below_clear     : 1;        /* marker to map draw/draw below */
        uint32              idle_flag       : 1;        /* idle warning was given and we count for disconnect */
        uint32              addme           : 1;        /* important: when set, a "connect" was initizialised as "player" */
        uint32              sound           : 1;        /* does the client want sound */
        uint32              ext_title_flag  : 1;        /* send ext title to client */
        uint32              image2          : 1;        /* Client wants image2/face2 commands */
        uint32              setup           : 1;
        uint32              rf_settings     : 1;
        uint32              rf_skills       : 1;
        uint32              rf_spells       : 1;
        uint32              rf_anims        : 1;
        uint32              rf_bmaps        : 1;

        sint16              look_position;  /* start of drawing of look window */
        sint16              look_position_container;  /* start of drawing of look window for a container */
} NewSocket;

typedef void (*func_uint8_int_ns) (char *, int, NewSocket *);

typedef struct CmdMapping_struct
{
    /* 0= no data tail, -1 = dynamic length, read in 2 bytes, x = fixed tail length */
    int                 data_len; 
    func_uint8_int_ns   cmdproc;
}_CmdMapping;

extern _CmdMapping cs_commands[];


typedef struct Socket_Info_struct
{
        struct timeval  timeout;    /* Timeout for select */
    int             max_filedescriptor; /* max filedescriptor on the system */
    int             nconns;     /* Number of connections */
    int             allocated_sockets;  /* number of allocated in init_sockets */
} Socket_Info;

#endif
