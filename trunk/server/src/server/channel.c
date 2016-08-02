 /*
    Daimonin SDL client, a client program for the Daimonin MMORPG.


  Copyright (C) 2003-2006 Michael Toennies

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

//#ifdef USE_CHANNELS


struct channels *channel_list_start = NULL;

/** The Main Function
 */
int command_channel(object_t *ob, char *params)
{
    char    channelname[MAX_CHANNEL_NAME+1];
    char    mode;
    int     channelnamelen;
    struct  channels        *channel=NULL;
    struct  player_channel  *pl_channel=NULL;
    struct  player_channel  *cpl=NULL;

    if (!params)
    {
        return 1;
    }

    /* Command Parser */
    if ((channelnamelen = strcspn(params, "-+ :?*!%$")) > MAX_CHANNEL_NAME)
    {
        ndi(NDI_UNIQUE, 0, ob, "Channelname is too long!");

        return 0;
    }

    channelname[0]=0;
    strncpy(channelname,params,channelnamelen);
    channelname[channelnamelen]=0;
    mode=params[channelnamelen];
    params=params+channelnamelen+1;   /* wir verbiegen den Zeigen, für den Rest */

    if (strlen(params)>210)
    {
        ndi(NDI_UNIQUE, 0, ob, "Message too long!");

        return 0;
    }
    /* end command-parser */

//    LOG(llevDebug, "channel_parse: name: %s, mode: %c, params: %s\n",channelname, mode, params);

    if (channelnamelen==0)              /* Handle the global functions */
    {
        if ((mode=='-') && (params[0]==0)) /* temp-on-off without leaving */
        {
            CONTR(ob)->channels_on=0;
            ndi(NDI_UNIQUE, 0, ob, "You have all Channels temporally disabled");
            return 0;
        }
        if ((mode=='+') && (params[0]==0)) /* temp-on-off without leaving */
        {
            CONTR(ob)->channels_on=1;
            ndi(NDI_UNIQUE, 0, ob, "You listen to all your channels again.");
            return 0;
        }
        if ((mode=='?') && (params[0]==0)) /* List all channels */
        {
            /* atm its only a quick listing, TODO: maybe descriptions for channels? */
            ndi(NDI_UNIQUE, 0, ob, "These channels exist:");

            for (channel=channel_list_start;channel;channel=channel->next)
            {
                /* We don't display channels we can't get on.
                 * This may be by level restriction ot gmaster_mode.
                 * VOLs, GMs, and SAs are not subject to level restrictions. */
                if (!compare_gmaster_mode(channel->gmaster_mode,
                                          CONTR(ob)->gmaster_mode) ||
/* Method stub for later to implement clan system
 * This function should return 1 if the player is in that clan.
 * channel->clan will be some sort of pointer to the clan info...
 */
//                  !is_player_in_clan(channel->clan) ||
                    (!(CONTR(ob)->gmaster_mode & (GMASTER_MODE_SA | GMASTER_MODE_GM | GMASTER_MODE_VOL)) &&
                     ob->level < channel->enter_lvl))
                        continue;
                if((pl_channel=findPlayerChannelFromName(CONTR(ob),CONTR(ob), channel->name, 1)))
                    ndi(NDI_UNIQUE|channel->color, 0, ob, "*(%d) [%c] %s", channel->pl_count, pl_channel->shortcut, channel->name);
                else
                    ndi(NDI_UNIQUE|channel->color, 0, ob, "   (%d) [%c] %s", channel->pl_count, channel->shortcut, channel->name);
            }
            return 0;
        }
        /* no global command...*/
        return 1;
    }

    /* lets handle all the channel-commands */
    if (mode=='+')  /* join channel */
    {
        addPlayerToChannel(CONTR(ob),channelname,params);
        return 0;
    }

    /* for all channelfuntions except joining we need the pointer of the desired channel in player's channel-list*/
    pl_channel=getPlChannelFromPlayerShortcut(CONTR(ob),channelname); /* first we try to get the channel over the players shortcut */
    if (!pl_channel)
    {   /* All the error messages are handled here in this function, should eventually splittet */
        pl_channel=findPlayerChannelFromName(CONTR(ob),CONTR(ob), channelname, 0);
    }
    if (!pl_channel)
    {
        return 1;
    }
//    LOG(llevDebug, "ch-sys: we have a channel: %s\n",pl_channel->channel->name);

    /* we have a channel                    */
    /* lets get on with all the other stuff */
    if (mode=='-') /* leave a channel */
    {
        char buf[MEDIUM_BUF];
        sprintf(buf, "You leave channel %s", pl_channel->channel->name);
        removeChannelFromPlayer(CONTR(ob), pl_channel, buf);
        return 0;
    }
    else if (mode=='?') /* list players on channel */
    {
        ndi(NDI_UNIQUE, 0, ob, "Listening on this channel:");

        /* ATM unfortunatly we can only send one player at a line,
         * cause clients append a newline to ndi's
         * In the future if we have 1000 Players or more, we should think about some limitation or disabling
         */
        for (cpl=pl_channel->channel->players;cpl;cpl=cpl->next_player)
        {
            if (!cpl->pl->privacy)
                ndi(NDI_UNIQUE, 0, ob, "%s",cpl->pl->ob->name);
        }
        return 0;
    }
    else if (mode==' ') /* normal channelmessage */
    {
        /* Of course we have to check, if player really wants to say somethinig */
        if (strlen(params)==0)
        {
            return 1;
        }
        /* Check for lvl-post restrictions. VOLs, GMs, and SAs can always post. */
        if (ob->level<pl_channel->channel->post_lvl &&
            !(CONTR(ob)->gmaster_mode & (GMASTER_MODE_SA | GMASTER_MODE_GM | GMASTER_MODE_VOL)))
        {
            ndi(NDI_UNIQUE, 0, ob, "You need at least level %d to post on this channel.",pl_channel->channel->post_lvl);
            return 0;
        }
        if (check_channel_mute(pl_channel))
            sendChannelMessage(CONTR(ob),pl_channel->channel, params);
        return 0;
    }
    else if (mode==':') /* emoted channelmessage */
    {
        if (strlen(params)==0)
        {
            return 1;
        }
        /* Check for lvl-post restrictions. VOLs, GMs, and SAs can always post. */
        if (ob->level<pl_channel->channel->post_lvl &&
            !(CONTR(ob)->gmaster_mode & (GMASTER_MODE_SA | GMASTER_MODE_GM | GMASTER_MODE_VOL)))
        {
            ndi(NDI_UNIQUE, 0, ob, "You need at least level %d to post on this channel.",pl_channel->channel->post_lvl);
            return 0;
        }
        if (check_channel_mute(pl_channel))
            sendChannelEmote(CONTR(ob), pl_channel->channel, params);
        return 0;
    }
    else if (mode=='*')
    {
#ifdef CHANNEL_HIST
        int lines=0;

        lines=atoi(params);
        if (lines<=0)
            lines=5; /*max 5 lines if he is not specific */
        if ((lines>10) &&
            !(CONTR(ob)->gmaster_mode & (GMASTER_MODE_SA | GMASTER_MODE_GM | GMASTER_MODE_VOL)))
            lines=10; /*players have 10 lines limit */

        sendChannelHist(pl_channel,lines);

        return 0;
#else
        return 1;
#endif
    }
    else if (mode=='!' && (CONTR(ob)->gmaster_mode & GMASTER_MODE_VOL))
    {
        if (strlen(params)==0)
        {
            /* we don't have server side colors anymore */
//            ndi(NDI_UNIQUE, 0, ob, "Colorsyntax: -<channelname>!color <1-210>");
            return 1;
        }
//        if (!strncasecmp(params,"color",5))
//        {
//            int color;
//            if (sscanf(params,"color %d",&color)==EOF)
//                ndi(NDI_UNIQUE, 0, ob, "Colorsyntax: -<channelname>!color <1-210>");
//            else
//            {
//                if ((color>210) || (color <1))
//                    ndi(NDI_UNIQUE, 0, ob, "Colorsyntax: -<channelname>!color <1-210>");
//                else
//                    pl_channel->color=color-1;
//            }
//        }
        /* we simply map the !mute-command to the command_channel_mute*/
        else if(!strncasecmp(params,"mute",4))
        {
            char    buf3[256];
            sprintf(buf3,"%s %s",pl_channel->channel->name,params+5);

            return command_channel_mute(ob, buf3);
        }
        else if (!strncasecmp(params, "mod", 3))
        {
            modify_channel_params(pl_channel,params+4);
        }
        else if (!strncasecmp(params, "add", 3))
        {
            forceAddPlayerToChannel(pl_channel, params+4);
        }
        else if (!strncasecmp(params, "kick", 4))
        {
            kickPlayerFromChannel(pl_channel, params+5);
        }

        return 0;
    }
    else if (mode=='%')   /* change player's shortcut */
    {
        if (strlen(params)==0)
        {
            ndi(NDI_UNIQUE, 0, ob, "Current shortcut for channel %s is %c.",pl_channel->channel->name, pl_channel->shortcut);
            return 0;
        }
        if (strlen(params)>1)
        {
            ndi(NDI_UNIQUE, 0, ob, "Shortcut can only be one char or number!");
            return 0;
        }
        if ((params[0]=='+') || (params[0]=='-') || (params[0]==' ') || (params[0]=='*') || (params[0]=='%') || (params[0]=='!') || (params[0]==':') || (params[0]=='#') || (params[0]=='$'))
        {
            ndi(NDI_UNIQUE, 0, ob, "This Shortcut can't be used!");
            return 0;
        }
        pl_channel->shortcut=params[0];
        ndi(NDI_UNIQUE, 0, ob, "Shortcut for channel %s changed to %c.",pl_channel->channel->name,pl_channel->shortcut);
        return 0;
    }
    else if (mode == '$')
    {
        char buf[HUGE_BUF];

        object_t *targetob = CONTR(ob)->mark;

        if (!targetob) // Don't do anything if there is no marked item.
        {
            ndi(NDI_UNIQUE, 0, ob, "You must ~M~ark an item to describe it.");
            return 0;
        }

        if (!QUERY_FLAG(targetob, FLAG_IDENTIFIED))
        {
            ndi(NDI_UNIQUE, 0, ob, "Unidentified items cannot be described.");
            return 0;
        }

        sprintf(buf, "%s %s (examine worth: %s)",
            query_name(targetob, ob, targetob->nrof, 1),
            describe_item(targetob),
            cost_string_from_value(targetob->value * targetob->nrof, COSTSTRING_SHORT));

        if (check_channel_mute(pl_channel))
        {
            sendChannelMessage(CONTR(ob),pl_channel->channel, buf);
        }
    }

    return 0;
}

