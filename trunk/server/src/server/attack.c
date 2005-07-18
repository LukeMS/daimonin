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

/* this is our "attackform to protection" table.
   it maps 32+ attack forms to our ~20 protections */
int protection_tab[NROFATTACKS] =
{
    PROTECT_PHYSICAL, 10, 5, 7, 6, 11, 9, 11, 4, 17, 8, 12, 12,   /* drain to mind and ghosthit to death... */
    18, 11, 10, 15, 17, 16, /* turn undead and godpower to holy - thats the hard one */
    10, 18, 14, 12, 18, 13, /* holy word to energy (pure grace power) and life steal is psionic */
    1, 2, 3, 15, 14, 19, 13  /* internal to holy... just a joke, we never use this entry */
};

#define ATTACK_HIT_DAMAGE(_op, _anum)       dam=dam*((double)_op->attack[_anum]*(double)0.01);dam>=1.0f?(damage=(int)dam):(damage=1)
#define ATTACK_RESIST_DAMAGE(_op, _anum)    dam=dam*((double)(100-_op->resist[_anum])*(double)0.01)
#define ATTACK_PROTECT_DAMAGE(_op, _anum)    dam=dam*((double)(100-_op->protection[protection_tab[_anum]])*(double)0.01)

/*#define ATTACK_DEBUG*/

/* some static defines */
static void thrown_item_effect(object *, object *);
static int  get_attack_mode(object **target, object **hitter, int *simple_attack);
static int  abort_attack(object *target, object *hitter, int simple_attack);
static int  attack_ob_simple(object *op, object *hitter, int base_dam, int base_wc);
static void send_attack_msg(object *op, object *hitter, int attacknum, int dam, int damage);


/* if we attack something, we start here.
 * this function just checks for right head part of possible multi arches,
 * and selects the right dam/wc
 * for special events, we can assign different dam/wc as coming on default from
 * object - in this cases, attack_ob_simple() is called from them internal.
 */
int attack_ob(object *op, object *hitter)
{
    if (op->head)
        op = op->head;
    if (hitter->head)
        hitter = hitter->head;
    return attack_ob_simple(op, hitter, hitter->stats.dam, hitter->stats.wc);
}

/* attack_ob_simple()
 * here we decide a attack will happen and how. blocking, parry, missing is handled
 * here inclusive the sounds. All whats needed before we count damage and say "hit you".
 */
static int attack_ob_simple(object *op, object *hitter, int base_dam, int base_wc)
{
    int     simple_attack, roll, dam = 0;
    uint32  type;
    tag_t   op_tag, hitter_tag;

    if (op->head)
        op = op->head;
    if (hitter->head)
        hitter = hitter->head;

    if (get_attack_mode(&op, &hitter, &simple_attack))
        goto error;

    if(trigger_object_plugin_event(EVENT_ATTACK, 
            op, hitter, hitter, NULL, NULL, NULL, NULL, SCRIPT_FIX_ALL))
        goto error;

    op_tag = op->count;
    hitter_tag = hitter->count;

    roll = random_roll(0, 20, hitter, PREFER_HIGH);

    /* Adjust roll for various situations. */
    if (!simple_attack)
        roll += adj_attackroll(hitter, op);

    if (hitter->type == PLAYER)
        CONTR(hitter)->anim_flags |= PLAYER_AFLAG_ENEMY; /* so we do one swing */

    /* Force player to face enemy */
    if (hitter->type == PLAYER)
    {
        rv_vector   dir;
        if(get_rangevector(hitter, op, &dir, RV_NO_DISTANCE))
        {
            if (hitter->head) 
            {
                hitter->head->anim_enemy_dir = dir.direction;
                hitter->head->facing = dir.direction;
            } 
            else 
            {
                hitter->anim_enemy_dir = dir.direction;
                hitter->facing = dir.direction;
            }
        }
    }

#if 0/* attack timing test */
    if(op->type ==PLAYER)
    {
        struct timeval time_Info;
        char buf[256];
        static long msec=1;
        gettimeofday(&time_Info, NULL);
        sprintf(buf,"monster %s swings: %f",hitter->name, (float)((double)(time_Info.tv_usec-msec)/(double)1000000));
        new_draw_info(NDI_UNIQUE, 0,op, buf);
        msec = time_Info.tv_usec;
    }

    if(hitter->type ==PLAYER)
    {
        struct timeval time_Info;
        char buf[256];
        static long msec=1;
        gettimeofday(&time_Info, NULL);
        sprintf(buf,"player %s swings: %f", hitter->name, (float)((double)(time_Info.tv_usec-msec)/(double)1000000));
        new_draw_info(NDI_UNIQUE, 0,hitter, buf);
        msec = time_Info.tv_usec;
    }
#endif

    /* See if we hit the creature */
    if (roll >= 20 || op->stats.ac <= base_wc + roll)
    {
        int hitdam  = base_dam;

        CLEAR_FLAG(op, FLAG_SLEEP); /* at this point NO ONE will still sleep */

        /* i don't use sub_type atm - using it should be smarter i the future */
        if (hitter->type == ARROW)
            play_sound_map(hitter->map, hitter->x, hitter->y, SOUND_ARROW_HIT, SOUND_NORMAL);
        else
        {
            if (hitter->attack[ATNR_SLASH])
                play_sound_map(hitter->map, hitter->x, hitter->y, SOUND_HIT_SLASH, SOUND_NORMAL);
            else if (hitter->attack[ATNR_CLEAVE])
                play_sound_map(hitter->map, hitter->x, hitter->y, SOUND_HIT_CLEAVE, SOUND_NORMAL);
            else if (hitter->attack[ATNR_PHYSICAL])
                play_sound_map(hitter->map, hitter->x, hitter->y, SOUND_HIT_IMPACT, SOUND_NORMAL);
            else
                play_sound_map(hitter->map, hitter->x, hitter->y, SOUND_HIT_PIERCE, SOUND_NORMAL);
        }

        if (!simple_attack)
        {
            /* A NPC call for help - this should be part of AI
                     if (op->type != PLAYER && ! can_see_enemy (op, hitter)
                         && ! get_owner (op) && rndm(0, op->stats.Int))
                         npc_call_help (op);
                    */
            /* old HIDE code
                     if (op->hide && QUERY_FLAG (hitter, FLAG_ALIVE)) {
                         make_visible (op);
                         if (op->type == PLAYER)
                             new_draw_info (NDI_UNIQUE, 0, op,
                                            "You were hit by a wild attack. "
                                            "You are no longer hidden!");
                     }
                    */

            /* thrown items (hitter) will have various effects
             * when they hit the victim.  For things like thrown daggers,
             * this sets 'hitter' to the actual dagger, and not the
             * wrapper object.
             */
            thrown_item_effect(hitter, op);
            if (was_destroyed(hitter, hitter_tag)
             || was_destroyed(op, op_tag)
             || abort_attack(op, hitter, simple_attack))
                goto leave;
        }

        /* Need to do at least 1 damage, otherwise there is no point
            * to go further and it will cause FPE's below.
            */
        if (hitdam <= 0)
            hitdam = 1;

        /* attacktype will be removed !*/
        type = hitter->attacktype;
        if (!type)
            type = AT_PHYSICAL;

        /* Handle monsters that hit back */
        if (!simple_attack && QUERY_FLAG(op, FLAG_HITBACK) && IS_LIVE(hitter))
        {
            hit_player(hitter, random_roll(0, (op->stats.dam), hitter, PREFER_LOW), op, op->attacktype);
            if (was_destroyed(op, op_tag)
             || was_destroyed(hitter, hitter_tag)
             || abort_attack(op, hitter, simple_attack))
                goto leave;
        }

        /* In the new attack code, it should handle multiple attack
         * types in its area, so remove it from here.
            * i increased dmg output ... from 1 to max to 50% to max.
         */
        dam = hit_player(op, random_roll(hitdam / 2 + 1, hitdam, hitter, PREFER_HIGH), hitter, type);
        if (was_destroyed(op, op_tag) || was_destroyed(hitter, hitter_tag) || abort_attack(op, hitter, simple_attack))
            goto leave;
    } /* end of if hitter hit op */
    /* if we missed, dam=0 */
    else
    {
#ifdef ATTACK_TIMING_DEBUG
        {
            char    buf[256];
            sprintf(buf, "MISS %s :%d>=%d-%d l:%d aw:%d/%d d:%d/%d h:%d/%d (%d)", hitter->name, op->stats.ac, base_wc,
                    roll, hitter->level, hitter->stats.ac, hitter->stats.wc, hitter->stats.dam_adj, hitter->stats.dam,
                    hitter->stats.hp, hitter->stats.maxhp_adj, op->stats.hp);
            new_draw_info(NDI_ALL, 1, NULL, buf);
            /*
            sprintf(buf,"MISS! attacker %s:: roll:%d ac:%d wc:%d dam:%d", hitter->name, roll,hitter->stats.ac,hitter->stats.wc, hitter->stats.dam);
            new_draw_info(NDI_UNIQUE, 0,hitter, buf);
            sprintf(buf,"defender %s:: ac:%d wc:%d dam:%d", op->name, op->stats.ac,op->stats.wc, op->stats.dam);
            new_draw_info(NDI_UNIQUE, 0,hitter, buf);
            */
        }
#endif
        if (hitter->type != ARROW)
        {
            if (hitter->type == PLAYER)
                play_sound_map(hitter->map, hitter->x, hitter->y, SOUND_MISS_PLAYER, SOUND_NORMAL);
            else
                play_sound_map(hitter->map, hitter->x, hitter->y, SOUND_MISS_MOB, SOUND_NORMAL);
        }
    }

    goto leave;

    error : dam = 1;
    goto leave;

    leave : return dam;
}

