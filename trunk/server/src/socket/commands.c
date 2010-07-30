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
    {0,         NULL}, /* ping */

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
    {"target",        command_target,         0.1f, 0}, /* enter combat and attack object in reach */
    {"combat",        command_combat,         0.1f, 0}, /* toggle attack mode of player */
    {"rest",          command_resting,        1.0f, 1},
    {"run",           command_run,            1.0f, 0},
    {"run_stop",      command_run_stop,       0.0f, 0},
    {"cast",          command_cast_spell,     0.0f, 1}, /* use time comes from spells! */
    {"push",          command_push_object,    1.0f, 1},
    {"right",         command_turn_right,     1.0f, 0},
    {"left",          command_turn_left,      1.0f, 0},
    {"say",           command_say,            1.0f, 0},
    {"gsay",          command_gsay,           1.0f, 0},
    {"shout",         command_shout,          1.0f, 0},
    {"tell",          command_tell,           1.0f, 0},
    {"who",           command_who,            0.0f, 1},
    {"mapinfo",       command_mapinfo,        0.0f, 1},
    {"motd",          command_motd,           0.0f, 0},
    {"time",          command_time,           1.0f, 0},
/* This will be moved to a help category (/help version) and thus eventually
 * handled client-side.
 * -- Smacky 20090613 */
    {"version",       command_version,        0.0f, 0},
    {"help",          command_help,           0.0f, 0},
    {"save",          command_save,           0.0f, 1},
    {"use_skill",     command_uskill,         0.1f, 1},
    {"ready_skill",   command_rskill,         0.1f, 1},
    {"privacy",       command_privacy,        0.0f, 1},
    {"egobind",       command_egobind,        1.0f, 1},
    {"invite",        command_party_invite,   4.0f, 1},
    {"join",          command_party_join,     0.1f, 1},
    {"deny",          command_party_deny,     0.1f, 1},
    {"leave",         command_party_leave,    4.0f, 1},
    {"remove",        command_party_remove,   4.0f, 1},
#ifdef USE_CHANNELS
    {"channel",       command_channel,        1.0f, 0}, /* channel system */
#endif
#ifdef _TESTSERVER
    {"stuck",         command_stuck,          0.0f, 1},
#endif
    {"vol",           command_vol,            0.0f, 1},
    {"gm",            command_gm,             0.0f, 1},
    {"mw",            command_mw,             0.0f, 1},
    {"mm",            command_mm,             0.0f, 1},
    {"sa",            command_sa,             0.0f, 1},
    {"gmasterlist",   command_gmasterlist,    0.0f, 1},
};

CommArray_s EmoteCommands[] =
{
    {"nod",     command_nod,     1.0f, 0},
    {"bounce",  command_bounce,  1.0f, 0},
    {"laugh",   command_laugh,   1.0f, 0},
    {"puke",    command_puke,    1.0f, 0},
    {"sigh",    command_sigh,    1.0f, 0},
    {"cry",     command_cry,     1.0f, 0},
    {"grin",    command_grin,    1.0f, 0},
    {"blush",   command_blush,   1.0f, 0},
    {"cough",   command_cough,   1.0f, 0},
    {"gasp",    command_gasp,    1.0f, 0},
    {"hiccup",  command_hiccup,  1.0f, 0},
    {"shiver",  command_shiver,  1.0f, 0},
    {"smirk",   command_smirk,   1.0f, 0},
    {"snicker", command_snicker, 1.0f, 0},
    {"spit",    command_spit,    1.0f, 0},
    {"twiddle", command_twiddle, 1.0f, 0},
    {"wink",    command_wink,    1.0f, 0},
    {"bleed",   command_bleed,   1.0f, 0},
    {"me",      command_me,      1.0f, 0},
    {"dance",   command_dance,   1.0f, 0},
    {"smile",   command_smile,   1.0f, 0},
    {"giggle",  command_giggle,  1.0f, 0},
    {"growl",   command_growl,   1.0f, 0},
    {"sulk",    command_sulk,    1.0f, 0},
    {"poke",    command_poke,    1.0f, 0},
    {"bow",     command_bow,     1.0f, 0},
    {"burp",    command_burp,    1.0f, 0},
    {"flip",    command_flip,    1.0f, 0},
    {"glare",   command_glare,   1.0f, 0},
    {"lick",    command_lick,    1.0f, 0},
    {"shrug",   command_shrug,   1.0f, 0},
    {"snap",    command_snap,    1.0f, 0},
    {"sniff",   command_sniff,   1.0f, 0},
    {"strut",   command_strut,   1.0f, 0},
    {"wave",    command_wave,    1.0f, 0},
    {"yawn",    command_yawn,    1.0f, 0},
    {"cringe",  command_cringe,  1.0f, 0},
    {"kiss",    command_kiss,    1.0f, 0},
    {"cackle",  command_cackle,  1.0f, 0},
    {"shake",   command_shake,   1.0f, 0},
    {"scream",  command_scream,  1.0f, 0},
    {"hug",     command_hug,     1.0f, 0},
    {"accuse",  command_accuse,  1.0f, 0},
    {"clap",    command_clap,    1.0f, 0},
    {"chuckle", command_chuckle, 1.0f, 0},
    {"frown",   command_frown,   1.0f, 0},
    {"groan",   command_groan,   1.0f, 0},
    {"pout",    command_pout,    1.0f, 0},
    {"slap",    command_slap,    1.0f, 0},
    {"sneeze",  command_sneeze,  1.0f, 0},
    {"snore",   command_snore,   1.0f, 0},
    {"thank",   command_thank,   1.0f, 0},
    {"whistle", command_whistle, 1.0f, 0},
    {"beg",     command_beg,     1.0f, 0},
    {"think",   command_think,   1.0f, 0},
};

