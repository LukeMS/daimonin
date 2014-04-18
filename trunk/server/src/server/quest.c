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

static char *skill_group_name[] = {
    "","agility ", "personality ","mental ","physical ","magic ","wisdom ","",""
};

/* recursive check the inventory for a specific quest item */
static object *find_quest_item(object *target, object *obj)
{
    object *tmp;

    for (tmp = target->inv; tmp; tmp = tmp->below)
    {
        if(tmp->type == CONTAINER)
        {
            object *item;

            if ((item = find_quest_item(tmp, obj)))
            {
                return item;
            }
        }
        else if (!QUERY_FLAG(tmp, FLAG_SYS_OBJECT) &&
                 tmp->type == obj->type &&
                 tmp->name == obj->name &&
                 tmp->title == obj->title)
        {
            return tmp;
        }
    }

    return NULL;
}

/* check the player we can drop a one-drop quest trigger item */
static inline object *find_one_drop_quest_item(object *target, object *obj)
{
    object *tmp;

    /* if we have a "fake" one drop item, we check the normal inventory it is there */
    if(!QUERY_FLAG(obj, FLAG_ONE_DROP))
        return find_quest_item(target, obj);

    if ((tmp=CONTR(target)->quest_one_drop) )
    {
        for (tmp = tmp->inv; tmp; tmp = tmp->below)
        {
            /* the race to arch compare is needed for faked items and to hold comp. to beta 3 and lower */
            if (tmp->name == obj->name && tmp->race == obj->arch->name && tmp->title == obj->title)
                return tmp;
        }
    }

    return NULL;
}


static inline object *find_quest_trigger(object *target, object *obj)
{
    object *tmp;

    if ((tmp=CONTR(target)->quests_type_normal) )
    {
        for (tmp = tmp->inv; tmp; tmp = tmp->below)
        {
            if (tmp->name == obj->name && (tmp->magic == obj->magic || tmp->magic == obj->last_heal))
                return tmp;
        }
    }

    return NULL;
}

static inline object *add_quest_item(object *target, object *obj)
{
    object *q_tmp;

    q_tmp = get_object();
    copy_object(obj, q_tmp);
    SET_FLAG(q_tmp, FLAG_KNOWN_CURSED);
    SET_FLAG(q_tmp, FLAG_KNOWN_MAGICAL);
    SET_FLAG(q_tmp, FLAG_IDENTIFIED);
    q_tmp = insert_ob_in_ob(q_tmp, target); /* real object to player */

    return q_tmp;
}

/* we give a player a one drop item. This also
 * adds this item to the quest_container - instead to quest
 * items which will be added when the next quest step is triggered.
 * (most times from a quest script)
 */
static inline object *add_one_drop_quest_item(object *target, object *obj)
{
    object *qt;

    /* only mark the item as "real" one drop if it has the flag */
    if(QUERY_FLAG(obj, FLAG_ONE_DROP))
    {
        object *qc;

        if (!CONTR(target)->quest_one_drop)
            add_quest_containers(target);

        qc = CONTR(target)->quest_one_drop;
        qt = get_object();
        copy_object_data(obj, qt); /* copy without put on active list */
        /* just to be on the secure side ... */
        qt->speed = 0.0f;
        CLEAR_FLAG(qt, FLAG_ANIMATE);
        CLEAR_FLAG(qt, FLAG_ALIVE);
        SET_FLAG(qt, FLAG_KNOWN_CURSED);
        SET_FLAG(qt, FLAG_KNOWN_MAGICAL);
        SET_FLAG(qt, FLAG_IDENTIFIED);
        /* we are storing the arch name of quest dummy items in race */
        FREE_AND_COPY_HASH(qt->race, obj->arch->name);
        insert_ob_in_ob(qt, qc); /* dummy copy in quest container */
        SET_FLAG(obj, FLAG_KNOWN_CURSED);
        SET_FLAG(obj, FLAG_KNOWN_MAGICAL);
        SET_FLAG(obj, FLAG_IDENTIFIED);
    }

    qt = add_quest_item(target, obj);

    return qt;
}

