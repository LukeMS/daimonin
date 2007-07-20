/*
    Daimonin, the Massive Multiuser Online Role Playing Game
    Server Applicatiom

    Copyright (C) 2001-2005 Michael Toennies

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

/* aggro.c
 * In this module we check and maintain the "aggro/damage history" object.
 * In that object the server stores damage and "aggressive action"
 * information done to a object (normally a non player, living object).
 * This information is used to determinate their "aggro". The target of their
 * aggression is usually the target for attacks or other action.
 * Because a aggressive act is often and most times damage, the aggro history
 * is also used for DOT & AE damage to control the right DoT (damage over time)
 * to have a defined DPS (damage per second).
 */
#ifndef __AGGRO_C
#define __AGGRO_C
#endif

#include <global.h>

static uint32 exp_calc_tag=1; /* used to tag the player/group */

/*
 *    TODO: Quick access of this structures by a pointer initialized for players in fix_player and placed
 *  in the player struct.
 */

/*
 *    TODO: The code don't handle pet damage itself - its redirected to players.
 *  i played around a bit but i think its better to use a special "pet damage"
 *  history and adding it at the same time to the owner. Using 2 values:
 *  one is the "real" damage we use for exp, one is the "counted" exp we use for aggro.
 */

/* aggro_get_damage()
 * Get a damage object from target for damage source hitter.
 * This is used for AoE spells and other target synchronized damage dealers
 * Return: NULL or damage info object for hitter.
 */
struct obj *aggro_get_damage(struct obj *target, struct obj *hitter)
{
    struct obj *tmp, *tmp2;
    /* damage objects are in a 2nd aggro_history object inside the base aggro history. */
    for(tmp=target->inv;tmp;tmp=tmp->below) /* get 1st container object */
    {
        if(tmp->type == TYPE_AGGRO_HISTORY )
        {
            for(tmp=tmp->inv;tmp;tmp=tmp->below)/* the special damage container - also AGGRO_HIST */
            {
                if(tmp->type == TYPE_AGGRO_HISTORY)
                {
                    for(tmp=tmp->inv;tmp;tmp=tmp2) /* here are the damage infos */
                    {
                        tmp2=tmp->below;
                        if(tmp->type == TYPE_DAMAGE_INFO)
                        {
                            if(tmp->weight_limit == hitter->weight_limit)
                                return tmp;
                            /* because we have this sucker accessed...lets do some gc on the fly */
                            if (tmp->damage_round_tag+DEFAULT_DMG_INVALID_TIME < ROUND_TAG)
                                remove_ob(tmp);
                        }
                    }
                    break;
                }
            }
            break;
        }
    }
    return tmp;
}

/*
 *
 */
struct obj *aggro_insert_damage(struct obj *target, struct obj *hitter)
{
    struct obj *tmp_t, *tmp;

    /* find or insert base container */
    for(tmp_t=target->inv;tmp_t;tmp_t=tmp_t->below)
    {
        if(tmp_t->type == TYPE_AGGRO_HISTORY )
            break;
    }
    if(!tmp_t)
        tmp_t = insert_ob_in_ob(arch_to_object(global_aggro_history_arch), target);

    /* find or insert 2nd damage info container */
    for(tmp=tmp_t->inv;tmp;tmp=tmp->below)
    {
        if(tmp->type == TYPE_AGGRO_HISTORY)
            break;
    }
    if(!tmp)
        tmp = insert_ob_in_ob(arch_to_object(global_aggro_history_arch),  tmp_t);

    tmp = insert_ob_in_ob(arch_to_object(global_dmg_info_arch), tmp);

    /* damage source is hitter and tick global_round_tag */
    tmp->weight_limit = hitter->weight_limit;
    /*LOG(-1,"add dmg: %x\n", hitter->weight_limit); */
    tmp->damage_round_tag = ROUND_TAG;

    return tmp;
}


/*
 * aggro_update_info()
 * This is the main setting function to update for a target the damage & aggro marker AFTER aggro and damage
 * is done from the hitter to the
 */
