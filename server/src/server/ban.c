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

    The author can be reached via e-mail to info@daimonin.org
*/

#include "global.h"

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

/* New ban code.
 * entries are hold in memory and only loaded/saved between server
 * starts.
 * Code will work dynamic and use the memorypool system.
 * It will interact with temp. ban invoked by kick command or others.
 *
 * Whatever: most of this code should be part of a login server and not
 * of the game server itself. MT-2005
 */

static void ban_inform_client(NewSocket *ns, objectlink_t *ol, ENUM_BAN_TYPE ban_type, const char *account, const char *name, char *ip);

/* returns a objectlink with ban_t
 * we use for both the memorypool system
 */
static objectlink_t *get_ban_node(void)
{
    objectlink_t        *ol   = objectlink_get(OBJLNK_FLAG_BAN);
    struct ban_t  *gptr = (struct ban_t *) get_poolchunk(pool_bannode);

    memset(gptr, 0, sizeof(struct ban_t));
    ol->objlink.ban = gptr;

    return ol;
}

/* free the gmaster node and the used objectlink
 */
static void free_ban_node(objectlink_t *ol)
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
    char    line_buf[MEDIUM_BUF], name[MEDIUM_BUF], account[MEDIUM_BUF];

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
        if (sscanf(line_buf, "%s %s %s %d %d", account, name, ip, &ticks, &ticks_left) < 5)
        {
            // Could be an old-level file, with no account support
            if (sscanf(line_buf, "%s %s %d %d", name, ip, &ticks, &ticks_left) < 4)
                LOG(llevBug, "BUG: malformed banfile file entry: %s\n", line_buf);
            else
                add_ban_entry(NULL, !strcmp(name, "_") ? NULL : name, !strcmp(ip, "_") ? NULL : ip, ticks, ticks_left);
        }
        else
            add_ban_entry(!strcmp(account, "_") ? NULL : account, !strcmp(name, "_") ? NULL : name, !strcmp(ip, "_") ? NULL : ip, ticks, ticks_left);
    }

    fclose(dmfile);
}

/* save the valid ban_file entries.
 */
