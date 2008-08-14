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

/* flags for hit_player_attacktype() */
/* important: the first 5 bits are the same is in material_base_xx in material.h */
#define HIT_FLAG_PARALYZED_ADD 32
#define HIT_FLAG_PARALYZED_REM 64
#define HIT_FLAG_DMG           128
#define HIT_FLAG_DMG_ACID      256
#define HIT_FLAG_DMG_WMAGIC    512
/* HIT_FLAG_WEAPON      ==    1024 is defined in global.h */

#define ATTACK_HIT_DAMAGE(_op, _anum)       dam=dam*((double)_op->attack[_anum]*(double)0.01);dam>=1.0f?(damage=(int)dam):(damage=1)
#define ATTACK_RESIST_DAMAGE(_op, _anum)    dam=dam*((double)(100-_op->resist[_anum])*(double)0.01)

/* some static defines */
static int  abort_attack(object *target, object *hitter, int env_attack);
static void send_attack_msg(object *op, object *hitter, int attacknum, int dam, int damage);
static int  hit_player_attacktype(object *op, object *hitter, int *flags, int damage, uint32 attacknum, int magic);

/* check and adjust all pre-attack issues like checking attack is valid,
 * attack objects are in the right state and such.
 */
static int get_attack_mode(object **target, object **hitter, int *env_attack)
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
        *env_attack = ENV_ATTACK_YES;
        return 0;
    }
    if (QUERY_FLAG(*target, FLAG_REMOVED)
        || QUERY_FLAG(*hitter, FLAG_REMOVED)
        || (*hitter)->map == NULL
        || !on_same_tileset((*hitter), (*target)))
    {
        LOG(llevBug, "BUG: hitter (arch %s, name %s) with no relation to target\n", (*hitter)->arch->name,
            query_name(*hitter));
        return 1;
    }
    *env_attack = ENV_ATTACK_NO;
    return 0;
}

/* compare the attacker and target and adjust the attack roll.
 * Like the attacker is invisible - adjust the roll depending on the
 * fact the target can see invisible or not - just as a example.
 */
static inline int adj_attackroll(object *hitter, object *target, int adjust)
{
    object *attacker    = hitter;

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
        adjust -= adjust/25; /* - 25% */

    if (QUERY_FLAG(target, FLAG_SCARED))
        adjust += adjust/10; /* + 10% */

    /* if we attack at a different 'altitude' its harder - remove 10% */
    if (QUERY_FLAG(attacker, FLAG_FLYING) != QUERY_FLAG(target, FLAG_FLYING))
        adjust -= adjust/10;
    else if (QUERY_FLAG(attacker, FLAG_LEVITATE) != QUERY_FLAG(target, FLAG_LEVITATE))
    adjust -= adjust/10;

    return adjust;
}

//#define ATTACK_DEBUG
/* here we decide a attack will happen and how. blocking, parry, missing is handled
 * here inclusive the sounds. All whats needed before we count damage and say "hit you".
 */
