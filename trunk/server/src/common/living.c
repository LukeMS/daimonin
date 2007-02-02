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

#include <global.h>

#define RN_DEFAULT      1
#define RN_DWARVEN      2
#define RN_ELVEN        4
#define RN_GNOMISH      8
#define RN_DROW         16
#define RN_ORCISH       32
#define RN_GOBLIN       64
#define RN_KOBOLD       128

#define RN_GIANT        256
#define RN_TINY         512
#define RN_GENIE        1024
#define RN_DEMONISH     2048
#define RN_DRACONISH    4096

_races          item_race_table[RACE_NAME_INIT] =
{
    {"",             RN_DEFAULT},  /* default value - human like */
    {"dwarven ",     RN_DWARVEN}, {"elven ",       RN_ELVEN}, {"gnomish ",     RN_GNOMISH}, {"drow ",        RN_DROW},
    {"orcish ",      RN_ORCISH}, {"goblin ",      RN_GOBLIN}, {"kobold ",      RN_KOBOLD},
    /* count also as tiny, but "unclean" */
    {"giant ",       RN_GIANT},  /* all demihumans "bigger as humans" */
    {"tiny ",        RN_TINY},  /* different small race (sprites, ...) */
    {"demonish ",    RN_DEMONISH},  /* this is usable from all sizes */
    {"draconish ",   RN_DRACONISH},   /* usable from all sizes */
    {"ogre ",       RN_GIANT}     /* count as giant */
};

/* when we carry more as this of our weight_limit, we get encumbered. */
#define ENCUMBRANCE_LIMIT 65.0f


int             dam_bonus[MAX_STAT + 1]         =
{
    50, 45, 40, 36, 33, 30, 26, 23, 20, 15, 10,/* 0-10: dmg malus */
        0, 1, 2, 3, 5, /* 11-15 = not much */
        10, 15, 20, 25, 30, 33, 36, 40, 45, 50, 60, 70, 80, 100, 120 /* 16-30 = good boni */
};

/* DEX  - wc boni */
int             thaco_bonus[MAX_STAT + 1]       =
{
    -5, -4, -4, -3, -3, -3, -2, -2, -2, -1, -1,/* 0-10: dmg malus */
        0, 0, 0, 0, 0, /* 11-15 = nothing */
    1, 1, 2, 2, 3, 3, 3, 4, 4, 5, 5, 5, 6, 7, 8 /* 16-30 = boni */
};

/* this is only used here for players */

/* CON - % of "real max hp" boni */
static float    con_bonus[MAX_STAT + 1]         =
{
    -0.8f, -0.6f, -0.5f, -0.4f, -0.35f, -0.3f, -0.25f, -0.2f, -0.15f, -0.11f, -0.07f, /* 0-10: mali */
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.1f, 0.15f, 0.2f, 0.25f, 0.3f, 0.35f, 0.4f, 0.45f, 0.5f, 0.55f, 0.6f, 0.7f, 0.8f,
    0.9f, 1.0f
};

static float    pow_bonus[MAX_STAT + 1]         =
{
    -0.8f, -0.6f, -0.5f, -0.4f, -0.35f, -0.3f, -0.25f, -0.2f, -0.15f, -0.11f, -0.07f, /* 0-10: mali */
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f, 1.0f, 1.1f, 1.4f, 1.6f, 1.8f,
    2.0f
};

static float    wis_bonus[MAX_STAT + 1]         =
{
    -0.8f, -0.6f, -0.5f, -0.4f, -0.35f, -0.3f, -0.25f, -0.2f, -0.15f, -0.11f, -0.07f, /* 0-10: mali */
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f, 1.0f, 1.1f, 1.4f, 1.6f, 1.8f,
    2.0f
};

/* new charisma system. As base value, you got in a shop 20% of the real
 * of a item. With this system, you can go up to 25% maximal or 5% minimum.
 * For buying, they are used reversed. you can buy for 95% as best or
 * 115% as worst.
 * This system use heavy user based economic - for player <-> NPC, we will
 * suffer badly value & payment differences.
 */
float           cha_bonus[MAX_STAT + 1]         =
{
    -0.15f, /* cha = 0 */ - 0.10f, -0.08f, -0.05f, -0.03f, -0.02f, /* 1-5*/ - 0.01f, -0.005f, -0.003f, -0.001f, 0.0f,
    /* 6-10*/
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, /* 11-15*/
    0.003f, 0.005f, 0.009f, 0.01f, 0.012f, /* 16-20*/
    0.014f, 0.016f, 0.019f, 0.021f, 0.023f, /* 21-25*/
    0.025f, 0.03f, 0.035f, 0.04f, 0.05f  /* 25-30*/
};


/* speed_bonus uses dex as its stat */
float           speed_bonus[MAX_STAT + 1]       =
{
    -0.4f, -0.4f, -0.3f, -0.3f, -0.2f, -0.2f, -0.2f, -0.1f, -0.1f, -0.1f, -0.05f, 0.0, 0.0f, 0.0f, 0.025f, 0.05f,
    0.075f, 0.1f, 0.125f, 0.15f, 0.175f, 0.2f, 0.225f, 0.25f, 0.275f, 0.3f, 0.325f, 0.35f, 0.4f, 0.45f, 0.5f
};

/* weight_limit - the absolute most a character can carry - a character can't
 * pick stuff up if it would put him above this limit.
 * value is in grams, so we don't need to do conversion later
 * These limits are probably overly generous, but being there were no values
 * before, you need to start someplace.
 */

uint32          weight_limit[MAX_STAT + 1]          =
{
    20000,  /* 0 */
    25000, 30000, 35000, 40000, 50000,      /* 5*/
    60000, 70000, 80000, 90000, 100000,    /* 10 */
    110000, 120000, 130000, 140000, 150000,/* 15 */
    165000, 180000, 195000, 210000, 225000,/* 20 */
    240000, 255000, 270000, 285000, 300000, /* 25 */
    325000, 350000, 375000, 400000, 450000  /*30 */
};

int             learn_spell[MAX_STAT + 1]       =
{
    0, 0, 0, 1, 2, 4, 8, 12, 16, 25, 36, 45, 55, 65, 70, 75, 80, 85, 90, 95, 100, 100, 100, 100, 100, 100, 100, 100,
    100, 100, 100
};
int             cleric_chance[MAX_STAT + 1]     =
{
    100, 100, 100, 100, 90, 80, 70, 60, 50, 40, 35, 30, 25, 20, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0
};
int             turn_bonus[MAX_STAT + 1]        =
{
    -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 5, 5, 6, 7, 8, 9, 10, 12, 15
};


float           lev_damage[MAXLEVEL + 1]            =
{
    1.0f, 1.0f, 1.25f, 1.5f, 1.75f, 2.0f, 2.25f, 2.5f, 2.75f, 3.0f, 3.25f, 3.5f, 3.75f, 4.0f, 4.25f, 4.5f, 4.75f, 5.0f,
    5.25f, 5.5f, 5.75f, 6.0f, 6.25f, 6.5f, 6.75f, 7.0f, 7.25f, 7.5f, 7.75f, 8.0f, 8.25f, 8.5f, 8.75f, 9.0f, 9.25f, 9.5f,
    9.75f, 10.0f, 10.25f, 10.5f, 10.75f, 11.0f, 11.25f, 11.5f, 11.75f, 12.0f, 12.25f, 12.5f, 12.75f, 13.0f, 13.25f,
    /* 50*/
    13.5f, 13.75f, 14.0f, 14.25f, 14.5f, 14.75f, 15.0f, 15.25f, 15.5f, 15.75f, 16.0f, 16.25f, 16.5f, 16.75f, 17.0f,
    17.25f, 17.5f, 17.75f, 18.0f, 18.25f, 18.5f, 18.75f, 19.0f, 19.25f, 19.5f, 19.75f, 20.0f, 20.25f, 20.5f, 20.75f,
    21.0f, 21.25f, 21.5f, 21.75f, 22.0f, 22.25f, 22.5f, 22.75f, 23.0f, 23.25f, 23.5f, 23.75f, 24.0f, 24.25f, 24.5f,
    24.75f, 25.0f, 25.25f, 25.5f, 25.75f, 26.0f, 26.25f, 26.5f, 26.75f, 27.0f, 27.25f, 27.5f, 27.75f, 28.0f, 28.25f
};


/* Max level is 100.  By making it 101, it means values 0->100 are valid.
 * Thus, we can use op->level directly, and it also works for level 0 people.
 */
int             savethrow[MAXLEVEL + 1]             =
{
    18, 18, 17, 16, 15, 14, 14, 13, 13, 12, 12, 12, 11, 11, 11, 11, 10, 10, 10, 10, 9, 9, 9, 9, 9, 8, 8, 8, 8, 8, 8, 7,
    7, 7, 7, 7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 5, 5, 5, 5, 5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3,
    3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1
};

char           *attacks[NROFATTACKS]            =
{
    "hit", "magical", "fire", "electricity", "cold", "confusion", "acid", "drain", "weaponmagic", "ghosthit", "poison",
    "slow", "paralyze", "turn undead", "fear", "cancellation", "depletion", "death", "chaos", "counterspell",
    "god power", "holy power", "blinding", "", "life stealing", "slash", "cleave", "pierce"
};

static char    *drain_msg[7]                    =
{
    "Oh no! You are weakened!", "You're feeling clumsy!", "You feel less healthy",
    "You suddenly begin to lose your memory!", "Your face gets distorted!", "Watch out, your mind is going!",
    "Your spirit feels drained!"
};
char           *restore_msg[7]                  =
{
    "You feel your strength return.", "You feel your agility return.", "You feel your health return.",
    "You feel your wisdom return.", "You feel your charisma return.", "You feel your memory return.",
    "You feel your spirits return."
};
char           *gain_msg[7]                     =
{
    "You feel stronger.", "You feel more agile.", "You feel healthy.", "You feel wiser.", "You seem to look better.",
    "You feel smarter.", "You feel more potent."
};
char           *lose_msg[7]                     =
{
    "You feel weaker!", "You feel clumsy!", "You feel less healthy!", "You lose some of your memory!", "You look ugly!",
    "You feel stupid!", "You feel less potent!"
};

char           *statname[7]                     =
{
    "strength", "dexterity", "constitution", "wisdom", "charisma", "intelligence", "power"
};

char           *short_stat_name[7]              =
{
    "Str", "Dex", "Con", "Wis", "Cha", "Int", "Pow"
};

/*
 * sets Str/Dex/con/Wis/Cha/Int/Pow in stats to value, depending on
 * what attr is (STR to POW).
 */

void set_attr_value(living *stats, int attr, signed char value)
{
    switch (attr)
    {
        case STR:
          stats->Str = value;
          break;
        case DEX:
          stats->Dex = value;
          break;
        case CON:
          stats->Con = value;
          break;
        case WIS:
          stats->Wis = value;
          break;
        case POW:
          stats->Pow = value;
          break;
        case CHA:
          stats->Cha = value;
          break;
        case INTELLIGENCE:
          stats->Int = value;
          break;
    }
}

/*
 * Like set_attr_value(), but instead the value (which can be negative)
 * is added to the specified stat.
 */

void change_attr_value(living *stats, int attr, signed char value)
{
    if (value == 0)
        return;
    switch (attr)
    {
        case STR:
          stats->Str += value;
          break;
        case DEX:
          stats->Dex += value;
          break;
        case CON:
          stats->Con += value;
          break;
        case WIS:
          stats->Wis += value;
          break;
        case POW:
          stats->Pow += value;
          break;
        case CHA:
          stats->Cha += value;
          break;
        case INTELLIGENCE:
          stats->Int += value;
          break;
        default:
          LOG(llevBug, "BUG: Invalid attribute in change_attr_value: %d\n", attr);
    }
}

/*
 * returns the specified stat.  See also set_attr_value().
 */

