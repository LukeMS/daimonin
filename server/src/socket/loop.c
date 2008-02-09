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

#ifdef NO_ERRNO_H
extern int  errno;
#else
#   include <errno.h>
#endif

char            _idle_warn_text[]    = "4 8 minutes idle warning!\nServer will disconnect you in 2 minutes.";
char            _idle_warn_text2[]    = "3 Max idle time reached! Server is closing connection.";
static fd_set   tmp_read, tmp_exceptions, tmp_write;

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
typedef void (*func_uint8_int_ns) (char *, int, NewSocket *);
typedef void (*func_uint8_int_pl)(char *, int, player *);

typedef struct NsCmdMapping_struct
{
    char               *cmdname;
    func_uint8_int_ns   cmdproc;
}NsCmdMapping;

typedef struct PlCmdMapping_struct
{
    char               *cmdname;
    func_uint8_int_pl   cmdproc;
}PlCmdMapping;


/*
 * CmdMapping is the dispatch table for the server, used in HandleClient,
 * which gets called when the client has input.  All commands called here
 * use the same parameter form (char* data, int len, int clientnum.
 * We do implicit casts, because the data that is being passed is
 * unsigned (pretty much needs to be for binary data), however, most
 * of these treat it only as strings, so it makes things easier
 * to cast it here instead of a bunch of times in the function itself.
 */
PlCmdMapping        plcommands[]    =
{
    { "ex",             ExamineCmd },
    { "ap",             ApplyCmd },
    { "mv",             MoveCmd },
    { "reply",          ReplyCmd},
    { "cm",             PlayerCmd},
    { "ncom",           NewPlayerCmd},
    { "lt",             LookAt},
    { "lock",           LockItem},
    { "mark",           MarkItem},
    { "tx",             command_talk_ex},
    { "/fire",          command_fire},
    { "nc",             command_new_char},
    { NULL, NULL}   /* terminator */
};

NsCmdMapping        nscommands[]    =
{
    { "addme",          AddMeCmd },
    { "setsound",       SetSound},
    { "setup",          SetUp},
    { "version",        VersionCmd },
    { "rf",             RequestFileCmd },
#ifdef SERVER_SEND_FACES
	{ "setfacemode",    SetFaceMode},
	{ "askface",        SendFaceCmd},   /* Added: phil */
    { "fr",             command_face_request},
#endif
    { NULL, NULL}   /* terminator */
};