/* This isn't used just for players, but in fact most objects.
 * op is the object to be hit, dam is the amount of damage, hitter
 * is what is hitting the object, and type is the attacktype.
 * dam is base damage - protections/vulnerabilities/slaying matches can
 * modify it.
 */

/* 05.2002: I rewroted this in major parts. I removed much of the to
   complex or redundant stuff. Most of the problems here should be handled outside.
   */
int hit_player(object *op, int dam, object *hitter, int type)
{
    object *hit_obj, *target_obj, *aggro_obj, *dmg_obj=NULL;
    int     maxdam      = 0;
    int     attacknum, hit_level;
    int     simple_attack;
    tag_t   op_tag, hitter_tag;
    int     rtn_kill    = 0;

    /* if our target has no_damage 1 set or is wiz, we can't hurt him */
    if (QUERY_FLAG(op, FLAG_WIZ) || QUERY_FLAG(op, FLAG_INVULNERABLE))
        return 0;

    if (hitter->head)
        hitter = hitter->head;
    if (op->head)
        op = op->head;

    /* now we are nasty: we add some "level boni" - in small words: If a high level
     * object hits a lower level object it becomes a level boni depending on the
     * level difference!
     * This will make out of level range fight ALOT harder.
     */

    /* TODO:i must fix here a bit for scrolls, rods, wands and other fix level stuff! */
    /* the trick: we get for the TARGET the real level - even for player.
     * but for hitter, we always use the SKILL LEVEL if player!
     */
    if (!(hit_obj = get_owner(hitter)))
        hit_obj = hitter;
    if (!(target_obj = get_owner(op)))
        target_obj = op;

    if (hit_obj->type == PLAYER)
        hit_level = SK_level(hit_obj); /* get from hitter object the right skill level! */
    else
        hit_level = hitter->level;

    if (hit_level == 0 || target_obj->level == 0) /* very useful sanity check! */
        LOG(llevDebug,
            "DEBUG: hit_player(): hit or target object level == 0(h:>%s< (o:>%s<) l->%d t:>%s< (>%s<)(o:>%s<) l->%d\n",
            query_name(hitter), query_name(get_owner(hitter)), hit_level, query_name(op), target_obj->arch->name,
            query_name(get_owner(op)), target_obj->level);

    /* this is hard cut - perhaps we do something more smart later.
     * no friendly object take damage from player now.
     */
    if (hit_obj->type == PLAYER && QUERY_FLAG(target_obj, FLAG_FRIENDLY))
        return 0;
    if (QUERY_FLAG(hit_obj, FLAG_FRIENDLY) && (target_obj->type == PLAYER || QUERY_FLAG(target_obj, FLAG_FRIENDLY)))
        return 0;

    if (hit_level > target_obj->level && hit_obj->type == MONSTER) /* i turned it now off for players! */
    {
        dam += (int)
               ((float)
                (dam / 2) * ((float) (hit_level - target_obj->level)
                           / (target_obj->level > 25 ? 25.0f : (float) target_obj->level)));
        /*LOG(llevDebug,"DMG-ADD: hl:%d tl_%d -> d:%d \n", hit_level,target_obj->level, dam);*/
    }
    /* something hit player (can be disease or poison too - break praying! */
    if (op->type == PLAYER && CONTR(op)->was_praying)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Your praying is disrupted.");
        CONTR(op)->praying = 0;
        CONTR(op)->was_praying = 0;
    }



    /* Check for pvp! Only when at THIS moment both possible player are in pvp area - then we do damage.
     * This avoid this kind of heros, standing on pvp border, firing in and running back to save.
     * on the other side, running in safe areas will help you when hunted - and thats always a great fun.
     */

    if (op->type == PLAYER || (get_owner(op) && op->owner->type == PLAYER))
    {
        if (hitter->type == PLAYER || (get_owner(hitter) && hitter->owner->type == PLAYER))
        {
            /* now we are sure player are involved. Get the real player object now and test! */
            if (!pvp_area(op->type == PLAYER ? op : get_owner(op), hitter->type == PLAYER ? hitter : get_owner(hitter)))
                return 0;
        }
    }


    /* this checks objects are valid, on same map and set them to head when needed! */
    /* also, simple_attack is set to 1, when one of the parts hav ->env != null
     * atm, this value is only used in the door attack */
    if (get_attack_mode(&op, &hitter, &simple_attack))
        return 0;

    op_tag = op->count;
    hitter_tag = hitter->count;

    /* A last safery check - this happened in the past sometimes */
    if (op->stats.hp < 0)
	{
        LOG(llevDebug, "BUG/FIXME: victim (arch %s, name %s (%x - %d)) already dead in hit_player()\n", op->arch->name, query_name(op), op, op->count);
		return 0;
    }

	/* Now - we really hit. Perhaps we do no damage - but our weapons/spell or
	 * whatever hits the target. And our target will find that perhaps not so
	 * funny... Lets check the aggro/damage marker.
	 */

	/* first, we check we have a dmg object.
	 * Only a few kind of damage dealing objects need it.
	 * AoE spells are one of it.
	 */
	if(QUERY_FLAG(hitter, FLAG_USE_DMG_INFO) )
	{
		dmg_obj = aggro_get_damage(op, hitter);
		/* be sure we do damage only all hitter->last_heal ticks */
		if(dmg_obj)
		{
			if(dmg_obj->damage_round_tag+hitter->last_heal >= ROUND_TAG)
				return 0;
			dmg_obj->damage_round_tag = ROUND_TAG;
		}
		else
			dmg_obj = aggro_insert_damage(op, hitter);
	}


    /* slaying door ... when i think about it, its broken,,,
     * and iam more and more sure we don't need it.
     * because: IF there is a closed door - then seek key.
     * Or get a spell. Knock spell is pure senseless when you can
     * crush the door. And because we have hybrid chars (can cast too)
     * we don't need to handle both for balancing.
     */
    /*
    if ( ! simple_attack && op->type == DOOR)
    {
           object *tmp;
           for (tmp = op->inv; tmp != NULL; tmp = tmp->below)
               if (tmp->type == RUNE)
           {
                   spring_trap (tmp, hitter);
                   if (was_destroyed (hitter, hitter_tag)
                       || was_destroyed (op, op_tag)
                       || abort_attack (op, hitter, simple_attack))
                       return 0;
                   break;
          }
    }
    */

    /* Go through and hit the player with each attacktype, one by one.
    * hit_player_attacktype only figures out the damage, doesn't inflict
    * it.  It will do the appropriate action for attacktypes with
    * effects (slow, paralization, etc.
    */
    for (attacknum = 0; attacknum < NROFATTACKS; attacknum++)
    {
        if (hitter->attack[attacknum])

        #ifdef ATTACK_DEBUG
        {
            int tmp;
            tmp = hit_player_attacktype(op, hitter, dam, attacknum, 0);
            /*new_draw_info_format(NDI_ALL|NDI_UNIQUE,5,NULL,"%s hits %s with attack #%d with %d damage\n",hitter->name, op->name, attacknum,tmp);*/
            maxdam += tmp;
        }
#else
        {
            /*          LOG(-1, "hitter: %f - %s (dam:%d/%d) (wc:%d/%d)(ac:%d/%d) ap:%d\n",hitter->speed,
                            hitter->name,hitter->stats.dam,op->stats.dam, hitter->stats.wc,op->stats.wc,
                            hitter->stats.ac,op->stats.ac,hitter->attack[attacknum]);
            */
            maxdam += hit_player_attacktype(op, hitter, dam, attacknum, 0);
        }
#endif
    }

    /* we insert the aggro data in the mob, and report to the AI system */
	aggro_obj = aggro_update_info(op, target_obj, hitter, hit_obj, maxdam<op->stats.hp?maxdam:op->stats.hp, 0);

    /* this is needed to send the hit number animations to the clients */
    if (op->damage_round_tag != ROUND_TAG)
    {
        op->last_damage = 0;
        op->damage_round_tag = ROUND_TAG;
    }
    if ((op->last_damage + maxdam) > 64000)
        op->last_damage = 64000;
    else
        op->last_damage += maxdam;

    /*LOG(-1,"DMG: %d\n", maxdam);*/
    op->stats.hp -= maxdam; /* thats the damage the target got */

    /* Eneq(@csd.uu.se): Check to see if monster runs away. */
    /* TODO: gecko: this should go into a behaviour... */
    if ((op->stats.hp >= 0)
     && QUERY_FLAG(op, FLAG_MONSTER)
     && op->stats.hp < (signed short) (((float) op->run_away / 100.0f) * (float) op->stats.maxhp))
    {
        SET_FLAG(op, FLAG_RUN_AWAY);
    }

    if (QUERY_FLAG(op, FLAG_TEAR_DOWN))
    {
        tear_down_wall(op);
        return maxdam;  /* nothing more to do for wall */
    }
    /* Start of creature kill processing */

    if ((rtn_kill = kill_object(op, dam, hitter, type)))
        return (maxdam + rtn_kill + 1); /* rtn_kill is here negative! */

    /* End of creature kill processing */

    /* Used to be ghosthit removal - we now use the ONE_HIT flag.  Note
     * that before if the player was immune to ghosthit, the monster
     * remained - that is no longer the case.
     */
    if (QUERY_FLAG(hitter, FLAG_ONE_HIT))
    {
        remove_ob(hitter); /* Remove, but don't drop inventory */
        check_walk_off(hitter, NULL, MOVE_APPLY_VANISHED);
    }
    /* Lets handle creatures that are splitting now */
    else if (type & AT_PHYSICAL && !OBJECT_FREE(op) && QUERY_FLAG(op, FLAG_SPLITTING))
    {
        int     i;
        int     friendly        = QUERY_FLAG(op, FLAG_FRIENDLY);
        int     unaggressive    = QUERY_FLAG(op, FLAG_UNAGGRESSIVE);
        object *owner           = get_owner(op);

        if (!op->other_arch)
        {
            LOG(llevBug, "BUG: SPLITTING without other_arch error.\n");
            return maxdam;
        }

        remove_ob(op);
        if (check_walk_off(op, NULL, MOVE_APPLY_VANISHED) == CHECK_WALK_OK)
        {
            for (i = 0; i < NROFNEWOBJS(op); i++)
            {
                /* This doesn't handle op->more yet */
                object *tmp = arch_to_object(op->other_arch);
                int     j;

                tmp->stats.hp = op->stats.hp;
                if (friendly)
                {
                    SET_FLAG(tmp, FLAG_FRIENDLY);
                    tmp->move_type = PETMOVE;
                    if (owner != NULL)
                        set_owner(tmp, owner);
                }
                if (unaggressive)
                    SET_FLAG(tmp, FLAG_UNAGGRESSIVE);
                j = find_first_free_spot(tmp->arch, op->map, op->x, op->y);
                if (j >= 0)
                {
                    /* Found spot to put this monster */
                    tmp->x = op->x + freearr_x[j],tmp->y = op->y + freearr_y[j];
                    insert_ob_in_map(tmp, op->map, NULL, 0);
                }
            }
        }
    }
    else if (type & AT_DRAIN && hitter->type == GRIMREAPER && hitter->value++ > 10) /* hm, i DON'T like the value use here ! mt- 05.2005 */
    {
        remove_ob(hitter);
        check_walk_off(hitter, NULL, MOVE_APPLY_VANISHED);
    }
    return maxdam;
}

