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
#include <ctype.h>

/* binary command tags are defined in the shared protocol.h
 * first value is data tail length:
 * 0 = no data, single command
 * -1 = dynamic data tail, firs 2 bytes of tail describe tail length-2
 * x = length of data tail in bytes
 */
_CmdMapping cs_commands[]    =
{
    {-1,        cs_cmd_ping},
    {-1,        cs_cmd_setup},
    {1,         cs_cmd_file},
    {-1,        cs_cmd_checkname},
    {-1,        cs_cmd_login},
    {-1,        cs_cmd_newchar},
    {-1,        cs_cmd_delchar},
    {-1,        cs_cmd_addme},
    {-1,        cs_cmd_face},
    {2,         cs_cmd_move},
    {4,         cs_cmd_apply},
    {4,         cs_cmd_examine},
    {12,        cs_cmd_moveobj},
    {-1,        cs_cmd_talk},
    {5,         cs_cmd_lock},
    {4,         cs_cmd_mark},
    {-1,        cs_cmd_fire},
    {-1,        cs_cmd_generic},

    {1,         NULL}   /* terminator */
};

/*
 * Normal game commands
 */
CommArray_s Commands[] =
{
    {"target",        command_target,         0.1f, 0, NULL}, /* enter combat and attack object in reach */
    {"combat",        command_combat,         0.1f, 0, NULL}, /* toggle attack mode of player */
    {"rest",          command_resting,        1.0f, 1, NULL}, /* enter resting mode */
	 {"run",           command_run,            1.0f, 0, NULL}, /* starts running*/
    {"run_stop",      command_run_stop,       0.0f, 0, NULL}, /* stops running*/
    {"cast",          command_cast_spell,     0.0f, 1, NULL}, /* use time comes from spells! */
    {"push",          command_push_object,    1.0f, 1, NULL}, /* move an object you are facing if movable */
    {"right",         command_turn_right,     1.0f, 0, NULL}, /* player rotates clockwise on same tile*/
    {"left",          command_turn_left,      1.0f, 0, NULL}, /* player rotates anticlockwise on same tile*/
    {"say",           command_say,            1.0f, 0, NULL}, /* local chat between players on the map*/
    {"gsay",          command_gsay,           1.0f, 0, NULL}, /* communicates between members of a group*/
    {"shout",         command_shout,          1.0f, 0, NULL}, /* global communication on /channel general*/
#ifndef USE_CHANNELS
    {"describe",      command_describe,       1.0f, 0, NULL}, /* describes a marked item in inventory */
#endif
    {"tell",          command_tell,           1.0f, 0, NULL}, /* sends message to specified player */
    {"who",           command_who,            0.0f, 1, NULL}, /* lists all players online*/
    {"mapinfo",       command_mapinfo,        0.0f, 1, NULL}, /* gives map information for map player is one*/
    {"motd",          command_motd,           0.0f, 0, NULL}, /* displays message of the day*/
    {"time",          command_time,           1.0f, 0, NULL}, /* displays in game time*/
    {"help",          command_help,           0.0f, 0, NULL}, /* displays help topics*/
    {"use_skill",     command_uskill,         0.1f, 1, NULL}, /* uses a specified skill*/
    {"ready_skill",   command_rskill,         0.1f, 1, NULL}, /* readies a specified skill*/
    {"privacy",       command_privacy,        0.0f, 1, NULL}, /* enters player in privacy mode*/
    {"egobind",       command_egobind,        1.0f, 1, NULL}, /* egobinds an item to the player */
    {"invite",        command_party_invite,   4.0f, 1, NULL}, /* invites specified player to join group*/
    {"join",          command_party_join,     0.1f, 1, NULL}, /* join a group when invited*/
    {"deny",          command_party_deny,     0.1f, 1, NULL}, /* deny joining a group when invited*/
    {"leave",         command_party_leave,    4.0f, 1, NULL}, /* leave a group*/
    {"remove",        command_party_remove,   4.0f, 1, NULL}, /* remove a specified player from a group*/
    {"pvp",           command_pvp_stats,      0.0f, 0, NULL}, /* lists pvp stats of specified player */
	{"level",         command_level,          0.0f, 0, NULL}, /* states specified player's level*/
#ifdef USE_CHANNELS
    {"channel",       command_channel,        1.0f, 0, NULL}, /* channel system */
#endif
#ifdef DAI_DEVELOPMENT_CONTENT
    {"stuck",         command_stuck,          0.0f, 1, NULL}, /*Moves player to default location*/
#endif
    {"vol",           command_vol,            0.0f, 1, NULL}, /*enters vol mode*/
    {"gm",            command_gm,             0.0f, 1, NULL}, /*enters gm mode*/
    {"mw",            command_mw,             0.0f, 1, NULL}, /*enters mw mode*/
    {"mm",            command_mm,             0.0f, 1, NULL}, /*enters mm mode*/
    {"sa",            command_sa,             0.0f, 1, NULL}, /*enters sa mode*/
    {"gmasterlist",   command_gmasterlist,    0.0f, 1, NULL}, /*lists accounts on gmasterfile*/
    {"password",      command_password,       0.0f, 1, NULL}, /* change pwd */
};

CommArray_s EmoteCommands[] =
{
    {"nod",     command_nod,     1.0f, 0, NULL},
    {"bounce",  command_bounce,  1.0f, 0, NULL},
    {"laugh",   command_laugh,   1.0f, 0, NULL},
    {"puke",    command_puke,    1.0f, 0, NULL},
    {"sigh",    command_sigh,    1.0f, 0, NULL},
    {"cry",     command_cry,     1.0f, 0, NULL},
    {"grin",    command_grin,    1.0f, 0, NULL},
    {"blush",   command_blush,   1.0f, 0, NULL},
    {"cough",   command_cough,   1.0f, 0, NULL},
    {"gasp",    command_gasp,    1.0f, 0, NULL},
    {"hiccup",  command_hiccup,  1.0f, 0, NULL},
    {"shiver",  command_shiver,  1.0f, 0, NULL},
    {"smirk",   command_smirk,   1.0f, 0, NULL},
    {"snicker", command_snicker, 1.0f, 0, NULL},
    {"spit",    command_spit,    1.0f, 0, NULL},
    {"twiddle", command_twiddle, 1.0f, 0, NULL},
    {"wink",    command_wink,    1.0f, 0, NULL},
    {"bleed",   command_bleed,   1.0f, 0, NULL},
    {"me",      command_me,      1.0f, 0, NULL},
    {"dance",   command_dance,   1.0f, 0, NULL},
    {"smile",   command_smile,   1.0f, 0, NULL},
    {"giggle",  command_giggle,  1.0f, 0, NULL},
    {"growl",   command_growl,   1.0f, 0, NULL},
    {"sulk",    command_sulk,    1.0f, 0, NULL},
    {"poke",    command_poke,    1.0f, 0, NULL},
    {"bow",     command_bow,     1.0f, 0, NULL},
    {"burp",    command_burp,    1.0f, 0, NULL},
    {"flip",    command_flip,    1.0f, 0, NULL},
    {"glare",   command_glare,   1.0f, 0, NULL},
    {"lick",    command_lick,    1.0f, 0, NULL},
    {"shrug",   command_shrug,   1.0f, 0, NULL},
    {"snap",    command_snap,    1.0f, 0, NULL},
    {"sniff",   command_sniff,   1.0f, 0, NULL},
    {"strut",   command_strut,   1.0f, 0, NULL},
    {"wave",    command_wave,    1.0f, 0, NULL},
    {"yawn",    command_yawn,    1.0f, 0, NULL},
    {"cringe",  command_cringe,  1.0f, 0, NULL},
    {"kiss",    command_kiss,    1.0f, 0, NULL},
    {"cackle",  command_cackle,  1.0f, 0, NULL},
    {"shake",   command_shake,   1.0f, 0, NULL},
    {"scream",  command_scream,  1.0f, 0, NULL},
    {"hug",     command_hug,     1.0f, 0, NULL},
    {"accuse",  command_accuse,  1.0f, 0, NULL},
    {"clap",    command_clap,    1.0f, 0, NULL},
    {"chuckle", command_chuckle, 1.0f, 0, NULL},
    {"frown",   command_frown,   1.0f, 0, NULL},
    {"groan",   command_groan,   1.0f, 0, NULL},
    {"pout",    command_pout,    1.0f, 0, NULL},
    {"slap",    command_slap,    1.0f, 0, NULL},
    {"sneeze",  command_sneeze,  1.0f, 0, NULL},
    {"snore",   command_snore,   1.0f, 0, NULL},
    {"thank",   command_thank,   1.0f, 0, NULL},
    {"whistle", command_whistle, 1.0f, 0, NULL},
    {"beg",     command_beg,     1.0f, 0, NULL},
    {"think",   command_think,   1.0f, 0, NULL},
};