//static inline int socket_prepare_commands(NewSocket *ns)
static int socket_prepare_commands(NewSocket *ns) // use this for debugging
{
    int toread, flag;
    ReadList *rb = &ns->readbuf;
    command_struct *cmdptr = NULL;

    while(rb->len >= 2)/* there is something in our in buffer amd its at last a valid length value */
    {
        flag = FALSE;
        if(rb->pos+1 >= MAXSOCKBUF_IN) /* our command length is splitted! */
            toread = (rb->buf[MAXSOCKBUF_IN-1] << 8) + rb->buf[0];
        else
            toread = (rb->buf[rb->pos] << 8) + rb->buf[rb->pos+1];

        /* don't have the time for a 100% binary client->server interface.
         * This flag will tell us its a binary command
         */
        if(toread&0x8000)
        {
            toread&=~0x8000;
            flag = TRUE;
        }

        /* lets check our "toread" value is senseful! */
        if(toread <= 0 || toread > MAXSOCKBUF_IN)
        {
            /* bogus command! kill client NOW */
            ns->status = Ns_Dead;
            return TRUE;
        }

        if (toread > rb->len-2) /* remember - perhaps the command is incomplete! */
            return FALSE;

        /* we have a valid, full command - grap & copy it from the read buffer to command struct */
        rb->pos +=2;
        if(rb->pos >= MAXSOCKBUF_IN)
            rb->pos -= MAXSOCKBUF_IN;

        /* lets get a command chunk which is big enough - this is still for testing,
         * i am sure we don't need so much different buffers.
         */
        if(toread < 16)
        {
            cmdptr = get_poolchunk(pool_cmd_buf16);
            cmdptr->pool = pool_cmd_buf16;
        }
        else if(toread < 32)
        {
            cmdptr = get_poolchunk(pool_cmd_buf32);
            cmdptr->pool = pool_cmd_buf32;
        }
        else if(toread < 64)
        {
            cmdptr = get_poolchunk(pool_cmd_buf64);
            cmdptr->pool = pool_cmd_buf64;
        }
        else if(toread < 128)
        {
            cmdptr = get_poolchunk(pool_cmd_buf128);
            cmdptr->pool = pool_cmd_buf128;
        }
        else if(toread < 256)
        {
            cmdptr = get_poolchunk(pool_cmd_buf256);
            cmdptr->pool = pool_cmd_buf256;
        }
        else if(toread < 1024)
        {
            cmdptr = get_poolchunk(pool_cmd_buf1024);
            cmdptr->pool = pool_cmd_buf1024;
        }
        else if(toread < 4096)
        {
            cmdptr = get_poolchunk(pool_cmd_buf4096);
            cmdptr->pool = pool_cmd_buf4096;
        }

        cmdptr->len = toread;
		/*LOG(-1,"we got something in the buffer...:: %d\n",cmdptr->len);*/

        if(rb->pos+toread <= MAXSOCKBUF_IN)
        {
            memcpy(cmdptr->buf, rb->buf+rb->pos, toread);
            rb->pos += toread;
        }
        else /* the command is splitted? */
        {
            int tmp_read, read_part;

            read_part = (rb->pos + toread) - MAXSOCKBUF_IN;
            tmp_read = toread - read_part;
            memcpy(cmdptr->buf, rb->buf+rb->pos, tmp_read);
            memcpy(cmdptr->buf+tmp_read, rb->buf, read_part);
            rb->pos = read_part;
        }

        /*LOG(-1,"READCMD(%d)(%d)(%d)(%d): >%s<\n", cmdptr->len, flag, toread, cmdptr->buf[0],cmdptr->buf);*/
        cmdptr->buf[toread] = 0; /* it ensures we are null terminated. nice */

        if(rb->pos == MAXSOCKBUF_IN)
            rb->pos = 0;
        rb->len -= (toread+2);

        /* lets pre-process "instant" commands.
         * in theorie we can grap them direct out of the readbuffer if
         * they are not splitted... but this is alot cleaner code.
         */
        if(flag)
        {
            /* TEST: we test here our binary commands. */
            if(cmdptr->buf[0] == CLIENT_CMD_STOP)
            {
                command_struct *cmdold=NULL, *cmdtmp = ns->cmd_start;
                int i;

                while (cmdtmp)
                {
                    /* lets check its a non-system command.
                     * With binary commands, this will ALOT easier.
                     */
                    for (i = 0; nscommands[i].cmdname != NULL; i++)
                    {
                        if ((int) strlen(nscommands[i].cmdname) <= ns->cmd_start->len &&
                            !strncmp(ns->cmd_start->buf, nscommands[i].cmdname, strlen(nscommands[i].cmdname)))
                        {
                            i = -1;
                            break;
                        }
                    }

                    if(i == -1) /* its a system command */
                    {
                        cmdold = cmdtmp;
                        cmdtmp = cmdtmp->next;

                    }
                    else /* a command we must skip! */
                    {
                        command_struct *cmdnext=cmdtmp->next;

                        if(cmdold)
                            cmdold->next = cmdnext;

                        if(ns->cmd_start == cmdtmp)
                            ns->cmd_start = cmdnext;

                        if(ns->cmd_end == cmdtmp)
                            ns->cmd_end = cmdnext;

                        return_poolchunk(cmdtmp, cmdtmp->pool);
                        cmdtmp = cmdnext;
                    }

                };
            }
            else /* something illegal from the client. Kill it! */
            {
                LOG(llevInfo,"HACKBUG: Wrong binary client command: %d from %s\n",
                    cmdptr->buf[0], STRING_SAFE(ns->ip_host));
                return_poolchunk(cmdptr, cmdptr->pool);
                ns->status = Ns_Dead;
                return TRUE;
            }

            return_poolchunk(cmdptr, cmdptr->pool);
            continue;
        }

		command_buffer_enqueue(ns, cmdptr);
    };

    return FALSE;
}

/* We have now a buffer we read in from the socket.
 * In that buffer are 0-x commands (and/or perhaps *one* last, incomplete one).
 * We read now in fifo way the commands from that buffer.
 * If its a system command, we just fire it directly.
 * If its a player command, we put it in the command queue.
 */