struct obj *aggro_update_info(struct obj *target, struct obj *target_owner,
                              struct obj *hitter, struct obj *hitter_owner, int dmg, int flags)
{
    struct obj *history = NULL, *aggro = NULL, *tmp = NULL;
    int skill_nr = 0;

    /* no legal hitter, no need for aggro.
     * TODO: perhaps we will add a kind of "neutral" damage
     * but atm a player will get full exp when killing a damaged target.
     * lets give the players some gifts.
     */

    if(hitter && hitter->chosen_skill)
        skill_nr = hitter->chosen_skill->stats.sp;
    else if(hitter_owner && hitter_owner->chosen_skill)
        skill_nr = hitter_owner->chosen_skill->stats.sp;

    /* debug...
    if(hitter && hitter->chosen_skill)
        new_draw_info_format( NDI_UNIQUE, 0, hitter->type!=PLAYER?hitter_owner:hitter, "SKILL-hitter: %s (%d)",
        skills[hitter->chosen_skill->stats.sp].name,hitter->chosen_skill->stats.sp);
    if(hitter_owner && hitter_owner->chosen_skill)
        new_draw_info_format( NDI_UNIQUE, 0, hitter->type!=PLAYER?hitter_owner:hitter, "SKILL-howner: %s (%d)",
        skills[hitter_owner->chosen_skill->stats.sp].name,hitter_owner->chosen_skill->stats.sp);
    */

    if(hitter_owner && (hitter_owner==hitter || !IS_LIVE(hitter_owner)))
        hitter_owner = NULL;
    if(hitter && !IS_LIVE(hitter))
        hitter = NULL;
    if(!hitter && !hitter_owner)
        return NULL;

    /* This is the interface to the AI system. At least for now =) */
    if(target->type == MONSTER)
    {
        /* Update hittee's friendship level towards hitter */
        object                 *root_hitter = hitter_owner ? hitter_owner : hitter;
        struct mob_known_obj   *enemy       = update_npc_knowledge(target, root_hitter, -dmg, 0);

        /* Attacking someone neutral always makes you an enemy (for now) */
        if (enemy && enemy->friendship > FRIENDSHIP_ATTACK && dmg > 0)
            enemy->friendship += FRIENDSHIP_ATTACK;
    }

    /* we only use aggro history for
     * a.) target is a player or IS_LIVE() (target_owner too)
     * b.) hitter and/or hitter_object is player/IS_LIVE
     */
    if(IS_LIVE(target))
    {

        /* get or create a aggro history container */
        for(history=target->inv;history;history=history->below)
        {
            if(history->type == TYPE_AGGRO_HISTORY)
                break;
        }
        if(!history)
            history = insert_ob_in_ob(arch_to_object(global_aggro_history_arch), target);

        /*
         *    TODO: hitter != NULL & hitter_owner != NULL
         *  thats means pet/golem and its owner.
         *  ATM we should have one ptr NULL.
         *  In any case, we use the owner.
         */
        if(hitter && hitter_owner)
            LOG(llevDebug,"TODO: hitter %s and owner %s passed aggro_update_check. Pet aggro handling not finished...\n", query_name(hitter), query_name(hitter_owner));
        if(hitter_owner)
            hitter=hitter_owner; /* TODO: change when we do pet/owner handling */

        if(hitter)
        {
            /* check for aggro history of this object */
            for(aggro=history->inv;aggro;aggro=tmp)
            {
                tmp=aggro->below;
                if(aggro->type == TYPE_DAMAGE_INFO)
                {
                    if(aggro->enemy_count == hitter->count)
                        break;
                    if(aggro->damage_round_tag+DEFAULT_DMG_INVALID_TIME < ROUND_TAG)
                        remove_ob(aggro);
                }
            }

            if(aggro)
            {
                aggro->stats.hp += dmg;
                /* TODO: set flags for some specials */
            }
            else
            {
                aggro = insert_ob_in_ob(arch_to_object(global_dmg_info_arch), history);
                aggro->enemy_count = hitter->count;    /* tag so we can identify the hitter */
                aggro->enemy = hitter;    /* so we can find later this damage dealer */
                aggro->stats.hp = dmg;
                aggro->update_tag = ROUND_TAG; /* using this we can determinate who does first damage */
                if(hitter->type == PLAYER)
                    aggro->last_sp = PLAYER;
            }
            aggro->damage_round_tag = ROUND_TAG; /* last time this hitter does some = right now */

            /* for players, we want to store the used skills */
            if(hitter->type == PLAYER)
            {
                for(tmp=aggro->inv;tmp;tmp=tmp->below)
                {
                    if(tmp->type == TYPE_DAMAGE_INFO && tmp->last_heal == skill_nr)
                        break;
                }
                if(!tmp)
                {
                    tmp = insert_ob_in_ob(arch_to_object(global_dmg_info_arch), aggro);
                    tmp->last_heal = skill_nr;
                    tmp->stats.hp = dmg;
                    tmp->last_sp = PLAYER;
                }
                else
                    tmp->stats.hp += dmg;
            }
        }

    }

