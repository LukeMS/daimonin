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

/*
 * Routines that is executed from objects based on their speed have been
 * collected in this file.
 */

#include <global.h>
#ifndef __CEXTRACT__
#include <sproto.h>
#endif

/* The following removes doors.  The functions check to see if similar 
 * doors are next to the one that is being removed, and if so, set it
 * so those will be removed shortly (in a cascade like fashion.)
 */

/* this is OLD door - atm i have NOT any of them in the arches of daimonin.
 * but this doors can be tricky used like cascading openings and so on.
 * So i let them in.
 */
/* the "cascading trick" works very simple - if we are here, we search for similiar
 * attached doors. if we find them, we give them speed. That means they will in some
 * ticks processed by active object process function. This funcion will come back here
 * and we go on and on - this will give the effect thats around a whole map a doorwall
 * will open one after one.
 */
/* i think i will add a "LOCKED_DOOR_CHAINED" type or something. for normal doors,
 * i don't want this cascading - but its a nice tricky map manipulation trick which
 * opens some fine fun or map makers.
 */
void remove_door(object *op) {
  int i;
  object *tmp;
  for(i=1;i<9;i+=2)
    if((tmp=present(DOOR,op->map,op->x+freearr_x[i],op->y+freearr_y[i]))!=NULL) {
      tmp->speed = 0.1f;
      update_ob_speed(tmp);
      tmp->speed_left= -0.2f;
    }

  if(op->other_arch)
  {
      tmp=arch_to_object(op->other_arch);
      tmp->x=op->x;tmp->y=op->y;tmp->map=op->map;tmp->level=op->level;
	  tmp->direction = op->direction;
	  if(QUERY_FLAG(tmp, FLAG_IS_TURNABLE) || QUERY_FLAG(tmp, FLAG_ANIMATE))
		SET_ANIMATION(tmp, (NUM_ANIMATIONS(tmp)/NUM_FACINGS(tmp))*tmp->direction+tmp->state);
      insert_ob_in_map(tmp,op->map,op,0);
  }
  if(op->sub_type1 == ST1_DOOR_NORMAL)
	  play_sound_map(op->map, op->x, op->y, SOUND_OPEN_DOOR, SOUND_NORMAL);
  remove_ob(op);
  free_object(op);
}

void remove_door2(object *op) {
  object *tmp;

  /* - NO cascading for locked doors! 
  int i;
  for(i=1;i<9;i+=2) {
    tmp=present(LOCKED_DOOR,op->map,op->x+freearr_x[i],op->y+freearr_y[i]);
    if(tmp && tmp->slaying == op->slaying)
	{
      tmp->speed = 0.1f;
      update_ob_speed(tmp);
      tmp->speed_left= -0.2f;
    }
  }*/

  /* mow 2 ways to handle open doors.
   * a.) if other_arch is set, we insert that object and remove the old door.
   * b.) if not set, we toggle from close to open when needed.
   */

	if(op->other_arch) /* if set, just exchange and delete old door */
	{
		tmp=arch_to_object(op->other_arch);
		tmp->state=0;	/* 0= closed, 1= opened */
		tmp->x=op->x;tmp->y=op->y;tmp->map=op->map;tmp->level=op->level;
		tmp->direction = op->direction;
		if(QUERY_FLAG(tmp, FLAG_IS_TURNABLE) || QUERY_FLAG(tmp, FLAG_ANIMATE))
			SET_ANIMATION(tmp, (NUM_ANIMATIONS(tmp)/NUM_FACINGS(tmp))*tmp->direction+tmp->state);
		insert_ob_in_map(tmp,op->map,op,0);
		if(op->sub_type1 == ST1_DOOR_NORMAL)
		  play_sound_map(op->map, op->x, op->y, SOUND_OPEN_DOOR, SOUND_NORMAL);
		remove_ob(op);
		free_object(op);
  }
  else if(!op->last_eat) /* if set, we are have opened a closed door - now handle autoclose */
  {
	  remove_ob(op); /* to trigger all the updates/changes on map and for player, we
					  * remove and reinsert it. a bit overhead but its secure and simple
					  */
	  op->last_eat = 1; /* mark this door as "its open" */
      op->speed = 0.1f;		/* put it on active list, so it will close automatically */
      update_ob_speed(op);
      op->speed_left= -0.2f;
	  op->state = 1; /* change to "open door" faces */
	  op->last_sp = op->stats.sp; /* init "open" counter */
	  /* save and clear blocksview and no_pass */
	  QUERY_FLAG(op,FLAG_BLOCKSVIEW) ?(op->stats.grace=1):(op->stats.grace=0);
	  QUERY_FLAG(op,FLAG_NO_PASS) ?(op->last_grace=1):(op->last_grace=0);
	  CLEAR_FLAG(op,FLAG_BLOCKSVIEW);
	  CLEAR_FLAG(op,FLAG_NO_PASS);
	  if(QUERY_FLAG(op, FLAG_IS_TURNABLE) || QUERY_FLAG(op, FLAG_ANIMATE))
		SET_ANIMATION(op, (NUM_ANIMATIONS(op)/NUM_FACINGS(op))*op->direction+op->state);
	  if(op->sub_type1 == ST1_DOOR_NORMAL)
		  play_sound_map(op->map, op->x, op->y, SOUND_OPEN_DOOR, SOUND_NORMAL);
      insert_ob_in_map(op,op->map,op,0);
  }
}

/* thats called from time.c - here we handle autoclosing doors */
void remove_door3(object *op) 
{
  if(!op->last_eat) /* thats a bug - active speed but not marked as active */
  {
	  LOG(llevBug,"BUG: door has speed but is not marked as active. (%s - map:%s (%d,%d))\n",query_name(op),
		  op->map?op->map->name:"(no map name!)", op->x, op->y);
	  op->last_eat = 0; /* thats not a real fix ... */
	  return;
  }
  if(!op->map) /* ouch */
  {
	  LOG(llevBug,"BUG: door with speed but no map?! killing object...done. (%s - (%d,%d))\n",
									query_name(op), op->x, op->y);
	  remove_ob(op);
      free_object(op);
	  return;
  }

  /* now check or counter - if not <=0 we still be open */
  if(op->last_sp-- > 0)
	  return;
  /* now we try to close the door. The rule is:
   * is the tile of the door not blocked by a no_pass object OR a player OR a mob - 
   * then close the door.
   * IF it is blocked - then restart a new "is open" phase.
   */

  /* her we can use or new & shiny blocked() - we simply check the given flags */
  if(blocked(NULL, op->map, op->x, op->y, TERRAIN_ALL) & (P_NO_PASS | P_IS_ALIVE|P_IS_PLAYER)) 
  {
	  /* let it open one more round */
	  op->last_sp = op->stats.sp; /* reinit "open" counter */

  }
  else /* ok - NOW we close it */
  {
	  remove_ob(op); /* to trigger all the updates/changes on map and for player, we
					  * remove and reinsert it. a bit overhead but its secure and simple
					  */
	  op->last_eat = 0; /* mark this door as "its closed" */
      op->speed = 0.0f;	/* remove from active list */
      op->speed_left= 0.0f;
      update_ob_speed(op);
	  op->state = 0; /* change to "close door" faces */
	  op->stats.grace=1?SET_FLAG(op,FLAG_BLOCKSVIEW):CLEAR_FLAG(op,FLAG_BLOCKSVIEW);
	  op->last_grace=1?SET_FLAG(op,FLAG_NO_PASS):CLEAR_FLAG(op,FLAG_NO_PASS);
	  op->stats.grace =0;op->last_grace=0;
	  if(QUERY_FLAG(op, FLAG_IS_TURNABLE) || QUERY_FLAG(op, FLAG_ANIMATE))
		SET_ANIMATION(op, (NUM_ANIMATIONS(op)/NUM_FACINGS(op))*op->direction+op->state);
	  if(op->sub_type1 == ST1_DOOR_NORMAL)
		  play_sound_map(op->map, op->x, op->y, SOUND_DOOR_CLOSE, SOUND_NORMAL);
      insert_ob_in_map(op,op->map,op,0);
	  if(QUERY_FLAG(op,FLAG_BLOCKSVIEW))
		  update_all_los(op->map, op->x, op->y);
  }
}

