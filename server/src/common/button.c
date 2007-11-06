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

/* When a map is loaded, its buttons are synced. We don't want
 * to trigger scripts then so we use this global to indicate it */
static int ignore_trigger_events = 0;

/* Send signal from op to connection ol */
static void signal_connection(object *op, oblinkpt *olp, object *activator, object *originator)
{
    object     *tmp;
    objectlink *ol;

    /* tmp->weight_limit == state of trigger */

    if(! ignore_trigger_events)
        if(trigger_object_plugin_event(EVENT_TRIGGER,
                op, activator, originator, NULL, NULL, NULL, NULL, SCRIPT_FIX_NOTHING))
            return;

    /*LOG(llevDebug, "push_button: %s (%d)\n", op->name, op->count);*/
    for (ol = olp; ol; ol = ol->next)
    {
        tmp = ol->objlink.ob;
        if (!tmp || tmp->count != ol->id)
        {
            LOG(llevBug, "BUG: Internal error in push_button (%s).\n", STRING_OBJ_NAME(op));
            continue;
        }

        /* a button link object can become freed when the map is saving.  As
         * a map is saved, objects are removed and freed, and if an object is
         * on top of a button, this function is eventually called.  If a map
         * is getting moved out of memory, the status of buttons and levers
         * probably isn't important - it will get sorted out when the map is
         * re-loaded.  As such, just exit this function if that is the case.
         */
        if (!OBJECT_ACTIVE(tmp))
        {
            LOG(llevDebug, "DEBUG: signal_connection(): button link with invalid object! (%x - %p). Button '%s' => object '%s'\n",
                QUERY_FLAG(tmp, FLAG_REMOVED), tmp,
                STRING_OBJ_NAME(op), STRING_OBJ_NAME(tmp));
            return;
        }

        if(! ignore_trigger_events && tmp != op)
            if(trigger_object_plugin_event(EVENT_TRIGGER,
                    tmp, activator, originator, NULL, NULL, NULL, NULL, SCRIPT_FIX_NOTHING))
                continue;

        switch (tmp->type)
        {
            case GATE:
            case PIT:
              tmp->weight_limit = tmp->stats.maxsp ? !op->weight_limit : op->weight_limit;
              tmp->speed = 0.5;
              update_ob_speed(tmp);
              break;

            case CF_HANDLE:
              SET_ANIMATION(tmp,
                            ((NUM_ANIMATIONS(tmp) / NUM_FACINGS(tmp)) * tmp->direction)
                          + (tmp->weight_limit = tmp->stats.maxsp ? !op->weight_limit : op->weight_limit));
              update_object(tmp, UP_OBJ_FACE);
              break;

            case SIGN:
              /* Ignore map loading triggers */
              if(ignore_trigger_events)
                  break;
              if (!tmp->stats.food || tmp->last_eat < tmp->stats.food)
              {
                  new_info_map(NDI_UNIQUE | NDI_NAVY, tmp->map, tmp->x, tmp->y, MAP_INFO_NORMAL, tmp->msg);
                  if (tmp->stats.food)
                      tmp->last_eat++;
              }
              break;

            case ALTAR:
              tmp->weight_limit = 1;
              SET_ANIMATION(tmp, ((NUM_ANIMATIONS(tmp) / NUM_FACINGS(tmp)) * tmp->direction) + tmp->weight_limit);
              update_object(tmp, UP_OBJ_FACE);
              break;

            case BUTTON:
            case PEDESTAL:
              tmp->weight_limit = op->weight_limit;
              SET_ANIMATION(tmp, ((NUM_ANIMATIONS(tmp) / NUM_FACINGS(tmp)) * tmp->direction) + tmp->weight_limit);
              update_object(tmp, UP_OBJ_FACE);
              break;

            case MOOD_FLOOR:
              /* Ignore map loading triggers */
              if(ignore_trigger_events)
                  break;
              do_mood_floor(tmp, op);
              break;

            case TIMED_GATE:
              /* Ignore map loading triggers */
              if(ignore_trigger_events)
                  break;
              tmp->speed = 0.5;
              update_ob_speed(tmp);  /* original values */
              tmp->weight_limit = !tmp->stats.maxsp;
              tmp->stats.sp = 1;
              tmp->stats.hp = tmp->stats.maxhp;
              break;

            case FIREWALL:
              /* Ignore map loading triggers */
              if(ignore_trigger_events)
                  break;
              if (op->last_eat) /* connection flag1 = on/off */
                  tmp->last_eat != 0 ? (tmp->last_eat = 0) : (tmp->last_eat = 1);
              /* (*move_firewall_func)(tmp); <- invoke the firewall (removed here)*/
              else
              {
                  /* "normal" connection - turn wall */
                  if (tmp->stats.maxsp) /* next direction */
                  {
                      if ((tmp->direction += tmp->stats.maxsp) > 8)
                          tmp->direction = (tmp->direction % 8) + 1;
                      animate_turning(tmp);
                  }
              }
              break;

            case DIRECTOR:
              /* Ignore map loading triggers */
              if(ignore_trigger_events)
                  break;
              if (tmp->stats.maxsp) /* next direction */
              {
                  if ((tmp->direction += tmp->stats.maxsp) > 8)
                      tmp->direction = (tmp->direction % 8) + 1;
                  animate_turning(tmp);
              }
              break;

            case TELEPORTER:
              /* Ignore map loading triggers
               * Rationale: connected teleports should only trigger
               * at connection state changes. */
              if(ignore_trigger_events)
                  break;
              move_teleporter(tmp);
              break;

            case CREATOR:
              /* Ignore map loading triggers */
              if(ignore_trigger_events)
                  break;
              /* Ignore "off" signals */
              if(! op->weight_limit)
                  break;
              move_creator(tmp);
              break;

            case TYPE_TIMER:
              /* any signal to a stopped timer means for it to start */
              if(tmp->speed == 0)
              {
                tmp->speed = MAX_TIME/1000000.0f; /* once per second */
                tmp->stats.hp = tmp->stats.maxhp;
                update_ob_speed(tmp);
              }
              break;

            case TYPE_CONN_SENSOR:
              /* Connection sensors don't listen to themselves */
              if(op != tmp)
                  move_conn_sensor(tmp);
              break;

            case TYPE_LIGHT_APPLY:
            case LIGHT_SOURCE: /* Dunno if this really works for LIGHT_SOURCE */
              if(op->weight_limit == 0 && tmp->glow_radius > 0)
                  turn_off_light(tmp);
              else if(op->weight_limit != 0 && tmp->glow_radius == 0)
                  turn_on_light(tmp);
              break;

            case SPAWN_POINT:
              /* Ignore map loading triggers */
              if(ignore_trigger_events)
                  break;
              if(op->weight_limit) /* Only trigger on positive edge */
                  spawn_point(tmp);
              break;
        }
    }
}

