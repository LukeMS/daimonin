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
#ifndef __CEXTRACT__
#include <sproto.h>
#endif
#include <ctype.h>

/* Added times to all the commands.  However, this was quickly done,
 * and probably needs more refinements.  All socket and DM commands
 * take 0 time.
 */

/*
 * Normal game commands
 */
CommArray_s Commands[] = {
  {"/stay",			command_stay,			1.0}, 
  {"/n",			command_north,			1.0},
  {"/e",			command_east,			1.0},
  {"/s",			command_south,			1.0},
  {"/w",			command_west,			1.0},
  {"/ne",			command_northeast,		1.0},
  {"/se",			command_southeast,		1.0},
  {"/sw",			command_southwest,		1.0},
  {"/nw",			command_northwest,		1.0},
  {"/apply",		command_apply,			1.0},	/* should be variable */
  {"/target",		command_target,			0.25f}, /* enter combat and attack object in reach */
  {"/combat",		command_combat,			0.25f}, /* toggle attack mode of player */
  {"/pray",			command_praying,		1.0},

  {"/run",			command_run,			1.0},
  {"/run_stop",		command_run_stop,		0.0},
  {"/cast",			command_cast_spell,		0.0},	/* use time comes from spells! */ 
  {"/say",			command_say,			1.0},
  {"/shout",		command_shout,			1.0},
  {"/tell",			command_tell,			1.0},
  {"/t_tell",		command_t_tell,			1.0},
  {"/who",			command_who,			5.0},
  {"/mapinfo",		command_mapinfo,		5.0},
  {"/motd",			command_motd,			5.0},
  {"/dm",			command_dm,				1.0},
  {"/reply",		command_reply,			1.0},
  {"/usekeys",		command_usekeys,		1.0},
  {"/time",			command_time,			1.0},
  {"/version",		command_version,		1.0},
  {"/mark",			command_mark,			1.0},
  {"/help",			command_help,			1.0},
  {"/save",			command_save,			1.0},
  {"/use_skill",	command_uskill,			0.2f},
  {"/ready_skill",	command_rskill,			0.2f},
/*  {"/sound",		command_sound,			1.0},*/
/*  {"/delete",		command_quit,			1.0},*/
/*  {"/pickup",		command_pickup,			1.0}, we don't want and need this anymore */
/*  {"/dropall",		command_dropall,		1.0}, */

 
#ifdef SEARCH_ITEMS
 /* {"/search",		command_search_items,	1.0},*/
#endif

/* temporary disabled commands
  {"party", command_party,	0.0}, need to recode the party system
  {"gsay", command_gsay,	1.0}, part of party system
  {"listen", command_listen,	0.0}, our channel system should work different
  {"drop", command_drop,	1.0},
  {"get", command_take,		1.0},
  {"skills", command_skills,	0.0},	well, the player has now a perm list
  {"output-sync", command_output_sync,	0.0},
  {"output-count", command_output_count,0.0},
  {"hiscore", command_hiscore,	0.0}, i have to think about hiscores
  {"examine", command_examine,	0.5}, should work in direction
  {"statistics", command_statistics,	0.0}, will be send to client later in status
*/

};

const int CommandsSize =sizeof(Commands) / sizeof(CommArray_s);

CommArray_s CommunicationCommands [] = {
  /* begin emotions */
  {"/nod", command_nod,			1.0},
  {"/dance", command_dance,		1.0},
  {"/kiss", command_kiss,		1.0},
  {"/bounce", command_bounce,	1.0},
  {"/smile", command_smile,		1.0},
  {"/cackle", command_cackle,	1.0},
  {"/laugh", command_laugh,		1.0},
  {"/giggle", command_giggle,	1.0},
  {"/shake", command_shake,		1.0},
  {"/puke", command_puke,		1.0},
  {"/growl", command_growl,		1.0},
  {"/scream", command_scream,	1.0},
  {"/sigh", command_sigh,		1.0},
  {"/sulk", command_sulk,		1.0},
  {"/hug", command_hug,			1.0},
  {"/cry", command_cry,			1.0},
  {"/poke", command_poke,		1.0},
  {"/accuse", command_accuse,	1.0},
  {"/grin", command_grin,		1.0},
  {"/bow", command_bow,			1.0},
  {"/clap", command_clap,		1.0},
  {"/blush", command_blush,		1.0},
  {"/burp", command_burp,		1.0},
  {"/chuckle", command_chuckle,	1.0},
  {"/cough", command_cough,		1.0},
  {"/flip", command_flip,		1.0},
  {"/frown", command_frown,		1.0},
  {"/gasp", command_gasp,		1.0},
  {"/glare", command_glare,		1.0},
  {"/groan", command_groan,		1.0},
  {"/hiccup", command_hiccup,	1.0},
  {"/lick", command_lick,		1.0},
  {"/pout", command_pout,		1.0},
  {"/shiver", command_shiver,	1.0},
  {"/shrug", command_shrug,		1.0},
  {"/slap", command_slap,		1.0},
  {"/smirk", command_smirk,		1.0},
  {"/snap", command_snap,		1.0},
  {"/sneeze", command_sneeze,	1.0},
  {"/snicker", command_snicker,	1.0},
  {"/sniff", command_sniff,		1.0},
  {"/snore", command_snore,		1.0},
  {"/spit", command_spit,		1.0},
  {"/strut", command_strut,		1.0},
  {"/thank", command_thank,		1.0},
  {"/twiddle", command_twiddle,	1.0},
  {"/wave", command_wave,		1.0},
  {"/whistle", command_whistle,	1.0},
  {"/wink", command_wink,		1.0},
  {"/yawn", command_yawn,		1.0},
  {"/beg", command_beg,			1.0},
  {"/bleed", command_bleed,		1.0},
  {"/cringe", command_cringe,	1.0},
  {"/think", command_think,		1.0},
};

