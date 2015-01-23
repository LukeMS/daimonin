/*
    Daimonin SDL client, a client program for the Daimonin MMORPG.

    Copyright (C) 2003-2006 Michael Toennies
    Channelsystem by Alderan

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
#ifdef USE_CHANNELS

#ifndef __CHANNEL_H
#define __CHANNEL_H


#define MAX_CHANNEL_NAME 12
#define MAX_CHANNEL_HIST_CHAR 256 /* its enough */
#define MAX_CHANNEL_HIST_LINES 50
#define CHANNEL_HIST

/* Names of standard channels */
#define CHANNEL_NAME_VOL "VOL"
#define CHANNEL_NAME_GM "GM"
#define CHANNEL_NAME_MW "MW"
#define CHANNEL_NAME_MM "MM"
#define CHANNEL_NAME_SA "SA"


/**
 * Channel-Player doubled-links list.
 * For each player on a channel we have such a node.
 * With the next_player chain we get all players on one channel,
 * with the next_channel chain we get all channels the player is on.
 */
typedef struct player_channel
{
    struct  player_channel  *next_channel;  /* Next chanel player is on */
    struct  player_channel  *next_player;   /* Next player which is on that channel */
    struct  channels        *channel;       /* pointer to channel player is listening */
            player_t          *pl;            /* pointer back to player struct */
    char                     shortcut;      /* user defined shortcut, overrules global channel shortcut */
    unsigned long            mute_freq;
    unsigned long            mute_counter;
    unsigned long            mute_msg_count;
    uint32                   mute_flags;
    int                      moderator;     // Whether or not the player is a moderator of this channel.

} _player_channel;

/**
 * The Main Channel-List
 * here we find name, pointer to all players that are 'on'
 * later we can add here flags as 'moderated' channel, or even restricted channels (for GM only,
 * or for clan channels..., a second list for moderators, admin of this channel, ban list....)
 * if a player enters sucessful this channel he will be add to the players list.
 */
typedef struct channels
{
    struct channels         *next;                      /* next channel */
    char                     name[MAX_CHANNEL_NAME+1];  /* name of channel */
    struct player_channel   *players;                   /* pointer to player_channel linklist */
    char                     shortcut;                  /* default shortcut, at entering will be copyed to pl_channel_list struct */
    int                      color;                     /* default color */
    int                      pl_count;                  /* player count */
    sint8                    post_lvl;                  /* lvl required to send message (VOLs, GMs, and SAs are exempt) */
    sint8                    enter_lvl;                 /* lvl required to enter (VOLs, GMs, and SAs are exempt) */
    int                      gmaster_mode;              /* gmaster_mode required to enter */
#ifdef CHANNEL_HIST
    char                     history[MAX_CHANNEL_HIST_LINES][MAX_CHANNEL_HIST_CHAR];
    uint8                    lines;
    uint8                    startline;
#endif
} _channels;

struct channels         *findGlobalChannelFromName(player_t *pl, char *name, int mute);
struct channels         *getChannelFromGlobalShortcut(player_t *pl, char *name);

struct player_channel   *getPlChannelFromPlayerShortcut(player_t *pl, char *name);
struct player_channel   *findPlayerChannelFromName(player_t *pl, player_t *wiz, char *name, int mute);

struct player_channel   *final_addChannelToPlayer(player_t *pl, struct channels *channel, char shortcut);
void                    addPlayerToChannel(player_t *pl, char *name, char *params);
void                    loginAddPlayerToChannel(player_t *pl, char *channelname, char shortcut, unsigned long mute);

struct channels    *final_addChannel(char *name, char shortcut, int color, sint8 post_lvl, sint8 enter_lvl, int gmaster_mode);
void    addDefaultChannels(player_t *pl);
void    load_channels(void);
void    save_channels(void);
void    leaveAllChannels(player_t *pl);

int     channelname_ok(char *cp);

void    removeChannelFromPlayer(player_t *pl, struct player_channel *pl_channel, char *msg);

void    sendChannelMessage(player_t *pl, struct channels *channel, char *params);
void    sendChannelEmote(player_t *pl, struct channels *channel, char *params);
#ifdef CHANNEL_HIST
void    addChannelHist(struct channels *channel, const char *name, char *msg, int mode);
void    sendChannelHist(struct player_channel *cpl, int lines);
#endif



int     command_channel(object_t *ob, char *params);
int     command_channel_create(object_t *ob, char *params);
int     command_channel_delete(object_t *ob, char *params);
int     command_channel_mute(object_t *ob, char *params);


void    channel_privacy(player_t *pl, int privacy);

int     check_channel_mute(struct player_channel *cpl);
void    modify_channel_params(struct player_channel *cpl, char *params);
void    forceAddPlayerToChannel(struct player_channel *cpl, char *params);
void    kickPlayerFromChannel(struct player_channel *cpl, char *params);

int     lua_channel_message(char *channelname, const char *name, char *message, int mode);


void sendVirtualChannelMsg(player_t *sender, char *channelname, player_t *target, char* msg, int color);

#endif
#endif /* ifndef __CHANNEL_H */