signed char get_attr_value(const living *const stats, const int attr)
{
    switch (attr)
    {
        case STR:
          return(stats->Str);
        case DEX:
          return(stats->Dex);
        case CON:
          return(stats->Con);
        case WIS:
          return(stats->Wis);
        case CHA:
          return(stats->Cha);
        case INTELLIGENCE:
          return(stats->Int);
        case POW:
          return(stats->Pow);
    }
    return 0;
}

/*
 * Ensures that all stats (str/dex/con/wis/cha/int) are within the
 * 1-30 stat limit.
 * not so "smart" as the solution before but simple, fast and easy.
 * MT-2004
 */

void check_stat_bounds(living *stats)
{
    if (stats->Str > MAX_STAT)
        stats->Str = MAX_STAT;
    else if (stats->Str < MIN_STAT)
        stats->Str = MIN_STAT;

    if (stats->Dex > MAX_STAT)
        stats->Dex = MAX_STAT;
    else if (stats->Dex < MIN_STAT)
        stats->Dex = MIN_STAT;

    if (stats->Con > MAX_STAT)
        stats->Con = MAX_STAT;
    else if (stats->Con < MIN_STAT)
        stats->Con = MIN_STAT;

    if (stats->Int > MAX_STAT)
        stats->Int = MAX_STAT;
    else if (stats->Int < MIN_STAT)
        stats->Int = MIN_STAT;

    if (stats->Wis > MAX_STAT)
        stats->Wis = MAX_STAT;
    else if (stats->Wis < MIN_STAT)
        stats->Wis = MIN_STAT;

    if (stats->Pow > MAX_STAT)
        stats->Pow = MAX_STAT;
    else if (stats->Pow < MIN_STAT)
        stats->Pow = MIN_STAT;

    if (stats->Cha > MAX_STAT)
        stats->Cha = MAX_STAT;
    else if (stats->Cha < MIN_STAT)
        stats->Cha = MIN_STAT;
}

#define ORIG_S(xyz,abc) (CONTR(op)->orig_stats.abc)

/* return 1 if we sucessfully changed a stat, 0 if nothing was changed. */
/* flag is set to 1 if we are applying the object, -1 if we are removing
 * the object.
 * It is the calling functions responsibilty to check to see if the object
 * can be applied or not.
 */
int change_abil(object *op, object *tmp)
{
    int flag = QUERY_FLAG(tmp,  FLAG_APPLIED) ? 1 : -1,i,j,success = 0;
    object                      refop;
    char                        message[MAX_BUF];
    int                         potion_max  = 0;

    /* remember what object was like before it was changed.  note that
     * refop is a local copy of op only to be used for detecting changes
     * found by fix_player.  refop is not a real object */
    memcpy(&refop, op, sizeof(object));

    if (op->type == PLAYER)
    {
        if (tmp->type == POTION)
        {
            for (j = 0; j < 7; j++)
            {
                i = get_attr_value(&(CONTR(op)->orig_stats), j);

                /* Check to see if stats are within limits such that this can be
                 * applied.
                 */
                if (((i + flag * get_attr_value(&(tmp->stats), j))
                  <= (20 + tmp->stats.sp + get_attr_value(&(op->arch->clone.stats), j)))
                 && i
                  > 0)
                {
                    change_attr_value(&(CONTR(op)->orig_stats), j,
                                      (signed char) (flag * get_attr_value(&(tmp->stats), j)));
                    tmp->stats.sp = 0;/* Fix it up for super potions */
                }
                else
                {
                    /* potion is useless - player has already hit the natural maximum */
                    potion_max = 1;
                }
            }
            /* This section of code ups the characters normal stats also.  I am not
             * sure if this is strictly necessary, being that fix_player probably
             * recalculates this anyway.
             */
            for (j = 0; j < 7; j++)
                change_attr_value(&(op->stats), j, (signed char) (flag * get_attr_value(&(tmp->stats), j)));
            check_stat_bounds(&(op->stats));
        } /* end of potion handling code */
    }

    /* reset attributes that fix_player doesn't reset since it doesn't search
     * everything to set */
    if (flag == -1)
    {
        op->path_attuned &= ~tmp->path_attuned,
        op->path_repelled &= ~tmp->path_repelled,
        op->path_denied &= ~tmp->path_denied;
    }
    /* call fix_player since op object could have whatever attribute due
     * to multiple items.  if fix_player always has to be called after
     * change_ability then might as well call it from here
     */
    FIX_PLAYER(op, "change_abil");

    if (tmp->attack[ATNR_CONFUSION])
    {
        success = 1;
        if (flag > 0)
            new_draw_info(NDI_UNIQUE | NDI_WHITE, 0, op, "Your hands begin to glow red.");
        else
            new_draw_info(NDI_UNIQUE | NDI_GREY, 0, op, "Your hands stop glowing red.");
    }
    if (QUERY_FLAG(op, FLAG_LIFESAVE) != QUERY_FLAG(&refop, FLAG_LIFESAVE))
    {
        success = 1;
        if (flag > 0)
        {
            new_draw_info(NDI_UNIQUE | NDI_WHITE, 0, op, "You feel very protected.");
        }
        else
        {
            new_draw_info(NDI_UNIQUE | NDI_GREY, 0, op, "You don't feel protected anymore.");
        }
    }
    if (QUERY_FLAG(op, FLAG_CAN_REFL_MISSILE) != QUERY_FLAG(&refop, FLAG_CAN_REFL_MISSILE))
    {
        success = 1;
        if (flag > 0)
        {
            new_draw_info(NDI_UNIQUE | NDI_WHITE, 0, op, "A magic force shimmers around you.");
        }
        else
        {
            new_draw_info(NDI_UNIQUE | NDI_GREY, 0, op, "The magic force fades away.");
        }
    }
    if (QUERY_FLAG(op, FLAG_CAN_REFL_SPELL) != QUERY_FLAG(&refop, FLAG_CAN_REFL_SPELL))
    {
        success = 1;
        if (flag > 0)
        {
            new_draw_info(NDI_UNIQUE | NDI_WHITE, 0, op, "You feel more safe now, somehow.");
        }
        else
        {
            new_draw_info(NDI_UNIQUE | NDI_GREY, 0, op, "Suddenly you feel less safe, somehow.");
        }
    }
    if (QUERY_FLAG(tmp, FLAG_FLYING))
    {
        if (flag > 0)
        {
            success = 1;
            /* if were already flying then now flying higher */
            if (QUERY_FLAG(op, FLAG_FLYING) == QUERY_FLAG(&refop, FLAG_FLYING))
                new_draw_info(NDI_UNIQUE | NDI_WHITE, 0, op, "You fly a little higher in the air.");
            else
            {
                new_draw_info(NDI_UNIQUE | NDI_GREY, 0, op, "You start to fly in the air!.");
                SET_MULTI_FLAG(op, FLAG_FLYING);
                if (op->speed > 1)
                    op->speed = 1;
            }
        }
        else
        {
            success = 1;
            /* if were already flying then now flying lower */
            if (QUERY_FLAG(op, FLAG_FLYING) == QUERY_FLAG(&refop, FLAG_FLYING))
                new_draw_info(NDI_UNIQUE | NDI_WHITE, 0, op, "You fly a little lower in the air.");
            else
            {
                new_draw_info(NDI_UNIQUE | NDI_GREY, 0, op, "You fly down to the ground.");
                check_walk_on(op, op, 0);
            }
        }
    }

    if (QUERY_FLAG(tmp, FLAG_LEVITATE))
    {
        if (flag > 0)
        {
            success = 1;
            /* if were already flying then now flying higher */
            if (QUERY_FLAG(op, FLAG_FLYING) == QUERY_FLAG(&refop, FLAG_FLYING))
                new_draw_info(NDI_UNIQUE | NDI_WHITE, 0, op, "You float a little higher in the air.");
            else
            {
                new_draw_info(NDI_UNIQUE | NDI_GREY, 0, op, "You start to float in the air!.");
                SET_MULTI_FLAG(op, FLAG_FLYING);
                if (op->speed > 1)
                    op->speed = 1;
            }
        }
        else
        {
            success = 1;
            /* if were already flying then now flying lower */
            if (QUERY_FLAG(op, FLAG_FLYING) == QUERY_FLAG(&refop, FLAG_FLYING))
                new_draw_info(NDI_UNIQUE | NDI_WHITE, 0, op, "You float a little lower in the air.");
            else
            {
                new_draw_info(NDI_UNIQUE | NDI_GREY, 0, op, "You float down to the ground.");
                check_walk_on(op, op, 0);
            }
        }
    }

    /* becoming UNDEAD... a special treatment for this flag. Only those not
     * originally undead may change their status */
    if (!QUERY_FLAG(&op->arch->clone, FLAG_UNDEAD))
        if (QUERY_FLAG(op, FLAG_UNDEAD) != QUERY_FLAG(&refop, FLAG_UNDEAD))
        {
            success = 1;
            if (flag > 0)
            {
                FREE_AND_COPY_HASH(op->race, "undead");
                new_draw_info(NDI_UNIQUE | NDI_GREY, 0, op, "Your lifeforce drains away!");
            }
            else
            {
                FREE_AND_CLEAR_HASH(op->race);
                if (op->arch->clone.race)
                    FREE_AND_COPY_HASH(op->race, op->arch->clone.race);
                new_draw_info(NDI_UNIQUE | NDI_WHITE, 0, op, "Your lifeforce returns!");
            }
        }

    if (QUERY_FLAG(op, FLAG_STEALTH) != QUERY_FLAG(&refop, FLAG_STEALTH))
    {
        success = 1;
        if (flag > 0)
        {
            new_draw_info(NDI_UNIQUE | NDI_WHITE, 0, op, "You walk more quietly.");
        }
        else
        {
            new_draw_info(NDI_UNIQUE | NDI_GREY, 0, op, "You walk more noisily.");
        }
    }
    if (QUERY_FLAG(op, FLAG_SEE_INVISIBLE) != QUERY_FLAG(&refop, FLAG_SEE_INVISIBLE))
    {
        success = 1;
        if (flag > 0)
        {
            new_draw_info(NDI_UNIQUE | NDI_WHITE, 0, op, "You see invisible things.");
        }
        else
        {
            new_draw_info(NDI_UNIQUE | NDI_GREY, 0, op, "Your vision becomes less clear.");
        }
    }
    if (QUERY_FLAG(op, FLAG_IS_INVISIBLE) != QUERY_FLAG(&refop, FLAG_IS_INVISIBLE))
    {
        success = 1;
        if (flag > 0)
        {
            new_draw_info(NDI_UNIQUE | NDI_WHITE, 0, op, "You become transparent.");
        }
        else
        {
            new_draw_info(NDI_UNIQUE | NDI_GREY, 0, op, "You can see yourself.");
        }
    }
    /* blinded you can tell if more blinded since blinded player has minimal
     * vision */
    if (QUERY_FLAG(tmp, FLAG_BLIND))
    {
        success = 1;
        if (flag > 0)
        {
            if (QUERY_FLAG(op, FLAG_WIZ))
                new_draw_info(NDI_UNIQUE | NDI_GREY, 0, op, "Your mortal self is blinded.");
            else
            {
                new_draw_info(NDI_UNIQUE | NDI_GREY, 0, op, "You are blinded.");
                SET_FLAG(op, FLAG_BLIND);
                if (op->type == PLAYER)
                    CONTR(op)->update_los = 1;
            }
        }
        else
        {
            if (QUERY_FLAG(op, FLAG_WIZ))
                new_draw_info(NDI_UNIQUE | NDI_WHITE, 0, op, "Your mortal self can now see again.");
            else
            {
                new_draw_info(NDI_UNIQUE | NDI_WHITE, 0, op, "Your vision returns.");
                CLEAR_FLAG(op, FLAG_BLIND);
                if (op->type == PLAYER)
                    CONTR(op)->update_los = 1;
            }
        }
    }

    if (QUERY_FLAG(op, FLAG_SEE_IN_DARK) != QUERY_FLAG(&refop, FLAG_SEE_IN_DARK))
    {
        success = 1;
        if (flag > 0)
        {
            new_draw_info(NDI_UNIQUE | NDI_WHITE, 0, op, "Your vision is better in the dark.");
        }
        else
        {
            new_draw_info(NDI_UNIQUE | NDI_GREY, 0, op, "You see less well in the dark.");
        }
    }

    if (QUERY_FLAG(op, FLAG_XRAYS) != QUERY_FLAG(&refop, FLAG_XRAYS))
    {
        success = 1;
        if (flag > 0)
        {
            if (QUERY_FLAG(op, FLAG_WIZ))
                new_draw_info(NDI_UNIQUE | NDI_WHITE, 0, op, "Your vision becomes a little clearer.");
            else
            {
                new_draw_info(NDI_UNIQUE | NDI_GREY, 0, op, "Everything becomes transparent.");
                if (op->type == PLAYER)
                    CONTR(op)->update_los = 1;
            }
        }
        else
        {
            if (QUERY_FLAG(op, FLAG_WIZ))
                new_draw_info(NDI_UNIQUE | NDI_WHITE, 0, op, "Your vision becomes a bit out of focus.");
            else
            {
                new_draw_info(NDI_UNIQUE | NDI_GREY, 0, op, "Everything suddenly looks very solid.");
                if (op->type == PLAYER)
                    CONTR(op)->update_los = 1;
            }
        }
    }
    if ((tmp->stats.hp || tmp->stats.maxhp) && op->type == PLAYER)
    {
        success = 1;
        if (flag * tmp->stats.hp > 0 || flag * tmp->stats.maxhp > 0)
            new_draw_info(NDI_UNIQUE | NDI_WHITE, 0, op, "You feel much more healthy!");
        else
            new_draw_info(NDI_UNIQUE | NDI_GREY, 0, op, "You feel much less healthy!");
    }
    if ((tmp->stats.sp || tmp->stats.maxsp) && op->type == PLAYER && tmp->type != SKILL)
    {
        success = 1;
        if (flag * tmp->stats.sp > 0 || flag * tmp->stats.maxsp > 0)
            new_draw_info(NDI_UNIQUE | NDI_WHITE, 0, op, "You feel one with the powers of magic!");
        else
            new_draw_info(NDI_UNIQUE | NDI_GREY, 0, op, "You suddenly feel very mundane.");
    }
    /* for the future when artifacts set this -b.t. */
    if ((tmp->stats.grace || tmp->stats.maxgrace) && op->type == PLAYER)
    {
        success = 1;
        if (flag * tmp->stats.grace > 0 || flag * tmp->stats.maxgrace)
            new_draw_info(NDI_UNIQUE | NDI_WHITE, 0, op, "You feel closer to your deity!");
        else
            new_draw_info(NDI_UNIQUE | NDI_GREY, 0, op, "You suddenly feel less holy.");
    }
    if (tmp->stats.food && op->type == PLAYER && tmp->type != POISONING && tmp->type != POTION_EFFECT)
    {
        success = 1;
        if (tmp->stats.food * flag > 0)
            new_draw_info(NDI_UNIQUE | NDI_WHITE, 0, op, "You feel your digestion slowing down.");
        else
            new_draw_info(NDI_UNIQUE | NDI_GREY, 0, op, "You feel your digestion speeding up.");
    }

    /* Messages for changed resistance */
    for (i = 0; i < NROFATTACKS; i++)
    {
        if (op->resist[i] != refop.resist[i])
        {
            success = 1;
            if (op->resist[i] > refop.resist[i])
            {
                sprintf(message, "Your resistance to %s rises to %d%%.", attack_name[i], op->resist[i]);
                new_draw_info(NDI_UNIQUE | NDI_GREEN, 0, op, message);
            }
            else
            {
                sprintf(message, "Your resistance to %s drops to %d%%.", attack_name[i], op->resist[i]);
                new_draw_info(NDI_UNIQUE | NDI_BLUE, 0, op, message);
            }
        }
    }

    if (tmp->type != EXPERIENCE && !potion_max)
    {
        for (j = 0; j < 7; j++)
        {
            if ((i = get_attr_value(&(tmp->stats), j)) != 0)
            {
                success = 1;
                if (i * flag > 0)
                    new_draw_info(NDI_UNIQUE | NDI_WHITE, 0, op, gain_msg[j]);
                else
                    new_draw_info(NDI_UNIQUE | NDI_GREY, 0, op, lose_msg[j]);
            }
        }
    }
    return success;
}

