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

static objectlink_t *get_first_button_link(object_t *button, uint8 mode);

/* Send signal from op to connection ol */
void signal_connection(object_t *op, object_t *activator, object_t *originator, map_t *m)
{
    objectlink_t *olp,
               *ol;
    object_t     *tmp;
    int         raceval = 0,
                sound_id,
                connection;

    /* tmp->weight_limit == state of trigger */

    if ((connection = get_button_value(op)) == -1)
        return; /* BUG logged in get_button_value() */

    if (connection)
    {
        if (m && m != op->map)
        {
            objectlink_t *oblp;

            for (oblp = m->buttons, olp = NULL; oblp; oblp = oblp->next)
            {
                if (oblp->value != connection)
                    continue;

                olp = oblp->objlink.link;

                if (olp->objlink.ob->type == TYPE_CONN_SENSOR &&
                    olp->objlink.ob->last_grace != oblp->value)
                {
                    olp = NULL;

                    continue;
                }

                break;
            }
        }
        else
            olp = get_first_button_link(op, 0);

        if (!olp)
        {
            LOG(llevMapbug, "MAPBUG:: %s/signal_connection(): No connected object found: %s[%d], connection %d on map '%s'!\n",
                 __FILE__, STRING_OBJ_NAME(op), TAG(op), connection,
                (m && m != op->map) ? m->path : op->map->path);

           return;
        }
    }
    else
        olp = NULL;

    if (m->in_memory != MAP_MEMORY_LOADING &&
        trigger_object_plugin_event(EVENT_TRIGGER, op, activator, originator, NULL, NULL, NULL, NULL, SCRIPT_FIX_NOTHING))
    {
        return;
    }

    /*LOG(llevDebug, "signal_connection: %s (%d)\n", op->name, op->count);*/
    for (ol = olp; ol; ol = ol->next)
    {
        tmp = ol->objlink.ob;
        if (!tmp || tmp->count != ol->id)
        {
            LOG(llevBug, "BUG: Internal error in signal_connection (%s).\n", STRING_OBJ_NAME(op));
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

        if (m->in_memory != MAP_MEMORY_LOADING &&
            tmp != op &&
            trigger_object_plugin_event(EVENT_TRIGGER, tmp, activator, originator, NULL, NULL, NULL, NULL, SCRIPT_FIX_NOTHING))
        {
            continue;
        }

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
#ifndef USE_OLD_UPDATE
              OBJECT_UPDATE_UPD(tmp, UPD_ANIM);
#else
              update_object(tmp, UP_OBJ_FACE);
#endif
              break;

            case SIGN:
              /* Ignore map loading triggers */
              if(m->in_memory == MAP_MEMORY_LOADING)
                  break;

              /* Ignore button up, otherwise two of everything for everybody else, */
              /* unless it's a timer or else it only triggers every other time. */
              if ((op->weight_limit == 0) && (op->type != TYPE_TIMER))
                  break;

              if (!tmp->stats.food || tmp->last_eat < tmp->stats.food)
              {
                  /* Don't send null buffer if sound only */
                  if (tmp->msg)
                      ndi_map(NDI_UNIQUE | NDI_NAVY, MSP_KNOWN(tmp), MAP_INFO_NORMAL, NULL, NULL, "%s",
                          tmp->msg);
                  if (tmp->stats.food)
                      tmp->last_eat++;
              }
              if (tmp->race)
              {
                  sscanf(tmp->race, "%d", &raceval);
                  if (raceval && tmp->slaying)
                  {
                      sound_id = lookup_sound(raceval-1, tmp->slaying);
                      if (sound_id >= 0)
                          play_sound_map(MSP_KNOWN(tmp), sound_id, raceval-1);
                  }
              }

              break;

            case ALTAR:
              tmp->weight_limit = 1;
              SET_ANIMATION(tmp, ((NUM_ANIMATIONS(tmp) / NUM_FACINGS(tmp)) * tmp->direction) + tmp->weight_limit);
#ifndef USE_OLD_UPDATE
              OBJECT_UPDATE_UPD(tmp, UPD_ANIM);
#else
              update_object(tmp, UP_OBJ_FACE);
#endif
              break;

            case BUTTON:
            case PEDESTAL:
              tmp->weight_limit = op->weight_limit;
              SET_ANIMATION(tmp, ((NUM_ANIMATIONS(tmp) / NUM_FACINGS(tmp)) * tmp->direction) + tmp->weight_limit);
#ifndef USE_OLD_UPDATE
              OBJECT_UPDATE_UPD(tmp, UPD_ANIM);
#else
              update_object(tmp, UP_OBJ_FACE);
#endif
              break;

            case TIMED_GATE:
              /* Ignore map loading triggers */
              if(m->in_memory == MAP_MEMORY_LOADING)
                  break;
              tmp->speed = 0.5;
              update_ob_speed(tmp);  /* original values */
              tmp->weight_limit = !tmp->stats.maxsp;
              tmp->stats.sp = 1;
              tmp->stats.hp = tmp->stats.maxhp;
              break;

            case FIREWALL:
              /* Ignore map loading triggers */
              if(m->in_memory == MAP_MEMORY_LOADING)
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

                      SET_ANIMATION(tmp, ((NUM_ANIMATIONS(tmp) / NUM_FACINGS(tmp)) * tmp->direction) + tmp->state);
#ifndef USE_OLD_UPDATE
                      OBJECT_UPDATE_UPD(tmp, UPD_ANIM);
#else
                      update_object(tmp, UP_OBJ_FACE);
#endif
                  }
              }
              break;

            case DIRECTOR:
              /* Ignore map loading triggers */
              if(m->in_memory == MAP_MEMORY_LOADING)
                  break;
              if (tmp->stats.maxsp) /* next direction */
              {
                  if ((tmp->direction += tmp->stats.maxsp) > 8)
                      tmp->direction = (tmp->direction % 8) + 1;

                      SET_ANIMATION(tmp, ((NUM_ANIMATIONS(tmp) / NUM_FACINGS(tmp)) * tmp->direction) + tmp->state);
#ifndef USE_OLD_UPDATE
                      OBJECT_UPDATE_UPD(tmp, UPD_ANIM);
#else
                      update_object(tmp, UP_OBJ_FACE);
#endif
              }
              break;

            case TELEPORTER:
              /* Ignore map loading triggers
               * Rationale: connected teleports should only trigger
               * at connection state changes. */
              if(m->in_memory == MAP_MEMORY_LOADING)
                  break;
              move_teleporter(tmp);
              break;

            case CREATOR:
              /* Ignore map loading triggers */
              if(m->in_memory == MAP_MEMORY_LOADING)
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
            case LIGHT_SOURCE:
              if (!op->weight_limit)
              {
                  if (tmp->glow_radius)
                  {
                      turn_off_light(tmp);
                  }
              }
              else
              {
                  if (!tmp->glow_radius)
                  {
                      turn_on_light(tmp);
                  }
              }
              break;

            case SPAWN_POINT:
              /* Ignore map loading triggers */
              if(m->in_memory == MAP_MEMORY_LOADING)
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
int check_button_down(object_t *what)
{
    uint32    fly,
              move;
    msp_t *msp;
    object_t   *this,
             *next;

    if (!what ||
        !what->map)
    {
        return 0;
    }

    fly = QUERY_FLAG(what, FLAG_FLY_ON);
    move = QUERY_FLAG(what, FLAG_WALK_ON);
    msp = MSP_KNOWN(what);

    if (what->type == BUTTON)
    {
        sint32 total_weight = 0;

        FOREACH_OBJECT_IN_MSP(this, msp, next)
        {
            object_t *head = (this->head) ? this->head : this;

            if (this != what &&
                ((IS_AIRBORNE(head)) ? fly : move))
            {
                total_weight += this->weight * ((this->nrof) ? this->nrof : 1) + this->carrying;
            }
        }

        what->weight_limit = (total_weight >= what->weight) ? 1 : 0;

        return what->weight_limit;
    }
    else if (what->type == PEDESTAL)
    {
        what->weight_limit = 0;

        FOREACH_OBJECT_IN_MSP(this, msp, next)
        {
            object_t *head = (this->head) ? this->head : this;

            if (this != what &&
                ((IS_AIRBORNE(head)) ? fly : move) &&
                (head->race == what->slaying ||
                 (what->slaying == shstr_cons.player &&
                  head->type == PLAYER)))
            {
                what->weight_limit = 1;

                return what->weight_limit;
            }
        }
    }

    return 0;
}

/*
 * Updates everything connected with the button op.
 * After changing the state of a button, this function must be called
 * to make sure that all gates and other buttons connected to the
 * button reacts to the (eventual) change of state.
 */
void update_button(object_t *op, object_t *activator, object_t *originator)
{
    object_t     *tmp;
    int         any_down = 0;
    sint32      old_value = op->weight_limit;
    objectlink_t *ol;
    int         has_links = 0;

    /* LOG(llevDebug, "update_button: %s (%d)\n", op->name, op->count); */
    for (ol = get_first_button_link(op, 1); ol; ol = ol->next)
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
#ifndef USE_OLD_UPDATE
        OBJECT_UPDATE_UPD(op, UPD_ANIM);
#else
        update_object(op, UP_OBJ_FACE);
#endif
        signal_connection(op, activator, originator, op->map); /* Make all other buttons the same */
    }
}