CommArray_s CommandsVOL[] =
{
    {"mutelevel", command_mutelevel, 0.0f, 1, CHANNEL_NAME_VOL},
	{"ban",       command_ban,       0.0f, 1, CHANNEL_NAME_VOL},
    {"kick",      command_kick,      0.0f, 1, CHANNEL_NAME_VOL},
    {"mute",      command_mute,      0.0f, 1, CHANNEL_NAME_VOL},
    {"ip",        command_ip,        0.0f, 1, NULL},
#ifdef USE_CHANNELS
    {"createchannel", command_channel_create, 0.0f, 1, CHANNEL_NAME_VOL}, /* channel system */
    {"deletechannel", command_channel_delete, 0.0f, 1, CHANNEL_NAME_VOL}, /* channel system */
    {"channelmute",   command_channel_mute,   0.0f, 1, CHANNEL_NAME_VOL}, /* channel system */
#endif
};

CommArray_s CommandsGM[] =
{
#ifdef DAI_DEVELOPMENT_CONTENT
    {"generate",       command_generate,       0.0f, 1, CHANNEL_NAME_GM},
    {"spawn",          command_spawn,          0.0f, 1, CHANNEL_NAME_GM},
    {"listarch",       command_listarch,       0.0f, 1, NULL},
#endif
    {"connections",    command_connections,    0.0f, 1, CHANNEL_NAME_GM},
    {"inventory",      command_inventory,      0.0f, 1, NULL},
    {"summon",         command_summon,         0.0f, 1, CHANNEL_NAME_GM},
    {"teleport",       command_teleport,       0.0f, 1, NULL},
    {"silence",        command_silence,        0.0f, 1, CHANNEL_NAME_GM},
    {"gmasterfile",    command_gmasterfile,    0.0f, 1, CHANNEL_NAME_GM},
    {"stats",          command_stats,          0.0f, 1, NULL},
    {"invisibility",   command_invisibility,   0.0f, 1, NULL},
};

CommArray_s CommandsMW[] =
{
#ifdef DAI_DEVELOPMENT_CONTENT
    {"summon",        command_summon,      0.0f, 1, CHANNEL_NAME_MW},
    {"addexp",        command_addexp,      0.0f, 1, CHANNEL_NAME_MW},
    {"setskill",      command_setskill,    0.0f, 1, CHANNEL_NAME_MW},
    {"setstat",       command_setstat,     0.0f, 1, CHANNEL_NAME_MW},
    {"wizpass",       command_wizpass,     0.0f, 1, NULL},
    {"matrix",        command_matrix,      0.0f, 1, NULL},
    {"stealth",       command_stealth,     0.0f, 1, NULL},
    {"teleport",      command_teleport,    0.0f, 1, NULL},
    {"resetmap",      command_resetmap,    0.0f, 1, CHANNEL_NAME_MW},
    {"goto",          command_goto,        0.0f, 1, NULL},
    {"reboot",        command_reboot,      0.0f, 1, CHANNEL_NAME_MW},
    {"dm_dev",        command_dm_dev,      0.0f, 1, NULL},
    {"dm_light",      command_dm_light,    0.0f, 1, NULL},
    {"set_map_light", command_setmaplight, 0.0f, 1, NULL},
    {"generate",      command_generate,    0.0f, 1, CHANNEL_NAME_MW},
    {"spawn",         command_spawn,       0.0f, 1, CHANNEL_NAME_MW},
    {"listarch",      command_listarch,    0.0f, 1, NULL},
#endif
    {"mspinfo",       command_mspinfo,     0.0f, 1, NULL},
};

CommArray_s CommandsMM[] =
{
#ifdef DAI_DEVELOPMENT_CONTENT
    {"serverspeed",   command_serverspeed, 0.0f, 1, CHANNEL_NAME_MM},
#else
    {"wizpass",       command_wizpass,     0.0f, 1, NULL},
    {"matrix",        command_matrix,      0.0f, 1, NULL},
    {"stealth",       command_stealth,     0.0f, 1, NULL},
    {"teleport",      command_teleport,    0.0f, 1, NULL},
    {"resetmap",      command_resetmap,    0.0f, 1, CHANNEL_NAME_MM},
    {"goto",          command_goto,        0.0f, 1, NULL},
    {"reboot",        command_reboot,      0.0f, 1, CHANNEL_NAME_MM},
    {"dm_dev",        command_dm_dev,      0.0f, 1, NULL},
    {"dm_light",      command_dm_light,    0.0f, 1, NULL},
    {"set_map_light", command_setmaplight, 0.0f, 1, NULL},
#endif
    {"gmasterfile",   command_gmasterfile, 0.0f, 1, CHANNEL_NAME_MM},
};

CommArray_s CommandsSA[] =
{
#ifndef DAI_DEVELOPMENT_CONTENT
    {"serverspeed",  command_serverspeed,       0.0f, 1, CHANNEL_NAME_SA},
    {"addexp",       command_addexp,            0.0f, 1, CHANNEL_NAME_SA},
    {"setskill",     command_setskill,          0.0f, 1, CHANNEL_NAME_SA},
    {"setstat",      command_setstat,           0.0f, 1, CHANNEL_NAME_SA},
#endif
    {"create",       command_create,            0.0f, 1, CHANNEL_NAME_SA},
    {"generate",     command_generate,          0.0f, 1, CHANNEL_NAME_SA},
    {"spawn",        command_spawn,             0.0f, 1, CHANNEL_NAME_SA},
    {"listarch",     command_listarch,          0.0f, 1, NULL},
    {"plugin",       command_loadplugin,        0.0f, 1, CHANNEL_NAME_SA},
    {"pluglist",     command_listplugins,       0.0f, 1, NULL},
    {"plugout",      command_unloadplugin,      0.0f, 1, CHANNEL_NAME_SA},
    {"dump",         command_dump,              0.0f, 1, CHANNEL_NAME_SA},
    {"d_active",     command_dumpactivelist,    0.0f, 1, CHANNEL_NAME_SA},
    {"d_arches",     command_dumpallarchetypes, 0.0f, 1, CHANNEL_NAME_SA},
    {"d_objects",    command_dumpallobjects,    0.0f, 1, CHANNEL_NAME_SA},
    {"d_belowfull",  command_dumpbelowfull,     0.0f, 1, CHANNEL_NAME_SA},
    {"d_below",      command_dumpbelow,         0.0f, 1, CHANNEL_NAME_SA},
    {"d_hash",       command_sstable,           0.0f, 1, CHANNEL_NAME_SA},
    {"check_fd",     command_check_fd,          0.0f, 1, CHANNEL_NAME_SA},
    {"malloc",       command_malloc,            0.0f, 1, CHANNEL_NAME_SA},

    /* old, outdated or disabled commands */
    /*
    {"listen", command_listen,    0.0}, // our channel system should work different
    {"get", command_take,     1.0},
    {"apply",         command_apply,          1.0f, 1}, // should be variable
    {"examine", command_examine,  0.5}, // should work in direction
    {"statistics", command_statistics,    0.0}, // will be send to client later in status
    {"archs", command_archs, 0.0},
    {"debug", command_debug,0.0},
    {"fix_me", command_fix_me,   0.0},
    {"free", command_free,0.0},
    {"logs", command_logs,   0.0},
    {"patch", command_patch,0.0},
    {"resistances", command_resistances, 0.0},
    {"remove", command_remove,0.0},
    {"set_god", command_setgod, 0.0},
    {"spellreset", command_spell_reset,0.0},
    {"style_info", command_style_map_info, 0.0},
    */
};