/* if we drop for example a spell object like a fireball to the map,
 * they move from tile to tile. Every time they check the object they
 * "hit on this map tile". If they find some - we are here.
 */
int hit_map(object *op, int dir, int type)
{
    object     *tmp, *next, *tmp_obj, *tmp_head;
    mapstruct  *map;
    int         x, y;
    int         mflags, retflag = 0;  /* added this flag..  will return 1 if it hits a monster */
    tag_t       op_tag, next_tag = 0;

    if (OBJECT_FREE(op))
    {
        LOG(llevBug, "BUG: hit_map(): free object\n");
        return 0;
    }

    if (QUERY_FLAG(op, FLAG_REMOVED) || op->env != NULL)
    {
        LOG(llevBug, "BUG: hit_map(): hitter (arch %s, name %s) not on a map\n", op->arch->name, query_name(op));
        return 0;
    }

    if (op->head)
        op = op->head;

    op_tag = op->count;

    if (!op->map)
    {
        LOG(llevBug, "BUG: hit_map(): %s has no map.\n", query_name(op));
        return 0;
    }

    x = op->x + freearr_x[dir];
    y = op->y + freearr_y[dir];
    if (!(map = out_of_map(op->map, &x, &y)))
        return 0;

    mflags = GET_MAP_FLAGS(map, x, y);

    /* peterm:  a few special cases for special attacktypes --counterspell
          must be out here because it strikes things which are not alive*/
    /* this will be handled different elsewhere!
     if(type&AT_COUNTERSPELL) {
       counterspell(op,dir);
       if(!(type & ~(AT_COUNTERSPELL|AT_MAGIC))){
    return 0;
       }
       type &= ~AT_COUNTERSPELL;
     }
    */

    next = get_map_ob(map, x, y);
    if (next)
        next_tag = next->count;

    if (!(tmp_obj = get_owner(op)))
        tmp_obj = op;
    if (tmp_obj->head)
        tmp_obj = tmp_obj->head;

    while (next)
    {
        if (was_destroyed(next, next_tag))
        {
            /* There may still be objects that were above 'next', but there is no
             * simple way to find out short of copying all object references and
             * tags into a temporary array before we start processing the first
             * object.  That's why we just abort.
             *
             * This happens whenever attack spells (like fire) hit a pile
             * of objects. This is not a bug - nor an error.
             *
             * Gecko: this may be a little different now, since we don't really destroy object until
             * end of timestep.
            */
            break;
        }
        tmp = next;
        next = tmp->above;
        if (next)
            next_tag = next->count;

        if (OBJECT_FREE(tmp))
        {
            LOG(llevBug, "BUG: hit_map(): found freed object (%s)\n", STRING_SAFE(tmp->arch->name));
            break;
        }

        /* Something could have happened to 'tmp' while 'tmp->below' was processed.
         * For example, 'tmp' was put in an icecube.
         * This is one of the few cases where on_same_map should not be used.
         */
        if (tmp->map != map || tmp->x != x || tmp->y != y)
            continue;

        /* we have found a player */
        if (QUERY_FLAG(tmp, FLAG_IS_PLAYER))
        {
            /* no damage from friendly objects */
            if (QUERY_FLAG(tmp_obj, FLAG_FRIENDLY))
                continue;
            hit_player(tmp, op->stats.dam, op, type);
            retflag |= 1;
            if (was_destroyed(op, op_tag))
                break;
        }
        else if (IS_LIVE(tmp))
        {
            tmp_head = tmp->head ? tmp->head : tmp;
            if (tmp_head->type == MONSTER)
            {
                if (tmp_obj->type == MONSTER) /* monster vs. monster */
                {
                    /* lets decide that monster of same kind don't hurt itself */
                    if (QUERY_FLAG(tmp_head, FLAG_FRIENDLY))
                    {
                        if (QUERY_FLAG(tmp_obj, FLAG_FRIENDLY))
                            continue;
                    }
                    else
                    {
                        if (!QUERY_FLAG(tmp_obj, FLAG_FRIENDLY))
                            continue;
                    }
                }
            }
            /*LOG(-1,"HM: %s hit %s (%d)with dam %d\n",op->name,tmp->name,tmp->type,op->stats.dam);*/
            hit_player(tmp, op->stats.dam, op, type);
            retflag |= 1;
            if (was_destroyed(op, op_tag))
                break;
        }
        else if (tmp->material && op->stats.dam > 0)
        {
            save_throw_object(tmp, type, op);
            if (was_destroyed(op, op_tag))
                break;
        }
    }

    return 0;
}

/* This returns the amount of damage hitter does to op with the
 * appropriate attacktype.  Only 1 attacktype should be set at a time.
 * This doesn't damage the player, but returns how much it should
 * take.  However, it will do other effects (paralyzation, slow, etc.)
 * Note - changed for PR code - we now pass the attack number and not
 * the attacktype.  Makes it easier for the PR code.  */
