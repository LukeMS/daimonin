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

/* real exp of a mob: base_exp * lev_exp[mob->level] */
float lev_exp[MAXLEVEL+1]= {
0.0f,
1.0f, 1.11f, 1.75f,3.2f,							/* normal needed mobs zo level (exp 100): 20,20.22.8 */
5.3f,	9.6f, 17.0f, 31.25f,58.1f,			/* 25, 30, 33 , 36, 40 */
88.8f, 104.1f, 120.0f,120.1f,140.0f,		/* 43 45 - 50 */
160.0f, 160.1f, 160.2f, 160.3f, 180.0f,
180.0f,180.0f,180.0f, 200.0f, 200.0f,
200.0f, 200.0f, 220.0f, 220.0f, 220.0f,
220.0f, 220.0f,220.0f, 240.0f, 240.0f,
240.0f,240.0f,240.0f,240.0f,240.0f,
240.0f, 260.0f,260.0f,260.0f,260.0f,
260.0f,260.0f,260.0f,260.0f,260.0f,
280.0f,280.0f,280.0f,280.0f,280.0f,
280.0f,280.0f,280.0f,280.0f,280.0f,
280.0f,280.0f,280.0f,280.0f,280.0f,
300.0f,300.0f,300.0f,300.0f,300.0f,
300.0f,300.0f,300.0f,300.0f,300.0f,
300.0f,300.0f,300.0f,300.0f,300.0f,
300.0f,300.0f, 320.0f, 320.0f, 320.f,
320.0f, 320.0f, 320.f,320.0f, 320.0f,
320.0f, 320.0f, 320.f,320.0f, 320.0f,
320.0f, 320.0f, 320.f,320.0f, 320.0f,
320.0f, 320.0f, 320.f,320.0f, 320.0f,
320.0f, 320.0f, 320.f,320.0f, 320.0f,
320.0f
};


uint32 new_levels[MAXLEVEL+2]={
0,
0,2000,4000, 8000,
16000,32000,64000,125000,250000,		/* 9 */
500000,900000,1400000,2000000,2600000,
3300000,4100000,4900000,5700000,6600000,	/* 19 */
7500000,8400000,9300000,10300000,11300000,
12300000,13300000,14400000,15500000,16600000,	/* 29 */
17700000,18800000,19900000,21100000,22300000,	
23500000,24700000,25900000,27100000,28300000,	/* 39 */
29500000,30800000,32100000,33400000,34700000,
36000000,37300000,38600000,39900000,41200000,	/* 49 */
42600000,44000000,45400000,46800000,48200000,
49600000,51000000,52400000,53800000,55200000,	/* 59 */
56600000,58000000,59400000,60800000,62200000,
63700000,65200000,66700000,68200000,69700000,	/* 69 */
71200000,72700000,74200000,75700000,77200000,
78700000,80200000,81700000,83200000,84700000,	/* 79 */
86200000,87700000,89300000,90900000,92500000,
94100000,95700000,97300000,98900000,100500000,	/* 89 */
102100000,103700000,105300000,106900000,108500000,
110100000,111700000,113300000,114900000,116500000,	/* 99 */
118100000,119700000,121300000,122900000,124500000, 	
126100000,127700000,129300000,130900000,785400000,
1570800000,1570800000	/* 110 */
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
												op->name, who==NULL?"<null>":who->name);
		return;
	}

    /*LOG(llevDebug,"LEVEL: %s ob:%s l: %d e: %d\n", who==NULL?"<null>":who->name, op->name,op->level, op->stats.exp);*/
    if(op->level < MAXLEVEL && op->stats.exp >= (sint32)level_exp(op->level+1,1.0)) 
	{
		op->level++;

		if (op == who && op->stats.exp > 1 && is_dragon_pl(who))
			dragon_level_gain(who);
	
		
		if(who && who->type == PLAYER && op->type!=EXPERIENCE && op->type!=SKILL && who->level >1)
		{ 

			if(who->level > 2)
				who->contr->levhp[who->level]=(char)((RANDOM()%who->arch->clone.stats.maxhp)+1);
			else
				who->contr->levhp[who->level]=(char)who->arch->clone.stats.maxhp;
		}
		if(op->level>1 && op->type==EXPERIENCE) 
		{
			if(who && who->type == PLAYER)
			{
				if(op->stats.Pow) /* mana */
				{
					if(op->level > 2)
						who->contr->levsp[op->level]=(char)((RANDOM()%who->arch->clone.stats.maxsp)+1);
					else
						who->contr->levsp[op->level]=(char)who->arch->clone.stats.maxsp;
				}
				else if(op->stats.Wis) /* grace */
				{
					if(op->level > 2)
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
