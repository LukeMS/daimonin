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

    The author can be reached via e-mail to info@daimonin.org
*/

#include <global.h>

#ifndef WIN32 /* ---win32 exclude unix headers */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#endif /* end win32 */

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#include <newserver.h>

#ifdef NO_ERRNO_H
extern int  errno;
#else
#   include <errno.h>
#endif

static fd_set   tmp_read, tmp_exceptions, tmp_write;

/* NOTE: i used for the parms of this static inline function different names,
 * even they point to the same variables. I did that because i experienced
 * in debugging mode problems. Seems VC had problems to see where which
 * variable name fits.
 */

/* copy a command data tail out of our round robin buffer */
static inline void copy_cmd_data(ReadList *b, char *rbuf, int len)
//static void copy_client_command(ReadList *b, char *rbuf, int len)
{
    /* lets grap the data tail from the round robin */
    if(b->pos + len <= MAXSOCKBUF_IN)
    {
        memcpy(rbuf, b->buf + b->pos, len);
        b->pos += len;
    }
    else /* the command is splitted? */
    {
        int tmp_read, read_part;

        read_part = (b->pos + len) - MAXSOCKBUF_IN;
        tmp_read = len - read_part;
        memcpy(rbuf, b->buf + b->pos, tmp_read);
        memcpy(rbuf + tmp_read, b->buf, read_part);
        b->pos = read_part;
    }

    rbuf[len] = 0; /* it ensures we are null terminated. nice */

    /* finally adjust the removed data tail length from our round robin */
    if(b->pos == MAXSOCKBUF_IN)
        b->pos = 0;
    b->len -= len;

}

/* for pre processing we need a valid data tail
 * if its broken in 2 parts in the round robin buffer, we must
 * first copy it in a temporary flat buffer.
 * NOTE: this is not thread safe atm because the static data_buffer,
 * to make it thread safe move it as non statical to pre_process_command()
 * NOTE 2: we can't ensure the data-tail is zero marked at the end for pre_process,
 * our string functions have to take care about it!
 */
static char *pre_copy_cmd_data(ReadList *readb)
{
    static char data_buffer[MAX_DATA_TAIL_LENGTH+1];

    /* can we grap the tail right out of the round robin? */
    if(readb->pos + readb->toread <= MAXSOCKBUF_IN) /* yes, its one block */
    {
        readb->pos += readb->toread; /* for our socket functions we have now copied and removed the tail */
        if(readb->pos == MAXSOCKBUF_IN)
            readb->pos = 0;
        readb->len -= readb->toread;
        return readb->buf + (readb->pos - readb->toread);
    }

    /* its in 2 parts. copy_client_command() will copy, adjust readb->pos and zero mark the buffer too */
    copy_cmd_data(readb, data_buffer, readb->len);
    return data_buffer;
}

/* We have a handfull commands we only allow in a right order
* and/or only at ONE time. Lets check we have that and our
* client does not try to cheat us
*/
/* This seems an odd function. I'm not sure I see the point of this
 * pre-processing *and* then duplicating the security checks in the
 * cs_cmd_*() functions.
 * -- Smacky 20110519 */