/* sort the commands for faster access */
const int CommandsSize = sizeof(Commands) / sizeof(CommArray_s);
const int EmoteCommandsSize = sizeof(EmoteCommands) / sizeof(CommArray_s);
const int CommandsVOLSize = sizeof(CommandsVOL) / sizeof(CommArray_s);
const int CommandsGMSize = sizeof(CommandsGM) / sizeof(CommArray_s);
const int CommandsMWSize = sizeof(CommandsMW) / sizeof(CommArray_s);
const int CommandsMMSize = sizeof(CommandsMM) / sizeof(CommArray_s);
const int CommandsSASize = sizeof(CommandsSA) / sizeof(CommArray_s);

_subcommand subcommands;

static int compare_A(const void *a, const void *b)
{
    return strcmp(((CommArray_s *) a)->name, ((CommArray_s *) b)->name);
}

void init_commands()
{
    qsort((char *)Commands, CommandsSize, sizeof(CommArray_s), compare_A);
    qsort((char *)EmoteCommands, EmoteCommandsSize, sizeof(CommArray_s), compare_A);
    qsort((char *)CommandsVOL, CommandsVOLSize, sizeof(CommArray_s), compare_A);
    qsort((char *)CommandsGM, CommandsGMSize, sizeof(CommArray_s), compare_A);
    qsort((char *)CommandsMW, CommandsMWSize, sizeof(CommArray_s), compare_A);
    qsort((char *)CommandsMM, CommandsMMSize, sizeof(CommArray_s), compare_A);
    qsort((char *)CommandsSA, CommandsSASize, sizeof(CommArray_s), compare_A);

    subcommands.add = add_string("add");
    subcommands.cancel = add_string("cancel");
    subcommands.list = add_string("list");
    subcommands.remove = add_string("remove");
    subcommands.verbose = add_string("verbose");
    subcommands.restart = add_string("restart");
    subcommands.shutdown = add_string("shutdown");
    subcommands.showtime = add_string("showtime");
    subcommands.showdate = add_string("showdate");
    subcommands.showseason = add_string("showseason");
}

/* Finds cmd if it exists for pl (determined by gmaster_mode).
 * If so, the particular command element is returned. Otherwise, NULL is
 * returned. */
CommArray_s *find_command(char *cmd, player_t *pl)
{
    CommArray_s *csp,
                 plugin_csp;

    if (find_plugin_command(cmd, (pl) ? pl->ob : NULL, &plugin_csp))
    {
        csp = &plugin_csp;

        return csp;
    }
    else if((csp = find_command_element(cmd, Commands, CommandsSize)))
        return csp;
    else if ((csp = find_command_element(cmd, EmoteCommands, EmoteCommandsSize)))
        return csp;
    else if ((!pl ||
              compare_gmaster_mode(GMASTER_MODE_VOL, pl->gmaster_mode)) &&
             (csp = find_command_element(cmd, CommandsVOL, CommandsVOLSize)))
        return csp;
    else if ((!pl ||
              compare_gmaster_mode(GMASTER_MODE_GM, pl->gmaster_mode)) &&
             (csp = find_command_element(cmd, CommandsGM, CommandsGMSize)))
        return csp;
    else if ((!pl ||
              compare_gmaster_mode(GMASTER_MODE_MW, pl->gmaster_mode)) &&
             (csp = find_command_element(cmd, CommandsMW, CommandsMWSize)))
        return csp;
    else if ((!pl ||
              compare_gmaster_mode(GMASTER_MODE_MM, pl->gmaster_mode)) &&
             (csp = find_command_element(cmd, CommandsMM, CommandsMMSize)))
        return csp;
    else if ((!pl ||
              compare_gmaster_mode(GMASTER_MODE_SA, pl->gmaster_mode)) &&
             (csp = find_command_element(cmd, CommandsSA, CommandsSASize)))
        return csp;

    return NULL;
}

CommArray_s *find_command_element(char *cmd, CommArray_s *commarray, int commsize)
{
    CommArray_s    *asp, dummy;
    char           *cp;

    for (cp = cmd; *cp; cp++)
        *cp = tolower(*cp);

    dummy.name = cmd;

    asp = (CommArray_s *)bsearch((void *)&dummy, (void *)commarray, commsize,
                                 sizeof(CommArray_s), compare_A);

    return asp;
}

/* We go through the list of queued commands we got from the client */
void process_command_queue(NewSocket *ns, player_t *pl)
{
    int cmd, cmd_count = 0;

    /* do some sanity checks ... we only allow a full enabled player to work out commands */
    if (ns->status != Ns_Playing ||
        !pl ||
        !(pl->state & ST_PLAYING) ||
        !pl->ob ||
        pl->ob->speed_left < 0.0)
    {
        return;
    }

    if (ns->cmd_start)
    {
        /* reset inactivity counter */
        ns->inactive_when = ROUND_TAG + INACTIVE_PLAYER1 * pticks_second;
        ns->inactive_flag = 0;

        /* Loop through this - maybe we have several complete packets here. */
        do
        {
#ifdef DEBUG_PROCESS_QUEUE
            LOG(llevDebug, "process_command_queue: Found cmdptr:%p . Cmd: %d\n",
                ns->cmd_start, ns->cmd_start->cmd);

#endif
            /* all commands we have was pre-processed, so we can be sure they are valid for active, playing Player. */
            /* well, some last sanity tests */
            if ((cmd = ns->cmd_start->cmd) < 0 ||
                cmd >= CLIENT_CMD_MAX_NROF)
            {
                LOG(llevDebug, "HACKBUG: Bad command from client (%d) cmd:(%d)\n",
                    ns->fd, cmd);
                ns->status = Ns_Dead;
                return;
            }

            /* simple and fast: we call the cmd function in binary style */
            cs_commands[cmd].cmdproc(ns->cmd_start->buf, ns->cmd_start->len, ns);
            /* and remove the command from the queue */
            command_buffer_clear(ns);

            /* have we to stop or one more command? */
            if (cmd_count++ >= 8 ||
                ns->status != Ns_Playing ||
                !pl ||
                !(pl->state & ST_PLAYING) ||
                !pl->ob ||
                pl->ob->speed_left < 0.0)
            {
                return;
            }
        }
        while (ns->cmd_start);
    }
}

void cs_cmd_ping(char *buf, int len, NewSocket *ns)
{
    char          *cp,
                   buf_reply[LARGE_BUF];
    int            len_reply;
    unsigned long  tick_ping,
                   tick_reply;

    if (ns->setup ||
        ns->status != Ns_Login ||
        !buf ||
        !len)
    {
        LOG(llevInfo, "HACKBUG:: Received illegal ping from IP >%s<!\n",
            STRING_SAFE(ns->ip_host));
        ns->status = Ns_Dead;

        return;
    }

    tick_ping = strtoul(buf, NULL, 16);
    cp = get_online_players_info(NULL, NULL, 0);
    tick_reply = strtoul(cp, NULL, 16);
    len_reply = sprintf(buf_reply, "%s", (tick_reply == tick_ping) ? "" : cp);
    SOCKBUF_REQUEST_BUFFER(ns, len_reply + 1);
    SockBuf_AddString(ACTIVE_SOCKBUF(ns), buf_reply, len_reply);
    SOCKBUF_REQUEST_FINISH(ns, SERVER_CMD_PING, SOCKBUF_DYNAMIC);
}

