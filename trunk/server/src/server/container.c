/*
    Daimonin, the Massive Multiuser Online Role Playing Game
    Server Applicatiom

    Copyright (C) 2001-2006 Michael Toennies

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

    The author can be reached via e-mail to info@daimonin.net
*/
#include <global.h>

/* this file manages container but also pickup and drop handling.
 * One reason is, that every object can have a inventory and
 * works "like a container". A CONTAINER is nothing more as a explicit
 * object type with inventory used to store items. But technical every
 * object is a container.
 */

/* Frees a monster trapped in container when opened by a player */
static void free_container_monster(object *monster, object *op)
{
    int     i;
    object *container   = monster->env;

    if (container == NULL)
        return;

    remove_ob(monster); /* in container, no walk off check */
    monster->x = container->x;
    monster->y = container->y;
    i = find_free_spot(monster->arch, monster, op->map, monster->x, monster->y, 0, SIZEOFFREE1);
    if (i != -1)
    {
        monster->x += freearr_x[i];
        monster->y += freearr_y[i];
    }
    fix_monster(monster);
    if (insert_ob_in_map(monster, op->map, monster, 0))
        new_draw_info_format(NDI_UNIQUE, 0, op, "A %s jumps out of the %s.", query_name(monster), query_name(container));
}


/* a player has opened a container - link him to the
* list of player which have (perhaps) it opened too.
*/
int container_link(player *const pl, object *const sack)
{
    int ret = 0;

    /* for safety reasons, lets check this is valid */
    if (sack->attacked_by)
    {
        if (sack->attacked_by->type != PLAYER
            || !CONTR(sack->attacked_by)
            || CONTR(sack->attacked_by)->container != sack)
        {
            LOG(llevBug, "BUG: container_link() - invalid player linked: <%s>\n", query_name(sack->attacked_by));
            sack->attacked_by = NULL;
        }
    }

    /* the open/close logic should be handled elsewhere.
    * for that reason, this function should only be called
    * when valid - broken open/close logic elsewhere is bad.
    * so, give a bug warning out!
    */
    if (pl->container)
    {
        LOG(llevBug, "BUG: container_link() - called from player with open container!: <%s> sack:<%s>\n",
            query_name(sack->attacked_by), query_name(sack));
        container_unlink(pl, sack);
    }

    /* ok, at this point we are SURE a player opens this container.
    * here we kick in the check for quest item given for container access-
    * = container (apply) event.
    */
    if(sack->event_flags & EVENT_FLAG_SPECIAL_QUEST)
        check_cont_quest_event(pl->ob, sack);

    pl->container = sack;
    pl->container_count = sack->count;

    pl->container_above = sack->attacked_by;

    if (sack->attacked_by)
        CONTR(sack->attacked_by)->container_below = pl->ob;
    else /* we are the first one opening this container */
    {
        SET_FLAG(sack, FLAG_APPLIED);
        if (sack->other_arch) /* faking open container face */
        {
            sack->face = sack->other_arch->clone.face;
            sack->animation_id = sack->other_arch->clone.animation_id;
            if (sack->animation_id)
                SET_ANIMATION(sack, (NUM_ANIMATIONS(sack) / NUM_FACINGS(sack)) * sack->direction);
            update_object(sack, UP_OBJ_FACE);
        }
        esrv_update_item(UPD_FLAGS | UPD_FACE, pl->ob, sack);
        container_trap(pl->ob, sack);   /* search & explode a rune in the container */
        ret = 1;
    }

    esrv_send_inventory(pl->ob, sack);
    pl->container_below = NULL; /* we are first element */
    sack->attacked_by = pl->ob;
    sack->attacked_by_count = pl->ob->count;

    return ret;
}