void insert_quest_item(struct obj *quest_trigger, struct obj *target)
{
    if (!target || target->type != PLAYER)
        return;

    /* its a one drop - it triggers when the drop chance is not set or triggers */
    if (QUERY_FLAG(quest_trigger, FLAG_ONE_DROP)
            && (!quest_trigger->last_grace || !(RANDOM() % (quest_trigger->last_grace+1)))) /* marks one drop quest items */
    {
        int tmp_lev = 0;

        if (quest_trigger->item_skill)
            tmp_lev = CONTR(target)->skillgroup_ptr[quest_trigger->item_skill-1]->level; /* use player struct shortcut ptrs */
        else
            tmp_lev = target->level;

        if(quest_trigger->item_level <= tmp_lev)
        {
            object *tmp;

            for (tmp = quest_trigger->inv; tmp; tmp = tmp->below)
            {
                if(!find_one_drop_quest_item(target, tmp))
                {
                    object *qt = add_one_drop_quest_item(target, tmp);

                    if (qt)
                    {
                        new_draw_info(NDI_UNIQUE | NDI_NAVY | NDI_VIM, 0, target, "You found the %s drop %s!",
                            (QUERY_FLAG(qt, FLAG_ONE_DROP)) ? "one" : "special",
                            query_name(qt, target, ARTICLE_NONE, 0));
                        play_sound_player_only(CONTR(target), SOUND_LEVEL_UP,
                            SOUND_NORMAL, 0, 0);
                    }
                }
            }
        }
    }
    else /* check the real quest stuff */
    {
        object *tmp, *quest;

        if ((quest = find_quest_trigger(target, quest_trigger)) &&
            quest->magic != (sint8)quest_trigger->last_heal)
        {
            char buf[MEDIUM_BUF] = "";

            quest->magic = (sint8)quest_trigger->last_heal;

            if(quest_trigger->msg)
                new_draw_info(NDI_UNIQUE | NDI_ORANGE, 0, target, "%s",
                              quest_trigger->msg);

            for (tmp = quest->inv; tmp; tmp = tmp->below)
            {
                if(!find_quest_item(target, tmp))
                {
                    add_quest_item(target, tmp);
                    sprintf(buf, "You found the quest item %s!\n",
                        query_name(tmp, target, ARTICLE_NONE, 0));
                }
            }
            
            if (quest->magic == quest->state)
            {
                sprintf(strchr(buf, '\0'), "Quest completed!");
            }

            update_quest(quest, ST1_QUEST_UPDATE_ARBITRARY, NULL, NULL, buf);
        }
    }
}

/* TODO: Temp function. Will do better in SEQSy */
static inline void remove_quest_items(player *pl, object *op)
{
    int     nrof = (op->nrof) ? op->nrof : 1;
    object *tmp;

    while (nrof &&
           (tmp = find_quest_item(pl->ob, op)))
    {
        int m = MIN(nrof, (int) tmp->nrof);
        int n = MAX(1, m);

        decrease_ob_nr(tmp, n);
        nrof = MAX(0, nrof - n);
    }
}

/* if we found a player with missing or incomplete quest containers,
 * we install & set them here. We check also the integrety of the ptrs.
 * IMPORTANT! don't call this function BEFORE the player has been send to his
 * first fix_player() or we will double the containers.
 */
void add_quest_containers(struct obj *op)
{
    player *pl;

    if(op->type != PLAYER)
        return;

    pl = CONTR(op);

    /* missing one drop container */
    if (!pl->quest_one_drop || !OBJECT_VALID(pl->quest_one_drop, pl->quest_one_drop_count) || pl->quest_one_drop->env != op)
    {
        pl->quest_one_drop = arch_to_object(archetype_global._quest_container);
        pl->quest_one_drop_count = pl->quest_one_drop->count;
        pl->quest_one_drop->sub_type1 = ST1_QUEST_ONE_DROP;
        FREE_AND_COPY_HASH(pl->quest_one_drop->name, "QC: onedrops");
        insert_ob_in_ob(pl->quest_one_drop, op);
    }


    if(!pl->quests_done || !OBJECT_VALID(pl->quests_done,pl->quests_done_count) || pl->quests_done->env != op)
    {
        pl->quests_done = arch_to_object(archetype_global._quest_container);
        pl->quests_done_count = pl->quests_done->count;
        pl->quests_done->sub_type1 = ST1_QUESTS_TYPE_DONE;
        FREE_AND_COPY_HASH(pl->quests_done->name, "QC: list done");
        insert_ob_in_ob(pl->quests_done, op);
    }

    if(!pl->quests_type_kill || !OBJECT_VALID(pl->quests_type_kill,pl->quests_type_kill_count) || pl->quests_type_kill->env != op)
    {
        pl->quests_type_kill = arch_to_object(archetype_global._quest_container);
        pl->quests_type_kill_count = pl->quests_type_kill->count;
        pl->quests_type_kill->sub_type1 = ST1_QUESTS_TYPE_KILL;
        FREE_AND_COPY_HASH(pl->quests_type_kill->name, "QC: kill");
        insert_ob_in_ob(pl->quests_type_kill, op);
    }

    if(!pl->quests_type_normal || !OBJECT_VALID(pl->quests_type_normal,pl->quests_type_normal_count) || pl->quests_type_normal->env != op)
    {
        pl->quests_type_normal = arch_to_object(archetype_global._quest_container);
        pl->quests_type_normal_count = pl->quests_type_normal->count;
        pl->quests_type_normal->sub_type1 = ST1_QUESTS_TYPE_NORMAL;
        FREE_AND_COPY_HASH(pl->quests_type_normal->name, "QC: normal");
        insert_ob_in_ob(pl->quests_type_normal, op);
    }
}