/* This command handles slash game commands like /say, /tell or /dm
* As a protocol level command, it works as transportation level for
* the "real" game commands.
* NOTE: A game command is always a string, if a command deals with real binary
* data we create a protocol level command for it
*/
void cs_cmd_generic(char *buf, int len, NewSocket *ns)
{
    player_t          *pl;
    object_t          *ob;
    char            *cp;
    CommArray_s     *csp;
    struct channels *channel=NULL;
    char             ch_buf[512];  /* Player cmds max 250 chars, so should be enough */

    /* we assume that our slash command is always a zero terminated string */
    if (!buf ||
        !len ||
        buf[len] != '\0' ||
        !(pl = ns->pl) ||
        !pl->ob ||
        ns->status != Ns_Playing)
    {
        ns->status = Ns_Dead;

        return;
    }

    ob = pl->ob;

    if (!(pl->state & ST_PLAYING))
    {
        ndi(NDI_UNIQUE, 0, ob, "You can not issue commands - state is not ST_PLAYING (%s)",
                             buf);

        return;
    }

    /* remove the command from the parameters */
    if ((cp = strchr(buf, ' ')))
    {
        *(cp++) = '\0';
        cp = cleanup_string(cp);
    }

    /* Find the command. */
    if (!(csp = find_command(buf, pl)))
    {
        ndi(NDI_UNIQUE, 0, ob, "'/%s' is not a valid command.", buf);
        return;
    }

    if (csp->notify)
    {
        /* It'd probably make sense to use the priority flag here, but I don't
         * think this can be set from the client yet, so its use would be
         * pointless ATM.
         * -- Smacky 20090607 */
        ndi(NDI_UNIQUE | NDI_YELLOW, 0, ob, "/%s %s",
                             buf, (cp) ? cp : "");
    }

    /* Execute the command, and check it's return value */
    switch (csp->func(ob, cp))
    {
        case COMMANDS_RTN_VAL_OK:
            /* Command was a success; send message to appropriate channel, if defined */
            if (csp->ch_name)
                if ((channel=findGlobalChannelFromName(NULL, csp->ch_name, TRUE)))
                {
                    sprintf(ch_buf, "%s%s -- /%s %s", STRING_OBJ_NAME(ob),
                            (pl->privacy) ? " (~Privacy mode~)" : "", buf, (cp) ? cp : "");
                    sendChannelMessage(NULL, channel, ch_buf);
                }
            break;

        case COMMANDS_RTN_VAL_SYNTAX:
            /* User doesn't know how to use the command properly */
            ndi(NDI_UNIQUE | NDI_WHITE, 0, ob, "Syntax error: Try ~/help /%s~",
                                 csp->name);
            return;

        case COMMANDS_RTN_VAL_ERROR:
            /* User has formatted command properly, but there was some other error
             * Maybe, they got the parameters wrong, e.g. /kick non-existant-player-name
             * The specific function should handle the output to the player */
            return;

        case COMMANDS_RTN_VAL_OK_SILENT:
            /* Command completed with no error, although no action was actually taken */
            break;

        default:
            // An unknown command return value here ... log it!
            LOG(llevInfo, "INFO:: Unknown return value from function %s\n", csp->name);
            return;
    }

    ob->speed_left -= csp->time;
}