int fill_command_buffer(NewSocket *ns, int len)
{
    char * data;
    int i, data_len;
    int found_command = 1;

    if(socket_prepare_commands(ns))
        return FALSE;

    if(ns->status >= Ns_Login)
        return TRUE;

    while (ns->cmd_start && found_command)
    {
		/* check its a system command.
			* If so, process it. If not, store it.
            */
            found_command = 0;
            for (i = 0; nscommands[i].cmdname != NULL; i++)
            {
                int cmdlen = (int)strlen(nscommands[i].cmdname); /* TODO: this can be precomputed to save a few us */
                if (cmdlen <= ns->cmd_start->len &&
                            !strncmp(ns->cmd_start->buf, nscommands[i].cmdname, cmdlen))
                {
                    /* pre process the command */

                    data = strchr((char *) ns->cmd_start->buf, ' ');
                    if (data)
                    {
                        *data = '\0';
                        data++;
                        data_len = ns->cmd_start->len - (int)(data - ns->cmd_start->buf);
                    }
                    else
                        data_len = 0;
                    nscommands[i].cmdproc(data, data_len, ns); /* and process cmd */

                    /* remove command & buffer */
                    command_buffer_clear(ns);

                    if (ns->addme) /* we have successful added this connect! */
                    {
                        /* NOTE: this socket is now copied to a player struct - disable it now */
                        ns->addme = 0;
                        ns->readbuf.buf = NULL;
                        ns->login_count = ROUND_TAG + pticks_socket_idle; /* reset idle counter */
                        socket_info.nconns--;
                        ns->status = Ns_Avail;
                        return TRUE;
                    }

                    if (ns->status == Ns_Dead)
                        return FALSE;

                    found_command = 1;
                    break;
                }
            }
    };

    if(! found_command)
    {
        LOG(llevDebug, "HACKBUG: Bad command from client (%s)\n", STRING_SAFE(ns->cmd_start->buf));
        ns->status = Ns_Dead;
        return FALSE;
    }

    return TRUE;
}

/* HandleClient is actually not named really well - we only get here once
 * there is input, so we don't do exception or other stuff here.
 * sock is the output socket information.  pl is the player associated
 * with this socket, null if no player (one of the init_sockets for just
 * starting a connection)
 */
void HandleClient(NewSocket *ns, player *pl)
{
    int len = 0, i, cmd_count = 0;
    char * data;

    /* Loop through this - maybe we have several complete packets here. */
    while (ns->cmd_start)
    {
		/*LOG(-1,"HandleClient: cmdptr:%x\n", ns->cmd_start);*/
        /* If it is a player, and they don't have any speed left, we
         * return, and will read in the data when they do have time.
         */
        if ( ns->status < Ns_Avail || ns->status >= Ns_Zombie ||
                        (pl && pl->state == ST_PLAYING && (!pl->ob || pl->ob->speed_left < 0.0f)))
            return;

		/*
        if (ns->outputxbuffer.len >= (int) (MAXXSOCKBUF * 0.85))
        {
            if (!ns->write_overflow)
            {
                ns->write_overflow = 1;
                LOG(llevDebug, "OVERFLOW: socket write overflow protection on!  (%s) (%d)\n", STRING_SAFE(ns->ip_host),
                    ns->outputxbuffer.len);
            }
            return;
        }
        else if (ns->write_overflow && (ns->outputxbuffer.len <= (int) (MAXXSOCKBUF * 0.35)))
        {
            ns->write_overflow = 0;
            LOG(llevDebug, "OVERFLOW: socket write overflow protection off! host (%s) (%d)\n", STRING_SAFE(ns->ip_host),
                ns->outputxbuffer.len);
        }
		*/

        /* reset idle counter */
        if (pl && pl->state == ST_PLAYING)
        {
            ns->login_count = ROUND_TAG + pticks_player_idle1;
            ns->idle_flag = 0;
        }

        /* preprocess command */
        data = strchr((char *) ns->cmd_start->buf, ' ');
        if (data)
        {
            *data = '\0';
            data++;
            len = ns->cmd_start->len - (int)(data - ns->cmd_start->buf);
        }
        else
            len = 0;


        for (i = 0; nscommands[i].cmdname != NULL; i++)
        {
            if (strcmp(ns->cmd_start->buf, nscommands[i].cmdname) == 0)
            {
                nscommands[i].cmdproc(data, len, ns);
                goto next_command;
            }
        }
        /* Only valid players can use these commands */
        if (pl)
        {
            for (i = 0; plcommands[i].cmdname != NULL; i++)
            {
                if (strcmp(ns->cmd_start->buf, plcommands[i].cmdname) == 0)
                {
                    plcommands[i].cmdproc(data, len, pl);
                    goto next_command;
                }
            }
        }

        /* If we get here, we didn't find a valid command.  Logging
         * this might be questionable, because a broken client/malicious
         * user could certainly send a whole bunch of invalid commands.
         */
        LOG(llevDebug, "HACKBUG: Bad command from client (%s) (%s)\n", STRING_SAFE(ns->cmd_start->buf), STRING_SAFE((char *) data));
        ns->status = Ns_Dead;

        next_command:
        /* remove command & buffer */
        command_buffer_clear(ns);

        if (cmd_count++ <= 8 && ns->status != Ns_Dead)
        {
            /* LOG(llevDebug,"MultiCmd: #%d /%s)\n", cmd_count, (char*)ns->inbuf.buf+2); */
            continue;
        }
        return;
    }
}