/**
 * Add Player To a Channel with the player-channel-linklist
 * Checks if player is already on that channel, and so on
 * @param pl     Pointer of player-struct
 * @param name   Shortcut or name (can be abbreviated) of channel player wants to join
 * @param params will be used to hold the pw for pw-protected (clan)channels
 * TODO: pw-channels
 */
void addPlayerToChannel(player_t *pl, char *name, char *params)
{
    struct channels         *channel=NULL;
    struct player_channel   *pl_channel=NULL;


    /* Check if player is already on the channel */
    /* Step 1: check for shortcut */
    pl_channel=getPlChannelFromPlayerShortcut(pl, name);
    if (pl_channel)
    {
        ndi(NDI_UNIQUE, 0, pl->ob, "You have already joined the channel %s",pl_channel->channel->name);
        return;
    }

    /* Step 2: ok lets check for channelname */
    for (pl_channel=pl->channels;pl_channel;pl_channel=pl_channel->next_channel)
    {
        if (!strncasecmp(pl_channel->channel->name, name, strlen(name)))
        {
            ndi(NDI_UNIQUE, 0, pl->ob, "You have already joined channel %s",pl_channel->channel->name);
            return;
        }
    }

    /* Ok player is not on a channel with that shortcut or name, good */
    /* lets try to find the channel, first of course with shortcut */
    channel=getChannelFromGlobalShortcut(pl, name);
    if (!channel) /* No channel with that (default)-shortcut, or no shortcut at all given */
    {             /* We have to find the channel by name, and also make sure its unique */
        channel=findGlobalChannelFromName(pl, name, 0); /* error message and non-unique channel name handled in the function, TODO: move Messages outside this function */
        if (!channel)
        {
            return;
        }
    }
    /* NOTE: we don't have to check for enter-restrictions here, its all handled i the different find-functions
     * The channel simply don't exist for that player */


/*TODO: bans, ..... */

    /* ok we have a channel, no ban, no restrictions, lets add the player */
    pl_channel=final_addChannelToPlayer(pl, channel, 0);

    /* ok player is added to channel, give message to player */
    ndi(NDI_UNIQUE, 0, pl->ob, "You enter channel %s.",channel->name);

/*TODO: implement DM/DM/VOL inform */

    return;
}