    /* TODO - handle target_owner. Lets say we attack a pet - thats target.
     * Then we need to tell target_owner whats going on and mark hitter/hitter_object as
     * bad guy here.
     */

    /* TODO: init history container object with master aggro holder */

    return aggro;
}


/* calc active skill dmg.
 * Analyze the dmg done by skills and return the most used 1-3 skills.
 */
static inline void calc_active_skill_dmg(object *op, int *skill1, int *skill2, int *skill3)
{
    object *skilldmg;
    player *hitter = CONTR(op->enemy);
    int d1=0,d2=0,d3=0;

    *skill1 = *skill2 = *skill3 = -1;

    op->level = 0;
    for(skilldmg=op->inv;skilldmg;skilldmg=skilldmg->below)
    {
        if(! hitter->skill_ptr[skilldmg->last_heal])
            continue;

        /* we need that for base exp calc */
        if(op->level < hitter->skill_ptr[skilldmg->last_heal]->level)
            op->level = hitter->skill_ptr[skilldmg->last_heal]->level;


        if(*skill1==-1 || d1 <= skilldmg->stats.hp)
        {
            *skill3 = *skill2;
            d3 = d2;
            *skill2 = *skill1;
            d2 = d1;
            *skill1 = skilldmg->last_heal;
            d1 = skilldmg->stats.hp;
        }
        else if(*skill2==-1 || d2 <= skilldmg->stats.hp)
        {
            *skill3 = *skill2;
            d3 = d2;
            *skill2 = skilldmg->last_heal;
            d2 = skilldmg->stats.hp;
        }
        else if(*skill3==-1 || d3 <= skilldmg->stats.hp)
        {
            *skill3 = skilldmg->last_heal;
            d3 = skilldmg->stats.hp;
        }
#ifdef DEBUG_AGGRO
        LOG(-1,".. used skill %d --> dmg done: %d\n", skilldmg->last_heal, skilldmg->stats.hp );
#endif
    }
}

/* add aggro and tell the player about it */
static inline int add_aggro_exp(object *hitter, int exp, int skillnr)
{
    if (exp)
    {
        if(CONTR(hitter)->exp_bonus)
        {
            int exp_bonus = (int)(((double)exp/100.0)*(double)CONTR(hitter)->exp_bonus);
            new_draw_info_format(NDI_UNIQUE, 0,hitter, "You got %d (+%d bonus) exp in %s.",
                    add_exp(hitter, exp+exp_bonus, skillnr), exp_bonus, skills[skillnr].name);
        }
        else
        {
            new_draw_info_format(NDI_UNIQUE, 0,hitter, "You got %d exp in %s.",
                    add_exp(hitter, exp, skillnr), skills[skillnr].name);
        }
        return TRUE;
    }
    /*
     * that message was now given one time and only when no skill has >0 exp gain
    else
        new_draw_info_format( NDI_UNIQUE, 0, hitter, "Your enemy was to low for exp.");
    */
    return FALSE;
}