/* be sure trigger has the right settings which kind of trigger it is,
 * or the insert will FAIL.
 */
void add_quest_trigger(struct obj *who, struct obj *trigger)
{
    player *pl;

    if (!trigger || trigger->type != TYPE_QUEST_TRIGGER)
    {
        LOG(llevBug, "BUG:: %s/add_quest_trigger(): trigger (%s[%d]) is not a quest trigger!\n",
            __FILE__, STRING_OBJ_NAME(trigger), TAG(trigger));

        return;
    }

    if (!who || who->type != PLAYER || !(pl = CONTR(who)))
    {
        LOG(llevBug, "BUG:: %s/add_quest_trigger(): who (%s[%d]) is not a player!\n",
            __FILE__, STRING_OBJ_NAME(who), TAG(who));

        return;
    }

    add_quest_containers(who);

    switch (trigger->sub_type1)
    {
        case ST1_QUEST_TRIGGER_NORMAL:
        case ST1_QUEST_TRIGGER_ITEM:
            insert_ob_in_ob(trigger, pl->quests_type_normal);

            break;

        case ST1_QUEST_TRIGGER_KILL:
        case ST1_QUEST_TRIGGER_KILL_ITEM:
            insert_ob_in_ob(trigger, pl->quests_type_kill);

            break;

        default:
            LOG(llevBug, "BUG:: %s/add_quest_trigger(): Wrong quest subtype: %d!\n",
                __FILE__, trigger->sub_type1);
    }
}

/* if we change the quest type and/or the quest_trigger status,
 * we must move the trigger from one container to another. */
/* trigger->last_eat is the max number of times this quest can be repeated (ie,
 * 0 means the quest can be done once).
 * trigger->stats.food is the number of repeats left. When it reaches -1, the
 * quest is not repeatable. */
void set_quest_status(struct obj *trigger, int q_status, int q_type)
{
    object *who = is_player_inv(trigger);

    if(!who)
    {
        LOG(llevBug, "set_quest_status(): trigger not in player/container\n", STRING_OBJ_NAME(trigger->env));
        return;
    }

    remove_ob(trigger);

    trigger->sub_type1 = q_type;
    trigger->stats.food = CLAMP(q_status, -1, trigger->last_eat);

    if(trigger->stats.food == -1)
    {
        object *target = NULL;

        if (q_type == ST1_QUEST_TRIGGER_ITEM)
        {
            target = trigger;
        }
        else if (q_type == ST1_QUEST_TRIGGER_KILL_ITEM)
        {
            target = trigger->inv;
        }

        for (; target; target = (target == trigger) ? NULL : target->below)
        {
            if (target->type == TYPE_QUEST_INFO ||
                target->type == TYPE_QUEST_TRIGGER)
            {
                object *item = target->inv;

                for (; item; item = item->below)
                {
                    if (!QUERY_FLAG(item, FLAG_SYS_OBJECT))
                    {
                        remove_quest_items(CONTR(who), item);
                    }
                }
            }
        }

        remove_ob_inv(trigger); /* clear quest data inventory before we neutralize */
        insert_ob_in_ob(trigger, CONTR(who)->quests_done);
    }
    else if (trigger->stats.food < trigger->last_eat)
    {
        object *target = NULL;

        if (q_type == ST1_QUEST_TRIGGER_ITEM)
        {
            target = trigger;
        }
        else if (q_type == ST1_QUEST_TRIGGER_KILL_ITEM)
        {
            target = trigger->inv;
        }

        for (; target; target = (target == trigger) ? NULL : target->below)
        {
            if (target->type == TYPE_QUEST_INFO ||
                target->type == TYPE_QUEST_TRIGGER)
            {
                object *item = target->inv;

                for (; item; item = item->below)
                {
                    if (!QUERY_FLAG(item, FLAG_SYS_OBJECT))
                    {
                        remove_quest_items(CONTR(who), item);
                    }
                }
            }
        }

        insert_ob_in_ob(trigger, CONTR(who)->quests_done);
    }
    else
        add_quest_trigger(who, trigger);
}

/* Checks the active status of trigger for pl, returning the appropriate
 * QSTAT_* code. Note that active status is not the same as status.
 *
 * Status includes whether or not a player is eligible for the quest (QSTAT_NO
 * or QSTAT_DISALLOW) and therefore cannot be QSTAT_UNKNOWN. Status can only be
 * ascertained with a script (because the eligibility criteria rely on certain
 * checks and lists defined only within the script).
 *
 * Active status therefore cannot check eligiblity. Hence QSTAT_UNKNOW says
 * nothing of eligibility, it just means the player is not doing/has not done
 * this quest (and that includes because the quest -- or player -- does not
 * exist or duff values have been passed to this function).
 *
 * Additionally, where a quest has been done but is still repeatable, QSTAT_NO
 * is returned. Again, this does not imply actual eligibility, it just
 * differentiates the return from QSTAT_DONE. */