//static inline int pre_process_command(NewSocket *ns)
static int pre_process_command(NewSocket *nsock) // use this for debugging
{
    ReadList *buf = &nsock->readbuf;
    int cmd = buf->cmd;

    /* Login: the simplest reason to pre-process a command */
    if (cmd <= CLIENT_CMD_LOGIN)
    {
        if(nsock->status != Ns_Login) /* cmds only allowed at this stage! */
        {
            LOG(llevDebug,"BOGUS CLIENT DATA: Invalid cmd: %d from socket %d at status Ns_Login\n", cmd, nsock->fd);
            nsock->status = Ns_Dead;
        }
        else
        {
            /* Here we have all account login procedure functions
             * They will take care about the right order of calling
             * itself, so we just fire them up here
             */
            if (!nsock->setup)
            {
                if(cmd == CLIENT_CMD_PING)
                    cs_cmd_ping(pre_copy_cmd_data(buf), buf->toread, nsock);
                else if(cmd == CLIENT_CMD_SETUP)
                    cs_cmd_setup(pre_copy_cmd_data(buf), buf->toread, nsock);
            }
            else
            {
                if(cmd == CLIENT_CMD_REQUESTFILE)
                    cs_cmd_file(pre_copy_cmd_data(buf), buf->toread, nsock);
                else if(cmd == CLIENT_CMD_CHECKNAME)
                    cs_cmd_checkname(pre_copy_cmd_data(buf), buf->toread, nsock);
                else if(cmd == CLIENT_CMD_LOGIN) /* will set status to Ns_Account */
                    cs_cmd_login(pre_copy_cmd_data(buf), buf->toread, nsock);
            }
        }

        return 1;
    }
    else if(cmd <= CLIENT_CMD_ADDME)
    {
        if(nsock->status != Ns_Account) /* cmds only allowed at this stage! */
        {
            LOG(llevDebug,"BOGUS CLIENT DATA: Invalid cmd %d from socket %d at status Ns_Account\n", cmd, nsock->fd);
            nsock->status = Ns_Dead;
        }
        else
        {
            if(cmd == CLIENT_CMD_NEWCHAR)
                cs_cmd_newchar(pre_copy_cmd_data(buf), buf->toread, nsock);
            else if(cmd == CLIENT_CMD_DELCHAR)
                cs_cmd_delchar(pre_copy_cmd_data(buf), buf->toread, nsock);
            else if(cmd == CLIENT_CMD_ADDME)
                cs_cmd_addme(pre_copy_cmd_data(buf), buf->toread, nsock);
        }
        return 1;
    }
    else if(cmd <= CLIENT_CMD_MOVE) /* move changes will invoke command stack manipulations */
    {
        /* allow as pre processed cmd only when we don't play - in the other case enqueue */
        if(cmd == CLIENT_CMD_FACE)
        {
            if(nsock->status >= Ns_Login && nsock->status < Ns_Playing)
            {
                cs_cmd_face(pre_copy_cmd_data(buf), buf->toread, nsock);
                return 1;
            }
        }

        /* TODO: enable command queue post flush for state move commands */

        /*
        //      command_move(pre_copy_cmd_data(buf), buf->toread, nsock);
        //    return 1;
        if(flag)
        {
        // TEST: we test here our binary commands.
        if(cmdptr->buf[0] == CLIENT_CMD_STOP)
        {
        command_struct *cmdold=NULL, *cmdtmp = ns->cmd_start;
        int i;

        while (cmdtmp)
        {
        // lets check its a non-system command.
        // With binary commands, this will ALOT easier.

        for (i = 0; nscommands[i].cmdname != NULL; i++)
        {
        if ((int) strlen(nscommands[i].cmdname) <= ns->cmd_start->len &&
        !strncmp(ns->cmd_start->buf, nscommands[i].cmdname, strlen(nscommands[i].cmdname)))
        {
        i = -1;
        break;
        }
        }

        if(i == -1) // its a system command
        {
        cmdold = cmdtmp;
        cmdtmp = cmdtmp->next;

        }
        else // a command we must skip!
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
        else // something illegal from the client. Kill it!
        {
        LOG(llevInfo,"HACKBUG: Wrong binary client command: %d from %s\n",
        cmdptr->buf[0], STRING_SAFE(ns->ip_host));
        return_poolchunk(cmdptr, cmdptr->pool);
        ns->status = Ns_Dead;
        return 1;
        }

        return_poolchunk(cmdptr, cmdptr->pool);
        continue;
        }
        */
    }

    return 0; /* nothing to do, ignore us */
}