int attack_ob(object *target, object *hitter, object *hit_obj)
{
    int     hitdam, env_attack, roll;
    tag_t   op_tag, hitter_tag;

    /* get_attack_mode will pre-check and adjust *ANY* topic.
     * Including setting ->head objects and checking maps
     */
    if (get_attack_mode(&target, &hitter, &env_attack))
        goto error;

    if(trigger_object_plugin_event(EVENT_ATTACK,
            target, hitter, hitter, NULL, NULL, NULL, NULL, SCRIPT_FIX_ALL))
        goto error;

    if(!hit_obj)
        hit_obj = hitter;

    op_tag = target->count;
    hitter_tag = hitter->count;
    hitdam  = hit_obj->stats.dam;

    /* Fight Step 1: Get the random hit value */
    roll = random_roll(0, 100);

    /* Fight Step 2: Adjust for special cases. Target is invinsible we can't see it? Is levitating? ... */
    if (env_attack == ENV_ATTACK_NO)
        roll = adj_attackroll(hitter, target, roll);

#ifdef ATTACK_DEBUG
    if (hitter->type == PLAYER)
        new_draw_info_format(NDI_RED, 0, hitter, "You roll: %d thac m:%d 0:%d (wc/roll: %d(%d) ac:%d)!", roll,hitter->stats.thac0,hitter->stats.thacm, hit_obj->stats.wc,hit_obj->stats.wc+roll,target->stats.ac);
    if (target->type == PLAYER)
        new_draw_info_format(NDI_RED, 0, target, "Hitter roll: %d thac m:%d 0:%d (wc/roll: %d(%d) ac:%d)!", roll,hitter->stats.thac0,hitter->stats.thacm, hit_obj->stats.wc,hit_obj->stats.wc+roll,target->stats.ac);
#endif

    if (hitter->type == PLAYER)
        CONTR(hitter)->anim_flags |= PLAYER_AFLAG_ENEMY; /* so we do one swing */

    /* Force player to face enemy */
    if (hitter->type == PLAYER)
    {
        rv_vector   dir;
        if(get_rangevector(hitter, target, &dir, RV_NO_DISTANCE))
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

    /* don't get in hp recovery mode when something is hitting you */
    if(target->type == PLAYER)
        CONTR(target)->damage_timer = PLAYER_HPGEN_DELAY;

    /* Fight Step 3: Lets see our base THAC triggers - garantied miss and direct hit */
    /* note 1: the attach messages for default THAC hit/miss are important when we add fight styles
     * note 2: notice the "<" instead of "<=". This will ensure that an object with thacm = 0, thac0 = 0 will
     * always hit (spells/traps...) and a thac = 101 will result in an always failed hit (why ever)
     */
    if(roll < hitter->stats.thacm) /* we have wc/ac independent miss! we call this default miss FUMBLE */
    {
        hitdam = 0;
        if (hitter->type == MONSTER || hitter->type == PLAYER)
        {
            if (target->type == PLAYER)
                new_draw_info_format(NDI_PURPLE, 0, target, "%s fumbles!", hitter->name);
            if (hitter->type == PLAYER)
                new_draw_info(NDI_ORANGE, 0, hitter, "You fumble!");

            if (hitter->type == PLAYER)
                play_sound_map(hitter->map, hitter->x, hitter->y, SOUND_MISS_PLAYER, SOUND_NORMAL);
            else
                play_sound_map(hitter->map, hitter->x, hitter->y, SOUND_MISS_MOB, SOUND_NORMAL);
        }
    }
    /* Fight Step 3 - 2nd part: if thac0 is >= roll then don't use AC/WC - we call it a DIRECT HIT */
    /* note: Normally this would be a good "critical hit" but i want use critical hits for fight
     * styles.
     */
    else if (roll >= hitter->stats.thac0)
    {
        if (hitter->type == MONSTER || hitter->type == PLAYER)
        {
            if (target->type == PLAYER)
                new_draw_info_format(NDI_PURPLE, 0, target, "%s Direct Hit! (+20%% damage)", hitter->name);
            if (hitter->type == PLAYER)
                new_draw_info(NDI_ORANGE, 0, hitter, "Direct Hit! (+20% damage)");
            hitdam = (int) (hitdam * 1.2f);
        }

        /* ugly but very useful goto */
        goto force_direct_hit;

    }
    /* Fight Step 4: compare AC with WC + thac-roll - if AC is lower we have a normal hit */
    else if (hit_obj->stats.wc + roll >= target->stats.ac)
    {
        force_direct_hit:
        CLEAR_FLAG(target, FLAG_SLEEP); /* at this point NO ONE will still sleep */

        /* i don't use sub_type atm - using it should be smarter in the future */
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

        /* we hook thrown potions and stuff in here? we should perhaps move this */
        if (env_attack == ENV_ATTACK_NO && hitter->type == POTION)
        {
            if (hitter->stats.sp != SP_NO_SPELL && spells[hitter->stats.sp].flags & SPELL_DESC_DIRECTION)
                cast_spell(hitter, hitter, hitter->direction, hitter->stats.sp, 1, spellPotion, NULL); /* apply potion ALWAYS fire on the spot the applier stands - good for healing - bad for firestorm */
            decrease_ob(hitter);

            if (was_destroyed(hitter, hitter_tag) || was_destroyed(target, op_tag)
                || abort_attack(target, hitter, env_attack))
                goto leave;
        }

        /* Need to do at least 1 damage, otherwise there is no point
            * to go further and it will cause FPE's below.
            */
        if (hitdam <= 0)
            hitdam = 1;

        /* Handle monsters that hit back */
        if (env_attack  == ENV_ATTACK_NO && QUERY_FLAG(target, FLAG_HITBACK) && IS_LIVE(hitter))
        {
            damage_ob(hitter, random_roll(0, target->stats.dam), target, env_attack);

            if (was_destroyed(target, op_tag) || was_destroyed(hitter, hitter_tag)
                || abort_attack(target, hitter, env_attack))
                goto leave;
        }

        /* the damage is between 70 and 100% of the (adjusted) base damage */
        hitdam = damage_ob(target, random_roll((int)(hitdam*0.7f)+1, hitdam), hitter, env_attack);
        if (was_destroyed(target, op_tag) || was_destroyed(hitter, hitter_tag) || abort_attack(target, hitter, env_attack))
            goto leave;
    }
    else /* we missed, dam=0 */
    {
        hitdam = 0;
        if (hitter->type != ARROW)
        {
            if (target->type == PLAYER)
                new_draw_info_format(NDI_PURPLE, 0, target, "%s misses you!", hitter->name);
            if (hitter->type == PLAYER)
                new_draw_info_format(NDI_ORANGE, 0, hitter, "you miss %s!", target->name);

            if (hitter->type == PLAYER)
                play_sound_map(hitter->map, hitter->x, hitter->y, SOUND_MISS_PLAYER, SOUND_NORMAL);
            else
                play_sound_map(hitter->map, hitter->x, hitter->y, SOUND_MISS_MOB, SOUND_NORMAL);
        }
    }

    goto leave;

    error : hitdam = 1;
    goto leave;

    leave : return hitdam;
}

/* damage_ob() (old: hit_player()) is called to generate damage - its the main hit function
 * when a monster, player or other "attackable" object is really damaged in terms of hp.
 */
int damage_ob(object *op, int dam, object *hitter, int env_attack)
{
    object *hit_obj, *target_obj, *aggro_obj, *dmg_obj=NULL;
    int     maxdam      = 0, flags = 0;
    int     attacknum, hit_level;
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
            "DEBUG: damage_ob(): hit or target object level == 0(h:>%s< (o:>%s<) l->%d t:>%s< (>%s<)(o:>%s<) l->%d\n",
            query_name(hitter), query_name(get_owner(hitter)), hit_level, query_name(op), target_obj->arch->name,
            query_name(get_owner(op)), target_obj->level);

    /* New (but still very basic) code for avoiding mobs (and players) on
     * the same side to damage each other. */
    if(get_friendship(hit_obj, target_obj) >= FRIENDSHIP_HELP)
    {
        LOG(llevDebug, "DEBUG: damage_ob(): friendly hit ('%s' => '%s' / '%s' => '%s') ignored\n",
                query_name(hitter), query_name(op),
                query_name(hit_obj), query_name(target_obj));
        return 0;
    }

    if (hit_level > target_obj->level && hit_obj->type == MONSTER) /* i turned it now off for players! */
    {
        dam += (int) ((float) (dam / 2) * ((float) (hit_level - target_obj->level)
                           / (target_obj->level > 25 ? 25.0f : (float) target_obj->level)));
        /*LOG(llevDebug,"DMG-ADD: hl:%d tl_%d -> d:%d \n", hit_level,target_obj->level, dam);*/
    }
    /* something hit player - can be disease or poison too - breaks resting or eating force! */
    if (op->type == PLAYER && CONTR(op)->rest_mode)
        CONTR(op)->rest_mode = 0;

    /* Check for pvp! Only when at THIS moment both possible player are in pvp area - then we do damage.
     * This avoid this kind of heros, standing on pvp border, firing in and running back to save.
     * on the other side, running in safe areas will help you when hunted - and thats always a great fun.
     */
    if (op->type == PLAYER || (target_obj!=op && op->owner->type == PLAYER))
    {
        if (hitter->type == PLAYER || (hit_obj!=hitter && hitter->owner->type == PLAYER))
        {
            /* now we are sure player are involved. Get the real player object now and test! */
            if (!pvp_area(op->type == PLAYER ? op : target_obj, hitter->type == PLAYER ? hitter : hit_obj))
                return 0;
        }
    }


    /* this checks objects are valid, on same map and set them to head when needed! */
    /* also, simple_attack is set to 1, when one of the parts hav ->env != null
     * atm, this value is only used in the door attack */
    if(env_attack == ENV_ATTACK_CHECK)
    {
        if (get_attack_mode(&op, &hitter, &env_attack))
            return 0;
    }

    op_tag = op->count;
    hitter_tag = hitter->count;

    /* A last safery check - this happened in the past sometimes */
    if (op->stats.hp <= 0)
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

    flags = 0; /* important flags to track actions BETWEEN single effect hits of ONE attack */

    /* Go through and hit the player with each attacktype, one by one.
    * hit_player_attacktype only figures out the damage, doesn't inflict
    * it.  It will do the appropriate action for attacktypes with
    * effects (slow, paralization, etc.
    */
    for (attacknum = 0; attacknum < NROFATTACKS; attacknum++)
    {
        if (hitter->attack[attacknum])
        {
            /*          LOG(-1, "hitter: %f - %s (dam:%d/%d) (wc:%d/%d)(ac:%d/%d) ap:%d\n",hitter->speed,
                            hitter->name,hitter->stats.dam,op->stats.dam, hitter->stats.wc,op->stats.wc,
                            hitter->stats.ac,op->stats.ac,hitter->attack[attacknum]);
            */
            maxdam += hit_player_attacktype(op, hitter, &flags, dam, attacknum, 0);
        }
    }

    /* don't get in hp recovery mode when something is hitting you */
    if(op->type == PLAYER)
        CONTR(op)->damage_timer = PLAYER_HPGEN_DELAY;

    /* attack is done - lets check we have possible item dmg */
    if(flags & HIT_FLAG_DMG)
    {
        int num = 1, chance = 2; /* base dmg chance for an dmg hit = 2% - for one item */

        if(op->type == PLAYER)
        {
            /* evil: bad dmg effects are stacking */
            if(flags & HIT_FLAG_DMG_ACID)
            {
                num +=2; /* base chance for dmg = 3 items */
                chance +=4; /* increase dmg chance by +4% */
            }
            if(flags & HIT_FLAG_DMG_WMAGIC)
            {
                num +=1; /* base chance for dmg = 2 items */
                chance +=8; /* increase dmg chance by 8%  */
            }
            flags &= (MATERIAL_BASE_PHYSICAL|MATERIAL_BASE_ELEMENTAL|MATERIAL_BASE_MAGICAL|MATERIAL_BASE_SPHERICAL|MATERIAL_BASE_SPECIAL);
            material_attack_damage(op, num, chance, flags);
        }

        /* now lets check our attacker is a player - if so, give weapon an extra chance of being damaged */
        if(hitter->type == PLAYER)
        {
            flags = (MATERIAL_BASE_PHYSICAL|MATERIAL_BASE_ELEMENTAL|MATERIAL_BASE_MAGICAL|MATERIAL_BASE_SPHERICAL|MATERIAL_BASE_SPECIAL|HIT_FLAG_WEAPON);
            material_attack_damage(hitter, num, chance/2, flags);
        }
    }

    /* we insert the aggro data in the mob, and report to the AI system */
    SET_FLAG(op, FLAG_NO_FIX_PLAYER);
    SET_FLAG(hitter, FLAG_NO_FIX_PLAYER);
    aggro_obj = aggro_update_info(op, target_obj, hitter, hit_obj, maxdam<op->stats.hp?maxdam:op->stats.hp, 0);
    CLEAR_FLAG(hitter, FLAG_NO_FIX_PLAYER);
    CLEAR_FLAG(op, FLAG_NO_FIX_PLAYER);

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

	/* lets kill, kill, kill... */
	if (op->stats.hp <= 0 && op->type == PLAYER && !QUERY_FLAG(op, FLAG_WIZ))
	{
		kill_player(op);
		return maxdam;  /* nothing more to do for wall */
	}
	/* Eneq(@csd.uu.se): Check to see if monster runs away. */
	/* TODO: gecko: this should go into a behaviour... */
	else if ((op->stats.hp >= 0) && QUERY_FLAG(op, FLAG_MONSTER)
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

    if ((rtn_kill = kill_object(op, dam, hitter, 0)))
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
    else if (!OBJECT_FREE(op) && QUERY_FLAG(op, FLAG_SPLITTING))
    {
        /* TODO: this might need some updating for the new AI system
         * gecko 2006-05-01 */
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
    return maxdam;
}

/* if we drop for example a spell object like a fireball to the map,
 * they move from tile to tile. Every time they check the object they
 * "hit on this map tile". If they find some - we are here.
 */
int hit_map(object *op, int dir)
{
    object     *tmp, *next, *tmp_obj;
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

        /* monsters on the same side don't hurt each other */
        if(get_friendship(tmp_obj, tmp) >= FRIENDSHIP_HELP)
            continue;

         /* Can only hit live objects (for now) */
         if(!IS_LIVE(tmp))
            continue;

        damage_ob(tmp, op->stats.dam, op, ENV_ATTACK_CHECK);
        retflag |= 1;
        if (was_destroyed(op, op_tag))
            break;
    }

    return 0;
}

/* we need this called spread in the function before because sometimes we want drop
* a message BEFORE we tell the damage and sometimes we want a message after it.
*/
static inline void send_resist_msg(object *op, object *hitter, int attacknum)
{
	if (op->type == PLAYER)
	{
		new_draw_info_format(NDI_GREY, 0, op, "You resists the %s attack!", attack_name[attacknum]);
	}
	/* i love C... ;) */
	if (hitter->type == PLAYER || ((hitter = get_owner(hitter)) && hitter->type == PLAYER))
	{
		new_draw_info_format(NDI_GREY, 0, hitter, "%s resists the %s attack!", op->name, attack_name[attacknum]);
	}
}

/* This returns the amount of damage hitter does to op with the
 * appropriate attacktype.  Only 1 attacktype should be set at a time.
 * This doesn't damage the player, but returns how much it should
 * take.  However, it will do other effects (paralyzation, slow, etc.)
 * Note - changed for PR code - we now pass the attack number and not
 * the attacktype.  Makes it easier for the PR code.  */
static int hit_player_attacktype(object *op, object *hitter, int *flags, int damage, uint32 attacknum, int magic)
{
    double  dam         = (double) damage;
    int     doesnt_slay = 1;

    /* just a sanity check */
    if (dam < 0)
    {
        LOG(llevBug, "BUG: hit_player_attacktype called with negative damage: %d from object: %s\n", dam, query_name(op));
        return 0;
    }

    /* any damage or "touch" will break the paralyze - except the effect was done in the same loop */
    if( !(*flags&HIT_FLAG_PARALYZED_ADD) && attacknum != ATNR_PARALYZE && QUERY_FLAG(op,FLAG_PARALYZED))
    {
        *flags |=HIT_FLAG_PARALYZED_REM; /* so we can track stacked hits */
        remove_paralyze(op);
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
            if (op->resist[attacknum] == 100)
            {
                dam = 0;
                send_attack_msg(op, hitter, attacknum, (int) dam, damage);
                return (int) dam;
            }

            if (op->resist[attacknum])
                ATTACK_RESIST_DAMAGE(op, attacknum); /* reduce to % resistance */
        }

        if (damage && dam < 1.0)
            dam = 1.0;

        send_attack_msg(op, hitter, attacknum, (int) dam, damage);
        return (int) dam;
    }

    /* quick check for immunity - if so, we skip here.
        * our formula is (100-resist)/100 - so test for 100 = zero division
     */
    if (op->resist[attacknum] == 100)
    {
        switch (attacknum)
        {
            case ATNR_PHYSICAL:
            case ATNR_SLASH:
            case ATNR_CLEAVE:
            case ATNR_PIERCE:
                *flags |=HIT_FLAG_DMG|MATERIAL_BASE_PHYSICAL; /* we are maybe immun - but some items not */
                /* be sure we remove paralyze when we do a "immun paralyze hit" */
                if(!(*flags&HIT_FLAG_PARALYZED_ADD) && QUERY_FLAG(op,FLAG_PARALYZED))
                    remove_paralyze(op);
                break;

            case ATNR_COLD:
            case ATNR_FIRE:
            case ATNR_ELECTRICITY:
            case ATNR_POISON:
            case ATNR_ACID:
            case ATNR_SONIC:
                *flags |=HIT_FLAG_DMG|MATERIAL_BASE_ELEMENTAL; /* we are maybe immun - but some items not */
                /* be sure we remove paralyze when we do a "immun paralyze hit" */
                if(!(*flags&HIT_FLAG_PARALYZED_ADD) && QUERY_FLAG(op,FLAG_PARALYZED))
                    remove_paralyze(op);
                break;

            case ATNR_MAGIC:
            case ATNR_LIGHT:
            case ATNR_SHADOW:
            case ATNR_PSIONIC:
            case ATNR_LIFESTEAL:
                *flags |=HIT_FLAG_DMG|MATERIAL_BASE_MAGICAL; /* we are maybe immun - but some items not */
                /* be sure we remove paralyze when we do a "immun paralyze hit" */
                if(!(*flags&HIT_FLAG_PARALYZED_ADD) && QUERY_FLAG(op,FLAG_PARALYZED))
                    remove_paralyze(op);
                break;

            case ATNR_DEATH:
            case ATNR_CHAOS:
            case ATNR_AETHER:
            case ATNR_NETHER:
                *flags |=HIT_FLAG_DMG|MATERIAL_BASE_SPHERICAL; /* we are maybe immun - but some items not */
                /* be sure we remove paralyze when we do a "immun paralyze hit" */
                if(!(*flags&HIT_FLAG_PARALYZED_ADD) && QUERY_FLAG(op,FLAG_PARALYZED))
                    remove_paralyze(op);
                break;

            case ATNR_WEAPONMAGIC:
            case ATNR_GODPOWER:
                *flags |=HIT_FLAG_DMG|MATERIAL_BASE_SPECIAL; /* we are maybe immun - but some items not */
                /* be sure we remove paralyze when we do a "immun paralyze hit" */
                if(!(*flags&HIT_FLAG_PARALYZED_ADD) && QUERY_FLAG(op,FLAG_PARALYZED))
                    remove_paralyze(op);
            break;
        }
		send_resist_msg(op, hitter, attacknum);
		return 0;
    }

    switch (attacknum)
    {
        case ATNR_PHYSICAL:
          check_physically_infect(op, hitter); /* quick check for desease! */

        /* these are "pure" damage attacks */
        case ATNR_SLASH:
        case ATNR_CLEAVE:
        case ATNR_PIERCE:
            *flags |=HIT_FLAG_DMG|MATERIAL_BASE_PHYSICAL;
            ATTACK_HIT_DAMAGE(hitter, attacknum);       /* get % of dam from this attack form */
            if (op->resist[attacknum])
                ATTACK_RESIST_DAMAGE(op, attacknum);    /* reduce to % resistance */
            if (damage && dam < 1.0)
                dam = 1.0;
            send_attack_msg(op, hitter, attacknum, (int) dam, damage);
            break;

        case ATNR_COLD:
        case ATNR_FIRE:
        case ATNR_ELECTRICITY:
        case ATNR_SONIC:
            *flags |=HIT_FLAG_DMG|MATERIAL_BASE_ELEMENTAL;
            ATTACK_HIT_DAMAGE(hitter, attacknum);       /* get % of dam from this attack form */
            if (op->resist[attacknum])
                ATTACK_RESIST_DAMAGE(op, attacknum);    /* reduce to % resistance */
            if (damage && dam < 1.0)
                dam = 1.0;
            send_attack_msg(op, hitter, attacknum, (int) dam, damage);
            break;

        case ATNR_MAGIC:
            *flags |=HIT_FLAG_DMG|MATERIAL_BASE_MAGICAL;
            ATTACK_HIT_DAMAGE(hitter, attacknum);       /* get % of dam from this attack form */
            if (op->resist[attacknum])
                ATTACK_RESIST_DAMAGE(op, attacknum);    /* reduce to % resistance */
            if (damage && dam < 1.0)
                dam = 1.0;
            send_attack_msg(op, hitter, attacknum, (int) dam, damage);
            break;

        case ATNR_WEAPONMAGIC:
            *flags |=(HIT_FLAG_DMG|HIT_FLAG_DMG_WMAGIC|MATERIAL_BASE_SPECIAL);
            ATTACK_HIT_DAMAGE(hitter, attacknum);       /* get % of dam from this attack form */
            if (op->resist[attacknum])
                ATTACK_RESIST_DAMAGE(op, attacknum);    /* reduce to % resistance */
            if (damage && dam < 1.0)
                dam = 1.0;
            send_attack_msg(op, hitter, attacknum, (int) dam, damage);
            break;

        case ATNR_POISON:
            *flags |=HIT_FLAG_DMG|MATERIAL_BASE_ELEMENTAL;
            ATTACK_HIT_DAMAGE(hitter, attacknum);       /* get % of dam from this attack form */
            if (op->resist[attacknum])
                ATTACK_RESIST_DAMAGE(op, attacknum);    /* reduce to % resistance */
            if (damage && dam < 1.0)
                dam = 1.0;
            send_attack_msg(op, hitter, attacknum, (int) dam, damage);
            /* i must adjust this... alive stand for mobs and player include
            * golems, undead and demons - but not for doors or "technical objects"
            * like earth walls.
            */
            if (dam && IS_LIVE(op))
                poison_player(op, hitter, (float) dam);
            break;

        case ATNR_ACID: /* do greater equipment damage */
            *flags |=(HIT_FLAG_DMG|HIT_FLAG_DMG_ACID|MATERIAL_BASE_ELEMENTAL);
            ATTACK_HIT_DAMAGE(hitter, attacknum);       /* get % of dam from this attack form */
            if (op->resist[attacknum])
                ATTACK_RESIST_DAMAGE(op, attacknum);    /* reduce to % resistance */
            if (damage && dam < 1.0)
                dam = 1.0;
            send_attack_msg(op, hitter, attacknum, (int) dam, damage);
            /*
            if(random_roll(0, (int)dam+4) >
            random_roll(0, 39)+2*tmp->magic) {
            if(op->type == PLAYER)
            new_draw_info_format(NDI_UNIQUE|NDI_RED,0, op,
            "The %s's acid corrodes your %s!",
            query_name(hitter), query_name(tmp));
            flag = 1;
            tmp->magic--;
            if(op->type == PLAYER)
            esrv_send_item(op, tmp);
            }
            */
            break;

        case ATNR_LIGHT: /* has a chance to blind the target */
            *flags |=HIT_FLAG_DMG|MATERIAL_BASE_MAGICAL;
            ATTACK_HIT_DAMAGE(hitter, attacknum);       /* get % of dam from this attack form */
            if (op->resist[attacknum])
                ATTACK_RESIST_DAMAGE(op, attacknum);    /* reduce to % resistance */
            if (damage && dam < 1.0)
                dam = 1.0;
            send_attack_msg(op, hitter, attacknum, (int) dam, damage);
            /* TODO: add blinding effect */
            break;

        case ATNR_SHADOW:
            *flags |=HIT_FLAG_DMG|MATERIAL_BASE_MAGICAL;
            ATTACK_HIT_DAMAGE(hitter, attacknum);       /* get % of dam from this attack form */
            if (op->resist[attacknum])
                ATTACK_RESIST_DAMAGE(op, attacknum);    /* reduce to % resistance */
            if (damage && dam < 1.0)
                dam = 1.0;
            send_attack_msg(op, hitter, attacknum, (int) dam, damage);
            /* TODO: add special effects */
            break;

        case ATNR_PSIONIC:
            *flags |=HIT_FLAG_DMG|MATERIAL_BASE_MAGICAL;
            ATTACK_HIT_DAMAGE(hitter, attacknum);       /* get % of dam from this attack form */
            if (op->resist[attacknum])
                ATTACK_RESIST_DAMAGE(op, attacknum);    /* reduce to % resistance */
            if (damage && dam < 1.0)
                dam = 1.0;
            send_attack_msg(op, hitter, attacknum, (int) dam, damage);
            /* TODO: add special effects */
            break;

        case ATNR_LIFESTEAL:
            *flags |=HIT_FLAG_DMG|MATERIAL_BASE_MAGICAL;
            ATTACK_HIT_DAMAGE(hitter, attacknum);       /* get % of dam from this attack form */
            if (op->resist[attacknum])
                ATTACK_RESIST_DAMAGE(op, attacknum);    /* reduce to % resistance */
            if (damage && dam < 1.0)
                dam = 1.0;
            send_attack_msg(op, hitter, attacknum, (int) dam, damage);
            /* TODO: add lifesteal effect (this is old code )
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
            break;

        case ATNR_DEATH:
            *flags |=HIT_FLAG_DMG|MATERIAL_BASE_SPHERICAL;
            ATTACK_HIT_DAMAGE(hitter, attacknum);       /* get % of dam from this attack form */
            if (op->resist[attacknum])
                ATTACK_RESIST_DAMAGE(op, attacknum);    /* reduce to % resistance */
            if (damage && dam < 1.0)
                dam = 1.0;
            send_attack_msg(op, hitter, attacknum, (int) dam, damage);
            /* TODO: add special effects */
            break;
        case ATNR_CHAOS:
            *flags |=HIT_FLAG_DMG|MATERIAL_BASE_SPHERICAL;
            ATTACK_HIT_DAMAGE(hitter, attacknum);       /* get % of dam from this attack form */
            if (op->resist[attacknum])
                ATTACK_RESIST_DAMAGE(op, attacknum);    /* reduce to % resistance */
            if (damage && dam < 1.0)
                dam = 1.0;
            send_attack_msg(op, hitter, attacknum, (int) dam, damage);
            /* TODO: add special effects */
            break;
        case ATNR_AETHER:
            *flags |=HIT_FLAG_DMG|MATERIAL_BASE_SPHERICAL;
            ATTACK_HIT_DAMAGE(hitter, attacknum);       /* get % of dam from this attack form */
            if (op->resist[attacknum])
                ATTACK_RESIST_DAMAGE(op, attacknum);    /* reduce to % resistance */
            if (damage && dam < 1.0)
                dam = 1.0;
            send_attack_msg(op, hitter, attacknum, (int) dam, damage);
            /* TODO: add special effects */
            break;
        case ATNR_NETHER:
            *flags |=HIT_FLAG_DMG|MATERIAL_BASE_SPHERICAL;
            ATTACK_HIT_DAMAGE(hitter, attacknum);       /* get % of dam from this attack form */
            if (op->resist[attacknum])
                ATTACK_RESIST_DAMAGE(op, attacknum);    /* reduce to % resistance */
            if (damage && dam < 1.0)
                dam = 1.0;
            send_attack_msg(op, hitter, attacknum, (int) dam, damage);
            /* TODO: add special effects */
            break;
        case ATNR_GODPOWER:
            *flags |=HIT_FLAG_DMG|MATERIAL_BASE_SPECIAL;
            ATTACK_HIT_DAMAGE(hitter, attacknum);       /* get % of dam from this attack form */
            if (op->resist[attacknum])
                ATTACK_RESIST_DAMAGE(op, attacknum);    /* reduce to % resistance */
            if (damage && dam < 1.0)
                dam = 1.0;
            send_attack_msg(op, hitter, attacknum, (int) dam, damage);
            /* TODO: add special effects */
            break;

        /* all the following attacks does no damage but effects */
        case ATNR_DRAIN:
            dam = 0.0;
            if(hitter->attack[attacknum] > (RANDOM()%100)) /* we hit with effect? */
            {
                if (hitter->type == PLAYER)
                    new_draw_info_format(NDI_ORANGE, 0, hitter, "You drain %s!", op->name);
                if (op->type == PLAYER)
                    new_draw_info_format(NDI_PURPLE, 0, op, "%s drains you!", hitter->name);

                /* give the target the chance to resist */
                if(op->resist[attacknum] > (RANDOM()%100)) /* resisted? */
                {
                    if (hitter->type == PLAYER)
                        new_draw_info_format(NDI_YELLOW, 0, hitter, "%s resists the drain!", op->name);
                    if (op->type == PLAYER)
                        new_draw_info(NDI_YELLOW, 0, op, "You resist!");
                }
                else /* effect has hit! */
                    drain_level(op, 1, op->type == PLAYER?0:1, 85+(RANDOM()%70));
            }
            break;

        case ATNR_DEPLETION:
            dam = 0.0;
            if(hitter->attack[attacknum] > (RANDOM()%100)) /* we hit with effect? */
            {
                if (hitter->type == PLAYER)
                    new_draw_info_format(NDI_ORANGE, 0, hitter, "You deplete %s!", op->name);
                if (op->type == PLAYER)
                    new_draw_info_format(NDI_PURPLE, 0, op, "%s depletes you!", hitter->name);

                /* give the target the chance to resist */
                if(op->resist[attacknum] > (RANDOM()%100)) /* resisted? */
                {
                    if (hitter->type == PLAYER)
                        new_draw_info_format(NDI_YELLOW, 0, hitter, "%s resists depletion!", op->name);
                    if (op->type == PLAYER)
                        new_draw_info(NDI_YELLOW, 0, op, "You resist!");
                }
                else /* effect has hit! */
                    drain_stat(op);
            }
            break;
        case ATNR_CORRUPTION:
            dam = 0.0;
            if(hitter->attack[attacknum] > (RANDOM()%100)) /* we hit with effect? */
            {
                if (hitter->type == PLAYER)
                    new_draw_info_format(NDI_ORANGE, 0, hitter, "You corrupt %s!", op->name);
                if (op->type == PLAYER)
                    new_draw_info_format(NDI_PURPLE, 0, op, "%s corrupts you!", hitter->name);

                /* give the target the chance to resist */
                if(op->resist[attacknum] > (RANDOM()%100)) /* resisted? */
                {
                    if (hitter->type == PLAYER)
                        new_draw_info_format(NDI_YELLOW, 0, hitter, "%s resists the corruption!", op->name);
                    if (op->type == PLAYER)
                        new_draw_info(NDI_YELLOW, 0, op, "You resist!");
                }
                else /* effect has hit! */
                    corrupt_stat(op);
            }
            break;
        case ATNR_CONFUSION:
            dam = 0.0;
            if(hitter->attack[attacknum] > (RANDOM()%100)) /* we hit with effect? */
            {
                if (hitter->type == PLAYER)
                    new_draw_info_format(NDI_ORANGE, 0, hitter, "You confuse %s!", op->name);
                if (op->type == PLAYER)
                    new_draw_info_format(NDI_PURPLE, 0, op, "%s confuses you!", hitter->name);

                /* give the target the chance to resist */
                if(op->resist[attacknum] > (RANDOM()%100)) /* resisted? */
                {
                    if (hitter->type == PLAYER)
                        new_draw_info_format(NDI_YELLOW, 0, hitter, "%s resist!", op->name);
                    if (op->type == PLAYER)
                        new_draw_info(NDI_YELLOW, 0, op, "You resists confusion!");
                }
                else /* effect has hit! */
                    confuse_player(op, hitter, 160);
            }
            break;
        case ATNR_SLOW:
            dam = 0.0;
            if(hitter->attack[attacknum] > (RANDOM()%100)) /* we hit with effect? */
            {
                if (hitter->type == PLAYER)
                    new_draw_info_format(NDI_ORANGE, 0, hitter, "You slow %s!", op->name);
                if (op->type == PLAYER)
                    new_draw_info_format(NDI_PURPLE, 0, op, "%s slows you!", hitter->name);

                /* give the target the chance to resist */
                if(op->resist[attacknum] > (RANDOM()%100)) /* resisted? */
                {
                    if (hitter->type == PLAYER)
                        new_draw_info_format(NDI_YELLOW, 0, hitter, "%s resist the slow effect!", op->name);
                    if (op->type == PLAYER)
                        new_draw_info(NDI_YELLOW, 0, op, "You resist!");
                }
                else /* effect has hit! */
                    slow_player(op,hitter, 5);
            }
            break;
        case ATNR_FEAR:
            dam = 0.0;
            if(hitter->attack[attacknum] > (RANDOM()%100)) /* we hit with effect? */
            {
                if (hitter->type == PLAYER)
                    new_draw_info_format(NDI_ORANGE, 0, hitter, "You scared %s!", op->name);
                if (op->type == PLAYER)
                    new_draw_info_format(NDI_PURPLE, 0, op, "%s scares you!", hitter->name);

                /* give the target the chance to resist */
                if(op->resist[attacknum] > (RANDOM()%100)) /* resisted? */
                {
                    if (hitter->type == PLAYER)
                        new_draw_info_format(NDI_YELLOW, 0, hitter, "%s resists fear!", op->name);
                    if (op->type == PLAYER)
                        new_draw_info(NDI_YELLOW, 0, op, "You resist!");
                }
                else /* effect has hit! */
                    fear_player(op,hitter, 5);
            }
            break;
        case ATNR_SNARE:
            dam = 0.0;
            if(hitter->attack[attacknum] > (RANDOM()%100)) /* we hit with effect? */
            {
                if (hitter->type == PLAYER)
                    new_draw_info_format(NDI_ORANGE, 0, hitter, "You snared %s!", op->name);
                if (op->type == PLAYER)
                    new_draw_info_format(NDI_PURPLE, 0, op, "%s snares you!", hitter->name);

                /* give the target the chance to resist */
                if(op->resist[attacknum] > (RANDOM()%100)) /* resisted? */
                {
                    if (hitter->type == PLAYER)
                        new_draw_info_format(NDI_YELLOW, 0, hitter, "%s resists the snare!", op->name);
                    if (op->type == PLAYER)
                        new_draw_info(NDI_YELLOW, 0, op, "You resist!");
                }
                else /* effect has hit! */
                    snare_player(op,hitter, 5);
            }
            break;

        case ATNR_PARALYZE:
            dam = 0.0;
            if(hitter->attack[attacknum] > (RANDOM()%100)) /* we hit with effect? */
            {
                if (hitter->type == PLAYER)
                    new_draw_info_format(NDI_ORANGE, 0, hitter, "You paralyzed %s!", op->name);
                if (op->type == PLAYER)
                    new_draw_info_format(NDI_PURPLE, 0, op, "%s paralyzes you!", hitter->name);

                /* give the target the chance to resist */
                if(op->resist[attacknum] > (RANDOM()%100)) /* resisted? */
                {
                    if (hitter->type == PLAYER)
                        new_draw_info_format(NDI_YELLOW, 0, hitter, "%s resists the paralyzation effect!", op->name);
                    if (op->type == PLAYER)
                        new_draw_info(NDI_YELLOW, 0, op, "You resist!");

                    if(!(*flags&HIT_FLAG_PARALYZED_ADD) && QUERY_FLAG(op,FLAG_PARALYZED)) /* well, shit happens */
                        remove_paralyze(op);
                }
                else /* effect has hit! */
                {
                    /* to avoid stacking paralyze effects, we can use here the flags settings */
                    paralyze_player(op,hitter, 5);
                    *flags |=HIT_FLAG_PARALYZED_ADD; /* so we can track stacked hits */

                }
            }
            break;
        case ATNR_COUNTERMAGIC:
            dam = 0.0;
            if(hitter->attack[attacknum] > (RANDOM()%100)) /* we hit with effect? */
            {
                if (hitter->type == PLAYER)
                    new_draw_info_format(NDI_ORANGE, 0, hitter, "You use countermagic on %s!", op->name);
                if (op->type == PLAYER)
                    new_draw_info_format(NDI_PURPLE, 0, op, "%s uses countermagic on you!", hitter->name);

                /* give the target the chance to resist */
                if(1 && op->resist[attacknum] > (RANDOM()%100)) /* resisted? */
                {
                    if (hitter->type == PLAYER)
                        new_draw_info_format(NDI_YELLOW, 0, hitter, "%s resists the countermagic!", op->name);
                    if (op->type == PLAYER)
                        new_draw_info(NDI_YELLOW, 0, op, "You resist!");
                }
                else /* effect has hit! */
                {
                    /* not implemented ATM */
                }
            }
            break;
        case ATNR_CANCELLATION:
            dam = 0.0;
            if(hitter->attack[attacknum] > (RANDOM()%100)) /* we hit with effect? */
            {
                if (hitter->type == PLAYER)
                    new_draw_info_format(NDI_ORANGE, 0, hitter, "You cancellate %s!", op->name);
                if (op->type == PLAYER)
                    new_draw_info_format(NDI_PURPLE, 0, op, "%s cancellates you!", hitter->name);

                /* give the target the chance to resist */
                if(1 && op->resist[attacknum] > (RANDOM()%100)) /* resisted? */
                {
                    if (hitter->type == PLAYER)
                        new_draw_info_format(NDI_YELLOW, 0, hitter, "%s resist the cancellation!", op->name);
                    if (op->type == PLAYER)
                        new_draw_info(NDI_YELLOW, 0, op, "You resist!");
                }
                else /* effect has hit! */
                {
                    /* not implemented ATM */
                }
            }
            break;
        default:
            LOG(llevBug,"attack(): find unimplemented special attack: #%d obj:%s\n", attacknum, query_name(hitter));
          break;
    }

    return (int) dam;
}


