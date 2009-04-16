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

    The author can be reached via e-mail to michtoen@daimonin.net
*/

#include "global.h"

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

/* New ban code.
 * entries are hold in memory and only loaded/saved between server
 * starts.
 * Code will work dynamic and use the memorypool system.
 * It will interact with temp. ban invoked by kick command or others.
 *
 * Whatever: most of this code should be part of a login server and not
 * of the game server itself. MT-2005
 */

static char *ban_buf_name_def = "3 You are banned from Daimonin.\nGoodbye.";
static char *ban_buf_ip_def = "3 IP is banned from Daimonin.\nGoodbye.";

/* returns a objectlink with ban_struct
 * we use for both the memorypool system
 */
static objectlink *get_ban_node(void)
{
    objectlink        *ol   = get_objectlink(OBJLNK_FLAG_BAN);
    struct ban_struct  *gptr = (struct ban_struct *) get_poolchunk(pool_bannode);

    memset(gptr, 0, sizeof(struct ban_struct));
    ol->objlink.ban = gptr;

    return ol;
}

/* free the gmaster node and the used objectlink
 */
static void free_ban_node(objectlink *ol)
{
    /* only called from remove_entry() - there is name hash deleted too */
    return_poolchunk(ol->objlink.ban, pool_bannode);
    return_poolchunk(ol, pool_objectlink);
}

/* load, parse and setup the ban file & system.
 */
void load_ban_file(void)
{
    FILE   *dmfile;
    int     ticks, ticks_left;
    char    ip[40];
    char    buf[HUGE_BUF];
    char    line_buf[MAX_BUF], name[MAX_BUF];

    LOG(llevInfo,"loading ban_file....\n");
    sprintf(buf, "%s/%s", settings.localdir, BANFILE);
    if ((dmfile = fopen(buf, "r")) == NULL)
    {
        LOG(llevDebug, "Could not find ban_file file.\n");
        return;
    }
    while (fgets(line_buf, 160, dmfile) != NULL)
    {
        if (line_buf[0] == '#')
            continue;
        if (sscanf(line_buf, "%s %s %d %d", name, ip, &ticks, &ticks_left) < 2)
            LOG(llevBug, "BUG: malformed banfile file entry: %s\n", line_buf);
        else
            add_ban_entry(!strcmp(name, "_") ? NULL : name, !strcmp(ip, "_") ? NULL : ip, ticks, ticks_left); /* "_" is a placeholder for IP only */
    }

    fclose(dmfile);
}

/* save the valid ban_file entries.
 */
void save_ban_file(void)
{
    char    filename[MAX_BUF];
    objectlink *ol, *ol_tmp;
    FILE   *fp;

    LOG(llevSystem,"write ban_file...\n");
    sprintf(filename, "%s/%s", settings.localdir, BANFILE);
    if ((fp = fopen(filename, "w")) == NULL)
    {
        LOG(llevBug, "BUG: Cannot open %s for writing\n", filename);
        return;
    }
    fprintf(fp, "# BAN_FILE (file is changed from server at runtime)\n");
    fprintf(fp, "# entry format is '<name> <ip> <ticks_init> <ticks_left>'\n");

    for(ol = ban_list_player;ol;ol=ol_tmp)
    {
        ol_tmp = ol->next;
        if(ol->objlink.ban->ticks_init != -1 &&  pticks >= ol->objlink.ban->ticks)
            remove_ban_entry(ol); /* is not valid anymore, gc it on the fly */
        else
        {
            fprintf(fp, "%s _ %d %d\n", ol->objlink.ban->name, ol->objlink.ban->ticks_init,
                                        ol->objlink.ban->ticks_init==-1?-1:(int)(ol->objlink.ban->ticks-pticks));
        }
    }

    for(ol = ban_list_ip;ol;ol=ol_tmp)
    {
        ol_tmp = ol->next;
        if(ol->objlink.ban->ticks_init != -1 &&  pticks >= ol->objlink.ban->ticks)
            remove_ban_entry(ol);
        else
        {
            fprintf(fp, "_ %s %d %d\n", ol->objlink.ban->ip?ol->objlink.ban->ip:"_", ol->objlink.ban->ticks_init,
                ol->objlink.ban->ticks_init==-1?-1:(int)(ol->objlink.ban->ticks-pticks));
        }
    }

    fclose(fp);
}