void generate_monster(object *gen) {
  int i;
  object *op,*head=NULL,*prev=NULL;
  archetype *at=gen->other_arch;

  if(GENERATE_SPEED(gen)&&rndm(0, GENERATE_SPEED(gen)-1))
    return;
  if(gen->other_arch==NULL) {
    LOG(llevBug,"BUG: Generator without other_arch: %s\n",query_name(gen));
    return;
  }
  i=find_free_spot(at,gen->map,gen->x,gen->y,1,9);
  if (i==-1) return;
  while(at!=NULL) {
    op=arch_to_object(at);
    op->x=gen->x+freearr_x[i]+at->clone.x;
    op->y=gen->y+freearr_y[i]+at->clone.y;
    if(head!=NULL)
      op->head=head,prev->more=op;
    if (rndm(0, 9)) generate_artifact(op, gen->map->difficulty,0,99);
    insert_ob_in_map(op,gen->map,gen,0);
    if (QUERY_FLAG(op, FLAG_FREED)) return;
    if(op->randomitems!=NULL)
      create_treasure(op->randomitems,op,GT_APPLY,
                      gen->map->difficulty,0);
    if(head==NULL)
      head=op;
    prev=op;
    at=at->more;
  }
}

void regenerate_rod(object *rod) {
  if(++rod->stats.food > rod->stats.hp/10 || rod->type == HORN) {
    rod->stats.food = 0;
    if (rod->stats.hp < rod->stats.maxhp) {
      rod->stats.hp += 1 + rod->stats.maxhp/10;
      if (rod->stats.hp > rod->stats.maxhp)
        rod->stats.hp = rod->stats.maxhp;
      fix_rod_speed(rod);
    }
  }
}

void remove_force(object *op) {
  if(op->env==NULL) {
    remove_ob(op);
    free_object(op);
    return;
  }
  CLEAR_FLAG(op, FLAG_APPLIED);
  change_abil(op->env,op);
  fix_player(op->env);
  remove_ob(op);
  free_object(op);
}

void remove_blindness(object *op) {
  if(--op->stats.food > 0)
    return;
  CLEAR_FLAG(op, FLAG_APPLIED);
  if(op->env!=NULL) { 
     change_abil(op->env,op);
     fix_player(op->env);
  }
  remove_ob(op);
  free_object(op);
}

void remove_confusion(object *op) {
  if(--op->stats.food > 0)
    return;
  if(op->env!=NULL) {
    CLEAR_FLAG(op->env, FLAG_CONFUSED);
    new_draw_info(NDI_UNIQUE, 0,op->env, "You regain your senses.\n");
  }
  remove_ob(op);
  free_object(op);
}

void execute_wor(object *op) {
  object *wor=op;
  while(op!=NULL&&op->type!=PLAYER)
    op=op->env;
  if(op!=NULL) {
    if(blocks_magic(op->map,op->x,op->y))
      new_draw_info(NDI_UNIQUE, 0,op,"You feel something fizzle inside you.");
    else
      enter_exit(op,wor);
  }
  remove_ob(wor);
  free_object(wor);
}

void poison_more(object *op) {
  if(op->env==NULL||!QUERY_FLAG(op->env,FLAG_ALIVE)||op->env->stats.hp<0) {
    remove_ob(op);
    free_object(op);
    return;
  }
  if(!op->stats.food) {
    /* need to remove the object before fix_player is called, else fix_player
     * will not do anything.
     */
    if(op->env->type==PLAYER) {
      CLEAR_FLAG(op, FLAG_APPLIED);
      fix_player(op->env);
      new_draw_info(NDI_UNIQUE, 0,op->env,"You feel much better now.");
    }
    remove_ob(op);
    free_object(op);
    return;
  }
  if(op->env->type==PLAYER) {
    op->env->stats.food--;
    new_draw_info(NDI_UNIQUE, 0,op->env,"You feel very sick...");
  }
  (void)hit_player(op->env, op->stats.dam, op,AT_INTERNAL);
}

/* TODO: i have not included damage to mobs/player on reverse up going gates!
 * Look in the code!
 * also, i included sounds for open & close gates! we need to add a tracker the
 * get is going up or down. 
 * ARGH: see the update_all_los() in this function.. is called not ONE time we
 * start open or close the door - its called EVERY animation ! this must be changed.
 */