/* remove a player from the container list.
* unlink is a bit more tricky - pl OR sack can be NULL.
* if pl == NULL, we unlink ALL players from sack.
* if sack == NULL, we unlink the current container from pl.
*/
int container_unlink(player *const pl, object *sack)
{
    object *tmp, *tmp2;

    if (pl == NULL && sack == NULL)
    {
        LOG(llevBug, "BUG: container_unlink() - *pl AND *sack == NULL!\n");
        return 0;
    }

    if (pl)
    {
        if (!pl->container)
            return 0;

        if (pl->container->count != pl->container_count)
        {
            pl->container = NULL;
            pl->container_count = 0;
            return 0;
        }

        sack = pl->container;
        update_object(sack, UP_OBJ_FACE);
        esrv_close_container(pl->ob);
        /* ok, there is a valid container - unlink the player now */
        if (!pl->container_below && !pl->container_above) /* we are only applier */
        {
            if (pl->container->attacked_by != pl->ob) /* we should be that object... */
            {
                LOG(llevBug, "BUG: container_unlink() - container link don't match player!: <%s> sack:<%s> (%s)\n",
                    query_name(pl->ob), query_name(sack->attacked_by), query_name(sack));
                return 0;
            }

            pl->container = NULL;
            pl->container_count = 0;

            CLEAR_FLAG(sack, FLAG_APPLIED);
            if (sack->other_arch)
            {
                sack->face = sack->arch->clone.face;
                sack->animation_id = sack->arch->clone.animation_id;
                if (sack->animation_id)
                    SET_ANIMATION(sack, (NUM_ANIMATIONS(sack) / NUM_FACINGS(sack)) * sack->direction);
                update_object(sack, UP_OBJ_FACE);
            }
            sack->attacked_by = NULL;
            sack->attacked_by_count = 0;
            esrv_update_item(UPD_FLAGS | UPD_FACE, pl->ob, sack);
            return 1;
        }

        /* because there is another player applying that container, we don't close it */

        if (!pl->container_below) /* we are first player in list */
        {
            /* mark above as first player applying this container */
            sack->attacked_by = pl->container_above;
            sack->attacked_by_count = pl->container_above->count;
            CONTR(pl->container_above)->container_below = NULL;

            pl->container_above = NULL;
            pl->container = NULL;
            pl->container_count = 0;
            return 0;
        }

        /* we are somehwere in the middle or last one - it don't matter */
        CONTR(pl->container_below)->container_above = pl->container_above;
        if (pl->container_above)
            CONTR(pl->container_above)->container_below = pl->container_below;

        pl->container_below = NULL;
        pl->container_above = NULL;
        pl->container = NULL;
        pl->container_count = 0;
        return 0;
    }

    CLEAR_FLAG(sack, FLAG_APPLIED);
    if (sack->other_arch)
    {
        sack->face = sack->arch->clone.face;
        sack->animation_id = sack->arch->clone.animation_id;
        if (sack->animation_id)
            SET_ANIMATION(sack, (NUM_ANIMATIONS(sack) / NUM_FACINGS(sack)) * sack->direction);
        update_object(sack, UP_OBJ_FACE);
    }
    tmp = sack->attacked_by;
    sack->attacked_by = NULL;
    sack->attacked_by_count = 0;

    /* if we are here, we are called with (NULL,sack) */
    while (tmp)
    {
        if (!CONTR(tmp) || CONTR(tmp)->container != sack) /* valid player in list? */
        {
            LOG(llevBug, "BUG: container_unlink() - container link list mismatch!: player?:<%s> sack:<%s> (%s)\n",
                query_name(tmp), query_name(sack), query_name(sack->attacked_by));
            return 1;
        }

        tmp2 = CONTR(tmp)->container_above;
        CONTR(tmp)->container = NULL;
        CONTR(tmp)->container_count = 0;
        CONTR(tmp)->container_below = NULL;
        CONTR(tmp)->container_above = NULL;
        esrv_update_item(UPD_FLAGS | UPD_FACE, tmp, sack);
        esrv_close_container(tmp);
        tmp = tmp2;
    }
    return 1;
}

/* examine the items in a container which gets readied or opened by player .
* Explode or trigger every trap & rune in there and free trapped monsters.
*/
int container_trap(object *const op, object *const container)
{
    int     ret = 0;
    object *tmp;

    for (tmp = container->inv; tmp; tmp = tmp->below)
    {
        if (tmp->type == RUNE) /* search for traps & runes */
        {
            ret++;
            spring_trap(tmp, op);
        }
        else if (tmp->type == MONSTER) /* search for monsters living in containers */
        {
            ret++;
            free_container_monster(tmp, op);
        }
    }

    return ret;/* ret=0 -> no trap or monster found/exploded/freed */
}

