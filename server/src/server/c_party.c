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
#include <stdarg.h>
#include <global.h>

/* Invite <name> to a group.
 * Command can only be used from a group leader
 * or someone without a group.
 * Invite command is send to the player <name> if
 * a.) group_mode allows inviting
 * b.) player is not in a group
 * c.) has no unanswered invite pending
 */
int command_party_invite ( object *pl, char *params)
{
    player *target, *activator = CONTR(pl);

    if(!activator)
        return 0;

    if (!params)
    {
        new_draw_info(NDI_UNIQUE, 0,pl, "Usage: /invite <player name>");
        return 0;
    }


    if(!(target = find_player(params)))
    {
        /* ok - its not a player we want invite.
         * now check for keywords like "on", "off" or "allow <name>".
         */
        if(!strcasecmp(params,"on"))
        {
            new_draw_info(NDI_UNIQUE, 0,pl, "Group: /invite unlocked.");
            /* important! to avoid side effect, a group mode change
             * will delete a pending invite!
            */
            if(activator->group_status & GROUP_STATUS_INVITE)
                command_party_deny (pl, NULL); /* automatic /deny */
            activator->group_mode = GROUP_MODE_JOIN;
            return 1;
        }

        if(!strcasecmp(params,"off"))
        {
            new_draw_info(NDI_UNIQUE, 0,pl, "Group: /invite locked.");
            if(activator->group_status & GROUP_STATUS_INVITE)
                command_party_deny (pl, NULL); /* automatic /deny */
            activator->group_mode = GROUP_MODE_DENY;
            return 1;
        }
        if(!strncasecmp(params,"allow ",6))
        {
            params = strchr(params,' ')+1; /* we KNOW there is a ' ' - that ptr+1 is start of name or '/0'*/
            if(find_player(params))
            {
                if(activator->group_status & GROUP_STATUS_INVITE)
                    command_party_deny (pl, NULL); /* automatic /deny */
                activator->group_mode = GROUP_MODE_INVITE;
                strcpy(activator->group_invite_name, params);
                new_draw_info_format(NDI_UNIQUE, 0,pl, "Group: /invite enabled for player %s.", STRING_SAFE(params));
            }
        }

        new_draw_info_format(NDI_UNIQUE, 0,pl, "/invite %s: offline or unknown player.", params);
        return 0;
    }

    if(target->ob == pl) /* silly idiot... we DON'T waste here a message */
        return 0;

    /* when the /invite sender is in a group ... */
    if(activator->group_status & GROUP_STATUS_GROUP)
    {
        /* only allow the invite when he is the leader */
        if(activator->group_leader != activator->ob)
        {
            /* can be handled client sided */
            new_draw_info(NDI_UNIQUE, 0,pl, "/invite: you are not the group leader.");
            return 0;
        }

        /* don't invite when your group is full */
        if(CONTR(activator->group_leader)->group_nrof == GROUP_MAX_MEMBER)
        {
            /* can be handled client sided */
            new_draw_info(NDI_UNIQUE, 0,pl, "/invite: the group is full.");
            return 0;
        }
    }

    /* target has pending /invite ? */
    if(target->group_status & GROUP_STATUS_INVITE)
    {
        /* we want avoid /invite spaming - so we don't give much information here */
        new_draw_info_format(NDI_UNIQUE, 0,pl, "/invite: %s has pending invite request.", query_name(target->ob));
        return 0;
    }

    /* target can be invited? */
    if(target->group_id != GROUP_NO) /* player has a group - GROUP_STATUS_GROUP should work to*/
    {
        new_draw_info_format(NDI_UNIQUE, 0,pl, "/invite: %s is in another group.", query_name(target->ob));
        return 0;
    }

    /* target allows /invite to him? */
    if(target->group_mode == GROUP_MODE_DENY ||
        (target->group_mode == GROUP_MODE_INVITE && strcmp(pl->name, target->group_invite_name)))
    {
        new_draw_info_format(NDI_UNIQUE, 0,pl, "/invite: %s don't allow invite.", query_name(target->ob));
        return 0;
    }

    /* ok... we can invite this guy. send the /invite to him and setup the invite */
    /* remember who has given the invite request so player can give a simple /join or /deny */
    target->group_status = GROUP_STATUS_INVITE;
    if(target->group_mode != GROUP_MODE_INVITE)
        strcpy(target->group_invite_name, pl->name); /* tricky copy... */
    target->group_leader = pl;
    target->group_leader_count = pl->count;

    /* send the /invite to our player */
    SOCKET_SET_BINARY_CMD(&global_sl, BINARY_CMD_INVITE);
    strcpy(global_sl.buf+global_sl.len, pl->name);
    global_sl.len += strlen(pl->name)+1;
    Send_With_Handling(&target->socket, &global_sl);

    new_draw_info_format(NDI_YELLOW, 0,pl, "You invited %s to join the group.", query_name(target->ob));

    return 1;
}

