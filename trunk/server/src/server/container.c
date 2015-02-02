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

    The author can be reached via e-mail to info@daimonin.org
*/
#include <global.h>

/* this file manages container handling. Every object can have a inventory and
 * works "like a container". A CONTAINER is nothing more as a explicit object
 * type with inventory used to store items. But technical every object is a
 * container. */

/* Frees a monster trapped in container when opened by a player */
/* FIXME: This is quite a nice idea but implemented wrong. Monsters must not
 * have environments and  (I think) this will fail/cause problems with
 * multiparts anyway. The containef monster should be a spawn point.
 *
 * -- Smacky 20140323 */
static void free_container_monster(object_t *monster, object_t *op)
{
    object_t *container = monster->env;
    map_t    *m;
    sint16    x,
              y;
    msp_t    *msp;
    sint8     i;

    if (container == NULL)
        return;

    remove_ob(monster); /* in container, no walk off check */
    m = op->map;
    x = monster->x = container->x;
    y = monster->y = container->y;
    msp = MSP_GET(m, x, y);
    i = overlay_find_free(msp, monster, 0, OVERLAY_3X3, 0);

    if (i != -1)
    {
        monster->x += OVERLAY_X(i);
        monster->y += OVERLAY_Y(i);
    }

    fix_monster(monster);

    if (insert_ob_in_map(monster, op->map, monster, 0))
    {
        ndi(NDI_UNIQUE, 0, op, "%s jumps out of %s!",
           QUERY_SHORT_NAME(monster, op),
           QUERY_SHORT_NAME(container, op));
    }
}

/* a player has opened a container - link him to the
* list of player which have (perhaps) it opened too.
*/
int container_link(player_t *const pl, object_t *const sack)
{
    int ret = 0;

    /* for safety reasons, lets check this is valid */
    if (sack->attacked_by)
    {
        if (sack->attacked_by->type != PLAYER
            || !CONTR(sack->attacked_by)
            || CONTR(sack->attacked_by)->container != sack)
        {
            LOG(llevBug, "BUG: container_link() - invalid player linked: <%s>\n", STRING_OBJ_NAME(sack->attacked_by));
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
            STRING_OBJ_NAME(sack->attacked_by), STRING_OBJ_NAME(sack));
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
        esrv_update_item(UPD_FLAGS | UPD_FACE, sack);
        container_trap(pl->ob, sack);   /* search & explode a rune in the container */
        ret = 1;
    }

    esrv_open_container(pl, sack);
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
int container_unlink(player_t *const pl, object_t *sack)
{
    object_t *tmp, *tmp2;

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
        esrv_close_container(pl);
        /* ok, there is a valid container - unlink the player now */
        if (!pl->container_below && !pl->container_above) /* we are only applier */
        {
            if (pl->container->attacked_by != pl->ob) /* we should be that object... */
            {
                LOG(llevBug, "BUG: container_unlink() - container link don't match player!: <%s> sack:<%s> (%s)\n",
                    STRING_OBJ_NAME(pl->ob), STRING_OBJ_NAME(sack->attacked_by), STRING_OBJ_NAME(sack));
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
            esrv_update_item(UPD_FLAGS | UPD_FACE, sack);
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
    esrv_update_item(UPD_FLAGS | UPD_FACE, sack);
    tmp = sack->attacked_by;
    sack->attacked_by = NULL;
    sack->attacked_by_count = 0;

    /* if we are here, we are called with (NULL,sack) */
    while (tmp)
    {
        if (!CONTR(tmp) || CONTR(tmp)->container != sack) /* valid player in list? */
        {
            LOG(llevBug, "BUG: container_unlink() - container link list mismatch!: player?:<%s> sack:<%s> (%s)\n",
                STRING_OBJ_NAME(tmp), STRING_OBJ_NAME(sack), STRING_OBJ_NAME(sack->attacked_by));
            return 1;
        }

        tmp2 = CONTR(tmp)->container_above;
        CONTR(tmp)->container = NULL;
        CONTR(tmp)->container_count = 0;
        CONTR(tmp)->container_below = NULL;
        CONTR(tmp)->container_above = NULL;
        esrv_close_container(CONTR(tmp));
        tmp = tmp2;
    }
    return 1;
}

/* examine the items in a container which gets readied or opened by player .
* Explode or trigger every trap & rune in there and free trapped monsters.
*/
int container_trap(object_t *const op, object_t *const container)
{
    int     ret = 0;
    object_t *tmp,
           *next;

    FOREACH_OBJECT_IN_OBJECT(tmp, container, next)
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