int quest_get_active_status(player *pl, object *trigger)
{
    object *this;

    /* (1) Check that the quest is known to the player at all (that means he is
     * doing or has done it). */
    /* (1a) If either pl or trigger is NULL or the wrong type then obviously
     * the quest is effectively unknown. */
    if (!trigger ||
        trigger->type != TYPE_QUEST_TRIGGER ||
        !pl)
    {
        return QSTAT_UNKNOWN;
    }
    /* (1b) Unless the trigger is in one of the player's quest containers it
     * must be unkown. */
    else
    {
        /* If there are missing quest containers, add them. */
        if (!pl->quests_done ||
            !pl->quests_type_normal ||
            !pl->quests_type_kill)
        {
            add_quest_containers(pl->ob);
        }

        if (pl->quests_done != trigger->env &&
            pl->quests_type_normal != trigger->env &&
            pl->quests_type_kill != trigger->env)
        {
            return QSTAT_UNKNOWN;
        }
    }

    /* (2) If it's in the done container, check if it's repeatable. The number
     * of repeats left in this quest are held in ->stats.food. When this
     * reaches -1, the quest is really done. */
    if (pl->quests_done == trigger->env)
    {
        if (trigger->stats.food == -1)
        {
            return QSTAT_DONE;
        }
        else
        {
            return QSTAT_NO;
        }
    }

    /* (3) So now we know it is known and not yet done. */
    /* (3a) Depending on it's subtype, check if it is still active (ie, the
     * player still has tasks to complete). */
    switch (trigger->sub_type1)
    {
        /* (3ai) Normal quests have 1 or more steps (the current number is in
         * ->magic, the total is in ->state). */
        case ST1_QUEST_TRIGGER_NORMAL:
        if (trigger->magic < trigger->state)
        {
            return QSTAT_ACTIVE;
        }

        break;

        /* (3aii) Kill quests require the player to kill 1 or more defined
         * targets (the current number is in ->level, the total is in
         * ->last_sp). */
        case ST1_QUEST_TRIGGER_KILL:
        for (this = trigger->inv; this; this = this->below)
        {
            if (this->type != TYPE_QUEST_UPDATE &&
                this->level < this->last_sp)
            {
                return QSTAT_ACTIVE;
            }
        }

        break;

        /* (3aiii) Item quests require the player to collect 1 or more items,
         * and killitem quests require this too but the player is rewarded with
         * 1 such item per kill. In both cases count the number of items in his
         * inv compared to the required number. */
        case ST1_QUEST_TRIGGER_KILL_ITEM:
        case ST1_QUEST_TRIGGER_ITEM:
        for (this = trigger->inv; this; this = this->below)
        {
            object *item = (trigger->sub_type1 == ST1_QUEST_TRIGGER_KILL_ITEM) ?
                this->inv : this;

            if (this->type != TYPE_QUEST_UPDATE &&
                item &&
                get_nrof_quest_item(pl->ob, item->arch->name, item->name, item->title) < item->nrof)
            {
                return QSTAT_ACTIVE;
            }
        }

        break;
    }

    /* (3b) If the quest was not found to be active above, it must be solved
     * (meaning the player has completed all the tasks, now he just needs to
     * report back to the quest giver. */
    return QSTAT_SOLVED;
}

