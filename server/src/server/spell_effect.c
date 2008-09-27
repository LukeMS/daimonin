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

/*
 * spell_failure()  handles the various effects for differing degrees
 * of failure badness.
 */
#ifdef SPELL_FAILURE_EFFECTS

/* Might be a better way to do this, but this should work */
#define ISQRT(val) ((int)sqrt((double) val))

void spell_failure(object *op, int failure, int power)
{
    if (failure <= -20 && failure > -40) /* wonder */
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Your spell causes an unexpected effect.");
        cast_cone(op, op, 0, 10, SP_WOW, spellarch[SP_WOW], SK_level(op),, 0);
    }
    else if (failure <= -40 && failure > -60) /* confusion */
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Your magic recoils on you!");
        confuse_player(op, op, 160);
    }
    else if (failure <= -60 && failure > -80) /* paralysis */
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Your magic recoils on you!");
        paralyze_player(op, op, 99);
    }
    else if (failure <= -80) /* blast the immediate area */
    {
        object *tmp;
        /* Safety check to make sure we don't get any mana storms in scorn */
        if (blocks_magic(op->map, op->x, op->y))
        {
            new_draw_info(NDI_UNIQUE, 0, op, "The magic warps and you are turned inside out!");
            damage_ob(tmp, 9998, op, ENV_ATTACK_CHECK);
        }
        else
        {
            new_draw_info(NDI_UNIQUE, 0, op, "You lose control of the mana!  The uncontrolled magic blasts you!");
            tmp = get_archetype("loose_magic");
            tmp->level = SK_level(op);
            tmp->x = op->x;tmp->y = op->y;

            /* increase the area of destruction a little for more powerful spells */
            tmp->stats.hp += ISQRT(power);

            if (power > 25)
                tmp->stats.dam = 25 + ISQRT(power);
            else
                tmp->stats.dam = power; /* nasty recoils! */

            tmp->stats.maxhp = tmp->count; /*??*/
            insert_ob_in_map(tmp, op->map, NULL, 0);
        }
    }
}
#endif

/* Oct 95 - hacked on this to bring in cosmetic differences for MULTIPLE_GOD hack -b.t. */

void prayer_failure(object *op, int failure, int power)
{
    const char *godname;

    if ((godname = determine_god(op)) == shstr_cons.none)
        godname = "Your spirit";

    if (failure <= -20 && failure > -40) /* wonder */
    {
        new_draw_info_format(NDI_UNIQUE, 0, op, "%s gives a sign to renew your faith.", godname);
        cast_cone(op, op, 0, 10, SP_WOW, spellarch[SP_WOW], SK_level(op), 0);
    }
    else if (failure <= -40 && failure > -60) /* confusion */
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Your diety touches your mind!");
        confuse_player(op, op, 160);
    }
    else if (failure <= -60 && failure > -150) /* paralysis */
    {
        new_draw_info_format(NDI_UNIQUE, 0, op, "%s requires you to pray NOW.", godname);
        new_draw_info(NDI_UNIQUE, 0, op, "You comply, ignoring all else.");
        paralyze_player(op, op, 99);
    }
    else if (failure <= -150) /* blast the immediate area */
    {
        new_draw_info_format(NDI_UNIQUE, 0, op, "%s smites you!", godname);
        cast_magic_storm(op, get_archetype("god_power"), power);
    }
}

/* Should really just replace all calls to cast_mana_storm to call
 * cast_magic_storm directly.
 */

void cast_mana_storm(object *op, int lvl)
{
    object *tmp = get_archetype("loose_magic");

    cast_magic_storm(op, tmp, lvl);
}


void cast_magic_storm(object *op, object *tmp, int lvl)
{
    if (!tmp)
        return; /* error */
    tmp->level = SK_level(op);
    tmp->x = op->x;tmp->y = op->y;
    tmp->stats.hp += lvl / 5;  /* increase the area of destruction */
    tmp->stats.dam = lvl; /* nasty recoils! */
    tmp->stats.maxhp = tmp->count; /*??*/
    insert_ob_in_map(tmp, op->map, op, 0);
}

/* TODO: we need a special flag here to aggravate monster.
 * Enemy will be deleted in check_enemy() if not friendly->non friendly or visa verse */
/* this is a pretty bad implementation and is not working with tiled maps. i
 * comment it out until we write a better aggravation system. MT */
/* TODO: it might be easy to implement now with the new AI system. Gecko 2006-05-01 */
void aggravate_monsters(object *op)
{
    /*
      int i,j;
      object *tmp;

      spell_effect(SP_AGGRAVATION, op->x, op->y, op->map, op);

      for (i = 0; i < MAP_WIDTH(op->map); i++)
        for (j = 0; j < MAP_HEIGHT(op->map); j++) {
          if (out_of_map(op->map, op->x + i , op->y + j))
            continue;
          for (tmp = get_map_ob(op->map, op->x + i, op->y + j);
               tmp; tmp = tmp->above)
            if (QUERY_FLAG(tmp, FLAG_MONSTER)) {
          CLEAR_FLAG(tmp, FLAG_SLEEP);
              if (!QUERY_FLAG(tmp, FLAG_FRIENDLY))
              {
                register_npc_enemy(tmp, op, NULL);
              }
            }
        }
    */
}


int recharge(object *op)
{
    object *wand;

    for (wand = op->inv; wand != NULL; wand = wand->below)
        if (wand->type == WAND && QUERY_FLAG(wand, FLAG_APPLIED))
            break;
    if (wand == NULL)
        return 0;
    if (!(random_roll(0, 3)))
    {
        new_draw_info_format(NDI_UNIQUE, 0, op, "The %s vibrates violently, then explodes!", query_name(wand));
        play_sound_map(op->map, op->x, op->y, SOUND_OB_EXPLODE, SOUND_NORMAL);
        destruct_ob(wand);
        return 1;
    }
    new_draw_info_format(NDI_UNIQUE, 0, op, "The %s glows with power.", query_name(wand));

    wand->stats.food += random_roll(1, spells[wand->stats.sp].charges);
    if (wand->arch && QUERY_FLAG(&wand->arch->clone, FLAG_ANIMATE))
    {
        SET_FLAG(wand, FLAG_ANIMATE);
        wand->speed = wand->arch->clone.speed;
        update_ob_speed(wand);
    }
    return 1;
}


/*  allows the choice of what sort of food object to make.
    If stringarg is NULL, it will create food dependent on level  --PeterM*/
int cast_create_food(object *op, object *caster, int dir, char *stringarg)
{
    return 0;
}


int cast_speedball(object *op, int dir, int type)
{
    object     *spb;
    mapstruct  *mt;
    int         xt, yt;

    xt = op->x + freearr_x[dir];
    yt = op->y + freearr_y[dir];
    if (!(mt = out_of_map(op->map, &xt, &yt)))
        return 0;

    spb = clone_arch(SPEEDBALL);
    if (blocked(spb, mt, xt, yt, spb->terrain_flag))
        return 0;

    spb->x = xt;
    spb->y = yt;
    spb->map = mt;
    spb->speed_left = -0.1f;
    if (type == SP_LARGE_SPEEDBALL)
        spb->stats.dam = 30;
    insert_ob_in_map(spb, mt, op, 0);
    return 1;
}

int probe(object *op)
{
    object *tmp;

    for (tmp = get_map_ob(op->map, op->x, op->y); tmp != NULL; tmp = tmp->above)
    {
        if (IS_LIVE(tmp))
        {
            if (op->owner && op->owner->type == PLAYER)
            {
#ifdef DEBUG_PROBE_IS_CHARM
                /* Temporarily made probe into charm to test pet code */
                if(add_pet(op->owner, tmp, 0) == 0)
                    new_draw_info_format(NDI_UNIQUE, 0, op->owner, "Your probe charms %s.", tmp->name);
#else
                new_draw_info_format(NDI_UNIQUE, 0, op->owner, "Your probe analyse %s.", tmp->name);
                if (tmp->head != NULL)
                    tmp = tmp->head;
                examine(op->owner, tmp, TRUE);
#endif
                return 1;
            }
        }
    }
    return 0;
}

int cast_invisible(object *op, object *caster, int spell_type)
{
    /* object *tmp; */

    /*
    if(op->invisible>1000) {
      new_draw_info(NDI_UNIQUE, 0,op,"You are already as invisible as you can get.");
      return 0;
    }
    */
    switch (spell_type)
    {
        case SP_INVIS:
          CLEAR_FLAG(op, FLAG_UNDEAD);
          break;
        case SP_INVIS_UNDEAD:
          SET_FLAG(op, FLAG_UNDEAD);
          break;
        case SP_IMPROVED_INVIS:
          break;
    }
    new_draw_info(NDI_UNIQUE, 0, op, "You can't see your hands!");
    update_object(op, UP_OBJ_FACE);

    /* Gecko: removed entirely. This is instead handled in mob AI core */
    /* Gecko: fixed to only go through active objects. Nasty loop anyway... */
    /* for (tmp = active_objects->active_next; tmp != NULL; tmp = tmp->active_next)
        if (tmp->enemy == op)
            register_npc_enemy(tmp, NULL);*/
    return 1;
}


int cast_earth2dust(object *op, object *caster)
{
    object     *tmp, *next;
    int         strength, i, j, xt, yt;
    mapstruct  *m;

    if (op->type != PLAYER)
        return 0;
    strength = spells[SP_EARTH_DUST].bdur + SP_level_strength_adjust(op, caster, SP_EARTH_DUST);
    strength = (strength > 15) ? 15 : strength;
    for (i = -strength; i < strength; i++)
        for (j = -strength; j < strength; j++)
        {
            xt = op->x + i;
            yt = op->y + j;
            if (!(m = out_of_map(op->map, &xt, &yt)))
                continue;
            for (tmp = get_map_ob(m, xt, yt); tmp != NULL; tmp = next)
            {
                next = tmp->above;
                if (tmp && QUERY_FLAG(tmp, FLAG_TEAR_DOWN))
                    damage_ob(tmp, 9998, op, ENV_ATTACK_CHECK);
            }
        }
    return 1;
}

/* puts a 'WORD_OF_RECALL_' object in player */
/* modified to work faster for higher level casters -- DAMN     */
int cast_wor(object *op, object *caster)
{
    object *dummy;

    if (op->type != PLAYER)
        return 0;
    if (blocks_magic(op->map, op->x, op->y))
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Something blocks your spell.");
        return 0;
    }
    dummy = get_archetype("force");
    if (dummy == NULL)
    {
        LOG(llevBug, "BUG: cast_wor(): get_object failed (%s - %s)!\n", query_name(op), query_name(caster));
        return 0;
    }
    if (op->owner)
        op = op->owner; /* better insert the spell in the player */
    dummy->speed = 0.002f * ((float) (spells[SP_WOR].bdur + SP_level_strength_adjust(op, caster, SP_WOR)));
    update_ob_speed(dummy);
    dummy->speed_left = -1;
    dummy->type = WORD_OF_RECALL;

    FREE_AND_ADD_REF_HASH(dummy->title, CONTR(op)->savebed_map);
    FREE_AND_ADD_REF_HASH(EXIT_PATH(dummy), CONTR(op)->orig_savebed_map);
    EXIT_STATUS(dummy) = CONTR(op)->bed_status;
    EXIT_X(dummy) = CONTR(op)->bed_x;
    EXIT_Y(dummy) = CONTR(op)->bed_y;

    (void) insert_ob_in_ob(dummy, op);
    new_draw_info(NDI_UNIQUE, 0, op, "You feel a force starting to build up inside you.");
    LOG(llevDebug, "Word of Recall for %s in %f ticks.\n", op->name,
        ((-dummy->speed_left) / (dummy->speed == 0 ? 0.0001 : dummy->speed)));
    LOG(llevDebug, "Word of Recall for player level %d, caster level %d: 0.002 * %d + %d\n", SK_level(op),
        SK_level(caster), spells[SP_WOR].bdur, SP_level_strength_adjust(op, caster, SP_WOR));
    return 1;
}

int cast_wow(object *op, int dir, int ability, SpellTypeFrom item)
{
    /*
      int sp;
      if(!random_roll(0, 3))
        return cast_cone(op,op,0,10,SP_WOW,spellarch[SP_WOW],SK_level(op),0);
      do
      {
        sp=random_roll(0, NROFREALSPELLS-1);
                   while(!spells[sp].active)
                   {
                       sp++;
                       if(sp>=NROFREALSPELLS)
                            sp=0;
                   }
      }
      while (!spells[sp].books);
      return cast_spell(op,op,dir,sp,ability,item,NULL);
    */
    return 0;
}

int perceive_self(object *op)
{
    char*cp = describe_item(op), buf[MAX_BUF];
    archetype              *at  = find_archetype("depletion");
    object                 *tmp;
    int                     i;

    tmp = find_god(determine_god(op));
    if (tmp)
        new_draw_info_format(NDI_UNIQUE, 0, op, "You worship %s", tmp->name);
    else
        new_draw_info(NDI_UNIQUE, 0, op, "You worship no god");

    tmp = present_arch_in_ob(at, op);

    if (*cp == '\0' && tmp == NULL)
        new_draw_info(NDI_UNIQUE, 0, op, "You feel very mundane");
    else
    {
        new_draw_info(NDI_UNIQUE, 0, op, "You have:");
        new_draw_info(NDI_UNIQUE, 0, op, cp);
        if (tmp != NULL)
        {
            for (i = 0; i < 7; i++)
            {
                if (get_attr_value(&tmp->stats, i) < 0)
                {
                    sprintf(buf, "Your %s is depleted by %d", statname[i], -(get_attr_value(&tmp->stats, i)));
                    new_draw_info(NDI_UNIQUE, 0, op, buf);
                }
            }
        }
    }
    return 1;
}

/* int cast_create_town_portal (object *op, object *caster, int dir)
 *
 * The town portal code was to much depending on the old map code.
 * Its ALOT easier to implement only "Recall Scrolls" (teleporting
 * back to a previous marked destination) or "Summon" (teleport
 * someone to you) as to play around with temporary portals
 * MT-2006
 */
int cast_create_town_portal(object *op, object *caster, int dir)
{
 return 0;
}

int cast_destruction(object *op, object *caster, int dam, int attacktype)
{
    int         i, j, r, xt, yt; /*  peterm:  added 'r' to make area of effect level dep.  */
    object     *tmp;
    mapstruct  *m;

    if (op->type != PLAYER)
        return 0;
    r = 5 + SP_level_strength_adjust(op, caster, SP_DESTRUCTION);
    dam += SP_level_dam_adjust(op, caster, SP_DESTRUCTION);
    for (i = -r; i < r; i++)
        for (j = -r; j < r; j++)
        {
            xt = op->x + i;
            yt = op->y + j;
            if (!(m = out_of_map(op->map, &xt, &yt)))
                continue;
            tmp = get_map_ob(m, xt, yt);
            while (tmp != NULL && (!QUERY_FLAG(tmp, FLAG_ALIVE) || tmp->type == PLAYER))
                tmp = tmp->above;
            if (tmp == NULL)
                continue;
            damage_ob(tmp, dam, op, ENV_ATTACK_CHECK);
        }
    return 1;
}

/* WARNING: DONT USE !!! First implement the out_of_map stuff below */
/* check all 3 blocked() here - they are not right set! */
int magic_wall(object *op, object *caster, int dir, int spell_type)
{
    object *tmp, *tmp2;
    int     i, posblocked = 0, negblocked = 0;
    /*
        mapstruct *mt;
        int xt,yt;

        xt=op->x+freearr_x[dir];
        yt=op->y+freearr_y[dir];
        if (!(mt=out_of_map (op->map, &nx, &ny)))
            return 0;
    */

    if (!dir)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "In what direction?");
        return 0;
    }
    if (blocked(NULL, op->map, op->x + freearr_x[dir], op->y + freearr_y[dir], op->terrain_flag))
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Something is in the way.");
        return 0;
    }
    switch (spell_type)
    {
        case SP_EARTH_WALL:
          tmp = get_archetype("earthwall");
          for (i = 0; i < NROFATTACKS; i++)
              tmp->resist[i] = 0;
          tmp->stats.hp = spells[spell_type].bdur + 10 * SP_level_strength_adjust(op, caster, spell_type);
          /* More solid, since they can be torn down */
          tmp->stats.maxhp = tmp->stats.hp;
          break;

        case SP_FIRE_WALL:
          tmp = get_archetype("firebreath");
          tmp->stats.hp = spells[spell_type].bdur + 5 * SP_level_strength_adjust(op, caster, spell_type);
          tmp->stats.dam = spells[spell_type].bdam + SP_level_dam_adjust(op, caster, spell_type);
          tmp->stats.food = 1;          /* so it doesn't propagate */
          SET_FLAG(tmp, FLAG_WALK_ON);
          SET_FLAG(tmp, FLAG_FLY_ON);
          set_owner(tmp, op);
          break;

        case SP_FROST_WALL:
          tmp = get_archetype("icestorm");
          tmp->stats.hp = spells[spell_type].bdur + 5 * SP_level_strength_adjust(op, caster, spell_type);
          tmp->stats.dam = spells[spell_type].bdam + SP_level_dam_adjust(op, caster, spell_type);

          tmp->stats.food = 1;      /* so it doesn't propagate */
          SET_FLAG(tmp, FLAG_WALK_ON);
          SET_FLAG(tmp, FLAG_FLY_ON);
          set_owner(tmp, op);
          break;

        case SP_WALL_OF_THORNS:
          tmp = get_archetype("thorns");
          tmp->stats.hp = spells[spell_type].bdur + 3 * SP_level_strength_adjust(op, caster, spell_type);
          tmp->stats.dam = spells[spell_type].bdam + SP_level_dam_adjust(op, caster, spell_type);
          SET_FLAG(tmp, FLAG_WALK_ON);
          set_owner(tmp, op);
          break;

        case SP_CHAOS_POOL:
          tmp = get_archetype("color_spray");
          tmp->stats.hp = spells[spell_type].bdur + 5 * SP_level_strength_adjust(op, caster, spell_type);
          tmp->stats.dam = spells[spell_type].bdam + SP_level_dam_adjust(op, caster, spell_type);

          tmp->stats.food = 1;  /* so the color spray object won't propagate */
          SET_FLAG(tmp, FLAG_WALK_ON);
          SET_FLAG(tmp, FLAG_FLY_ON);
          set_owner(tmp, op);
          break;

        case SP_DARKNESS:
          tmp = get_archetype("darkness");
          tmp->stats.food = spells[SP_DARKNESS].bdur + SP_level_strength_adjust(op, caster, SP_DARKNESS);
          break;

        case SP_COUNTERWALL:
          tmp = get_archetype("counterspell");
          tmp->stats.hp = spells[spell_type].bdur + 5 * SP_level_strength_adjust(op, caster, spell_type);
          tmp->stats.dam = spells[spell_type].bdam + SP_level_dam_adjust(op, caster, spell_type);
          tmp->stats.food = 1;
          tmp->level = SK_level(op);
          SET_FLAG(tmp, FLAG_WALK_ON);
          SET_FLAG(tmp, FLAG_FLY_ON);
          set_owner(tmp, op);
          break;

        default:
          LOG(llevBug, "BUG: Unimplemented magic_wall spell: %d\n", spell_type);
          return 0;
    }
    tmp->x = op->x + freearr_x[dir],tmp->y = op->y + freearr_y[dir];
    if ((tmp = insert_ob_in_map(tmp, op->map, op, 0)) == NULL)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Something destroys your wall");
        return 0;
    }

    /*  This code causes the wall to extend to a distance of 5 in
        each direction, or until an obstruction is encountered.
        posblocked and negblocked help determine how far the
        created wall can extend, it won't go extend through
        blocked spaces.  */

    for (i = 1; i < 5; i++)
    {
        int x, y, dir2;

        dir2 = (dir < 4) ? (dir + 2) : dir - 2;

        x = tmp->x + i * freearr_x[dir2]; y = tmp->y + i * freearr_y[dir2];
        if (!blocked(NULL, op->map, x, y, op->terrain_flag) && !posblocked)
        {
            tmp2 = get_object();
            copy_object(tmp, tmp2);
            tmp2->x = x; tmp2->y = y;
            if (!insert_ob_in_map(tmp2, op->map, op, 0))
                continue;
        }
        else
            posblocked = 1;
        x = tmp->x - i * freearr_x[dir2]; y = tmp->y - i * freearr_y[dir2];
        if (!blocked(NULL, op->map, x, y, op->terrain_flag) && !negblocked)
        {
            tmp2 = get_object();
            copy_object(tmp, tmp2);
            tmp2->x = x; tmp2->y = y;
            if (!insert_ob_in_map(tmp2, op->map, op, 0))
                continue;
        }
        else
            negblocked = 1;
    }

    /*
       if(op->type==PLAYER)
    draw_client_map(op);
       else
    */
    if (!op->type == PLAYER)
        SET_FLAG(op, FLAG_SCARED); /* We don't want them to walk through the wall! */
    return 1;
}

