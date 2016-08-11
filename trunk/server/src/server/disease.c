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

/*  This file contains all the code implementing diseases,
    except for odds and ends in attack.c and in
     living.c*/



/*

For DISEASES:
Stat            Property        Definition

attacktype      Attack effects  Attacktype of the disease. usu. AT_GODPOWER.
other_arch      Creation        object created and dropped when symptom moved.
title           Message         When the "disease" "infects" something, it will
                                print "title victim!!!" to the player who owns
                the "disease".
wc+             Infectiousness  How well the plague spreads person-to-person
magic+          Range           range of infection
Stats*          Disability      What stats are reduced by the disease (str con...)
maxhp+          Persistence     How long the disease can last OUTSIDE the host.
weight_limit    TimeLeft        Counter for persistence
dam^            Damage          How much damage it does (%?).
maxgrace+       Duration        How long before the disease is naturally cured.
food            DurCount        Counter for Duration

speed           Speed           How often the disease moves.
last_sp^        Lethargy        Percentage of max speed--10 = 10% speed.

maxsp^          Mana deplete    Saps mana.
ac^             Progressiveness How the diseases increases in severity.
last_eat*^      Deplete food    saps food if negative
last_heal       GrantImmunity   If nonzero, disease does NOT grant immunity
                                when it runs out

exp             experience      experience awarded when plague cured
hp*^            ReduceRegen     reduces regeneration of disease-bearer
sp*^            ReduceSpRegen   reduces spellpoint regeneration

name            Name            Name of the plague
msg             message         What the plague says when it strikes.
race            those affected  species/race the plague strikes (* = everything)
level           Plague Level    General description of the plague's deadliness
armour          Attenuation     reduction in wc per generation of disease.
                                This builds in a self-limiting factor.


Explanations:
* means this # should be negative to cause adverse effect.
+ means that this effect is modulated in spells by ldur
^ means that this effect is modulated in spells by ldam

attacktype is the attacktype used by the disease to smite "dam" damage with.

wc/127 is the chance of someone in range catching it.

magic is the range at which infection may occur.  If negative, the range is
NOT level dependent.

Stats are stat modifications.  These should typically be negative.

maxhp is how long the disease will persist if the host dies and "drops" it,
      in "disease moves", i.e., moves of the disease.  If negative, permanent.


weight_limit is the counter for maxhp, it starts at maxhp and drops...

dam     if positive, it is straight damage.  if negative, a %-age.

maxgrace  how long in "disease moves" the disease lasts in the host, if negative,
          permanent until cured.

food    if negative, disease is permanent.  otherwise, decreases at <speed>,
        disease goes away at food=0, set to "maxgrace" on infection.

speed is the speed of the disease, how fast "disease moves" occur.

last_sp is the lethargy imposed on the player by the disease.  A lethargy
       of "1" reduces the players speed to 1% of its normal value.

maxsp how much mana is sapped per "disease move".  if negative, a %-age is
     taken.

ac  every "disease move" the severity of the symptoms are increased by
    ac/100.  (severity = 1 + (accumlated_progression)/100)

last_eat  increases food usage if negative.



For SYMPTOMS:

Stats            modify stats
hp               modify regen
weight_limit     progression counter (multiplier = weight_limit/100)
food             modify food use (from last_eat in DISEASE)
maxsp            suck mana ( as noted for DISEASE)
last_sp          Lethargy
msg              What to say
speed            speed of movement, from DISEASE


*/


#include <global.h>

/*  IMPLEMENTATION NOTES

     Diseases may be contageous.  They are objects which exist in a player's
inventory.  They themselves do nothing, except modify Symptoms, or
spread to other live objects.  Symptoms are what actually damage the player:
these are their own object. */

/* check if victim is susceptible to disease. */
static int is_susceptible_to_disease(object_t *victim, object_t *disease)
{
    if (!IS_LIVE(victim))
        return 0;
    if (strstr(disease->race, "*") && !QUERY_FLAG(victim, FLAG_UNDEAD))
        return 1;
    if (strstr(disease->race, "undead") && QUERY_FLAG(victim, FLAG_UNDEAD))
        return 1;
    if ((victim->race && strstr(disease->race, victim->race)) || strstr(disease->race, victim->name))
        return 1;

    return 0;
}

