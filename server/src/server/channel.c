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

    The author can be reached via e-mail to info@daimonin.net
*/
#include <global.h>

#ifdef USE_CHANNELS


struct channels *channel_list_start = NULL;


/** The Main Function
 */
int command_channel(object *ob, char *params)
{
    char    channelname[MAX_CHANNEL_NAME+1];
    char    mode;
    int     channelnamelen;
    struct  channels        *channel=NULL;
    struct  player_channel  *pl_channel=NULL;
    struct  player_channel  *cpl=NULL;


    channelname[0]=0;
    if (!params)
    {
        printChannelUsage(ob);
        return 1;
    }

    /* Command Parser */
    channelnamelen=strcspn(params,"-+ :?*!%");
    if (channelnamelen>MAX_CHANNEL_NAME)
    {
        new_draw_info_format(NDI_UNIQUE, 0, ob, "Channelname is too long!");
        return 1;                   /* Dont know, if i have to return 0, 1 or -1 if failing :)*/
    }
    strncpy(channelname,params,channelnamelen);
    channelname[channelnamelen]=0;
    mode=params[channelnamelen];
    params=params+channelnamelen+1;   /* wir verbiegen den Zeigen, für den Rest */

    if (strlen(params)>210)
    {
        new_draw_info_format(NDI_UNIQUE, 0, ob, "Message too long!");
        return 1;
    }
    /* end command-parser */

//    LOG(llevDebug, "channel_parse: name: %s, mode: %c, params: %s\n",channelname, mode, params);

    if (channelnamelen==0)              /* Handle the global functions */
    {
        if (mode=='-') /* temp-on-off without leaving */
        {
            CONTR(ob)->channels_on=FALSE;
            new_draw_info_format(NDI_UNIQUE, 0, ob, "You have all Channels temporally disabled");
            return 1;
        }
        if (mode=='+') /* temp-on-off without leaving */
        {
            CONTR(ob)->channels_on=TRUE;
            new_draw_info_format(NDI_UNIQUE, 0, ob, "You listen to all your channels again.");
            return 1;
        }
        if (mode=='?') /* List all channels */
        {
            /* atm its only a quick listing, TODO: maybe descriptions for channels? */

            new_draw_info_format(NDI_UNIQUE, 0, ob, "These channels exists: ");
            for (channel=channel_list_start;channel;channel=channel->next)
            {
                /* we don't diplsay channels we can't get on, >=VOL see always all channels, even if there lvl is to low*/
                if (CONTR(ob)->gmaster_mode < GMASTER_MODE_VOL &&
                   (channel->enter_lvl==-1 || ob->level<channel->enter_lvl))
                        continue;
                if(pl_channel=findPlayerChannelFromName(CONTR(ob),CONTR(ob), channel->name, TRUE))
                    new_draw_info_format(NDI_UNIQUE|pl_channel->color, 0, ob, "*(%d) [%c] %s", channel->pl_count, pl_channel->shortcut, channel->name);
                else
                    new_draw_info_format(NDI_UNIQUE|channel->color, 0, ob, "   (%d) [%c] %s", channel->pl_count, channel->shortcut, channel->name);
            }
            return 1;
        }
        /* no global command...*/
        printChannelUsage(ob);
        return 1;
    }

    /* lets handle all the channel-commands */
    if (mode=='+')  /* join channel */
    {
        addPlayerToChannel(CONTR(ob),channelname,params);
        return 1;
    }

    /* for all channelfuntions except joining we need the pointer of the desired channel in player's channel-list*/
    pl_channel=getPlChannelFromPlayerShortcut(CONTR(ob),channelname); /* wfirst we try to get the channel over the players shortcut */
    if (!pl_channel)
    {   /* All the error messages are handled here in this function, should eventually splittet */
        pl_channel=findPlayerChannelFromName(CONTR(ob),CONTR(ob), channelname, FALSE);
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
        removeChannelFromPlayer(CONTR(ob), pl_channel);
        new_draw_info_format(NDI_UNIQUE, 0, ob, "You leave channel %s",pl_channel->channel->name);
        return 1;
    }
    else if (mode=='?') /* list players on channel */
    {
        new_draw_info_format(NDI_UNIQUE, 0, ob, "On this channel listens:");

        /* ATM unfortunatly we can only send one player at a line,
         * cause clients append a newline to new_draw_info's
         * In the future if we have 1000 Players or more, we should think about some limitation or disabling
         */
        for (cpl=pl_channel->channel->players;cpl;cpl=cpl->next_player)
        {
            if (!cpl->pl->dm_stealth)
                new_draw_info_format(NDI_UNIQUE, 0, ob, "%s",cpl->pl->ob->name);
        }
        return 1;
    }
    else if (mode==' ') /* normal channelmessage */
    {
        /* Of course we have to check, if player really wants to say somethinig */
        if (strlen(params)==0)
        {
            return 1;
        }
        /* Check for lvl-post restrictions. >=VOL can always post. */
        if (ob->level<pl_channel->channel->post_lvl && CONTR(ob)->gmaster_mode < GMASTER_MODE_VOL)
        {
            new_draw_info_format(NDI_UNIQUE, 0, ob, "You need at least level %d to post on this channel.",pl_channel->channel->post_lvl);
            return 1;
        }
        if (check_channel_mute(pl_channel))
            sendChannelMessage(CONTR(ob),pl_channel, params);
        return 1;
    }
    else if (mode==':') /* emoted channelmessage */
    {
        if (strlen(params)==0)
        {
            return 1;
        }
        if (ob->level<pl_channel->channel->post_lvl && CONTR(ob)->gmaster_mode < GMASTER_MODE_VOL)
        {
            new_draw_info_format(NDI_UNIQUE, 0, ob, "You need at least level %d to post on this channel.",pl_channel->channel->post_lvl);
            return 1;
        }
        if (check_channel_mute(pl_channel))
            sendChannelEmote(CONTR(ob),pl_channel, params);
        return 1;
    }
    else if (mode=='*')
    {
        /* TODO: Channelhistory */
        return 1;
    }
    else if (mode=='!')
    {
        if (strlen(params)==0)
        {
            new_draw_info_format(NDI_UNIQUE, 0, ob, "Colorsyntax: -<channelname>!color <1-210>");
            return 1;
        }
        if (!strncasecmp(params,"color",strlen("color")))
        {
            int color;
            if (sscanf(params,"color %d",&color)==EOF)
                new_draw_info_format(NDI_UNIQUE, 0, ob, "Colorsyntax: -<channelname>!color <1-210>");
            else
            {
                if ((color>210) || (color <1))
                    new_draw_info_format(NDI_UNIQUE, 0, ob, "Colorsyntax: -<channelname>!color <1-210>");
                else
                    pl_channel->color=color-1;
            }
        }
        /* we simply map the !mute-command to the command_channel_mute*/
        else if(!strncasecmp(params,"mute",strlen("mute")))
        {
            char    buf3[256];
            sprintf(buf3,"%s %s",pl_channel->channel->name,params+5);
            command_channel_mute(ob, buf3);
            return 1;
        }
        else if (!strncasecmp(params, "mod", strlen("mod")))
        {
            modify_channel_params(pl_channel,params+4);
        }

        /* TODO: Administrativ Commands for Channel Admins */
        return 1;
    }
    else if (mode=='%')   /* change player's shortcut */
    {
        if (strlen(params)==0)
        {
            new_draw_info_format(NDI_UNIQUE, 0, ob, "Current shortcut for channel %s is %c.",pl_channel->channel->name, pl_channel->shortcut);
            return 1;
        }
        if (strlen(params)>1)
        {
            new_draw_info_format(NDI_UNIQUE, 0, ob, "Shortcut can only be one char or number!");
            return 1;
        }
        if ((params[0]=='+') || (params[0]=='-') || (params[0]==' ') || (params[0]=='*') || (params[0]=='%') || (params[0]=='!') || (params[0]==':') || (params[0]=='#'))
        {
            new_draw_info_format(NDI_UNIQUE, 0, ob, "This Shortcut can't be used!");
            return 1;
        }
        pl_channel->shortcut=params[0];
        new_draw_info_format(NDI_UNIQUE, 0, ob, "Shortcut for channel %s changed to %c.",pl_channel->channel->name,pl_channel->shortcut);
        return 1;
    }

    return 1;
}
/**
 * Add Player To a Channel with the player-channel-linklist
 * Checks if player is already on that channel, and so on
 * @param pl     Pointer of player-struct
 * @param name   Shortcut or name (can be abbreviated) of channel player wants to join
 * @param params will be used to hold the pw for pw-protected (clan)channels
 * TODO: pw-channels, lvl-based restrictions, bans, and GM-based restrictions
 */