/* Pick up commands follow */
/* pl = player (not always - monsters can use this now)
* op is the object to put tmp into,
* tmp is the object to pick up, nrof is the number to
* pick up (0 means all of them)
*/
static void pick_up_object(object *pl, object *op, object *tmp, uint32 nrof)
{
    /* buf needs to be big (more than 256 chars) because you can get
    * very long item names.
    */
    char    buf[HUGE_BUF];
    object *env         = tmp->env;
    uint32  tmp_nrof    = tmp->nrof ? tmp->nrof : 1;

    if (pl->type == PLAYER)
        CONTR(pl)->rest_mode = 0;

    /* IF the player is flying & trying to take the item out of a container
    * that is in his inventory, let him.  tmp->env points to the container
    * (sack, luggage, etc), tmp->env->env then points to the player (nested
    * containers not allowed as of now)
    */
    if (!QUERY_FLAG(pl, FLAG_WIZ) && is_player_inv(tmp) != pl && IS_AIRBORNE(pl))
    {
        if(QUERY_FLAG(pl, FLAG_FLYING))
            new_draw_info(NDI_UNIQUE, 0, pl, "You are flying; you can't reach the ground!");
        else
            new_draw_info(NDI_UNIQUE, 0, pl, "You are levitating; you can't reach the ground!");
        return;
    }
    if (QUERY_FLAG(tmp, FLAG_NO_DROP))
        return;

    if (nrof > tmp_nrof || nrof == 0)
        nrof = tmp_nrof;

    /* Figure out how much weight this object will add to the player */
    if (!QUERY_FLAG(pl, FLAG_WIZ) && (pl->carrying + (tmp->weight * nrof) + tmp->carrying) > CONTR(pl)->weight_limit)
    {
        object *tmp_pl = is_player_inv(tmp);

        /* last check - we allow inv to inv transfer from same player */
        if(!tmp_pl || tmp_pl != is_player_inv(op))
        {
            new_draw_info(NDI_UNIQUE, 0, pl, "That item is to heavy to pick up.");
            return;
        }
    }

    /* As usual, try to run the plugin _after_ tests, but _before_ side effects */
    /* Non-zero return value means to abort the pickup */
    if(trigger_object_plugin_event(EVENT_PICKUP, tmp, pl, op, NULL,
        (int *)&tmp_nrof, NULL, NULL, SCRIPT_FIX_ALL))
        return;

    if (tmp->type == CONTAINER)
        container_unlink(NULL, tmp);

    if (QUERY_FLAG(tmp, FLAG_UNPAID))
    {
        if (QUERY_FLAG(tmp, FLAG_NO_PICK)) /* this is a clone shop - clone a item for inventory */
        {
            tmp = ObjectCreateClone(tmp);
            CLEAR_FLAG(tmp, FLAG_NO_PICK);
            SET_FLAG(tmp, FLAG_STARTEQUIP);
            tmp->nrof = nrof;
            tmp_nrof = nrof;
            sprintf(buf, "You pick up the %s for %s from the storage.", query_name(tmp), query_cost_string(tmp, pl, F_BUY, COSTSTRING_SHORT));
        }
        else /* this is a unique shop item */
            sprintf(buf, "The %s will cost you %s.", query_name(tmp), query_cost_string(tmp, pl, F_BUY, COSTSTRING_SHORT));
    }
    else if (nrof < tmp_nrof)
        sprintf(buf,"You pick up %d out of the %s.", nrof, query_name(tmp));
    else
        sprintf(buf,"You pick up the %s.", query_name(tmp));

    if (nrof != tmp_nrof)
    {
        object*tmp2 =   tmp, *tmp2_cont = tmp->env;
        tag_t           tmp2_tag    = tmp2->count;
        tmp = get_split_ob(tmp, nrof);
        if (!tmp)
        {
            new_draw_info(NDI_UNIQUE, 0, pl, errmsg);
            return;
        }
        /* Tell a client what happened rest of objects */
        if (pl->type == PLAYER)
        {
            if (was_destroyed(tmp2, tmp2_tag))
                esrv_del_item(CONTR(pl), tmp2_tag, tmp2_cont);
            else
                esrv_send_item(pl, tmp2);
        }
    }
    else
    {
        /* If the object is in a container, send a delete to the client.
        * - we are moving all the items from the container to elsewhere,
        * so it needs to be deleted.
        */
        if (!QUERY_FLAG(tmp, FLAG_REMOVED))
        {
            int result;
            if (tmp->env && pl->type == PLAYER)
                esrv_del_item(CONTR(pl), tmp->count, tmp->env);

            /* walk_off check added 2007-01-23, Gecko */
            remove_ob(tmp);
            if ((result = check_walk_off(tmp, pl, MOVE_APPLY_VANISHED)) != CHECK_WALK_OK)
            {
                if(result == CHECK_WALK_DESTROYED)
                    sprintf(buf, "Trying to pick up the %s unfortunately destroyed it.\n", query_name(tmp));
                else
                    sprintf(buf, "You somehow lost the %s when picking it up.\n", query_name(tmp));
                new_draw_info(NDI_UNIQUE, 0, pl, buf);
                return;
            }
        }
    }

    new_draw_info(NDI_UNIQUE, 0, pl, buf);
    tmp = insert_ob_in_ob(tmp, op);

    if(op->type != PLAYER)
    {
        sprintf(buf,"You put it into %s.", query_name(op));
        new_draw_info(NDI_UNIQUE, 0, pl, buf);
    }

    /* All the stuff below deals with client/server code, and is only
    * usable by players
    */
    if (pl->type != PLAYER)
        return;

    esrv_send_item(pl, tmp);
    /* These are needed to update the weight for the container we
    * are putting the object in, and the players weight, if different.
    */
    esrv_update_item(UPD_WEIGHT, pl, op);
    if (op != pl)
        esrv_send_item(pl, pl);

    /* Update the container the object was in */
    if (env && env != pl && env != op)
        esrv_update_item(UPD_WEIGHT, pl, env);
}


