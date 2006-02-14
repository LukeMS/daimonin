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

typedef struct _level_color
{
    int                 green;
    int                 blue;
    int                 yellow;
    int                 orange;
    int                 red;
    int                 purple;
}_level_color;

extern _level_color level_color[201];

enum
{
    /* fire modes submited from client */
    FIRE_MODE_NONE                              = -1,
    FIRE_MODE_BOW,
    FIRE_MODE_SPELL,
    FIRE_MODE_WAND,
    FIRE_MODE_SKILL,
    FIRE_MODE_THROW,
    FIRE_MODE_SUMMON
};

typedef enum rangetype
{
    range_bottom        = -1,
    range_none          = 0,
    range_bow           = 1,
    range_magic         = 2,
    range_wand          = 3,
    range_rod           = 4,
    range_scroll        = 5,
    range_horn          = 6,
    range_skill         = 7,
    range_potion        = 8,
    range_dust          = 9,
    range_size          = 10
}    rangetype;


typedef enum usekeytype
{
    key_inventory       = 0,
    keyrings            = 1,
    containers          = 2,
}    usekeytype;

/* used for item damage system */
enum
{
    PLAYER_EQUIP_MAIL,
    PLAYER_EQUIP_GAUNTLET,
    PLAYER_EQUIP_BRACER,
    PLAYER_EQUIP_HELM,
    PLAYER_EQUIP_BOOTS,
    PLAYER_EQUIP_CLOAK,
    PLAYER_EQUIP_GIRDLE,
    PLAYER_EQUIP_SHIELD,
    PLAYER_EQUIP_RRING,
    /* this must 1 entry before LRING! */
    PLAYER_EQUIP_LRING,
    PLAYER_EQUIP_AMULET,
    PLAYER_EQUIP_WEAPON1,
    PLAYER_EQUIP_BOW,
    PLAYER_EQUIP_MTOOL,
    PLAYER_EQUIP_MAX
    /* last index */
};

/* we can include more flags here... */

#define PLAYER_AFLAG_NO 0
#define PLAYER_AFLAG_FIGHT 1    /* if this flag is set, show player fight animation */
#define PLAYER_AFLAG_ENEMY 2    /* if this flag is set at END of a animation,
                                 * set fight flag and clear this flag. It is set in hit_player()
                                    * when the player swings to an enemy
                                    */
#define PLAYER_AFLAG_ADDFRAME 4 /* intern */

/* mute engine */

#define MUTE_MODE_SHOUT 1
#define MUTE_MODE_SAY   2

#define MUTE_FLAG_SHOUT         0x01
#define MUTE_FLAG_SHOUT_WARNING 0x02
#define MUTE_FLAG_SAY           0x04
#define MUTE_FLAG_SAY_WARNING   0x08

#define MUTE_FREQ_SHOUT     16
#define MUTE_FREQ_SAY       16
#define MUTE_AUTO_NORMAL    (8*10)
#define MUTE_AUTO_HARD      (8*30)
#define MUTE_MSG_FREQ       (8*5)

#define PLAYER_HPGEN_DELAY 10

/* QUCKHACK - can be removed for 1.0 */
#define PLAYER_FILE_VERSION_DEFAULT 0
#define PLAYER_FILE_VERSION_BETA3	1
#define PLAYER_FILE_VERSION_BETA4	2

#ifdef WIN32
#pragma pack(push,1)
#endif

