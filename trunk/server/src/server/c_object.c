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
#include <loader.h>
#ifndef __CEXTRACT__
#include <sproto.h>
#endif

/*
 * Object id parsing functions
 */

#define OBLINKMALLOC(p) if(!((p)=(objectlink *)malloc(sizeof(objectlink))))\
                          fatal(OUT_OF_MEMORY);

#define ADD_ITEM(NEW,COUNT)\
	  if(!first) {\
	    OBLINKMALLOC(first);\
	    last=first;\
	  } else {\
	    OBLINKMALLOC(last->next);\
	    last=last->next;\
	  }\
	  last->next=NULL;\
	  last->ob=(NEW);\
          last->id=(COUNT);

/* Search the inventory of 'pl' for what matches best with params.
 * we use item_matched_string above - this gives us consistent behaviour
 * between many commands.  Return the best match, or NULL if no match.
 */
object *find_best_object_match(object *pl, char *params)
{
    object *tmp, *best=NULL;
    int match_val=0,tmpmatch;

    for (tmp=pl->inv; tmp; tmp=tmp->below) {
	if (IS_SYS_INVISIBLE(tmp)) continue;
	if ((tmpmatch=item_matched_string(pl, tmp, params))>match_val) {
	    match_val=tmpmatch;
	    best=tmp;
	}
    }
    return best;
}


int command_uskill ( object *pl, char *params) {
   if (!params) {
        new_draw_info(NDI_UNIQUE, 0,pl, "Usage: use_skill <skill name>");
        return 0;
   }
   if(pl->type == PLAYER)
        pl->contr->praying=0;
   return use_skill(pl,params);
}

int command_rskill ( object *pl, char *params) {
   int skillno;

   if (!params) {
        new_draw_info(NDI_UNIQUE, 0,pl, "Usage: ready_skill <skill name>");
        return 0;
   }
   if(pl->type == PLAYER)
       pl->contr->praying=0;
   skillno=lookup_skill_by_name(params);
   if (skillno==-1) {
	new_draw_info_format(NDI_UNIQUE,0,pl,"Couldn't find the skill %s", params);
	return 0;
   }
   return change_skill(pl,skillno);
}