/*
 * JRG (Grommit) 6-Nov-2007
 * Remove button down checks to separate function
 * (called by update_button)
 */
int check_button_down(object *tmp)
{
    object     *ab, *head;
    unsigned int fly, move;
    int tot;
    int is_down = 0;

    if (tmp->type == BUTTON)
    {
        fly = QUERY_FLAG(tmp, FLAG_FLY_ON);
        move = QUERY_FLAG(tmp, FLAG_WALK_ON);
        tot = 0;
        for (ab = GET_BOTTOM_MAP_OB(tmp); ab != NULL; ab = ab->above)
        {
            if (ab != tmp && (fly ? QUERY_FLAG(ab, FLAG_FLYING) : move))
                tot += ab->weight * (ab->nrof ? ab->nrof : 1) + ab->carrying;
        }
        tmp->weight_limit = (tot >= tmp->weight) ? 1 : 0;
        if (tmp->weight_limit)
            is_down = 1;
    }
    else if (tmp->type == PEDESTAL)
    {
        tmp->weight_limit = 0;
        fly = QUERY_FLAG(tmp, FLAG_FLY_ON);
        move = QUERY_FLAG(tmp, FLAG_WALK_ON);
        for (ab = GET_BOTTOM_MAP_OB(tmp); ab != NULL; ab = ab->above)
        {
            head = ab->head ? ab->head : ab;
            if (ab != tmp
             && (fly ? QUERY_FLAG(ab, FLAG_FLYING) : move)
             && (head->race == tmp->slaying || (tmp->slaying == shstr_cons.player && head->type == PLAYER)))
                tmp->weight_limit = 1;
        }
        if (tmp->weight_limit)
            is_down = 1;
    }
    return is_down;
 }