/* This is the Setup cmd - easy first implementation */
void cs_cmd_setup(char *buf, int len, NewSocket *ns)
{
    int     s;
    char   *cmd, *param, tmpbuf[MAX_DATA_TAIL_LENGTH+128], *cmdback;
    uint32  rel = 0,
            maj = 0,
            min = 0;

    /* lets do some sanity checks */
    if (!buf || ns->status != Ns_Login || !len || buf[len-1] != 0)
    {
        LOG(llevInfo, "HACKBUG: invalid setup data part from %s (%s %d %d)\n",
            STRING_SAFE(ns->ip_host), buf, len, buf[len-1]);
        ns->status = Ns_Dead;
        return;
    }

    if (ns->setup)
    {
        LOG(llevInfo, "HACKBUG: double call of setup cmd from socket %s\n", STRING_SAFE(ns->ip_host));
        ns->status = Ns_Dead;
        return;
    }
    ns->setup = 1; /* all ok, mark setup as already send */

    /* Default geolocation. If the client sends a geo tag (below) these will be
     * reset to 'true' values. */
    ns->lx = 0;
    ns->ly = 0;

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

    /* ac = account creation, fc = faces & server data can be polled */
    strcat(cmdback, "ac 1 fc 1 ");

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

        if (!strcmp(cmd, "dv"))
        {
            char *cp1 = NULL,
                 *cp2 = NULL;

            if ((cp1 = strchr(param, '.')))
            {
                *cp1++ = '\0';
                rel = (uint32)strtoul(param, NULL, 10);

                if ((cp2 = strchr(cp1, '.')))
                {
                    *cp2++ = '\0';
                    maj = (uint32)strtoul(cp1, NULL, 10);
                    min = (uint32)strtoul(cp2, NULL, 10);
                }
            }

            ns->version_rel = rel;
            ns->version_maj = maj;
            ns->version_min = min;
            sprintf(tmpbuf, "%u.%u.%u",
                    DAI_VERSION_RELEASE, DAI_VERSION_MAJOR, DAI_VERSION_MINOR);
            strcat(cmdback, tmpbuf);
        }
        else if (!strcmp(cmd, "pv"))
        {
            ns->protocol_version = (uint32)strtoul(param, (char **)NULL, 10);
            sprintf(tmpbuf, "%d", PROTOCOL_VERSION);
            strcat(cmdback, tmpbuf);
        }
        else if (!strcmp(cmd, "sn"))
        {
            char   *cp;
            int     x   = -1;
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

            if (x == -1)
            {
                ns->sound = 0;
                strcat(cmdback, "OK");
            }
            else
            {
                ns->sound = 1;

                /* we check now the loaded file data - if different
                * we tell it the client - if not, we skip here
                */
                if (SrvClientFiles[SRV_CLIENT_SOUNDS].len_ucomp != x || SrvClientFiles[SRV_CLIENT_SOUNDS].crc != y)
                {
                    sprintf(tmpbuf, "%d|%x", SrvClientFiles[SRV_CLIENT_SOUNDS].len_ucomp,
                        SrvClientFiles[SRV_CLIENT_SOUNDS].crc);
                    strcat(cmdback, tmpbuf);
                }
                else
                    strcat(cmdback, "OK");
            }
        }
        else if (!strcmp(cmd, "mz"))
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
        else if (!strcmp(cmd, "geo"))
        {
            char   *cp = param;

            ns->lx = (sint16)strtol(cp, &cp, 10);
            ns->ly = (sint16)strtol(cp + 1, &cp, 10);
            strcat(cmdback, "OK");
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

    LOG(llevInfo, "SEND SETUP: %s\n",cmdback);
    SOCKBUF_REQUEST_FINISH(ns, SERVER_CMD_SETUP, strlen(cmdback));

    /* lets check the client version is ok. If not, we send back the setup command
    * but then we go in zombie mode
    */
    if (DAI_VERSION_RELEASE != rel ||
        (DAI_VERSION_RELEASE == rel &&
         DAI_VERSION_MAJOR != maj))
    {
        LOG(llevInfo, "Daimonin version mismatch client: %u.%u.%u server: %u.%u.%u\n",
            rel, maj, min, DAI_VERSION_RELEASE, DAI_VERSION_MAJOR,
            DAI_VERSION_MINOR);
        ns->inactive_when = ROUND_TAG + INACTIVE_ZOMBIE * pticks_second;
        ns->status = Ns_Zombie; /* we hold the socket open for a *bit* */
        ns->inactive_flag = 1;

        return;
    }

    if (PROTOCOL_VERSION != ns->protocol_version)
    {
        LOG(llevInfo, "Protocol version mismatch client:(%u) server:(%u)\n",
            ns->protocol_version, PROTOCOL_VERSION);
        ns->inactive_when = ROUND_TAG + INACTIVE_ZOMBIE * pticks_second;
        ns->status = Ns_Zombie; /* we hold the socket open for a *bit* */
        ns->inactive_flag = 1;
        return;
    }
}


/* request a srv_file! */
void cs_cmd_file(char *buf, int len, NewSocket *ns)
{
    int id;

    /* *only* allow this command between the first login and the "addme" command! */
    if (!ns->setup || ns->status != Ns_Login || !buf || len != 1)
    {
        LOG(llevInfo, "RF: received bad rf command for IP:%s\n", STRING_SAFE(ns->ip_host));
        ns->status = Ns_Dead;
        return;
    }

    id = buf[0]; /* aka "get a 8bit char from buffer */
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

/* Moves an object (typically, container to inventory
* move <to> <tag> <nrof>
*/
void cs_cmd_moveobj(char *buf, int len, NewSocket *ns)
{
    player_t *pl;
    tag_t   loc,
            tag;
    uint32  nrof;
    object_t *who,
           *what;

    if (!buf ||
        len < 3 * PARM_SIZE_INT ||
        !(pl = ns->pl) ||
        ns->status != Ns_Playing)
    {
        ns->status = Ns_Dead;

        return;
    }

    loc = (tag_t)GetInt_Buffer(buf);
    tag = (tag_t)GetInt_Buffer(buf);
    nrof = GetInt_Buffer(buf);
    who = pl->ob;

    /* Lag may mean real circumstances have changed before the cmd is received
     * by the server so make sure the tag still refers to a visible object. */
    if (!(what = esrv_get_ob_from_count(who, tag)))
    {
        return;
    }

    /* Drop to floor. */
    if (!loc &&
        (!pl->container ||
         !pl->container->map))
    {
        what = drop_to_floor(who, what, nrof);
    }
    /* Pick up. */
    else
    {
        object_t *where = esrv_get_ob_from_count(who, loc);

        /* Pick up to inventory. */
        if (where == who ||
            (what == pl->container &&
             where == what))
        {
            what = pick_up(who, what, NULL, nrof);
        }
        /* Pick up to container. */
        else
        {
            what = pick_up(who, what, where, nrof);
        }
    }
}

#ifdef SERVER_SEND_FACES

void cs_cmd_face(char *params, int len, NewSocket *ns)
{
    short face;

    if (!ns->setup || ns->status >= Ns_Zombie || !params || len< (PARM_SIZE_SHORT) )
    {
        ns->status = Ns_Dead;
        return;
    }

    face = GetShort_Buffer(params);
    if (face != 0)
        esrv_send_face(ns, face, 1);
}
#endif

void cs_cmd_move(char *buf, int len, NewSocket *ns)
{
    player_t *pl = ns->pl;
    int dir, mode;

    if (!buf || len<(PARM_SIZE_CHAR*2) || !pl || !pl->ob || ns->status != Ns_Playing)
    {
        ns->status = Ns_Dead;
        return;
    }

    dir = GetChar_Buffer(buf);
    mode = GetChar_Buffer(buf);

    move_player(pl->ob, dir, TRUE);

    /*
    if (params)
    {
    if (params[0] == 'r' && !CONTR(op)->run_on)
    CONTR(op)->run_on = 1;
    }
    move_player(op, dir, TRUE);
    int command_stay(object_t *op, char *params)
    {
    fire(op, 0);
    return 0;
    }
    */
}


/* Client wants to examine some object.  So lets do so. */
void cs_cmd_examine(char *buf, int len, NewSocket *ns)
{
    player_t *pl = ns->pl;
    object_t *op;
    long    tag;

    if (!buf || len<PARM_SIZE_INT || !pl || ns->status != Ns_Playing)
    {
        ns->status = Ns_Dead;
        return;
    }

    tag = GetInt_Buffer(buf);
    op  = esrv_get_ob_from_count(pl->ob, tag);

    if (!op)
    {
        /*LOG(llevDebug, "Player '%s' tried examine the unknown object (%d)\n",pl->ob->name, tag);*/
        return;
    }
    examine(pl->ob, op, TRUE);
}

/* Client wants to apply some object.  Lets do so. */
void cs_cmd_apply(char *buf, int len, NewSocket *ns)
{
    player_t *pl;
    uint32  tag;
    object_t *op;

    if (!buf ||
        len < PARM_SIZE_INT ||
        !(pl = ns->pl) ||
        ns->status != Ns_Playing)
    {
        ns->status = Ns_Dead;

        return;
    }

    /* Sort of a hack, but if the player saves and the player then manually
     * applies a savebed (or otherwise tries to do stuff), we run into
     * trouble. */
    if (QUERY_FLAG(pl->ob, FLAG_REMOVED))
    {
        return;
    }

    tag = GetInt_Buffer(buf);

    /* If the high bit is set, player applied a fake object. */
    /* FIXME: Is this safe? This means we assume to never have >0x1fffffff
     * objects concurrently in play which seems to me to be reasonable. That's
     * a helluva big number.
     *
     * Smacky 20130211 */
    if ((tag & 0xe0000000) == 0xe0000000 || // bits 29-31 = end inv delimeter
        (tag & 0xc0000000) == 0xc0000000)   // bits 30-31 = start inv delimeter
    {
        ndi(NDI_UNIQUE, 0, pl->ob, "inv delim!");
    }
    else if ((tag & 0x80000000))            // bit 31 = next/prev
    {
        pl->socket.look_position = tag & 0x7fffffff;
        esrv_send_below(pl);
    }
    /* Otherwise this is a real object. Make sure it's something player can
     * really see. */
    else if ((op  = esrv_get_ob_from_count(pl->ob, tag)))
    {
        (void)apply_object(pl->ob, op, 0);
    }
}

/* Client wants to apply some object.  Lets do so. */
void cs_cmd_lock(char *data, int len, NewSocket *ns)
{
    int     flag, tag;
    player_t *pl = ns->pl;
    object_t *op;

    if (!data || len < (PARM_SIZE_CHAR+PARM_SIZE_INT) || !pl || !pl->ob || ns->status != Ns_Playing)
    {
        ns->status = Ns_Dead;
        return;
    }

    flag = GetChar_Buffer(data);
    tag = GetInt_Buffer(data);
    op = esrv_get_ob_from_count(pl->ob, tag);

    /* can happen as result of latency or client/server async. */
    if (!op)
        return;

    /* only lock item inside the players own inventory */
    if (is_player_inv(op) != pl->ob)
    {
        ndi(NDI_UNIQUE, 0, pl->ob, "You can't lock items outside your inventory!");
        return;
    }
    if (!flag)
        CLEAR_FLAG(op, FLAG_INV_LOCKED);
    else
        SET_FLAG(op, FLAG_INV_LOCKED);

#ifndef USE_OLD_UPDATE
    OBJECT_UPDATE_UPD(op, UPD_FLAGS);
#else
    esrv_update_item(UPD_FLAGS, op);
#endif
}

/* Client wants to apply some object.  Lets do so. */
void cs_cmd_mark(char *data, int len, NewSocket *ns)
{
    int     tag;
    object_t *op;
    player_t *pl = ns->pl;

    if (!data || len<PARM_SIZE_INT || !pl || !pl->ob || ns->status != Ns_Playing)
    {
        ns->status = Ns_Dead;
        return;
    }

    tag = GetInt_Buffer(data);
    op = esrv_get_ob_from_count(pl->ob, tag);

    if(!op || tag == (int)pl->mark_count)
    {
        pl->mark = NULL;
        pl->mark_count = -1;
    }
    else
    {
        pl->mark = op;
        pl->mark_count = op->count;
    }

    /*LOG(llevNoLog,"MARKITEM2 (%d) (%d)\n", tag, op->count);*/
    SOCKBUF_REQUEST_BUFFER(&pl->socket, SOCKET_SIZE_SMALL);
    SockBuf_AddInt(ACTIVE_SOCKBUF(&pl->socket),  pl->mark_count);
    SOCKBUF_REQUEST_FINISH(&pl->socket, SERVER_CMD_MARK, SOCKBUF_DYNAMIC);
}

void cs_cmd_talk(char *data, int len, NewSocket *ns)
{
    player_t *pl;
    sint8   mode;

    if (!data ||
        !len ||
        !(pl = ns->pl) ||
        !pl->ob ||
        ns->status != Ns_Playing)
    {
        ns->status = Ns_Dead;

        return;
    }

    mode = GetChar_Buffer(data);

    switch (mode)
    {
        case GUI_NPC_MODE_NO:
            break;

        case GUI_NPC_MODE_NPC:
            talk_to_npc(pl, data);

            break;

        case GUI_NPC_MODE_QUEST:
            quest_list_command(pl->ob, data);

            break;

        default:
            LOG(llevBug, "BUG:: %s/cs_cmd_talk(): Unknown mode (%d) from player %s: >%s<!\n",
                __FILE__, mode, STRING_OBJ_NAME(pl->ob), data);
    }
}

/* this command is triggered when we use the CTRL key in the client and invoke the
* range/firing system there.
* we have ATM 3 types of action:
* a.) we fire an applied object (throw item, bow/arrow, rod...)
* b.) we fire an spell icon
* c.) we fire an skill icon
* Options b.) and c.) will invoke to cast the spell or use the skill
*/

void cs_cmd_fire(char *params, int len, NewSocket *ns)
{
    int            dir;
    altact_mode_t  mode;
    player_t        *pl = ns->pl;
    object_t        *op,
                  *weapon = NULL,
                  *ammo = NULL;
    float          ticks = -1.0;

    if (!pl ||
        !(op = pl->ob) ||
        ns->status != Ns_Playing ||
        !params ||
        len < (2 * PARM_SIZE_INT + PARM_SIZE_CHAR) ||
        params[len - 1])
    {
        ns->status = Ns_Dead;

        return;
    }

    dir = GetInt_Buffer(params);
    mode = GetInt_Buffer(params);
    /* third param is a string, params points to it start now - a '\0' is at
     * the end as tested at start */

    /* first, call move_player() to determinate we CAN move.
     * have in mind we are perhaps confused - so dir can change! */
    if ((dir = move_player(op, dir, FALSE)) == -1)
    {
        return;
    }

    /* NOT IMPLEMENTED: check for loss of invisiblity/hide */
    if (action_makes_visible(op))
    {
        make_visible(op);
    }

    /* TODO: Remove at 0.11.0. Compatibility code for pre-0.10.6 clients. */
    if (mode == ALTACT_MODE_ARCHERY)
    {
        if (!(weapon = pl->equipment[PLAYER_EQUIP_BOW]))
        {
            return;
        }

        if (IS_DEVICE(weapon))
        {
            mode = ALTACT_MODE_DEVICE;
        }
        else if (weapon->type == ARROW)
        {
            mode = ALTACT_MODE_THROWING;
        }
    }
    else if (mode == ALTACT_MODE_PRAYER)
    {
        mode = ALTACT_MODE_SPELL;
    }

    switch (mode)
    {
        case ALTACT_MODE_ARCHERY:
            if ((weapon = pl->equipment[PLAYER_EQUIP_BOW]) &&
                (ammo = pl->equipment[PLAYER_EQUIP_AMUN]) &&
                weapon->type == BOW &&
                ammo->type == ARROW &&
                ((weapon->sub_type1 == RANGE_WEAP_BOW &&
                  ammo->sub_type1 == ST1_MISSILE_BOW &&
                  change_skill(op, SK_RANGE_BOW)) ||
                 (weapon->sub_type1 == RANGE_WEAP_XBOWS &&
                  ammo->sub_type1 == ST1_MISSILE_CBOW &&
                  change_skill(op, SK_RANGE_XBOW)) ||
                 (weapon->sub_type1 == RANGE_WEAP_SLINGS &&
                  ammo->sub_type1 == ST1_MISSILE_SSTONE &&
                  change_skill(op, SK_RANGE_SLING))))
            {
                if (check_skill_action_time(op, op->chosen_skill))
                {
                    ticks = fire_bow(op, dir);
                }
            }

            break;

        case ALTACT_MODE_SPELL:
            command_cast_spell(op, params); // handles action_time internally

            break;

        case ALTACT_MODE_SKILL:
            command_uskill(op, params); // handles action_time internally

            break;

        case ALTACT_MODE_DEVICE:
            if ((weapon = pl->equipment[PLAYER_EQUIP_BOW]) &&
                IS_DEVICE(weapon) &&
                change_skill(op, SK_MAGIC_DEVICES))
            {
                if (check_skill_action_time(op, op->chosen_skill))
                {
                    ticks = fire_magic_tool(op, weapon, dir);
                }
            }

            break;

        case ALTACT_MODE_THROWING:
            if ((weapon = pl->equipment[PLAYER_EQUIP_BOW]) &&
                weapon->type == ARROW &&
                (weapon->sub_type1 & ST1_MISSILE_THROW) &&
                change_skill(op, SK_THROWING))
            {
                if (check_skill_action_time(op, op->chosen_skill))
                {
                    ticks = do_throw(op, dir);
                }
            }

            break;
    }

    if (ticks >= 0.0)
    {
        LOG(llevDebug, "AC-fire: %2.2f\n", ticks);
        set_action_time(op, ticks);
    }

    op->anim_enemy_dir = (dir) ? dir : op->facing;
}


/* must be called from create account to mark a name as been taken */
void cs_cmd_checkname(char *buf, int len, NewSocket *ns)
{
    int i, ret = ACCOUNT_STATUS_OK;
    const char *hash_name = NULL;
    char filename[MEDIUM_BUF];

    /* If the command isn't perfect, kill the socket. */
    if (!buf ||
        len < MIN_ACCOUNT_NAME + 1 ||
        len > MAX_ACCOUNT_NAME + 1 ||
        buf[len - 1] ||
        ns->status != Ns_Login)
    {
        ns->status = Ns_Dead;

        return;
    }

    /* the client should block any invalid name - if we have one here its bogus */
    if(!account_name_valid(buf))
    {
        ns->status = Ns_Dead;
        return;
    }

    /* first, check its a already used account name */
    sprintf(filename, "%s/%s/%s/%s/%s.acc", settings.localdir, settings.accountdir, get_subdir(buf), buf, buf);
    hash_name = add_string(buf);
    /* perhaps this socket tried twice? delete any previous try */
    FREE_AND_CLEAR_HASH(ns->pl_account.create_name);

    /* lets check the name is in use - we don't must browse the player list, we don't allow
     * to play without the char was created with player file anymore.
     */
    if (access(filename, F_OK) == 0)
        ret = ACCOUNT_STATUS_EXISTS;
    else /* now check every connected socket */
    {
        for (i = 1; i < socket_info.allocated_sockets; i++)
        {
            /* check only connected sockets */
            if (init_sockets[i].status > Ns_Avail && init_sockets[i].status < Ns_Zombie )
            {
                if(hash_name == init_sockets[i].pl_account.create_name)
                {
                    ret = ACCOUNT_STATUS_EXISTS;
                    break;
                }
            }
        }
    }

    if(ret == ACCOUNT_STATUS_OK) /* nothing found... */
    {
        ns->pl_account.create_name = hash_name; /* we reuse the hash ref */
    }
    else
    {
        LOG(llevDebug,"Account: account_create(): Account %s already exists!\n", filename);
        FREE_AND_CLEAR_HASH(hash_name); /* we don't use the hash ref */
    }

    account_create_msg(ns, ret);
}

/* login to a account or create it (create must have called checkname first to reserve the name */
void cs_cmd_login(char *buf, int len, NewSocket *ns)
{
    char           *pass;
    account_status  ret = ACCOUNT_STATUS_OK;
    int             mode;
    const char     *hash_account;

    /* If the command isn't perfect, kill the socket. */
    if (!buf ||
        len < MIN_ACCOUNT_NAME + MIN_ACCOUNT_PASSWORD + 3 ||
        len > MAX_ACCOUNT_NAME + MAX_ACCOUNT_PASSWORD + 3 ||
        buf[len - 1] ||
        ns->status != Ns_Login)
    {
        ns->status = Ns_Dead;

        return;
    }

    mode = GetChar_Buffer(buf);

    /* we have a char + 2 string, both with 0 as endmarker - check we have 2 valid strings */
    if ((int)(strlen(buf) + MIN_ACCOUNT_PASSWORD + 3) > len)
    {
        ns->status = Ns_Dead;
        return;
    }

    pass = buf + strlen(buf) + 1;

    /* Ensure name and pass are valid and not the same. */
    if (!account_name_valid(buf) ||
        !password_valid(pass) ||
        !strcasecmp(buf, pass))
    {
        ns->status = Ns_Dead;
        return;
    }

    /* client told us to create this account */
    if(mode == ACCOUNT_MODE_CREATE)
    {
        /* is the client honest? then we have checked the name */
        if(!ns->pl_account.create_name || strcmp(buf, ns->pl_account.create_name))
        {
            ns->status = Ns_Dead;
            return;
        }
        ret = account_create(&ns->pl_account, buf, pass);
    }

    hash_account = add_string(buf); /* generate a hash - used for example when we compare account names */

    if(check_banned(ns, hash_account, NULL, 0))
        ret = ACCOUNT_STATUS_BANNED;

    if(ret == ACCOUNT_STATUS_OK) /* still all ok? then load this account */
        ret = account_load(&ns->pl_account, buf, pass);

    /* in any case we give now a response */
    account_send_client(ns, ret); /* can be also "sorry, no account" */

    if(ret) /* something is wrong, send a clear account command with status only */
    {
        /* add here a counter with temp ip ban for 30sec to avoid login hammering */

        if (ret == ACCOUNT_STATUS_WRONGPWD &&
            clogfile != tlogfile)
        {
            CHATLOG("LOGIN: IP >%s< Account >%s<... WRONG PASSWORD!\n",
                ns->ip_host, buf);
        }
    }
    else /* player is logged in to his account */
    {
        if (clogfile != tlogfile)
        {
            CHATLOG("LOGIN: IP >%s< Account >%s< Client version %u.%u.%u... OK!\n",
                    ns->ip_host, buf, ns->version_rel, ns->version_maj,
                    ns->version_min);
        }

        /* only place where we go in account selection and allow newchar and addme */
        ns->status = Ns_Account;
    }
}

/* try to add (login) a player <name> from account logged in on socket ns */
void cs_cmd_addme(char *buf, int len, NewSocket *ns)
{
    player_t     *pl = NULL;
    addme_login_msg error_msg;
    const char *hash_name;

    /* If the cmd isn't perfect, kill the socket. */
    if (!buf ||
        len < MIN_PLAYER_NAME + 1 ||
        len > MAX_PLAYER_NAME + 1 ||
        buf[len - 1] ||
        ns->status != Ns_Account)
    {
        ns->status = Ns_Dead;

        return;
    }

    /* the client MUST have send us a valid name. If not we are very, very angry ... */
    if (!player_name_valid(buf))
    {
        ns->status = Ns_Dead;
        return;
    }

    hash_name = add_string(buf); /* generate a hash - used for example when we compare player names */

    /* lets see the player is banned - if so don't even try to log */
    if (check_banned(ns, NULL, hash_name, 0))
    {
        LOG(llevInfo, "Banned player %s tried to add. [%s]\n", hash_name, ns->ip_host);
        error_msg = ADDME_MSG_BANNED;
    }
    else
    {
        char    double_login_warning[] = "3 Double login! Kicking older instance!";
        player_t *ptmp = first_player;

        for (; ptmp; ptmp = ptmp->next)
        {
            /* If a same-named player is already playing, kick him (he's a
             * ghost) BEFORE we load the player file for this new instance.
             * This avoids duping. */
            if ((ptmp->state & ST_PLAYING) &&
                ptmp->ob->name == hash_name)
            {
                LOG(llevInfo, "Double login! Kicking older instance!");
                Write_String_To_Socket(ns, SERVER_CMD_DRAWINFO,
                                       double_login_warning,
                                       strlen(double_login_warning));
                ptmp->state &= ~ST_PLAYING;
                ptmp->state |= ST_ZOMBIE;
                ptmp->socket.status = Ns_Dead;
                remove_ns_dead_player(ptmp);/* super hard kick! */

                continue;
            }
        }

        /* lets try to login! ns is our socket, the player name must be a hash
        * the return value will tell us player is now loaded & active or there is a problem.
        * login_player() will put the player on the map and send all initial commands in the
        * right order - after it, the player is already playing accept there is an error!
        */
        error_msg = player_load(ns, hash_name);

        /* small trick - we use the socket player relink to point to the new player struct from login_player */
        pl = ns->pl;
        ns->pl = NULL;
        /*LOG(llevNoLog,"Socket: pl->socket: %x fd:%d :: ns: %x fd:%d\n", &pl->socket, pl->socket.fd, ns, ns->fd);*/
    }

    FREE_AND_CLEAR_HASH(hash_name); /* clear this reference */

    if (clogfile != tlogfile)
    {
        CHATLOG("ADDME: IP >%s< Account >%s< Player >%s<... %s!\n",
                ns->ip_host, (!pl) ? ns->pl_account.name : pl->account_name,
                buf, (error_msg == ADDME_MSG_BANNED) ? "BANNED" :
                ((error_msg == ADDME_MSG_OK) ? "OK" : "FAILED/ABORTED"));
    }

    /* now check the login was a success or not */
    if (error_msg != ADDME_MSG_OK)
    {
        LOG(llevDebug, "ADDMEFAIL: login failed for %s on account %s with error %d\n", buf, ns->pl_account.name, error_msg);
        player_addme_failed(ns, error_msg);
    }
    else
    {
        /* forget this 2 settings and watch the socket exploding */
        pl->socket.readbuf.toread = 0; /* mark this addme cmd as done on the copied, no active socket */
        ns->addme = 1; /* mark the old socket as invalid because mirrored */

        /* give out some more initial info */
        ndi(NDI_UNIQUE, 0, pl->ob, "This is %s.", version_string());
        display_motd(pl->ob);
#ifdef USE_CHANNELS
#ifdef ANNOUNCE_CHANNELS
        /* TODO: We should instead give useful info here like which channels
         * are available. */
        ndi(NDI_UNIQUE | NDI_RED, 0, pl->ob, "We are testing out a new channel-system!\nMake sure you have a client with channel-support.\nSee forums on www.daimonin.org!");
#endif
#endif
//        ndi(NDI_UNIQUE | NDI_RED, 0, pl->ob, "Testing!");
    }
}

/* Client wants create a new player. We check the name is taken or not (and the stats are ok of course).
 * If the name is taken, we answer with an addme_fails and the player can try another name.
 * If the stats are bad we have a hacker and we are angry again
 * Is all ok we create the player file, add the player name to the account index, save it for security
 * and send a new account data command to the client so it leaves char creation and presents the new
 * account data
 */
void cs_cmd_newchar(char *buf, int len, NewSocket *ns)
{
    int     gender, race, skill_nr, ret = ADDME_MSG_OK;
    shstr_t  *pass;
    char    filename[MEDIUM_BUF];

    /* if the cmd isn't perfect, kill the socket. */
    if (ns->pl_account.nrof_chars == ACCOUNT_MAX_PLAYER ||
        !buf ||
        len < MIN_PLAYER_NAME + 3 + 5 ||
        len > MAX_PLAYER_NAME + 17 + 5 ||
        buf[len - 1] ||
        ns->status != Ns_Account)
    {
        ns->status = Ns_Dead;

        return;
    }

    race = GetChar_Buffer(buf);
    gender = GetChar_Buffer(buf);
    skill_nr = GetChar_Buffer(buf);
    pass = add_string(buf + strlen(buf) + 1);

    if (!player_name_valid(buf))
    {
        ns->status = Ns_Dead;
        return;
    }

    /* simple check now with a valid character name - if the player file exists the name is taken.
     * There is no way anymore that a character can be accessed/played without a player
     * name AND the new player file is always created instantly.
     */
    sprintf(filename, "%s/%s/%s/%s/%s.pl", settings.localdir, settings.playerdir, get_subdir(buf), buf, buf);

    if (access(filename, F_OK) == 0)
    {
        FILE *fp;

        if ((fp = fopen(filename, "r")))
        {
            char real_password[TINY_BUF];
            /* Old player files have password ... as the first line. So if the
             * name exists but this is not the first line we can surmise it is
             * a new-style player already associated with an account. */
            if (fscanf(fp, "password %s", real_password) != 1)
            {
                fclose(fp);
                ret = ADDME_MSG_TAKEN;
            }
            /* Otherwise it must be old-style and therefore reclaimable. */
            else
            {
                shstr_t *password = add_string(real_password);

                fclose(fp);

                /* If the password from the client does not match the password
                 * in the save file, the reclaim fails. */
                if (pass != password)
                {
                    ret = ADDME_MSG_WRONGPWD;

                    /* The first time, the client (should -- see protocol.h)
                     * send a dummy password. This simply marks that attempt as
                     * a testing the waters attempt so we know not to log it as
                     * password guessing. */
                    if (pass != shstr_cons.nopass)
                    {
                        CHATLOG("RECLAIM: IP >%s< Account >%s< Player >%s<... WRONG PASSWORD!\n",
                                ns->ip_host, ns->pl_account.name, buf);
                    }
                }
                /* A match means we reclaim this name. Do this by backing up
                 * the old save dir to the account so we can add a new-style
                 * player below. */
                else
                {
                     shstr_t *name = NULL;

                     /* Add a dummy player so that delete player will work! */
                     ns->pl_account.level[ns->pl_account.nrof_chars] = 1; /* we always start with level 1 */
                     ns->pl_account.race[ns->pl_account.nrof_chars] = race;
                     ns->pl_account.gender[ns->pl_account.nrof_chars] = gender;
                     strcpy(ns->pl_account.charname[ns->pl_account.nrof_chars], buf);
                     ns->pl_account.nrof_chars += 1;
                     account_save(&ns->pl_account, ns->pl_account.name); /* ignore problems here, we have later a 2nd try perhaps */

                    /* This is basically the business end from cs_cmd_delchar()
                     * below. */
                    name = add_string(buf);
                    ret = account_delete_player(ns, name);
                    FREE_AND_CLEAR_HASH(name);

                    if (ret == ACCOUNT_STATUS_EXISTS)
                    {
                        ns->status = Ns_Dead;

                        return;
                    }

                    account_save(&ns->pl_account, ns->pl_account.name);

                    if (ret != ACCOUNT_STATUS_OK)
                    {
                        ret = ADDME_MSG_CORRUPT;
                    }
                    else
                    {
                        //account_send_client(ns, ACCOUNT_STATUS_OK);
                        ret = ADDME_MSG_OK;
                    }

                    CHATLOG("RECLAIM: IP >%s< Account >%s< Player >%s<... OK!\n",
                            ns->ip_host, ns->pl_account.name, buf);
                }

                FREE_AND_CLEAR_HASH(password);
            }
        }
        else
        {
            ret = ADDME_MSG_CORRUPT;
        }
    }

    FREE_AND_CLEAR_HASH(pass);

    if (ret == ADDME_MSG_OK)
    {
        player_t *pl = NULL;

        /* name is ok... now we have to do some work by filling up a player structure.
         * in this way we can use the standard save_player() function which is alot more
         * clean as trying here own stuff - we only must have now command_new_char()
         * in the future.
         */

        ret = player_create(ns, &pl, buf, race, gender, skill_nr);

        if(ret == ADDME_MSG_OK && pl)
        {
            /* only by OK as return value we have a valid pl pointer */

            if(!player_save(pl->ob)) /* if we can't save we don't add this char to the account */
                ret = ADDME_MSG_CORRUPT;

            /* Force an out-of-loop gc to delete the player struct & object NOW */
            pl->ob->type = DEAD_OBJECT; /* we tell the object destructor the player struct is invalid */
            CLEAR_FLAG(pl->ob, FLAG_FRIENDLY); /* avoid friendly list handling */
            object_gc();
        }
    }

    if(ret == ADDME_MSG_OK)
    {
        /* character is valid, created and saved - now update our account data */
        ns->pl_account.level[ns->pl_account.nrof_chars] = 1; /* we always start with level 1 */
        ns->pl_account.race[ns->pl_account.nrof_chars] = race;
        ns->pl_account.gender[ns->pl_account.nrof_chars] = gender;
        strcpy(ns->pl_account.charname[ns->pl_account.nrof_chars], buf);
        ns->pl_account.nrof_chars += 1;

        /* to ensure valid accounts we save it now */
        account_save(&ns->pl_account, ns->pl_account.name); /* ignore problems here, we have later a 2nd try perhaps */

        /* all done - now update the account info of the client */
        account_send_client(ns, ACCOUNT_STATUS_OK); /* its always OK, we send here addme_fails when something is wrong */
    }
    else /* something is wrong, use addme_fails to tell it player (normally ADDME_MSG_TAKEN) */
    {
        player_addme_failed(ns, ret);
    }
}

/* delete a character from an account.
 * The player file is moved with a time tag inside the account folder for backup
 */
void cs_cmd_delchar(char *buf, int len, NewSocket *ns)
{
    int ret;
    shstr_t *name = NULL;

    /* if the cmd isn't perfect, kill the socket. */
    if (!ns->pl_account.nrof_chars ||
        !buf ||
        len < MIN_PLAYER_NAME + 1 ||
        len > MAX_PLAYER_NAME + 1 ||
        buf[len - 1] ||
        ns->status != Ns_Account)
    {
        ns->status = Ns_Dead;

        return;
    }

    if (!player_name_valid(buf))
    {
        ns->status = Ns_Dead;
        return;
    }

    /* name is ok, now try to remove from account and move the player file.
     * account_delete_player() will take care about the flow */
    name = add_string(buf);
    ret = account_delete_player(ns, name);
    FREE_AND_CLEAR_HASH(name);

    /* no player with that name is part of this account.
     * this is a hack or a nasty sync problem - the client MUST send us a name which is part of account
     */
    if(ret == ACCOUNT_STATUS_EXISTS)
    {
        ns->status = Ns_Dead;
        return;
    }

    /* save the account and update the client in any case - if we don't do it it stays in delete wait status*/
    account_save(&ns->pl_account, ns->pl_account.name);

    if(ret != ACCOUNT_STATUS_OK) /* just tell the client we had a problem ... no need for details */
        player_addme_failed(ns, ADDME_MSG_CORRUPT);

    /* we just refresh a valid account, so force an OK status */
    account_send_client(ns, ACCOUNT_STATUS_OK);
}