int hit_player_attacktype(object *op, object *hitter, int damage, uint32 attacknum, int magic)
{
    double  dam         = (double) damage;
    int     doesnt_slay = 1;

    /* just a sanity check */
    if (dam < 0)
    {
        LOG(llevBug, "BUG: hit_player_attacktype called with negative damage: %d from object: %s\n", dam, query_name(op));
        return 0;
    }

    if (hitter->slaying)
    {
        if (((op->race != NULL) && strstr(hitter->slaying, op->race))
         || (op->arch && (op->arch->name != NULL) && strstr(op->arch->name, hitter->slaying)))
        {
            doesnt_slay = 0;
            if (QUERY_FLAG(hitter, FLAG_IS_ASSASSINATION))
                damage = (int) ((double) damage * 2.25);
            else
                damage = (int) ((double) damage * 1.75);
            dam = (double) damage;
        }
    }

    /* AT_INTERNAL is supposed to do exactly dam.  Put a case here so
     * people can't mess with that or it otherwise get confused.  */
    /* I extended this  function - it now maps the special damage to the
     * depending on the attacking object. For example we fake a poison attack
     * from a poison object - we need to handle this here because we don't want
     * repoison the target every time recursive.
     */
    if (attacknum == ATNR_INTERNAL)
    {
        /* adjust damage */
        dam = dam * ((double) hitter->attack[ATNR_INTERNAL] / 100.0);

        /* handle special object attacks */
        if (hitter->type == POISONING) /* we have a poison force object (thats the poison we had inserted) */
        {
            attacknum = ATNR_POISON; /* map to poison... */
            if (op->resist[attacknum] == 100 || op->protection[protection_tab[attacknum]] == 100)
            {
                dam = 0;
                send_attack_msg(op, hitter, attacknum, (int) dam, damage);
                goto jump_show_dmg;
            }

            if (op->resist[attacknum])
                ATTACK_RESIST_DAMAGE(op, attacknum); /* reduce to % resistance */
            ATTACK_PROTECT_DAMAGE(op, attacknum);        /* reduce to % protection */
        }

        if (damage && dam < 1.0)
            dam = 1.0;

        send_attack_msg(op, hitter, attacknum, (int) dam, damage);
        goto jump_show_dmg;
    }

    /* quick check for immunity - if so, we skip here.
        * our formula is (100-resist)/100 - so test for 100 = zero division
     */
    if (op->resist[attacknum] == 100 || op->protection[protection_tab[attacknum]] == 100)
    {
        dam = 0;
        send_attack_msg(op, hitter, attacknum, (int) dam, damage);
        goto jump_show_dmg;
    }

    switch (attacknum)
    {
        case ATNR_PHYSICAL:
          check_physically_infect(op, hitter); /* quick check for desease! */
        case ATNR_SLASH:
        case ATNR_CLEAVE:
        case ATNR_PIERCE:
#ifdef ATTACK_DEBUG
          /*new_draw_info_format(NDI_ALL|NDI_UNIQUE,5,NULL,"** Start attack #%d with %f damage\n",attacknum,dam);*/
#endif
          ATTACK_HIT_DAMAGE(hitter, attacknum); /* get % of dam from this attack form */
#ifdef ATTACK_DEBUG
          /*new_draw_info_format(NDI_ALL|NDI_UNIQUE,5,NULL,"** After attack[%d]: %f damage\n",hitter->attack[attacknum],dam);*/
#endif
          if (op->resist[attacknum])
              ATTACK_RESIST_DAMAGE(op, attacknum); /* reduce to % resistance */
#ifdef ATTACK_DEBUG
          /*new_draw_info_format(NDI_ALL|NDI_UNIQUE,5,NULL,"** After resist[%d]: %f damage\n",op->resist[attacknum],dam);*/
#endif
          ATTACK_PROTECT_DAMAGE(op, attacknum);        /* reduce to % protection */
#ifdef ATTACK_DEBUG
          /*new_draw_info_format(NDI_ALL|NDI_UNIQUE,5,NULL,"** After protect[%d]: %f damage\n",op->protection[protection_tab[attacknum]],dam);*/
#endif

          if (damage && dam < 1.0)
              dam = 1.0;

          send_attack_msg(op, hitter, attacknum, (int) dam, damage);
          break;

        case ATNR_POISON:
          ATTACK_HIT_DAMAGE(hitter, attacknum);
          if (op->resist[attacknum])
              ATTACK_RESIST_DAMAGE(op, attacknum);
          ATTACK_PROTECT_DAMAGE(op, attacknum);

          /* i must adjust this... alive stand for mobs and player include
                 * golems, undead and demons - but not for doors or "technical objects"
                 * like earth walls.
                 */
          /* if we had done any damage AND this is a ALIVE creature - poison it!
                 * We don't need to calc level here - the level is implicit calced
                 * in the damage!
                 */
          if (damage && dam < 1.0)
              dam = 1.0;

          send_attack_msg(op, hitter, attacknum, (int) dam, damage);

          if (dam && IS_LIVE(op))
              poison_player(op, hitter, (float) dam);

          break;

          /* TODO:
             * Here comes in all attacks we have not really implemented.
             * atm - below this function in old cf code - use it as base.
             */
        default:
          /*LOG(llevBug,"attack(): find unimplemented special attack: #%d obj:%s\n", attacknum, query_name(hitter));*/
          ATTACK_HIT_DAMAGE(hitter, attacknum); /* get % of dam from this attack form */
          if (op->resist[attacknum])
              ATTACK_RESIST_DAMAGE(op, attacknum); /* reduce to % resistance */
          ATTACK_PROTECT_DAMAGE(op, attacknum);        /* reduce to % protection */

          if (damage && dam < 1.0)
              dam = 1.0;
          send_attack_msg(op, hitter, attacknum, (int) dam, damage);
          break;
    }

    jump_show_dmg : return (int) dam;
}


/* we need this called spread in the function before because sometimes we want drop
 * a message BEFORE we tell the damage and sometimes we want a message after it.
 */
static void send_attack_msg(object *op, object *hitter, int attacknum, int dam, int damage)
{
    if (op->type == PLAYER)
    {
        new_draw_info_format(NDI_PURPLE, 0, op, "%s hit you for %d (%d) damage.", hitter->name, (int) dam,
                             ((int) dam) - damage);
    }
    if (hitter->type == PLAYER)
    {
        new_draw_info_format(NDI_ORANGE, 0, hitter, "You hit %s for %d (%d) %s.", op->name, (int) dam,
                             ((int) dam) - damage, attacktype_desc[attacknum]);
    }
}

/* OLD CODE FOR ATTACKS!
 * i let it in to show what and where we must browse to reimplement and/or
 * change it
    case ATNR_CONFUSION:
    case ATNR_SLOW:
    case ATNR_PARALYZE:
    case ATNR_FEAR:
    case ATNR_CANCELLATION:
    case ATNR_DEPLETE:
    case ATNR_BLIND: {
    int level_diff = MIN(110, MAX(0, op->level - hitter->level));

        if (op->speed &&
        (QUERY_FLAG(op, FLAG_MONSTER) || op->type==PLAYER) &&
        !(rndm(0, (attacknum == ATNR_SLOW?6:3)-1)) &&
        ((random_roll(1, 20, op, PREFER_LOW) +
          op->resist[attacknum]/10) < savethrow[level_diff])) {

      if (attacknum == ATNR_CONFUSION) confuse_player(op,hitter,(int)dam);
      else if (attacknum == ATNR_SLOW) slow_player(op,hitter,(int)dam);
      else if (attacknum == ATNR_PARALYZE) paralyze_player(op,hitter,(int)dam);
      else if (attacknum == ATNR_FEAR) SET_FLAG(op, FLAG_SCARED);
      else if (attacknum == ATNR_CANCELLATION) cancellation(op);
      else if (attacknum == ATNR_DEPLETE) drain_stat(op);
      else if (attacknum == ATNR_BLIND  && !QUERY_FLAG(op,FLAG_UNDEAD) &&
           !QUERY_FLAG(op,FLAG_GENERATOR)) blind_player(op,hitter,(int)dam);
    }
    dam = 0;
    } break;
    case ATNR_ACID:
      {
    int flag=0;

    if (!op_on_battleground(op, NULL, NULL) &&
        (op->resist[ATNR_ACID] < 50))
      {
        object *tmp;
        for(tmp=op->inv; tmp!=NULL; tmp=tmp->below) {
        if(!QUERY_FLAG(tmp, FLAG_APPLIED) ||
           (tmp->resist[ATNR_ACID] >= 10))
          continue;
        if(!(tmp->material & M_IRON))
          continue;
        if(tmp->magic < -4)
          continue;
        if(tmp->type==RING ||
           tmp->type==GIRDLE ||
           tmp->type==AMULET ||
           tmp->type==WAND ||
           tmp->type==ROD ||
           tmp->type==HORN)
          continue;

        if(rndm(0, (int)dam+4) >
            random_roll(0, 39, op, PREFER_HIGH)+2*tmp->magic) {
            if(op->type == PLAYER)
            new_draw_info_format(NDI_UNIQUE|NDI_RED,0, op,
                 "The %s's acid corrodes your %s!",
                 query_name(hitter), query_name(tmp));
            flag = 1;
            tmp->magic--;
            if(op->type == PLAYER)
            esrv_send_item(op, tmp);
        }
        }
        if(flag)
          fix_player(op);
    }
      }
      break;
    case ATNR_DRAIN:
      {
    int rate;

    if(op->resist[ATNR_DRAIN] > 0)
      rate = 50 + op->resist[ATNR_DRAIN] / 2;
    else if(op->resist[ATNR_DRAIN] < 0)
      rate = 5000 / (100 - op->resist[ATNR_DRAIN]);

    if(!rate)
      return 0;

    if(op->stats.exp <= rate) {
        if(op->type == GOLEM)
        dam = 999;
        else
        dam = hit_player_attacktype(op, hitter, (int)dam, ATNR_PHYSICAL, magic);
    } else {
        if(hitter->stats.hp<hitter->stats.maxhp &&
           (op->level > hitter->level) &&
           random_roll(0, (op->level-hitter->level+2), hitter, PREFER_HIGH)>3)
          hitter->stats.hp++;

        if (!op_on_battleground(hitter, NULL, NULL)) {

        add_exp(op,-op->stats.exp/rate, CHOSEN_SKILL_NO);
        }
        dam = 0;
    }
      } break;
    case ATNR_TIME:
      {
    if (QUERY_FLAG(op,FLAG_UNDEAD)) {
        object *owner = get_owner(hitter) == NULL ? hitter : get_owner(hitter);
            object *god = find_god (determine_god (owner));
            int div = 1;

            if (! god || ! god->slaying ||
         strstr (god->slaying, shstr.undead) == NULL)
                div = 2;
        if (op->level * div <
        (turn_bonus[owner->stats.Wis]+owner->level +
         (op->resist[ATNR_TIME]/100)))
          SET_FLAG(op, FLAG_SCARED);
    }
    else
      dam = 0;
      } break;
    case ATNR_DEATH:
    break;
    case ATNR_CHAOS:
    LOG(llevBug, "BUG: %s was hit by %s with non-specific chaos.\n",
                                    query_name(op), query_name(hitter));
    dam = 0;
    break;
    case ATNR_COUNTERSPELL:
    LOG(llevBug, "BUG: %s was hit by %s with counterspell attack.\n",
                                    query_name(op),query_name(hitter));
    dam = 0;
    break;
    case ATNR_HOLYWORD:
      {

    object *owner = get_owner(hitter)==NULL?hitter:get_owner(hitter);

    if((op->level+(op->resist[ATNR_HOLYWORD]/100)) <
       owner->level+turn_bonus[owner->stats.Wis])
      SET_FLAG(op, FLAG_SCARED);
      } break;
    case ATNR_LIFE_STEALING:
      {
    int new_hp;
    if ((op->type == GOLEM) || (QUERY_FLAG(op, FLAG_UNDEAD))) return 0;
    if (op->resist[ATNR_DRAIN] >= op->resist[ATNR_LIFE_STEALING])
      dam = (dam*(100 - op->resist[ATNR_DRAIN])) / 3000;
    else dam = (dam*(100 - op->resist[ATNR_LIFE_STEALING])) / 3000;
    if (dam > (op->stats.hp+1)) dam = op->stats.hp+1;
    new_hp = hitter->stats.hp + (int)dam;
    if (new_hp > hitter->stats.maxhp) new_hp = hitter->stats.maxhp;
    if (new_hp > hitter->stats.hp) hitter->stats.hp = new_hp;
      }

*/