/* drain (corrupt) mana and/or grace
 * for 10% + random(10%).
 */
void corrupt_stat(object *op)
{
    if(op->stats.sp > op->stats.grace) /* drain mana */
    {
        /* sp - 10% to 20% */
        op->stats.sp = (int)((1.0f -(0.1f+( ((float)(RANDOM()%11))*0.01f)))*(float)op->stats.sp);
        if(op->type == PLAYER)
            new_draw_info(NDI_UNIQUE, 0, op, "You lose some mana!");
    }
    else if(op->stats.grace) /* drain grace */
    {
        /* grace - 10% to 20% */
        op->stats.grace = (int)((1.0f -(0.1f+( ((float)(RANDOM()%11))*0.01f)))*(float)op->stats.grace);
        if(op->type == PLAYER)
            new_draw_info(NDI_UNIQUE, 0, op, "You lose some grace!");
    }
}

/*
 * Stat draining by Vick 930307
 * (Feeling evil, I made it work as well now.  -Frank 8)
 */

void drain_stat(object *op)
{
    drain_specific_stat(op, RANDOM() % 7);
}

void drain_specific_stat(object *op, int deplete_stats)
{
    object     *tmp;
    static archetype  *at = NULL;

    if (!at)
    {
        at = find_archetype("depletion");
        if (!at)
        {
            LOG(llevBug, "BUG: Couldn't find archetype depletion.\n");
            return;
        }
    }

    tmp = present_arch_in_ob(at, op);
    if (!tmp)
    {
        tmp = arch_to_object(at);
        tmp = insert_ob_in_ob(tmp, op);
        SET_FLAG(tmp, FLAG_APPLIED);

        if(op->type != PLAYER) /* create a temp., self destructing force for mobs */
        {
            tmp->stats.food = 122+(RANDOM()%70);
            tmp->speed = 1.0;
            update_ob_speed(tmp);
            SET_FLAG(tmp, FLAG_IS_USED_UP);
        }
    }

    change_attr_value(&tmp->stats, deplete_stats, -1);
    if(op->type == PLAYER)
        new_draw_info(NDI_UNIQUE, 0, op, drain_msg[deplete_stats]);

    FIX_PLAYER(op, "drain_specific_stat");
}

/* Drain the "real" level of a player or mob.
 * This can be permanent or temporary (if we use drain on a mob).
 * It works more or less like the depletion force
 * mode: 0 means permanent(until active removed), 1 means temporary,
 * then the effect removes itself.
 */
void drain_level(object *op, int level, int mode, int ticks)
{
    object *force;
    static archetype  *at = NULL;

    if (!at)
    {
        at = find_archetype("drain");
        if (!at)
        {
            LOG(llevBug, "BUG: Couldn't find archetype drain.\n");
            return;
        }
    }

    if(mode==0)
        force = present_arch_in_ob(at, op);
    else
        force = present_arch_in_ob_temp(at, op);
    if (!force)
    {
        force = arch_to_object(at);
        force = insert_ob_in_ob(force, op);
        SET_FLAG(force, FLAG_APPLIED);

        if(mode) /* create a temp., self destructing force */
        {
            force->stats.food = ticks;
            force->speed = 1.0;
            update_ob_speed(force);
            SET_FLAG(force, FLAG_IS_USED_UP);
        }
    }

    force->level += level;
    FIX_PLAYER(op, "drain_level"); /* will redirect to fix_monster() automatically */
    if(op->type == PLAYER)
        new_draw_info(NDI_UNIQUE, 0, op, "You lose a level!");
}


/*
 * Updates all abilities given by applied objects in the inventory
 * of the given object.  Note: This function works for both monsters
 * and players; the "player" in the name is purely an archaic inheritance.
 */
/* July 95 - inserted stuff to handle new skills/exp system - b.t.
   spell system split, grace points now added to system  --peterm
 */
/* Oct 02 - i reworked this function in all parts. Beside a major speed up, i split it
 * in a player and a monster function. All calls goes still to fix_player, but it filters
 * the mobs out and call fix_monster. This function is still a heavy weight. There
 * is a lot of abuse and redundant call of this function, so it is worth to monitor it. MT
 */