/* we need this called spread in the function before because sometimes we want drop
 * a message BEFORE we tell the damage and sometimes we want a message after it.
 */
static void send_attack_msg(object *op, object *hitter, int attacknum, int dam, int damage)
{
    if (op->type == PLAYER)
    {
        new_draw_info_format(NDI_PURPLE, 0, op, "%s hits you for %d (%d) damage.", hitter->name, (int) dam,
                             ((int) dam) - damage);
    }
	/* i love C... ;) */
    if (hitter->type == PLAYER || ((hitter = get_owner(hitter)) && hitter->type == PLAYER))
    {
        new_draw_info_format(NDI_ORANGE, 0, hitter, "You hit %s for %d (%d) with %s.", op->name, (int) dam,
                             ((int) dam) - damage, attack_name[attacknum]);
    }
}


/* GROS: This code comes from damage_ob. It has been made external to
 * allow script procedures to "kill" objects in a combat-like fashion.
 * It was initially used by (kill-object) developed for the Collector's
 * Sword. Note that nothing has been changed from the original version
 * of the following code.
 */
/* ok, when i have finished the different attacks i must clean this up here too
 * looks like some artifact code in here - MT-2003
 */
int kill_object(object *op, int dam, object *hitter, int typeX)
{
    object     *corpse_owner, *owner, *old_hitter; /* this is used in case of servant monsters */
    int         maxdam              = 0;
    int         battleg             = 0;    /* true if op standing on battleground */
    mapstruct  *map;
    char        *buf_ptr, buf2[MAX_BUF];

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
                if(hitter->type == MONSTER && OBJECT_VALID(hitter->owner, hitter->owner_count))
                {
                    sprintf(buf, "Your %s killed %s.", query_name(hitter), query_name(op));
                    sprintf(buf2, "%s's %s killed %s.", query_name(owner), query_name(hitter), query_name(op));
                }
                else
                {
                    sprintf(buf, "You killed %s with %s.", query_name(op), query_name(hitter));
                    sprintf(buf2, "%s killed %s with %s.", query_name(owner), query_name(op), query_name(hitter));
                }

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

			/* here "remove" the killed object, drop the items inside and add
			 * quests and quest items/one drops to group of corpse_owner 
			 */
            destruct_ob(op); 
        }
   }

    return maxdam;
}