/* add a player or a ip to the ban list with a time
 * value in ticks (which means how long that entry get banned).
 * tick:-1 = perm. ban
 */
struct objectlink *add_ban_entry(char *banned, char *ip, int ticks, int ticks_left)
{
    objectlink *ol = get_ban_node();

    if(!banned && !ip)
        return NULL;

    ol->objlink.ban->ticks_init = ticks;
    ol->objlink.ban->ticks_left = ticks_left;
    ol->objlink.ban->ticks = pticks+ticks_left;
    if(ip)
        ol->objlink.ban->ip = strdup_local(ip);
    if(banned)
        FREE_AND_COPY_HASH(ol->objlink.ban->name, banned);

    LOG(-1,"Banning: %s (IP: %s) for %d seconds (%d sec left).\n", STRING_SAFE(ol->objlink.ban->name),
            STRING_SAFE(ip), ticks/8,ol->objlink.ban->ticks_init==-1?-1:(int)(ol->objlink.ban->ticks-pticks)/8);

    if(banned) /* add to name list */
        objectlink_link(&ban_list_player, NULL, NULL, ban_list_player, ol);
    else
        objectlink_link(&ban_list_ip, NULL, NULL, ban_list_ip, ol); /* IP list */

    return (struct objectlink *)ol;
}

/* remove a ban entry from the ban list.
 * triggered automatically from the ban system or
 * manual by /ban remove <text> command.
 */
void remove_ban_entry(struct oblnk *entry)
{
    if(entry->objlink.ban->ip)
        free(entry->objlink.ban->ip);
    if(entry->objlink.ban->name)
    {
        FREE_ONLY_HASH(entry->objlink.ban->name);
        objectlink_unlink(&ban_list_player, NULL, entry);
    }
    else
        objectlink_unlink(&ban_list_ip, NULL, entry);

    free_ban_node(entry);
}

/* check the player or IP is banned.
 * if name of player is NULL, check the IP.
 * Note: name is shared hash string
 */