void move_gate(object *op) { /* 1 = going down, 0 = goind up */
    object *tmp;

    if(op->stats.wc < 0 || (int)op->stats.wc  >= (NUM_ANIMATIONS(op)/NUM_FACINGS(op))) 
	{
		dump_object(op);
		LOG(llevBug,"BUG: Gate error: animation was %d, max=%d\n:%s\n",op->stats.wc,(NUM_ANIMATIONS(op)/NUM_FACINGS(op)),errmsg);
		op->stats.wc=0;
    }

    /* We're going down (or reverse up) */
    if(op->value) 
	{
		if(--op->stats.wc<=0)  /* Reached bottom, let's stop */
		{
		    op->stats.wc=0;
		    if(op->arch->clone.speed)
				op->value=0;
			else 
			{
				op->speed = 0;
				update_ob_speed(op);
			}
		}

		if((int)op->stats.wc < ((NUM_ANIMATIONS(op)/NUM_FACINGS(op))/2+1)) 
		{
			if(op->last_heal) /* if != 0, we have a reversed timed gate, which starts open */
			{
				SET_FLAG(op, FLAG_NO_PASS);    /* The coast is clear, block the way */
				if(!op->arch->clone.stats.ac)
					SET_FLAG(op, FLAG_BLOCKSVIEW);
			}
			else 
			{
				CLEAR_FLAG(op, FLAG_NO_PASS);
				CLEAR_FLAG(op, FLAG_BLOCKSVIEW);
			}
			update_all_los(op->map, op->x, op->y);
		}

		op->state = (uint8) op->stats.wc;
		SET_ANIMATION(op, (NUM_ANIMATIONS(op)/NUM_FACINGS(op))*op->direction+op->state);
		update_object(op,UP_OBJ_CHANGE);
		return;
    }

    /* First, lets see if we are already at the top */
    if((unsigned char) op->stats.wc==((NUM_ANIMATIONS(op)/NUM_FACINGS(op))-1))
	{
		/* Check to make sure that only non pickable and non rollable
		 * objects are above the gate.  If so, we finish closing the gate,
		 * otherwise, we fall through to the code below which should lower
		 * the gate slightly.
		 */

		for (tmp=op->above; tmp!=NULL; tmp=tmp->above) 
		{
			if (!QUERY_FLAG(tmp, FLAG_NO_PICK) || QUERY_FLAG(tmp, FLAG_CAN_ROLL) || QUERY_FLAG(tmp, FLAG_ALIVE))
				break;
		}

		if (tmp==NULL)
		{
			if(op->arch->clone.speed)
				op->value=1;
			else 
			{
				op->speed = 0;
				update_ob_speed(op); /* Reached top, let's stop */
			}
			return;
		}
    }

    if(op->stats.food) {    /* The gate is going temporarily down */
	if(--op->stats.wc<=0) { /* Gone all the way down? */
	    op->stats.food=0;	    /* Then let's try again */
	    op->stats.wc=0;
	}
    } else {                /* The gate is still going up */
	op->stats.wc++;

	if((int)op->stats.wc >= ((NUM_ANIMATIONS(op)/NUM_FACINGS(op))))
	    op->stats.wc=(signed char)(NUM_ANIMATIONS(op)/NUM_FACINGS(op))-1;

	/* If there is something on top of the gate, we try to roll it off.
	 * If a player/monster, we don't roll, we just hit them with damage
	 */
	if((int)op->stats.wc >= (NUM_ANIMATIONS(op)/NUM_FACINGS(op))/2) {
	    /* Halfway or further, check blocks */
	    /* First, get the top object on the square. */
	    for(tmp=op->above;tmp!=NULL && tmp->above!=NULL;tmp=tmp->above);

	    if(tmp!=NULL) {
		if(QUERY_FLAG(tmp, FLAG_ALIVE)) {
		    hit_player(tmp, random_roll(1, op->stats.dam, tmp, PREFER_LOW), op, AT_PHYSICAL);
		    if(tmp->type==PLAYER) 
			new_draw_info_format(NDI_UNIQUE, 0, tmp,
					     "You are crushed by the %s!",op->name);
		} else 
		    /* If the object is not alive, and the object either can
		     * be picked up or the object rolls, move the object
		     * off the gate.
		     */
		    if(!QUERY_FLAG(tmp, FLAG_ALIVE)
			&& (!QUERY_FLAG(tmp, FLAG_NO_PICK)
			   ||QUERY_FLAG(tmp,FLAG_CAN_ROLL))) {
		    /* If it has speed, it should move itself, otherwise: */
		    int i=find_free_spot(tmp->arch,op->map,op->x,op->y,1,9);

		    /* If there is a free spot, move the object someplace */
		    if (i!=-1) {
			remove_ob(tmp);
			tmp->x+=freearr_x[i],tmp->y+=freearr_y[i];
			insert_ob_in_map(tmp,op->map,op,0);
		    }
		}
	    }

	    /* See if there is still anything blocking the gate */
	    for (tmp=op->above; tmp!=NULL; tmp=tmp->above) 
		if (!QUERY_FLAG(tmp, FLAG_NO_PICK)
			|| QUERY_FLAG(tmp, FLAG_CAN_ROLL)
			|| QUERY_FLAG(tmp, FLAG_ALIVE))
		    break;

	    /* IF there is, start putting the gate down  */
	    if(tmp) {
		    op->stats.food=1;
	    } else {
		if(op->last_heal) /* if != 0, we have a reversed timed gate, which starts open */
		{
		    CLEAR_FLAG(op, FLAG_NO_PASS);
		    CLEAR_FLAG(op, FLAG_BLOCKSVIEW);
		}
		else
		{
			SET_FLAG(op, FLAG_NO_PASS);    /* The coast is clear, block the way */
			if(!op->arch->clone.stats.ac)
			    SET_FLAG(op, FLAG_BLOCKSVIEW);
		}
		update_all_los(op->map, op->x, op->y);
	    }
	} /* gate is halfway up */

	op->state = (uint8) op->stats.wc;
	SET_ANIMATION(op, (NUM_ANIMATIONS(op)/NUM_FACINGS(op))*op->direction + op->state);
	update_object(op,UP_OBJ_CHANGE);
    } /* gate is going up */
}

/*  hp      : how long door is open/closed
 *  maxhp   : initial value for hp
 *  sp      : 1 = open, 0 = close
 */
void move_timed_gate(object *op)
{
  int v = op->value;

  if (op->stats.sp) {
    move_gate(op);
    if (op->value != v)   /* change direction ? */
      op->stats.sp = 0;
    return;
  } 
  if (--op->stats.hp <= 0) { /* keep gate down */
    move_gate(op);
    if (op->value != v) {  /* ready ? */
	op->speed = 0;
	update_ob_speed(op);
    }
  }
}

/*  slaying:    name of the thing the detector is to look for
 *	 speed:      frequency of 'glances'
 *	 connected:  connected value of detector
 *  sp:         1 if detection sets buttons
 *              -1 if detection unsets buttons
 */

void move_detector(object *op) 
{
    object *tmp;
    int last = op->value;
    int detected;
    detected = 0;

    for(tmp = get_map_ob(op->map,op->x,op->y);tmp!=NULL&&!detected;tmp=tmp->above) {
		object *tmp2;
		if(op->stats.hp) {
		  for(tmp2= tmp->inv;tmp2;tmp2=tmp2->below) {
			 if(op->slaying && !strcmp(op->slaying,tmp->name)) detected=1;
			 if(tmp2->type==FORCE &&tmp2->slaying && !strcmp(tmp2->slaying,op->slaying)) detected=1;
		  }
		}
	if (op->slaying && !strcmp(op->slaying,tmp->name)) {
	  detected = 1;
	}
	else if (tmp->type==SPECIAL_KEY && tmp->slaying==op->slaying)
	    detected=1;
    }

    /* the detector sets the button if detection is found */
    if(op->stats.sp == 1)  {
	if(detected && last == 0) {
	    op->value = 1;
	    push_button(op);
	}
	if(!detected && last == 1) {
	    op->value = 0;
	    push_button(op);
	}
    }
    else { /* in this case, we unset buttons */
	if(detected && last == 1) {
	    op->value = 0;
	    push_button(op);
	}
	if(!detected && last == 0) {
	    op->value = 1;
	    push_button(op);
	}
    }
}


