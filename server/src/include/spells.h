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

#define SPELL_USE_INTERN 0x00 /* special spells - don't list them as avaible spell */
#define SPELL_USE_CAST   0x01 /* spell can be casted normal */
#define SPELL_USE_BALM   0x02
#define SPELL_USE_DUST   0x04
#define SPELL_USE_SCROLL 0x08
#define SPELL_USE_HORN   0x10
#define SPELL_USE_WAND   0x20
#define SPELL_USE_ROD    0x40
#define SPELL_USE_POTION 0x80
#define SPELL_USE_BOOK   0x100 /* well, normally we don't use spellbooks as random stuff
                                * except some special "quest" spells for quest monster
                               */

#define SPELL_TYPE_NATURAL 0 /* special case: this is use like a spell but natural ability - effect is non magical */
#define SPELL_TYPE_WIZARD  1 /* base mage spell: using mana */
#define SPELL_TYPE_PRIEST  2 /* base cleric spell: using grace */
#define SPELL_TYPE_NROF    2 /* This should be highest number of SPELL_TYPES */

#define SPELL_DESC_TOWN         0x01    /* Spell is safe to cast in as TOWN marked maps */
#define SPELL_DESC_DIRECTION    0x02    /* Spell is fired in a direction (bullets, bolts, ... ) */
/* stack the next flags to define whats possible with this spell */
#define SPELL_DESC_SELF         0x04    /* Spell can be cast on self (with target system) */
#define SPELL_DESC_FRIENDLY     0x08    /* Spell can be cast on friendly (with target system) */
#define SPELL_DESC_ENEMY        0x10    /* Spell can be cast on enemy (with target system) */
#define SPELL_DESC_GROUP        0x20    /* Spell can be cast on group members  */
/* end stack flags */
#define SPELL_DESC_SUMMON       0x40    /* Spell summons something */
#define SPELL_DESC_PARALYZED    0x80    /* Spell can be casted even when paralyzed (TODO) */

#define SPELL_DESC_WIS          0x100   /* special flag - if set, this is a "prayer" using WIS
                                                * when not set it use INT as is a spell
                                             */
#define SPELL_ACTIVE 1
#define SPELL_DEACTIVE 0

typedef struct spell_struct
{
    char    name[BIG_NAME];   /* name of this spell */
    int     type;                /* Type of spell: wizard, priest, ... */
    int     level;           /* Level required to cast this spell */
    int     sp;              /* Spellpoint-cost to cast it */
    int		time;            /* How many ticks it takes to cast the spell */
    int     scrolls;         /* thats from 1 to <scrolls> nrof we will generate for potions/scrolls... */
    int     charges;         /* If it can be used in wands, max # of charges */
    int     range;         /* if target spell, this is max range to target */
    float   value_mul;      /* used when we have a item of tihs spell kind.
                             * a magic potion has vaule x. We do: (x * value_mul)*level
                             */
	int     bdam;		/*  base damage  */
    int     bdur;		/*  base duration  */
    int     ldam;		/*  damage adjustment for level  */
    int     ldur;		/*  duration adjustment for level  */
	int     spl;		/*  we add spl points to sp */
	int     spl_level;	/*  every spl_level until */
	int     spl_max;	/*  as long its lower as spl_max */
    int     fumble_factor;     /* replaces level in spell fumble calcs*/
    int     sound; /* number of sound id for this sound */

    int     spell_use;       /* Define to what items this spell can be bound (potion, rod,,, ) */
    uint32  flags;           /* Used for SPELL_DESC_xx flags */
    uint32  path;           /* Path this spell belongs to */
    char   *archname;    /* Pointer to archetype used by spell */
    int     is_active;           /* if 0 then spell is disabled and can't be cast or used */
} spell;

/* When adding new spells, don't insert into the middle of the list -
 * add to the end of the list.  Some archetypes and treasures require
 * the spell numbers to be as they are.
 */

#define SP_NO_SPELL -1

enum spellnrs
{
    /* HERE STARTS THE DAIMONIN LIST */
    SP_FIRESTORM,
    SP_ICESTORM,
    SP_MINOR_HEAL,
    SP_CURE_POISON,
    SP_CURE_DISEASE,
    SP_STRENGTH,
    SP_IDENTIFY,
    SP_DETECT_MAGIC,
    SP_DETECT_CURSE,
    SP_REMOVE_CURSE,
    SP_REMOVE_DAMNATION,
    SP_CAUSE_LIGHT,
    SP_FIREBOLT,
    SP_BULLET,
    SP_FROSTBOLT,
    SP_REMOVE_DEPLETION,
    SP_PROBE,
    SP_REMOVE_DEATHSICK,
    SP_RESTORATION,
    SP_S_LIGHTNING,