/*
 * Updates every button on the map (by calling update_button() for them).
 * This is only done when a map loads.
 */

void update_buttons(map_t *m)
{
    objectlink_t *obp;

    for (obp = m->buttons; obp; obp = obp->next)
    {
        objectlink_t *ol;

        for (ol = obp->objlink.link; ol; ol = ol->next)
        {
            object_t *link = ol->objlink.ob;
            uint8   type;

            if (!link ||
                link->count != ol->id)
            {
                LOG(llevBug, "BUG:: %s:update_buttons(): Internal error (%s[%d] (%dx%d):%d, connected %d).\n",
                    __FILE__, STRING_OBJ_NAME(link), TAG(link),
                    (link) ? link->x : -1, (link) ? link->y : -1, ol->id,
                    obp->value);

                continue;
            }

            type = link->type;

            if (type == BUTTON ||
                type == PEDESTAL)
            {
                update_button(link, NULL, NULL);
            }
            else if (type == CHECK_INV)
            {
                uint32    fly = QUERY_FLAG(link, FLAG_FLY_ON),
                          move = QUERY_FLAG(link, FLAG_WALK_ON);
                msp_t *msp = MSP_KNOWN(link);
                object_t   *this,
                         *next;

                FOREACH_OBJECT_IN_MSP(this, msp, next)
                {
                    if (this != link &&
                        (IS_AIRBORNE(this) ? fly : move))
                    {
                        check_inv(this, link);
                    }
                }
            }
            else if (type == TRIGGER_BUTTON ||
                     type == TRIGGER_PEDESTAL ||
                     type == TRIGGER_ALTAR)
            {
                /* check_trigger will itself sort out the numbers of
                 * items above the trigger */
                check_trigger(link, link, NULL);
            }
            else if (type == TYPE_CONN_SENSOR)
            {
                move_conn_sensor(link);
            }
            else if (type == TYPE_ENV_SENSOR)
            {
                move_environment_sensor(link);
            }
            else if (type == CF_HANDLE ||
                     type == TRIGGER)
            {
                signal_connection(link, NULL, NULL, link->map);
            }
        }
    }
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

int check_altar_sacrifice(object_t *altar, object_t *sacrifice)
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

int operate_altar(object_t *altar, object_t **sacrifice)
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
    {
        ndi_map(NDI_WHITE, MSP_KNOWN(altar), MAP_INFO_NORMAL, NULL, NULL, "%s",
            altar->msg);
    }

    return 1;
}