int update_quest(struct obj *trigger, uint8 subtype, struct obj *info, char *text, char *vim)
{
    player        *pl;
    object        *ob;
    timeanddate_t  tad;

    /* The trigger must be in a player's quest container. */
    if (!(ob = is_player_inv(trigger)) ||
        !(pl = CONTR(ob)) ||
        trigger->env->type != TYPE_QUEST_CONTAINER)
    {
        LOG(llevBug, "BUG:: %s/update_quest(): trigger not in player's quest container!\n",
                     __FILE__);

        return 0;
    }

    if (info &&
        info->type != TYPE_QUEST_INFO)
    {
        LOG(llevBug, "BUG:: %s:update_quest(): Ignoring update, info must be type %u (is %u)!\n",
            __FILE__, TYPE_QUEST_INFO, info->type);

        return 0;
    }

    /* Non-arbitrary updates are simply running totals (x/y) of kills or
     * killitems (in future perhaps others). Arbitrary updates are anything
     * else (including those added by script). Arbitraries always produce a new
     * update. Non-arbitraries overwrite the previous update. */
    if (subtype != ST1_QUEST_UPDATE_ARBITRARY)
    {
        /* Remove the first (therefore only) exact match update. */
        for (ob = trigger->inv; ob; ob = ob->below)
        {
            if (ob->type == TYPE_QUEST_UPDATE &&
                ob->sub_type1 == subtype &&
                ob->race == info->race &&
                ob->name == info->name &&
                ob->title == info->title &&
                ob->slaying == info->slaying &&
                ob->weight_limit == info->weight_limit)
            {
                remove_ob(ob);

                break;
            }
        }
    }

    /* Create an empty quest_update of the specified subtype. */
    ob = arch_to_object(archetype_global._quest_update);
    ob->sub_type1 = subtype;

    /* Set the info attributes if info != NULL. */
    if (!info)
    {
        FREE_AND_CLEAR_HASH(ob->race);
        FREE_AND_CLEAR_HASH(ob->name);
        FREE_AND_CLEAR_HASH(ob->title);
        FREE_AND_CLEAR_HASH(ob->slaying);
        ob->weight_limit = 0;
    }
    else
    {
        if (!info->race)
        {
            FREE_AND_CLEAR_HASH(ob->race);
        }
        else
        {
            FREE_AND_ADD_REF_HASH(ob->race, info->race);
        }

        if (!info->name)
        {
            FREE_AND_CLEAR_HASH(ob->name);
        }
        else
        {
            FREE_AND_ADD_REF_HASH(ob->name, info->name);
        }

        if (!info->title)
        {
            FREE_AND_CLEAR_HASH(ob->title);
        }
        else
        {
            FREE_AND_ADD_REF_HASH(ob->title, info->title);
        }

        if (!info->slaying)
        {
            FREE_AND_CLEAR_HASH(ob->slaying);
        }
        else
        {
            FREE_AND_ADD_REF_HASH(ob->slaying, info->slaying);
        }

        ob->weight_limit = info->weight_limit;
    }

    /* Give the update a timestamp, write text to it, and insert it in
     * trigger. */
    get_tad(&tad, 0);
    ob->custom_attrset = print_tad(&tad, TAD_SHOWTIME | TAD_SHOWDATE);
    FREE_AND_COPY_HASH(ob->msg, (text) ? text : vim);
    insert_ob_in_ob(ob, trigger);

    /* Set FLAG_BLIND on trigger to indicate it has an unread update. */
    SET_FLAG(trigger, FLAG_BLIND);

    /* Notify player that a quest has been updated. */
    new_draw_info(NDI_UNIQUE | NDI_NAVY | NDI_VIM, 0, pl->ob,
                         "Quest updated!\n%s", vim);
    play_sound_player_only(pl, SOUND_LEVEL_UP, SOUND_NORMAL, 0, 0);

    return 1;
}

/* If we are here, a player has killed or invoked the kill of something
 * Now, check the kill quest events (if there are some) and adjust the
 * quest_trigger.
 * NOTE: Be sure you checked quests_type_kill before!
 */
void check_kill_quest_event(struct obj *pl, struct obj *op)
{
    object *tmp, *tmp_info;

    if (!CONTR(pl)->quests_type_kill)
        add_quest_containers(pl);

    /* browse the quest triggers */
    for (tmp = CONTR(pl)->quests_type_kill->inv; tmp; tmp = tmp->below)
    {
        /* inside the quest triggers are quest_info object with the kill info */
        for(tmp_info=tmp->inv;tmp_info;tmp_info=tmp_info->below)
        {
            if ((!tmp_info->race || tmp_info->race == op->arch->name) &&
                (!tmp_info->name || tmp_info->name == tmp_info->arch->name || tmp_info->name == op->name) &&
                (!tmp_info->title || tmp_info->title == op->title) &&
                (!tmp_info->slaying || tmp_info->slaying == op->race) &&
                (!tmp_info->weight_limit || tmp_info->weight_limit <= op->level))
            {
                char buf[MEDIUM_BUF];

                /* the drop/kill chance can be random ... */
                if(tmp_info->last_grace > 1 && (RANDOM() % tmp_info->last_grace))
                    continue; /* good kill, bad luck, no item */

                /* ok, we have a hit... now lets check what we have - kill or kill item */
                if(tmp->sub_type1 == ST1_QUEST_TRIGGER_KILL_ITEM)
                {
                    uint32 nrof;

                    /* Its real: give the item inside the quest_info the player */
                    if(!tmp_info->inv)
                    {
                        LOG(llevBug,"BUG: check_kill_quest_event:: tmp_info has no inventory item (quest %s)\n",
                                STRING_OBJ_NAME(tmp));
                        continue;
                    }
                    else
                    {
                        object *newob;

                        /* don't give out more as needed items */
                        nrof = get_nrof_quest_item(pl, tmp_info->inv->arch->name, tmp_info->inv->name, tmp_info->inv->title);
                        if(nrof++ >= (tmp_info->inv->nrof?tmp_info->inv->nrof:1))
                            continue;

                        /* create a clone and put it in player inventory */
                        newob = get_object();
                        copy_object(tmp_info->inv, newob);
                        newob->nrof = newob->arch->clone.nrof; /* IMPORTANT: the quest item nrof is used as nrof we need! */
                        (void)insert_ob_in_ob(newob, pl);
                    }

                    if(nrof > tmp_info->inv->nrof)
                    {
                        nrof = tmp_info->inv->nrof;
                        if(nrof)
                            nrof = 1;
                    }

                    sprintf(buf, "Quest %s\n%s: %d/%d",
                        STRING_OBJ_NAME(tmp),
                        query_name(tmp_info->inv, NULL, ARTICLE_NONE, 0), nrof,
                        (tmp_info->inv->nrof) ? tmp_info->inv->nrof : 1);
                    update_quest(tmp, ST1_QUEST_UPDATE_KILLITEM, tmp_info, NULL, buf);
                }
                else if(tmp_info->level < tmp_info->last_sp) /* pure kill quest - alot easier */
                {
                    sprintf(buf, "Quest %s\n%s: %d/%d",
                            STRING_OBJ_NAME(tmp), query_name(op, NULL, ARTICLE_NONE, 0),
                            ++tmp_info->level, tmp_info->last_sp);
                    update_quest(tmp, ST1_QUEST_UPDATE_KILL, tmp_info, NULL, buf);
                }
            }
        }
    }
}