    NROFREALSPELLS, /* LAST ENTRY */

    /* ALL DOWN HERE ARE ARTIFACTS FROM CROSSFIRE */
    SP_S_FIREBALL,
    SP_M_FIREBALL,
    /*0*/
    SP_L_FIREBALL,
    SP_L_LIGHTNING,
    SP_M_MISSILE,
    /*5*/
    SP_BOMB,
    SP_FIRE_ELEM,
    SP_EARTH_ELEM,
    SP_WATER_ELEM,
    /*10*/
    SP_AIR_ELEM,
    SP_D_DOOR,
    SP_EARTH_WALL,
    SP_PARALYZE,
    /*15*/
    SP_MAGIC_MAPPING,
    SP_TURN_UNDEAD,
    SP_FEAR,
    SP_POISON_CLOUD,
    SP_WOW,
    /*20*/
    SP_DESTRUCTION,
    SP_PERCEIVE,
    SP_WOR,
    SP_INVIS,
    SP_INVIS_UNDEAD,
    /*25*/
    /*SP_probe*/    SP_LARGE_BULLET,
    SP_IMPROVED_INVIS,
    SP_HOLY_WORD,
    /*30*/
    SP_MED_HEAL,
    SP_MAJOR_HEAL,
    SP_HEAL,
    SP_CREATE_FOOD,
    SP_EARTH_DUST,
    /*35*/
    SP_ARMOUR,
    SP_DEXTERITY,
    SP_CONSTITUTION,
    SP_CHARISMA,
    /*40*/
    SP_FIRE_WALL,
    SP_FROST_WALL,
    SP_PROT_COLD,
    SP_PROT_ELEC,
    SP_PROT_FIRE,
    /*45*/
    SP_PROT_POISON,
    SP_PROT_SLOW,
    SP_PROT_PARALYZE,
    SP_PROT_DRAIN,
    SP_PROT_MAGIC,
    /*50*/
    SP_PROT_ATTACK,
    SP_LEVITATE,
    SP_SMALL_SPEEDBALL,
    SP_LARGE_SPEEDBALL,
    SP_HELLFIRE,
    /*55*/
    SP_FIREBREATH,
    SP_LARGE_ICESTORM,
    SP_CHARGING,
    SP_POLYMORPH,
    SP_CANCELLATION,
    /*60*/
    SP_MASS_CONFUSION,
    SP_PET,
    SP_SLOW,
    SP_REGENERATE_SPELLPOINTS,
    /*65*/
    SP_PROT_CONFUSE,
    SP_PROT_CANCEL,
    SP_PROT_DEPLETE,
    SP_ALCHEMY,
    /*70*/

    SP_DETECT_MONSTER,
    /*75*/
    SP_DETECT_EVIL,
    SP_HEROISM,
    SP_AGGRAVATION,
	SP_CONFUSION,
    /*80*/
    SP_GOLEM,
    SP_SHOCKWAVE,
    SP_COLOR_SPRAY,
    SP_HASTE,
    SP_FACE_OF_DEATH,
    /*85*/
    SP_BALL_LIGHTNING,
    SP_METEOR_SWARM,
    SP_METEOR,
    SP_MYSTIC_FIST,
    SP_RAISE_DEAD,
    /*90*/
    SP_RESURRECTION,
    SP_REINCARNATION,
    /* mlee's spells*/