CommArray_s CommandsVOL[] =
{
    {"mutelevel", command_mutelevel, 0.0f, 1},
    {"kick",      command_kick,      0.0f, 1},
    {"mute",      command_mute,      0.0f, 1},
    {"ip",        command_ip,        0.0f, 1},
#ifdef USE_CHANNELS
    {"createchannel", command_channel_create, 0.0f, 1}, /* channel system */
    {"deletechannel", command_channel_delete, 0.0f, 1}, /* channel system */
    {"channelmute",   command_channel_mute,   0.0f, 1}, /* channel system */
#endif
};

CommArray_s CommandsGM[] =
{
    {"dm_connections", command_dm_connections, 0.0f, 1},
    {"generate",       command_generate,       0.0f, 1},
    {"inventory",      command_inventory,      0.0f, 1},
    {"summon",         command_summon,         0.0f, 1},
    {"teleport",       command_teleport,       0.0f, 1},
    {"ban",            command_ban,            0.0f, 1},
    {"silence",        command_silence,        0.0f, 1},
    {"gmasterfile",    command_gmasterfile,    0.0f, 1},
    {"eavesdrop",      command_eavesdrop,      0.0f, 1},
    {"stats",          command_stats,          0.0f, 1},
};

CommArray_s CommandsMW[] =
{
#ifdef _TESTSERVER
    {"summon",        command_summon,      0.0f, 1},
    {"stealth",       command_stealth,     0.0f, 1},
    {"addexp",        command_addexp,      0.0f, 1},
    {"setskill",      command_setskill,    0.0f, 1},
    {"setstat",       command_setstat,     0.0f, 1},
    {"wizpass",       command_wizpass,     0.0f, 1},
    {"teleport",      command_teleport,    0.0f, 1},
    {"resetmap",      command_reset,       0.0f, 1},
    {"goto",          command_goto,        0.0f, 1},
    {"reboot",        command_reboot,      0.0f, 1},
    {"dm_invis",      command_dm_invis,    0.0f, 1},
    {"dm_dev",        command_dm_dev,      0.0f, 1},
    {"dm_light",      command_dm_light,    0.0f, 1},
    {"set_map_light", command_setmaplight, 0.0f, 1},
#endif
    {"generate",      command_generate,    0.0f, 1},
    {"mspinfo",       command_mspinfo,     0.0f, 1},
};

CommArray_s CommandsMM[] =
{
#ifdef _TESTSERVER
    {"serverspeed",   command_serverspeed, 0.0f, 1},
    {"create",        command_create,      0.0f, 1},
#else
    {"stealth",       command_stealth,     0.0f, 1},
    {"wizpass",       command_wizpass,     0.0f, 1},
    {"teleport",      command_teleport,    0.0f, 1},
    {"resetmap",      command_reset,       0.0f, 1},
    {"goto",          command_goto,        0.0f, 1},
    {"reboot",        command_reboot,      0.0f, 1},
    {"dm_invis",      command_dm_invis,    0.0f, 1},
    {"dm_dev",        command_dm_dev,      0.0f, 1},
    {"dm_light",      command_dm_light,    0.0f, 1},
    {"set_map_light", command_setmaplight, 0.0f, 1},
#endif
    {"gmasterfile",   command_gmasterfile, 0.0f, 1},
};