/**
 * Here we try to find a matching channel in the whole channellist
 * If we find more than one channel, we give the player a message
 * with all matching channels
 * @param pl PlayerStruct of Player or NULL. If NULL, *all* channels are legit,
 * no NDI feedback is given, and no multiple matches are checked for.
 * @param name Name (or Part of name) from channel.
 * @param mute, No 'error' output, useful if we implement some channel_kick
 * commands. If pl is NULL, this is set to 1.
 * @return pointer to matching channel or NULL (no match or multiple matches
 * found).
 */
struct channels *findGlobalChannelFromName(player_t *pl, char *name, int mute)
{
    struct channels *match,
                    *duplicate;

    /* Check for the first match. */
    for (match = channel_list_start; match; match = match->next)
    {
        if (pl &&
            /* We don't display channels we can't get on. This may be by level
             * restriction or gmaster_mode. VOLs, GMs, and SAs are not subject
             * to level restrictions. */
            (!compare_gmaster_mode(match->gmaster_mode, pl->gmaster_mode) ||
            /* Method stub for later to implement clan system. This function
             * should return 1 if the player is in that clan. channel->clan
             * will be some sort of pointer to the clan info... */
//             !is_player_in_clan(match->clan) ||
             (!(pl->gmaster_mode & (GMASTER_MODE_SA | GMASTER_MODE_GM | GMASTER_MODE_VOL)) &&
              pl->ob->level < match->enter_lvl)))
        {
            continue;
        }

        if (!strncasecmp(match->name, name, strlen(name)))
        {
            break;
        }
    }

    /* No match? */
    if (!match)
    {
        if (!mute &&
            pl)
        {
            ndi(NDI_UNIQUE, 0, pl->ob, "There is no channel with that name.");
        }

        return NULL;
    }
    /* If pl is NULL, just return this first match. */
    else if (!pl)
    {
        return match;
    }

    /* Check for any further matches. */
    for (duplicate = match->next; duplicate; duplicate = duplicate->next)
    {
        /* We don't display channels we can't get on. This may be by level
         * restriction or gmaster_mode. VOLs, GMs, and SAs are not subject to
         * level restrictions. */
        if (!compare_gmaster_mode(duplicate->gmaster_mode, pl->gmaster_mode) ||
            /* Method stub for later to implement clan system. This function
             * should return 1 if the player is in that clan. channel->clan
             * will be some sort of pointer to the clan info... */
//          !is_player_in_clan(match->clan) ||
            (!(pl->gmaster_mode & (GMASTER_MODE_SA | GMASTER_MODE_GM | GMASTER_MODE_VOL)) &&
             pl->ob->level < duplicate->enter_lvl))
        {
            continue;
        }

        if (!strncasecmp(duplicate->name, name, strlen(name)))
        {
            if (match)
            {
                if (!mute)
                {
                    ndi(NDI_UNIQUE, 0, pl->ob, "That channelname was not unique. The following channels match:");
                    ndi(NDI_UNIQUE, 0, pl->ob, "  ~%s~", match->name);
                }

                match = NULL;
            }

            if (!mute)
            {
                ndi(NDI_UNIQUE, 0, pl->ob, "  ~%s~", duplicate->name);
            }
        }
    }

    return match;
}

/**
 * Here we try to find the channel over the players shortcut
 * @param pl PlayerStruct of Player
 * @param name Shortcut
 * @return pointer to player-channel-linklist
 */
struct player_channel *getPlChannelFromPlayerShortcut(player_t *pl, char *name)
{
    struct player_channel *pl_channel;

    if (strlen(name)!=1) {return NULL;} /*Shortcut only 1 char*/
    if (name[0]=='#') {return NULL;}    /* # is the placeholder for no shortcut */
    for (pl_channel=pl->channels;pl_channel;pl_channel=pl_channel->next_channel)
    {
        if (pl_channel->shortcut==name[0])
        {
            return pl_channel;        /* Ok we have one */
        }
    }
    return NULL; /* No channel found */
}
/**
 * try to find a channel over the global (default) shortcut
 * @param pl PlayerStruct of Player
 * @param name Shortcut
 * @return pointer to channel or NULL
 */
struct channels *getChannelFromGlobalShortcut(player_t *pl, char *name)
{
    struct channels *channel;

    if (strlen(name)!=1) {return NULL;}
    if (name[0]=='#') {return NULL;}

    for (channel=channel_list_start;channel;channel=channel->next)
    {
                /* We don't display channels we can't get on.
                 * This may be by level restriction ot gmaster_mode.
                 * VOLs, GMs, and SAs are not subject to level restrictions. */
                if (!compare_gmaster_mode(channel->gmaster_mode,
                                       pl->gmaster_mode) ||
/* Method stub for later to implement clan system
 * This function should return 1 if the player is in that clan.
 * channel->clan will be some sort of pointer to the clan info...
 */
//                  !is_player_in_clan(channel->clan) ||
                    (!(pl->gmaster_mode & (GMASTER_MODE_SA | GMASTER_MODE_GM | GMASTER_MODE_VOL)) &&
                     pl->ob->level < channel->enter_lvl))
                continue;/* restricted channel */
        if(channel->shortcut==name[0])
            return channel;
    }
    return NULL;
}
/**
 * Allocates an player_channel-linklist node, sorts alphabetically it in the two link-chains
 * @param pl PlayerStruct of Player
 * @param channel pointer to channel to add player to
 * @param shortcut shortcut, or 0 for default shortcut
 * @return pointer to new player-channel-linklist
 */
struct player_channel *final_addChannelToPlayer(player_t *pl, struct channels *channel, char shortcut)
{
    struct player_channel *node;
    struct player_channel *ptr=NULL, *ptr1=NULL;

    node = (struct player_channel *) get_poolchunk(pool_player_channel);
    node->channel=channel;
    node->pl=pl;
    node->mute_freq=0;
    node->mute_counter=0;
    node->mute_msg_count=0;
    node->mute_flags=0;

    if (shortcut)
        node->shortcut=shortcut;
    else
        node->shortcut=channel->shortcut; /* default shortcut */

    /* We sort alphabetically */

    /* First Element of List? */
    if(pl->channels==NULL)
    {
        pl->channels=node;
        node->next_channel=NULL;
    }
    else
    {
        ptr=pl->channels;
        while(ptr != NULL && (strcasecmp(ptr->channel->name,channel->name) < 0))
        {
            ptr1=ptr;
            ptr=ptr->next_channel;
        }
        /* If node 'smaller' as first element, insert at beginning */
        if(ptr==pl->channels)
        {
            pl->channels=node;
            node->next_channel=ptr;
        }
        /* last position, or in the middle ptr1 holds the forerunner */
        else
        {
            ptr1->next_channel=node;
            node->next_channel=ptr;
        }
    }