static int abort_attack(object *target, object *hitter, int env_attack)
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
          || !on_same_tileset(hitter, target))
        return 1;
    else
        new_mode = 0;
    return new_mode != env_attack;
}

/* hit_with_arrow() disassembles the missile, attacks the victim and
 * reassembles the missile.
 *
 * It returns a pointer to the reassembled missile, or NULL if the missile
 * isn't available anymore.
 */
/* A new logic for missiles in B4 and beyond.
 * Instead of disassembles a payload missile, attack and reassembles
 * we have now 2 classes of missiles: "native" missiles like arrow
 * objects or payload objects... for example a chair throwing around.
 * In that case we don't disassemble the item anymore now, because
 * the "missile" object is the THROW_OBJ itself and not their payload.
 * we just send the missile object to attack and all and if needed
 * we call stop_missile() which will handle all the stuff like disassemble
 * and putting arrows and map and all.
 */
object * hit_with_arrow(object *op, object *victim)
{
    object *hitter;
    int     hit_something   = 0;
    tag_t   hitter_tag;

	hitter = op;
    hitter_tag = hitter->count;

    if(!trigger_object_plugin_event(EVENT_ATTACK,
                hitter, hitter, victim, NULL, NULL, NULL, NULL, SCRIPT_FIX_ALL))
    {
        /*LOG(-1, "hit: %s (%d %d)\n", hitter->name, op->stats.dam, op->stats.wc);*/
        hit_something = attack_ob(victim, hitter, op);
    }

