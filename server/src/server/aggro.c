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

    The author can be reached via e-mail to info@daimonin.org
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
object_t *aggro_get_damage(object_t *target, object_t *hitter)
{
    object_t *this,
           *next;

    /* damage objects are in a 2nd aggro_history object inside the base aggro history. */
    FOREACH_OBJECT_IN_OBJECT(this, target, next)
    {
        if (this->type == TYPE_AGGRO_HISTORY)
        {
            FOREACH_OBJECT_IN_OBJECT(this, this, next)
            {
                if (this->type == TYPE_AGGRO_HISTORY)
                {
                    FOREACH_OBJECT_IN_OBJECT(this, this, next) /* here are the damage infos */
                    {
                        if (this->type == TYPE_DAMAGE_INFO)
                        {
                            if (this->weight_limit == hitter->weight_limit)
                            {
                                return this;
                            }
                            /* because we have this sucker accessed...lets do some gc on the fly */
                            else if (this->damage_round_tag + DEFAULT_DMG_INVALID_TIME < ROUND_TAG)
                            {
                                remove_ob(this);
                            }
                        }
                    }

                    break;
                }
            }

            break;
        }
    }

    return this;
}

/*
 *
 */
object_t *aggro_insert_damage(object_t *target, object_t *hitter)
{
    object_t *tmp_t,
           *next,
           *tmp;

    /* find or insert base container */
    FOREACH_OBJECT_IN_OBJECT(tmp_t, target, next)
    {
        if(tmp_t->type == TYPE_AGGRO_HISTORY )
            break;
    }

    if(!tmp_t)
        tmp_t = insert_ob_in_ob(arch_to_object(archetype_global._aggro_history), target);

    /* find or insert 2nd damage info container */
    FOREACH_OBJECT_IN_OBJECT(tmp, tmp_t, next)
    {
        if(tmp->type == TYPE_AGGRO_HISTORY)
            break;
    }

    if(!tmp)
        tmp = insert_ob_in_ob(arch_to_object(archetype_global._aggro_history),  tmp_t);

    tmp = insert_ob_in_ob(arch_to_object(archetype_global._dmg_info), tmp);

    /* damage source is hitter and tick global_round_tag */
    tmp->weight_limit = hitter->weight_limit;
    /*LOG(llevNoLog,"add dmg: %x\n", hitter->weight_limit); */
    tmp->damage_round_tag = ROUND_TAG;

    return tmp;
}


/*
 * aggro_update_info()
 * This is the main setting function to update for a target the damage & aggro marker AFTER aggro and damage
 * is done from the hitter to the
 */