/*
* Check if an item op can be put into a sack. If pl exists then tell
* a player the reason of failure.
* returns 1 if it will fit, 0 if it will not.  nrof is the number of
* objects (op) we want to put in.  We specify it separately instead of
* using op->nrof because often times, a player may have specified a
* certain number of objects to drop, so we can pass that number, and
* not need to use split_ob and stuff.
*/
int sack_can_hold(const object *const pl, const object *const sack, const object *const op, const uint32 nrof)
{
    if (!QUERY_FLAG(sack, FLAG_APPLIED))
        new_draw_info_format(NDI_UNIQUE, 0, pl, "The %s is not active.", query_short_name(sack, pl));
    else if (sack == op)
        new_draw_info_format(NDI_UNIQUE, 0, pl, "You can't put the %s into itself.", query_short_name(sack, pl));
    else if ((sack->race && (sack->sub_type1 & 1) != ST1_CONTAINER_CORPSE)
        && (sack->race != op->race || op->type == CONTAINER || (sack->stats.food && sack->stats.food != op->type)))
        new_draw_info_format(NDI_UNIQUE, 0, pl, "You can put only %s into the %s.", sack->race,
                             query_short_name(sack, pl));
    else if (op->type == SPECIAL_KEY && sack->slaying && op->slaying)
        new_draw_info_format(NDI_UNIQUE, 0, pl, "You don't want put the key into %s.", query_short_name(sack, pl));
    else
    {
        if(sack->weight_limit == 0 || (sack->weight_limit > 0 && sack->weight_limit > sack->carrying + (sint32)
        ((op->type==CONTAINER && op->weapon_speed!=1.0f)?((sint32)op->damage_round_tag+op->weight):WEIGHT_NROF(op, nrof))))
            return TRUE;

        new_draw_info_format(NDI_UNIQUE, 0, pl, "The %s is too heavy for the %s!",
                                    query_short_name(op, pl),query_name(sack));
    }

    return FALSE;
}