void remove_ns_dead_player(player *pl)
{

	LOG(llevDebug, "remove_ns_dead_player(%s): state:%d gmaster:%d g_status:%d\n", STRING_OBJ_NAME(pl->ob),pl->state,
		pl->gmaster_mode, pl->group_status);
    if (pl->state != ST_DEAD)
    {
        /* remove the player from global gmaster lists */
        if(pl->gmaster_mode != GMASTER_MODE_NO)
            remove_gmaster_list(pl);

        /* remove player from party */
        if(pl->group_status & GROUP_STATUS_GROUP)
            party_remove_member(pl, TRUE);

        if(gmaster_list_DM || gmaster_list_GM)
        {
            objectlink *ol;
            char buf_dm[128];

            sprintf(buf_dm,"%s leaves the game (%d still playing).", query_name(pl->ob), player_active - 1);

            for(ol = gmaster_list_DM;ol;ol=ol->next)
                new_draw_info(NDI_UNIQUE, 0,ol->objlink.ob, buf_dm);

            for(ol = gmaster_list_GM;ol;ol=ol->next)
                new_draw_info(NDI_UNIQUE, 0,ol->objlink.ob, buf_dm);
        }

        container_unlink(pl, NULL);
        save_player(pl->ob, 0);

        if (!QUERY_FLAG(pl->ob, FLAG_REMOVED))
        {
            terminate_all_pets(pl->ob);
            leave_map(pl->ob);
        }

        LOG(llevDebug, "remove_ns_dead_player(): %s leaving\n", STRING_OBJ_NAME(pl->ob));
        leave(pl, 1);
    }

    free_player(pl); /* we *,must* do this here and not in the memory pool - it needs to be a syncron action */
}

/* lets check 2 things:
 * a.) mass connection from one IP
 * b.) or is the ip (range) banned
 * return TRUE means banned, FALSE is ok.
 */