/* We have a container with a quest_trigger type container in it.
 * Its automatically given and/or looking for a quest_trigger of
 * the player
 */
void check_cont_quest_event(struct obj *pl, struct obj *sack)
{
    object *tmp;

    /* lets browse the inventory of the container for quest_trigger object */
    for(tmp=sack->inv;tmp;tmp=tmp->below)
    {
        if(tmp->type ==TYPE_QUEST_TRIGGER && (tmp->sub_type1 == ST1_QUEST_TRIGGER_NORMAL ||
                    tmp->sub_type1 == ST1_QUEST_TRIGGER_ITEM))
            insert_quest_item(tmp, pl);
    }
}

/* recursive check the (legal, non hidden with containers) inventory for nrof of an item */
uint32 get_nrof_quest_item(const struct obj *target, const char *aname, const char *name, const char *title)
{
    int nrof = 0;
    object *tmp;

    for (tmp = target->inv; tmp; tmp = tmp->below)
    {
        if(QUERY_FLAG(tmp, FLAG_SYS_OBJECT)) /* search only "real" inventory */
            continue;
        if (tmp->arch->name == aname && tmp->name == name && tmp->title == title)
            nrof += tmp->nrof?tmp->nrof:1;
        if(tmp->type == CONTAINER)
            nrof += get_nrof_quest_item(tmp, aname, name, title);
    }

    return nrof;
}

/* lets find a quest with name */
struct obj *quest_find_name(const struct obj *pl, const char *name)
{
    struct obj *tmp;
    const char *namehash = find_string(name);

    if(!namehash) /* unknown name = no quest known */
        return NULL;

    if (CONTR(pl)->quests_type_normal)
    {
        for (tmp = CONTR(pl)->quests_type_normal->inv; tmp; tmp = tmp->below)
        {
            if(tmp->name == namehash)
                return tmp;

        }
    }

    if (CONTR(pl)->quests_type_kill)
    {
        for (tmp = CONTR(pl)->quests_type_kill->inv; tmp; tmp = tmp->below)
        {
            if(tmp->name == namehash)
                return tmp;

        }
    }

    if (CONTR(pl)->quests_done)
    {
        for (tmp = CONTR(pl)->quests_done->inv; tmp; tmp = tmp->below)
        {
            if(tmp->name == namehash)
                return tmp;

        }
    }

    return NULL;
}

/* count all open (pending) quests */
int quest_count_pending(const struct obj *pl)
{
    struct obj *tmp;
    int qc=0;

    if (CONTR(pl)->quests_type_normal)
    {
        for (tmp = CONTR(pl)->quests_type_normal->inv; tmp; tmp = tmp->below, qc++)
            ;
    }
    if (CONTR(pl)->quests_type_kill)
    {
        for (tmp = CONTR(pl)->quests_type_kill->inv; tmp; tmp = tmp->below, qc++)
            ;
    }

    return qc;
}

