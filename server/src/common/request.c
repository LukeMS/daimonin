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

	The author can be reached via e-mail to info@daimonin.net
*/

/*
 * This file implements all of the goo on the server side for handling
 * clients.  It's got a bunch of global variables for keeping track of
 * each of the clients.
 *
 * Note:  All functions that are used to process data from the client
 * have the prototype of (char *data, int datalen, int client_num).  This
 * way, we can use one dispatch table.
 *
 */

#include <global.h>

/* This block is basically taken from socket.c - I assume if it works there,
 * it should work here.
 */
#ifndef WIN32 /* ---win32 exclude unix headers */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#endif /* win32 */

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

static int  cs_stat_skillexp[]    =
{
    CS_STAT_SKILLEXP_AGILITY, CS_STAT_SKILLEXP_PERSONAL, CS_STAT_SKILLEXP_MENTAL, CS_STAT_SKILLEXP_PHYSIQUE,
    CS_STAT_SKILLEXP_MAGIC, CS_STAT_SKILLEXP_WISDOM
};


/* This is the Setup cmd - easy first implementation */
void SetUp(char *buf, int len, NewSocket *ns)
{
    int     s;
    char   *cmd, *param, tmpbuf[MAX_BUF], *cmdback;

    if (!buf || !len)
        return;

    if(!ns->version)
    {
        LOG(llevInfo, "HACKBUG: setup command before version %s\n", STRING_SAFE(ns->ip_host));
        ns->status = Ns_Dead;
        return;
    }
    if (ns->setup)
    {
        LOG(llevInfo, "HACKBUG: double call of setup cmd from socket %s\n", STRING_SAFE(ns->ip_host));
        ns->status = Ns_Dead;
        return;
    }
    ns->setup = 1;

    /* run through the cmds of setup
     * syntax is setup <cmdname1> <parameter> <cmdname2> <parameter> ...
     *
     * we send the status of the cmd back, or a FALSE is the cmd is the server unknown
     * The client then must sort this out
     */

    LOG(llevInfo, "Get SetupCmd:: %s\n", buf);
	/* we collect our buffer stuff by hand and use strlen() then to get the size */
	cmdback = SOCKBUF_REQUEST_BUFFER(ns, 250);

	/* create the endian template so the client can shift right */
	*((uint16*) (cmdback)) = 0x0201;
	*((uint32*) (cmdback+2)) = 0x04030201;

	cmdback[6] = 0; /* faked string end so we can simply strcat below */

    for (s = 0; s < len;)
    {
        cmd = &buf[s];

        /* find the next space, and put a null there */
        for (; s < len && buf[s] && buf[s] != ' '; s++)
            ;
        buf[s++] = 0;
        while (s < len && buf[s] == ' ')
            s++;

        if (s >= len)
            break;

        param = &buf[s];

        for (; s < len && buf[s] && buf[s] != ' '; s++)
            ;
        buf[s++] = 0;
        while (s < len && buf[s] == ' ')
            s++;

        strcat(cmdback, " ");
        strcat(cmdback, cmd);
        strcat(cmdback, " ");

        if (!strcmp(cmd, "sound"))
        {
            ns->sound = atoi(param);
            strcat(cmdback, param);
        }
        else if (!strcmp(cmd, "darkness"))
        {
            ns->darkness = atoi(param);
            strcat(cmdback, param);
        }
        else if (!strcmp(cmd, "map2cmd"))
        {
            ns->map2cmd = atoi(param);
            /* if beyond this size, need to use map2cmd no matter what */
            if (ns->mapx > 11 || ns->mapy > 11)
                ns->map2cmd = 1;
            strcat(cmdback, ns->map2cmd ? "1" : "0");
        }
        else if (!strcmp(cmd, "facecache"))
        {
#ifdef SERVER_SEND_FACES
            ns->facecache = atoi(param);
            strcat(cmdback, param);
#endif
        }
        else if (!strcmp(cmd, "faceset"))
        {
#ifdef SERVER_SEND_FACES
            int q   = atoi(param);

            if (is_valid_faceset(q))
                ns->faceset = q;
            sprintf(tmpbuf, "%d", ns->faceset);
            strcat(cmdback, tmpbuf);
            /* if the client is using faceset, it knows about image2 command */
            ns->image2 = 1;
#endif
        }
        else if (!strcmp(cmd, "mapsize"))
        {
            int     x, y = 0;
            char   *cp;

            x = atoi(param);
            for (cp = param; *cp != 0; cp++)
                if (*cp == 'x' || *cp == 'X')
                {
                    y = atoi(cp + 1);
                    break;
                }
            if (x <9 || y <9 || x>MAP_CLIENT_X || y> MAP_CLIENT_Y)
            {
                sprintf(tmpbuf, " %dx%d", MAP_CLIENT_X, MAP_CLIENT_Y);
                strcat(cmdback, tmpbuf);
            }
            else
            {
                ns->mapx = x;
                ns->mapy = y;
                ns->mapx_2 = x / 2;
                ns->mapy_2 = y / 2;
                /* better to send back what we are really using and not the
                     * param as given to us in case it gets parsed differently.
                     */
                sprintf(tmpbuf, "%dx%d", x, y);
                strcat(cmdback, tmpbuf);
            }
        }
        else if (!strcmp(cmd, "skf"))
        {
            char   *cp;
            int     x   = 0;
            uint32  y   = 0;

                 /* is x our files len and y the crc */
            for (cp = param; *cp != 0; cp++)
                if (*cp == '|')
                {
                    *cp = 0;
                    x = atoi(param);
                    y = strtoul(cp + 1, NULL, 16);
                    break;
                }
                 /* we check now the loaded file data - if different
                      * we tell it the client - if not, we skip here
                      */
            if (SrvClientFiles[SRV_CLIENT_SKILLS].len_ucomp != x || SrvClientFiles[SRV_CLIENT_SKILLS].crc != y)
            {
                sprintf(tmpbuf, "%d|%x", SrvClientFiles[SRV_CLIENT_SKILLS].len_ucomp,
                        SrvClientFiles[SRV_CLIENT_SKILLS].crc);
                strcat(cmdback, tmpbuf);
            }
            else
                strcat(cmdback, "OK");
        }
        else if (!strcmp(cmd, "spf"))
        {
            char   *cp;
            int     x   = 0;
            uint32  y   = 0;

                 /* is x our files len and y the crc */
            for (cp = param; *cp != 0; cp++)
                if (*cp == '|')
                {
                    *cp = 0;
                    x = atoi(param);
                    y = strtoul(cp + 1, NULL, 16);
                    break;
                }
                 /* we check now the loaded file data - if different
                      * we tell it the client - if not, we skip here
                      */
            if (SrvClientFiles[SRV_CLIENT_SPELLS].len_ucomp != x || SrvClientFiles[SRV_CLIENT_SPELLS].crc != y)
            {
                sprintf(tmpbuf, "%d|%x", SrvClientFiles[SRV_CLIENT_SPELLS].len_ucomp,
                        SrvClientFiles[SRV_CLIENT_SPELLS].crc);
                strcat(cmdback, tmpbuf);
            }
            else
                strcat(cmdback, "OK");
        }
        else if (!strcmp(cmd, "stf"))
        {
            char   *cp;
            int     x   = 0;
            uint32  y   = 0;

                 /* is x our files len and y the crc */
            for (cp = param; *cp != 0; cp++)
                if (*cp == '|')
                {
                    *cp = 0;
                    x = atoi(param);
                    y = strtoul(cp + 1, NULL, 16);
                    break;
                }
                 /* we check now the loaded file data - if different
                      * we tell it the client - if not, we skip here
                      */
            if (SrvClientFiles[SRV_CLIENT_SETTINGS].len_ucomp != x || SrvClientFiles[SRV_CLIENT_SETTINGS].crc != y)
            {
                sprintf(tmpbuf, "%d|%x", SrvClientFiles[SRV_CLIENT_SETTINGS].len_ucomp,
                        SrvClientFiles[SRV_CLIENT_SETTINGS].crc);
                strcat(cmdback, tmpbuf);
            }
            else
                strcat(cmdback, "OK");
        }
        else if (!strcmp(cmd, "bpf"))
        {
            char   *cp;
            int     x   = 0;
            uint32  y   = 0;

                 /* is x our files len and y the crc */
            for (cp = param; *cp != 0; cp++)
                if (*cp == '|')
                {
                    *cp = 0;
                    x = atoi(param);
                    y = strtoul(cp + 1, NULL, 16);
                    break;
                }
                 /* we check now the loaded file data - if different
                      * we tell it the client - if not, we skip here
                      */
            if (SrvClientFiles[SRV_CLIENT_BMAPS].len_ucomp != x || SrvClientFiles[SRV_CLIENT_BMAPS].crc != y)
            {
                sprintf(tmpbuf, "%d|%x", SrvClientFiles[SRV_CLIENT_BMAPS].len_ucomp,
                        SrvClientFiles[SRV_CLIENT_BMAPS].crc);
                strcat(cmdback, tmpbuf);
            }
            else
                strcat(cmdback, "OK");
        }
        else if (!strcmp(cmd, "amf"))
        {
            char   *cp;
            int     x   = 0;
            uint32  y   = 0;

                 /* is x our files len and y the crc */
            for (cp = param; *cp != 0; cp++)
                if (*cp == '|')
                {
                    *cp = 0;
                    x = atoi(param);
                    y = strtoul(cp + 1, NULL, 16);
                    break;
                }
                 /* we check now the loaded file data - if different
                      * we tell it the client - if not, we skip here
                      */
            if (SrvClientFiles[SRV_CLIENT_ANIMS].len_ucomp != x || SrvClientFiles[SRV_CLIENT_ANIMS].crc != y)
            {
                sprintf(tmpbuf, "%d|%x", SrvClientFiles[SRV_CLIENT_ANIMS].len_ucomp,
                        SrvClientFiles[SRV_CLIENT_ANIMS].crc);
                strcat(cmdback, tmpbuf);
            }
            else
                strcat(cmdback, "OK");
        }
        else
        {
            /* Didn't get a setup command we understood -
               * report a failure to the client.
               */
            strcat(cmdback, "FALSE");
        }
    } /* for processing all the setup commands */

    /*LOG(llevInfo,"SendBack SetupCmd:: %s\n", cmdback);*/
	SOCKBUF_REQUEST_FINISH(ns, BINARY_CMD_SETUP, strlen(cmdback));
}