#ifdef DEBUG_FIX_PLAYER
void fix_player(object *op, char *debug_msg)
#else
void fix_player(object *op)
#endif
{
    int                 snare_penalty = 0,slow_penalty = 0, ring_count = 0, skill_level_drain=0, skill_level_max = 1;
    int                 tmp_item, old_glow, max_boni_hp = 0, max_boni_sp = 0, max_boni_grace = 0;
    int                 i, j, inv_flag, inv_see_flag, light, weapon_weight, best_wc, best_ac, wc, ac, base_reg;
    int                 resists_boni[NROFATTACKS], resists_mali[NROFATTACKS], thac0=0, thacm=0;
    int                 potion_resist_boni[NROFATTACKS], potion_resist_mali[NROFATTACKS], potion_attack[NROFATTACKS];
    object             *tmp, *tmp_ptr, *skill_weapon = NULL;
    player             *pl;
    float               f, max = 9, added_speed = 0, bonus_speed = 0, speed_reduce_from_disease = 1, tmp_con, reg_sec;


    /*LOG(llevDebug,"FIX_PLAYER called (%s} %s\n", query_name(op), QUERY_FLAG(op, FLAG_NO_FIX_PLAYER)?"IGNORED":"");*/
    if (QUERY_FLAG(op, FLAG_NO_FIX_PLAYER))
    {
        /* we are calling fix_player with this flag for example when manually applying an item */
#ifdef DEBUG_FIX_PLAYER_SKIPPED
        LOG(llevDebug, "FIX_PLAYER(%s [%x]): >> *SKIP*\n", query_name(op), op->count);
#endif
        return;
    }
    /* ok, in crossfire, fix_player is called for objects not for players
     * we redirect mobs to fix_monster() and let only player pass
     */
    if (QUERY_FLAG(op, FLAG_MONSTER) && op->type != PLAYER)
    {
#ifdef DEBUG_FIX_PLAYER
        LOG(llevDebug, "fix_player(%s [%x]): >> non player - redirect to fix_monster (%s)\n", 
                query_name(op),op->count, debug_msg);
#else
        LOG(llevDebug, "fix_player(%s [%x]): >> non player - redirect to fix_monster\n", 
                query_name(op),op->count);
#endif
        fix_monster(op);
        return;
    }

    /* for secure */
    if (op->type != PLAYER)
    {
#ifdef DEBUG_FIX_PLAYER
        LOG(llevDebug, "fix_player(): called from non Player/Mob object: %s [%x] (type %d) (%s)\n", 
                query_name(op), op->count, op->type, debug_msg);
#else
        LOG(llevDebug, "fix_player(): called from non Player/Mob object: %s [%x] (type %d)\n", 
                query_name(op), op->count, op->type);
#endif
        return;
    }

#ifdef DEBUG_FIX_PLAYER
    LOG(llevDebug, "FIX_PLAYER(%s [%x]): >> %s\n", query_name(op), op->count, debug_msg);
#endif

    pl = CONTR(op);
    inv_flag = inv_see_flag = weapon_weight = best_wc = best_ac = wc = ac = 0;

    op->stats.Str = pl->orig_stats.Str;
    op->stats.Dex = pl->orig_stats.Dex;
    op->stats.Con = pl->orig_stats.Con;
    op->stats.Int = pl->orig_stats.Int;
    op->stats.Wis = pl->orig_stats.Wis;
    op->stats.Pow = pl->orig_stats.Pow;
    op->stats.Cha = pl->orig_stats.Cha;

    pl->guild_force = pl->selected_weapon = pl->skill_weapon = NULL;
    pl->quest_one_drop = pl->quests_done = pl->quests_type_kill = pl->quests_type_normal = NULL;
    pl->digestion = 3;
    pl->gen_hp = 1;
    pl->gen_sp = 1;
    pl->gen_grace = 1;
    pl->gen_sp_armour = 0;
    pl->exp_bonus = 0;
    pl->encumbrance = 0;
    pl->set_skill_weapon = pl->set_skill_archery = NO_SKILL_READY;
    /* the default skill groups for non guild players */
    pl->base_skill_group[0]=SKILLGROUP_PHYSIQUE;
    pl->base_skill_group[1]=SKILLGROUP_AGILITY;
    pl->base_skill_group[2]=SKILLGROUP_WISDOM;
    pl->base_skill_group_exp[0]=pl->base_skill_group_exp[1]=pl->base_skill_group_exp[2]=100;


    /* for players, we adjust with the values */
    ac = op->arch->clone.stats.ac;
    wc = op->arch->clone.stats.wc;
    op->stats.wc = wc;
    op->stats.ac = ac;
    op->stats.dam = op->arch->clone.stats.dam;

    op->stats.maxhp = op->arch->clone.stats.maxhp;
    op->stats.maxsp = op->arch->clone.stats.maxsp;
    op->stats.maxgrace = op->arch->clone.stats.maxgrace;

    pl->levhp[1] = (char) op->stats.maxhp;
    pl->levsp[1] = (char) op->stats.maxsp;
    pl->levgrace[1] = (char) op->stats.maxgrace;

    old_glow = op->glow_radius;
    light = op->arch->clone.glow_radius;

    op->stats.thac0 = op->arch->clone.stats.thac0;
    op->stats.thacm = op->arch->clone.stats.thacm;

    op->speed = op->arch->clone.speed;
    op->weapon_speed = op->arch->clone.weapon_speed;
    op->path_attuned = op->arch->clone.path_attuned;
    op->path_repelled = op->arch->clone.path_repelled;
    op->path_denied = op->arch->clone.path_denied;
    op->terrain_flag = op->arch->clone.terrain_flag;        /* reset terrain moving abilities */

    /* only adjust skills which has no own level/exp values */
    if (op->chosen_skill && !op->chosen_skill->last_eat && op->chosen_skill->exp_obj)
        op->chosen_skill->level = op->chosen_skill->exp_obj->level;

    FREE_AND_CLEAR_HASH(op->slaying);

    /* HOTFIX: we parted refl_xxx from can_refl_xxx */
    CLEAR_FLAG(op, FLAG_REFL_MISSILE);
    CLEAR_FLAG(op, FLAG_REFL_SPELL);

    if (QUERY_FLAG(op, FLAG_IS_INVISIBLE))
        inv_flag = 1;
    if (QUERY_FLAG(op, FLAG_SEE_INVISIBLE))
        inv_see_flag = 1;
    if (!QUERY_FLAG(&op->arch->clone, FLAG_XRAYS))
        CLEAR_FLAG(op, FLAG_XRAYS);
    if (!QUERY_FLAG(&op->arch->clone, FLAG_CAN_PASS_THRU))
        CLEAR_MULTI_FLAG(op, FLAG_CAN_PASS_THRU);
    if (!QUERY_FLAG(&op->arch->clone, FLAG_IS_ETHEREAL))
        CLEAR_MULTI_FLAG(op, FLAG_IS_ETHEREAL);
    if (!QUERY_FLAG(&op->arch->clone, FLAG_IS_INVISIBLE))
        CLEAR_MULTI_FLAG(op, FLAG_IS_INVISIBLE);
    if (!QUERY_FLAG(&op->arch->clone, FLAG_SEE_INVISIBLE))
        CLEAR_FLAG(op, FLAG_SEE_INVISIBLE);
    if (!QUERY_FLAG(&op->arch->clone, FLAG_LIFESAVE))
        CLEAR_FLAG(op, FLAG_LIFESAVE);
    if (!QUERY_FLAG(&op->arch->clone, FLAG_STEALTH))
        CLEAR_FLAG(op, FLAG_STEALTH);
    if (!QUERY_FLAG(&op->arch->clone, FLAG_BLIND))
        CLEAR_FLAG(op, FLAG_BLIND);
    if (!QUERY_FLAG(&op->arch->clone, FLAG_SLOWED))
        CLEAR_FLAG(op, FLAG_SLOWED);
    if (!QUERY_FLAG(&op->arch->clone, FLAG_FEARED))
        CLEAR_FLAG(op, FLAG_FEARED);
    /* rooted is set when a snare effect has reached 100% */
    if (!QUERY_FLAG(&op->arch->clone, FLAG_ROOTED))
        CLEAR_FLAG(op, FLAG_ROOTED);
    if (!QUERY_FLAG(&op->arch->clone, FLAG_CONFUSED))
        CLEAR_FLAG(op, FLAG_CONFUSED);
    if (!QUERY_FLAG(&op->arch->clone, FLAG_PARALYZED))
        CLEAR_FLAG(op, FLAG_PARALYZED);
    if (!QUERY_FLAG(&op->arch->clone, FLAG_FLYING))
        CLEAR_MULTI_FLAG(op, FLAG_FLYING);
    if (!QUERY_FLAG(&op->arch->clone, FLAG_LEVITATE))
        CLEAR_MULTI_FLAG(op, FLAG_LEVITATE);
    if (!QUERY_FLAG(&op->arch->clone, FLAG_CAN_REFL_SPELL))
        CLEAR_FLAG(op, FLAG_CAN_REFL_SPELL);
    if (!QUERY_FLAG(&op->arch->clone, FLAG_CAN_REFL_MISSILE))
        CLEAR_FLAG(op, FLAG_CAN_REFL_MISSILE);
    if (!QUERY_FLAG(&op->arch->clone, FLAG_UNDEAD))
        CLEAR_FLAG(op, FLAG_UNDEAD);
    if (!QUERY_FLAG(&op->arch->clone, FLAG_SEE_IN_DARK))
        CLEAR_FLAG(op, FLAG_SEE_IN_DARK);

    memset(&potion_resist_boni, 0, sizeof(potion_resist_boni));
    memset(&potion_resist_mali, 0, sizeof(potion_resist_mali));
    memset(&potion_attack, 0, sizeof(potion_attack));

    /* initializing player arrays from the values in player archetype clone:  */
    memset(&pl->equipment, 0, sizeof(pl->equipment));
    memcpy(&op->attack, &op->arch->clone.attack, sizeof(op->attack));
    memcpy(&op->resist, &op->arch->clone.resist, sizeof(op->resist));
    for (i = 0; i < NROFATTACKS; i++)
    {
        if (op->resist[i] > 0)
        {
            resists_boni[i] = op->resist[i];
            resists_mali[i] = 0;
        }
        else
        {
            resists_mali[i] = -(op->resist[i]);
            resists_boni[i] = 0;
        }
    }
    for (i = 0; i < NROFSKILLGROUPS; i++)
        pl->highest_skill[i]=NULL;

    /* ok, now we browse the inventory... there is not only our equipment - there
     * are all our skills, forces and hidden system objects.
     */
    for (tmp = op->inv; tmp != NULL; tmp = tmp_ptr)
    {
        tmp_ptr = tmp->below;
        /*
         * add here more types we can and must skip.
         */
        if (tmp->type == SCROLL
             || tmp->type == EXPERIENCE
             || tmp->type == POTION
             || tmp->type == CONTAINER
             || tmp->type == CLOSE_CON
             || tmp->type == TYPE_LIGHT_REFILL
             || tmp->type == WAND
             || tmp->type == MONSTER)
            continue;
        else if(tmp->type == TYPE_GUILD_FORCE)
        {
            pl->guild_force = tmp;
            pl->base_skill_group[0]=tmp->last_eat;
            pl->base_skill_group[1]=tmp->last_sp;
            pl->base_skill_group[2]=tmp->last_heal;

            pl->base_skill_group_exp[0]=tmp->last_grace;
            pl->base_skill_group_exp[1]=tmp->magic;
            pl->base_skill_group_exp[2]=tmp->state;
        }
        else if(tmp->type == TYPE_QUEST_CONTAINER)
        {
            /* one drop container */
            /* TODO: this should be replaced with a switch statement */
            if(tmp->sub_type1 == ST1_QUEST_ONE_DROP)
            {
                pl->quest_one_drop = tmp;
                pl->quest_one_drop_count = tmp->count;
            }
            else if(tmp->sub_type1 == ST1_QUESTS_TYPE_DONE)
            {
                pl->quests_done = tmp;
                pl->quests_done_count = tmp->count;
            }
            else if(tmp->sub_type1 == ST1_QUESTS_TYPE_NORMAL)
            {
                pl->quests_type_normal = tmp;
                pl->quests_type_normal_count = tmp->count;
            }
            else if(tmp->sub_type1 == ST1_QUESTS_TYPE_KILL)
            {
                pl->quests_type_kill = tmp;
                pl->quests_type_kill_count = tmp->count;
            }
            else /* this really should not happens... */
            {
                LOG(llevBug,"BUG: fix_player(): found illegal quest container (st: %d) in player %s\n",
                    tmp->sub_type1, query_name(op));
                remove_ob(tmp);
            }
            continue;
        }
        /* all skills, not only the applied ones */
       else  if (tmp->type == SKILL)
        {
            /* create list of highest skills */
            if(!pl->highest_skill[tmp->magic] || tmp->stats.exp > pl->highest_skill[tmp->magic]->stats.exp)
                pl->highest_skill[tmp->magic] = tmp;

            /* TODO: no "crafting/trade" skills here */
            /* get highest single skill - thats our "main" skill */
            if (tmp->level > skill_level_max)
                skill_level_max = tmp->level;
            /* lets remember the best bare hand skill */
            if (tmp->stats.dam > 0)
            {
                if (!skill_weapon || skill_weapon->stats.dam < tmp->stats.dam)
                    skill_weapon = tmp;
            }
        }

       /* this is needed, because our applied light can be overruled by a light giving
        * object like holy glowing aura force or something
        */
       if (tmp->glow_radius > light)
       {
           /* don't use this item when it is a 'not applied TYPE_LIGHT_APPLY' */
           if (tmp->type != TYPE_LIGHT_APPLY || QUERY_FLAG(tmp, FLAG_APPLIED))
               light = tmp->glow_radius;
       }

        /* this checks all applied items in the inventory */
        if (QUERY_FLAG(tmp, FLAG_APPLIED))
        {
            switch (tmp->type) /* still applied stuff */
            {
                case ROD:
                case HORN:
                    pl->equipment[PLAYER_EQUIP_MTOOL] = tmp;
                    break;

                case TYPE_LIGHT_APPLY:
                  if (tmp->glow_radius > light)
                      light = tmp->glow_radius;
                  break;

                  /* the new weapon skill system is more complex
                     * when new applied, set_skill_weapon is set.
                     * But we took care to safely set it here.
                     */
                case WEAPON:
                  pl->equipment[PLAYER_EQUIP_WEAPON1] = tmp;
                  pl->selected_weapon = tmp; /* thats our weapon */
                  i = tmp->sub_type1 % 4;
                  if (i == WEAP_1H_IMPACT)
                      pl->set_skill_weapon = SK_MELEE_WEAPON;
                  else if (i == WEAP_1H_SLASH)
                      pl->set_skill_weapon = SK_SLASH_WEAP;
                  else if (i == WEAP_1H_CLEAVE)
                      pl->set_skill_weapon = SK_CLEAVE_WEAP;
                  else
                      pl->set_skill_weapon = SK_PIERCE_WEAP;

                  /* the new weapon_speed system has nothing to do with the old one,
                             * so take care weapon speed is always our wielded weapon -
                             * after we have all added ,we adjust it later for dex bonus, etc (todo)
                                  */
                  op->weapon_speed = tmp->weapon_speed;
                  if (!op->weapon_speed)
                      LOG(llevBug, "BUG: monster/player %s applied weapon %s without weapon speed!\n", op->name,
                          tmp->name);
                  wc += (tmp->stats.wc + tmp->magic);
                  if (tmp->stats.ac && tmp->stats.ac + tmp->magic > 0)
                      ac += tmp->stats.ac + tmp->magic;
                  op->stats.dam += (tmp->stats.dam + tmp->magic);
                  weapon_weight = tmp->weight;
                  if (tmp->slaying != NULL)
                      FREE_AND_COPY_HASH(op->slaying, tmp->slaying);

                  pl->encumbrance += (sint16) (3 * tmp->weight / 1000);
                  pl->digestion += tmp->stats.food;
                  pl->gen_sp += tmp->stats.sp;
                  pl->gen_grace += tmp->stats.grace;
                  pl->gen_hp += tmp->stats.hp;
                  pl->gen_sp_armour += tmp->last_heal;
                  thac0 += tmp->stats.thac0;
                  thacm += tmp->stats.thacm;

                  for (i = 0; i < 7; i++)
                      change_attr_value(&(op->stats), i, get_attr_value(&(tmp->stats), i));
                  break;

                  /* all armours + rings and amulets */
                case RING:
                  pl->equipment[PLAYER_EQUIP_RRING + ring_count] = tmp;
                  ring_count++;
                  goto fix_player_no_armour;
                case AMULET:
                  pl->equipment[PLAYER_EQUIP_AMULET] = tmp;
                  if(tmp->last_grace)
                      pl->exp_bonus += tmp->last_grace;
                  goto fix_player_no_armour;

                case BRACERS:
                  pl->equipment[PLAYER_EQUIP_BRACER] = tmp;
                  goto fix_player_jump1;
                case ARMOUR:
                  pl->equipment[PLAYER_EQUIP_MAIL] = tmp;
                  pl->encumbrance += (int) tmp->weight / 1000;
                  goto fix_player_jump1;
                case SHIELD:
                  pl->equipment[PLAYER_EQUIP_SHIELD] = tmp;
                  pl->encumbrance += (int) tmp->weight / 2000;
                  goto fix_player_jump1;
                case GIRDLE:
                  pl->equipment[PLAYER_EQUIP_GIRDLE] = tmp;
                  goto fix_player_jump1;
                case HELMET:
                  pl->equipment[PLAYER_EQUIP_HELM] = tmp;
                  goto fix_player_jump1;
                case SHOULDER:
                    pl->equipment[PLAYER_EQUIP_SHOULDER] = tmp;
                    goto fix_player_jump1;
                case LEGS:
                    pl->equipment[PLAYER_EQUIP_LEGS] = tmp;
                    goto fix_player_jump1;
                case BOOTS:
                  pl->equipment[PLAYER_EQUIP_BOOTS] = tmp;
                  goto fix_player_jump1;
                case GLOVES:
                  pl->equipment[PLAYER_EQUIP_GAUNTLET] = tmp;
                  goto fix_player_jump1;
                case CLOAK:
                  pl->equipment[PLAYER_EQUIP_CLOAK] = tmp;

                  fix_player_jump1:
                  /* thats now used for ALL armours except rings and amulets */
                  if (ARMOUR_SPEED(tmp) && (float) ARMOUR_SPEED(tmp) / 10.0f < max)
                      max = ARMOUR_SPEED(tmp) / 10.0f;

                  fix_player_no_armour: /* jump in for non armour like rings, ... */
                  /* i must control this - at last food has for used_up forces
                             * a differnet meaning
                             * add some of this below when used from other applied objects too!
                             */
                  max_boni_hp += tmp->stats.maxhp;
                  max_boni_sp += tmp->stats.maxsp;
                  max_boni_grace += tmp->stats.maxgrace;
                  pl->digestion += tmp->stats.food;
                  pl->gen_sp += tmp->stats.sp;
                  pl->gen_grace += tmp->stats.grace;
                  pl->gen_hp += tmp->stats.hp;
                  pl->gen_sp_armour += tmp->last_heal;
                  thac0 += tmp->stats.thac0;
                  thacm += tmp->stats.thacm;

                  for (i = 0; i < 7; i++)
                      change_attr_value(&(op->stats), i, get_attr_value(&(tmp->stats), i));

                  if (tmp->stats.wc)
                      wc += (tmp->stats.wc + tmp->magic);
                  if (tmp->stats.dam)
                      op->stats.dam += (tmp->stats.dam + tmp->magic);
                  if (tmp->stats.ac)
                      ac += (tmp->stats.ac + tmp->magic);
                  break;

                case BOW:
                  pl->equipment[PLAYER_EQUIP_BOW] = tmp;
                  /* as a special bonus range weapons can be permanent applied and
                             * will add stat boni!
                             */
                  for (i = 0; i < 7; i++)
                      change_attr_value(&(op->stats), i, get_attr_value(&(tmp->stats), i));

                  if (tmp->sub_type1 == RANGE_WEAP_BOW)
                      pl->set_skill_archery = SK_MISSILE_WEAPON;
                  else if (tmp->sub_type1 == RANGE_WEAP_XBOWS)
                      pl->set_skill_archery = SK_XBOW_WEAP;
                  else
                      pl->set_skill_archery = SK_SLING_WEAP;
                  break;

                  /* more exotic stuff! */

                case POTION_EFFECT:
                  /* no protection from potion effect -resist only! */
                  for (i = 0; i < 7; i++)
                      change_attr_value(&(op->stats), i, get_attr_value(&(tmp->stats), i));
                  /* collect highest boni & malus - only highest one count,
                             * no adding potion effects of same resist!
                             */
                  thac0 += tmp->stats.thac0;
                  thacm += tmp->stats.thacm;
                  for (i = 0; i < NROFATTACKS; i++)
                  {
                      /* collect highest/lowest resistance */
                      if (tmp->resist[i] > potion_resist_boni[i])
                          potion_resist_boni[i] = tmp->resist[i];
                      else if (tmp->resist[i] < potion_resist_mali[i])
                          potion_resist_mali[i] = tmp->resist[i];

                      /* collect highest attack!
                                     * Remember: no attack mali - this is a unsigned value
                                     */
                      if (tmp->attack[i] > potion_attack[i])
                          potion_attack[i] = tmp->attack[i];
                  }
                  break;

                case SKILL:
                  /* skills modifying the character -b.t. */
                  thac0 += tmp->stats.thac0;
                  thacm += tmp->stats.thacm;
                  if (tmp->stats.dam > 0)   /* skill is a 'weapon' */
                  {
                      wc += tmp->stats.wc;
                      ac += tmp->stats.ac;
                      op->weapon_speed = tmp->weapon_speed ;
                      weapon_weight = tmp->weight;
                      op->stats.dam += tmp->stats.dam;
                  }
                  else
                  {
                      op->stats.dam += tmp->stats.dam;
                      wc += tmp->stats.wc;
                      ac += tmp->stats.ac;
                  }
                  if (tmp->slaying != NULL)
                      FREE_AND_COPY_HASH(op->slaying, tmp->slaying);

                  pl->encumbrance += (int) 3 * tmp->weight / 1000;

                  break;

                  /* now calc resistance and stuff for all the rest applyable objects! */
                  /* i am not 100% sure this is safe for *all* objects - i have used for that
                         * reason not default here.
                         */
                case TYPE_AGE_FORCE:
                  pl->age_force = tmp; /* store our age force */
                  pl->age = tmp->stats.hp;
                  pl->age_max = tmp->stats.maxhp;
                  pl->age_add = tmp->stats.sp;
                  pl->age_changes = tmp->stats.grace;
                  if (pl->age >= (sint16) (((float) pl->age_max / 100.0f) * 60.0f))
                      SET_FLAG(op, FLAG_IS_AGED);
                  else
                      CLEAR_FLAG(op, FLAG_IS_AGED);

                case FORCE:
                    if(tmp->sub_type1 == ST1_FORCE_SNARE)
                    {
                        if(tmp->last_heal >= 100)
                            SET_FLAG(op, FLAG_ROOTED);
                        snare_penalty += tmp->last_heal;
                    }
                    else if(tmp->sub_type1 == ST1_FORCE_PARALYZE)
                        SET_FLAG(op, FLAG_PARALYZED);
                    else if(tmp->sub_type1 == ST1_FORCE_CONFUSED)
                        SET_FLAG(op, FLAG_CONFUSED);
                    else if(tmp->sub_type1 == ST1_FORCE_BLIND)
                        SET_FLAG(op, FLAG_BLIND);
                    else if(tmp->sub_type1 == ST1_FORCE_FEAR)
                        SET_FLAG(op, FLAG_FEARED);
                    else if(tmp->sub_type1 == ST1_FORCE_SLOWED) /* slowness */
                    {
                        slow_penalty += tmp->last_heal; 
                        SET_FLAG(op, FLAG_SLOWED);
                    }
                    else if(tmp->sub_type1 == ST1_FORCE_DRAIN) /* level drain */
                    {
                            skill_level_drain += tmp->level; 
                    }
                    else
                    {
                        if (ARMOUR_SPEED(tmp) && (float) ARMOUR_SPEED(tmp) / 10.0f < max)
                            max = ARMOUR_SPEED(tmp) / 10.0f;

                        for (i = 0; i < 7; i++)
                            change_attr_value(&(op->stats), i, get_attr_value(&(tmp->stats), i));
                        if (tmp->stats.wc)
                            wc += (tmp->stats.wc + tmp->magic);
                        if (tmp->stats.dam)
                            op->stats.dam += (tmp->stats.dam + tmp->magic);
                        if (tmp->stats.ac)
                            ac += (tmp->stats.ac + tmp->magic);
                        if (tmp->stats.maxhp && tmp->type != TYPE_AGE_FORCE)
                            op->stats.maxhp += tmp->stats.maxhp;
                        if (tmp->stats.maxsp && tmp->type != TYPE_AGE_FORCE)
                            op->stats.maxsp += tmp->stats.maxsp;
                        if (tmp->stats.maxgrace && tmp->type != TYPE_AGE_FORCE)
                            op->stats.maxgrace += tmp->stats.maxgrace;
                    }
                    goto fix_player_jump_resi;

                case DISEASE:
                case SYMPTOM:
                  speed_reduce_from_disease = (float) tmp->last_sp / 100.0f;
                  if (speed_reduce_from_disease == 0.0f)
                      speed_reduce_from_disease = 1.0f;

                case POISONING:
                  for (i = 0; i < 7; i++)
                      change_attr_value(&(op->stats), i, get_attr_value(&(tmp->stats), i));

                fix_player_jump_resi:

                  thac0 += tmp->stats.thac0;
                  thacm += tmp->stats.thacm;
                  /* calculate resistance and attacks */
                  for (i = 0; i < NROFATTACKS; i++)
                  {
                      /* we add resists boni/mali */
                      if (tmp->resist[i] > 0)
                      {
                          if(tmp->resist[i]>=100)
                              resists_boni[i] = 100;
                          else if(resists_boni[i]<100)
                              resists_boni[i] += ((100 - resists_boni[i]) * tmp->resist[i]) / 100;
                      }
                      else if (tmp->resist[i] < 0)
                          resists_mali[i] += ((100 - resists_mali[i]) * (-tmp->resist[i])) / 100;

                      /* and we use adding attack boni - i set this for 120% max...
                                     * exclude all what is damaging itself with attack[]
                                    */
                      if (tmp->type != DISEASE && tmp->type != SYMPTOM && tmp->type != POISONING)
                      {
                          if (tmp->attack[i] > 0)
                          {
                              if ((op->attack[i] + tmp->attack[i]) <= 120)
                                  op->attack[i] += tmp->attack[i];
                              else
                                  op->attack[i] = 120;
                          }
                      }
                  }
                  break;

                  /* catch item which are "applied" but should not -
                         * or we forgot to catch them here!
                         */
                default:
                  LOG(llevDebug, "DEBUG: fix_player(): unexpected applied object %s (%d)(clear flag now!)\n",
                      query_name(tmp), tmp->type);
                  CLEAR_FLAG(tmp, FLAG_APPLIED);
                  continue;
                  break;
            }/*switch*/

            op->terrain_flag |= tmp->terrain_type;    /* we just add a given terrain */
            op->path_attuned |= tmp->path_attuned;
            op->path_repelled |= tmp->path_repelled;
            op->path_denied |= tmp->path_denied;
            if (QUERY_FLAG(tmp, FLAG_LIFESAVE))
                SET_FLAG(op, FLAG_LIFESAVE);
            if (QUERY_FLAG(tmp, FLAG_REFL_SPELL))
                SET_FLAG(op, FLAG_CAN_REFL_SPELL);
            if (QUERY_FLAG(tmp, FLAG_REFL_MISSILE))
                SET_FLAG(op, FLAG_CAN_REFL_MISSILE);
            if (QUERY_FLAG(tmp, FLAG_STEALTH))
                SET_FLAG(op, FLAG_STEALTH);
            if (QUERY_FLAG(tmp, FLAG_UNDEAD) && !QUERY_FLAG(&op->arch->clone, FLAG_UNDEAD))
                SET_FLAG(op, FLAG_UNDEAD);
            if (QUERY_FLAG(tmp, FLAG_XRAYS))
                SET_FLAG(op, FLAG_XRAYS);
            if (QUERY_FLAG(tmp, FLAG_BLIND))
                SET_FLAG(op, FLAG_BLIND);
            if (QUERY_FLAG(tmp, FLAG_SEE_IN_DARK))
                SET_FLAG(op, FLAG_SEE_IN_DARK);

            if (QUERY_FLAG(tmp, FLAG_SEE_INVISIBLE))
                SET_FLAG(op, FLAG_SEE_INVISIBLE);
            if (QUERY_FLAG(tmp, FLAG_MAKE_INVISIBLE))
                SET_MULTI_FLAG(op, FLAG_IS_INVISIBLE);
            if (QUERY_FLAG(tmp, FLAG_CAN_PASS_THRU))
                SET_MULTI_FLAG(op, FLAG_CAN_PASS_THRU);
            if (QUERY_FLAG(tmp, FLAG_MAKE_ETHEREAL))
                SET_MULTI_FLAG(op, FLAG_IS_ETHEREAL);
            if (QUERY_FLAG(tmp, FLAG_LEVITATE))
                SET_MULTI_FLAG(op, FLAG_LEVITATE);

            if (QUERY_FLAG(tmp, FLAG_FLYING))
            {
                SET_MULTI_FLAG(op, FLAG_FLYING);
                if (!QUERY_FLAG(op, FLAG_WIZ))
                    max = 1;
            }


            op->stats.thac0-=thac0;
            op->stats.thacm+=thacm;

            /* slow penalty .. careful with this! might be changing */
            if (tmp->stats.exp && tmp->type != EXPERIENCE && tmp->type != SKILL)
            {
                if (tmp->stats.exp > 0)
                {
                    added_speed += (float) tmp->stats.exp / 3.0f;
                    bonus_speed += 1.0f + (float) tmp->stats.exp / 3.0f;
                }
                else
                    added_speed += (float) tmp->stats.exp;
            }
        } /* if applied */
    } /* Item is equipped - end of for loop going through items. */

    /* now collect all our real armour stuff - this will now add *after* all force
     * or non potion effects effecting resist,attack or protection - also this wii
     * give us a sorted adding.
     * But the most important point is that we calc *here* and only here the equipment
     * quality modifier for players.
     * Note, that for bows/arrows the calculation is done "one the fly" when the
     * throw/fire object is created!
     */
    for (j = 0; j < PLAYER_EQUIP_MAX; j++)
    {
        if (pl->equipment[j])
        {
            tmp = pl->equipment[j];
            tmp_con = (float) tmp->item_condition / 100.0f;

            /* calculate resistance and attacks */
            for (i = 0; i < NROFATTACKS; i++)
            {
                /* we add resists boni/mali */
                if (tmp->resist[i] > 0 && resists_boni[i]<100)
                {
                    tmp_item = (int) ((float) tmp->resist[i] * tmp_con);
                    if(tmp_item>=100)
                        resists_boni[i]=100;
                    else
                        resists_boni[i] += ((100 - resists_boni[i]) * tmp_item) / 100;
                }
                else if (tmp->resist[i] < 0)
                    resists_mali[i] += ((100 - resists_mali[i]) * (-tmp->resist[i])) / 100;

                /* and we use adding attack boni - i set this for 120% max...
                         * exclude all what is damaging itself with attack[]
                        */
                if (tmp->type != BOW)
                {
                    if (tmp->attack[i] > 0)
                    {
                        tmp_item = tmp->attack[i];
                        if(i >= ATNR_GODPOWER) /* Important: Don't add condition to damage attack - we did it to dmg! */
                            tmp_item = (int) ((float)tmp_item * tmp_con);
                        if ((op->attack[i] + tmp_item) <= 120)
                            op->attack[i] += tmp_item;
                        else
                            op->attack[i] = 120;
                    }
                }
            }
        }
    }

    /* 'total resistance = total protections - total vulnerabilities'.
     * If there is an uncursed potion in effect, granting more protection
     * than that, we take: 'total resistance = resistance from potion'.
     * If there is a cursed (and no uncursed) potion in effect, we take
     * 'total resistance = vulnerability from cursed potion'.
     */

    /* now we add in all our values... we add in our potions effects as well as
      * our attack boni and/or protections.
      */
    for (j = 1,i = 0; i < NROFATTACKS; i++,j <<= 1)
    {
        if (potion_attack[i])
        {
            if ((op->attack[i] + potion_attack[i]) > 120)
                op->attack[i] = 120;
            else
                op->attack[i] += potion_attack[i];
        }

        /* add in the potion resists boni/mali */
        if (potion_resist_boni[i] > 0 && resists_boni[i] <100)
        {
            if(potion_resist_boni[i]>=100)
                resists_boni[i] = 100;
            else
                resists_boni[i] += ((100 - resists_boni[i]) * potion_resist_boni[i]) / 100;
        }

        if (potion_resist_mali[i] < 0)
            resists_mali[i] += ((100 - resists_mali[i]) * (-potion_resist_mali[i])) / 100;

        /* and generate the REAL resists of the player! */
        op->resist[i] = resists_boni[i] - resists_mali[i];
    }

    check_stat_bounds(&(op->stats));

    /* now the speed thing... */
    op->speed += speed_bonus[op->stats.Dex];

    if (added_speed >= 0)
        op->speed += added_speed / 10.0f;
    else /* Something wrong here...: */
        op->speed /= 1.0f - added_speed;
    if (op->speed > max)
        op->speed = max;

    /* calculate speed */

    /* we do now this: we have a weight_limit x. until we don't carry more
     * as Y% of x, we are not encumbered. The rest of 100% - Y% (lets say 100% - 66% = 34%)
     * is now our value from 1% encumbrance to 99% encumb. - this encum% will be removed from
     * our current speed. The advantage is we use only ONE weight value and we can direct
     * calculate from weight limit to speed limit.
     */

    /* first, calculate real speed */
    op->speed += bonus_speed / 10.0f;
    /* Put a lower limit on speed.  Note with this speed, you move once every
     * 100 ticks or so.  This amounts to once every 12 seconds of realtime.
     */
    op->speed = op->speed * speed_reduce_from_disease;

    /* don't reduce under this value */
    if (op->speed >= 0.15f)
    {
        f = ((float) weight_limit[op->stats.Str] / 100.0f) * ENCUMBRANCE_LIMIT; /* = max kg we can carry */
        if (((sint32) f) <= op->carrying)
        {
            if (op->carrying >= (sint32) weight_limit[op->stats.Str])
                op->speed = 0.01f; /* ouch */
            else
            {
                f = ((float) weight_limit[op->stats.Str] - f); /* total encumbrance weight part */
                f = ((float) weight_limit[op->stats.Str] - op->carrying) / f; /* value from 0.0 to 1.0 encumbrance */

                if (f < 0.0f)
                    f = 0.0f;
                else if (f > 1.0f)
                    f = 1.0f;

                op->speed *= f;
            }
        }
    }

    /* lets have a fair min. speed.
     * When we have added smooth scrolling, the whole
     * handling will change so or so. MT-06.2005
     */
    if (op->speed < 0.15f)
        op->speed = 0.15f; /* ouch */
    else if (op->speed > 1.0f)
        op->speed = 1.0f;
    update_ob_speed(op);

    op->glow_radius = light;

    /* we must do -old_gow + light */
    if (op->map && old_glow != light)
        adjust_light_source(op->map, op->x, op->y, light - old_glow);

    /* for player, max hp depend on general level, sp on magic exp, grace on wisdom exp level
     * NOTE: all values are adjusted from clone at function start.
     */
    skill_level_max = skill_level_max-skill_level_drain;
    if(skill_level_max <1)
        skill_level_max=1;
    op->level = skill_level_max;
    op->stats.maxhp += op->arch->clone.stats.maxhp + op->arch->clone.stats.maxhp; /* *3 is base */

    for (i = 1; i <= op->level; i++)
        op->stats.maxhp += pl->levhp[i];

    skill_level_drain = pl->exp_obj_ptr[SKILLGROUP_MAGIC]->level;
    if(skill_level_drain > skill_level_max)
        skill_level_drain = skill_level_max;
    for (i = 1; i <= skill_level_drain; i++)
        op->stats.maxsp += pl->levsp[i];

    skill_level_drain = pl->exp_obj_ptr[SKILLGROUP_WISDOM]->level;
    if(skill_level_drain > skill_level_max)
        skill_level_drain = skill_level_max;
    for (i = 1; i <= skill_level_drain; i++)
        op->stats.maxgrace += pl->levgrace[i];

    /* now adjust with the % of the stats mali/boni.
     */
    op->stats.maxhp += (int) ((float) op->stats.maxhp * con_bonus[op->stats.Con]) + max_boni_hp;
    op->stats.maxsp += (int) ((float) op->stats.maxsp * pow_bonus[op->stats.Pow]) + max_boni_sp;
    op->stats.maxgrace += (int) ((float) op->stats.maxgrace * wis_bonus[op->stats.Wis]) + max_boni_grace;

    if (op->stats.maxhp < 1)
        op->stats.maxhp = 1;
    if (op->stats.maxsp < 1)
        op->stats.maxsp = 1;
    if (op->stats.maxgrace < 1)
        op->stats.maxgrace = 1;
    /* when this is set, this object comes fresh in game.
     * we must adjust now hp,sp and grace with the max values.
     * if hp/sp/grace == -1, then set it to max value.
     * if it != 0, then leave it.
     * in this form, we can put "hurt or wounded" objects to map.
     */


    if (op->stats.hp == -1)
        op->stats.hp = op->stats.maxhp;
    if (op->stats.sp == -1)
        op->stats.sp = op->stats.maxsp;
    if (op->stats.grace == -1)
        op->stats.grace = op->stats.maxgrace;

    /* cap the pools to <=max */
    if (op->stats.hp > op->stats.maxhp)
        op->stats.hp = op->stats.maxhp;
    if (op->stats.sp > op->stats.maxsp)
        op->stats.sp = op->stats.maxsp;
    if (op->stats.grace > op->stats.maxgrace)
        op->stats.grace = op->stats.maxgrace;


    /* highest skill will be the "real" power level for ac! */
    /* i let this in BUT: in current system exp_obj level is always
     * highest level in group and player level is highest exp_obj.
     * means, op->level is always == skill_level_max!
     */
    op->stats.ac = ac + skill_level_max;

    /* Now we have all collected - the best bare hand skill and/or the applied weapon.
      * If we have no weapon selected, use the bare hand skill for attacking and wc/dam .
      * if we have a weapon, use the weapon and forget the bare hand skill.
      */

    if (pl->set_skill_weapon == NO_SKILL_READY) /* ok, no weapon in our hand - we must use our hands */
    {
        f = 1.0f;
        if (skill_weapon)
        {
            /* now we must add this special skill attack */
            for (i = 0; i < NROFATTACKS; i++)
            {
                if (op->attack[i] + skill_weapon->attack[i] > 120)
                    op->attack[i] = 120;
                else
                    op->attack[i] += skill_weapon->attack[i];
            }

            pl->skill_weapon = skill_weapon;
            op->stats.wc = wc + skill_weapon->level;
            op->stats.dam = (sint16) ((float) op->stats.dam * lev_damage[skill_weapon->level]);
        }
        else
            LOG(llevBug, "BUG: fix_player(): player %s has no hth skill!\n", op->name);
    }
    else /* weapon in hand */
    {
        f = (float) (pl->equipment[PLAYER_EQUIP_WEAPON1]->item_condition) / 100.0f;
        /* ouch - weapon without the skill applied... */
        if (!pl->skill_ptr[pl->set_skill_weapon])
            LOG(llevBug, "BUG: fix_player(): player %s has weapon selected but not the skill #%d!!!\n", op->name,
                pl->set_skill_weapon);
        else
        {
            op->stats.wc = wc + pl->skill_ptr[pl->set_skill_weapon]->level;
            op->stats.dam = (sint16) ((float) op->stats.dam * lev_damage[pl->skill_ptr[pl->set_skill_weapon]->level]);
        }
    }

    /* now the last adds - stat boni to dam and wc! */
    op->stats.dam = (int)((float)(op->stats.dam+dam_bonus[op->stats.Str])/10.0f);
    if (op->stats.dam < 0)
        op->stats.dam = 0;

//    LOG(-1,"ADJUST: real dmg: %d - cond: %f - new: %d\n", op->stats.dam, f, (sint16)((float) op->stats.dam * f));
    op->stats.dam = (sint16)((float) op->stats.dam * f); /* and finally the item condition! */

    op->stats.wc += thaco_bonus[op->stats.Dex];

    /* adjust swing speed and move speed by slow penalty */
    if(QUERY_FLAG(op,FLAG_FEARED))
    {
        int m;

        m = op->stats.wc/8; /* 15% mali wc */
        if(!m)
            m=1;
        op->stats.wc -= m;

        m = op->stats.ac/8; /* 15% mali ac */
        if(!m)
            m=1;
        op->stats.ac -= m;

        slow_penalty +=15; /* add a 15% slowness factor to swing & movement */
    }

    /* first, we cap slow effects to 80% */
    if(slow_penalty > 80)
        slow_penalty = 80;

    if(slow_penalty)
    {
        snare_penalty += slow_penalty;
        op->weapon_speed *= ((float)(100+slow_penalty)/100.0f);
    }

    /* we don't cap snare effects - if >=100% we set root flag */
    if(snare_penalty)
    {
        if(snare_penalty >= 100)
        {
            SET_FLAG(op,FLAG_ROOTED);
            op->weapon_speed = 0.15f;
        }
        else
        {
            op->speed *= ((float)(100-snare_penalty)/100.0f);
        }
    }

    /* this is right, root & paralyze flags will handle no movement
     * for all other movement penalties, this is the cap
     */
    if(op->weapon_speed < 0.15f)
        op->weapon_speed = 0.15f;

    /* thats for the client ... */
    if(QUERY_FLAG(op,FLAG_PARALYZED))
    {
        pl->weapon_sp = 0;
        pl->speed = 0;
    }
    else
    {
        if(QUERY_FLAG(op,FLAG_ROOTED))
            pl->speed = 0;
        else
        {
            pl->weapon_sp = (int) (op->weapon_speed * 1000.0f);
            pl->speed = op->speed;
        }
    }

    /* Regenerate HP */
    base_reg = 38; /* default value */
    if (CONTR(op)->gen_hp > 0)
    {
        CONTR(op)->reg_hp_num = (CONTR(op)->gen_hp / 2) + 1; /* how much we gain if we gain */
        base_reg = (base_reg - CONTR(op)->gen_hp) / CONTR(op)->gen_hp;
    }
    else
    {
        CONTR(op)->reg_hp_num = 1;
        base_reg += (CONTR(op)->gen_hp * CONTR(op)->gen_hp) + 5;
    }
    if (base_reg < 0)
        base_reg = 0;
    if (op->last_heal > base_reg) /* that can happens when we have changed equipment! */
        op->last_heal = base_reg;

    CONTR(op)->base_hp_reg = base_reg; /* thats the real hp reg count in ticks */

    reg_sec = (pticks_second / (float) base_reg) * (float) CONTR(op)->reg_hp_num;
    if (reg_sec > 100)
        reg_sec = 99.9f;
    else if (reg_sec && reg_sec < 0.1)
        reg_sec = 0.1f;
    CONTR(op)->gen_client_hp = (uint16) (reg_sec * 10.0f); /* the value for the client */

    /* Regenerate Mana */
    base_reg = 35; /* default value */
    if (CONTR(op)->gen_sp > 0)
    {
        CONTR(op)->reg_sp_num = (CONTR(op)->gen_sp / 3) + 1;
        base_reg = ((base_reg + CONTR(op)->gen_sp_armour) - CONTR(op)->gen_sp) / CONTR(op)->gen_sp;
    }
    else
    {
        CONTR(op)->reg_sp_num = 1;
        base_reg += (CONTR(op)->gen_sp * CONTR(op)->gen_sp) + 5 + CONTR(op)->gen_sp_armour;
    }
    if (base_reg < 0)
        base_reg = 0;
    if (op->last_sp > base_reg) /* that can happens when we have changed equipment! */
        op->last_sp = base_reg;

    CONTR(op)->base_sp_reg = base_reg; /* thats the real sp reg count in ticks */

    reg_sec = (pticks_second / (float) base_reg) * (float) CONTR(op)->reg_sp_num;
    if (reg_sec > 100)
        reg_sec = 99.9f;
    else if (reg_sec && reg_sec < 0.1)
        reg_sec = 0.1f;
    CONTR(op)->gen_client_sp = (uint16) (reg_sec * 10.0f);

    /* Regenerate Grace */
    base_reg = 25; /* default value */
    if (CONTR(op)->gen_grace > 0)
    {
        CONTR(op)->reg_grace_num = (CONTR(op)->gen_grace / 3) + 1;
        base_reg = (base_reg - CONTR(op)->gen_grace) / CONTR(op)->gen_grace;
    }
    else
    {
        CONTR(op)->reg_grace_num = 1;
        base_reg += (CONTR(op)->gen_grace * CONTR(op)->gen_grace) + 5;
    }

    if (base_reg < 0)
        base_reg = 0;
    if (op->last_grace > base_reg) /* that can happens when we have changed equipment! */
        op->last_grace = base_reg;

    CONTR(op)->base_grace_reg = base_reg; /* thats the real sp reg count in ticks */

    reg_sec = (pticks_second / (float) base_reg) * (float) CONTR(op)->reg_grace_num;
    if (reg_sec > 100)
        reg_sec = 99.9f;
    else if (reg_sec && reg_sec < 0.1)
        reg_sec = 0.1f;
    CONTR(op)->gen_client_grace = (uint16) (reg_sec * 10.0f);

    /*LOG(-1,"PLAYER ADJUST: %s -> hp(%d %d %d %d) sp(%d %d %d %d) gr(%d %d %d %d)\n",op->name,
        op->stats.hp,op->stats.maxhp,op->arch->clone.stats.maxhp,op->stats.maxhp_adj,
        op->stats.sp,op->stats.maxsp,op->arch->clone.stats.maxsp,op->stats.maxsp_adj,
        op->stats.grace,op->stats.maxgrace,op->arch->clone.stats.maxgrace,op->stats.maxgrace_adj);
    */
    if (QUERY_FLAG(op, FLAG_IS_INVISIBLE))
    {
        if (!inv_flag)
            update_object(op, UP_OBJ_LAYER); /* we must reinsert us in the invisible chain */
    }
    else if (inv_flag) /* and !FLAG_IS_INVISIBLE */
        update_object(op, UP_OBJ_LAYER);

    if (QUERY_FLAG(op, FLAG_SEE_INVISIBLE))
    {
        if (!inv_see_flag)
            pl->socket.update_tile = 0;
    }
    else if (inv_see_flag) /* and !FLAG_SEE_INVISIBLE */
        pl->socket.update_tile = 0;
}