void animate_trigger (object *op)
{
  if((unsigned char)++op->stats.wc >= NUM_ANIMATIONS(op)/NUM_FACINGS(op)) {
    op->stats.wc = 0;
    check_trigger(op,NULL);
  } else {
	op->state = (uint8) op->stats.wc;
	SET_ANIMATION(op, (NUM_ANIMATIONS(op)/NUM_FACINGS(op))*op->direction + op->state);
    update_object(op,UP_OBJ_FACE);
  }
}

/* close or open pit. op->value is set when something connected to the pit
 * is triggered.
 */
void move_pit(object *op) {
    object *next,*tmp;

    if(op->value) { /* We're opening */
		if(--op->stats.wc<=0) { /* Opened, let's stop */
			op->stats.wc=0;
			op->speed = 0;
			update_ob_speed(op);
			SET_FLAG(op, FLAG_WALK_ON);
			for (tmp=op->above; tmp!=NULL; tmp=next)
			{
				next=tmp->above;
				move_apply(op,tmp,tmp);
			}
		}
		op->state = (uint8) op->stats.wc;
		SET_ANIMATION(op, (NUM_ANIMATIONS(op)/NUM_FACINGS(op))*op->direction + op->state);
		update_object(op,UP_OBJ_FACE);
		return;
    }
    /* We're closing */
    CLEAR_FLAG(op, FLAG_WALK_ON);
    op->stats.wc++;
    if((int)op->stats.wc >= NUM_ANIMATIONS(op)/NUM_FACINGS(op))
	op->stats.wc=NUM_ANIMATIONS(op)/NUM_FACINGS(op)-1;
	op->state = (uint8) op->stats.wc;
    SET_ANIMATION(op, (NUM_ANIMATIONS(op)/NUM_FACINGS(op))*op->direction + op->state);
    update_object(op,UP_OBJ_FACE);
    if((unsigned char) op->stats.wc==(NUM_ANIMATIONS(op)/NUM_FACINGS(op)-1)) {
	op->speed = 0;
	update_ob_speed(op); /* closed, let's stop */
	return;
    }
}


/* stop_item() returns a pointer to the stopped object.  The stopped object
 * may or may not have been removed from maps or inventories.  It will not
 * have been merged with other items.
 *
 * This function assumes that only items on maps need special treatment.
 *
 * If the object can't be stopped, or it was destroyed while trying to stop
 * it, NULL is returned.
 *
 * fix_stopped_item() should be used if the stopped item should be put on
 * the map.
 */
object *stop_item (object *op)
{
    if (op->map == NULL)
        return op;

    switch (op->type)
    {
    case THROWN_OBJ:
        {
            object *payload = op->inv;
            if (payload == NULL)
                return NULL;
            remove_ob (payload);
            remove_ob (op);
            free_object (op);
            return payload;
        }

    case ARROW:
        if (op->speed >= MIN_ACTIVE_SPEED)
            op = fix_stopped_arrow (op);
        return op;

    case CONE:
        if (op->speed < MIN_ACTIVE_SPEED) {
            return op;
        } else {
            return NULL;
	}

    default:
        return op;
    }
}

/* fix_stopped_item() - put stopped item where stop_item() had found it.
 * Inserts item into the old map, or merges it if it already is on the map.
 *
 * 'map' must be the value of op->map before stop_item() was called.
 */
void fix_stopped_item (object *op, mapstruct *map, object *originator)
{
    if (map == NULL)
        return;
    if (QUERY_FLAG (op, FLAG_REMOVED))
        insert_ob_in_map (op, map, originator,0);
    else if (op->type == ARROW)
        merge_ob (op, NULL);   /* only some arrows actually need this */
}


object *fix_stopped_arrow (object *op)
{
	if(op->type != ARROW)
		return op;
	/* Small chance of breaking */
	/*
    if(rndm(0, 99) < op->stats.food) {
        remove_ob (op);
	free_object(op);
	return NULL;
    }*/

    op->direction=0;
    CLEAR_FLAG(op, FLAG_WALK_ON);
    CLEAR_FLAG(op, FLAG_FLY_ON);
    CLEAR_FLAG(op, FLAG_FLYING);

	/* food is a self destruct marker - that long the item will need to be destruct! */
	if(op->stats.food && op->type == ARROW)
	{
		SET_FLAG(op,FLAG_IS_USED_UP);
		SET_FLAG(op,FLAG_NO_PICK);
		op->type = MISC_OBJECT; /* important to neutralize the arrow! */
		op->speed = 0.1f;
		op->speed_left = 0.0f;
	}
	else
		op->speed = 0;
    update_ob_speed(op);
    op->stats.wc = (sint8) op->last_heal;
    op->stats.dam= op->stats.hp;
    /* Reset these to zero, so that CAN_MERGE will work properly */
    op->last_heal = 0;
    op->stats.hp = 0;
    op->face=op->arch->clone.face;
    op->owner=NULL; /* So that stopped arrows will be saved */
    update_object (op,UP_OBJ_FACE);
    return op;
}

/* stop_arrow() - what to do when a non-living flying object
 * has to stop. Sept 96 - I added in thrown object code in
 * here too. -b.t.
 *
 * Returns a pointer to the stopped object (which will have been removed
 * from maps or inventories), or NULL if was destroyed.
 */

void stop_arrow (object *op)
{
#ifdef PLUGINS
    /* GROS: Handle for plugin stop event */
    if(op->event_hook[EVENT_STOP] != NULL)
    {
        CFParm CFP;
        int k, l, m;
        k = EVENT_STOP;
        l = SCRIPT_FIX_NOTHING;
        m = 0;
        CFP.Value[0] = &k;
        CFP.Value[1] = NULL;
        CFP.Value[2] = op;
        CFP.Value[3] = NULL;
        CFP.Value[4] = NULL;
        CFP.Value[5] = &m;
        CFP.Value[6] = &m;
        CFP.Value[7] = &m;
        CFP.Value[8] = &l;
        CFP.Value[9] = op->event_hook[k];
        CFP.Value[10]= op->event_options[k];
        if (findPlugin(op->event_plugin[k])>=0)
            ((PlugList[findPlugin(op->event_plugin[k])].eventfunc) (&CFP));
    }
#endif
	play_sound_map(op->map, op->x, op->y, SOUND_DROP_THROW, SOUND_NORMAL);
    if (op->inv) {
		object *payload = op->inv;
		remove_ob (payload);
		/* we have a thrown potion here.
		 * This potion has NOT hit a target.
		 * it has hitten a wall or just dropped to the ground.
		 * 1.) its a AE spell... detonate it.
		 * 2.) its something else - shatter the potion.
		 */
		if(payload->type == POTION)
		{
			if (payload->stats.sp!= SP_NO_SPELL && spells[payload->stats.sp].flags&SPELL_DESC_DIRECTION)
				cast_spell(payload,payload, payload->direction, payload->stats.sp, 1, spellPotion,NULL); /* apply potion ALWAYS fire on the spot the applier stands - good for healing - bad for firestorm */
			free_object (payload);
		}
		else
		{
			clear_owner(payload);
	        insert_ob_in_map (payload, op->map, payload,0);
		}
        remove_ob (op);
		free_object (op);
    } else {
        op = fix_stopped_arrow (op);
        if (op)
            merge_ob (op, NULL);
    }
}