/* The client has requested to be added to the game.  This is what
 * takes care of it.  We tell the client how things worked out.
 * I am not sure if this file is the best place for this function.  however,
 * it either has to be here or init_sockets needs to be exported.
 */
void AddMeCmd(char *buf, int len, NewSocket *ns)
{
    player     *pl=NULL;

    /* add_player() will move the login to the next step - adding a "connect"
     * as a uninitialized player AND send a get_name() which is "tell me your name".
     * ->addme flag is only a temp flag for the socket loop - because we close here
     * the socket or, when all is ok, we change to Ns_Login mode.
     */
    if (!ns->setup || !ns->version || ns->status != Ns_Add || !(pl=add_player(ns)))
    {
		Write_Command_To_Socket(ns, BINARY_CMD_ADDME_FAIL);
        ns->login_count = ROUND_TAG+(uint32)(10.0f * pticks_second);
        ns->status = Ns_Zombie; /* we hold the socket open for a *bit* */
        ns->idle_flag = 1;

    }
    else
    {
        /* lets preset our old socket so it can be given back */
        ns->addme = 1;
        ns = &pl->socket; /* map us to our copied socket - command queue was copied too! */

        /* remove now the FIRST command in the command queue - that MUST be our addme.
         * don't remove the poolchunk, thats done in the caller functio´n */
        ns->cmd_start = ns->cmd_start->next;
        if(!ns->cmd_start)
            ns->cmd_end = NULL;

        Write_Command_To_Socket(ns, BINARY_CMD_ADDME_SUC);
        LOG(llevDebug, "addme_cmd(): socket %d\n", ns->fd);
    }
}