/*
 * Updates everything connected with the button op.
 * After changing the state of a button, this function must be called
 * to make sure that all gates and other buttons connected to the
 * button reacts to the (eventual) change of state.
 */
void update_button(object *op, object *activator, object *originator)
{
    object     *tmp;
    int         any_down = 0;
    sint32      old_value = op->weight_limit;
    objectlink *ol;
    int         has_links = 0;

    /* LOG(llevDebug, "update_button: %s (%d)\n", op->name, op->count); */
    for (ol = get_button_links(op); ol; ol = ol->next)
    {
        has_links = 1;
        if (!ol->objlink.ob || ol->objlink.ob->count != ol->id)
        {
            LOG(llevDebug, "Internal error in update_button (%s).\n", op->name);
            continue;
        }
        tmp = ol->objlink.ob;
        if (check_button_down(tmp))
            any_down = 1;
    }

    /* JRG (Grommit) fix 6-Nov-2007 */
    if (!has_links)
    {
        /* No connection, but operate this button anyway (to trigger script if any) */
        if (check_button_down(op))
            any_down = 1;
    }

    if (any_down) /* If any other buttons were down, force this to remain down */
        op->weight_limit = 1;

    /* If this button hasn't changed, don't do anything */
    if (op->weight_limit != old_value)
    {
        SET_ANIMATION(op, ((NUM_ANIMATIONS(op) / NUM_FACINGS(op)) * op->direction) + op->weight_limit);
        update_object(op, UP_OBJ_FACE);
        push_button(op, activator, originator); /* Make all other buttons the same */
    }
}

/*
 * Updates every button on the map (by calling update_button() for them).
 * This is only done when a map loads.
 */

void update_buttons(mapstruct *m)
{
    objectlink *ol;
    oblinkpt   *obp;
    object     *ab, *tmp;
    unsigned int fly, move;

    /* Don't trigger plugin events from this function */
    ignore_trigger_events = 1;

    for (obp = m->buttons; obp; obp = obp->next)
    {
        for (ol = obp->objlink.link; ol; ol = ol->next)
        {
            if (!ol->objlink.ob || ol->objlink.ob->count != ol->id)
            {
                LOG(llevBug, "BUG: Internal error in update_button (%s (%dx%d):%d, connected %d ).\n",
                    ol->objlink.ob ? STRING_SAFE(ol->objlink.ob->name) : "null",
                    ol->objlink.ob ? ol->objlink.ob->x : -1, ol->objlink.ob ? ol->objlink.ob->y : -1, ol->id, obp->value);
                continue;
            }
            switch(ol->objlink.ob->type)
            {
                case BUTTON:
                case PEDESTAL:
                    update_button(ol->objlink.ob, NULL, NULL);
                    break;

                case CHECK_INV:
                    tmp = ol->objlink.ob;
                    fly = QUERY_FLAG(tmp, FLAG_FLY_ON);
                    move = QUERY_FLAG(tmp, FLAG_WALK_ON);

                    for (ab = GET_BOTTOM_MAP_OB(tmp); ab != NULL; ab = ab->above)
                    {
                        if (ab != tmp && (fly ? QUERY_FLAG(ab, FLAG_FLYING) : move))
                            check_inv(ab, tmp);
                    }
                    break;

                case TRIGGER_BUTTON:
                case TRIGGER_PEDESTAL:
                case TRIGGER_ALTAR:
                    /* check_trigger will itself sort out the numbers of
                     * items above the trigger */
                    check_trigger(ol->objlink.ob, ol->objlink.ob, NULL);
                    break;

                case TYPE_CONN_SENSOR:
                    move_conn_sensor(ol->objlink.ob);
                    break;

                case TYPE_ENV_SENSOR:
                    move_environment_sensor(ol->objlink.ob);
                    break;

                case CF_HANDLE:
                case TRIGGER:
                    push_button(ol->objlink.ob, NULL, NULL);
                    break;

                default:
                    break;
            }
        }
    }

    ignore_trigger_events = 0;
}