/* cast_light() - I wanted this to be able to blind opponents who stand
 * adjacent to the caster, so I couldnt use magic_wall(). -b.t.
 */
/* badly outdated of course - MT 2004*/
int cast_light(object *op, object *caster, int dir)
{
    object*target = NULL, *tmp = NULL;
    mapstruct      *m;
    int             x, y, dam = spells[SP_LIGHT].bdam + SP_level_dam_adjust(op, caster, SP_LIGHT);

    if (!dir)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "In what direction?");
        return 0;
    }

    x = op->x + freearr_x[dir],y = op->y + freearr_y[dir];

    if ((m = out_of_map(op->map, &x, &y)))
    {
        for (target = get_map_ob(m, x, y); target; target = target->above)
            if (QUERY_FLAG(target, FLAG_MONSTER))
            {
                /* coky doky. got a target monster. Lets make a blinding attack */
                if (target->head)
                    target = target->head;
                damage_ob(target, dam, op, ENV_ATTACK_CHECK);
                return 1; /* one success only! */
            }
    }

    /* ok, looks groovy to just insert a new light on the map */
    tmp = get_archetype("light");
    if (!tmp)
    {
        LOG(llevBug, "BUG: spell arch for cast_light() missing (%s).\n", query_name(caster));
        return 0;
    }

    /* no live target, perhaps a wall is in the way? */
    if (blocked(tmp, m, x, y, tmp->terrain_flag))
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Something is in the way.");
        return 0;
    }
    tmp->speed = 0.000001f * (float) (spells[SP_LIGHT].bdur - (10 * SP_level_strength_adjust(op, caster, SP_LIGHT)));
    if (tmp->speed < MIN_ACTIVE_SPEED)
        tmp->speed = MIN_ACTIVE_SPEED;
    tmp->x = x,tmp->y = y;
    insert_ob_in_map(tmp, m, op, 0);

    /*if(op->type==PLAYER) draw_client_map(op);*/
    return 1;
}

/* WARNING!! DON'T USE ! no out_of_map() here
 * I also want include better position checks here
 */
int dimension_door(object *op, int dir)
{
    uint32  dist;

    if (op->type != PLAYER)
        return 0;

    if (!dir)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "In what direction?");
        return 0;
    }

    if (CONTR(op)->count)
    {
        for (dist = 0; dist < CONTR(op)->count; dist++)
            if (blocks_magic(op->map, op->x + freearr_x[dir] * (dist + 1), op->y + freearr_y[dir] * (dist + 1)))
                break;

        if (dist < CONTR(op)->count)
        {
            new_draw_info(NDI_UNIQUE, 0, op, "Something blocks your magic.\n");
            CONTR(op)->count = 0;
            return 0;
        }
        CONTR(op)->count = 0;

        /* If the player is trying to dimension door to solid rock, choose
         * a random place on the map to put the player.
         * Changed in 0.94.3 so that the player can not get put in
         * a no magic spot.
         */
        if (blocked(op, op->map, op->x + freearr_x[dir] * dist, op->y + freearr_y[dir] * dist, op->terrain_flag))
        {
            int x = random_roll(0, MAP_WIDTH(op->  map) - 1),y = random_roll(0, MAP_HEIGHT(op->map) - 1);

            if (blocked(op, op->map, x, y, op->terrain_flag) || blocks_magic(op->map, x, y))
            {
                new_draw_info(NDI_UNIQUE, 0, op, "You cast your spell, but nothing happens.\n");
                return 1; /* Maybe the penalty should be more severe... */
            }

            remove_ob(op);
            op->x = x,op->y = y;
            if (insert_ob_in_map(op, op->map, op, 0))
                return 1;
        }
    }
    else
    {
        /* Player didn't specify a distance, so lets see how far
        * we can move the player.
        */
        for (dist = 0;
             !blocks_view(op->map, op->x + freearr_x[dir] * (dist + 1), op->y + freearr_y[dir] * (dist + 1))
          && !blocks_magic(op->map,
                           op->x
                         + freearr_x[dir] * (dist + 1),
                           op->y
                         + freearr_y[dir] * (dist + 1));
             dist++)
            ;

        /* If the destinate is blocked, keep backing up until we
         * find a place for the player.
         */
        for (;
             dist > 0 && blocked(op, op->map, op->x + freearr_x[dir] * dist, op->y + freearr_y[dir] * dist,
                                 op->terrain_flag);
             dist--)
            ;

        if (!dist)
        {
            new_draw_info(NDI_UNIQUE, 0, op, "Your spell failed!\n");
            return 0;
        }
    }

    /* Actually move the player now */
    remove_ob(op);
    op->x += freearr_x[dir] * dist,op->y += freearr_y[dir] * dist;
    if ((op = insert_ob_in_map(op, op->map, op, 0)) == NULL)
        return 1;

    op->speed_left = -FABS(op->speed) * 5; /* Freeze them for a short while */
    return 1;
}

int cast_heal(object *op, int level, object *target, int spell_type)
{
    archetype  *at;
    object     *temp;
    int         heal = 0, success = 0;

    /*LOG(-1,"dir: %d (%s -> %s)\n", dir, op?op->name:"<no op>",tmp?tmp->name:"<no tmp>");*/

    if (!op || !target)
    {
        LOG(llevBug, "BUG: cast_heal(): target or caster NULL (op: %s target: %s)\n", query_name(op), query_name(target));
        return 0;
    }

    switch (spell_type)
    {
        case SP_CURE_DISEASE:
          if (cure_disease(target, op))
              success = 1;
          break;

        case SP_CURE_POISON:
          at = find_archetype("poisoning");

          if (op != target && target->type == PLAYER)
              new_draw_info_format(NDI_UNIQUE, 0, target, "%s casts cure poison on you!",
                                   op->name ? op->name : "someone");
          if (op != target && op->type == PLAYER)
              new_draw_info_format(NDI_UNIQUE, 0, op, "You casts cure poison on %s!",
                                   target->name ? target->name : "someone");

          for (temp = target->inv; temp != NULL; temp = temp->below)
          {
              if (temp->arch == at)
              {
                  success = 1;
                  temp->stats.food = 1;
              }
          }

          if (success)
          {
              if (target->type == PLAYER)
                  new_draw_info(NDI_UNIQUE, 0, target, "Your body feels cleansed.");
              if (op != target && op->type == PLAYER)
                  new_draw_info_format(NDI_UNIQUE, 0, op, "%s body seems cleansed.",
                                       target->name ? target->name : "someone");
          }
          else
          {
              if (target->type == PLAYER)
                  new_draw_info(NDI_UNIQUE, 0, target, "Your are not poisoned.");
              if (op != target && op->type == PLAYER)
                  new_draw_info_format(NDI_UNIQUE, 0, op, "%s is not poisoned.", target->name ? target->name : "someone");
          }
          break;

        case SP_CURE_CONFUSION:
          at = find_archetype("confusion");

          if (op != target && target->type == PLAYER)
              new_draw_info_format(NDI_UNIQUE, 0, target, "%s casts cure confusion on you!",
                                   op->name ? op->name : "someone");
          if (op != target && op->type == PLAYER)
              new_draw_info_format(NDI_UNIQUE, 0, op, "You casts cure confusion on %s!",
                                   target->name ? target->name : "someone");

          for (temp = target->inv; temp != NULL; temp = temp->below)
          {
              if (temp->arch == at)
              {
                  success = 1;
                  temp->stats.food = 1;
              }
          }

          if (success)
          {
              if (target->type == PLAYER)
                  new_draw_info(NDI_UNIQUE, 0, target, "Your mind feels clearer.");
              if (op != target && op->type == PLAYER)
                  new_draw_info_format(NDI_UNIQUE, 0, op, "%s mind seems clearer.",
                                       target->name ? target->name : "someone");
          }
          else
          {
              if (target->type == PLAYER)
                  new_draw_info(NDI_UNIQUE, 0, target, "Your are not confused.");
              if (op != target && op->type == PLAYER)
                  new_draw_info_format(NDI_UNIQUE, 0, op, "%s is not confused.", target->name ? target->name : "someone");
          }
          break;

        case SP_CURE_BLINDNESS:
          at = find_archetype("blindness");

          if (op != target && target->type == PLAYER)
              new_draw_info_format(NDI_UNIQUE, 0, target, "%s casts cure blindness on you!",
                                   op->name ? op->name : "someone");
          if (op != target && op->type == PLAYER)
              new_draw_info_format(NDI_UNIQUE, 0, op, "You casts cure blindness on %s!",
                                   target->name ? target->name : "someone");

          for (temp = target->inv; temp != NULL; temp = temp->below)
          {
              if (temp->arch == at)
              {
                  success = 1;
                  temp->stats.food = 1;
              }
          }

          if (success)
          {
              if (target->type == PLAYER)
                  new_draw_info(NDI_UNIQUE, 0, target, "Your vision begins to return.");
              if (op != target && op->type == PLAYER)
                  new_draw_info_format(NDI_UNIQUE, 0, op, "%s vision seems to return.",
                                       target->name ? target->name : "someone");
          }
          else
          {
              if (target->type == PLAYER)
                  new_draw_info(NDI_UNIQUE, 0, target, "Your are not blinded.");
              if (op != target && op->type == PLAYER)
                  new_draw_info_format(NDI_UNIQUE, 0, op, "%s is not blinded.", target->name ? target->name : "someone");
          }
          break;

        case SP_MINOR_HEAL:
          success = 1;
          heal = random_roll(2, 5 + level) + 6;
          if (op->type == PLAYER)
          {
              if (heal > 0)
                  new_draw_info_format(NDI_UNIQUE, 0, op, "The prayer heals %s for %d hp!",
                                       op == target ? "you" : query_name(target), heal);
              else
                  new_draw_info(NDI_UNIQUE, 0, op, "The healing prayer fails!");
          }

          if (op != target && target->type == PLAYER)
          {
              if (heal > 0)
                  new_draw_info_format(NDI_UNIQUE, 0, target, "%s casts minor healing on you healing %d hp!", op->name,
                                       heal);
              else
                  new_draw_info_format(NDI_UNIQUE, 0, target, "%s casts minor healing on you but it fails!", op->name);
          }
          break;
          /*
            case SP_MED_HEAL:
              heal=random_roll_roll(3, 6)+4;
              new_draw_info(NDI_UNIQUE, 0,tmp, "Your wounds start to fade.");
              break;
            case SP_MAJOR_HEAL:
              new_draw_info(NDI_UNIQUE, 0,tmp, "Your skin looks as good as new!");
              heal=random_roll_roll(4, 8)+8;
              break;
            case SP_HEAL:
              heal=tmp->stats.maxhp;
              new_draw_info(NDI_UNIQUE, 0,tmp, "You feel just fine!");
              break;
            case SP_CURE_CONFUSION:
              at=find_archetype("confusion");
              poison=present_arch_in_ob(at,tmp);
              if (poison) {
                success = 1;
                new_draw_info(NDI_UNIQUE, 0,tmp, "Your mind feels clearer");
                poison->stats.food = 1;
              }
              break;
            case SP_RESTORATION:
              if (cast_heal (op, op, SP_CURE_POISON))
                success = 1;
              if (cast_heal (op, op, SP_CURE_CONFUSION))
                success = 1;
              if (cast_heal (op, op, SP_CURE_DISEASE))
                success = 1;
              if (tmp->stats.food < 999) {
                success = 1;
                tmp->stats.food=999;
              }

              if (cast_heal (op, op, SP_HEAL))
                  success = 1;
              return success;
            */
    }

    if (heal > 0)
    {
        if (reduce_symptoms(target, heal))
            success = 1;
        if (target->stats.hp < target->stats.maxhp)
        {
            success = 1;
            target->stats.hp += heal;
            if (target->stats.hp > target->stats.maxhp)
                target->stats.hp = target->stats.maxhp;
        }
    /* although last_damage is unsigned, wo put an negative value in it, in draw_client_map2 we convert back to signed int */
        if (target->damage_round_tag != ROUND_TAG)
        {
            target->last_damage = 0;
            target->damage_round_tag = ROUND_TAG;
        }
        target->last_damage -= heal;
    }

    if (success)
    {
        op->speed_left = -FABS(op->speed) * 3;
    }

    if (insert_spell_effect(spells[spell_type].archname, target->map, target->x, target->y))
        LOG(llevDebug, "insert_spell_effect() failed: spell:%d, obj:%s target:%s\n", spell_type, query_name(op),
            query_name(target));
    return success;
}

int cast_regenerate_spellpoints(object *op)
{
    object *tmp = op;


    if (tmp == NULL)
        return 0;

    tmp->stats.sp = tmp->stats.maxsp;
    new_draw_info(NDI_UNIQUE, 0, tmp, "Magical energies surge through your body!");
    return 1;
}