void pick_up(object *const op, object *const ori)
{
    int         ego_mode;
    object     *alt = ori, *tmp             = NULL;
    mapstruct  *tmp_map         = NULL;
    int         count;
    tag_t       tag;

    /* Decide which object to pick. */
    if (alt)
    {
        if (!can_pick(op, alt))
        {
            new_draw_info_format(NDI_UNIQUE, 0, op, "You can't pick up %s.", alt->name);
            goto leave;
        }
        tmp = alt;
    }
    else
    {
        if (op->below == NULL || !can_pick(op, op->below))
        {
            new_draw_info(NDI_UNIQUE, 0, op, "There is nothing to pick up here.");
            goto leave;
        }
        tmp = op->below;
    }

    /* lets check we have an ego item */
    if((ego_mode = check_ego_item(op, tmp)) )
    {
        /* only disallow handling of bound items not yours */
        if(ego_mode == EGO_ITEM_BOUND_PLAYER)
        {
            if(op->type == PLAYER)
                new_draw_info (NDI_UNIQUE, 0, op, "This is not your ego item!");
            return;
        }
    }

    if (tmp->type == CONTAINER)
        container_unlink(NULL, tmp);

    /* Try to catch it. */
    tmp_map = tmp->map;

	if (!can_pick(op, tmp))
        goto leave;

    if (op->type == PLAYER)
    {
        count = CONTR(op)->count;
        if (count == 0)
            count = tmp->nrof;
    }
    else
        count = tmp->nrof;

    /* container is open, so use it */
    if (op->type == PLAYER && CONTR(op)->container)
    {
        alt = CONTR(op)->container;
        if (alt != tmp->env && !sack_can_hold(op, alt, tmp, count))
            goto leave;
    }
    else
    {
        /* non container pickup */
        for (alt = op->inv; alt; alt = alt->below)
            if (alt->type == CONTAINER
                && QUERY_FLAG(alt, FLAG_APPLIED)
                && alt->race
                && alt->race == tmp->race
                && sack_can_hold(op, alt, tmp, count))
                break;  /* perfect match */

        if (!alt)
            for (alt = op->inv; alt; alt = alt->below)
                if (alt->type == CONTAINER && QUERY_FLAG(alt, FLAG_APPLIED) && sack_can_hold(NULL, alt, tmp, count))
                    break;  /* General container comes next */
        if (!alt)
            alt = op; /* No free containers */
    }
    if (tmp->env == alt)
    {
        /* here it could be possible to check rent,
        * if someone wants to implement it
        */
        alt = op;
    }
#ifdef PICKUP_DEBUG
    if (op->type == PLAYER)
        printf("Pick_up(): %s picks %s (%d) and inserts it %s.\n", op->name, tmp->name, CONTR(op)->count, alt->name);
    else
        printf("Pick_up(): %s picks %s and inserts it %s.\n", op->name, tmp->name, alt->name);
#endif

    /* startequip items are not allowed to be put into containers: */
    if (op->type == PLAYER && alt->type == CONTAINER && QUERY_FLAG(tmp, FLAG_STARTEQUIP))
    {
        new_draw_info(NDI_UNIQUE, 0, op, "This object cannot be put into containers!");
        goto leave;
    }

    tag = tmp->count;
    pick_up_object(op, alt, tmp, count);
    if (op->type == PLAYER)
        CONTR(op)->count = 0;

leave:;

}