static int check_ip_ban(NewSocket *sock, char *ip)
{
    int         count, i;
    player      *next_tmp, *pl, *ptmp = NULL;

    /* lets first check sensless connected sockets
     * from same IP.
     * Mark all from same IP as dead.
     * Note: We accept *one* other login
     * - perhaps someone use here a shared IP.
     * We search from last socket to first.
     * So we skip the oldest login automatically.
     */

	/* check we have limited server access */
	if(!settings.login_allow)
	{
		if(strcmp(settings.login_ip, ip))
		{
			LOG(llevDebug, "login_allow-failed: IP don't match login_ip:%s\n", settings.login_ip);
			sock->status = Ns_Dead;
			return FALSE;
		}
		else
			LOG(llevDebug, "login_allow-OK: IP match login_ip:%s\n", settings.login_ip);
	}

    count = 0;
    for (i = socket_info.allocated_sockets - 1; i > 0; i--)
    {
        if (init_sockets[i].status != Ns_Avail && sock != &init_sockets[i] && !strcmp(init_sockets[i].ip_host, ip))
        {
            if (++count > 1 || init_sockets[i].status <= Ns_Zombie)
            {
                LOG(llevDebug, "check_ip_ban(): socket flood. mark Ns_Dead: %d (IP %s)\n",
                                i, init_sockets[i].ip_host );
                init_sockets[i].status = Ns_Dead;
                free_newsocket(&init_sockets[i]);
                init_sockets[i].status = Ns_Avail;
                socket_info.nconns--;
            }
        }
    }

    /* we first check our ban list. Perhaps this IP is on it */
    if(check_banned(sock, NULL, ip))
        return FALSE; /* *IF* banned, we have turned the socket to a Ns_Zombie... */

    /* now check the players we have */
    count = 0;
    for (pl = first_player; pl; pl = next_tmp)
    {
        next_tmp = pl->next;
    if(!strcmp(pl->socket.ip_host, ip)) /* we have someone playing from same IP? */
    {
        if (pl->socket.status != Ns_Playing)
        {
        pl->socket.status = Ns_Dead;
            remove_ns_dead_player(pl);
        }
        else /* allow 2 logged in *real* playing accounts online from same IP */
        {
            count++;
        if (!ptmp)
            ptmp = pl;
        else
        {
            /* lets compare the idle time.
             * if needed we will kick the login with the highest idle time
                     */
            if (ptmp->socket.login_count <= pl->socket.login_count)
                ptmp = pl;

                /* now the tricky part: if we have to many
                     * connects from that IP, we KICK the login
                     * with the highest idle time
             */
            if (count > 1)
            {
                LOG(llevDebug, "check_ip_ban(): connection flood: mark player %s Ns_Dead (IP %s)\n",
                    query_name(pl->ob), ptmp->socket.ip_host);
            ptmp->socket.status = Ns_Dead;
            }
        }
            }
        }
    }
    return FALSE;
}

/* This checks the sockets for input and exceptions, does the right thing.  A
 * bit of this code is grabbed out of socket.c
 * There are 2 lists we need to look through - init_sockets is a list
 */