int cast_change_attr(object *op, object *caster, object *target, int dir, int spell_type)
{
    object *tmp     = target;
    object *tmp2    = NULL;
    object *force   = NULL;
    int     is_refresh = 0, msg_flag = 1;
    int     atnr = 0, path = 0;        /* see protection spells */
    int     i;

    if (tmp == NULL)
        return 0;

    /* we ID the buff force with spell_type... if we find one, we have old effect.
     * if not, we create a fresh force.
     */
    for (tmp2 = tmp->inv; tmp2 != NULL; tmp2 = tmp2->below)
    {
        if (tmp2->type == FORCE)
        {
            if (tmp2->weight_limit == spell_type)
            {
                force = tmp2;    /* the old effect will be "refreshed" */
                is_refresh = 1;
                new_draw_info(NDI_UNIQUE, 0, op, "You recast the spell while in effect.");
            }
            else if ((spell_type == SP_BLESS && tmp2->weight_limit == SP_HOLY_POSSESSION)
                  || (spell_type == SP_HOLY_POSSESSION && tmp2->weight_limit == SP_BLESS))
            {
                /* both bless AND holy posession are not allowed! */
                new_draw_info(NDI_UNIQUE, 0, op, "No more blessings for you.");
                return 0;
            }
        }
    }
    if (force == NULL)
        force = get_archetype("force");
    force->weight_limit = spell_type;  /* mark this force with the originating spell */


    i = 0;   /* (-> protection spells) */
    switch (spell_type)
    {
        case SP_STRENGTH:
          force->speed_left = -1;
          if (tmp->type != PLAYER)
          {
              if (op->type == PLAYER)
                  new_draw_info(NDI_UNIQUE, 0, op, "You can't cast this kind of spell on your target.");
              return 0;
          }
          else if (op->type == PLAYER && op != tmp)
              new_draw_info_format(NDI_UNIQUE, 0, tmp, "%s casts strength on you!", op->name ? op->name : "someone");


          if (force->stats.Str < 2)
          {
              force->stats.Str++;
              if (op->type == PLAYER && op != tmp)
                  new_draw_info_format(NDI_UNIQUE, 0, op, "%s get stronger.", tmp->name ? tmp->name : "someone");
          }
          else
          {
              msg_flag = 0;
              new_draw_info(NDI_UNIQUE, 0, tmp, "You don't grow stronger but the spell is refreshed.");
              if (op->type == PLAYER && op != tmp)
                  new_draw_info_format(NDI_UNIQUE, 0, op, "%s don't grow stronger but the spell is refreshed.",
                                       tmp->name ? tmp->name : "someone");
          }

          if (insert_spell_effect(spells[SP_STRENGTH].archname, target->map, target->x, target->y))
              LOG(llevDebug, "insert_spell_effect() failed: spell:%d, obj:%s caster:%s target:%s\n", spell_type,
                  query_name(op), query_name(caster), query_name(target));

          break;


          /*---------- old -------------- */

        case SP_RAGE:
          {
              if (tmp->type != PLAYER)
                  break;
              /* Str, Dex, Con */
              cast_change_attr(op, caster, target, dir, SP_HEROISM);
              /* haste */
              cast_change_attr(op, caster, target, dir, SP_HASTE);
              /* armour */
              cast_change_attr(op, caster, target, dir, SP_ARMOUR);
              /* regeneration */
              cast_change_attr(op, caster, target, dir, SP_REGENERATION);
              /* weapon class */
              /* ADD POSITIVE WC ADD HERE */
              force->stats.wc += SP_level_dam_adjust(op, caster, SP_BLESS);

              break;
          }
        case SP_DEXTERITY:
          if (tmp->type != PLAYER)
              break;
          if (!(random_roll(0, (MAX(1, (10 - 30 + tmp->stats.Dex))) - 1)))
          {
              for (i = 20,force->stats.Dex = 1; i > tmp->stats.Dex; i -= 2)
                  force->stats.Dex++;
          }
          else
          {
              new_draw_info(NDI_UNIQUE, 0, op, "You grow no more agile."); force->stats.Dex = 0;
          }

          break;
        case SP_CONSTITUTION:
          if (tmp->type != PLAYER)
              break;
          if (!(random_roll(0, (MAX(1, (10 - 30 + tmp->stats.Con))) - 1)))
          {
              for (i = 20,force->stats.Con = 1; i > tmp->stats.Con; i -= 2)
                  force->stats.Con++;
          }
          else
          {
              new_draw_info(NDI_UNIQUE, 0, op, "You don't feel any healthier."); force->stats.Con = 0;
          }

          break;
        case SP_CHARISMA:
          if (tmp->type != PLAYER)
              break;
          if (!(random_roll(0, (MAX(1, (10 - 30 + tmp->stats.Cha))) - 1)))
          {
              for (i = 20,force->stats.Cha = 1; i > tmp->stats.Cha; i -= 2)
                  force->stats.Cha++;
          }
          else
          {
              new_draw_info(NDI_UNIQUE, 0, op, "You are no easier to look at."); force->stats.Cha = 0;
          }

          break;
        case SP_IRONWOOD_SKIN:
        case SP_ARMOUR:
          {
              /* armour MAY be used multiple times. */
              force->weight_limit = 0;
              /* With PR code, I wonder if this could get merged in with the other protection spells */
              /* peterm, modified so that it uses level-depend functions */
              /* change this to POSITIVE AC!! */
              force->stats.ac = 2 + SP_level_dam_adjust(op, caster, spell_type);
              if ((tmp->stats.ac - force->stats.ac) < -20)
                  force->stats.ac = tmp->stats.ac + 20;

              force->resist[ATNR_PHYSICAL] = 5 + 4 * SP_level_dam_adjust(op, caster, spell_type);
              if (force->resist[ATNR_PHYSICAL] > 25)
                  force->resist[ATNR_PHYSICAL] = 25;
              if (tmp->resist[ATNR_PHYSICAL] > 70
               && force->resist[ATNR_PHYSICAL] > (100 - tmp->resist[ATNR_PHYSICAL]) / 3)
                  force->resist[ATNR_PHYSICAL] = 3;  /* diminishing returns at high armor. */
              new_draw_info(NDI_UNIQUE, 0, tmp, "A force shimmers around you.");
              break;
          }
        case SP_CONFUSION:
          force->resist[ATNR_CONFUSION] = 50;   /*??? It was here before PR */
          break;
        case SP_HEROISM:
          if (tmp->type != PLAYER)
              break;
          cast_change_attr(op, caster, target, dir, SP_STRENGTH);
          cast_change_attr(op, caster, target, dir, SP_DEXTERITY);
          cast_change_attr(op, caster, target, dir, SP_CONSTITUTION);
          break;
        case SP_HOLY_POSSESSION:
          {
              object   *god = find_god(determine_god(op));
              int       i;

              if (god)
              {
                  if (god->slaying)
                      FREE_AND_COPY_HASH(force->slaying, god->slaying);

                  /* Only give out good benefits, not bad */
                  for (i = 0; i < NROFATTACKS; i++)
                  {
                      if (god->resist[i] > 0)
                      {
                          force->resist[i] = god->resist[i];
                          if (force->resist[i] > 95)
                              force->resist[i] = 95;
                      }
                      else
                          force->resist[i] = 0; /* adding of diff. types not allowed */
                  }

                  force->path_attuned |= god->path_attuned;
                  new_draw_info_format(NDI_UNIQUE, 0, tmp, "You are possessed by the essence of %s!", god->name);
              }
              else
                  new_draw_info(NDI_UNIQUE, 0, op, "Your blessing seems empty.");
              if (tmp != op && op->type == PLAYER && tmp->type == PLAYER)
              {
                  new_draw_info_format(NDI_UNIQUE, 0, op, "You bless %s mightily!", tmp->name);
                  new_draw_info_format(NDI_UNIQUE, 0, tmp, "%s blessed you mightily!", op->name);
              }
              /* ADD POSITIVE WC ADD HERE */
              force->stats.wc += SP_level_dam_adjust(op, caster, SP_HOLY_POSSESSION);
              force->stats.ac += SP_level_dam_adjust(op, caster, SP_HOLY_POSSESSION);
              break;
          }
        case SP_REGENERATION:
          force->stats.hp = 1 + SP_level_dam_adjust(op, caster, SP_REGENERATION);
          break;
        case SP_CURSE:
          {
              object   *god = find_god(determine_god(op));
              if (god)
              {
                  force->path_repelled |= god->path_repelled;
                  force->path_denied |= god->path_denied;
                  new_draw_info_format(NDI_UNIQUE, 0, tmp, "You are a victim of %s's curse!", god->name);
              }
              else
                  new_draw_info(NDI_UNIQUE, 0, op, "Your curse seems empty.");

              if (tmp != op && caster->type == PLAYER)
                  new_draw_info_format(NDI_UNIQUE, 0, caster, "You curse %s!", tmp->name);
              /* change this to negative ! */
              /* ADD POSITIVE WC ADD HERE */
              force->stats.ac -= SP_level_dam_adjust(op, caster, SP_CURSE);
              force->stats.wc -= SP_level_dam_adjust(op, caster, SP_CURSE);
              break;
          }
        case SP_BLESS:
          {
              object   *god = find_god(determine_god(op));
              if (god)
              {
                  int   i;

                  /* Only give out good benefits, and put a max on it */
                  for (i = 0; i < NROFATTACKS; i++)
                  {
                      if (god->resist[i] > 0)
                      {
                          force->resist[i] = god->resist[i];
                          if (force->resist[i] > 30)
                              force->resist[i] = 30;
                      }
                      else
                          force->resist[i] = 0; /* adding of diff. types not allowed */
                  }
                  force->path_attuned |= god->path_attuned;
                  new_draw_info_format(NDI_UNIQUE, 0, tmp, "You receive the blessing of %s.", god->name);
              }
              else
                  new_draw_info(NDI_UNIQUE, 0, op, "Your blessing seems empty.");

              if (tmp != op && op->type == PLAYER && tmp->type == PLAYER)
              {
                  new_draw_info_format(NDI_UNIQUE, 0, op, "You bless %s.", tmp->name);
                  new_draw_info_format(NDI_UNIQUE, 0, tmp, "%s blessed you.", op->name);
              }
              /* ADD POSITIVE WC ADD HERE */
              force->stats.wc += SP_level_dam_adjust(op, caster, SP_BLESS);
              force->stats.ac += SP_level_dam_adjust(op, caster, SP_BLESS);
              break;
          }
        case SP_DARK_VISION:
          SET_FLAG(force, FLAG_SEE_IN_DARK);
          break;
          /* attacktype-protection spells: */
          /*
        case SP_PROT_COLD:
          if (!i)
              atnr = ATNR_COLD, path = PATH_FROST, i = 1;
        case SP_PROT_FIRE:
          if (!i)
              atnr = ATNR_FIRE, path = PATH_FIRE, i = 1;
        case SP_PROT_ELEC:
          if (!i)
              atnr = ATNR_ELECTRICITY, path = PATH_ELEC, i = 1;
        case SP_PROT_POISON:
          if (!i)
              atnr = ATNR_POISON, i = 1;
        case SP_PROT_SLOW:
          if (!i)
              atnr = ATNR_SLOW, i = 1;
        case SP_PROT_PARALYZE:
          if (!i)
              atnr = ATNR_PARALYZE, path = PATH_MIND, i = 1;
        case SP_PROT_DRAIN:
          if (!i)
              atnr = ATNR_DRAIN, path = PATH_DEATH, i = 1;
        case SP_PROT_ATTACK:
          if (!i)
              atnr = ATNR_PHYSICAL, path = PATH_PROT, i = 1;
        case SP_PROT_MAGIC:
          if (!i)
              atnr = ATNR_MAGIC, i = 1;
        case SP_PROT_CONFUSE:
          if (!i)
              atnr = ATNR_CONFUSION, path = PATH_MIND, i = 1;
        case SP_PROT_CANCEL:
          if (!i)
              atnr = ATNR_CANCELLATION, i = 1;
        case SP_PROT_DEPLETE:
          if (!i)
              atnr = ATNR_DEPLETE, path = PATH_DEATH, i = 1;
*/
          /* The amount of prot. granted depends on caster's skill-level and
           * on attunement to spellpath, if there is a related one: */
          force->resist[atnr] = (20
                               + 30 * SK_level(caster) / 100
                               + ((caster->path_attuned & path) ? 10 : 0)
                               - ((caster->path_repelled & path) ? 10 : 0))
                              / ((caster->path_denied & path) ? 2 : 1);
          break;
        case SP_LEVITATE:
          SET_MULTI_FLAG(force, FLAG_LEVITATE);
          break;
          /* The following immunity spells are obsolete... -AV */
        case SP_IMMUNE_COLD:
          force->resist[ATNR_COLD] = 100;
          break;
        case SP_IMMUNE_FIRE:
          force->resist[ATNR_FIRE] = 100;
          break;
        case SP_IMMUNE_ELEC:
          force->resist[ATNR_ELECTRICITY] = 100;
          break;
        case SP_IMMUNE_POISON:
          force->resist[ATNR_POISON] = 100;
          break;
        case SP_IMMUNE_SLOW:
          force->resist[ATNR_SLOW] = 100;
          break;
        case SP_IMMUNE_PARALYZE:
          force->resist[ATNR_PARALYZE] = 100;
          break;
        case SP_IMMUNE_DRAIN:
          force->resist[ATNR_DRAIN] = 100;
          break;
        case SP_IMMUNE_ATTACK:
          force->resist[ATNR_PHYSICAL] = 100;
          break;
        case SP_IMMUNE_MAGIC:
          force->resist[ATNR_MAGIC] = 100;
          break;

        case SP_INVULNERABILITY:
        case SP_PROTECTION:
          /* Don't give them everything, so can't do a simple loop.
           * Added holyword & blind with PR's - they seemed to be
           * misising before.
           * Note: These Spells shouldn't be used. Especially not on players! -AV
           */
          if (spell_type == SP_INVULNERABILITY)
              i = 100;
          else
              i = 30;
          force->resist[ATNR_PHYSICAL] = i;
          force->resist[ATNR_FIRE] = i;
          force->resist[ATNR_ELECTRICITY] = i;
          force->resist[ATNR_COLD] = i;
          force->resist[ATNR_CONFUSION] = i;
          force->resist[ATNR_ACID] = i;
          force->resist[ATNR_DRAIN] = i;
          force->resist[ATNR_POISON] = i;
          force->resist[ATNR_SLOW] = i;
          force->resist[ATNR_PARALYZE] = i;
          force->resist[ATNR_FEAR] = i;
          force->resist[ATNR_DEPLETION] = i;
          force->resist[ATNR_DEATH] = i;


        case SP_HASTE:
          force->stats.exp = (3 + SP_level_dam_adjust(op, caster, SP_HASTE));
          if (op->speed > 0.2 * SP_level_strength_adjust(op, caster, SP_HASTE))
              force->stats.exp = 0;
          break;
        case SP_XRAY:
          SET_FLAG(force, FLAG_XRAYS);
          break;
    }
    force->speed_left = -1 - SP_level_strength_adjust(op, caster, spell_type) * 0.1f;

    if (!is_refresh)
    {
        SET_FLAG(force, FLAG_APPLIED);
        force = insert_ob_in_ob(force, tmp);
    }
    if (msg_flag)
    {
        if(tmp->type == PLAYER)
            change_abil(tmp, force, 0); /* Mostly to display any messages */
        else
            FIX_PLAYER(tmp ,"cast change attr - bug? bogus call - fix monster?"); /* fix monster? */
    }

    return 1;
}


#define MAX_PET_MONSTERS 5
char    mage_pet_monsters[MAX_PET_MONSTERS][16]     =
{
    "bat", "spider", "stalker", "beholder", "dark_elf"
};
int     mage_num_called[MAX_PET_MONSTERS]           =
{
    2, 1, 1, 2, 3
};
char    priest_pet_monsters[MAX_PET_MONSTERS][16]   =
{
    "bee", "killer_bee", "devil", "angel", "panther"
};
int     priest_num_called[MAX_PET_MONSTERS]         =
{
    3, 2, 2, 2, 5
};
char    altern_pet_monsters[MAX_PET_MONSTERS][16]   =
{
    "bird", "pixie", "skeleton", "skull", "vampire"
};
int     altern_num_called[MAX_PET_MONSTERS]         =
{
    1, 1, 2, 1, 1
};

/* this pet monster stuff is total crap!
** We should replace it with:
struct summoned_mon int foo {
        char * mon_arch;
        int  num_summoned;
        }
struct summoned_mon pets_summoned = {
       { "bird", 5 },
       { "vampire", 6},
       { NULL, 0 }     -* terminator *-
}
**
*/


int summon_pet(object *op, int dir, SpellTypeFrom item)
{
    LOG(llevBug, "BUG: unimplemented summon pet spell used\n");
    return 0;
    /* TODO: update for the new AI and pet systems. Gecko 2006-04-30 */
#if 0


    int         level, number, i;
    char       *monster;
    archetype  *at;

    /*
      level = ((op->head?op->head->level:op->level) / 4);
    */
    level = ((op->head ? op->head->level : SK_level(op)) / 4);
    if (level >= MAX_PET_MONSTERS)
        level = MAX_PET_MONSTERS - 1;
    switch (random_roll(0, 2))
    {
        case 0:
          number = priest_num_called[level];
          monster = priest_pet_monsters[level];
          break;
        case 1:
          number = mage_num_called[level];
          monster = mage_pet_monsters[level];
          break;
        default:
          number = altern_num_called[level];
          monster = altern_pet_monsters[level];
          break;
    }
    at = find_archetype(monster);
    if (at == NULL)
    {
        LOG(llevBug, "BUG: Unknown archetype in summon pet: %s (%s)\n", monster, query_name(op));
        return 0;
    }
    if (!dir)
        dir = find_free_spot(at, op->map, op->x, op->y, 1, SIZEOFFREE);
    /* careful here - we give clone as settings for terrain to arch_blocked() */
    if ((dir == -1) || arch_blocked(at, &at->clone, op->map, op->x + freearr_x[dir], op->y + freearr_y[dir]))
    {
        new_draw_info(NDI_UNIQUE, 0, op, "There is something in the way.");
        return 0;
    }
    if (item != spellNormal)
        /* op->stats.sp -= 5 + 10*level + op->level; */
        op->stats.sp -= 5 + 10 * level + SK_level(op);
    for (i = 1; i < number + 1; i++)
    {
        archetype      *atmp;
        object*prev =   NULL, *head = NULL; /* We want to summon dragons *grin* */

        for (atmp = at; atmp != NULL; atmp = atmp->more)
        {
            object *tmp;
            tmp = arch_to_object(atmp);
            if (atmp == at)
            {
                set_owner(tmp, op);
                SET_FLAG(tmp, FLAG_MONSTER);
                if (op->type == PLAYER)
                {
                    tmp->stats.exp = 0;
                    SET_FLAG(tmp, FLAG_FRIENDLY);
                    tmp->move_type = PETMOVE;
                }
                else if (QUERY_FLAG(op, FLAG_FRIENDLY))
                {
                    object *owner   = get_owner(op);
                    if (owner != NULL)
                    {
                        set_owner(tmp, owner);
                        tmp->move_type = PETMOVE;
                        SET_FLAG(tmp, FLAG_FRIENDLY);
                    }
                }
                tmp->speed_left = -1;
                update_npc_knowledge(tmp, op->enemy, FRIENDSHIP_ATTACK, 0);
                tmp->type = 0;
            }
            if (head == NULL)
                head = tmp;
            tmp->x = op->x + freearr_x[dir] + tmp->arch->clone.x;
            tmp->y = op->y + freearr_y[dir] + tmp->arch->clone.y;
            tmp->map = op->map;
            if (head != tmp)
                tmp->head = head, prev->more = tmp;
            prev = tmp;
        }
        head->direction = dir;
        /* Some monsters are sleeping by default - clear that */
        CLEAR_FLAG(head, FLAG_SLEEP);
        head = insert_ob_in_map(head, op->map, op, 0);
        if (head != NULL && head->randomitems != NULL)
        {
            object *tmp;
            create_treasure_list(head->randomitems, head, GT_APPLY, head->level, ART_CHANCE_UNSET, 0);
            for (tmp = head->inv; tmp != NULL; tmp = tmp->below)
                if (!tmp->nrof)
                    SET_FLAG(tmp, FLAG_NO_DROP);
        }
        dir = absdir(dir + 1);
        if (arch_blocked(at, &at->clone, op->map, op->x + freearr_x[dir], op->y + freearr_y[dir]))
        {
            if (i < number)
            {
                new_draw_info(NDI_UNIQUE, 0, op, "There is something in the way,");
                new_draw_info(NDI_UNIQUE, 0, op, "no more pets for this casting.");
                if (item != spellNormal)
                {
                    /* op->stats.sp += (5 + 12 * level + op->level) / (number - i); */
                    op->stats.sp += (5 + 12 * level + SK_level(op)) / (number - i);
                    if (op->stats.sp < 0)
                        op->stats.sp = 0;
                }
                return 1;
            }
        }
    }
    if (item != spellNormal && op->stats.sp < 0)
        op->stats.sp = 0;
    return 1;
#endif
}