int command_apply (object *op, char *params)
{
    if(op->type == PLAYER)
    	op->contr->praying=0;
  if (!params) {
    player_apply_below(op);
    return 0;
  }
  else {
    enum apply_flag aflag = 0;
    object *inv;

    while (*params==' ') params++;
    if (!strncmp(params,"-a ",3)) {
	aflag=AP_APPLY;
	params+=3;
    }
    if (!strncmp(params,"-u ",3)) {
	aflag=AP_UNAPPLY;
	params+=3;
    }
    while (*params==' ') params++;

    inv=find_best_object_match(op, params);
    if (inv) {
	player_apply(op,inv,aflag,0);
    } else
	  new_draw_info_format(NDI_UNIQUE, 0, op,
	    "Could not find any match to the %s.",params);
  }
  return 0;
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
int sack_can_hold (object *pl, object *sack, object *op, int nrof) {
    char buf[MAX_BUF];
    buf[0] = 0;

    if (! QUERY_FLAG (sack, FLAG_APPLIED))
	sprintf (buf, "The %s is not active.", query_name(sack));
    if (sack == op)
		sprintf (buf, "You can't put the %s into itself.", query_name(sack));
    if (sack->race && (sack->race != op->race || op->type == CONTAINER
		       || (sack->stats.food && sack->stats.food != op->type)))
	sprintf (buf, "You can put only %s into the %s.", sack->race,
		 query_name(sack));
    if (op->type == SPECIAL_KEY && sack->slaying && op->slaying)
		sprintf (buf, "You don't want put the key into %s.", query_name(sack));
	/*LOG(-1,"SACK: wl:%d carry:%d op->weight:%d Str:%d nrof:%d\n",sack->weight_limit,sack->carrying,op->weight,sack->stats.Str,nrof);*/
    if (sack->weight_limit && sack->carrying+
			((((nrof?nrof:1)*op->weight)+(op->type==CONTAINER?op->carrying:0))
			*(100 - sack->stats.Str) / 100) > (sint32)sack->weight_limit)
		sprintf (buf, "That won't fit in the %s!", query_name(sack));
    if (buf[0]) {
	if (pl)
	    new_draw_info(NDI_UNIQUE, 0,pl, buf);
	return 0;
    }
    return 1;
}

/* Pick up commands follow */
/* pl = player (not always - monsters can use this now)
 * op is the object to put tmp into, 
 * tmp is the object to pick up, nrof is the number to
 * pick up (0 means all of them)
 */
static void pick_up_object (object *pl, object *op, object *tmp, int nrof)
{
    /* buf needs to be big (more than 256 chars) because you can get
     * very long item names.
     */
    char buf[HUGE_BUF];
    object *env=tmp->env;
    uint32 weight, effective_weight_limit;
    int tmp_nrof = tmp->nrof ? tmp->nrof : 1;

    if(pl->type == PLAYER)
    	pl->contr->praying=0;

    /* IF the player is flying & trying to take the item out of a container 
     * that is in his inventory, let him.  tmp->env points to the container 
     * (sack, luggage, etc), tmp->env->env then points to the player (nested 
     * containers not allowed as of now)
     */
    if(QUERY_FLAG(pl, FLAG_FLYING) && !QUERY_FLAG(pl, FLAG_WIZ) && 
       is_player_inv(tmp)!=pl) {
	new_draw_info(NDI_UNIQUE, 0,pl, "You are levitating, you can't reach the ground!");
	return;
    }
    if (QUERY_FLAG (tmp, FLAG_NO_DROP))
	return;
    if(QUERY_FLAG(tmp,FLAG_WAS_WIZ) && !QUERY_FLAG(pl, FLAG_WAS_WIZ)) {
	new_draw_info(NDI_UNIQUE, 0,pl, "The object disappears in a puff of smoke!");
	new_draw_info(NDI_UNIQUE, 0,pl, "It must have been an illusion.");
	if (pl->type==PLAYER) esrv_del_item (pl->contr, tmp->count, tmp->env);
	if ( ! QUERY_FLAG (tmp, FLAG_REMOVED))
            remove_ob (tmp);
	free_object(tmp);
	return;
    }
    
    if (nrof > tmp_nrof || nrof == 0)
	nrof = tmp_nrof;
    /* Figure out how much weight this object will add to the player */
    weight = tmp->weight * nrof;
    if (tmp->inv) weight += tmp->carrying * (100 - tmp->stats.Str) / 100;
    if (pl->stats.Str <= MAX_STAT)
        effective_weight_limit = weight_limit[pl->stats.Str];
    else
        effective_weight_limit = weight_limit[MAX_STAT];
    if ((pl->carrying + weight) > effective_weight_limit) {
	new_draw_info(NDI_UNIQUE, 0,pl,"That item is too heavy for you to pick up.");
	return;
    }
	

	if(tmp->type == CONTAINER)
		container_unlink(NULL,tmp);

#ifndef REAL_WIZ
    if(QUERY_FLAG(pl, FLAG_WAS_WIZ))
	SET_FLAG(tmp, FLAG_WAS_WIZ);
#endif

    if (nrof != tmp_nrof) {
	object *tmp2 = tmp, *tmp2_cont = tmp->env;
        tag_t tmp2_tag = tmp2->count;
	tmp = get_split_ob (tmp, nrof);
	if(!tmp) {
	    new_draw_info(NDI_UNIQUE, 0,pl, errmsg);
	    return;
	}
	/* Tell a client what happened rest of objects */
	if (pl->type == PLAYER) {
	    if (was_destroyed (tmp2, tmp2_tag))
		esrv_del_item (pl->contr, tmp2_tag,tmp2_cont);
	    else
		esrv_send_item (pl, tmp2);
	}
    } else {
	/* If the object is in a container, send a delete to the client.
	 * - we are moving all the items from the container to elsewhere,
	 * so it needs to be deleted.
	 */
        if ( ! QUERY_FLAG (tmp, FLAG_REMOVED)) {
	    if (tmp->env && pl->type==PLAYER) 
	        esrv_del_item (pl->contr, tmp->count,tmp->env);
	    remove_ob(tmp); /* Unlink it */
	}
    }
    if(QUERY_FLAG(tmp, FLAG_UNPAID))
	{
	    if(QUERY_FLAG(tmp, FLAG_NO_PICK)) /* this is a clone shop - clone a item for inventory */
		{
			tmp = ObjectCreateClone(tmp);
			CLEAR_FLAG(tmp, FLAG_NO_PICK);
			SET_FLAG(tmp, FLAG_STARTEQUIP);
			sprintf(buf,"You pick up a %s from the storage.", query_name(tmp));
		}
		else /* this is a unique shop item */
			sprintf(buf,"%s will cost you %s.", query_name(tmp),
												query_cost_string(tmp,pl,F_BUY));
	}
    else
		sprintf(buf,"You pick up the %s.", query_name(tmp));

    new_draw_info(NDI_UNIQUE, 0,pl,buf);
    tmp = insert_ob_in_ob(tmp, op);

#ifdef PLUGINS
      /* Gecko: Copied from drop_object */
      /* GROS: Handle for plugin drop event */
      if(tmp->event_flags & EVENT_FLAG_PICKUP)
      {
        CFParm CFP;
        CFParm *CFR;
        int k, l, m, rtn_script;
		object *event_obj = get_event_object(tmp, EVENT_PICKUP);
        m = 0;
        k = EVENT_PICKUP;
        l = SCRIPT_FIX_ALL;
        CFP.Value[0] = &k;
        CFP.Value[1] = pl; /* Gecko: We want the player/monster not the container (?) */
        CFP.Value[2] = tmp; 
        CFP.Value[3] = op; /* Gecko: Container id goes here */
        CFP.Value[4] = NULL;
        CFP.Value[5] = &tmp_nrof; /* nr of objects */
        CFP.Value[6] = &m;
        CFP.Value[7] = &m;
        CFP.Value[8] = &l;
        CFP.Value[9] = (char *)event_obj->race;
        CFP.Value[10]= (char *)event_obj->slaying;
        if (findPlugin(event_obj->name)>=0)
        {
          CFR = ((PlugList[findPlugin(event_obj->name)].eventfunc) (&CFP));
          rtn_script = *(int *)(CFR->Value[0]);
          /* Gecko: don't know why this is here, but it looks like it can mess things up... */
          if (rtn_script!=0) return;
        }
      }
#endif      
      
    /* All the stuff below deals with client/server code, and is only
     * usable by players
     */
    if(pl->type!=PLAYER) return;

    esrv_send_item (pl, tmp);
    /* These are needed to update the weight for the container we
     * are putting the object in, and the players weight, if different.
     */
    esrv_update_item (UPD_WEIGHT, pl, op);
    if (op!=pl) esrv_send_item (pl, pl);

    /* Update the container the object was in */
    if (env && env!=pl && env!=op) esrv_update_item (UPD_WEIGHT, pl, env);
}


void pick_up(object *op,object *alt) 
/* modified slightly to allow monsters use this -b.t. 5-31-95 */
{
    int need_fix_tmp = 0;
    object *tmp=NULL;
    mapstruct *tmp_map=NULL;
    int count;
    tag_t tag;

    /* Decide which object to pick. */
    if (alt)
    {
        if ( ! can_pick (op, alt)) {
            new_draw_info_format (NDI_UNIQUE, 0, op, "You can't pick up %s.", alt->name);
	    goto leave;
        }
        tmp = alt;
    }
    else
    {
        if (op->below == NULL || ! can_pick (op, op->below)) {
             new_draw_info (NDI_UNIQUE, 0, op, "There is nothing to pick up here.");
             goto leave;
        }
        tmp = op->below;
    }

	if(tmp->type == CONTAINER)
		container_unlink(NULL,tmp);

    /* Try to catch it. */
    tmp_map = tmp->map;
    tmp = stop_item (tmp);
    if (tmp == NULL)
        goto leave;
    need_fix_tmp = 1;
    if ( ! can_pick (op, tmp))
        goto leave;

    if (op->type==PLAYER) {
	count=op->contr->count;
	if (count==0) count = tmp->nrof;
    }
    else
	count=tmp->nrof;

    /* container is open, so use it */
    if (op->type==PLAYER && op->contr->container) {
	alt = op->contr->container;
	if (alt != tmp->env && !sack_can_hold (op, alt, tmp,count))
	    goto leave;
    } else { /* non container pickup */
	for (alt=op->inv; alt; alt=alt->below)
	    if (alt->type==CONTAINER && QUERY_FLAG(alt, FLAG_APPLIED) &&
		alt->race && alt->race==tmp->race &&
		sack_can_hold (NULL, alt, tmp,count))
		break;  /* perfect match */
	
	if (!alt)
	    for (alt=op->inv; alt; alt=alt->below)
		if (alt->type==CONTAINER && QUERY_FLAG(alt, FLAG_APPLIED) &&
		    sack_can_hold (NULL, alt, tmp,count))
		    break;  /* General container comes next */
	if (!alt)
	    alt = op; /* No free containers */
    }
    if(tmp->env == alt) {
	/* here it could be possible to check rent,
	 * if someone wants to implement it
	 */
	alt = op;
    }
#ifdef PICKUP_DEBUG
   if(op->type == PLAYER)
      printf ("Pick_up(): %s picks %s (%d) and inserts it %s.\n",op->name, tmp->name,  op->contr->count, alt->name);
   else
      printf ("Pick_up(): %s picks %s and inserts it %s.\n",op->name, tmp->name, alt->name);
#endif

    /* startequip items are not allowed to be put into containers: */
    if (op->type == PLAYER && alt->type == CONTAINER
	&& QUERY_FLAG (tmp, FLAG_STARTEQUIP))
    {
        new_draw_info (NDI_UNIQUE, 0, op,
                       "This object cannot be put into containers!");
        goto leave;
    }

    tag = tmp->count;
    pick_up_object (op, alt, tmp, count);
    if (was_destroyed (tmp, tag) || tmp->env)
        need_fix_tmp = 0;
    if (op->type == PLAYER)
       op->contr->count=0;
    goto leave;

  leave:
    if (need_fix_tmp)
        fix_stopped_item (tmp, tmp_map, op);
}


/*
 *  This function was part of drop, now is own function. 
 *  Player 'op' tries to put object 'tmp' into sack 'sack', 
 *  if nrof is non zero, then nrof objects is tried to put into sack. 
 */
void put_object_in_sack (object *op, object *sack, object *tmp, long nrof) 
{
    tag_t tmp_tag, tmp2_tag;
    object *tmp2, *tmp_cont;
	/*object *sack2;*/
    char buf[MAX_BUF];
  
    if(op->type != PLAYER) {
        LOG(llevDebug ,"puty_object_in_sack: op not a player\n");
        return;
    }

    if (sack==tmp) 
		return;	/* Can't put an object in itself */
    if (sack->type != CONTAINER) 
	{
      new_draw_info_format(NDI_UNIQUE, 0,op,"The %s is not a container.", query_name(sack));
      return;
    }

	if(tmp->type == CONTAINER)
		container_unlink(NULL,tmp);

	/*
    if (QUERY_FLAG(tmp,FLAG_STARTEQUIP)) {
      new_draw_info_format(NDI_UNIQUE, 0,op,
	"You cannot put the %s in the container.", query_name(tmp));
      return;
    }
	*/

      /* Eneq(@csd.uu.se): If the object to be dropped is a container
       * we instead move the contents of that container into the active
       * container, this is only done if the object has something in it.
       */
	/* we really don't need this anymore - after i had fixed the "can fit in" 
	 * now we put containers with something in REALLY in other containers.
	*/
	/*
    if (tmp->type == CONTAINER && tmp->inv) {

      sack2 = tmp;
      new_draw_info_format(NDI_UNIQUE, 0,op, "You move the items from %s into %s.",
		    query_name(tmp), query_name(op->container));
      for (tmp2 = tmp->inv; tmp2; tmp2 = tmp) {
	  tmp = tmp2->below;
	if (sack_can_hold(op, op->container, tmp2,tmp2->nrof))
	  put_object_in_sack (op, sack, tmp2, 0);
	else {
	  sprintf(buf,"Your %s fills up.", query_name(op->container));
	  new_draw_info(NDI_UNIQUE, 0,op, buf);
	  break;
	}
      }
      esrv_update_item (UPD_WEIGHT, op, sack2);
      return;
    }
	*/
    if (! sack_can_hold (op, sack, tmp,(nrof?nrof:tmp->nrof)))
      return;

    if(QUERY_FLAG(tmp, FLAG_APPLIED)) {
      if (apply_special (op, tmp, AP_UNAPPLY | AP_NO_MERGE))
          return;
    }

    /* we want to put some portion of the item into the container */
    if (nrof && tmp->nrof != (uint32) nrof) {
	object *tmp2 = tmp, *tmp2_cont=tmp->env;
        tmp2_tag = tmp2->count;
	tmp = get_split_ob (tmp, nrof);

	if(!tmp) {
	    new_draw_info(NDI_UNIQUE, 0,op, errmsg);
	    return;
	}
	/* Tell a client what happened other objects */ 
	if (was_destroyed (tmp2, tmp2_tag))
	      esrv_del_item (op->contr, tmp2_tag, tmp2_cont);
	else	/* this can proably be replaced with an update */
	      esrv_send_item (op, tmp2);
    } else
	remove_ob(tmp);

    sprintf(buf, "You put the %s in ", query_name(tmp));
    strcat (buf, query_name(sack));
    strcat (buf, ".");
    tmp_tag = tmp->count;
	tmp_cont = tmp->env;
    tmp2 = insert_ob_in_ob(tmp, sack);
    new_draw_info(NDI_UNIQUE, 0,op,buf);
    fix_player(op); /* This is overkill, fix_player() is called somewhere */
		  /* in object.c */

    /* If an object merged (and thus, different object), we need to
     * delete the original.
     */
    if (tmp2 != tmp)
	esrv_del_item (op->contr, tmp_tag, tmp_cont);

    esrv_send_item (op, tmp2);
    /* update the sacks and players weight */
    esrv_update_item (UPD_WEIGHT, op, sack);
    esrv_update_item (UPD_WEIGHT, op, op);
}

/*
 *  This function was part of drop, now is own function.
 *  Player 'op' tries to drop object 'tmp', if tmp is non zero, then
 *  nrof objects is tried to dropped.
 * This is used when dropping objects onto the floor.
 */
void drop_object (object *op, object *tmp, long nrof)
{
    char buf[MAX_BUF];
    object *floor;

    if (QUERY_FLAG(tmp, FLAG_NO_DROP)) {
#if 0
      /* Eneq(@csd.uu.se): Objects with NO_DROP defined can't be dropped. */
      new_draw_info(NDI_UNIQUE, 0,op, "This item can't be dropped.");
#endif
      return;
    }

    if (op->type == PLAYER)
        op->contr->praying=0;

    if(QUERY_FLAG(tmp, FLAG_APPLIED)) {
      if (apply_special (op, tmp, AP_UNAPPLY | AP_NO_MERGE))
          return;		/* can't unapply it */
    }

	if(tmp->type == CONTAINER)
		container_unlink(NULL,tmp);

    /* We are only dropping some of the items.  We split the current objec
     * off
     */
    if(nrof && tmp->nrof != (uint32) nrof) {
	object *tmp2 = tmp, *tmp2_cont=tmp->env;
        tag_t tmp2_tag = tmp2->count;
	tmp = get_split_ob (tmp, nrof);
	if(!tmp) {
	    new_draw_info(NDI_UNIQUE, 0,op, errmsg);
	    return;
	}
	/* Tell a client what happened rest of objects.  tmp2 is now the
	 * original object
	 */
	 if (op->type == PLAYER)
	 {
                if (was_destroyed (tmp2, tmp2_tag))
                        esrv_del_item (op->contr, tmp2_tag, tmp2_cont);
                else
                        esrv_send_item (op, tmp2);
	};
    } else
      remove_ob (tmp);
#ifdef PLUGINS
      /* GROS: Handle for plugin drop event */
      if(tmp->event_flags&EVENT_FLAG_DROP)
      {
        CFParm CFP;
        CFParm *CFR;
        int k, l, m, rtn_script;
		object *event_obj = get_event_object(tmp, EVENT_DROP);
        m = 0;
        k = EVENT_DROP;
        l = SCRIPT_FIX_ALL;
        CFP.Value[0] = &k;
        CFP.Value[1] = op;
        CFP.Value[2] = tmp;
        CFP.Value[3] = NULL;
        CFP.Value[4] = NULL;
        CFP.Value[5] = &nrof; /* Gecko: Moved nrof to numeric arg to avoid problems */
        CFP.Value[6] = &m;
        CFP.Value[7] = &m;
        CFP.Value[8] = &l;
        CFP.Value[9] = (char *)event_obj->race;
        CFP.Value[10]= (char *)event_obj->slaying;
        if (findPlugin(event_obj->name)>=0)
        {
          CFR = ((PlugList[findPlugin(event_obj->name)].eventfunc) (&CFP));
          rtn_script = *(int *)(CFR->Value[0]);
          if (rtn_script!=0) return;
        }
      }
#endif
    if (QUERY_FLAG (tmp, FLAG_STARTEQUIP)) {
      if (op->type==PLAYER)
	  {
		sprintf(buf,"You drop the %s.", query_name(tmp));
		new_draw_info(NDI_UNIQUE, 0,op,buf);

		if (QUERY_FLAG (tmp, FLAG_UNPAID))
			new_draw_info(NDI_UNIQUE, 0,op,"The shop magic put it back to the storage.");
		else
			new_draw_info(NDI_UNIQUE, 0,op,"The one-drop item vanish to nowhere as you drop it!");
		esrv_del_item (op->contr, tmp->count, tmp->env);
	  }
      free_object(tmp);
      fix_player(op);
      return;
    }

/*  If SAVE_INTERVAL is commented out, we never want to save
 *  the player here.
 */
#ifdef SAVE_INTERVAL
    /* I'm not sure why there is a value check - since the save
     * is done every SAVE_INTERVAL seconds, why care the value
     * of what he is dropping?
     */
    if (op->type == PLAYER && !QUERY_FLAG(tmp, FLAG_UNPAID) &&
      (tmp->nrof ? tmp->value * tmp->nrof : tmp->value > 2000) &&
      (op->contr->last_save_time + SAVE_INTERVAL) <= time(NULL)) {
	  save_player(op, 1);
	  op->contr->last_save_time = time(NULL);
    }
#endif /* SAVE_INTERVAL */


    floor = GET_MAP_OB_LAYER (op->map, op->x, op->y,0);
    if( floor && floor->type == SHOP_FLOOR &&
       !QUERY_FLAG(tmp, FLAG_UNPAID) && tmp->type != MONEY)
	{
		sell_item(tmp,op);
		/* ok, here we insert then the unique shops too, now we have only clone shops */
		if(QUERY_FLAG(tmp, FLAG_UNPAID)) /* ok, we have really selled it - not only droped */
		{
			if (op->type == PLAYER)
			{
				new_draw_info_format(NDI_UNIQUE, 0, op,"The shop magic put it to the storage.");
				esrv_del_item (op->contr, tmp->count,tmp->env);
			}
			free_object(tmp);
			fix_player(op);
			if (op->type == PLAYER)
				esrv_send_item (op, op);

			return;
		}
	}

    tmp->x = op->x;
    tmp->y = op->y;

    if (op->type == PLAYER)
        esrv_del_item (op->contr, tmp->count, tmp->env);

    insert_ob_in_map(tmp, op->map, op,0);

    SET_FLAG (op, FLAG_NO_APPLY);
    remove_ob(op);
    insert_ob_in_map(op, op->map, op, INS_NO_MERGE | INS_NO_WALK_ON);
    CLEAR_FLAG (op, FLAG_NO_APPLY);

	/* Need to update the weight for the player */
    if (op->type == PLAYER)
    {
		fix_player(op);
		esrv_send_item (op, op);
    }
}

void drop(object *op, object *tmp)
{
    /* Hopeful fix for disappearing objects when dropping from a container -
     * somehow, players get an invisible object in the container, and the
     * old logic would skip over invisible objects - works fine for the
     * playes inventory, but drop inventory wants to use the next value.
     */
	/* hmhm... THIS looks strange... for the new invisible system, i removed it MT-11-2002 */
	/* if somewhat get invisible, the we still can handle it - SYS_INV is now always forbidden 
    if (IS_SYS_INVISIBLE(tmp)) {
	if (tmp->env && tmp->env->type != PLAYER) {
	    remove_ob(tmp);
	    free_object(tmp);
	    return;
	} else {
	    while(tmp!=NULL && IS_SYS_INVISIBLE(tmp))
		tmp=tmp->below;
	}
    }
    */
    if (tmp==NULL) {
      new_draw_info(NDI_UNIQUE, 0,op,"You don't have anything to drop.");
      return;
    }
    if (QUERY_FLAG(tmp, FLAG_INV_LOCKED)) {
      new_draw_info(NDI_UNIQUE, 0,op,"This item is locked");
      return;
    }
    if (QUERY_FLAG(tmp, FLAG_NO_DROP)) {
#if 0
      /* Eneq(@csd.uu.se): Objects with NO_DROP defined can't be dropped. */
      new_draw_info(NDI_UNIQUE, 0,op, "This item can't be dropped.");
#endif
      return;
    }


    if (op->type == PLAYER)
    {
	    if (op->contr->container) 
			put_object_in_sack (op, op->contr->container, tmp, op->contr->count);
		else
			drop_object (op, tmp, op->contr->count);
		op->contr->count = 0;
    }
	else
		drop_object(op,tmp,0);

}



/* Command will drop all items that have not been locked */
int command_dropall (object *op, char *params) {

  object * curinv, *nextinv;

  if(op->inv == NULL) {
    new_draw_info(NDI_UNIQUE, 0,op,"Nothing to drop!");
    return 0;
  }
  
  curinv = op->inv;

  /* 
    This is the default.  Drops everything not locked or considered
    not something that should be dropped.
  */
  /*
    Care must be taken that the next item pointer is not to money as
    the drop() routine will do unknown things to it when dropping
    in a shop. --Tero.Pelander@utu.fi
  */

  if(params==NULL) {
    while(curinv != NULL) {
      nextinv = curinv->below;
      while (nextinv && nextinv->type==MONEY)
	nextinv = nextinv->below;
      if(! QUERY_FLAG(curinv,FLAG_INV_LOCKED) && curinv->type != MONEY &&
				curinv->type != FOOD && curinv->type != KEY && curinv->type != SPECIAL_KEY && 
				(curinv->type != GEM && curinv->type != TYPE_JEWEL && curinv->type != TYPE_NUGGET) &&
				!IS_SYS_INVISIBLE(curinv) && (curinv->type!=CONTAINER || 
				(op->type == PLAYER && op->contr->container!=curinv)))
	{
		if (QUERY_FLAG(op, FLAG_STARTEQUIP))
			drop(op,curinv);
       }
      curinv = nextinv;
    }
  }

  else if(strcmp(params, "weapons") == 0) {
    while(curinv != NULL) {
      nextinv = curinv->below;
      while (nextinv && nextinv->type==MONEY)
	nextinv = nextinv->below;
      if(! QUERY_FLAG(curinv,FLAG_INV_LOCKED) && ((curinv->type == WEAPON) ||
	 (curinv->type == BOW) || (curinv->type == ARROW)))
	{
		if (QUERY_FLAG(op, FLAG_STARTEQUIP))
			drop(op,curinv);
	}
      curinv = nextinv;
    }
  }
  
  else if(strcmp(params, "armor") == 0 || strcmp(params, "armour") == 0) {
    while(curinv != NULL) {
      nextinv = curinv->below;
      while (nextinv && nextinv->type==MONEY)
	nextinv = nextinv->below;
      if(! QUERY_FLAG(curinv,FLAG_INV_LOCKED) && ((curinv->type == ARMOUR) ||
	 curinv->type == SHIELD || curinv->type==HELMET))
	{
		if (QUERY_FLAG(op, FLAG_STARTEQUIP))
		  drop(op,curinv);
	}
      curinv = nextinv;
    }
  }

  else if(strcmp(params, "misc") == 0) {
    while(curinv != NULL) {
      nextinv = curinv->below;
      while (nextinv && nextinv->type==MONEY)
	nextinv = nextinv->below;
      if(! QUERY_FLAG(curinv,FLAG_INV_LOCKED) && ! QUERY_FLAG(curinv,FLAG_APPLIED)) {
	switch(curinv->type) {
	case HORN:
	case BOOK:
	case SPELLBOOK:
	case GIRDLE:
	case AMULET:
	case RING:
	case CLOAK:
	case BOOTS:
	case GLOVES:
	case BRACERS:
	case SCROLL:
	case ARMOUR_IMPROVER:
	case WEAPON_IMPROVER:
	case WAND:
	case ROD:
	case POTION:
		if (QUERY_FLAG(op, FLAG_STARTEQUIP))
		  drop(op,curinv);
	  curinv = nextinv;
	  break;
	default:
	  curinv = nextinv;
	  break;
	}
      }
      curinv = nextinv;
    }
  }

  return 0;
}

/* Object op wants to drop object(s) params.  params can be a
 * comma seperated list.
 */

int command_drop (object *op, char *params)
{
    object  *tmp, *next;
    int did_one=0;

    if (!params) {
	new_draw_info(NDI_UNIQUE,0, op, "Drop what?");
	return 0;
    } else {
	for (tmp=op->inv; tmp; tmp=next) {
	    next=tmp->below;
	    if (QUERY_FLAG(tmp,FLAG_NO_DROP) || IS_SYS_INVISIBLE(tmp)) continue;
	    if (item_matched_string(op,tmp,params)) {
		drop(op, tmp);
		did_one=1;
	    }
	}
	if (!did_one) new_draw_info(NDI_UNIQUE, 0,op,"Nothing to drop.");
    }
    if (op->type==PLAYER)
        op->contr->count=0;
    return 0;
}

int command_examine (object *op, char *params)
{
  if(op->type == PLAYER)
      op->contr->praying=0;
  if (!params) {
    object *tmp=op->below;
    while (tmp && !LOOK_OBJ(tmp)) tmp=tmp->below;
    if (tmp) examine(op,tmp);
  }
  else {
    object *tmp=find_best_object_match(op,params);
    if (tmp)
        examine(op,tmp);
    else
	    new_draw_info_format(NDI_UNIQUE,0,op,"Could not find an object that matches %s",params);
  }
  return 0;
}

/* Gecko: added a recursive part to search so that we also search in containers */
static object *find_marked_object_rec(object *op, object **marked, uint32 *marked_count)
{
    object *tmp, *tmp2;
    
    /* This may seem like overkill, but we need to make sure that they
     * player hasn't dropped the item.  We use count on the off chance that
     * an item got reincarnated at some point.
     */
    for (tmp=op->inv; tmp; tmp=tmp->below) 
    {
		if (IS_SYS_INVISIBLE(tmp))
			continue;
		if (tmp == *marked)
		{
			if (tmp->count == *marked_count)
				return tmp;
			else 
			{
				*marked=NULL;
				*marked_count=0;
				return NULL;
			}
		} else if(tmp->inv) {
            tmp2 = find_marked_object_rec(tmp, marked, marked_count);
            if(tmp2)
                return tmp2;
            if(*marked == NULL)
                return NULL;
        }
    }
    return NULL;
}
    

/* op should be a player, params is any params.
 * If no params given, we print out the currently marked object.
 * otherwise, try to find a matching object - try best match first.
 */
int command_mark(object *op, char *params)
{
    if (!op->contr) return 1;
	op->contr->praying=0;
    if (!params) {
	object *mark=find_marked_object(op);
	if (!mark) new_draw_info(NDI_UNIQUE,0,op,"You have no marked object.");
	else new_draw_info_format(NDI_UNIQUE,0,op,"%s is marked.", query_name(mark));
    }
    else {
	object *mark1=find_best_object_match(op, params);
	if (!mark1) {
	    new_draw_info_format(NDI_UNIQUE,0,op,"Could not find an object that matches %s",params);
	    return 1;
	}
	else {
	    op->contr->mark=mark1;
	    op->contr->mark_count=mark1->count;
	    new_draw_info_format(NDI_UNIQUE,0,op,"Marked item %s", query_name(mark1));
	    return 0;
	}
    }
    return 0;	/*shouldnt get here */
}


/* op should be a player.
 * we return the object the player has marked with the 'mark' command
 * below.  If no match is found (or object has changed), we return
 * NULL.  We leave it up to the calling function to print messages if
 * nothing is found.
 */
object *find_marked_object(object *op)
{
  
    if(op->type != PLAYER)
        return NULL;

    if (!op || !op->contr) return NULL;
    if (!op->contr->mark)
		return NULL;

    return find_marked_object_rec(op, &op->contr->mark, &op->contr->mark_count);
}


/* op is the player
 * tmp is the monster being examined.
 */
void examine_monster(object *op,object *tmp) {
    object *mon=tmp->head?tmp->head:tmp;
	char *gender;
	char *att;
	int val, val2, i;

	op->contr->praying=0;
	if(QUERY_FLAG(mon,FLAG_IS_MALE))
	{
		if(QUERY_FLAG(mon,FLAG_IS_FEMALE))
        {
			gender = "hermaphrodite";
			att = "It";
        } 
		else 
		{
			gender = "male";
			att = "He";
		}
	}
	else if(QUERY_FLAG(mon,FLAG_IS_FEMALE))
	{
		gender = "female";
		att = "She";
	}
	else
	{
		gender = "neuter";
		att = "It";
	}

	if(QUERY_FLAG(mon,FLAG_IS_GOOD))
		new_draw_info_format(NDI_UNIQUE, 0,op,"%s is a good aligned %s %s.",att,gender,mon->race);
	else if(QUERY_FLAG(mon,FLAG_IS_EVIL))
		new_draw_info_format(NDI_UNIQUE, 0,op,"%s is a evil aligned %s %s.",att,gender,mon->race);
	else if(QUERY_FLAG(mon,FLAG_IS_NEUTRAL))
		new_draw_info_format(NDI_UNIQUE, 0,op,"%s is a neutral aligned %s %s.",att,gender,mon->race);
	else
		new_draw_info_format(NDI_UNIQUE, 0,op,"%s is a %s %s.",att,gender,mon->race);

	if(mon->type == PLAYER)
		new_draw_info_format(NDI_UNIQUE, 0,op,"%s is level %d and %d years old%s.",att,mon->level,mon->contr->age,QUERY_FLAG(mon,FLAG_IS_AGED)?" (magical aged)":"");
	else
		new_draw_info_format(NDI_UNIQUE, 0,op,"%s is level %d%s.",att,mon->level,QUERY_FLAG(mon,FLAG_IS_AGED)?" and unatural aged":"");

	new_draw_info_format(NDI_UNIQUE, 0,op,"%s has a base damage of %d and hp of %d.",att,mon->stats.dam,mon->stats.maxhp);
	new_draw_info_format(NDI_UNIQUE, 0,op,"%s has a wc of %d and an ac of %d.",att,mon->stats.wc,mon->stats.ac);

	for(val=val2=-1,i=0;i<NROFATTACKS;i++)
	{
		if(mon->resist[i] > 0)
			val = i;
		else if(mon->resist[i] < 0)
			val = i;
	}
	if(val!=-1)
		new_draw_info_format(NDI_UNIQUE, 0,op,"%s can natural resist some attacks.",att);
	if(val2!=-1)
		new_draw_info_format(NDI_UNIQUE, 0,op,"%s is natural vulnerable to some attacks.",att);
    
	for(val=-1,val2=i=0;i<NROFPROTECTIONS;i++)
	{
		if(mon->protection[i] > val2)
		{
			val = i;
			val2=mon->protection[i];
		}
	}
	if(val!=-1)
		new_draw_info_format(NDI_UNIQUE, 0,op,"Best armour protection seems to be for %s.",protection_name[val]);

    if(QUERY_FLAG(mon,FLAG_UNDEAD))
	new_draw_info(NDI_UNIQUE, 0,op,"It is an undead force.");
    /* Anyone know why this used to use the clone value instead of the
     * maxhp field?  This seems that it should give more accurate results.
     */
    switch((mon->stats.hp+1)*4/(mon->stats.maxhp+1)) { /* From 1-4 */
	case 1:
	    new_draw_info_format(NDI_UNIQUE, 0,op,"%s is in a bad shape.",att);
	    break;
	case 2:
	    new_draw_info_format(NDI_UNIQUE, 0,op,"%s is hurt.",att);
	    break;
	case 3:
	    new_draw_info_format(NDI_UNIQUE, 0,op,"%s is somewhat hurt.",att);
	    break;
	default:
	    new_draw_info_format(NDI_UNIQUE, 0,op,"%s is in excellent shape.",att);
	    break;
    }
    if(present_in_ob(POISONING,mon)!=NULL)
		new_draw_info_format(NDI_UNIQUE, 0,op,"%s looks very ill.",att);
}

char *long_desc(object *tmp)
 {
	static char buf[VERY_BIG_BUF];
	char *cp;

	if(tmp==NULL)
		return "";
	buf[0]='\0';

	switch(tmp->type)
	{
		case RING:
		case SKILL:
		case WEAPON:
		case ARMOUR:
		case BRACERS:
		case HELMET:
		case SHIELD:
		case BOOTS:
		case GLOVES:
		case AMULET:
		case GIRDLE:
		case BOW:
		case ARROW:
		case CLOAK:
		case FOOD:
		case DRINK:
		case HORN:
		case WAND:
		case ROD:
		case FLESH:
		case CONTAINER:
			if(*(cp=describe_item(tmp))!='\0')
			{
				int len;

				strncat(buf,query_name(tmp), VERY_BIG_BUF-1);

				buf[VERY_BIG_BUF-1]=0;
				len=strlen(buf);
				if (len<VERY_BIG_BUF-5 && ((tmp->type != AMULET && tmp->type != RING) || tmp->title))
				{
					/* Since we know the length, we save a few cpu cycles by using
					* it instead of calling strcat */
					strcpy(buf+len," ");
					len++;
					strncpy(buf+len, cp, VERY_BIG_BUF-len-1);
					buf[VERY_BIG_BUF-1]=0;
				}
			}
		break;
	}

	if(buf[0]=='\0')
	{

      strncat(buf,query_name(tmp), VERY_BIG_BUF-1);
      buf[VERY_BIG_BUF-1]=0;
	}

	return buf;
}

void examine(object *op, object *tmp) {
    char buf[VERY_BIG_BUF];
	char tmp_buf[64];
    int i;

    if (tmp == NULL || tmp->type == CLOSE_CON)
	return;

    /* Only quetzals can see the resistances on flesh. To realize
	this, we temporarily flag the flesh with SEE_INVISIBLE */
	if (op->type == PLAYER && tmp->type == FLESH && is_dragon_pl(op))
	    SET_FLAG(tmp, FLAG_SEE_INVISIBLE);

    strcpy(buf,"That is ");
	strncat(buf, long_desc(tmp), VERY_BIG_BUF-strlen(buf)-1);
	buf[VERY_BIG_BUF-1]=0;

	if (op->type == PLAYER && tmp->type == FLESH)
	    CLEAR_FLAG(tmp, FLAG_SEE_INVISIBLE);

	/* only add this for usable items, not for objects like walls or floors for example */
	if(!QUERY_FLAG(tmp, FLAG_IDENTIFIED) && !QUERY_FLAG(tmp, FLAG_NO_PICK))
		strncat(buf, " (unidentified)", VERY_BIG_BUF-strlen(buf)-1);
	buf[VERY_BIG_BUF-1]=0;

    new_draw_info(NDI_UNIQUE, 0,op,buf);
    buf[0]='\0';

    if(QUERY_FLAG(tmp, FLAG_MONSTER))
	{
		new_draw_info_format(NDI_UNIQUE, 0,op,"%s.", describe_item(tmp->head?tmp->head:tmp));
		examine_monster(op,tmp);
	}
    /* we don't double use the item_xxx arch commands, so they are always valid */
    else if(QUERY_FLAG(tmp, FLAG_IDENTIFIED)) 
    {
		/* if one of this is set, we have a ego item */
		if(QUERY_FLAG(tmp,FLAG_IS_GOOD))
			new_draw_info_format(NDI_UNIQUE, 0,op,"It is good aligned.");
		else if(QUERY_FLAG(tmp,FLAG_IS_EVIL))
			new_draw_info_format(NDI_UNIQUE, 0,op,"It is evil aligned.");
		else if(QUERY_FLAG(tmp,FLAG_IS_NEUTRAL))
			new_draw_info_format(NDI_UNIQUE, 0,op,"It is neutral aligned.");

		if(tmp->item_level)
        {
            if(tmp->item_skill)
            {                    
                sprintf(buf,"It needs a level of %d in %s to use.",tmp->item_level, 
                                        find_skill_exp_skillname(op, tmp->item_skill));
                new_draw_info(NDI_UNIQUE, 0,op,buf);
            }
            else
            {
                sprintf(buf,"It needs a level of %d to use.",tmp->item_level);
                new_draw_info(NDI_UNIQUE, 0,op,buf);
            }                
        }
        if(tmp->item_quality)
        {
			if(QUERY_FLAG(tmp, FLAG_INDESTRUCTIBLE))
			{
				sprintf(buf,"Qua: %d Con: Indestructible.", tmp->item_quality);
				new_draw_info(NDI_UNIQUE, 0,op,buf);
			}
			else
			{
				sprintf(buf,"Qua: %d Con: %d.", tmp->item_quality, tmp->item_condition);
				new_draw_info(NDI_UNIQUE, 0,op,buf);

				if(QUERY_FLAG(tmp, FLAG_PROOF_ELEMENTAL) || QUERY_FLAG(tmp, FLAG_PROOF_MAGIC)||
					QUERY_FLAG(tmp, FLAG_PROOF_SPHERE)||QUERY_FLAG(tmp, FLAG_PROOF_PHYSICAL))
				{
					int ft=0;

					strcpy(buf,"It is ");
					if(QUERY_FLAG(tmp, FLAG_PROOF_PHYSICAL))
					{
						strcpy(tmp_buf,"acid-proof");
						ft=1;
					}
					if(QUERY_FLAG(tmp, FLAG_PROOF_ELEMENTAL))
					{
						if(ft)
							strcat(buf,tmp_buf);
						strcpy(tmp_buf,"elemental-proof");
						ft+=1;
					}
					if(QUERY_FLAG(tmp, FLAG_PROOF_MAGIC))
					{
						if(ft)
						{
							if(ft>1)
								strcat(buf,", ");
							strcat(buf,tmp_buf);
						}
						strcpy(tmp_buf,"magic-proof");
						ft+=1;
					}
					if(QUERY_FLAG(tmp, FLAG_PROOF_SPHERE))
					{
						if(ft)
						{
							if(ft>1)
								strcat(buf,", ");
							strcat(buf,tmp_buf);
						}
						strcpy(tmp_buf,"sphere-proof");
						ft+=1;
					}
					if(ft)
					{
						if(ft>1)
							strcat(buf," and ");
						strcat(buf,tmp_buf);
						strcat(buf,".");
					}
					new_draw_info(NDI_UNIQUE, 0,op,buf);
				}
				if((QUERY_FLAG(tmp, FLAG_VUL_ELEMENTAL) && !QUERY_FLAG(tmp, FLAG_PROOF_ELEMENTAL)) ||
					(QUERY_FLAG(tmp, FLAG_VUL_MAGIC) && !QUERY_FLAG(tmp, FLAG_PROOF_MAGIC)) ||
					(QUERY_FLAG(tmp, FLAG_VUL_SPHERE) && !QUERY_FLAG(tmp, FLAG_PROOF_SPHERE)) ||
					(QUERY_FLAG(tmp, FLAG_VUL_PHYSICAL) && !QUERY_FLAG(tmp, FLAG_PROOF_PHYSICAL)))
				{
					int ft=0;

					strcpy(buf,"It is vulnerable from ");
					if(QUERY_FLAG(tmp, FLAG_VUL_PHYSICAL) && !QUERY_FLAG(tmp, FLAG_PROOF_PHYSICAL))
					{
						strcpy(tmp_buf,"physical");
						ft=1;
					}
					if(QUERY_FLAG(tmp, FLAG_VUL_ELEMENTAL) && !QUERY_FLAG(tmp, FLAG_PROOF_ELEMENTAL))
					{
						if(ft)
							strcat(buf,tmp_buf);
						strcpy(tmp_buf,"elemental");
						ft+=1;
					}
					if(QUERY_FLAG(tmp, FLAG_VUL_MAGIC) && !QUERY_FLAG(tmp, FLAG_PROOF_MAGIC))
					{
						if(ft)
						{
							if(ft>1)
								strcat(buf,", ");
							strcat(buf,tmp_buf);
						}
						strcpy(tmp_buf,"magic");
						ft+=1;
					}
					if(QUERY_FLAG(tmp, FLAG_VUL_SPHERE) && !QUERY_FLAG(tmp, FLAG_PROOF_SPHERE))
					{
						if(ft)
						{
							if(ft>1)
								strcat(buf,", ");
							strcat(buf,tmp_buf);
						}
						strcpy(tmp_buf,"sphere");
						ft+=1;
					}
					if(ft)
					{
						if(ft>1)
							strcat(buf," and ");
						strcat(buf,tmp_buf);
						strcat(buf,".");
					}
					new_draw_info(NDI_UNIQUE, 0,op,buf);
				}

			}
        }
        buf[0]='\0';    
    }
    
    switch(tmp->type) {
	case SPELLBOOK:
	    if(QUERY_FLAG(tmp, FLAG_IDENTIFIED) && tmp->stats.sp >= 0 && tmp->stats.sp <= NROFREALSPELLS ) {
		if(tmp->sub_type1 == ST1_SPELLBOOK_CLERIC)
		    sprintf(buf,"%s is a %d level prayer.",
			    spells[tmp->stats.sp].name,spells[tmp->stats.sp].level);
		else
		    sprintf(buf,"%s is a %d level spell.",
			    spells[tmp->stats.sp].name,spells[tmp->stats.sp].level);
	    }
	break;

	case BOOK:
	    if(tmp->msg!=NULL)
		strcpy(buf,"Something is written in it.");
	break;

	case CONTAINER:
	    if(tmp->race!=NULL) {
		if(tmp->weight_limit && tmp->stats.Str<100)
		    sprintf (buf,"It can hold only %s and its weight limit is %.1f kg.", 
			 tmp->race, (float)tmp->weight_limit/(10.0f * (float)(100 - tmp->stats.Str)));
		else
		    sprintf (buf,"It can hold only %s and its weight limit is %.1f kg.", tmp->race,
				(float)tmp->weight_limit/(10.0f * (float)(100 - tmp->stats.Str)));
	    } else
		if(tmp->weight_limit && tmp->stats.Str<100)
		    sprintf (buf,"Its weight limit is %.1f kg.", 
			     (float)tmp->weight_limit/(10.0f * (float)(100 - tmp->stats.Str)));
	    break;

	case WAND:
	    if(QUERY_FLAG(tmp, FLAG_IDENTIFIED))
		sprintf(buf,"It has %d charges left.",tmp->stats.food);
	    break;
    }

    if(buf[0]!='\0')
	new_draw_info(NDI_UNIQUE, 0,op,buf);

    if(tmp->material) {
	strcpy(buf,"It is made of: ");
	for(i=0; i < NROFMATERIALS; i++) {
	  if(tmp->material & (1<<i)) {
	    strcat(buf, material[i].name);
	    strcat(buf, " ");
	  }
	}
	new_draw_info(NDI_UNIQUE, 0,op,buf);
    }

    if(tmp->weight) {
	sprintf(buf,tmp->nrof>1?"They weigh %3.3f kg.":"It weighs %3.3f kg.",
            (float)(tmp->nrof?tmp->weight*tmp->nrof:tmp->weight)/1000.0f);
	new_draw_info(NDI_UNIQUE, 0,op,buf);
    }

	if(QUERY_FLAG(tmp, FLAG_STARTEQUIP) || QUERY_FLAG(tmp, FLAG_ONE_DROP)) 
	{
		if(QUERY_FLAG(tmp, FLAG_UNPAID)) /* thats a unpaid clone shop item */
		{
			new_draw_info_format(NDI_UNIQUE, 0,op,"%s would cost you %s.",
								tmp->nrof>1?"They":"It",query_cost_string(tmp,op,F_BUY));
		}
		else /* it is a real one drop item */
		{
			new_draw_info_format(NDI_UNIQUE, 0,op,"%s one-drop item%s.",
									tmp->nrof>1?"They are":"It is a",tmp->nrof>1?"s":"" );
			if(QUERY_FLAG(tmp, FLAG_IDENTIFIED)) 
			{
				if(tmp->value)
					new_draw_info_format(NDI_UNIQUE, 0,op,"But %s worth %s.",
							tmp->nrof>1?"they are":"it is",	query_cost_string(tmp,op,F_TRUE));
				else
					new_draw_info_format(NDI_UNIQUE, 0,op,"%s worthless.",tmp->nrof>1?"They are":"It is");
		
			}
		}
	}
	else if(tmp->value && tmp->type != PLAYER && !QUERY_FLAG(tmp,FLAG_ALIVE))
	{
		if(QUERY_FLAG(tmp, FLAG_IDENTIFIED)) 
		{
			if(QUERY_FLAG(tmp, FLAG_UNPAID))
				new_draw_info_format(NDI_UNIQUE, 0,op,"%s would cost you %s.",
								tmp->nrof>1?"They":"It",query_cost_string(tmp,op,F_BUY));
			else
			{
				new_draw_info_format(NDI_UNIQUE, 0,op,"%s worth %s.",
						tmp->nrof>1?"They are":"It is", query_cost_string(tmp,op,F_TRUE));
				goto dirty_little_jump1;

			}
		}
		else 
		{
				object *floor;
				dirty_little_jump1:
				floor = GET_MAP_OB_LAYER (op->map, op->x, op->y,0);
				if(floor && floor->type == SHOP_FLOOR && tmp->type != MONEY)
				{
					int charisma = op->stats.Cha;  /* used for SK_BARGAINING modification */

					/* this skill give us a charisma boost */
					if (find_skill(op,SK_BARGAINING)) 
					{
						charisma += 4;
						if(charisma>30) 
							charisma = 30;
					}
					new_draw_info_format(NDI_UNIQUE, 0,op,"This shop will pay you %s (%0.1f%%).",
											query_cost_string(tmp,op,F_SELL),20.0f+100.0f*cha_bonus[charisma]);
				}
		}
	}
	else if (tmp->type != PLAYER && !QUERY_FLAG(tmp,FLAG_ALIVE) )
	{
		if(QUERY_FLAG(tmp, FLAG_IDENTIFIED)) 
		{
			if(QUERY_FLAG(tmp, FLAG_UNPAID))
				new_draw_info_format(NDI_UNIQUE, 0,op,"%s would cost nothing.",tmp->nrof>1?"They":"It");
			else 
				new_draw_info_format(NDI_UNIQUE, 0,op,"%s worthless.",tmp->nrof>1?"They are":"It is");
		}
    }
   
    /* Does the object have a message?  Don't show message for all object
     * types - especially if the first entry is a match
     */
    if(tmp->msg && tmp->type != EXIT && tmp->type != BOOK && 
       tmp->type != CORPSE && !QUERY_FLAG(tmp, FLAG_WALK_ON) && 
       strncasecmp(tmp->msg, "@match",7)) {

	/* This is just a hack so when identifying hte items, we print
	 * out the extra message
	 */
	if (need_identify(tmp) && QUERY_FLAG(tmp, FLAG_IDENTIFIED))
	    new_draw_info(NDI_UNIQUE, 0,op, "The object has a story:");

	new_draw_info(NDI_UNIQUE, 0,op,tmp->msg);
    }
    new_draw_info(NDI_UNIQUE, 0,op," "); /* Blank line */
	
	if(QUERY_FLAG(op,FLAG_WIZ))
	{
		dump_object(tmp);
		new_draw_info(NDI_UNIQUE, 0,op,errmsg);
	}
}

/*
 * inventory prints object's inventory. If inv==NULL then print player's
 * inventory. 
 * [ Only items which are applied are showed. Tero.Haatanen@lut.fi ]
 */
void inventory(object *op,object *inv) {
  object *tmp;
  char *in;
  int items = 0, length;

  if (inv==NULL && op==NULL) {
    new_draw_info(NDI_UNIQUE, 0,op,"Inventory of what object?");
    return;
  }
  tmp = inv ? inv->inv : op->inv;

  while (tmp) {
    if ((!IS_SYS_INVISIBLE(tmp) && 
        (inv==NULL || inv->type == CONTAINER || QUERY_FLAG(tmp, FLAG_APPLIED)))
         || (!op || QUERY_FLAG(op, FLAG_WIZ)))
      items++;
    tmp=tmp->below;
  }
  if (inv==NULL) { /* player's inventory */
    if (items==0) {
      new_draw_info(NDI_UNIQUE, 0,op,"You carry nothing.");
      return;
    } else {
      length = 28;
      in = "";
      new_draw_info(NDI_UNIQUE, 0,op,"Inventory:");
    }
  } else {
    if (items==0) 
      return;
    else { 
      length = 28;
      in = "  ";
    }
  }
  for (tmp=inv?inv->inv:op->inv; tmp; tmp=tmp->below) {
    if((!op||!QUERY_FLAG(op, FLAG_WIZ)) && (IS_SYS_INVISIBLE(tmp) || 
       (inv && inv->type != CONTAINER && !QUERY_FLAG(tmp, FLAG_APPLIED))))
      continue;
    if((!op || QUERY_FLAG(op, FLAG_WIZ)))
      new_draw_info_format(NDI_UNIQUE, 0,op ,"%s- %-*.*s (%5d) %-8s", in, length, length,
		     query_name(tmp), tmp->count,query_weight(tmp));
    else
      new_draw_info_format(NDI_UNIQUE,0, op, "%s- %-*.*s %-8s", in, length+8, 
		     length+8, query_name(tmp),
                     query_weight(tmp));
  }
  if(!inv && op) {
    new_draw_info_format(NDI_UNIQUE,0, op ,"%-*s %-8s",
            41,"Total weight :",query_weight(op));
  }
}


int command_pickup (object *op, char *params)
{
  uint32 i;
  char putstring[128];

  if(op->type != PLAYER) {
    LOG(llevDebug ,"command_pickup: op not a player\n");
    return 1;
  }
      
  if(!params) {
    /* if the new mode is used, just print the settings */
    /* yes, a GOTO is ugly, but its simpple and should stay until this
     * mode is cleanly integrated and the old one deprecated */
    if(op->contr->mode & PU_NEWMODE) 
    {
      i=op->contr->mode;
      goto NEWPICKUP;
    }
    LOG(llevDebug ,"command_pickup: !params\n");
    set_pickup_mode(op, (op->contr->mode > 6)? 0: op->contr->mode+1);
    return 0;
  }
  if(params==NULL || !sscanf(params, "%ud", &i) || i<0 ) {
    LOG(llevDebug ,"command_pickup: params==NULL\n");
    new_draw_info(NDI_UNIQUE, 0,op,"Usage: pickup <0-7> or <value_density> .");
    return 1;
  }
  set_pickup_mode(op,i);

/* To me, all this output is meaningless - it certainly is not
 * humanly readable, and if anything will create more questions/bugs
 * as people will ask 'what does this strange message mean'.
 */

#if 0
  sprintf(putstring,"command_pickup: set_pickup_mode\ndec %u, 0x%x", i, i);
  new_draw_info(NDI_UNIQUE, 0,op,putstring);
  sprintf(putstring,"0b");

  for(j=0;j<32;j++) 
  {
    strcat(putstring,((i>>(31-j))&0x01)?"1":"0");
    if(!((j+1)%4))strcat(putstring," ");
  }
  new_draw_info(NDI_UNIQUE, 0,op,putstring);
#endif

NEWPICKUP:
#if 1
  if(!(i & PU_NEWMODE)) return 1;

  sprintf(putstring,"%d NEWMODE",i & PU_NEWMODE?1:0);
  new_draw_info(NDI_UNIQUE, 0,op,putstring);
  sprintf(putstring,"%d DEBUG",i & PU_DEBUG?1:0);
  new_draw_info(NDI_UNIQUE, 0,op,putstring);
  sprintf(putstring,"%d INHIBIT",i & PU_INHIBIT?1:0);
  new_draw_info(NDI_UNIQUE, 0,op,putstring);
  sprintf(putstring,"%d STOP",i & PU_STOP?1:0);
  new_draw_info(NDI_UNIQUE, 0,op,putstring);

  sprintf(putstring,"%d <= x pickup weight/value RATIO (0==off)",(i & PU_RATIO)*5);
  new_draw_info(NDI_UNIQUE, 0,op,putstring);

  sprintf(putstring,"%d FOOD",i & PU_FOOD?1:0);
  new_draw_info(NDI_UNIQUE, 0,op,putstring);
  sprintf(putstring,"%d DRINK",i & PU_DRINK?1:0);
  new_draw_info(NDI_UNIQUE, 0,op,putstring);
  sprintf(putstring,"%d VALUABLES",i & PU_VALUABLES?1:0);
  new_draw_info(NDI_UNIQUE, 0,op,putstring);

  sprintf(putstring,"%d BOW",i & PU_BOW?1:0);
  new_draw_info(NDI_UNIQUE, 0,op,putstring);
  sprintf(putstring,"%d ARROW",i & PU_ARROW?1:0);
  new_draw_info(NDI_UNIQUE, 0,op,putstring);

  sprintf(putstring,"%d HELMET",i & PU_HELMET?1:0);
  new_draw_info(NDI_UNIQUE, 0,op,putstring);
  sprintf(putstring,"%d SHIELD",i & PU_SHIELD?1:0);
  new_draw_info(NDI_UNIQUE, 0,op,putstring);
  sprintf(putstring,"%d ARMOUR",i & PU_ARMOUR?1:0);
  new_draw_info(NDI_UNIQUE, 0,op,putstring);

  sprintf(putstring,"%d BOOTS",i & PU_BOOTS?1:0);
  new_draw_info(NDI_UNIQUE, 0,op,putstring);
  sprintf(putstring,"%d GLOVES",i & PU_GLOVES?1:0);
  new_draw_info(NDI_UNIQUE, 0,op,putstring);
  sprintf(putstring,"%d CLOAK",i & PU_CLOAK?1:0);
  new_draw_info(NDI_UNIQUE, 0,op,putstring);
  sprintf(putstring,"%d KEY",i & PU_KEY?1:0);
  new_draw_info(NDI_UNIQUE, 0,op,putstring);

  sprintf(putstring,"%d MISSILEWEAPON",i & PU_MISSILEWEAPON?1:0);
  new_draw_info(NDI_UNIQUE, 0,op,putstring);
  sprintf(putstring,"%d ALLWEAPON",i & PU_ALLWEAPON?1:0);
  new_draw_info(NDI_UNIQUE, 0,op,putstring);
  sprintf(putstring,"%d MAGICAL",i & PU_MAGICAL?1:0);
  new_draw_info(NDI_UNIQUE, 0,op,putstring);
  sprintf(putstring,"%d POTION",i & PU_POTION?1:0);
  new_draw_info(NDI_UNIQUE, 0,op,putstring);

  new_draw_info(NDI_UNIQUE, 0,op,"");
#endif

  return 1;
}

void set_pickup_mode(object *op,int i) {
  if(op->type != PLAYER) {
    LOG(llevDebug ,"set_pickup_mode: op not a player\n");
    return;
  }
  
  switch(op->contr->mode=i) {
    case 0:
      new_draw_info(NDI_UNIQUE, 0,op,"Mode: Don't pick up.");
      break;
    case 1:
      new_draw_info(NDI_UNIQUE, 0,op,"Mode: Pick up one item.");
      break;
    case 2:
      new_draw_info(NDI_UNIQUE, 0,op,"Mode: Pick up one item and stop.");
      break;
    case 3:
      new_draw_info(NDI_UNIQUE, 0,op,"Mode: Stop before picking up.");
      break;
    case 4:
      new_draw_info(NDI_UNIQUE, 0,op,"Mode: Pick up all items.");
      break;
    case 5:
      new_draw_info(NDI_UNIQUE, 0,op,"Mode: Pick up all items and stop.");
      break;
    case 6:
      new_draw_info(NDI_UNIQUE, 0,op,"Mode: Pick up all magic items.");
      break;
    case 7:
      new_draw_info(NDI_UNIQUE, 0,op,"Mode: Pick up all coins and jewels");
      break;
    }
}

#ifdef SEARCH_ITEMS
int command_search_items (object *op, char *params)
{
  char buf[MAX_BUF];

  if(op->type != PLAYER) {
    LOG(llevDebug ,"command_search_items: op not a player\n");
    return 0;
  }
  
  if(params == NULL) {
	if(op->contr->search_str[0]=='\0') {
	  new_draw_info(NDI_UNIQUE, 0,op,"Example: &/search &magic+1");
	  new_draw_info(NDI_UNIQUE, 0,op,"Would automatically pick up all");
	  new_draw_info(NDI_UNIQUE, 0,op,"items containing the word 'magic+1'.");
	  return 1;
	}
	op->contr->search_str[0]='\0';
	new_draw_info(NDI_UNIQUE, 0,op,"Search mode turned off.");
	fix_player(op);
	return 1;
  }
  if((int)strlen(params) >= MAX_BUF) {
	new_draw_info(NDI_UNIQUE, 0,op,"Search string too long.");
	return 1;
      }
  strcpy(op->contr->search_str, params);
      sprintf(buf,"Searching for '%s'.",op->contr->search_str);
      new_draw_info(NDI_UNIQUE, 0,op,buf);
	fix_player(op);
      return 1;
}
#endif /* SEARCH_ITEMS */
