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
#include <funcpoint.h>

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

_races item_race_table[RACE_NAME_INIT] = {
    "",             RN_DEFAULT,  /* default value - human like */
        "dwarven ",     RN_DWARVEN,
        "elven ",       RN_ELVEN,
        "gnomish ",     RN_GNOMISH,
        "drow ",        RN_DROW,
        "orcish ",      RN_ORCISH,
        "goblin ",      RN_GOBLIN,
        "kobold ",      RN_KOBOLD,  /* count also as tiny, but "unclean" */
        "giant ",       RN_GIANT,  /* all demihumans "bigger as humans" */
        "tiny ",        RN_TINY,  /* different small race (sprites, ...) */
        "demonish ",    RN_DEMONISH,  /* this is usable from all sizes */
        "draconish ",   RN_DRACONISH,   /* usable from all sizes */
        "ogre ",		RN_GIANT     /* count as giant */
};

/* when we carry more as this of our weight_limit, we get encumbered. */
#define ENCUMBRANCE_LIMIT 65.0f

static int con_bonus[MAX_STAT + 1]={
  -6,-5,-4,-3,-2,-1,-1,0,0,0,0,1,2,3,4,5,6,7,8,9,10,12,14,16,18,20,
  22,25,30,40,50
};
/* changed the name of this to "sp_bonus" from "int_bonus" 
 * because Pow can now be the stat that controls spellpoint
 * advancement. -b.t.
 */
static int sp_bonus[MAX_STAT + 1]={
  -10,-10,-9,-8,-7,-6,-5,-4,-3,-2,-1,0,1,2,3,4,5,6,7,8,9,10,12,15,20,25,
  30,40,50,70,100
};

static int grace_bonus[MAX_STAT +1] = {
    -10,-10,-9,-8,-7,-6,-5,-4,-3,-2,-1,0,1,2,3,4,5,6,7,8,9,10,12,15,20,25,
  30,40,50,70,100
};

/* new charisma system. As base value, you got in a shop 20% of the real
 * of a item. With this system, you can go up to 25% maximal or 5% minimum.
 * For buying, they are used reversed. you can buy for 95% as best or
 * 115% as worst.
 * This system use heavy user based economic - for player <-> NPC, we will
 * suffer badly value & payment differences.
 */
float cha_bonus[MAX_STAT + 1]={
	-0.15f, /* cha = 0 */
	-0.10f, -0.08f,	-0.05f, 	-0.03f,	-0.02f, /* 1-5*/
	-0.01f, -0.005f,	-0.003f, 	0.001f,	0.0f, /* 6-10*/
	0.0f, 0.0f,	0.0f, 	0.0f,	0.0f, /* 11-15*/
	0.003f, 0.005f,	0.009f, 	0.01f,	0.012f, /* 16-20*/
	0.014f, 0.016f,	0.019f, 	0.021f,	0.023f, /* 21-25*/
	0.025f, 0.03f,	0.035f, 	0.04f,	0.05f  /* 25-30*/
};


int dex_bonus[MAX_STAT + 1]={
  -4,-3,-2,-2,-1,-1,-1,0,0,0,0,0,0,0,1,1,1,2,2,2,3,3,3,4,4,4,5,5,6,6,7
};

/* speed_bonus uses dex as its stat */
float speed_bonus[MAX_STAT + 1]={
  -0.4f, -0.4f, -0.3f, -0.3f, -0.2f, 
  -0.2f, -0.2f, -0.1f, -0.1f, -0.1f,
  -0.05f, 0.0, 0.0f, 0.0f, 0.05f, 0.1f,
  0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f,
  0.7f, 0.8f, 0.9f, 1.0f,
  1.1f, 1.2f, 1.3f, 1.4f, 1.5f
};

/* dam_bonus, thaco_bonus, weight limit all are based on
 * strength.
 */
int dam_bonus[MAX_STAT + 1]={
  -2,-2,-2,-1,-1,-1,0,0,0,0,0,0,1,1,1,2,2,2,3,3,3,4,4,5,5,6,7,8,9,10,12
};

/* for throwing/range items, dex is used to add this to wc */
int thaco_bonus[MAX_STAT + 1]={
  -2,-2,-1,-1,0,0,0,0,0,0,0,0,0,0,1,1,1,2,2,2,3,3,3,4,4,5,5,6,7,8,10
};

/* weight_limit - the absolute most a character can carry - a character can't
 * pick stuff up if it would put him above this limit.
 * value is in grams, so we don't need to do conversion later
 * These limits are probably overly generous, but being there were no values
 * before, you need to start someplace.
 */

sint32 weight_limit[MAX_STAT+ 1] = {
    20000,  /* 0 */
    25000,30000,35000,40000,50000,	    /* 5*/
    60000,70000,80000,90000,100000,    /* 10 */
    110000,120000,130000,140000,150000,/* 15 */
    165000,180000,195000,210000,225000,/* 20 */
    240000,255000,270000,285000,300000, /* 25 */
    325000,350000,375000,400000,450000  /*30 */ 
};