int create_bomb(object *op, object *caster, int dir, int spell_type, char *name)
{
    object         *tmp;
    int dx = op->x +freearr_x[dir], dy = op->y + freearr_y[dir];
    if (wall(op->map, dx, dy))
    {
        new_draw_info(NDI_UNIQUE, 0, op, "There is something in the way.");
        return 0;
    }
    tmp = get_archetype(name);

    /*  level dependencies for bomb  */
    tmp->stats.dam = spells[spell_type].bdam + SP_level_dam_adjust(op, caster, spell_type);
    tmp->stats.hp = spells[spell_type].bdur + SP_level_strength_adjust(op, caster, spell_type);
    tmp->level = casting_level(caster, spell_type);

    set_owner(tmp, op);
    tmp->x = dx,tmp->y = dy;
    insert_ob_in_map(tmp, op->map, op, 0);
    return 1;
}

void animate_bomb(object *op)
{
    int         i;
    object     *env;
    archetype  *at;

    if (op->state != NUM_ANIMATIONS(op) - 1)
        return;
    at = find_archetype("splint");
    for (env = op; env->env != NULL; env = env->env)
        ;

    if (op->env)
    {
        if (env->map == NULL)
            return;

        if (env->type == PLAYER)
            esrv_del_item(CONTR(env), op->count, op->env);

        destruct_ob(op);
        op->x = env->x;
        op->y = env->y;
        if ((op = insert_ob_in_map(op, env->map, op, 0)) == NULL)
            return;
    }
    if (at)
        for (i = 1; i < 9; i++)
            fire_arch(op, op, op->x, op->y, i, at, 0, op->level, 0);

    explode_object(op);
}


int fire_cancellation(object *op, int dir, archetype *at, int magic)
{
    object *tmp;

    if (at == NULL)
        return 0;

    tmp = arch_to_object(at);

    if (tmp == NULL)
        return 0;

    tmp->x = op->x,tmp->y = op->y;
    tmp->direction = dir;

    if (op->type == PLAYER)
        set_owner(tmp, op);

    if ((tmp = insert_ob_in_map(tmp, op->map, op, 0)) != NULL)
        move_cancellation(tmp);
    return 1;
}

void move_cancellation(object *op)
{
    remove_ob(op);
    op->x += DIRX(op),op->y += DIRY(op);
    if (!op->direction || wall(op->map, op->x, op->y))
        return;
    if (reflwall(op->map, op->x, op->y, op))
    {
        op->direction = absdir(op->direction + 4);
        insert_ob_in_map(op, op->map, op, 0);
        return;
    }
    if ((op = insert_ob_in_map(op, op->map, op, 0)) != NULL)
        hit_map(op, 0);
}

void cancellation(object *op)
{
    object *tmp;

    if (IS_LIVE(op) || op->type == CONTAINER || op->type == THROWN_OBJ)
    {
        /* Recur through the inventory */
        for (tmp = op->inv; tmp != NULL; tmp = tmp->below)
                cancellation(tmp);
    }
    else/* Nullify this object. */ if (FABS(op->magic) <= (random_roll(0, 5)))
    {
        op->magic = 0;
        CLEAR_FLAG(op, FLAG_DAMNED);
        CLEAR_FLAG(op, FLAG_CURSED);
        CLEAR_FLAG(op, FLAG_KNOWN_MAGICAL);
        CLEAR_FLAG(op, FLAG_KNOWN_CURSED);
        if (op->env && op->env->type == PLAYER)
        {
            esrv_send_item(op->env, op);
        }
    }
}


/* Create a missile (nonmagic - magic +4). Will either create bolts or arrows
 * based on whether a crossbow or bow is equiped. If neither, it defaults to
 * arrows.
 * Sets the plus based on the casters level. It is also settable with the
 * invoke command. If the caster attempts to create missiles with too
 * great a plus, the default is used.
 * The # of arrows created also goes up with level, so if a 30th level mage
 * wants LOTS of arrows, and doesn't care what the plus is he could
 * create nonnmagic arrows, or even -1, etc...
 *
 * Written by Ben Fennema (huma@netcom.com) - bugs fixed by Raphael Quinet
 */

int cast_create_missile(object *op, object *caster, int dir, char *stringarg)
{
    int     missile_plus    = 0;
    char   *missile_name;
    object *tmp, *missile;
    tag_t   tag;

    missile_name = "arrow";

    for (tmp = op->inv; tmp != NULL; tmp = tmp->below)
        if (tmp->type == BOW && QUERY_FLAG(tmp, FLAG_APPLIED))
        {
            if (strstr(tmp->race, "bolt")) /* crossbow bolts */
                missile_name = "bolt";
            break;
        }

    if (stringarg)
        missile_plus = atoi(stringarg);
    if (!stringarg || ((1 + SP_level_strength_adjust(op, caster, SP_CREATE_MISSILE)) - (3 * missile_plus)) < 0)
        missile_plus = spells[SP_CREATE_MISSILE].bdam + SP_level_dam_adjust(op, caster, SP_CREATE_MISSILE);
    if (missile_plus > 4)
        missile_plus = 4;
    else if (missile_plus < -4)
        missile_plus = -4;

    if (find_archetype(missile_name) == NULL)
    {
        LOG(llevDebug, "Cast create_missile: could not find archtype %s\n", missile_name);
        return 0;
    }
    missile = get_archetype(missile_name);
    missile->nrof = spells[SP_CREATE_MISSILE].bdur * ((1 + SP_level_strength_adjust(op, caster, SP_CREATE_MISSILE))
                                                    - (3 * missile_plus));
    if (missile->nrof < 1)
        missile->nrof = 1;

    missile->magic = missile_plus;
    missile->value = 0; /* it would be too easy to get money by creating
                        arrows +4 and selling them, even with value = 1 */

    SET_FLAG(missile, FLAG_IDENTIFIED);
    tag = missile->count;
    if (!cast_create_obj(op, caster, missile, dir) && op->type == PLAYER && !was_destroyed(missile, tag))
    {
        tmp = get_owner(op);
        if (!tmp)
            pick_up(op, missile);
        else
            pick_up(tmp, missile);
    }
    return 1;
}


/* Alchemy code by Mark Wedel (master@rahul.net)
 *
 * This code adds a new spell, called alchemy.  Alchemy will turn
 * objects to gold nuggets, the value of the gold nuggets being
 * about 90% of that of the item itself.  It uses the value of the
 * object before charisma adjustments, because the nuggets themselves
 * will be will be adjusted by charisma when sold.
 *
 * Large nuggets are worth 25 gp each (base).  You will always get
 * the maximum number of large nuggets you could get.
 * Small nuggets are worth 1 gp each (base).  You will get from 0
 * to the max amount of small nuggets as you could get.
 *
 * For example, if an item is worth 110 gold, you will get
 * 4 large nuggets, and from 0-10 small nuggets.
 *
 * There is also a chance (1:30) that you will get nothing at all
 * for the object.  There is also a maximum weight that will be
 * alchemied.
 */

/* I didn't feel like passing these as arguements to the
 * two functions that need them.  Real values are put in them
 * when the spell is cast, and these are freed when the spell
 * is finished.
 */
/*static object *small, *large;*/

/* Give half price when we alchemy money (This should hopefully
 * make it so that it isn't worth it to alchemy money, sell
 * the nuggets, alchemy the gold from that, etc.
 * Otherwise, give 9 silver on the gold for other objects,
 * so that it would still be more affordable to haul
 * the stuff back to town.
 */
/*
static void alchemy_object(object *obj, int *small_nuggets,
     int *large_nuggets, int *weight)
{
    int value=(int)query_cost(obj, NULL, F_TRUE);

    if (QUERY_FLAG(obj, FLAG_UNPAID))
    value=0;
    else if (obj->type==MONEY || obj->type==GEM || obj->type==TYPE_JEWEL || obj->type==TYPE_PEARL || obj->type==TYPE_NUGGET)
    value /=3;
    else if (QUERY_FLAG(obj,FLAG_UNPAID)) value=0;
    else
    value = (int)((double)value * 0.9);

    if ((obj->value>0) && random_roll(0, 29)) {
#ifdef LOSSY_ALCHEMY
    int tmp = (value % large->value) / small->value;

    *large_nuggets += value/ large->value;
    if (tmp)
        *small_nuggets += random_roll(1, tmp);
#else
    static int value_store;
    int count;
    value_store += value;
    count = value_store / large->value;
    *large_nuggets += count;
    value_store -= count * large->value;
    count = value_store / small->value;
    *small_nuggets += count;
    value_store -= count * small->value;
#endif
    }

    if (*small_nuggets * small->value >= large->value) {
    (*large_nuggets)++;
    *small_nuggets -= large->value / small->value;
    if (*small_nuggets && large->value % small->value)
        (*small_nuggets)--;
    }
    weight += obj->weight;
    remove_ob(obj);
}
*/

/*
static void update_map(object *op, int small_nuggets, int large_nuggets,
    int x, int y)
{
    object *tmp;

    if (small_nuggets) {
        tmp = get_object();
        copy_object(small, tmp);
        tmp-> nrof = small_nuggets;
        tmp->x = x;
        tmp->y = y;
        insert_ob_in_map(tmp, op->map, op,0);
    }
    if (large_nuggets) {
        tmp = get_object();
        copy_object(large, tmp);
        tmp-> nrof = large_nuggets;
        tmp->x = x;
        tmp->y = y;
        insert_ob_in_map(tmp, op->map, op,0);
    }
}
*/

/* i will changing alchemy. MT */
/* weight_max = 100000 + 50000*op->level; */
int alchemy(object *op)
{
    /*
        int x,y,weight=0,weight_max,large_nuggets,small_nuggets,did_alc=0;
        object *next,*tmp;

      if(op->type!=PLAYER)
        return 0;
      weight_max = 100000 + 50000*SK_level(op);
      small=get_archetype("smallnugget"),
      large=get_archetype("largenugget");

    #ifdef ALCHEMY
      for(y= op->y-1;y<=op->y+1;y++) {
        for(x= op->x-1;x<=op->x+1;x++) {
          if(out_of_map(op->map,x,y) || wall(op->map,x,y) ||
         blocks_view(op->map,x,y))
            continue;
        for(tmp=get_map_ob(op->map,x,y);tmp!=NULL;tmp=next) {
              next=tmp->above;
          if(QUERY_FLAG(tmp,FLAG_IS_CAULDRON)) {
            attempt_do_alchemy(op, tmp);
            did_alc=1;
            continue;
          }
        }
        }
      }
      if (did_alc) return 1;
    #endif

      for(y= op->y-1;y<=op->y+1;y++) {
        for(x= op->x-1;x<=op->x+1;x++) {
          if(out_of_map(op->map,x,y) || wall(op->map,x,y) ||
            blocks_view(op->map,x,y))
            continue;

        small_nuggets=0;
        large_nuggets=0;

        for(tmp=get_map_ob(op->map,x,y);tmp!=NULL;tmp=next) {
              next=tmp->above;
          if (tmp->weight>0 && !QUERY_FLAG(tmp, FLAG_NO_PICK)
              && !QUERY_FLAG(tmp, FLAG_ALIVE)) {
            if (tmp->inv) {
            object *next1,*tmp1;
            for (tmp1 = tmp->inv; tmp1!=NULL; tmp1=next1) {
                next1 = tmp1->below;
                if (tmp1->weight>0 && !QUERY_FLAG(tmp1, FLAG_NO_PICK) && !QUERY_FLAG(tmp1, FLAG_ALIVE))
                    alchemy_object(tmp1, &small_nuggets, &large_nuggets,
                   &weight);
            }
            }
            alchemy_object(tmp, &small_nuggets, &large_nuggets, &weight);

            if (weight>weight_max) {
            update_map(op, small_nuggets, large_nuggets, x, y);
            return 1;
            }
          }

     }

        update_map(op, small_nuggets, large_nuggets, x, y);

        }
      }
      */
    return 1;
}

int remove_depletion(object *op, object *target)
{
    archetype  *at;
    object     *depl;
    int         i, success = 0;

    if ((at = find_archetype("depletion")) == NULL)
    {
        LOG(llevBug, "BUG: Could not find archetype depletion");
        return 0;
    }

    if (!op || !target)
        return success;


    if (target->type != PLAYER)
    {
        if (op->type == PLAYER) /* fake messages for non player... */
        {
            new_draw_info_format(NDI_UNIQUE, 0, op, "You cast depletion on %s.", query_base_name(target, op));
            new_draw_info(NDI_UNIQUE, 0, op, "There is no depletion.");
        }
        return success;
    }
    if (op != target)
    {
        if (op->type == PLAYER)
            new_draw_info_format(NDI_UNIQUE, 0, op, "You cast depletion on %s.", query_base_name(target, op));
        else if (target->type == PLAYER)
            new_draw_info_format(NDI_UNIQUE, 0, target, "%s cast remove depletion on you.", query_base_name(op, target));
    }


    if ((depl = present_arch_in_ob(at, target)) != NULL)
    {
        for (i = 0; i < 7; i++)
        {
            if (get_attr_value(&depl->stats, i))
            {
                success++;
                new_draw_info(NDI_UNIQUE, 0, target, restore_msg[i]);
            }
        }
        remove_ob(depl);
        FIX_PLAYER(target ,"remove depletion");
    }

    if (op != target && op->type == PLAYER)
    {
        if (success)
            new_draw_info(NDI_UNIQUE, 0, op, "Your prayer removes some depletion.");
        else
            new_draw_info(NDI_UNIQUE, 0, op, "There is no depletion.");
    }

    if (op != target && target->type == PLAYER && !success) /* if success, target got infos before */
        new_draw_info(NDI_UNIQUE, 0, target, "There is no depletion.");

    insert_spell_effect(spells[SP_REMOVE_DEPLETION].archname, target->map, target->x, target->y);
    return success;
}

int restoration(object *caster, object *target)
{
    int success = 0;
    object *force;
    static archetype  *at = NULL;

    if (!at)
    {
        at = find_archetype("drain");
        if (!at)
        {
            LOG(llevBug, "BUG: Couldn't find archetype drain.\n");
            return 0;
        }
    }

    /* try to find and remove the draining forces */
    SET_FLAG(target, FLAG_NO_FIX_PLAYER);
    force = present_arch_in_ob(at, target);
    if(force)
    {
        success = 1;
        remove_ob(force);
    }
    force = present_arch_in_ob_temp(at, target);
    if(force)
    {
        success = 1;
        remove_ob(force);
    }
    CLEAR_FLAG(target, FLAG_NO_FIX_PLAYER);

    if(caster)
    {
        if (caster->type == PLAYER)
        {
            if(caster == target)
                new_draw_info(NDI_UNIQUE, 0, caster, "You cast restoration on yourself.");
            else
                new_draw_info_format(NDI_UNIQUE, 0, caster, "You cast restoration on %s.", query_base_name(target, caster));
        }
        if (caster != target && target->type == PLAYER)
        {
            new_draw_info_format(NDI_UNIQUE, 0, target, "%s cast restoration on you.", query_base_name(caster, target));
        }
    }

    if(success)
    {

        FIX_PLAYER(target ,"restoration");
        if(target->type == PLAYER)
            new_draw_info(NDI_UNIQUE, 0, target, "Your level is restored!");
        if (caster && caster != target && target->type == PLAYER)
            new_draw_info_format(NDI_UNIQUE, 0, caster, "You restored %s.", query_base_name(caster, target));
    }

    return success;
}

int remove_deathsick(object *op, object *target)
{
    archetype  *at;
    object     *depl;
    int         i, success = 0;

    if ((at = find_archetype("deathsick")) == NULL)
    {
        LOG(llevBug, "BUG: Could not find archetype deathsick");
        return 0;
    }

    if (!op || !target)
        return success;


    if (target->type != PLAYER)
    {
        if (op->type == PLAYER) /* fake messages for non player... */
        {
            new_draw_info_format(NDI_UNIQUE, 0, op, "You cast remove death sickness on %s.", query_base_name(target, op));
            new_draw_info(NDI_UNIQUE, 0, op, "There is no death sickness.");
        }
        return success;
    }
    if (op != target)
    {
        if (op->type == PLAYER)
            new_draw_info_format(NDI_UNIQUE, 0, op, "You cast remove death sickness on %s.", query_base_name(target, op));
        else if (target->type == PLAYER)
            new_draw_info_format(NDI_UNIQUE, 0, target, "%s cast remove death sickness on you.", query_base_name(op, target));
    }


    if ((depl = present_arch_in_ob(at, target)) != NULL)
    {
        for (i = 0; i < 7; i++)
        {
            if (get_attr_value(&depl->stats, i))
            {
                success++;
                new_draw_info(NDI_UNIQUE, 0, target, restore_msg[i]);
            }
        }
        remove_ob(depl);
        FIX_PLAYER(target, "remove deathsick");
    }

    if (op != target && op->type == PLAYER)
    {
        if (success)
            new_draw_info(NDI_UNIQUE, 0, op, "Your prayer removes some death sickness.");
        else
            new_draw_info(NDI_UNIQUE, 0, op, "There is no death sickness.");
    }

    if (op != target && target->type == PLAYER && !success) /* if success, target got infos before */
        new_draw_info(NDI_UNIQUE, 0, target, "There is no death sickness.");

    insert_spell_effect(spells[SP_REMOVE_DEATHSICK].archname, target->map, target->x, target->y);
    return success;
}