/*
*  This function was part of drop, now is own function.
*  Player 'op' tries to put object 'tmp' into sack 'sack',
*  if nrof is non zero, then nrof objects is tried to put into sack.
*/
void put_object_in_sack(object *const op, object *const sack, object *tmp, const uint32 nrof)
{
    int     ego_mode;
    tag_t   tmp_tag, tmp2_tag;
    object *tmp2, *tmp_cont;
    /*object *sack2;*/
    char    buf[MAX_BUF];

    if (op->type != PLAYER)
    {
        LOG(llevDebug, "put_object_in_sack: op not a player\n");
        return;
    }

    /* lets check we have an ego item */
    if((ego_mode = check_ego_item(op, tmp)) || (!ego_mode && check_ego_item(op, sack)) )
    {
        /* only disallow handling of bound items not yours */
        if(ego_mode == EGO_ITEM_BOUND_PLAYER)
        {
            if(op->type == PLAYER)
                new_draw_info (NDI_UNIQUE, 0, op, "This is not your ego item!");
            return;
        }
    }
    if (sack == tmp)
        return;

    if (check_magical_container(tmp,sack))
    {
        if(op->type == PLAYER)
            new_draw_info (NDI_UNIQUE, 0, op, "You can't put a magical container in another!");
        return; /* Can't put an object in itself */
    }

    if (sack->type != CONTAINER)
    {
        new_draw_info_format(NDI_UNIQUE, 0, op, "The %s is not a container.", query_name(sack));
        return;
    }

    if (tmp->type == CONTAINER)
        container_unlink(NULL, tmp);

    if (!sack_can_hold(op, sack, tmp, (nrof ? nrof : tmp->nrof)))
        return;

    if (QUERY_FLAG(tmp, FLAG_APPLIED))
    {
        if (apply_special(op, tmp, AP_UNAPPLY | AP_NO_MERGE))
            return;
    }

    /* FIXME: There's quite some code duplicated from pick_up_object here */
    /* we want to put some portion of the item into the container */
    if (nrof && tmp->nrof != nrof)
    {
        object*tmp2 =   tmp, *tmp2_cont = tmp->env;
        tmp2_tag = tmp2->count;
        tmp = get_split_ob(tmp, nrof);

        if (!tmp)
        {
            new_draw_info(NDI_UNIQUE, 0, op, errmsg);
            return;
        }
        /* Tell a client what happened other objects */
        if (was_destroyed(tmp2, tmp2_tag))
            esrv_del_item(CONTR(op), tmp2_tag, tmp2_cont);
        else    /* this can proably be replaced with an update */
            esrv_send_item(op, tmp2);
    }
    else if (!QUERY_FLAG(tmp, FLAG_UNPAID) || !QUERY_FLAG(tmp, FLAG_NO_PICK))
    {
        remove_ob(tmp);
        if (check_walk_off(tmp, NULL, MOVE_APPLY_VANISHED) != CHECK_WALK_OK)
            return;
    }
    else /* don't delete clone shop objects - clone them!*/
        tmp = ObjectCreateClone(tmp);

    if (QUERY_FLAG(tmp, FLAG_UNPAID))
    {
        if (QUERY_FLAG(tmp, FLAG_NO_PICK)) /* this is a clone shop - clone a item for inventory */
        {
            CLEAR_FLAG(tmp, FLAG_NO_PICK);
            SET_FLAG(tmp, FLAG_STARTEQUIP);
            sprintf(buf, "You pick up %s for %s from the storage.", query_name(tmp), query_cost_string(tmp, op, F_BUY, COSTSTRING_SHORT));
        }
        else /* this is a unique shop item */
            sprintf(buf, "%s will cost you %s.", query_name(tmp), query_cost_string(tmp, op, F_BUY, COSTSTRING_SHORT));
        new_draw_info(NDI_UNIQUE, 0, op, buf);
    }

    sprintf(buf, "You put the %s into ", query_name(tmp));
    strcat(buf, query_name(sack));
    strcat(buf, ".");
    tmp_tag = tmp->count;
    tmp_cont = tmp->env;
    tmp2 = insert_ob_in_ob(tmp, sack);
    new_draw_info(NDI_UNIQUE, 0, op, buf);
    FIX_PLAYER(op, "put_object_in_sack"); /* This is overkill, fix_player() is called somewhere */
    /* in object.c */

    /* If an object merged (and thus, different object), we need to
    * delete the original.
    */
    if (tmp2 != tmp)
        esrv_del_item(CONTR(op), tmp_tag, tmp_cont);

    esrv_send_item(op, tmp2);
    /* update the sacks and players weight */
    esrv_update_item(UPD_WEIGHT, op, sack);
    esrv_update_item(UPD_WEIGHT, op, op);
}