/* GROS: This code comes from hit_player. It has been made external to
 * allow script procedures to "kill" objects in a combat-like fashion.
 * It was initially used by (kill-object) developed for the Collector's
 * Sword. Note that nothing has been changed from the original version
 * of the following code.
 */

/* ok, when i have finished the different attacks i must clean this up here too
 * looks like some artifact code in here - MT-2003
 */
int kill_object(object *op, int dam, object *hitter, int type)
{
    object     *corpse_owner, *owner, *old_hitter; /* this is used in case of servant monsters */
    int         maxdam              = 0;
    int         battleg             = 0;    /* true if op standing on battleground */
    mapstruct  *map;
	char		*buf_ptr, buf2[MAX_BUF];

    /* Object has been killed.  Lets clean it up */
    if (op->stats.hp <= 0)
    {
        if(trigger_object_plugin_event(EVENT_DEATH, 
                    op, hitter, op, NULL, NULL, NULL, NULL, SCRIPT_FIX_ALL))
            return 0; /* Cheat death */
        
#if 0        
        CFParm      CFP;
        /* GROS: Handle for the global kill event */
        evtid = EVENT_GKILL;
        CFP.Value[0] = (void *) (&evtid);
        CFP.Value[1] = (void *) (hitter);
        CFP.Value[2] = (void *) (op);
        GlobalEvent(&CFP);
#endif


        corpse_owner = owner = old_hitter = NULL;
        maxdam = op->stats.hp - 1;

        /* very old door code for destroyable code.*/
        /*if (op->type == DOOR)
        {
            op->speed = 0.1f;
            update_ob_speed(op);
            op->speed_left = -0.05f;
            return maxdam;
        }*/

        /* Show Damage System for clients
         * whatever is dead now, we check map. If it on map, we redirect last_damage
         * to map space, giving player the chance to see the last hit damage they had
         *  done. If there is more as one object killed on a single map tile, we overwrite
         *  it now. This visual effect works pretty good. MT
         */

        /* no pet/player/monster checking now, perhaps not needed */

        if (op->damage_round_tag == ROUND_TAG)
        {
            /* is on map */
            if ((map = op->map)) /* hm, can we sure we are on a legal map position... hope so */
            {
                SET_MAP_DAMAGE(op->map, op->x, op->y, op->last_damage);
                SET_MAP_RTAG(op->map, op->x, op->y, ROUND_TAG);
            }
        }

        if (op->map)
            play_sound_map(op->map, op->x, op->y, SOUND_PLAYER_KILLS, SOUND_NORMAL);

        /* old golem/npc code
        if (QUERY_FLAG(op, FLAG_FRIENDLY) && op->type != PLAYER)
        {
            if (get_owner(op) != NULL && op->owner->type == PLAYER)
            {
                send_golem_control(op, GOLEM_CTR_RELEASE);
                CONTR(op->owner)->golem = NULL;
            }

            op->speed = 0;
            update_ob_speed(op);
            destruct_ob(op);
            return maxdam;
        } */

        /* do some checks */
        if ((owner = get_owner(hitter)) == NULL)
            owner = hitter;
        battleg = op_on_battleground(op, NULL, NULL);

        buf_ptr = NULL;

        /* Create kill message */
        if (owner->type == PLAYER)
        {
            char        buf[MAX_BUF];

            /* old pet code */
            /* if (owner != NULL)
            {
                sprintf(buf, "%s killed %s with %s%s.", hitter->owner->name, query_name(op), query_name(hitter),
                    battleg ? " (duel)" : "");
                old_hitter = hitter;
                owner->exp_obj = hitter->exp_obj;
                hitter = hitter->owner;
            }*/
			buf_ptr = buf2;
            if (owner != hitter)
            {
                sprintf(buf, "You killed %s with %s.", query_name(op), query_name(hitter));
                sprintf(buf2, "%s killed %s with %s.", query_name(owner), query_name(op), query_name(hitter));
                old_hitter = hitter;
                owner->exp_obj = hitter->exp_obj;
            }
            else
            {
                sprintf(buf2, "%s killed %s.",  query_name(owner), query_name(op));
                sprintf(buf, "You killed %s.", query_name(op));
            }
            new_draw_info(NDI_WHITE, 0, owner, buf);
        }

        /* Give exp and create the corpse. Decide we get a loot or not */
        if (op->type != PLAYER)
        {
            if (!battleg)
                corpse_owner = aggro_calculate_exp(op, owner, buf_ptr);

            op->speed = 0;
            update_ob_speed(op); /* remove from active list (if on) */

            /* destruct_ob() will remove the killed mob/object from the game.
             * It will also trigger the drop of a corpse & with the loot (inv of that object).
             * FLAG_STARTEQUIP will avoid to drop the standard loot for example when
             * a npc killed another npc.
             */
            if(corpse_owner)
            {
                /* drop_inv() will use ->enemy to create the "bounty look" for a corpse container */
                if(corpse_owner->type == PLAYER)
                {
                    op->enemy = corpse_owner;
                    op->enemy_count = corpse_owner->count;
                }
                else /* mob/npc kill - force a droped corpse without items */
                {
                    op->enemy = NULL;
                    SET_FLAG(op, FLAG_CORPSE_FORCED);
                    SET_FLAG(op, FLAG_STARTEQUIP);
                }
            }
            else
            {
                op->enemy = NULL;
                SET_FLAG(op, FLAG_STARTEQUIP);
            }

            destruct_ob(op);
        }
   }

    return maxdam;
}

static int get_attack_mode(object **target, object **hitter, int *simple_attack)
{
    if (OBJECT_FREE(*target) || OBJECT_FREE(*hitter))
    {
        LOG(llevBug, "BUG: get_attack_mode(): freed object\n");
        return 1;
    }
    if ((*target)->head)
        *target = (*target)->head;
    if ((*hitter)->head)
        *hitter = (*hitter)->head;
    if ((*hitter)->env != NULL || (*target)->env != NULL)
    {
        *simple_attack = 1;
        return 0;
    }
    if (QUERY_FLAG(*target, FLAG_REMOVED)
     || QUERY_FLAG(*hitter, FLAG_REMOVED)
     || (*hitter)->map
     == NULL
     || !on_same_map((*hitter),
                     (*target)))
    {
        LOG(llevBug, "BUG: hitter (arch %s, name %s) with no relation to target\n", (*hitter)->arch->name,
            query_name(*hitter));
        return 1;
    }
    *simple_attack = 0;
    return 0;
}

static int abort_attack(object *target, object *hitter, int simple_attack)
{
    /* Check if target and hitter are still in a relation similar to the one
     * determined by get_attack_mode().  Returns true if the relation has changed.
     */
    int new_mode;

    if (hitter->env == target || target->env == hitter)
        new_mode = 1;
    else if (QUERY_FLAG(target, FLAG_REMOVED)
          || QUERY_FLAG(hitter, FLAG_REMOVED)
          || hitter->map == NULL
          || !on_same_map(hitter, target))
        return 1;
    else
        new_mode = 0;
    return new_mode != simple_attack;
}

/* op is the arrow, tmp is what is stopping the arrow.
 *
 * Returns 1 if op was inserted into tmp's inventory, 0 otherwise.
 */
static int stick_arrow(object *op, object *tmp)
{
    /* If the missile hit a player, we insert it in their inventory.
     * However, if the missile is heavy, we don't do so (assume it falls
     * to the ground after a hit).  What a good value for this is up to
     * debate - 5000 is 5 kg, so arrows, knives, and other light weapons
     * stick around.
     */
    if (op->weight <= 5000 && tmp->stats.hp >= 0)
    {
        if (tmp->head != NULL)
            tmp = tmp->head;
        remove_ob(op);
        check_walk_off(op, NULL, MOVE_APPLY_VANISHED);
        op = insert_ob_in_ob(op, tmp);
        if (tmp->type == PLAYER)
            esrv_send_item(tmp, op);

        return 1;
    }
    else
        return 0;
}

/* hit_with_arrow() disassembles the missile, attacks the victim and
 * reassembles the missile.
 *
 * It returns a pointer to the reassembled missile, or NULL if the missile
 * isn't available anymore.
 */
object * hit_with_arrow(object *op, object *victim)
{
    object *container, *hitter;
    int     hit_something   = 0;
    tag_t   victim_tag, hitter_tag;
    sint16  victim_x, victim_y;