    /* hopefully the walk_off event was triggered somewhere there */
    if (was_destroyed(hitter, hitter_tag) || hitter->env != NULL)
        return NULL;

    /* Missile hit victim */
    if (hit_something)
    {
        trigger_object_plugin_event(EVENT_STOP,
                hitter, victim, NULL, NULL, NULL, NULL, NULL, SCRIPT_FIX_ALL);

		stop_missile(hitter);
        return NULL;
    }

    return op;
}

/* ATM no tear down wall in the game, function must be checked first MT -09.2005 */
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
                tmp->stats.dam = (int) (((dam + RANDOM() % ((int) dam + 1)) * LEVEL_DAMAGE(hitter->level)) * 0.9f);
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
                    create_food_buf_force(op, hitter, tmp); /* this insert the food force in player too */
                    new_draw_info(NDI_UNIQUE, 0, op, "You suddenly feel very ill.");
                }
                else /* and here we have hit with weapon or something */
                {
                    if (op->stats.Con > 1 && !(RANDOM() % 2))
                    {
                        tmp->stats.Con = (sint8) ((hitter->level / 2 + RANDOM() % (hitter->level * 8 + 1) / 10) * 0.1f + 2.0f);
                        if (tmp->stats.Con >= op->stats.Con)
                            tmp->stats.Con = op->stats.Con - 1;
                        tmp->stats.Con *= -1;
                    }

                    if (op->stats.Str > 1 && !(RANDOM() % 2))
                    {
                        tmp->stats.Str = (sint8) ((hitter->level / 2 + RANDOM() % (hitter->level * 8 + 1) / 10) * 0.1f + 2.0f);
                        if (tmp->stats.Str >= op->stats.Str)
                            tmp->stats.Str = op->stats.Str - 1;
                        tmp->stats.Str *= -1;
                    }

                    if (op->stats.Dex > 1 && !(RANDOM() % 2))
                    {
                        tmp->stats.Dex = (sint8) ((hitter->level / 2 + RANDOM() % (hitter->level * 8 + 1) / 10) * 0.1f + 2.0f);
                        if (tmp->stats.Dex >= op->stats.Dex)
                            tmp->stats.Dex = op->stats.Dex - 1;
                        tmp->stats.Dex *= -1;
                    }

                    if (op->stats.Int > 1 && !(RANDOM() % 2))
                    {
                        tmp->stats.Int = (sint8) ((hitter->level / 2 + RANDOM() % (hitter->level * 8 + 1) / 10) * 0.1f + 2.0f);
                        if (tmp->stats.Int >= op->stats.Int)
                            tmp->stats.Int = op->stats.Int - 1;
                        tmp->stats.Int *= -1;
                    }

                    if (op->stats.Cha > 1 && !(RANDOM() % 2))
                    {
                        tmp->stats.Cha = (sint8) ((hitter->level / 2 + RANDOM() % (hitter->level * 8 + 1) / 10) * 0.1f + 2.0f);
                        if (tmp->stats.Cha >= op->stats.Cha)
                            tmp->stats.Cha = op->stats.Cha - 1;
                        tmp->stats.Cha *= -1;
                    }

                    if (op->stats.Pow > 1 && !(RANDOM() % 2))
                    {
                        tmp->stats.Pow = (sint8) ((hitter->level / 2 + RANDOM() % (hitter->level * 8 + 1) / 10) * 0.1f + 2.0f);
                        if (tmp->stats.Pow >= op->stats.Pow)
                            tmp->stats.Pow = op->stats.Pow - 1;
                        tmp->stats.Pow *= -1;
                    }

                    if (op->stats.Wis > 1 && !(RANDOM() % 2))
                    {
                        tmp->stats.Wis = (sint8) ((hitter->level / 2 + RANDOM() % (hitter->level * 8 + 1) / 10) * 0.1f + 2.0f);
                        if (tmp->stats.Wis >= op->stats.Wis)
                            tmp->stats.Wis = op->stats.Wis - 1;
                        tmp->stats.Wis *= -1;
                    }

                    new_draw_info_format(NDI_UNIQUE, 0, op, "%s has poisoned you!", query_name(hitter));
                    insert_ob_in_ob(tmp, op);
                    SET_FLAG(tmp, FLAG_APPLIED);
                    FIX_PLAYER(op ,"attack - poison");
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
    static archetype  *at  = NULL;
    object     *tmp;
    int        max_slow = FALSE;

    if (!at)
    {
        at = find_archetype("slowness");
        if (!at)
        {
            LOG(llevBug, "BUG: Couldn't find archetype slowness.\n");
            return;
        }
    }

    if ((tmp = present_arch_in_ob(at, op)) == NULL)
    {

        tmp = arch_to_object(at);
        tmp = insert_ob_in_ob(tmp, op);
    }
    else
    {
        tmp->last_heal += dam; /* increase the "slowness" factor by dam% */
        if(tmp->last_heal>70)
        {
            max_slow = TRUE;
            tmp->last_heal = 70;
        }
    }

    tmp->stats.food += 8; /* slow effect last longer */
    if(tmp->stats.food >= 25)
        tmp->stats.food = 25;


    if(!max_slow)
    {
        if (op->type == PLAYER)
            new_draw_info(NDI_UNIQUE, 0, op, "The world suddenly moves faster!");
        if (!QUERY_FLAG(op, FLAG_CONFUSED) && op->map)
            new_info_map_except_format(NDI_UNIQUE|NDI_GREY, op->map, op->x, op->y, MAP_INFO_NORMAL, NULL, op, "%s suddenly moves slower!", STRING_SAFE(op->name));
        SET_FLAG(tmp, FLAG_APPLIED);
        FIX_PLAYER(op ," attack - slow"); /* will set FLAG_SLOWED */
    }

}