void addPlayerToChannel(player *pl, char *name, char *params)
{
    char   buf[255];
    struct channels         *channel=NULL;
    struct player_channel   *pl_channel=NULL;


    /* Check if player is already on the channel */
    /* Step 1: check for shortcut */
    pl_channel=getPlChannelFromPlayerShortcut(pl, name);
    if (pl_channel)
    {
        new_draw_info_format(NDI_UNIQUE, 0, pl->ob, "You have already joined the channel %s",pl_channel->channel->name);
        return;
    }

    /* Step 2: ok lets check for channelname */
    for (pl_channel=pl->channels;pl_channel;pl_channel=pl_channel->next_channel)
    {
        if (!strncasecmp(pl_channel->channel->name, name, strlen(name)))
        {
            new_draw_info_format(NDI_UNIQUE, 0, pl->ob, "You have already joined channel %s",pl_channel->channel->name);
            return;
        }
    }

    /* Ok player is not a channel with that shortcut or name, good */
    /* lets try to find the channel, first of course with shortcut */
    channel=getChannelFromGlobalShortcut(pl, name);
    if (!channel) /* No channel with that (default)-shortcut, or no shortcut at all given */
    {             /* We have to find the channel by name, and also make sure its unique */
        channel=findGlobalChannelFromName(pl, name, FALSE); /* error message and non-unique channel name handled in the function, TODO: move Messages outside this function */
        if (!channel)
        {
            return;
        }
    }
    /* NOTE: we don't have to check for enter-restrictions here, its all handled i the different find-functions
     * The channel simply don't exist for that player */


/*TODO: bans, ..... */

    /* ok we have a channel, no ban, no restrictions, lets add the player */
    pl_channel=final_addChannelToPlayer(pl, channel, 0, -1);

    /* ok player is added to channel, give message to player */
    new_draw_info_format(NDI_UNIQUE, 0, pl->ob, "You enter channel %s.",channel->name);

/*TODO: implement DM/DM/VOL inform */

    return;
}