void trigger_move(object_t *op, int state, object_t *originator) /* 1 down and 0 up */
{
    op->stats.wc = state;
    op->weight_limit = !op->weight_limit;
    signal_connection(op, originator, originator, op->map);

    if (state)
    {
        float reset = (op->stats.exp) ? (float)op->stats.exp : 30.0f;

        op->speed = 1.0f / reset;
        update_ob_speed(op);
        op->speed_left = -1.0f;
    }
    else
    {
        op->speed = 0.0f;
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
int check_trigger(object_t *op, object_t *cause, object_t *originator)
{
    int     push = 0, tot = 0;
    int     in_movement = op->stats.wc || op->speed;

    switch (op->type)
    {
        case TRIGGER_BUTTON:
          if (op->weight > 0)
          {
              if (cause)
              {
                  uint32    fly = QUERY_FLAG(op, FLAG_FLY_ON),
                            move = QUERY_FLAG(op, FLAG_WALK_ON);
                  msp_t *msp = MSP_KNOWN(op);
                  object_t   *this,
                           *next;

                  FOREACH_OBJECT_IN_MSP(this, msp, next)
                  {
                       object_t *head = (this->head) ? this->head : this;

                       if (this != op &&
                           ((IS_AIRBORNE(head)) ? fly : move))
                      {
                          tot += this->weight * (this->nrof ? this->nrof : 1) + this->carrying;
                      }
                  }

                  if (tot >= op->weight)
                  {
                      push = 1;
                  }

                  if (op->stats.ac == push)
                  {
                      return 0;
                  }

                  op->stats.ac = push;
                  SET_ANIMATION(op, (NUM_ANIMATIONS(op) / NUM_FACINGS(op)) * op->direction + push);
#ifndef USE_OLD_UPDATE
                  OBJECT_UPDATE_UPD(op, UPD_ANIM);
#else
                  update_object(op, UP_OBJ_FACE);
#endif

                  if (in_movement ||
                      !push)
                  {
                      return 0;
                  }
              }

              trigger_move(op, push, originator);
          }

          return 0;

        case TRIGGER_PEDESTAL:
          if (cause)
          {
              uint32    fly = QUERY_FLAG(op, FLAG_FLY_ON),
                        move = QUERY_FLAG(op, FLAG_WALK_ON);
              msp_t *msp = MSP_KNOWN(op);
              object_t   *this,
                       *next;

              FOREACH_OBJECT_IN_MSP(this, msp, next)
              {
                  object_t *head = (this->head) ? this->head : this;

                 if (this != op &&
                     ((IS_AIRBORNE(head)) ? fly : move) &&
                     (head->race == op->slaying ||
                      (op->slaying == shstr_cons.player &&
                       head->type == PLAYER)))
                  {
                      push = 1;

                      break;
                  }
              }

              if (op->stats.ac == push)
              {
                  return 0;
              }

              op->stats.ac = push;
              SET_ANIMATION(op, (NUM_ANIMATIONS(op) / NUM_FACINGS(op)) * op->direction + push);
#ifndef USE_OLD_UPDATE
              OBJECT_UPDATE_UPD(op, UPD_ANIM);
#else
              update_object(op, UP_OBJ_FACE);
#endif

              if (in_movement ||
                  !push)
              {
                  return 0;
              }
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
#ifndef USE_OLD_UPDATE
                  OBJECT_UPDATE_UPD(op, UPD_ANIM);
#else
                  update_object(op, UP_OBJ_FACE);
#endif

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
#ifndef USE_OLD_UPDATE
              OBJECT_UPDATE_UPD(op, UPD_ANIM);
#else
              update_object(op, UP_OBJ_FACE);
#endif

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
#ifndef USE_OLD_UPDATE
          OBJECT_UPDATE_UPD(op, UPD_ANIM);
#else
          update_object(op, UP_OBJ_FACE);
#endif
          trigger_move(op, push, originator);
          return 1;

        default:
          LOG(llevDebug, "Unknown trigger type: %s (%d)\n", op->name, op->type);
          return 0;
    }
}

/* Parse a comma-separated list of connections and add the button to each of them */
void add_button_links(object_t *button, map_t *map, char *connected)
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
void add_button_link(object_t *button, map_t *map, int connected)
{
    objectlink_t   *obp;
    objectlink_t *ol  = objectlink_get(OBJLNK_FLAG_OB);

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
        obp = objectlink_get(OBJLNK_FLAG_LINK);
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

void remove_button_link(object_t *op)
{
    objectlink_t   *obp;
    objectlink_t **olp, *ol;
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
                return_poolchunk(ol, pool_objectlink);
                foundone = 1;
            }
    if(! foundone) {
        LOG(llevError, "remove_button_linked(): couldn't find object.\n");
        CLEAR_FLAG(op, FLAG_IS_LINKED);
    }
}

/* Return the first objectlink in the objects linked to this one. mode is a bit
 * of a hack. If 1 then in the case of CONN_SENSORS, make sure the return is
 * the output link. */
static objectlink_t *get_first_button_link(object_t *button, uint8 mode)
{
    objectlink_t   *obp;
    objectlink_t *ol;

    if (!button->map)
    {
        return NULL;
    }

    for (obp = button->map->buttons; obp; obp = obp->next)
    {
        if(mode &&
           button->type == TYPE_CONN_SENSOR &&
           button->last_grace != obp->value)
        {
            continue;
        }

        for (ol = obp->objlink.link; ol; ol = ol->next)
        {
            if (ol->objlink.ob == button &&
                ol->id == button->count)
            {
                return obp->objlink.link;
            }
        }
    }

    return NULL;
}

/*
 * Made as a separate function to increase efficiency
 */

/* Get the "connection id" for a button */
int get_button_value(object_t *button)
{
    objectlink_t   *obp;
    objectlink_t *ol;

    if (!button)
    {
        LOG(llevBug, "BUG:: %s/get_button_value(): button == NULL!\n",
            __FILE__);

        return -1;
    }

    if (!button->map)
    {
        LOG(llevMapbug, "MAPBUG:: %s/get_button_value(): Button not on map: %s[%d]!\n",
            __FILE__, STRING_OBJ_NAME(button), TAG(button));

        return -1;
    }

    for (obp = button->map->buttons; obp; obp = obp->next)
        for (ol = obp->objlink.link; ol; ol = ol->next)
            if (ol->objlink.ob == button && ol->id == button->count)
                return obp->value;

    return 0;
}

/* this function returns the object it matches, or NULL if non.
 * It will descend through containers to find the object.
 *
 * trig object attributes used:
 *      slaying = match object slaying (if sp != 0) or name field
 *      race = match object archetype name field
 *      if hp != 0, hp = match object type
 *      if FLAG_SEE_INVISIBLE, require all non-empty fields to match
 */

object_t * check_inv_recursive(object_t *op, object_t *trig)
{
    object_t *tmp,
           *next,
           *ret = NULL;

    FOREACH_OBJECT_IN_OBJECT(tmp, op, next)
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

void check_inv(object_t *op, object_t *trig)
{
    object_t *match;

    if (op->type != PLAYER)
        return;
    match = check_inv_recursive(op, trig);
    if (match && trig->last_sp)
    {
        if (trig->last_heal)
            decrease_ob_nr(match, 1);

        trig->weight_limit = !trig->weight_limit;
        signal_connection(trig, op, op, trig->map);
    }
    else if (!match && !trig->last_sp)
    {
        trig->weight_limit = !trig->weight_limit;
        signal_connection(trig, op, op, trig->map);
    }
}


/* This does a minimal check of the button link consistency for object
 * map.  All it really does it much sure the object id link that is set
 * matches what the object has.
 */
void verify_button_links(map_t *map)
{
    objectlink_t   *obp;
    objectlink_t *ol;

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