/* calc exp for a single player.
 */
static inline int aggro_exp_single(object *victim, object *aggro, int base)
{
    object *hitter = aggro->enemy, *tmp;
    player *pl = CONTR(hitter);
    int s1=-1,s2,s3, e1, e2, e3, ret=FALSE, exp=0;

    /* calc active dmg */
    calc_active_skill_dmg(aggro, &s1, &s2, &s3);

    /* check kill quests */
    if(pl->quests_type_kill && pl->quests_type_kill->inv)
        check_kill_quest_event(hitter, victim);

        /* thats important. EXP gain is related to the level of our highest used skill.
     * that exp is parted to the used skills. That *can* right used give alot more
     * exp as in the old style. Because the exp is counted BEFORE we calc the exp
     * max. cap. Killing higher mobs will give alot more total exp.
     */
    exp = base;
    calc_skill_exp(hitter, victim, 1.0f, aggro->level, &exp);
#ifdef DEBUG_AGGRO
    LOG(-1,".. A-Level: %d -> exp %d\n", aggro->level, exp);
#endif
    if(s1 == -1) /* we have not found any skill dmg - assign exp to guild base skills */
    {
#ifdef DEBUG_AGGRO
        LOG(-1,".. no skill dmg - use guild base dmg\n");
#endif
		new_draw_info(NDI_UNIQUE, 0, hitter, "You didn't fight this time.\nYou trained your default guild skills.");
        if((tmp = pl->highest_skill[pl->base_skill_group[0]]))
        {
            e1 = calc_skill_exp(hitter, victim, 0.55f, tmp->level, &exp);
            if(pl->base_skill_group_exp[0] != 100)
                e1 = (e1 * pl->base_skill_group_exp[0])/100;
            ret |=add_aggro_exp(hitter, e1, tmp->stats.sp);
        }
        if((tmp = pl->highest_skill[pl->base_skill_group[1]]))
        {
            e2 = calc_skill_exp(hitter, victim, 0.30f, tmp->level, &exp);
            if(pl->base_skill_group_exp[1] != 100)
                e2 = (e2 * pl->base_skill_group_exp[1])/100;
            ret |=add_aggro_exp(hitter, e2, tmp->stats.sp);
        }
        if((tmp = pl->highest_skill[pl->base_skill_group[2]]))
        {
            e3 = calc_skill_exp(hitter, victim, 0.15f, tmp->level, &exp);
            if(pl->base_skill_group_exp[2] != 100)
                e3 = (e3 * pl->base_skill_group_exp[2])/100;
            ret |=add_aggro_exp(hitter, e3, tmp->stats.sp);
        }
    }
    else if(s2 == -1) /* 100% exp in skill s1 */
    {
        e1 = calc_skill_exp(hitter, victim, 1.0f, pl->skill_ptr[s1]->level, &exp);
        if(pl->base_skill_group_exp[0] != 100)
            e1 = (e1 * pl->base_skill_group_exp[0])/100;
        ret |= add_aggro_exp(hitter, e1, s1);
#ifdef DEBUG_AGGRO
        LOG(-1,".. 100%% to skill %s (%d)\n", STRING_SAFE(pl->skill_ptr[s1]->name), e1);
#endif
    }
    else if(s3 == -1) /* 65% s1, 35% in s2 */
    {
        e1 = calc_skill_exp(hitter, victim, 0.65f, pl->skill_ptr[s1]->level, &exp);
        if(pl->base_skill_group_exp[0] != 100)
            e1 = (e1 * pl->base_skill_group_exp[0])/100;
        ret |=add_aggro_exp(hitter, e1, s1);
        e2 = calc_skill_exp(hitter, victim, 0.35f, pl->skill_ptr[s2]->level, &exp);
        if(pl->base_skill_group_exp[1] != 100)
            e2 = (e2 * pl->base_skill_group_exp[1])/100;
        ret |=add_aggro_exp(hitter, e2, s2);
#ifdef DEBUG_AGGRO
        LOG(-1,".. 65%% to skill %s (%d), 35%% to %s (%d)\n",
            STRING_SAFE(pl->skill_ptr[s1]->name), e1,
            STRING_SAFE(pl->skill_ptr[s2]->name), e2);
#endif
    }
    else /* 50% in s1, 30% in s2, 20% in s3 */
    {
        e1 = calc_skill_exp(hitter, victim, 0.55f, pl->skill_ptr[s1]->level, &exp);
        if(pl->base_skill_group_exp[0] != 100)
            e1 = (e1 * pl->base_skill_group_exp[0])/100;
        ret |=add_aggro_exp(hitter, e1, s1);
        e2 = calc_skill_exp(hitter, victim, 0.30f, pl->skill_ptr[s2]->level, &exp);
        if(pl->base_skill_group_exp[1] != 100)
            e2 = (e2 * pl->base_skill_group_exp[1])/100;
        ret |=add_aggro_exp(hitter, e2, s2);
        e3 = calc_skill_exp(hitter, victim, 0.15f, pl->skill_ptr[s3]->level, &exp);
        if(pl->base_skill_group_exp[2] != 100)
            e3 = (e3 * pl->base_skill_group_exp[2])/100;
        ret |=add_aggro_exp(hitter, e3, s3);
#ifdef DEBUG_AGGRO
        LOG(-1,".. 50%% to skill %s (%d), 30%% to %s (%d), 20%% to %s (%d)\n",
            STRING_SAFE(pl->skill_ptr[s1]->name),e1,
            STRING_SAFE(pl->skill_ptr[s2]->name),e2,
            STRING_SAFE(pl->skill_ptr[s3]->name),e3);
#endif
    }

    /* if *all* possible skill exp has been zero because mob was to low - drop a message */
    if(ret == FALSE)
        new_draw_info_format( NDI_UNIQUE, 0, hitter, "Your enemy was too low for exp.");

    return ret;
}