int move_disease(object_t *disease)
{
    /*  first task is to determine if the disease is inside or outside of someone.
    If outside, we decrement 'hp' until we're gone. */

    if (disease->env == NULL)
    {
        /* we're outside of someone */
        disease->weight_limit--;
        if (disease->weight_limit == 0)
        {
            /* TODO: This is not a correct usage, but diseases are currently
             * broken anyway.
             *
             * -- Smacky 20150204 */
            /* drop inv since disease may carry secondary infections */
            (void)kill_object(disease, NULL, NULL, NULL);
            return 1;
        }
    }
    else
    {
        /* if we're inside a person, have the disease run its course */
        /* negative foods denote "perpetual" diseases. */
        if (disease->stats.food > 0 && is_susceptible_to_disease(disease->env, disease))
        {
            disease->stats.food--;
            if (disease->stats.food == 0)
            {
                remove_symptoms(disease);  /* remove the symptoms of this disease */
                grant_immunity(disease);

                /* TODO: This is not a correct usage, but diseases are currently
                 * broken anyway.
                 *
                 * -- Smacky 20150204 */
                /* drop inv since disease may carry secondary infections */
                (void)kill_object(disease, NULL, NULL, NULL);
                return 1;
            }
        }
    }

    /*  check to see if we infect others */
    check_infection(disease);

    /* impose or modify the symptoms of the disease */
    if (disease->env)
        do_symptoms(disease);

    return 0;
}

/* remove any symptoms of disease */

int remove_symptoms(object_t *disease)
{
    object_t *symptom;
    symptom = find_symptom(disease);
    if (symptom != NULL)
    {
        object_t *victim  = symptom->env;
        /* TODO: This is not a correct usage, but diseases are currently
         * broken anyway.
         *
         * -- Smacky 20150204 */
        (void)kill_object(symptom, NULL, NULL, NULL);
        if (victim)
            FIX_PLAYER(victim ,"remove symptoms ");
    }
    return 0;
}

/* argument is a disease */
object_t * find_symptom(object_t *disease)
{
    object_t *walk,
           *next;

    /* check the inventory for symptoms */
    FOREACH_OBJECT_IN_OBJECT(walk, disease->env, next)
    {
        if (walk->name == disease->name &&
            walk->type == SYMPTOM)
        {
            return walk;
        }
    }

    return NULL;
}

/*  searches around for more victims to infect */
int check_infection(object_t *disease)
{
    object_t     *outbreak = (disease->env) ? disease->env : disease;
    sint16      x,
                y;
    int         range;

    if (!outbreak->map)
    {
        return 0;
    }

    range = ABS(disease->magic);

    for (x = outbreak->x - range; x < outbreak->x + range; x++)
    {
        for (y = outbreak->y - range; y < outbreak->y + range; y++)
        {
            map_t *m = outbreak->map;
            msp_t  *msp = MSP_GET2(m, x, y);
            object_t    *this,
                      *next;

            if (!msp)
            {
                continue;
            }

            FOREACH_OBJECT_IN_MSP(this, msp, next)
            {
                infect_object(this, disease, 0);
            }
        }
    }

    return 1;
}


/*  check to see if an object is infectable:
     objects with immunity aren't infectable.
     objects already infected aren't infectable.
     dead objects aren't infectable.
     undead objects are infectible only if specifically named.
*/
int infect_object(object_t *victim, object_t *disease, int force)
{
    object_t *tmp,
           *next;
    object_t *new_disease;

    /* only use the head */
    if (victim->head)
        victim = victim->head;

    /* don't infect inanimate objects */
    if (victim->type != MONSTER && victim->type != PLAYER)
        return 0;

    /* check and see if victim can catch disease:  diseases
      are specific */
    if (!is_susceptible_to_disease(victim, disease))
        return 0;

    /* roll the dice on infection before doing the inventory check!  */
    if (!force && (random_roll(0, 126) >= disease->stats.wc))
        return 0;

    FOREACH_OBJECT_IN_OBJECT(tmp, victim, next)
    {
        if (tmp->type == SIGN || tmp->type == DISEASE)  /* possibly an immunity, or diseased*/
            if (tmp->name == disease->name && tmp->level >= disease->level)
                return 0;  /*Immune! */
    }


    /*  If we've gotten this far, go ahead and infect the victim.  */
    new_disease = get_object();
    copy_object(disease, new_disease);
    new_disease->stats.food = disease->stats.maxgrace;
    new_disease->weight_limit = disease->stats.maxhp;
    new_disease->stats.wc -= disease->last_grace;  /* self-limiting factor */

    /* Unfortunately, set_owner does the wrong thing to the skills pointers
      resulting in exp going into the owners *current* chosen skill. */

    if (get_owner(disease))
    {
        set_owner(new_disease, disease->owner);
        new_disease->chosen_skill = disease->chosen_skill;
        new_disease->skillgroup = disease->skillgroup;
    }
    else
    {
        /* for diseases which are passed by hitting, set owner and praying skill*/
        if (disease->env && disease->env->type == PLAYER)
        {
            object_t *player  = disease->env;

            /* hm, we should for hit use the weapon? or the skill attached to this
               * specific disease? hmmm
               */
            new_disease->chosen_skill = find_skill(player, SK_DIVINE_PRAYERS);

            if (new_disease->chosen_skill)
            {
                set_owner(new_disease, player);
                new_disease->skillgroup = new_disease->chosen_skill->skillgroup;
            }
        }
    }

    insert_ob_in_ob(new_disease, victim);
    CLEAR_FLAG(new_disease, FLAG_NO_PASS);
    if (new_disease->owner && new_disease->owner->type == PLAYER)
    {
        char    buf[SMALL_BUF];
        /* if the disease has a title, it has a special infection message */
        /* This messages is printed in the form MESSAGE victim */
        if (new_disease->title)
            sprintf(buf, "%s %s!!", disease->title, victim->name);
        else
            sprintf(buf, "You infect %s with your disease, %s!", victim->name, new_disease->name);
        if (victim->type == PLAYER)
            ndi(NDI_UNIQUE | NDI_RED, 0, new_disease->owner, "%s", buf);
        else
            ndi(0, 4, new_disease->owner, "%s", buf);
    }
    if (victim->type == PLAYER)
        ndi(NDI_UNIQUE | NDI_RED, 0, victim, "You suddenly feel ill.");
    return 1;
}



