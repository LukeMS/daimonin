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

typedef struct _level_color {
	int grey;
	int green;
	int blue;
	int yellow;
	int orange;
	int red;
	int purple;
}_level_color;

extern _level_color level_color[];

enum { /* fire modes submited from client */
    FIRE_MODE_NONE=-1,
        FIRE_MODE_BOW,
        FIRE_MODE_SPELL,
        FIRE_MODE_WAND,
        FIRE_MODE_SKILL,
        FIRE_MODE_THROW,
        FIRE_MODE_SUMMON
};

typedef enum rangetype {
  range_bottom = -1, range_none = 0, range_bow = 1, range_magic = 2,
  range_wand = 3, range_rod = 4, range_scroll = 5, range_horn = 6,
  range_skill = 7,range_potion =8, range_dust = 9,
  range_size = 10
} rangetype;


typedef enum usekeytype {
    key_inventory=0,
    keyrings=1,
    containers=2,
} usekeytype;

/* used for item damage system */
enum {
		PLAYER_EQUIP_MAIL,
		PLAYER_EQUIP_GAUNTLET,
        PLAYER_EQUIP_BRACER,
		PLAYER_EQUIP_HELM,
		PLAYER_EQUIP_BOOTS,
        PLAYER_EQUIP_CLOAK,
        PLAYER_EQUIP_GIRDLE,
		PLAYER_EQUIP_SHIELD,
        PLAYER_EQUIP_RRING, /* this must 1 entry before LRING! */
        PLAYER_EQUIP_LRING,
        PLAYER_EQUIP_AMULET,
		PLAYER_EQUIP_WEAPON1,
        PLAYER_EQUIP_BOW,
		PLAYER_EQUIP_MAX 
		/* last index */
};

/* we can include more flags here... */
  
#define PLAYER_AFLAG_NO 0
#define PLAYER_AFLAG_FIGHT 1	/* if this flag is set, show player fight animation */
#define PLAYER_AFLAG_ENEMY 2	/* if this flag is set at END of a animation, 
								 * set fight flag and clear this flag. It is set in hit_player()
                                 * when the player swings to an enemy 
                                 */
#define PLAYER_AFLAG_ADDFRAME 4	/* intern */

#ifdef WIN32
#pragma pack(push,1)
#endif