/*
 * set the new dragon name after gaining levels or
 * changing ability focus (later this can be extended to
 * eventually change the player's face and animation)
 *
 * Note that the title is written to 'own_title' in the
 * player struct. This should be changed to 'ext_title'
 * as soon as clients support this!
 * Please, anyone, write support for 'ext_title'.
 */
void set_dragon_name(object *pl, object *abil, object *skin)
{
    int atnr    = -1;  /* attacknumber of highest level */
    int level   = 0;  /* highest level */
    int i;

    /* first, look for the highest level */
    for (i = 0; i < NROFATTACKS; i++)
    {
        if (atnr_is_dragon_enabled(i) && (atnr == -1 || abil->resist[i] > abil->resist[atnr]))
        {
            level = abil->resist[i];
            atnr = i;
        }
    }

    /* now if there are equals at highest level, pick the one with focus,
       or else at random */
    if (atnr_is_dragon_enabled(abil->stats.exp) && abil->resist[abil->stats.exp] >= level)
        atnr = abil->stats.exp;

    level = (int) (level / 5.);

    /* now set the new title */
    /*
    if (CONTR(pl) != NULL) {
      if(level == 0)
        sprintf(CONTR(pl)->title, "%s hatchling", attacks[atnr]);
      else if (level == 1)
        sprintf(CONTR(pl)->title, "%s wyrm", attacks[atnr]);
       else if (level == 2)
        sprintf(CONTR(pl)->title, "%s wyvern", attacks[atnr]);
      else if (level == 3)
        sprintf(CONTR(pl)->title, "%s dragon", attacks[atnr]);
      else {
        if (skin->resist[atnr] > 80)
    sprintf(CONTR(pl)->title, "legendary %s dragon", attacks[atnr]);
        else if (skin->resist[atnr] > 50)
    sprintf(CONTR(pl)->title, "ancient %s dragon", attacks[atnr]);
        else
    sprintf(CONTR(pl)->title, "big %s dragon", attacks[atnr]);
      }
    }
    */
}