/**
 * Here we try to find a matching channel in the whole channellist
 * If we find more than one channel, we give the player a message
 * with all matching channels
 * @param pl PlayerStruct of Player
 * @param name Name (or Part of name) from channel
 * @param mute, No 'error' output, useful if we implement some channel_kick commands
 * @return pointer to matching channel
 */
struct channels *findGlobalChannelFromName(player *pl, char *name, int mute)
{
    struct channels *c, *tmp;

    c=channel_list_start;
    while (c)
    {
        if (pl->gmaster_mode < GMASTER_MODE_VOL &&
           (c->enter_lvl==-1 || pl->ob->level<c->enter_lvl))
           {
                c=c->next;
                continue;
           }
        if (!strncasecmp(c->name, name, strlen(name)))
            break;
        c=c->next;
    }
    if (!c)
    {
        if (!mute)
        {
            new_draw_info_format(NDI_UNIQUE, 0, pl->ob, "There is no channel with that name.");
        }
        return c;
    }
    for (tmp=c->next;tmp;tmp=tmp->next)
    {
        if (pl->gmaster_mode < GMASTER_MODE_VOL &&
           (tmp->enter_lvl==-1 || pl->ob->level<tmp->enter_lvl))
                continue;
        if (!strncasecmp(tmp->name, name, strlen(name)))
        {
            if (c)
            {
                if (!mute)
                {
                    new_draw_info_format(NDI_UNIQUE, 0, pl->ob, "That channelname was not unique");
                    new_draw_info_format(NDI_UNIQUE, 0, pl->ob, "The following channels match:");
                    new_draw_info_format(NDI_UNIQUE, 0, pl->ob, "%s", c->name);
                }
                c=NULL;
            }
            if (!mute) {new_draw_info_format(NDI_UNIQUE, 0, pl->ob, "%s", tmp->name);}
        }
    }
    return c;
}
/**
 * Here we try to find the channel over the players shortcut
 * @param pl PlayerStruct of Player
 * @param name Shortcut
 * @return pointer to player-channel-linklist
 */
struct player_channel *getPlChannelFromPlayerShortcut(player *pl, char *name)
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
struct channels *getChannelFromGlobalShortcut(player *pl, char *name)
{
    struct channels *channel;

    if (strlen(name)!=1) {return NULL;}
    if (name[0]=='#') {return NULL;}