/* positive answer to /invite
 * player pl tries to joins the group which has given invite
 * this *can* fail when the group is gone, invite give has
 * left or the group is full now
 */
int command_party_join ( object *pl, char *params)
{
    player *target, *activator = CONTR(pl);

    if(!activator)
        return 0;

    /* /join command without /invite - message handled client sided */
    if(!(activator->group_status & GROUP_STATUS_INVITE))
        return 0;

    /* easy way to find source player for /invite */
    if(activator->group_leader && activator->group_leader->count == activator->group_leader_count)
        target = CONTR(activator->group_leader);
    else if(!(target = find_player(activator->group_invite_name))) /* hard way */
    {
        new_draw_info_format(NDI_UNIQUE, 0,pl, "/join: %s is offline.", activator->group_invite_name);
        return 0;
    }

    /* now check: if target has invited you, but target joined in the time another
     * group, this invite should fail.
     */
    if(target->group_status & GROUP_STATUS_GROUP && target->group_leader != target->ob)
        new_draw_info_format(NDI_UNIQUE, 0,pl, "/join: %s has joined another group.", query_name(target->ob));

    /* target is the group leader. activator the member who wants join */
    party_add_member(target, activator);
    return 1;
}

/* negative answer to /invite.
 * Its important that the inviting player don't get a message
 * when the other denyed the invite - that will make abusing of
 * invite alot less funny.
 */
int command_party_deny ( object *pl, char *params)
{
    player *activator = CONTR(pl);

    if(!activator)
        return 0;

    /* /deny command without /invite - message handled client sided */
    if(!(activator->group_status & GROUP_STATUS_INVITE))
        return 0;

    /* message is redundant because the invite window will vanish as signal */
    /* new_draw_info_format(NDI_UNIQUE, 0,pl, "You denied the invite."); */
    activator->group_status = GROUP_STATUS_FREE; /* simple action */
    return 1;
}

/* remove yourself from a group.
 */
int command_party_leave ( object *pl, char *params)
{
    player *activator = CONTR(pl);

    if(!activator)
        return 0;

    /* /leave command without group - message handled client sided */
    if(!(activator->group_status & GROUP_STATUS_GROUP))
        return 0;

    party_message(0,NDI_YELLOW, 0, activator->group_leader, pl, "%s left the group.", query_name(pl));
    new_draw_info(NDI_YELLOW, 0,pl, "You left the group.");
    party_remove_member(CONTR(pl), FALSE);

    return 1;
}

/* Group leader only.
 * remove (kick) a group member.
 */
int command_party_remove ( object *pl, char *params)
{
    player *activator = CONTR(pl);
    player *target;

    if (!params)
    {
        new_draw_info(NDI_UNIQUE, 0,pl, "Usage: /remove <player name>");
        return 0;
    }

    /* allow only group leader to use this command */
    if(!(activator->group_status & GROUP_STATUS_GROUP) || activator->group_leader != activator->ob)
    {
        /* message can be handled client sided */
        return 0;
    }

    target = find_player(params);
    /* player unknown or not in our group? */
    if(!target || !(target->group_status & GROUP_STATUS_GROUP) || activator->group_id != target->group_id )
    {
        new_draw_info_format(NDI_YELLOW, 0, pl, "/remove: %s is not in your group.", STRING_SAFE(params));
        return 0;
    }
    party_message(0,NDI_YELLOW, 0, pl, target->ob, "%s was removed from group.", query_name(target->ob));
    new_draw_info(NDI_YELLOW, 0, target->ob, "You was removed from group.");
    party_remove_member(target, FALSE);

    return 1;
}

/* Adding & remove members to a group.
 * These functions don't have any error handling.
 * Its not needed when they are called right but don't
 * manipulate the group chain from any other source as here!
 */