    /*This list is 'double-linked', lets add to the second link-chain alphabetically */
    ptr=ptr1=NULL;

    /* First Element of List? */
    if(channel->players==NULL)
    {
        channel->players=node;
        node->next_player=NULL;
    }
    else
    {
        ptr=channel->players;
        while(ptr != NULL && (strcasecmp(ptr->pl->ob->name,pl->ob->name) < 0))
        {
            ptr1=ptr;
            ptr=ptr->next_player;
        }
        /* If node 'smaller' as first element, insert at beginning */
        if(ptr==channel->players)
        {
            channel->players=node;
            node->next_player=ptr;
        }
        /* last position, or in the middle ptr1 holds the forerunner */
        else
        {
            ptr1->next_player=node;
            node->next_player=ptr;
        }
    }

    if (!pl->privacy)
        channel->pl_count++;
    pl->channel_count++;
    return node;
}

int channelname_ok(char *cp)
{
    char *tmp=cp;

    if(*cp==' ') /* we start with a whitespace? in this case we don't trim - kick*/
        return 0;

    for(;*cp!='\0';cp++)
    {
        if(!((*cp>='a'&&*cp<='z')||(*cp>='A'&&*cp<='Z')||(*cp>='0'&&*cp<='9'))&&*cp!='.'&&*cp!='_')
            return 0;
    }
    if(!strcasecmp(tmp,"on") || !strcasecmp(tmp,"off") || !strcasecmp(tmp,"allow") || !strcasecmp(tmp,"channel") || !strcasecmp(tmp,"group"))
        return 0;
    return 1;
}
/**
 * Here we try to find a matching channel in the players channellist
 * If we find more than one channel, we give the player a message
 * with all matching channels. used for addPlayerToChannel and other stuff
 * @param pl PlayerStruct of Player
 * @param wiz Player to whom the error messages go (channel_kick  and so on...)
 * @param name Name (or Part of name) from channel
 * @param mute, No 'error' output, useful if we implement some channel_kick commands
 * @return pointer to matching player_channel-link
 */
struct player_channel *findPlayerChannelFromName(player_t *pl, player_t *wiz, char *name, int mute)
{
    struct  player_channel *c, *tmp;
    char   buf[50];

    if (pl==wiz)
    {
        strcpy(buf,"You aren't");
    }
    else
    {
        sprintf(buf,"%s isn't",pl->ob->name);
    }

    c=pl->channels;
    while (c)
    {
        if (!strncasecmp(c->channel->name, name, strlen(name)))
            break;
        c=c->next_channel;
    }
    if (!c)
    {
        if (!mute)
        {
            ndi(NDI_UNIQUE, 0, wiz->ob, "%s on a channel with that name.",buf);
        }
        return c;
    }

    for (tmp=c->next_channel;tmp;tmp=tmp->next_channel)
    {
        if (!strncasecmp(tmp->channel->name, name, strlen(name)))
        {
            if (c)
            {
                if (!mute)
                {
                    ndi(NDI_UNIQUE, 0, wiz->ob, "That channelname was not unique");
                    ndi(NDI_UNIQUE, 0, wiz->ob, "The following channels match:");
                    ndi(NDI_UNIQUE, 0, wiz->ob, "%s", c->channel->name);
                }
                c=NULL;
            }
            if (!mute) {ndi(NDI_UNIQUE, 0, wiz->ob, "%s", tmp->channel->name);}
        }
    }
    return c;

}
/**
 * Removes the Player from the channel, frees the player_channel-link
 * @param pl PlayerStruct of Player
 * @param pl_channel player_channel-link for that player/channel
 * @param msg Message to send to player (eg, reason for removal) or NULL for none.
 */
void removeChannelFromPlayer(player_t *pl, struct player_channel *pl_channel, char *msg)
{
    struct player_channel *pl_node, *pl_tmp=NULL;

    for(pl_node = pl->channels;pl_node;pl_node = pl_node->next_channel)
    {
        if(pl_channel->channel==pl_node->channel)
        {
            if(pl_tmp)
                pl_tmp->next_channel = pl_node->next_channel;
            else
                pl->channels = pl_node->next_channel;
            break;
        }
        pl_tmp = pl_node;
    }
    pl_tmp=NULL;
    for(pl_node = pl_channel->channel->players;pl_node;pl_node = pl_node->next_player)
    {
        if(pl==pl_node->pl)
        {
            if(pl_tmp)
                pl_tmp->next_player = pl_node->next_player;
            else
                pl_channel->channel->players = pl_node->next_player;
            break;
        }
        pl_tmp = pl_node;
    }

    if (!pl->privacy)
        pl_channel->channel->pl_count--;
    pl->channel_count--;

    return_poolchunk(pl_channel,pool_player_channel);

    if (msg)
        ndi(NDI_UNIQUE, 0, pl->ob, "%s", msg);

    return;
}

/**
 * Sends a normal message to all player who listens
 * @param pl PlayerStruct of Sender or NULL for a system message.
 * @param pl_channel player_channel-link for that player/channel
 * @param params Message
 */
void sendChannelMessage(player_t *pl, struct channels *channel, char *params)
{
    struct player_channel *cpl;
    sockbuf_struct        *sockbuf;
    char                   from[MAX_PLAYER_NAME],
                           buf[LARGE_BUF];

    sprintf(from, "%s", (pl) ? pl->ob->name : "|Daimonin|");
    CHATLOG("CH:%s:%s >%s<\n", channel->name, from, params);
    sprintf(buf, "%c%c%s %s:%s", 0, channel->color, channel->name, from, params);
    sockbuf = SOCKBUF_COMPOSE(SERVER_CMD_CHANNELMSG, buf, strlen(buf + 2) + 2, 0);

    for (cpl = channel->players; cpl; cpl = cpl->next_player)
    {
        if (cpl->pl->channels_on)
        {
            SOCKBUF_ADD_TO_SOCKET(&(cpl->pl->socket), sockbuf);
        }
    }

    SOCKBUF_COMPOSE_FREE(sockbuf);

#ifdef CHANNEL_HIST
    addChannelHist(channel, from, params, 0);
#endif
}