/* Move an arrow along its course.  op is the arrow or thrown object.
 */

void move_arrow(object *op) {
    object *tmp=NULL;
    int new_x, new_y;
    int was_reflected;
    mapstruct *m=op->map;

    if(op->map==NULL) {
	LOG(llevBug, "BUG: Arrow %s had no map.\n", query_name(op));
	remove_ob(op);
	free_object(op);
	return;
    }

    /* we need to stop thrown objects and arrows at some point. Like here. */ 
    if(op->type==THROWN_OBJ)
	{
        if (op->inv == NULL)
            return;
    }

	if(op->last_sp-- < 0) 
	{ 
	    stop_arrow (op);
	    return; 
	}

    /* Calculate target map square */
    new_x = op->x + DIRX(op);
    new_y = op->y + DIRY(op);
    was_reflected = 0;

    /* See if there is any living object on target map square */
    if((m= out_of_map(op->map,&new_x,&new_y)))        
        tmp = get_map_ob (m, new_x, new_y);

    while (tmp && ! QUERY_FLAG (tmp, FLAG_ALIVE))
	tmp = tmp->above;

    /* A bad problem was that a monster throw or fire something and then
     * it runs in it. Not only this is a sync. problem, the monster will also
     * hit themself and used as his own enemy! Result is, that many monsters
     * start to hit themself dead.
     * I removed both: No monster can be hit from his own missile and it can't 
     * be his own enemy. - MT, 25.11.01 */
    
    if (tmp && tmp != op->owner)
    {        
        /* Found living object, but it is reflecting the missile.  Update
         * as below. (Note that for living creatures there is a small
         * chance that reflect_missile fails.)
         */
        if (QUERY_FLAG (tmp, FLAG_REFL_MISSILE) && (!QUERY_FLAG(tmp,
	    FLAG_ALIVE) || (rndm(0, 99)) < 90-op->level/10))
        {
            int number = op->face->number;
	    
            op->direction = absdir (op->direction + 4);
            op->state = 0;
            if (GET_ANIM_ID (op)) {
                number += 4;
                if (number > GET_ANIMATION (op, 8))
                    number -= 8;
                op->face = &new_faces[number];
            }
            if (wall (m, new_x, new_y)) {
                /* Target is standing on a wall.  Let arrow turn around before
                 * the wall. */
                new_x = op->x;
                new_y = op->y;
            }
            was_reflected = 1;   /* skip normal movement calculations */
        }
        else
        {
            /* Attack the object. */
            op = hit_with_arrow (op, tmp);
            if (op == NULL)
                return;
        }
    }

    if ( ! was_reflected && wall (m, new_x, new_y))
    {
	/* if the object doesn't reflect, stop the arrow from moving */
	if(!QUERY_FLAG(op, FLAG_REFLECTING) || !(rndm(0, 19))) {
	    stop_arrow (op);
	    return;
	} else {    /* object is reflected */
	    /* If one of the major directions (n,s,e,w), just reverse it */
	    if(op->direction&1) {
		op->direction=absdir(op->direction+4);
	    } else {
		/* The below is just logic for figuring out what direction
		 * the object should now take.
		 */
	
		int left= wall(op->map,op->x+freearr_x[absdir(op->direction-1)],
		       op->y+freearr_y[absdir(op->direction-1)]),
		right=wall(op->map,op->x+freearr_x[absdir(op->direction+1)],
		   op->y+freearr_y[absdir(op->direction+1)]);

		if(left==right)
		    op->direction=absdir(op->direction+4);
		else if(left)

			op->direction=absdir(op->direction+2);
		else if(right)
		    op->direction=absdir(op->direction-2);
	    }
	    /* Is the new direction also a wall?  If show, shuffle again */
	    if(wall(op->map,op->x+DIRX(op),op->y+DIRY(op))) {
		int left= wall(op->map,op->x+freearr_x[absdir(op->direction-1)],
			 op->y+freearr_y[absdir(op->direction-1)]),
		right=wall(op->map,op->x+freearr_x[absdir(op->direction+1)],
		     op->y+freearr_y[absdir(op->direction+1)]);

		if(!left)
		    op->direction=absdir(op->direction-1);
		else if(!right)
		    op->direction=absdir(op->direction+1);
		else {		/* is this possible? */
		    stop_arrow (op);
		    return;
		}
	    }
	    /* update object image for new facing */
	    /* many thrown objects *don't* have more than one face */
	    if(GET_ANIM_ID(op))
		SET_ANIMATION(op, (NUM_ANIMATIONS(op)/NUM_FACINGS(op))*op->direction);
	} /* object is reflected */
    } /* object ran into a wall */

    /* Move the arrow. */
    remove_ob (op);
    op->x = new_x;
    op->y = new_y;
    insert_ob_in_map (op, m, op,0);
}

/* This routine doesnt seem to work for "inanimate" objects that
 * are being carried, ie a held torch leaps from your hands!. 
 * Modified this routine to allow held objects. b.t. */