/*
 * This function is called when a dragon-player gains
 * an overall level. Here, the dragon might gain new abilities
 * or change the ability-focus.
 */
void dragon_level_gain(object *who)
{
    object *abil    = NULL;    /* pointer to dragon ability force*/
    object *skin    = NULL;    /* pointer to dragon skin force*/
    object *tmp     = NULL;     /* tmp. object */
    char    buf[MAX_BUF];      /* tmp. string buffer */

    /* now grab the 'dragon_ability'-forces from the player's inventory */
    for (tmp = who->inv; tmp != NULL; tmp = tmp->below)
    {
        if (tmp->type == FORCE)
        {
            if (tmp->arch->name == shstr_cons.dragon_ability_force)
                abil = tmp;
            if (tmp->arch->name == shstr_cons.dragon_skin_force)
                skin = tmp;
        }
    }
    /* if the force is missing -> bail out */
    if (abil == NULL)
        return;

    /* The ability_force keeps track of maximum level ever achieved.
       New abilties can only be gained by surpassing this max level */
    if (who->level > abil->level)
    {
        /* increase our focused ability */
        abil->resist[abil->stats.exp]++;

        if (abil->resist[abil->stats.exp] > 0 && abil->resist[abil->stats.exp] % 5 == 0)
        {
            /* time to hand out a new ability-gift */
            dragon_ability_gain(who, abil->stats.exp, (int) ((1 + abil->resist[abil->stats.exp]) / 5.));
        }

        if (abil->last_eat > 0 && atnr_is_dragon_enabled(abil->last_eat))
        {
            /* apply new ability focus */
            sprintf(buf, "Your metabolism now focuses on %s!", attack_name[abil->last_eat]);
            new_draw_info(NDI_UNIQUE | NDI_BLUE, 0, who, buf);

            abil->stats.exp = abil->last_eat;
            abil->last_eat = 0;
        }

        abil->level = who->level;
    }

    /* last but not least, set the new title for the dragon */
    set_dragon_name(who, abil, skin);
}