/* add a player to a group */
void party_add_member(player *leader, player *member)
{
    object *tmp;
    int i;

    /* don't allow more as GROUP_MAX_MEMBER people in a group */
    if(leader->group_nrof == GROUP_MAX_MEMBER)
    {
        new_draw_info(NDI_YELLOW, 0, member->ob, "Group is full.");
        return;
    }

    if(!(tmp=leader->group_next)) /* new group! */
    {
        leader->group_status = GROUP_STATUS_GROUP;
        leader->group_id = global_group_tag++; /* unique group id - it also tells how many groups has been build */
        leader->group_next = member->ob;
        leader->group_leader = leader->ob;
        leader->group_nrof = 2; /* only leader has nrof counter */

        member->group_prev = leader->ob;
        member->group_next = NULL;
    }
    else /* existing group, attach our player */
    {
        /* get last valid member of the group */
        for(;CONTR(tmp)->group_next;tmp=CONTR(tmp)->group_next)
            ;

        CONTR(tmp)->group_next = member->ob;
        member->group_prev = tmp;
        member->group_next = NULL;
        leader->group_nrof++;
    }

    member->group_status = GROUP_STATUS_GROUP;
    member->group_id = leader->group_id;
    member->group_leader = leader->ob;

    for(i=0,tmp=leader->ob;tmp;tmp=CONTR(tmp)->group_next,i++)
        CONTR(tmp)->group_nr=i;

    party_message(0,NDI_YELLOW, 0,leader->ob, member->ob, "%s joined the group.", query_name(member->ob));
    if( leader->group_nrof == 2)
        new_draw_info(NDI_YELLOW, 0, leader->ob, "Use /gsay for group speak or /help group for help.");
    new_draw_info(NDI_YELLOW, 0, member->ob, "You joined the group.");
    new_draw_info(NDI_YELLOW, 0, member->ob, "Use /gsay for group speak or /help group for help.");
    party_client_group_status(member->ob);

}

/* help function to clear group data */
static inline void party_clear_links(player *pl)
{
    pl->group_status = GROUP_STATUS_FREE;
    pl->group_id = GROUP_NO;
    pl->group_leader = NULL;
    pl->group_next = NULL;
    pl->group_prev = NULL;
}

/* remove a player from a group */
void party_remove_member(player *member, int flag)
{
    object *tmp, *leader;
    int i;

    /* 3 things can happen:
     * a.) member just leaves - unlink from rest of group
     * b.) member group has 2 members left - disband group.
     * c.) member is leader, unlink and make next member to leader.
     */

    /* hm, that should not happen ! */
    if(CONTR(member->group_leader)->group_nrof <= 1)
    {
        LOG(llevBug,"BUG: party_remove_member() called for group_nrof <=1 (%s) (%d)\n", query_name(member->ob), member->group_nrof);
        party_clear_links(member);
        return;
    }

    if(flag)
    {
        party_message(0,NDI_YELLOW, 0, member->group_leader , member->ob, "%s left the group.", query_name(member->ob));
        new_draw_info(NDI_YELLOW, 0, member->ob, "You left the group.");
    }

    /* if only 1 member in the group is left - destruct the whole group! */
    if(CONTR(member->group_leader)->group_nrof == 2)
    {
        party_client_group_kill(member->group_leader);
        party_client_group_kill((tmp=CONTR(member->group_leader)->group_next));

        party_clear_links(CONTR(member->group_leader));
        party_clear_links(CONTR(tmp));

        return;

    }

    /* group will stay but we must remove member */
    if(member->group_leader == member->ob) /* we are leader */
    {
        CONTR(member->group_next)->group_prev = NULL; /* unlink the leader */

        for(tmp=member->group_next;tmp;tmp=CONTR(tmp)->group_next)
            CONTR(tmp)->group_leader = member->group_next;

        leader = member->group_next;
    }
    else /* we are member 2+ */
    {
        /* MUST be legal because we are not first member */
        CONTR(member->group_prev)->group_next = member->group_next;
        if(member->group_next)
            CONTR(member->group_next)->group_prev = member->group_prev;

        leader = member->group_leader;
    }

    /* adjust slot numbers for all group members */
    for(i=0,tmp=leader;tmp;tmp=CONTR(tmp)->group_next,i++)
        CONTR(tmp)->group_nr=i;
    CONTR(leader)->group_nrof = i;

    /* shutdown the group interface for member and clear all his links */
    party_client_group_kill(member->ob);
    party_clear_links(member);

    party_client_group_status(leader);
}

/* send a message to every group member of group of leader.
 * except member source
 */
void party_message(int mode, int flags, int pri,object *leader, object *source, char *format, ...)
{
    object *tmp;
    char buf[HUGE_BUF];

    va_list ap;
    va_start(ap, format);
    vsprintf(buf, format, ap);
    va_end(ap);

    for(tmp=CONTR(leader)->group_leader;tmp;tmp=CONTR(tmp)->group_next)
    {
        if(tmp != source && (!(mode&PMSG_MODE_NOEXP) || !(CONTR(tmp)->group_status &GROUP_STATUS_NOEXP)) )
            new_draw_info(flags, pri, tmp, buf);
    }
}

/* These group of function are used to communicate with the graphical
 * interface of the client to show & update the group information.
 */

/* give the client of ALL group members the command "you are in a group, here are the data".
 * The client can create out of this the group status window.
 * This is used for start a group but also for changes.
 * This is normally only called once when a member is added or one left.
 */