/*  this function monitors the symptoms caused by the disease (if any),
causes symptoms, and modifies existing symptoms in the case of
existing diseases.  */

int do_symptoms(object_t *disease)
{
    object_t *symptom;
    object_t *victim;
    object_t *tmp;
    victim = disease->env;
    /* This is a quick hack - for whatever reason, disease->env will point
     * back to disease, causing endless loops.  Why this happens really needs
     * to be found, but this should at least prevent the infinite loops.
     */
    if (victim == NULL || victim == disease)
        return 0;/* no-one to inflict symptoms on */
    symptom = find_symptom(disease);
    if (symptom == NULL) /* no symptom?  need to generate one! */
    {
        object_t *new_symptom;
        /* first check and see if the carrier of the disease
        is immune.  If so, no symptoms!  */
        if (!is_susceptible_to_disease(victim, disease))
            return 0;

        /* check for an actual immunity */

        /* do an immunity check */
        /* hm, disease should be NEVER in a tail of a multi part object_t */
        if (victim->head)
            tmp = victim->head->inv;
        else
            tmp = victim->inv;

        for (/* tmp initialized in if, above */; tmp; tmp = tmp->below)
        {
            if (tmp->type == SIGN)  /* possibly an immunity, or diseased*/
                if (tmp->name == disease->name && tmp->level >= disease->level)
                    return 0;  /*Immune! */
        }

        new_symptom = get_archetype("symptom");

        /* Something special done with dam.  We want diseases to be more
        random in what they'll kill, so we'll make the damage they
        do random, note, this has a weird effect with progressive diseases.*/
        if (disease->stats.dam != 0)
        {
            int dam = disease->stats.dam;
            /* reduce the damage, on average, 50%, and making things random. */
            dam = random_roll(1, ABS(dam));
            if (disease->stats.dam < 0)
                dam = -dam;
            new_symptom->stats.dam = dam;
        }



        new_symptom->stats.maxsp = disease->stats.maxsp;
        new_symptom->stats.food = new_symptom->stats.maxgrace;

        SHSTR_FREE_AND_ADD_STRING(new_symptom->name, disease->name);
        new_symptom->level = disease->level;
        new_symptom->speed = disease->speed;
        new_symptom->weight_limit = 0;
        new_symptom->stats.Str = disease->stats.Str;
        new_symptom->stats.Dex = disease->stats.Dex;
        new_symptom->stats.Con = disease->stats.Con;
        new_symptom->stats.Wis = disease->stats.Wis;
        new_symptom->stats.Int = disease->stats.Int;
        new_symptom->stats.Pow = disease->stats.Pow;
        new_symptom->stats.Cha = disease->stats.Cha;
        new_symptom->stats.sp = disease->stats.sp;
        new_symptom->stats.food = disease->last_eat;
        new_symptom->stats.maxsp = disease->stats.maxsp;
        new_symptom->last_sp = disease->last_sp;
        new_symptom->stats.exp = 0;
        new_symptom->stats.hp = disease->stats.hp;
        SHSTR_FREE_AND_ADD_STRING(new_symptom->msg, disease->msg);
        new_symptom->other_arch = disease->other_arch;

        set_owner(new_symptom, disease->owner);
        /* Unfortunately, set_owner does the wrong thing to the skills pointers
        resulting in exp going into the owners *current* chosen skill. */
        new_symptom->chosen_skill = disease->chosen_skill;
        new_symptom->skillgroup = disease->skillgroup;

        CLEAR_FLAG(new_symptom, FLAG_NO_PASS);
        insert_ob_in_ob(new_symptom, victim);
        return 1;
    }

    /* now deal with progressing diseases:  we increase the debility
       caused by the symptoms.  */

    if (disease->stats.ac != 0)
    {
        float   scale;
        symptom->weight_limit += disease->stats.ac;
        scale = 1.0f + (float) symptom->weight_limit / 100.0f;
        /* now rescale all the debilities */
        symptom->stats.Str = (int) (scale * disease->stats.Str);
        symptom->stats.Dex = (int) (scale * disease->stats.Dex);
        symptom->stats.Con = (int) (scale * disease->stats.Con);
        symptom->stats.Wis = (int) (scale * disease->stats.Wis);
        symptom->stats.Int = (int) (scale * disease->stats.Int);
        symptom->stats.Pow = (int) (scale * disease->stats.Pow);
        symptom->stats.Cha = (int) (scale * disease->stats.Cha);
        symptom->stats.dam = (int) (scale * disease->stats.dam);
        symptom->stats.sp = (int) (scale * disease->stats.sp);
        symptom->stats.food = (int) (scale * disease->last_eat);
        symptom->stats.maxsp = (int) (scale * disease->stats.maxsp);
        symptom->last_sp = (int) (scale * disease->last_sp);
        symptom->stats.exp = 0;
        symptom->stats.hp = (int) (scale * disease->stats.hp);
        SHSTR_FREE_AND_ADD_STRING(symptom->msg, disease->msg);
        symptom->other_arch = disease->other_arch;
    }
    SET_FLAG(symptom, FLAG_APPLIED);
    FIX_PLAYER(victim ,"do symptoms");
    return 1;
}