/**
 * Sends a EMOTE message to all player who listens, in their own configured color
 * @param pl PlayerStruct of Sender or NULL for a system message.
 * @param pl_channel player_channel-link for that player/channel
 * @param params Message
 */
/* XXX: Why is this a separate func from semdChannelMessage()? Can't we just
 * use a fourth 0 or 1 param, emote?
 * -- Smacky 20120204 */
void sendChannelEmote(player_t *pl, struct channels *channel, char *params)
{
    struct player_channel *cpl;
    sockbuf_struct        *sockbuf;
    char                   from[MAX_PLAYER_NAME],
                           buf[LARGE_BUF];

    sprintf(from, "%s", (pl) ? pl->ob->name : "|Daimonin|");
    CHATLOG("CH:%s:%s >%s<\n", channel->name, from, params);
    sprintf(buf, "%c%c%s %s:%s", 1, channel->color, channel->name, from, params);
    sockbuf = SOCKBUF_COMPOSE(SERVER_CMD_CHANNELMSG, buf, strlen(buf + 2) + 2, 0);

    for (cpl = channel->players; cpl; cpl = cpl->next_player)
    {
        if (cpl->pl->channels_on)
        {
            SOCKBUF_ADD_TO_SOCKET(&(cpl->pl->socket), sockbuf);
        }
    }

    SOCKBUF_COMPOSE_FREE(sockbuf);

#ifdef CHANNEL_HIST
    addChannelHist(channel, from, params, 1);
#endif
}

/**
 * New Players, or old savefiles...
 * Add the player to the default channels
 * @param pl PlayerStruct of Sender
 */
void addDefaultChannels(player_t *pl)
{
//    loginAddPlayerToChannel(pl,"Auction",0,-1,0);
    loginAddPlayerToChannel(pl,"Quest",0,0);
    loginAddPlayerToChannel(pl,"General",0,0);
    loginAddPlayerToChannel(pl,"Help",0,0);
    pl->channel_count=3;
    return;
}
/**
 * Wrapper function for final_addChannelToPlayer at login
 * (we need to make sure the channel still exists
 * @param pl PlayerStruct of Player
 * @param channel pointer to channel to add player to
 * @param shortcut shortcut, or 0 for default shortcut
 * @param color color for that channel, or -1 for default color
 */
void loginAddPlayerToChannel(player_t *pl, char *channelname, char shortcut, unsigned long mute)
{
    /* first lets check if the channel still exists
     * channelname must be an exact match, not that player logs out, with channel
     * 'foo' on, later the channel gets closed/deleted, a new channel 'foobar' exitst
     * and player is at login added to this 0 one
     */
     struct channels *channel;
     struct player_channel *cpl;

     /* Don't add the gmaster channels at login. */
     if ((channel = findGlobalChannelFromName(pl, channelname, (int)mute)) &&
         channel->gmaster_mode != GMASTER_MODE_NO)
         return;

     for (channel=channel_list_start;channel;channel=channel->next)
     {
         if (!strcasecmp(channel->name,channelname))
         {
            cpl=final_addChannelToPlayer(pl, channel, shortcut);
            if (mute>0)
                cpl->mute_counter=pticks+mute;
         }
     }

    return;
}
/**
 * Loads the channels from the channel-savefile
 *
 */
void load_channels(void)
{
    FILE   *channelfile;
    char    buf[HUGE_BUF];
    char    line_buf[MEDIUM_BUF];
    char    channelname[MAX_CHANNEL_NAME];
    int     channelcolor=-1;
    int     channelenterlevel=1, channelpostlevel=1;
    int     channelgmastermode=GMASTER_MODE_NO;
    char    defaultshortcut='#';

    LOG(llevInfo,"loading channel_file2....\n");
    sprintf(buf, "%s/channel_file2", settings.localdir);

    if ((channelfile = fopen(buf, "r")) == NULL)
    {
        LOG(llevDebug, "Could not find channel_file. Using default channels.\n");
        final_addChannel(CHANNEL_NAME_SA, 'S', 3, 1, 1, GMASTER_MODE_SA);
        final_addChannel(CHANNEL_NAME_MM, 'M', 3, 1, 1, GMASTER_MODE_MM);
        final_addChannel(CHANNEL_NAME_MW, 'W', 3, 1, 1, GMASTER_MODE_MW);
        final_addChannel(CHANNEL_NAME_GM, 'G', 3, 1, 1, GMASTER_MODE_GM);
        final_addChannel(CHANNEL_NAME_VOL, 'V', 3, 1, 1, GMASTER_MODE_VOL);
        final_addChannel("Auction", 'a', 2, 1, 1, GMASTER_MODE_NO);
        final_addChannel("General", 'g', -1, 1, 1, GMASTER_MODE_NO);
        final_addChannel("Help", 'h', 4, 1, 1, GMASTER_MODE_NO);
        final_addChannel("Quest", 'q', 5, 1, 1, GMASTER_MODE_NO);

        return;
    }

    while (fgets(line_buf, 160, channelfile) != NULL)
    {
        if (line_buf[0] == '#')
            continue;

        if (sscanf(line_buf, "%s %c %d %d %d %d",
                   channelname, &defaultshortcut, &channelcolor,
                   &channelpostlevel, &channelenterlevel,
                   &channelgmastermode) < 6)
            LOG(llevBug, "BUG: malformed channelfile file entry: %s\n", line_buf);
        else
            final_addChannel(channelname, defaultshortcut, channelcolor,
                             channelpostlevel, channelenterlevel,
                             channelgmastermode);
    }

    fclose(channelfile);

    return;
}
/**
 * Adds a new Channel
 * @param name Name of channel
 * @param chortcut Defaultshortcut of channel/ '#' means no shortcut!
 * @param color default color for that channel / -1 default shout color (orange)
 * @param enter_lvl level for entering
 * @param gmaster_mode gmaster_mode of channel (see gmaster.c/compare_gmaster_mode())
 */
struct channels *final_addChannel(char *name, char shortcut, int color, sint8 post_lvl, sint8 enter_lvl, int gmaster_mode)
{

    struct channels *node;
    struct channels *ptr, *ptr1=NULL;

    node = (struct channels *) malloc(sizeof(struct channels));
    strncpy(node->name,name,MAX_CHANNEL_NAME);
    node->name[MAX_CHANNEL_NAME]=0; /* sanity string ending */
    node->shortcut=shortcut;
    node->players=NULL;
#ifdef CHANNEL_HIST
    memset(node->history, 0, sizeof(node->history));
    node->lines=0;
    node->startline=0;
#endif
    if (color>-1)
        node->color=color;
    else
        node->color=NDI_ORANGE;
    node->pl_count=0;
    node->post_lvl = post_lvl;
    node->enter_lvl = enter_lvl;
    node->gmaster_mode = gmaster_mode;

