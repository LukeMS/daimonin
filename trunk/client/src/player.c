/*
    Daimonin SDL client, a client program for the Daimonin MMORPG.


  Copyright (C) 2003 Michael Toennies

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

/* This file handles various player related functions.  This includes
 * both things that operate on the player item, cpl structure, or
 * various commands that the player issues.
 *
 *  This file does most of the handling of commands from the client to
 *  server (see server_cmd.c for server->client)
 *
 *  does most of the work for sending messages to the server
 *   Again, most of these appear self explanatory.  Most send a bunch of
 *   commands like apply, examine, fire, run, etc.  This looks like it
 *   was done by Mark to remove the old keypress stupidity I used.
 */

/* This translates the numeric direction id's into the actual direction
 * commands.  This lets us send the actual command (ie, 'north'), which
 * makes handling on the server side easier.
 */

#include "include.h"

/*
 *  Initialiazes player item, information is received from server
 */

Client_Player   cpl;
_server_level   server_level;

player_attackredraw_t player_attackredraw[NROFATTACKS] =
{
    { "IM", "impact", MAP_REDRAW_FLAG_NO },
    { "SL", "slash", MAP_REDRAW_FLAG_NO },
    { "CL", "cleave", MAP_REDRAW_FLAG_NO },
    { "PI", "pierce", MAP_REDRAW_FLAG_NO },

    { "FI", "fire", MAP_REDRAW_FLAG_FIRE },
    { "CO", "cold", MAP_REDRAW_FLAG_COLD },
    { "EL", "electricity", MAP_REDRAW_FLAG_ELECTRICITY },
    { "PO", "poison", MAP_REDRAW_FLAG_NO },
    { "AC", "acid", MAP_REDRAW_FLAG_NO },
    { "SO", "sonic", MAP_REDRAW_FLAG_NO },

    { "CH", "channelling", MAP_REDRAW_FLAG_NO },
    { "CO", "corruption", MAP_REDRAW_FLAG_NO },
    { "PS", "psionic", MAP_REDRAW_FLAG_NO },
    { "LI", "light", MAP_REDRAW_FLAG_LIGHT },
    { "SH", "shadow", MAP_REDRAW_FLAG_SHADOW },
    { "LS", "lifesteal", MAP_REDRAW_FLAG_NO },
    
    { "AE", "aether", MAP_REDRAW_FLAG_NO },
    { "NE", "nether", MAP_REDRAW_FLAG_NO },
    { "CH", "chaos", MAP_REDRAW_FLAG_NO },
    { "DE", "death", MAP_REDRAW_FLAG_NO },

    { "WM", "weaponmagic", MAP_REDRAW_FLAG_NO },
    { "GO", "godpower", MAP_REDRAW_FLAG_NO },

    { "DR", "drain", MAP_REDRAW_FLAG_NO },
    { "DP", "depletion", MAP_REDRAW_FLAG_NO },
    { "CM", "countermagic", MAP_REDRAW_FLAG_NO },
    { "CA", "cancellation", MAP_REDRAW_FLAG_NO },
    { "CF", "confusion", MAP_REDRAW_FLAG_NO },
    { "FE", "fear", MAP_REDRAW_FLAG_NO },
    { "SL", "slow", MAP_REDRAW_FLAG_NO },
    { "PA", "paralyze", MAP_REDRAW_FLAG_NO },
    { "SN", "snare", MAP_REDRAW_FLAG_NO },

    { "??", "internal", MAP_REDRAW_FLAG_NO },
};

player_groupnames_t player_skill_group[] =
{
    { "Ag", "Agility" },
    { "Me", "Mental" },
    { "Ma", "Magical" },
    { "Pe", "Personal" },
    { "Ph", "Physical" },
    { "Wi", "Wisdom" },
    { "Mi", "Misc"},
    { NULL, NULL }
};