object_t *aggro_update_info(object_t *target, object_t *hitter, object_t *hitter_owner, int dmg)
{
    object_t *history,
           *next,
           *aggro = NULL,
           *tmp;
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
        ndi( NDI_UNIQUE, 0, hitter->type!=PLAYER?hitter_owner:hitter, "SKILL-hitter: %s (%d)",
        hitter->chosen_skill->name, hitter->chosen_skill->stats.sp);
    if(hitter_owner && hitter_owner->chosen_skill)
        ndi( NDI_UNIQUE, 0, hitter->type!=PLAYER?hitter_owner:hitter, "SKILL-howner: %s (%d)",
        hitter_owner->chosen_skill->name, hitter_owner->chosen_skill->stats.sp);
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
        object_t                 *root_hitter = hitter_owner ? hitter_owner : hitter;
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
        FOREACH_OBJECT_IN_OBJECT(history, target, next)
        {
            if(history->type == TYPE_AGGRO_HISTORY)
                break;
        }
        if(!history)
            history = insert_ob_in_ob(arch_to_object(archetype_global._aggro_history), target);

        /*
         *    TODO: hitter != NULL & hitter_owner != NULL
         *  thats means pet/golem and its owner.
         *  ATM we should have one ptr NULL.
         *  In any case, we use the owner.
         */
        if(hitter && hitter_owner)
            LOG(llevDebug,"TODO: hitter %s and owner %s passed aggro_update_check. Pet aggro handling not finished...\n", STRING_OBJ_NAME(hitter), STRING_OBJ_NAME(hitter_owner));
        if(hitter_owner)
            hitter=hitter_owner; /* TODO: change when we do pet/owner handling */

        if(hitter)
        {
            /* check for aggro history of this object_t */
            FOREACH_OBJECT_IN_OBJECT(aggro, history, next)
            {
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
                aggro = insert_ob_in_ob(arch_to_object(archetype_global._dmg_info), history);
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
                FOREACH_OBJECT_IN_OBJECT(tmp, aggro, next)
                {
                    if(tmp->type == TYPE_DAMAGE_INFO && tmp->last_heal == skill_nr)
                        break;
                }
                if(!tmp)
                {
                    tmp = insert_ob_in_ob(arch_to_object(archetype_global._dmg_info), aggro);
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
static inline void calc_active_skill_dmg(object_t *op, int *skill1, int *skill2, int *skill3)
{
    object_t *skilldmg,
           *next;
    player_t *hitter = CONTR(op->enemy);
    int d1=0,d2=0,d3=0;

    *skill1 = *skill2 = *skill3 = -1;
    op->level = 0;

    FOREACH_OBJECT_IN_OBJECT(skilldmg, op, next)
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
        LOG(llevNoLog,".. used skill %d --> dmg done: %d\n", skilldmg->last_heal, skilldmg->stats.hp );
#endif
    }
}

static int give_default_guild_exp(player_t *pl, int base_exp)
{
    object_t *skill;
    int e1 = 0;
    int e2 = 0;
    int e3 = 0;

    if (pl->base_skill_group[0] >= 0 &&
        (skill = pl->highest_skill[pl->base_skill_group[0]]) &&
        skill->level > 1)
    {
        e1 = (int) ((float) base_exp * 0.55f);
        e1 = exp_from_base_skill(pl, e1, skill->stats.sp);
        add_exp(pl->ob, e1, skill->stats.sp, 1);
    }

    if (pl->base_skill_group[1] >= 0 &&
        (skill = pl->highest_skill[pl->base_skill_group[1]]) &&
        skill->level > 1)
    {
        e2 = (int) ((float) base_exp * 0.30f);
        e2 = exp_from_base_skill(pl, e2, skill->stats.sp);
        add_exp(pl->ob, e2, skill->stats.sp, 1);
    }

    if (pl->base_skill_group[2] >= 0 &&
        (skill = pl->highest_skill[pl->base_skill_group[2]]) &&
        skill->level > 1)
    {
        e3 = (int) ((float) base_exp * 0.15f);
        e3 = exp_from_base_skill(pl, e3, skill->stats.sp);
        add_exp(pl->ob, e3, skill->stats.sp, 1);
    }

    if (e1 > 0 || e2 > 0 || e3 > 0)
    {
        ndi(NDI_UNIQUE | NDI_WHITE, 0, pl->ob, "You didn't fight this time.\nYou trained your default guild skills.");
    }

    return 1;
}

/* calc exp for a single player.
 */
static inline int aggro_exp_single(object_t *victim, object_t *aggro, int base)
{
    object_t *hitter = aggro->enemy;
    player_t *pl = CONTR(hitter);
    int s1=-1,s2,s3, e1 = 0, e2, e3, exp=0;

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
    LOG(llevNoLog,".. A-Level: %d -> exp %d\n", aggro->level, exp);
#endif
    if(s1 == -1) /* we have not found any skill dmg - assign exp to guild base skills */
    {
        e1 = give_default_guild_exp(pl, exp);
    }
    else if(s2 == -1) /* 100% exp in skill s1 */
    {
        e1 = calc_skill_exp(hitter, victim, 1.0f, pl->skill_ptr[s1]->level, &exp);
        e1 = exp_from_base_skill(pl, e1, s1);
        (void)add_exp(hitter, e1, s1, 1);
#ifdef DEBUG_AGGRO
        LOG(llevNoLog,".. 100%% to skill %s (%d)\n", STRING_SAFE(pl->skill_ptr[s1]->name), e1);
#endif
    }
    else if(s3 == -1) /* 65% s1, 35% in s2 */
    {
        e1 = calc_skill_exp(hitter, victim, 0.65f, pl->skill_ptr[s1]->level, &exp);
        e1 = exp_from_base_skill(pl, e1, s1);
        e2 = calc_skill_exp(hitter, victim, 0.35f, pl->skill_ptr[s2]->level, &exp);
        e2 = exp_from_base_skill(pl, e2, s2);
        (void)add_exp(hitter, e1, s1, 1);
        (void)add_exp(hitter, e2, s2, 1);

#ifdef DEBUG_AGGRO
        LOG(llevNoLog,".. 65%% to skill %s (%d), 35%% to %s (%d)\n",
            STRING_SAFE(pl->skill_ptr[s1]->name), e1,
            STRING_SAFE(pl->skill_ptr[s2]->name), e2);
#endif
    }
    else /* 55% in s1, 30% in s2, 15% in s3 */
    {

        e1 = calc_skill_exp(hitter, victim, 0.55f, pl->skill_ptr[s1]->level, &exp);
        e1 = exp_from_base_skill(pl, e1, s1);
        e2 = calc_skill_exp(hitter, victim, 0.30f, pl->skill_ptr[s2]->level, &exp);
        e2 = exp_from_base_skill(pl, e2, s2);
        e3 = calc_skill_exp(hitter, victim, 0.30f, pl->skill_ptr[s3]->level, &exp);
        e3 = exp_from_base_skill(pl, e3, s3);
        (void)add_exp(hitter, e1, s1, 1);
        (void)add_exp(hitter, e2, s2, 1);
        (void)add_exp(hitter, e3, s3, 1);

#ifdef DEBUG_AGGRO
        LOG(llevNoLog,".. 50%% to skill %s (%d), 30%% to %s (%d), 20%% to %s (%d)\n",
            STRING_SAFE(pl->skill_ptr[s1]->name),e1,
            STRING_SAFE(pl->skill_ptr[s2]->name),e2,
            STRING_SAFE(pl->skill_ptr[s3]->name),e3);
#endif
    }

    /* if *all* possible skill exp has been zero because mob was to low - drop a message */
    if (!e1)
    {
        ndi(NDI_UNIQUE | NDI_GREY, 0, hitter, "Your enemy was too low for exp.");
    }

    return e1;
}


/* we test a group member in "in range" aka near the kill spot.
 * If the group member is not near, it gets no exp.
 * We avoid in this way multi spot farming
 */
static inline int in_group_exp_range(object_t *victim, object_t *hitter, object_t *member)
{
    int i;
    map_t *tmp_map, *map = member->map;

    /* we do 2 tests: Is member on the map or a DIRECT attached map
     * from victim or hitter? If not - no exp.
     */

    /* some sanity checks... */
    if(QUERY_FLAG(victim,FLAG_REMOVED) )
    {
        LOG(llevDebug,"in_group_exp_range(): victim %s is removed!\n", STRING_OBJ_NAME(victim));
        return 0;
    }
    if(hitter && QUERY_FLAG(hitter,FLAG_REMOVED) ) /* secure... */
    {
        LOG(llevDebug,"in_group_exp_range(): hitter %s is removed!\n", STRING_OBJ_NAME(hitter));
        return 0;
    }
    if(QUERY_FLAG(member,FLAG_REMOVED) ) /* secure... */
    {
        LOG(llevDebug,"in_group_exp_range(): member %s is removed!\n", STRING_OBJ_NAME(member));
        return 0;
    }

    /* quick check the easiest cases */
    if (map == victim->map || (hitter && map == hitter->map))
    {
#ifdef DEBUG_AGGRO
        LOG(llevNoLog,"->%s on same map as victim/hitter!\n", STRING_OBJ_NAME(member));
#endif
        return 1;
    }


    for (tmp_map = victim->map, i = 0; i < TILING_DIRECTION_NROF; i++)
    {
        if (tmp_map->tiling.tile_map[i] == map)
        {
#ifdef DEBUG_AGGRO
            LOG(llevNoLog,"->%s on attached map from victim!\n", STRING_OBJ_NAME(member));
#endif
            return 1;
        }
    }

    if(hitter)
    {
        for (tmp_map = hitter->map, i = 0; i < TILING_DIRECTION_NROF; i++)
        {
            if (tmp_map->tiling.tile_map[i] == map)
            {
#ifdef DEBUG_AGGRO
                    LOG(llevNoLog,"->%s on attached map from hitter!\n", STRING_OBJ_NAME(member));
#endif
                    return 1;
            }
        }
    }

#ifdef DEBUG_AGGRO
    LOG(llevNoLog,"->%s is out of range!\n", STRING_OBJ_NAME(member));
#endif

    /* don't give this group member quest items from victim */
    CONTR(member)->group_status |= GROUP_STATUS_NOQUEST;
    return 0;
}


/* calc exp for a group
 * CONTR() will access here always players
 */
static inline int aggro_exp_group(object_t *victim, object_t *aggro)
{
    object_t *leader = CONTR(aggro->enemy)->group_leader;
    object_t *high = leader, *tmp;
    int exp=0, t;
    object_t *force;
    int tmp_drain_level = 0, high_drain_level = 0;

    /* here comes the bad news for group playing:
     * The maximal used exp is counted by the highest REAL level
     * of all group member.
     * As a reward for group playing, every member increase that
     * exp by 10% except the leader. So, a full group of 6 will
     * get 160% of the exp as when the highest player kills it
     * with highest skill.
     */
#ifdef DEBUG_AGGRO
    LOG(llevNoLog,".. GROUP:: " );
#endif
    /* first thing: we get the highest member */
    for(tmp=leader;tmp;tmp=CONTR(tmp)->group_next)
    {
        /* we need to take drain into account before we assume that is the level of the player.
         * it's probably not the most efficient way of doing it as calculations involving drain
         * should only be used with stats instead of drain affecting level system-wide.
         * however, this is an emergency bugfix, so it's better just to fix the bug first... */
        if ((force = present_arch_in_ob(archetype_global._drain, tmp)))
            tmp_drain_level = force->level;

        if(high->level + high_drain_level < tmp->level + tmp_drain_level)
        {
            high = tmp;
            high_drain_level = tmp_drain_level;
        }
    }


    /* lets calc the base exp for this kill. We will punish low chars
     * for killing low creatures with a high group member guarding/healing
     * them.
     */
    calc_skill_exp(high, victim, 1.0f, high->level + high_drain_level, &exp);

    t=exp;
    /* adjust exp for nrof group members */
    exp = (int)((float)exp*(0.9f+(0.1f*(float) CONTR(leader)->group_nrof)));
#ifdef DEBUG_AGGRO
    LOG(llevNoLog," high member: %s (level %d)\n--> exp: %d (%d) --> member exp: %d\n", STRING_OBJ_NAME(high), high->level + high_drain_level, exp, t ,exp/CONTR(leader)->group_nrof);
#endif

    /* exp is 0 - one member used a to high skill to kill */
    if(!exp)
    {
        party_message(0, NDI_UNIQUE | NDI_GREY, 0, leader, NULL, "%s has too high level for exp.",
            QUERY_SHORT_NAME(high, NULL));

        /* No exp don't means no quests... So, we check it here - and we fake
         * a in_group_exp_range() check, so we set NOEXP right for the quest trigger
         * check when we drop the victim inventory */
        for(tmp=leader;tmp;tmp=CONTR(tmp)->group_next)
        {
            player_t *pl = CONTR(tmp);

            if(!in_group_exp_range(victim, aggro->enemy == tmp?NULL:aggro->enemy, tmp))
                pl->group_status |= GROUP_STATUS_NOQUEST; /* outside map range */
            else
                pl->group_status &= ~GROUP_STATUS_NOQUEST;

            /* check kill quests */
            if(pl->quests_type_kill && pl->quests_type_kill->inv)
                check_kill_quest_event(tmp, victim);
        }

        return 0;
    }

    exp /= CONTR(leader)->group_nrof;
    if(exp<4)
        exp =4; /* to have something senseful to part */
    /* at last ... part the exp to the group members */
    for(tmp=leader;tmp;tmp=CONTR(tmp)->group_next)
    {
        player_t *pl = CONTR(tmp);
        /* skip exp when we are not in range to victim/hitter.
         * mark group_status as NO_EXP - thats important and used
         * from the quest item function (= no quest item for leecher)
         */
#ifdef DEBUG_AGGRO
        LOG(llevNoLog,"GROUP_MEMBER: %s (%p)\n", STRING_OBJ_NAME(tmp), pl);
#endif
        if(!in_group_exp_range(victim, aggro->enemy == tmp?NULL:aggro->enemy, tmp))
        {
            pl->group_status |= GROUP_STATUS_NOQUEST; /* mark tmp as loser and skip exp */
            continue;
        }

        pl->group_status &= ~GROUP_STATUS_NOQUEST;

       if(pl->quests_type_kill && pl->quests_type_kill->inv)
            check_kill_quest_event(tmp, victim);

        // If the pl has not contributed damage to the slaughter of this mob, only give them 75% exp.
        if(pl->exp_calc_tag == exp_calc_tag)
            aggro_exp_single(victim, pl->exp_calc_obj, exp);
        else
            give_default_guild_exp(pl, (int)((float)exp * 0.75f));

    }

    return 1;
#ifdef DEBUG_AGGRO
    LOG(llevNoLog,"\n" );
#endif
}


/*
 *  Analyze all aggro info in this object and give player exp basing on this info.
 *    Well, if we ever merge libcross.a with the server we should merge all in one exp.c module
 *  If slayer is != NULL we use it to determinate we have kill steal or a NPC kill.
 *  We decide here what we will do in that cases.
 *  Return: The corpse owner (NULL: There is no owner, target was to low, NPC kill...)
 */
object_t *aggro_calculate_exp(object_t *victim, object_t *slayer)
{
    object_t *tmp,
           *tmp3,
           *history,
           *next,
           *highest_hitter = NULL;
    int ret, total_dmg=0,total_dmg_all=0, highest_dmg;

    /* slayer is not a player (if the kill hitter was a pet, slayer was set to owner) */
//    if(slayer && slayer->type != PLAYER)
//        return;

    /* TODO: don't give exp for player pets - lets add a "iam a pet" flag here later */

    FOREACH_OBJECT_IN_OBJECT(history, victim, next)
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
    FOREACH_OBJECT_IN_OBJECT(tmp, history, next)
    {
        total_dmg_all += tmp->stats.hp;
        /* remove illegal enemy pointer and/or non player dmg */
        if(tmp->type != TYPE_DAMAGE_INFO || tmp->damage_round_tag+DEFAULT_DMG_INVALID_TIME < ROUND_TAG ||
                          !tmp->enemy || tmp->enemy->count!=tmp->enemy_count /*|| tmp->enemy->type!=PLAYER*/)
            remove_ob(tmp);
        else
            total_dmg += tmp->stats.hp;
    }
#ifdef DEBUG_AGGRO
    LOG(llevNoLog,"%s (%d) KILLED (%d). All dmg: %d  - player dmg: %d \n", STRING_OBJ_NAME(victim), victim->count, history->stats.hp, total_dmg_all, total_dmg);
#endif
    highest_dmg = -1;
    /* now run through the dmg left and give all their share of the exp */
    while(history->inv)
    {
        tmp = history->inv;
#ifdef DEBUG_AGGRO
        LOG(llevNoLog,"--> %s [%s] (%x)--> dmg done: %d\n", STRING_OBJ_NAME(tmp->enemy),tmp->last_sp == PLAYER?"player":"non player",tmp->enemy->count, tmp->stats.hp );
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
                FOREACH_OBJECT_IN_OBJECT(tmp3, history, next)
                {
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
    LOG(llevNoLog," -> highest_hitter: %s ", STRING_OBJ_NAME(highest_hitter->enemy));
#endif

    /* we have a winner... highest_hitter is now a non player, single player or a group */
    if(!highest_hitter->enemy || highest_hitter->enemy->type!=PLAYER) /* NPC kill - no exp, no loot */
    {
#ifdef DEBUG_AGGRO
        LOG(llevNoLog,"--> NPC kill.\nend.\n");
#endif
        return highest_hitter->enemy;
    }

    if(CONTR(highest_hitter->enemy)->group_status & GROUP_STATUS_GROUP)
        ret = aggro_exp_group(victim, highest_hitter);
    else
        ret = aggro_exp_single(victim, highest_hitter, -1);
#ifdef DEBUG_AGGRO
    LOG(llevNoLog,"end.\n");
#endif

    /* be sure not to drop items */
    if(ret == 0)
        SET_FLAG(victim, FLAG_NO_DROP);

    return highest_hitter->enemy; /* used to create the corpse bounty */
}