    for (channel=channel_list_start;channel;channel=channel->next)
    {
        if (pl->gmaster_mode < GMASTER_MODE_VOL &&
           (channel->enter_lvl==-1 || pl->ob->level<channel->enter_lvl))
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
 * @param color color for that channel, or -1 for default color
 * @return pointer to new player-channel-linklist
 */
struct player_channel *final_addChannelToPlayer(player *pl, struct channels *channel, char shortcut, int color)
{
    struct player_channel *node;
    struct player_channel *ptr=NULL, *ptr1=NULL;

    node = (struct player_channel *) malloc(sizeof(struct player_channel));
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
    if (color>-1)
        node->color=color;
    else
        node->color=channel->color;

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

    if (!pl->dm_stealth)
        channel->pl_count++;
    pl->channel_count++;
    return node;
}


void printChannelUsage(object *ob)
{
    new_draw_info_format(NDI_UNIQUE, 0, ob, "Usage:\n       -<channel>[ ][:]<Text>");
    new_draw_info_format(NDI_UNIQUE, 0, ob, "       -<channel>[+-?%]");
    new_draw_info_format(NDI_UNIQUE, 0, ob, "       -[+-?]");
    return;
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
    if(!strcasecmp(tmp,"on") || !strcasecmp(tmp,"off") || !strcasecmp(tmp,"allow") || !strcasecmp(tmp,"channel"))
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
struct player_channel *findPlayerChannelFromName(player *pl, player *wiz, char *name, int mute)
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
            new_draw_info_format(NDI_UNIQUE, 0, wiz->ob, "%s on a channel with that name.",buf);
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
                    new_draw_info_format(NDI_UNIQUE, 0, wiz->ob, "That channelname was not unique");
                    new_draw_info_format(NDI_UNIQUE, 0, wiz->ob, "The following channels match:");
                    new_draw_info_format(NDI_UNIQUE, 0, wiz->ob, "%s", c->channel->name);
                }
                c=NULL;
            }
            if (!mute) {new_draw_info_format(NDI_UNIQUE, 0, wiz->ob, "%s", tmp->channel->name);}
        }
    }
    return c;

}
/**
 * Removes the Player from the channel, frees the player_channel-link
 * @param pl PlayerStruct of Player
 * @param pl_channel player_channel-link for that player/channel
 */
void removeChannelFromPlayer(player *pl, struct player_channel *pl_channel)
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

    if (!pl->dm_stealth)
        pl_channel->channel->pl_count--;
    pl->channel_count--;

    free(pl_channel);

    return;
}
/**
 * Sends a normal message to all player who listens, in their own configured color
 * @param pl PlayerStruct of Sender
 * @param pl_channel player_channel-link for that player/channel
 * @param params Message
 */
void sendChannelMessage(player *pl,struct player_channel *pl_channel, char *params)
{
    struct player_channel *cpl;
    char buf[512]; /* Player commands can only be around 250chars, so with this value, we are on the safe side */
    char prefix[30];
    uint8 color;


    SockList    sl;
    unsigned char slbuf[HUGE_BUF];

    LOG(llevInfo, "CLOG CH:%s:%s >%s<\n", pl_channel->channel->name, pl->ob->name, params);

    /* TODO: channel history */

    sl.buf = slbuf;
    SOCKET_SET_BINARY_CMD(&sl, BINARY_CMD_CHANNELMSG);
    SockList_AddShort(&sl, (NDI_PLAYER | NDI_UNIQUE | NDI_ORANGE | NDI_SHOUT) & NDI_FLAG_MASK);
    sprintf(buf,"%s %s:%s",pl_channel->channel->name, pl->ob->name, params);
    strcpy((char *)sl.buf + sl.len, buf);
    sl.len += strlen(buf);


    for (cpl=pl_channel->channel->players;cpl;cpl=cpl->next_player)
    {
        if (cpl->pl->channels_on)
        {
            color=cpl->color;
            sl.buf[1]=((NDI_PLAYER | NDI_UNIQUE |NDI_SHOUT)>>8)&0xff;
            sl.buf[2]=color;
            Send_With_Handling(&(cpl->pl->socket), &sl);
        }
    }
    return;
}
/**
 * Sends a EMOTE message to all player who listens, in their own configured color
 * @param pl PlayerStruct of Sender
 * @param pl_channel player_channel-link for that player/channel
 * @param params Message
 */