/* slowly reworking this struct - some old values in - MT2003 */
typedef struct pl_player
{
    /* this is not cleared with memset - seek for offsetof((....,maplevel) */
    struct pl_player   *prev;               /* Pointer to the prev player. if NULL, this is the first one */
    struct pl_player   *next;               /* Pointer to next player, NULL if this is last */
    /* first and last player in player list can direct
                                 * accessed by first_player/last_player global ptr */
    NewSocket           socket;             /* Socket information for this player */

    /* all this is set to 0 with memset */
    char                maplevel[MAX_BUF];      /* Name of the map the player is on */
	int					map_update_cmd;			/* for server->client protocol */
	int					map_update_tile;			/* for server->client protocol */
    struct mapdef      *last_update;			/* when moving on tiled maps, player can change
                                                 * map without triggering mapevents and new_map_cmd.
                                                 * This will break client cache and script events.
                                                 * This value will used as control value.*/

    object             *ob;                     /* The object representing the player */
    object             *golem;                  /* Which golem is controlled */
    object             *anim_enemy;             /* which enemy we hack. needed to skip extra swing animation */

    object             *selected_weapon;        /* thats the weapon in our hand */
    object             *skill_weapon;           /* thats the hth skill we use when we not use a weapon (like karate) */
    object             *target_object;          /* our target */

    object             *equipment[PLAYER_EQUIP_MAX]; /* pointers to applied items in the players inventory */
    object             *skill_ptr[NROFSKILLS];       /* quick jump table to skill objects in the players inv. */

    int                 base_skill_group[3];            /* guild/base exp skill groups for default exp gain */
    object             *highest_skill[NROFSKILLGROUPS]; /* highest skill of every skill group */

    object             *exp_obj_ptr[NROFSKILLGROUPS];       /* skill exp groups ptr (agility, mental,..) */
    int                 last_exp_obj_exp[NROFSKILLGROUPS];   /* shadow variables for client updating */
    int                 last_exp_obj_level[NROFSKILLGROUPS]; /* sic */

    object             *mark;       /* marked object */
    object             *age_force;  /* quick jump to our age force */

    object             *map_below;  /* ptr used from local map player chain */
    object             *map_above;
    object             *container;          /* Current container being used. */
    uint32              container_count;        /* the count of the container */
    object             *container_above;    /* that points to a PLAYER ob, accessing this container too!
                                                  * if this is NULL, we are the "last" one looking in ->container.
                                                  */
    object             *container_below;    /* same as above - if this is NULl, we are "last" looking the container */


    uint32              anim_enemy_count;   /* hm, this can be kicked now - i do it for a quick hack to
                                                 * implement the animations. use is_melee_range() instead.
                                                 */

    int                 target_hp;              /* for the client target HP marker - special shadow*/
    int                 set_skill_weapon;       /* skill number of used weapon skill for fast access */
    int                 set_skill_archery;      /* same for archery */
    int                 bed_x;                      /* x,y - coordinates of respawn (savebed) */
    int                 bed_y;
    uint32              golem_count;                /* Which golem is controlled - the id count */

    int                 firemode_type;        /* firemode_xxx are set from command_fire() */
    int                 firemode_tag1;
    int                 firemode_tag2;

    int                  gmaster_mode;
    struct oblnk        *gmaster_node;

    /* mute and "communication" frequency control */
    uint32              mute_flags;
    unsigned long       mute_freq_shout;
    unsigned long       mute_freq_say;
    unsigned long       mute_msg_count; /* tell only all x seconds the player "you are still muted" */
    unsigned long       mute_counter; /* must "mutes" all except "talk".
                                       * So, don't get muted when on a map with magic mouth
                                       */

    /* "skill action timers" - used for action delays like cast time */
    uint32              action_casting;
    uint32              action_range;

    object               *quest_one_drop;
    object               *quests_done;
    object               *quests_type_normal;
    object               *quests_type_kill;

    tag_t                quest_one_drop_count;
    tag_t                quests_done_count;
    tag_t                quests_type_normal_count;
    tag_t                quests_type_kill_count;

    uint32              exp_calc_tag;               /* used from aggro.c/exp.c */
    object             *exp_calc_obj;
    uint32              mark_count;                 /* count or mark object */
    sint32              skill_exp[NROFSKILLS];		/* shadow register for updating skill values to client */
    uint32              target_object_count;		/* count of target - NOTE: if we target ourself, this count it 0
                                                    * this will kick us out of enemy target routines - all functions
                                                    * who handle self target know it and use only target_object -
                                                    * for our own player object the pointer will never change for us.
                                                    */
    uint32              target_map_pos;				/* last target search position */
    uint32              mode;						/* Mode of player for pickup. */
    sint32              group_id;					/* unique group id number - this is a unique number like the object count */
    sint32              group_status;				/* status of invite or group */
	int					exp_bonus;					/* extra exp bonus */
    int                 weapon_sp;					/* weapon speed - float *1000 for the client */
	int					map_tile_x, map_tile_y;		/* these is our last position of map we send to client */ 
	int					map_off_x, map_off_y;		/* scroll offset between 2 maps of client update */ 

    /* we don't need here the count of group object links- because the game will explicit
     * link/unlink party members when their player object change.
     * exception is group leader - its only used to confirm a invite
     */
    object             *group_leader;				/* pointer to group leader or invite */
    uint32              group_leader_count;			/* for invite... */
    object             *group_prev;					/* previous member of group */
    object             *group_next;					/* next member of group */

    uint32              update_ticker;				/* global_round tick where player was updated */
    float               last_speed;
    float               speed;						/* shadow speed value, set in fix_player() to cover flag effects */

    sint16              target_level;
    sint16              age;        /* the age of our player */
    sint16              age_add;    /* unnatural changes to our age - can be removed by restoration */
    sint16              age_changes; /* permanent changes .... very bad (or good when younger) */
    sint16              age_max;    /* the age of our player */
    sint16              skill_level[NROFSKILLS];    /* shadow register for updint skill levels to client */
    sint16              encumbrance;  /*  How much our player is encumbered  */
    uint16              anim_flags;             /* some anim flags for special player animation handling */

    uint16              nrofknownspells;    /* Index in the above array */
    sint16              known_spells[NROFREALSPELLS]; /* Spells known by the player */

    char                target_hp_p;                /* for the client target HP real % value*/

    signed char         digestion;          /* Any bonuses/penalties to digestion */
    signed char         gen_sp_armour;      /* Penalty to sp regen from armour */

    signed char         gen_hp;             /* Bonuses to regeneration speed of hp */
    signed char         gen_sp;             /* Bonuses to regeneration speed of sp */
    signed char         gen_grace;          /* Bonuses to regeneration speed of grace */

    int                 reg_hp_num;                 /* thats how much every reg tick we get */
    int                 reg_sp_num;
    int                 reg_grace_num;
	int					damage_timer;				/* hp recovery timer for last hp */

    sint16              base_hp_reg;                /* our real tick counter for hp regenerations */
    sint16              base_sp_reg;                /* our real tick counter for sp regenerations */
    sint16              base_grace_reg;         /* our real tick counter for grace regenerations */

    /* send to client - shadow & prepared gen_xx values */
    uint16              gen_client_hp;              /* Bonuses to regeneration speed of hp */
    uint16              gen_client_sp;              /* Bonuses to regeneration speed of sp */
    uint16              gen_client_grace;           /* Bonuses to regeneration speed of grace */
    uint16              last_gen_hp;
    uint16              last_gen_sp;
    uint16              last_gen_grace;

    uint8                group_mode;                    /* group mode use GROUP_MODE_XX */
    uint8               group_nrof;                    /* number of players in group */
    uint8               group_nr;                    /* player is #group_nr in his group - used for fast update */


    char                group_invite_name[MAX_PLAYER_NAME+1]; /* GROUP_MODE_INVITE: This player name can invite you
                                                               * we need string name here to handle for example a logout
                                                               * of this player.
                                                               */

    int					last_weapon_sp;
	int					p_ver;

    char                firemode_name[BIG_NAME*2];
    char                quick_name[BIG_NAME*3]; /* thats rank + name +" the xxxx" */
    char                savebed_map[MAX_BUF];  /* map where player will respawn after death */
    /* for smaller map sizes, only the the first elements are used (ie, upper left) */
    int                 blocked_los[MAP_CLIENT_X][MAP_CLIENT_Y]; /* in fact we only need char size, but use int for faster access */
    char                ext_title[MAX_EXT_TITLE];   /* for client: <Rank> <Name>\n<Gender> <Race> <Profession> */
    char                levhp[MAXLEVEL + 1];            /* What the player gained on that level */
    char                levsp[MAXLEVEL + 1];
    char                levgrace[MAXLEVEL + 1];
    sint8               last_resist[NROFATTACKS];   /* shadow register for client update resist table */

    uint32              player_loaded       : 1;            /* this flags is set when the player is loaded from file
                                                                     * and not just created. It is used to overrule the "no save
                                                                     * when exp is 0" rule - which can lead inventory duping.
                                                                     */

    uint32              name_changed        : 1;            /* If true, the player has set a name. */
    uint32              update_los          : 1;                /* If true, update_los() in draw(), and clear */
    uint32              combat_mode         : 1;            /* if true, player is in combat mode, attacking with weapon */
    uint32              praying             : 1;                /* if true, player is praying and gaining fast grace */
    uint32              was_praying         : 1;            /* internal used by praying to send pray msg to player */

    /* some dm flags */
    uint32              dm_stealth          : 1;            /* 1= no "XX enter the game" and no entry in /who */
    uint32              dm_light            : 1;                /* 1= all maps are shown in daylight for the dm */
    uint32              dm_removed_from_map : 1;    /* internal dm flag: player was removed from a map */

    /* all values before this line are tested and proofed */


    uint32              known_spell         : 1;     /* True if you know the spell of the wand */
    uint32              last_known_spell    : 1;/* What was last updated with draw_stats() */
    uint32              update_skills       : 1;   /* update skill list when set */

    uint32              silent_login        : 1;

    rangetype           shoottype;        /* Which range-attack is being used by player */
    rangetype           last_shoot;       /* What was last updated with draw_stats() */
    usekeytype          usekeys;          /* Method for finding keys for doors */
    sint16              chosen_spell;       /* Type of readied spell */
    sint16              chosen_item_spell;  /* Type of spell that the item fires */
    uint16              last_flags;     /* fire/run on flags for last tick */
    uint32              count;       /* Any numbers typed before a command */

    unsigned char       state;
    unsigned char       listening; /* Which priority will be used in info_all */

    unsigned char       fire_on;
    unsigned char       run_on;
    uint32              last_weight_limit;  /* Last weight limit transmitted to client */
    living              orig_stats;       /* Can be less in case of poisoning */
    living              last_stats;       /* Last stats drawn with draw_stats() */
    signed long         last_value;  /* Same usage as last_stats */
    long                last_weight;


    char                title[BIG_NAME]; /* we use ext. title now - we should remove this now! */

    unsigned char       last_level;

    char                killer[BIG_NAME];  /* Who killed this player. */
    char                last_cmd;
    char                last_tell[MAX_NAME];   /* last player that told you something [mids 01/14/2002] */

    char                password[MAX_PLAYER_PASSWORD]; /* 2 (seed) + 11 (crypted) + 1 (EOS) + 2 (safety) = 16 */

#ifdef AUTOSAVE
    uint32              last_save_tick;
#endif


    /* i disabled this now - search for last_used in the code.
     * perhaps we need this in the future.
     */

    /*object *last_used;*/     /* Pointer to object last picked or applied */
    /*long last_used_id;*/     /* Safety measures to be sure it's the same */

    /* All pets owned by this player */
    objectlink         *pets;
} player;


#ifdef WIN32
#pragma pack(pop)
#endif