void fear_player(object *op, object *hitter, int dam)
{
    static archetype  *at  = NULL;
    object     *tmp;
    int        max_slow = FALSE;

    if (!at)
    {
        at = find_archetype("fear");
        if (!at)
        {
            LOG(llevBug, "BUG: Couldn't find archetype fear.\n");
            return;
        }
    }

    if ((tmp = present_arch_in_ob(at, op)) == NULL)
    {

        tmp = arch_to_object(at);
        tmp = insert_ob_in_ob(tmp, op);
    }

    tmp->stats.food += 8; /* slow effect last longer */
    if(tmp->stats.food >= 25)
        tmp->stats.food = 25;


    if(!max_slow)
    {
        if (op->type == PLAYER)
            new_draw_info(NDI_UNIQUE, 0, op, "You are scared to death!");
        if (!QUERY_FLAG(op, FLAG_CONFUSED) && op->map)
            new_info_map_except_format(NDI_UNIQUE|NDI_GREY, op->map, op->x, op->y, MAP_INFO_NORMAL, NULL, op, "%s suddenly looks scared!", STRING_SAFE(op->name));
        SET_FLAG(tmp, FLAG_APPLIED);
        FIX_PLAYER(op ,"attack fear"); /* will set FLAG_FEAR */
    }

}