void party_client_group_status(object *member)
{
    object *tmp;
    char buf[HUGE_BUF]= "";
    char buf2[HUGE_BUF];

    /* create group status data - change to binary after testing*/
    for(tmp=CONTR(member)->group_leader;tmp;tmp=CONTR(tmp)->group_next)
    {
        sprintf(buf2,"|%s %d %d %d %d %d %d %d", tmp->name, tmp->stats.hp,tmp->stats.maxhp,
            tmp->stats.sp,tmp->stats.maxsp, tmp->stats.grace,tmp->stats.maxgrace, tmp->level);
        strcat(buf, buf2);
    }

    SOCKET_SET_BINARY_CMD(&global_sl, BINARY_CMD_GROUP);
    strcpy(global_sl.buf+global_sl.len, buf);
    global_sl.len += strlen(buf)+1;

    /* broadcast command to all members */
    for(tmp=CONTR(member)->group_leader;tmp;tmp=CONTR(tmp)->group_next)
        Send_With_Handling(&CONTR(tmp)->socket, &global_sl);
}

/* tell a member that he has no group! */
void party_client_group_kill(object *member)
{
    /* we use gruop init with zero players as "no group" marker */
    SOCKET_SET_BINARY_CMD(&global_sl, BINARY_CMD_GROUP);
    Send_With_Handling(&CONTR(member)->socket, &global_sl);
}

/* TODO: optimize update handling
 * Search for party_client_group_update() in source.
 * It can be optimized by adding a tag and a small loop so we
 * check/update ALL group members at once when one gets triggered.
 * Then we generate one update
 */

/* update a member data for all group members */
void party_client_group_update(object *member, int flag)
{
    object *tmp;
    player *pl, *plm;
    char buf2[HUGE_BUF];
    char buf[HUGE_BUF];

    /* TODO: change to binary data/cmd after testing using GROUP_UPDATE_xxx */

	plm = CONTR(member);
#ifdef DEBUG_GROUP_UPDATE
	LOG(-1,"GROUP UPDATE: %s (gid: %d) (leader: %s) --> ", query_name(member), plm->group_nr,query_name(plm->group_leader));
#endif
    sprintf(buf2,"|%d %d %d %d %d %d %d %d\n",plm->group_nr,
        member->stats.hp, member->stats.maxhp,
        member->stats.sp, member->stats.maxsp,
        member->stats.grace, member->stats.maxgrace, member->level);

#ifdef DEBUG_GROUP_UPDATE
	LOG(-1,"gstats: %s\n", buf2);
#endif

    strcpy(buf,buf2);
    plm->update_ticker = ROUND_TAG;

    for(tmp=plm->group_leader;tmp;tmp=pl->group_next)
    {
#ifdef DEBUG_GROUP_UPDATE
		LOG(-1,"GROUP UPDATE: %s (gid: %d) (leader: %s)::\n", query_name(tmp), CONTR(tmp)->group_nr,query_name(CONTR(tmp)->group_leader));
#endif
        if((pl = CONTR(tmp))->update_ticker != ROUND_TAG)
        {
            /* TODO: use GROUP_UPDATE_xxx for a binary cmd which really holds
             * only the different data!
             * ATM we transer alot redundant data.
             * (but the "tricky update" thingy works usinf update_ticker)
             */
            if( pl->last_stats.hp != pl->ob->stats.hp ||
                pl->last_stats.maxhp != pl->ob->stats.maxhp ||
                pl->last_stats.sp != pl->ob->stats.sp ||
                pl->last_stats.maxsp != pl->ob->stats.maxsp ||
                pl->last_stats.grace != pl->ob->stats.grace ||
                pl->last_stats.maxgrace != pl->ob->stats.maxgrace ||
                pl->last_level != pl->ob->level)
            {
                sprintf(buf2,"|%d %d %d %d %d %d %d %d\n", pl->group_nr,
                    tmp->stats.hp, tmp->stats.maxhp,
                    tmp->stats.sp, tmp->stats.maxsp,
                    tmp->stats.grace, tmp->stats.maxgrace,tmp->level);

#ifdef DEBUG_GROUP_UPDATE
				LOG(-1,"gstats: %s\n", buf2);
#endif
                strcat(buf,buf2);
                pl->update_ticker = ROUND_TAG;
            }
        }
    }

    SOCKET_SET_BINARY_CMD(&global_sl, BINARY_CMD_GROUP_UPDATE);
    /*Send_With_Handling(&CONTR(member)->socket, &global_sl);*/

    strcpy(global_sl.buf+global_sl.len, buf);
    global_sl.len += strlen(buf)+1;

    /* broadcast command to all members */
    for(tmp=plm->group_leader;tmp;tmp=CONTR(tmp)->group_next)
        Send_With_Handling(&CONTR(tmp)->socket, &global_sl);
}