void save_ban_file(void)
{
    char    filename[MEDIUM_BUF];
    objectlink_t *ol, *ol_tmp;
    FILE   *fp;

    LOG(llevSystem,"write ban_file...\n");
    sprintf(filename, "%s/%s", settings.localdir, BANFILE);
    if ((fp = fopen(filename, "w")) == NULL)
    {
        LOG(llevBug, "BUG: Cannot open %s for writing\n", filename);
        return;
    }
    fprintf(fp, "# BAN_FILE (file is changed from server at runtime)\n");
    fprintf(fp, "# entry format is '<account> <name> <ip> <ticks_init> <ticks_left>'\n");
    fprintf(fp, "# or '<name> <ip> <ticks_init> <ticks_left>' (depreciated)\n");

    for(ol = ban_list_player;ol;ol=ol_tmp)
    {
        ol_tmp = ol->next;
        if(ol->objlink.ban->ticks_init != -1 &&  pticks >= ol->objlink.ban->ticks)
            remove_ban_entry(ol); /* is not valid anymore, gc it on the fly */
        else
        {
            fprintf(fp, "_ %s _ %d %d\n", ol->objlink.ban->name, ol->objlink.ban->ticks_init,
                                        ol->objlink.ban->ticks_init==-1?-1:(int)(ol->objlink.ban->ticks-pticks));
        }
    }

    for(ol = ban_list_account;ol;ol=ol_tmp)
    {
        ol_tmp = ol->next;
        if(ol->objlink.ban->ticks_init != -1 &&  pticks >= ol->objlink.ban->ticks)
            remove_ban_entry(ol); /* is not valid anymore, gc it on the fly */
        else
        {
            fprintf(fp, "%s _ _ %d %d\n", ol->objlink.ban->account, ol->objlink.ban->ticks_init,
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
            fprintf(fp, "_ _ %s %d %d\n", ol->objlink.ban->ip?ol->objlink.ban->ip:"_", ol->objlink.ban->ticks_init,
                ol->objlink.ban->ticks_init==-1?-1:(int)(ol->objlink.ban->ticks-pticks));
        }
    }

    fclose(fp);
}

/* add an account, player or IP to the ban list with a time
 * value in ticks (which means how long that entry get name).
 * tick:-1 = perm. ban
 * IMPORTANT NOTE - most ban code assumes an account, player name or IP exists
 * only ONCE in the ban lists; so don't ever add, without first doing a remove
 * to get rid of the previous entry
 */
struct objectlink_t *add_ban_entry(const char *account, const char *name, char *ip, int ticks, int ticks_left)
{
    objectlink_t *ol = get_ban_node();

    if(!account && !name && !ip)
        return NULL;

    ol->objlink.ban->ticks_init = ticks;
    ol->objlink.ban->ticks_left = ticks_left;
    ol->objlink.ban->ticks = pticks + ticks_left;
    if(ip)
        ol->objlink.ban->ip = strdup_local(ip);
    if(name)
    {
        SHSTR_FREE_AND_ADD_STRING(ol->objlink.ban->name, name);
    }
    if(account)
    {
        SHSTR_FREE_AND_ADD_STRING(ol->objlink.ban->account, account);
    }

    LOG(llevNoLog,"Banning: Account: %s / Player: %s / IP: %s for %d seconds (%d sec left).\n", STRING_SAFE(ol->objlink.ban->account),
        STRING_SAFE(ol->objlink.ban->name), STRING_SAFE(ip), ticks/8,ol->objlink.ban->ticks_init==-1?-1:(int)(ol->objlink.ban->ticks-pticks)/8);

    if(name) /* add to name list */
        objectlink_link(&ban_list_player, NULL, NULL, ban_list_player, ol);
    else if(account) /* add to account list */
        objectlink_link(&ban_list_account, NULL, NULL, ban_list_account, ol);
    else /* IP list */
        objectlink_link(&ban_list_ip, NULL, NULL, ban_list_ip, ol);

    return (struct objectlink_t *)ol;
}

/* remove a ban entry from the ban list.
 * triggered automatically from the ban system or
 * manual by /ban remove <text> command.
 */
void remove_ban_entry(struct objectlink_t *entry)
{
    if(entry->objlink.ban->ip)
        free(entry->objlink.ban->ip); // TODO - can't this bit of code go below in the last 'else' ??
    if(entry->objlink.ban->name)
    {
        SHSTR_FREE(entry->objlink.ban->name);
        objectlink_unlink(&ban_list_player, NULL, entry);
    }
    else if(entry->objlink.ban->account)
    {
        SHSTR_FREE(entry->objlink.ban->account);
        objectlink_unlink(&ban_list_account, NULL, entry);
    }
    else
        objectlink_unlink(&ban_list_ip, NULL, entry);

    free_ban_node(entry);
}

/* Check if the account, player name or IP is banned.
 * Note: accout and name are both shared hash strings */
int check_banned(NewSocket *ns, const char *account, const char *name, char *ip)
{
    objectlink_t *ol;

    if(name)
    {
        for(ol = ban_list_player;ol;ol=ol->next)
        {
            /* lets check the entry is still valid */
            if(ol->objlink.ban->ticks_init != -1 &&  pticks >= ol->objlink.ban->ticks)
                remove_ban_entry(ol); /* is not valid anymore, gc it on the fly */
            else
                if(ol->objlink.ban->name == name)
                {
                    ban_inform_client(ns, ol, BANTYPE_CHAR, account, name, ip);
                    return 1;
                }
        }
    }

    if(account)
    {
        for(ol = ban_list_account;ol;ol=ol->next)
        {
            /* lets check the entry is still valid */
            if(ol->objlink.ban->ticks_init != -1 &&  pticks >= ol->objlink.ban->ticks)
                remove_ban_entry(ol); /* is not valid anymore, gc it on the fly */
            else
                if(ol->objlink.ban->account == account)
                {
                    ban_inform_client(ns, ol, BANTYPE_ACCOUNT, account, name, ip);
                    return 1;
                }
        }
    }

    if (ip)
    {
        for(ol = ban_list_ip; ol; ol = ol->next)
        {
            if(ol->objlink.ban->ticks_init != -1 &&  pticks >= ol->objlink.ban->ticks)
                remove_ban_entry(ol); /* is not valid anymore, gc it on the fly */
            else
                if(ip_compare(ol->objlink.ban->ip,ip))
                {
                    ban_inform_client(ns, ol, BANTYPE_IP, account, name, ip);
                    return 1;
                }
        }
    }

    return 0;
}

static void ban_inform_client(NewSocket *ns, objectlink_t *ol, ENUM_BAN_TYPE ban_type, const char *account, const char *name, char *ip)
{
    /* because name banning is handled from login procedure,
     * we tell here the client how long the name is banned - the
     * rest is done by the login procedure.
    */
    int  h, m, s;
    char buf[MEDIUM_BUF] = "";

    s = (ol->objlink.ban->ticks - pticks) / 8;

    if(ol->objlink.ban->ticks_init == -1) /* perm ban */
    {
        if (ban_type == BANTYPE_ACCOUNT)
            sprintf(buf, "3 You are banned from Daimonin.\nGoodbye.");
        else if (ban_type == BANTYPE_CHAR)
            sprintf(buf, "3 Your character, %s, is banned from Daimonin.\nGoodbye.", name);
        else // ban_type = BANTYPE_IP
            sprintf(buf, "3 Your IP is banned from Daimonin.\nGoodbye.");
    }

    else if (s <= 90)
        sprintf(buf, "2 Login is banned for %d seconds!\nDon't try to log in before this!", s);

    else if (s < 60*60)
        sprintf(buf, "2 Login is banned for %d minutes!\nDon't try to log in before this!", s/60);

    else
    {
        h = s/(60*60);
        m = (s-h*(60*60))/60;

        sprintf(buf, "2 Login is blocked for %dh %dm!\nDon't try to log in before this!", h, m);
    }

    Write_String_To_Socket(ns, SERVER_CMD_DRAWINFO, buf, strlen(buf));
    player_addme_failed(ns, ADDME_MSG_BANNED);

    /* someone is trying to login again & again to banned account/char/ip?
     * Lets teach them to avoid it */
    if(++ns->pwd_try == 3)
    {
        LOG(llevInfo,"BANNED LOGIN: 3 login tries: Account: %s / Player: %s / IP: %s\n",
            STRING_SAFE(ol->objlink.ban->account),
            STRING_SAFE(ol->objlink.ban->name),
            STRING_SAFE(ns->ip_host));

        s = ol->objlink.ban->ticks_init;
        if(ol->objlink.ban->ticks_init == -1) /* perm ban */
            sprintf(buf, "3 Don't try to login when you are banned!");
        else
        {
            sprintf(buf, "3 Don't try to login when you are banned!\nYour ban is doubled!");

            s *= 2;

            remove_ban_entry(ol);

            if (ban_type == BANTYPE_IP)
                add_ban_entry(NULL, NULL, ip, s, s);
            else if (ban_type == BANTYPE_ACCOUNT)
                add_ban_entry(account, NULL, 0, s, s);
            else if (ban_type == BANTYPE_CHAR)
                add_ban_entry(NULL, name, 0, s, s);
        }

        Write_String_To_Socket(ns, SERVER_CMD_DRAWINFO, buf , strlen(buf));
        player_addme_failed(ns, ADDME_MSG_DISCONNECT); /* tell client we failed and kick him away */
        ns->inactive_when = ROUND_TAG + INACTIVE_ZOMBIE * pticks_second;
        ns->status = Ns_Zombie; /* we hold the socket open for a *bit* */
        ns->inactive_flag = 1;
    }
}