    /* Disassemble missile */
    if (op->inv)
    {
        container = op;
        hitter = op->inv;
        remove_ob(hitter);
        insert_ob_in_map(hitter, container->map, hitter, INS_NO_MERGE | INS_NO_WALK_ON);
        /* Note that we now have an empty THROWN_OBJ on the map.  Code that
         * might be called until this THROWN_OBJ is either reassembled or
         * removed at the end of this function must be able to deal with empty
         * THROWN_OBJs. */
    }
    else
    {
        container = NULL;
        hitter = op;
    }

    /* Try to hit victim */
    victim_x = victim->x;
    victim_y = victim->y;
    victim_tag = victim->count;
    hitter_tag = hitter->count;
    
    if(!trigger_object_plugin_event(EVENT_ATTACK, 
                hitter, hitter, victim, NULL, NULL, NULL, NULL, SCRIPT_FIX_ALL))
    {
        /*LOG(-1, "hit: %s (%d %d)\n", hitter->name, op->stats.dam, op->stats.wc);*/
        hit_something = attack_ob_simple(victim, hitter, op->stats.dam, op->stats.wc);
    }

    /* Arrow attacks door, rune of summoning is triggered, demon is put on
     * arrow, move_apply() calls this function, arrow sticks in demon,
     * attack_ob_simple() returns, and we've got an arrow that still exists
     * but is no longer on the map. Ugh. (Beware: Such things can happen at
     * other places as well!) */
    /* hopefully the walk_off event was triggered somewhere there */
    if (was_destroyed(hitter, hitter_tag) || hitter->env != NULL)
    {
        if (container)
        {
            remove_ob(container);
            check_walk_off(container, NULL, MOVE_APPLY_VANISHED);
        }
        return NULL;
    }

    /* Missile hit victim */
    if (hit_something)
    {
        trigger_object_plugin_event(EVENT_STOP, 
                hitter, victim, NULL, NULL, NULL, NULL, NULL, SCRIPT_FIX_ALL);
            
        /* Stop arrow */
        if (container == NULL)
        {
            hitter = fix_stopped_arrow(hitter);
            if (hitter == NULL)
                return NULL;
        }
        else
        {
            remove_ob(container);
            check_walk_off(container, NULL, MOVE_APPLY_VANISHED);
        }

        /* Try to stick arrow into victim */
        /* disabled - this will not work very well now with
             * the loot system of corpses.. if several people shot
             * at same guys.
             */
        /*
              if ( ! was_destroyed (victim, victim_tag)
                   && stick_arrow (hitter, victim))
                  return NULL;
            */

        /* Else try to put arrow on victim's map square */
        if ((victim_x != hitter->x || victim_y != hitter->y) && !wall(hitter->map, victim_x, victim_y))
        {
            remove_ob(hitter);
            if (check_walk_off(hitter, NULL, MOVE_APPLY_DEFAULT) == CHECK_WALK_OK)
            {
                hitter->x = victim_x;
                hitter->y = victim_y;
                insert_ob_in_map(hitter, victim->map, hitter, 0);
            }
        }
        else
        {
            /* Else leave arrow where it is */
            hitter = merge_ob(hitter, NULL);
        }

        return NULL;
    }

    /* Missile missed victim - reassemble missile */
    if (container)
    {
        remove_ob(hitter); /* technical remove, no walk check */
        insert_ob_in_ob(hitter, container);
    }
    return op;
}


void tear_down_wall(object *op)
{
    int perc    = 0;

    if (!op->stats.maxhp)
    {
        LOG(llevBug, "BUG: TEAR_DOWN wall %s had no maxhp.\n", op->name);
        perc = 1;
    }
    else if (!GET_ANIM_ID(op))
    {
        /* Object has been called - no animations, so remove it */
        if (op->stats.hp < 0)
            destruct_ob(op); /* Should update LOS */
        return; /* no animations, so nothing more to do */
    }
    perc = NUM_ANIMATIONS(op) - ((int) NUM_ANIMATIONS(op) * op->stats.hp) / op->stats.maxhp;
    if (perc >= (int) NUM_ANIMATIONS(op))
        perc = NUM_ANIMATIONS(op) - 1;
    else if (perc < 1)
        perc = 1;
    SET_ANIMATION(op, perc);
    update_object(op, UP_OBJ_FACE);
    if (perc == NUM_ANIMATIONS(op) - 1)
    {
        /* Reached the last animation */
        if (op->face == blank_face)
                /* If the last face is blank, remove the ob */
            destruct_ob(op); /* Should update LOS */
        else
        {
            /* The last face was not blank, leave an image */
            CLEAR_FLAG(op, FLAG_BLOCKSVIEW);
            CLEAR_FLAG(op, FLAG_NO_PASS);
            CLEAR_FLAG(op, FLAG_ALIVE);
        }
    }
}


/* TODO: i have not the time now - but later we should do this: a marker value
 * in the poison force flags kind of poison - food/poison force or weapon/spell
 * poison. Weapon/spell force will block now poisoning, but will not be blocked
 * by food/poison forces - and food/poison forces can stack. MT-2003
 * NOTE: only poison now insert a poison force - food&drink insert different forces.
 * For Poison objects, we use simply the base dam * level
 */
void poison_player(object *op, object *hitter, float dam)
{
    float       pmul;
    archetype  *at  = find_archetype("poisoning");
    object     *tmp = present_arch_in_ob(at, op);

    /* this is to avoid stacking poison forces... Like we get hit 10 times
     * by a spell and sword and get 10 poison force in us
     * The bad point is, that we can cheat here... Lets say we poison us self
     * with a mild food poison and then we battle the god of poison - he can't
     * hurt us with poison until the food poison wears out
     */

    /* we only poison players and mobs! */
    if (op->type != PLAYER && !QUERY_FLAG(op, FLAG_MONSTER))
        return;

    if (tmp == NULL || hitter->type == POISON)
    {
        if ((tmp = arch_to_object(at)) == NULL)
        {
            LOG(llevBug, "BUG: Failed to clone arch poisoning.\n");
            return;
        }
        else
        {
            if (hitter->type == POISON)
            {
                dam /= 2.0f;
                tmp->stats.dam = (int) (((dam + RANDOM() % ((int) dam + 1)) * lev_damage[hitter->level]) * 0.9f);
                if (tmp->stats.dam > op->stats.maxhp / 3)
                    tmp->stats.dam = op->stats.maxhp / 3;
                if (tmp->stats.dam < 1)
                    tmp->stats.dam = 1;
            }
            else /* spell or weapon will be handled different! */
            {
                dam /= 2.0f;
                tmp->stats.dam = (int) ((int) dam + RANDOM() % (int) (dam + 1));
                if (tmp->stats.dam > op->stats.maxhp / 3)
                    tmp->stats.dam = op->stats.maxhp / 3;
                if (tmp->stats.dam < 1)
                    tmp->stats.dam = 1;
            }

            tmp->level = hitter->level;
            copy_owner(tmp, hitter);   /*  so we get credit for poisoning kills */

            /* now we adjust numbers of ticks of the DOT force and speed of DOT ticks */
            if (hitter->type == POISON)
            {
                tmp->stats.food = hitter->last_heal; /* # of ticks */
                tmp->speed = tmp->speed_left; /* speed of ticks */
            }

            if (op->type == PLAYER)
            {
                /* spells should add here too later */
                /* her we handle consuming poison */
                if (hitter->type == POISON)
                {
                    create_food_force(op, hitter, tmp); /* this insert the food force in player too */
                    new_draw_info(NDI_UNIQUE, 0, op, "You suddenly feel very ill.");
                }
                else /* and here we have hit with weapon or something */
                {
                    if (op->stats.Con > 1 && !(RANDOM() % 2))
                    {
                        pmul = (hitter->level + (RANDOM() % (hitter->level / 4 + 1)) / 2) * 0.01f + 0.2f;
                        tmp->stats.Con = 1 + (sint8) (pmul * (op->stats.Con - 1));
                        if (tmp->stats.Con >= op->stats.Con)
                            tmp->stats.Con = op->stats.Con - 1;
                        tmp->stats.Con *= -1;
                    }

                    if (op->stats.Str > 1 && !(RANDOM() % 2))
                    {
                        pmul = (hitter->level + (RANDOM() % (hitter->level / 4 + 1)) / 2) * 0.01f + 0.2f;
                        tmp->stats.Str = 1 + (sint8) (pmul * (op->stats.Str - 1));
                        if (tmp->stats.Str >= op->stats.Str)
                            tmp->stats.Str = op->stats.Str - 1;
                        tmp->stats.Str *= -1;
                    }

                    if (op->stats.Dex > 1 && !(RANDOM() % 2))
                    {
                        pmul = (hitter->level + (RANDOM() % (hitter->level / 4 + 1)) / 2) * 0.01f + 0.2f;
                        tmp->stats.Dex = 1 + (sint8) (pmul * (op->stats.Dex - 1));
                        if (tmp->stats.Dex >= op->stats.Dex)
                            tmp->stats.Dex = op->stats.Dex - 1;
                        tmp->stats.Dex *= -1;
                    }
                    if (op->stats.Int > 1 && !(RANDOM() % 3))
                    {
                        pmul = (hitter->level + (RANDOM() % (hitter->level / 4 + 1)) / 2) * 0.01f + 0.2f;
                        tmp->stats.Int = 1 + (sint8) (pmul * (op->stats.Int - 1));
                        if (tmp->stats.Int >= op->stats.Int)
                            tmp->stats.Int = op->stats.Int - 1;
                        tmp->stats.Int *= -1;
                    }
                    if (op->stats.Cha > 1 && !(RANDOM() % 3))
                    {
                        pmul = (hitter->level + (RANDOM() % (hitter->level / 4 + 1)) / 2) * 0.01f + 0.2f;
                        tmp->stats.Cha = 1 + (sint8) (pmul * (op->stats.Cha - 1));
                        if (tmp->stats.Cha >= op->stats.Cha)
                            tmp->stats.Cha = op->stats.Cha - 1;
                        tmp->stats.Cha *= -1;
                    }
                    if (op->stats.Pow > 1 && !(RANDOM() % 4))
                    {
                        pmul = (hitter->level + (RANDOM() % (hitter->level / 4 + 1)) / 2) * 0.01f + 0.2f;
                        tmp->stats.Pow = 1 + (sint8) (pmul * (op->stats.Pow - 1));
                        if (tmp->stats.Pow >= op->stats.Pow)
                            tmp->stats.Pow = op->stats.Pow - 1;
                        tmp->stats.Pow *= -1;
                    }
                    if (op->stats.Wis > 1 && !(RANDOM() % 4))
                    {
                        pmul = (hitter->level + (RANDOM() % (hitter->level / 4 + 1)) / 2) * 0.01f + 0.2f;
                        tmp->stats.Wis = 1 + (sint8) (pmul * (op->stats.Wis - 1));
                        if (tmp->stats.Wis >= op->stats.Wis)
                            tmp->stats.Wis = op->stats.Wis - 1;
                        tmp->stats.Wis *= -1;
                    }

                    new_draw_info_format(NDI_UNIQUE, 0, op, "%s has poisoned you!", query_name(hitter));
                    insert_ob_in_ob(tmp, op);
                    SET_FLAG(tmp, FLAG_APPLIED);
                    fix_player(op);
                }
            }
            else /* its a mob! */
            {
                if (hitter->type == POISON) /* mob eats poison.. */
                {
                    /* TODO */
                }
                else /* is hit from poison force! */
                {
                    insert_ob_in_ob(tmp, op);
                    SET_FLAG(tmp, FLAG_APPLIED);
                    fix_monster(op);
                    if (hitter->type == PLAYER)
                        new_draw_info_format(NDI_UNIQUE, 0, hitter, "You poisoned %s!", query_name(op));
                    else if (get_owner(hitter) && hitter->owner->type == PLAYER)
                        new_draw_info_format(NDI_UNIQUE, 0, hitter->owner, "%s poisoned %s!", query_name(hitter),
                                             query_name(op));
                }
            }
        }
        tmp->speed_left = 0;
    }
    else
        tmp->stats.food++;
}