void snare_player(object *op, object *hitter, int dam)
{
    static archetype  *at  = NULL;
    object     *tmp;
    int        max_slow = FALSE;

    if (!at)
    {
        at = find_archetype("snare");
        if (!at)
        {
            LOG(llevBug, "BUG: Couldn't find archetype snare.\n");
            return;
        }
    }

    if ((tmp = present_arch_in_ob(at, op)) == NULL)
    {

        tmp = arch_to_object(at);
        tmp = insert_ob_in_ob(tmp, op);
    }
    else
    {
        if(tmp->last_heal+dam>100)
        {
            if(tmp->last_heal<100)
                max_slow = TRUE;
            tmp->last_heal = 100;
        }
        else
            tmp->last_heal += dam; /* increase the "slowness" factor by dam% */
    }

    tmp->stats.food += 8; /* slow effect last longer */
    if(tmp->stats.food >= 25)
        tmp->stats.food = 25;


    if(!max_slow)
    {
        if (op->type == PLAYER)
        {
            if(tmp->last_heal < 100)
                new_draw_info(NDI_UNIQUE, 0, op, "You are snared!");
            else
                new_draw_info(NDI_UNIQUE, 0, op, "You are rooted!");
        }
        if (!QUERY_FLAG(op, FLAG_CONFUSED) && op->map)
        {
            if(tmp->last_heal < 100)
                new_info_map_except_format(NDI_UNIQUE|NDI_GREY, op->map, op->x, op->y, MAP_INFO_NORMAL, NULL, op, "%s suddenly walks slower!", STRING_SAFE(op->name));
            else
                new_info_map_except_format(NDI_UNIQUE|NDI_GREY, op->map, op->x, op->y, MAP_INFO_NORMAL, NULL, op, "%s suddenly stops walking!", STRING_SAFE(op->name));
        }
        SET_FLAG(tmp, FLAG_APPLIED);
        FIX_PLAYER(op ,"attack snear "); /* will set FLAG_SNEAR */
    }

}