//static inline int socket_prepare_commands(NewSocket *ns)
static int socket_prepare_commands(NewSocket *ns) // use this for debugging
{
    int toread;
    command_struct *cmdptr = NULL;
    ReadList *rb = &ns->readbuf;

    while(rb->len) /* there is something in our in buffer - at last we got a command tag */
    {
        /* sanity check - skip all data from as invalid marked sockets */
        if(ns->status >= Ns_Zombie)
        {
            rb->len = rb->pos = 0;
            return 1;
        }

        if(rb->toread) /* we have already a cmd in the readbuf and wait for the data tail? */
            toread = rb->toread;
        else /* we have first to find a legal cmd */
        {
            // sanity check for a legal command
            if( (rb->cmd = rb->buf[rb->pos]) < 0 || rb->cmd >= CLIENT_CMD_MAX_NROF)
            {
                LOG(llevDebug,"BOGUS CLIENT DATA: Invalid command from socket %d: %d\n", ns->fd, rb->cmd);
                rb->len = rb->pos = 0;
                ns->status = Ns_Dead;
                return 1;
            }

#ifdef DEBUG_PROCESS_QUEUE
            LOG(llevDebug,"Found Command: %d\n", rb->cmd);
#endif

            /* there is our command - now lets see we have a data part or not */
            if(cs_commands[rb->cmd].data_len) /* != 0 means there is a data tail */
            {
                if(cs_commands[rb->cmd].data_len > 0) /* >0 means we have a fixed tail length we expect */
                {
                    /* force a fixed data tail length - if the client is sending something wrong,
                     * our command functions will find it out. Never trust a client, if we got cheated
                     * here the client can also cheat the length values in a dynamic command
                     */
                    toread = cs_commands[rb->cmd].data_len;
                }
                else
                {
                    /* 2 bytes after the command tells us about the data length - check they are there */
                    if(rb->len < 3) /* cmd + 2 bytes length */
                        return 0; /* we have to wait */

                    /* our 2 length bytes can appear in 3 states in our round robin buffer */
                    if(rb->pos+1 >= MAXSOCKBUF_IN) /* both bytes are beyond the border */
                        toread = (rb->buf[0] << 8) + rb->buf[1];
                    else if(rb->pos+2 >= MAXSOCKBUF_IN) /* the 2nd bytes is at buf[0] */
                        toread = (rb->buf[rb->pos+1] << 8) + rb->buf[0];
                    else
                        toread = (rb->buf[rb->pos+1] << 8) + rb->buf[rb->pos+2];

                    /* adjust buffer position - 2 bytes for toread length */
                    rb->pos +=2;
                    if(rb->pos >= MAXSOCKBUF_IN)
                        rb->pos -= MAXSOCKBUF_IN;
                    rb->len -= 2;
                }

                /* lets check our "toread" value is senseful! */
                if(toread <= 0 || toread > MAXSOCKBUF_IN)
                {
                    /* bogus command! kill client NOW */
                    LOG(llevDebug,"BOGUS CLIENT DATA: Invalid command (%d)length from socket %d: %d\n", rb->cmd, ns->fd, toread);
                    rb->len = rb->pos = 0;
                    ns->status = Ns_Dead;
                    return 1;
                }

                /* Ok... now we have a valid command with a valid length
                * NOW there are 2 options:
                * 1.) we already have the whole data block in our buffer
                * 2.) or not
                * in the 2nd case we do a nice trick.
                * we store the cmd and the already processed length in the readbuffer struct
                * so we only check the next time for the missing length.
                */
                rb->toread = toread;
            }
            else /* single command - we don't deal with data tails or toread */
            {
                rb->toread = toread = 0; /* sanity settings - there is nothing to do in terms data */
            }

            /* data or not - at last we have found a valid command - adjust the readbuffer */
            if(++rb->pos >= MAXSOCKBUF_IN)
                rb->pos -= MAXSOCKBUF_IN;
            rb->len--;
        }

        /* HERE and ONLY HERE we check the right length */
        if (toread > rb->len) /* for single command toread is 0 and always valid */
            return 0; /* we have to wait */

        /* we have a full command!
         * Now we process the command right out of the buffer in real time.
         * if there is nothing we can do right now, we will go on and store the
         * command in our readbuffer system
         */
        if(pre_process_command(ns) )
        {
            if (ns->addme)
            {
                /* this socket was copied to a player struct after ADDME
                * ns-> is now a invalid mirror - diasble it and give it
                * back to the socket listener
                */
                ns->addme = 0;
                /* account name ptrs are was deleted as we mirrored the socket */
                ns->readbuf.buf = NULL;
                ns->readbuf.len = ns->readbuf.toread = 0; /* sanity settings */
                ns->inactive_when = ROUND_TAG + INACTIVE_SOCKET * pticks_second; // reset idle counter
                socket_info.nconns--;
                ns->status = Ns_Avail;
                return 1; /* leave this instance, the copied socket in the player will go */
            }
            rb->toread = 0; /* sanity setting */
            continue; /* ok, command is processed... try to fetch the next one */
        }

        if(ns->status != Ns_Playing) /* we enqueue *only* for active players! */
        {
            LOG(llevDebug,"BOGUS CLIENT DATA: Invalid command (%d)for NOT Ns_Playing state (%d - %d) from socket %d: %d\n", rb->cmd, Ns_Playing, ns->status, ns->fd, toread);
            rb->len = rb->pos = 0;
            ns->status = Ns_Dead;
            return 1;
        }

        rb->toread = 0; /* turn off the cmd cache marker - we must do it after pre_process_command()! */

        /* We have a command, lets store it in the regular command queue
         * TODO NOTE: we *can* do here alot more as storing.
         * for example can we implement a "weight system", sorting the commands in a "important"
         * list and other in "do it when time".
         * But for know lets get a command chunk which is big enough - this is still for testing,
         * i am sure we don't need so much different buffers.
         */
        if(toread < 16) /* for single commands we still need a queue - so lets use the 16 byte ones */
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

        /* we store the command and the toread length */
        cmdptr->cmd = rb->cmd;
        cmdptr->len = toread;

        if(toread) /* not a single command? */
            copy_cmd_data(rb, cmdptr->buf, toread);

        /* and kick the command in the queue... we are done! next! */
        command_buffer_enqueue(ns, cmdptr);
    };

    return 0;
}