void sendChannelEmote(player *pl,struct player_channel *pl_channel, char *params)
{
    struct player_channel *cpl;
    char buf[512];
    char prefix[30];
    uint8 color;

    SockList    sl;
    unsigned char slbuf[HUGE_BUF];

    /* TODO: channel history */
    LOG(llevInfo, "CLOG CH:%s:%s >%s<\n", pl_channel->channel->name, pl->ob->name, params);

    sl.buf = slbuf;
    SOCKET_SET_BINARY_CMD(&sl, BINARY_CMD_CHANNELMSG);
    SockList_AddShort(&sl, (NDI_PLAYER | NDI_UNIQUE | NDI_ORANGE | NDI_SHOUT | NDI_EMOTE) & NDI_FLAG_MASK);
    sprintf(buf,"%s %s:%s",pl_channel->channel->name, pl->ob->name, params);
    strcpy((char *)sl.buf + sl.len, buf);
    sl.len += strlen(buf);

    for (cpl=pl_channel->channel->players;cpl;cpl=cpl->next_player)
    {
        if (cpl->pl->channels_on)
        {
            color=cpl->color;
            sl.buf[1]=((NDI_PLAYER | NDI_UNIQUE | NDI_SHOUT | NDI_EMOTE)>>8)&0xff;
            sl.buf[2]=color;
            Send_With_Handling(&(cpl->pl->socket), &sl);
        }
    }

    return;
}
/**
 * New Players, or old savefiles...
 * Add the player to the default channels
 * @param pl PlayerStruct of Sender
 */
