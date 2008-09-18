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

    The author can be reached via e-mail to info@daimonin.net
*/

#if !defined(__PLAYER_H)
#define __PLAYER_H

/* imported from server - attack.h file */
typedef enum _attacks
{
    /* We start with the double used attacks - for resist & protection too */
    /* damage type: physical */
    ATNR_PHYSICAL, /* = impact */
    ATNR_SLASH,
    ATNR_CLEAVE,
    ATNR_PIERCE,

    /* damage type: elemental */
    ATNR_FIRE,
    ATNR_COLD,
    ATNR_ELECTRICITY,
    ATNR_POISON,
    ATNR_ACID,
    ATNR_SONIC,

    /* damage type: magical */
    ATNR_FORCE,
    ATNR_PSIONIC,
    ATNR_LIGHT,
    ATNR_SHADOW,
    ATNR_LIFESTEAL,

    /* damage type: sphere */
    ATNR_AETHER,
    ATNR_NETHER,
    ATNR_CHAOS,
    ATNR_DEATH,

    /* damage: type only effect by invulnerable */
    ATNR_WEAPONMAGIC,
    ATNR_GODPOWER,

    /* at this point attack effects starts - only resist maps to it */
    ATNR_DRAIN,
    ATNR_DEPLETION,
    ATNR_CORRUPTION,
    ATNR_COUNTERMAGIC,
    ATNR_CANCELLATION,
    ATNR_CONFUSION,
    ATNR_FEAR,
    ATNR_SLOW,
    ATNR_PARALYZE,
    ATNR_SNARE,

    /* and the real special one here */
    ATNR_INTERNAL,
    NROFATTACKS /* index (= 32 ATM) */
}_attacks;

typedef struct Stat_struct
{
    int   Str, Dex, Con, Wis, Cha, Int, Pow;
    int   wc, ac;     /* Weapon Class and Armour Class */
    int   level;
    int  hp;         /* Hit Points. */
    int  maxhp;
    int  temphp;
    uint32 hptick;
    int  sp;         /* Spell points.  Used to cast spells. */
    int  maxsp;      /* Max spell points. */
    int  tempsp;
    uint32 sptick;
    int  grace;      /* Spell points.  Used to cast spells. */
    int  maxgrace;       /* Max spell points. */
    int  tempgrace;
    uint32 gracetick;
    int  exp_level;
    int  exp;            /* Experience */
    int  food;       /* food and resting heartbeat*/
    int  dam;            /* How much damage this object does when hitting */
    int  dist_dam;
    int  dist_wc;
    float   speed;       /* speed encumbrance in %.1f format */
    float   spell_fumble;  /* base chance to fumble a spell */
    float   weapon_sp;      /* Gets converted to a float for display */
    float   dist_time;      /* Gets converted to a float for display */
    float   dps;        /* thats damage / 10 */
    float   dist_dps;        /* thats damage / 10 */
    uint16  flags;      /* contains fire on/run on flags */
    sint8  protection[NROFATTACKS];     /* Resistant values */
    uint32  protection_change   : 1; /* Resistant value has changed */
    sint16  skill_level[MAX_SKILL];  /* Level and experience totals for */
    sint32  skill_exp[MAX_SKILL];    /* skills */
}
Stats;

typedef enum _inventory_win
{
    IWIN_BELOW,
    IWIN_INV
}   _inventory_win;

typedef struct Account_Struct
{
    int     count; /* number of chars we have created */
    int     selected; /* we have which character selected? */
    char    name[ACCOUNT_MAX_PLAYER][MAX_PLAYER_NAME+1]; /* List of account chars */
    int     level[ACCOUNT_MAX_PLAYER];
    int     race[ACCOUNT_MAX_PLAYER];
} Account;

extern Account account;