/*
 * Push the specified object.  This can affect other buttons/gates/handles
 * altars/pedestals/holes in the whole map.
 */
void push_button(object *op, object *pusher, object *originator)
{
    signal_connection(op, get_button_links(op), pusher, originator);
}

void use_trigger(object *op, object *user)
{
    /* Toggle value */
    op->weight_limit = !op->weight_limit;
    push_button(op, user, user);
}

/* We changed this function to fit in the anim system. This is used from
 * DIRECTORS and FIREWALLS but not from all is_turning objects (even not in crossfire,
 * throwing weapons use different system). This is of course confusing - directors
 * used is_throwing with 8 animations, throwing weapons with 9.
 * In Daimonin, i change it for use with facings 9 and facings 25 ext anim system.
 * To do it, we must change 2 things: All is_throwing using objects must have a facings 9
 * or facings 25 animation. All must use ->direction to set the animation.
 * In the old version, the object here sets animation by using ->sp. We must change this,
 * that this technical value (sp menas for directors the direction of reflection and
 * for firewalls the fire direction) map to the right animation value - this is simple
 * the right setting. And then all problems are gone - if we setup the animation right,
 * we can do all the things our animation can do - means also in frame animation of
 * our turning objects and so on. MT 2003.
 */
void animate_turning(object *op) /* only one part objects */
{
    /* here we move through or frames - we animate the animation */
    /* state animation should be done from our animation handler now -
     * if is_animated 0 set, we don't want animate here too.
    */
    SET_ANIMATION(op, ((NUM_ANIMATIONS(op) / NUM_FACINGS(op)) * op->direction) + op->state);
    update_object(op, UP_OBJ_FACE);
}

#define ARCH_SACRIFICE(xyz) ((xyz)->slaying)
#define NROF_SACRIFICE(xyz) ((xyz)->stats.food)

/* Returns true if the sacrifice meets the needs of the altar.
 *
 * Function put in (0.92.1) so that identify altars won't grab money
 * unnecessarily - we can see if there is sufficient money, see if something
 * needs to be identified, and then remove money if needed.
 *
 * 0.93.4: Linked objects (ie, objects that are connected) can not be
 * sacrificed.  This fixes a bug of trying to put multiple altars/related
 * objects on the same space that take the same sacrifice.
 */

int check_altar_sacrifice(object *altar, object *sacrifice)
{
    if (!IS_LIVE(sacrifice) && !QUERY_FLAG(sacrifice, FLAG_IS_LINKED))
    {
        if ((ARCH_SACRIFICE(altar) == sacrifice->arch->name
          || ARCH_SACRIFICE(altar) == sacrifice->name
          || ARCH_SACRIFICE(altar) == sacrifice->slaying)
         && NROF_SACRIFICE(altar)
         <= (sint16)
            (sacrifice->nrof ? sacrifice->nrof : 1))
            return 1;
        if (ARCH_SACRIFICE(altar) == shstr_cons.money
         && sacrifice->type == MONEY
         && sacrifice->nrof * sacrifice->value >= (uint32) NROF_SACRIFICE(altar))
            return 1;
    }
    return 0;
}


/*
 * operate_altar checks if sacrifice was accepted and removes sacrificed
 * objects.  If sacrifice was succeed return 1 else 0.  Might be better to
 * call check_altar_sacrifice (above) than depend on the return value,
 * since operate_altar will remove the sacrifice also.
 *
 * If this function returns 1, '*sacrifice' is modified to point to the
 * remaining sacrifice, or is set to NULL if the sacrifice was used up.
 */