int check_banned(NewSocket *ns, const char *name, char *ip)
{
    objectlink *ol, *ol_tmp;
    int   h;
    int   m;
    int   s;
    char  buf[256];

    if(name)
    {
        for(ol = ban_list_player;ol;ol=ol_tmp)
        {
            ol_tmp = ol->next;
            /* lets check the entry is still valid */
            /*LOG(-1,"CHECK-NAME: %s with %s (pticks: %d to %d)\n", name, STRING_SAFE(ol->objlink.ban->name),
              pticks, ol->objlink.ban->ticks);*/
            if(ol->objlink.ban->ticks_init != -1 &&  pticks >= ol->objlink.ban->ticks)
                remove_ban_entry(ol); /* is not valid anymore, gc it on the fly */
            else
            {
                if(ol->objlink.ban->name == name)
                {
                    /* because name banning is handled from login procedure,
                     * we tell here the client how long the name is banned - the
                     * rest is done by the login procedure.
                    */
                    char *ban_buf_name;
                    s = ol->objlink.ban->ticks_init/8;

                    if(ol->objlink.ban->ticks_init == -1) /* perm ban */
                    {
                        ban_buf_name = ban_buf_name_def;
                    }
                    else
                    {
                        if (s <= 90) /* we are nice for all under 90 seconds (1.5 minutes) */
                        {
                            sprintf(buf, "2 Name %s is blocked for %d seconds!\nDon't try to log in to it again!",name, s);
                            ban_buf_name = buf;
                        }
                        else
                        {
                            if (s < 60*60)
                            {
                                m = s/60;
                                sprintf(buf, "3 Name %s is banned for %d minutes!\nDon't try to log in to it again!",name, m);
                                ban_buf_name = buf;
                            }
                            else /* must be an ass... */
                            {
                                h = s/(60*60);
                                m = (s-h*(60*60))/60;
                                sprintf(buf, "3 Name %s is banned for %dh %dm!\nDon't try to log in to it again!",name,h,m);
                                ban_buf_name = buf;
                            }
                        }
                        Write_String_To_Socket(ns, BINARY_CMD_DRAWINFO, ban_buf_name, strlen(ban_buf_name));
                        player_addme_failed(ns, ADDME_MSG_BANNED);
                    }

                    /* someone is trying to login again & again to banned char? Lets teach him to avoid it */
                    if(++ns->pwd_try == 3)
                    {
                        char password_warning[] = "3 Don't login to banned chars!\nTry log in again not before 2 minutes!";

                        LOG(llevInfo,"BANNED NAME: 3 login tries (2min): IP %s (player: %s).\n",ns->ip_host,name);
                        add_ban_entry(NULL, ns->ip_host, 8*60*2, 8*60*2); /* 2 min temp ban for this ip */
                        Write_String_To_Socket(ns, BINARY_CMD_DRAWINFO, password_warning , strlen(password_warning));
                        player_addme_failed(ns, ADDME_MSG_DISCONNECT); /* tell client we failed and kick him away */
                        ns->login_count = ROUND_TAG+(uint32)(10.0f * pticks_second);
                        ns->status = Ns_Zombie; /* we hold the socket open for a *bit* */
                        ns->idle_flag = 1;
                    }
                    return TRUE;
                }
            }
        }
    }
    else /* compare ip */
    {
        char *ban_tmp;
        char *ban_buf_ip;
        int   match;    /* should really be bool */

        for(ol = ban_list_ip; ol; ol = ol_tmp)
        {
            ol_tmp = ol->next;
            LOG( -1,"CHECK-IP: >%s< with >%s< - pticks: %d left: %d (%d)\n",
              STRING_SAFE(ip), STRING_SAFE(ol->objlink.ban->ip),
              pticks, ol->objlink.ban->ticks,ol->objlink.ban->ticks_init);

            if(ol->objlink.ban->ticks_init != -1 &&  pticks >= ol->objlink.ban->ticks)
            {
                remove_ban_entry(ol); /* is not valid anymore, gc it on the fly */
            }
            else
            {
                s = ol->objlink.ban->ticks_init/8;
                ban_tmp = ol->objlink.ban->ip;

                match = ip_compare(ban_tmp,ip);

                if (match)
                {
                    /* match found - ip is banned */
                    break;      /* break outer loop */
                }
            }
        }

        if (match && ol)
        {
            /* IP is banned */
            if(ol->objlink.ban->ticks_init == -1) /* perm ban */
            {
                ban_buf_ip = ban_buf_ip_def;
            }
            else
            {
                if (s<=90) /* we are nice for all under 90 seconds (1.5 minutes). Thats most times technical tmp bans */
                {
                    sprintf(buf, "2 Login is blocked for %d seconds!\nDon't try to log in before.\nLogin timer reset to %d seconds!",s,s);
                    ban_buf_ip = buf;
                    s = ol->objlink.ban->ticks_init;

                    add_ban_entry(NULL, ns->ip_host, s, s);
                }
                else
                {
                    if (s < 60*60)
                    {
                        m = s/60;
                        sprintf(buf, "3 IP is banned for %d minutes!\nDon't try to log in before.\nLogin timer reset!",m);
                        ban_buf_ip = buf;
                        s = ol->objlink.ban->ticks_init;
                        remove_ban_entry(ol);
                        add_ban_entry(NULL, ns->ip_host, s, s);
                    }
                    else /* must be an ass... */
                    {
                        h = s/(60*60);
                        m = (s-h*(60*60))/60;
                        sprintf(buf, "3 IP is banned for %dh %dm!\nDon't try to log in before.\nAdding one hour ban time!",h,m);
                        ban_buf_ip = buf;
                        s = (ol->objlink.ban->ticks-pticks)+(60*60); /* added one hour */
                        if(s> 3*ol->objlink.ban->ticks_init)
                            s = -1; /* ban this guy now permanent */
                        else
                        {
                            remove_ban_entry(ol);
                            add_ban_entry(NULL, ns->ip_host, s, s);
                        }
                    }
                }
            }

            LOG(-1,"***BANNED IP Login: %s\n", ns->ip_host);
            Write_String_To_Socket(ns, BINARY_CMD_DRAWINFO, ban_buf_ip, strlen(ban_buf_ip));
            player_addme_failed(ns, ADDME_MSG_DISCONNECT); /* tell client something is wrong and we leave */
            ns->login_count = ROUND_TAG+(uint32)(5.0f * pticks_second);
            ns->status = Ns_Zombie; /* we hold the socket open for a *bit* */
            ns->idle_flag = 1;
            return TRUE; /* ip matches... kick our friend */
        }
    }

    return FALSE;
}
