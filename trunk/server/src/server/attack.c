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

    The author can be reached via e-mail to info@daimonin.org
*/
#include <global.h>

/* flags for HitPlayerAttacktype() */
/* important: the first 5 bits are the same is in material_base_xx in material.h */
#define HIT_FLAG_PARALYZED_ADD 32
#define HIT_FLAG_PARALYZED_REM 64
#define HIT_FLAG_DMG           128
#define HIT_FLAG_DMG_ACID      256
#define HIT_FLAG_DMG_WMAGIC    512
/* HIT_FLAG_WEAPON      ==    1024 is defined in global.h */

#define ATTACK_HIT_DAMAGE(_op, _anum)       dam=dam*((double)_op->attack[_anum]*(double)0.01);dam>=1.0f?(damage=(int)dam):(damage=1)
#define ATTACK_RESIST_DAMAGE(_op, _anum)    dam=dam*((double)(100-_op->resist[_anum])*(double)0.01)

/* resist use the same names as attacks - they map 1:1 to it */
/* TODO: Re the kill_object() detail (4th column): not all attacks can kill
 * anyway and these needn't be final; I just quicklt worked down the list
 * until I got bored.
 *
 * -- Smacky 20150209 */
attack_name_t attack_name[NROFATTACKS] =
{
    { "IM", "impact", NULL, "beaten to death by" },
    { "SL", "slash", NULL, "sliced 'n' diced by" },
    { "CL", "cleave", NULL, "hacked to pieces by" },
    { "PI", "pierce", NULL, "poked fatally by" },
    { "FI", "fire", NULL, "burned to a crisp by" },
    { "CO", "cold", NULL, "frozen by" },
    { "EL", "electricity", NULL, "electrocuted by" },
    { "PO", "poison", NULL, "poisoned by" },
    { "AC", "acid", NULL, "dissolved by" },
    { "SO", "sonic", NULL, "sonically shattered by" },
    { "CH", "channelling", NULL, "mentally overwhelmed by" },
    { "CR", "corruption", NULL, "spiritually corrupted by" },
    { "PS", "psionic", NULL, "psionically defeated by" },
    { "LI", "light", NULL, NULL },
    { "SH", "shadow", NULL, NULL },
    { "LS", "lifesteal", NULL, "withered by" },
    { "AE", "aether", NULL, NULL },
    { "NE", "nether", NULL, NULL },
    { "CH", "chaos", NULL, NULL },
    { "DE", "death", NULL, NULL },
    { "WM", "weaponmagic", NULL, NULL },
    { "GO", "godpower", NULL, NULL },
    { "DR", "drain", NULL, NULL },
    { "DP", "depletion", NULL, NULL },
    { "CM", "countermagic", NULL, NULL },
    { "CA", "cancellation", NULL, NULL },
    { "CF", "confusion", NULL, NULL },
    { "FE", "fear", NULL, NULL },
    { "SW", "slow", NULL, NULL },
    { "PA", "paralyze", NULL, NULL },
    { "SN", "snare", NULL, NULL },
    { "??", "internal", NULL, NULL },
};

/* If you want to weight things so certain resistances show up more often than
 * others, just add more entries in the table for the protections you want to
 * show up. */
attack_nr_t resist_table[] =
{
    ATNR_SLASH, ATNR_CLEAVE, ATNR_PIERCE, ATNR_IMPACT, ATNR_CHANNELLING, ATNR_FIRE, ATNR_ELECTRICITY, ATNR_COLD,
    ATNR_CONFUSION, ATNR_ACID, ATNR_DRAIN, ATNR_SHADOW, ATNR_POISON, ATNR_SLOW, ATNR_PARALYZE, ATNR_LIGHT, ATNR_FEAR,
    ATNR_SLASH, ATNR_DEPLETION, ATNR_CLEAVE, ATNR_SONIC, ATNR_IMPACT, ATNR_SNARE, ATNR_LIFESTEAL, ATNR_PSIONIC,
    ATNR_NETHER, ATNR_PIERCE, ATNR_SLASH, ATNR_CLEAVE, ATNR_PIERCE, ATNR_IMPACT, ATNR_CHANNELLING, ATNR_FIRE,
    ATNR_ELECTRICITY, ATNR_COLD, ATNR_CONFUSION, ATNR_ACID, ATNR_DRAIN, ATNR_LIGHT, ATNR_POISON, ATNR_SLOW,
    ATNR_PARALYZE, ATNR_SNARE, ATNR_FEAR, ATNR_CANCELLATION, ATNR_DEPLETION, ATNR_COUNTERMAGIC, ATNR_SONIC, ATNR_CORRUPTION,
    ATNR_SNARE, ATNR_LIFESTEAL, ATNR_PSIONIC, ATNR_NETHER, ATNR_AETHER, ATNR_DEATH, ATNR_CHAOS, ATNR_GODPOWER,
    ATNR_WEAPONMAGIC
};

/* some static defines */
static int GetAttackMode(object_t **target, object_t **hitter, attack_envmode_t *env_attack);
static attack_envmode_t  AbortAttack(object_t *target, object_t *hitter, attack_envmode_t env_attack);
static void SendAttackMsg(object_t *op, object_t *hitter, int attacknum, int dam,
                          int damage);
static int  HitPlayerAttacktype(object_t *op, object_t *hitter, int *flags, int damage, uint32 attacknum, int magic);

/* check and adjust all pre-attack issues like checking attack is valid,
 * attack objects are in the right state and such.
 */