const int CommunicationCommandSize = sizeof(CommunicationCommands)/ sizeof(CommArray_s);

/*
 * Wizard commands (for both)
 */
CommArray_s WizCommands [] = {
  /* OVERSEER [OV] commands */
  /* OV's should be able to help player, kick player and (temp) ban players.
   * Examine some game part (like the inventory of other players),
   * but they should NOT control the game flow. No map reset, no teleport to other
   * player, no goto to maps. 
   * This will allow to give honest players some power but they will still
   * not be able to cheat. Means, they will self be able to play on.
   */
  {"/summon", command_summon,0.0},
  {"/kick", command_kick, 0.0},
  {"/inventory", command_inventory,0.0},
  {"/plugin",command_loadplugin,0.0},
  {"/pluglist",command_listplugins,0.0},
  
  /* DM/WIZ commands */
  {"/teleport", command_teleport,0.0},
  {"/goto", command_goto,0.0},
  {"/shutdown", command_shutdown, 0.0},
  {"/reset", command_reset,0.0},
  {"/plugout",command_unloadplugin,0.0},
  {"/create", command_create,0.0},

  {"/archs", command_archs,	0.0},
  {"/abil", command_abil,0.0},
  {"/addexp", command_addexp,0.0},
  {"/debug", command_debug,0.0},
  {"/dump", command_dump,0.0},
  {"/dumpbelow", command_dumpbelow,0.0},
  {"/dumpfriendlyobjects", command_dumpfriendlyobjects,0.0},
  {"/dumpallarchetypes", command_dumpallarchetypes,0.0},
  {"/dumpallmaps", command_dumpallmaps,0.0},
  {"/dumpallobjects", command_dumpallobjects,0.0},
  {"/dumpmap", command_dumpmap,0.0},
  {"/fix_me", command_fix_me,	0.0},
  {"/forget_spell", command_forget_spell, 0.0},
  {"/free", command_free,0.0},
  {"/invisible", command_invisible,0.0},
  {"/malloc", command_malloc,	0.0},
  {"/maps", command_maps,	0.0},
  {"/learn_special_prayer", command_learn_special_prayer, 0.0},
  {"/learn_spell", command_learn_spell, 0.0},
  {"/logs", command_logs,	0.0},
  {"/overlay_save", command_save_overlay, 0.0},
  {"/players", command_players,	0.0},
  {"/patch", command_patch,0.0},
  {"/printlos", command_printlos,0.0},
  {"/resistances", command_resistances,	0.0},
  {"/remove", command_remove,0.0},
  {"/strings", command_strings,	0.0},
  {"/set_god", command_setgod, 0.0},
  {"/speed", command_speed,0.0},
  {"/spellreset", command_spell_reset,0.0},
#ifdef DEBUG
  {"/sstable", command_sstable,	0.0},
#endif
  {"/ssdumptable", command_ssdumptable,0.0},
  {"/stats", command_stats,0.0},
  {"/style_info", command_style_map_info, 0.0},	/* Costly command, so make it wiz only */
  {"/wizpass", command_wizpass,0.0},
#ifdef DEBUG_MALLOC_LEVEL
  {"/verify", command_malloc_verify,0.0},
#endif
};
const int WizCommandsSize =sizeof(WizCommands) / sizeof(CommArray_s);

static int compare_A(const void *a, const void *b)
{
  return strcmp(((CommArray_s *)a)->name, ((CommArray_s *)b)->name);
}

void init_commands()
{
  qsort((char *)Commands, CommandsSize, sizeof(CommArray_s), compare_A);
  qsort((char *)CommunicationCommands, CommunicationCommandSize, sizeof(CommArray_s), compare_A);
  qsort((char *)WizCommands, WizCommandsSize, sizeof(CommArray_s), compare_A);
}