    /* First Element of List? */
    if (!channel_list_start)
    {
        channel_list_start = node;
        node->next = NULL;
    }
    else
    {
        ptr = channel_list_start;

        while (ptr &&
               (!compare_gmaster_mode(ptr->gmaster_mode, node->gmaster_mode) ||
                strcasecmp(ptr->name, name) < 0))
        {
            ptr1 = ptr;
            ptr = ptr->next;
        }

        /* If node 'smaller' as first element, insert at beginning */
        if (ptr == channel_list_start)
        {
            channel_list_start = node;
            node->next = ptr;
        }
        /* last position, or in the middle ptr1 holds the forerunner */
        else
        {
            ptr1->next = node;
            node->next = ptr;
        }
    }

    return node;
}
/**
 * called when player leaves the game to remove him completely and free all player_channel-links
 * @param pl Pointer to player struct
 */
void leaveAllChannels(player_t *pl)
{
    struct player_channel  *node, *tmp=NULL;

    node=pl->channels;
    while (node)
    {
        tmp=node;
        node=node->next_channel;
        removeChannelFromPlayer(pl, tmp, NULL);
    }
//    LOG(llevDebug,"channel: leaveAll: %s\n",pl->ob->name);
    return;
}
/**
 * called when player changes privacy state...
 * @param pl Pointer to player struct
 * @param privacy value of privacy
 */
void channel_privacy(player_t *pl, int privacy)
{
    struct player_channel *pl_channel;
    for (pl_channel=pl->channels;pl_channel;pl_channel=pl_channel->next_channel)
    {
        if (privacy)
            pl_channel->channel->pl_count--;
        else
            pl_channel->channel->pl_count++;
    }
    return;
}
/* Chanelsupport must also enable in the plugin (#define USE_CHANNELS in plugin_lua.h) */
/**
 * LUA-Plugin-Hook for Sending Messages on Channels from lua-scripts
 * @param channelname name of channel (NO abbreviations)
 * @param name name of sender/mob
 * @param message message to send
 * @param mode 0=normal, 1=emote
 * @return zero=success, non-zero=failure
 */
int lua_channel_message(char *channelname,  const char *name, char *message, int mode)
{
    struct channels *channel;
    int              r;

    for (channel=channel_list_start;channel;channel=channel->next)
    {
        if (!strcasecmp(channel->name,channelname)) /* lua: exact name */
        {
            struct player_channel *cpl=NULL;
            sockbuf_struct *sockbuf;
            char buf[HUGE_BUF];

            CHATLOG("LUA-CH:%s:%s >%s<\n", channel->name, name, message);

            /* TODO: channel history */
#ifdef CHANNEL_HIST
            addChannelHist(channel, name, message, mode);
#endif
            sprintf(buf,"%c%c%s %s:%s",mode, channel->color, channel->name, name, message);
            sockbuf = SOCKBUF_COMPOSE(SERVER_CMD_CHANNELMSG, buf, strlen(buf+2)+2, 0);

            for (cpl=channel->players;cpl;cpl=cpl->next_player)
                if (cpl->pl->channels_on)
                    SOCKBUF_ADD_TO_SOCKET(&(cpl->pl->socket), sockbuf);

            SOCKBUF_COMPOSE_FREE(sockbuf);
            r = 0; // success
            break;
        }
    }

    if (!channel)
    {
        LOG(llevDebug,"LUA: channelMsg: no channel with that name: %s\n",channelname);
        r = 1; // failure
    }

    return r;
}

/* save the defined channels
 */
void save_channels(void)
{
    char    filename[MEDIUM_BUF];
    struct channels *channel;
    FILE   *fp;

    LOG(llevSystem,"write channel_file2...\n");
    sprintf(filename, "%s/channel_file2", settings.localdir);
    if ((fp = fopen(filename, "w")) == NULL)
    {
        LOG(llevBug, "BUG: Cannot open %s for writing\n", filename);
        return;
    }
    fprintf(fp, "# CHANNEL_FILE (file is changed from server at runtime)\n");
    fprintf(fp, "# Take care when editing this file by hand:\n");
    fprintf(fp, "# entry format is:\n# <channelname> <defaultshortcut> <defaultcolor> <postlevel> <enterlevel> <gmastermode>\n");

    for (channel=channel_list_start;channel;channel=channel->next)
    {
        fprintf(fp,"%s %c %d %d %d %d\n",channel->name,channel->shortcut, channel->color,channel->post_lvl,channel->enter_lvl,channel->gmaster_mode);
    }

    fclose(fp);
}

int command_channel_create(object_t *ob, char *params)
{
    char    channelname[MAX_CHANNEL_NAME];
    int     channelcolor=-1;
    int     channelenterlevel=1, channelpostlevel=1;
    int     channelgmastermode=GMASTER_MODE_NO;
    char    defaultshortcut='#';
    struct channels *channel;

    if (!params)
    {
        ndi(NDI_UNIQUE, 0, ob, "Syntax: /createchannel <name> <defaultshortcut> <defaultcolor> <postlevel> <enterlevel> <gmastermode>");
        ndi(NDI_UNIQUE, 0, ob, "set defaultshortcut to '#' for no defaultshortcut");
        ndi(NDI_UNIQUE, 0, ob, "set defaultcolor to '-1' for no defaultcolor");
        return 0;
    }

    if (sscanf(params, "%s %c %d %d %d %d", channelname, &defaultshortcut, &channelcolor, &channelpostlevel, &channelenterlevel, &channelgmastermode) < 6)
    {
        ndi(NDI_UNIQUE, 0, ob, "Syntax: /createchannel <name> <defaultshortcut> <defaultcolor> <postlevel> <enterlevel> <gmastermode>");
        ndi(NDI_UNIQUE, 0, ob, "set defaultshortcut to '#' for no defaultshortcut");
        ndi(NDI_UNIQUE, 0, ob, "set defaultcolor to '-1' for no defaultcolor");
        return 0;
    }
    else
    {
        if (strlen(channelname)>MAX_CHANNEL_NAME)
        {
            ndi(NDI_UNIQUE, 0, ob, "Channelname is too long!");
            return 0;
        }
        if (!channelname_ok(channelname))
        {
            ndi(NDI_UNIQUE, 0, ob, "This Channelname is not allowed!");
            return 0;
        }
        if (strlen(channelname)<3)
        {
            ndi(NDI_UNIQUE, 0, ob, "Channelname must be at least 3 chars long!");
            return 0;
        }

        for (channel=channel_list_start;channel;channel=channel->next)
        {
            if (!strcasecmp(channel->name,channelname) || ((channel->shortcut==defaultshortcut) && (defaultshortcut!='#')))
            {
                ndi(NDI_UNIQUE, 0, ob, "Channel with that name or shortcut already exists!");
                return 0;
            }
        }
        final_addChannel(channelname,defaultshortcut,channelcolor, channelpostlevel, channelenterlevel, channelgmastermode);
        ndi(NDI_UNIQUE, 0, ob, "Channel %s is created.",channelname);
        CHATLOG("Create:>%s<: %s, %d, %d, %d\n", ob->name, channelname, channelpostlevel, channelenterlevel, channelgmastermode);
        save_channels();
    }

    return 0;

}