void slow_player(object *op, object *hitter, int dam)
{
    archetype  *at  = find_archetype("slowness");
    object     *tmp;
    if (at == NULL)
    {
        LOG(llevBug, "BUG: Can't find slowness archetype.\n");
    }
    if ((tmp = present_arch_in_ob(at, op)) == NULL)
    {
        tmp = arch_to_object(at);
        tmp = insert_ob_in_ob(tmp, op);
        new_draw_info(NDI_UNIQUE, 0, op, "The world suddenly moves very fast!");
    }
    else
        tmp->stats.food++;
    SET_FLAG(tmp, FLAG_APPLIED);
    tmp->speed_left = 0;
    fix_player(op);
}

void confuse_player(object *op, object *hitter, int dam)
{
    object *tmp;
    int     maxduration;

    tmp = present_in_ob(CONFUSION, op);
    if (!tmp)
    {
        tmp = get_archetype("confusion");
        tmp = insert_ob_in_ob(tmp, op);
    }

    /* Duration added per hit and max. duration of confusion both depend
       on the player's resistance */
    tmp->stats.food += MAX(1, 5 * (100 - op->resist[ATNR_CONFUSION]) / 100);
    maxduration = MAX(2, 30 * (100 - op->resist[ATNR_CONFUSION]) / 100);
    if (tmp->stats.food > maxduration)
        tmp->stats.food = maxduration;

    if (op->type == PLAYER && !QUERY_FLAG(op, FLAG_CONFUSED))
        new_draw_info(NDI_UNIQUE, 0, op, "You suddenly feel very confused!");
    SET_FLAG(op, FLAG_CONFUSED);
}

void blind_player(object *op, object *hitter, int dam)
{
    object *tmp, *owner;

    /* Save some work if we know it isn't going to affect the player */
    if (op->resist[ATNR_BLIND] == 100)
        return;

    tmp = present_in_ob(BLINDNESS, op);
    if (!tmp)
    {
        tmp = get_archetype("blindness");
        SET_FLAG(tmp, FLAG_BLIND);
        SET_FLAG(tmp, FLAG_APPLIED);
        /* use floats so we don't lose too much precision due to rounding errors.
         * speed is a float anyways.
         */
        tmp->speed = tmp->speed * ((float) 100.0 - (float) op->resist[ATNR_BLIND]) / (float) 100;

        tmp = insert_ob_in_ob(tmp, op);
        change_abil(op, tmp);   /* Mostly to display any messages */
        fix_player(op);        /* This takes care of some other stuff */

        if (hitter->owner)
            owner = get_owner(hitter);
        else
            owner = hitter;

        new_draw_info_format(NDI_UNIQUE, 0, owner, "Your attack blinds %s!", query_name(op));
    }
    tmp->stats.food += dam;
    if (tmp->stats.food > 10)
        tmp->stats.food = 10;
}

void paralyze_player(object *op, object *hitter, int dam)
{
    float   effect, max;
    /* object *tmp; */

    /* Do this as a float - otherwise, rounding might very well reduce this to 0 */
    effect = (float) dam * (float) 3.0 * ((float) 100.0 - (float) op->resist[ATNR_PARALYZE]) / (float) 100;

    if (effect == 0)
        return;

    SET_FLAG(op, FLAG_PARALYZED); /* we mark this object as paralyzed */

    op->speed_left -= FABS(op->speed) * effect;
    /* tmp->stats.food+=(signed short) effect/op->speed; */

    /* max number of ticks to be affected for. */
    max = ((float) 100 - (float) op->resist[ATNR_PARALYZE]) / (float) 2;
    if (op->speed_left < -(FABS(op->speed) * max))
        op->speed_left = (float) - (FABS(op->speed) * max);

    /*      tmp->stats.food = (signed short) (max/FABS(op->speed)); */
}


/* Attempts to kill 'op'.  hitter is the attack object, dam i
 * the computed damaged.
 */
void deathstrike_player(object *op, object *hitter, int *dam)
{
    /*  The intention of a death attack is to kill outright things
    **  that are a lot weaker than the attacker, have a chance of killing
    **  things somewhat weaker than the caster, and no chance of
    **  killing something equal or stronger than the attacker.
    **  Also, if a deathstrike attack has a slaying, any monster
    **  whose name or race matches a comma-delimited list in the slaying
    **  field of the deathstriking object  */

    int atk_lev, def_lev, kill_lev;

    if (hitter->slaying)
        if (!((QUERY_FLAG(op, FLAG_UNDEAD) && strstr(hitter->slaying, shstr.undead))
           || (op->race && strstr(hitter->slaying, op->race))))
            return;

    def_lev = op->level;
    if (def_lev < 1)
    {
        LOG(llevBug, "BUG: arch %s, name %s with level < 1\n", op->arch->name, query_name(op));
        def_lev = 1;
    }
    atk_lev = SK_level(hitter) / 2;
    /* LOG(llevDebug,"Deathstrike - attack level %d, defender level %d\n",
       atk_lev, def_lev); */

    if (atk_lev >= def_lev)
    {
        kill_lev = random_roll(0, atk_lev - 1, hitter, PREFER_HIGH);

        /* Note that the below effectively means the ratio of the atk vs
         * defener level is important - if level 52 character has very little
         * chance of killing a level 50 monster.  This should probably be
         * redone.
         */
        if (kill_lev >= def_lev)
        {
            *dam = op->stats.hp + 10; /* take all hp. they can still save for 1/2 */
            /* I think this doesn't really do much.  Because of
               * integer rounding, this only makes any difference if the
               * attack level is double the defender level.
               */
            *dam *= kill_lev / def_lev;
        }
    }
    else
    {
        *dam = 0;  /* no harm done */
    }
}

/* thrown_item_effect() - handles any special effects of thrown
 * items (like attacking living creatures--a potion thrown at a
 * monster).
 */
static void thrown_item_effect(object *hitter, object *victim)
{
    if (!IS_LIVE(hitter))
    {
        /* May not need a switch for just 2 types, but this makes it
         * easier for expansion.
         */
        /* i removed a resist check here - we handle resist checks BEFORE this (skill "avoid get hit" or whatever)
         * OR after this (resist against the damage this here perhaps does)
         */
        switch (hitter->type)
        {
            case POTION:
              /* OLD: should player get a save throw instead of checking magic protection? */
              /*if(QUERY_FLAG(victim,FLAG_ALIVE)&&!QUERY_FLAG(victim,FLAG_UNDEAD))
                    apply_potion(victim,hitter);
                */
              /* ok, we do something new here:
                 * a potion has hit a object.
                 * at this stage, we only explode area effects.
                 * ALL other potion will take here no effect.
                 * later we should take care about confusion & paralyze.
                 * we should NEVER allow cure/heal stuff here - it will
                 * allow to many possible exploits.
                 */
              if (hitter->stats.sp != SP_NO_SPELL && spells[hitter->stats.sp].flags & SPELL_DESC_DIRECTION)
                  cast_spell(hitter, hitter, hitter->direction, hitter->stats.sp, 1, spellPotion, NULL); /* apply potion ALWAYS fire on the spot the applier stands - good for healing - bad for firestorm */
              decrease_ob(hitter);
              break;

            case POISON:
              /* poison drinks */
              /* As with potions, should monster get a save? */
              if (IS_LIVE(victim) && !QUERY_FLAG(victim, FLAG_UNDEAD))
                  apply_poison(victim, hitter);
              break;

              /* Removed case statements that did nothing.
                 * food may be poisonous, but monster must be willing to eat it,
                 * so we don't handle it here.
                 * Containers should perhaps break open, but that code was disabled.
                 */
        }
    }
}

