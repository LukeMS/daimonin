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

/* drop a monster on the map, by copying a monster object or
 * monster object head. Add treasures.
 */
static object * spawn_monster(object *gen, object *orig, int range)
{
    int         i;
    object     *op, *head = NULL, *prev = NULL, *ret = NULL;
    archetype  *at  = gen->arch;

    i = find_first_free_spot2(at, orig->map, orig->x, orig->y, 0, range);
    if (i == -1)
        return NULL;
    while (at != NULL)
    {
        op = get_object();
        if (head == NULL) /* copy single/head from spawn inventory */
        {
            gen->type = MONSTER;
            copy_object(gen, op);
            gen->type = SPAWN_POINT_MOB;
            ret = op;
        }
        else /* but the tails for multi arch from the clones */
        {
            copy_object(&at->clone, op);
        }
        op->x = orig->x + freearr_x[i] + at->clone.x;
        op->y = orig->y + freearr_y[i] + at->clone.y;
        op->map = orig->map;
        if (head != NULL)
            op->head = head,prev->more = op;
        if (OBJECT_FREE(op))
            return NULL;
        if (op->randomitems != NULL)
            create_treasure_list(op->randomitems, op, 0, op->level ? op->level : orig->map->difficulty, ART_CHANCE_UNSET, 0);
        if (head == NULL)
            head = op;
        prev = op;
        at = at->more;
    }
    return ret; /* return object ptr to our spawn */
}

/* check the current darkness on this map allows to spawn
 * 0: not allowed, 1: allowed
 */
/* Is this completely right? Why doesn't this function refer to
 * global_darkness_table[world_darkness], just world_darkness?
 * Why are we only interested in the map darkness level for spawns,
 * not the actual darkness on the square? -- Smacky 20080130 */
static inline int spawn_point_darkness(object *spoint, int darkness)
{
    int map_light;

    if (!spoint->map)
        return 0;

    if (MAP_OUTDOORS(spoint->map) && world_darkness <= MAP_DARKNESS(spoint->map))
        map_light = world_darkness;
    else
    {
        if (MAP_DARKNESS(spoint->map) == -1)
            map_light = MAX_DARKNESS;
        else
            map_light = MAP_DARKNESS(spoint->map);
    }

    if (darkness < 0)
    {
        if (map_light < -darkness)
            return 1;
    }
    else
    {
        if (map_light > darkness)
            return 1;
    }
    return 0;
}


/* we have a mob - now insert a copy of all items the spawn point mob has.
 * take care about RANDOM DROP objects.
 * usually these items are put from the map maker inside the spawn mob inv.
 * remember that these are additional items to the treasures list ones.
 */
static void insert_spawn_mob_loot(object *op, object *mob, object *tmp)
{
    object *tmp2, *next, *next2, *item;

    for (; tmp; tmp = next)
    {
        next = tmp->below;
        if (tmp->type == TYPE_RANDOM_DROP)
        {
            if (!tmp->weight_limit || !(RANDOM() % (tmp->weight_limit + 1))) /* skip this container - drop the ->inv */
            {
                for (tmp2 = tmp->inv; tmp2; tmp2 = next2)
                {
                    next2 = tmp2->below;
                    if (tmp2->type == TYPE_RANDOM_DROP)
                        LOG(llevDebug,
                            "DEBUG:: Spawn:: RANDOM_DROP (102) not allowed inside RANDOM_DROP.mob:>%s< map:%s (%d,%d)\n",
                            query_name(mob), op->map
                          ? op->map->path
                          : "BUG: S-Point without map!", op->x, op->y);
                    else
                    {
                        item = get_object();
                        copy_object(tmp2, item);
                        insert_ob_in_ob(item, mob);      /* and put it in the mob */
                        if(tmp2->inv)
                            insert_spawn_mob_loot(op, item, tmp2->inv);
                    }
                }
            }
        }
        else /* remember this can be sys_objects too! */
        {
            item = get_object();
            copy_object(tmp, item);
            insert_ob_in_ob(item, mob);      /* and put it in the mob */
            if(tmp->inv)
                insert_spawn_mob_loot(op, item, tmp->inv);
        }
    }
}


/* central spawn point function.
 * Control, generate or remove the generated object.
 */