void forceAddPlayerToChannel(struct player_channel *cpl, char *params)
{
    player_t *pl=NULL;

    if (!params)
    {
        ndi(NDI_UNIQUE, 0, cpl->pl->ob, "Syntax: -<channel>!add <player>");
        return;
    }
    if (!(pl=find_player(params)))
    {
        ndi(NDI_UNIQUE, 0, cpl->pl->ob, "Player %s not found.",params);
        return;
    }
    final_addChannelToPlayer(pl, cpl->channel, 0);
    ndi(NDI_UNIQUE, 0, cpl->pl->ob, "You added player %s to channel %s.",pl->ob->name, cpl->channel->name);
    ndi(NDI_UNIQUE, 0, pl->ob, "You were added to channel %s by %s.",cpl->channel->name, cpl->pl->ob->name);
    CHATLOG("Pl >%s< added pl %s to channel %s\n", cpl->pl->ob->name, pl->ob->name, cpl->channel->name);

    return;

}
void kickPlayerFromChannel(struct player_channel *cpl, char *params)
{
    player_t *pl=NULL;
    struct player_channel *kick;
    char buf[MEDIUM_BUF];

#if 0
    if (!params)
    {
        ndi(NDI_UNIQUE, 0, cpl->pl->ob, "Syntax: -<channel>!kick <player>");
        return;
    }
    if (!(pl=find_player(params)))
    {
        ndi(NDI_UNIQUE, 0, cpl->pl->ob, "Player %s not found.",params);
        return;
    }

    kick=findPlayerChannelFromName(pl, cpl->pl, cpl->channel->name, 1);
    sprintf(buf, "You were kicked from channel %s by %s.", cpl->channel->name, cpl->pl->ob->name);
    removeChannelFromPlayer(pl, kick, buf);
    ndi(NDI_UNIQUE, 0, cpl->pl->ob, "You kicked player %s from channel %s.",pl->ob->name, cpl->channel->name);
    CHATLOG("Pl >%s< kicked pl %s from channel %s\n", cpl->pl->ob->name, pl->ob->name, cpl->channel->name);
#endif // 0

    return;

}


int command_channel_mute(object_t *ob, char *params)
{
    struct channels *channel=NULL;
    struct player_channel *cpl;

    char channelname[256]="";
    char playername[256]="";
    int seconds=0;

    if (!params)
//    {
//        ndi(NDI_UNIQUE, 0, ob, "Syntax: /channemute <channel> <who> <howmuch>");
        return 1;
//    }

    sscanf(params, "%s %s %d", channelname, playername, &seconds);

    if(seconds<0)
//    {
//        ndi(NDI_UNIQUE, 0, ob, "/channelmute command: illegal seconds parameter (%d)", seconds);
        return 1;
//    }

    for (channel=channel_list_start;channel;channel=channel->next)
    {
        if (!strcasecmp(channel->name,channelname))
        {
            break;
        }
    }
    if (!channel)
    {
        ndi(NDI_UNIQUE, 0, ob, "No channel with that name.");
        return 0;
    }

    for (cpl=channel->players;cpl;cpl=cpl->next_player)
    {
        if (!strcasecmp(cpl->pl->ob->name,playername))
            break;
    }

    if (!cpl)
    {
        ndi(NDI_UNIQUE, 0, ob, "%s is not on that channel.",playername);
        return 0;
    }
    /* ok now we have all checks done... i hate it that we have to do such an effort in coding for only some assholes */

    if(!seconds) /* unmute player */
    {
        ndi(NDI_UNIQUE, 0, ob, "/channelmute command: umuting player %s on channel %s!", playername, channelname);
        cpl->mute_counter = 0;
    }
    else
    {
        ndi(NDI_UNIQUE, 0, ob, "/channelmute command: mute player %s for %d seconds on channel %s!", playername, seconds, channelname);
        cpl->mute_counter = pticks+seconds*(1000000/MAX_TIME);
    }

    return 0;
}

int command_channel_delete(object_t *ob, char *params)
{
    struct channels *channel=NULL, *ch_ptr1=NULL;
    struct player_channel *cpl;

    if (!params)
//    {
//        ndi(NDI_UNIQUE, 0, ob, "Syntax: /deletechannel <name>");
        return 1;
//    }
    for (channel=channel_list_start;channel;channel=channel->next)
    {
        if (!strcasecmp(channel->name,params))
        {
            break;
        }
    }
    if (!channel)
    {
        ndi(NDI_UNIQUE, 0, ob, "No channel with that name.");
        return 0;
    }

    for (cpl=channel->players;cpl;cpl=cpl->next_player)
    {
        char buf[MEDIUM_BUF];
        sprintf(buf, "Channel '%s' is now closed!", channel->name);
        removeChannelFromPlayer(cpl->pl, cpl, buf);
    }
    for (ch_ptr1=channel_list_start;ch_ptr1;ch_ptr1=ch_ptr1->next)
    {
        if (channel==ch_ptr1->next)
        {
           ch_ptr1->next=channel->next;
           ndi(NDI_UNIQUE, 0, ob, "Channel '%s' deleted.",channel->name);
           free(channel);
           CHATLOG("Delete:>%s<: %s\n", ob->name, params);
           save_channels();
           return 0;
        }
    }
    return 0;
}

