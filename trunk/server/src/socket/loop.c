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
#ifndef __CEXTRACT__
#include <sproto.h>
#include <sockproto.h>
#endif

#ifndef WIN32 /* ---win32 exclude unix headers */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#endif /* end win32 */

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#include <newserver.h>


static fd_set tmp_read, tmp_exceptions, tmp_write;

/*****************************************************************************
 * Start of command dispatch area.
 * The commands here are protocol commands. 
 ****************************************************************************/

/* Either keep this near the start or end of the file so it is
 * at least reasonablye easy to find.
 * There are really 2 commands - those which are sent/received
 * before player joins, and those happen after the player has joined.
 * As such, we have function types that might be called, so
 * we end up having 2 tables.
 */

typedef void (*func_uint8_int_ns) (char*, int, NewSocket *);

struct NsCmdMapping {
    char *cmdname;
    func_uint8_int_ns  cmdproc;
};

typedef void (*func_uint8_int_pl)(char*, int, player *);
struct PlCmdMapping {
    char *cmdname;
    func_uint8_int_pl cmdproc;
};

/*
 * CmdMapping is the dispatch table for the server, used in HandleClient,
 * which gets called when the client has input.  All commands called here 
 * use the same parameter form (char* data, int len, int clientnum.
 * We do implicit casts, because the data that is being passed is
 * unsigned (pretty much needs to be for binary data), however, most
 * of these treat it only as strings, so it makes things easier
 * to cast it here instead of a bunch of times in the function itself.
 */
/* the goal is to move all 'techical' commands to this position.
 * then we must but all this in a central function - commands.c i think.
 * I let the command to full names for better debugging - later we must think
 * about pack & packing, i think.  
 * Last goal is to reduce the single commands. Adding mapscroll in the mapupdate
 * command for example. MT-11-2002
 */
static struct PlCmdMapping plcommands[] = {
    { "ex",	ExamineCmd },
    { "ap",		ApplyCmd },
    { "mv",		MoveCmd },
    { "reply",		ReplyCmd},
    { "cm",	PlayerCmd},
    { "ncom",		(func_uint8_int_pl)NewPlayerCmd},
    { "lt",		LookAt},
    { "mapredraw",	MapRedrawCmd},	/* Added: phil */
    { "lock",		(func_uint8_int_pl)LockItem},
    { "mark",		(func_uint8_int_pl)MarkItem},
	{"/fire",		command_fire},
	{"fr",			command_face_request},
	{"nc",			command_new_char},
    { NULL, NULL}	/* terminator */
};

static struct NsCmdMapping nscommands[] = {
    { "addme",		AddMeCmd },
    { "askface",	SendFaceCmd},	/* Added: phil */
    { "requestinfo",	RequestInfo},
    { "setfacemode",	SetFaceMode},
    { "setsound",	SetSound},
    { "setup",		SetUp},
    { "version",	VersionCmd },
    { "rf",	RequestFileCmd },
    { NULL, NULL}	/* terminator */
};

/* RequestInfo is sort of a meta command - there is some specific
 * request of information, but we call other functions to provide
 * that information.
 */
void RequestInfo(char *buf, int len, NewSocket *ns)
{
    char    *params=NULL, *cp;
    /* No match */
    char bigbuf[MAX_BUF];
    int slen;

    /* Set up replyinfo before we modify any of the buffers - this is used
     * if we don't find a match.
     */
    /*strcpy(bigbuf,"replyinfo ");*/
    slen = 1;
	bigbuf[0] =BINARY_CMD_REPLYINFO;
	bigbuf[1]=0;
    safe_strcat(bigbuf, buf, &slen, sizeof(bigbuf));

    /* find the first space, make it null, and update the
     * params pointer.
     */
    for (cp = buf; *cp != '\0'; cp++)
	if (*cp==' ') {
	    *cp = '\0';
	    params = cp + 1;
	    break;
	}
    if (!strcmp(buf, "image_info")) send_image_info(ns, params);
    else if (!strcmp(buf,"image_sums")) send_image_sums(ns, params);
    else Write_String_To_Socket(ns, BINARY_CMD_REPLYINFO, bigbuf, len);
}


/* HandleClient is actually not named really well - we only get here once
 * there is input, so we don't do exception or other stuff here.
 * sock is the output socket information.  pl is the player associated
 * with this socket, null if no player (one of the init_sockets for just
 * starting a connection)
 */