void change_object(object *op) { /* Doesn`t handle linked objs yet */
  object *tmp,*env,*pl;
  int i,j;

  /* In non-living items only change when food value is 0 */
  if(!QUERY_FLAG(op,FLAG_ALIVE)) {
	if(op->stats.food-- > 0) return; 
	else
    {
        if(op->type == TYPE_LIGHT_APPLY)
        {
            CLEAR_FLAG(op,FLAG_CHANGING);
            op->glow_radius*=-1;
            
            if ((tmp=is_player_inv(op))!=NULL)
            {
                new_draw_info_format(NDI_UNIQUE, 0, tmp,
                    "The %s burnt out.", query_name(op));
                fix_player(tmp);
            }
            else            
                tmp = op;

			update_object(tmp,UP_OBJ_CHANGE);
            update_all_map_los(tmp->map);
            if(op->other_arch == NULL)
            {
                op->stats.food=0; /* other_arch == NULL means it can be refilled */
                CLEAR_FLAG(op,FLAG_ANIMATE);
                op->face = op->arch->clone.face;                
                update_object(op,UP_OBJ_FACE);
                if((pl=is_player_inv(op))!=NULL)
                    esrv_send_item(pl, op);
                return;
            }
        }
        op->stats.food=1; /* so 1 other_arch is made */
    }
  }

  if(op->other_arch==NULL) {
      LOG(llevBug,"BUG: Change object (%s) without other_arch error.\n", op->name);
      return;
  }
  env=op->env;
  remove_ob(op);
  for(i=0;i<NROFNEWOBJS(op);i++) {
    tmp=arch_to_object(op->other_arch);
    tmp->stats.hp=op->stats.hp; /* The only variable it keeps. */
    if(env) {
        tmp->x=env->x,tmp->y=env->y;
	tmp=insert_ob_in_ob(tmp,env);
	/* If this object is the players inventory, we need to tell the
	 * client of the change.  Insert_ob_in_map takes care of the
	 * updating the client, so we don't need to do that below.
	 */
	if ((pl=is_player_inv(env))!=NULL) {
	    esrv_del_item(pl->contr, op->count);
	    esrv_send_item(pl, tmp);
	}
    } else {
        j=find_first_free_spot(tmp->arch,op->map,op->x,op->y);
	if (j==-1)  /* No free spot */
	    free_object(tmp);
	else {
	    tmp->x=op->x+freearr_x[j],tmp->y=op->y+freearr_y[j];
	    insert_ob_in_map(tmp,op->map,op,0);
	}
    }
  }
  free_object(op);
}


/* i do some heavy changes to telporters.
 * First, with tiled maps it is a big problem, that teleporters
 * can only move player over maps. Second, i added a "no_teleport"
 * flag to the engine.
 * The teleporter will now teleport ANY object on the tile node - also
 * multi arch objects which are with one part on the teleporter.
 * WARNING: Also system objects will be teleported when they don't
 * have a "no_teleport" flag.
 * Because we can teleport multi arch monster now with a single tile
 * teleporter, i removed multi arch teleporters.
 */

void move_teleporter(object *op) {
    object *tmp, *next;

	/* get first object of this map node */
    for(tmp=get_map_ob(op->map,op->x,op->y);tmp!=NULL;tmp=next)
	{
		next = tmp->above;
		if (QUERY_FLAG(tmp, FLAG_NO_TELEPORT)) 
			continue;
    
		/* teleport to different map */
		if(EXIT_PATH(op)) 
		{ 
#ifdef PLUGINS
			/* GROS: Handle for plugin TRIGGER event */
			if(op->event_hook[EVENT_TRIGGER] != NULL)
			{
				CFParm CFP;
				CFParm* CFR;
				int k, l, m;
				int rtn_script = 0;
				m = 0;
				k = EVENT_TRIGGER;
				l = SCRIPT_FIX_NOTHING;
				CFP.Value[0] = &k;
				CFP.Value[1] = tmp; /* activator first */
				CFP.Value[2] = op; /* thats whoisme */
				CFP.Value[3] = NULL;
				CFP.Value[4] = NULL;
				CFP.Value[5] = &m;
				CFP.Value[6] = &m;
				CFP.Value[7] = &m;
				CFP.Value[8] = &l;
				CFP.Value[9] = op->event_hook[k];
				CFP.Value[10]= op->event_options[k];
				if (findPlugin(op->event_plugin[k])>=0)
				{
					CFR = (PlugList[findPlugin(op->event_plugin[k])].eventfunc) (&CFP);
					rtn_script = *(int *)(CFR->Value[0]);
				}
				if (rtn_script!=0) return;
			}
#endif
			enter_exit(tmp, op);
		}
		else if(EXIT_X(op)!=-1&&EXIT_Y(op)!=-1) /* teleport inside this map */
		{
			/* use OUT_OF_REAL_MAP() - we want be truly on THIS map */
			if (!OUT_OF_REAL_MAP(op->map,EXIT_X(op),EXIT_Y(op)))
			{
				LOG(llevBug, "BUG: Removed illegal teleporter (map: %s (%d,%d)) -> (%d,%d)\n",
											op->map->name, op->x, op->y, EXIT_X(op), EXIT_Y(op));
				remove_ob(op);
				free_object(op);
				return;
			}
#ifdef PLUGINS
			/* GROS: Handle for plugin TRIGGER event */
			if(op->event_hook[EVENT_TRIGGER] != NULL)
			{
				CFParm CFP;
				CFParm* CFR;
				int k, l,m;
				int rtn_script = 0;
				m = 0;
				k = EVENT_TRIGGER;
				l = SCRIPT_FIX_NOTHING;
				CFP.Value[0] = &k;
				CFP.Value[1] = tmp;
				CFP.Value[2] = op;
				CFP.Value[3] = NULL;
				CFP.Value[4] = NULL;
				CFP.Value[5] = &m;
				CFP.Value[6] = &m;
				CFP.Value[7] = &m;
				CFP.Value[8] = &l;
				CFP.Value[9] = op->event_hook[k];
				CFP.Value[10]= op->event_options[k];
				if (findPlugin(op->event_plugin[k])>=0)
				{
					CFR = (PlugList[findPlugin(op->event_plugin[k])].eventfunc) (&CFP);
					rtn_script = *(int *)(CFR->Value[0]);
				}
				if (rtn_script!=0) return;
			}
#endif
			transfer_ob(tmp,EXIT_X(op),EXIT_Y(op),0,op, NULL);
		 }
		else
		{
			/* Random teleporter */
#ifdef PLUGINS
			/* GROS: Handle for plugin TRIGGER event */
			if(op->event_hook[EVENT_TRIGGER] != NULL)
			{
				CFParm CFP;
				CFParm* CFR;
				int k, l, m;
				int rtn_script = 0;
				m = 0;
				k = EVENT_TRIGGER;
				l = SCRIPT_FIX_NOTHING;
				CFP.Value[0] = &k;
				CFP.Value[1] = op;
				CFP.Value[2] = tmp;
				CFP.Value[3] = NULL;
				CFP.Value[4] = NULL;
				CFP.Value[5] = &m;
				CFP.Value[6] = &m;
				CFP.Value[7] = &m;
				CFP.Value[8] = &l;
				CFP.Value[9] = op->event_hook[k];
				CFP.Value[10]= op->event_options[k];
				if (findPlugin(op->event_plugin[k])>=0)
				{
					CFR = (PlugList[findPlugin(op->event_plugin[k])].eventfunc) (&CFP);
					rtn_script = *(int *)(CFR->Value[0]);
				}
				if (rtn_script!=0) return;
			}
#endif
			teleport(op, TELEPORTER, tmp);
		}
	}
}


/*  This object will teleport someone to a different map
    and will also apply changes to the player from its inventory.
    This was invented for giving classes, but there's no reason it
    can't be generalized.
*/