/* send quest list for player op */
void send_quest_list(struct obj *pl)
{
    struct obj *tmp;
    int count = 0;
    char buf[HUGE_BUF];

    sprintf(buf, "<hf=\"st_003\"t=\"Quest list\"><at=\"Close\">");

    if (CONTR(pl)->quests_type_normal)
    {
        for (tmp = CONTR(pl)->quests_type_normal->inv; tmp; tmp = tmp->below)
        {
            char tmp_buf[8];

            if(tmp->item_skill > sizeof(skill_group_name) / sizeof(*skill_group_name))
            {
                LOG(llevBug, "BUG: quest item with invalid item_skill (%d): %s\n", tmp->item_skill, STRING_OBJ_NAME(tmp));
                tmp->item_skill = 0;
            }

            if (tmp->last_eat > 0 && tmp->stats.food > -1)
                sprintf(tmp_buf, ", %d", (tmp->last_eat - tmp->stats.food + 1));
            else
                *tmp_buf = '\0';

            sprintf(strchr(buf, '\0'), "<lt=\"%s%s%s (%s%d)%s\"c=\"s%d\">",
                    (QUERY_FLAG(tmp, FLAG_BLIND)) ? "+ " : "",
                    STRING_SAFE(tmp->name), tmp_buf,
                    skill_group_name[tmp->item_skill], tmp->item_level,
                    (quest_get_active_status(CONTR(pl), tmp) == QSTAT_SOLVED) ? " (complete)" : "",
                    ++count);
        }
    }

    if (CONTR(pl)->quests_type_kill)
    {
        for (tmp = CONTR(pl)->quests_type_kill->inv; tmp; tmp = tmp->below)
        {
            char tmp_buf[8];

            if(tmp->item_skill > sizeof(skill_group_name) / sizeof(*skill_group_name))
            {
                LOG(llevBug, "BUG: quest item with invalid item_skill (%d): %s\n", tmp->item_skill, STRING_OBJ_NAME(tmp));
                tmp->item_skill = 0;
            }

            if (tmp->last_eat > 0 && tmp->stats.food > -1)
                sprintf(tmp_buf, ", %d", (tmp->last_eat - tmp->stats.food + 1));
            else
                *tmp_buf = '\0';

            sprintf(strchr(buf, '\0'), "<lt=\"%s%s%s (%s%d)%s\"c=\"s%d\">",
                    (QUERY_FLAG(tmp, FLAG_BLIND)) ? "+ " : "",
                    STRING_SAFE(tmp->name), tmp_buf,
                    skill_group_name[tmp->item_skill], tmp->item_level,
                    (quest_get_active_status(CONTR(pl), tmp) == QSTAT_SOLVED) ? " (complete)" : "",
                    ++count);
        }
    }

    sprintf(strchr(buf, '\0'), "<mt=\"Quests: %d/%d\"b=\"%s\">",
            count, QUESTS_PENDING_MAX,
            (count) ? "Choose for details:" : "You have no open quests.");

    gui_npc(pl, GUI_NPC_MODE_QUEST, buf);
}

/* helper function - return a quest object which is #x of the players quests */
static inline struct obj * find_quest_nr(struct obj *pl, int tag, char *cmd)
{
    struct obj *tmp;
    int count = 0;

    if (CONTR(pl)->quests_type_normal)
    {
        for (tmp = CONTR(pl)->quests_type_normal->inv; tmp; tmp = tmp->below)
        {
            if(tag == ++count)
                return tmp;
        }
    }

    if (CONTR(pl)->quests_type_kill)
    {
        for (tmp = CONTR(pl)->quests_type_kill->inv; tmp; tmp = tmp->below)
        {
            if(tag == ++count)
                return tmp;
        }
    }

    LOG(llevNoLog,"QLIST-CMD: unknown quest tag from player %s: %s\n", STRING_OBJ_NAME(pl),cmd);
    return NULL;
}

