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

static inline int find_one_drop_quest_item(object *target, object *obj)
{
    object *tmp;

    if ((tmp=CONTR(target)->quest_one_drop) )
    {
        for (tmp = tmp->inv; tmp; tmp = tmp->below)
        {
            if (tmp->name == obj->name && tmp->race == obj->arch->name && tmp->title == obj->title)
                return 1;
        }
    }

    return 0;
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

/* recursive check the inventory for a specific quest item */
static int find_quest_item(object *target, object *obj)
{
    object *tmp;

    for (tmp = target->inv; tmp; tmp = tmp->below)
    {
        if(tmp->type == CONTAINER)
        {
            if(find_quest_item(tmp, obj))
                return 1;
        }
        else if (tmp->type == obj->type && tmp->name == obj->name && tmp->title == obj->title)
            return 1;
    }

    return 0;
}

/* we give a player a one drop item. This also
 * adds this item to the quest_container - instead to quest
 * items which will be added when the next quest step is triggered.
 * (most times from a quest script)
 */
static inline int add_one_drop_quest_item(object *target, object *obj)
{
    object *tmp, *q_tmp;

    if (!CONTR(target)->quest_one_drop)
        add_quest_containers(target);

    tmp = CONTR(target)->quest_one_drop;
    q_tmp = get_object();
    copy_object_data(obj, q_tmp); /* copy without put on active list */
    /* just to be on the secure side ... */
    q_tmp->speed = 0.0f;
    CLEAR_FLAG(q_tmp, FLAG_ANIMATE);
    CLEAR_FLAG(q_tmp, FLAG_ALIVE);
    /* we are storing the arch name of quest dummy items in race */
    FREE_AND_COPY_HASH(q_tmp->race, obj->arch->name);

    insert_ob_in_ob(q_tmp, tmp); /* dummy copy in quest container */
    SET_FLAG(obj, FLAG_IDENTIFIED);

    /*CLEAR_FLAG(obj, FLAG_QUEST_ITEM);*/

    q_tmp = get_object();
    copy_object(obj, q_tmp);
    /*CLEAR_FLAG(q_tmp, FLAG_SYS_OBJECT);*/
    insert_ob_in_ob(q_tmp, target); /* real object to player */
    esrv_send_item(target, q_tmp);

    return 1;
}

static inline int add_quest_item(object *target, object *obj)
{
    object *q_tmp;

    q_tmp = get_object();
    copy_object(obj, q_tmp);
    insert_ob_in_ob(q_tmp, target); /* real object to player */
    esrv_send_item(target, q_tmp);

    return 1;
}

void insert_quest_item(struct obj *quest_trigger, struct obj *target)
{
    char    auto_buf[MAX_BUF];
    int flag = FALSE;

    if (!target || target->type != PLAYER)
        return;

    if (QUERY_FLAG(quest_trigger, FLAG_ONE_DROP)) /* marks one drop quest items */
    {
        if(quest_trigger->level <= target->level)
        {
            object *tmp;

            for (tmp = quest_trigger->inv; tmp; tmp = tmp->below)
            {
                if(!find_one_drop_quest_item(target, tmp))
                {
                    if(quest_trigger->sub_type1 == ST1_QUEST_TRIGGER_CONT && !flag)
                        new_draw_info(NDI_UNIQUE | NDI_WHITE, 0, target, "You find something inside!");
                    flag = TRUE;
                    add_one_drop_quest_item(target, tmp);
                    sprintf(auto_buf, "You solved the one drop quest %s!", query_name(tmp));
                    new_draw_info(NDI_UNIQUE | NDI_NAVY, 0, target, auto_buf);
                }
            }
        }
    }
    else /* check the real quest stuff */
    {
        object *tmp, *quest;

        if((quest = find_quest_trigger(target, quest_trigger)))
        {
            for (tmp = quest_trigger->inv; tmp; tmp = tmp->below)
            {
                if(!find_quest_item(target, tmp))
                {

                    if(quest_trigger->sub_type1 == ST1_QUEST_TRIGGER_CONT && !flag)
                        new_draw_info(NDI_UNIQUE | NDI_WHITE, 0, target, "You find something inside!");
                    flag = TRUE;
                    add_quest_item(target, tmp);
                    sprintf(auto_buf, "You found the quest item %s!", query_name(tmp));
                    new_draw_info(NDI_UNIQUE | NDI_NAVY, 0, target, auto_buf);
                }
            }

            if((quest->magic != (sint8) quest_trigger->last_heal || flag) && quest_trigger->msg)
                new_draw_info(NDI_UNIQUE | NDI_ORANGE, 0, target, quest_trigger->msg);
            if(quest->magic != (sint8) quest_trigger->last_heal)
                quest->magic = (sint8) quest_trigger->last_heal;
        }
    }

    if(flag)
        play_sound_player_only(CONTR(target), SOUND_LEVEL_UP, SOUND_NORMAL, 0, 0);
}

/* if we found a player with missing or incomplete quest containers,
 * we install & set them here. We check also the integrety of the ptrs.
 * IMPORTANT! don't call this function BEFORE the player has been send to his
 * first fix_player() or we will double the containers.
 */
void add_quest_containers(struct obj *op)
{
    static archetype *archt = NULL;
    player *pl;

    if(op->type != PLAYER)
        return;

    /* lets fetch the quest_container static for speed up things */
    if(!archt)
        archt = find_archetype("quest_container");

    pl = CONTR(op);

    /* missing one drop container */
    if (!pl->quest_one_drop || !OBJECT_VALID(pl->quest_one_drop, pl->quest_one_drop_count) || pl->quest_one_drop->env != op)
    {
        pl->quest_one_drop = arch_to_object(archt);
        pl->quest_one_drop_count = pl->quest_one_drop->count;
        pl->quest_one_drop->sub_type1 = ST1_QUEST_ONE_DROP;
        insert_ob_in_ob(pl->quest_one_drop, op);
    }


    if(!pl->quests_done || !OBJECT_VALID(pl->quests_done,pl->quests_done_count) || pl->quests_done->env != op)
    {
        pl->quests_done = arch_to_object(archt);
        pl->quests_done_count = pl->quests_done->count;
        pl->quests_done->sub_type1 = ST1_QUESTS_TYPE_DONE;
        insert_ob_in_ob(pl->quests_done, op);
    }

    if(!pl->quests_type_cont || !OBJECT_VALID(pl->quests_type_cont,pl->quests_type_cont_count) || pl->quests_type_cont->env != op)
    {
        pl->quests_type_cont = arch_to_object(archt);
        pl->quests_type_cont_count = pl->quests_type_cont->count;
        pl->quests_type_cont->sub_type1 = ST1_QUESTS_TYPE_CONT;
        insert_ob_in_ob(pl->quests_type_cont, op);
    }

    if(!pl->quests_type_kill || !OBJECT_VALID(pl->quests_type_kill,pl->quests_type_kill_count) || pl->quests_type_kill->env != op)
    {
        pl->quests_type_kill = arch_to_object(archt);
        pl->quests_type_kill_count = pl->quests_type_kill->count;
        pl->quests_type_kill->sub_type1 = ST1_QUESTS_TYPE_KILL;
        insert_ob_in_ob(pl->quests_type_kill, op);
    }

    if(!pl->quests_type_normal || !OBJECT_VALID(pl->quests_type_normal,pl->quests_type_normal_count) || pl->quests_type_normal->env != op)
    {
        pl->quests_type_normal = arch_to_object(archt);
        pl->quests_type_normal_count = pl->quests_type_normal->count;
        pl->quests_type_normal->sub_type1 = ST1_QUESTS_TYPE_NORMAL;
        insert_ob_in_ob(pl->quests_type_normal, op);
    }
}

/* be sure trigger has the right settings which kind of trigger it is,
 * or the insert will FAIL.
 */
void add_quest_trigger(struct obj *who, struct obj *trigger)
{

    add_quest_containers(who);

    if(trigger->sub_type1 == ST1_QUEST_TRIGGER_NORMAL) /* normal */
        insert_ob_in_ob(trigger, CONTR(who)->quests_type_normal);
    else if(trigger->sub_type1 == ST1_QUEST_TRIGGER_KILL) /* kill */
        insert_ob_in_ob(trigger, CONTR(who)->quests_type_kill);
    else if(trigger->sub_type1 == ST1_QUEST_TRIGGER_CONT) /* cont */
        insert_ob_in_ob(trigger, CONTR(who)->quests_type_cont);
    else
    {
        LOG(llevBug, "BUG: add_quest_trigger(): wrong quest type %d\n", trigger->sub_type1);
        return;
    }
}

/* if we change the quest type and/or the quest_trigger status,
 * we must move the trigger from one container to another.
 */
void set_quest_status(struct obj *trigger, int q_status, int q_type)
{
    object *who = is_player_inv(trigger);

    if(!who)
    {
        LOG(llevBug, "set_quest_status(): trigger not in player/container\n", query_name(trigger->env));
        return;
    }

    remove_ob(trigger);

    trigger->sub_type1 = q_type;
    trigger->last_eat = q_status;

    if(q_status == -1) /* move it to quests_done */
        insert_ob_in_ob(trigger, CONTR(who)->quests_done);
    else
        add_quest_trigger(who, trigger);
}


/* If we are here, a player has killed or invoked the kill of something
 * Now, check the kill quest events (if there are some) and adjust the
 * quest_trigger.
 * NOTE: Be sure you checked quests_type_kill before!
 */
void check_kill_quest_event(struct obj *pl, struct obj *op)
{
    object *tmp;


    for (tmp = CONTR(pl)->quests_type_kill->inv; tmp; tmp = tmp->below)
    {
        /* hey, we got one! */
        if(tmp->other_arch == op->arch && tmp->slaying == op->name && tmp->title == op->title)
        {
            /* we have any event left in this trigger? */
            if(tmp->level < tmp->last_sp)
            {
                char buf[MAX_BUF];

                tmp->level++;
                /* tell player he has triggered a kill quest again */
                sprintf(buf, "You killed %d of %d %s.", tmp->level, tmp->last_sp,query_name(op));
                new_draw_info(NDI_NAVY, 0, pl, buf);
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
        if(tmp->type ==TYPE_QUEST_TRIGGER && tmp->sub_type1 == ST1_QUEST_TRIGGER_CONT)
            insert_quest_item(tmp, pl);
    }
}