/* we test a group member in "in range" aka near the kill spot.
 * If the group member is not near, it gets no exp.
 * We avoid in this way multi spot farming
 */
static inline int in_group_exp_range(object *victim, object *hitter, object *member)
{
    int i;
    mapstruct *tmp_map, *map = member->map;

    /* we do 2 tests: Is member on the map or a DIRECT attached map
     * from victim or hitter? If not - no exp.
     */

    /* some sanity checks... */
    if(QUERY_FLAG(victim,FLAG_REMOVED) )
    {
        LOG(llevDebug,"in_group_exp_range(): victim %s is removed!\n", query_name(victim));
        return FALSE;
    }
    if(hitter && QUERY_FLAG(hitter,FLAG_REMOVED) ) /* secure... */
    {
        LOG(llevDebug,"in_group_exp_range(): hitter %s is removed!\n", query_name(hitter));
        return FALSE;
    }
    if(QUERY_FLAG(member,FLAG_REMOVED) ) /* secure... */
    {
        LOG(llevDebug,"in_group_exp_range(): member %s is removed!\n", query_name(member));
        return FALSE;
    }

    /* quick check the easiest cases */
    if (map == victim->map || (hitter && map == hitter->map))
    {
#ifdef DEBUG_AGGRO
        LOG(-1,"->%s on same map as victim/hitter!\n", query_name(member));
#endif
        return TRUE;
    }


    for (tmp_map = victim->map, i = 0; i < TILED_MAPS; i++)
    {
        if (tmp_map->tile_map[i] == map)
        {
#ifdef DEBUG_AGGRO
            LOG(-1,"->%s on attached map from victim!\n", query_name(member));
#endif
            return TRUE;
        }
    }

    if(hitter)
    {
        for (tmp_map = hitter->map, i = 0; i < TILED_MAPS; i++)
        {
            if (tmp_map->tile_map[i] == map)
            {
#ifdef DEBUG_AGGRO
                    LOG(-1,"->%s on attached map from hitter!\n", query_name(member));
#endif
                    return TRUE;
            }
        }
    }

#ifdef DEBUG_AGGRO
    LOG(-1,"->%s is out of range!\n", query_name(member));
#endif

	/* don't give this group member quest items from victim */
	CONTR(member)->group_status |= GROUP_STATUS_NOQUEST;
    return FALSE;
}


