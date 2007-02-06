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

static char *skill_group_name[] = {
    "","agility ", "personality ","mental ","physical ","magic ","wisdom ","",""
};

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

/* check the player we can drop a one-drop quest trigger item */
static inline int find_one_drop_quest_item(object *target, object *obj)
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

    /* only mark the item as "real" one drop if it has the flag */
    if(QUERY_FLAG(obj, FLAG_ONE_DROP))
    {
        tmp = CONTR(target)->quest_one_drop;
        q_tmp = get_object();
        copy_object_data(obj, q_tmp); /* copy without put on active list */
        /* just to be on the secure side ... */
        q_tmp->speed = 0.0f;
        CLEAR_FLAG(q_tmp, FLAG_ANIMATE);
        CLEAR_FLAG(q_tmp, FLAG_ALIVE);
        SET_FLAG(q_tmp, FLAG_IDENTIFIED);
        /* we are storing the arch name of quest dummy items in race */
        FREE_AND_COPY_HASH(q_tmp->race, obj->arch->name);
        insert_ob_in_ob(q_tmp, tmp); /* dummy copy in quest container */
        SET_FLAG(obj, FLAG_IDENTIFIED);
    }

    /* now give it the player */
    q_tmp = get_object();
    copy_object(obj, q_tmp);
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
    int flag = FALSE;

    if (!target || target->type != PLAYER)
        return;

    /* its a one drop - it triggers when the drop chance is not set or triggers */
    if (QUERY_FLAG(quest_trigger, FLAG_ONE_DROP)
            && (!quest_trigger->last_grace || !(RANDOM() % (quest_trigger->last_grace+1)))) /* marks one drop quest items */
    {
        int tmp_lev = 0;

        if (quest_trigger->item_skill)
            tmp_lev = CONTR(target)->exp_obj_ptr[quest_trigger->item_skill-1]->level; /* use player struct shortcut ptrs */
        else
            tmp_lev = target->level;

        if(quest_trigger->item_level <= tmp_lev)
        {
            char    auto_buf[MAX_BUF];
            object *tmp;

            for (tmp = quest_trigger->inv; tmp; tmp = tmp->below)
            {
                if(!find_one_drop_quest_item(target, tmp))
                {
                    flag = TRUE;
                    SET_FLAG(tmp, FLAG_IDENTIFIED); /* be sure the one drop is IDed - nicer gaming experience */
                    add_one_drop_quest_item(target, tmp);

                    if(QUERY_FLAG(tmp,FLAG_ONE_DROP))
                        sprintf(auto_buf, "You found the one drop %s!", query_short_name(tmp, target));
                    else
                        sprintf(auto_buf, "You found the special drop %s!", query_short_name(tmp, target));
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
            for (tmp = quest->inv; tmp; tmp = tmp->below)
            {
                if(!find_quest_item(target, tmp))
                {
                    flag = TRUE;
                    add_quest_item(target, tmp);
                    new_draw_info_format(NDI_UNIQUE | NDI_NAVY, 0, target, "You found the quest item %s!", query_short_name(tmp, target));
                }
            }

            if(quest->magic != (sint8) quest_trigger->last_heal ||flag)
            {
                if(!flag)
                    new_draw_info_format(NDI_UNIQUE | NDI_NAVY, 0, target, "You completed the quest '%s'!", query_short_name(quest, target));

                if(quest_trigger->msg)
                    new_draw_info(NDI_UNIQUE | NDI_ORANGE, 0, target, quest_trigger->msg);

                quest->magic = (sint8) quest_trigger->last_heal;
            }
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
        FREE_AND_COPY_HASH(pl->quest_one_drop->name, "QC: onedrops");
        insert_ob_in_ob(pl->quest_one_drop, op);
    }


    if(!pl->quests_done || !OBJECT_VALID(pl->quests_done,pl->quests_done_count) || pl->quests_done->env != op)
    {
        pl->quests_done = arch_to_object(archt);
        pl->quests_done_count = pl->quests_done->count;
        pl->quests_done->sub_type1 = ST1_QUESTS_TYPE_DONE;
        FREE_AND_COPY_HASH(pl->quests_done->name, "QC: list done");
        insert_ob_in_ob(pl->quests_done, op);
    }

    if(!pl->quests_type_kill || !OBJECT_VALID(pl->quests_type_kill,pl->quests_type_kill_count) || pl->quests_type_kill->env != op)
    {
        pl->quests_type_kill = arch_to_object(archt);
        pl->quests_type_kill_count = pl->quests_type_kill->count;
        pl->quests_type_kill->sub_type1 = ST1_QUESTS_TYPE_KILL;
        FREE_AND_COPY_HASH(pl->quests_type_kill->name, "QC: kill");
        insert_ob_in_ob(pl->quests_type_kill, op);
    }

    if(!pl->quests_type_normal || !OBJECT_VALID(pl->quests_type_normal,pl->quests_type_normal_count) || pl->quests_type_normal->env != op)
    {
        pl->quests_type_normal = arch_to_object(archt);
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

    add_quest_containers(who);

    if(trigger->sub_type1 == ST1_QUEST_TRIGGER_NORMAL || trigger->sub_type1 == ST1_QUEST_TRIGGER_ITEM)
        insert_ob_in_ob(trigger, CONTR(who)->quests_type_normal);
    else if(trigger->sub_type1 == ST1_QUEST_TRIGGER_KILL ||
            trigger->sub_type1 == ST1_QUEST_TRIGGER_KILL_ITEM) /* kill */
        insert_ob_in_ob(trigger, CONTR(who)->quests_type_kill);
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
    {
        remove_ob_inv(trigger); /* clear quest data inventory before we neutralize */
        insert_ob_in_ob(trigger, CONTR(who)->quests_done);
    }
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
    object *tmp, *tmp_info;

    if (!CONTR(pl)->quests_type_kill)
        add_quest_containers(pl);

    /* browse the quest triggers */
    for (tmp = CONTR(pl)->quests_type_kill->inv; tmp; tmp = tmp->below)
    {
        /* inside the quest triggers are quest_info object with the kill info */
        for(tmp_info=tmp->inv;tmp_info;tmp_info=tmp_info->below)
        {
            if( tmp_info->race && tmp_info->race != op->arch->name)
                continue; /* quick check we have something legal */

            /* we have a hit when one of this 3 possible names match */
            if( (tmp_info->name && tmp_info->name == op->name)
                    || (tmp_info->title && tmp_info->title == op->name)
                    || (tmp_info->slaying && tmp_info->slaying == op->name))
            {
                /* the drop/kill chance can be random ... */
                if(tmp_info->last_grace && (RANDOM() % (tmp_info->last_grace+1)))
                    continue; /* good kill, bad luck, no item */

                /* ok, we have a hit... now lets check what we have - kill or kill item */
                if(tmp->sub_type1 == ST1_QUEST_TRIGGER_KILL_ITEM)
                {
                    uint32 nrof;

                    /* Its real: give the item inside the quest_info the player */
                    if(!tmp_info->inv)
                    {
                        LOG(llevBug,"BUG: check_kill_quest_event:: tmp_info has no inventory item (quest %s)\n",
                                query_name(tmp));
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
                        esrv_send_item(pl, insert_ob_in_ob(newob, pl));
                    }

                    if(nrof > tmp_info->inv->nrof)
                    {
                        nrof = tmp_info->inv->nrof;
                        if(nrof)
                            nrof = 1;
                    }

                    new_draw_info_format(NDI_NAVY, 0, pl, "Quest %s\n%s: %d/%d",
                            STRING_SAFE(tmp->name), query_short_name(tmp_info->inv, NULL),
                            nrof, (tmp_info->inv->nrof?tmp_info->inv->nrof:1));
                }
                else if(tmp_info->level < tmp_info->last_sp) /* pure kill quest - alot easier */
                {
                    new_draw_info_format(NDI_NAVY, 0, pl, "Quest %s\n%s: %d/%d",STRING_SAFE(tmp->name),
                            query_name(op), ++tmp_info->level, tmp_info->last_sp);
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


static inline int check_quest_complete(struct obj *target, struct obj *quest)
{
    object *tmp;

    /* FIXME: Is this test really correct? The following seems more correct:
     * if(!quest || quest->magic < quest->state || quest->last_eat == -1)
     * (state stores the "finish" step for "normal" quests, quest->magic is the counter)
     * Gecko 2006-11-15 */
    if(!quest || quest->magic < quest->last_heal || quest->last_eat == -1)
        return FALSE;

    /* quest is complete when... */
    if(quest->sub_type1 == ST1_QUEST_TRIGGER_KILL)
    {
        /* ... we have killed the defined number of all target objects */
        for (tmp = quest->inv; tmp; tmp = tmp->below)
        {
            if(tmp->last_sp > tmp->level)
                return FALSE;
        }
        return TRUE;
    }
    /* ... we have the defined number of items in our inventory */
    else if(quest->sub_type1 == ST1_QUEST_TRIGGER_KILL_ITEM)
    {
        for (tmp = quest->inv; tmp; tmp = tmp->below)
        {
            if(!tmp->inv)
                continue;
            if(get_nrof_quest_item(target, tmp->inv->arch->name, tmp->inv->name, tmp->inv->title) < tmp->inv->nrof)
                return FALSE;
        }
        return TRUE;

    }
    else if(quest->sub_type1 == ST1_QUEST_TRIGGER_ITEM)
    {
        for (tmp = quest->inv; tmp; tmp = tmp->below)
        {
            if(get_nrof_quest_item(target, tmp->arch->name, tmp->name, tmp->title) < tmp->nrof)
                return FALSE;
        }
        return TRUE;

    }
    else /* normal */
    {
        if(quest->state == quest->magic)
            return TRUE;
    }

    return FALSE;
}


/* lets find a quest with name */
extern struct obj *quest_find_name(const struct obj *pl, const char *name)
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
    char msg[MAX_BUF], buf[HUGE_BUF]="";

    if (CONTR(pl)->quests_type_normal)
    {
        for (tmp = CONTR(pl)->quests_type_normal->inv; tmp; tmp = tmp->below)
        {
            if(tmp->item_skill > sizeof(skill_group_name) / sizeof(*skill_group_name))
            {
                LOG(llevBug, "BUG: quest item with invalid item_skill (%d): %s\n", tmp->item_skill, STRING_OBJ_NAME(tmp));
                tmp->item_skill = 0;
            }

            sprintf(msg,"<lt=\"%s °(%s%d) %s°\" c=\"#%d\">",STRING_SAFE(tmp->name),
                    skill_group_name[tmp->item_skill], tmp->item_level,
                    check_quest_complete(pl, tmp)?"(complete)":"", ++count);
            strcat(buf, msg);
        }
    }

    if (CONTR(pl)->quests_type_kill)
    {
        for (tmp = CONTR(pl)->quests_type_kill->inv; tmp; tmp = tmp->below)
        {
            if(tmp->item_skill > sizeof(skill_group_name) / sizeof(*skill_group_name))
            {
                LOG(llevBug, "BUG: quest item with invalid item_skill (%d): %s\n", tmp->item_skill, STRING_OBJ_NAME(tmp));
                tmp->item_skill = 0;
            }

            sprintf(msg,"<lt=\"%s °(%s%d) %s°\" c=\"#%d\">",STRING_SAFE(tmp->name),
                    skill_group_name[tmp->item_skill], tmp->item_level,
                    check_quest_complete(pl, tmp)?"(complete)":"", ++count);
            strcat(buf, msg);
        }
    }

    if(count)
        sprintf(msg, "<bt=\"Close\"><wb=\"Q\"><hf=\"quests\"b=\"Quests: °%d°/°%d°\"><mt=\"QUEST LIST\"b=\"Click for details:\">", count, QUESTS_PENDING_MAX);
    else
        sprintf(msg, "<bt=\"Close\"><wb=\"Q\"><hf=\"quests\"b=\"Quests: °0°/°%d°\"><mt=\"QUEST LIST\"b=\"You has no open or pending quests.\">", QUESTS_PENDING_MAX);

    strcat(buf, msg);

    gui_interface(pl, NPC_INTERFACE_MODE_NPC, buf, NULL);
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

    LOG(-1,"QLIST-CMD: unknown quest tag from player %s: %s\n", query_name(pl),cmd);
    return NULL;
}

/* player has send a "tx" talk extension - analyse it */
void quest_list_command(struct obj *pl, char *cmd)
{
    if(!pl || cmd == '\0')
        return;

    if(*cmd == 'D') /* player ask for deleting a quest */
    {
        int nr = atoi(cmd+1);
        struct obj *quest = find_quest_nr(pl, nr, cmd);

        if(quest)
        {
            char buf[MAX_BUF];

            sprintf( buf,"<wb=\"Q\"><at=\"Remove\"c=\"X%d\"><dt=\"Back\"c=\"#%d\"><xt=\"Remove Quest?\"b=\"\n\n°You want remove this quest from quest list?°\">", nr, nr);
            gui_interface(pl, NPC_INTERFACE_MODE_NPC, quest->msg, buf);
        }
    }
    else if(*cmd == 'X') /* player has confirmed to skip a quest */
    {
        int nr = atoi(cmd+1);
        struct obj *quest = find_quest_nr(pl, nr, cmd);

        if(quest)
        {
            new_draw_info_format(NDI_UNIQUE | NDI_ORANGE, 0, pl, "Quest '%s' removed from quest list!", quest->name);
            remove_ob(quest);
            send_quest_list(pl);
        }
    }
    else if(*cmd == 'L') /* show a quest */
    {
        send_quest_list(pl);
    }
    else if(*cmd == '#') /* show a quest */
    {
        int nr = atoi(cmd+1);
        struct obj *quest = find_quest_nr(pl, nr, cmd);

        if(quest)
        {
            char buf[HUGE_BUF], msg[MAX_BUF];

            sprintf(buf,"<at=\"Back\"c=\"L\"><dt=\"Skip Quest\"c=\"D%d\"><xt=\"%s\"", nr, STRING_SAFE(quest->name));

            /* now create the status block lines */
            if(quest->sub_type1 == ST1_QUEST_TRIGGER_ITEM)
            {
                uint32 c;

                object *tmp;

                strcat(buf,"b=\"\n\n°STATUS:°");
                for (tmp = quest->inv; tmp; tmp = tmp->below)
                {
                    if((c = get_nrof_quest_item(pl, tmp->arch->name, tmp->name, tmp->title)) < tmp->nrof)
                    {
                        sprintf(msg, "\n%s: %d/%d", STRING_SAFE(tmp->name), c, tmp->nrof);
                    }
                    else
                    {
                        sprintf(msg, "\n°%s: %d/%d (complete)°", STRING_SAFE(tmp->name), c, tmp->nrof);
                    }
                    strcat(buf, msg);
                }
                strcat(buf, "\"");
            }
            else if(quest->sub_type1 == ST1_QUEST_TRIGGER_KILL)
            {
                object *tmp;

                strcat(buf,"b=\"\n\n°STATUS:°");
                for (tmp = quest->inv; tmp; tmp = tmp->below)
                {
                    if(tmp->last_sp > tmp->level) /* not done */
                    {
                        sprintf(msg, "\n%s: %d/%d", STRING_SAFE(tmp->name), tmp->level, tmp->last_sp);
                    }
                    else
                    {
                        sprintf(msg, "\n°%s: %d/%d (complete)°", STRING_SAFE(tmp->name), tmp->level, tmp->last_sp);
                    }
                    strcat(buf, msg);
                }
                strcat(buf, "\"");
            }
            else if(quest->sub_type1 == ST1_QUEST_TRIGGER_KILL_ITEM)
            {
                uint32 c;
                object *tmp;

                strcat(buf,"b=\"\n\n°STATUS:°");
                for (tmp = quest->inv; tmp; tmp = tmp->below)
                {
                    if(!tmp->inv)
                        continue;

                    if((c =get_nrof_quest_item(pl, tmp->inv->arch->name, tmp->inv->name, tmp->inv->title)) < tmp->inv->nrof)
                    {
                        sprintf(msg, "\n%s: %d/%d", STRING_SAFE(tmp->inv->name), c, tmp->inv->nrof);
                    }
                    else
                    {
                        sprintf(msg, "\n°%s: %d/%d (complete)°", STRING_SAFE(tmp->inv->name), c, tmp->inv->nrof);
                    }
                    strcat(buf, msg);
                }
                strcat(buf, "\"");

            }
            else /* normal */
            {
                if(quest->state == quest->magic)
                    strcat(buf,"b=\"\n\n°STATUS:°\n°Complete!°\"");
                else
                    strcat(buf,"b=\"\n\n°STATUS:°\nIncomplete\"");
            }
            strcat(buf,"><wb=\"Q\">");
            gui_interface(pl, NPC_INTERFACE_MODE_NPC, quest->msg, buf);
        }
    }
    else
        LOG(-1,"QLIST-CMD: unknown tag from player %s: %s\n", query_name(pl),cmd);
}