int operate_altar(object *altar, object **sacrifice)
{
    if (!altar->map)
    {
        LOG(llevBug, "BUG: operate_altar(): altar has no map\n");
        return 0;
    }

    if (!altar->slaying || altar->value)
        return 0;

    if (!check_altar_sacrifice(altar, *sacrifice))
        return 0;

    /* check_altar_sacrifice should have already verified that enough money
     * has been dropped.
     */
    if (ARCH_SACRIFICE(altar) == shstr_cons.money)
    {
        int number  = (int) (NROF_SACRIFICE(altar) / (*sacrifice)->value);

        /* Round up any sacrifices.  Altars don't make change either */
        if (NROF_SACRIFICE(altar) % (*sacrifice)->value)
            number++;
        *sacrifice = decrease_ob_nr(*sacrifice, number);
    }
    else
        *sacrifice = decrease_ob_nr(*sacrifice, NROF_SACRIFICE(altar));

    if (altar->msg)
        new_info_map(NDI_WHITE, altar->map, altar->x, altar->y, MAP_INFO_NORMAL, altar->msg);
    return 1;
}

void trigger_move(object *op, int state, object *originator) /* 1 down and 0 up */
{
    op->stats.wc = state;
    if (state)
    {
        use_trigger(op, originator);
        op->speed = 1.0f / (float) op->arch->clone.stats.exp;
        update_ob_speed(op);
        op->speed_left = -1;
    }
    else
    {
        use_trigger(op, originator);
        op->speed = 0;
        update_ob_speed(op);
    }
}


/*
 * cause != NULL: something has moved on top of op
 *
 * cause == NULL: nothing has moved, we have been called from
 * animate_trigger().
 *
 * TRIGGER_ALTAR: Returns 1 if 'cause' was destroyed, 0 if not.
 *
 * TRIGGER: Returns 1 if handle could be moved, 0 if not.
 *
 * TRIGGER_BUTTON, TRIGGER_PEDESTAL: Returns 0.
 */
