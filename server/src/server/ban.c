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

/* returns a objectlink with ban_struct
 * we use for both the memorypool system
 */
static objectlink *get_ban_node(void)
{
	objectlink		*ol   = get_objectlink(OBJLNK_FLAG_BAN);
	struct ban_struct  *gptr = (struct ban_struct *) get_poolchunk(pool_bannode);

	memset(gptr, 0, sizeof(struct ban_struct));
	ol->objlink.ban = gptr;

	return ol;
}

/* free the gmaster node and the used objectlink
 */
static void free_ban_node(objectlink *ol)
{
	return_poolchunk(ol->objlink.ban, pool_bannode);
	return_poolchunk(ol, pool_objectlink);
}

/* load, parse and setup the ban file & system.
 */
void load_ban_file(void)
{
	FILE   *dmfile;
	int		ticks, ticks_left;
	char	mode;
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
        if (sscanf(line_buf, "%s %c %d %d", name, &mode, &ticks, &ticks_left) < 2)
            LOG(llevBug, "BUG: malformed banfile file entry: %s\n", line_buf);
		else
		{
			if(!name || name[0]=='\0' || (mode != 'i' && mode !='p'))
				LOG(llevBug, "BUG: malformed banfile file entry: %s\n", line_buf);
			else
				add_ban_entry(name, ticks, ticks_left, mode);
		}
	}
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
    fprintf(fp, "# entry format is '<tag> I|P ticks_init ticks_left'\n");
	
	for(ol = ban_list_player;ol;ol=ol_tmp)
	{
		ol_tmp = ol->next;
		if(ol->objlink.ban->ticks_init != -1 &&  pticks >= ol->objlink.ban->ticks)
			remove_ban_entry(ol); /* is not valid anymore, gc it on the fly */
		else 
		{
			fprintf(fp, "%s %c %d %d\n",ol->objlink.ban->tag,ol->objlink.ban->mode,
										ol->objlink.ban->ticks_init, 
										ol->objlink.ban->ticks_init==-1?-1:ol->objlink.ban->ticks-pticks);			
		}
	}

	for(ol = ban_list_ip;ol;ol=ol_tmp)
	{
		ol_tmp = ol->next;
		if(ol->objlink.ban->ticks_init != -1 &&  pticks >= ol->objlink.ban->ticks)
			remove_ban_entry(ol);
		else 
		{
			fprintf(fp, "%s %c %d %d\n",ol->objlink.ban->tag,ol->objlink.ban->mode,
				ol->objlink.ban->ticks_init, 
				ol->objlink.ban->ticks_init==-1?-1:ol->objlink.ban->ticks-pticks);			
		}
	}
	
    fclose(fp);
}

/* add a player or a ip to the ban list with a time
 * value in ticks (which means how long that entry get banned).
 * tick:-1 = perm. ban
 */
struct objectlink	*add_ban_entry(char *banned, int ticks, int ticks_left, int mode)
{
	objectlink *ol = get_ban_node();

	ol->objlink.ban->mode = mode;
	ol->objlink.ban->ticks_init = ticks; 
	ol->objlink.ban->ticks_left = ticks_left;
	ol->objlink.ban->ticks = pticks+ticks_left;
	strcpy(ol->objlink.ban->tag, banned);

	if(mode == 'p')
		objectlink_link(&ban_list_player, NULL, NULL, ban_list_player, ol);
	else
	{
		ol->objlink.ban->ip = inet_addr(banned); /* ip4 */
		objectlink_link(&ban_list_ip, NULL, NULL, ban_list_ip, ol);
	}
	
	return (struct objectlink *)ol;
}

/* remove a ban entry from the ban list.
 * triggered automatically from the ban system or
 * manual by /ban remove <text> command.
 */
void remove_ban_entry(struct oblnk *entry)
{
	if(entry->objlink.ban->mode == 'p')
		objectlink_unlink(&ban_list_player, NULL, entry);
	else
		objectlink_unlink(&ban_list_ip, NULL, entry);

	free_ban_node(entry);
}

/* check the player or IP is banned.
 * if name of player is NULL, check the IP.
 */
int check_banned(char *name, uint32 ip)
{
	objectlink *ol, *ol_tmp;

	if(name) 
	{
		for(ol = ban_list_player;ol;ol=ol_tmp)
		{
			ol_tmp = ol->next;
			/* lets check the entry is still valid */ 
			/*LOG(-1,"CHECK-IP: %s with %s (pticks: %d to %d)\n", name, ol->objlink.ban->tag,pticks, ol->objlink.ban->ticks);*/
			if(ol->objlink.ban->ticks_init != -1 &&  pticks >= ol->objlink.ban->ticks)
				remove_ban_entry(ol); /* is not valid anymore, gc it on the fly */
			else if(!strcasecmp(ol->objlink.ban->tag,name))
				return TRUE; /* ip matches... kick our friend */
		}
	}
	else /* compare ip */
	{
		for(ol = ban_list_ip;ol;ol=ol_tmp)
		{
			ol_tmp = ol->next;
/*			LOG(-1,"CHECK-IP: %s (%d)(%d)(%d) with %d - pticks: %d left: %d (%d)\n", ol->objlink.ban->tag,
				ol->objlink.ban->ip,ip,inet_addr("127.0.0.1"),pticks, ol->objlink.ban->ticks,ol->objlink.ban->ticks_init);*/
			if(ol->objlink.ban->ticks_init != -1 &&  pticks >= ol->objlink.ban->ticks)
				remove_ban_entry(ol); /* is not valid anymore, gc it on the fly */
			else if(ol->objlink.ban->ip == ip)
				return TRUE; /* ip matches... kick our friend */
		}
	}

	return FALSE;
}