/* calc exp for a group
 * CONTR() will access here always players
 */
static inline int aggro_exp_group(object *victim, object *aggro, char *kill_msg)
{
    object *leader = CONTR(aggro->enemy)->group_leader;
    object *high = leader, *tmp, *member;
    int exp=0, t;

    /* here comes the bad news for group playing:
     * The maximal used exp is counted by the highest REAL level
     * of all group member.
     * As a reward for group playing, every member increase that
     * exp by 10% except the leader. So, a full group of 6 will
     * get 160% of the exp as when the highest player kills it
     * with highest skill.
     */
#ifdef DEBUG_AGGRO
    LOG(-1,".. GROUP:: " );
#endif
    /* first thing: we get the highest member */
    for(tmp=leader;tmp;tmp=CONTR(tmp)->group_next)
    {
        if(high->level < tmp->level)
            high = tmp;
    }


    /* lets calc the base exp for this kill. We will punish low chars
     * for killing low creatures with a high group member guarding/healing
     * them.
     */
    calc_skill_exp(high, victim, 1.0f, high->level, &exp);

    t=exp;
    /* adjust exp for nrof group members */
    exp = (int)((float)exp*(0.9f+(0.1f*(float) CONTR(leader)->group_nrof)));
#ifdef DEBUG_AGGRO
    LOG(-1," high member: %s (level %d)\n--> exp: %d (%d) --> member exp: %d\n", query_name(high), high->level, exp, t ,exp/CONTR(leader)->group_nrof);
#endif

	/* exp is 0 - one member used a to high skill to kill */
    if(!exp)
    {
		party_message(0,NDI_UNIQUE, 0, leader, NULL, "%s has too high level for exp.",query_name(high));

		/* No exp don't means no quests... So, we check it here - and we fake
		 * a in_group_exp_range() check, so we set NOEXP right for the quest trigger
		 * check when we drop the vicitim inventory (even when we skip normal
		 * loot with startequip flag)
		 */
		for(tmp=leader;tmp;tmp=CONTR(tmp)->group_next)
		{
			player *pl = CONTR(tmp);

			if(!in_group_exp_range(victim, aggro->enemy == tmp?NULL:aggro->enemy, tmp))
				pl->group_status |= GROUP_STATUS_NOQUEST; /* outside map range */
			else
				pl->group_status &= ~GROUP_STATUS_NOQUEST;

			/* check kill quests */
			if(pl->quests_type_kill && pl->quests_type_kill->inv)
				check_kill_quest_event(tmp, victim);
		}

		return FALSE;
    }

    exp /= CONTR(leader)->group_nrof;
    if(exp<4)
        exp =4; /* to have something senseful to part */
    /* at last ... part the exp to the group members */
    for(tmp=leader;tmp;tmp=CONTR(tmp)->group_next)
    {
        player *pl = CONTR(tmp);
        /* skip exp when we are not in range to victim/hitter.
         * mark group_status as NO_EXP - thats important and used
         * from the quest item function (= no quest item for leecher)
         */
#ifdef DEBUG_AGGRO
        LOG(-1,"GROUP_MEMBER: %s (%x)\n", query_name(tmp), pl);
#endif
        if(!in_group_exp_range(victim, aggro->enemy == tmp?NULL:aggro->enemy, tmp))
        {
            pl->group_status |= GROUP_STATUS_NOQUEST; /* mark tmp as loser and skip exp */
            continue;
        }

        /* We have have moved the kill message because we want it BEFORE
         * the exp gain messages... ugly, but well, better as double calc the exp gain
         */
        if(kill_msg && aggro->enemy != tmp)
            new_draw_info(NDI_YELLOW, 0, tmp, kill_msg);

        pl->group_status &= ~GROUP_STATUS_NOQUEST;
        if(pl->exp_calc_tag == exp_calc_tag)
        {
			/* aggo_exp_single() checks for check_kill_quest_event() */
            aggro_exp_single(victim, pl->exp_calc_obj, exp);
        }
        else /* this member has not done any dmg to the mob - assign exp to guild exp list */
        {
            int e, ret=0;
#ifdef DEBUG_AGGRO
            LOG(-1,".. no skill dmg - use guild base dmg\n");
#endif
			new_draw_info(NDI_UNIQUE, 0, tmp, "You didn't fight this time.\nYou trained your default guild skills.");

			/* check kill quests */
			if(pl->quests_type_kill && pl->quests_type_kill->inv)
				check_kill_quest_event(tmp, victim);

			if((member = pl->highest_skill[pl->base_skill_group[0]]))
            {
                e = calc_skill_exp(tmp, victim, 0.50f, member->level, &exp);
                if(pl->base_skill_group_exp[0] != 100)
                    e = (e * pl->base_skill_group_exp[0])/100;
                ret |=add_aggro_exp(tmp, e, member->stats.sp);
            }
            if((member = pl->highest_skill[pl->base_skill_group[1]]))
            {
                e = calc_skill_exp(tmp, victim, 0.30f, member->level, &exp);
                if(pl->base_skill_group_exp[1] != 100)
                    e = (e * pl->base_skill_group_exp[1])/100;
                ret |=add_aggro_exp(tmp, e, member->stats.sp);
            }
            if((member = pl->highest_skill[pl->base_skill_group[2]]))
            {
                e = calc_skill_exp(tmp, victim, 0.20f, member->level, &exp);
                if(pl->base_skill_group_exp[2] != 100)
                    e = (e * pl->base_skill_group_exp[2])/100;
                ret |=add_aggro_exp(tmp, e, member->stats.sp);
            }
        }
    }

    return TRUE;
#ifdef DEBUG_AGGRO
    LOG(-1,"\n" );
#endif
}