    SP_IMMUNE_COLD,
    SP_IMMUNE_ELEC,
    SP_IMMUNE_FIRE,
    /*95*/
    SP_IMMUNE_POISON,
    SP_IMMUNE_SLOW,
    SP_IMMUNE_PARALYZE,
    SP_IMMUNE_DRAIN,
    SP_IMMUNE_MAGIC,
    /*100*/
    SP_IMMUNE_ATTACK,
    SP_INVULNERABILITY,
    SP_PROTECTION,
    /*105*/
    /*Some more new spells by peterm */
    SP_RUNE_FIRE,
    SP_RUNE_FROST,
    SP_RUNE_SHOCK,
    SP_RUNE_BLAST,
    SP_RUNE_DEATH,
    SP_RUNE_MARK,
    SP_BUILD_DIRECTOR,
    /*110*/
    SP_CHAOS_POOL,
    SP_BUILD_BWALL,
    SP_BUILD_LWALL,
    SP_BUILD_FWALL,
    SP_RUNE_MAGIC,
    /*115*/
    SP_RUNE_DRAINSP,
    SP_RUNE_ANTIMAGIC,
    SP_RUNE_TRANSFER,
    SP_TRANSFER,
    SP_MAGIC_DRAIN,
    /*120*/
    SP_COUNTER_SPELL,
    SP_DISPEL_RUNE,
    SP_CURE_CONFUSION,
    SP_RESTORATION2,
    SP_SUMMON_EVIL_MONST,
    /*125*/
    SP_COUNTERWALL,
    SP_CAUSE_MEDIUM,
    SP_CAUSE_HEAVY,
    SP_CHARM,
    /*130*/
    SP_BANISHMENT,
    SP_CREATE_MISSILE,
    SP_SHOW_INVIS,
    SP_XRAY,
    SP_PACIFY,
    /*135*/
    SP_SUMMON_FOG,
    SP_STEAMBOLT,
    /* lots of new cleric spells,many need MULTIPLE_GODS defined to be
    * very usefull - b.t. */
    SP_COMMAND_UNDEAD,
    SP_HOLY_ORB,
    SP_SUMMON_AVATAR,
    /*140*/
    SP_HOLY_POSSESSION,
    SP_BLESS,
    SP_CURSE,
    SP_REGENERATION,
    SP_CONSECRATE,
    /*145*/
    SP_SUMMON_CULT,
    SP_CAUSE_CRITICAL,
    SP_HOLY_WRATH,
    SP_RETRIBUTION,
    SP_FINGER_DEATH,
    /*150*/
    SP_INSECT_PLAGUE,
    SP_HOLY_SERVANT,
    SP_WALL_OF_THORNS,
    SP_STAFF_TO_SNAKE,
    SP_LIGHT,
    /*155*/
    SP_DARKNESS,
    SP_NIGHTFALL,
    SP_DAYLIGHT,
    SP_SUNSPEAR,
    SP_FAERY_FIRE,
    /*160*/
    SP_CURE_BLINDNESS,
    SP_DARK_VISION,
    SP_BULLET_SWARM,
    SP_BULLET_STORM,
    SP_CAUSE_MANY,
    /*165*/
    SP_S_SNOWSTORM,
    SP_M_SNOWSTORM,
    SP_L_SNOWSTORM,
    SP_CAUSE_EBOLA,
    /*170*/
    SP_CAUSE_FLU,
    SP_CAUSE_PLAGUE,
    SP_CAUSE_LEPROSY,
    SP_CAUSE_SMALLPOX,
    SP_CAUSE_PNEUMONIC_PLAGUE,
    /*175*/
    SP_CAUSE_ANTHRAX,
    SP_CAUSE_TYPHOID,
    SP_MANA_BLAST,
    SP_S_MANABALL,
    SP_M_MANABALL,
    /*180*/
    SP_L_MANABALL,
    SP_MANA_BOLT,
    SP_DANCING_SWORD,
    SP_ANIMATE_WEAPON,
    SP_CAUSE_COLD,
    /* 185 */
    SP_DIVINE_SHOCK,
    SP_WINDSTORM,
    /* the below NIY */
    SP_SANCTUARY,
    SP_PEACE,
    SP_SPIDERWEB,
    /* 190 */
    SP_CONFLICT,
    SP_RAGE,
    SP_FORKED_LIGHTNING,
    SP_POISON_FOG,
    SP_FLAME_AURA,
    /* 195 */
    SP_VITRIOL,
    SP_VITRIOL_SPLASH,
    SP_IRONWOOD_SKIN,
    SP_WRATHFUL_EYE,
    SP_TOWN_PORTAL,
    /* 200 */
    SP_MISSILE_SWARM,
    SP_CAUSE_RABIES
};

#define IS_SUMMON_SPELL(spell) (spells[type].flags&SPELL_DESC_SUMMON)


#define PATH_DMG_MULT(__op,__spell) (((__op->path_attuned & __spell->path) ? 1.25 : 1) * \
	((__op->path_repelled & __spell->path) ? 0.7 : 1))
#define PATH_SP_MULT(__op,__spell) (((__op->path_attuned & __spell->path) ? 1.25 : 1) * \
                ((__op->path_repelled & __spell->path) ? 0.8 : 1))
#define PATH_TIME_MULT(__op,__spell) (((__op->path_attuned & __spell->path) ? 1.25 : 1) * \
                ((__op->path_repelled & __spell->path) ? 0.8 : 1))

extern char        *spellpathnames[NRSPELLPATHS];
extern archetype   *spellarch[NROFREALSPELLS];

/* i added spellNPC here as special... its used for example to force scripted npc
 * action which is normally ingame not legal - like shopkeepers who casts self only
 * spells like identify over players.
 */
typedef enum SpellTypeFrom
{
    spellNormal,
    spellWand,
    spellRod,
    spellHorn,
    spellScroll,
    spellPotion,
    spellNPC
}    SpellTypeFrom;