void HandleClient(NewSocket *ns, player *pl)
{
	int len=0,i, cmd_count=0;
    unsigned char *data;

    /* Loop through this - maybe we have several complete packets here. */
    while (1)
	{
		/* If it is a player, and they don't have any speed left, we
		* return, and will read in the data when they do have time.
        */
		if (pl && pl->state==ST_PLAYING && pl->ob != NULL && pl->ob->speed_left < 0) 
			return;
	    
		i=SockList_ReadPacket(ns->fd, &ns->inbuf, MAXSOCKBUF-1);

		if (i<0) 
		{
#ifdef ESRV_DEBUG
			LOG(llevDebug,"HandleClient: Read error on connection player %s\n", (pl?pl->ob->name:"None"));
#endif
			/* Caller will take care of cleaning this up */
			ns->status =Ns_Dead;
			return;
		}

		/* Still dont have a full packet */
		if (i==0) 
			return;

		/* First, break out beginning word.  There are at least
		* a few commands that do not have any paremeters.  If
		* we get such a command, don't worry about trying
		* to break it up.
		*/
		data = (unsigned char *)strchr((char*)ns->inbuf.buf +2, ' ');
		if (data)
		{
			*data='\0';
			data++;
			len = ns->inbuf.len - (data - ns->inbuf.buf);
		}
		else 
			len=0;

		ns->inbuf.buf[ns->inbuf.len]='\0';  /* Terminate buffer - useful for string data */
		for (i=0; nscommands[i].cmdname !=NULL; i++)
		{
			if (strcmp((char*)ns->inbuf.buf+2,nscommands[i].cmdname)==0)
			{
				nscommands[i].cmdproc((char*)data,len,ns);
				ns->inbuf.len=0;
				goto next_command;
			}
		}
		/* Only valid players can use these commands */
		if (pl) 
		{
			for (i=0; plcommands[i].cmdname !=NULL; i++)
			{
				if (strcmp((char*)ns->inbuf.buf+2,plcommands[i].cmdname)==0) 
				{
					plcommands[i].cmdproc((char*)data,len,pl);
					ns->inbuf.len=0;
					goto next_command;
				}
			}
		}

		/* If we get here, we didn't find a valid command.  Logging
		* this might be questionable, because a broken client/malicious
		* user could certainly send a whole bunch of invalid commands.
		*/

		LOG(llevDebug,"Bad command from client (%s)\n",ns->inbuf.buf+2);
		ns->status =Ns_Dead;
		return;

		next_command:
		if(cmd_count++<20)
		{
			/* LOG(llevDebug,"MultiCmd: #%d /%s)\n", cmd_count, (char*)ns->inbuf.buf+2); */
			continue;
		}
		break;
    }
}


/*****************************************************************************
 *
 * Low level socket looping - select calls and watchdog udp packet
 * sending.
 *
 ******************************************************************************/

#ifdef WATCHDOG
/*
 * Tell watchdog that we are still alive
 * I put the function here since we should hopefully already be getting
 * all the needed include files for socket support 
 */

void watchdog(void)
{
  static int fd=-1;
  static struct sockaddr_in insock;

  if (fd==-1)
    {
      struct protoent *protoent;

      if ((protoent=getprotobyname("udp"))==NULL ||
        (fd=socket(PF_INET, SOCK_DGRAM, protoent->p_proto))==-1)
      {
        return;
      }
      insock.sin_family=AF_INET;
      insock.sin_port=htons((unsigned short)13325);
      insock.sin_addr.s_addr=inet_addr("127.0.0.1");
    }
  sendto(fd,(void *)&fd,1,0,(struct sockaddr *)&insock,sizeof(insock));
}
#endif

static void block_until_new_connection()
{

    struct timeval Timeout;
    fd_set readfs;
    int cycles;

    LOG(llevInfo, "Waiting for connections...\n");

    cycles=1;
    do {
	/* Every minutes is a bit often for updates - especially if nothing is going
	 * on.  This slows it down to every 6 minutes.
	 */
	cycles++;
	if (cycles%2 == 0)
	    tick_the_clock();
	if (cycles == 7) {
	    metaserver_update();
	    cycles=1;
	}
	FD_ZERO(&readfs);
	FD_SET((uint32)init_sockets[0].fd, &readfs);
	Timeout.tv_sec=60;
	Timeout.tv_usec=0;
#ifdef WATCHDOG
	watchdog();
#endif
	flush_old_maps();
	}
    while (select(socket_info.max_filedescriptor, &readfs, NULL, NULL, &Timeout)==0);

    reset_sleep(); /* Or the game would go too fast */
}