/* This handles the general commands from the client (ie, north, fire, cast,
 * etc.)
 */
void PlayerCmd(char *buf, int len, player *pl)
{
    if (!buf || !len || !pl || pl->socket.status == Ns_Dead)
    {
        if(pl)
            pl->socket.status = Ns_Dead;
        return;
    }

    /* The following should never happen with a proper or honest client.
     * Therefore, the error message doesn't have to be too clear - if
     * someone is playing with a hacked/non working client, this gives them
     * an idea of the problem, but they deserve what they get
     */
    if (pl->state != ST_PLAYING)
    {
        new_draw_info_format(NDI_UNIQUE, 0, pl->ob, "You can not issue commands - state is not ST_PLAYING (%s)", buf);
        return;
    }
    /* Check if there is a count.  In theory, a zero count could also be
     * sent, so check for that also.
     */
    if (atoi(buf) || buf[0] == '0')
    {
        pl->count = atoi((char *) buf);
        buf = strchr(buf, ' '); /* advance beyond the numbers */
        if (!buf)
        {
#ifdef ESRV_DEBUG
            LOG(llevDebug, "PlayerCmd: Got count but no command.");
#endif
            return;
        }
        buf++;
    }

    /* In c_new.c */
    execute_newserver_command(pl->ob, (char *) buf);
    /* Perhaps something better should be done with a left over count.
     * Cleaning up the input should probably be done first - all actions
     * for the command that issued the count should be done before any other
     * commands.
     */

    pl->count = 0;
}