/* adj_attackroll() - adjustments to attacks by various conditions */

int adj_attackroll(object *hitter, object *target)
{
    object *attacker    = hitter;
    int     adjust      = 0;

    /* safety */
    if (!target || !hitter || !hitter->map || !target->map || !on_same_map(hitter, target))
    {
        LOG(llevBug, "BUG: adj_attackroll(): hitter and target not on same map\n");
        return 0;
    }

    /* aimed missiles use the owning object's sight */
    if (is_aimed_missile(hitter))
    {
        if ((attacker = get_owner(hitter)) == NULL)
            attacker = hitter;
    }
    else if (!IS_LIVE(hitter))
        return 0;

    /* determine the condtions under which we make an attack.
     * Add more cases, as the need occurs. */

    /* is invisible means, we can't see it - same for blind */
    if (IS_INVISIBLE(target, attacker) || QUERY_FLAG(attacker, FLAG_BLIND))
        adjust -= 12;

    if (QUERY_FLAG(attacker, FLAG_SCARED))
        adjust -= 3;

    if (QUERY_FLAG(target, FLAG_UNAGGRESSIVE))
        adjust += 1;

    if (QUERY_FLAG(target, FLAG_SCARED))
        adjust += 1;

    if (QUERY_FLAG(attacker, FLAG_CONFUSED))
        adjust -= 3;

    /* if we attack at a different 'altitude' its harder */
    if (QUERY_FLAG(attacker, FLAG_FLYING) != QUERY_FLAG(target, FLAG_FLYING))
        adjust -= 2;
	else if (QUERY_FLAG(attacker, FLAG_LEVITATE) != QUERY_FLAG(target, FLAG_LEVITATE))
			adjust -= 2;

#if 0
  /* slower attacks are less likely to succeed. We should use a
   * comparison between attacker/target speeds BUT, players have
   * a generally faster speed, so this will wind up being a HUGE
   * disadantage for the monsters! Too bad, because missiles which
   * fly fast should have a better chance of hitting a slower target.
   */
  if(hitter->speed<target->speed)
    adjust += ((float) hitter->speed-target->speed);
#endif

#if 0
  LOG(llevDebug,"adj_attackroll() returns %d (%d)\n",adjust,attacker->count);
#endif

    return adjust;
}


/* determine if the object is an 'aimed' missile */
int is_aimed_missile(object *op)
{
    if (op
     && QUERY_FLAG(op, FLAG_FLYING)
     && (op->type == ARROW || op->type == THROWN_OBJ || op->type == FBULLET || op->type == FBALL))
        return 1;
    return 0;
}

/* improved melee test function.
 * test for objects are in range for melee attack.
 * used from attack() functions but also from animation().
 * Return:
 * 0: enemy target is not in melee range.
 * 1: target is in range and we face it.
 * TODO: 2: target is range but not in front.
 * TODO: 3: target is in back
 * NOTE: check for valid target outside here.
 */
int is_melee_range(object *hitter, object *enemy)
{
    int         xt, yt, s;
    object     *tmp;
    mapstruct  *mt;

    for (s = 0; s < 9; s++) /* check squares around AND our own position */
    {
        xt = hitter->x + freearr_x[s];
        yt = hitter->y + freearr_y[s];

        if (!(mt = out_of_map(hitter->map, &xt, &yt)))
            continue;

        for (tmp = enemy; tmp != NULL; tmp = tmp->more)
        {
            if (tmp->map == mt && tmp->x == xt && tmp->y == yt) /* strike! */
                return 1;
        }
    }

    return 0;
}

/* did_make_save_item just checks to make sure the item actually
 * made its saving throw based on the tables.  It does not take
 * any further action (like destroying the item).
 */
/* i disabled this atm! MT-2003 */
int did_make_save_item(object *op, int type, object *originator)
{
    int i, saves = 0, materials = 0, number;


    if (1)
        return 1;

    if (type & AT_CANCELLATION)
        number = ATNR_CANCELLATION;
    else if (type & AT_COLD)
        number = ATNR_COLD;
    else if (type & AT_ELECTRICITY)
        number = ATNR_ELECTRICITY;
    else if (type & AT_FIRE)
        number = ATNR_FIRE;
    else if (type & AT_PHYSICAL)
        number = ATNR_PHYSICAL;
    /* If we are hite by pure magic, the item can get destroyed.
     * But if hit by AT_MAGIC | AT_CONFUSION, it should have no effect.
     */
    else if (type == AT_MAGIC) /* Only pure magic, not paralyze, etc */
        number = ATNR_MAGIC;
    else
        return 1;

    /* If the object is immune, no effect */
    if (op->resist[number] == 100)
        return 1;


    for (i = 0; i < NROFMATERIALS; i++)
    {
        if (op->material & (1 << i))
        {
            materials++;
            if (rndm(1, 20) >= material[i].save[number] - op->magic - op->resist[number] / 100)
                saves++;
            /* if the attack is too weak */
            if ((20 - material[i].save[number]) / 3 > originator->stats.dam)
                saves++;
        }
    }
    if (saves == materials || materials == 0)
        return 1;
    if ((saves == 0) || (rndm(1, materials) > saves))
        return 0;
    return 1;
}

/* This function calls did_make_save_item.  It then performs the
 * appropriate actions to the item (such as burning the item up,
 * calling cancellation, etc.)
 */
/* these stuff don't work in this way anymore! we don' use the attackmode */
void save_throw_object(object *op, int type, object *originator)
{
    if (!did_make_save_item(op, type, originator))
    {
        object     *env = op->env;
        int x = op->x, y = op->y;
        mapstruct  *m   = op->map;

        op = stop_item(op);
        if (op == NULL)
            return;

        /* Hacked the following so that type LIGHTER will work.
         * Also, objects which are potenital "lights" that are hit by
         * flame/elect attacks will be set to glow. "lights" are any
         * object with +/- glow_radius and an "other_arch" to change to.
         * (and please note that we cant fail our save and reach this
         * function if the object doesnt contain a material that can burn.
         * So forget lighting magical swords on fire with this!) -b.t.
         */
        if (type & (AT_FIRE | AT_ELECTRICITY) && op->other_arch && op->glow_radius)
        {
            const char *arch    = op->other_arch->name; /* this should be refcount! */

            op = decrease_ob_nr(op, 1);
            if (op)
                fix_stopped_item(op, m, originator);
            if ((op = get_archetype(arch)) != NULL)
            {
                if (env)
                {
                    op->x = env->x,op->y = env->y;
                    insert_ob_in_ob(op, env);
                    if (env->type == PLAYER && CONTR(env))
                        esrv_send_item(env, op);
                }
                else
                {
                    op->x = x,op->y = y;
                    insert_ob_in_map(op, m, originator, 0);
                }
            }
            return;
        }
        if (type & AT_CANCELLATION)
        {
            /* Cancellation. */
            cancellation(op);
            fix_stopped_item(op, m, originator);
            return;
        }
        if (op->nrof > 1)
        {
            op = decrease_ob_nr(op, rndm(0, op->nrof - 1));
            if (op)
                fix_stopped_item(op, m, originator);
        }
        else
        {
            if (op->env)
            {
                object *tmp = is_player_inv(op->env);

                if (tmp)
                {
                    esrv_del_item(CONTR(tmp), op->count, op->env);
                    esrv_update_item(UPD_WEIGHT, tmp, tmp);
                }
            }
            if (!QUERY_FLAG(op, FLAG_REMOVED))
                destruct_ob(op);
        }
        if (type & (AT_FIRE | AT_ELECTRICITY))
        {
            if (env)
            {
                op = get_archetype("burnout");
                op->x = env->x,op->y = env->y;
                insert_ob_in_ob(op, env);
            }
            else
            {
                replace_insert_ob_in_map("burnout", originator);
            }
        }
        return;
    }
    /* The value of 50 is arbitrary. */
    if (type & AT_COLD && (op->resist[ATNR_COLD] < 50) && !QUERY_FLAG(op, FLAG_NO_PICK) && (RANDOM() & 2))
    {
        /*
               object *tmp;
               archetype *at = find_archetype("icecube");
               if (at == NULL)
                 return;
               op = stop_item (op);
               if (op == NULL)
                   return;
               if ((tmp = present_arch(at,op->map,op->x,op->y)) == NULL) {
                 tmp = arch_to_object(at);
                 tmp->x=op->x,tmp->y=op->y;
                 insert_ob_in_map(tmp,op->map,originator,0);
               }
               if ( !QUERY_FLAG(op, FLAG_REMOVED) )
                   remove_ob(op);
               (void) insert_ob_in_ob(op,tmp);
               return;
            */
    }
}
