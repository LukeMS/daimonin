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
/*
 * Command parser
 */

#include <global.h>

#include <ctype.h>

/* Added times to all the commands.  However, this was quickly done,
 * and probably needs more refinements.  All socket and DM commands
 * take 0 time.
 */

/* *** The system command lists are are in /socket/loop.c **** */

/*
 * Normal game commands
 */
CommArray_s Commands[]                  =
{
    {"/stay",         command_stay,           1.0f},
    {"/n",            command_north,          1.0f},
    {"/e",            command_east,           1.0f},
    {"/s",            command_south,          1.0f},
    {"/w",            command_west,           1.0f},
    {"/ne",           command_northeast,      1.0f},
    {"/se",           command_southeast,      1.0f},
    {"/sw",           command_southwest,      1.0f},
    {"/nw",           command_northwest,      1.0f},
    {"/apply",        command_apply,          1.0f},
    /* should be variable */
    {"/target",       command_target,         0.1f}, /* enter combat and attack object in reach */
    {"/combat",       command_combat,         0.1f}, /* toggle attack mode of player */
    {"/pray",         command_praying,        0.2f},
    {"/run",          command_run,            1.0f},
    {"/run_stop",     command_run_stop,       0.01f},
    {"/cast",         command_cast_spell,     0.0f},
    /* use time comes from spells! */
    {"/say",          command_say,            1.0f},
    {"/gsay",         command_gsay,           1.0f},
    {"/shout",        command_shout,          1.0f},
    {"/tell",         command_tell,           1.0f},
    {"/talk",         command_talk,           1.0f},
	{"/who",          command_who,            5.0f},
	{"/qlist",        command_questlist,      5.0f},
    {"/mapinfo",      command_mapinfo,        5.0f},
    {"/motd",         command_motd,           5.0f},
    {"/reply",        command_reply,          1.0f},
    {"/usekeys",      command_usekeys,        1.0f},
    {"/time",         command_time,           1.0f},
    {"/version",      command_version,        1.0f},
    {"/help",         command_help,           1.0f},
    {"/save",         command_save,           1.0f},
    {"/use_skill",    command_uskill,         0.1f},
    {"/ready_skill",  command_rskill,         0.1f},
    {"/silent_login", command_silent_login,   0.0f},
	{"/egobind",	  command_egobind,		  1.0f},

    /* group commands */
    {"/invite",        command_party_invite,    4.0f},
    {"/join",            command_party_join,        0.1f},
    {"/deny",            command_party_deny,        0.1f},
    {"/leave",        command_party_leave,    4.0f},
    {"/remove",        command_party_remove,    4.0f},

    {"/dm",           command_dm,             1.0f},
    {"/gm",           command_gm,             1.0f},
    {"/vol",          command_vol,            1.0f},
    /* VOL/GM/DM */
    {"/dm_list",        command_dm_list,1.0f},
    {"/malloc",            command_malloc,   0.0},        /* check the server stats */
    {"/kick",            command_kickcmd, 0.0},            /* kick with a 1m temp login ban */

    {"/mute",            command_mute,1.0f},            /* max 5 min for VOL */

    /* GM/DM */
    {"/summon",            command_summon,1.0},
    {"/teleport",        command_teleport,1.0f},
    {"/ban",            command_ban,0.0},

    {"/silence",        command_silence,0.0},

    /*  {"/sound",        command_sound,          1.0},*/
    /*  {"/delete",     command_quit,           1.0},*/
    /*  {"/pickup",     command_pickup,         1.0}, we don't want and need this anymore */
    /*  {"/dropall",        command_dropall,        1.0}, */

    /* temporary disabled commands
    {"listen", command_listen,    0.0}, our channel system should work different
    {"drop", command_drop,    1.0},
    {"get", command_take,     1.0},
    {"hiscore", command_hiscore,  0.0}, i have to think about hiscores
    {"examine", command_examine,  0.5}, should work in direction
    {"statistics", command_statistics,    0.0}, will be send to client later in status
    */
};

const int   CommandsSize                = sizeof(Commands) / sizeof(CommArray_s);

CommArray_s CommunicationCommands[] =
{
    /* begin emotions */
    {"/nod", command_nod,         1.0}, {"/dance", command_dance,     1.0}, {"/kiss", command_kiss,       1.0},
    {"/bounce", command_bounce,   1.0}, {"/smile", command_smile,     1.0}, {"/cackle", command_cackle,   1.0},
    {"/laugh", command_laugh,     1.0}, {"/giggle", command_giggle,   1.0}, {"/shake", command_shake,     1.0},
    {"/puke", command_puke,       1.0}, {"/growl", command_growl,     1.0}, {"/scream", command_scream,   1.0},
    {"/sigh", command_sigh,       1.0}, {"/sulk", command_sulk,       1.0}, {"/hug", command_hug,         1.0},
    {"/cry", command_cry,         1.0}, {"/poke", command_poke,       1.0}, {"/accuse", command_accuse,   1.0},
    {"/grin", command_grin,       1.0}, {"/bow", command_bow,         1.0}, {"/clap", command_clap,       1.0},
    {"/blush", command_blush,     1.0}, {"/burp", command_burp,       1.0}, {"/chuckle", command_chuckle, 1.0},
    {"/cough", command_cough,     1.0}, {"/flip", command_flip,       1.0}, {"/frown", command_frown,     1.0},
    {"/gasp", command_gasp,       1.0}, {"/glare", command_glare,     1.0}, {"/groan", command_groan,     1.0},
    {"/hiccup", command_hiccup,   1.0}, {"/lick", command_lick,       1.0}, {"/pout", command_pout,       1.0},
    {"/shiver", command_shiver,   1.0}, {"/shrug", command_shrug,     1.0}, {"/slap", command_slap,       1.0},
    {"/smirk", command_smirk,     1.0}, {"/snap", command_snap,       1.0}, {"/sneeze", command_sneeze,   1.0},
    {"/snicker", command_snicker, 1.0}, {"/sniff", command_sniff,     1.0}, {"/snore", command_snore,     1.0},
    {"/spit", command_spit,       1.0}, {"/strut", command_strut,     1.0}, {"/thank", command_thank,     1.0},
    {"/twiddle", command_twiddle, 1.0}, {"/wave", command_wave,       1.0}, {"/whistle", command_whistle, 1.0},
    {"/wink", command_wink,       1.0}, {"/yawn", command_yawn,       1.0}, {"/beg", command_beg,         1.0},
    {"/bleed", command_bleed,     1.0}, {"/cringe", command_cringe,   1.0}, {"/think", command_think,     1.0},
    {"/me", command_me,           1.0},
};