void move_player_changer(object *op) {
  object *player;
  object *walk;
  char c;
   if(op->above!=NULL) {
    if(EXIT_PATH(op)) {
      if(op->above->type==PLAYER) {
#ifdef PLUGINS
      /* GROS: Handle for plugin TRIGGER event */
      if(op->event_hook[EVENT_TRIGGER] != NULL)
      {
        CFParm CFP;
        CFParm* CFR;
        int k, l, m;
        int rtn_script = 0;
        m = 0;
        k = EVENT_TRIGGER;
        l = SCRIPT_FIX_NOTHING;
        CFP.Value[0] = &k;
        CFP.Value[1] = op->above;
        CFP.Value[2] = op;
        CFP.Value[3] = NULL;
        CFP.Value[4] = NULL;
        CFP.Value[5] = &m;
        CFP.Value[6] = &m;
        CFP.Value[7] = &m;
        CFP.Value[8] = &l;
        CFP.Value[9] = op->event_hook[k];
        CFP.Value[10]= op->event_options[k];
        if (findPlugin(op->event_plugin[k])>=0)
        {
          CFR = (PlugList[findPlugin(op->event_plugin[k])].eventfunc) (&CFP);
          rtn_script = *(int *)(CFR->Value[0]);
        }
        if (rtn_script!=0) return;
      }
#endif
	player=op->above;
	for(walk=op->inv;walk!=NULL;walk=walk->below) 
	  apply_changes_to_player(player,walk);
	link_player_skills(op->above);
	esrv_send_inventory(op->above,op->above);
	esrv_update_item(UPD_FACE, op->above, op->above);
	
	/* update players death & WoR home-position */
	sscanf(EXIT_PATH(op), "%c", &c);
	if (c == '/') {
	  strcpy(player->contr->savebed_map, EXIT_PATH(op));
	  player->contr->bed_x = EXIT_X(op), player->contr->bed_y = EXIT_Y(op);
	}
	else
	  LOG(llevDebug, "WARNING: destination '%s' in player_changer must be an absolute path!",
	      EXIT_PATH(op));
	
	enter_exit(op->above,op);
	save_player(player, 1);
      }
      else
        return;
    }
   }
}

/*  peterm:  firewalls generalized to be able to shoot any type
    of spell at all.  the stats.dam field of a firewall object
    contains it's spelltype.      The direction of the wall is stored
    in op->direction. walls can have hp, so they can be torn down. */
/* added some new features to FIREWALL - on/off features by connected,
 * advanced spell selection and full turnable by connected and
 * autoturn. MT-2003
 */
void move_firewall(object *op) {
  if ( ! op->map || !op->last_eat || op->stats.dam == -1) /* last_eat 0 = off */
    return;   /* dm has created a firewall in his inventory or no legal spell selected */
  cast_spell(op,op,op->direction,op->stats.dam, 1,spellNPC,NULL);
}

void move_firechest(object *op) {
  if ( ! op->map)
    return;   /* dm has created a firechest in his inventory */
  fire_a_ball(op,rndm(1, 8),7);
}


/*  move_player_mover:  this function takes a "player mover" as an
 * argument, and performs the function of a player mover, which is:

 * a player mover finds any players that are sitting on it.  It
 * moves them in the op->stats.sp direction.  speed is how often it'll move.
 * If attacktype is nonzero it will paralyze the player.  If lifesave is set,
 * it'll dissapear after hp+1 moves.  If hp is set and attacktype is set,
 * it'll paralyze the victim for hp*his speed/op->speed

*/
void move_player_mover(object *op) {
    object *victim, *nextmover;
	mapstruct *mt;
    int xt,yt,dir = op->direction;

	if( !(blocked(NULL, op->map,op->x,op->y,TERRAIN_NOTHING)&(P_IS_ALIVE|P_IS_PLAYER)))
			return;
    /* Determine direction now for random movers so we do the right thing */
    if (!dir) dir=rndm(1, 8);
    for(victim=get_map_ob(op->map,op->x,op->y); victim !=NULL; victim=victim->above) {
	if((QUERY_FLAG(victim, FLAG_ALIVE)||QUERY_FLAG(victim, FLAG_IS_PLAYER))&& 
							(!(QUERY_FLAG(victim,FLAG_FLYING))||op->stats.maxhp)) {

	    if(QUERY_FLAG(op,FLAG_LIFESAVE)&&op->stats.hp--<0) {
		remove_ob(op);
		free_object(op);
		return;
	    }

		xt = op->x+freearr_x[dir];
		yt = op->y+freearr_y[dir];
		if (!(mt=out_of_map (op->map, &xt, &yt)))
			return;

	    for(nextmover=get_map_ob(mt,xt,yt); nextmover !=NULL; nextmover=nextmover->above) {
		if(nextmover->type == PLAYERMOVER) 
		    nextmover->speed_left=-0.99f;
		if(QUERY_FLAG(nextmover,FLAG_ALIVE)) {
		    op->speed_left=-1.1f;  /* wait until the next thing gets out of the way */
		}
	    }

	    if(victim->type==PLAYER) { 
		/*  only level >=1 movers move people */
		if(op->level) {
		    /* Following is a bit of hack.  We need to make sure it
		     * is cleared, otherwise the player will get stuck in
		     * place.  This can happen if the player used a spell to
		     * get to this space.
		     */
		    victim->contr->fire_on=0;
		    victim->speed_left=-FABS(victim->speed);
		    move_player(victim, dir);
		}
		else return;
	    }
	    else move_object(victim,dir);

	    if(!op->stats.maxsp&&op->attacktype) op->stats.maxsp=2;

	    if(op->attacktype)  /* flag to paralyze the player */
		victim->speed_left= -FABS(op->stats.maxsp*victim->speed/op->speed);
	}
    }
}


/*  move_creator (by peterm) 
  it has the creator object create it's other_arch right on top of it.
  connected:  what will trigger it
  hp:  how many times it may create before stopping
  lifesave:  if set, it'll never disappear but will go on creating
	everytime it's triggered
  other_arch:  the object to create
*/
/* not multi arch fixed, i think MT */
void move_creator(object *op) {
  object *tmp;
  if(!op->other_arch)
	  return;
  op->stats.hp--;
  if(op->stats.hp < 0 && !QUERY_FLAG(op,FLAG_LIFESAVE)) 
	{ op->stats.hp=-1;return;}
  tmp=arch_to_object(op->other_arch);
  if(op->slaying) {
	 if (tmp->name) free_string (tmp->name);
	 if (tmp->title) free_string (tmp->title);
	 tmp->name = add_string(op->slaying);
	 tmp->title = add_string(op->slaying);
  }
  tmp->x=op->x;tmp->y=op->y;tmp->map=op->map;tmp->level=op->level;
  insert_ob_in_map(tmp,op->map,op,0);
}