int check_trigger(object *op, object *cause, object *originator)
{
    object *tmp;
    int     push = 0, tot = 0;
    int     in_movement = op->stats.wc || op->speed;

    switch (op->type)
    {
        case TRIGGER_BUTTON:
          if (op->weight > 0)
          {
              if (cause)
              {
                  for (tmp = GET_BOTTOM_MAP_OB(op); tmp; tmp = tmp->above)
                      if ((!QUERY_FLAG(tmp, FLAG_FLYING)&&!QUERY_FLAG(tmp, FLAG_LEVITATE)))
                          tot += tmp->weight * (tmp->nrof ? tmp->nrof : 1) + tmp->carrying;
                  if (tot >= op->weight)
                      push = 1;
                  if (op->stats.ac == push)
                      return 0;
                  op->stats.ac = push;
                  SET_ANIMATION(op, (NUM_ANIMATIONS(op) / NUM_FACINGS(op)) * op->direction + push);
                  update_object(op, UP_OBJ_FACE);
                  if (in_movement || !push)
                      return 0;
              }
              trigger_move(op, push, originator);
          }
          return 0;

        case TRIGGER_PEDESTAL:
          if (cause)
          {
              for (tmp = GET_BOTTOM_MAP_OB(op); tmp; tmp = tmp->above)
              {
                  object   *head    = tmp->head ? tmp->head : tmp;
                  if(((!QUERY_FLAG(head, FLAG_FLYING)&&!QUERY_FLAG(head, FLAG_LEVITATE)) || QUERY_FLAG(op, FLAG_FLY_ON))
                   && (head->race == op->slaying || (op->slaying == shstr_cons.player && head->type == PLAYER))
                   && tmp != op)
                  {
                      push = 1;
                      break;
                  }
              }
              if (op->stats.ac == push)
                  return 0;
              op->stats.ac = push;
              SET_ANIMATION(op, (NUM_ANIMATIONS(op) / NUM_FACINGS(op)) * op->direction + push);
              update_object(op, UP_OBJ_FACE);
              if (in_movement || !push)
                  return 0;
          }
          trigger_move(op, push, originator);
          return 0;

        case TRIGGER_ALTAR:
          if (cause)
          {
              if (in_movement)
                  return 0;
              if (operate_altar(op, &cause))
              {
                  SET_ANIMATION(op, (NUM_ANIMATIONS(op) / NUM_FACINGS(op)) * op->direction + 1);
                  update_object(op, UP_OBJ_FACE);

                  if (op->last_sp >= 0)
                  {
                      trigger_move(op, 1, originator);
                      if (op->last_sp > 0)
                          op->last_sp = -op->last_sp;
                  }
                  else
                  {
                      /* for trigger altar with last_sp, the ON/OFF
                       status (-> +/- value) is "simulated": */
                      op->weight_limit = !op->weight_limit;
                      trigger_move(op, 1, originator);
                      op->last_sp = -op->last_sp;
                      op->weight_limit = !op->weight_limit;
                  }
                  return cause == NULL;
              }
              else
              {
                  return 0;
              }
          }
          else
          {
              SET_ANIMATION(op, (NUM_ANIMATIONS(op) / NUM_FACINGS(op)) * op->direction);
              update_object(op, UP_OBJ_FACE);

              /* If trigger_altar has "last_sp > 0" set on the map,
                 it will push the connected value only once per sacrifice.
                 Otherwise (default), the connected value will be
                 pushed twice: First by sacrifice, second by reset! -AV */
              if (!op->last_sp)
                  trigger_move(op, 0, originator);
              else
              {
                  op->stats.wc = 0;
                  op->weight_limit = !op->weight_limit;
                  op->speed = 0;
                  update_ob_speed(op);
              }
          }
          return 0;

        case TRIGGER:
          if (cause)
          {
              if (in_movement)
                  return 0;
              push = 1;
          }
          SET_ANIMATION(op, (NUM_ANIMATIONS(op) / NUM_FACINGS(op)) * op->direction + push);
          update_object(op, UP_OBJ_FACE);
          trigger_move(op, push, originator);
          return 1;

        default:
          LOG(llevDebug, "Unknown trigger type: %s (%d)\n", op->name, op->type);
          return 0;
    }
}

/* Parse a comma-separated list of connections and add the button to each of them */
void add_button_links(object *button, mapstruct *map, char *connected)
{
    char *pos = connected;

    do {
        int connection = atoi(pos);
        if(connection)
            add_button_link(button, map, connection);

        /* Actually, we only allow multiple connections for the connection sensor */
        /* TODO: this check isn 100% ok since we aren't 100% sure about the objects type when this is called */
        if(button->type != TYPE_CONN_SENSOR)
            break;

        if((pos = strchr(pos, ',')))
            pos++;
    } while (pos);
}

/* Add a button to a single connection */
void add_button_link(object *button, mapstruct *map, int connected)
{
    oblinkpt   *obp;
    objectlink *ol  = get_objectlink(OBJLNK_FLAG_OB);

    if (!map)
    {
        LOG(llevBug, "BUG: Tried to add button-link without map.\n");
        return;
    }
    button->path_attuned = (uint16) connected;  /* peterm:  I need this so I can rebuild
                                       a connected map from a template map. */
    /*  LOG(llevDebug,"adding button %s (%d)\n", button->name, connected);*/

    SET_FLAG(button, FLAG_IS_LINKED);

    ol->objlink.ob = button;
    ol->id = button->count;

    for (obp = map->buttons; obp && obp->value != connected; obp = obp->next)
        ;

    if (obp)
    {
        ol->next = obp->objlink.link;
        obp->objlink.link = ol;
    }
    else
    {
        obp = get_objectlinkpt();
        obp->value = connected;

        obp->next = map->buttons;
        map->buttons = obp;
        obp->objlink.link = ol;
    }
}

/*
 * Remove the object from the linked lists of buttons in the map.
 * This is only needed by editors.
 */