int remove_curse(object *op, object *target, int type, SpellTypeFrom src)
{
    object *tmp;
    int     success = 0;

    if (!op || !target)
        return success;

    if (op != target)
    {
        if (op->type == PLAYER)
            new_draw_info_format(NDI_UNIQUE, 0, op, "You cast remove %s on %s.",
                                 type == SP_REMOVE_CURSE ? "curse" : "damnation", query_base_name(target, op));
        else if (target->type == PLAYER)
            new_draw_info_format(NDI_UNIQUE, 0, target, "%s cast remove %s on you.", query_base_name(op, target),
                                 type == SP_REMOVE_CURSE ? "curse" : "damnation");
    }

    /* Player remove xx only removes applied stuff, npc remove clears ALL */
    for (tmp = target->inv; tmp; tmp = tmp->below)
    {
        if ((src == spellNPC || QUERY_FLAG(tmp, FLAG_APPLIED))
         && (QUERY_FLAG(tmp, FLAG_CURSED) || (type == SP_REMOVE_DAMNATION && QUERY_FLAG(tmp, FLAG_DAMNED))))
        {
            if (tmp->level <= SK_level(op))
            {
                success++;
                if (type == SP_REMOVE_DAMNATION)
                    CLEAR_FLAG(tmp, FLAG_DAMNED);
                CLEAR_FLAG(tmp, FLAG_CURSED);
                if (!QUERY_FLAG(tmp, FLAG_PERM_CURSED))
                    CLEAR_FLAG(tmp, FLAG_KNOWN_CURSED);
                if (target->type == PLAYER)
                    esrv_send_item(target, tmp);
            }
            else /* level of the items is to high for this remove curse */
            {
                if (target->type == PLAYER)
                    new_draw_info_format(NDI_UNIQUE, 0, target, "The %s curse is stronger as the prayer!",
                                         query_base_name(tmp, target));
                else if (op != target && op->type == PLAYER)
                    new_draw_info_format(NDI_UNIQUE, 0, op, "The %s curse of %s is stronger as your prayer!",
                                         query_base_name(tmp, op), query_base_name(target, op));
            }
        }
    }

    if (op != target && op->type == PLAYER)
    {
        if (success)
            new_draw_info(NDI_UNIQUE, 0, op, "Your prayer removes some curses.");
        else
            new_draw_info_format(NDI_UNIQUE, 0, op, "%s items seems uncursed.", query_base_name(target, op));
    }

    if (target->type == PLAYER)
    {
        if (success)
            new_draw_info(NDI_UNIQUE, 0, target, "You feel like someone is helping you.");
        else
        {
            if (src == spellNormal)
                new_draw_info(NDI_UNIQUE, 0, target, "You are not using any cursed items.");
            else
                new_draw_info(NDI_UNIQUE, 0, target, "You hear manical laughter in the distance.");
        }
    }

    insert_spell_effect(spells[SP_REMOVE_CURSE].archname, target->map, target->x, target->y);
    return success;
}

/* main identify function to identify objects.
 * mode extension: 0: identify 1 to x items, depending
 * luck & wisdom (nethack style). This is default for player identify.
 * mode 1: identify all unidentified items in the inventory of op.
 * mode 2: identify marked item (not implemented)
 * i added a "identify level" - thats the "power" of the identify spell.
 * if the item has a higher level as the identify then the item
 * can'T be identified from this spell/skills.
 */
int cast_identify(object *op, int level, object *single_ob, int mode)
{
    object *tmp;
    int     success = 0, success2 = 0, random_val = 0;
    int     chance  = 8 + op->stats.Wis;

    if (chance < 1)
        chance = 1;

    /* iam to lazy to put the id stuff in own function... */
    if (mode == IDENTIFY_MODE_MARKED)
    {
        tmp = single_ob;
        goto inside_jump1;
    }

    insert_spell_effect(spells[SP_IDENTIFY].archname, op->map, op->x, op->y);
    for (tmp = op->inv; tmp ; tmp = tmp->below)
    {
        inside_jump1:
        if (!QUERY_FLAG(tmp, FLAG_IDENTIFIED) && !IS_SYS_INVISIBLE(tmp) && (need_identify(tmp) ||mode ==IDENTIFY_MODE_MARKED))
        {
            success2++;
            if (level < tmp->level)
            {
                if (op->type == PLAYER)
                    new_draw_info_format(NDI_UNIQUE, 0, op, "The %s is too powerful for this identify!",
                                         query_base_name(tmp, op));
            }
            else
            {
                identify(tmp);
                if (op->type == PLAYER)
                {
                    new_draw_info_format(NDI_UNIQUE, 0, op, "You have %s.", long_desc(tmp, op));
                    if (tmp->msg)
                    {
                        new_draw_info(NDI_UNIQUE, 0, op, "The item has a story:");
                        new_draw_info(NDI_UNIQUE, 0, op, tmp->msg);
                    }
                }

                if (IDENTIFY_MODE_NORMAL
                 && ((random_val = random_roll(0, chance - 1)) > (chance - ++success - 2)))
                    break;
            }
        }
        if (mode == IDENTIFY_MODE_MARKED)
            break;
    }
    /* If all the power of the spell has been used up, don't go and identify
     * stuff on the floor.  Only identify stuff on the floor if the spell
     * was not fully used.
     */
    /* i disable this... this is useful if we have like in cf 100 dead mobs
     * with 1000 items on the floor - but we don't want have this much junk
     * lot
     *
    if(IDENTIFY_MODE_ALL)
    {
      for(tmp = get_map_ob(op->map,op->x,op->y);tmp!=NULL;tmp=tmp->above)
      if (!QUERY_FLAG(tmp, FLAG_IDENTIFIED) && !IS_SYS_INVISIBLE(tmp) &&
    need_identify(tmp))
      {
        identify(tmp);
        if (op->type==PLAYER) {
    new_draw_info_format(NDI_UNIQUE, 0,op,
        "On the ground is %s.", long_desc(tmp, op));
    if (tmp->msg) {
      new_draw_info(NDI_UNIQUE, 0,op, "The item has a story:");
      new_draw_info(NDI_UNIQUE, 0,op, tmp->msg);
    }
    esrv_send_item(op, tmp);
        }
      }
    }
    */
    if (op->type == PLAYER && (!success && !success2))
        new_draw_info(NDI_UNIQUE, 0, op, "You can't reach anything unidentified in your inventory.");

    return success2;
}

/* gthe routine under this one is the old detect routine - i
 * rewrote it in many parts (and detect inv. works absolutly different now)
 */
int cast_detection(object *op, object *target, int type)
{
    int         nx, ny, suc = FALSE, sucmap = FALSE;
    object     *tmp = NULL;
    mapstruct  *m = NULL;

    switch (type)
    {
        case SP_DETECT_MAGIC:
          if (op->type == PLAYER && target != op)
              new_draw_info_format(NDI_UNIQUE, 0, op, "You cast detect magic on %s.",
                                   target->name ? target->name : "someone");

          if (target->type != PLAYER) /* only use self or players */
          {
              if (op->type == PLAYER)
                  new_draw_info(NDI_UNIQUE, 0, op, "This spell works only on players.");
              return 0;
          }
          if (target != op)
              new_draw_info_format(NDI_UNIQUE, 0, target, "%s casts detect magic on you.",
                                   op->name ? op->name : "someone");


          /* detect targets inv */
          for (tmp = target->inv; tmp; tmp = tmp->below)
          {
              if (!QUERY_FLAG(tmp, FLAG_SYS_OBJECT)
               && (!QUERY_FLAG(tmp, FLAG_KNOWN_MAGICAL) && !QUERY_FLAG(tmp, FLAG_IDENTIFIED) && is_magical(tmp)))
              {
                  SET_FLAG(tmp, FLAG_KNOWN_MAGICAL);
                  suc = TRUE;
                  esrv_send_item(target, tmp);
              }
          }

          nx = target->x;ny = target->y;
          if (!(m = out_of_map(target->map, &nx, &ny)))
              return 0;

          for (tmp = GET_MAP_OB(m, nx, ny); tmp != NULL; tmp = tmp->above)
          {
              if (!QUERY_FLAG(tmp, FLAG_SYS_OBJECT)
               && (!QUERY_FLAG(tmp, FLAG_KNOWN_MAGICAL) && !QUERY_FLAG(tmp, FLAG_IDENTIFIED) && is_magical(tmp)))
              {
                  SET_FLAG(tmp, FLAG_KNOWN_MAGICAL);
                  suc = TRUE;
                  sucmap = TRUE;
              }
          }
          break;

        case SP_DETECT_CURSE:
          if (op->type == PLAYER && target != op)
              new_draw_info_format(NDI_UNIQUE, 0, op, "You cast detect curse on %s.",
                                   target->name ? target->name : "someone");

          if (target->type != PLAYER) /* only use self or players */
          {
              if (op->type == PLAYER)
                  new_draw_info(NDI_UNIQUE, 0, op, "This spell works only on players.");
              return 0;
          }
          if (target != op)
              new_draw_info_format(NDI_UNIQUE, 0, target, "%s casts detect curse on you.",
                                   op->name ? op->name : "someone");

          for (tmp = target->inv; tmp; tmp = tmp->below)
          {
              if (!QUERY_FLAG(tmp, FLAG_SYS_OBJECT)
               && (!QUERY_FLAG(tmp, FLAG_KNOWN_CURSED) && (QUERY_FLAG(tmp, FLAG_CURSED) || QUERY_FLAG(tmp, FLAG_DAMNED))))
              {
                  SET_FLAG(tmp, FLAG_KNOWN_CURSED);
                  suc = TRUE;
                  esrv_send_item(target, tmp);
              }
          }
          nx = target->x;ny = target->y;
          if (!(m = out_of_map(target->map, &nx, &ny)))
              return 0;

          for (tmp = GET_MAP_OB(m, nx, ny); tmp != NULL; tmp = tmp->above)
          {
              if (!QUERY_FLAG(tmp, FLAG_SYS_OBJECT)
               && (!QUERY_FLAG(tmp, FLAG_KNOWN_CURSED) && (QUERY_FLAG(tmp, FLAG_CURSED) || QUERY_FLAG(tmp, FLAG_DAMNED))))
              {
                  suc = TRUE;
                  sucmap = TRUE;
                  SET_FLAG(tmp, FLAG_KNOWN_CURSED);
              }
          }

          break;
    }

    if (sucmap) /* we have something changed in this tile */
        INC_MAP_UPDATE_COUNTER(m, nx, ny);

    if (suc)
    {
        if (op->type == PLAYER)
            new_draw_info(NDI_UNIQUE, 0, op, "The spell detects something.");
        if (target->type == PLAYER && target != op)
            new_draw_info(NDI_UNIQUE, 0, target, "The spell detects something.");
    }
    else
    {
        if (op->type == PLAYER)
            new_draw_info(NDI_UNIQUE, 0, op, "The spell detects nothing.");
        if (target->type == PLAYER && target != op)
            new_draw_info(NDI_UNIQUE, 0, target, "The spell detects nothing.");
    }

    if (insert_spell_effect(spells[type].archname, target->map, target->x, target->y))
        LOG(llevDebug, "insert_spell_effect() failed: spell:%d, obj:%s target:%s\n", type, query_name(op),
            query_name(target));

    return 1;
}

/* this size should really be based on level or spell parameter -
 * even if out of the view, the setting of these values can be useful
 * simply so that when you come across them they have already been
 * set for you.
 */
/*
static int cast_detection_old(object *op, object *target,int type) {
    object *tmp, *last=NULL;
    int x,y,done_one,nx,ny;
    archetype *detect_arch;
    mapstruct   *m;

    detect_arch = find_archetype("detect_magic");
    if (detect_arch == (archetype *) NULL)
    {
        LOG(llevBug, "BUG: Couldn't find archetype detect_magic (%s).\n", query_name(op));
        return 0;
    }

    for (x = op->x - MAP_CLIENT_X/2; x <= op->x + MAP_CLIENT_X/2; x++)
    for (y = op->y - MAP_CLIENT_Y/2; y <= op->y + MAP_CLIENT_Y/2; y++) {

        done_one = 0;
        nx=x;
        ny=y;
        if(!(m = out_of_map(op->map, &nx, &ny)))
            continue;

        if (type==SP_SHOW_INVIS)
            last = get_map_ob(m, nx, ny);
        else {
        for (tmp=get_map_ob(m, nx, ny); tmp; tmp=tmp->above) last=tmp;

        for (tmp=last; tmp; tmp=tmp->below) {
            if (tmp->type == FLOOR) break;
            else last=tmp;
        }

        if (tmp) last=tmp->above;
        }

        for (tmp = last;
        tmp && (!done_one || type==SP_DETECT_MAGIC || type==SP_DETECT_CURSE);
        tmp=tmp->above) {

        switch(type) {

            case SP_DETECT_MAGIC:
            if (!QUERY_FLAG(tmp,FLAG_KNOWN_MAGICAL) && !QUERY_FLAG(tmp, FLAG_IDENTIFIED) &&
                is_magical(tmp)) {

                SET_FLAG(tmp,FLAG_KNOWN_MAGICAL);
                if(tmp->type==RUNE && tmp->attacktype&AT_MAGIC)
                tmp->stats.Cha/=4;
                done_one = 1;
            }
            break;

            case SP_DETECT_MONSTER:
            if ((QUERY_FLAG(tmp, FLAG_MONSTER) || tmp->type==PLAYER)) {
                done_one = 2;
                last=tmp;
            }
            break;


            case SP_DETECT_EVIL:
            done_one = 0;
            if(QUERY_FLAG(tmp,FLAG_MONSTER) && tmp->race) {
                object *god=find_god(determine_god(op));
                if(god && god->slaying && strstr(god->slaying,tmp->race)) {
                last=tmp;
                done_one = 2;
                }
            }
            break;

            case SP_DETECT_CURSE:
            if (!QUERY_FLAG(tmp, FLAG_KNOWN_CURSED) &&
            (QUERY_FLAG(tmp, FLAG_CURSED) ||
             QUERY_FLAG(tmp, FLAG_DAMNED))) {
            SET_FLAG(tmp, FLAG_KNOWN_CURSED);
            done_one = 1;
            }
            break;

        }
         }

        if (done_one) {
        object *detect_ob = arch_to_object(detect_arch);
        detect_ob->x = x;
        detect_ob->y = y;
        if (done_one == 2 && last) {
            detect_ob->face = last->face;
            detect_ob->animation_id = last->animation_id;
            detect_ob->anim_speed = last->anim_speed;
            detect_ob->last_anim=0;
            if (!QUERY_FLAG(last, FLAG_ANIMATE)) CLEAR_FLAG(detect_ob, FLAG_ANIMATE);
        }
        insert_ob_in_map(detect_ob, op->map, op,0);
        }
    }

    if ((type == SP_DETECT_MAGIC || type == SP_DETECT_CURSE) && op->type == PLAYER) {
    done_one = 0;
    for (tmp = op->inv; tmp; tmp = tmp->below) {
        if (!QUERY_FLAG(tmp, FLAG_IDENTIFIED))

        switch(type) {
            case SP_DETECT_MAGIC:
            if (is_magical(tmp) && !QUERY_FLAG(tmp,FLAG_KNOWN_MAGICAL)) {
                SET_FLAG(tmp,FLAG_KNOWN_MAGICAL);
                esrv_send_item (op, tmp);
                done_one = 1;
            }
            break;

            case SP_DETECT_CURSE:
            if (!QUERY_FLAG(tmp, FLAG_KNOWN_CURSED) &&
            (QUERY_FLAG(tmp, FLAG_CURSED) ||
             QUERY_FLAG(tmp, FLAG_DAMNED))) {
            SET_FLAG(tmp, FLAG_KNOWN_CURSED);
            esrv_send_item (op, tmp);
            done_one = 1;
            }
            break;
        }
    }
    }
    return 1;
}
*/
/* Shamelessly hacked from PeterM's cast_charm and destruction code
 *  - b.t. thomas@nomad.astro.psu.edu
 */

/* Changes in the spell code to make it more powerfull - now it can
 * pacify multi-square creatures, and has greater range - Aug 95 b.t.
 */
/* New modification -- w/ Multigod hack, now if its a member of an aligned
 * race, we automatically pacify it. b.t.
 */

int cast_pacify(object *op, object *weap, archetype *arch, int spellnum)
{
    int         i, r, j, xt, yt;
    object     *tmp, *effect;
    mapstruct  *m;
    object     *god = find_god(determine_god(op));

    r = 1 + SP_level_strength_adjust(op, weap, SP_PACIFY);

    for (i = -r; i < r; i++)
        for (j = -r; j < r; j++)
        {
            xt = op->x + i;
            yt = op->y + i;
            if (!(m = out_of_map(op->map, &xt, &yt)))
                continue;
            for (tmp = get_map_ob(m, xt, yt); tmp && (!QUERY_FLAG(tmp, FLAG_MONSTER)); tmp = tmp->above)
                ;
            if (!tmp)
                continue;
            if (tmp->type == PLAYER)
                continue;

            /* we only go through checking if the monster is not aligned
            member, we dont worship a god, or monster has no race */
            if (!tmp->race || !god || !god->race || !strstr(god->race, tmp->race))
            {
                /* multiple square monsters only when caster is => level of creature */
                if ((tmp->more || tmp->head) && (SK_level(op) < tmp->level))
                    continue;
                if (weap->slaying)      /* selective pacify */
                    if (tmp->race != weap->slaying && tmp->name != weap->slaying)
                        continue;
                /* if(op->level <( (RANDOM()%(2*tmp->level+1))-(op->stats.Cha-10)/2)) continue; */
                if (SK_level(op) < random_roll(0, 2 * tmp->level) - (op->stats.Cha - 10) / 2)
                    continue;
            }

            if ((effect = get_archetype("detect_magic")))
            {
                effect->x = tmp->x;
                effect->y = tmp->y;
                insert_ob_in_map(effect, tmp->map, op, 0);
            }
            SET_FLAG(tmp, FLAG_UNAGGRESSIVE);
        }

    return 1;
}

/* summon fog code. I could'nt decide whether this
 * could just go into another routine (like create_
 * the_feature) or have it alone. For now, its separate
 * function. This code just creates a variable amount of
 * fog archetypes around the character.
 * Implementation by b.t. (thomas@nomad.astro.psu.edu)
 * (based on create bomb code)
 */