CommArray_s CommandsSA[] =
{
#ifndef _TESTSERVER
    {"serverspeed",  command_serverspeed,       0.0f, 1},
    {"create",       command_create,            0.0f, 1},
    {"addexp",       command_addexp,            0.0f, 1},
    {"setskill",     command_setskill,          0.0f, 1},
    {"setstat",      command_setstat,           0.0f, 1},
#endif
    {"plugin",       command_loadplugin,        0.0f, 1},
    {"pluglist",     command_listplugins,       0.0f, 1},
    {"plugout",      command_unloadplugin,      0.0f, 1},
    {"dump",         command_dump,              0.0f, 1},
    {"d_active",     command_dumpactivelist,    0.0f, 1},
    {"d_arches",     command_dumpallarchetypes, 0.0f, 1},
    {"d_objects",    command_dumpallobjects,    0.0f, 1},
    {"d_belowfull",  command_dumpbelowfull,     0.0f, 1},
    {"d_below",      command_dumpbelow,         0.0f, 1},
    {"d_hash",       command_sstable,           0.0f, 1},
    {"check_fd",     command_check_fd,          0.0f, 1},
    {"malloc",       command_malloc,            0.0f, 1},

    /* old, outdated or disabled commands */
    /*
    TODO: fix password related commands who changed by the account patch
    {"dm_pwd", command_dm_password,0.0},
    {"dm_load", command_dmload,0.0}, // disabled because account patch

    {"dropall",        command_dropall,        1.0},
    {"listen", command_listen,    0.0}, // our channel system should work different
    {"drop", command_drop,    1.0},
    {"get", command_take,     1.0},
    {"apply",         command_apply,          1.0f, 1}, // should be variable
    {"examine", command_examine,  0.5}, // should work in direction
    {"statistics", command_statistics,    0.0}, // will be send to client later in status
    {"archs", command_archs, 0.0},
    {"debug", command_debug,0.0},
    {"fix_me", command_fix_me,   0.0},
    {"forget_spell", command_forget_spell, 0.0},
    {"free", command_free,0.0},
    {"invisible", command_invisible,0.0},
    {"learn_special_prayer", command_learn_special_prayer, 0.0},
    {"learn_spell", command_learn_spell, 0.0},
    {"logs", command_logs,   0.0},
    {"patch", command_patch,0.0},
    {"printlos", command_printlos,0.0},
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
    subcommands.restart = add_string("restart");
    subcommands.shutdown = add_string("shutdown");
}

/* Finds cmd if it exists for pl (determined by gmaster_mode). 
 * If so, the particular command element is returned. Otherwise, NULL is
 * returned. */
CommArray_s *find_command(char *cmd, player *pl)
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

#define DEBUG_PROCESS_QUEUE

/* We go through the list of queued commands we got from the client */
void process_command_queue(NewSocket *ns, player *pl)
{
    int cmd, cmd_count = 0;

    /* do some sanity checks ... we only allow a full enabled player to work out commands */
    if ( ns->status != Ns_Playing || !(pl->state&ST_PLAYING) || (pl && (!pl->ob || pl->ob->speed_left < 0.0f)))
        return;

    /* Loop through this - maybe we have several complete packets here. */
    while (ns->cmd_start)
    {
#ifdef DEBUG_PROCESS_QUEUE
        LOG(llevDebug, "process_command_queue: Found cmdptr:%p . Cmd: %d\n", ns->cmd_start, ns->cmd_start->cmd);
#endif

        /* reset idle counter */
        ns->login_count = ROUND_TAG + pticks_player_idle1;
        ns->idle_flag = 0;

        /* all commands we have was pre-processed, so we can be sure they are valid for active, playing Player. */

        /* well, some last sanity tests */
        if((cmd = ns->cmd_start->cmd) < 0 || cmd >= CLIENT_CMD_MAX_NROF)
        {
            LOG(llevDebug, "HACKBUG: Bad command from client (%d) cmd:(%d)\n", ns->fd, cmd);
            ns->status = Ns_Dead;
            return;
        }

        /* simple and fast: we call the cmd function in binary style */
        cs_commands[cmd].cmdproc(ns->cmd_start->buf, ns->cmd_start->len, ns);
        /* and remove the command from the queue */
        command_buffer_clear(ns);

        /* have we to stop or one more command? */
        if (cmd_count++ >= 8 || ns->status != Ns_Playing || !(pl->state&ST_PLAYING) || (pl && (!pl->ob || pl->ob->speed_left < 0.0f)))
            return;
    }
}

/* This command handles slash game commands like /say, /tell or /dm
* As a protocol level command, it works as transportation level for
* the "real" game commands.
* NOTE: A game command is always a string, if a command deals with real binary
* data we create a protocol level command for it
*/
void cs_cmd_generic(char *buf, int len, NewSocket *ns)
{
    player      *pl;
    object      *ob;
    char        *cp;
    CommArray_s *csp;

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
        new_draw_info(NDI_UNIQUE, 0, ob, "You can not issue commands - state is not ST_PLAYING (%s)",
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
        new_draw_info(NDI_UNIQUE, 0, ob, "'/%s' is not a valid command.",
                             buf);

        return;
    }

    if (csp->notify)
    {
        /* It'd probably make sense to use the priority flag here, but I don't
         * think this can be set from the client yet, so its use would be
         * pointless ATM.
         * -- Smacky 20090607 */
        new_draw_info(NDI_UNIQUE | NDI_YELLOW, 0, ob, "/%s %s",
                             buf, (cp) ? cp : "");
    }

    /* command_foo() funcs return zero/non-zero to indicate success/failure. */
    if (csp->func(ob, cp))
    {
        new_draw_info(NDI_UNIQUE | NDI_WHITE, 0, ob, "Syntax error: Try ~/help /%s~",
                             csp->name);

        return;
    }

    ob->speed_left -= csp->time;
}

/* This is the Setup cmd - easy first implementation */
void cs_cmd_setup(char *buf, int len, NewSocket *ns)
{
    int     s;
    char   *cmd, *param, tmpbuf[MAX_DATA_TAIL_LENGTH+128], *cmdback;

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

        if (!strcmp(cmd, "pv"))
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
    SOCKBUF_REQUEST_FINISH(ns, BINARY_CMD_SETUP, strlen(cmdback));

    /* lets check the client version is ok. If not, we send back the setup command
    * but then we go in zombie mode
    */
    if (PROTOCOL_VERSION != ns->protocol_version)
    {
        LOG(llevInfo, "Protocol version mismatch client:(%u) server:(%u)\n",
            ns->protocol_version, PROTOCOL_VERSION);
        ns->login_count = ROUND_TAG+(uint32)(10.0f * pticks_second);
        ns->status = Ns_Zombie; /* we hold the socket open for a *bit* */
        ns->idle_flag = 1;
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
    player *pl = ns->pl;
    tag_t loc, tag;
    long nrof;

    if (!buf || len< 3*PARM_SIZE_INT || !pl || ns->status != Ns_Playing)
    {
        ns->status = Ns_Dead;
        return;
    }

    loc = (tag_t)GetInt_Buffer(buf);
    tag = (tag_t)GetInt_Buffer(buf);
    nrof = (long)GetInt_Buffer(buf);

    esrv_move_object(pl->ob, loc, tag, nrof);
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
    player *pl = ns->pl;
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
    int command_stay(object *op, char *params)
    {
    fire(op, 0);
    return 0;
    }
    */
}


/* Client wants to examine some object.  So lets do so. */
void cs_cmd_examine(char *buf, int len, NewSocket *ns)
{
    player *pl = ns->pl;
    object *op;
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
    uint32  tag;
    object *op;
    player *pl = ns->pl;

    if (!buf || len<PARM_SIZE_INT || !pl || ns->status != Ns_Playing)
    {
        ns->status = Ns_Dead;
        return;
    }
    /* sort of a hack, but if the player saves and the player then manually
    * applies a savebed (or otherwise tries to do stuff), we run into trouble.
    */
    if (QUERY_FLAG(pl->ob, FLAG_REMOVED))
        return;

    tag = GetInt_Buffer(buf);
    op  = esrv_get_ob_from_count(pl->ob, tag);

    /* If the high bit is set, player applied a pseudo object. */
    if (tag & 0x80000000)
    {
        pl->socket.look_position = tag & 0x7fffffff;
        pl->socket.update_tile = 0;
        return;
    }

    if (!op)
    {
        /*LOG(llevDebug, "Player '%s' tried apply the unknown object (%d)\n",pl->ob->name, tag);*/
        return;
    }

    player_apply(pl->ob, op, 0, 0);
}

/* Client wants to apply some object.  Lets do so. */
void cs_cmd_lock(char *data, int len, NewSocket *ns)
{
    int     flag, tag;
    player *pl = ns->pl;
    object *op;

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
        new_draw_info(NDI_UNIQUE, 0, pl->ob, "You can't lock items outside your inventory!");
        return;
    }
    if (!flag)
        CLEAR_FLAG(op, FLAG_INV_LOCKED);
    else
        SET_FLAG(op, FLAG_INV_LOCKED);

    esrv_update_item(UPD_FLAGS, pl->ob, op);
}

/* Client wants to apply some object.  Lets do so. */
void cs_cmd_mark(char *data, int len, NewSocket *ns)
{
    int     tag;
    object *op;
    player *pl = ns->pl;

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
    SOCKBUF_REQUEST_FINISH(&pl->socket, BINARY_CMD_MARK, SOCKBUF_DYNAMIC);
}

void cs_cmd_talk(char *data, int len, NewSocket *ns)
{
    player *pl;
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
                __FILE__, mode, query_name(pl->ob), data);
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
    int     dir, type;
    player *pl = ns->pl;
    object *op;

    if (!pl || !pl->ob || ns->status != Ns_Playing || !params
        || len<(2*PARM_SIZE_INT+PARM_SIZE_CHAR) || params[len-1])
    {
        ns->status = Ns_Dead;
        return;
    }

    op = pl->ob;
    dir = GetInt_Buffer(params);
    type = GetInt_Buffer(params);
    /* third param is a string, params points to it start now - a '\0' is at the end as tested at start */

    /* first, call move_player() to determinate we CAN move.
    * have in mind we are perhaps confused - so dir can change!
    */
    dir =     move_player(op, dir, FALSE);

    if(dir == -1) /* move_player() disallow our move! */
        return;

    if (type == FIRE_MODE_SPELL)
        command_cast_spell(op, params);
    else if (type == FIRE_MODE_SKILL)
        command_uskill(op, params);
    else /* arrow, rod... */
        fire(op, dir);

    if (dir)
        op->anim_enemy_dir = dir;
    else
        op->anim_enemy_dir = op->facing;
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

    if(ret == ACCOUNT_STATUS_OK) /* still all ok? then load this account */
        ret = account_load(&ns->pl_account, buf, pass);

    /* in any case we give now a response */
    account_send_client(ns, ret); /* can be also "sorry, no account" */

    if(ret) /* something is wrong, send a clear account command with status only */
    {
        /* add here a counter with temp ip ban for 30sec to avoid login hammering */
    }
    else /* player is logged in to his account */
    {
        /* only place where we go in account selection and allow newchar and addme */
        ns->status = Ns_Account;
    }
}