void doeric_server(int update, struct timeval *timeout)
{
    int     i, pollret, update_client=update&SOCKET_UPDATE_CLIENT, update_player=update&SOCKET_UPDATE_PLAYER;
    uint32  update_below;

#if WIN32 || !HAVE_GETADDRINFO
    struct sockaddr_in      addr;
#else
    struct sockaddr_storage addr;
#endif
    unsigned int        addrlen = sizeof(addr);
    player                     *pl, *next;

    /* would it not be possible to use FD_CLR too and avoid the
     * reseting every time?
     */
    FD_ZERO(&tmp_read);
    FD_ZERO(&tmp_write);
    FD_ZERO(&tmp_exceptions);

    for (i = 0; i < socket_info.allocated_sockets; i++)
    {
        if (init_sockets[i].status == Ns_Dead)
        {
            free_newsocket(&init_sockets[i]);
            init_sockets[i].status = Ns_Avail;
            socket_info.nconns--;
        }
        else if (init_sockets[i].status != Ns_Avail) /* ns_add... */
        {
            if (init_sockets[i].status > Ns_Wait) /* exclude socket #0 which listens for new connects */
            {
                /* kill this after 3 minutes idle... */
                if (init_sockets[i].login_count < ROUND_TAG)
                {
                    free_newsocket(&init_sockets[i]);
                    init_sockets[i].status = Ns_Avail;
                    socket_info.nconns--;
                    continue;
                }
            }
            FD_SET((uint32) init_sockets[i].fd, &tmp_read);
            FD_SET((uint32) init_sockets[i].fd, &tmp_exceptions);
            /* Only check for writing if we actually want to write */
            if (init_sockets[i].sockbuf_start || (init_sockets[i].sockbuf && init_sockets[i].sockbuf->len))
                FD_SET((uint32) init_sockets[i].fd, &tmp_write);
        }
    }

    /* Go through the players.  Let the loop set the next pl value,
     * since we may remove some
     */
    for (pl = first_player; pl != NULL;)
    {
        if (pl->socket.status == Ns_Dead)
        {
            player *npl = pl->next;

            remove_ns_dead_player(pl);
            pl = npl;
        }
        else
        {
            if(pl->socket.status != Ns_Zombie)
            {
                if (!pl->socket.idle_flag && pl->socket.login_count < ROUND_TAG && !QUERY_FLAG(pl->ob, FLAG_WIZ))
                {
                    pl->socket.login_count = ROUND_TAG + pticks_player_idle2;
                    pl->socket.idle_flag = 1;
                    Write_String_To_Socket(&pl->socket, BINARY_CMD_DRAWINFO, _idle_warn_text, strlen(_idle_warn_text));
                }
                else if (pl->socket.login_count < ROUND_TAG && !QUERY_FLAG(pl->ob, FLAG_WIZ))
                {
                    Write_String_To_Socket(&pl->socket, BINARY_CMD_DRAWINFO, _idle_warn_text2, strlen(_idle_warn_text2));
                    pl->socket.login_count = ROUND_TAG+(uint32)(2.0f * pticks_second);
                    pl->socket.status = Ns_Zombie; /* we hold the socket open for a *bit* */
                    pl->socket.idle_flag = 1;
                    pl = pl->next;
                    continue;
                }
            }
            else
            {
                if (pl->socket.login_count < ROUND_TAG) /* time to kill! */
                {
                    player *npl = pl->next;
                    pl->socket.status = Ns_Dead;
                    remove_ns_dead_player(pl);  /* or player has left game */
                    pl = npl;
                    continue;
                }
            }

            FD_SET((uint32) pl->socket.fd, &tmp_read);
            FD_SET((uint32) pl->socket.fd, &tmp_exceptions);
            /* Only check for writing if we actually want to write */
            if (pl->socket.sockbuf_start || (pl->socket.sockbuf && pl->socket.sockbuf->len))
                FD_SET((uint32) pl->socket.fd, &tmp_write);
            pl = pl->next;
        }
    }

    /* our one and only select() - after this call, every player socket has signaled us
     * in the tmp_xxxx objects the signal status: FD_ISSET will check socket for socket
     * for thats signal and trigger read, write or exception (error on socket).
     */
    pollret = select(socket_info.max_filedescriptor, &tmp_read, &tmp_write, &tmp_exceptions, timeout);

    if (pollret == -1)
    {
        LOG(llevBug, "BUG: doeric_server(): error on select\n");
        return;
    }

    /* Following adds a new connection */
    if (pollret && FD_ISSET(init_sockets[0].fd, &tmp_read))
    {
    NewSocket *newsock;

        LOG(llevInfo, "CONNECT from... ");
    newsock = socket_get_available();

        newsock->fd = accept(init_sockets[0].fd, (struct sockaddr *) &addr, &addrlen);
        if (newsock->fd != -1)
        {
#if WIN32 || !HAVE_GETADDRINFO
        char *tmp_ip = inet_ntoa(addr.sin_addr);
#else
        char tmp_ip[NI_MAXHOST];

        getnameinfo((struct sockaddr *) &addr, addrlen, tmp_ip, sizeof(tmp_ip), NULL, 0, NI_NUMERICHOST);
#endif
            LOG(llevDebug, " ip %s (socket %d) (%x)\n", tmp_ip, newsock->fd, newsock);
            InitConnection(newsock, tmp_ip);
            check_ip_ban(newsock, tmp_ip);
            if(newsock->status <= Ns_Zombie) /* set from ban check */
            {
                newsock->status = Ns_Add;
				SOCKBUF_ADD_TO_SOCKET(newsock, global_sockbuf_version);
            }
        }
        else
            LOG(llevDebug, "Error on accept!\n");
    }

    /* Check for any exceptions/input on the sockets */
    if (pollret)
        for (i = 1; i < socket_info.allocated_sockets; i++)
        {
            if (init_sockets[i].status == Ns_Avail)
            {
                continue;
            }

            if (FD_ISSET(init_sockets[i].fd, &tmp_exceptions))
            {
                free_newsocket(&init_sockets[i]);
                init_sockets[i].status = Ns_Avail;
                socket_info.nconns--;
                continue;
            }

            if (FD_ISSET(init_sockets[i].fd, &tmp_read))
            {
                if (read_socket_buffer(&init_sockets[i]) < 0)
                {
                    LOG(llevDebug, "Drop ConnectionA: host %s\n", init_sockets[i].ip_host);
                    init_sockets[i].status = Ns_Dead;
                }
                else
                    fill_command_buffer(&init_sockets[i], MAXSOCKBUF_IN - 1);
            }

            if (init_sockets[i].status == Ns_Dead)
            {
                free_newsocket(&init_sockets[i]);
                init_sockets[i].status = Ns_Avail;
                socket_info.nconns--;
                continue;
            }

            if (FD_ISSET(init_sockets[i].fd, &tmp_write))
                write_socket_buffer(&init_sockets[i]);

            if (init_sockets[i].status == Ns_Dead)
            {
                free_newsocket(&init_sockets[i]);
                init_sockets[i].status = Ns_Avail;
                socket_info.nconns--;
            }
        }

    /* This does roughly the same thing, but for the players now */
    for (pl = first_player; pl != NULL; pl = next)
    {
        next = pl->next;

        /* kill players if we have problems */
        if (pl->socket.status == Ns_Dead || FD_ISSET(pl->socket.fd, &tmp_exceptions))
        {
            remove_ns_dead_player(pl);
        }
        else
        {
            /* this will be triggered when its POSSIBLE to read
                     * from the socket - this tells us not there is really
                     * something!
                     */
            if (FD_ISSET(pl->socket.fd, &tmp_read))
            {
                if (read_socket_buffer(&pl->socket) < 0)
                {
                    LOG(llevDebug, "Drop ConnectionB: %s (%s)\n", query_name(pl->ob), pl->socket.ip_host);
                    pl->socket.status = Ns_Dead;
                }
                else
                    fill_command_buffer(&pl->socket, MAXSOCKBUF_IN - 1);
            }

            if (pl->socket.status == Ns_Dead) /* perhaps something was bad in HandleClient() */
            {
                remove_ns_dead_player(pl);  /* or player has left game */
            }
            else
            {
                /* Update the players stats once per tick.  More efficient than
                 * sending them whenever they change, and probably just as useful
                 * (why is update the stats per tick more efficent as we set a update sflag??? MT)
                 */
                if (update_client)
                {
                    if (update_player && pl->state == ST_PLAYING)
                    {
                        esrv_update_stats(pl);
                        if (pl->update_skills)
                        {
                            esrv_update_skills(pl);
                            pl->update_skills = 0;
                        }
                        draw_client_map(pl->ob);

                        if ( pl->ob->map && (update_below = GET_MAP_UPDATE_COUNTER(pl->ob->map, pl->ob->x, pl->ob->y))
                             != pl->socket.update_tile)
                        {
                            esrv_draw_look(pl->ob);
                            pl->socket.update_tile = update_below;
                        }
                    }

                    if (FD_ISSET(pl->socket.fd, &tmp_write))
                        write_socket_buffer(&pl->socket);
                }
            }
        }
    } /* for() end */
}