/** Adjust the monster's datas for level, map settings and game settings
 * when put in play.
 */
void fix_monster(object *op)
{
    int wc_mali=0, ac_mali=0, snare_penalty=0, slow_penalty=0;
    object *base, *tmp, *spawn_info=NULL, *bow=NULL;
    float   tmp_add;

    if (op->head) /* don't adjust tails or player - only single objects or heads */
        return;

#ifdef DEBUG_FIX_MONSTER
    LOG(llevDebug, "FIX_MONSTER(%s [%x]): called\n", query_name(op), op->count);
#endif

    base = insert_base_info_object(op); /* will insert or/and return base info */
    op->level = base->level;

    if (!QUERY_FLAG(&op->arch->clone, FLAG_BLIND))
        CLEAR_FLAG(op, FLAG_BLIND);
    if (!QUERY_FLAG(&op->arch->clone, FLAG_SLOWED))
        CLEAR_FLAG(op, FLAG_SLOWED);
    if (!QUERY_FLAG(&op->arch->clone, FLAG_FEARED))
        CLEAR_FLAG(op, FLAG_FEARED);
    /* rooted is set when a snare effect has reached 100% */
    if (!QUERY_FLAG(&op->arch->clone, FLAG_ROOTED))
        CLEAR_FLAG(op, FLAG_ROOTED);
    if (!QUERY_FLAG(&op->arch->clone, FLAG_CONFUSED))
        CLEAR_FLAG(op, FLAG_CONFUSED);
    if (!QUERY_FLAG(&op->arch->clone, FLAG_PARALYZED))
        CLEAR_FLAG(op, FLAG_PARALYZED);

    CLEAR_FLAG(op, FLAG_READY_BOW);
    CLEAR_FLAG(op, FLAG_READY_SPELL);
    for (tmp = op->inv; tmp; tmp = tmp->below)
    {
        /* handle forces */
        if(tmp->type == FORCE)
        {
            if(tmp->sub_type1 == ST1_FORCE_FEAR)
            {
                SET_FLAG(op, FLAG_FEARED);
            }
            else if(tmp->sub_type1 == ST1_FORCE_SNARE)
            {
                snare_penalty += tmp->last_heal; 

                if(snare_penalty >= 100)
                    SET_FLAG(op, FLAG_ROOTED);
            }
            else if(tmp->sub_type1 == ST1_FORCE_SLOWED)
            {
                slow_penalty += tmp->last_heal; 
                SET_FLAG(op, FLAG_SLOWED);
            }
            else if(tmp->sub_type1 == ST1_FORCE_PARALYZE)
            {
                SET_FLAG(op, FLAG_PARALYZED);
            }
            else if(tmp->sub_type1 == ST1_FORCE_DRAIN) /* level drain */
            {
                op->level -= tmp->level;
                if(op->level < 1)
                    op->level = 1;
            }
            else if(tmp->sub_type1 == ST1_FORCE_DEPLETE) /* depletion */
            {
                /* nothing special here - depletion is at first an anti player feature */
                wc_mali = tmp->stats.Str+tmp->stats.Con+tmp->stats.Int;
                ac_mali = tmp->stats.Dex+tmp->stats.Wis+tmp->stats.Pow;
            }
        }
        else if (tmp->type == BOW && !bow  && tmp->sub_type1 == 128)
        {
            bow = tmp;
            SET_FLAG(op, FLAG_READY_BOW);
            SET_FLAG(tmp, FLAG_APPLIED);
        }
        else if (tmp->type == ABILITY)
            SET_FLAG(op, FLAG_READY_SPELL);
        else if(tmp->type == SPAWN_POINT_INFO)
            spawn_info = tmp;
    }

    /* assign amun to our ability bow - and put them inside the inventory of it
     * one time a bit more work here but then fast access to amun for range firing
     * mobs.
     */
    if(bow)
    {
        object *tmp2;

        for (tmp = op->inv; tmp; tmp = tmp2)
        {
            if(tmp->type == ARROW && tmp->race == bow->race)
            {
                remove_ob(tmp);
                insert_ob_in_ob(tmp, bow);
            }
            tmp2=tmp->below;
        }

    }

    /* pre adjust */
    op->stats.maxhp = (base->stats.maxhp * (op->level + 3) + (op->level / 2) * base->stats.maxhp) / 10;
    if(op->stats.maxhp <= 0)
        op->stats.maxhp = 1;
    op->stats.maxsp = base->stats.maxsp * (op->level + 1);
    if(op->stats.maxsp <= 0)
        op->stats.maxsp = 1;
    op->stats.maxgrace = base->stats.maxgrace * (op->level + 1);
    if(op->stats.maxgrace <= 0)
        op->stats.maxgrace = 1;

    /* remember: -1 is a place holder - if == -1, we fill in max value.
     * if the value is != -1, object has explicit set to a different value
     * (for example to simulate on a map a injured target) or its previous
     * damaged and saved
     */

    if (op->stats.hp == -1)
        op->stats.hp = op->stats.maxhp;
    if (op->stats.sp == -1)
        op->stats.sp = op->stats.maxsp;
    if (op->stats.grace == -1)
        op->stats.grace = op->stats.maxgrace;

    /* cap the pools to <=max */
    if (op->stats.hp > op->stats.maxhp)
        op->stats.hp = op->stats.maxhp;
    if (op->stats.sp > op->stats.maxsp)
        op->stats.sp = op->stats.maxsp;
    if (op->stats.grace > op->stats.maxgrace)
        op->stats.grace = op->stats.maxgrace;

    op->stats.ac = base->stats.ac + op->level + (op->level / 6) - (ac_mali*2);
    /* + level/5 to catch up the equipment improvements of
     * the players in armour items.
     */
    op->stats.wc = base->stats.wc + op->level + (op->level / 4) - (wc_mali*2);
    op->stats.dam = base->stats.dam;

    /* adjust swing speed and move speed by slow penalty */
    if(QUERY_FLAG(op,FLAG_FEARED))
    {
        int m;

        m = op->stats.wc/8; /* 15% mali wc */
        if(!m)
            m=1;
        op->stats.wc -= m;

        m = op->stats.ac/8; /* 15% mali ac */
        if(!m)
            m=1;
        op->stats.ac -= m;

        slow_penalty +=15; /* add a 15% slowness factor to swing & movement */
    }

    set_mobile_speed(op, 0);

    /* first, we cap slow effects to 80% */
    if(slow_penalty > 80)
        slow_penalty = 80;

    if(slow_penalty)
    {
        snare_penalty += slow_penalty;
        op->weapon_speed *= ((float)(100+slow_penalty)/100.0f);
    }

    /* we don't cap snare effects - if >=100% we set root flag */
    if(snare_penalty)
    {
        if(snare_penalty >= 100)
        {
            SET_FLAG(op,FLAG_ROOTED);
            op->weapon_speed = 0.15f;
        }
        else
        {
            op->speed *= ((float)(100-snare_penalty)/100.0f);
        }
    }

    /* post adjust */
    if ((tmp_add = lev_damage[op->level / 3] - 0.75f) < 0)
        tmp_add = 0;
    op->stats.dam = (sint16) (((float) op->stats.dam * ((lev_damage[(op->level < 0) ? 0 : op->level] + tmp_add)
                     * (0.925f + 0.05 * (op->level / 10)))) / 10.0f);

    /* Set up AI in op->custom_attrset */
    if(! MOB_DATA(op))
    {
        op->custom_attrset = get_poolchunk(pool_mob_data);
        MOB_DATA(op)->behaviours = setup_behaviours(op);
    }

    /* insert a quick jump in the MOB_DATA(() to its spawn info
     * Wonderful, i can skip the spawn point info loop now.
     * Thats a nice speed increase! MT
     */
    MOB_DATA(op)->spawn_info = spawn_info;
}

