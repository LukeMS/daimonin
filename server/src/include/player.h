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
#define NUM_OUTPUT_BUFS	5
typedef struct {
  char *buf;			/* Actual string pointer */
  uint32 first_update;		/* First time this message was stored  */
  uint16 count;			/* How many times we got this message */
} Output_Buf;

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



enum {
		PLAYER_EQUIP_WEAPON1,
		PLAYER_EQUIP_SHIELD,
		PLAYER_EQUIP_HEAD,
		PLAYER_EQUIP_MAIL,
		PLAYER_EQUIP_BOOTS,
		PLAYER_EQUIP_GAUNTLET,
        PLAYER_EQUIP_BRACER,
        PLAYER_EQUIP_ROBE,
        PLAYER_EQUIP_GIRDLE,
        PLAYER_EQUIP_BOW,
        PLAYER_EQUIP_AMULET,
        PLAYER_EQUIP_WAND,
        PLAYER_EQUIP_RRING,
        PLAYER_EQUIP_LRING,
        PLAYER_EQUIP_SKILL,
        PLAYER_EQUIP_LIGHT,
		PLAYER_EQUIP_MAX, /* last index */
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
typedef struct pl 
{
	struct pl *next;				/* Pointer to next player, NULL if this is last */
	object *ob;						/* The object representing the player */
	int bed_x;						/* x,y - coordinates of respawn (savebed) */
	int bed_y;
	object *golem;					/* Which golem is controlled */
	uint32 golem_count;				/* Which golem is controlled - the id count */
	object *anim_enemy;				/* which enemy we hack. needed to skip extra swing animation */
	uint32 anim_enemy_count;		/* hm, this can be kicked now - i do it for a quick hack to
									 * implement the animations. use is_melee_range() instead.
									 */

	object *target_object;			/* our target */
	uint32 target_object_count;     /* count of target - NOTE: if we target ourself, this count it 0
	                                 * this will kick us out of enemy target routines - all functions
	                                 * who handle self target know it and use only target_object - 
	                                 * for our own player object the pointer will never change for us.
	                                 */
	uint32 target_map_pos;			/* last target search position */

	uint32 name_changed:1;			/* If true, the player has set a name. */
	uint32 do_los:1;				/* If true, update_los() in draw(), and clear */
	uint32 combat_mode:1;		    /* if true, player is in combat mode, attacking with weapon */
	uint32 praying:1;				/* if true, player is praying and gaining fast grace */
	uint32 was_praying:1;			/* internal used by praying to send pray msg to player */

	struct mapdef *last_update;		/* when moving on tiled maps, player can change 
									 * map without triggering mapevents and new_map_cmd.
									 * This will break client cache and script events.
									 * This value will used as control value.*/

	NewSocket socket;				/* Socket information for this player */

	uint16 anim_flags;				/* some anim flags for special player animation handling */

	char maplevel[MAX_BUF];			/* Name of the map the player is on */
	char quick_name[BIG_NAME*3];	/* thats rank + name +" the xxxx" */

/* all values before this line are tested and proofed */

	uint32 mode;					/* Mode of player for pickup. */
  
  char savebed_map[MAX_BUF];  /* map where player will respawn after death */
  int removed;                /* Flag telling if ob is to be inserted */
  sint16 known_spells[NROFREALSPELLS]; /* Spells known by the player */
  uint16 nrofknownspells; 	/* Index in the above array */

  unsigned known_spell:1;     /* True if you know the spell of the wand */
  unsigned last_known_spell:1;/* What was last updated with draw_stats() */
  uint32 tmp_invis:1;       /* Will invis go away when we attack ? */
  uint32 update_skills:1;   /* update skill list when set */

  rangetype shoottype;	      /* Which range-attack is being used by player */
  rangetype last_shoot;	      /* What was last updated with draw_stats() */
  usekeytype usekeys;	      /* Method for finding keys for doors */
  sint16 chosen_spell;		/* Type of readied spell */
  sint16 last_spell;		/* What spell draw_stats() last displayed */
  sint16 chosen_item_spell;	/* Type of spell that the item fires */
  uint32 count;       /* Any numbers typed before a command */
  unsigned char prev_cmd;     /* Previous command executed */
  unsigned char prev_fire_on;
  unsigned char prev_keycode; /* Previous command executed */
  unsigned char key_down;     /* Last move-key still held down */
  signed char digestion;	/* Any bonuses/penalties to digestion */
  signed char gen_hp;		/* Bonuses to regeneration speed of hp */
  signed char gen_sp;		/* Bonuses to regeneration speed of sp */
  signed char gen_sp_armour;	/* Penalty to sp regen from armour */
  signed char gen_grace;	/* Bonuses to regeneration speed of grace */
#ifdef EXPLORE_MODE
  uint32 explore:1;         /* if True, player is in explore mode */
#endif
  int maxhp_malus;			/* every level, the player gets x hp from xmax hp. the diff accumulate here */
  int maxsp_malus;			/* same of sp */
  int maxgrace_malus;		/* same for grace */

  int  last_skill_index;	       /* this is init from init_player_exp() */
  object *last_skill_ob[MAX_EXP_CAT];  /* the exp object */
  sint32  last_skill_exp[MAX_EXP_CAT]; /* shadow register. if != exp. obj update client */
  char  last_skill_level[MAX_EXP_CAT]; /* same for level */
  char  last_skill_id[MAX_EXP_CAT]; /* Thats the CS_STATS_ id for client STATS cmd*/

  object *equipment[PLAYER_EQUIP_MAX];
  unsigned char state;
  unsigned char listening; /* Which priority will be used in info_all */

  unsigned char fire_on;
  char firemode_name[64];
  int firemode_type;        /* firemode_xxx are set from command_fire() */
  int firemode_tag1;
  int firemode_tag2;
  unsigned char run_on;
  unsigned char idle;      /* How long this player has been idle */
  unsigned char has_hit;   /* If set, weapon_sp instead of speed will count */
  float weapon_sp;         /* Penalties to speed when fighting w speed >ws/10*/
  float last_weapon_sp;    /* Last turn */
  uint16 last_flags;	    /* fire/run on flags for last tick */
  sint32  last_weight_limit;	/* Last weight limit transmitted to client */
  living orig_stats;       /* Can be less in case of poisoning */
  living last_stats;       /* Last stats drawn with draw_stats() */
  float last_speed;
  signed long last_value;  /* Same usage as last_stats */
  long last_weight;
  sint16	last_protection[NROFPROTECTIONS];	 /* Resistance against attacks (this goes now to players) */
  object *skill_ptr[NROFSKILLS];
  sint32 skill_exp[NROFSKILLS];          /* shadow register for updating skill values to client */
  signed short skill_level[NROFSKILLS];
  int set_skill_weapon;
  int set_skill_archery;
  object *selected_weapon;   /* thats the weapon in our hand */
  object *skill_weapon;   /* thats the hth skill we use when we not use a weapon (like karate) */
#ifdef USE_SWAP_STATS
  int Swap_First;
#endif

  
  /* i disabled this now - search for last_used in the code.
   * perhaps we need this in the future.
   */
  
  /*object *last_used;*/     /* Pointer to object last picked or applied */
  /*long last_used_id;*/     /* Safety measures to be sure it's the same */


  /* for smaller map sizes, only the the first elements are used (ie, upper left) */
  sint8	    blocked_los[MAP_CLIENT_X][MAP_CLIENT_Y];
  
  char title[BIG_NAME];

  sint16	age;		/* the age of our player */
  sint16	age_add;	/* unnatural changes to our age - can be removed by restoration */
  sint16	age_changes; /* permanent changes .... very bad (or good when younger) */
  sint16	age_max;	/* the age of our player */
  object   *age_force;	/* quick jump to our age force */

  char ext_title[MAX_EXT_TITLE]; /* for client: <Rank> <Name>\n<Gender> <Race> <Profession> */


  signed char levhp[11]; /* What the player gained on that level */
  signed char levsp[11];
  signed char levgrace[11];
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

  short encumbrance;  /*  How much our player is encumbered  */
/*
  This is part of a somewhat complex output buffer system - i really was not
  able to see the sense or the real use in the short time i watch it. It seems
  to me a outdated part of crossfire, partly disabled and ignored by the advanced
  output functions. I disabled it - lets watch was happens. MT-11-2002
  Output_Buf	outputs[NUM_OUTPUT_BUFS];
  uint16	outputs_sync;
  uint16	outputs_count;
  */
  object	*mark;		/* marked object */
  uint32	mark_count;	/* count or mark object */
} player;


/* not really the player, but tied pretty closely */  
typedef struct party_struct {
  sint16 partyid;
  char * partyleader;
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