/* the socket is marked as "dead" - means we drop the connection and
 * auto-remove/logout the player
 */
void remove_ns_dead_player(player_t *pl)
{
    LOG(llevInfo, "remove_ns_dead_player(%s): state:%d gmaster:%d g_status:%d\n",
        STRING_OBJ_NAME(pl->ob),pl->state, pl->gmaster_mode, pl->group_status);

    if (!(pl->state & ST_DEAD))
    {
        /* remove the player from global gmaster lists */
        if (pl->gmaster_mode != GMASTER_MODE_NO)
        {
            remove_gmaster_list(pl);
        }

        /* remove player from party */
        if ((pl->group_status & GROUP_STATUS_GROUP))
        {
            party_remove_member(pl, 1);
        }

        /* TODO: Use channels. */
        if (gmaster_list_VOL ||
            gmaster_list_GM ||
            gmaster_list_SA)
        {
            objectlink_t *ol;
            char        buf[SMALL_BUF];

            sprintf(buf, "%s leaves the game (%d still playing).",
                QUERY_SHORT_NAME(pl->ob, NULL), player_active - 1);

            for (ol = gmaster_list_VOL; ol; ol = ol->next)
            {
                ndi(NDI_UNIQUE, 0, ol->objlink.ob, "%s", buf);
            }

            for (ol = gmaster_list_GM; ol; ol = ol->next)
            {
                ndi(NDI_UNIQUE, 0, ol->objlink.ob, "%s", buf);
            }

            for (ol = gmaster_list_SA; ol; ol = ol->next)
            {
                ndi(NDI_UNIQUE, 0, ol->objlink.ob, "%s", buf);
            }
        }

        container_unlink(pl, NULL);

        if (!QUERY_FLAG(pl->ob, FLAG_REMOVED))
        {
            terminate_all_pets(pl->ob);
            activelist_remove(pl->ob);
            (void)leave_map(pl, NULL);
        }

        player_save(pl->ob);
        LOG(llevDebug, "remove_ns_dead_player(): %s leaving\n", STRING_OBJ_NAME(pl->ob));
        leave(pl, 1);
    }

    /* TODO: IDK if this is at all necessary. */
    pl->socket.status = Ns_Dead;
    free_player(pl); /* we *,must* do this here and not in the memory pool - it needs to be a syncron action */
    (void)get_online_players_info(NULL, pl, 1);
}

/* lets check 2 things:
 * a.) mass connection from one IP
 * b.) or is the ip (range) banned
 * return 1 means banned, 0 is ok.
 */