const int   CommunicationCommandSize    = sizeof(CommunicationCommands) / sizeof(CommArray_s);

/*
 * Wizard commands (for both)
 */
CommArray_s WizCommands[]           =
{
    {"/dm_set",          command_dm_set,         0.0f},
    {"/plugin",command_loadplugin,0.0},
    {"/pluglist",command_listplugins,0.0},

    {"/inventory",        command_inventory,1.0f},    /* inv check of player x for exampel to check quest items */
    /* DM/WIZ commands */
    {"/goto", command_goto,0.0},
    {"/shutdown", command_start_shutdown,0.0},
    {"/shutdown_now", command_shutdown, 0.0},
    {"/resetmap", command_reset,0.0},
    {"/plugout",command_unloadplugin,0.0},
    {"/create", command_create,0.0},
	{"/addexp", command_addexp,0.0},
	{"/setskill", command_setskill,0.0},
    {"/maps", command_maps,   0.0},
    {"/dump", command_dump,0.0}, /* dump info of object nr. x */

    {"/dm_stealth", command_dm_stealth,0.0},
    {"/dm_dev", command_dm_dev,0.0},
    {"/dm_light", command_dm_light,0.0},
	{"/dm_pwd", command_dm_password,0.0},
    {"/d_active", command_dumpactivelist,0.0},
    {"/d_arches", command_dumpallarchetypes,0.0},
    {"/d_maps", command_dumpallmaps,0.0},
    {"/d_map", command_dumpmap,0.0},
    {"/d_objects", command_dumpallobjects,0.0},
    {"/d_belowfull", command_dumpbelowfull,0.0},
    {"/d_below", command_dumpbelow,0.0},
    {"/d_hash", command_sstable,  0.0},
    {"/set_map_light", command_setmaplight,0.0},
    {"/stats", command_stats,0.0},
    {"/check_fd", command_check_fd,0.0},
    {"/dm_speed", command_speed,0.0},


    /*
    {"/archs", command_archs, 0.0},
    {"/abil", command_abil,0.0},
    {"/debug", command_debug,0.0},
    {"/fix_me", command_fix_me,   0.0},
    {"/forget_spell", command_forget_spell, 0.0},
    {"/free", command_free,0.0},
    {"/invisible", command_invisible,0.0},
    {"/learn_special_prayer", command_learn_special_prayer, 0.0},
    {"/learn_spell", command_learn_spell, 0.0},
    {"/logs", command_logs,   0.0},
    {"/players", command_players, 0.0},
    {"/patch", command_patch,0.0},
    {"/printlos", command_printlos,0.0},
    {"/resistances", command_resistances, 0.0},
    {"/remove", command_remove,0.0},
    {"/set_god", command_setgod, 0.0},
    {"/spellreset", command_spell_reset,0.0},
    {"/style_info", command_style_map_info, 0.0},
    {"/wizpass", command_wizpass,0.0},
    */
};

/* *** The system command lists are are in /socket/loop.c **** */

const int   WizCommandsSize             = sizeof(WizCommands) / sizeof(CommArray_s);

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

/* Send a "remove NPC interface" command to the client.
 * For example if a player->npc communication stops because
 * the npc is gone (moved away, dead) or stops talking.
 * This is needed to end the asynchron communication way.
 */
void send_clear_interface(player *pl)
{
    SOCKET_SET_BINARY_CMD(&global_sl, BINARY_CMD_INTERFACE);
    Send_With_Handling(&pl->socket, &global_sl);
}

void initialize_command_buffer16(command_struct *cmdbuf)
{
	cmdbuf->buf = (char*)malloc(16);	
}
void initialize_command_buffer32(command_struct *cmdbuf)
{
	cmdbuf->buf = (char*)malloc(32);	
}
void initialize_command_buffer64(command_struct *cmdbuf)
{
	cmdbuf->buf = (char*)malloc(64);	
}
void initialize_command_buffer128(command_struct *cmdbuf)
{
	cmdbuf->buf = (char*)malloc(128);	
}
void initialize_command_buffer256(command_struct *cmdbuf)
{
	cmdbuf->buf = (char*)malloc(256);	
}
void initialize_command_buffer1024(command_struct *cmdbuf)
{
	cmdbuf->buf = (char*)malloc(1024);	
}
void initialize_command_buffer4096(command_struct *cmdbuf)
{
	cmdbuf->buf = (char*)malloc(4096);	
}