/*
*  This function was part of drop, now is own function.
*  Player 'op' tries to drop object 'tmp', if tmp is non zero, then
*  nrof objects is tried to dropped.
* This is used when dropping objects onto the floor.
*/
void drop_object(object *const op, object *tmp, const uint32 nrof)
{
    char    buf[MAX_BUF];
    object *floor;
    uint32 tmp_nrof;

    if (QUERY_FLAG(tmp, FLAG_NO_DROP) && !QUERY_FLAG(op, FLAG_WIZ))
    {
#if 0
        /* Eneq(@csd.uu.se): Objects with NO_DROP defined can't be dropped. */
        new_draw_info(NDI_UNIQUE, 0,op, "This item can't be dropped.");
#endif
        return;
    }

    if (op->type == PLAYER)
        CONTR(op)->rest_mode = 0;

    if (QUERY_FLAG(tmp, FLAG_APPLIED))
    {
        if (apply_special(op, tmp, AP_UNAPPLY | AP_NO_MERGE))
            return;     /* can't unapply it */
    }

    if (tmp->type == CONTAINER)
        container_unlink(NULL, tmp);

    /* We are only dropping some of the items.  We split the current object */
    if (nrof && tmp->nrof != nrof)
    {
        object*tmp2 =   tmp, *tmp2_cont = tmp->env;
        tag_t           tmp2_tag    = tmp2->count;
        tmp = get_split_ob(tmp, nrof);
        if (!tmp)
        {
            new_draw_info(NDI_UNIQUE, 0, op, errmsg);
            return;
        }
        /* Tell a client what happened rest of objects.  tmp2 is now the
        * original object
        */
        if (op->type == PLAYER)
        {
            if (was_destroyed(tmp2, tmp2_tag))
                esrv_del_item(CONTR(op), tmp2_tag, tmp2_cont);
            else
                esrv_send_item(op, tmp2);
            /* Update the container the object was in */
            if (tmp2->env && tmp2->env != op && tmp2->env != tmp2)
                esrv_update_item(UPD_WEIGHT, op, tmp2->env);
        }
    }
    else
    {
        remove_ob(tmp);
        if (tmp->env && tmp->env != op && tmp->env != tmp)
            esrv_update_item(UPD_WEIGHT, op, tmp->env);
    }

    tmp_nrof = nrof;
    if(trigger_object_plugin_event(EVENT_DROP, tmp, op, NULL, NULL, (int *)&tmp_nrof, NULL, NULL, SCRIPT_FIX_ALL))
    {
        /* TODO: handle cases where we might want to reinsert the object */
        return;
    }

    if (QUERY_FLAG(tmp, FLAG_STARTEQUIP))
    {
        if (op->type == PLAYER)
        {
            sprintf(buf, "You drop the %s.", query_name(tmp));
            new_draw_info(NDI_UNIQUE, 0, op, buf);

            if (QUERY_FLAG(tmp, FLAG_UNPAID))
                new_draw_info(NDI_UNIQUE, 0, op, "The shop magic put it back to the storage.");
            else
                new_draw_info(NDI_UNIQUE, 0, op, "The °NO-DROP° item vanishes to nowhere as you drop it!");
            esrv_del_item(CONTR(op), tmp->count, tmp->env);
        }
        FIX_PLAYER(op,"drop_object - startequip");
        return;
    }

    floor = GET_MAP_OB_LAYER(op->map, op->x, op->y, 0);
    if (floor && floor->type == SHOP_FLOOR && !QUERY_FLAG(tmp, FLAG_UNPAID) && tmp->type != MONEY)
    {
        sell_item(tmp, op, -1);
        /* ok, here we insert then the unique shops too, now we have only clone shops */
        if (QUERY_FLAG(tmp, FLAG_UNPAID)) /* ok, we have really selled it - not only droped */
        {
            if (op->type == PLAYER)
            {
                new_draw_info_format(NDI_UNIQUE, 0, op, "The shop magic put it to the storage.");
                esrv_del_item(CONTR(op), tmp->count, tmp->env);
            }
            FIX_PLAYER(op ,"drop_object - unpaid");
            if (op->type == PLAYER)
                esrv_send_item(op, op);

            return;
        }
    }

    tmp->x = op->x;
    tmp->y = op->y;

    if (op->type == PLAYER)
        esrv_del_item(CONTR(op), tmp->count, tmp->env);

    insert_ob_in_map(tmp, op->map, op, 0);

    SET_FLAG(op, FLAG_NO_APPLY);
    remove_ob(op);
    insert_ob_in_map(op, op->map, op, INS_NO_MERGE | INS_NO_WALK_ON);
    CLEAR_FLAG(op, FLAG_NO_APPLY);

    /* Need to update the weight for the player */
    if (op->type == PLAYER)
    {
        FIX_PLAYER(op ,"drop object - end");
        esrv_send_item(op, op);
    }
}

void drop(object *const op, object *const ori)
{
    if (ori == NULL)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "You don't have anything to drop.");
        return;
    }
    if (QUERY_FLAG(ori, FLAG_INV_LOCKED))
    {
        new_draw_info(NDI_UNIQUE, 0, op, "This item is locked");
        return;
    }
    if (QUERY_FLAG(ori, FLAG_NO_DROP))
    {
#if 0
        /* Eneq(@csd.uu.se): Objects with NO_DROP defined can't be dropped. */
        new_draw_info(NDI_UNIQUE, 0,op, "This item can't be dropped.");
#endif
        return;
    }


    if (op->type == PLAYER)
    {
        if (CONTR(op)->container)
            put_object_in_sack(op, CONTR(op)->container, ori, CONTR(op)->count);
        else
            drop_object(op, ori, CONTR(op)->count);
        CONTR(op)->count = 0;
    }
    else
        drop_object(op, ori, 0);
}