void confuse_player(object *op, object *hitter, int ticks)
{
    static archetype  *at  = NULL;
    object     *tmp;

    if (!at)
    {
        at = find_archetype("confusion");
        if (!at)
        {
            LOG(llevBug, "BUG: Couldn't find archetype confusion.\n");
            return;
        }
    }

    if ((tmp = present_arch_in_ob(at, op)) == NULL)
    {

        tmp = arch_to_object(at);
        tmp = insert_ob_in_ob(tmp, op);
    }

    tmp->stats.food += 8;
    if(tmp->stats.food >= 25)
        tmp->stats.food = 25;

    if (op->type == PLAYER)
        new_draw_info(NDI_UNIQUE, 0, op, "You suddenly feel very confused!");
    if (!QUERY_FLAG(op, FLAG_CONFUSED) && op->map)
        new_info_map_except_format(NDI_UNIQUE|NDI_GREY, op->map, op->x, op->y, MAP_INFO_NORMAL, NULL, op, "%s looks confused!", STRING_SAFE(op->name));

    SET_FLAG(tmp, FLAG_APPLIED);
    SET_FLAG(op, FLAG_CONFUSED);
}

/* remove confusion effect and force (if there is one)
* Note: This is for explicit remove - in time.c the force can
* auto destruct itself without calling this function. This.
*/
void remove_confusion(object *op)
{
    static archetype  *at  = NULL;
    object     *tmp;

    CLEAR_FLAG(op,FLAG_BLIND);

    if (!at)
    {
        at = find_archetype("confusion");
        if (!at)
        {
            LOG(llevBug, "BUG: Couldn't find archetype confusion.\n");
            return;
        }
    }

    if ((tmp = present_arch_in_ob(at, op)) == NULL)
        return;

    remove_ob(tmp);

    if (op->type == PLAYER)
        new_draw_info(NDI_UNIQUE, 0, op, "You regain your senses.");
    if (op->map)
        new_info_map_except_format(NDI_UNIQUE|NDI_GREY, op->map, op->x, op->y, MAP_INFO_NORMAL, NULL, op, "%s regains his senses!", STRING_SAFE(op->name));

    FIX_PLAYER(op ,"attack - remove confusion");
}

void blind_player(object *op, object *hitter, int dam)
{
    static archetype  *at  = NULL;
    object     *tmp;

    if (!at)
    {
        at = find_archetype("blindness");
        if (!at)
        {
            LOG(llevBug, "BUG: Couldn't find archetype blindness.\n");
            return;
        }
    }

    if ((tmp = present_arch_in_ob(at, op)) == NULL)
    {

        tmp = arch_to_object(at);
        tmp = insert_ob_in_ob(tmp, op);
    }

    tmp->stats.food += 8;
    if(tmp->stats.food >= 25)
        tmp->stats.food = 25;

    if (op->type == PLAYER)
        new_draw_info(NDI_UNIQUE, 0, op, "You suddenly are blinded!");
    if (!QUERY_FLAG(op, FLAG_CONFUSED) && op->map)
        new_info_map_except_format(NDI_UNIQUE|NDI_GREY, op->map, op->x, op->y, MAP_INFO_NORMAL, NULL, op, "%s looks blinded!", STRING_SAFE(op->name));

    SET_FLAG(tmp, FLAG_APPLIED);
    SET_FLAG(op, FLAG_BLIND);
}

/* remove blindness effect and force (if there is one)
* Note: This is for explicit remove - in time.c the force can
* auto destruct itself without calling this function. This.
*/
void remove_blindness(object *op)
{
    static archetype  *at  = NULL;
    object     *tmp;

    CLEAR_FLAG(op,FLAG_BLIND);

    if (!at)
    {
        at = find_archetype("blindness");
        if (!at)
        {
            LOG(llevBug, "BUG: Couldn't find archetype blindness.\n");
            return;
        }
    }

    if ((tmp = present_arch_in_ob(at, op)) == NULL)
        return;

    remove_ob(tmp);

    if (op->type == PLAYER)
        new_draw_info(NDI_UNIQUE, 0, op, "You can see again.");
    if (op->map)
        new_info_map_except_format(NDI_UNIQUE|NDI_GREY, op->map, op->x, op->y, MAP_INFO_NORMAL, NULL, op, "%s can see again!", STRING_SAFE(op->name));

    FIX_PLAYER(op ,"attack - remove blind");
}


void paralyze_player(object *op, object *hitter, int dam)
{
    static archetype  *at  = NULL;
    object     *tmp;

    if (!at)
    {
        at = find_archetype("paralyze");
        if (!at)
        {
            LOG(llevBug, "BUG: Couldn't find archetype paralyze.\n");
            return;
        }
    }

    if ((tmp = present_arch_in_ob(at, op)) == NULL)
    {

        tmp = arch_to_object(at);
        tmp = insert_ob_in_ob(tmp, op);
    }
    else
    {
        /* paralyze effects don't stack - means don't increase when hit. */
        return;
    }

    if (op->type == PLAYER)
        new_draw_info(NDI_UNIQUE, 0, op, "You suddenly feel paralyzed!");
    if (!QUERY_FLAG(op, FLAG_CONFUSED) && op->map)
        new_info_map_except_format(NDI_UNIQUE|NDI_GREY, op->map, op->x, op->y, MAP_INFO_NORMAL, NULL, op, "%s looks paralyzed!", STRING_SAFE(op->name));

    SET_FLAG(tmp, FLAG_APPLIED);
    FIX_PLAYER(op ,"attach paralyze"); /* will set FLAG_PARALYZE */
}

/* remove paralyze effect and force (if there is one)
 * Note: This is for explicit remove - in time.c the force can
 * auto destruct itself without calling this function. This.
*/
void remove_paralyze(object *op)
{
    static archetype  *at  = NULL;
    object     *tmp;

    CLEAR_FLAG(op,FLAG_PARALYZED);

    if (!at)
    {
        at = find_archetype("paralyze");
        if (!at)
        {
            LOG(llevBug, "BUG: Couldn't find archetype paralyze.\n");
            return;
        }
    }

    if ((tmp = present_arch_in_ob(at, op)) == NULL)
        return;

    remove_ob(tmp);

    if (op->type == PLAYER)
        new_draw_info(NDI_UNIQUE, 0, op, "You can move again.");
    if (op->map)
        new_info_map_except_format(NDI_UNIQUE|NDI_GREY, op->map, op->x, op->y, MAP_INFO_NORMAL, NULL, op, "%s can move again!", STRING_SAFE(op->name));

    FIX_PLAYER(op ,"attack - remove paralyze");
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