/* slowly reworking this struct - some old values in - MT2003 */
typedef struct pl_player
{
	/* this is not cleared with memset - seek for offsetof((....,maplevel) */
	struct pl_player *next;				/* Pointer to next player, NULL if this is last */
	NewSocket socket;				/* Socket information for this player */

	/* all this is set to 0 with memset */
	char maplevel[MAX_BUF];			/* Name of the map the player is on */
	struct mapdef *last_update;		/* when moving on tiled maps, player can change 
									 * map without triggering mapevents and new_map_cmd.
									 * This will break client cache and script events.
									 * This value will used as control value.*/

	object *ob;						/* The object representing the player */
	object *golem;					/* Which golem is controlled */
	object *anim_enemy;				/* which enemy we hack. needed to skip extra swing animation */

    object *selected_weapon;   /* thats the weapon in our hand */
	object *skill_weapon;   /* thats the hth skill we use when we not use a weapon (like karate) */
	object *target_object;			/* our target */
	object *sp_exp_ptr;		/* exp object pointer to sp (mana) defining exp object */
	object *grace_exp_ptr;	/* exp object pointer to grace (mana) defining exp object */
	object *equipment[PLAYER_EQUIP_MAX]; /* pointers to applied items in the players inventory */
	object *skill_ptr[NROFSKILLS];	/* quick jump table to skill objects in the players inv. */
	object *last_skill_ob[MAX_EXP_CAT];  /* the exp object table */
	object *mark;		/* marked object */
	object *age_force;	/* quick jump to our age force */

	object *map_below;	/* ptr used from local map player chain */
	object *map_above;
	object *container;			/* Current container being used. */
	uint32 container_count;		/* the count of the container */
	object *container_above;	/* that points to a PLAYER ob, accessing this container too! 
	                             * if this is NULL, we are the "last" one looking in ->container.
	                             */
	object *container_below;    /* same as above - if this is NULl, we are "last" looking the container */


	uint32 anim_enemy_count;	/* hm, this can be kicked now - i do it for a quick hack to
								 * implement the animations. use is_melee_range() instead.
								 */

	int target_hp;				/* for the client target HP marker - special shadow*/
	int set_skill_weapon;		/* skill number of used weapon skill for fast access */
	int set_skill_archery;		/* same for archery */
	int bed_x;						/* x,y - coordinates of respawn (savebed) */
	int bed_y;
	uint32 golem_count;				/* Which golem is controlled - the id count */

	int firemode_type;        /* firemode_xxx are set from command_fire() */
	int firemode_tag1;
	int firemode_tag2;

	uint32	mark_count;				/* count or mark object */
	sint32 last_skill_exp[MAX_EXP_CAT]; /* shadow register. if != exp. obj update client */
	sint32 skill_exp[NROFSKILLS];	 /* shadow register for updating skill values to client */
	uint32 target_object_count;     /* count of target - NOTE: if we target ourself, this count it 0
	                                 * this will kick us out of enemy target routines - all functions
	                                 * who handle self target know it and use only target_object - 
	                                 * for our own player object the pointer will never change for us.
	                                 */
	uint32 target_map_pos;			/* last target search position */
	uint32 mode;					/* Mode of player for pickup. */

	float last_speed;

	sint16 client_dam; /* condition adjusted damage send to client */
    sint16 age;		/* the age of our player */
    sint16 age_add;	/* unnatural changes to our age - can be removed by restoration */
    sint16 age_changes; /* permanent changes .... very bad (or good when younger) */
    sint16 age_max;	/* the age of our player */
	sint16 skill_level[NROFSKILLS];	/* shadow register for updint skill levels to client */
	sint16 encumbrance;  /*  How much our player is encumbered  */
	uint16 anim_flags;				/* some anim flags for special player animation handling */

	uint16 nrofknownspells; 	/* Index in the above array */
	sint16 known_spells[NROFREALSPELLS]; /* Spells known by the player */
	
	char target_hp_p;				/* for the client target HP real % value*/
	char weapon_sp;					/* weapon speed index (mainly used for client) */

    signed char digestion;			/* Any bonuses/penalties to digestion */
	signed char gen_sp_armour;		/* Penalty to sp regen from armour */

	signed char gen_hp;				/* Bonuses to regeneration speed of hp */
	signed char gen_sp;				/* Bonuses to regeneration speed of sp */
	signed char gen_grace;			/* Bonuses to regeneration speed of grace */

	int reg_hp_num;					/* thats how much every reg tick we get */
	int reg_sp_num;
	int reg_grace_num;

	sint16 base_hp_reg;				/* our real tick counter for hp regenerations */
	sint16 base_sp_reg;				/* our real tick counter for sp regenerations */
	sint16 base_grace_reg;			/* our real tick counter for grace regenerations */

	/* send to client - shadow & prepared gen_xx values */
	uint16 gen_client_hp;				/* Bonuses to regeneration speed of hp */
	uint16 gen_client_sp;				/* Bonuses to regeneration speed of sp */
	uint16 gen_client_grace;			/* Bonuses to regeneration speed of grace */
	uint16 last_gen_hp;
	uint16 last_gen_sp;
	uint16 last_gen_grace;

	char last_weapon_sp;

	char last_skill_level[MAX_EXP_CAT]; /* shadow register client exp group for level */
	char last_skill_id[MAX_EXP_CAT]; /* Thats the CS_STATS_ id for client STATS cmd*/
	char firemode_name[BIG_NAME*2];
	char quick_name[BIG_NAME*3];	/* thats rank + name +" the xxxx" */
	char savebed_map[MAX_BUF];  /* map where player will respawn after death */
	/* for smaller map sizes, only the the first elements are used (ie, upper left) */
	int blocked_los[MAP_CLIENT_X][MAP_CLIENT_Y]; /* in fact we only need char size, but use int for faster access */
	char ext_title[MAX_EXT_TITLE];	/* for client: <Rank> <Name>\n<Gender> <Race> <Profession> */
	char levhp[MAXLEVEL+1];			/* What the player gained on that level */
	char levsp[MAXLEVEL+1];
	char levgrace[MAXLEVEL+1];
	sint8 last_protection[NROFPROTECTIONS];	/* shadow register for client update resistance table */

	uint32 name_changed:1;			/* If true, the player has set a name. */
	uint32 update_los:1;				/* If true, update_los() in draw(), and clear */
	uint32 combat_mode:1;		    /* if true, player is in combat mode, attacking with weapon */
	uint32 praying:1;				/* if true, player is praying and gaining fast grace */
	uint32 was_praying:1;			/* internal used by praying to send pray msg to player */

	/* some dm flags */
	uint32 dm_stealth:1;			/* 1= no "XX enter the game" and no entry in /who */
	uint32 dm_light:1;				/* 1= all maps are shown in daylight for the dm */ 
	uint32 dm_removed_from_map:1;	/* internal dm flag: player was removed from a map */

/* all values before this line are tested and proofed */

  
  uint32 known_spell:1;     /* True if you know the spell of the wand */
  uint32 last_known_spell:1;/* What was last updated with draw_stats() */
  uint32 update_skills:1;   /* update skill list when set */
#ifdef EXPLORE_MODE
  uint32 explore:1;         /* if True, player is in explore mode */
#endif

  rangetype shoottype;	      /* Which range-attack is being used by player */
  rangetype last_shoot;	      /* What was last updated with draw_stats() */
  usekeytype usekeys;	      /* Method for finding keys for doors */
  sint16 chosen_spell;		/* Type of readied spell */
  sint16 chosen_item_spell;	/* Type of spell that the item fires */
  uint16 last_flags;	    /* fire/run on flags for last tick */
  uint32 count;       /* Any numbers typed before a command */

  int  last_skill_index;	       /* this is init from init_player_exp() */

  unsigned char state;
  unsigned char listening; /* Which priority will be used in info_all */

  unsigned char fire_on;
  unsigned char run_on;
  unsigned char idle;      /* How long this player has been idle */
  uint32  last_weight_limit;	/* Last weight limit transmitted to client */
  living orig_stats;       /* Can be less in case of poisoning */
  living last_stats;       /* Last stats drawn with draw_stats() */
  signed long last_value;  /* Same usage as last_stats */
  long last_weight;

  
  char title[BIG_NAME]; /* we use ext. title now - we should remove this now! */

  unsigned char last_level;

  char killer[BIG_NAME];  /* Who killed this player. */
  char last_cmd;
  char last_tell[MAX_NAME];   /* last player that told you something [mids 01/14/2002] */
							  /* this is a typcial client part - no need to use the server
							   * to store or handle this! */

  char write_buf[MAX_BUF];
  char input_buf[MAX_BUF];
  char password[16]; /* 2 (seed) + 11 (crypted) + 1 (EOS) + 2 (safety) = 16 */
#ifdef SAVE_INTERVAL
  time_t last_save_time;
#endif /* SAVE_INTERVAL */
#ifdef AUTOSAVE
  long last_save_tick;
#endif
  sint16    party_number;
  sint16    party_number_to_join; /* used when player wants to join a party
				 but we will have to get password first
				 so we have to remember which party to
				 join */
#ifdef SEARCH_ITEMS
  char search_str[MAX_BUF];
#endif /* SEARCH_ITEMS */

  /* i disabled this now - search for last_used in the code.
   * perhaps we need this in the future.
   */
  
  /*object *last_used;*/     /* Pointer to object last picked or applied */
  /*long last_used_id;*/     /* Safety measures to be sure it's the same */
} player;


/* not really the player, but tied pretty closely */  
typedef struct party_struct {
  sint16 partyid;
  const char * partyleader;
  char passwd[9];
  struct party_struct *next;
  char *partyname;
#ifdef PARTY_KILL_LOG
  struct party_kill
  {
    char killer[MAX_NAME+1],dead[MAX_NAME+1];
    uint32 exp;
  } party_kills[PARTY_KILL_LOG];
#endif
  uint32 total_exp,kills;
} partylist;



#ifdef WIN32
#pragma pack(pop)
#endif