player_groupnames_t player_spell_group[] =
{
    { "Lf", "Life" },
    { "De", "Death" },
    { "El", "Elemental" },
    { "En", "Energy" },
    { "Sp", "Spirit" },
    { "Pr", "Protection" },
    { "Lg", "Light" },
    { "Ne", "Nether" },
    { "Na", "Nature" },
    { "Sh", "Shadow" },
    { "Ch", "Chaos" },
    { "Ea", "Earth" },
    { "Co", "Conjuration" },
    { "Ab", "Abjuration" },
    { "Tr", "Transmutation" },
    { "Ar", "Arcane" },
    { NULL, NULL }
};

void clear_player(void)
{
    memset(quick_slots, -1, sizeof(quick_slots));
    free_all_items(cpl.sack);
    free_all_items(cpl.below);
    free_all_items(cpl.ob);
    cpl.ob = player_item();
    init_player_data();
}

void new_player(uint32 tag, char *name, uint32 weight, short face)
{
    cpl.ob->tag = tag;
    cpl.ob->weight = weight;
    cpl.ob->face = face;
    copy_name(cpl.ob->d_name, name);
}


/* Show a basic help message */
void show_help()
{}

/* This is an extended command (ie, 'who, 'whatever, etc).  In general,
 * we just send the command to the server, but there are a few that
 * we care about (bind, unbind)
 *
 * The command past to us can not be modified - if it is a keybinding,
 * we get passed the string that is that binding - modifying it effectively
 * changes the binding.
 */

void extended_command(const char *ocommand)
{}

void set_weight_limit(uint32 wlim)
{
    cpl.weight_limit = wlim;
}

void init_player_data(void)
{
    int i;

    new_player(0, "", 0, 0);

    cpl.fire_on = cpl.firekey_on = 0;
    cpl.resize_twin = 0;
    cpl.resize_twin_marker = 0;
    cpl.run_on = cpl.runkey_on = 0;
    cpl.inventory_win = IWIN_BELOW;

    cpl.count_left = 0;
    cpl.container_tag = -996;
    cpl.container = NULL;
    memset(&cpl.stats, 0, sizeof(Stats));
    cpl.stats.maxsp = 1;
    cpl.stats.maxhp = 1;
    cpl.gen_hp = 0.0f;
    cpl.gen_sp = 0.0f;
    cpl.gen_grace = 0.0f;
    cpl.target_hp = 0;
    cpl.stats.hptick = cpl.stats.sptick = cpl.stats.gracetick = LastTick;

    cpl.stats.maxgrace = 1;
    cpl.stats.speed = 100.0f;
    cpl.stats.spell_fumble = 0.0f;
    cpl.input_text[0] = '\0';

    cpl.stats.hptick = cpl.stats.sptick = cpl.stats.gracetick = LastTick;

    cpl.title[0] = '\0';
    cpl.alignment[0] = '\0';
    cpl.gender[0] = '\0';
    cpl.range[0] = '\0';

    for (i = 0; i < range_size; i++)
        cpl.ranges[i] = NULL;

    cpl.map_x = 0;
    cpl.map_y = 0;

    cpl.ob->nrof = 1;

    /* this is set from title in stat cmd */
    strcpy(cpl.pname, "");
    strcpy(cpl.title, "");

    cpl.menustatus = MENU_NO;
    cpl.menustatus = MENU_NO;
    cpl.count_left = 0;
    cpl.stats.maxsp = 1;    /* avoid div by 0 errors */
    cpl.stats.maxhp = 1;    /* ditto */
    cpl.stats.maxgrace = 1; /* ditto */
    /* ditto - displayed weapon speed is weapon speed/speed */
    cpl.stats.speed = 100.0f;
    cpl.stats.spell_fumble = 0.0f;
    cpl.stats.weapon_sp = 0;
    cpl.action_time_max = 0.0f;
    cpl.action_timer = 0.0f;
    cpl.input_text[0] = '\0';
    cpl.range[0] = '\0';

    for (i = 0; i < range_size; i++)
        cpl.ranges[i] = NULL;
    cpl.map_x = 0;
    cpl.map_y = 0;
    cpl.container_tag = -997;
    cpl.container = NULL;
    cpl.magicmap = NULL;
}