int check_channel_mute(struct player_channel *cpl)
{
    /* first we check for a 'global mute' */
    if(cpl->pl->mute_counter)
    {
        if(cpl->pl->mute_counter <= pticks) /* its ok */
            cpl->pl->mute_counter=0;
        else /* player is muted */
        {
            if(cpl->pl->mute_msg_count<=pticks)
            {
                unsigned long tmp = (cpl->pl->mute_counter-pticks)/(1000000/MAX_TIME);

                ndi( NDI_UNIQUE, 0, cpl->pl->ob, "You are still muted for %d second(s).", (int)(tmp?tmp:1));
                cpl->pl->mute_msg_count = pticks+MUTE_MSG_FREQ;
            }
            return 0;
        }
    }
    /* now we check for the channel itself... */
    else if(cpl->mute_counter)
    {
        if(cpl->mute_counter <= pticks) /* its ok */
            cpl->mute_counter=0;
        else /* player is muted */
        {
            if(cpl->mute_msg_count<=pticks)
            {
                unsigned long tmp = (cpl->mute_counter-pticks)/(1000000/MAX_TIME);

                ndi( NDI_UNIQUE, 0, cpl->pl->ob, "You are still muted for %d second(s) on this channel.", (int)(tmp?tmp:1));
                cpl->mute_msg_count = pticks+MUTE_MSG_FREQ;
            }
            return 0;
        }
    }
    else /* no old mute - lets check for new*/
    {
        if(cpl->mute_freq<=pticks) /* can we do a shout class communication ?*/
        {
            /* yes, all fine */
            cpl->mute_freq=pticks+MUTE_FREQ_SHOUT;
            cpl->mute_flags &= ~(MUTE_FLAG_SHOUT|MUTE_FLAG_SHOUT_WARNING);
        }
        else /* nope - don't process the comm. action and tell the player why */
        {
            if(!(cpl->mute_flags & MUTE_FLAG_SHOUT)) /* friendly message not to spam */
            {
                ndi( NDI_UNIQUE, 0, cpl->pl->ob, "Please wait 2 seconds between messages on the same channel.");
                cpl->mute_flags |= MUTE_FLAG_SHOUT;
                return 0;
            }
            else if(!(cpl->mute_flags & MUTE_FLAG_SHOUT_WARNING)) /* first & last warning */
            {
                ndi( NDI_UNIQUE|NDI_ORANGE, 0, cpl->pl->ob, "Auto-Mute Warning: Please wait 2 seconds!");
                cpl->mute_flags |= MUTE_FLAG_SHOUT_WARNING;
                return 0;
            }
            else /* mute him */
            {
                ndi( NDI_UNIQUE|NDI_RED, 0, cpl->pl->ob, "Auto-Mute: Don't spam! You are muted for %d seconds!",(int)(MUTE_AUTO_NORMAL/(1000000/MAX_TIME)));
                cpl->mute_counter = pticks+MUTE_AUTO_NORMAL;
                return 0;
            }
        }
    }
    return 1;
}

void modify_channel_params(struct player_channel *cpl, char *params)
{
    int enter_lvl=0;
    int post_lvl=0;

    if (!params)
    {
        ndi(NDI_UNIQUE, 0, cpl->pl->ob, "Syntax: -<channel>!mod <postlevel> <enterlevel>");
        return;
    }

    sscanf(params, "%d %d", &post_lvl, &enter_lvl);

    if (post_lvl>0)
        cpl->channel->post_lvl=post_lvl;

    if (enter_lvl>0)
        cpl->channel->enter_lvl=enter_lvl;

    ndi(NDI_UNIQUE, 0, cpl->pl->ob, "Channel: %s - postlvl: %d, enterlvl: %d",cpl->channel->name, cpl->channel->post_lvl,cpl->channel->enter_lvl);

    save_channels();

    return;

}

void sendVirtualChannelMsg(player_t *sender, char *channelname, player_t *target, char* msg, int color)
{
    sockbuf_struct *sockbuf;
    char buf[512];

    sprintf(buf,"%c%c%s %s:%s",2,color, channelname, sender->ob->name, msg);

    sockbuf = SOCKBUF_COMPOSE(SERVER_CMD_CHANNELMSG, buf, strlen(buf+2)+2, 0);

    SOCKBUF_ADD_TO_SOCKET(&(target->socket), sockbuf);

    SOCKBUF_COMPOSE_FREE(sockbuf);
    return;
}

#ifdef CHANNEL_HIST
void sendChannelHist(struct player_channel *cpl, int lines)
{
    int i, line;

    i=1;
    if (lines>cpl->channel->lines)
        lines=cpl->channel->lines;

    line=cpl->channel->startline+(cpl->channel->lines-lines);

    if (line>=MAX_CHANNEL_HIST_LINES)
        line=line-MAX_CHANNEL_HIST_LINES;

    while(i<=lines)
    {
        SOCKBUF_REQUEST_BUFFER(&(cpl->pl->socket), SOCKET_SIZE_SMALL);
        SockBuf_AddChar(ACTIVE_SOCKBUF(&(cpl->pl->socket)),cpl->channel->history[line][0]);
        SockBuf_AddChar(ACTIVE_SOCKBUF(&(cpl->pl->socket)),(uint8) cpl->channel->color);
        SockBuf_AddString(ACTIVE_SOCKBUF(&(cpl->pl->socket)), cpl->channel->history[line]+1, strlen(cpl->channel->history[line]+1));
        SOCKBUF_REQUEST_FINISH(&(cpl->pl->socket), SERVER_CMD_CHANNELMSG, SOCKBUF_DYNAMIC);

        line++;
        if (line==MAX_CHANNEL_HIST_LINES)
            line=0;
        i++;

    }
    CHATLOG("Pl >%s< called chan-hist from %s and got %d msg\n", cpl->pl->ob->name, cpl->channel->name, (i-1));

    return;
}

void addChannelHist(struct channels *channel, const char* name, char *msg, int mode)
{
    char *line=NULL;
    char  buf[LARGE_BUF];
    char  timestr[TINY_BUF];
    time_t  zeit;

    if (channel->lines==MAX_CHANNEL_HIST_LINES)
    {
        line=channel->history[channel->startline];
        channel->startline++;
        if (channel->startline==MAX_CHANNEL_HIST_LINES)
            channel->startline=0;
    }
    else
    {
        line=channel->history[channel->lines];
        channel->lines++;
    }
    if (line)
    {
        time(&zeit);
        strftime(timestr,sizeof(timestr),"[%H:%M]",gmtime(&zeit));
        sprintf(buf,"%d%s %s:%s %s",mode, channel->name, name, msg, timestr);
        strncpy(line,buf,MAX_CHANNEL_HIST_CHAR);
        line[MAX_CHANNEL_HIST_CHAR-1]='\0';

    }

    return;
}
#endif



//#endif