static int check_ip_ban(NewSocket *sock, char *ip)
{
    int       count, i;
    player_t *next, *pl, *ptmp = NULL;

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
            return 0;
        }
        else
            LOG(llevDebug, "login_allow-OK: IP match login_ip:%s\n", settings.login_ip);
    }

    count = 0;
    for (i = socket_info.allocated_sockets - 1; i > 0; i--)
    {
        if (init_sockets[i].status != Ns_Avail && sock != &init_sockets[i] && !strcmp(init_sockets[i].ip_host, ip))
        {
            if(init_sockets[i].status == Ns_Zombie || (init_sockets[i].status != Ns_Dead && ++count >= settings.max_cons_from_one_ip))
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
    if(check_banned(sock, NULL, NULL, ip))
        return 0; /* *IF* banned, we have turned the socket to a Ns_Zombie... */

    /* now check the players we have */
    count = 0;
    for (pl = first_player; pl; pl = next)
    {
        next = pl->next;

        if(!strcmp(pl->socket.ip_host, ip)) /* we have someone playing from same IP? */
        {
            if (pl->socket.status != Ns_Playing)
            {
                remove_ns_dead_player(pl);
                continue;
            }
            else /* allow 2 logged in *real* playing accounts online from same IP */
            {
                count++;

                if (!ptmp)
                {
                    ptmp = pl;
                }
                else
                {
                    /* lets compare the inactivity time.
                     * if needed we will kick the login with the highest inactivity time */
                    if (ptmp->socket.inactive_when <= pl->socket.inactive_when)
                    {
                        ptmp = pl;
                    }
        
                    /* now the tricky part: if we have to many
                     * connects from that IP, we KICK the login
                     * with the highest idle time */
                    if (count > (settings.max_cons_from_one_ip - 1))
                    {
                        LOG(llevDebug, "check_ip_ban(): connection flood: mark player %s Ns_Dead (IP %s)\n",
                            STRING_OBJ_NAME(pl->ob), ptmp->socket.ip_host);
                        remove_ns_dead_player(ptmp);
                        continue;
                    }
                }
            }
        }
    }

    return 0;
}

/* This checks the sockets for input and exceptions, does the right thing.  A
 * bit of this code is grabbed out of socket.c
 * There are 2 lists we need to look through - init_sockets is a list
 */
void doeric_server(int update, struct timeval *timeout)
{
    int     i, pollret;
    int     update_client=update&SOCKET_UPDATE_CLIENT; /* if set we try to poll the socket only */
    int     update_player=update&SOCKET_UPDATE_PLAYER; /* poll socket & do a player turn */

#if WIN32 || !HAVE_GETADDRINFO
    struct sockaddr_in      addr;
#else
    struct sockaddr_storage addr;
#endif
    unsigned int        addrlen = sizeof(addr);
    player_t                     *pl, *next;

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
            continue;
        }
        else if (init_sockets[i].status != Ns_Avail) /* ns_add... */
        {
            if (init_sockets[i].status > Ns_Wait) /* exclude socket #0 which listens for new connects */
            {
                /* kill this after too long inactive... */
                if (init_sockets[i].inactive_when < ROUND_TAG)
                {
                    free_newsocket(&init_sockets[i]);
                    init_sockets[i].status = Ns_Avail;
                    socket_info.nconns--;
                    continue;
                }
            }

            FD_SET((uint32) init_sockets[i].fd, &tmp_read);
            FD_SET((uint32) init_sockets[i].fd, &tmp_exceptions);
            FD_SET((uint32) init_sockets[i].fd, &tmp_write);
        }
    }

    for (pl = first_player; pl; pl = next)
    {
        next = pl->next;

        if (pl->socket.status == Ns_Dead)
        {
            remove_ns_dead_player(pl);
            continue;
        }

        if (pl->socket.inactive_when < ROUND_TAG)
        {
            if (pl->socket.status != Ns_Zombie)
            {
                if (!(pl->gmaster_mode & GMASTER_MODE_SA))
                {
                    if (!pl->socket.inactive_flag)
                    {
                        pl->socket.inactive_when = ROUND_TAG + INACTIVE_PLAYER2 * pticks_second;
                        pl->socket.inactive_flag = 1;
                        ndi(NDI_UNIQUE | NDI_RED, 0, pl->ob, "Inactivity warning! Server will disconnect you in %u seconds.",
                            INACTIVE_PLAYER2);
                    }
                    else
                    {
                        ndi(NDI_UNIQUE | NDI_RED, 0, pl->ob, "Max inactivity time reached! Server is closing connection.");
                        pl->socket.inactive_when = ROUND_TAG + INACTIVE_ZOMBIE * pticks_second;
                        pl->socket.status = Ns_Zombie; /* we hold the socket open for a *bit* */
                        pl->socket.inactive_flag = 1;
                        continue;
                    }
                }
            }
            else /* time to kill! */
            {
                remove_ns_dead_player(pl);
                continue;
            }
        }

        FD_SET((uint32) pl->socket.fd, &tmp_read);
        FD_SET((uint32) pl->socket.fd, &tmp_exceptions);
        FD_SET((uint32) pl->socket.fd, &tmp_write);
    }

    /* our one and only select() - after this call, every player socket has signaled us
     * in the tmp_xxxx objects the signal status: FD_ISSET will check socket for socket
     * for thats signal and trigger read, write or exception (error on socket).
     */
    pollret = select(FD_SETSIZE, &tmp_read, &tmp_write, &tmp_exceptions, timeout);

    if (pollret == -1)
    {
        LOG(llevBug, "BUG: select failed: %s [%d][%d]\n", strerror(errno),errno,socket_info.max_filedescriptor);
        return;
    }

    /* Following adds a new connection */
    if (pollret &&
        FD_ISSET(init_sockets[0].fd, &tmp_read))
    {
        NewSocket *newsock = socket_get_available();

        LOG(llevInfo, "CONNECT from... ");
        newsock->fd = accept(init_sockets[0].fd, (struct sockaddr *) &addr, &addrlen);
        if (newsock->fd != -1)
        {
#if WIN32 || !HAVE_GETADDRINFO
        char *tmp_ip = inet_ntoa(addr.sin_addr);
#else
        char tmp_ip[NI_MAXHOST];

        getnameinfo((struct sockaddr *) &addr, addrlen, tmp_ip, sizeof(tmp_ip), NULL, 0, NI_NUMERICHOST);
#endif
            LOG(llevDebug, " ip %s (socket %d) (%p)\n", tmp_ip, newsock->fd, newsock);
            InitConnection(newsock, tmp_ip);
            check_ip_ban(newsock, tmp_ip);
            if(newsock->status <= Ns_Zombie) /* set from ban check */
                newsock->status = Ns_Login;  /* enhance status from Ns_Wait to Ns_Login to allow account login */
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
                    socket_prepare_commands(&init_sockets[i]);
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
        if (pl->socket.status == Ns_Dead ||
            FD_ISSET(pl->socket.fd, &tmp_exceptions))
        {
            remove_ns_dead_player(pl);
            continue;
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
                    LOG(llevDebug, "Drop ConnectionB: %s (%s)\n", STRING_OBJ_NAME(pl->ob), pl->socket.ip_host);
                    pl->socket.status = Ns_Dead;
                }
                else
                {
                    socket_prepare_commands(&pl->socket);
                }
            }

            if (pl->socket.status == Ns_Dead) /* perhaps something was bad? */
            {
                remove_ns_dead_player(pl);  /* or player has left game */
                continue;
            }
            else
            {
                /* Update the players stats once per tick.  More efficient than
                 * sending them whenever they change, and probably just as useful
                 * (why is update the stats per tick more efficent as we set a update sflag??? MT)
                 */
                if (update_client)
                {
                    if (update_player &&
                        (pl->state & ST_PLAYING))
                    {
                        esrv_update_stats(pl);

                        if (pl->update_skills)
                        {
                            send_skilllist_cmd(pl, -1, SPLIST_MODE_UPDATE);
                            pl->update_skills = 0;
                        }

                        draw_client_map(pl);
                    }

                    if (FD_ISSET(pl->socket.fd, &tmp_write))
                    {
                        write_socket_buffer(&pl->socket);
                    }
                }
            }
        }
    } /* for() end */
}