void remove_button_link(object *op)
{
    oblinkpt   *obp;
    objectlink **olp, *ol;
    int foundone = 0;

    if (op->map == NULL)
    {
        LOG(llevBug, "BUG: remove_button_link() in object without map.\n");
        return;
    }
    if (!QUERY_FLAG(op, FLAG_IS_LINKED))
    {
        LOG(llevBug, "BUG: remove_button_linked() in unlinked object.\n");
        return;
    }
    for (obp = op->map->buttons; obp; obp = obp->next)
        for (olp = &obp->objlink.link; (ol = *olp); olp = &ol->next)
            if (ol->objlink.ob == op)
            {
                /*        LOG(llevDebug, "Removed link %d in button %s and map %s.\n",
                           obp->weight_limit, op->name, op->map->path);
                */
                *olp = ol->next;
                free_objectlink_simple(ol);
                foundone = 1;
            }
    if(! foundone) {
        LOG(llevError, "remove_button_linked(): couldn't find object.\n");
        CLEAR_FLAG(op, FLAG_IS_LINKED);
    }
}

/*
 * Return the first objectlink in the objects linked to this one
 * In the case of CONN_SENSORS, make sure it is the output link
 */

objectlink * get_button_links(object *button)
{
    oblinkpt   *obp;
    objectlink *ol;

    if (!button->map)
        return NULL;
    for (obp = button->map->buttons; obp; obp = obp->next)
    {
        if(button->type == TYPE_CONN_SENSOR && button->last_grace != obp->value)
            continue;
        for (ol = obp->objlink.link; ol; ol = ol->next)
            if (ol->objlink.ob == button && ol->id == button->count)
                return obp->objlink.link;
    }
    return NULL;
}

/*
 * Made as a separate function to increase efficiency
 */

/* Get the "connection id" for a button */
int get_button_value(object *button)
{
    oblinkpt   *obp;
    objectlink *ol;

    if (!button->map)
        return 0;
    for (obp = button->map->buttons; obp; obp = obp->next)
        for (ol = obp->objlink.link; ol; ol = ol->next)
            if (ol->objlink.ob == button && ol->id == button->count)
                return obp->value;
    return 0;
}

/* This routine makes monsters who are
 * standing on the 'mood floor' change their
 * disposition if it is different.
 * If floor is to be triggered must have
 * a speed of zero (default is 1 for all
 * but the charm floor type).
 * by b.t. thomas@nomad.astro.psu.edu
 */

/* TODO: this needs to be updated for the new AI system if it is going to
 * be used at all. Gecko 2005-04-30 */
void do_mood_floor(object *op, object *op2)
{
    LOG(llevBug, "BUG: mood floor used (not implemented yet)\n");
#if 0
    object *tmp;
    object *tmp2;

    for (tmp = op->above; tmp; tmp = tmp->above)
        if (QUERY_FLAG(tmp, FLAG_MONSTER))
            break;

    /* doesn't effect players, and if there is a player on this space, won't also
     * be a monster here.
     */
    if (!tmp || tmp->type == PLAYER)
        return;

    switch (op->last_sp)
    {
        case 0:
          /* furious--make all monsters mad */
          if (QUERY_FLAG(tmp, FLAG_UNAGGRESSIVE))
              CLEAR_FLAG(tmp, FLAG_UNAGGRESSIVE);
          if (QUERY_FLAG(tmp, FLAG_FRIENDLY))
          {
              CLEAR_FLAG(tmp, FLAG_FRIENDLY);
              /* lots of checks here, but want to make sure we don't
               * dereference a null value
               */
              if (tmp->type == GOLEM && tmp->owner && tmp->owner->type == PLAYER && CONTR(tmp->owner)->golem == tmp)
              {
                  send_golem_control(tmp, GOLEM_CTR_RELEASE);
                  CONTR(tmp->owner)->golem = NULL;
              }
              tmp->owner = NULL;
          }
          break;
        case 1:
          /* angry -- get neutral monsters mad */
          if (QUERY_FLAG(tmp, FLAG_UNAGGRESSIVE) && !QUERY_FLAG(tmp, FLAG_FRIENDLY))
              CLEAR_FLAG(tmp, FLAG_UNAGGRESSIVE);
          break;
        case 2:
          /* calm -- pacify unfriendly monsters */
          if (!QUERY_FLAG(tmp, FLAG_UNAGGRESSIVE))
              SET_FLAG(tmp, FLAG_UNAGGRESSIVE);
          break;
        case 3:
          /* make all monsters fall asleep */
          if (!QUERY_FLAG(tmp, FLAG_SLEEP))
              SET_FLAG(tmp, FLAG_SLEEP);
          break;
        case 4:
          /* charm all monsters */

          if (op == op2)
              break;         /* only if 'connected' */

          for (tmp2 = get_map_ob(op2->map, op2->x, op2->y); /* finding an owner */
                                  tmp2->type != PLAYER; tmp2 = tmp2->above)
              if (tmp2->above == NULL)
                  break;

          if (tmp2->type != PLAYER)
              break;
          set_owner(tmp, tmp2);
          SET_FLAG(tmp, FLAG_MONSTER);
          tmp->stats.exp = 0;
          SET_FLAG(tmp, FLAG_FRIENDLY);
          tmp->move_type = PETMOVE;
          break;

        default:
          break;
    }
#endif
}