static int GetAttackMode(object_t **target, object_t **hitter, attack_envmode_t *env_attack)
{
    if (OBJECT_FREE(*target) || OBJECT_FREE(*hitter))
    {
        LOG(llevBug, "BUG:: %s/GetAttackMode(): freed object\n", __FILE__);
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

    if (QUERY_FLAG(*target, FLAG_REMOVED) ||
        QUERY_FLAG(*hitter, FLAG_REMOVED) ||
        !(*hitter)->map ||
        !on_same_tileset((*hitter)->map, (*target)->map))
    {
        LOG(llevBug, "BUG:: %s/GetAttackMode(): hitter (%s[%d]) with no relation to target (%s[%d])!\n",
            __FILE__, STRING_OBJ_NAME(*hitter), TAG(*hitter),
            STRING_OBJ_NAME(*target), TAG(*target));
        return 1;
    }
    *env_attack = ENV_ATTACK_NO;
    return 0;
}

/* compare the attacker and target and adjust the attack roll.
 * Like the attacker is invisible - adjust the roll depending on the
 * fact the target can see invisible or not - just as a example.
 */
static inline int adj_attackroll(object_t *hitter, object_t *target, int adjust)
{
    object_t *attacker    = hitter;

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
    if (IS_NORMAL_INVIS_TO(target, attacker) ||
        QUERY_FLAG(attacker, FLAG_BLIND))
    {
        adjust -= adjust / 25; /* - 25% */
    }

    if (QUERY_FLAG(target, FLAG_SCARED))
        adjust += adjust/10; /* + 10% */

    /* if we attack at a different 'altitude' its harder - remove 10% */
    if (IS_AIRBORNE(attacker) != IS_AIRBORNE(target))
        adjust -= adjust/10;

    return adjust;
}

//#define ATTACK_DEBUG
/* here we decide a attack will happen and how. blocking, parry, missing is handled
 * here inclusive the sounds. All whats needed before we count damage and say "hit you".
 */
int attack_ob(object_t *target, object_t *hitter, object_t *hit_obj)
{
    attack_envmode_t env_attack;
    int     hitdam, roll;
    tag_t   op_tag, hitter_tag;

    /* GetAttackMode will pre-check and adjust *ANY* topic.
     * Including setting ->head objects and checking maps
     */
    if (GetAttackMode(&target, &hitter, &env_attack))
        goto error;

    if(trigger_object_plugin_event(EVENT_ATTACK, target, hitter, hitter, NULL, NULL, NULL, NULL, SCRIPT_FIX_ALL))
        goto error;

    if(!hit_obj)
        hit_obj = hitter;

    op_tag = target->count;
    hitter_tag = hitter->count;
    hitdam  = hit_obj->stats.dam;

    /* Missile hits on mobs who are looking the other way are stealth hits and
     * do 1.5x damage. */
    /* TODO: To prevent exploits (A lures the mob so B can follow behind
     * scoring multiple stealth hits) we should only allow a stealth hit when
     * the mob is not alert. */
    /* Done, but the 'new' AI system of ai knowledge and the cf-era ob->enemy
     * are not especially well integrated yet so it may have some problems. */
    if (target->type == MONSTER &&
        !OBJECT_VALID(target->enemy, target->enemy_count) &&
        is_aimed_missile(hit_obj) &&
        MOB_DATA(target) != NULL &&
        !mob_can_see_obj(target, hitter, MOB_DATA(target)->known_mobs))
    {
        ndi(NDI_ORANGE, 0, hitter, "Stealth attack direct hit! (+50%% damage)");
        hitdam = (int)((double)hitdam * 1.5);

        goto force_direct_hit;
    }

    /* Fight Step 1: Get the random hit value */
    roll = RANDOM_ROLL(0, 100);

    /* Fight Step 2: Adjust for special cases. Target is invinsible we can't see it? Is levitating? ... */
    if (env_attack == ENV_ATTACK_NO)
        roll = adj_attackroll(hitter, target, roll);

#ifdef ATTACK_DEBUG
    if (hitter->type == PLAYER)
        ndi(NDI_RED, 0, hitter, "You roll: %d thac m:%d 0:%d (wc/roll: %d(%d) ac:%d)!", roll,hitter->stats.thac0,hitter->stats.thacm, hit_obj->stats.wc,hit_obj->stats.wc+roll,target->stats.ac);
    if (target->type == PLAYER)
        ndi(NDI_RED, 0, target, "Hitter roll: %d thac m:%d 0:%d (wc/roll: %d(%d) ac:%d)!", roll,hitter->stats.thac0,hitter->stats.thacm, hit_obj->stats.wc,hit_obj->stats.wc+roll,target->stats.ac);
#endif

    if (hitter->type == PLAYER)
        CONTR(hitter)->anim_flags |= PLAYER_AFLAG_ENEMY; /* so we do one swing */

    /* Force player to face enemy */
    if (hitter->type == PLAYER)
    {
        rv_t   dir;
        if(RV_GET_OBJ_TO_OBJ(hitter, target, &dir, RV_NO_DISTANCE))
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
                ndi(NDI_PURPLE, 0, target, "%s fumbles!", hitter->name);
            if (hitter->type == PLAYER)
                ndi(NDI_ORANGE, 0, hitter, "You fumble!");

            if (hitter->type == PLAYER)
                play_sound_map(MSP_KNOWN(hitter), SOUND_MISS_PLAYER, SOUND_NORMAL);
            else
                play_sound_map(MSP_KNOWN(hitter), SOUND_MISS_MOB, SOUND_NORMAL);
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
                ndi(NDI_PURPLE, 0, target, "%s Direct Hit! (+20%% damage)", hitter->name);
            if (hitter->type == PLAYER)
                ndi(NDI_ORANGE, 0, hitter, "Direct Hit! (+20%% damage)");
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
            play_sound_map(MSP_KNOWN(hitter), SOUND_ARROW_HIT, SOUND_NORMAL);
        else
        {
            if (hitter->attack[ATNR_SLASH])
                play_sound_map(MSP_KNOWN(hitter), SOUND_HIT_SLASH, SOUND_NORMAL);
            else if (hitter->attack[ATNR_CLEAVE])
                play_sound_map(MSP_KNOWN(hitter), SOUND_HIT_CLEAVE, SOUND_NORMAL);
            else if (hitter->attack[ATNR_IMPACT])
                play_sound_map(MSP_KNOWN(hitter), SOUND_HIT_IMPACT, SOUND_NORMAL);
            else
                play_sound_map(MSP_KNOWN(hitter), SOUND_HIT_PIERCE, SOUND_NORMAL);
        }

        /* we hook thrown potions and stuff in here? we should perhaps move this */
        if (env_attack == ENV_ATTACK_NO && hitter->type == POTION)
        {
            if (hitter->stats.sp != SP_NO_SPELL && spells[hitter->stats.sp].flags & SPELL_DESC_DIRECTION)
                cast_spell(hitter, hitter, hitter->direction, hitter->stats.sp, 1, spellPotion, NULL); /* apply potion ALWAYS fire on the spot the applier stands - good for healing - bad for firestorm */
            decrease_ob_nr(hitter, 1);

            if (!OBJECT_VALID(hitter, hitter_tag) ||
                !OBJECT_VALID(target, op_tag) ||
                AbortAttack(target, hitter, env_attack))
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
            damage_ob(hitter, RANDOM_ROLL(0, target->stats.dam), target, env_attack);

            if (!OBJECT_VALID(target, op_tag) ||
                !OBJECT_VALID(hitter, hitter_tag) ||
                AbortAttack(target, hitter, env_attack))
                goto leave;
        }

        /* the damage is between 70 and 100% of the (adjusted) base damage */
        hitdam = damage_ob(target, RANDOM_ROLL((int)(hitdam*0.7f)+1, hitdam), hit_obj, env_attack);
        if (!OBJECT_VALID(target, op_tag) ||
            !OBJECT_VALID(hitter, hitter_tag) ||
            AbortAttack(target, hitter, env_attack))
            goto leave;
    }
    else /* we missed, dam=0 */
    {
        hitdam = 0;
        if (hitter->type != ARROW)
        {
            if (target->type == PLAYER)
                ndi(NDI_PURPLE, 0, target, "%s misses you!", hitter->name);
            if (hitter->type == PLAYER)
                ndi(NDI_ORANGE, 0, hitter, "you miss %s!", target->name);

            if (hitter->type == PLAYER)
                play_sound_map(MSP_KNOWN(hitter), SOUND_MISS_PLAYER, SOUND_NORMAL);
            else
                play_sound_map(MSP_KNOWN(hitter), SOUND_MISS_MOB, SOUND_NORMAL);
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
int damage_ob(object_t *op, int dam, object_t *hitter, attack_envmode_t env_attack)
{
    object_t *hit_obj, *target_obj, *aggro_obj, *dmg_obj=NULL;
    int     maxdam      = 0, flags = 0;
    int     attacknum, hit_level;
    tag_t   op_tag, hitter_tag;
    int     rtn_kill    = 0;

    /* if our target is invulnerable or is an SA, we can't hurt him. */
    if (QUERY_FLAG(op, FLAG_INVULNERABLE) ||
        (GET_GMASTER_MODE(op) & GMASTER_MODE_SA))
    {
        return 0;
    }

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
     * but for hitter, we always use the TYPE_SKILL LEVEL if player!
     */
    if (!(hit_obj = get_owner(hitter)))
        hit_obj = hitter;
    if (!(target_obj = get_owner(op)))
        target_obj = op;

    if (hit_obj->type == PLAYER)
        hit_level = SK_level(hit_obj); /* get from hitter object the right skill level! */
    else
        hit_level = hitter->level;

    if (hit_level == 0 ||
        target_obj->level == 0) /* very useful sanity check! */
    {
        object_t *hown = get_owner(hitter),
               *town = get_owner(target_obj);

        LOG(llevDebug, "DEBUG:: %s/damage_ob(): hit or target object level == 0 (h:%s[%d] o:%s[%d] l->%d, t:%s[%d] o:%s[%d] l->%d)!\n",
            __FILE__, STRING_OBJ_NAME(hitter), TAG(hitter),
            STRING_OBJ_NAME(hown), TAG(hown), hit_level,
            STRING_OBJ_NAME(target_obj), TAG(target_obj),
            STRING_OBJ_NAME(town), TAG(town), target_obj->level);
    }

    /* New (but still very basic) code for avoiding mobs (and players) on
     * the same side to damage each other. */
    if(get_friendship(hit_obj, target_obj) >= FRIENDSHIP_HELP)
    {
        LOG(llevDebug, "DEBUG:: %s/damage_ob(): friendly hit (%s[%d] => %s[%d] / %s[%d] => %s[%d]) ignored\n",
            __FILE__, STRING_OBJ_NAME(hitter), TAG(hitter),
            STRING_OBJ_NAME(op), TAG(op), STRING_OBJ_NAME(hit_obj),
            TAG(hit_obj), STRING_OBJ_NAME(target_obj), TAG(target_obj));
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
    if (hit_obj->type == PLAYER &&
        hit_obj->map &&
        target_obj->type == PLAYER &&
        target_obj->map)
    {
        if (!(MSP_KNOWN(hit_obj)->flags & MSP_FLAG_PVP) ||
            !(MSP_KNOWN(target_obj)->flags & MSP_FLAG_PVP))
        {
            return 0;
        }
    }

    /* this checks objects are valid, on same map and set them to head when needed! */
    /* also, simple_attack is set to 1, when one of the parts hav ->env != null
     * atm, this value is only used in the door attack */
    if(env_attack == ENV_ATTACK_CHECK)
    {
        if (GetAttackMode(&op, &hitter, &env_attack))
            return 0;
    }

    op_tag = op->count;
    hitter_tag = hitter->count;

    /* A last safery check - this happened in the past sometimes */
    if (op->stats.hp <= 0)
    {
        LOG(llevDebug, "DEBUG:: %s/damage_ob(): victim (%s[%d])) already dead!\n",
            __FILE__, STRING_OBJ_NAME(op), TAG(op));
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
    * HitPlayerAttacktype only figures out the damage, doesn't inflict
    * it.  It will do the appropriate action for attacktypes with
    * effects (slow, paralization, etc.
    */
    for (attacknum = 0; attacknum < NROFATTACKS; attacknum++)
    {
        if (hitter->attack[attacknum])
        {
            /*          LOG(llevNoLog, "hitter: %f - %s (dam:%d/%d) (wc:%d/%d)(ac:%d/%d) ap:%d\n",hitter->speed,
                            hitter->name,hitter->stats.dam,op->stats.dam, hitter->stats.wc,op->stats.wc,
                            hitter->stats.ac,op->stats.ac,hitter->attack[attacknum]);
            */
            maxdam += HitPlayerAttacktype(op, hitter, &flags, dam, attacknum, 0);
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
    aggro_obj = aggro_update_info(op, hitter, hit_obj, maxdam<op->stats.hp?maxdam:op->stats.hp);
    CLEAR_FLAG(hitter, FLAG_NO_FIX_PLAYER);
    CLEAR_FLAG(op, FLAG_NO_FIX_PLAYER);

    /* this is needed to send the hit number animations to the clients */
    if (op->damage_round_tag != ROUND_TAG)
    {
        op->last_damage = 0;
        op->damage_round_tag = ROUND_TAG;
    }
    op->last_damage += maxdam;

    /*LOG(llevNoLog,"DMG: %d\n", maxdam);*/
    op->stats.hp -= maxdam; /* thats the damage the target got */

    /* lets kill, kill, kill... */
    if (op->stats.hp <= 0)
    {
        /* TODO: This is just a quick and ugly hack to make kill messages
         * semi-random for entertainment. I'll tidy and rewrite this whole
         * function one day so the message reflects the attack that actually
         * caused death.
         *
         * -- Smacky 20150209 */
        const char *headline = NULL,
                   *detail = NULL;

        if (hitter->type == ARROW)
        {
            if (!RANDOM_ROLL(0, 9))
            {
                detail = "struck down by";
            }
        }
        else
        {
            sint8  i;

            for (i = 0; i < NROFATTACKS; i++)
            {
                uint8 v = hitter->attack[i];

                if (v &&
                    !RANDOM_ROLL(0, 9))
                {
                    headline = attack_name[i].kill_headline;
                    detail = attack_name[i].kill_detail;
                    break;
                }
            }
        }

        if (!kill_object(op, hitter, headline, detail))
        {
            /* Show Damage System for clients
             * whatever is dead now, we check map. If it on map, we redirect last_damage
             * to map space, giving player the chance to see the last hit damage they had
             * done. If there is more as one object killed on a single map tile, we overwrite
             * it now. This visual effect works pretty good. MT */
            /* no pet/player/monster checking now, perhaps not needed */
            if (op->map)
            {
                msp_t *msp = MSP_KNOWN(op);

                if (op->damage_round_tag == ROUND_TAG)
                {
                    msp->last_damage = op->last_damage;
                    msp->round_tag = ROUND_TAG;
                }

                play_sound_map(msp, SOUND_PLAYER_KILLS, SOUND_NORMAL);
            }

            return maxdam;
        }
    }
    /* Eneq(@csd.uu.se): Check to see if monster runs away. */
    /* TODO: gecko: this should go into a behaviour... */
    else if (op->type == MONSTER &&
             op->stats.hp < (signed short)(((float)op->run_away / 100.0) * (float)op->stats.maxhp))
    {
        SET_FLAG(op, FLAG_RUN_AWAY);
    }

    /* Used to be ghosthit removal - we now use the ONE_HIT flag.  Note
     * that before if the player was immune to ghosthit, the monster
     * remained - that is no longer the case.
     */
    if (QUERY_FLAG(hitter, FLAG_ONE_HIT))
    {
        remove_ob(hitter); /* Remove, but don't drop inventory */
        move_check_off(hitter, NULL, MOVE_FLAG_VANISHED);
    }
    /* Lets handle creatures that are splitting now */
    else if (!OBJECT_FREE(op) && QUERY_FLAG(op, FLAG_SPLITTING))
    {
        /* TODO: this might need some updating for the new AI system
         * gecko 2006-05-01 */
        int     i;
        int     friendly        = QUERY_FLAG(op, FLAG_FRIENDLY);
        int     unaggressive    = QUERY_FLAG(op, FLAG_UNAGGRESSIVE);
        object_t *owner           = get_owner(op);

        if (!op->other_arch)
        {
            LOG(llevBug, "BUG:: %s/damage_ob(): %s[%d] attempted split without other_arch!\n",
                __FILE__, STRING_OBJ_NAME(op), TAG(op));
            return maxdam;
        }

        remove_ob(op);
        if (move_check_off(op, NULL, MOVE_FLAG_VANISHED) == MOVE_RETURN_SUCCESS)
        {
            msp_t *msp = MSP_KNOWN(op);

            for (i = 0; i < NROFNEWOBJS(op); i++)
            {
                /* This doesn't handle op->more yet */
                object_t *tmp = arch_to_object(op->other_arch);
                sint8   j;

                tmp->stats.hp = op->stats.hp;
                if (friendly)
                {
                    SET_FLAG(tmp, FLAG_FRIENDLY);
                    if (owner != NULL)
                        set_owner(tmp, owner);
                }
                if (unaggressive)
                    SET_FLAG(tmp, FLAG_UNAGGRESSIVE);
                j = overlay_find_free(msp, tmp, 1, OVERLAY_7X7, 0);
                if (j != -1)
                {
                    /* Found spot to put this monster */
                    tmp->x = op->x + OVERLAY_X(j);
                    tmp->y = op->y + OVERLAY_Y(j);
                    (void)insert_ob_in_map(tmp, op->map, NULL, 0);
                }
            }
        }
    }
    return maxdam;
}

/* hit_map() causes hitter to damage every damageable object on msp.
 *
 * I'm not entirely sure how useful it really is to have msp as a separate
 * parameter as you probably only want to damage those objects where hitter
 * actually is. It does mean that hitter needn't be there (or even on a map at
 * all), so I guess we have a bit more flexibility in when we call this
 * function, if that's helpful.
 *
 * Before ddamaging an object any ATTACK script on that object is run and if it
 * returns true it escapes damage. */
sint32 hit_map(object_t *hitter, msp_t *msp)
{
    tag_t     hitter_tag;
    object_t *owner,
             *this,
             *next;
    sint32    r = 0;

    if (!msp->last)
    {
        return 0; // nothing here
    }

    hitter_tag = hitter->count;

    if (!(owner = get_owner(hitter)))
    {
        owner = hitter;
    }

    FOREACH_OBJECT_IN_MSP(this, msp, next)
    {
        tag_t this_tag = this->count;

        if (this == hitter ||                                 // don't hurt self
            get_friendship(owner, this) >= FRIENDSHIP_HELP || // friends don't hurt friends
            !IS_LIVE(this))                                   // can only hit live objects (for now)
        {
            continue;
        }

        if (!trigger_object_plugin_event(EVENT_ATTACK, this, hitter, hitter, NULL, NULL, NULL, NULL, SCRIPT_FIX_ALL))
        {
            /* If hitter is no longer valid, that's an end to the function. */
            if (!OBJECT_VALID(hitter, hitter_tag))
            {
                break;
            }

            /* As long as this is still valid, damage it. */
            if (OBJECT_VALID(this, this_tag))
            {
                damage_ob(this, hitter->stats.dam, hitter, ENV_ATTACK_CHECK);
                r++;
            }

            /* If hitter is no longer valid, that's an end to the function. */
            if (!OBJECT_VALID(hitter, hitter_tag))
            {
                break;
            }
        }
    }

    return r;
}

/* we need this called spread in the function before because sometimes we want drop
* a message BEFORE we tell the damage and sometimes we want a message after it.
*/
static inline void send_resist_msg(object_t *op, object_t *hitter, int attacknum)
{
    if (op->type == PLAYER)
    {
        ndi(NDI_GREY, 0, op, "You resist the %s attack!", attack_name[attacknum].name);
    }
    /* i love C... ;) */
    if (hitter->type == PLAYER || ((hitter = get_owner(hitter)) && hitter->type == PLAYER))
    {
        ndi(NDI_GREY, 0, hitter, "%s resists the %s attack!", op->name, attack_name[attacknum].name);
    }
}

/* This returns the amount of damage hitter does to op with the
 * appropriate attacktype.  Only 1 attacktype should be set at a time.
 * This doesn't damage the player, but returns how much it should
 * take.  However, it will do other effects (paralyzation, slow, etc.)
 * Note - changed for PR code - we now pass the attack number and not
 * the attacktype.  Makes it easier for the PR code.  */
static int HitPlayerAttacktype(object_t *op, object_t *hitter, int *flags, int damage, uint32 attacknum, int magic)
{
    double dam = (double) damage;
    int    doesnt_slay = 1;
    uint8  chance;

    /* just a sanity check */
    if (dam < 0)
    {
        LOG(llevBug, "BUG:: %s/HitPlayerAttacktype(): called with negative damage: %d from object %s[%d]!\n",
            __FILE__, (int)dam, STRING_OBJ_NAME(op), TAG(op));
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
        if (hitter->type == FORCE &&
            hitter->sub_type1 == ST1_FORCE_POISON) /* we have a poison force object (thats the poison we had inserted) */
        {
            attacknum = ATNR_POISON; /* map to poison... */
            if (op->resist[attacknum] == 100)
            {
                dam = 0;
                SendAttackMsg(op, hitter, attacknum, (int) dam, damage);
                return (int) dam;
            }

            if (op->resist[attacknum])
                ATTACK_RESIST_DAMAGE(op, attacknum); /* reduce to % resistance */
        }

        if (damage && dam < 1.0)
            dam = 1.0;

        SendAttackMsg(op, hitter, attacknum, (int) dam, damage);
        return (int) dam;
    }

    /* quick check for immunity - if so, we skip here.
        * our formula is (100-resist)/100 - so test for 100 = zero division
     */
    if (op->resist[attacknum] == 100)
    {
        switch (attacknum)
        {
            case ATNR_IMPACT:
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

            case ATNR_CHANNELLING:
            case ATNR_CORRUPTION:
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

    chance = MAX(0, hitter->attack[attacknum] - op->resist[attacknum]);

    switch (attacknum)
    {
        case ATNR_IMPACT:
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
            SendAttackMsg(op, hitter, attacknum, (int) dam, damage);
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
            SendAttackMsg(op, hitter, attacknum, (int) dam, damage);
            break;

        case ATNR_CHANNELLING:
            *flags |=HIT_FLAG_DMG|MATERIAL_BASE_MAGICAL;
            dam = hitter->stats.sp; // use mana to directly damage
            ATTACK_HIT_DAMAGE(hitter, attacknum);       /* get % of dam from this attack form */
            if (op->resist[attacknum])
                ATTACK_RESIST_DAMAGE(op, attacknum);    /* reduce to % resistance */
            if (damage && dam < 1.0)
                dam = 1.0;
            SendAttackMsg(op, hitter, attacknum, (int) dam, damage);
            break;

        case ATNR_WEAPONMAGIC:
            *flags |=(HIT_FLAG_DMG|HIT_FLAG_DMG_WMAGIC|MATERIAL_BASE_SPECIAL);
            ATTACK_HIT_DAMAGE(hitter, attacknum);       /* get % of dam from this attack form */
            if (op->resist[attacknum])
                ATTACK_RESIST_DAMAGE(op, attacknum);    /* reduce to % resistance */
            if (damage && dam < 1.0)
                dam = 1.0;
            SendAttackMsg(op, hitter, attacknum, (int) dam, damage);
            break;

        case ATNR_POISON:
            *flags |=HIT_FLAG_DMG|MATERIAL_BASE_ELEMENTAL;
            ATTACK_HIT_DAMAGE(hitter, attacknum);       /* get % of dam from this attack form */
            if (op->resist[attacknum])
                ATTACK_RESIST_DAMAGE(op, attacknum);    /* reduce to % resistance */
            if (damage && dam < 1.0)
                dam = 1.0;
            SendAttackMsg(op, hitter, attacknum, (int) dam, damage);
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
            SendAttackMsg(op, hitter, attacknum, (int) dam, damage);
            /*
            if (RANDOM_ROLL(0, (int)dam + 4) > RANDOM_ROLL(0, 39) + 2 * tmp->magic)
            {
                if (op->type == PLAYER)
                {
                    ndi(NDI_UNIQUE | NDI_RED, 0, op, "%s's acid corrodes %s!",
                        QUERY_SHORT_NAME(hitter, NULL),
                        QUERY_SHORT_NAME(tmp, op));
                }

                flag = 1;
                tmp->magic--;
                esrv_send_item(tmp);
            }
            */
            break;

        case ATNR_CORRUPTION:
            *flags |=HIT_FLAG_DMG|MATERIAL_BASE_MAGICAL;
            ATTACK_HIT_DAMAGE(hitter, attacknum);       /* get % of dam from this attack form */
            if (op->resist[attacknum])
                ATTACK_RESIST_DAMAGE(op, attacknum);    /* reduce to % resistance */
            if (damage && dam < 1.0)
                dam = 1.0;
            leech_hind(hitter, op, attacknum, (int)dam, damage, chance);
            dam = 0.0; // don't do actual (hp) damage
            break;

        case ATNR_LIGHT: /* has a chance to blind the target */
            *flags |=HIT_FLAG_DMG|MATERIAL_BASE_MAGICAL;
            ATTACK_HIT_DAMAGE(hitter, attacknum);       /* get % of dam from this attack form */
            if (op->resist[attacknum])
                ATTACK_RESIST_DAMAGE(op, attacknum);    /* reduce to % resistance */
            if (damage && dam < 1.0)
                dam = 1.0;
            SendAttackMsg(op, hitter, attacknum, (int) dam, damage);
            /* TODO: add blinding effect */
            break;

        case ATNR_SHADOW:
            *flags |=HIT_FLAG_DMG|MATERIAL_BASE_MAGICAL;
            ATTACK_HIT_DAMAGE(hitter, attacknum);       /* get % of dam from this attack form */
            if (op->resist[attacknum])
                ATTACK_RESIST_DAMAGE(op, attacknum);    /* reduce to % resistance */
            if (damage && dam < 1.0)
                dam = 1.0;
            SendAttackMsg(op, hitter, attacknum, (int) dam, damage);
            break;

        case ATNR_PSIONIC:
            *flags |=HIT_FLAG_DMG|MATERIAL_BASE_MAGICAL;
            ATTACK_HIT_DAMAGE(hitter, attacknum);       /* get % of dam from this attack form */
            if (op->resist[attacknum])
                ATTACK_RESIST_DAMAGE(op, attacknum);    /* reduce to % resistance */
            if (damage && dam < 1.0)
                dam = 1.0;
            leech_hind(hitter, op, attacknum, (int)dam, damage, chance);
            dam = 0.0; // don't do actual (hp) damage
            break;

        case ATNR_LIFESTEAL:
            *flags |=HIT_FLAG_DMG|MATERIAL_BASE_MAGICAL;
            ATTACK_HIT_DAMAGE(hitter, attacknum);       /* get % of dam from this attack form */
            if (op->resist[attacknum])
                ATTACK_RESIST_DAMAGE(op, attacknum);    /* reduce to % resistance */
            if (damage && dam < 1.0)
                dam = 1.0;
            leech_hind(hitter, op, attacknum, (int)dam, damage, chance);
            dam = 0.0; // don't do actual (hp) damage
            /* (this is old code )
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
            SendAttackMsg(op, hitter, attacknum, (int) dam, damage);
            /* TODO: add special effects */
            break;
        case ATNR_CHAOS:
            *flags |=HIT_FLAG_DMG|MATERIAL_BASE_SPHERICAL;
            ATTACK_HIT_DAMAGE(hitter, attacknum);       /* get % of dam from this attack form */
            if (op->resist[attacknum])
                ATTACK_RESIST_DAMAGE(op, attacknum);    /* reduce to % resistance */
            if (damage && dam < 1.0)
                dam = 1.0;
            SendAttackMsg(op, hitter, attacknum, (int) dam, damage);
            /* TODO: add special effects */
            break;
        case ATNR_AETHER:
            *flags |=HIT_FLAG_DMG|MATERIAL_BASE_SPHERICAL;
            ATTACK_HIT_DAMAGE(hitter, attacknum);       /* get % of dam from this attack form */
            if (op->resist[attacknum])
                ATTACK_RESIST_DAMAGE(op, attacknum);    /* reduce to % resistance */
            if (damage && dam < 1.0)
                dam = 1.0;
            SendAttackMsg(op, hitter, attacknum, (int) dam, damage);
            /* TODO: add special effects */
            break;
        case ATNR_NETHER:
            *flags |=HIT_FLAG_DMG|MATERIAL_BASE_SPHERICAL;
            ATTACK_HIT_DAMAGE(hitter, attacknum);       /* get % of dam from this attack form */
            if (op->resist[attacknum])
                ATTACK_RESIST_DAMAGE(op, attacknum);    /* reduce to % resistance */
            if (damage && dam < 1.0)
                dam = 1.0;
            SendAttackMsg(op, hitter, attacknum, (int) dam, damage);
            /* TODO: add special effects */
            break;
        case ATNR_GODPOWER:
            *flags |=HIT_FLAG_DMG|MATERIAL_BASE_SPECIAL;
            ATTACK_HIT_DAMAGE(hitter, attacknum);       /* get % of dam from this attack form */
            if (op->resist[attacknum])
                ATTACK_RESIST_DAMAGE(op, attacknum);    /* reduce to % resistance */
            if (damage && dam < 1.0)
                dam = 1.0;
            SendAttackMsg(op, hitter, attacknum, (int) dam, damage);
            /* TODO: add special effects */
            break;

        /* all the following attacks does no damage but effects */
        case ATNR_DRAIN:
            dam = 0.0;
            if(hitter->attack[attacknum] > (RANDOM()%100)) /* we hit with effect? */
            {
                if (hitter->type == PLAYER)
                    ndi(NDI_ORANGE, 0, hitter, "You drain %s!", op->name);
                if (op->type == PLAYER)
                    ndi(NDI_PURPLE, 0, op, "%s drains you!", hitter->name);

                /* give the target the chance to resist */
                if(op->resist[attacknum] > (RANDOM()%100)) /* resisted? */
                {
                    if (hitter->type == PLAYER)
                        ndi(NDI_YELLOW, 0, hitter, "%s resists the drain!", op->name);
                    if (op->type == PLAYER)
                        ndi(NDI_YELLOW, 0, op, "You resist!");
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
                    ndi(NDI_ORANGE, 0, hitter, "You deplete %s!", op->name);
                if (op->type == PLAYER)
                    ndi(NDI_PURPLE, 0, op, "%s depletes you!", hitter->name);

                /* give the target the chance to resist */
                if(op->resist[attacknum] > (RANDOM()%100)) /* resisted? */
                {
                    if (hitter->type == PLAYER)
                        ndi(NDI_YELLOW, 0, hitter, "%s resists depletion!", op->name);
                    if (op->type == PLAYER)
                        ndi(NDI_YELLOW, 0, op, "You resist!");
                }
                else /* effect has hit! */
                    drain_stat(op);
            }
            break;
        case ATNR_CONFUSION:
            dam = 0.0;
            if(hitter->attack[attacknum] > (RANDOM()%100)) /* we hit with effect? */
            {
                if (hitter->type == PLAYER)
                    ndi(NDI_ORANGE, 0, hitter, "You confuse %s!", op->name);
                if (op->type == PLAYER)
                    ndi(NDI_PURPLE, 0, op, "%s confuses you!", hitter->name);

                /* give the target the chance to resist */
                if(op->resist[attacknum] > (RANDOM()%100)) /* resisted? */
                {
                    if (hitter->type == PLAYER)
                        ndi(NDI_YELLOW, 0, hitter, "%s resist!", op->name);
                    if (op->type == PLAYER)
                        ndi(NDI_YELLOW, 0, op, "You resists confusion!");
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
                    ndi(NDI_ORANGE, 0, hitter, "You slow %s!", op->name);
                if (op->type == PLAYER)
                    ndi(NDI_PURPLE, 0, op, "%s slows you!", hitter->name);

                /* give the target the chance to resist */
                if(op->resist[attacknum] > (RANDOM()%100)) /* resisted? */
                {
                    if (hitter->type == PLAYER)
                        ndi(NDI_YELLOW, 0, hitter, "%s resist the slow effect!", op->name);
                    if (op->type == PLAYER)
                        ndi(NDI_YELLOW, 0, op, "You resist!");
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
                    ndi(NDI_ORANGE, 0, hitter, "You scared %s!", op->name);
                if (op->type == PLAYER)
                    ndi(NDI_PURPLE, 0, op, "%s scares you!", hitter->name);

                /* give the target the chance to resist */
                if(op->resist[attacknum] > (RANDOM()%100)) /* resisted? */
                {
                    if (hitter->type == PLAYER)
                        ndi(NDI_YELLOW, 0, hitter, "%s resists fear!", op->name);
                    if (op->type == PLAYER)
                        ndi(NDI_YELLOW, 0, op, "You resist!");
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
                    ndi(NDI_ORANGE, 0, hitter, "You snared %s!", op->name);
                if (op->type == PLAYER)
                    ndi(NDI_PURPLE, 0, op, "%s snares you!", hitter->name);

                /* give the target the chance to resist */
                if(op->resist[attacknum] > (RANDOM()%100)) /* resisted? */
                {
                    if (hitter->type == PLAYER)
                        ndi(NDI_YELLOW, 0, hitter, "%s resists the snare!", op->name);
                    if (op->type == PLAYER)
                        ndi(NDI_YELLOW, 0, op, "You resist!");
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
                    ndi(NDI_ORANGE, 0, hitter, "You paralyzed %s!", op->name);
                if (op->type == PLAYER)
                    ndi(NDI_PURPLE, 0, op, "%s paralyzes you!", hitter->name);

                /* give the target the chance to resist */
                if(op->resist[attacknum] > (RANDOM()%100)) /* resisted? */
                {
                    if (hitter->type == PLAYER)
                        ndi(NDI_YELLOW, 0, hitter, "%s resists the paralyzation effect!", op->name);
                    if (op->type == PLAYER)
                        ndi(NDI_YELLOW, 0, op, "You resist!");

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
                    ndi(NDI_ORANGE, 0, hitter, "You use countermagic on %s!", op->name);
                if (op->type == PLAYER)
                    ndi(NDI_PURPLE, 0, op, "%s uses countermagic on you!", hitter->name);

                /* give the target the chance to resist */
                if(1 && op->resist[attacknum] > (RANDOM()%100)) /* resisted? */
                {
                    if (hitter->type == PLAYER)
                        ndi(NDI_YELLOW, 0, hitter, "%s resists the countermagic!", op->name);
                    if (op->type == PLAYER)
                        ndi(NDI_YELLOW, 0, op, "You resist!");
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
                    ndi(NDI_ORANGE, 0, hitter, "You cancellate %s!", op->name);
                if (op->type == PLAYER)
                    ndi(NDI_PURPLE, 0, op, "%s cancellates you!", hitter->name);

                /* give the target the chance to resist */
                if(1 && op->resist[attacknum] > (RANDOM()%100)) /* resisted? */
                {
                    if (hitter->type == PLAYER)
                        ndi(NDI_YELLOW, 0, hitter, "%s resist the cancellation!", op->name);
                    if (op->type == PLAYER)
                        ndi(NDI_YELLOW, 0, op, "You resist!");
                }
                else /* effect has hit! */
                {
                    /* not implemented ATM */
                }
            }
            break;
        default:
            LOG(llevBug,"BUG:: %s/HitPlayerAttacktype(): Unknown attack: %d on %s[%d]!\n",
                __FILE__, attacknum, STRING_OBJ_NAME(hitter), TAG(hitter));
          break;
    }

    return (int) dam;
}

/* we need this called spread in the function before because sometimes we want drop
 * a message BEFORE we tell the damage and sometimes we want a message after it.
 */
static void SendAttackMsg(object_t *op, object_t *hitter, int attacknum, int dam,
                          int damage)
{
    /* TODO: This is a bit hacky because we need to maintain compatibility with
     * the entire 0.10 series. In 0.11.0 we will just send a small sockbuf of
     * critical numbers and name strings; the client can then produce a full
     * message from that. */
    /* The basic of this is to communicate the attack to 0.10.6 clients. */
    if (op->type == PLAYER)
    {
        ndi(NDI_PURPLE, 0, op, "%s hits you for %d (%+d) damage with %s.",
                      hitter->name, (int)dam, ((int)dam) - damage,
                      attack_name[attacknum].name);
    }

    if (hitter->type == PLAYER ||
        ((hitter = get_owner(hitter)) &&
        hitter->type == PLAYER))
    {
        ndi(NDI_ORANGE, 0, hitter, "You hit %s for %d (%+d) damage with %s.",
                      op->name, (int)dam, ((int)dam) - damage,
                      attack_name[attacknum].name);
    }
}

/* Check if target and hitter are still in a relation similar to the one
 * determined by GetAttackMode(). */
static attack_envmode_t AbortAttack(object_t *target, object_t *hitter, attack_envmode_t env_attack)
{
    if (QUERY_FLAG(target, FLAG_REMOVED) ||
        QUERY_FLAG(hitter, FLAG_REMOVED) ||
        !(hitter->env == target &&
          target->env == hitter &&
          hitter->map &&
          on_same_tileset(hitter->map, target->map)))
    {
        return ENV_ATTACK_YES;
    }

    return env_attack != ENV_ATTACK_NO;
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
object_t * hit_with_arrow(object_t *op, object_t *victim)
{
    object_t *owner;
    int     hit_something   = 0;
    tag_t   owner_tag;

    owner = op->owner;
    owner_tag = op->owner_count;

    if(!trigger_object_plugin_event(EVENT_ATTACK,
                victim, op, owner, NULL, NULL, NULL, NULL, SCRIPT_FIX_ALL))
    {
        /*LOG(llevNoLog, "hit: %s (%d %d)\n", hitter->name, op->stats.dam, op->stats.wc);*/
        hit_something = attack_ob(victim, owner, op);
    }

    /* hopefully the walk_off event was triggered somewhere there */
    if (!OBJECT_VALID(owner, owner_tag) || owner->env != NULL)
        return NULL;

    /* Missile hit victim */
    if (hit_something)
    {
        trigger_object_plugin_event(EVENT_STOP,
                op, victim, NULL, NULL, NULL, NULL, NULL, SCRIPT_FIX_ALL);

        stop_missile(op);
        return NULL;
    }

    return op;
}

void poison_player(object_t *op, object_t *hitter, float dam)
{

/* TODO: i have not the time now - but later we should do this: a marker value
 * in the poison force flags kind of poison - food/poison force or weapon/spell
 * poison. Weapon/spell force will block now poisoning, but will not be blocked
 * by food/poison forces - and food/poison forces can stack. MT-2003
 * NOTE: only poison now insert a poison force - food&drink insert different forces.
 * For Poison objects, we use simply the base dam * level
 *
 * _people_: I changed it so that poison foods use the same object as a poison attack.
 *           Poison object stats are still handled the same way, though.
 */

    archetype_t  *at  = find_archetype("poisoning");
    object_t     *tmp = present_arch_in_ob(at, op);

    /* this is to avoid stacking poison forces... Like we get hit 10 times
     * by a spell and sword and get 10 poison force in us
     * The bad point is, that we can cheat here... Lets say we poison us self
     * with a mild food poison and then we battle the god of poison - he can't
     * hurt us with poison until the food poison wears out
     */

    /* we only poison players and mobs! */
    if (op->type != PLAYER && op->type != MONSTER)
        return;

    if (tmp == NULL || hitter->type == POISON)
    {
        if ((tmp = arch_to_object(at)) == NULL)
        {
            LOG(llevBug, "BUG:: %s/poison_player(): Failed to clone arch poisoning.\n",
                __FILE__);
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
                tmp->speed_left = tmp->speed; /* speed of ticks */
            }

            if (op->type == PLAYER)
            {
                /* spells should add here too later */
                /* her we handle consuming poison */
                if (hitter->type == POISON)
                {
                    //create_food_buf_force(op, hitter, tmp); /* this calculates the food force and inserts it into the player */
                    ndi(NDI_UNIQUE, 0, op, "You suddenly feel very ill.");
                }
                else /* and here we have hit with weapon or something */
                {
                    /* this is where we deal with stat loss from poison attacks */
                    if (op->stats.Con > 1 && !(RANDOM() % 2))
                    {
                        tmp->stats.Con = (sint8) (hitter->level / 10 + RANDOM() % (hitter->level * 8) / 100.0f + 2.0f);
                        tmp->stats.Con *= -1;
                    }
                    if (op->stats.Str > 1 && !(RANDOM() % 2))
                    {
                        tmp->stats.Str = (sint8) (hitter->level / 10 + RANDOM() % (hitter->level * 8) / 100.0f + 2.0f);
                        tmp->stats.Str *= -1;
                    }
                    if (op->stats.Dex > 1 && !(RANDOM() % 2))
                    {
                        tmp->stats.Dex = (sint8) (hitter->level / 10 + RANDOM() % (hitter->level * 8) / 100.0f + 2.0f);
                        tmp->stats.Dex *= -1;
                    }
                    if (op->stats.Int > 1 && !(RANDOM() % 2))
                    {
                        tmp->stats.Int = (sint8) (hitter->level / 10 + RANDOM() % (hitter->level * 8) / 100.0f + 2.0f);
                        tmp->stats.Int *= -1;
                    }
                    if (op->stats.Cha > 1 && !(RANDOM() % 2))
                    {
                        tmp->stats.Cha = (sint8) (hitter->level / 10 + RANDOM() % (hitter->level * 8) / 100.0f + 2.0f);
                        tmp->stats.Cha *= -1;
                    }
                    if (op->stats.Pow > 1 && !(RANDOM() % 2))
                    {
                        tmp->stats.Pow = (sint8) (hitter->level / 10 + RANDOM() % (hitter->level * 8) / 100.0f + 2.0f);
                        tmp->stats.Pow *= -1;
                    }
                    if (op->stats.Wis > 1 && !(RANDOM() % 2))
                    {
                        tmp->stats.Wis = (sint8) (hitter->level / 10 + RANDOM() % (hitter->level * 8) / 100.0f + 2.0f);
                        tmp->stats.Wis *= -1;
                    }

                    ndi(NDI_UNIQUE, 0, op, "%s has poisoned you!",
                        QUERY_SHORT_NAME(hitter, NULL));
                }
                tmp = check_obj_stat_buffs(tmp, op);
                insert_ob_in_ob(tmp, op);
                FIX_PLAYER(op , "attack - poison");
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
                        ndi(NDI_UNIQUE, 0, hitter, "You poisoned %s!",
                            QUERY_SHORT_NAME(op, NULL));
                    else if (get_owner(hitter) && hitter->owner->type == PLAYER)
                        ndi(NDI_UNIQUE, 0, hitter->owner, "%s poisoned %s!",
                            QUERY_SHORT_NAME(hitter, NULL),
                            QUERY_SHORT_NAME(op, NULL));
                }
            }
        }
    }
    else
        tmp->stats.food++;
}

void slow_player(object_t *op, object_t *hitter, int dam)
{
    static archetype_t  *at  = NULL;
    object_t     *tmp;
    int        max_slow = 0;

    if (!at)
    {
        at = find_archetype("slowness");
        if (!at)
        {
            LOG(llevBug, "BUG:: %s/slow_player(): Couldn't find archetype slowness.\n",
                __FILE__);
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
            max_slow = 1;
            tmp->last_heal = 70;
        }
    }

    tmp->stats.food += 8; /* slow effect last longer */
    if(tmp->stats.food >= 25)
        tmp->stats.food = 25;


    if(!max_slow)
    {
        ndi(NDI_UNIQUE, 0, op, "The world suddenly moves faster!");

        if (!QUERY_FLAG(op, FLAG_CONFUSED) &&
             op->map)
        {
            ndi_map(NDI_UNIQUE | NDI_GREY, MSP_KNOWN(op), MAP_INFO_NORMAL, NULL, op, "%s suddenly moves slower!",
                QUERY_SHORT_NAME(op, NULL));
        }

        SET_FLAG(tmp, FLAG_APPLIED);
        FIX_PLAYER(op ," attack - slow"); /* will set FLAG_SLOWED */
    }

}

void fear_player(object_t *op, object_t *hitter, int dam)
{
    static archetype_t  *at  = NULL;
    object_t     *tmp;
    int        max_slow = 0;

    if (!at)
    {
        at = find_archetype("fear");
        if (!at)
        {
            LOG(llevBug, "BUG:: %s/fear_player(): Couldn't find archetype fear!\n",
                __FILE__);
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
        ndi(NDI_UNIQUE, 0, op, "You are scared to death!");

        if (!QUERY_FLAG(op, FLAG_CONFUSED) &&
            op->map)
        {
            ndi_map(NDI_UNIQUE | NDI_GREY, MSP_KNOWN(op), MAP_INFO_NORMAL, NULL, op, "%s suddenly looks scared!",
                QUERY_SHORT_NAME(op, NULL));
        }

        SET_FLAG(tmp, FLAG_APPLIED);
        FIX_PLAYER(op ,"attack fear"); /* will set FLAG_FEAR */
    }

}

void snare_player(object_t *op, object_t *hitter, int dam)
{
    static archetype_t  *at  = NULL;
    object_t     *tmp;
    int        max_slow = 0;

    if (!at)
    {
        at = find_archetype("snare");
        if (!at)
        {
            LOG(llevBug, "BUG:: %s/snare_player(): Couldn't find archetype snare!\n",
                __FILE__);
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
                max_slow = 1;
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
       ndi(NDI_UNIQUE, 0, op, "You are %s!",
           (tmp->last_heal < 100) ? "snared" : "rooted");

        if (!QUERY_FLAG(op, FLAG_CONFUSED) &&
            op->map)
        {
            ndi_map(NDI_UNIQUE | NDI_GREY, MSP_KNOWN(op), MAP_INFO_NORMAL, NULL, op, "%s suddenly %s!",
                QUERY_SHORT_NAME(op, NULL)                                                              ,
                (tmp->last_heal < 100) ? "walks slower" : "stops walking");
        }

        SET_FLAG(tmp, FLAG_APPLIED);
        FIX_PLAYER(op ,"attack snear "); /* will set FLAG_SNEAR */
    }

}

void confuse_player(object_t *op, object_t *hitter, int ticks)
{
    static archetype_t  *at  = NULL;
    object_t     *tmp;

    if (!at)
    {
        at = find_archetype("confusion");
        if (!at)
        {
            LOG(llevBug, "BUG:: %s/confuse_player(): Couldn't find archetype confusion!\n",
                __FILE__);
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

    ndi(NDI_UNIQUE, 0, op, "You suddenly feel very confused!");

    if (!QUERY_FLAG(op, FLAG_CONFUSED) &&
        op->map)
    {
        ndi_map(NDI_UNIQUE | NDI_GREY, MSP_KNOWN(op), MAP_INFO_NORMAL, NULL, op, "%s looks confused!",
            QUERY_SHORT_NAME(op, NULL));
    }

    SET_FLAG(tmp, FLAG_APPLIED);
    SET_FLAG(op, FLAG_CONFUSED);
}

void blind_player(object_t *op, object_t *hitter, int dam)
{
    static archetype_t  *at  = NULL;
    object_t     *tmp;

    if (!at)
    {
        at = find_archetype("blindness");
        if (!at)
        {
            LOG(llevBug, "BUG:: %s/blind_player(): Couldn't find archetype blindness!\n",
                __FILE__);
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

    ndi(NDI_UNIQUE, 0, op, "You suddenly are blinded!");

    if (!QUERY_FLAG(op, FLAG_CONFUSED) &&
         op->map)
    {
        ndi_map(NDI_UNIQUE | NDI_GREY, MSP_KNOWN(op), MAP_INFO_NORMAL, NULL, op, "%s looks blinded!",
            QUERY_SHORT_NAME(op, NULL));
    }

    SET_FLAG(tmp, FLAG_APPLIED);
    SET_FLAG(op, FLAG_BLIND);
}


void paralyze_player(object_t *op, object_t *hitter, int dam)
{
    static archetype_t  *at  = NULL;
    object_t     *tmp;

    if (!at)
    {
        at = find_archetype("paralyze");
        if (!at)
        {
            LOG(llevBug, "BUG:: %s/paralyze_player(): Couldn't find archetype paralyze!\n",
                __FILE__);
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

    ndi(NDI_UNIQUE, 0, op, "You suddenly feel paralyzed!");

    if (!QUERY_FLAG(op, FLAG_CONFUSED) &&
        op->map)
    {
        ndi_map(NDI_UNIQUE | NDI_GREY, MSP_KNOWN(op), MAP_INFO_NORMAL, NULL, op, "%s looks paralyzed!",
            QUERY_SHORT_NAME(op, NULL));
    }

    SET_FLAG(tmp, FLAG_APPLIED);
    FIX_PLAYER(op ,"attach paralyze"); /* will set FLAG_PARALYZE */
}

/* remove paralyze effect and force (if there is one)
 * Note: This is for explicit remove - in time.c the force can
 * auto destruct itself without calling this function. This.
*/
void remove_paralyze(object_t *op)
{
    static archetype_t  *at  = NULL;
    object_t     *tmp;

    CLEAR_FLAG(op,FLAG_PARALYZED);

    if (!at)
    {
        at = find_archetype("paralyze");
        if (!at)
        {
            LOG(llevBug, "BUG:: %s/remove_paralyze(): Couldn't find archetype paralyze!\n",
                __FILE__);
            return;
        }
    }

    if ((tmp = present_arch_in_ob(at, op)) == NULL)
        return;

    remove_ob(tmp);
    ndi(NDI_UNIQUE, 0, op, "You can move again.");

    if (op->map)
    {
        ndi_map(NDI_UNIQUE | NDI_GREY, MSP_KNOWN(op), MAP_INFO_NORMAL, NULL, op, "%s can move again!",
            QUERY_SHORT_NAME(op, NULL));
    }

    FIX_PLAYER(op ,"attack - remove paralyze");
}

/* determine if the object is an 'aimed' missile */
int is_aimed_missile(object_t *op)
{
    if (op &&
        QUERY_FLAG(op, FLAG_FLYING) &&
        IS_ARROW(op))
    {
        return 1;
    }

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
 * NOTE: check for valid target outside here. */
int is_melee_range(object_t *hitter, object_t *enemy)
{
    uint8 i;

    for (i = 0; i < 9; i++) /* check squares around AND our own position */
    {
        map_t *m = hitter->map;
        sint16     x = hitter->x + OVERLAY_X(i),
                   y = hitter->y + OVERLAY_Y(i);
        msp_t  *msp = MSP_GET2(m, x, y);
        object_t    *this;

        if (!msp)
        {
            continue;
        }

        for (this = enemy; this; this = this->more)
        {
            if (this->map == m &&
                this->x == x &&
                this->y == y)
            {
                return 1;
            }
        }
    }

    return 0;
}