/* move_marker --peterm@soda.csua.berkeley.edu
   when moved, a marker will search for a player sitting above
   it, and insert an invisible, weightless force into him
   with a specific code as the slaying field.
   At that time, it writes the contents of its own message
   field to the player.  The marker will decrement hp to
   0 and then delete itself every time it grants a mark.
   unless hp was zero to start with, in which case it is infinite.*/

void move_marker(object *op) {
  object *tmp,*tmp2;
  
  for(tmp=get_map_ob(op->map,op->x,op->y);tmp!=NULL;tmp=tmp->above) {
	 

    if(tmp->type == PLAYER) { /* we've got someone to MARK */

		/* remove an old force with a slaying field == op->name */
      for(tmp2=tmp->inv;tmp2 !=NULL; tmp2=tmp2->below) {
		  if(tmp2->type == FORCE && tmp2->slaying && !strcmp(tmp2->slaying,op->name)) break;
      }
		if(tmp2) {
		  remove_ob(tmp2);
		  free_object(tmp2);
		}

      /* cycle through his inventory to look for the MARK we want to place */
      for(tmp2=tmp->inv;tmp2 !=NULL; tmp2=tmp2->below) {
		  if(tmp2->type == FORCE && tmp2->slaying && !strcmp(tmp2->slaying,op->slaying)) break;
      }
      
      /* if we didn't find our own MARK */
      if(tmp2==NULL) {
	         
		  object *force = get_archetype("force");
		  force->speed = 0;
		  if(op->stats.food) {
			 force->speed = 0.01f;
			 force->speed_left = (float)-op->stats.food;
		  }
		  update_ob_speed (force);
		  /* put in the lock code */
		  force->slaying = add_string(op->slaying);
		  insert_ob_in_ob(force,tmp);
		  if(op->msg)
		    new_draw_info(NDI_UNIQUE|NDI_NAVY,0,tmp,op->msg);
		  if(op->stats.hp > 0) { 
		    op->stats.hp--;
		    if(op->stats.hp==0) {
		      /* marker expires--granted mark number limit */
		      remove_ob(op);
		      free_object(op);
		      return;
		    }
		  }
      }

    }

  }
}
 
int process_object(object *op) {


  if(QUERY_FLAG(op, FLAG_MONSTER))
    if(move_monster(op) || QUERY_FLAG(op, FLAG_FREED)) 
      return 1;

  if(QUERY_FLAG(op, FLAG_CHANGING)&&!op->state) {
    change_object(op);
    return 1;
  }
  if(QUERY_FLAG(op, FLAG_GENERATOR))
    generate_monster(op);
  if(QUERY_FLAG(op, FLAG_IS_USED_UP)&&--op->stats.food<=0) {
    if(QUERY_FLAG(op, FLAG_APPLIED))
      remove_force(op);
    else {
	/* IF necessary, delete the item from the players inventory */
	object *pl=is_player_inv(op);
	if (pl)
	    esrv_del_item(pl->contr, op->count);
      remove_ob(op);
      free_object(op);
    }
    return 1;
  }
#ifdef PLUGINS
  /* GROS: Handle for plugin time event */
  if(op->event_hook[EVENT_TIME] != NULL)
  {
    CFParm CFP;
    int k, l, m;
    k = EVENT_TIME;
    l = SCRIPT_FIX_NOTHING;
    m = 0;
    CFP.Value[0] = &k;
    CFP.Value[1] = NULL;
    CFP.Value[2] = op;
    CFP.Value[3] = NULL;
    CFP.Value[4] = NULL;
    CFP.Value[5] = &m;
    CFP.Value[6] = &m;
    CFP.Value[7] = &m;
    CFP.Value[8] = &l;
    CFP.Value[9] = op->event_hook[k];
    CFP.Value[10]= op->event_options[k];
    if (findPlugin(op->event_plugin[k])>=0)
        ((PlugList[findPlugin(op->event_plugin[k])].eventfunc) (&CFP));
  }
#endif
  switch(op->type) {
  case ROD:
  case HORN:
    regenerate_rod(op);
    return 1;
  case FORCE:
  case POTION_EFFECT:
    remove_force(op);
    return 1;
  case SPAWN_POINT:
    spawn_point(op);
    return 0;
  case BLINDNESS:
    remove_blindness(op);
    return 0;
  case CONFUSION:
    remove_confusion(op);
    return 0;
  case POISONING:
    poison_more(op);
    return 0;
  case DISEASE:
	 move_disease(op);
	 return 0;
  case SYMPTOM:
	 move_symptom(op);
	 return 0;
  case WORD_OF_RECALL:
    execute_wor(op);
    return 0;
  case BULLET:
    move_fired_arch(op);
    return 0;
  case MMISSILE:
    move_missile(op);
    return 0;
  case THROWN_OBJ:
  case ARROW:
    move_arrow(op);
    return 0;
  case FBULLET:
    move_fired_arch(op);
    return 0;
  case FBALL:
  case POISONCLOUD:
    explosion(op);
    return 0;
  case LIGHTNING: /* It now moves twice as fast */
    move_bolt(op);
    return 0;
  case CONE:
    move_cone(op);
    return 0;
  case DOOR:
    remove_door(op);
    return 0;
  case LOCKED_DOOR:
    remove_door3(op); /* handle autoclosing */
    return 0;
  case TELEPORTER:
    move_teleporter(op);
    return 0;
  case BOMB:
    animate_bomb(op);
    return 0;
  case GOLEM:
    move_golem(op);
    return 0;
  case EARTHWALL:
    hit_player(op, 2, op, AT_PHYSICAL);
    return 0;
  case FIREWALL:
    move_firewall(op);
    return 0;
  case FIRECHEST:
    move_firechest(op);
    return 0;
  case MOOD_FLOOR:
    do_mood_floor(op, op);
    return 0;
  case GATE:
    move_gate(op);
    return 0;
  case TIMED_GATE:
    move_timed_gate(op);
    return 0;
  case TRIGGER:
  case TRIGGER_BUTTON:
  case TRIGGER_PEDESTAL:
  case TRIGGER_ALTAR:
    animate_trigger(op);
    return 0;
  case DETECTOR:
	 move_detector(op);
    return 0;
  case PIT:
    move_pit(op);
    return 0;
  case DEEP_SWAMP:
    move_deep_swamp(op);
    return 0;
  case CANCELLATION:
    move_cancellation(op);
    return 0;
  case BALL_LIGHTNING:
    move_ball_lightning(op);
    return 0;
  case SWARM_SPELL:
    move_swarm_spell(op);
    return 0;
  case RUNE:
    return 0;
  case PLAYERMOVER:
    move_player_mover(op);
    return 0;
  case CREATOR:
    move_creator(op);
    return 0;
  case MARKER:
    move_marker(op);
    return 0;
  case PLAYER_CHANGER:
    move_player_changer(op);
    return 0;
  case AURA:
    move_aura(op);
    return 0;
  case PEACEMAKER:
    move_peacemaker(op);
    return 0;
  }

  return 0;
}

