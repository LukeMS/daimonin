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

    The author can be reached via e-mail to info@daimonin.net
*/

#include <global.h>
#include <ctype.h>

/* binary command tags are defined in the shared protocol.h */
_CmdMapping cs_commands[]    =
{
    {0,         NULL}, /* ping */

    {-1,        cs_cmd_setup},
    {1,         cs_cmd_file},
    {0,         cs_cmd_addme},
    {-1,        cs_cmd_reply},
    {-1,        cs_cmd_newchar},
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
CommArray_s Commands[]                  =
{
    {"apply",        command_apply,          1.0f},
    /* should be variable */
    {"target",       command_target,         0.1f}, /* enter combat and attack object in reach */
    {"combat",       command_combat,         0.1f}, /* toggle attack mode of player */
    {"rest",         command_resting,        1.0f},
    {"run",          command_run,            1.0f},
    {"run_stop",     command_run_stop,       0.01f},
    {"cast",         command_cast_spell,     0.0f},
    {"push",         command_push_object,    1.0f},
    {"right",        command_turn_right,     1.0f},
    {"left",         command_turn_left,      1.0f},
    /* use time comes from spells! */
    {"say",          command_say,            1.0f},
    {"gsay",         command_gsay,           1.0f},
    {"shout",        command_shout,          1.0f},
    {"tell",         command_tell,           1.0f},
    {"talk",         command_talk,           1.0f},
    {"who",          command_who,            5.0f},
    {"qlist",        command_questlist,      5.0f},
    {"mapinfo",      command_mapinfo,        5.0f},
    {"motd",         command_motd,           5.0f},
    {"usekeys",      command_usekeys,        1.0f},
    {"time",         command_time,           1.0f},
    {"version",      command_version,        1.0f},
    {"help",         command_help,           1.0f},
    {"save",         command_save,           1.0f},
    {"use_skill",    command_uskill,         0.1f},
    {"ready_skill",  command_rskill,         0.1f},
    {"silent_login", command_silent_login,   0.0f},
    {"egobind",      command_egobind,        1.0f},

#ifdef USE_CHANNELS
    {"channel",      command_channel,        1.0f}, /* channel system */
    {"createchannel", command_channel_create, 1.0f}, /* channel system */
    {"deletechannel", command_channel_delete, 1.0f}, /* channel system */
    {"channelmute",  command_channel_mute,     1.0f}, /* channel system */
#endif
#ifdef _TESTSERVER
    {"stuck",        command_stuck,          0.0},
#endif
    /* group commands */
    {"invite",        command_party_invite,    4.0f},
    {"join",            command_party_join,        0.1f},
    {"deny",            command_party_deny,        0.1f},
    {"leave",        command_party_leave,    4.0f},
    {"remove",        command_party_remove,    4.0f},

    {"dm",           command_dm,             1.0f},
    {"gm",           command_gm,             1.0f},
    {"vol",          command_vol,            1.0f},
    /* VOL/GM/DM */
	{"mutelevel",      command_mutelevel,1.0},
    {"dm_list",        command_dm_list,1.0f},
    {"malloc",            command_malloc,   0.0},        /* check the server stats */
    {"kick",            command_kickcmd, 0.0},            /* kick with a 1m temp login ban */

    {"restart",         command_restart,0.0},

    {"mute",            command_mute,1.0f},            /* max 5 min for VOL */

    /* GM/DM */
    {"summon",            command_summon,1.0},
    {"teleport",        command_teleport,1.0f},
    {"ban",            command_ban,0.0},
    {"gm_set",          command_gm_set,         0.0f},
    {"silence",        command_silence,0.0},
    {"gm_set",          command_gm_set,         0.0f},
    /*  {"sound",        command_sound,          1.0},*/
    /*  {"delete",     command_quit,           1.0},*/
    /*  {"dropall",        command_dropall,        1.0}, */

    /* temporary disabled commands
    {"listen", command_listen,    0.0}, our channel system should work different
    {"drop", command_drop,    1.0},
    {"get", command_take,     1.0},
    {"examine", command_examine,  0.5}, should work in direction
    {"statistics", command_statistics,    0.0}, will be send to client later in status
    */
};

CommArray_s CommunicationCommands[] =
{
    /* begin emotions */
    {"nod", command_nod,         1.0}, {"dance", command_dance,     1.0}, {"kiss", command_kiss,       1.0},
    {"bounce", command_bounce,   1.0}, {"smile", command_smile,     1.0}, {"cackle", command_cackle,   1.0},
    {"laugh", command_laugh,     1.0}, {"giggle", command_giggle,   1.0}, {"shake", command_shake,     1.0},
    {"puke", command_puke,       1.0}, {"growl", command_growl,     1.0}, {"scream", command_scream,   1.0},
    {"sigh", command_sigh,       1.0}, {"sulk", command_sulk,       1.0}, {"hug", command_hug,         1.0},
    {"cry", command_cry,         1.0}, {"poke", command_poke,       1.0}, {"accuse", command_accuse,   1.0},
    {"grin", command_grin,       1.0}, {"bow", command_bow,         1.0}, {"clap", command_clap,       1.0},
    {"blush", command_blush,     1.0}, {"burp", command_burp,       1.0}, {"chuckle", command_chuckle, 1.0},
    {"cough", command_cough,     1.0}, {"flip", command_flip,       1.0}, {"frown", command_frown,     1.0},
    {"gasp", command_gasp,       1.0}, {"glare", command_glare,     1.0}, {"groan", command_groan,     1.0},
    {"hiccup", command_hiccup,   1.0}, {"lick", command_lick,       1.0}, {"pout", command_pout,       1.0},
    {"shiver", command_shiver,   1.0}, {"shrug", command_shrug,     1.0}, {"slap", command_slap,       1.0},
    {"smirk", command_smirk,     1.0}, {"snap", command_snap,       1.0}, {"sneeze", command_sneeze,   1.0},
    {"snicker", command_snicker, 1.0}, {"sniff", command_sniff,     1.0}, {"snore", command_snore,     1.0},
    {"spit", command_spit,       1.0}, {"strut", command_strut,     1.0}, {"thank", command_thank,     1.0},
    {"twiddle", command_twiddle, 1.0}, {"wave", command_wave,       1.0}, {"whistle", command_whistle, 1.0},
    {"wink", command_wink,       1.0}, {"yawn", command_yawn,       1.0}, {"beg", command_beg,         1.0},
    {"bleed", command_bleed,     1.0}, {"cringe", command_cringe,   1.0}, {"think", command_think,     1.0},
    {"me", command_me,           1.0},
};

/*
 * Wizard commands (for both)
 */
CommArray_s WizCommands[]           =
{
    {"dm_set",          command_dm_set,         0.0f},
    {"plugin",command_loadplugin,0.0},
    {"pluglist",command_listplugins,0.0},

    {"inventory",        command_inventory,1.0f},    /* inv check of player x for exampel to check quest items */
    /* DM/WIZ commands */
    {"goto", command_goto,0.0},
    {"shutdown", command_start_shutdown,0.0},
    {"shutdown_now", command_shutdown, 0.0},
    {"resetmap", command_reset,0.0},
    {"plugout",command_unloadplugin,0.0},
    {"create", command_create,0.0},
    {"addexp", command_addexp,0.0},
    {"setskill", command_setskill,0.0},
    {"maps", command_maps,   0.0},
    {"dump", command_dump,0.0}, /* dump info of object nr. x */

    {"dm_invis", command_dm_invis,0.0},
    {"dm_stealth", command_dm_stealth,0.0},
    {"dm_dev", command_dm_dev,0.0},
    {"dm_light", command_dm_light,0.0},
    {"dm_pwd", command_dm_password,0.0},
    {"d_active", command_dumpactivelist,0.0},
    {"d_arches", command_dumpallarchetypes,0.0},
    {"d_maps", command_dumpallmaps,0.0},
    {"d_map", command_dumpmap,0.0},
    {"d_objects", command_dumpallobjects,0.0},
    {"d_belowfull", command_dumpbelowfull,0.0},
    {"d_below", command_dumpbelow,0.0},
    {"d_hash", command_sstable,  0.0},
    {"set_map_light", command_setmaplight,0.0},
    {"stats", command_stats,0.0},
    {"check_fd", command_check_fd,0.0},
    {"dm_speed", command_speed,0.0},
	{"dm_load",	command_dmload,0.0},


    /*
    {"archs", command_archs, 0.0},
    {"abil", command_abil,0.0},
    {"debug", command_debug,0.0},
    {"fix_me", command_fix_me,   0.0},
    {"forget_spell", command_forget_spell, 0.0},
    {"free", command_free,0.0},
    {"invisible", command_invisible,0.0},
    {"learn_special_prayer", command_learn_special_prayer, 0.0},
    {"learn_spell", command_learn_spell, 0.0},
    {"logs", command_logs,   0.0},
    {"players", command_players, 0.0},
    {"patch", command_patch,0.0},
    {"printlos", command_printlos,0.0},
    {"resistances", command_resistances, 0.0},
    {"remove", command_remove,0.0},
    {"set_god", command_setgod, 0.0},
    {"spellreset", command_spell_reset,0.0},
    {"style_info", command_style_map_info, 0.0},
    {"wizpass", command_wizpass,0.0},
    */
};

/* sort the commands for faster access */
const int   CommandsSize                = sizeof(Commands) / sizeof(CommArray_s);
const int   CommunicationCommandSize    = sizeof(CommunicationCommands) / sizeof(CommArray_s);
const int   WizCommandsSize = sizeof(WizCommands) / sizeof(CommArray_s);

static int compare_A(const void *a, const void *b)
{
    return strcmp(((CommArray_s *) a)->name, ((CommArray_s *) b)->name);
}

void init_commands()
{
    qsort((char *) Commands, CommandsSize, sizeof(CommArray_s), compare_A);
    qsort((char *) CommunicationCommands, CommunicationCommandSize, sizeof(CommArray_s), compare_A);
    qsort((char *) WizCommands, WizCommandsSize, sizeof(CommArray_s), compare_A);
}

CommArray_s * find_command_element(char *cmd, CommArray_s *commarray, int commsize)
{
	CommArray_s    *asp, dummy;
	char           *cp;

	for (cp = cmd; *cp; cp++)
		*cp = tolower(*cp);

	dummy.name = cmd;
	asp = (CommArray_s *) bsearch((void *) &dummy, (void *) commarray, commsize, sizeof(CommArray_s), compare_A);
	return asp;
}

#define DEBUG_PROCESS_QUEUE

/* We go through the list of queued commands we got from the client */
void process_command_queue(NewSocket *ns, player *pl)
{
    int cmd, cmd_count = 0;

    /* do some sanity checks ... we only allow a full enabled player to work out commands */
    if ( ns->status != Ns_Playing || pl->state!=ST_PLAYING || (pl && (!pl->ob || pl->ob->speed_left < 0.0f)))
        return;

    /* Loop through this - maybe we have several complete packets here. */
    while (ns->cmd_start)
    {
#ifdef DEBUG_PROCESS_QUEUE
        LOG(llevDebug, "process_command_queue: Found cmdptr:%x . Cmd: %d\n", ns->cmd_start, ns->cmd_start->cmd);
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
        if (cmd_count++ >= 8 || ns->status != Ns_Playing || pl->state!=ST_PLAYING || (pl && (!pl->ob || pl->ob->speed_left < 0.0f)))
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
    CommArray_s    *csp = NULL, plug_csp;
    char           *cp;
    player *pl = ns->pl;
    object *ob;

    /* we assume that our slash command is always a zero terminated string */
    if (!buf || !len || buf[len] != 0 || !pl || !pl->ob || ns->status >= Ns_Zombie)
    {
        ns->status = Ns_Dead;
        return;
    }

    ob = pl->ob;

    if (pl->state != ST_PLAYING)
    {
        new_draw_info_format(NDI_UNIQUE, 0, ob, "You can not issue commands - state is not ST_PLAYING (%s)", buf);
        return;
    }

    /* remove the command from the parameters */
    cp = strchr(buf, ' ');
    if (cp)
    {
        *(cp++) = '\0';
        cp = cleanup_string(cp);
        if (cp && *cp == '\0')
            cp = NULL;
    }

    if(find_plugin_command(buf, ob, &plug_csp))
        csp = &plug_csp;

    if (!csp)
        csp = find_command_element(buf, Commands, CommandsSize);

    if (!csp)
        csp = find_command_element(buf, CommunicationCommands, CommunicationCommandSize);

    if (!csp && QUERY_FLAG(ob, FLAG_WIZ))
        csp = find_command_element(buf, WizCommands, WizCommandsSize);

    if (csp == NULL)
    {
        new_draw_info_format(NDI_UNIQUE, 0, ob, "'%s' is not a valid command.", buf);
        return;
    }

    ob->speed_left -= csp->time;
    csp->func(ob, cp);

}

/* This is the Setup cmd - easy first implementation */
void cs_cmd_setup(char *buf, int len, NewSocket *ns)
{
    int     s;
    char   *cmd, *param, tmpbuf[MAX_DATA_TAIL_LENGTH+128], *cmdback;

    /* lets do some sanity checks */
    if (!buf || ns->status != Ns_Add || !len || buf[len-1] != 0)
    {
        LOG(llevInfo, "HACKBUG: invalid setup data part from %s (%x %d %d)\n", 
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

        if (!strcmp(cmd, "cs"))
        {
            ns->cs_version = atoi(param);
            sprintf(tmpbuf, "%d", VERSION_CS);
            strcat(cmdback, tmpbuf);
        }
        else if (!strcmp(cmd, "sc"))
        {
            ns->sc_version = atoi(param);
            sprintf(tmpbuf, "%d", VERSION_SC);
            strcat(cmdback, tmpbuf);
        }
        else if (!strcmp(cmd, "sound"))
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

    LOG(llevInfo, "SEND SETUP: %s\n",cmdback);
    SOCKBUF_REQUEST_FINISH(ns, BINARY_CMD_SETUP, strlen(cmdback));

    /* lets check the client version is ok. If not, we send back the setup command
    * but then we go in zombie mode
    */
    if (VERSION_SC != ns->sc_version || VERSION_SC != ns->cs_version)
    {
        LOG(llevInfo, "VERSION: version mismatch client:(%d,%d) server:(%d,%d)\n",
            ns->sc_version, ns->cs_version, VERSION_SC, VERSION_CS);
        ns->login_count = ROUND_TAG+(uint32)(10.0f * pticks_second);
        ns->status = Ns_Zombie; /* we hold the socket open for a *bit* */
        ns->idle_flag = 1;
        return;
    }


}

/* The client has requested to be added to the game.  This is what
* takes care of it.  We tell the client how things worked out.
* I am not sure if this file is the best place for this function.  however,
* it either has to be here or init_sockets needs to be exported.
* TODO: addme will be our one and only "put us in the game" function.
* It will be extended with options, works as a "addme login <playername> <password>"
* or a "addme create <playernam> <password> <data...>".
* The whole system of name request will be removed - also the dreaded add
* of "fake" player structs before the server knows which player should added.
* For the initial binary protocol, we leave this for now to make things easier.
*  MT-2008
*/
void cs_cmd_addme(char *buf, int len, NewSocket *ns)
{
    player     *pl=NULL;

    /* add_player() will move the login to the next step - adding a "connect"
    * as a uninitialized player AND send a get_name() which is "tell me your name".
    * ->addme flag is only a temp flag for the socket loop - because we close here
    * the socket or, when all is ok, we change to Ns_Login mode.
    */
    if (!ns->setup || ns->status != Ns_Add || len || !(pl=add_player(ns)))
    {
        Write_Command_To_Socket(ns, BINARY_CMD_ADDME_FAIL);
        ns->login_count = ROUND_TAG+(uint32)(10.0f * pticks_second);
        ns->status = Ns_Zombie; /* we hold the socket open for a *bit* */
        ns->idle_flag = 1;

    }
    else
    {
        /* lets preset our old socket so it can be given back */
        ns->addme = 1; /* mark the old socket as invalid */
        ns = &pl->socket; /* map us to our copied socket - command queue was copied too! */

        Write_Command_To_Socket(ns, BINARY_CMD_ADDME_SUC);
        LOG(llevDebug, "addme_cmd(): socket %d\n", ns->fd);
    }
}

/* This is a reply to a previous query. */
void cs_cmd_reply(char *buf, int len, NewSocket *ns)
{
    player *pl = ns->pl;
    char    write_buf[MAX_BUF];

    if (!buf || !len || !pl || ns->status >= Ns_Zombie)
    {
        ns->status = Ns_Dead;
        return;
    }

    strcpy(write_buf, ":");
    strncat(write_buf, buf, 250);
    write_buf[250] = 0;
    ns->ext_title_flag = 1;

    switch (pl->state)
    {
    case ST_PLAYING:
        ns->status = Ns_Dead;
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
        ns->status = Ns_Dead;
        LOG(llevBug, "BUG: Unknown input state: %d\n", pl->state);
        break;
    }
}

/* request a srv_file! */
void cs_cmd_file(char *buf, int len, NewSocket *ns)
{
    int id;

    /* *only* allow this command between the first login and the "addme" command! */
    if (!ns->setup || ns->status != Ns_Add || !buf || len != 1)
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

    if (!buf || len< 3*PARM_SIZE_INT || !pl || ns->status >= Ns_Zombie)
    {
        ns->status = Ns_Dead;
        return;
    }

    loc = (tag_t)GetInt_Buffer(buf);
    tag = (tag_t)GetInt_Buffer(buf);
    nrof = (long)GetInt_Buffer(buf);
    esrv_move_object(pl->ob, loc, tag, nrof);
}


void cs_cmd_newchar(char *buf, int len, NewSocket *ns)
{
    player *pl = ns->pl;

    if (!buf || !len || len > MAX_BUF || !pl || !pl->ob || ns->status >= Ns_Zombie)
    {
        ns->status = Ns_Dead;
        return;
    }

    if (pl->state != ST_CREATE_CHAR)
    {
        LOG(llevDebug, "SHACK:: %s: command_new_char send at from time\n", query_name(pl->ob));
        ns->status = Ns_Dead;
        return;
    }

    command_new_char(buf, len, pl);
}

/* TODO: merge both and fix */
#ifdef SERVER_SEND_FACES
/*
void cs_cmd_send_face(char *buff, int len, NewSocket *ns)
{
    long    tmpnum;
    short   facenum;

    if (!buff || len < PARM_SIZE_INT)
        return;

    tmpnum  = GetInt_Buffer(buff);
    facenum = (short)(tmpnum & 0xffff);

    if (facenum != 0)
        esrv_send_face(ns, facenum, 1);
}
*/
void cs_cmd_face(char *params, int len, NewSocket *ns)
{
    /*
    int i, count;

    if (!params || !len)
        return;
    count = *(uint8 *) params;
*/
    /*
    for (i = 0; i < count; i++)
    {
        if (esrv_send_face(ns, *((short *) (params + 1) + i), 0) == SEND_FACE_OUT_OF_BOUNDS)
        {
            LOG(llevInfo, "CLIENT BUG: command_face_request (%d) out of bounds. host: %s. close connection.\n",
                *((short *) (params + 1) + i), STRING_SAFE(ns->ip_host));
            ns->status = Ns_Dead;
            return;
        }
    }
    */
}
#endif

void cs_cmd_move(char *buf, int len, NewSocket *ns)
{
    player *pl = ns->pl;
    int dir, mode;

    if (!buf || len<(PARM_SIZE_CHAR*2) || !pl || !pl->ob || ns->status >= Ns_Zombie)
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

    if (!buf || len<PARM_SIZE_INT || !pl || ns->status >= Ns_Zombie)
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

    if (!buf || len<PARM_SIZE_INT || !pl || ns->status >= Ns_Zombie)
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

    if (!data || len < (PARM_SIZE_CHAR+PARM_SIZE_INT) || !pl || !pl->ob || ns->status >= Ns_Zombie)
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

    if (!data || len<PARM_SIZE_INT || !pl || !pl->ob || ns->status >= Ns_Zombie)
    {
        ns->status = Ns_Dead;
        return;
    }

    tag = GetInt_Buffer(data);
    op = esrv_get_ob_from_count(pl->ob, tag);

    if(!op || tag == pl->mark_count)
    {
        pl->mark = NULL;
        pl->mark_count = -1;
    }
    else
    {
        pl->mark = op;
        pl->mark_count = op->count;
    }

    /*LOG(-1,"MARKITEM2 (%d) (%d)\n", tag, op->count);*/
    SOCKBUF_REQUEST_BUFFER(&pl->socket, SOCKET_SIZE_SMALL);
    SockBuf_AddInt(ACTIVE_SOCKBUF(&pl->socket),  pl->mark_count);
    SOCKBUF_REQUEST_FINISH(&pl->socket, BINARY_CMD_MARK, SOCKBUF_DYNAMIC);
    /*new_draw_info_format(NDI_UNIQUE, 0, pl->ob, "Marked item %s", query_name(op));*/
}

/* The talk extended is used to "fake" a normal /talk command but use
* extra parameter to talk to the server direct or non living items instead
* of talking to a NPC.
*/
void cs_cmd_talk(char *data, int len, NewSocket *ns)
{
    player *pl = ns->pl;

    if (!data || len<2 || !pl || !pl->ob || ns->status >= Ns_Zombie)
    {
        ns->status = Ns_Dead;
        return;
    }

    /* the talk ex command has the same command as the
    * "normal" /talk, except a head part which are max. 2
    * numbers: "<mode> <count>"
    */
    if(*data == 'Q' && *(data+1)==' ') /* quest list tag */
    {
        quest_list_command(pl->ob, data+2);
    }
    else
    {
        data[len]='\0'; /* sanity string end */
        LOG(-1,"TX-CMD: unknown tag (len %d) from player %s: >%s<\n", len, query_name(pl->ob),data);
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

    if (!pl || !pl->ob || ns->status >= Ns_Zombie || !params 
        || len<(2*PARM_SIZE_INT+PARM_SIZE_CHAR) || params[len] != 0)
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
    dir = 	move_player(op, dir, FALSE);

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