void quest_list_command(struct obj *pl, char *cmd)
{
    if(!pl || cmd == '\0')
        return;

    if(*cmd == 'd') /* player ask for deleting a quest */
    {
        int nr = atoi(cmd+1);
        struct obj *quest;

        if ((quest = find_quest_nr(pl, nr, cmd)))
        {
            char buf[HUGE_BUF];

            sprintf(buf, "%s<mt=\"Skip quest?\"b=\"If you skip a quest, it will be removed from your quest list.\n\n"\
                         "The quest will still be available, should you wish to try again, but you will need to return to the NPC who offers it.\n\n"\
                         "Do you want to skip this quest?\"><at=\"Confirm\"c=\"x%d\"><dt=\"Don't skip\"c=\"l\">",
                    (quest->msg) ? quest->msg : "", nr);
            gui_npc(pl, GUI_NPC_MODE_QUEST, buf);
        }
    }
    else if(*cmd == 'x') /* player has confirmed to skip a quest */
    {
        int nr = atoi(cmd+1);
        struct obj *quest = find_quest_nr(pl, nr, cmd);

        if(quest)
        {
            new_draw_info(NDI_UNIQUE | NDI_ORANGE, 0, pl, "Quest '%s' removed from quest list!", quest->name);
            remove_ob(quest);
            send_quest_list(pl);
        }
    }
    else if(*cmd == 's') /* show a quest */
    {
        int     nr = atoi(cmd+1);
        object *quest;

        if ((quest = find_quest_nr(pl, nr, cmd)))
        {
            char    buf[HUGE_BUF];
            object *update;

            sprintf(buf, "%s<at=\"List quests\"c=\"l\"><dt=\"Skip quest\"c=\"d%d\"><mt=\"%s\"",
                    (quest->msg) ? quest->msg : "", nr,
                    (quest->name) ? quest->name : "Quest details");

            /* now create the status block lines */
            if(quest->sub_type1 == ST1_QUEST_TRIGGER_ITEM)
            {
                uint32 c;

                object *tmp;
                sprintf(strchr(buf, '\0'), "b=\"|Quest status:| %s\n",
                        (quest_get_active_status(CONTR(pl), quest) == QSTAT_SOLVED) ?
                        "|Complete!|" : "Incomplete");

                for (tmp = quest->inv; tmp; tmp = tmp->below)
                {
                    if (tmp->type == TYPE_QUEST_UPDATE)
                    {
                        continue;
                    }

                    if((c = get_nrof_quest_item(pl, tmp->arch->name, tmp->name, tmp->title)) < tmp->nrof)
                    {
                        sprintf(strchr(buf, '\0'), "\n%s: ~%d~/~%d~",
                                STRING_SAFE(tmp->name), c, tmp->nrof);
                    }
                    else
                    {
                        sprintf(strchr(buf, '\0'), "\n%s: ~%d~/~%d~ (|complete|)",
                                STRING_SAFE(tmp->name), c, tmp->nrof);
                    }
                }
            }
            else if(quest->sub_type1 == ST1_QUEST_TRIGGER_KILL)
            {
                object *tmp;

                sprintf(strchr(buf, '\0'), "b=\"|Quest status:| %s\n",
                        (quest_get_active_status(CONTR(pl), quest) == QSTAT_SOLVED) ?
                        "|Complete!|" : "Incomplete");

                for (tmp = quest->inv; tmp; tmp = tmp->below)
                {
                    if (tmp->type == TYPE_QUEST_UPDATE)
                    {
                        continue;
                    }

                    if(tmp->last_sp > tmp->level) /* not done */
                    {
                        sprintf(strchr(buf, '\0'), "\n%s: ~%d~/~%d~",
                                STRING_SAFE(tmp->name), tmp->level, tmp->last_sp);
                    }
                    else
                    {
                        sprintf(strchr(buf, '\0'), "\n%s: ~%d~/~%d~ (|complete|)",
                                STRING_SAFE(tmp->name), tmp->level, tmp->last_sp);
                    }
                }
            }
            else if(quest->sub_type1 == ST1_QUEST_TRIGGER_KILL_ITEM)
            {
                uint32 c;
                object *tmp;

                sprintf(strchr(buf, '\0'), "b=\"|Quest status:| %s\n",
                        (quest_get_active_status(CONTR(pl), quest) == QSTAT_SOLVED) ?
                        "|Complete!|" : "Incomplete");

                for (tmp = quest->inv; tmp; tmp = tmp->below)
                {
                    if (tmp->type == TYPE_QUEST_UPDATE)
                    {
                        continue;
                    }

                    if(!tmp->inv)
                        continue;

                    if((c =get_nrof_quest_item(pl, tmp->inv->arch->name, tmp->inv->name, tmp->inv->title)) < tmp->inv->nrof)
                    {
                        sprintf(strchr(buf, '\0'), "\n%s: ~%d~/~%d~",
                                STRING_SAFE(tmp->inv->name), c, tmp->inv->nrof);
                    }
                    else
                    {
                        sprintf(strchr(buf, '\0'), "\n%s: ~%d~/~%d~ (|complete|)",
                                STRING_SAFE(tmp->inv->name), c, tmp->inv->nrof);
                    }
                }
            }
            else /* normal */
            {
                sprintf(strchr(buf, '\0'), "b=\"|Quest status:| %s",
                        (quest_get_active_status(CONTR(pl), quest) == QSTAT_SOLVED) ?
                        "|Complete!|" : "Incomplete");
            }

            strcat(buf, "\n\">");

            for (update = quest->inv; update; update = update->below)
            {
                if (update->type != TYPE_QUEST_UPDATE)
                {
                    continue;
                }

                /* For various reasons we don't want too big an interface
                 * string. Generally about 2000 characters is a nice limit. */
                if (strlen(buf) > 2000)
                {
                    sprintf(strchr(buf, '\0'), "<ut=\"%s\"b=\"%s\">",
                            "...","Older updates not shown");
                    break;
                }

                /* Pre-r7336 the timestamp was held in ->title (as a shstr).
                 * Now it is in custom_attrset (as a normal string) but we must
                 * allow for old-style updates too. */
                sprintf(strchr(buf, '\0'), "<ut=\"%s\"b=\"%s\">",
                        (!update->custom_attrset) ? update->title : (char *)update->custom_attrset,
                        update->msg);
            }

            gui_npc(pl, GUI_NPC_MODE_QUEST, buf);
            CLEAR_FLAG(quest, FLAG_BLIND);
        }
    }
    else /* list quests */
    {
        send_quest_list(pl);
    }
}