/* This handles the general commands from the client (ie, north, fire, cast,
 * etc.)  It is a lot like PlayerCmd above, but is called with the
 * 'ncom' method which gives more information back to the client so it
 * can throttle.
 */
/* this command is somewhat bugged - we get several ncom corruption messages where
 * the size is < 7 - even this seems impossible in the client. We simple skip it here.
 * reworking the commands is on the todo so we spare the debug time and implement it new
 * and better from scratch.
 */
void NewPlayerCmd(char *buf, int len, player *pl)
{
    uint16  packet;
    int     repeat;
    char    command[MAX_BUF];
    /*    SockList sl;*/

    if (!buf || !len || !pl || pl->socket.status == Ns_Dead)
    {
        if(pl)
            pl->socket.status = Ns_Dead;
        return;
    }

    if (len < 7)
    {
        /*LOG(llevBug,"BUG: Corrupt ncom command from player %s - not long enough (len: %d)- discarding\n", pl->ob->name,len);*/
        return;
    }

    packet = GetShort_String((uint8 *) buf);
    repeat = GetInt_String((uint8 *) buf + 2);
    /* -1 is special - no repeat, but don't update */
    if (repeat != -1)
    {
        pl->count = repeat;
    }
    if ((len - 4) >= MAX_BUF)
        len = MAX_BUF - 5;

    strncpy(command, (char *) buf + 6, len - 4);
    command[len - 4] = '\0';

    /* The following should never happen with a proper or honest client.
     * Therefore, the error message doesn't have to be too clear - if
     * someone is playing with a hacked/non working client, this gives them
     * an idea of the problem, but they deserve what they get
     */
    if (pl->state != ST_PLAYING)
    {
        new_draw_info_format(NDI_UNIQUE, 0, pl->ob, "You can not issue commands - state is not ST_PLAYING (%s)", buf);
        return;
    }

    /* In c_new.c */
    execute_newserver_command(pl->ob, command);
    pl->count = 0;

}


/* This is a reply to a previous query. */
void ReplyCmd(char *buf, int len, player *pl)
{
    char                write_buf[MAX_BUF];

    /* This is to synthesize how the data would be stored if it
     * was normally entered.  A bit of a hack, and should be cleaned up
     * once all the X11 code is removed from the server.
     *
     * We pass 13 to many of the functions because this way they
     * think it was the carriage return that was entered, and the
     * function then does not try to do additional input.
     */

    if (!buf || !len || !pl || pl->socket.status == Ns_Dead)
    {
        if(pl)
            pl->socket.status = Ns_Dead;
        return;
    }

    strcpy(write_buf, ":");
    strncat(write_buf, buf, 250);
    write_buf[250] = 0;
    pl->socket.ext_title_flag = 1;

    switch (pl->state)
    {
        case ST_PLAYING:
          pl->socket.status = Ns_Dead;
          LOG(llevBug, "BUG: Got reply message with ST_PLAYING input state (player %s)\n", query_name(pl->ob));
          break;

        case ST_GET_NAME:
          receive_player_name(pl->ob, MAX_PLAYER_NAME, write_buf+1);
          break;

        case ST_GET_PASSWORD:
        case ST_CONFIRM_PASSWORD:
          receive_player_password(pl->ob, MAX_PLAYER_NAME, write_buf);
          break;

        default:
          pl->socket.status = Ns_Dead;
          LOG(llevBug, "BUG: Unknown input state: %d\n", pl->state);
          break;
    }
}