int summon_fog(object *op, object *caster, int dir, int spellnum)
{
    object *tmp;
    int     i, dx = op->x + freearr_x[dir], dy = op->y + freearr_y[dir];

    if (!spellarch[spellnum])
        return 0;

    for (i = 1; i < MIN(2 + SP_level_strength_adjust(op, caster, spellnum), SIZEOFFREE); i++)
    {
        if (wall(op->map, dx, dy))
        {
            new_draw_info(NDI_UNIQUE, 0, op, "There is something in the way.");
            return 0;
        }
        tmp = get_archetype(spellarch[spellnum]->name);
        tmp->x = dx,tmp->y = dy;    /* all fog starts in 1 place */
#ifdef WALL_CREDIT              /* does someone get exp for the kills? */
        set_owner(tmp, op);     /* note however, that after 'fog' moves */
        /* it is no longer owned. It is unlikely */
        /* that players will garner much exp with */
        /* this spell */
#endif
        insert_ob_in_map(tmp, op->map, op, 0);
    }

    return 1;
}

/*  create_the_feature:  peterm  */
/*  implementation of the spells which build directors, lightning
 *   walls, bullet walls, and fireballwalls.
 */
/* WARNING - do a blocked() check for the spell object - not for the op or caster */
int create_the_feature(object *op, object *caster, int dir, int spell_effect)
{
    object     *tmp = NULL;
    mapstruct  *m;
    char        buf1[20];
    int         xt, yt, putflag = 0;

    if (!dir)
        dir = op->facing;
    else
        putflag = 1;

    if (blocked(op, op->map, op->x + freearr_x[dir], op->y + freearr_y[dir], op->terrain_flag))
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Something is in the way.");
        return 0;
    }

    switch (spell_effect)
    {
        case SP_BUILD_DIRECTOR:
          sprintf(buf1, "director_%d", dir);
          tmp = get_archetype(buf1);
          SET_FLAG(tmp, FLAG_IS_USED_UP);
          tmp->stats.food = spells[spell_effect].bdur + 10 * SP_level_strength_adjust(op, caster, spell_effect);
          tmp->stats.hp = spells[spell_effect].bdam + 5 * SP_level_dam_adjust(op, caster, spell_effect);
          tmp->stats.maxhp = tmp->stats.hp;
          break;

        case SP_BUILD_LWALL:
          sprintf(buf1, "lightningwall_%d", dir);
          tmp = get_archetype(buf1);
          SET_FLAG(tmp, FLAG_IS_USED_UP);
          SET_FLAG(tmp, FLAG_TEAR_DOWN);
          SET_FLAG(tmp, FLAG_ALIVE);
          tmp->stats.food = spells[spell_effect].bdur + 10 * SP_level_strength_adjust(op, caster, spell_effect);
          tmp->stats.hp = spells[spell_effect].bdam + 5 * SP_level_dam_adjust(op, caster, spell_effect);
          tmp->stats.maxhp = tmp->stats.hp;
          break;

        case SP_BUILD_BWALL:
          sprintf(buf1, "lbulletwall_%d", dir);
          tmp = get_archetype(buf1);
          SET_FLAG(tmp, FLAG_IS_USED_UP);
          SET_FLAG(tmp, FLAG_TEAR_DOWN);
          SET_FLAG(tmp, FLAG_ALIVE);
          tmp->stats.food = spells[spell_effect].bdur + 10 * SP_level_strength_adjust(op, caster, spell_effect);
          tmp->stats.hp = spells[spell_effect].bdam + 5 * SP_level_dam_adjust(op, caster, spell_effect);
          tmp->stats.maxhp = tmp->stats.hp;
          break;

        case SP_BUILD_FWALL:
          sprintf(buf1, "firewall_%d", dir);
          tmp = get_archetype(buf1);
          SET_FLAG(tmp, FLAG_IS_USED_UP);
          SET_FLAG(tmp, FLAG_TEAR_DOWN);
          SET_FLAG(tmp, FLAG_ALIVE);
          tmp->stats.food = spells[spell_effect].bdur + 10 * SP_level_strength_adjust(op, caster, spell_effect);
          tmp->stats.hp = spells[spell_effect].bdam + 5 * SP_level_dam_adjust(op, caster, spell_effect);
          tmp->stats.maxhp = tmp->stats.hp;
          break;
    }

    if (op->type == PLAYER)
        set_owner(tmp, op);

    tmp->level = SK_level(op) / 2;  /*  so that the spell that the wall casts
                                    inherit part of the effectiveness of
                                    of the wall builder */
    tmp->x = op->x;
    tmp->y = op->y;
    xt = tmp->x + freearr_x[dir];
    yt = tmp->y + freearr_y[dir];
    if (!(m = out_of_map(op->map, &xt, &yt)))
        return 0;
    if (putflag)
    {
        tmp->x = xt;
        tmp->y = yt;
    }
    if ((tmp = insert_ob_in_map(tmp, m, op, 0)) == NULL)
        return 1;
    if (!op->type == PLAYER)
        SET_FLAG(op, FLAG_SCARED); /* We don't want them to walk through the wall! */
    return 1;
}


/*  cast_transfer:  peterm  */
/*  following spell transfers mana from one person to another.
    right now, it's no respecter of maximum sp limits.  WOn't
    fix that, regard it as a feature.  be nice to make someone's
    head explode if they supercharge too much, though.  */

int cast_transfer(object *op, int dir)
{
    object     *plyr    = NULL;
    mapstruct  *m;
    int         xt, yt;
    /*  see if we can find someone to give sp to.  */

    xt = op->x + freearr_x[dir];
    yt = op->y + freearr_y[dir];
    if ((m = out_of_map(op->map, &xt, &yt)))
    {
        for (plyr = get_map_ob(m, xt, yt); plyr != NULL; plyr = plyr->above)
            if (IS_LIVE(plyr))
                break;
    }
    /*  If we did not find a player in the specified direction, transfer
    to anyone on top of us. */
    if (plyr == NULL)
        for (plyr = get_map_ob(op->map, op->x, op->y); plyr != NULL; plyr = plyr->above)
            if (IS_LIVE(plyr))
                break;

    if (plyr)
    {
        /* DAMN: added spell strength adjust; higher level casters transfer mana faster */
        int maxsp   = plyr->stats.maxsp;
        int sp      = (plyr->stats.sp + spells[SP_TRANSFER].bdam + SP_level_dam_adjust(op, op, SP_TRANSFER));

        plyr->stats.sp = sp;

        new_draw_info(NDI_UNIQUE, 0, plyr, "You feel energy course through you.");
        if (sp >= maxsp * 2)
        {
            new_draw_info(NDI_UNIQUE, 0, plyr, "Your head explodes!");
            fire_arch(op, plyr, op->x, op->y ,0, spellarch[SP_L_FIREBALL], SP_L_FIREBALL, op->level,0);
            /* Explodes a large fireball centered at player */
            /* damage_ob(plyr, 9998, op, ENV_ATTACK_CHECK);*/
            plyr->stats.sp = 2 * maxsp;
        }
        else if (sp >= maxsp * 1.88)
            new_draw_info(NDI_UNIQUE, NDI_ORANGE, plyr, "You feel like your head is going to explode.");
        else if (sp >= maxsp * 1.66)
            new_draw_info(NDI_UNIQUE, 0, plyr, "You get a splitting headache!");
        else if (sp >= maxsp * 1.5)
        {
            new_draw_info(NDI_UNIQUE, 0, plyr, "CHaOs fills your world.");
            confuse_player(op, op, 160);
        }
        else if (sp >= maxsp * 1.25)
            new_draw_info(NDI_UNIQUE, 0, plyr, "You start hearing voices.");
        return 1;
    }
    else
        return 0;
}

/*  drain_magic:  peterm  */
/*  drains all the magic out of the victim.  */
int drain_magic(object *op, int dir)
{
    object     *tmp = NULL;
    mapstruct  *m;
    int         xt, yt;
    double      mana, rate;

    xt = op->x + freearr_x[dir];
    yt = op->y + freearr_y[dir];
    if ((m = out_of_map(op->map, &xt, &yt)))
    {
        for (tmp = get_map_ob(m, xt, yt); tmp != NULL; tmp = tmp->above)
            if (IS_LIVE(tmp))
                break;
        /*  If we did not find a player in the specified direction, transfer
        to anyone on top of us. */
    }

    if (tmp == NULL)
        for (tmp = get_map_ob(op->map, op->x, op->y); tmp != NULL; tmp = tmp->above)
            if (IS_LIVE(tmp))
                break;

    /* DAMN: Percent spell point loss determined by caster level
       Caster gains percent of drained mana, also determined by caster level */
    if (tmp && op != tmp)
    {
        rate = (double) (spells[SP_MAGIC_DRAIN].bdam + 5 * SP_level_dam_adjust(op, op, SP_MAGIC_DRAIN)) / 100.0;
        if (rate > 0.95)
            rate = 0.95;
        mana = tmp->stats.sp * rate;
        tmp->stats.sp -= (sint16) mana;
        if (IS_LIVE(op))
        {
            rate = (double) (spells[SP_MAGIC_DRAIN].bdam + 5 * SP_level_strength_adjust(op, op, SP_MAGIC_DRAIN))
                 / 100.0;
            if (rate > 0.95)
                rate = 0.95;
            mana = mana * rate;
            op->stats.sp += (sint16) mana;
        }
        return 1;
    }
    else
    {
        return 0;
    }
}

/*  counterspell:  peterm  */
/*  an object of type counterspell will nullify cone objects,
    explosion objects, and anything else that |=magic.  */

void counterspell(object *op, int dir)
{
    object     *tmp;
    mapstruct  *m;
    int         xt, yt, nflag = 0;

    xt = op->x + freearr_x[dir];
    yt = op->y + freearr_y[dir];
    if (!(m = out_of_map(op->map, &xt, &yt)))
        return;

    for (tmp = get_map_ob(m, xt, yt); tmp != NULL; tmp = tmp->above,nflag = 0)
    {
        /* Basially, if the object is magical and not counterspell,
         * we will more or less remove the object.  Changed in 0.94.3
         * so that we don't kill monsters with this (some monsters attacktype
         * has magic in it).
                */
        /* don't attack our own spells */
        if (tmp->owner && tmp->owner == op->owner)
            continue;

        if (tmp->material == 0
         /*
         && tmp->attacktype & AT_FORCE
         && !(tmp->attacktype & AT_COUNTERMAGIC)
         */
         && !QUERY_FLAG(tmp, FLAG_MONSTER))
            nflag = 1;
        else
            switch (tmp->type)
            {
                case CONE:
                case FBALL:
                case LIGHTNING:
                case FBULLET:
                case MMISSILE:
                case SPEEDBALL:
                case BOMB:
                case POISONCLOUD:
                case CANCELLATION:
                case SWARM_SPELL:
                case BALL_LIGHTNING:
                  nflag = 1;
                  break;

                case RUNE:
                  nflag = 2;
                  break;

                default:
                  nflag = 0;
            }
        switch (nflag)
        {
            case 1:
              {
                  /* { if(op->level > tmp->level) { */
                  if (SK_level(op) > tmp->level)
                  {
                      remove_ob(tmp);
                      check_walk_off(tmp, NULL, MOVE_APPLY_VANISHED);
                  }
                  break;
              }
            case 2:
              {
                  if (random_roll(0, 149) == 0)
                  {
                      tmp->stats.hp--;  /* weaken the rune */
                      if (!tmp->stats.hp)
                      {
                          remove_ob(tmp);
                          check_walk_off(tmp, NULL, MOVE_APPLY_VANISHED);
                      }
                  }
                  break;
              }
        }
    }
}



/*  peterm:  function which summons hostile monsters and
  places them in nearby squares.  */

int summon_hostile_monsters(object *op, int n, const char *monstername)
{
    int i;
    for (i = 0; i < n; i++)
        put_a_monster(op, monstername);
    return n;
}


/*  charm spell by peterm@soda.berkeley.edu
    searches nearby squares for monsters to charm.  Each of them
    is subject to being charmed, that is, to becoming the pet
    monster of the caster.  Monsters larger than 1 square
    are uncharmeable right now.  */

/* Aug 95 - hack on the code to make it charm only undead for
 * priests, and not charm undead for magicians -b.t
 */

int cast_charm(object *op, object *caster, archetype *arch, int spellnum)
{
    LOG(llevBug, "BUG: unimplemented spell 'charm'\n");
    return 0;
    /* TODO: update for the new AI and pet systems. Gecko 2006-04-30 */
#if 0
    int         i, xt, yt;
    object     *tmp, *effect;
    mapstruct  *m;

    if (op->type != PLAYER)
        return 0;
    for (i = 1; i < MIN(9 + SP_level_strength_adjust(op, caster, spellnum), SIZEOFFREE); i++)
    {
        xt = op->x + freearr_x[i];
        yt = op->x + freearr_x[i];
        if (!(m = out_of_map(op->map, &xt, &yt)))
            continue;
        for (tmp = get_map_ob(m, xt, yt); tmp && (!QUERY_FLAG(tmp, FLAG_MONSTER)); tmp = tmp->above)
            ;
        if (!tmp)
            continue;
        if (tmp->type == PLAYER)
            continue;
        if (QUERY_FLAG(tmp, FLAG_UNDEAD))
            continue;
        if (tmp->more || tmp->head)
            continue;  /* multiple square monsters NOT */
        /* if(op->level <( (RANDOM()%(2*tmp->level+1))-(op->stats.Cha-10)/2)) continue; */
        if (SK_level(op) < random_roll(0, 2 * tmp->level) - (op->stats.Cha - 10) / 2)
            continue;

        if ((effect = get_archetype("detect_magic")))
        {
            effect->x = tmp->x;
            effect->y = tmp->y;
            insert_ob_in_map(effect, tmp->map, op, 0);
        }
        set_owner(tmp, op);
        SET_FLAG(tmp, FLAG_MONSTER);
        SET_FLAG(tmp, FLAG_FRIENDLY);
        tmp->stats.exp = 0;
        tmp->move_type = PETMOVE;
    }
    return 1;
#endif
}

int cast_charm_undead(object *op, object *caster, archetype *arch, int spellnum)
{
    LOG(llevBug, "BUG: unimplemented spell 'charm undead'\n");
    return 0;
    /* TODO: update for the new AI and pet systems. Gecko 2006-04-30 */
#if 0
    int         i, bonus, xt, yt;
    mapstruct  *m;
    object     *tmp, *effect, *god = find_god(determine_god(op));

    if (op->type != PLAYER)
        return 0;
    if (QUERY_FLAG(caster, FLAG_UNDEAD) || (god->race && strstr(god->race, shstr_cons.undead) != NULL))
    {
        bonus = 5;
    }
    else if (god->slaying && strstr(god->slaying, shstr_cons.undead) != NULL)
    {
        bonus = -5;
    }
    else
    {
        bonus = -1;
    }
    for (i = 1; i < MIN(9 + SP_level_strength_adjust(op, caster, spellnum), SIZEOFFREE); i++)
    {
        xt = op->x + freearr_x[i];
        yt = op->x + freearr_x[i];
        if (!(m = out_of_map(op->map, &xt, &yt)))
            continue;
        for (tmp = get_map_ob(m, xt, yt); tmp && (!QUERY_FLAG(tmp, FLAG_MONSTER)); tmp = tmp->above)
            ;
        if (!tmp)
            continue;
        if (tmp->type == PLAYER)
            continue;
        if (!QUERY_FLAG(tmp, FLAG_UNDEAD))
            continue;
        if (tmp->more || tmp->head)
            continue;  /* multiple square monsters NOT */
        if (SK_level(op) + bonus < random_roll(0, 2 * tmp->level) - (op->stats.Wis - 10) / 2)
            continue;

        if ((effect = get_archetype("detect_magic")))
        {
            effect->x = tmp->x;
            effect->y = tmp->y;
            insert_ob_in_map(effect, tmp->map, op, 0);
        }
        set_owner(tmp, op);
        SET_FLAG(tmp, FLAG_MONSTER);
        SET_FLAG(tmp, FLAG_FRIENDLY);
        tmp->stats.exp = 0;
        tmp->move_type = PETMOVE;
    }
    return 1;
#endif
}

/* Returns a monster (chosen at random) that this particular player (and his
 * god) find acceptable.  This checks level, races allowed by god, etc
 * to determine what is acceptable.
 * This returns NULL if no match was found.
 */

object * choose_cult_monster(object *pl, object *god, int summon_level)
{
    char        buf[MAX_BUF], *race;
    int         racenr, mon_nr, i;
    racelink   *list;
    objectlink *tobl;
    object     *otmp;

    /* Determine the number of races available */
    racenr = 0;
    strcpy(buf, god->race);
    race = strtok(buf, ",");
    while (race)
    {
        racenr++;
        race = strtok(NULL, ",");
    }

    /* next, randomly select a race from the aligned_races string */
    if (racenr > 1)
    {
        racenr = random_roll(0, racenr - 1);
        strcpy(buf, god->race);
        race = strtok(buf, ",");
        for (i = 0; i < racenr; i++)
            race = strtok(NULL, ",");
    }
    else
        race = (char *) god->race;


    /* see if a we can match a race list of monsters.  This should not
     * happen, so graceful recovery isn't really needed, but this sanity
     * checking is good for cases where the god archetypes mismatch the
     * race file
     */
    if ((list = find_racelink(race)) == NULL)
    {
        new_draw_info_format(NDI_UNIQUE, 0, pl, "The spell fails! %s's creatures are beyond", god->name);
        new_draw_info(NDI_UNIQUE, 0, pl, "the range of your summons.");
        LOG(llevDebug, "choose_cult_monster() requested non-existant aligned race!\n");
        return 0;
    }

    /* search for an apprplritate monster on this race list */
    mon_nr = 0;
    for (tobl = list->member; tobl; tobl = tobl->next)
    {
        otmp = tobl->objlink.ob;
        if (!otmp || !QUERY_FLAG(otmp, FLAG_MONSTER))
            continue;
        if (otmp->level <= summon_level)
            mon_nr++;
    }
    /* If this god has multiple race entries, we should really choose another.
     * But then we either need to track which ones we have tried, or just
     * make so many calls to this function, and if we get so many without
     * a valid entry, assuming nothing is available and quit.
     */
    if (!mon_nr)
        return NULL;
    mon_nr = random_roll(0, mon_nr - 1);
    for (tobl = list->member; tobl; tobl = tobl->next)
    {
        otmp = tobl->objlink.ob;
        if (!otmp || !QUERY_FLAG(otmp, FLAG_MONSTER))
            continue;
        if (otmp->level <= summon_level && !mon_nr--)
            return otmp;
    }
    /* This should not happen */
    LOG(llevDebug, "choose_cult_monster() mon_nr was set, but did not find a monster\n");
    return NULL;
}