/* try to add (login) a player <name> from account logged in on socket ns */
void cs_cmd_addme(char *buf, int len, NewSocket *ns)
{
    player     *pl = NULL;
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
    if (check_banned(ns, hash_name, 0))
    {
        LOG(llevInfo, "Banned player %s tried to add. [%s]\n", hash_name, ns->ip_host);
        error_msg = ADDME_MSG_BANNED;
    }
    else
    {
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
        CHATLOG("LOGIN: IP >%s< Account >%s< Player >%s<... %s!\n",
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
        start_info(pl->ob);
        display_motd(pl->ob);
#ifdef USE_CHANNELS
#ifdef ANNOUNCE_CHANNELS
        /* TODO: We should instead give useful info here like which channels
         * are available.
        new_draw_info(NDI_UNIQUE | NDI_RED, 0, pl->ob, "We are testing out a new channel-system!\nMake sure you have a client with channel-support.\nSee forums on www.daimonin.org!"); */
#endif
#endif
//        new_draw_info(NDI_UNIQUE | NDI_RED, 0, pl->ob, "Testing!");
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
    shstr  *pass;
    char    filename[MEDIUM_BUF];

    /* if the cmd isn't perfect, kill the socket. */
    if (ns->pl_account.nrof_chars == ACCOUNT_MAX_PLAYER ||
        !buf ||
        len < MIN_PLAYER_NAME + 6 + 5 ||
        len > MAX_PLAYER_NAME + 16 + 5 ||
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
                ret = ADDME_MSG_TAKEN;
            }
            /* Otherwise it must be old-style and therefore reclaimable. */
            else
            {
                shstr *password = add_string(real_password);

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
                     /* Add a dummy player so that delete player will work! */
                     ns->pl_account.level[ns->pl_account.nrof_chars] = 1; /* we always start with level 1 */
                     ns->pl_account.race[ns->pl_account.nrof_chars] = race;
                     ns->pl_account.gender[ns->pl_account.nrof_chars] = gender;
                     strcpy(ns->pl_account.charname[ns->pl_account.nrof_chars], buf);
                     ns->pl_account.nrof_chars += 1;
                     account_save(&ns->pl_account, ns->pl_account.name); /* ignore problems here, we have later a 2nd try perhaps */

                    /* This is basically the business end from cs_cmd_delchar()
                     * below. */
                    if ((ret = account_delete_player(&ns->pl_account, buf)) ==
                        ACCOUNT_STATUS_EXISTS)
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
        player *pl = NULL;

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
     * account_delete_player() will take care about the flow
     */
    ret = account_delete_player(&ns->pl_account, buf);

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
