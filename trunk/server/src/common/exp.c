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

/* there are some more exp calc funtions in skill_util.c - but thats part
 * of /server/server and not of crosslib.a - so we can't move then easily 
 * on this place - another reason to kill the crosslib.a asap.
 */

#include <stdio.h>
#include <global.h>


float lev_exp[MAXLEVEL+1]= {
0.0f,
1.0f, 1.11f, 1.75f,3.2f,	
5.5f, 10.0f, 20.0f, 35.25f, 66.1f,
137.0f, 231.58f, 240.00f, 247.62f, 254.55f,
260.87f, 266.67f, 272.00f, 276.92f, 281.48f,
285.71f, 289.66f, 293.33f, 296.77f, 300.00f,
303.03f, 305.88f, 308.57f, 311.11f, 313.51f,
315.79f, 317.95f, 320.00f, 321.95f, 323.81f,
325.58f, 327.27f, 328.89f, 330.43f, 331.91f,
333.33f, 334.69f, 336.00f, 337.25f, 338.46f,
339.62f, 340.74f, 341.82f, 342.86f, 343.86f,
344.83f, 345.76f, 346.67f, 347.54f, 348.39f,
349.21f, 350.00f, 350.77f, 351.52f, 352.24f,
352.94f, 353.62f, 354.29f, 354.93f, 355.56f,
356.16f, 356.76f, 357.33f, 357.89f, 358.44f,
358.97f, 359.49f, 360.00f, 360.49f, 360.98f,
361.45f, 361.90f, 362.35f, 362.79f, 363.22f,
363.64f, 364.04f, 364.44f, 364.84f, 365.22f,
365.59f, 365.96f, 366.32f, 366.67f, 367.01f,
367.35f, 367.68f, 368.00f, 368.32f, 368.63f,
368.93f, 369.23f, 369.52f, 369.81f, 370.09f,
370.37f, 370.64f, 370.91f, 371.17f, 371.43f, 
371.68f, 371.93f, 372.17f, 372.41f, 372.65f,
372.88f
};

/* around level 11 you need 38+(2*(your_level-11)) yellow
 * mobs with a base exp of 125 to level up. 
 * Every level >11 needs 100.000 exp more as the one before but
 * also one mob more to kill.
 * This avoid things like: "you geht 342.731.123 exp from this mob,
 * you have now 1.345.535.545.667 exp." 
 * even here we have around 500.000.000 max exp - thats a pretty big
 * number.
 */
uint32 new_levels[MAXLEVEL+2]={
0,
0,1500,4000, 8000,
16000,32000,64000,125000,250000,		/* 9 */
500000,1100000, 2300000, 3600000, 5000000,
6500000, 8100000, 9800000, 11600000, 13500000, /* 19 */
15500000, 17600000, 19800000, 22100000, 24500000,
27000000,29600000, 32300000, 35100000, 38000000, /* 29 */
 41000000, 44100000, 47300000, 50600000, 54000000,
 57500000, 61100000, 64800000, 68600000, 72500000, /* 39 */
 76500000,80600000, 84800000, 89100000, 93500000,
 98000000,102600000, 107300000, 112100000, 117000000, /* 49 */
 122000000,127100000, 132300000, 137600000, 143000000, 
 148500000, 154100000, 159800000, 165600000, 171500000, /*59 */
 177500000, 183600000, 189800000, 196100000, 202500000,
 209000000,215600000, 222300000, 229100000, 236000000, /* 69 */
 243000000,250100000, 257300000, 264600000, 272000000, 
 279500000, 287100000, 294800000, 302600000, 310500000, /* 79 */
 318500000,326600000, 334800000, 343100000, 351500000, 
 360000000,368600000, 377300000, 386100000, 395000000, /* 89 */
 404000000, 413100000, 422300000, 431600000, 441000000, 
 450500000, 460100000, 469800000, 479600000, 489500000, /* 99 */
 499500000,509600000, 519800000, 530100000, 540500000, 
 551000000, 561600000, 572300000, 583100000, 594000000, /* 109 */
 605000000, 700000000 /* 111 is only a dummy */
}; 

_level_color level_color[MAXLEVEL+1];

/*
   Since this is nowhere defined ...
   Both come in handy at least in function add_exp()
*/
#define MAX_EXPERIENCE  new_levels[MAXLEVEL]

