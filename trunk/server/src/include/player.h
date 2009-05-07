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
    FIRE_MODE_NONE = -1,
	FIRE_MODE_BOW,
	FIRE_MODE_SPELL,
	FIRE_MODE_SKILL,
	FIRE_MODE_INIT
};

typedef enum usekeytype
{
    key_inventory       = 0,
    keyrings            = 1,
    containers          = 2,
}    usekeytype;

/* used for item damage system */
enum
{
    PLAYER_EQUIP_MAIL = 0, /* First entry should always be 0 */
    PLAYER_EQUIP_GAUNTLET,
    PLAYER_EQUIP_BRACER,
    PLAYER_EQUIP_HELM,
    PLAYER_EQUIP_SHOULDER,
    PLAYER_EQUIP_LEGS,
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
    PLAYER_EQUIP_AMUN,
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
#define PLAYER_FILE_VERSION_BETA4   2

#ifdef WIN32
#pragma pack(push,1)
#endif

/* slowly reworking this struct - some old values in - MT2003 */
typedef struct pl_player
{
    /* this is not cleared with memset - seek for offsetof((....,maplevel) */

    struct pl_player   *prev;               /* Pointer to the prev player. if NULL, this is the first one */
    struct pl_player   *next;               /* Pointer to next player, NULL if this is last */

    NewSocket           socket;             /* Socket information for this player */

    /* WARNING!: maplevel MUST be the first struct member after socket! */
    /* start of hash strings ptr... */
    const char          *maplevel;              /* Name of the map the player is on */

    const char          *instance_name;         /* Name of the map the player is on */
    const char          *group_invite_name;     /* GROUP_MODE_INVITE: This player name can invite you */
    const char          *killer;                /* Who killed this player. */
    const char          *savebed_map;           /* map where player will respawn after death */
    const char          *orig_savebed_map;      /* map where player will respawn after death (original map) */
    const char          *orig_map;              /* Name of the map the player is on (original map) */
    /* we store the account name here too so we can later load player from dm accounts */
    const char          *account_name;          /* name of the account this player is part of */
    /* hash strings end*/

	uint32              name_changed        : 1;            /* If true, the player has set a name. */
	uint32              update_los          : 1;                /* If true, update_los() in draw(), and clear */
	uint32              combat_mode         : 1;            /* if true, player is in combat mode, attacking with weapon */
	uint32              rest_mode           : 1;            /* if true, player is going "resting" - resting mode will be interrupted when player moves or get hit */
	uint32              rest_sitting        : 1;            /* if true, player is sitting - sitting + rest mode = regeneration */

	/* some dm flags */
    uint32              dm_invis            : 1;    /* 1= is invisible, see invisible */
	uint32              dm_stealth          : 1;            /* 1= no "XX enter the game" and no entry in /who */
	uint32              dm_removed_from_map : 1;    /* internal dm flag: player was removed from a map */

	uint32              personal_light      : 3;   /* must be enough bits to hold MAX_DARKNESS */
	uint32              known_spell         : 1;   /* True if you know the spell of the wand */
	uint32              last_known_spell    : 1;   /* What was last updated with draw_stats() */
	uint32              update_skills       : 1;   /* update skill list when set */

	uint32              silent_login        : 1;

    /* Instance system */
    long                instance_id;            /* instance_id is unique per server restart */
    int                 instance_num;           /* instance_num is unique per instance */
    uint32              instance_flags;         /* status info of the instance */

    int                 map_update_cmd;         /* for server->client protocol */
    int                 map_update_tile;        /* for server->client protocol */
    struct mapdef      *last_update;            /* when moving on tiled maps, player can change
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
    int                 base_skill_group_exp[3];        /* % adjustment for exp gain */
    object             *guild_force;
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


	int			        state;				/* player system state... PLAYING, LOGIN IN... */
    uint32              anim_enemy_count;   /* hm, this can be kicked now - i do it for a quick hack to
                                             * implement the animations. use is_melee_range() instead.
                                             * for the client: skill/weapon values for the distance weapon
                                             * - calculated in fix_player() */
	int                 dist_dps;
	int                 dist_last_dps;
	int                 dist_wc;
	int                 dist_last_wc;
	int                 dist_action_time;
	int                 dist_last_action_time;

	int					carrying_last;			/* determinate we have to rebuild speed */

	int                 dam_bonus;				/* damage bonus from equipment (additional to the weapons) */
	int					wc_bonus;
    int                 dps;                    /* damge per second value from fix_player() for client and info */
    int                 last_dps;
    int                 target_hp;              /* for the client target HP marker - special shadow*/
    int                 set_skill_weapon;       /* skill number of used weapon skill for fast access */
    int                 set_skill_archery;      /* same for archery */
    int                 bed_x;                      /* x,y - coordinates of respawn (savebed) */
    int                 bed_y;
    int                 map_x;                      /* x,y - coordinates of login/start map */
    int                 map_y;
    uint32              golem_count;                /* Which golem is controlled - the id count */

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
    uint32              action_timer;

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
    sint32              skill_exp[NROFSKILLS];      /* shadow register for updating skill values to client */
    uint32              target_object_count;        /* count of target - NOTE: if we target ourself, this count it 0
                                                    * this will kick us out of enemy target routines - all functions
                                                    * who handle self target know it and use only target_object -
                                                    * for our own player object the pointer will never change for us.
                                                    */
    uint32              target_map_pos;             /* last target search position */
    uint32              mode;                       /* Mode of player for pickup. */
    sint32              group_id;                   /* unique group id number - this is a unique number like the object count */
    sint32              group_status;               /* status of invite or group */
    int                 exp_bonus;                  /* extra exp bonus */
    int                 weapon_sp;                  /* weapon speed - float *1000 for the client */
    int                 map_tile_x, map_tile_y;     /* these is our last position of map we send to client */
    int                 map_off_x, map_off_y;       /* scroll offset between 2 maps of client update */

	int                 speed_enc_base;				/* calculated in fix_player to recalc speed with changed weight */
	int					speed_enc_limit;
	int					speed_reduce_from_disease;
	int                 speed_enc;                  /* fix_player(): % of the weight & armour encumbrance effecting speed. */
    int                 last_speed_enc;             /* last speed_enc value send to client */

    int                 spell_fumble;               /* fix_player(): chance to fumble a spell (armour effects or others) */
    int                 last_spell_fumble;          /* last value send to client */
    uint32              weight_limit;               /* real weight limit from fix_player(): weight + str stats add */
    /* we don't need here the count of group object links- because the game will explicit
     * link/unlink party members when their player object change.
     * exception is group leader - its only used to confirm a invite
     */
    object             *group_leader;               /* pointer to group leader or invite */
    uint32              group_leader_count;         /* for invite... */
    object             *group_prev;                 /* previous member of group */
    object             *group_next;                 /* next member of group */

    uint32              update_ticker;              /* global_round tick where player was updated */
    float               speed;                      /* shadow speed value, set in fix_player() to cover flag effects */

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

    char                target_hp_p;        /* for the client target HP real % value*/

    int                 gen_hp;                     /* Bonuses to regeneration speed of hp in % */
    int                 gen_sp;                     /* Bonuses to regeneration speed of sp in % */
    int                 gen_grace;                  /* Bonuses to regeneration speed of grace in % */
    int                 last_gen_hp;                /* used for client update */
    int                 last_gen_sp;
    int                 last_gen_grace;

    int                 reg_hp_num;                 /* thats how much every reg tick we get in points */
    int                 reg_sp_num;
    int                 reg_grace_num;
    int                 reg_timer;                  /* used to call regeneration functions */
    int                 damage_timer;               /* hp recovery timer for last hp */
    int                 resting_reg_timer;          /* when resting this timer manages the pre waiting time */
    int                 normal_reg_timer;           /* we handle normal regeneration all x seconds */

    int                 food_status;                /* show regeneration status to client */
    int                 last_food_status;

    sint16              map_status;                     /* type of map we have saved */

    uint8               bed_status;
    uint8               group_mode;                    /* group mode use GROUP_MODE_XX */
    uint8               group_nrof;                    /* number of players in group */
    uint8               group_nr;                    /* player is #group_nr in his group - used for fast update */


    int                 last_weapon_sp;
    int                 p_ver;


    /* for smaller map sizes, only the the first elements are used (ie, upper left) */
    int                 blocked_los[MAP_CLIENT_X][MAP_CLIENT_Y]; /* in fact we only need char size, but use int for faster access */

    char                levhp[MAXLEVEL + 1];            /* What the player gained on that level */
    char                levsp[MAXLEVEL + 1];
    char                levgrace[MAXLEVEL + 1];
    sint8               last_resist[NROFATTACKS];   /* shadow register for client update resist table */

	char                quick_name[BIG_NAME*3];     /* thats rank + name +" the xxxx" */
	char                ext_title[MAX_EXT_TITLE];   /* for client: <Rank> <Name>\n<Gender> <Race> <Profession> */

	usekeytype          usekeys;          /* Method for finding keys for doors */

    uint16              last_flags;         /* fire/run on flags for last tick */
    uint32              count;              /* Any numbers typed before a command */

    unsigned char       listening; /* Which priority will be used in info_all */

    unsigned char       run_on;
    uint32              last_weight_limit;  /* Last weight limit transmitted to client */
    living              orig_stats;       /* Can be less in case of poisoning */
    living              last_stats;       /* Last stats drawn with draw_stats() */
    long                last_weight;
    unsigned char       last_level;		/* client data: level player */


#ifdef AUTOSAVE
    uint32              last_save_tick;
#endif

#ifdef USE_CHANNELS
    struct player_channel  *channels;      /*channels player is 'on' */
    int                 channels_on;    /*temp disable all channels */
    unsigned int            channel_count;  /*channel count */
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