/* summons a monster - the monster chosen is determined by the god
 * that is worshiiped.  return 0 on failure, 1 on success
 */

int summon_cult_monsters(object *op, int old_dir)
{
    object *mon, *otmp, *god = find_god(determine_god(op));
    int     tries = 0, i, summon_level, number, dir;
    char    buf[MAX_BUF];

    /* find deity */
    if (!god)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "You worship no living deity!");
        return 0;
    }
    else if (!god->race)
    {
        new_draw_info_format(NDI_UNIQUE, 0, op, "%s has no creatures that you may summon!", god->name);
        return 0;
    }
    /* the summon level */
    i = SK_level(op) + op->stats.Wis / 10;
    if (i == 0)
        i = 1;
    summon_level = random_roll(0, i - 1);
    if (op->path_attuned & PATH_CONJURATION)
        summon_level += 5;
    if (op->path_repelled & PATH_CONJURATION)
        summon_level -= 5;

    do
    {
        /* Need to set dir each time, as it may get clobbered */
        dir = old_dir;
        mon = choose_cult_monster(op, god, summon_level);
        tries++;

        /* As per note in choose_cult_monster, if we have multiple race
         * entries, we should really try again.
         */
        if (!mon)
        {
            new_draw_info_format(NDI_UNIQUE, 0, op, "%s fails to send anything.", god->name);
            return 0;
        }
        /* Now lets see if we can find a place for this monster. */
        if (!dir)
            dir = find_free_spot(mon->arch, op->map, op->x, op->y, 1, SIZEOFFREE);

        /* This only checks for the head of the monster.  We still need
         * to check for the body.  But if there is no space for the
         * head, trying a different monster won't help, so might as well
         * return now.
         */
        if ((dir == -1) || arch_blocked(mon->arch, mon, op->map, op->x + freearr_x[dir], op->y + freearr_y[dir]))
        {
            dir = -1;
            if (tries == 5)
            {
                new_draw_info(NDI_UNIQUE, 0, op, "There is something in the way.");
                return 0;
            }
            else
                continue;  /* Try a different monster */
        }
    }
    while (dir == -1);

    /* Aha - we have found a monster - lets customize it an put it on the
     * map.
     */

    if (mon->level > (summon_level / 2))
        number = random_roll(1, 2);
    else
        number = random_roll(2, 2);

    for (i = 1; i < number + 1; i++)
    {
        object *head;

        /* this allows multisq monsters to be done right */
        if (!(head = fix_summon_pet(mon->arch, op, dir, 0)))
            continue;

        /* now, a little bit of tailoring. If the monster is much lower in
         * level than the summon_level, we get a better monster */

        if ((head->level + 5) < summon_level)
        {
            int ii;

            for (ii = summon_level - (head->level) - 5; ii > 0; ii--)
            {
                switch (random_roll(1, 3))
                {
                    case 1:
                      head->stats.wc--;
                      break;
                    case 2:
                      head->stats.ac--;
                      break;
                    case 3:
                      head->stats.dam += 3;
                      break;
                    default:
                      break;
                }
                head->stats.hp += 3;
            }

            head->stats.maxhp = head->stats.hp;
            for (otmp = head; otmp; otmp = otmp->more)
            {
                if (otmp->name)
                {
                    if (summon_level > 30 + head->level)
                        sprintf(buf, "Arch %s of %s", head->name, god->name);
                    else
                        sprintf(buf, "%s of %s", head->name, god->name);

                    FREE_AND_COPY_HASH(otmp->name, buf);
                }
            }
        } /* if monster level is much less than character level */

        head = insert_ob_in_map(head, op->map, op, 0);

        if (head != NULL && head->randomitems != NULL)
        {
            object *tmp;
            create_treasure_list(head->randomitems, head, GT_APPLY, 6, ART_CHANCE_UNSET, 0);
            for (tmp = head->inv; tmp != NULL; tmp = tmp->below)
                if (!tmp->nrof)
                    SET_FLAG(tmp, FLAG_NO_DROP);
        }
        dir = absdir(dir + 1);
        if (arch_blocked(mon->arch, mon, op->map, op->x + freearr_x[dir], op->y + freearr_y[dir]))
        {
            if (i < number)
            {
                new_draw_info(NDI_UNIQUE, 0, op, "There is something in the way,");
                new_draw_info(NDI_UNIQUE, 0, op, "no more monsters for this casting.");
                return 1;
            }
        }
    } /* Loop to insert all the monster */
    return 1;
}

/* summon_avatar() - taken from the code which summons golems. We
 * cant use because we need to throw in a few extra's here. b.t.
 */

int summon_avatar(object *op, object *caster, int dir, archetype *at, int spellnum)
{
    object *tmp;
    char    buf[MAX_BUF];
    object *god = find_god(determine_god(caster));

    if (god)
        at = determine_holy_arch(god, (spellnum == SP_HOLY_SERVANT) ? "holy servant" : "avatar");
    else
    {
        new_draw_info(NDI_UNIQUE, 0, op, "You must worship a god first.");
        return 0;
    }
    if (!at)
    {
        new_draw_info_format(NDI_UNIQUE, 0, op, "%s has no %s for you to call.", god->name,
                             spellnum == SP_SUMMON_AVATAR ? "avatar" : "servant");
        return 0;
    }

    /* safety checks... */
    if (op->type == PLAYER)
        if (CONTR(op)->golem != NULL && !OBJECT_FREE(CONTR(op)->golem))
        {
            control_golem(CONTR(op)->golem, dir);
            return 0;
        }
    if (!dir)
        dir = find_free_spot(at, op->map, op->x, op->y, 1, SIZEOFFREE);

    if ((dir == -1) || arch_blocked(at, NULL, op->map, op->x + freearr_x[dir], op->y + freearr_y[dir]))
    {
        new_draw_info(NDI_UNIQUE, 0, op, "There is something in the way.");
        return 0;
    }

    if (!(tmp = fix_summon_pet(at, op, dir, GOLEM)))
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Your spell fails.");
        return 0;
    }

    /*  This sets the level dependencies on dam, wc and hp */
    /* ADD POSITIVE WC ADD HERE */
    tmp->stats.wc -= SP_level_strength_adjust(op, caster, spellnum);
    tmp->stats.hp += spells[spellnum].bdur + 12 * SP_level_strength_adjust(op, caster, spellnum);
    tmp->stats.dam += spells[spellnum].bdam + (2 * SP_level_dam_adjust(op, caster, spellnum));
    if (tmp->stats.dam < 0)
        tmp->stats.dam = 127;  /*seen this go negative!*/
    if (tmp->other_arch)
        tmp->other_arch = NULL;

    /* tailor it to the gods nature */
    if (tmp)
    {
        object *tmp2;
        for (tmp2 = tmp; tmp2; tmp2 = tmp2->more)
        {
            sprintf(buf, "%s of %s", spellnum == SP_SUMMON_AVATAR ? "Avatar" : "Servant", god->name);
            FREE_AND_COPY_HASH(tmp2->name, buf);
        }
    }
    memcpy(tmp->resist, god->resist, sizeof(tmp->resist));
    FREE_AND_CLEAR_HASH2(tmp->race);
    FREE_AND_CLEAR_HASH2(tmp->slaying);
    if (god->race)
        FREE_AND_COPY_HASH(tmp->race, god->race);
    if (god->slaying)
        FREE_AND_COPY_HASH(tmp->slaying, god->slaying);

    /*  make experience increase in proportion to the strength of
     *  the summoned creature. */
    tmp->stats.exp *= SP_level_spellpoint_cost(op, caster, spellnum) / spells[spellnum].sp;
    tmp->speed_left = -1;
    tmp->x = op->x + freearr_x[dir],tmp->y = op->y + freearr_y[dir];
    tmp->direction = dir;
    if (op->type == PLAYER)
    {
        /* give the player control of the golem */
        send_golem_control(tmp, GOLEM_CTR_ADD);
        CONTR(op)->golem = tmp;
    }
    insert_ob_in_map(tmp, op->map, op, 0);
    return 1;
}

/* fix_summon_pet() - this makes multisquare/single square monsters
 * properly for map insertion.
 */
object * fix_summon_pet(archetype *at, object *op, int dir, int type)
{
    LOG(llevBug, "BUG: unimplemented function 'fix_summon_pet()' called\n");
    return 0;
    /* TODO: update for the new AI and pet systems. Gecko 2006-04-30 */
#if 0
    archetype      *atmp;
    object*tmp =    NULL, *prev = NULL, *head = NULL;

    for (atmp = at; atmp != NULL; atmp = atmp->more)
    {
        tmp = arch_to_object(atmp);
        if (atmp == at)
        {
            if (type != GOLEM)
                SET_FLAG(tmp, FLAG_MONSTER);
            set_owner(tmp, op);
            if (op->type == PLAYER)
            {
                tmp->stats.exp = 0;
                SET_FLAG(tmp, FLAG_FRIENDLY);
                if (type == GOLEM)
                    CLEAR_FLAG(tmp, FLAG_MONSTER);
            }
            else
            {
                if (QUERY_FLAG(op, FLAG_FRIENDLY))
                {
                    object *owner   = get_owner(op);
                    if (owner != NULL)
                    {
                        /* For now, we transfer ownership */
                        set_owner(tmp, owner);
                        tmp->move_type = PETMOVE;
                        SET_FLAG(tmp, FLAG_FRIENDLY);
                    }
                }
            }
            if (op->type != PLAYER || type != GOLEM)
            {
                tmp->move_type = PETMOVE;
                tmp->speed_left = -1;
                tmp->type = 0;
                update_npc_knowledge(tmp->enemy, op->enemy, FRIENDSHIP_ATTACK, 0);
            }
            else
                tmp->type = GOLEM;
        }
        if (head == NULL)
            head = tmp;
        tmp->x = op->x + freearr_x[dir] + tmp->arch->clone.x;
        tmp->y = op->y + freearr_y[dir] + tmp->arch->clone.y;
        tmp->map = op->map;
        if (head != tmp)
            tmp->head = head, prev->more = tmp;
        prev = tmp;
    }
    head->direction = dir;

    /* need to change some monster attr to prevent problems/crashing */
    if (head->last_heal)
        head->last_heal = 0;
    if (head->last_eat)
        head->last_eat = 0;
    if (head->last_grace)
        head->last_grace = 0;
    if (head->last_sp)
        head->last_sp = 0;
    if (head->other_arch)
        head->other_arch = NULL;
    if (QUERY_FLAG(head, FLAG_CHANGING))
        CLEAR_FLAG(head, FLAG_CHANGING);
    if (QUERY_FLAG(head, FLAG_STAND_STILL))
        CLEAR_FLAG(head, FLAG_STAND_STILL);
    if (QUERY_FLAG(head, FLAG_GENERATOR))
        CLEAR_FLAG(head, FLAG_GENERATOR);
    if (QUERY_FLAG(head, FLAG_SPLITTING))
        CLEAR_FLAG(head, FLAG_SPLITTING);

    return head;
#endif
}

/* cast_consecrate() - a spell to make an altar your god's */

int cast_consecrate(object *op)
{
    char    buf[MAX_BUF];

    object *tmp, *god = find_god(determine_god(op));

    if (!god)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "You can't consecrate anything if you don't worship a god!");
        return 0;
    }

    for (tmp = op->below; tmp; tmp = tmp->below)
    {
        if (tmp->type == FLOOR)
            break;
        if (tmp->type == HOLY_ALTAR)
        {
            /* We use SK_level here instead of path_level mod because I think
               * all the gods should give equal chance of re-consecrating altars
               */
            if (tmp->level > SK_level(op))
            {
                new_draw_info_format(NDI_UNIQUE, 0, op, "You are not poweful enough to reconsecrate the %s", tmp->name);
                return 0;
            }
            else
            {
                /* If we got here, we are consecrating an altar */
                sprintf(buf, "%s of %s", tmp->arch->clone.name, god->name);
                FREE_AND_COPY_HASH(tmp->name, buf);
                tmp->level = SK_level(op);
                tmp->other_arch = god->arch;
                if (op->type == PLAYER)
                    esrv_update_item(UPD_NAME, op, tmp);
                new_draw_info_format(NDI_UNIQUE, 0, op, "You consecrated the altar to %s!", god->name);
                return 1;
            }
        }
    }
    new_draw_info(NDI_UNIQUE, 0, op, "You are not standing over an altar!");
    return 0;
}


/* finger_of_death() - boss high-level cleric spell. */

int finger_of_death(object *op, object *caster, int dir)
{
    object *hitter, *target = get_pointed_target(op, dir);
    int     success = 1;

    if (!target || QUERY_FLAG(target, FLAG_CAN_REFL_SPELL))
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Nothing happens.");
        return 0;
    }

    /* we create a hitter object -- the spell */
    hitter = get_archetype("face_of_death");
    hitter->level = casting_level(caster, SP_FINGER_DEATH);
		
    set_owner(hitter, op);
    hitter->x = target->x;
    hitter->y = target->y;
    hitter->stats.maxhp = hitter->count; /*??*/

    /* there are 'grave' consequences for using this spell on the unliving! */
    if (QUERY_FLAG(target, FLAG_UNDEAD))
    {
        success = 0;
        if (random_roll(0, 2))
        {
            new_draw_info(NDI_UNIQUE, 0, op, "Idiot! Your spell boomerangs!");
            hitter->x = op->x;
            hitter->y = op->y;
        }
        else
        {
            new_draw_info_format(NDI_UNIQUE, 0, op, "The %s looks stronger!", query_name(target));
            target->stats.hp = target->stats.maxhp * 2;
            return 0;
        }
    }

    insert_ob_in_map(hitter, op->map, op, 0);

    return success;
}

/* animate_weapon -
 * Generalization of staff_to_snake.  Makes a golem out of the caster's weapon.
 * The golem is based on the archetype specified, modified by the caster's level
 * and the attributes of the weapon.  The weapon is inserted in the golem's
 * inventory so that it falls to the ground when the golem dies.    -- DAMN */

/* WARNING - include out_of_map()  check here AND adjust blocked for the new arch we
 * perhaps generate
 */