#define MAX_EXP_IN_OBJ new_levels[MAXLEVEL]/(MAX_EXP_CAT - 1) 

/*
 * Returns how much experience is needed for a player to become
 * the given level.
 */

uint32 level_exp(int level,double expmul) 
{
	return (uint32)(expmul * (double)new_levels[level]);
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
/*    int del_exp=0; */
    int limit=0;
    
    /*LOG(llevBug,"ADD: add_exp() called for $d!\n", exp); */
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
        LOG(llevDebug,"TODO: add_exp(): called for %s with exp %d. CHOSEN_SKILL_NO set. TODO: select skill.\n",query_name(op),exp);
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
         LOG(llevDebug,"TODO: add_exp(): called for %s with skill nr %d / %d exp - object has not this skill.\n",query_name(op),skill_nr, exp);
         return 0; /* TODO: groups comes later  - now we skip all times */
    }

    /* if we are full in this skill, then nothing is to do */
    if(exp_skill->level >= MAXLEVEL)
        return 0;    
            
    op->contr->update_skills=1; /* we will sure change skill exp, mark for update */
    exp_ob = exp_skill->exp_obj;            

    if(!exp_ob)
    {
	LOG(llevBug,"BUG: add_exp() skill:%s - no exp_op found!!\n",query_name(exp_skill));
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
												query_name(op),
                                                who==NULL?"<null>":query_name(who));
		return;
	}

    /*LOG(llevDebug,"LEVEL: %s ob:%s l: %d e: %d\n", who==NULL?"<null>":who->name, op->name,op->level, op->stats.exp);*/
    if(op->level < MAXLEVEL && op->stats.exp >= (sint32)level_exp(op->level+1,1.0)) 
	{
		op->level++;

		/* show the player some effects... */
		if(op->type == SKILL && who && who->type == PLAYER && who->map)
		{
			object *effect_ob;

			play_sound_player_only (who->contr, SOUND_LEVEL_UP, SOUND_NORMAL, 0, 0);

			if(level_up_arch)
			{
				/* prepare effect */
				effect_ob = arch_to_object(level_up_arch);
				effect_ob->map = who->map;
				effect_ob->x = who->x;
				effect_ob->y = who->y;

				if(!insert_ob_in_map(effect_ob, effect_ob->map, NULL,0))
				{
					/* something is wrong - kill object */
					if(!QUERY_FLAG(effect_ob,FLAG_REMOVED))
						remove_ob(effect_ob);
					free_object(effect_ob);
				}
			}
		}

		if (op == who && op->stats.exp > 1 && is_dragon_pl(who))
			dragon_level_gain(who);
	
		
		if(who && who->type == PLAYER && op->type!=EXPERIENCE && op->type!=SKILL && who->level >1)
		{ 

			if(who->level > 4)
				who->contr->levhp[who->level]=(char)((RANDOM()%who->arch->clone.stats.maxhp)+1);
			else if(who->level > 2)
				who->contr->levhp[who->level]=(char)((RANDOM()%(who->arch->clone.stats.maxhp/2))+1)+(who->arch->clone.stats.maxhp/2);
			else
				who->contr->levhp[who->level]=(char)who->arch->clone.stats.maxhp;
		}
		if(op->level>1 && op->type==EXPERIENCE) 
		{
			if(who && who->type == PLAYER)
			{
				if(op->stats.Pow) /* mana */
				{
					if(op->level > 4)
						who->contr->levsp[op->level]=(char)((RANDOM()%who->arch->clone.stats.maxsp)+1);
					else
						who->contr->levsp[op->level]=(char)who->arch->clone.stats.maxsp;
				}
				else if(op->stats.Wis) /* grace */
				{
					if(op->level > 4)
						who->contr->levgrace[op->level]=(char)((RANDOM()%who->arch->clone.stats.maxgrace)+1);
					else
						who->contr->levgrace[op->level]=(char)who->arch->clone.stats.maxgrace;
				}
			}
			sprintf(buf,"You are now level %d in %s based skills.",op->level,op->name);
			if(who) 
				(*draw_info_func)(NDI_UNIQUE|NDI_RED, 0, who,buf);
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

		if(who) 
			fix_player(who);
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
        LOG(llevBug,"BUG: adjust_exp() - called for non player or non skill: skill: %s -> player: %s\n",query_name(op), query_name(pl));
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
	/* this is the old collection system  - all skills of a exp group add
	 * we changed that to "best skill count"
	 */
	/*
    for(tmp=pl->inv;tmp;tmp=tmp->below)
    {
        if(tmp->type==SKILL && skills[tmp->stats.sp].category == sk_nr)
        {
            if((sk_exp += tmp->stats.exp) > (sint32)MAX_EXPERIENCE)
                sk_exp = MAX_EXPERIENCE;
        }
    }
	*/
    for(tmp=pl->inv;tmp;tmp=tmp->below)
    {
        if(tmp->type==SKILL && skills[tmp->stats.sp].category == sk_nr)
        {
			if(tmp->stats.exp > sk_exp)
				sk_exp = tmp->stats.exp;
        }
    }
    /* set the exp of the exp. object to our best skill of this group */
    op->exp_obj->stats.exp = sk_exp;
    
    /* now we collect all exp. objects exp */
    pl_exp=0;
	/* this is old adding function - we use now the best group 
    for(i=0;i<MAX_EXP_CAT-1;i++)
    {
        if((pl_exp += pl->contr->last_skill_ob[i]->stats.exp) > (sint32)MAX_EXPERIENCE)
            pl_exp = MAX_EXPERIENCE;
    }
	*/
    for(i=0;i<MAX_EXP_CAT-1;i++)
    {
		if(pl->contr->last_skill_ob[i]->stats.exp > pl_exp)
			pl_exp = pl->contr->last_skill_ob[i]->stats.exp;
    }

    /* last action: set our player exp to highest group */
    pl->stats.exp = pl_exp;
    
    return exp;	/* return the actual amount changed stats.exp we REALLY have added to our skill */ 
}

/* we are now VERY friendly - but not because we want. With the
 * new sytem, we never lose level, just % of the exp we gained for
 * the next level. Why? Because dropping the level on purpose by
 * dying again & again will allow under some special circumstances
 * rich players to use exploits.
 * This here is newbie friendly and it allows to make the higher
 * level simply harder. By losing increased levels at high levels
 * you need at last to make recover easy. Now you will not lose much
 * but it will be hard in any case to get exp in high levels.
 * This is a just a design adjustment.
 */
void apply_death_exp_penalty(object *op)
{
    object *tmp;
	float loss_p;
	long level_exp, loss_exp;

    op->contr->update_skills=1; /* we will sure change skill exp, mark for update */
    for(tmp=op->inv;tmp;tmp=tmp->below)
    {
		/* only adjust skills with level and a positive exp value - negative exp has special meaning */
        if(tmp->type==SKILL && tmp->level && tmp->last_eat == 1)
        { 
			/* first, lets check there are exp we can drain. */
			level_exp = tmp->stats.exp - new_levels[tmp->level];
			if(level_exp < 0) /* just a sanity check */
				LOG(llevBug," DEATH_EXP: Skill %s (%d %d) for player %s -> less exp as level need!\n", 
									query_name(tmp), tmp->level, tmp->stats.exp, query_name(op));
			if(!level_exp)
				continue;

			loss_p = 0.9f - (((float)tmp->level/10.0f)*0.1f);
			loss_exp = level_exp - (int)((float)level_exp * loss_p);

			/* again some sanity checks */
			if(loss_exp>0 && loss_exp <= level_exp)
			{
				adjust_exp(op, tmp,-loss_exp);            
				player_lvl_adj(op,tmp);
			}
        }
    }

    for(tmp=op->inv;tmp;tmp=tmp->below)
    {
        if(tmp->type==EXPERIENCE && tmp->stats.exp)
            player_lvl_adj(op,tmp); /* adjust exp objects levels */
    }
    player_lvl_adj(op,NULL);        /* and at last adjust the player level */
}

/* find exp level difference multiplier.
 * If the return value is 0, the level gap is to high
 * for lot drop, exp and/or attacking.
 * who_ is the "activ" object (attacker, activator)
 * op_ is the target 
 */
int calc_level_difference(int who_lvl, int op_lvl)
{
	float tmp;

	tmp = (float)(who_lvl - op_lvl)*2.75f;

	if(tmp > (float)op_lvl*1.6f && (who_lvl - op_lvl)>2)
		return 0;

	return tmp<1.0f?1:(int)tmp;
}