/* request a srv_file! */
void RequestFileCmd(char *buf, int len, NewSocket *ns)
{
    int id;

    /* *only* allow this command between the first login and the "addme" command! */
    if (!ns->setup || !ns->version || ns->status != Ns_Add || !buf || !len)
    {
        LOG(llevInfo, "RF: received bad rf command for IP:%s\n", STRING_SAFE(ns->ip_host));
        ns->status = Ns_Dead;
        return;
    }

    id = atoi(buf);
    if (id < 0 || id >= SRV_CLIENT_FILES)
    {
        LOG(llevInfo, "RF: received bad rf command for IP:%s\n", STRING_SAFE(ns->ip_host));
        ns->status = Ns_Dead;
        return;
    }

    if (id == SRV_CLIENT_SKILLS)
    {
        if (ns->rf_skills)
        {
            LOG(llevInfo, "RF: received bad rf command - double call skills \n");
            ns->status = Ns_Dead;
            return;
        }
        else
            ns->rf_skills = 1;
    }
    else if (id == SRV_CLIENT_SPELLS)
    {
        if (ns->rf_spells)
        {
            LOG(llevInfo, "RF: received bad rf command - double call spells \n");
            ns->status = Ns_Dead;
            return;
        }
        else
            ns->rf_spells = 1;
    }
    else if (id == SRV_CLIENT_SETTINGS)
    {
        if (ns->rf_settings)
        {
            LOG(llevInfo, "RF: received bad rf command - double call settings \n");
            ns->status = Ns_Dead;
            return;
        }
        else
            ns->rf_settings = 1;
    }
    else if (id == SRV_CLIENT_BMAPS)
    {
        if (ns->rf_bmaps)
        {
            LOG(llevInfo, "RF: received bad rf command - double call bmaps \n");
            ns->status = Ns_Dead;
            return;
        }
        else
            ns->rf_bmaps = 1;
    }
    else if (id == SRV_CLIENT_ANIMS)
    {
        if (ns->rf_anims)
        {
            LOG(llevInfo, "RF: received bad rf command - double call anims \n");
            ns->status = Ns_Dead;
            return;
        }
        else
            ns->rf_anims = 1;
    }

    LOG(llevDebug, "Client %s rf #%d\n", ns->ip_host, id);
	SOCKBUF_ADD_TO_SOCKET(ns, SrvClientFiles[id].sockbuf);
}


/* Client tells its version.  If there is a mismatch, we close the
 * socket.  In real life, all we should care about is the client having
 * something older than the server.  If we assume the client will be
 * backwards compatible, having it be a later version should not be a
 * problem.
 */
void VersionCmd(char *buf, int len, NewSocket *ns)
{
    char   *cp;


    if (!buf || !len || ns->version)
    {
        LOG(llevInfo, "CS: received corrupted version command\n");
        ns->status = Ns_Dead;
        return;
    }

    ns->version = 1;
    ns->cs_version = atoi(buf);
    ns->sc_version = ns->cs_version;
    if (VERSION_CS != ns->cs_version)
    {
        LOG(llevInfo, "CS: csversion mismatch (%d,%d)\n", VERSION_CS, ns->cs_version);
        ns->status = Ns_Dead;
        return;
    }
    cp = strchr(buf + 1, ' ');
    if (!cp)
    {
        LOG(llevInfo, "CS: invalid version cmd: %s\n", buf);
        ns->status = Ns_Dead;
        return;
    }
    ns->sc_version = atoi(cp);
    if (VERSION_SC != ns->sc_version)
    {
        LOG(llevInfo, "CS: scversion mismatch (%d,%d)\n", VERSION_SC, ns->sc_version);
        ns->status = Ns_Dead;
        return;
    }
    cp = strchr(cp + 1, ' ');
    if (!cp || strncmp("Daimonin SDL Client", cp + 1, 19))
    {
        if (cp)
            LOG(llevInfo, "CS: connection from false client of type <%s>\n", cp);
        else
            LOG(llevInfo, "CS: connection from false client (invalid name)\n");
        ns->status = Ns_Dead;
        return;
    }
}