/*  grants immunity to plagues we've seen before.  */
int grant_immunity(object_t *disease)
{
    object_t *immunity;
    object_t *walk,
           *next;

    /* Don't give immunity to this disease if last_heal is set. */
    if (disease->last_heal)
        return 0;

    /*  first, search for an immunity of the same name */
    FOREACH_OBJECT_IN_OBJECT(walk, disease->env, next)
    {
        if (walk->type == 98 && disease->name == walk->name)
        {
            walk->level = disease->level;
            return 1; /* just update the existing immunity. */
        }
    }

    immunity = get_archetype("immunity");
    SHSTR_FREE_AND_ADD_STRING(immunity->name, disease->name);
    immunity->level = disease->level;
    CLEAR_FLAG(immunity, FLAG_NO_PASS);
    insert_ob_in_ob(immunity, disease->env);
    return 1;
}


/*  make the symptom do the nasty things it does  */

int move_symptom(object_t *symptom)
{
    object_t *victim  = symptom->env;
    object_t *new_ob;
    int     sp_reduce;
    if (victim == NULL || victim->map == NULL)
    {
        /* outside a monster/player, die immediately */
        remove_ob(symptom);
        move_check_off(symptom, NULL, MOVE_FLAG_VANISHED);
        return 0;
    }
    if (symptom->stats.dam > 0)
        damage_ob(victim, symptom->stats.dam, symptom, ENV_ATTACK_CHECK);
    else
        damage_ob(victim, (int) MAX((float) 1,
                    (float) - victim->stats.maxhp * (float) symptom->stats.dam / (float) 100.0), symptom, ENV_ATTACK_CHECK);
    if (symptom->stats.maxsp > 0)
        sp_reduce = symptom->stats.maxsp;
    else
        sp_reduce = (int) MAX((float) 1, (float) victim->stats.maxsp * (float) symptom->stats.maxsp / (float) 100.0);
    victim->stats.sp = MAX(0, victim->stats.sp - sp_reduce);

    /* create the symptom "other arch" object and drop it here
     * under every part of the monster */
    /* The victim may well have died. */
    if (victim->map == NULL)
        return 0;
    if (symptom->other_arch)
    {
        object_t *tmp;
        tmp = victim;
        if (tmp->head != NULL)
            tmp = tmp->head;
        for (/*tmp initialized above */; tmp != NULL; tmp = tmp->more)
        {
            new_ob = arch_to_object(symptom->other_arch);
            new_ob->x = tmp->x;
            new_ob->y = tmp->y;
            new_ob->map = victim->map;
            insert_ob_in_map(new_ob, victim->map, victim, INS_NO_MERGE | INS_NO_WALK_ON);
        }
    }
    if (victim->type == PLAYER)
        ndi(NDI_UNIQUE | NDI_RED, 0, victim, "%s", symptom->msg);

    return 1;
}