void spawn_point(object *op)
{
    int     rmt;
    object *tmp, *mob, *next;

    if (op->enemy)
    {
        if (OBJECT_VALID(op->enemy, op->enemy_count)) /* all ok, our spawn have fun */
        {
            if (op->last_eat) /* check darkness if needed */
            {
                /* 1 = darkness is ok */
                if (spawn_point_darkness(op, op->last_eat))
                    return;

                /* darkness has changed - now remove the spawned monster */
                remove_ob(op->enemy);
                check_walk_off(op->enemy, NULL, MOVE_APPLY_VANISHED);
            }
            else
                return;
        }
        op->enemy = NULL; /* spawn point has nothing spawned */
    }

    /* a set sp value will override the spawn chance.
     * with that "trick" we force for map loading the
     * default spawns of the map because sp is 0 as default.
     */
    /*LOG(-1,"SPAWN...(%d,%d)",op->x, op->y);*/
    if (op->stats.sp == -1)
    {
        int gg;
        /* now lets decide we will have a spawn event */
        if (op->last_grace <= -1) /* never */
        {
            /*LOG(-1," closed (-1)\n");*/
            return;
        }
        if (op->last_grace && (gg = (RANDOM() % (op->last_grace + 1)))) /* if >0 and random%x is NOT null ... */
        {
            /*LOG(-1," chance: %d (%d)\n",gg,op->last_grace);*/
            return;
        }

        op->stats.sp = (RANDOM() % SPAWN_RANDOM_RANGE);
    }
    /*LOG(-1," hit!: %d\n",op->stats.sp);*/

    if (!op->inv) /* spawn point without inventory! */
    {
        LOG(llevBug, "BUG: Spawn point without inventory!! --> map %s (%d,%d)\n",
            op->map ? (op->map->path ? op->map->path : ">no path<") : ">no map<", op->x, op->y);
        /* kill this spawn point - its useless and need to fixed from the map maker/generator */
        remove_ob(op);
        check_walk_off(op, NULL, MOVE_APPLY_VANISHED);
        return;
    }

    /* now we move through the spawn point inventory and
     * get the mob with a number under this value AND nearest.
     */
    for (rmt = 0,mob = NULL,tmp = op->inv; tmp; tmp = next)
    {
        next = tmp->below;

        /* ignore events (scripts) */
        if (tmp->type == TYPE_EVENT_OBJECT)
            continue;

        if (tmp->type != SPAWN_POINT_MOB)
            LOG(llevBug, "BUG: spawn point in map %s (%d,%d) with wrong type object (%d) in inv: %s\n",
                op->map ? op->map->path : "<no map>", op->x, op->y, tmp->type, query_name(tmp));
        else if ((int) tmp->enemy_count <= op->stats.sp && (int) tmp->enemy_count >= rmt)
        {
            /* we have a possible hit - control special settings now */
            if (tmp->last_eat) /* darkness */
            {
                /* 1: darkness on map of spawn point is ok */
                if (!spawn_point_darkness(op, tmp->last_eat))
                    continue;
            }

            rmt = (int) tmp->enemy_count;
            mob = tmp;
        }
        /*      LOG(llevInfo,"inv -> %s (%d :: %d - %f)\n", tmp->name, op->stats.sp, tmp->enemy_count, tmp->speed_left);*/
    }
    /* we try only ONE time a respawn of a pre setting - so careful! */
    rmt = op->stats.sp;
    op->stats.sp = -1;
    if (!mob) /* well, this time we spawn nothing */
        return;

    tmp = mob->inv; /* quick save the def mob inventory */
    if (!(mob = spawn_monster(mob, op, op->last_heal)))
        return; /* that happens when we have no free spot....*/

    if (trigger_object_plugin_event(EVENT_TRIGGER, op, mob, NULL, NULL, NULL, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
        return;

    /* setup special monster -> spawn point values */
    op->last_eat = 0;
    if (mob->last_eat) /* darkness controled spawns */
    {
        op->last_eat = mob->last_eat;
        mob->last_eat = 0;
    }
    insert_spawn_mob_loot(op, mob, tmp);

    op->last_sp = rmt; /* this is the last rand() for what we have spawned! */

    op->enemy = mob; /* chain the mob to our spawn point */
    op->enemy_count = mob->count;

    /* perhaps we have later more unique mob setting - then we can store it here too.
     */
    tmp = arch_to_object(op->other_arch); /* create spawn info */
    tmp->owner = op; /* chain spawn point to our mob */
    tmp->owner_count = op->count;
    insert_ob_in_ob(tmp, mob);      /* and put it in the mob */

    SET_MULTI_FLAG(mob, FLAG_SPAWN_MOB); /* FINISH: now mark our mob as a spawn */
    fix_monster(mob); /* fix all the values and add in possible abilities or forces ... */
    if (!insert_ob_in_map(mob, mob->map, op, 0)) /* *now* all is done - *now* put it on map */
        return;
}

/*
 * Linked Spawns (Mobs) Code.
 * We implement "linked mobs" with the spawn points - so its "linked spawns"
 * Linked spawns is nothing more as a 2 or more linked spawn points who are
 * triggered by a signal at once.
 * An Example: 3 linked spawn points are around a campfire. On a hill is another
 * one linked to it - so its 4 linked spawns. If now the scout on the hill get
 * attacked & find a target, the signal goes to all linked mobs. So, the 3
 * mobs on the campfire joins in the fight.
 * Important: Linked spawns are a  pre AI mechanism and has nothing to do with "social mobs"
 * which are part of the AI. Linked mobs are used to trigger a situation like
 * "as you attack all mobs from left and 2 behind the trees graps their weapon and start
 * fighting" - after we create with the linked mobs system this situation we give it to
 * the AI as usual.
 *
 * Beside that we can use the linked spawns for more tricky technical setup, related to quests:
 * Instead of trigger a linked attack, we check with the linked list for special events.
 * Like: if all linked spawns point mobs are killed, call a script. Spawn a master mob or
 * do some other special event. This is a often used quest setup. We can chain in this way
 * (by using a script) different maps like if all mobs of linked group A are killed, we open
 * the castle doors and trigger linked group B or call the castle guards by triggering the AI.
 */

/* This is the basic implementation. MT-07.2005 */

static inline objectlink *get_linked_spawn(object *spawn)
{
    objectlink        *ol;

    for(ol = spawn->map->linked_spawn_list;ol;ol = ol->next)
    {
        /* slaying string is the linked spawn ID */
        if(ol->objlink.ob->slaying == spawn->slaying)
            return ol;

    }

    return NULL;
}

/* LIFO queue for the base links - spawns point of every list
 * are in a LIFO queue too
 * called when a map with linked spawn is loaded
 */
objectlink *add_linked_spawn(object *spawn)
{
    objectlink        *ol;

    ol = get_linked_spawn(spawn);
    if(!ol) /* new one? create base link */
    {
        ol = get_objectlink(OBJLNK_FLAG_OB);
        ol->next = spawn->map->linked_spawn_list;
        spawn->map->linked_spawn_list = ol; /* add base link it to the map */
    }
    ol->value++;
    spawn->attacked_by = ol->objlink.ob; /* add this spawn point the the base link */
    ol->objlink.ob = spawn;

#ifdef DEBUG_LINK_SPAWN
    LOG( llevDebug, "LINK_SPAWN::add: map: %s (%d,%d) - %d\n",
         STRING_SAFE(spawn->map->path), spawn->x, spawn->y, ol->value);
#endif
    return ol;
}

/* remove the link spawn base link list.
 * called when a map struct is removed.
 */
void remove_linked_spawn_list(mapstruct *map)
{
    objectlink *ol, *tmp;

    if(!(ol = map->linked_spawn_list))
        return;

#ifdef DEBUG_LINK_SPAWN
    LOG( llevDebug, "LINK_SPAWN::remove linked spawns for map %s\n", STRING_SAFE(map->path ));
#endif

    for(;ol;ol = tmp)
    {
        tmp = ol->next;
        free_objectlink_simple(ol);
    }

    map->linked_spawn_list = NULL;
}

/* send a signal through the linked mobs
*/
void send_link_spawn_signal(object *spawn, object *target, int signal)
{
    objectlink        *ol = get_linked_spawn(spawn);

    if(!ol) /* sanity check */
    {
        LOG( llevDebug, "BUG LINK_SPAWN::send_link_spawn_signal(): (map: %s (%d,%d))- LinkID: %s\n",
            spawn->map?STRING_SAFE(spawn->map->path):"NULL", spawn->x, spawn->y, STRING_SAFE(spawn->slaying));
        return;
    }

    /* Assign an enemy to all linked spawns */
    if(signal & LINK_SPAWN_ENEMY)
    {
        object *obj;

        for(obj=ol->objlink.ob; obj; obj = obj->attacked_by)
        {
            if(obj==spawn) /* we don't need to set here the caller */
                continue;

            /* obj = spawn point - obj->enemy = possible spawn */
            if(obj->enemy && obj->enemy->enemy != target)
            {
                struct mob_known_obj *enemy;

                /* we can now do here wonderful things like:
                 * - random assign from a linked spawn pool mobs who attack
                 * - check target is in group and decide then how much from the
                 *   linked pool we throw in the fight
                 * and much more...
                 */
                enemy = update_npc_knowledge(obj->enemy, target, FRIENDSHIP_ATTACK, 0);

                if(enemy) /* in this mode, we don't force the register */
                {
                    /* we need a "set_enemy" in the ai code beside "choose_enemy" */
                    obj->enemy->enemy = enemy->obj;
                    MOB_DATA(obj->enemy)->enemy = enemy;
                    obj->enemy->enemy_count = enemy->obj_count;
                    MOB_DATA(obj->enemy)->idle_time = 0;
                    set_mobile_speed(obj->enemy, 0);


#ifdef DEBUG_LINK_SPAWN
                    LOG( llevDebug, "LINK_SPAWN::target enemy: map %s (%d,%d)\n",
                         STRING_SAFE(obj->map->path), obj->x, obj->y);
#endif
                }
            }
        }
    }
}