static void remove_ns_dead_player(player *pl)
{	
	/* remove DM entry */
	if(QUERY_FLAG(pl->ob,FLAG_WIZ))
		gbl_active_DM = NULL;

	if(gbl_active_DM)
		new_draw_info_format(NDI_UNIQUE, 0,gbl_active_DM,"%s leaves the game.", query_name(pl->ob));

	container_unlink(pl,NULL);
	save_player(pl->ob, 0);
			
	if(!QUERY_FLAG(pl->ob,FLAG_REMOVED))
	{
		terminate_all_pets(pl->ob);
		leave_map(pl->ob);
	}
			
	leave(pl,1);
	final_free_player(pl);

}

/* This checks the sockets for input and exceptions, does the right thing.  A 
 * bit of this code is grabbed out of socket.c
 * There are 2 lists we need to look through - init_sockets is a list
 * 
 */
void doeric_server()
{
    int i, pollret;
	uint32 update_below;
    struct sockaddr_in addr;
    int addrlen=sizeof(struct sockaddr);
    player *pl, *next;

#ifdef CS_LOGSTATS
    if ((time(NULL)-cst_lst.time_start)>=CS_LOGTIME)
	write_cs_stats();
#endif

	/* would it not be possible to use FD_CLR too and avoid the 
	 * reseting every time?
	 */
    FD_ZERO(&tmp_read);
    FD_ZERO(&tmp_write);
    FD_ZERO(&tmp_exceptions);

    for(i=0;i<socket_info.allocated_sockets;i++) 
	{
		if (init_sockets[i].status == Ns_Dead)
		{
			free_newsocket(&init_sockets[i]);
			init_sockets[i].status = Ns_Avail;
			socket_info.nconns--;
		} 
		else if (init_sockets[i].status != Ns_Avail) /* ns_add... */
		{
			FD_SET((uint32)init_sockets[i].fd, &tmp_read);
			FD_SET((uint32)init_sockets[i].fd, &tmp_write);
			FD_SET((uint32)init_sockets[i].fd, &tmp_exceptions);
		}
    }

    /* Go through the players.  Let the loop set the next pl value,
     * since we may remove some
     */
    for (pl=first_player; pl!=NULL; )
	{
		if (pl->socket.status == Ns_Dead)
		{
			player *npl=pl->next;

			remove_ns_dead_player(pl);
			pl=npl;
		}
		else 
		{
			FD_SET((uint32)pl->socket.fd, &tmp_read);
			FD_SET((uint32)pl->socket.fd, &tmp_write);
			FD_SET((uint32)pl->socket.fd, &tmp_exceptions);
			pl=pl->next;
		}
    }

    if (socket_info.nconns==1 && first_player==NULL) 
		block_until_new_connection();

    /* Reset timeout each time, since some OS's will change the values on
     * the return from select.
     */
    socket_info.timeout.tv_sec = 0;
    socket_info.timeout.tv_usec = 0;

	/* our one and only select() - after this call, every player socket has signaled us
	 * in the tmp_xxxx objects the signal status: FD_ISSET will check socket for socket
	 * for thats signal and trigger read, write or exception (error on socket).
	 */
    pollret= select(socket_info.max_filedescriptor, &tmp_read, &tmp_write, 
		    &tmp_exceptions, &socket_info.timeout);

    if (pollret==-1) 
	{
		LOG(llevDebug,"doeric_server: error on select\n");
		return;
    }

    /* Following adds a new connection */
    if (pollret && FD_ISSET(init_sockets[0].fd, &tmp_read)) 
	{
		int newsocknum=0;

		LOG(llevInfo,"CONNECT from... ");
		/* If this is the case, all sockets currently in used */
		if (socket_info.allocated_sockets <= socket_info.nconns)
		{
			init_sockets = realloc(init_sockets,sizeof(NewSocket)*(socket_info.nconns+1));
			if (!init_sockets)
				LOG(llevError,"\nERROR: doeric_server(): out of memory\n");
			newsocknum = socket_info.allocated_sockets;
			socket_info.allocated_sockets++;
			init_sockets[newsocknum].status = Ns_Avail;
		}
		else 
		{
			int j;

			for (j=1; j<socket_info.allocated_sockets; j++)
			{
				if (init_sockets[j].status == Ns_Avail) {
					newsocknum=j;
					break;
				}
			}
		}
		init_sockets[newsocknum].fd=accept(init_sockets[0].fd, (struct sockaddr *)&addr, &addrlen);
		i = ntohl(addr.sin_addr.s_addr);
		if (init_sockets[newsocknum].fd!=-1)
		{   
			LOG(llevDebug," ip %d.%d.%d.%d (add to sock #%d)\n",(i>>24)&255, (i>>16)&255, (i>>8)&255, i&255,newsocknum);
			InitConnection(&init_sockets[newsocknum],i);
			socket_info.nconns++;
		}
		else
			LOG(llevDebug,"Error on accept! (S#:%d IP:%d.%d.%d.%d)",newsocknum,(i>>24)&255, (i>>16)&255, (i>>8)&255, i&255);

    }

    /* Check for any exceptions/input on the sockets */
    if (pollret) for(i=1;i<socket_info.allocated_sockets;i++) 
	{
		if (init_sockets[i].status == Ns_Avail) 
			continue;
		if (FD_ISSET(init_sockets[i].fd,&tmp_exceptions))
		{
			free_newsocket(&init_sockets[i]);
			init_sockets[i].status = Ns_Avail;
			socket_info.nconns--;
			continue;
		}
		if (FD_ISSET(init_sockets[i].fd, &tmp_read))
		    HandleClient(&init_sockets[i], NULL);

		if (FD_ISSET(init_sockets[i].fd, &tmp_write))
		    init_sockets[i].can_write=1;
    }

    /* This does roughly the same thing, but for the players now */
    for (pl=first_player; pl!=NULL; pl=next) 
	{
		next=pl->next;
		if (pl->socket.status==Ns_Dead)
			continue;

		/* kill players if we have problems */
		if (FD_ISSET(pl->socket.fd,&tmp_exceptions))
			remove_ns_dead_player(pl);
		else 
		{
			/* all ok - read from socket? */
			if (FD_ISSET(pl->socket.fd, &tmp_read))
				HandleClient(&pl->socket, pl);

		    if (pl->socket.status==Ns_Dead) /* perhaps something was bad in HandleClient() */
				remove_ns_dead_player(pl);	/* or player has left game */
			else
			{

				/* Update the players stats once per tick.  More efficient than
				* sending them whenever they change, and probably just as useful
				* (why is update the stats per tick more efficent as we set a update sflag??? MT)
				*/
				esrv_update_stats(pl);
				if(pl->update_skills)
					esrv_update_skills(pl);
				pl->update_skills=0;
				draw_client_map(pl->ob);

				if( pl->ob->map && (update_below=GET_MAP_UPDATE_COUNTER(pl->ob->map,pl->ob->x,pl->ob->y)) >= 
												pl->socket.update_tile) 
				{
					esrv_draw_look(pl->ob);
					pl->socket.update_tile = update_below+1;
				}

				/* and do a quick write ... */
				if (FD_ISSET(pl->socket.fd,&tmp_write))
				{
					if (!pl->socket.can_write)
					{
						pl->socket.can_write=1;
						write_socket_buffer(&pl->socket);
					}
				}
				else 
					pl->socket.can_write=0;
			}
		}
    } /* for() end */
}

void doeric_server_write(void)
{
    player *pl, *next;

   /* This does roughly the same thing, but for the players now */
    for (pl=first_player; pl!=NULL; pl=next) 
	{
		next=pl->next;

		/* we don't care about problems here... let remove player at start of next loop! */
		if (pl->socket.status==Ns_Dead || FD_ISSET(pl->socket.fd,&tmp_exceptions))
			continue;

		/* and *now* write back to player */
		if (FD_ISSET(pl->socket.fd,&tmp_write))
		{
			/* i see no really sense here... can_write is REALLY
			* only set if socket() marks the write channel as free.
			* and can_write is in loop flow only set here.
			* i think this was added as a "there is something in a buffer"
			* and then changed in context.
			*/
			if (!pl->socket.can_write)
			{
					pl->socket.can_write=1;
					write_socket_buffer(&pl->socket);
			}
			else 
				pl->socket.can_write=0;
		}
    } /* for() end */
}