int learn_spell[MAX_STAT + 1]={
  0,0,0,1,2,4,8,12,16,25,36,45,55,65,70,75,80,85,90,95,100,100,100,100,100,
  100,100,100,100,100,100
};
int cleric_chance[MAX_STAT + 1]={
  100,100,100,100,90,80,70,60,50,40,35,30,25,20,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,0
};
int turn_bonus[MAX_STAT + 1]={
  -1,-1,-1,-1,-1,-1,-1,-1,0,0,0,1,1,1,2,2,2,3,3,3,4,4,5,5,6,7,8,9,10,12,15
};
int fear_bonus[MAX_STAT + 1]={
  3,3,3,3,2,2,2,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

/*
   Since this is nowhere defined ...
   Both come in handy at least in function add_exp()
*/

#define MAX_EXPERIENCE  new_levels[MAXLEVEL]

#define MAX_EXP_IN_OBJ new_levels[MAXLEVEL]/(MAX_EXP_CAT - 1) 

float lev_damage[MAXLEVEL+1]= {
	1.0f,
	1.0f, 1.25f, 1.5f, 1.75f, 2.0f, 2.25f, 2.5f, 2.75f, 3.0f, 3.25f,
	3.5f, 3.75f, 4.0f, 4.25f, 4.5f, 4.75f, 5.0f, 5.25f, 5.5f, 5.75f,
	6.0f, 6.25f, 6.5f, 6.75f, 7.0f, 7.25f, 7.5f, 7.75f, 8.0f, 8.25f,
	8.5f, 8.75f, 9.0f, 9.25f, 9.5f, 9.75f, 10.0f, 10.25f, 10.5f, 10.75f,
	11.0f, 11.25f, 11.5f, 11.75f, 12.0f, 12.25f, 12.5f, 12.75f, 13.0f, 13.25f, /* 50*/
	13.5f, 13.75f, 14.0f, 14.25f, 14.5f, 14.75f, 15.0f, 15.25f, 15.5f, 15.75f,
	16.0f, 16.25f, 16.5f, 16.75f, 17.0f, 17.25f, 17.5f, 17.75f, 18.0f, 18.25f,
	18.5f, 18.75f, 19.0f, 19.25f, 19.5f, 19.75f, 20.0f, 20.25f, 20.5f, 20.75f,
	21.0f, 21.25f, 21.5f, 21.75f, 22.0f, 22.25f, 22.5f, 22.75f, 23.0f, 23.25f,
	23.5f, 23.75f, 24.0f, 24.25f, 24.5f, 24.75f, 25.0f, 25.25f, 25.5f, 25.75f,
	26.0f, 26.25f, 26.5f, 26.75f, 27.0f, 27.25f, 27.5f, 27.75f, 28.0f, 28.25f
};


/* Max level is 100.  By making it 101, it means values 0->100 are valid.
 * Thus, we can use op->level directly, and it also works for level 0 people.
 */
int savethrow[MAXLEVEL+1]={
  18,
  18,17,16,15,14,14,13,13,12,12,12,11,11,11,11,10,10,10,10, 9,
   9, 9, 9, 9, 8, 8, 8, 8, 8, 8, 7, 7, 7, 7, 7, 7, 7, 6, 6, 6,
   6, 6, 6, 6, 6, 5, 5, 5, 5, 5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4,
   4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2,
   2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

char *attacks[NROFATTACKS] = {
  "hit", "magical", "fire", "electricity", "cold", "confusion",
  "acid", "drain", "weaponmagic", "ghosthit", "poison", "slow",
  "paralyze", "turn undead", "fear", "cancellation", "depletion", "death",
  "chaos","counterspell","god power","holy power","blinding", "",
  "life stealing", "slash", "cleave", "pierce"
};

static char *drain_msg[7] = {
  "Oh no! You are weakened!",
  "You're feeling clumsy!",
  "You feel less healthy",
  "You suddenly begin to lose your memory!",
  "Your face gets distorted!",
  "Watch out, your mind is going!", 
  "Your spirit feels drained!"
};
char *restore_msg[7] = {
  "You feel your strength return.",
  "You feel your agility return.",
  "You feel your health return.",
  "You feel your wisdom return.",
  "You feel your charisma return.",
  "You feel your memory return.", 
  "You feel your spirits return."
};
char *gain_msg[7] = {
	"You feel stronger.",
	"You feel more agile.",
	"You feel healthy.",
	"You feel wiser.",
	"You seem to look better.",
	"You feel smarter.", 
	"You feel more potent."
};
char *lose_msg[7] = {
	"You feel weaker!",
	"You feel clumsy!",
	"You feel less healthy!",
	"You lose some of your memory!",
	"You look ugly!",
	"You feel stupid!", 
	"You feel less potent!"
};

char *statname[7] = {
  "strength", "dexterity", "constitution", "wisdom", "charisma", "intelligence","power" 
};

char *short_stat_name[7] = {
  "Str", "Dex", "Con", "Wis", "Cha", "Int","Pow" 
};

/*
 * sets Str/Dex/con/Wis/Cha/Int/Pow in stats to value, depending on
 * what attr is (STR to POW).
 */

void
set_attr_value(living *stats,int attr,signed char value) {
  switch(attr) {
  case STR:
    stats->Str=value;
    break;
  case DEX:
    stats->Dex=value;
    break;
  case CON:
    stats->Con=value;
    break;
  case WIS:
    stats->Wis=value;
    break;
  case POW:
    stats->Pow=value;
    break;
  case CHA:
    stats->Cha=value;
    break;
  case INT:
    stats->Int=value;
    break;
  }
}

/*
 * Like set_attr_value(), but instead the value (which can be negative)
 * is added to the specified stat.
 */

void
change_attr_value(living *stats,int attr,signed char value) {
  if (value==0) return;
  switch(attr) {
  case STR:
    stats->Str+=value;
    break;
  case DEX:
    stats->Dex+=value;
    break;
  case CON:
    stats->Con+=value;
    break;
  case WIS:
    stats->Wis+=value;
    break;
  case POW:
    stats->Pow+=value;
    break;
  case CHA:
    stats->Cha+=value;
    break;
  case INT:
    stats->Int+=value;
    break;
  default:
	LOG(llevBug,"BUG: Invalid attribute in change_attr_value: %d\n", attr);
  }
}

/*
 * returns the specified stat.  See also set_attr_value().
 */

signed char
get_attr_value(living *stats,int attr) {
  switch(attr) {
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
  case INT:
    return(stats->Int);
  case POW:
    return(stats->Pow);
  }
  return 0;
}

/*
 * Ensures that all stats (str/dex/con/wis/cha/int) are within the
 * 1-30 stat limit.
 */

void check_stat_bounds(living *stats) {
  int i,v;
  for(i=0;i<7;i++)
    if((v=get_attr_value(stats,i))>MAX_STAT)
      set_attr_value(stats,i,MAX_STAT);
    else if(v<MIN_STAT)
      set_attr_value(stats,i,MIN_STAT);
}

#define ORIG_S(xyz,abc)	(op->contr->orig_stats.abc)

/* return 1 if we sucessfully changed a stat, 0 if nothing was changed. */
/* flag is set to 1 if we are applying the object, -1 if we are removing
 * the object.
 * It is the calling functions responsibilty to check to see if the object
 * can be applied or not.
 */
int change_abil(object *op, object *tmp) {
  int flag=QUERY_FLAG(tmp,FLAG_APPLIED)?1:-1,i,j,success=0;
  object refop;
  char message[MAX_BUF];
  int potion_max=0;
  
  /* remember what object was like before it was changed.  note that
   * refop is a local copy of op only to be used for detecting changes
   * found by fix_player.  refop is not a real object */
  memcpy(&refop, op, sizeof(object));

  if(op->type==PLAYER) {
    if (tmp->type==POTION) {
      for(j=0;j<7;j++) {
        i = get_attr_value(&(op->contr->orig_stats),j);

	/* Check to see if stats are within limits such that this can be
	 * applied.
	 */
        if (((i+flag*get_attr_value(&(tmp->stats),j))<=
	    (20+tmp->stats.sp + get_attr_value(&(op->arch->clone.stats),j)))
	    && i>0)
	{
            change_attr_value(&(op->contr->orig_stats),j,
                      (signed char)(flag*get_attr_value(&(tmp->stats),j)));
	    tmp->stats.sp=0;/* Fix it up for super potions */
	}
	else {
	  /* potion is useless - player has already hit the natural maximum */
	  potion_max = 1;
	}
      }
    /* This section of code ups the characters normal stats also.  I am not
     * sure if this is strictly necessary, being that fix_player probably
     * recalculates this anyway.
     */
    for(j=0;j<7;j++)
      change_attr_value(&(op->stats),j,(signed char) (flag*get_attr_value(&(tmp->stats),j)));
    check_stat_bounds(&(op->stats));
    } /* end of potion handling code */
  }

  /* reset attributes that fix_player doesn't reset since it doesn't search
   * everything to set */
  if(flag == -1)
    op->path_attuned&=~tmp->path_attuned,
    op->path_repelled&=~tmp->path_repelled,
    op->path_denied&=~tmp->path_denied;

  /* call fix_player since op object could have whatever attribute due
   * to multiple items.  if fix_player always has to be called after
   * change_ability then might as well call it from here
   */
  fix_player(op);

  if(tmp->attack[ATNR_CONFUSION]) {
    success=1;
    if(flag>0)
      (*draw_info_func)(NDI_UNIQUE|NDI_WHITE, 0, op,"Your hands begin to glow red.");
    else
      (*draw_info_func)(NDI_UNIQUE|NDI_GREY, 0, op,"Your hands stop glowing red.");
  }
  if ( QUERY_FLAG(op,FLAG_LIFESAVE) != QUERY_FLAG(&refop,FLAG_LIFESAVE)){
    success=1;
    if(flag>0) {
      (*draw_info_func)(NDI_UNIQUE|NDI_WHITE, 0, op,"You feel very protected.");
    } else {
      (*draw_info_func)(NDI_UNIQUE|NDI_GREY, 0, op,"You don't feel protected anymore.");
    }
  }
  if ( QUERY_FLAG(op,FLAG_REFL_MISSILE) != QUERY_FLAG(&refop,FLAG_REFL_MISSILE)){
    success=1;
    if(flag>0) {
      (*draw_info_func)(NDI_UNIQUE|NDI_WHITE, 0, op,"A magic force shimmers around you.");
    } else {
      (*draw_info_func)(NDI_UNIQUE|NDI_GREY, 0, op,"The magic force fades away.");
    }
  }
  if ( QUERY_FLAG(op,FLAG_REFL_SPELL) != QUERY_FLAG(&refop,FLAG_REFL_SPELL)){
    success=1;
    if(flag>0) {
      (*draw_info_func)(NDI_UNIQUE|NDI_WHITE, 0, op,"You feel more safe now, somehow.");
    } else {
      (*draw_info_func)(NDI_UNIQUE|NDI_GREY, 0, op,"Suddenly you feel less safe, somehow.");
    }
  }
  if(QUERY_FLAG(tmp,FLAG_FLYING)) {
    if(flag>0) {
      success=1;
      /* if were already flying then now flying higher */
      if ( QUERY_FLAG(op,FLAG_FLYING) == QUERY_FLAG(&refop,FLAG_FLYING))
        (*draw_info_func)(NDI_UNIQUE|NDI_WHITE, 0, op,"You float a little higher in the air.");
      else {
        (*draw_info_func)(NDI_UNIQUE|NDI_GREY, 0, op,"You start to float in the air!.");
        SET_FLAG(op,FLAG_FLYING);
        if(op->speed>1)
          op->speed=1;
      }
    } else {
      success=1;
      /* if were already flying then now flying lower */
      if ( QUERY_FLAG(op,FLAG_FLYING) == QUERY_FLAG(&refop,FLAG_FLYING))
        (*draw_info_func)(NDI_UNIQUE|NDI_WHITE, 0, op,"You float a little lower in the air.");
      else {
        (*draw_info_func)(NDI_UNIQUE|NDI_GREY, 0, op,"You float down to the ground.");
	check_walk_on (op, op);
      }
    }
  }

  /* becoming UNDEAD... a special treatment for this flag. Only those not
   * originally undead may change their status */ 
  if(!QUERY_FLAG(&op->arch->clone,FLAG_UNDEAD)) 
    if ( QUERY_FLAG(op,FLAG_UNDEAD) != QUERY_FLAG(&refop,FLAG_UNDEAD)) {
      success=1;
      if(flag>0) {
        if(op->race) free_string(op->race); 
	op->race=add_string("undead");
        (*draw_info_func)(NDI_UNIQUE|NDI_GREY, 0, op,"Your lifeforce drains away!");
      } else {
        if(op->race) free_string(op->race); 
        if(op->arch->clone.race) 
	   op->race=add_string(op->arch->clone.race);  
        (*draw_info_func)(NDI_UNIQUE|NDI_WHITE, 0, op,"Your lifeforce returns!");
      }
    }

  if ( QUERY_FLAG(op,FLAG_STEALTH) != QUERY_FLAG(&refop,FLAG_STEALTH)){
    success=1;
    if(flag>0) {
      (*draw_info_func)(NDI_UNIQUE|NDI_WHITE, 0, op,"You walk more quietly.");
    } else {
      (*draw_info_func)(NDI_UNIQUE|NDI_GREY, 0, op,"You walk more noisily.");
    }
  }
  if ( QUERY_FLAG(op,FLAG_SEE_INVISIBLE) != QUERY_FLAG(&refop,FLAG_SEE_INVISIBLE)){
    success=1;
    if(flag>0) {
      (*draw_info_func)(NDI_UNIQUE|NDI_WHITE, 0, op,"You see invisible things.");
    } else {
      (*draw_info_func)(NDI_UNIQUE|NDI_GREY, 0, op,"Your vision becomes less clear.");
    }
  }
  if ( QUERY_FLAG(op,FLAG_IS_INVISIBLE) != QUERY_FLAG(&refop,FLAG_IS_INVISIBLE)){
    success=1;
    if(flag>0) {
      (*draw_info_func)(NDI_UNIQUE|NDI_WHITE, 0, op,"You become transparent.");
    } else {
      (*draw_info_func)(NDI_UNIQUE|NDI_GREY, 0, op,"You can see yourself.");
    }
  }
  /* blinded you can tell if more blinded since blinded player has minimal
   * vision */
  if(QUERY_FLAG(tmp,FLAG_BLIND)) {
    success=1;
    if(flag>0) {
      if(QUERY_FLAG(op,FLAG_WIZ))
        (*draw_info_func)(NDI_UNIQUE|NDI_GREY, 0, op,"Your mortal self is blinded.");
      else { 
        (*draw_info_func)(NDI_UNIQUE|NDI_GREY, 0, op,"You are blinded.");
        SET_FLAG(op,FLAG_BLIND);
        if(op->type==PLAYER)
          op->contr->do_los=1;
      }  
    } else {
      if(QUERY_FLAG(op,FLAG_WIZ))
        (*draw_info_func)(NDI_UNIQUE|NDI_WHITE, 0, op,"Your mortal self can now see again.");
      else {
        (*draw_info_func)(NDI_UNIQUE|NDI_WHITE, 0, op,"Your vision returns.");
        CLEAR_FLAG(op,FLAG_BLIND);
        if(op->type==PLAYER)
          op->contr->do_los=1;
      }  
    }  
  }

  if ( QUERY_FLAG(op,FLAG_SEE_IN_DARK) != QUERY_FLAG(&refop,FLAG_SEE_IN_DARK)){
    success=1;
    if(flag>0) {
        (*draw_info_func)(NDI_UNIQUE|NDI_WHITE, 0, op,"Your vision is better in the dark.");
        if(op->type==PLAYER)
          op->contr->do_los=1;
    } else {
        (*draw_info_func)(NDI_UNIQUE|NDI_GREY, 0, op,"You see less well in the dark.");
        if(op->type==PLAYER)
          op->contr->do_los=1;
    }  
  }  

  if ( QUERY_FLAG(op,FLAG_XRAYS) != QUERY_FLAG(&refop,FLAG_XRAYS)){
    success=1;
    if(flag>0) {
      if(QUERY_FLAG(op,FLAG_WIZ))
        (*draw_info_func)(NDI_UNIQUE|NDI_WHITE, 0, op,"Your vision becomes a little clearer.");
      else {
        (*draw_info_func)(NDI_UNIQUE|NDI_GREY, 0, op,"Everything becomes transparent.");
        if(op->type==PLAYER)
          op->contr->do_los=1;
      }
    } else {
      if(QUERY_FLAG(op,FLAG_WIZ))
        (*draw_info_func)(NDI_UNIQUE|NDI_WHITE, 0, op,"Your vision becomes a bit out of focus.");
      else {
        (*draw_info_func)(NDI_UNIQUE|NDI_GREY, 0, op,"Everything suddenly looks very solid.");
        if(op->type==PLAYER)
          op->contr->do_los=1;
      }
    }
  }
  if(tmp->stats.hp && op->type==PLAYER) {
    success=1;
    if(flag*tmp->stats.hp>0)
      (*draw_info_func)(NDI_UNIQUE|NDI_WHITE, 0, op,"You feel much more healthy!");
    else
      (*draw_info_func)(NDI_UNIQUE|NDI_GREY, 0, op,"You feel much less healthy!");
  }
  if(tmp->stats.sp && op->type==PLAYER && tmp->type!=SKILL) {
    success=1;
    if(flag*tmp->stats.sp>0)
      (*draw_info_func)(NDI_UNIQUE|NDI_WHITE, 0, op,"You feel one with the powers of magic!");
    else
      (*draw_info_func)(NDI_UNIQUE|NDI_GREY, 0, op,"You suddenly feel very mundane.");
  }
  /* for the future when artifacts set this -b.t. */
  if(tmp->stats.grace && op->type==PLAYER) {
    success=1;
     if(flag*tmp->stats.grace>0)
      (*draw_info_func)(NDI_UNIQUE|NDI_WHITE, 0, op,"You feel closer to your deity!");
    else 
      (*draw_info_func)(NDI_UNIQUE|NDI_GREY, 0, op,"You suddenly feel less holy.");
  } 
  if(tmp->stats.food && op->type==PLAYER && tmp->type != POISONING) {
    success=1;
    if(tmp->stats.food*flag>0)
      (*draw_info_func)(NDI_UNIQUE|NDI_WHITE, 0, op,"You feel your digestion slowing down.");
    else
      (*draw_info_func)(NDI_UNIQUE|NDI_GREY, 0, op,"You feel your digestion speeding up.");
  }

  /* Messages for changed resistance */
  for (i=0; i<NROFATTACKS; i++) {    
    if (op->resist[i] != refop.resist[i]) {
      success=1;
      if (op->resist[i] > refop.resist[i])
	  {
		sprintf(message, "Your resistance to %s rises to %d%%.",
	       change_resist_msg[i], op->resist[i]);
	   (*draw_info_func)(NDI_UNIQUE|NDI_GREEN, 0, op, message);
	  }
      else
	  {
		sprintf(message, "Your resistance to %s drops to %d%%.",
	       change_resist_msg[i], op->resist[i]);
		(*draw_info_func)(NDI_UNIQUE|NDI_BLUE, 0, op, message);
      }
    }
  }
  /* Messages for changed resistance */
  for (i=0; i<NROFPROTECTIONS; i++) {    
    if (op->protection[i] != refop.protection[i]) {
      success=1;
      if (op->protection[i] > refop.protection[i])
	  {
		sprintf(message, "Your protection to %s rises to %d%%.",
	       protection_name[i], op->protection[i]);
		(*draw_info_func)(NDI_UNIQUE|NDI_GREEN, 0, op, message);
	  }
      else
	  {
		sprintf(message, "Your protection to %s drops to %d%%.",
	       protection_name[i], op->protection[i]);
		(*draw_info_func)(NDI_UNIQUE|NDI_BLUE, 0, op, message);
	  }
    }
  }

     if(tmp->type!=EXPERIENCE && !potion_max) {
	for (j=0; j<7; j++) {
	    if ((i=get_attr_value(&(tmp->stats),j))!=0) {
		success=1;
		if (i * flag > 0)
		    (*draw_info_func)(NDI_UNIQUE|NDI_WHITE, 0, op, gain_msg[j]);
		else
		    (*draw_info_func)(NDI_UNIQUE|NDI_GREY, 0, op, lose_msg[j]);
	    }
	}
    }
  if(tmp->stats.luck) {
    success=1;
    if(flag>0) {
      (*draw_info_func)(NDI_UNIQUE|NDI_GREY, 0, op,"You feel less lucky.");
    } else {
      (*draw_info_func)(NDI_UNIQUE|NDI_WHITE, 0, op,"You feel more lucky.");
    }
  }
    return success;
}

/*
 * Stat draining by Vick 930307
 * (Feeling evil, I made it work as well now.  -Frank 8)
 */

void drain_stat(object *op) {
  drain_specific_stat(op, RANDOM()%7);
}

void drain_specific_stat(object *op, int deplete_stats) {
  object *tmp;
  archetype *at;

  at = find_archetype("depletion");
  if (!at) {
    LOG(llevBug, "BUG: Couldn't find archetype depletion.\n");
    return;
  } else {
    tmp = present_arch_in_ob(at, op);
    if (!tmp) {
      tmp = arch_to_object(at);
      tmp = insert_ob_in_ob(tmp, op);
      SET_FLAG(tmp,FLAG_APPLIED);
    }
  }

  (*draw_info_func)(NDI_UNIQUE, 0, op, drain_msg[deplete_stats]);
  change_attr_value(&tmp->stats, deplete_stats, -1);
  fix_player(op);
}

/*
 * A value of 0 indicates timeout, otherwise change the luck of the object.
 * via an applied bad_luck object.
 */

void change_luck(object *op, int value) {
  object *tmp;
  archetype *at;
  at = find_archetype("luck");
  if (!at)
    LOG(llevBug, "BUG: Couldn't find archetype luck.\n");
  else {
    tmp = present_arch_in_ob(at, op);
    if (!tmp) {
      if (!value)
        return;
      tmp = arch_to_object(at);
      tmp = insert_ob_in_ob(tmp, op);
      SET_FLAG(tmp,FLAG_APPLIED);
    }
    if (value) {
      op->stats.luck+=value;
      tmp->stats.luck+=value;
    } else {
      if (!tmp->stats.luck) {
        LOG(llevDebug, "Internal error in change_luck().\n");
        return;
      }
      /* Randomly change the players luck.  Basically, we move it
       * back neutral (if greater>0, subtract, otherwise add)
       * I believe this is supposed to be > and not >= - this means
       * if your luck is -1/1, it won't get adjusted - only when your
       * luck is worse can you hope for improvment.
       * note that if we adjusted it with it is -1/1, that check above
       * for 0 luck will happen, resulting in error.
       */
      if (RANDOM()%(FABS(tmp->stats.luck)) > RANDOM()%30)
        tmp->stats.luck += tmp->stats.luck>0?-1:1;
    }
  }
}

/*
 * Subtracts stat-bonuses given by the class which the player has chosen.
 */

void remove_statbonus(object *op) {
  op->stats.Str -= op->arch->clone.stats.Str;
  op->stats.Dex -= op->arch->clone.stats.Dex;
  op->stats.Con -= op->arch->clone.stats.Con;
  op->stats.Wis -= op->arch->clone.stats.Wis;
  op->stats.Pow -= op->arch->clone.stats.Pow;
  op->stats.Cha -= op->arch->clone.stats.Cha;
  op->stats.Int -= op->arch->clone.stats.Int;
  op->contr->orig_stats.Str -= op->arch->clone.stats.Str;
  op->contr->orig_stats.Dex -= op->arch->clone.stats.Dex;
  op->contr->orig_stats.Con -= op->arch->clone.stats.Con;
  op->contr->orig_stats.Wis -= op->arch->clone.stats.Wis;
  op->contr->orig_stats.Pow -= op->arch->clone.stats.Pow;
  op->contr->orig_stats.Cha -= op->arch->clone.stats.Cha;
  op->contr->orig_stats.Int -= op->arch->clone.stats.Int;
}

/*
 * Adds stat-bonuses given by the class which the player has chosen.
 */

void add_statbonus(object *op) {
  op->stats.Str += op->arch->clone.stats.Str;
  op->stats.Dex += op->arch->clone.stats.Dex;
  op->stats.Con += op->arch->clone.stats.Con;
  op->stats.Wis += op->arch->clone.stats.Wis;
  op->stats.Pow += op->arch->clone.stats.Pow;
  op->stats.Cha += op->arch->clone.stats.Cha;
  op->stats.Int += op->arch->clone.stats.Int;
  op->contr->orig_stats.Str += op->arch->clone.stats.Str;
  op->contr->orig_stats.Dex += op->arch->clone.stats.Dex;
  op->contr->orig_stats.Con += op->arch->clone.stats.Con;
  op->contr->orig_stats.Wis += op->arch->clone.stats.Wis;
  op->contr->orig_stats.Pow += op->arch->clone.stats.Pow;
  op->contr->orig_stats.Cha += op->arch->clone.stats.Cha;
  op->contr->orig_stats.Int += op->arch->clone.stats.Int;
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
void fix_player(object *op) 
{
	int i,j, inv_flag,inv_see_flag, light,weapon_weight, best_wc, best_ac, wc, ac;
	int resists[NROFATTACKS], vuln[NROFATTACKS], potion_resist[NROFATTACKS];
	int protect_boni[NROFPROTECTIONS];
	object *grace_obj=NULL,*mana_obj=NULL,*hp_obj=NULL,*wc_obj=NULL,*tmp,
		*skill_weapon=NULL;
	float f,max=9,added_speed=0,bonus_speed=0,speed_reduce_from_disease=1;

	
	if(QUERY_FLAG(op,FLAG_NO_FIX_PLAYER))
	{
		LOG(llevDebug,"fix_player(): called for object %s with FLAG_NO_FIX_PLAYER set\n", op->name);
		return;
	}
	/* ok, in crossfire, fix_player is called for objects not for players
	 * we redirect mobs to fix_monster() and let only player pass
	 */
	if(QUERY_FLAG(op,FLAG_MONSTER) && op->type != PLAYER)
	{
		fix_monster(op);
		return;
	}

	/* for secure */
	if(op->type != PLAYER)
	{
		LOG(llevDebug,"fix_player(): called from non Player/Mob object: %s (type %d)\n", op->name, op->type);
		return;
	}

	inv_flag=inv_see_flag=light=weapon_weight=best_wc=best_ac=wc=ac=0;

    for(i=0;i<7;i++) 
      set_attr_value(&(op->stats),i,get_attr_value(&(op->contr->orig_stats),i));


	op->contr->selected_weapon =op->contr->skill_weapon = NULL;
	op->contr->digestion = 0;
	op->contr->gen_hp = 0;
	op->contr->gen_sp = 0;
	op->contr->gen_grace = 0;
	op->contr->gen_sp_armour = 10;
    op->contr->set_skill_weapon = NO_SKILL_READY;	/* the used skills for fast access */
    op->contr->set_skill_archery = NO_SKILL_READY; 

    op->contr->encumbrance=0;

	/* for players, we adjust with the values */	
	ac=op->arch->clone.stats.ac;
	wc=op->arch->clone.stats.wc;
	op->stats.wc = wc;
	op->stats.ac = ac;
	op->stats.dam=op->arch->clone.stats.dam;

	op->stats.maxhp=op->arch->clone.stats.maxhp;
	op->stats.maxsp=op->arch->clone.stats.maxsp;
	op->stats.maxgrace=op->arch->clone.stats.maxgrace;
	
	op->stats.luck=op->arch->clone.stats.luck;
	op->speed = op->arch->clone.speed;
	op->weapon_speed = op->arch->clone.weapon_speed;
	op->path_attuned=op->arch->clone.path_attuned;
	op->path_repelled=op->arch->clone.path_repelled;
	op->path_denied=op->arch->clone.path_denied;
	op->terrain_flag = op->arch->clone.terrain_flag;		/* reset terrain moving abilities */

    /* only adjust skills which has no own level/exp values */
    if(op->chosen_skill&&!op->chosen_skill->last_eat&&op->chosen_skill->exp_obj)
       op->chosen_skill->level=op->chosen_skill->exp_obj->level;

	if(op->slaying!=NULL) 
	{
		free_string(op->slaying);
		op->slaying=NULL;
	}
	if (QUERY_FLAG (op, FLAG_IS_INVISIBLE))
		inv_flag = 1;
	if (QUERY_FLAG (op, FLAG_SEE_INVISIBLE))
		inv_see_flag = 1;
	if ( ! QUERY_FLAG (&op->arch->clone, FLAG_XRAYS))
		CLEAR_FLAG(op, FLAG_XRAYS);
	if ( ! QUERY_FLAG (&op->arch->clone, FLAG_CAN_PASS_THRU))
		CLEAR_FLAG(op, FLAG_CAN_PASS_THRU);
	if ( ! QUERY_FLAG (&op->arch->clone, FLAG_IS_ETHEREAL))
		CLEAR_FLAG(op, FLAG_IS_ETHEREAL);
	if ( ! QUERY_FLAG (&op->arch->clone, FLAG_IS_INVISIBLE))
		CLEAR_FLAG(op, FLAG_IS_INVISIBLE);
	if ( ! QUERY_FLAG (&op->arch->clone, FLAG_SEE_INVISIBLE))
		CLEAR_FLAG(op, FLAG_SEE_INVISIBLE);
	if ( ! QUERY_FLAG (&op->arch->clone, FLAG_LIFESAVE))
		CLEAR_FLAG(op,FLAG_LIFESAVE);
	if ( ! QUERY_FLAG (&op->arch->clone, FLAG_STEALTH))
		CLEAR_FLAG(op,FLAG_STEALTH);
	if ( ! QUERY_FLAG (&op->arch->clone, FLAG_BLIND))
		CLEAR_FLAG(op,FLAG_BLIND);
	if ( ! QUERY_FLAG (&op->arch->clone, FLAG_FLYING))
		CLEAR_FLAG(op, FLAG_FLYING);
	if ( ! QUERY_FLAG (&op->arch->clone, FLAG_REFL_SPELL))
		CLEAR_FLAG(op,FLAG_REFL_SPELL);
	if ( ! QUERY_FLAG (&op->arch->clone, FLAG_REFL_MISSILE))
		CLEAR_FLAG(op,FLAG_REFL_MISSILE);
	if(!QUERY_FLAG(&op->arch->clone,FLAG_UNDEAD))
		CLEAR_FLAG(op,FLAG_UNDEAD);
	if ( ! QUERY_FLAG (&op->arch->clone, FLAG_SEE_IN_DARK))
		CLEAR_FLAG(op,FLAG_SEE_IN_DARK);

	/* initializing resistances from the values in player archetype clone:  */
	memcpy(&op->resist, &op->arch->clone.resist, sizeof(op->resist));
	for (i=0;i<NROFATTACKS;i++) 
	{
		if (op->resist[i] > 0)
		{
			resists[i]= op->resist[i];
			vuln[i]=0;
		}
		else
		{
			vuln[i]= -(op->resist[i]);
			resists[i]=0;
		}
		op->attack[i] = 0;
		potion_resist[i]=0;
	}

    /* for players which cannot use armour, they gain AC -1 per 3 levels,
     * plus a small amount of physical resist, those poor suckers. ;) */
	/* I think we should REALLY overwork this ... with the new protection/resist
	 * system this kind of player chars will be doomed. MT.
	 */
	if(!QUERY_FLAG(op,FLAG_USE_ARMOUR)) 
	{
		ac=MAX(-10,ac - op->level/3);
		resists[ATNR_PHYSICAL] += ((100-resists[AT_PHYSICAL])*(80*op->level/MAXLEVEL))/100;
		resists[ATNR_SLASH] += ((100-resists[AT_SLASH])*(80*op->level/MAXLEVEL))/100;
		resists[ATNR_PIERCE] += ((100-resists[AT_PIERCE])*(80*op->level/MAXLEVEL))/100;
		resists[ATNR_CLEAVE] += ((100-resists[AT_CLEAVE])*(80*op->level/MAXLEVEL))/100;
	}

	/* iam not sure about "natural" protection of players - i disabled it for now */
	memset(&op->protection,0, sizeof(op->protection));  
	memset(&protect_boni,0, sizeof(protect_boni));  
  
	/* ok, now we browse the inventory... there is not only our equipment - there
	 * are all our skills, forces and hidden system objects.
	 */
	for(tmp=op->inv;tmp!=NULL;tmp=tmp->below) 
	{
		/* this is needed, because our applied light can overruled by a light giving
		 * object like holy glowing aura force or something 
	     */
		if (tmp->glow_radius > light)
		{
			/* don't use this item when it is a 'not applied TYPE_LIGHT_APPLY' */
			if(tmp->type != TYPE_LIGHT_APPLY || QUERY_FLAG(tmp,FLAG_APPLIED))
				light=tmp->glow_radius;
		}

		/* all skills, not only the applied ones */
		if(tmp->type == SKILL)
		{
			/* lets remember the best bare hand skill */
			if(tmp->stats.dam>0)
			{
				if(!skill_weapon || skill_weapon->stats.dam < tmp->stats.dam)
					skill_weapon = tmp;
			}
			op->contr->skill_ptr[tmp->stats.sp] = tmp; /* safe in table for quick access */
		}
		else if(tmp->type == TYPE_AGE_FORCE)
		{
			op->contr->age_force = tmp; /* store our age force */
			op->contr->age = tmp->stats.hp;
			op->contr->age_max = tmp->stats.maxhp;
			op->contr->age_add = tmp->stats.sp;
			op->contr->age_changes = tmp->stats.grace;
			if(op->contr->age >= (sint16)(((float)op->contr->age_max/100.0f)*60.0f) )
				SET_FLAG(op, FLAG_IS_AGED);
			else
				CLEAR_FLAG(op, FLAG_IS_AGED);
		}

		/* this checks all applied, not container like items in the inventory */
		if(QUERY_FLAG(tmp,FLAG_APPLIED) && tmp->type!=CONTAINER && tmp->type!=CLOSE_CON)
		{
			/* The meaning of stats in skill or experience objects is different -
			 * we use them solely to link skills to experience, thus it is 
			 * inappropriate to allow these applied objects to change stats.
             * An exception is exp_wis, containing info about god-properties!
			 */
			if(tmp->type!=EXPERIENCE || (tmp->type==EXPERIENCE && tmp->stats.Wis))
			{
				if (tmp->type != POTION)
				{
					for(i=0;i<7;i++)
						change_attr_value(&(op->stats),i,get_attr_value(&(tmp->stats),i));
				}

				/* these are the items that currently can change digestion, regeneration,
				 * spell point recovery and mana point recovery.  Seems sort of an arbitary
				 * list, but other items store other info into stats array. */
				if ( (tmp->type == EXPERIENCE)  || (tmp->type == WEAPON)
							|| (tmp->type == ARMOUR)   || (tmp->type == HELMET)
							|| (tmp->type == SHIELD)   || (tmp->type == RING)
							|| (tmp->type == BOOTS)    || (tmp->type == GLOVES)
							|| (tmp->type == AMULET )  || (tmp->type == GIRDLE)
							|| (tmp->type == BRACERS ) || (tmp->type == CLOAK) 
							|| (tmp->type == DISEASE)  || (tmp->type == FORCE))
				{
					op->contr->digestion     += tmp->stats.food;
					op->contr->gen_hp        += tmp->stats.hp;
					op->contr->gen_sp        += tmp->stats.sp;
					op->contr->gen_grace     += tmp->stats.grace;

					/* Experience objects use last_heal for permanent exp, so don't add those in.*/
					if (tmp->type != EXPERIENCE)
						op->contr->gen_sp_armour += tmp->last_heal;
				}
			}

			if(tmp->type==SYMPTOM)
			{
				speed_reduce_from_disease = (float) tmp->last_sp / 100.0f;
				if(speed_reduce_from_disease ==0.0f)
						speed_reduce_from_disease = 1.0f;
			}

			/* Pos. and neg. protections are counted seperate (-> pro/vuln).
			 * (Negative protections are calculated extactly like positive.)
			 * Resistance from potions are treated special as well. If there's
			 * more than one potion-effect, the bigger prot.-value is taken.
			 */
			if (tmp->type != POTION)
			{
				for (i=0; i<NROFPROTECTIONS; i++) 
				{
					if (tmp->protection[i] > 0) 
						protect_boni[i] += ((100-protect_boni[i])*tmp->protection[i])/100;
				}
              
				for (i=0; i<NROFATTACKS; i++)
				{
					/* hm, stat potions which gives resist? perm? */
					if (tmp->arch->clone.type == FORCE &&
									tmp->resist[i] && tmp->type==POTION_EFFECT)
					{
						if (potion_resist[i])
							potion_resist[i] = MAX(potion_resist[i], tmp->resist[i]);
						else
							potion_resist[i] = tmp->resist[i];
					}
					else 
					{
						if (tmp->resist[i] > 0) 
							resists[i] += ((100-resists[i])*tmp->resist[i])/100;
						else if (tmp->resist[i] < 0)
							vuln[i] += ((100-vuln[i])*(-tmp->resist[i]))/100;

						if (tmp->type!=BOW && tmp->type != SYMPTOM && tmp->type != POISONING) 
						{
							if (tmp->attack[i] > 0)
							{
								op->attack[i] += tmp->attack[i];
								if(op->attack[i]>100)
									op->attack[i]=100;
							}
						}
					}
				}
			}

			op->terrain_flag |= tmp->terrain_type;    /* we just add a given terrain */
			op->path_attuned|=tmp->path_attuned;
			op->path_repelled|=tmp->path_repelled;
			op->path_denied|=tmp->path_denied;
			op->stats.luck+=tmp->stats.luck;
			if(QUERY_FLAG(tmp,FLAG_LIFESAVE))
				SET_FLAG(op,FLAG_LIFESAVE);
			if(QUERY_FLAG(tmp,FLAG_REFL_SPELL))
				SET_FLAG(op,FLAG_REFL_SPELL);
			if(QUERY_FLAG(tmp,FLAG_REFL_MISSILE))
				SET_FLAG(op,FLAG_REFL_MISSILE);
			if(QUERY_FLAG(tmp,FLAG_STEALTH))
				SET_FLAG(op,FLAG_STEALTH);
			if(QUERY_FLAG(tmp,FLAG_UNDEAD)&&!QUERY_FLAG(&op->arch->clone,FLAG_UNDEAD))
				SET_FLAG(op,FLAG_UNDEAD);
			if(QUERY_FLAG(tmp,FLAG_XRAYS))
				SET_FLAG(op,FLAG_XRAYS);
			if(QUERY_FLAG(tmp,FLAG_BLIND)) 
				SET_FLAG(op,FLAG_BLIND);
			if(QUERY_FLAG(tmp,FLAG_SEE_IN_DARK)) 
				SET_FLAG(op,FLAG_SEE_IN_DARK);

			if(QUERY_FLAG(tmp,FLAG_SEE_INVISIBLE)) 
				SET_FLAG(op,FLAG_SEE_INVISIBLE); 
			if(QUERY_FLAG(tmp,FLAG_MAKE_INVISIBLE)) 
				SET_FLAG(op,FLAG_IS_INVISIBLE);
			if(QUERY_FLAG(tmp,FLAG_CAN_PASS_THRU)) 
				SET_FLAG(op,FLAG_CAN_PASS_THRU); 
			if(QUERY_FLAG(tmp,FLAG_MAKE_ETHEREAL))
			{
				SET_FLAG(op,FLAG_CAN_PASS_THRU); 
				SET_FLAG(op,FLAG_IS_ETHEREAL); 
			}
			if(QUERY_FLAG(tmp,FLAG_FLYING))
			{
				SET_FLAG(op,FLAG_FLYING);
				if(!QUERY_FLAG(op,FLAG_WIZ))
					max=1;
			}

			/* slow penalty when i remember it right... */
			if(tmp->stats.exp && tmp->type!=EXPERIENCE && tmp->type != SKILL)
			{
				if(tmp->stats.exp > 0) 
				{
					added_speed+=(float)tmp->stats.exp/3.0f;
					bonus_speed+=1.0f+(float)tmp->stats.exp/3.0f;
				} 
				else
					added_speed+=(float)tmp->stats.exp;
			}

			switch(tmp->type) 
			{

				/* EXPERIENCE objects. What we are doing here is looking for "relevant" 
				 * experience objects. Some of these will be used to calculate 
				 * level-based changes in player status. For expample, the 
				 * experience object which has exp_obj->stats.Str set controls the 
				 * wc bonus of the player. -b.t.
				 */
				case EXPERIENCE: 
					if (tmp->stats.Str && !wc_obj) 
						wc_obj = tmp;
					if (tmp->stats.Con && !hp_obj) 
						hp_obj = tmp;
					if (tmp->stats.Pow && !mana_obj)  /* for spellpoint determ */ 
						mana_obj = tmp;
					if (tmp->stats.Wis && !grace_obj)
						grace_obj = tmp; 
				break;

				case SKILL: 		/* skills modifying the character -b.t. */
					if(tmp->stats.dam>0)  	/* skill is a 'weapon' */
					{
						wc-=(tmp->stats.wc+tmp->magic);
						ac-=(tmp->stats.ac+tmp->magic);
						op->weapon_speed = tmp->weapon_speed ;
    					weapon_weight=tmp->weight;
	    				op->stats.dam+=(tmp->stats.dam+tmp->magic);
					}
					else /* be careful with ->magic in applyable skills */
					{
						/* i added magic to wc/ac even when they are not set */
	    				op->stats.dam+=tmp->magic;
						wc-=(tmp->stats.wc+tmp->magic);
						ac-=(tmp->stats.ac+tmp->magic);
					}
					if(tmp->slaying!=NULL)
						FREE_AND_COPY(op->slaying,tmp->slaying);

						op->contr->encumbrance+=(int)3*tmp->weight/1000;

				break;

				case SHIELD:
				op->contr->encumbrance+=(int)tmp->weight/2000;
				case RING:
				case AMULET:
				case GIRDLE:
				case HELMET:
				case BOOTS:
				case GLOVES:
				case CLOAK:
			        if(tmp->stats.wc)
						wc-=(tmp->stats.wc+tmp->magic);
					if(tmp->stats.dam)
						op->stats.dam+=(tmp->stats.dam+tmp->magic);
					if(tmp->stats.ac)
						ac-=(tmp->stats.ac+tmp->magic);
				break;

				case BOW:
					i = tmp->sub_type1;
					if(i == RANGE_WEAP_BOW)
						op->contr->set_skill_archery = SK_MISSILE_WEAPON;
					else if(i == RANGE_WEAP_XBOWS)
						op->contr->set_skill_archery = SK_XBOW_WEAP;
					else
						op->contr->set_skill_archery = SK_SLING_WEAP;          
				break; 

		        /* the new weapon skill system is more complex
				 * when new applied, set_skill_weapon is set. 
				 * But we took care to safely set it here.
				 */ 
				case WEAPON:
					op->contr->selected_weapon = tmp; /* thats our weapon */
		            i = tmp->sub_type1%4;
					if(i == WEAP_1H_IMPACT)
						op->contr->set_skill_weapon = SK_MELEE_WEAPON;
					else if(i == WEAP_1H_SLASH)
						op->contr->set_skill_weapon = SK_SLASH_WEAP;
					else if(i == WEAP_1H_CLEAVE)
						op->contr->set_skill_weapon = SK_CLEAVE_WEAP;
					else
						op->contr->set_skill_weapon = SK_PIERCE_WEAP;

					/* the new weapon_speed system has nothing to do with the old one,
					 * so take care weapon speed is always our wielded weapon -
					 * after we have all added ,we adjust it later for dex bonus, etc (todo)
                     */
					op->weapon_speed = tmp->weapon_speed;
					if(!op->weapon_speed)
						LOG(llevBug,"BUG: monster/player %s applied weapon %s without weapon speed!\n", op->name, tmp->name);
					wc-=(tmp->stats.wc+tmp->magic);
					if(tmp->stats.ac&&tmp->stats.ac+tmp->magic>0)
						ac-=tmp->stats.ac+tmp->magic;
					op->stats.dam+=(tmp->stats.dam+tmp->magic);
					weapon_weight=tmp->weight;
					if(tmp->slaying!=NULL) 
						FREE_AND_COPY(op->slaying,tmp->slaying);
#ifdef PLUGINS
					if (tmp->event_hook[EVENT_ATTACK] != NULL)
					{
						if (op->current_weapon_script)
							free_string(op->current_weapon_script);
						op->current_weapon_script=add_string(query_name(tmp));
					}
#endif
					op->current_weapon = tmp;

					op->contr->encumbrance+=(int)3*tmp->weight/1000;
				break;

				case ARMOUR: /* Only the best of these three are used: */
					op->contr->encumbrance+=(int)tmp->weight/1000;
				/* thats the ugly old d&d bracer ac/wc boni way to work.
				 * i think i will kill this next, i remember i never used
				 * it in the old days i played pencil&paper d&d.
				 */
				case BRACERS:
				case FORCE:
					if(tmp->stats.wc) 
					{ 
						if(best_wc<tmp->stats.wc+tmp->magic)
						{
							wc+=best_wc;
							best_wc=tmp->stats.wc+tmp->magic;
						} 
						else
							wc+=tmp->stats.wc+tmp->magic;
					}
					if(tmp->stats.ac) 
					{
						if(best_ac<tmp->stats.ac+tmp->magic)
						{
							ac+=best_ac; /* Remove last bonus */
							best_ac=tmp->stats.ac+tmp->magic;
						}
						else /* To nullify the below effect */
							ac+=tmp->stats.ac+tmp->magic;
					}
					if(tmp->stats.wc)
						wc-=(tmp->stats.wc+tmp->magic);
					if(tmp->stats.ac)
						ac-=(tmp->stats.ac+tmp->magic);
					if(ARMOUR_SPEED(tmp)&&(float)ARMOUR_SPEED(tmp)/10.0f<max)
						max=ARMOUR_SPEED(tmp)/10.0f;
				break;
			}
		}
	} /* Item is equipped - end of for loop going through items. */

	/* 'total resistance = total protections - total vulnerabilities'.
	 * If there is an uncursed potion in effect, granting more protection
	 * than that, we take: 'total resistance = resistance from potion'.
	 * If there is a cursed (and no uncursed) potion in effect, we take
	 * 'total resistance = vulnerability from cursed potion'. 
	 */

	op->attacktype = 0;
	for (j=1,i=0; i<NROFATTACKS; i++,j<<=1)
	{
		if(op->attack[i])
			op->attacktype |= j;
      
		op->resist[i] = resists[i] - vuln[i];
		if (potion_resist[i] && (potion_resist[i] > op->resist[i] || potion_resist[i] < 0))
			op->resist[i] = potion_resist[i];
	}

	for (i=0; i<NROFPROTECTIONS; i++) 
		op->protection[i] = protect_boni[i];

	check_stat_bounds(&(op->stats));


	/* now the speed thing... */
	op->speed+=speed_bonus[op->stats.Dex];
#ifdef SEARCH_ITEMS
	if (op->contr->search_str[0])
		op->speed -= 1.0f;
#endif
	if(added_speed>=0)
		op->speed+=added_speed/10.0f;
	else /* Something wrong here...: */
		op->speed /= 1.0f-added_speed;
	if(op->speed>max)
		op->speed=max;

	/* calculate speed */

	/* we do now this: we have a weight_limit x. until we don't carry more
	 * as Y% of x, we are not encumbered. The rest of 100% - Y% (lets say 100% - 66% = 34%)
	 * is now our value from 1% encumbrance to 99% encumb. - this encum% will be removed from
	 * our current speed. The advantage is we use only ONE weight value and we can direct 
	 * calculate from weight limit to speed limit.
	 */

	/* first, calculate real speed */
	op->speed+=bonus_speed/10.0f;
	/* Put a lower limit on speed.  Note with this speed, you move once every
	 * 100 ticks or so.  This amounts to once every 12 seconds of realtime.
	 */
	op->speed = op->speed * speed_reduce_from_disease;

	/* don't reduce under this value */
	if (op->speed<0.01f) 
		op->speed=0.01f;
	else
	{
		f = ((float)weight_limit[op->stats.Str]/100.0f)*ENCUMBRANCE_LIMIT; /* = max kg we can carry */
		if(((sint32)f)<=op->carrying)
		{
			if(op->carrying>=weight_limit[op->stats.Str])
				op->speed=0.01f; /* ouch */
			else
			{

				f = ((float)weight_limit[op->stats.Str]-f); /* total encumbrance weight part */
				f = ((float)weight_limit[op->stats.Str]-op->carrying)/f; /* value from 0.0 to 1.0 encumbrance */

				if(f<0.0f)
					f=0.0f;
				else if(f>1.0f)
					f = 1.0f;

				op->speed*=f;

				if(op->speed<0.01f)
					op->speed=0.01f; /* ouch */
			}
		}

	}
	update_ob_speed(op);

	op->weapon_speed_add = op->weapon_speed;
	op->contr->weapon_sp = op->weapon_speed;  

	/* Prevent overflows of wc - best you can get is ABS(120) - this
     * should be more than enough - remember, AC is also in 8 bits,
	 * so its value is the same.
	 */
	if (wc>120)
		wc=120;
	else if (wc<-120) 
		wc=-120;

	if (ac>120) 
		ac=120;
	else if (ac<-120)
		ac=-120;

	op->glow_radius = light;

	/* for player, max hp depend on general level, sp on magic exp, grace on wisdom exp level
	 * NOTE: all values are adjusted from clone at function start.
	 */
	op->stats.maxhp = op->stats.maxhp * (op->level+3) - op->contr->maxhp_malus;
	op->stats.maxsp = op->stats.maxsp * ((mana_obj!=NULL?mana_obj->level:op->level)+1) - op->contr->maxsp_malus;
	op->stats.maxgrace = op->stats.maxgrace * ((grace_obj!=NULL?grace_obj->level:op->level)+1) - op->contr->maxgrace_malus;

	/* when this is set, this object comes fresh in game.
	 * we must adjust now hp,sp and grace with the max values.
	 * if hp/sp/grace == -1, then set it to max value.
	 * if it != 0, then leave it.
	 * in this form, we can put "hurt or wounded" objects to map.
	 */

	
	if(op->stats.hp==-1)
		op->stats.hp = op->stats.maxhp;
	if(op->stats.sp==-1)
		op->stats.sp = op->stats.maxsp;
	if(op->stats.grace==-1)
			op->stats.grace = op->stats.maxgrace;

	/* cap the pools to <=max */
	if(op->stats.hp>op->stats.maxhp)
		op->stats.hp = op->stats.maxhp;
	if(op->stats.sp>op->stats.maxsp)
		op->stats.sp = op->stats.maxsp;
	if(op->stats.grace>op->stats.maxgrace)
		op->stats.grace = op->stats.maxgrace;

	/* hm, perhaps we are changing this to physical level? ... i will have it in mind
	 * to install here something complexer */

	op->stats.ac = ac - op->level; /* here our true level come in play - valuable ac */


	 /* Now we have all collected - the best bare hand skill and/or the applied weapon.
	  * If we have no weapon selected, use the bare hand skill for attacking and wc/dam .
	  * if we have a weapon, use the weapon and forget the bare hand skill.
	  */

	 if(op->contr->set_skill_weapon==NO_SKILL_READY) /* ok, no weapon in our hand - we must use our hands */
	 {
		 if(skill_weapon) 
		 {
			op->contr->skill_weapon = skill_weapon;
			op->stats.wc = wc - skill_weapon->level;
			op->stats.dam = (sint16)((float)op->stats.dam * lev_damage[skill_weapon->level]);
		 }
		 else
			LOG(llevBug,"BUG: fix_player(): player %s has no hth skill!\n",op->name);
	 }
	 else /* weapon in hand */
	 {
		 /* ouch - weapon without the skill applied... */
		 if(!op->contr->skill_ptr[op->contr->set_skill_weapon])
			LOG(llevBug,"BUG: fix_player(): player %s has weapon selected but not the skill #%d!!!\n",op->name, op->contr->set_skill_weapon);
		 else
		 {
			op->stats.wc = wc - op->contr->skill_ptr[op->contr->set_skill_weapon]->level;
			op->stats.dam = (sint16)((float)op->stats.dam * lev_damage[op->contr->skill_ptr[op->contr->set_skill_weapon]->level]);
		 }
	 }

	
	/*LOG(-1,"PLAYER ADJUST: %s -> hp(%d %d %d %d) sp(%d %d %d %d) gr(%d %d %d %d)\n",op->name,
		op->stats.hp,op->stats.maxhp,op->arch->clone.stats.maxhp,op->stats.maxhp_adj, 
		op->stats.sp,op->stats.maxsp,op->arch->clone.stats.maxsp,op->stats.maxsp_adj, 
		op->stats.grace,op->stats.maxgrace,op->arch->clone.stats.maxgrace,op->stats.maxgrace_adj);
	*/
	if(QUERY_FLAG(op,FLAG_IS_INVISIBLE) )
	{
		if(!inv_flag)
			update_object(op, UP_OBJ_INV);
	}
	else if(inv_flag) /* and !FLAG_IS_INVISIBLE */
		update_object(op, UP_OBJ_INV);

	if(QUERY_FLAG(op,FLAG_SEE_INVISIBLE) )
	{
		if(!inv_see_flag)
			op->contr->socket.update_look=1;
	}
	else if(inv_see_flag) /* and !FLAG_SEE_INVISIBLE */
		op->contr->socket.update_look=1;
}

/*
 * Returns true if the given player is a legal class.
 * The function to add and remove class-bonuses to the stats doesn't
 * check if the stat becomes negative, thus this function
 * merely checks that all stats are 1 or more, and returns
 * false otherwise.
 */

int allowed_class(object *op) {
  return op->stats.Dex>0&&op->stats.Str>0&&op->stats.Con>0&&
         op->stats.Int>0&&op->stats.Wis>0&&op->stats.Pow>0&&
	 op->stats.Cha>0;
}

/*
 * Returns how much experience is needed for a player to become
 * the given level.
 */

uint32 level_exp(int level,double expmul) {
    static long int bleep=1650000; 

    if(level<=100)
	    return (uint32)(expmul * (double)new_levels[level]);

    /*  return required_exp; */
	return (uint32)(expmul*(double)(new_levels[100]+bleep*(level-100)));
}

/* add_exp() new algorithm. Revamped experience gain/loss routine. 
 * Based on the old add_exp() function - but tailored to add experience 
 * to experience objects. The way this works-- the code checks the 
 * current skill readied by the player (chosen_skill) and uses that to
 * identify the appropriate experience object. Then the experience in
 * the object, and the player's overall score are updated. In the case
 * of exp loss, all exp categories which have experience are equally
 * reduced. The total experience score of the player == sum of all 
 * exp object experience.  - b.t. thomas@astro.psu.edu 
 */
/* The old way to determinate the right skill which is used for exp gain
 * was broken. Best way to show this is, to cast some fire balls in a mob
 * and then changing the hand weapon some times. You will get some "no
 * ready skill warnings". 
 * I reworked the whole system and the both main exp gain and add functions
 * add_exp() and adjust_exp(). Its now much faster, easier and more accurate. MT
 * exp lose by dead is handled from apply_death_exp_penalty().
 */
sint32 add_exp(object *op, int exp, int skill_nr) {
    object *exp_ob=NULL;    /* the exp. object into which experience will go */ 
    object *exp_skill=NULL; /* the real skill object */ 
    int del_exp=0;
    int limit=0;
    
    /* safety */
    if(!op) 
    { 
    	LOG(llevBug,"BUG: add_exp() called for null object!\n"); 
	    return 0; 
    }
            
    if(op->type != PLAYER) 
        return 0; /* no exp gain for mobs */

            
    /* ok, we have global exp gain or drain - we must grap a skill for it! */
    if(skill_nr == CHOSEN_SKILL_NO)
    {
        /* TODO: select skill */
        LOG(llevDebug,"TODO: add_exp(): called for %s with exp %d. CHOSEN_SKILL_NO set. TODO: select skill.\n",op->name,exp);
        return 0;
    }
            
    /* now we grap the skill exp. object from the player shortcut ptr array */
    exp_skill = op->contr->skill_ptr[skill_nr];
                    
    if(!exp_skill) /* safety check */
    {
        /* our player don't have this skill?
         * This can happens when group exp is devided.
         * We must get a useful sub or we skip the exp.
         */
         LOG(llevDebug,"TODO: add_exp(): called for %s with skill nr %d / %d exp - object has not this skill.\n",op->name,skill_nr, exp);
         return 0; /* TODO: groups comes later  - now we skip all times */
    }

    /* if we are full in this skill, then nothing is to do */
    if(exp_skill->level >= MAXLEVEL)
        return 0;    
            
    op->contr->update_skills=1; /* we will sure change skill exp, mark for update */
    exp_ob = exp_skill->exp_obj;            

    if(!exp_ob)
    {
	LOG(llevBug,"BUG: add_exp() skill:%s - no exp_op found!!\n",exp_skill->name);
	return 0;
    }
    
    /* General adjustments for playbalance */ 
    /* I set limit to 1/4 of a level - thats enormous much */
    limit=(new_levels[exp_skill->level+1]-new_levels[exp_skill->level])/4;
    if (exp > limit) 
        exp=limit;
                
    exp = adjust_exp(op, exp_skill,exp);   /* first we see what we can add to our skill */ 
            
    /* adjust_exp has adjust the skill and all exp_obj and player exp */
    /* now lets check for level up in all categories */
    player_lvl_adj(op,exp_skill);   
    player_lvl_adj(op,exp_ob);   
    player_lvl_adj(op,NULL);   
 
	/* reset the player exp_obj to NULL */
    /* I let this in but because we use single skill exp and skill nr now,
     * this broken exp_obj concept can be removed
     */
	if(op->exp_obj)
        op->exp_obj = NULL;

    return (sint32) exp; /* thats the real exp we have added to our skill */
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
void set_dragon_name(object *pl, object *abil, object *skin) {
  int atnr=-1;  /* attacknumber of highest level */
  int level=0;  /* highest level */
  int i;
  
  /* first, look for the highest level */
  for(i=0; i<NROFATTACKS; i++) {
    if (atnr_is_dragon_enabled(i) &&
	(atnr==-1 || abil->resist[i] > abil->resist[atnr])) {
      level = abil->resist[i];
      atnr = i;
    }
  }
  
  /* now if there are equals at highest level, pick the one with focus,
     or else at random */
  if (atnr_is_dragon_enabled(abil->stats.exp) &&
      abil->resist[abil->stats.exp] >= level)
    atnr = abil->stats.exp;
  
  level = (int)(level/5.);
  
  /* now set the new title */
  if (pl->contr != NULL) {
    if(level == 0)
      sprintf(pl->contr->title, "%s hatchling", attacks[atnr]);
    else if (level == 1)
      sprintf(pl->contr->title, "%s wyrm", attacks[atnr]);
     else if (level == 2)
      sprintf(pl->contr->title, "%s wyvern", attacks[atnr]);
    else if (level == 3)
      sprintf(pl->contr->title, "%s dragon", attacks[atnr]);
    else {
      /* special titles for extra high resistance! */
      if (skin->resist[atnr] > 80)
	sprintf(pl->contr->title, "legendary %s dragon", attacks[atnr]);
      else if (skin->resist[atnr] > 50)
	sprintf(pl->contr->title, "ancient %s dragon", attacks[atnr]);
      else
	sprintf(pl->contr->title, "big %s dragon", attacks[atnr]);
    }
  }
}

/*
 * This function is called when a dragon-player gains
 * an overall level. Here, the dragon might gain new abilities
 * or change the ability-focus.
 */
void dragon_level_gain(object *who) {
  object *abil = NULL;    /* pointer to dragon ability force*/
  object *skin = NULL;    /* pointer to dragon skin force*/
  object *tmp = NULL;     /* tmp. object */
  char buf[MAX_BUF];      /* tmp. string buffer */
  
  /* now grab the 'dragon_ability'-forces from the player's inventory */
  for (tmp=who->inv; tmp!=NULL; tmp=tmp->below) {
    if (tmp->type == FORCE) {
      if (strcmp(tmp->arch->name, "dragon_ability_force")==0)
	abil = tmp;
      if (strcmp(tmp->arch->name, "dragon_skin_force")==0)
	skin = tmp;
    }
  }
  /* if the force is missing -> bail out */
  if (abil == NULL) return;
  
  /* The ability_force keeps track of maximum level ever achieved.
     New abilties can only be gained by surpassing this max level */
  if (who->level > abil->level) {
    /* increase our focused ability */
    abil->resist[abil->stats.exp]++;
    
    if (abil->resist[abil->stats.exp]>0 && abil->resist[abil->stats.exp]%5 == 0) {
      /* time to hand out a new ability-gift */
      (*dragon_gain_func)(who, abil->stats.exp,
			       (int)((1+abil->resist[abil->stats.exp])/5.));
    }
    
    if (abil->last_eat > 0 && atnr_is_dragon_enabled(abil->last_eat)) {
      /* apply new ability focus */
      sprintf(buf, "Your metabolism now focuses on %s!",
	      change_resist_msg[abil->last_eat]);
      (*draw_info_func)(NDI_UNIQUE|NDI_BLUE, 0, who, buf);
      
      abil->stats.exp = abil->last_eat;
      abil->last_eat = 0;
    }
    
    abil->level = who->level;
  }
  
  /* last but not least, set the new title for the dragon */
  set_dragon_name(who, abil, skin);
}

/* player_lvl_adj() - for the new exp system. we are concerned with
 * whether the player gets more hp, sp and new levels.
 * -b.t.
 */
void player_lvl_adj(object *who, object *op) {
   
	char buf[MAX_BUF];

    if(!op)        /* when rolling stats */ 
	op = who;

	if(op->type == SKILL && !op->last_eat) /* no exp gain for indirect skills */
	{
		LOG(llevBug,"BUG: player_lvl_adj() called for indirect skill %s (who: %s)\n", 
												op->name, who==NULL?"<null>":who->name);
		return;
	}

    /*LOG(llevDebug,"LEVEL: %s ob:%s l: %d e: %d\n", who==NULL?"<null>":who->name, op->name,op->level, op->stats.exp);*/
    if(op->level < MAXLEVEL && op->stats.exp >= (sint32)level_exp(op->level+1,1.0)) 
	{
		op->level++;

		if (op == who && op->stats.exp > 1 && is_dragon_pl(who))
			dragon_level_gain(who);
	
		if(who && (who->level < 11) && op->type!=EXPERIENCE && op->type!=SKILL) 
		{ 
			who->contr->levhp[who->level] = die_roll(2, 4, who, PREFER_HIGH)+1;
			who->contr->levsp[who->level] = die_roll(2, 3, who, PREFER_HIGH);
			who->contr->levgrace[who->level]=die_roll(2, 2, who, PREFER_HIGH)-1;
		}

		if(who) 
			fix_player(who);
		if(op->level>1 && op->type==EXPERIENCE) 
		{
			sprintf(buf,"You are now level %d in %s based skills.",op->level,op->name);
			if(who) (*draw_info_func)(NDI_UNIQUE|NDI_RED, 0, who,buf);
		}
		else if(op->level>1 && op->type==SKILL) 
		{
			sprintf(buf,"You are now level %d in the skill %s.",op->level,op->name);
			if(who) 
				(*draw_info_func)(NDI_UNIQUE|NDI_RED, 0, who,buf);
		}
		else
		{
			sprintf(buf,"You are now level %d.",op->level);
			if(who) 
				(*draw_info_func)(NDI_UNIQUE|NDI_RED, 0, who,buf);
		}
		player_lvl_adj(who,op); /* To increase more levels */
    } 
	else if(op->level>1&&op->stats.exp<(sint32)level_exp(op->level,1.0)) 
	{
		op->level--;

		if(who) 
			fix_player(who);

		if(op->type==EXPERIENCE) 
		{
			sprintf(buf,"-You are now level %d in %s based skills.",op->level,op->name);
			if(who) 
				(*draw_info_func)(NDI_UNIQUE|NDI_RED, 0, who,buf);
		}
		else if(op->type==SKILL) 
		{
			sprintf(buf,"-You are now level %d in the skill %s.",op->level,op->name);
			if(who) 
				(*draw_info_func)(NDI_UNIQUE|NDI_RED, 0, who,buf);
		}
		else
		{
			sprintf(buf,"-You are now level %d.",op->level);
			if(who) 
				(*draw_info_func)(NDI_UNIQUE|NDI_RED, 0, who,buf);
		}
		player_lvl_adj(who,op); /* To decrease more levels */
    }
}

/* Ensure that the permanent experience requirements in an exp object are met. */
/* GD */
void calc_perm_exp(object *op)
{
    int p_exp_min;

    /* Sanity checks. */
    if (op->type != EXPERIENCE) {
        LOG(llevBug, "BUG: calc_minimum_perm_exp called on a non-experience object!");
        return;
    }
    if (!(settings.use_permanent_experience)) {
        LOG(llevBug, "BUG: calc_minimum_perm_exp called whilst permanent experience disabled!");
        return;
    }

    /* The following fields are used: */
    /* stats.exp: Current exp in experience object. */
    /* last_heal: Permanent experience earnt. */
    
    /* Ensure that our permanent experience minimum is met. */
    p_exp_min = (int)(PERM_EXP_MINIMUM_RATIO * (float)(op->stats.exp));
    /*LOG(llevBug, "BUG: Experience minimum check: %d p-min %d p-curr %d curr.\n", p_exp_min, op->last_heal, op->stats.exp);*/
    if (op->last_heal < p_exp_min)
        op->last_heal = p_exp_min;

    /* Cap permanent experience. */
    if (op->last_heal < 0)
        op->last_heal = 0;
    else if (op->last_heal > (sint32)MAX_EXP_IN_OBJ)
        op->last_heal = MAX_EXP_IN_OBJ;
}

/* adjust_exp() - make sure that we don't exceed max or min set on
 * experience
 * I use this function now as global experience add and sub routine.
 * it should only called for the skills object from players.
 * This function adds or subs the exp and updates all skill objects and
 * the player global exp.
 * You need to call player_lvl_adj() after it.
 * This routine use brute force and goes through the whole inventory. We should
 * use a kind of skill container for speed this up. MT 
 */
int adjust_exp(object *pl, object *op, int exp) {
    object *tmp;
    int i,sk_nr;
    sint32 sk_exp,pl_exp;


    /* be sure this is a skill object from a active player */
    if(op->type != SKILL || !pl || pl->type != PLAYER) 
    {
        LOG(llevBug,"BUG: adjust_exp() - called for non player or non skill: skill: %s -> player: %s\n",
            op->name, pl==NULL?"<NO CONTR>":pl->name);
        return 0;
    }

    /* add or sub the exp and cap it. it must be >=0 and <= MAX_EXPERIENCE */
    op->stats.exp += exp;
    if(op->stats.exp < 0)
    {
        exp -= op->stats.exp;
        op->stats.exp = 0;
    }
    
    if(op->stats.exp>(sint32)MAX_EXPERIENCE) 
    {
        exp = exp - (op->stats.exp - MAX_EXPERIENCE);
        op->stats.exp=MAX_EXPERIENCE;
    }

    /* now we collect the exp of all skills which are in the same exp. object category */  
    sk_nr = skills[op->stats.sp].category;
    sk_exp =0;
    for(tmp=pl->inv;tmp;tmp=tmp->below)
    {
        if(tmp->type==SKILL && skills[tmp->stats.sp].category == sk_nr)
        {
            if((sk_exp += tmp->stats.exp) > (sint32)MAX_EXPERIENCE)
                sk_exp = MAX_EXPERIENCE;
        }
    }

    /* set the exp of the exp. object to our skill */
    op->exp_obj->stats.exp = sk_exp;
    
    /* now we collect all exp. objects exp */
    pl_exp=0;
    for(i=0;i<MAX_EXP_CAT-1;i++)
    {
        if((pl_exp += pl->contr->last_skill_ob[i]->stats.exp) > (sint32)MAX_EXPERIENCE)
            pl_exp = MAX_EXPERIENCE;
    }

    /* last action: set our player exp */
    pl->stats.exp = pl_exp;
    
    return exp;	/* return the actual amount changed stats.exp we REALLY have added to our skill */ 
}

/* check_dm_add_exp() - called from c_wiz.c. Needed by ALLOW_SKILLS
 * code. -b.t.
 */

int check_dm_add_exp_to_obj(object *exp_ob, int i) {

    if((exp_ob->stats.exp + i) < 0) 
	i= -1*(exp_ob->stats.exp);
    else if((exp_ob->stats.exp +i)> (sint32)MAX_EXP_IN_OBJ)
	i= MAX_EXP_IN_OBJ - exp_ob->stats.exp;
    return i;
}

/* OLD: Applies a death penalty experience.  20% or 3 levels, whichever is
   less experience lost. */
/* NEW: We run through the skills and reduce every skill by 5%-skilllevel/10. 
   but min 1%. This is only the first try - better will come */
void apply_death_exp_penalty(object *op)
{
    object *tmp;
    long int del_exp=0;
    int loss_5p, loss;

    op->contr->update_skills=1; /* we will sure change skill exp, mark for update */
    for(tmp=op->inv;tmp;tmp=tmp->below)
    {
        if(tmp->type==SKILL && tmp->stats.exp)
        { 
            loss_5p = 10 -(tmp->level/10);
            if(loss_5p >10)
                loss_5p =10;
            if (loss_5p <=0)
                loss_5p = 1;

            loss = (int) (((float)tmp->stats.exp/100.0f)*(float)loss_5p);
            if(loss<=0)
                loss = 1;
            if (loss>500000)
                loss=500000;
            if(loss >=tmp->stats.exp)
                loss = tmp->stats.exp;
            del_exp+=adjust_exp(op, tmp,-loss);
            
            player_lvl_adj(op,tmp);
        }
    }

    for(tmp=op->inv;tmp;tmp=tmp->below)
    {
        if(tmp->type==EXPERIENCE && tmp->stats.exp)
            player_lvl_adj(op,tmp); /* adjust exp objects levels */
    }
    player_lvl_adj(op,NULL);        /* and at last adjust the player level */
}

/* This adjust the monsters dats to level, map settings and game settings
 * when put in play.
 */
void fix_monster(object *op)
{
	object *base;
	float tmp_add;
	
	if(op->head) /* don't adjust tails or player - only single objects or heads */
		return;

	base = insert_base_info_object(op); /* will insert or/and return base info */

	/* pre adjust */
	op->stats.maxhp = base->stats.maxhp * (op->level+3);
	op->stats.maxsp = base->stats.maxsp * (op->level+1);
	op->stats.maxgrace = base->stats.maxgrace * (op->level+1);
	
	/* remember: -1 is a place holder - if == -1, we fill in max value.
	 * if the value is != -1, object has explicit set to a different value
	 * (for example to simulate on a map a injured target) or its previous
	 * damaged and saved
	 */

	if(op->stats.hp==-1)
		op->stats.hp = op->stats.maxhp;
	if(op->stats.sp==-1)
		op->stats.sp = op->stats.maxsp;
	if(op->stats.grace==-1)
			op->stats.grace = op->stats.maxgrace;

	/* cap the pools to <=max */
	if(op->stats.hp>op->stats.maxhp)
		op->stats.hp = op->stats.maxhp;
	if(op->stats.sp>op->stats.maxsp)
		op->stats.sp = op->stats.maxsp;
	if(op->stats.grace>op->stats.maxgrace)
		op->stats.grace = op->stats.maxgrace;

	op->stats.ac = base->stats.ac - op->level;
	op->stats.wc = base->stats.wc - op->level;

	op->stats.dam =  base->stats.dam;

	/* post adjust */

	if((tmp_add = lev_damage[op->level/3]-0.75f) <0)
		tmp_add =0;
	op->stats.dam = (sint16) ((float)op->stats.dam * (lev_damage[op->level]+tmp_add));

}

/* insert and initialize base info object in object op 
 * Return: ptr to inserted base_info 
 */
object *insert_base_info_object(object *op)
{
	object *tmp, *head;
	archetype *at;

	op->head!=NULL?(head=op->head):(head=op);

	if(op->type == PLAYER)
	{
		LOG(llevBug,"insert_base_info_object() Try to inserting base_info in player %s!\n", query_name(head));
		return NULL;
	}

	if((tmp=find_base_info_object(head)))
		return tmp;

	/* prepare base info */
    tmp = arch_to_object(base_info_archetype);
	at = tmp->arch; /* if not saved, we will truly change tmp to op archetype object */
	copy_object_data(head, tmp); /* copy without put on active list */
	tmp->arch=at;
	tmp->speed_left = tmp->speed;	/* we copy the real speed to speed_left - so we ensure this can't be hit the active list */ 	
	tmp->speed=0.0f; /* ensure this object will not be active in any way */
	tmp->type = TYPE_BASE_INFO;
	tmp->face = tmp->arch->clone.face;
	SET_FLAG(tmp,FLAG_NO_DROP);
	CLEAR_FLAG(tmp,FLAG_ANIMATE);
	CLEAR_FLAG(tmp,FLAG_FRIENDLY);
	CLEAR_FLAG(tmp,FLAG_ALIVE);
	CLEAR_FLAG(tmp,FLAG_MONSTER);
    insert_ob_in_ob(tmp,head); /* and put it in the mob */

	return tmp;
}

/* find base_info in *op
 * Return: ptr to inserted base_info 
 */
object *find_base_info_object(object *op)
{
	object *tmp;
	
	for(tmp=op->inv;tmp;tmp=tmp->below)
    {
		if(tmp->type == TYPE_BASE_INFO)
			return tmp;
	}

	return NULL; /* no base_info in this object found */
}