int animate_weapon(object *op, object *caster, int dir, archetype *at, int spellnum)
{
    LOG(llevBug, "BUG: unimplemented spell 'animate_weapon'\n");
    return 0;
    /* TODO: update for the new AI and pet systems. Gecko 2006-04-30 */
#if 0
    object *weapon, *tmp;
    char    buf[MAX_BUF];
    int     a, i, j;
    int     magic;

    if (!at)
    {
        LOG(llevBug, "BUG: animate_weapon failed: missing archetype (%s - %s)!\n", query_name(op), query_name(caster));
        return 0;
    }
    /* if player already has a golem, abort */
    if (op->type == PLAYER)
        if (CONTR(op)->golem != NULL && !OBJECT_FREE(CONTR(op)->golem))
        {
            control_golem(CONTR(op)->golem, dir);
            return 0;
        }
    if (op->type != PLAYER)
        return 0;  /* exit if it's not a player using this spell. */
    /* if no direction specified, pick one */
    if (!dir)
        dir = find_free_spot(NULL, op->map, op->x, op->y, 1, 9);
    /* if there's no place to put the golem, abort */
    if ((dir == -1) || blocked(op, op->map, op->x + freearr_x[dir], op->y + freearr_y[dir], op->terrain_flag))
    {
        new_draw_info(NDI_UNIQUE, 0, op, "There is something in the way.");
        return 0;
    }

    if (spellnum == SP_DANCING_SWORD)
    {
        archetype  *weapon_at   = find_archetype("sword");
        if (weapon_at)
        {
            weapon = &(weapon_at->clone);
        }
        else
        {
            LOG(llevBug, "BUG: animate_weapon failed: missing archetype! (%s - %s)!\n", query_name(op),
                query_name(caster));
            return 0;
        }
    }
    else
    {
        /* get the weapon to transform */
        for (weapon = op->inv; weapon; weapon = weapon->below)
            if (weapon->type == WEAPON && QUERY_FLAG(weapon, FLAG_APPLIED))
                break;
        if (!weapon)
        {
            if (op->type == PLAYER)
            {
                new_draw_info(NDI_UNIQUE, 0, op, "You need to wield a weapon to animate it.");
            }
            return 0;
        }
        else if (spellnum == SP_STAFF_TO_SNAKE && weapon->name != shstr_cons.quarterstaff)
        {
            if (op->type == PLAYER)
            {
                new_draw_info(NDI_UNIQUE, 0, op, "The spell fails to transform your weapon.");
            }
            return 0;
        }
        else if (op->type == PLAYER && (QUERY_FLAG(weapon, FLAG_CURSED) || QUERY_FLAG(weapon, FLAG_DAMNED)))
        {
            new_draw_info(NDI_UNIQUE, 0, op, "You can't animate it.  It won't let go of your hand.");
            return 0;
        }
    }
    magic = weapon->magic > 0 ? weapon->magic : -1 * weapon->magic;

    /* create the golem object */
    tmp = arch_to_object(at);

    /* if animated by a player, give the player control of the golem */
    if (op->type == PLAYER)
    {
        CLEAR_FLAG(tmp, FLAG_MONSTER);
        SET_FLAG(tmp, FLAG_FRIENDLY);
        tmp->stats.exp = 0;
        tmp->type = GOLEM;
        set_owner(tmp, op);
        CONTR(op)->golem = tmp;
        send_golem_control(tmp, GOLEM_CTR_ADD);
    }
    else
    {
        /* If spell is cast by a pet, and the weapon is not cursed, make the animated
         * weapon a pet. */
        if (QUERY_FLAG(op, FLAG_FRIENDLY) && !QUERY_FLAG(weapon, FLAG_CURSED) && !QUERY_FLAG(weapon, FLAG_DAMNED))
        {
            object *owner   = get_owner(op);
            if (owner != NULL)
            {
                set_owner(tmp, owner);
                tmp->move_type = PETMOVE;
                SET_FLAG(tmp, FLAG_FRIENDLY);
            }
        }
        /* otherwise, make the golem an enemy */
        SET_FLAG(tmp, FLAG_MONSTER);
    }

    /* ok, tailor the golem's characteristics based on the weapon */
    if (spellnum == SP_STAFF_TO_SNAKE || spellnum == SP_ANIMATE_WEAPON)
    {
        if (apply_special(op, weapon, AP_UNAPPLY | AP_IGNORE_CURSE | AP_NO_MERGE))
        {
            LOG(llevBug, "BUG: animate_weapon(): can't unapply weapon (%s - %s)!\n", query_name(op), query_name(caster));
            return 0;
        }
        remove_ob(weapon);
        insert_ob_in_ob(weapon, tmp);
        if (op->type == PLAYER)
            esrv_send_item(op, weapon);
        SET_FLAG(tmp, FLAG_USE_WEAPON);
        if (apply_special(tmp, weapon, AP_APPLY))
            LOG(llevBug, "BUG: animate_weapon(): golem can't apply weapon (%s - %s)!\n", query_name(op),
                query_name(caster));
    }

    /* modify weapon's animated wc */
    /* ADD POSITIVE WC ADD HERE */
    tmp->stats.wc = tmp->stats.wc
                  - SP_level_dam_adjust(op, caster, spellnum)
                  - 5 * weapon->stats.Dex - 2 * weapon->stats.Str
                  - magic;
    /* Modify hit points for weapon */
    tmp->stats.maxhp = tmp->stats.maxhp
                     + spells[spellnum].bdur
                     + 4 * SP_level_strength_adjust(op, caster, spellnum) + 8 * magic
                     + 12 * weapon->stats.Con;
    /* Modify weapon's damage */
    tmp->stats.dam = spells[spellnum].bdam
                   + weapon->stats.dam
                   + magic
                   + 2 * SP_level_dam_adjust(op, caster, spellnum) + 5 * weapon->stats.Str;
    /* sanity checks */
    /* ADD POSITIVE WC ADD HERE */
    if (tmp->stats.maxhp < 0)
        tmp->stats.maxhp = 10;
    tmp->stats.hp = tmp->stats.maxhp;
    if (tmp->stats.dam < 0)
        tmp->stats.dam = 127;
    /*LOG(llevDebug,"animate_weapon: wc:%d  hp:%d  dam:%d.\n", tmp->stats.wc, tmp->stats.hp, tmp->stats.dam);*/
    /* attacktype */
    /*
    if (!tmp->attacktype)
        tmp->attacktype = AT_PHYSICAL;
    */
    for (i = 0; i < NROFMATERIALS; i++)
        for (j = 0; j < NROFATTACKS; j++)
            if (weapon->material & (1 << i))
            {
                /* There was code here to try to even out the saving
                 * throws.  This is probably not ideal, but works
                 * for the time being.
                 */
                if (material[i].save[j] < 3)
                    tmp->resist[j] = 40;
                else if (material[i].save[j] > 14)
                    tmp->resist[j] = -50;
            }

    /* Set weapon's immunity */
    tmp->resist[ATNR_CONFUSION] = 100;
    tmp->resist[ATNR_POISON] = 100;
    tmp->resist[ATNR_SLOW] = 100;
    tmp->resist[ATNR_PARALYZE] = 100;
    tmp->resist[ATNR_FEAR] = 100;
    tmp->resist[ATNR_DEATH] = 100;

    /* Improve weapon's armour value according to best save vs. physical of its material */
    for (a = 0,i = 0; i < NROFMATERIALS; i++)
    {
        if (weapon->material & (1 << i) && material[i].save[0] > a)
        {
            a = material[i].save[0];
        }
    }
    tmp->resist[ATNR_PHYSICAL] = 100
                               - (int)
                                 ((100.0 - (float) tmp->resist[ATNR_PHYSICAL])
                                / (30.0 - 2.0 * (a > 14 ? 14.0 : (float) a)));
    LOG(llevDebug, "animate_weapon: slaying %s\n", tmp->slaying ? tmp->slaying : "nothing");

    /* Determine golem's speed */
    /* This formula is apparently broken.  I write a replacement. */
    /*  tmp->speed = ((weapon->last_sp>0.1)?weapon->last_sp:0.1) -
      (((weapon->last_sp>0.1)?weapon->last_sp:0.1)/
       ( (1.0 * (float)SP_level_dam_adjust(op, caster, spellnum) +
    1.5 * (float)weapon->stats.Dex +
    2.0 * (float)weapon->stats.Str +
    3.0 * (float)magic +
    5.0 * (float)weapon->stats.exp) )
       * ((float)((tmp->weight>1000)? tmp->weight : 1000)
       / ((float)(weapon->weight>1000)? weapon->weight : 1000))); */

    /* replacement  */
    tmp->speed = 0.4f + 0.1f * (float) SP_level_dam_adjust(op, caster, spellnum);

    if (tmp->speed > 3.33f)
        tmp->speed = 3.33f;
    LOG(llevDebug, "animate_weapon: armour:%d  speed:%f  exp:%d.\n", tmp->resist[ATNR_PHYSICAL], tmp->speed,
        tmp->stats.exp);

    /* spell-dependent finishing touches and descriptive text */
    switch (spellnum)
    {
        case SP_STAFF_TO_SNAKE:
          new_draw_info(NDI_UNIQUE, 0, op, "Your staff becomes a serpent and leaps to the ground!");
          break;

        case SP_ANIMATE_WEAPON:
          new_draw_info_format(NDI_UNIQUE, 0, op, "Your %s flies from your hand and hovers in mid-air!", weapon->name);
          sprintf(buf, "animated %s", weapon->name);
          FREE_AND_COPY_HASH(tmp->name, buf);

          tmp->face = weapon->face;
          tmp->animation_id = weapon->animation_id;
          tmp->anim_speed = weapon->anim_speed;
          tmp->last_anim = weapon->last_anim;
          tmp->state = weapon->state;
          if (QUERY_FLAG(weapon, FLAG_ANIMATE))
          {
              SET_FLAG(tmp, FLAG_ANIMATE);
          }
          else
          {
              CLEAR_FLAG(tmp, FLAG_ANIMATE);
          }
          update_ob_speed(tmp);
          break;

        case SP_DANCING_SWORD:
          new_draw_info(NDI_UNIQUE, 0, op, "A magical sword appears in mid air, eager to slay your foes for you!");
          break;
        default:
          break;
    }

    /*  make experience increase in proportion to the strength of the summoned creature. */
    tmp->stats.exp *= SP_level_spellpoint_cost(op, caster, spellnum) / spells[spellnum].sp;
    tmp->speed_left = -1;
    tmp->x = op->x + freearr_x[dir],tmp->y = op->y + freearr_y[dir];
    tmp->direction = dir;
    insert_ob_in_map(tmp, op->map, op, 0);
    return 1;
#endif
}


int cast_daylight(object *op)
{
    return 0;
}


int cast_nightfall(object *op)
{
    return 0;
}

/* cast_faery_fire() - this spell primary purpose is to light
 * up all single-space monsters on a map. Magic immune and
 * multi-space monsters are currently not supposed to light
 * up. If USE_LIGHTING is not defined, this spell is only
 * capable of doing minor fire damage. I hacked this out of
 * the destruction code. -b.t.
 */

int cast_faery_fire(object *op, object *caster)
{
    int         r, dam, i, j, success = 0, factor, xt, yt;
    object     *tmp;
    mapstruct  *m;

    if (op->type != PLAYER)
        return 0;
    /* the smaller this is, the longer it glows */
    factor = spells[SP_FAERY_FIRE].bdur + SP_level_strength_adjust(op, caster, SP_FAERY_FIRE);
    r = spells[SP_FAERY_FIRE].bdam + SP_level_dam_adjust(op, caster, SP_FAERY_FIRE);
    r = 5;
    factor = 10;
    dam = (SK_level(op) / 10) + 1;
    for (i = -r; i < r; i++)
        for (j = -r; j < r; j++)
        {
            xt = op->x + i;
            yt = op->y + i;
            if (!(m = out_of_map(op->map, &xt, &yt)))
                continue;
            tmp = get_map_ob(m, xt, yt);
            while (tmp
                != NULL
                && (!QUERY_FLAG(tmp, FLAG_ALIVE)
                 || tmp->type == PLAYER
                 || tmp->more
                 || tmp->head))
                tmp = tmp->above;
            if (tmp == NULL)
                continue;
            if (make_object_glow(tmp, 1, factor))
            {
                object *effect  = get_archetype("detect_magic");
                success++;
                if (effect)
                {
                    effect->x = tmp->x;
                    effect->y = tmp->y;
                    insert_ob_in_map(effect, op->map, op, 0);
                }
            }
        }
    return success;
}

/* make_object_glow() - currently only makes living objects glow.
 * we do this by creating a "force" and inserting it in the
 * object. if time is 0, the object glows permanently. To truely
 * make this work for non-living objects, we would have to
 * give them the capability to have an inventory. b.t.
 */
/* outdated too with new lightning system - MT 2004 */
int make_object_glow(object *op, int radius, int time)
{
    object *tmp;

    /* some things are unaffected... */
    if (op->path_denied & PATH_LIGHT)
        return 0;

    tmp = get_archetype("force");
    tmp->speed = 0.000001f * (float) time;
    tmp->x = op->x,tmp->y = op->y;
    if (tmp->speed < MIN_ACTIVE_SPEED)
        tmp->speed = MIN_ACTIVE_SPEED; /* safety */
    tmp = insert_ob_in_ob(tmp, op);

    if (!tmp->env || op != tmp->env)
    {
        LOG(llevBug, "BUG: make_object_glow() failed to insert glowing force in %s\n", query_name(op));
        return 0;
    }
    return 1;
}


/*  cast_cause_disease:  this spell looks along <dir> from the
player and infects someone. */

int cast_cause_disease(object *op, object *caster, int dir, archetype *disease_arch, int type)
{
    int         x, y, i, xt, yt;
    object     *walk;
    mapstruct  *m;

    x = op->x;  y = op->y;

    /* search in a line for a victim */
    for (i = 0; i < 5; i++)
    {
        x += freearr_x[dir]; y += freearr_y[dir];
        xt = x;yt = y;
        if (!(m = out_of_map(op->map, &xt, &yt)))
            continue;
        /* search this square for a victim */
        for (walk = get_map_ob(m, xt, yt); walk; walk = walk->above)
            if (QUERY_FLAG(walk, FLAG_MONSTER) || !(walk->type == PLAYER))
            {
                /* found a victim */

                object *disease = arch_to_object(disease_arch);
                set_owner(disease, op);
                disease->stats.exp = 0;
                disease->level = op->level;

                /* Try to get the experience into the correct category */
                if (op->chosen_skill && op->chosen_skill->exp_obj)
                    disease->exp_obj = op->chosen_skill->exp_obj;

                /*do level adjustments */
                if (disease->stats.wc)
                    disease->stats.wc += SP_level_strength_adjust(op, caster, type) / 2;

                if (disease->magic > 0)
                    disease->magic += SP_level_strength_adjust(op, caster, type) / 4;

                if (disease->stats.maxhp > 0)
                    disease->stats.maxhp += SP_level_strength_adjust(op, caster, type);

                if (disease->stats.maxgrace > 0)
                    disease->stats.maxgrace += SP_level_strength_adjust(op, caster, type);

                if (disease->stats.dam)
                {
                    if (disease->stats.dam > 0)
                        disease->stats.dam += SP_level_dam_adjust(op, caster, type);
                    else
                        disease->stats.dam -= SP_level_dam_adjust(op, caster, type);
                }

                if (disease->last_sp)
                {
                    disease->last_sp -= 2 * SP_level_dam_adjust(op, caster, type);
                    if (disease->last_sp < 1)
                        disease->last_sp = 1;
                }

                if (disease->stats.maxsp)
                {
                    if (disease->stats.maxsp > 0)
                        disease->stats.maxsp += SP_level_dam_adjust(op, caster, type);
                    else
                        disease->stats.maxsp -= SP_level_dam_adjust(op, caster, type);
                }

                if (disease->stats.ac)
                    disease->stats.ac += SP_level_dam_adjust(op, caster, type);

                if (disease->last_eat)
                    disease->last_eat -= SP_level_dam_adjust(op, caster, type);

                if (disease->stats.hp)
                    disease->stats.hp -= SP_level_dam_adjust(op, caster, type);

                if (disease->stats.sp)
                    disease->stats.sp -= SP_level_dam_adjust(op, caster, type);


                if (infect_object(walk, disease, 1))
                {
                    char    buf[128];
                    object *flash;  /* visual effect for inflicting disease */
                    sprintf(buf, "You inflict %s on %s!", disease->name, walk->name);
                    new_draw_info(NDI_UNIQUE, 0, op, buf);
                    flash = get_archetype("detect_magic");
                    flash->x = xt;
                    flash->y = yt;
                    flash->map = walk->map;
                    insert_ob_in_map(flash, walk->map, op, 0);
                    return 1;
                }
            }
        /* no more infecting through walls -
              * we will use PASS_THRU but P_NO_PASS only will stop us
              */
        if ((wall(m, xt, yt) & (~(P_NO_PASS | P_PASS_THRU| P_PASS_ETHEREAL))) == P_NO_PASS)
            return 0;
    }
    new_draw_info(NDI_UNIQUE, 0, op, "No one caught anything!");
    return 0;
}



/* move aura function.  An aura is a part of someone's inventory,
which he carries with him, but which acts on the map immediately
around him.
Aura parameters:
food:  duration counter.
attacktype:  aura's attacktype
other_arch:  archetype to drop where we attack

 */

void move_aura(object *aura)
{
    int         i;
    object     *env, *new_ob;
    mapstruct  *m;
    int         nx, ny;

    /* auras belong in inventories */
    env = aura->env;

    /* no matter what we've gotta remove the aura...
       we'll put it back if its time isn't up.  */
    remove_ob(aura);

    /* exit if we're out of gas */
    if (aura->stats.food-- < 0)
        return;

    /* auras only exist in inventories */
    if (env == NULL || env->map == NULL)
        return;

    aura->x = env->x;
    aura->y = env->y;

    /* we need to jump out of the inventory for a bit
       in order to hit the map conveniently. */
    if (!insert_ob_in_map(aura, env->map, aura, 0))
        return;
    for (i = 1; i < 9; i++)
    {
        hit_map(aura, i);
        if (aura->other_arch)
        {
            nx = aura->x + freearr_x[i];
            ny = aura->y + freearr_y[i];
            /* we're done if the "i" square next to us is full */
            if (!(m = out_of_map(aura->map, &nx, &ny)))
                continue;
            if (wall(m, nx, ny))
                continue;
            new_ob = arch_to_object(aura->other_arch);
            new_ob->x = nx;
            new_ob->y = ny;
            insert_ob_in_map(new_ob, m, aura, 0);
        }
    }
    /* put the aura back in the player's inventory */
    remove_ob(aura);
    insert_ob_in_ob(aura, env);
}



void move_peacemaker(object *op)
{
    LOG(llevBug, "BUG: unimplemented function 'move_peacemaker'\n");
    /* TODO: update for the new AI and pet systems. Gecko 2006-04-30 */
#if 0
    object *tmp;
    char    buf[MAX_BUF];


    for (tmp = get_map_ob(op->map, op->x, op->y); tmp != NULL; tmp = tmp->above)
    {
        int     atk_lev, def_lev;
        object *victim  = tmp;
        if (tmp->head)
            victim = tmp->head;
        if (!QUERY_FLAG(victim, FLAG_MONSTER))
            continue;
        if (QUERY_FLAG(victim, FLAG_UNAGGRESSIVE))
            continue;
        if (victim->stats.exp == 0)
            continue;
        def_lev = MAX(1, victim->level);
        atk_lev = MAX(1, op->level);
        if (random_roll(0, atk_lev - 1) > def_lev)
        {
            /* make this sucker peaceful. */
            victim->stats.dam = 0;
            add_exp(op->owner, victim->stats.exp, op->chosen_skill->stats.sp);
            victim->stats.exp = 0;
            victim->stats.sp = 0;
            victim->stats.grace = 0;
            victim->stats.Pow = 0;
            victim->move_type = RANDO2;
            SET_FLAG(victim, FLAG_UNAGGRESSIVE);
            SET_FLAG(victim, FLAG_RUN_AWAY);
            SET_FLAG(victim, FLAG_RANDOM_MOVE);
            CLEAR_FLAG(victim, FLAG_MONSTER);
            if (victim->name)
            {
                sprintf(buf, "%s no longer feels like fighting.", victim->name);
                new_draw_info(NDI_UNIQUE, 0, op->owner, buf);
            }
        }
    }
#endif
}


int cast_cause_conflict(object *op, object *caster, archetype *spellarch, int type)
{
    LOG(llevBug, "BUG: unimplemented spell 'casuse conflict'\n");
    return 0;
    /* TODO: update for the new AI and pet systems. Gecko 2006-04-30 */
#if 0
    int         i, j, xt, yt;
    int         r;    /*  peterm:  added to make area of effect level dep.  */
    int         level;
    object     *tmp;
    mapstruct  *m;

    if (op->type != PLAYER)
        return 0;
    r = 5 + SP_level_strength_adjust(op, caster, type);
    for (i = -r; i < r; i++)
        for (j = -r; j < r; j++)
        {
            xt = op->x + i;
            yt = op->y + i;
            if (!(m = out_of_map(op->map, &xt, &yt)))
                continue;
            tmp = get_map_ob(m, xt, yt);
            while (tmp != NULL && (!QUERY_FLAG(tmp, FLAG_ALIVE) || tmp->type == PLAYER))
                tmp = tmp->above;
            if (tmp == NULL)
                continue;
            if (tmp->head)
                continue;  /* only hit the head with this one */

            /* OK, now set the monster on other monsters */
            level = MAX(1, SK_level(caster) / 2);
            if (random_roll(0, level - 1) > tmp->level)
            {
                /* successfully induced conflict */
                char    buf[MAX_BUF];
                SET_FLAG(tmp, FLAG_BERSERK);
                if (tmp->name)
                {
                    sprintf(buf, "You've clouded %s's mind.  He turns on his friends!", tmp->name);
                    new_draw_info(NDI_RED, 0, op, buf);
                }
            }
        }
    return 1;
#endif
}