void addDefaultChannels(player *pl)
{
//    loginAddPlayerToChannel(pl,"Auction",0,-1,0);
    loginAddPlayerToChannel(pl,"Quest",0,-1,0);
    loginAddPlayerToChannel(pl,"General",0,-1,0);
    loginAddPlayerToChannel(pl,"Help",0,-1,0);
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
void loginAddPlayerToChannel(player *pl, char *channelname, char shortcut, int color, unsigned long mute)
{
    /* first lets check if the channel still exists
     * channelname must be an exact match, not that player logs out, with channel
     * 'foo' on, later the channel gets closed/deleted, a new channel 'foobar' exitst
     * and player is at login added to this FALSE one
     */
     struct channels *channel;
     struct player_channel *cpl;

     for (channel=channel_list_start;channel;channel=channel->next)
     {
         if (!strcasecmp(channel->name,channelname))
         {
            cpl=final_addChannelToPlayer(pl, channel, shortcut, color);
            if (mute>0)
                cpl->mute_counter=pticks+mute;
         }
     }

    return;
}
/**
 * Loads the channels from the channel-savefile
 * atm there is no saving, only hardcoded channels
 */
void load_channels()
{
    FILE   *channelfile;
    char    buf[HUGE_BUF];
    char    line_buf[MAX_BUF];
    char    channelname[MAX_CHANNEL_NAME];
    int     channelcolor=-1;
    int     channelenterlevel=1, channelpostlevel=1;
    char    defaultshortcut='#';

    LOG(llevInfo,"loading channel_file....\n");
    sprintf(buf, "%s/channel_file", settings.localdir);
    if ((channelfile = fopen(buf, "r")) == NULL)
    {
        LOG(llevDebug, "Could not find channel_file file Using default channels.\n");
        final_addChannel("Auction",'a',2,2,1);
        final_addChannel("Quest",'q',5,2,1);
        final_addChannel("General",'g',-1,2,1);
        final_addChannel("Help",'h',4,1,1);
        final_addChannel("VOL",'V',3,1,-1);
        return;
    }
    while (fgets(line_buf, 160, channelfile) != NULL)
    {
        if (line_buf[0] == '#')
            continue;

        if (sscanf(line_buf, "%s %c %d %d %d", channelname, &defaultshortcut, &channelcolor, &channelpostlevel, &channelenterlevel) < 5)
            LOG(llevBug, "BUG: malformed channelfile file entry: %s\n", line_buf);
        else
            final_addChannel(channelname,defaultshortcut,channelcolor, channelpostlevel, channelenterlevel);
    }

    fclose(channelfile);

    return;
}
/**
 * Adds a new Channel
 * @param name Name of channel
 * @param chortcut Defaultshortcut of channel/ '#' means no shortcut!
 * @param color default color for that channel / -1 default shout color (orange)
 * @param enter_lvl level for entering (-1 is VOL)
 */
struct channels *final_addChannel(char *name, char shortcut, int color, sint8 post_lvl, sint8 enter_lvl)
{

    struct channels *node;
    struct channels *ptr, *ptr1;

    node = (struct channels *) malloc(sizeof(struct channels));
    strncpy(node->name,name,MAX_CHANNEL_NAME);
    node->name[MAX_CHANNEL_NAME]=0; /* sanity string ending */
    node->shortcut=shortcut;
    node->players=NULL;
    if (color>-1)
        node->color=color;
    else
        node->color=NDI_ORANGE;
    node->pl_count=0;
    node->post_lvl = post_lvl;
    node->enter_lvl = enter_lvl;
    /* First Element of List? */
    if(channel_list_start==NULL)
    {
        channel_list_start=node;
        node->next=NULL;
    }
    else
    {
        ptr=channel_list_start;
        while(ptr != NULL && (strcasecmp(ptr->name,name) < 0))
        {
            ptr1=ptr;
            ptr=ptr->next;
        }
        /* If node 'smaller' as first element, insert at beginning */
        if(ptr==channel_list_start)
        {
            channel_list_start=node;
            node->next=ptr;
        }
        /* last position, or in the middle ptr1 holds the forerunner */
        else
        {
            ptr1->next=node;
            node->next=ptr;
        }
    } //Ende else

    return node;
}
/**
 * called when player leaves the game to remove him completely and free all player_channel-links
 * @param pl Pointer to player struct
 */
void leaveAllChannels(player *pl)
{
    struct player_channel  *node, *tmp=NULL;

    node=pl->channels;
    while (node)
    {
        tmp=node;
        node=node->next_channel;
        removeChannelFromPlayer(pl, tmp);
    }
//    LOG(llevDebug,"channel: leaveAll: %s\n",pl->ob->name);
    return;
}
/**
 * called when DM changes stealth state...
 * @param pl Pointer to player struct
 * @param dm_stealth value of dm_stealth
 */
void channel_dm_stealth(player *pl, int dm_stealth)
{
    struct player_channel *pl_channel;
    for (pl_channel=pl->channels;pl_channel;pl_channel=pl_channel->next_channel)
    {
        if (dm_stealth==1)
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
 */
void    lua_channel_message(char *channelname, char *name, char *message, int mode)
{
    struct channels *channel;
    for (channel=channel_list_start;channel;channel=channel->next)
    {
        if (!strcasecmp(channel->name,channelname)) /* lua: exact name */
        {

            struct player_channel *cpl=NULL;
            char buf[HUGE_BUF];
            char prefix[30];
            uint8 color;


            SockList    sl;
            unsigned char slbuf[HUGE_BUF];

            LOG(llevInfo, "CLOG LUA-CH:%s:%s >%s<\n", channel->name, name, message);

            /* TODO: channel history */

            sl.buf = slbuf;
            SOCKET_SET_BINARY_CMD(&sl, BINARY_CMD_CHANNELMSG);
            if (mode==1)
            {
                SockList_AddShort(&sl, (NDI_PLAYER | NDI_UNIQUE | NDI_ORANGE | NDI_SHOUT | NDI_EMOTE) & NDI_FLAG_MASK);
            }
            else
                SockList_AddShort(&sl, (NDI_PLAYER | NDI_UNIQUE | NDI_ORANGE | NDI_SHOUT) & NDI_FLAG_MASK);

            sprintf(buf,"%s %s:%s",channel->name, name, message);
            strcpy((char *)sl.buf + sl.len, buf);
            sl.len += strlen(buf);


            for (cpl=channel->players;cpl;cpl=cpl->next_player)
            {
                if (cpl->pl->channels_on)
                {
                    color=cpl->color;
                    if (mode==0)
                        sl.buf[1]=((NDI_PLAYER | NDI_UNIQUE |NDI_SHOUT)>>8)&0xff;
                    else
                        sl.buf[1]=((NDI_PLAYER | NDI_UNIQUE |NDI_SHOUT | NDI_EMOTE )>>8)&0xff;

                    sl.buf[2]=color;
                    Send_With_Handling(&(cpl->pl->socket), &sl);
                }
            }
            return;
        }
    }
    if (!channel)
        LOG(llevDebug,"LUA: channelMsg: no channel with that name: %s\n",channelname);
    return;

}



/* save the defined channels
 */
void save_channels(void)
{
    char    filename[MAX_BUF];
    struct channels *channel;
    FILE   *fp;

    LOG(llevSystem,"write channel_file...\n");
    sprintf(filename, "%s/channel_file", settings.localdir);
    if ((fp = fopen(filename, "w")) == NULL)
    {
        LOG(llevBug, "BUG: Cannot open %s for writing\n", filename);
        return;
    }
    fprintf(fp, "# CHANNEL_FILE (file is changed from server at runtime)\n");
    fprintf(fp, "# Take care when editing this file by hand:\n");
    fprintf(fp, "# entry format is:\n# <channelname> <defaultshortcut> <defaultcolor> <postlevel> <enterlevel>\n");

    for (channel=channel_list_start;channel;channel=channel->next)
    {
        fprintf(fp,"%s %c %d %d %d\n",channel->name,channel->shortcut, channel->color,channel->post_lvl,channel->enter_lvl);
    }

    fclose(fp);
}

int command_channel_create(object *ob, char *params)
{
    char    channelname[MAX_CHANNEL_NAME];
    int     channelcolor=-1;
    int     channelenterlevel=1, channelpostlevel=1;
    char    defaultshortcut='#';
    struct channels *channel;

    if (CONTR(ob)->gmaster_mode < GMASTER_MODE_VOL)
        return 0;
    if (!params)
    {
        new_draw_info_format(NDI_UNIQUE, 0, ob, "Syntax: /createchannel <name> <defaultshortcut> <defaultcolor> <postlevel> <enterlevel>");
        new_draw_info_format(NDI_UNIQUE, 0, ob, "set defaultshortcut to '#' for no defaultshortcut");
        new_draw_info_format(NDI_UNIQUE, 0, ob, "set defaultcolor to '-1' for no defaultcolor");
        return 1;
    }

    if (sscanf(params, "%s %c %d %d %d", channelname, &defaultshortcut, &channelcolor, &channelpostlevel, &channelenterlevel) < 5)
    {
        new_draw_info_format(NDI_UNIQUE, 0, ob, "Syntax: /createchannel <name> <defaultshortcut> <defaultcolor> <postlevel> <enterlevel>");
        new_draw_info_format(NDI_UNIQUE, 0, ob, "set defaultshortcut to '#' for no defaultshortcut");
        new_draw_info_format(NDI_UNIQUE, 0, ob, "set defaultcolor to '-1' for no defaultcolor");
        return 1;
    }
    else
    {
        if (strlen(channelname)>MAX_CHANNEL_NAME)
        {
            new_draw_info_format(NDI_UNIQUE, 0, ob, "Channelname is too long!");
            return 1;
        }
        if (!channelname_ok(channelname))
        {
            new_draw_info_format(NDI_UNIQUE, 0, ob, "This Channelname is not allowed!");
            return 1;
        }
        if (strlen(channelname)<3)
        {
            new_draw_info_format(NDI_UNIQUE, 0, ob, "Channelname must be at least 3 chars long!");
            return 1;
        }

        for (channel=channel_list_start;channel;channel=channel->next)
        {
            if (!strcasecmp(channel->name,channelname) || ((channel->shortcut==defaultshortcut) && (defaultshortcut!='#')))
            {
                new_draw_info_format(NDI_UNIQUE, 0, ob, "Channel with that name or shortcut already exists!");
                return 1;
            }
        }
        final_addChannel(channelname,defaultshortcut,channelcolor, channelpostlevel, channelenterlevel);
        new_draw_info_format(NDI_UNIQUE, 0, ob, "Channel %s is created.",channelname);
        LOG(llevInfo, "CLOG Create:>%s<: %s, %d, %d\n", ob->name, channelname, channelpostlevel, channelenterlevel);
        save_channels();
    }

    return 1;

}

int command_channel_mute(object *ob, char *params)
{
    struct channels *channel=NULL;
    struct player_channel *cpl;

    char channelname[256]="";
    char playername[256]="";
    int seconds=0;

    if (CONTR(ob)->gmaster_mode < GMASTER_MODE_VOL)
        return 0;
    if (!params)
    {
        new_draw_info_format(NDI_UNIQUE, 0, ob, "Syntax: /channemute <channel> <who> <howmuch>");
        return 1;
    }

    sscanf(params, "%s %s %d", channelname, playername, &seconds);

    if(seconds<0)
    {
        new_draw_info_format(NDI_UNIQUE, 0, ob, "/channelmute command: illegal seconds parameter (%d)", seconds);
        return 0;
    }

    for (channel=channel_list_start;channel;channel=channel->next)
    {
        if (!strcasecmp(channel->name,channelname))
        {
            break;
        }
    }
    if (!channel)
    {
        new_draw_info_format(NDI_UNIQUE, 0, ob, "No channel with that name.");
        return 0;
    }

    for (cpl=channel->players;cpl;cpl=cpl->next_player)
    {
        if (!strcasecmp(cpl->pl->ob->name,playername))
            break;
    }

    if (!cpl)
    {
        new_draw_info_format(NDI_UNIQUE, 0, ob, "%s is not on that channel.",playername);
        return 0;
    }
    /* ok now we have all checks done... i hate it that we have to do such an effort in coding for only some assholes */

    if(!seconds) /* unmute player */
    {
        new_draw_info_format(NDI_UNIQUE, 0, ob, "/channelmute command: umuting player %s on channel %s!", playername, channelname);
        cpl->mute_counter = 0;
    }
    else
    {
        new_draw_info_format(NDI_UNIQUE, 0, ob, "/channelmute command: mute player %s for %d seconds on channel %s!", playername, seconds, channelname);
        cpl->mute_counter = pticks+seconds*(1000000/MAX_TIME);
    }

    return 1;


}

int command_channel_delete(object *ob, char *params)
{
    struct channels *channel=NULL, *ch_ptr1=NULL;;
    struct player_channel *cpl;

    if (CONTR(ob)->gmaster_mode < GMASTER_MODE_VOL)
        return 0;
    if (!params)
    {
        new_draw_info_format(NDI_UNIQUE, 0, ob, "Syntax: /deletechannel <name>");
        return 1;
    }
    for (channel=channel_list_start;channel;channel=channel->next)
    {
        if (!strcasecmp(channel->name,params))
        {
            break;
        }
    }
    if (!channel)
    {
        new_draw_info_format(NDI_UNIQUE, 0, ob, "No channel with that name.");
        return 1;
    }

    for (cpl=channel->players;cpl;cpl=cpl->next_player)
    {
        new_draw_info_format(NDI_UNIQUE, 0, cpl->pl->ob, "Channel '%s' is now closed!",channel->name);
        removeChannelFromPlayer(cpl->pl, cpl);
    }
    for (ch_ptr1=channel_list_start;ch_ptr1;ch_ptr1=ch_ptr1->next)
    {
        if (channel==ch_ptr1->next)
        {
           ch_ptr1->next=channel->next;
           new_draw_info_format(NDI_UNIQUE, 0, ob, "Channel '%s' deleted.",channel->name);
           free(channel);
           LOG(llevInfo, "CLOG Delete:>%s<: %s\n", ob->name, params);
           save_channels();
           return 1;
        }
    }

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

                new_draw_info_format( NDI_UNIQUE, 0, cpl->pl->ob, "You are still muted for %d second(s).", (int)(tmp?tmp:1));
                cpl->pl->mute_msg_count = pticks+MUTE_MSG_FREQ;
            }
            return FALSE;
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

                new_draw_info_format( NDI_UNIQUE, 0, cpl->pl->ob, "You are still muted for %d second(s) on this channel.", (int)(tmp?tmp:1));
                cpl->mute_msg_count = pticks+MUTE_MSG_FREQ;
            }
            return FALSE;
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
                new_draw_info( NDI_UNIQUE, 0, cpl->pl->ob, "Please wait 2 seconds between messages on the same channel.");
                cpl->mute_flags |= MUTE_FLAG_SHOUT;
                return FALSE;
            }
            else if(!(cpl->mute_flags & MUTE_FLAG_SHOUT_WARNING)) /* first & last warning */
            {
                new_draw_info( NDI_UNIQUE|NDI_ORANGE, 0, cpl->pl->ob, "Auto-Mute Warning: Please wait 2 seconds!");
                cpl->mute_flags |= MUTE_FLAG_SHOUT_WARNING;
                return FALSE;
            }
            else /* mute him */
            {
                new_draw_info_format( NDI_UNIQUE|NDI_RED, 0, cpl->pl->ob, "Auto-Mute: Don't spam! You are muted for %d seconds!",(int)(MUTE_AUTO_NORMAL/(1000000/MAX_TIME)));
                cpl->mute_counter = pticks+MUTE_AUTO_NORMAL;
                return FALSE;
            }
        }
    }
    return TRUE;
}

void modify_channel_params(struct player_channel *cpl, char *params)
{
    int enter_lvl=0;
    int post_lvl=0;

    if (cpl->pl->gmaster_mode < GMASTER_MODE_VOL)
        return;
    if (!params)
    {
        new_draw_info_format(NDI_UNIQUE, 0, cpl->pl->ob, "Syntax: -<channel>!mod <postlevel> <enterlevel>");
        return;
    }

    sscanf(params, "%d %d", &post_lvl, &enter_lvl);

    if (post_lvl>0)
        cpl->channel->post_lvl=post_lvl;

    if (enter_lvl>0)
        cpl->channel->enter_lvl=enter_lvl;

    new_draw_info_format(NDI_UNIQUE, 0, cpl->pl->ob, "Channel: %s - postlvl: %d, enterlvl: %d",cpl->channel->name, cpl->channel->post_lvl,cpl->channel->enter_lvl);

    save_channels();

    return;

}



#endif