typedef struct Player_Struct
{
    item                   *ob;     /* Player object */
    item                   *below;      /* Items below the player (pl.below->inv) */
    item                   *sack;       /* inventory of a open container */

    item                   *container;  /* ptr to open container */
    sint32                  container_tag;  /* tag of the container */
    item                   *ranges[range_size]; /* Object that is used for that */


    sint32                  weight_limit;
    uint32                  count;      /* Repeat count on command */
    int                     target_mode;
    int                     target_code;
    int                     target_color;
    int                     inventory_win;  /* inventory windows */
    int                     menustatus;
    int      mark_count;
    int                     loc;
    int                     tag;
    int                     nrof;
    int                     skill_g;            /* skill group and entry of ready skill */
    int                     skill_e;

    int                     warn_hp;

    int                     win_inv_slot;
    int                     win_inv_tag;
    int                     win_quick_tag;
    int                     win_pdoll_tag;
    int                     win_inv_start;
    int                     win_inv_count;
    int                     win_inv_ctag;

    int                     win_below_slot;
    int                     win_below_tag;
    int                     win_below_start;
    int                     win_below_count;
    int                     win_below_ctag;

    int                     input_mode; /* mode: no, console(textstring), numinput */
    int                     nummode;

    float                   gen_hp;         /* hp, mana and grace reg. */
    float                   gen_sp;
    float                   gen_grace;

    uint32                  no_echo             : 1;    /* If TRUE, don't echo keystrokes */
    uint32                  fire_on             : 1;    /* True if fire key is pressed = action key (ALT;CTRL)*/
    uint32                  run_on              : 1;    /* True if run key is on = action key (ALT;CTRL)*/
    uint32                  resize_twin         : 1;
    uint32                  resize_twin_marker  : 1;
    uint32                  firekey_on          : 1;    /* True if fire key is pressed = permanent mode*/
    uint32                  runkey_on           : 1;    /* sic! */
    uint32                  echo_bindings       : 1;/* If true, echo the command that the key */

    sint32                   window_weight;
    sint32                   real_weight;


    uint16                  count_left; /* count for commands */
    uint16                  mmapx, mmapy;   /* size of magic map */
    uint16                  pmapx, pmapy;   /* Where the player is on the magic map */
    uint16                  mapxres, mapyres;/* resolution to draw on the magic map */

    Boolean                 warn_statdown;
    Boolean                 warn_statup;
    Boolean                 warn_drain;
    Stats                   stats;      /* Player stats */
    rangetype               shoottype;  /* What type of range attack player has */

    uint8                  *magicmap;   /* Magic map data */
    uint8                   showmagic;  /* If 0, show normal map, otherwise, show
                                        * magic map.
                                        */
    uint8                   command_window; /* How many outstanding commands to allow */
    uint8                   ready_spell;    /* Index to spell that is readied */
    /* player knows */
    uint8                   map_x, map_y;   /* These are offset values.  See object.c */


    char                    target_hp;  /* hp of our target in % */
    /* i cant see where we need that */
    //    char                    last_command[MAX_BUF];  /* Last command entered */
    char                    input_text[MAX_BUF];    /* keys typed (for long commands) */
    char                    acc_name[MAX_ACCOUNT_NAME+1];   /* account name + '\0' */
    char                    name[MAX_PLAYER_NAME+1];        /* name of char which is selected to play */
    char                    password[MAX_ACCOUNT_PASSWORD+1];  /* account password */
    char                    spells[255][40];    /* List of all the spells the */
    char                    target_name[MAX_BUF];   /* Rank & Name of char*/
    char                    num_text[300];
    char                    skill_name[128];
    char                    rankandname[MAX_BUF];
    char                    pname[MAX_BUF]; /* Name of char*/
    char                    title[MAX_BUF]; /* Race & Profession of character */
    char                    rank[MAX_BUF];  /* rank */
    char                    race[MAX_BUF];  /* alignment */
    char                    godname[MAX_BUF];   /* alignment */
    char                    alignment[MAX_BUF]; /* alignment */
    char                    gender[MAX_BUF];    /* Gender */
    char                    range[MAX_BUF]; /* Range attack chosen */
    char     player_reply[64];
}
Client_Player;

extern Client_Player    cpl;        /* Player object. */

typedef struct _server_level
{
    int    level;
    uint32  exp[500];
}
_server_level;

extern _server_level    server_level;

extern void     new_player(uint32 tag, char *name, uint32 weight, short face);
extern void     show_help(void);
extern void     extended_command(const char *ocommand);
extern char    *complete_command(char *command);
void            init_player_data(void);
void            widget_show_player_doll(int x, int y);
void            widget_show_player_doll_event(int x, int y, int MEvent);
void            widget_player_data_event(int x, int y);
void            widget_show_player_data(int x, int y);
void            set_weight_limit(uint32 wlim);
void            clear_player(void);

void        widget_player_stats(int x, int y);
void        widget_show_main_lvl(int x, int y);
void        widget_show_skill_exp(int x, int y);
void        widget_show_regeneration(int x, int y);
void        widget_skillgroups(int x, int y);
void        widget_menubuttons(int x, int y);
void        widget_menubuttons_event(int x, int y, int MEvent);
void        widget_skill_exp_event(int x, int y, int MEvent);

void        widget_show_statometer(int x, int y);

#endif