/* this function returns the object it matches, or NULL if non.
 * It will descend through containers to find the object.
 *
 * trig object attributes used:
 *      slaying = match object slaying (if sp != 0) or name field
 *      race = match object archetype name field
 *      if hp != 0, hp = match object type
 *      if FLAG_CONFUSED, require all non-empty fields to match
 */

object * check_inv_recursive(object *op, object *trig)
{
    object *tmp, *ret = NULL;

    for (tmp = op->inv; tmp; tmp = tmp->below)
    {
        int match_type = 0;
        int match_slaying = 0;
        int match_arch = 0;

        if (tmp->inv)
            if((ret = check_inv_recursive(tmp, trig)))
                return ret;

        /* compare type */
        if (trig->stats.hp && tmp->type == trig->stats.hp)
            match_type = 1;

        /* compare slaying with name or slaying */
        if (trig->slaying &&
            trig->slaying == (trig->stats.sp ? tmp->slaying : tmp->name))
                match_slaying = 1;

        /* compare arch name */
        if (trig->race && tmp->arch->name == trig->race)
                match_arch = 1;

        /* AND or OR logic? */
	if (QUERY_FLAG(trig, FLAG_SEE_INVISIBLE))
        {
            if((trig->stats.hp == 0 || match_type) &&
                    (trig->slaying == NULL || match_slaying) &&
                    (trig->race == NULL || match_arch))
                return tmp;
        }
        else if(match_arch || match_slaying || match_type)
            return tmp;
    }
    return NULL;
}

/* check_inv(), a function to search the inventory,
 * of a player and then based on a set of conditions,
 * the square will activate connected items.
 * Monsters can't trigger this square (for now)
 * Values are:  last_sp = 1/0 obj/no obj triggers
 *      last_heal = 1/0  remove/dont remove obj if triggered
 * -b.t. (thomas@nomad.astro.psu.edu
 */

void check_inv(object *op, object *trig)
{
    object *match;

    if (op->type != PLAYER)
        return;
    match = check_inv_recursive(op, trig);
    if (match && trig->last_sp)
    {
        if (trig->last_heal)
            decrease_ob(match);
        use_trigger(trig, op);
    }
    else if (!match && !trig->last_sp)
        use_trigger(trig, op);
}


/* This does a minimal check of the button link consistency for object
 * map.  All it really does it much sure the object id link that is set
 * matches what the object has.
 */
void verify_button_links(mapstruct *map)
{
    oblinkpt   *obp;
    objectlink *ol;

    if (!map)
        return;

    for (obp = map->buttons; obp; obp = obp->next)
    {
        for (ol = obp->objlink.link; ol; ol = ol->next)
        {
            if (ol->id != ol->objlink.ob->count)
                LOG(llevError, "verify_button_links: object %s on list is corrupt (%d!=%d)\n", ol->objlink.ob->name,
                    ol->id, ol->objlink.ob->count);
        }
    }
}