/*
 *  Analyze all aggro info in this object and give player exp basing on this info.
 *    Well, if we ever merge libcross.a with the server we should merge all in one exp.c module
 *  If slayer is != NULL we use it to determinate we have kill steal or a NPC kill.
 *  We decide here what we will do in that cases.
 *  Return: The corpse owner (NULL: There is no owner, target was to low, NPC kill...)
 */
object *aggro_calculate_exp(struct obj *victim, struct obj *slayer, char *kill_msg)
{
    object *tmp, *tmp2, *tmp3, *history, *highest_hitter=NULL;
    int ret, total_dmg=0,total_dmg_all=0, highest_dmg;

    /* slayer is not a player (if the kill hitter was a pet, slayer was set to owner) */
//    if(slayer && slayer->type != PLAYER)
//        return;

    /* TODO: don't give exp for player pets - lets add a "iam a pet" flag here later */

    for(history=victim->inv;history;history=history->below)
    {
        if(history->type == TYPE_AGGRO_HISTORY )
            break;
    }
    if(!history)
        return NULL;

    /* count damage, ignore non player damage.
     * we are fair - if helped us a NPC, we ignored
     * their damage here.
    */
    exp_calc_tag++; /* increase tag counter */

    /* TODO: lets look we have a kill steal here. */
    if(slayer)
    {
    }