/* sound related functions. */

void SetSound(char *buf, int len, NewSocket *ns)
{
    if (!buf || !len)
        return;

    ns->sound = atoi(buf);
}

/* Moves an object (typically, container to inventory
 * move <to> <tag> <nrof>
 */
void MoveCmd(char *buf, int len, player *pl)
{
    int vals[3], i;

    if (!buf || !len || !pl || pl->socket.status == Ns_Dead)
    {
        if(pl)
            pl->socket.status = Ns_Dead;
        return;
    }

    /* A little funky here.  We only cycle for 2 records, because
     * we obviously am not going to find a space after the third
     * record.  Perhaps we should just replace this with a
     * sscanf?
     */
    for (i = 0; i < 2; i++)
    {
        vals[i] = atoi(buf);
        if (!(buf = strchr(buf, ' ')))
        {
            LOG(llevInfo, "CLIENT(BUG): Incomplete move command: %s from player %s\n", buf, query_name(pl->ob));
            return;
        }
        buf++;
    }
    vals[2] = atoi(buf);

    esrv_move_object(pl->ob, vals[0], vals[1], vals[2]);
}



/******************************************************************************
 *
 * Start of commands the server sends to the client.
 *
 ******************************************************************************/

/*
 * send_query asks the client to query the user.  This way, the client knows
 * it needs to send something back (vs just printing out a message
 */
void send_query(NewSocket *ns, uint8 flags, char *text)
{
    char    buf[MAX_BUF];

    sprintf(buf, "%d %s", flags, text ? text : "");
    Write_String_To_Socket(ns, BINARY_CMD_QUERY, buf, strlen(buf));
}

void esrv_update_skills(player *pl)
{
    object *tmp2;
    int     i;
    char    buf[256];
    char    tmp[2048]; /* we should careful set a big enough buffer here */

    sprintf(tmp, "%d ", SPLIST_MODE_UPDATE);

    for (i = 0; i < NROFSKILLS; i++)
    {
        /* update exp skill we have only */
        if (pl->skill_ptr[i] && pl->skill_ptr[i]->last_eat)
        {
            tmp2 = pl->skill_ptr[i];
            /* send only when really something has changed */
            if (tmp2->stats.exp != pl->skill_exp[i] || tmp2->level != pl->skill_level[i])
            {
                if (tmp2->last_eat == 1)
                    sprintf(buf, "/%s|%d|%d", tmp2->name, tmp2->level, tmp2->stats.exp);
                else if (tmp2->last_eat == 2)
                    sprintf(buf, "/%s|%d|-2", tmp2->name, tmp2->level);
                else
                    sprintf(buf, "/%s|%d|-1", tmp2->name, tmp2->level);
                strcat(tmp, buf);
                pl->skill_exp[i] = tmp2->stats.exp;
                pl->skill_level[i] = tmp2->level;
            }
        }
    }

    Write_String_To_Socket(&pl->socket, BINARY_CMD_SKILL_LIST, tmp, strlen(tmp));
}

/*
 * esrv_update_stats sends a statistics update.  We look at the old values,
 * and only send what has changed.  Stat mapping values are in newclient.h
 * Since this gets sent a lot, this is actually one of the few binary
 * commands for now.
 */