/*  possibly infect due to direct physical contact */
int check_physically_infect(object_t *victim, object_t *hitter)
{
    object_t *walk,
           *next;

    /* search for diseases, give every disease a chance to infect */
    FOREACH_OBJECT_IN_OBJECT(walk, hitter, next)
    {
        if (walk->type == DISEASE)
        {
            infect_object(victim, walk, 0);
        }
    }

    return 1;
}

/*  find a disease in someone*/
object_t * find_disease(object_t *victim)
{
    object_t *walk,
           *next;

    FOREACH_OBJECT_IN_OBJECT(walk, victim, next)
    {
        if (walk->type == DISEASE)
        {
            return walk;
        }
    }

    return NULL;
}

/* do the cure disease stuff, from the spell "cure disease" */

int cure_disease(object_t *sufferer, object_t *caster)
{
    object_t *disease, *next;
    int     casting_level, is_disease = 0;

    if (caster)
        casting_level = caster->level;
    else
    {
        caster = sufferer;
        casting_level = 1000;  /* if null caster, CURE all.  */
    }
    if (caster != sufferer && sufferer->type == PLAYER)
        ndi(NDI_UNIQUE, 0, sufferer, "%s casts cure disease on you!",
                             caster->name ? caster->name : "someone");

    FOREACH_OBJECT_IN_OBJECT(disease, sufferer, next)
    {
        if (disease->type == DISEASE)
        {
            /* attempt to cure this disease */
            /* If caster lvel is higher than disease level, cure chance
               * is automatic.  If lower, then the chance is basically
               * 1 in level_diff - if there is a 5 level difference, chance
               * is 1 in 5.
               */
            is_disease = 1;
            if ((casting_level >= disease->level)
             || (!(random_roll(0, (disease->level - casting_level - 1)))))
            {
                if (sufferer->type == PLAYER)
                    ndi(NDI_UNIQUE, 0, sufferer, "You are healed from disease %s.", disease->name);
                if (sufferer != caster && caster->type == PLAYER)
                    ndi(NDI_UNIQUE, 0, caster, "You heal %s from disease %s.", sufferer->name,
                                         disease->name);
                remove_symptoms(disease);
                remove_ob(disease);
                /* we assume the caster has the right casting skill applied */
                if (caster && !caster->type == PLAYER)
                    add_exp(caster, disease->stats.exp, caster->chosen_skill->stats.sp, 1);
            }
            else
            {
                if (sufferer->type == PLAYER)
                    ndi(NDI_UNIQUE, 0, sufferer, "The disease %s resists the cure prayer!",
                                         disease->name);
                if (sufferer != caster && caster->type == PLAYER)
                    ndi(NDI_UNIQUE, 0, caster, "The disease %s resists the cure prayer!", disease->name);
            }
        }
    }
    if (!is_disease)
    {
        if (sufferer->type == PLAYER)
            ndi(NDI_UNIQUE, 0, sufferer, "You are not diseased!");
        if (sufferer != caster && caster->type == PLAYER)
            ndi(NDI_UNIQUE, 0, caster, "%s is not diseased!",
                                 sufferer->name ? sufferer->name : "someone");
    }
    return 1;
}

/* reduces disease progression:  reduce_symptoms
 * return true if we actually reduce a disease.
 */

int reduce_symptoms(object_t *sufferer, int reduction)
{
    object_t *walk,
           *next;
    int     success = 0;

    FOREACH_OBJECT_IN_OBJECT(walk, sufferer, next)
    {
        if (walk->type == SYMPTOM)
        {
            if (walk->weight_limit > 0)
            {
                success = 1;
                walk->weight_limit = MAX(0, walk->weight_limit - 2 * reduction);
                /* give the disease time to modify this symptom,
                     * and reduce its severity.  */
                walk->speed_left = 0;
            }
        }
    }
    if (success)
        ndi(NDI_UNIQUE, 0, sufferer, "Your illness seems less severe.");
    return success;
}