void doeric_server_write(void)
{
    player *pl, *next;
    uint32  update_below;
    fd_set writeset;
    struct timeval timeout = {0, 0};

    /* This does roughly the same thing, but for the players now */
    for (pl = first_player; pl != NULL; pl = next)
    {
        next = pl->next;

        /* we don't care about problems here... let remove player at start of next loop! */
        if (pl->socket.status == Ns_Dead || FD_ISSET(pl->socket.fd, &tmp_exceptions))
        {
            remove_ns_dead_player(pl);
            continue;
        }
        if (pl->state == ST_PLAYING)
        {
            esrv_update_stats(pl);
            if (pl->update_skills)
            {
                esrv_update_skills(pl);
                pl->update_skills = 0;
            }
            draw_client_map(pl->ob);

            if (pl->ob->map
                && (update_below = GET_MAP_UPDATE_COUNTER(pl->ob->map, pl->ob->x, pl->ob->y)) != pl->socket.update_tile)
            {
                esrv_draw_look(pl->ob);
                pl->socket.update_tile = update_below;
            }
        }

        /* Check that a write won't block */
        FD_ZERO(&writeset);
        FD_SET((uint32)pl->socket.fd, &writeset);

        /* tv_usec=0 should be work too */
        timeout.tv_sec = 0;
        timeout.tv_usec = 0;
        select(pl->socket.fd + 1, NULL, &writeset, NULL, &timeout);

        /* and *now* write back to player */
        if (FD_ISSET(pl->socket.fd, &writeset))
            write_socket_buffer(&pl->socket);
    } /* for() end */
}