void esrv_update_stats(player *pl)
{
    int         i, group_update = 0; /* set to true when a group update stat has changed */
	sockbuf_struct * AddIf_SOCKBUF_PTR;
    uint16      flags;

	SOCKBUF_REQUEST_BUFFER(&pl->socket, 128);
	AddIf_SOCKBUF_PTR = ACTIVE_SOCKBUF(&pl->socket);

    /* small trick: we want send the hp bar of our target to the player.
     * We want send a char with x% the target has of full hp.
     * To avoid EVERY time the % calculation, we store the real HP
     * - if it has changed, we calc the % and use them normal.
     * this simple compare will not deal in speed but we safe
     * some unneeded calculations.
     */
    if (pl->target_object != pl->ob) /* never send our own status - client will sort this out */
    {
        /* we don't care about count - target function will readjust itself */
        if (pl->target_object && pl->target_object->stats.hp != pl->target_hp) /* just for secure...*/
        {
            /* well, i would like to avoid this calc here but we won't give the player the true hp value of target */
            char hp = (char) (((float) pl->target_object->stats.hp / (float) pl->target_object->stats.maxhp) * 100.0f);
            pl->target_hp = pl->target_object->stats.hp;
            AddIfChar(pl->target_hp_p, hp, CS_STAT_TARGET_HP);
        }
    }

    AddIfShort(pl->last_gen_hp, pl->gen_hp, CS_STAT_REG_HP);
    AddIfShort(pl->last_gen_sp, pl->gen_sp, CS_STAT_REG_MANA);
    AddIfShort(pl->last_gen_grace, pl->gen_grace, CS_STAT_REG_GRACE);
    AddIfCharFlag(pl->last_level, pl->ob->level, group_update, GROUP_UPDATE_LEVEL, CS_STAT_LEVEL);
    AddIfInt(pl->last_weight_limit, pl->weight_limit, CS_STAT_WEIGHT_LIM);
    AddIfInt(pl->last_weapon_sp, pl->weapon_sp, CS_STAT_WEAP_SP);
    AddIfInt(pl->last_speed_enc, pl->speed_enc, CS_STAT_SPEED);
    AddIfInt(pl->last_spell_fumble, pl->spell_fumble, CS_STAT_SPELL_FUMBLE);

    if (pl->ob != NULL)
    {
        /* these will update too when we are in a group */
        AddIfIntFlag(pl->last_stats.hp, pl->ob->stats.hp, group_update, GROUP_UPDATE_HP, CS_STAT_HP);
        AddIfIntFlag(pl->last_stats.maxhp, pl->ob->stats.maxhp, group_update, GROUP_UPDATE_MAXHP, CS_STAT_MAXHP);
        AddIfShortFlag(pl->last_stats.sp, pl->ob->stats.sp, group_update, GROUP_UPDATE_SP, CS_STAT_SP);
        AddIfShortFlag(pl->last_stats.maxsp, pl->ob->stats.maxsp, group_update, GROUP_UPDATE_MAXSP, CS_STAT_MAXSP);
        AddIfShortFlag(pl->last_stats.grace, pl->ob->stats.grace, group_update, GROUP_UPDATE_GRACE, CS_STAT_GRACE);
        AddIfShortFlag(pl->last_stats.maxgrace, pl->ob->stats.maxgrace, group_update, GROUP_UPDATE_MAXGRACE,CS_STAT_MAXGRACE);

        AddIfChar(pl->last_stats.Str, pl->ob->stats.Str, CS_STAT_STR);
        AddIfChar(pl->last_stats.Int, pl->ob->stats.Int, CS_STAT_INT);
        AddIfChar(pl->last_stats.Pow, pl->ob->stats.Pow, CS_STAT_POW);
        AddIfChar(pl->last_stats.Wis, pl->ob->stats.Wis, CS_STAT_WIS);
        AddIfChar(pl->last_stats.Dex, pl->ob->stats.Dex, CS_STAT_DEX);
        AddIfChar(pl->last_stats.Con, pl->ob->stats.Con, CS_STAT_CON);
        AddIfChar(pl->last_stats.Cha, pl->ob->stats.Cha, CS_STAT_CHA);

        AddIfInt(pl->last_stats.exp, pl->ob->stats.exp, CS_STAT_EXP);
        AddIfShort(pl->last_stats.wc, pl->ob->stats.wc, CS_STAT_WC);
        AddIfShort(pl->last_stats.ac, pl->ob->stats.ac, CS_STAT_AC);
        AddIfShort(pl->last_dps, pl->dps, CS_STAT_DAM);
        AddIfShort(pl->last_food_status, pl->food_status, CS_STAT_FOOD);

		AddIfShort(pl->dist_last_wc, pl->dist_wc, CS_STAT_DIST_WC);
		AddIfShort(pl->dist_last_dps, pl->dist_dps, CS_STAT_DIST_DPS);
		AddIfInt(pl->dist_last_action_time, pl->dist_action_time, CS_STAT_DIST_TIME);
    }

    for (i = 0; i < NROFSKILLGROUPS_ACTIVE; i++)
    {
        AddIfInt(pl->last_exp_obj_exp[i], pl->exp_obj_ptr[i]->stats.exp, cs_stat_skillexp[i]);
        AddIfChar(pl->last_exp_obj_level[i], pl->exp_obj_ptr[i]->level, cs_stat_skillexp[i]+1);
    }

    flags = 0;
    if (pl->run_on)
        flags |= SF_RUNON;
    /* we add additional player status flags - in old style, you got a msg
     * in the text windows when you get xray of get blineded - we will skip
     * this and add the info here, so the client can track it down and make
     * it the user visible in it own, server indepentend way.
     */

    if (QUERY_FLAG(pl->ob, FLAG_BLIND)) /* player is blind */
        flags |= SF_BLIND;
    if (QUERY_FLAG(pl->ob, FLAG_XRAYS)) /* player has xray */
        flags |= SF_XRAYS;
    if (QUERY_FLAG(pl->ob, FLAG_SEE_IN_DARK)) /* player has infravision */
        flags |= SF_INFRAVISION;
    AddIfShort(pl->last_flags, flags, CS_STAT_FLAGS);

    /* TODO: Add a fix_player marker here for all values who MUST be altered in fix_player */
    for (i = 0; i < NROFATTACKS; i++)
        AddIfChar(pl->last_resist[i], pl->ob->resist[i], CS_STAT_RES_START+i);

    if (pl->socket.ext_title_flag)
    {
        generate_ext_title(pl);
        SockBuf_AddChar( AddIf_SOCKBUF_PTR , CS_STAT_EXT_TITLE);
        i = (int) strlen(pl->ext_title);
        SockBuf_AddChar( AddIf_SOCKBUF_PTR , i+1);
		SockBuf_AddString( AddIf_SOCKBUF_PTR, pl->ext_title, i);
        pl->socket.ext_title_flag = 0;
    }
    /* Only send it away if we have some actual data */
    if (SOCKBUF_REQUEST_BUFSIZE( AddIf_SOCKBUF_PTR ))
		SOCKBUF_REQUEST_FINISH(&pl->socket, BINARY_CMD_STATS, SOCKBUF_DYNAMIC);
	else
		SOCKBUF_REQUEST_RESET(&pl->socket);

    if(group_update && pl->group_status & GROUP_STATUS_GROUP && pl->update_ticker != ROUND_TAG)
        party_client_group_update(pl->ob, group_update);
    pl->update_ticker = ROUND_TAG;
}


void esrv_new_player(player *pl, uint32 weight)
{
    int len;
	sockbuf_struct *sptr;

	SOCKBUF_REQUEST_BUFFER(&pl->socket, 128);
	sptr = ACTIVE_SOCKBUF(&pl->socket);

	SockBuf_AddInt(sptr, pl->ob->count);
    SockBuf_AddInt(sptr, weight);
    SockBuf_AddInt(sptr, pl->ob->face->number);
    SockBuf_AddChar(sptr,(len=strlen(pl->ob->name)));
	SockBuf_AddString(sptr, pl->ob->name, len);

	SOCKBUF_REQUEST_FINISH(&pl->socket, BINARY_CMD_PLAYER, SOCKBUF_DYNAMIC);
}

/* Need to send an animation sequence to the client.
 * We will send appropriate face commands to the client if we haven't
 * sent them the face yet (this can become quite costly in terms of
 * how much we are sending - on the other hand, this should only happen
 * when the player logs in and picks stuff up.
 */
/* This function is not used - it was disabled with the client
 * bmpa & anim cache patch (beta 2). This code is still here -
 * perhaps we use it later again - MT
 */
void esrv_send_animation(NewSocket *ns, short anim_num)
{
}