/* insert and initialize base info object in object op
 * Return: ptr to inserted base_info
 */
object * insert_base_info_object(object *op)
{
    object     *tmp, *head;
    objectlink *ol;
    object     *outermost;

    op->head != NULL ? (head = op->head) : (head = op);

    if (op->type == PLAYER)
    {
        LOG(llevBug, "insert_base_info_object() Try to inserting base_info in player %s!\n", query_name(head));
        return NULL;
    }

    if ((tmp = find_base_info_object(head)))
        return tmp;

    tmp = get_object();
    tmp->arch = op->arch;
    /* we don't need to trigger the
     * treasurelist link/unlink stuff here.
     * IF we ever need the original treasurelist
     * in the baseinfo, just removes this lines,
     * it will do no harm and copy_object will link it in.
     */
    ol = head->randomitems;
    head->randomitems = NULL;
    copy_object_data(head, tmp); /* copy without put on active list */
    head->randomitems = ol;
    tmp->type = TYPE_BASE_INFO;
    tmp->speed_left = tmp->speed;
    tmp->speed = 0.0f; /* ensure this object will not be active in any way */
    tmp->face = base_info_archetype->clone.face;
    SET_FLAG(tmp, FLAG_NO_DROP);
    CLEAR_FLAG(tmp, FLAG_ANIMATE);
    CLEAR_FLAG(tmp, FLAG_FRIENDLY);
    CLEAR_FLAG(tmp, FLAG_ALIVE);
    CLEAR_FLAG(tmp, FLAG_MONSTER);
    CLEAR_FLAG(tmp, FLAG_IN_ACTIVELIST);
    insert_ob_in_ob(tmp, head); /* and put it in the mob */

    /* Store position (for returning home after aggro is lost...) */
    /* Has to be done after insert_ob_in_ob() */
    outermost = head;
    while(outermost != NULL && outermost->map == NULL)
        outermost = outermost->env;
    if(outermost && outermost->map)
    {
        tmp->x = outermost->x;
        tmp->y = outermost->y;
        FREE_AND_ADD_REF_HASH(tmp->slaying, outermost->map->path);
    }
    else
    {
        FREE_AND_CLEAR_HASH(tmp->slaying);
        LOG(llevDebug, "insert_base_info_object(): Can't set up home location for '%s' - not even close to a map.\n", query_name(head));
    }

    return tmp;
}

/* find base_info in *op
 * Return: ptr to inserted base_info
 */
object * find_base_info_object(object *op)
{
    object *tmp;

    for (tmp = op->inv; tmp; tmp = tmp->below)
    {
        if (tmp->type == TYPE_BASE_INFO)
            return tmp;
    }

    return NULL; /* no base_info in this object found */
}

/** Set set the movement speed of a mobile.
 * 
 * The base speed of a mob is stored in base->speed_left.
 * The actual speed is stored in op->speed as base->speed_left * factor.
 *
 * Possible factors: 
 * 1 = mob is slowed (by magic) (minimum factor)
 * 2 = normal mob speed - moving normal
 * 3 = mob is moving fast
 * 4 = mob is running/attack speed
 * 5 = mob is hasted and moving full speed (maximum factor)
 *
 * @input op mob object to set speed for
 * @input factor a speed factor for forcing a speed, or 0 to automatically
 * compute a factor based on the AI state, spells etc.
 */
void set_mobile_speed(object *op, int factor)
{
    object *base;
    float   old_speed;
    int actual_factor;

    base = insert_base_info_object(op); /* will insert or/and return base info */

    old_speed = op->speed;

    if (factor) /* if factor != 0, we force a setting of this speed */
        actual_factor = factor;
    else /* we will generate the speed by setting of the mobile */
    {
        /* TODO: more logic when we add haste/slow spells */
        
        if(op->type == MONSTER && MOB_DATA(op))
            actual_factor = MOB_DATA(op)->move_speed_factor; /* AI-selected speed */
        else if (OBJECT_VALID(op->enemy, op->enemy_count))
            actual_factor = 4; /* Attack speed */
        else
            actual_factor = 2; /* Backup */
    }
        
    op->speed = base->speed_left * CLAMP(actual_factor, 1, 5);
    
     LOG(-1,"SET SPEED: %s ->%f (=%d*%f) o:%f\n", query_name(op), op->speed, actual_factor, base->speed_left, old_speed);
    /* update speed if needed */
    if ((old_speed && !op->speed) || (!old_speed && op->speed))
        update_ob_speed(op);
}