    /* lets sort out every illegal damage which would count */
    for(tmp=history->inv;tmp;tmp=tmp2)
    {
        tmp2 = tmp->below;
        total_dmg_all += tmp->stats.hp;
        /* remove illegal enemy pointer and/or non player dmg */
        if(tmp->type != TYPE_DAMAGE_INFO || tmp->damage_round_tag+DEFAULT_DMG_INVALID_TIME < ROUND_TAG ||
                          !tmp->enemy || tmp->enemy->count!=tmp->enemy_count /*|| tmp->enemy->type!=PLAYER*/)
            remove_ob(tmp);
        else
            total_dmg += tmp->stats.hp;
    }
#ifdef DEBUG_AGGRO
    LOG(-1,"%s (%d) KILLED (%d). All dmg: %d  - player dmg: %d \n", query_name(victim), victim->count, history->stats.hp, total_dmg_all, total_dmg);
#endif
    highest_dmg = -1;
    /* now run through the dmg left and give all their share of the exp */
    while(history->inv)
    {
        tmp = history->inv;
#ifdef DEBUG_AGGRO
        LOG(-1,"--> %s [%s] (%x)--> dmg done: %d\n", query_name(tmp->enemy),tmp->last_sp == PLAYER?"player":"non player",tmp->enemy->count, tmp->stats.hp );
#endif
        if(tmp->enemy && tmp->enemy->type == PLAYER) /* player? */
        {
            if(CONTR(tmp->enemy)->group_id == GROUP_NO) /* single player? */
            {
                if(tmp->stats.hp > highest_dmg)
                {
                    highest_dmg = tmp->stats.hp;
                    highest_hitter = tmp;
                }
                remove_ob(tmp);
            }
            else /* group */
            {
                int tag = CONTR(tmp->enemy)->group_id, g_dmg=tmp->stats.hp;

                /* i *love* our object system */
                CONTR(tmp->enemy)->exp_calc_tag = exp_calc_tag; /* marks this group member as active for this exp gain */
                CONTR(tmp->enemy)->exp_calc_obj = tmp; /* thats the related aggro/dmg history object as shortcut */
                remove_ob(tmp);

                /* count grp dmg at once.
                 * we can safely remove the group members - we still can
                 * access them over the group links
                 */
                for(tmp3=history->inv;tmp3;tmp3=tmp2)
                {
                    tmp2 = tmp3->below;
                    if(CONTR(tmp3->enemy)->group_id == tag)
                    {
                        g_dmg+=tmp3->stats.hp;
                        CONTR(tmp3->enemy)->exp_calc_tag = exp_calc_tag;
                        CONTR(tmp3->enemy)->exp_calc_obj = tmp3;
                        remove_ob(tmp3);
                    }
                }

                if(g_dmg > highest_dmg)
                {
                    highest_dmg = g_dmg;
                    highest_hitter = tmp;
                }
            }
        }
        else /* non player */
        {
            if(tmp->stats.hp > highest_dmg)
            {
                highest_dmg = tmp->stats.hp;
                highest_hitter = tmp;
            }
            remove_ob(tmp);
        }
    }
    if(!highest_hitter) /* funny situation: A shot arrow to C, B killed A, arrow killed C - slayer A is dead = no highest_hitter */
        return NULL;

#ifdef DEBUG_AGGRO
    LOG(-1," -> highest_hitter: %s ", query_name(highest_hitter->enemy));
#endif

    /* we have a winner... highest_hitter is now a non player, single player or a group */
    if(!highest_hitter->enemy || highest_hitter->enemy->type!=PLAYER) /* NPC kill - no exp, no loot */
    {
#ifdef DEBUG_AGGRO
        LOG(-1,"--> NPC kill.\nend.\n");
#endif
        return highest_hitter->enemy;
    }

    if(CONTR(highest_hitter->enemy)->group_id != GROUP_NO)
        ret = aggro_exp_group(victim, highest_hitter, kill_msg);
    else
        ret = aggro_exp_single(victim, highest_hitter, -1);
#ifdef DEBUG_AGGRO
    LOG(-1,"end.\n");
#endif

    /* be sure not to drop items */
    if(ret == FALSE)
        SET_FLAG(victim, FLAG_STARTEQUIP);

    return highest_hitter->enemy; /* used to create the corpse bounty */
}
