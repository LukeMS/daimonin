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
#ifndef __CEXTRACT__
#include <sproto.h>
#endif

/*
 * When parsing a message-struct, the msglang struct is used
 * to contain the values.
 * This struct will be expanded as new features are added.
 * When things are stable, it will be parsed only once.
 */

typedef struct _msglang {
  char **messages;	/* An array of messages */
  char ***keywords;	/* For each message, an array of strings to match */
} msglang;


extern spell spells[NROFREALSPELLS];
static object *spawn_monster(object *gen, object *orig, int range);

/* update (or clear) an npc's enemy. Performs m ost of the housekeeping
 * related to switching enemies. 
 * You should always use this method to set (or clear) a npc's enemy.
 * 
 * If enemy is given an aggro wp may be set up.
 * If rv is given, it will be filled out with the vector to enemy
 * 
 * enemy and/or rv may be NULL
 */
void set_npc_enemy(object *npc, object *enemy, rv_vector *rv) 
{
    object *aggro_wp;
    rv_vector rv2;
   
    /* Do nothing if new enemy == old enemy */
    if(enemy == npc->enemy && (enemy == NULL || enemy->count == npc->enemy_count))
        return;
  
    /* Players don't need waypoints, speed updates or aggro counters */
    if(npc->type == PLAYER) {
        npc->enemy = enemy;
        npc->enemy_count = enemy->count;
        return;
    }
    
    /* Non-waypoint related stuff */
    if(enemy) {
        if(rv == NULL)
            rv = &rv2;
        get_rangevector(npc, enemy, rv, 0x8); 
        npc->enemy_count = enemy->count;
        
        npc->last_eat = 0;	/* important: thats our "we lose aggro count" - reset to zero here */
        
        /* monster has changed status from normal to attack - lets hear it! */
        if(npc->enemy == NULL && !QUERY_FLAG(npc,FLAG_FRIENDLY))
            play_sound_map(npc->map, npc->x, npc->y, SOUND_GROWL, SOUND_NORMAL);
    } 
    npc->enemy = enemy;
    /* Update speed */
    set_mobile_speed(npc, 0);

    /* Setup aggro waypoint */
    if(! aggro_wp_archetype) {
#ifdef DEBUG_PATHFINDING        
        LOG(llevDebug,"set_npc_enemy(): Aggro waypoints disabled\n"); 
#endif            
        return;
    }
    
    /* TODO: check intelligence against lower limit to allow pathfind */
    aggro_wp = get_aggro_waypoint(npc);
    
    /* Create a new aggro wp for npc? */
    if(!aggro_wp && enemy) {
        aggro_wp = arch_to_object(aggro_wp_archetype);
        insert_ob_in_ob(aggro_wp, npc);
        SET_FLAG(aggro_wp, FLAG_DAMNED); /* Mark as aggro WP */
        aggro_wp->owner = npc;
#ifdef DEBUG_PATHFINDING        
        LOG(llevDebug,"set_npc_enemy(): created wp for '%s'\n", npc->name); 
#endif            
    }                                            

    /* Set up waypoint target (if we actually got a waypoint) */
    if(aggro_wp) {
        if(enemy) {
            aggro_wp->enemy_count = npc->enemy_count;
            aggro_wp->enemy = enemy;
            FREE_AND_ADD_REF_HASH(aggro_wp->name, enemy->name);
#ifdef DEBUG_PATHFINDING        
            LOG(llevDebug,"set_npc_enemy(): got wp for '%s' -> '%s'\n", npc->name, enemy->name); 
#endif            
        } else {
            aggro_wp->enemy = NULL;
#ifdef DEBUG_PATHFINDING        
            LOG(llevDebug,"set_npc_enemy(): cleared aggro wp for '%s'\n", npc->name); 
#endif            
        }
    }
}

/* checks npc->enemy and returns that enemy if still valid,
 * NULL otherwise.
 * this is map tile aware.
 */
/* i had removed the random target leave, this invokes problems with friendly
 * objects, getting attacked and defending herself - they don't try to attack
 * again then but perhaps get attack on and on 
 * If we include a aggravated flag in , we can handle evil vs evil and good vs good
 * too. MT
*/
object *check_enemy(object *npc, rv_vector *rv) {
    
    /* if this is pet, let him attack the same enemy as his owner
     * TODO: when there is no ower enemy, try to find a target,
     * which CAN attack the owner. */
    if ((npc->move_type & HI4) == PETMOVE)
    {
        if (npc->owner != NULL)
        {
			/* if owner enemy != pet enemy, change it! */
			if(npc->owner->enemy && (npc->enemy != npc->owner->enemy ||
									npc->enemy_count != npc->enemy->count))
                set_npc_enemy(npc, npc->owner->enemy, NULL);
        }
	    else
		{
			if(npc->enemy)
                set_npc_enemy(npc, NULL, NULL);
		}
    }
	
	/* first check the easy stuff... enemy ptr, count, remove and "still somewhere we can reach".
	 * also check here for "self hate" ;)
	 !on_same_map(npc, npc->enemy) || <- this is in can_detect_enemy()
	 */

    /*LOG(-1,"CHECK_START: %s -> %s (%x - %x)\n", query_name(npc),query_name(npc->enemy), npc->enemy?npc->enemy->count:2,npc->enemy_count);*/
    if(npc->enemy == NULL)
        return NULL;

    if ( !OBJECT_VALID(npc->enemy, npc->enemy_count) || npc == npc->enemy) {
        set_npc_enemy(npc, NULL, NULL);
		return NULL;
    }

	/* check flags for friendly npc and aggressive mobs (unaggressive will not handled here -
	 * it only means it don't seek targets - its not a "no fight" flag.
	 * important: here we add our aggravate flag - then friendly will attack friendly (and attack
	 * on until flag is removed = ring of conflict. Same for aggro mobs.
	 */
	if(QUERY_FLAG(npc, FLAG_FRIENDLY))
	{
		/* NPC should not attack players or other friendly units on purpose */
		if(npc->enemy->type == PLAYER || QUERY_FLAG(npc->enemy, FLAG_FRIENDLY)) {
            set_npc_enemy(npc, NULL, NULL);
			return NULL;
        }
	}
	else /* and the same for unfriendly */
	{
		/* this is a important check - without this, a single area spell from a 
		 * mob will aggravate all other mobs to him - they will slaughter themself
		 * and not the player.
		 */
		if( !QUERY_FLAG(npc->enemy, FLAG_FRIENDLY) && npc->enemy->type!=PLAYER) {
            set_npc_enemy(npc, NULL, NULL);
			return NULL;
        }
	}

    return can_detect_enemy(npc,npc->enemy,rv)?npc->enemy:NULL;
}


/* Tries to find an enmy for npc.  We pass the range vector since
 * our caller will find the information useful.
 * Currently, only move_monster calls this function.
 * Note: find_enemy() don't find a enemy - it checks only the old one is valid
 * and hitable and change target to a better enemy when possible. 
 */
object *find_enemy(object *npc, rv_vector *rv)
{
    object *tmp=NULL;

	/* BERSERK is not activated atm - like aggravation & conflict - will come */
    /* if we berserk, we don't care about others - we attack all we can find */
    if(QUERY_FLAG(npc,FLAG_BERSERK))
    {
        npc->attacked_by = NULL;     /* always clear the attacker entry */    
 		tmp = find_nearest_living_creature(npc);
 		if (tmp) get_rangevector(npc, tmp, rv, 0);
 		return tmp;
    }

    /* Here is the main enemy selection.
     * We want this: if there is an enemy, attack him until its not possible or 
     * one of both is dead.
     * If we have no enemy and we are...
     * a monster: try to find a player, a pet or a friendly monster
     * a friendly: only target a monster which is targeting you first or targeting a player
     * a pet: attack player enemy or a monster
     */

    /* pet move */
    if ((npc->move_type & HI4) == PETMOVE)
    {
        npc->attacked_by = NULL;     /* always clear the attacker entry */    
        tmp= get_pet_enemy(npc,rv);
		npc->last_eat = 0;
	 	if (tmp) get_rangevector(npc, tmp, rv, 0);
			return tmp;
    }
    /* we check our old enemy. */

	tmp=check_enemy(npc, rv); /* if tmp != 0, we have succesful callled get_rangevector() too */
  
	
    /*LOG(-1,"CHECK: mob %s -> <%s> (%s (%d - %d))\n", query_name(npc),query_name(tmp),npc->attacked_by?npc->attacked_by->name:"xxx",
		 npc->attacked_by_distance, (int)rv->distance);
	*/
    if(!tmp || (npc->attacked_by && npc->attacked_by_distance <(int)rv->distance))
    {
        if(OBJECT_VALID(npc->attacked_by, npc->attacked_by_count)) /* if we have an attacker, check him */
        {
            /* TODO: thats not finished */
            /* we don't want a fight evil vs evil or good against non evil */
            if((QUERY_FLAG(npc, FLAG_FRIENDLY) && QUERY_FLAG(npc->attacked_by, FLAG_FRIENDLY)) ||
                    (!QUERY_FLAG(npc, FLAG_FRIENDLY) && 
                     (!QUERY_FLAG(npc->attacked_by, FLAG_FRIENDLY) && npc->attacked_by->type!=PLAYER)) )        
                CLEAR_FLAG(npc,FLAG_SLEEP); /* skip it, but lets wakeup */
            else if(on_same_map(npc, npc->attacked_by)) /* thats the only thing we must know... */
            {
                CLEAR_FLAG(npc,FLAG_SLEEP); /* well, NOW we really should wake up! */

                set_npc_enemy(npc, npc->attacked_by, rv);

                npc->attacked_by = NULL;     /* always clear the attacker entry */

                return npc->enemy; /* yes, we face our attacker! */
            }
        }
        /* i think to add here a counter to determinate a mob lost a enemy or the enemy 
		 * was x rounds out of range - then perhaps we should search a new one 
		 */
        /* we have no legal enemy or attacker, so we try to target a new one */
        if(!QUERY_FLAG(npc, FLAG_UNAGGRESSIVE))
        {
			if(QUERY_FLAG(npc, FLAG_FRIENDLY))
		 		tmp = find_nearest_living_creature(npc);
			else
		        tmp = get_nearest_player(npc);
           
            if(tmp != npc->enemy)
                set_npc_enemy(npc, tmp, rv);
        }
    }
	else /* here we have a valid enemy before we called this function and its still the same */
	{
		/* if our enemy is to far away ... */
		if((int)rv->distance >= (MAX_AGGRO_RANGE>npc->stats.Wis?MAX_AGGRO_RANGE:npc->stats.Wis))
		{
			/* then start counting... */
			if(++npc->last_eat > MAX_AGGRO_TIME)
			{
				/* last try - if for some reason a valid attacker there - give it a try */
				if(OBJECT_VALID(npc->attacked_by, npc->attacked_by_count)) 
                    set_npc_enemy(npc, npc->attacked_by, rv);
                else if(npc->enemy)
                    set_npc_enemy(npc, NULL, NULL);
                
                return npc->enemy;
			}
		}
		else
			npc->last_eat = 0; /* our mob is aggroed again - because target is in range again */
	}

    npc->attacked_by = NULL;     /* always clear the attacker entry */        
    return tmp;
}

/* Sees if this monster should wake up.
 * Currently, this is only called from move_monster, and
 * if enemy is set, then so should be rv.
 */

int check_wakeup(object *op, object *enemy, rv_vector *rv) {
    int radius = op->stats.Wis>MIN_MON_RADIUS?op->stats.Wis:MIN_MON_RADIUS;

    /* Trim work - if no enemy, no need to do anything below */
    if (!enemy) return 0;

    /* blinded monsters can only find nearby objects to attack */
    if(QUERY_FLAG(op, FLAG_BLIND) && !QUERY_FLAG(op, FLAG_SEE_INVISIBLE)) 
	radius = MIN_MON_RADIUS;

    /* This covers the situation where the monster is in the dark 
     * and has an enemy. If the enemy has no carried light (or isnt 
     * glowing!) then the monster has trouble finding the enemy. 
     * Remember we already checked to see if the monster can see in 
     * the dark. */

    else if(op->map&&op->map->darkness>0&&enemy&&!IS_INVISIBLE(enemy,op)&&
	    !stand_in_light(enemy)&&(!QUERY_FLAG(op,FLAG_SEE_IN_DARK)||
	    !QUERY_FLAG(op,FLAG_SEE_INVISIBLE))) {
		int dark = radius/(op->map->darkness);
		radius = (dark>MIN_MON_RADIUS)?(dark+1):MIN_MON_RADIUS;
    }
    else if(!QUERY_FLAG(op,FLAG_SLEEP)) return 1;

    /* enemy should already be on this map, so don't really need to check
     * for that.
     */
    if (rv->distance < QUERY_FLAG(enemy, FLAG_STEALTH)?(radius/2)+1:radius) {
	CLEAR_FLAG(op,FLAG_SLEEP);
	return 1;
    }
    return 0;
}

/* determine if we can 'detect' the enemy. Check for walls blocking the
 * los. Also, just because its hidden/invisible, we may be sensitive/smart 
 * enough (based on Wis & Int) to figure out where the enemy is. -b.t. 
 * modified by MSW to use the get_rangevector so that map tiling works
 * properly.  I also so odd code in place that checked for x distance
 * OR y distance being within some range - that seemed wrong - both should
 * be within the valid range. MSW 2001-08-05
 */
/* have not checked this function MT -2003 */
/* better to check this function asap :) looks weird in many parts... MT-2003 - moved to todo list */
int can_detect_enemy (object *op, object *enemy, rv_vector *rv) {

    /* null detection for any of these condtions always */
    if(!op || !enemy || !op->map || !enemy->map)
        return 0;

    /* If the monster (op) has no way to get to the enemy, do nothing */
    if (!on_same_map(op, enemy))
        return 0;

#if 0
    /* this causes problems, dunno why.. */
    /* are we trying to look through a wall? */ 
    /* probably isn't safe for multipart maps either */
    if(path_to_player(op->head?op->head:op,enemy,0)==0) return 0;
#endif

	/* we check for sys_invisible and normal */
	/* this tmp_invis MUST BE CHECKED - i don't use it in new invis code - add or remove */
	if(IS_INVISIBLE(enemy,op) && (!enemy->contr || !enemy->contr->tmp_invis))
	    return 0;

    get_rangevector(op, enemy, rv, 0);


		/* opponent is unseen? We still have a chance to find them if
     * they are 1) standing in dark square.
     */
    if(!can_see_enemy(op,enemy)) {
	int radius = MIN_MON_RADIUS;
	/* This is percentage change of being discovered while standing
	 * *adjacent* to the monster */
	int hide_discovery = enemy->hide?op->stats.Int/5:-1;

	/* The rest of this is for monsters. Players are on their own for
	 * finding enemies!
	 */
	if(op->type==PLAYER) return 0;

	/* Determine Detection radii */
	if(!enemy->hide)  /* to detect non-hidden (eg dark/invis enemy) */
	    radius = (op->stats.Wis/5)+1>MIN_MON_RADIUS?(op->stats.Wis/5)+1:MIN_MON_RADIUS;
	else { /* a level/INT/Dex adjustment for hiding */
	    object *sk_hide;
	    int bonus = (op->level/2) + (op->stats.Int/5);

	    if(enemy->type==PLAYER) {
		if((sk_hide = find_skill(enemy,SK_HIDING)))
		    bonus -= sk_hide->level;
		else { 
		    LOG(llevBug,"BUG: can_detect_enemy() got hidden player w/o hiding skill!");
		    make_visible(enemy);
		    radius=radius<MIN_MON_RADIUS?MIN_MON_RADIUS:radius;
		}
	    }
	    else /* enemy is not a player */
		bonus -= enemy->level;

	    radius += bonus/5;
	    hide_discovery += bonus*5;
	} /* else creature has modifiers for hiding */

	/* Radii stealth adjustment. Only if you are stealthy 
	 * will you be able to sneak up closer to creatures */ 
	if(QUERY_FLAG(enemy,FLAG_STEALTH)) 
	    radius = radius/2, hide_discovery = hide_discovery/3;

	/* Radii adjustment for enemy standing in the dark */ 
	if(op->map->darkness>0 && !stand_in_light(enemy)) {

	    /* on dark maps body heat can help indicate location with infravision.
	     * There was a check for immunity for fire here (to increase radius) -
	     * I'm not positive if that makes sense - something could be immune to fire
	     * but not be any warmer blooded than something else.
	     */
	    if(QUERY_FLAG(op,FLAG_SEE_IN_DARK) && is_true_undead(enemy))
		radius += op->map->darkness/2;
	    else
		radius -= op->map->darkness/2;

	    /* op next to a monster (and not in complete darkness) 
	    * the monster should have a chance to see you. */
	    if(radius<MIN_MON_RADIUS && op->map->darkness<5 && rv->distance<=1)
		radius = MIN_MON_RADIUS;
	} /* if on dark map */

	/* Lets not worry about monsters that have incredible detection
	 * radii, we only need to worry here about things the player can
	 * (potentially) see. 
	 * Increased this from 5 to 13 - with larger map code, things that
	 * far out are visible.  Note that the distance field in the
	 * vector is real distance, so in theory this should be 18 to
	 * find that.
	 */
	if(radius>10) radius = 13;

	/* Enemy in range! Now test for detection */
	if ((int) rv->distance <= radius) {
	    /* ah, we are within range, detected? take cases */
	    if(!IS_INVISIBLE(enemy,op)) /* enemy in dark squares... are seen! */
		return 1;
	    else if(enemy->hide||(enemy->contr&&enemy->contr->tmp_invis)) { 
		/* hidden or low-quality invisible */  

		/* There is a a small chance each time we check this function 
		 * that we can detect hidden enemy. This means the longer you stay 
		 * near something, the greater the chance you have of being 
		 * discovered. */
		if(enemy->hide && (rv->distance <= 1) && (RANDOM()%100<=hide_discovery)) {
		    make_visible(enemy);
		    /* inform players of new status */
		    if(enemy->type==PLAYER && player_can_view(enemy,op)) 
			new_draw_info_format(NDI_UNIQUE,0, enemy,
					     "You are discovered by %s!",op->name);
		    return 1; /* detected enemy */ 
		} /* if enemy is hiding */

		/* If the hidden/tmp_invis enemy is nearby we accellerate the time of 
		 * becoming unhidden/visible (ie as finding the enemy is easier)
		 * In order to leave actual discovery (invisible=0 state) to
		 * be handled above (not here) we only decrement so that 
		 * enemy->invisible>1 is preserved. 
		 */
		/* ok, here we need some more tricky. invisible means always invisible
		 * until a.) the effect wears out or b.) we can see it (by get see_invisible).
		 * hiding is something really different.
		 * I think i will handle player different - here simple spells will be broken
		 * when you attack invisible. But heavy magic like the one ring will still
		 * in effect even we attack. Or better: the item gets a "timeout" of x seconds
		 * - then the item reinstall the invisibility. MT-11-2002 
		 */
		/*
		if((enemy->invisible-=RANDOM()%(op->stats.Int+2))<1) 
		    enemy->invisible=1;
		*/
	    } 
		
		/* enemy is hidding or invisible */

	    /* MESSAGING: SO we didnt find them (wah!), we may warn a 
	     * player that the monster is getting close to discovering them. 
	     *
	     * We only warn the player if: 
	     *   1) player has los to the monster
	     *   2) random value based on player Int value
	     */
	    if(enemy->type==PLAYER 
	       && (RANDOM()%(enemy->stats.Int+10)> MAX_STAT/2)
	       && player_can_view(enemy,op)) { 
		    new_draw_info_format(NDI_UNIQUE,0, enemy, 
					 "You see %s noticing your position.", query_name(op));
	    } /* enemy is a player */

        return 0;
	} /* creature is withing range */
    } /* if creature can see its enemy */
    /* returning 1 here suggests to me that if the enemy is visible, no matter
     * how far away, the creature can see them.  Is that really what we want?
	 */

    return 1;
}

/* determine if op stands in a lighted square. This is not a very
 * intellegent algorithm. For one thing, we ignore los here, SO it 
 * is possible for a bright light to illuminate a player on the 
 * other side of a wall (!). 
 */

int stand_in_light( object *op) {

    if(!op) return 0;
    if(op->glow_radius) return 1;

    if(op->map) {
		mapstruct *m;
		int x, y, xt, yt;

	/* Check the spacs with the max light radius to see if any of them
	 * have lights, and if the light is bright enough to illuminate
	 * this object.  Like the los.c logic, this presumes a square
	 * lighting area.
	 */
	for (x = op->x - MAX_LIGHT_RADII; x< op->x + MAX_LIGHT_RADII; x++) {
	    for (y = op->y - MAX_LIGHT_RADII; y< op->y + MAX_LIGHT_RADII; y++) {
		xt=x;yt=y;
		if (!(m=out_of_map(op->map, &xt, &yt))) continue;

		if (GET_MAP_LIGHT(m, xt, yt) > MAX(abs(x - op->x), abs(y - op->y))) return 1;
	    }
	}
    }
    return 0;
}

/* assuming no walls/barriers, lets check to see if its *possible* 
 * to see an enemy. Note, "detection" is different from "seeing".
 * See can_detect_enemy() for more details. -b.t.
 */
int can_see_enemy (object *op, object *enemy) {
  object *looker = op->head?op->head:op;

  /* safety */
  if(!looker||!enemy||!QUERY_FLAG(looker,FLAG_ALIVE))
    return 0; 

  /* we dont give a full treatment of xrays here (shorter range than normal,
   * see through walls). Should we change the code elsewhere to make you 
   * blind even if you can xray? */
  if(QUERY_FLAG(looker,FLAG_BLIND)&&
    (!QUERY_FLAG(looker,FLAG_SEE_INVISIBLE)||QUERY_FLAG(looker,FLAG_XRAYS)))
    return 0;

  /* checking for invisible things */
  if(IS_INVISIBLE(enemy,looker)) {
  
    /* HIDDEN ENEMY. by definition, you can't see hidden stuff! 
     * However,if you carry any source of light, then the hidden
     * creature is seeable (and stupid) */
    if(has_carried_lights(enemy)) { 
      if(enemy->hide) { 
	make_visible(enemy);
        new_draw_info(NDI_UNIQUE,0, enemy,
	  "Your light reveals your hidding spot!");
      }
      return 1;
    } else if (enemy->hide) return 0; 

    /* INVISIBLE ENEMY. */
    if(!QUERY_FLAG(looker,FLAG_SEE_INVISIBLE)
      &&(is_true_undead(looker)==(int)(QUERY_FLAG(enemy,FLAG_UNDEAD))))
      return 0;

  } else if(looker->type==PLAYER) /* for players, a (possible) shortcut */
      if(player_can_view(looker,enemy)) return 1;

  /* ENEMY IN DARK MAP. Without infravision, the enemy is not seen 
   * unless they carry a light or stand in light. Darkness doesnt
   * inhibit the undead per se (but we should give their archs
   * CAN_SEE_IN_DARK, this is just a safety  
   * we care about the enemy maps status, not the looker.
   * only relevant for tiled maps, but it is possible that the
   * enemy is on a bright map and the looker on a dark - in that
   * case, the looker can still see the enemy
   */
  if(enemy->map->darkness>0&&!stand_in_light(enemy) 
     &&(!QUERY_FLAG(looker,FLAG_SEE_IN_DARK)||
        !is_true_undead(looker)||!QUERY_FLAG(looker,FLAG_XRAYS)))
    return 0;

  return 1;
}

/* Waypoint fields:
 * slaying - destination map (for aggro waypoints: path destination map)
 * title   - name of next wp in chain
 * msg     - precomputed path
 * owner   - object that carries wp
 * ownercount - count of owner
 * x,y     - end location for path stored in msg
 * stats:
 *   hp, sp  - destination x and y
 *   ac      - wait timer (<= wc)
 *   wc      - wait time
 *   grace   - acceptable distance
 *   food    - current waypoint index in precomputed path
 *   dam     - closest distance to the (local) target
 *      
 * flags:
 *   cursed - active (only one active wp per mob!)
 *   paralyzed - path computation requested
 *   damned - set for aggro waypoints, clear for "normal"
 *   confused - set when we fail to find a path (npc is already on best tile)
 *              cleared whenever a path is found.
 */

/* Find a monster's currently active waypoint, if any */
object *get_active_waypoint(object *op) {
  object *wp = NULL;
  
  for(wp=op->inv;wp!=NULL;wp=wp->below)
    if(wp->type == TYPE_WAYPOINT_OBJECT && QUERY_FLAG(wp, FLAG_CURSED)) 
      break;
  
  return wp;
}

/* Find a monster's current aggro wp, if any */
object *get_aggro_waypoint(object *op) {
  object *wp = NULL;
  
  for(wp=op->inv;wp!=NULL;wp=wp->below)
    if(wp->type == TYPE_WAYPOINT_OBJECT && QUERY_FLAG(wp, FLAG_DAMNED)) 
      break;
  
  return wp;
}
                    
/* Find a monster's waypoint by name (used for getting the next waypoint) */
object *find_waypoint(object *op, const char *name) {
  object *wp = NULL;

  if(name == NULL)
      return NULL;
  
  for(wp=op->inv;wp!=NULL;wp=wp->below)
    if(wp->type == TYPE_WAYPOINT_OBJECT && strcmp(wp->name, name) == 0) 
      break;
  
  return wp;
}

/* Perform a path computation for the waypoint object 
 * This function is called whenever our path request is dequeued 
 */
void waypoint_compute_path(object *waypoint) {
    object *op = waypoint->env;
    mapstruct *destmap = op->map;
    path_node *path;
   
    /* Store final path destination (used by aggro wp) */
    if(QUERY_FLAG(waypoint, FLAG_DAMNED)) {
		if(waypoint->enemy)
		{
			FREE_AND_COPY_HASH(waypoint->slaying, waypoint->enemy->map->path);
			waypoint->x = waypoint->stats.hp = waypoint->enemy->x;
			waypoint->y = waypoint->stats.sp = waypoint->enemy->y;
		}
    }
    
    if(waypoint->slaying != NULL && *waypoint->slaying != '\0') {
        char temp_path[HUGE_BUF];
        /* TODO: handle unique maps? */
        destmap = ready_map_name(normalize_path(op->map->path, waypoint->slaying, temp_path), 0);
    }

    if(destmap == NULL) {
        LOG(llevBug,"BUG: waypoint_compute_path(): invalid destination map '%s'\n", waypoint->slaying);
        return;
    }
    
    path = compress_path(find_path(op, op->map, op->x, op->y, destmap, waypoint->stats.hp, waypoint->stats.sp));
    if(path) {        
        /* Skip the first path element (always the starting position) */
        path = path->next;
        if(! path) {
            SET_FLAG(waypoint, FLAG_CONFUSED);
            return;
        }
        
#ifdef DEBUG_PATHFINDING
        {
            path_node *tmp;
            LOG(llevDebug,"waypoint_compute_path(): '%s' new path -> '%s': ", op->name, waypoint->name);
            for(tmp = path; tmp; tmp = tmp->next) 
                LOG(llevDebug,"(%d,%d) ", tmp->x, tmp->y);
            LOG(llevDebug,"\n");
        }
#endif        

        if(waypoint->msg)
            FREE_AND_CLEAR_HASH(waypoint->msg);

        waypoint->msg = encode_path(path);
        waypoint->stats.food = 0; /* path offset */

        waypoint->stats.Str = 0;  /* number of fails */
        waypoint->stats.dam = 30000; /* best distance */
        CLEAR_FLAG(waypoint, FLAG_CONFUSED);
    } else 
        LOG(llevBug,"BUG: waypoint_compute_path(): no path to destination ('%s' -> '%s')\n", op->name, waypoint->name);
}

/* Move towards waypoint target */
void waypoint_move(object *op, object *waypoint) {
    mapstruct *destmap = op->map;
    rv_vector local_rv, global_rv, *dest_rv;
    int dir;
    sint16 new_offset = 0, success = 0;
    
    if(waypoint == NULL || op == NULL)
        return;

    /* Aggro or static waypoint? */
    if(QUERY_FLAG(waypoint, FLAG_DAMNED)) {
        /* Verify enemy */
        if(waypoint->enemy == op->enemy && waypoint->enemy_count == op->enemy_count &&
           OBJECT_VALID(waypoint->enemy, waypoint->enemy_count)) {
            destmap = waypoint->enemy->map;
            waypoint->stats.hp = waypoint->enemy->x;
            waypoint->stats.sp = waypoint->enemy->y;
        } else {
            /* owner has either switched or lost enemy. This should work for both cases
             * switched -> similar to if target moved 
             * lost -> we shouldn't be called again without new data
             */
            waypoint->enemy = op->enemy;
            waypoint->enemy_count = op->enemy_count;
            return;
        }
    } else {
        /* Find the destination map if specified in waypoint (otherwise use current map) */
        if(waypoint->slaying != NULL && *waypoint->slaying != '\0') {
            char temp_path[HUGE_BUF];
            /* TODO: handle unique maps? */
            destmap = ready_map_name(normalize_path(op->map->path, waypoint->slaying, temp_path), 0);
        }
    }

    if(destmap == NULL) {
        LOG(llevBug,"BUG: waypoint_move(): invalid destination map '%s' for '%s' -> '%s'\n", waypoint->slaying,
                op->name, waypoint->name);
        return;
    }
    
    if(! get_rangevector_from_mapcoords(op->map, op->x, op->y, destmap, waypoint->stats.hp, waypoint->stats.sp, &global_rv, 2|8)) 
        return;
    dest_rv = &global_rv;
  
    /* Reached the final destination? */
    if((int)global_rv.distance <= waypoint->stats.grace) {
        object *nextwp = NULL;            

        /* Just arrived? */
        if(waypoint->stats.ac == 0) {
#ifdef DEBUG_PATHFINDING
            LOG(llevDebug,"move_waypoint(): '%s' reached destination '%s'\n", op->name, waypoint->name);
#endif
            
#ifdef PLUGINS
            /* GROS: Handle for plugin trigger event */
            if(waypoint->event_flags&EVENT_FLAG_TRIGGER)
            {
                CFParm CFP;
                CFParm* CFR;
                int k, l, m;
                int rtn_script = 0;
                object *event_obj = get_event_object(waypoint, EVENT_TRIGGER);
                m = 0;

                k = EVENT_TRIGGER;
                l = SCRIPT_FIX_NOTHING;
                CFP.Value[0] = &k;
                CFP.Value[1] = op;
                CFP.Value[2] = waypoint;
                CFP.Value[3] = NULL;
                CFP.Value[4] = NULL;
                CFP.Value[5] = &m;
                CFP.Value[6] = &m;
                CFP.Value[7] = &m;
                CFP.Value[8] = &l;
                CFP.Value[9] = (char *)event_obj->race;
                CFP.Value[10]= (char *)event_obj->slaying;
                if (findPlugin(event_obj->name)>=0)
                {
                    CFR = (PlugList[findPlugin(event_obj->name)].eventfunc) (&CFP);
                    rtn_script = *(int *)(CFR->Value[0]);
                    if (rtn_script!=0) return;
                }
            }
#endif
        } /* Just arrived */
        
        /* Waiting at this WP? */
        /* TODO: use timers instead? */
        if(waypoint->stats.ac < waypoint->stats.wc) {
            waypoint->stats.ac++;
            return;
        }
        
        waypoint->stats.ac = 0;               /* clear timer */
        CLEAR_FLAG(waypoint, FLAG_CURSED);    /* set inactive */
        FREE_AND_CLEAR_HASH(waypoint->msg);   /* remove precomputed path */

        /* Start over with the new waypoint, if any*/
        if(!QUERY_FLAG(waypoint, FLAG_DAMNED)) {
            nextwp = find_waypoint(op, waypoint->title);
            if(nextwp) {
#ifdef DEBUG_PATHFINDING
                LOG(llevDebug,"waypoint_move(): '%s' next WP: '%s'\n", op->name, waypoint->title);
#endif
                SET_FLAG(nextwp, FLAG_CURSED);
                waypoint_move(op, get_active_waypoint(op));
            } else {
#ifdef DEBUG_PATHFINDING
                LOG(llevDebug,"waypoint_move(): '%s' no next WP\n", op->name);
#endif
            }
        }

        waypoint->enemy = NULL;

        return;
    } /* If we reached the waypoint destination */
 
    /*
     * Handle precomputed paths
     */

    /* If we finished our current path. Clear it so that we can get a new one. */
    if(waypoint->msg != NULL && (waypoint->msg[waypoint->stats.food] == '\0' || global_rv.distance <= 0)) 
        FREE_AND_CLEAR_HASH(waypoint->msg);
            
    /* Get new path if target has moved much since the path was created */
    if(QUERY_FLAG(waypoint, FLAG_DAMNED) && waypoint->msg != NULL &&
            (waypoint->stats.hp != waypoint->x || waypoint->stats.sp != waypoint->y)) {
        rv_vector rv;
        /* TODO: unique maps */
        mapstruct *path_destmap = ready_map_name(waypoint->slaying, 0);        
        get_rangevector_from_mapcoords(destmap, waypoint->stats.hp, waypoint->stats.sp, path_destmap, waypoint->x, waypoint->y, &rv, 8);

        if(rv.distance > 1 && rv.distance > global_rv.distance) {
#ifdef DEBUG_PATHFINDING
            LOG(llevDebug,"waypoint_move(): path_distance = %d for '%s' -> '%s'. Discarding old path.\n", rv.distance, op->name, op->enemy->name);
#endif            
            FREE_AND_CLEAR_HASH(waypoint->msg);
        }
    }

    /* Are we far enough from the target to require a path? */
    if(global_rv.distance > 1) {
        if(waypoint->msg == NULL) {
            /* Request a path if we don't have one */
            request_new_path(waypoint);
        } else {
            /* If we have precalculated path, take direction to next subwaypoint */
            int destx = waypoint->stats.hp, desty = waypoint->stats.sp;        
            new_offset = waypoint->stats.food;
           
            if(get_path_next(waypoint->msg, &new_offset, &destmap, &destx, &desty)) {
                get_rangevector_from_mapcoords(op->map, op->x, op->y, destmap, destx, desty, &local_rv, 2 | 8);
                dest_rv = &local_rv;
            } else {
                /* We seem to have an invalid path string. */
                LOG(llevBug,"BUG: waypoint_move(): invalid path string '%s' in '%s' -> '%s'\n", 
                        waypoint->msg, op->name, waypoint->name);
                FREE_AND_CLEAR_HASH(waypoint->msg);
                request_new_path(waypoint);
            }
        }
    } 
    
    /* Did we get closer to our goal last time? */
    if((int)dest_rv->distance < waypoint->stats.dam)  {
        waypoint->stats.dam = dest_rv->distance;
        waypoint->stats.Str = 0; /* Number of times we failed getting closer to (sub)goal */
    } else if(waypoint->stats.Str++ > 4) {
        /* Discard the current path, so that we can get a new one */
        FREE_AND_CLEAR_HASH(waypoint->msg);
    }
   
    if(global_rv.distance > 1 && waypoint->msg == NULL && QUERY_FLAG(waypoint, FLAG_CONFUSED)) {
#ifdef DEBUG_PATHFINDING        
        LOG(llevDebug,"waypoint_move(): no path found. '%s' standing still\n", op->name);
#endif        
        return;
    }
    
    /*
     * Perform the actual move 
     */
    
    dir = dest_rv->direction;    
   
    if(QUERY_FLAG(op,FLAG_CONFUSED))
        dir = absdir(dir + RANDOM()%3 + RANDOM()%3 - 2);

    if (dir && !QUERY_FLAG(op,FLAG_STAND_STILL)) {
        if(move_object(op,dir)) /* Can the monster move directly toward waypoint? */
            success = 1;
        else {
            int diff;
            /* Try move around corners otherwise */
            for(diff = 1; diff <= 2; diff++) {
                /* try different detours */
                int m = 1-(RANDOM()&2);          /* Try left or right first? */
                if(move_object(op,absdir(dir + diff*m)) ||
                        move_object(op,absdir(dir - diff*m))) {
                    success = 1;
                    break;
                }
            }
        }

        /* If we had a local destination and we got close enough to it, accept it... 
         * (re distance check: dest_rv->distance is distance before the step. 
         *  if the move was succesful, we got closer to the dest, otherwise
         *  we can accept a small distance from it)
         */
        if(dest_rv == &local_rv && dest_rv->distance == 1) {
            waypoint->stats.food = new_offset;
            waypoint->stats.Str = 0;  /* number of fails */
            waypoint->stats.dam = 30000; /* best distance */
        }
    } /* if monster is not standing still */    
}
    
/*
 * Move-monster returns 1 if the object has been freed, otherwise 0.
 */

int move_monster(object *op) {
    int dir, special_dir = 0, diff;
    object  *owner, *enemy, *part, *tmp;
    rv_vector	rv;
 
	if(op->head)
	{
		LOG(llevBug,"BUG: move_monster(): called from tail part. (%s -- %s)\n", query_name(op), op->arch->name);
		return 0;
	}
 
	/* Monsters not on maps don't do anything.  These monsters are things
     * Like royal guards in city dwellers inventories.
     */
    if (!op->map)
		return 0;

    CLEAR_FLAG(op,FLAG_PARALYZED); /* if we are here, we never paralyzed anymore */

    /* for target facing, we copy this value here for fast access */
    /* for some reason, rv is not set right for targeted enemy all times */
    /* so i call it here direct again */
    op->anim_enemy_dir = -1;
    op->anim_moving_dir = -1;

	/* Here is the heart of the mob attack & target area.
	 * find_enemy() checks the old enemy or get us a new one.
	 */	
	tmp = op->enemy;
    if (QUERY_FLAG(op, FLAG_NO_ATTACK)) /* we never ever attack */ 
    {
        if(op->enemy)
            set_npc_enemy(op, NULL, NULL);
        enemy = NULL;
    }
    else if((enemy= find_enemy(op, &rv)))
    {

        op->anim_enemy_dir = rv.direction;
        if(!enemy->attacked_by ||(enemy->attacked_by && enemy->attacked_by_distance >(int)rv.distance )) 
        {
        /* we have an enemy, just tell him we want him dead */
            enemy->attacked_by = op;       /* our ptr */
            enemy->attacked_by_count = op->count; /* our tag */
            enemy->attacked_by_distance = (sint16) rv.distance;  /* NOW the attacked foe knows how near we are */
        }
    }

    /*  generate hp, if applicable */
    if(op->stats.Con&&op->stats.hp<op->stats.maxhp) {

	/* last heal is in funny units.  Dividing by speed puts
	 * the regeneration rate on a basis of time instead of
	 * #moves the monster makes.  The scaling by 8 is
	 * to capture 8th's of a hp fraction regens 
	 */

	if(++op->last_heal>5)
	{
		op->last_heal = 0;
		op->stats.hp+=op->stats.Con;
	}
	/* So if the monster has gained enough HP that they are no longer afraid */
	if (QUERY_FLAG(op,FLAG_RUN_AWAY) &&
	    op->stats.hp >= (signed short)(((float)op->run_away/(float)100)*
                        (float)op->stats.maxhp))
	    CLEAR_FLAG(op, FLAG_RUN_AWAY);

	if(op->stats.hp>op->stats.maxhp)
	    op->stats.hp=op->stats.maxhp;
    }

    /* generate sp, if applicable */
    if(op->stats.Pow&&op->stats.sp<op->stats.maxsp) {

	/*  last_sp is in funny units.  Dividing by speed puts
         * the regeneration rate on a basis of time instead of
         * #moves the monster makes.  The scaling by 8 is
         * to capture 8th's of a sp fraction regens 
	 */

	 op->last_sp+= (int)((float)(8*op->stats.Pow)/FABS(op->speed));
	op->stats.sp+=op->last_sp/128;  /* causes Pow/16 sp/tick */
	op->last_sp%=128;
	if(op->stats.sp>op->stats.maxsp)
	    op->stats.sp=op->stats.maxsp;
    }

    if(QUERY_FLAG(op, FLAG_SCARED)&&!(RANDOM()%20))
	CLEAR_FLAG(op,FLAG_SCARED); /* Time to regain some "guts"... */


    if(QUERY_FLAG(op, FLAG_SLEEP)||QUERY_FLAG(op, FLAG_BLIND)
       ||((op->map->darkness>0)&&!QUERY_FLAG(op,FLAG_SEE_IN_DARK)
	  &&!QUERY_FLAG(op,FLAG_SEE_INVISIBLE))) {
	    if(!check_wakeup(op,enemy,&rv))
            return 0;
    }

    /* check if monster pops out of hidden spot */
    if(op->hide) do_hidden_move(op);

    if(op->pick_up)
	monster_check_pickup(op);

    /*
    if(op->will_apply)
	monster_apply_below(op);
	*/

    /* If we don't have an enemy, do special movement or the like */
    if(!enemy) {
        if(QUERY_FLAG(op, FLAG_ONLY_ATTACK)) 
        {
            remove_ob(op);
	        free_object(op);
            return 1;
	    }
        if(!QUERY_FLAG(op, FLAG_STAND_STILL)) 
        {
            
            if (op->move_type & HI4)
            {
                switch (op->move_type & HI4)
                {
	    	        case (PETMOVE):
		                pet_move (op);
		            break;
		            case (CIRCLE1):
		                circ1_move (op);
		            break;
		            case (CIRCLE2):
		                circ2_move (op);
		            break;
		            case (PACEV):
		                pace_movev(op);
		            break;
		            case (PACEH):
		                pace_moveh(op);
		            break;
		            case (PACEV2):
		                pace2_movev (op);
		            break;
		            case (PACEH2):
		                pace2_moveh (op);
		            break;
		            case (RANDO):
		                rand_move (op);
		            break;
		            case (RANDO2):
		                move_randomly (op);
		            break;
                            case (WPOINT):
                                waypoint_move(op, get_active_waypoint(op));
                            break; 
	            }
        
	            /*if(QUERY_FLAG(op, FLAG_FREED)) return 1; */ /* hm, when freed dont lets move him anymore */
	            return 0;
	        }
    	    else if (QUERY_FLAG(op,FLAG_RANDOM_MOVE))
                (void) move_randomly(op);

        } /* stand still */

        return 0;
    } /* no enemy */

    /* We have an enemy.  Block immediately below is for pets */
    if((op->type&HI4) == PETMOVE && (owner = get_owner(op)) != NULL && !on_same_map(op,owner))
    {
	    follow_owner(op, owner);
	    if(QUERY_FLAG(op, FLAG_REMOVED) && FABS(op->speed) > MIN_ACTIVE_SPEED)
        {
	        remove_friendly_object(op);
	        free_object(op);
	        return 1;
	    }
        return 0;
    }
   
    /* doppleganger code to change monster facing to that of the nearest player */
    /* Disabled this since 
     * 1) we don't have dopplegangers and 
     * 2) the strcmp() is a waste of CPU cycles. We should use a flag or something instead.
     * Gecko 2004-01-13    
    if ( (op->race != NULL)&& strcmp(op->race,"doppleganger") == 0)
    {
	    op->face = enemy->face; 
	    strcpy(op->name,enemy->name); * btw - if op->name uses hashed strings this is BAD *
    }
    */

    /* Move the check for scared up here - if the monster was scared,
     * we were not doing any of the logic below, so might as well save
     * a few cpu cycles.
     */
    if (!QUERY_FLAG(op, FLAG_SCARED))
    {
	    rv_vector   rv1;

        /* now we test every part of an object .... this is a real ugly piece of code */
	    for (part=op; part!=NULL; part=part->more)
        {
	        get_rangevector(part, enemy, &rv1, 0x1);	
	        dir=rv1.direction;

            /* hm, not sure about this part - in original was a scared flag here too
             * but that we test above... so can be old code here */
	        if(QUERY_FLAG(op,FLAG_RUN_AWAY))
		        dir=absdir(dir+4);
	        if(QUERY_FLAG(op,FLAG_CONFUSED))
		        dir = absdir(dir + RANDOM()%3 + RANDOM()%3 - 2);

			if(op->stats.Dex && !(RANDOM()%op->stats.Dex))
			{
				if(QUERY_FLAG(op,FLAG_CAST_SPELL))
				{
					if(monster_cast_spell(op,part,enemy,dir,&rv1))
						return 0;
				}
				if(QUERY_FLAG(op,FLAG_READY_RANGE)&&!(RANDOM()%3))
				{
				    if(monster_use_wand(op,part,enemy,dir))
				        return 0;
				}
				if(QUERY_FLAG(op,FLAG_READY_RANGE)&&!(RANDOM()%4))
				{
					if(monster_use_rod(op,part,enemy,dir))
				        return 0;
				}
				if(QUERY_FLAG(op,FLAG_READY_RANGE)&&!(RANDOM()%5))
				{
				    if(monster_use_horn(op,part,enemy,dir))
				        return 0;
				}
				if(QUERY_FLAG(op,FLAG_READY_SKILL)&&!(RANDOM()%3))
				{	/*
				    if(monster_use_skill(op,part,enemy,dir))
				        return 0;
					*/
					/* allow skill use AND melee attack! */
					monster_use_skill(op,part,enemy,dir);
				}
				if(QUERY_FLAG(op,FLAG_READY_BOW)&&!(RANDOM()%4))
				{
				    if(monster_use_bow(op,part,enemy,dir) &&!(RANDOM()%2))
				        return 0;
				}
			}
	    } /* for processing of all parts */        
    } /* If not scared */

    get_rangevector(op, enemy, &rv, 0); /* TODO: haven't we already done this in check_enemy? */
    part = rv.part;
    dir=rv.direction; /* dir is now direction towards enemy */

    if(QUERY_FLAG(op, FLAG_SCARED) || QUERY_FLAG(op,FLAG_RUN_AWAY))        
    	dir=absdir(dir+4);

    if(QUERY_FLAG(op,FLAG_CONFUSED))
	dir = absdir(dir + RANDOM()%3 + RANDOM()%3 - 2);

    if(!QUERY_FLAG(op, FLAG_SCARED)) {
        if (op->move_type & LO4)
        {        
            switch (op->move_type & LO4)
            {
                case DISTATT:
                    special_dir = dist_att (dir,op,enemy,part,&rv);
                    break;
                case RUNATT:
                    special_dir = run_att (dir,op,enemy,part,&rv);
                    break;
                case HITRUN:
                    special_dir = hitrun_att(dir,op,enemy);
                    break;
                case WAITATT:
                    special_dir = wait_att (dir,op,enemy,part,&rv);
                    break;
                case RUSH: /* why is here non ? */
                case ALLRUN:
                    break; 
                case DISTHIT:
                    special_dir = disthit_att (dir,op,enemy,part,&rv);
                    break;
                case WAIT2:
                    special_dir = wait_att2 (dir,op,enemy,part,&rv);
                    break;
                default:
                    LOG(llevDebug,"Illegal low mon-move: %d\n",op->move_type & LO4);
            }
            if(! special_dir)
                return 0;
        }
    }
    
    /* try to move closer to enemy, or follow whatever special attack behaviour is */
    if (!QUERY_FLAG(op,FLAG_STAND_STILL) && 
            (QUERY_FLAG(op, FLAG_SCARED) || QUERY_FLAG(op,FLAG_RUN_AWAY) || 
             !can_hit(part,enemy,&rv) || ((op->move_type & LO4) && special_dir != dir))) 
    { 
        object *aggro_wp = get_aggro_waypoint(op);

        /* TODO: make (intelligent) monsters go to last known position of enemy if
         *       out of range/sight */

        /* If special attack move -> follow it instead of going towards enemy */           
        if(((op->move_type & LO4) && special_dir != dir)) {
            aggro_wp = NULL;
            dir = special_dir;                    
        }

        /* If valid aggro wp (and no special attack), and not scared, use it for movement */
        if(aggro_wp && aggro_wp->enemy && aggro_wp->enemy == op->enemy && rv.distance > 1 &&
                !QUERY_FLAG(op, FLAG_SCARED) && !QUERY_FLAG(op, FLAG_RUN_AWAY)) {
            waypoint_move(op, aggro_wp);
            return 0;                
        } else {
            int maxdiff = (QUERY_FLAG(op, FLAG_ONLY_ATTACK) || RANDOM()&1) ? 1 : 2;

            if(move_object(op,dir)) /* Can the monster move directly toward player? */
                return 0;

            /* Try move around corners if !close */
            for(diff = 1; diff <= maxdiff; diff++) {
                /* try different detours */
                int m = 1-(RANDOM()&2);          /* Try left or right first? */
                if(move_object(op,absdir(dir + diff*m)) ||
                        move_object(op,absdir(dir - diff*m)))
                    return 0;
            }
        }
    } /* if monster is not standing still */

    /*
     * Eneq(@csd.uu.se): Patch to make RUN_AWAY or SCARED monsters move a random
     * direction if they can't move away.
     */
    if (!QUERY_FLAG(op, FLAG_ONLY_ATTACK)&&(QUERY_FLAG(op,FLAG_RUN_AWAY)||QUERY_FLAG(op, FLAG_SCARED)))
	if(move_randomly(op))
	    return 0;

    /* Hit enemy if possible */
    if(!QUERY_FLAG(op, FLAG_SCARED)&&can_hit(part,enemy,&rv))
    {
        if(QUERY_FLAG(op,FLAG_RUN_AWAY))
        {
	       
			part->stats.wc-=10;
            if(op->weapon_speed_left<=0) /* as long we are >0, we are not ready to swing */                
            {
    	        (void)skill_attack(enemy,part,0,NULL);
                op->weapon_speed_left+=FABS((int)op->weapon_speed_left)+1;
            }
			part->stats.wc+=10;
	    }
        else
        {
            if(op->weapon_speed_left<=0) /* as long we are >0, we are not ready to swing */                
            {
                (void)skill_attack(enemy,part,0,NULL);
                op->weapon_speed_left+=FABS((int)op->weapon_speed_left)+1;
            }
        }
    } /* if monster is in attack range */

    if(QUERY_FLAG(part,FLAG_FREED))    /* Might be freed by ghost-attack or hit-back */
    	return 1;
    if(QUERY_FLAG(op, FLAG_ONLY_ATTACK))
    {
	    remove_ob(op);
	    free_object(op);
	    return 1;
    }
    return 0;
}

/* Returns the nearest living creature (monster or generator).
 * Modified to deal with tiled maps properly.
 * Also fixed logic so that monsters in the lower directions were more
 * likely to be skipped - instead of just skipping the 'start' number
 * of direction, revisit them after looking at all the other spaces.
 *
 * Note that being this may skip some number of spaces, it will
 * not necessarily find the nearest living creature - it basically
 * chooses one from within a 3 space radius, and since it skips
 * the first few directions, it could very well choose something 
 * 3 spaces away even though something directly north is closer.
 *
 * this function is map tile aware.
 * i much more prefer a "monster list" chained to the maps - like the
 * friendly list will in future.
 */
object *find_nearest_living_creature(object *npc) {
    int i,j=0,start;
    int nx,ny, friendly_attack = TRUE;
    mapstruct *m;
    object *tmp;

	/* must add pet check here too soon */
	/* friendly non berserk unit only attack mobs - not other friendly or players */
	if(!QUERY_FLAG(npc,FLAG_BERSERK) &&QUERY_FLAG(npc,FLAG_FRIENDLY))
		friendly_attack = FALSE;

    start = (RANDOM()%8)+1;
    for(i=start;j<SIZEOFFREE;j++, i=(i+1)%SIZEOFFREE)
	{
		nx = npc->x + freearr_x[i];
		ny = npc->y + freearr_y[i];
		if (!(m=out_of_map(npc->map,&nx,&ny))) 
			continue;

		/* quick check - if nothing alive or player skip test for targets */
		if (!(GET_MAP_FLAGS(m, nx, ny) & (P_IS_ALIVE|P_IS_PLAYER)))
			continue;

	    tmp=get_map_ob(m,nx,ny);

		if(friendly_attack) /* attack player & friendly */
		{
			/* attack all - monster, player & friendly - loop more is not monster AND not player */
		    while(tmp!=NULL && !QUERY_FLAG(tmp,FLAG_MONSTER) && tmp->type!=PLAYER )
                tmp=tmp->above;

		}
		else
		{
			/* loop on when not monster or player or friendly */
		    while(tmp!=NULL && (!QUERY_FLAG(tmp,FLAG_MONSTER) || tmp->type==PLAYER || (QUERY_FLAG(tmp,FLAG_FRIENDLY))) )
                tmp=tmp->above;
		}

		if(tmp && can_see_monsterP(m,nx,ny,i))
		    return tmp;
    }
    return NULL;  /* nothing found */
}


int move_randomly(object *op) {
    int i;

    /* Give up to 15 chances for a monster to move randomly */
    for(i=0;i<15;i++) {
	if(move_object(op,RANDOM()%8+1))
	    return 1;
    }
    return 0;
}

int can_hit(object *ob1,object *ob2, rv_vector *rv) {
    if(QUERY_FLAG(ob1,FLAG_CONFUSED)&&!(RANDOM()%3))
	return 0;
    return abs(rv->distance_x)<2&&abs(rv->distance_y)<2;
}

/*Someday we may need this check */
int can_apply(object *who,object *item) {
  return 1;
}

#define MAX_KNOWN_SPELLS 20

object *monster_choose_random_spell(object *monster) {
  object *altern[MAX_KNOWN_SPELLS];
  object *tmp;
  spell *spell;
  int i=0,j;

  for(tmp=monster->inv;tmp!=NULL;tmp=tmp->below)
    if(tmp->type==ABILITY||tmp->type==SPELLBOOK) {
	 /*  Check and see if it's actually a useful spell */
		if((spell=find_spell(tmp->stats.sp))!=NULL 
			&&!(spell->path&(PATH_INFO|PATH_TRANSMUTE|PATH_TRANSFER|PATH_LIGHT))) {
			 if(tmp->stats.maxsp)
				for(j=0;i<MAX_KNOWN_SPELLS&&j<tmp->stats.maxsp;j++)
				  altern[i++]=tmp;
			 else
				altern[i++]=tmp;
			 if(i==MAX_KNOWN_SPELLS)
				break;
		}
	
	 }
  if(!i)
    return NULL;
  return altern[RANDOM()%i];
}

int monster_cast_spell(object *head, object *part,object *pl,int dir, rv_vector *rv) {
    object *spell_item;
    spell *sp;
    int sp_typ, ability;
   /* object *owner;
    rv_vector	rv1;*/

	/*LOG(-1,"CAST: dir:%d (%d)- target:%s\n", dir, rv->direction, query_name(head->enemy) );*/
    /* TODO: Remove this here - this should not decided here! */
	/*
    if(!(RANDOM()%3)) 
    	return 0;
	*/
    
    /* If you want monsters to cast spells over friends, this spell should
     * be removed.  It probably should be in most cases, since monsters still
     * don't care about residual effects (ie, casting a cone which may have a 
     * clear path to the player, the side aspects of the code will still hit
     * other monsters)
     */
	/*
    if(!(dir=path_to_player(part,pl,0)))
        return 0;
    */
	/* Might hit owner with spell - well we don't care anymore - we will handle this in attack.c *
	*
    if(QUERY_FLAG(head,FLAG_FRIENDLY) && (owner = get_owner(head)) != NULL)
    {
	    get_rangevector(head, owner, &rv1, 0x1);
	    if(dirdiff(dir,rv1.direction) < 2)
        {
	        return 0; /
        }
    }
	*/
    if(QUERY_FLAG(head,FLAG_CONFUSED))
		dir = absdir(dir + RANDOM()%3 + RANDOM()%3 - 2);

	if((spell_item=monster_choose_random_spell(head))==NULL) 
	{
		LOG(llevDebug,"DEBUG: monster_cast_spell: No spell found! Turned off spells in %s (%s) (%d,%d)\n",
			query_name(head),head->map?(head->map->name?head->map->name:"<no map name>"):"<no map!>", head->x, head->y );
		CLEAR_FLAG(head, FLAG_CAST_SPELL); /* Will be turned on when picking up book */
		return 0;
	}

    if(spell_item->stats.hp)
	{
		/* Alternate long-range spell: check how far away enemy is */
		if(rv->distance>6)
			sp_typ=spell_item->stats.hp;
		else
			sp_typ=spell_item->stats.sp;
    } 
	else
		sp_typ=spell_item->stats.sp;

    if((sp=find_spell(sp_typ))==NULL)
	{
		LOG(llevDebug,"DEBUG: monster_cast_spell: Can't find spell #%d for mob %s (%s) (%d,%d)\n", sp_typ,
			query_name(head),head->map?(head->map->name?head->map->name:"<no map name>"):"<no map!>", head->x, head->y );
		return 0;
    }

    if (sp->flags&SPELL_DESC_SELF) /* Spell should be cast on caster (ie, heal, strength) */
		dir = 0;
  
    /* Monster doesn't have enough spell-points */
    if(head->stats.sp<SP_level_spellpoint_cost(head,head,sp_typ))
	    return 0;

	/* Note: in cf is possible to give the mob a spellbook - this will be used
	 * as "spell source" (aka ability object like) too. I will remove this - 
	 * using special prepared stuff like this is more useful.
	 * Also i noticed tha "long range stuff" - can this be handled from 
	 * spellbooks too???
	 */
    ability = (spell_item->type==ABILITY && QUERY_FLAG(spell_item,FLAG_IS_MAGICAL) );

    /* If we cast a spell, only use up casting_time speed */
    head->speed_left+=(float)1.0 - (float) spells[sp_typ].time/(float)20.0*(float)FABS(head->speed); 

    head->stats.sp-=SP_level_spellpoint_cost(head,head,sp_typ);
    
	/*LOG(-1,"CAST2: dir:%d (%d)- target:%s\n", dir, rv->direction, query_name(head->enemy) );*/
    return cast_spell(part,part,dir,sp_typ,ability, spellNormal,NULL);
}

/* monster_use_skill()-implemented 95-04-28 to allow monster skill use.
 * Note that monsters do not need the skills SK_MELEE_WEAPON and
 * SK_MISSILE_WEAPON to make those respective attacks, if we
 * required that we would drastically increase the memory
 * requirements of CF!! 
 *
 * The skills we are treating here are all but those. -b.t. 
 *
 * At the moment this is only useful for throwing, perhaps for
 * stealing. TODO: This should be more integrated in the game. -MT, 25.11.01
 */  

int monster_use_skill(object *head, object *part, object *pl,int dir) {
object *skill, *owner;

  if(!(dir=path_to_player(part,pl,0)))
    return 0;
  if(QUERY_FLAG(head,FLAG_FRIENDLY) && (owner = get_owner(head)) != NULL) {
    int dir2 = find_dir_2(head->x-owner->x, head->y-owner->y);
    if(dirdiff(dir,dir2) < 1)
      return 0; /* Might hit owner with skill -thrown rocks for example ?*/
  }
  if(QUERY_FLAG(head,FLAG_CONFUSED))
    dir = absdir(dir + RANDOM()%3 + RANDOM()%3 - 2);

  /* skill selection - monster will use the next unused skill.
   * well...the following scenario will allow the monster to 
   * toggle between 2 skills. One day it would be nice to make
   * more skills available to monsters.  
   */
 
  for(skill=head->inv;skill!=NULL;skill=skill->below)
    if(skill->type==SKILL && skill!=head->chosen_skill) { 
        head->chosen_skill=skill; 
        break;
    }

  if(!skill && !head->chosen_skill) {
    LOG(llevDebug,"Error: Monster %s (%d) has FLAG_READY_SKILL without skill.\n",
        head->name,head->count);
    CLEAR_FLAG(head, FLAG_READY_SKILL);
    return 0;
  }

/* use skill */
  return do_skill(head,dir,NULL);
}


/* For the future: Move this function together with case 3: in fire() */

int monster_use_wand(object *head,object *part,object *pl,int dir) {
  object *wand, *owner;
  if(!(dir=path_to_player(part,pl,0)))
    return 0;
  if(QUERY_FLAG(head,FLAG_FRIENDLY) && (owner = get_owner(head)) != NULL) {
    int dir2 = find_dir_2(head->x-owner->x, head->y-owner->y);
    if(dirdiff(dir,dir2) < 2)
      return 0; /* Might hit owner with spell */
  }
  if(QUERY_FLAG(head,FLAG_CONFUSED))
    dir = absdir(dir + RANDOM()%3 + RANDOM()%3 - 2);
  for(wand=head->inv;wand!=NULL;wand=wand->below)
    if(wand->type==WAND&&QUERY_FLAG(wand,FLAG_APPLIED))
      break;
  if(wand==NULL) {
    LOG(llevBug,"BUG: Monster %s (%d) HAS_READY_WAND() without wand.\n",query_name(head),head->count);
    CLEAR_FLAG(head, FLAG_READY_RANGE);
    return 0;
  }
  if(wand->stats.food<=0) {
    manual_apply(head,wand,0);
    CLEAR_FLAG(head, FLAG_READY_RANGE);
    if (wand->arch) {
      CLEAR_FLAG(wand, FLAG_ANIMATE);
      wand->face = wand->arch->clone.face;
      wand->speed = 0;
      update_ob_speed(wand);
    }
    return 0;
  }
  if(cast_spell(part,wand,dir,wand->stats.sp,0,spellWand,NULL)) {
    wand->stats.food--;
    return 1;
  }
  return 0;
}

int monster_use_rod(object *head,object *part,object *pl,int dir) {
  object *rod, *owner;
  if(!(dir=path_to_player(part,pl,0)))
    return 0;
  if(QUERY_FLAG(head,FLAG_FRIENDLY) && (owner = get_owner(head)) != NULL) {
    int dir2 = find_dir_2(head->x-owner->x, head->y-owner->y);
    if(dirdiff(dir,dir2) < 2)
      return 0; /* Might hit owner with spell */
  }
  if(QUERY_FLAG(head,FLAG_CONFUSED))
    dir = absdir(dir + RANDOM()%3 + RANDOM()%3 - 2);
  for(rod=head->inv;rod!=NULL;rod=rod->below)
    if(rod->type==ROD&&QUERY_FLAG(rod,FLAG_APPLIED))
      break;
  if(rod==NULL) {
    LOG(llevBug,"BUG: Monster %s (%d) HAS_READY_ROD() without rod.\n",query_name(head),head->count);
    CLEAR_FLAG(head, FLAG_READY_RANGE);
    return 0;
  }
  if(rod->stats.hp<spells[rod->stats.sp].sp) {
    return 0; /* Not recharged enough yet */
  }
  if(cast_spell(part,rod,dir,rod->stats.sp,0,spellRod,NULL)) {
    drain_rod_charge(rod);
    return 1;
  }
  return 0;
}

int monster_use_horn(object *head,object *part,object *pl,int dir) {
  object *horn, *owner;
  if(!(dir=path_to_player(part,pl,0)))
    return 0;
  if(QUERY_FLAG(head,FLAG_FRIENDLY) && (owner = get_owner(head)) != NULL) {
    int dir2 = find_dir_2(head->x-owner->x, head->y-owner->y);
    if(dirdiff(dir,dir2) < 2)
      return 0; /* Might hit owner with spell */
  }
  if(QUERY_FLAG(head,FLAG_CONFUSED))
    dir = absdir(dir + RANDOM()%3 + RANDOM()%3 - 2);
  for(horn=head->inv;horn!=NULL;horn=horn->below)
    if(horn->type==ROD&&QUERY_FLAG(horn,FLAG_APPLIED))
      break;
  if(horn==NULL) {
    LOG(llevBug,"BUG: Monster %s (%d) HAS_READY_HORN() without horn.\n",query_name(head),head->count);
    CLEAR_FLAG(head, FLAG_READY_RANGE);
    return 0;
  }
  if(horn->stats.hp<spells[horn->stats.sp].sp) {
    return 0; /* Not recharged enough yet */
  }
  if(cast_spell(part,horn,dir,horn->stats.sp,0,spellHorn,NULL)) {
    drain_rod_charge(horn);
    return 1;
  }
  return 0;
}

int monster_use_bow(object *head, object *part, object *pl, int dir) {
  object *bow, *arrow;
  /*object *owner;*/
  int tag;

  /* this can be interesting in the future for some spells.
  if(!(dir=path_to_player(part,pl,0)))
    return 0;

  if(QUERY_FLAG(head,FLAG_FRIENDLY) && (owner = get_owner(head)) != NULL) {
    int dir2 = find_dir_2(head->x-owner->x, head->y-owner->y);
    if(dirdiff(dir,dir2) < 1)
      return 0; 
  }
  */

	if(QUERY_FLAG(head,FLAG_CONFUSED))
		dir = absdir(dir + RANDOM()%3 + RANDOM()%3 - 2);

  for(bow=head->inv;bow!=NULL;bow=bow->below)
    if(bow->type==BOW&&QUERY_FLAG(bow,FLAG_APPLIED))
      break;
  if(bow==NULL) {
    LOG(llevBug,"BUG: Monster %s (%d) HAS_READY_BOW() without bow.\n",query_name(head),head->count);
    CLEAR_FLAG(head, FLAG_READY_BOW);
    return 0;
  }
  if((arrow=find_arrow(head,bow->race)) == NULL) {
    /* Out of arrows */
    manual_apply(head,bow,0);
    CLEAR_FLAG(head, FLAG_READY_BOW);
    return 0;
  }
  /* thats a infinitve arrow! dupe it. */
  if(QUERY_FLAG(arrow,FLAG_SYS_OBJECT) )
  {
		object *new_arrow = get_object();
		copy_object(arrow,new_arrow);
		CLEAR_FLAG(new_arrow,FLAG_SYS_OBJECT);
		new_arrow->nrof=0;

		/* now setup the self destruction */
		new_arrow->stats.food = 20;
		arrow = new_arrow;
  }
  else
	  arrow=get_split_ob(arrow,1);

  set_owner(arrow,head);
  arrow->direction=dir;
  arrow->x=part->x,arrow->y=part->y;
  arrow->speed = 1;
  update_ob_speed(arrow);
  arrow->speed_left=0;
  SET_ANIMATION(arrow, (NUM_ANIMATIONS(arrow)/NUM_FACINGS(arrow))*dir);
  arrow->level = head->level;
  arrow->last_heal = arrow->stats.wc; /* save original wc and dam */
  arrow->stats.hp = arrow->stats.dam; 
  arrow->stats.dam+= bow->stats.dam+bow->magic+arrow->magic; /* NO_STRENGTH */
  arrow->stats.dam=FABS((int)((float)(arrow->stats.dam  *lev_damage[head->level])));
  arrow->stats.wc= 10 + (bow->magic +bow->stats.wc + arrow->magic + arrow->stats.wc-head->level);
  arrow->stats.wc_range = bow->stats.wc_range;
  arrow->map=head->map;
  arrow->last_sp = 12; /* we use fixed value for mobs */
  SET_FLAG(arrow, FLAG_FLYING);
  SET_FLAG(arrow, FLAG_FLY_ON);
  SET_FLAG(arrow, FLAG_WALK_ON);
  tag = arrow->count;
  insert_ob_in_map(arrow,head->map,head,0);
  play_sound_map(arrow->map, arrow->x, arrow->y,SOUND_THROW, SOUND_NORMAL);
  if (!was_destroyed(arrow, tag))
    move_arrow(arrow);
  return 1;
}

int check_good_weapon(object *who, object *item) {
  object *other_weap;
  int prev_dam=who->stats.dam;
  for(other_weap=who->inv;other_weap!=NULL;other_weap=other_weap->below)
    if(other_weap->type==item->type&&QUERY_FLAG(other_weap,FLAG_APPLIED))
      break;
  if(other_weap==NULL) /* No other weapons */
    return 1;
  if (monster_apply_special(who,item,0)) {
    LOG(llevMonster,"Can't wield %s(%d).\n",item->name,item->count);
    return 0;
  }
  if(who->stats.dam < prev_dam && !QUERY_FLAG(other_weap,FLAG_FREED)) {
    /* New weapon was worse.  (Note ^: Could have been freed by merging) */
    if (monster_apply_special(who,other_weap,0))
      LOG(llevMonster,"Can't rewield %s(%d).\n",item->name,item->count);
    return 0;
  }
  return 1;
}

int check_good_armour(object *who, object *item) {
  object *other_armour;
  int prev_ac = who->stats.ac;
  for (other_armour = who->inv; other_armour != NULL;
       other_armour = other_armour->below)
    if (other_armour->type == item->type && QUERY_FLAG(other_armour,FLAG_APPLIED))
      break;
  if (other_armour == NULL) /* No other armour, use the new */
    return 1;
  if (monster_apply_special(who, item,0)) {
    LOG(llevMonster, "Can't take off %s(%d).\n",item->name,item->count);
    return 0;
  }
  if(who->stats.ac > prev_ac && !QUERY_FLAG(other_armour,FLAG_FREED)) {
    /* New armour was worse. *Note ^: Could have been freed by merging) */
    if (monster_apply_special(who, other_armour,0))
      LOG(llevMonster,"Can't rewear %s(%d).\n", item->name, item->count);
    return 0;
  }
  return 1;
}

/*
 * monster_check_pickup(): checks for items that monster can pick up.
 *
 * Vick's (vick@bern.docs.uu.se) fix 921030 for the sweeper blob.
 * Each time the blob passes over some treasure, it will
 * grab it a.s.a.p.
 *
 * Eneq(@csd.uu.se): This can now be defined in the archetypes, added code
 * to handle this.
 *
 * This function was seen be continueing looping at one point (tmp->below
 * became a recursive loop.  It may be better to call monster_check_apply
 * after we pick everything up, since that function may call others which
 * affect stacking on this space.
 */

void monster_check_pickup(object *monster) {
  object *tmp,*next;
  int next_tag = 0;

  for(tmp=monster->below;tmp!=NULL;tmp=next) {
    next=tmp->below;
    if (next) next_tag = next->count;
    if (monster_can_pick(monster,tmp)) {
      remove_ob(tmp);
      tmp = insert_ob_in_ob(tmp,monster);

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
        CFP.Value[1] = monster; 
        CFP.Value[2] = tmp; 
        CFP.Value[3] = monster; /* No container...*/
        CFP.Value[4] = NULL;
        CFP.Value[5] = &tmp->nrof; /* nr of objects */
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
      
      (void) monster_check_apply(monster,tmp);
    }
    /* We could try to re-establish the cycling, of the space, but probably
     * not a big deal to just bail out.
     */
    if (next && was_destroyed(next, next_tag)) return;
  }
}

/*
 * monster_can_pick(): If the monster is interested in picking up
 * the item, then return 0.  Otherwise 0.
 * Instead of pick_up, flags for "greed", etc, should be used.
 * I've already utilized flags for bows, wands, rings, etc, etc. -Frank.
 */

int monster_can_pick(object *monster, object *item) {
  int flag=0;
  if(!can_pick(monster,item))
    return 0;
  if(QUERY_FLAG(item,FLAG_UNPAID))
    return 0;
  if (monster->pick_up&64)           /* All */
    flag=1;
  else switch(item->type) {
  case MONEY:
  case GEM:
  case TYPE_JEWEL:
  case TYPE_NUGGET:
    flag=monster->pick_up&2;
    break;
  case FOOD:
    flag=monster->pick_up&4;
    break;
  case WEAPON:
    flag=(monster->pick_up&8)||QUERY_FLAG(monster,FLAG_USE_WEAPON);
    break;
  case ARMOUR:
  case SHIELD:
  case HELMET:
  case BOOTS:
  case GLOVES:
  case GIRDLE:
    flag=(monster->pick_up&16)||QUERY_FLAG(monster,FLAG_USE_ARMOUR);
    break;
  case SKILL:
    flag=QUERY_FLAG(monster,FLAG_CAN_USE_SKILL);
    break;
  case RING:
    flag=QUERY_FLAG(monster,FLAG_USE_RING);
    break;
  case WAND:
    flag=QUERY_FLAG(monster,FLAG_USE_RANGE);
    break;
  case SPELLBOOK:
    flag=(monster->arch!=NULL&&QUERY_FLAG((&monster->arch->clone),FLAG_CAST_SPELL));
    break;
  case BOW:
  case ARROW:
    flag=QUERY_FLAG(monster,FLAG_USE_BOW);
    break;
  }
  if (((!(monster->pick_up&32))&&flag) || ((monster->pick_up&32)&&(!flag)))
    return 1;
  return 0;
}

/*
 * monster_apply_below():
 * Vick's (vick@bern.docs.uu.se) @921107 -> If a monster who's
 * eager to apply things, encounters something apply-able,
 * then make him apply it
 */

void monster_apply_below(object *monster) {
  object *tmp, *next;

  for(tmp=monster->below;tmp!=NULL;tmp=next) {
    next=tmp->below;
    switch (tmp->type) {
    case CF_HANDLE:
    case TRIGGER:
      if (monster->will_apply&1)
        manual_apply(monster,tmp,0);
      break;
    case TREASURE:
      if (monster->will_apply&2)
        manual_apply(monster,tmp,0);
      break;
    case SCROLL:  /* Ideally, they should wait until they meet a player */
      if (QUERY_FLAG(monster,FLAG_USE_SCROLL))
        manual_apply(monster,tmp,0); 
      break;
    }
    if (QUERY_FLAG (tmp, FLAG_IS_FLOOR))
        break;
  }
}

/*
 * monster_check_apply() is meant to be called after an item is
 * inserted in a monster.
 * If an item becomes outdated (monster found a better item),
 * a pointer to that object is returned, so it can be dropped.
 * (so that other monsters can pick it up and use it)
 */

/* Sept 96, fixed this so skills will be readied -b.t.*/

/* scary function - need rework. even in crossfire its changed now */
void monster_check_apply(object *mon, object *item) {

  /* and because its scary, we stop here... */
  /* this function is simply to bad - for example will potions applied
   * not depending on the situation... why applying a heal potion when
   * full hp? firestorm potion when standing next to own people?
   * IF we do some AI stuff here like using items we must FIRST
   * add a AI - then doing the things. Think first, act later!
   */
  if(1)
	  return;
  if(item->type==SPELLBOOK&&
     mon->arch!=NULL&&(QUERY_FLAG((&mon->arch->clone),FLAG_CAST_SPELL))) {
    SET_FLAG(mon, FLAG_CAST_SPELL);
    return;
  }
  if(QUERY_FLAG(mon,FLAG_USE_BOW) && item->type==ARROW)
  { /* Check for the right kind of bow */
    object *bow;
    for(bow=mon->inv;bow!=NULL;bow=bow->below)
      if(bow->type==BOW && bow->race==item->race) {
        SET_FLAG(mon, FLAG_READY_BOW);
        LOG(llevMonster,"Found correct bow for arrows.\n");
        if(!QUERY_FLAG(bow, FLAG_APPLIED))
          manual_apply(mon,bow,0);
        break;
      }
  }

/* hm, this should all handled in can_apply... need rework later MT-11-2002 */
  if (can_apply(mon,item)) {
    int flag=0;
    switch(item->type) {
    case TREASURE:
      flag=0;
    break;
    case POTION:
      flag=0;
      break;
    case FOOD: /* Can a monster eat food ?  Yes! (it heals) */
      flag=0;
      break;
    case WEAPON:
/*
 * Apply only if it's a better weapon than the used one.
 * All "standard" monsters need to adjust their wc to use the can_apply on
 * weapons.
 */
      flag=QUERY_FLAG(mon,FLAG_USE_WEAPON)&& check_good_weapon(mon,item);
      break;
    case ARMOUR:
    case HELMET:
    case SHIELD:
      flag=(QUERY_FLAG(mon,FLAG_USE_ARMOUR)&&
            check_good_armour(mon,item));
      break;
    case SKILL:
      if((flag=QUERY_FLAG(mon,FLAG_CAN_USE_SKILL))) {
        if(!QUERY_FLAG(item,FLAG_APPLIED)) manual_apply(mon,item,0);
        if (item->type==SKILL&&present_in_ob(SKILL,mon)!=NULL)
	  SET_FLAG(mon, FLAG_READY_SKILL);
      }
      break;
    case RING:
      flag=QUERY_FLAG(mon,FLAG_USE_RING);
      break;
    case WAND:
      flag=QUERY_FLAG(mon,FLAG_USE_RANGE);
      break;
    case BOW:
      flag=QUERY_FLAG(mon,FLAG_USE_BOW);
    }

    if (flag) {
        if(!QUERY_FLAG(item,FLAG_APPLIED))
          manual_apply(mon,item,0);
        if (item->type==BOW&&present_in_ob((unsigned char) item->stats.maxsp,mon)!=NULL)
	  SET_FLAG(mon, FLAG_READY_BOW);
    }
    return;
#if 0
    if(!QUERY_FLAG(item,FLAG_APPLIED))
      return item;
    {
      object *tmp;
      for(tmp=mon->inv;tmp!=NULL;tmp=tmp->below)
        if(tmp!=item&&tmp->type==item->type)
          return tmp;
    }
#endif
  }
  return;
}

void npc_call_help(object *op) {
  int x,y,xt,yt;
  mapstruct *m;
  object *npc;

  for(x = -3; x < 4; x++)
    for(y = -3; y < 4; y++) {
		xt=op->x+x;
		yt=op->y+y;
      if(!(m=out_of_map(op->map,&xt,&yt)))
        continue;
      for(npc = get_map_ob(m,xt,yt);npc!=NULL;npc=npc->above)
        if(QUERY_FLAG(npc, FLAG_ALIVE)&&QUERY_FLAG(npc, FLAG_UNAGGRESSIVE))
            set_npc_enemy(npc, op->enemy, NULL);
    }
}


int dist_att (int dir , object *ob, object *enemy, object *part, rv_vector *rv) {

    if (can_hit(part,enemy,rv))
    	return dir;
    if (rv->distance < 10)
    	return absdir(dir+4);
    else if (rv->distance>18)
    	return dir;
    return 0;
}

int run_att (int dir, object *ob, object *enemy,object *part, rv_vector *rv) {

    if ((can_hit (part,enemy,rv) && ob->move_status <20) || ob->move_status <20) {
	ob->move_status++;
	return (dir);
    }
    else if (ob->move_status >20)
	ob->move_status = 0;
    return absdir (dir+4);
}

int hitrun_att (int dir, object *ob,object *enemy) {
    if (ob->move_status ++ < 25)  
	return dir;
    else if (ob->move_status <50) 
	return absdir (dir+4); 
    else 
	ob->move_status = 0;
    return absdir(dir+4);
}

int wait_att (int dir, object *ob,object *enemy,object *part,rv_vector *rv) {

    int inrange = can_hit (part, enemy,rv);
      
    if (ob->move_status || inrange)
	ob->move_status++;

    if (ob->move_status == 0)
	return 0;
    else if (ob->move_status <10)
	return dir;
    else if (ob->move_status <15)
	return absdir(dir+4);
    ob->move_status = 0;
    return 0;
}

int disthit_att (int dir, object *ob, object *enemy, object *part,rv_vector *rv) {

    /* The logic below here looked plain wrong before.  Basically, what should
     * happen is that if the creatures hp percentage falls below run_away,
     * the creature should run away (dir+4)
     * I think its wrong for a creature to have a zero maxhp value, but
     * at least one map has this set, and whatever the map contains, the
     * server should try to be resilant enough to avoid the problem
     */
    if (ob->stats.maxhp && (ob->stats.hp*100)/ob->stats.maxhp < ob->run_away)
	return absdir(dir+4);
    return dist_att (dir,ob,enemy,part,rv);
}

int wait_att2 (int dir, object *ob,object *enemy,object *part, rv_vector *rv) {
    if (rv->distance < 9)
	return absdir (dir+4);
    return 0;
}

void circ1_move (object *ob) {
  static int circle [12] = {3,3,4,5,5,6,7,7,8,1,1,2};
  if(++ob->move_status > 11)
    ob->move_status = 0;
  if (!(move_object(ob,circle[ob->move_status])))
    (void) move_object(ob,RANDOM()%8+1);
}

void circ2_move (object *ob) {
  static int circle[20] = {3,3,3,4,4,5,5,5,6,6,7,7,7,8,8,1,1,1,2,2};
  if(++ob->move_status > 19)
    ob->move_status = 0;
  if(!(move_object(ob,circle[ob->move_status])))
    (void) move_object(ob,RANDOM()%8+1);
}

void pace_movev(object *ob) {
  if (ob->move_status++ > 6)
    ob->move_status = 0;
  if (ob->move_status < 4)
    (void) move_object (ob,5);
  else
    (void) move_object(ob,1);
}

void pace_moveh (object *ob) {
  if (ob->move_status++ > 6)
    ob->move_status = 0;
  if (ob->move_status < 4)
    (void) move_object(ob,3);
  else
    (void) move_object(ob,7);
}

void pace2_movev (object *ob) {
  if (ob->move_status ++ > 16)
    ob->move_status = 0;
  if (ob->move_status <6)
    (void) move_object (ob,5);
  else if (ob->move_status < 8)
    return;
  else if (ob->move_status <13)
    (void) move_object (ob,1);
  else return;
}       

void pace2_moveh (object *ob) {
  if (ob->move_status ++ > 16)
    ob->move_status = 0;
  if (ob->move_status <6)
    (void) move_object (ob,3);
  else if (ob->move_status < 8)
    return;
  else if (ob->move_status <13)
    (void) move_object (ob,7);
  else return;
}       

void rand_move (object *ob) {
  int i;
  if (ob->move_status <1 || ob->move_status >8 ||
      !(move_object(ob,ob->move_status|| ! (RANDOM()% 9))))
    for (i = 0; i < 5; i++)
      if (move_object(ob,ob->move_status = RANDOM()%8+1))
        return;
}

static void free_messages(msglang *msgs) {
  int messages, keywords;

  if (!msgs)
    return;
  for(messages = 0; msgs->messages[messages]; messages++) {
    if(msgs->keywords[messages]) {
      for(keywords = 0; msgs->keywords[messages][keywords]; keywords++)
        free(msgs->keywords[messages][keywords]);
      free(msgs->keywords[messages]);
    }
    free(msgs->messages[messages]);
  }
  free(msgs->messages);
  free(msgs->keywords);
  free(msgs);
}

static msglang *parse_message(const char *msg) {
  msglang *msgs;
  int nrofmsgs, msgnr, i;
  char *cp, *line, *last, *tmp;
  char *buf = strdup_local(msg);

  /* First find out how many messages there are.  A @ for each. */
  for (nrofmsgs = 0, cp = buf; *cp; cp++)
    if (*cp == '@')
      nrofmsgs++;
  if (!nrofmsgs) {
      free(buf);
      return NULL;
  }

  msgs = (msglang *) malloc(sizeof(msglang));
  msgs->messages = (char **) malloc(sizeof(char *) * (nrofmsgs + 1));
  msgs->keywords = (char ***) malloc(sizeof(char **) * (nrofmsgs + 1));
  for(i=0; i<=nrofmsgs; i++) {
    msgs->messages[i] = NULL;
    msgs->keywords[i] = NULL;
  }

  for (last = NULL, cp = buf, msgnr = 0;*cp; cp++)
    if (*cp == '@') {
      int nrofkeywords, keywordnr;
      *cp = '\0'; cp++;
      if(last != NULL)
	  {
        msgs->messages[msgnr++] = strdup_local(last);
		tmp = msgs->messages[msgnr-1];
	    for(i=(int)strlen(tmp);i;i--)
		{
		  if(*(tmp+i) && *(tmp+i) != 0x0a && *(tmp+i) != 0x0d)
			  break;
			*(tmp+i)= 0;
		}
		
	  }
      if(strncmp(cp,"match",5)) {
        LOG(llevBug,"BUG: parse_message(): Unsupported command in message.\n");
        free(buf);
        return NULL;
      }
      for(line = cp + 6, nrofkeywords = 0; *line != '\n' && *line; line++)
        if(*line == '|')
          nrofkeywords++;
      if(line > cp + 6)
        nrofkeywords++;
      if(nrofkeywords < 1) {
        LOG(llevBug,"BUG: parse_message():Too few keywords in message.\n");
        free(buf);
        free_messages(msgs);
        return NULL;
      }
      msgs->keywords[msgnr] = (char **) malloc(sizeof(char **) * (nrofkeywords +1));
      msgs->keywords[msgnr][nrofkeywords] = NULL;
      last = cp + 6;
      cp = strchr(cp,'\n');
      if(cp != NULL)
        cp++;
      for(line = last, keywordnr = 0;line<cp && *line;line++)
        if(*line == '\n' || *line == '|') {
          *line = '\0';
          if (last != line)
            msgs->keywords[msgnr][keywordnr++] = strdup_local(last);
	  else {
	        if (keywordnr<nrofkeywords)
		{
		   /* Whoops, Either got || or |\n in @match. Not good */
		   msgs->keywords[msgnr][keywordnr++] = strdup_local("xxxx");
		   /* We need to set the string to something sensible to    *
		    * prevent crashes later. Unfortunately, we can't set to *
		    * NULL, as that's used to terminate the for loop in     *
		    * talk_to_npc.  Using xxxx should also help map         *
		    * developers track down the problem cases.              */
		   LOG(llevBug, "BUG: parse_message(): Tried to set a zero length message in parse_message\n");
		   /* I think this is a error worth reporting at a reasonably *
		    * high level. When logging gets redone, this should       *
		    * be something like MAP_ERROR, or whatever gets put in    *
		    * place. */
		   if (keywordnr>1)
			   /* This is purely addtional information, should *
			    * only be gieb if asked */
			   LOG(llevDebug, "Msgnr %d, after keyword %s\n",msgnr+1,msgs->keywords[msgnr][keywordnr-2]);
		   else
			   LOG(llevDebug, "Msgnr %d, first keyword\n",msgnr+1);
		}
	  }
          last = line + 1;
        }
	  /*
	   * your eyes aren't decieving you, this is code repetition.  However,
	   * the above code doesn't catch the case where line<cp going into the
	   * for loop, skipping the above code completely, and leaving undefined
           * data in the keywords array.  This patches it up and solves a crash
	   * bug.  garbled 2001-10-20
	   */
	  if (keywordnr < nrofkeywords) {
		  LOG(llevBug, "BUG: parse_message(): Map developer screwed up match statement"
		      " in parse_message\n");
		   if (keywordnr>1)
			   /* This is purely addtional information, should *
			    * only be gieb if asked */
			   LOG(llevDebug, "Msgnr %d, after keyword %s\n", msgnr+1,
				   msgs->keywords[msgnr][keywordnr-2]);
		   else
			   LOG(llevDebug, "Msgnr %d, first keyword\n",msgnr+1);
#if 0
/* Removed this block - according to the compiler, this has no effect,
 * and looking at the if statement above, the certainly appears to be the
 * case.
 */
	      for(keywordnr; keywordnr <= nrofkeywords; keywordnr++)
		      msgs->keywords[msgnr][keywordnr] = strdup_local("xxxx");
#endif
	  }
      last = cp;
    }
  if(last != NULL)
    msgs->messages[msgnr++] = strdup_local(last);

	tmp = msgs->messages[msgnr-1];
	for(i=(int)strlen(tmp);i;i--)
	{
		if(*(tmp+i) && *(tmp+i) != 0x0a && *(tmp+i) != 0x0d)
			break;
		*(tmp+i)= 0;
	}
  free(buf);
  return msgs;
}

#if 0
/* Removed because it wasn't used according to gcc */
static void dump_messages(msglang *msgs) {
  int messages, keywords;
  for(messages = 0; msgs->messages[messages]; messages++) {
    LOG(llevDebug, "@match ");
    for(keywords = 0; msgs->keywords[messages][keywords]; keywords++)
      LOG(llevDebug, "%s ",msgs->keywords[messages][keywords]);
    LOG(llevDebug, "\n%s\n",msgs->messages[messages]);
  }
}
#endif

/* i changed this... This function is not to understimate when player talk alot
 * in areas which alot if map objects... This is one of this little extra cpu eaters
 * which adds cput time here and there.
 * i added P_MAGIC_EAR as map flag - later we should use a chained list in the map headers
 * perhaps. I also removed the npcs from the map search and use the target system. 
 * This IS needed because in alot of cases in the past you was not able to target the
 * npc you want - if the search routine find another npc first, the other was silenced.
 * MT-2003
 */
void communicate(object *op, char *txt) 
{
	object *npc;
	mapstruct *m;
	int i,xt,yt;

    char buf[MAX_BUF];

    if (!txt)
		return;


	/* with target, only player can talk to npc... for npc to npc talk we need so or so a script,
	 * and there we have then to add the extra interface.
	 */

	/* thats the whisper code - i will add a /whisper for it and remove it from here */
	/*
	if(op->type == PLAYER)
	{
		if(op->contr->target_object && op->contr->target_object_count==op->contr->target_object->count)
		{
			if(op->contr->target_object->type == PLAYER)
			{
				if(op != op->contr->target_object)
				{
				    sprintf(buf, "%s whispers to you: ",query_name(op));
					strncat(buf, txt, MAX_BUF - strlen(buf)-1);
					buf[MAX_BUF-1]=0;
					new_draw_info(NDI_WHITE|NDI_GREEN,0, op->contr->target_object, buf);
				    sprintf(buf, "you whispers to %s: ",query_name(op->contr->target_object));
					strncat(buf, txt, MAX_BUF - strlen(buf)-1);
					buf[MAX_BUF-1]=0;
					new_draw_info(NDI_WHITE|NDI_GREEN,0, op, buf);
				    sprintf(buf, "%s whispers something to %s.",query_name(op),query_name(op->contr->target_object));
					new_info_map_except2(NDI_WHITE,op->map, op, op->contr->target_object, buf);
					if(op->contr->target_object->map && op->contr->target_object->map != op->map)
						new_info_map_except2(NDI_WHITE,op->contr->target_object->map, op, op->contr->target_object, buf);
				}
				else 
				{
				    sprintf(buf, "%s says: ",query_name(op));
					strncat(buf, txt, MAX_BUF - strlen(buf)-1);
					buf[MAX_BUF-1]=0;
					new_info_map(NDI_WHITE,op->map, buf);
				}
			}
			else 
			{
			    sprintf(buf, "%s says to %s: ",query_name(op),query_name(op->contr->target_object));
				strncat(buf, txt, MAX_BUF - strlen(buf)-1);
				buf[MAX_BUF-1]=0;
				new_info_map_except(NDI_WHITE,op->map, op, buf);
				if(op->contr->target_object->map && op->contr->target_object->map != op->map)
					new_info_map_except(NDI_WHITE,op->contr->target_object->map, op, buf);

				sprintf(buf, "you say to %s: ",query_name(op->contr->target_object));
				strncat(buf, txt, MAX_BUF - strlen(buf)-1);
				buf[MAX_BUF-1]=0;
				new_draw_info(NDI_WHITE,0, op, buf);
				talk_to_npc(op, op->contr->target_object,txt);
			}
		}
		else
		{
		    sprintf(buf, "%s says: ",query_name(op));
			strncat(buf, txt, MAX_BUF - strlen(buf)-1);
			buf[MAX_BUF-1]=0;
			new_info_map(NDI_WHITE,op->map, buf);
		}
	}
	else
	{
		sprintf(buf, "%s says: ",query_name(op));
		strncat(buf, txt, MAX_BUF - strlen(buf)-1);
		buf[MAX_BUF-1]=0;
		new_info_map(NDI_WHITE,op->map, buf);
	}
	*/

	sprintf(buf, "%s says: ",query_name(op));
	strncat(buf, txt, MAX_BUF - strlen(buf)-1);
	buf[MAX_BUF-1]=0;
	new_info_map(NDI_WHITE,op->map, buf);

	for(i = 0; i <= SIZEOFFREE2; i++)
	{
		xt = op->x+freearr_x[i];
		yt = op->y+freearr_y[i];
		if ((m=out_of_map(op->map, &xt, &yt)))
		{
			if(GET_MAP_FLAGS(m,xt,yt) & (P_MAGIC_EAR | P_IS_ALIVE)) /* quick check we have a magic ear */
			{
				/* ok, browse now only on demand */
				for(npc = get_map_ob(m,xt,yt);npc != NULL; npc = npc->above) 
				{
					/* the ear ... don't break because it can be mutiple on a node or more in the area */
					if (npc->type == MAGIC_EAR)
						(void) talk_to_wall(npc, txt); /* Maybe exit after 1. success? */
					else if (QUERY_FLAG(npc,FLAG_ALIVE))
						talk_to_npc(op, npc,txt);
				}
			}
		}
	}
}

/* this communication thingy is ugly - ugly in the way it use malloc for getting
buffers for storing parts of the msg text??? There should be many smarter ways
to handle it
*/
int talk_to_npc(object *op, object *npc, char *txt) {
  msglang *msgs;
  int i,j;
  object *cobj;
#ifdef PLUGINS
  /* GROS: Handle for plugin say event */
  if(npc->event_flags&EVENT_FLAG_SAY)
  {
    CFParm CFP;
    int k, l, m;
	object *event_obj = get_event_object(npc, EVENT_SAY);
    k = EVENT_SAY;
    l = SCRIPT_FIX_ACTIVATOR;
    m = 0;
    CFP.Value[0] = &k;
    CFP.Value[1] = op;
    CFP.Value[2] = npc;
    CFP.Value[3] = NULL;
    CFP.Value[4] = txt;
    CFP.Value[5] = &m;
    CFP.Value[6] = &m;
    CFP.Value[7] = &m;
    CFP.Value[8] = &l;
    CFP.Value[9] = (char *)event_obj->race;
    CFP.Value[10]= (char *)event_obj->slaying;
    if (findPlugin(event_obj->name)>=0)
    {
        ((PlugList[findPlugin(event_obj->name)].eventfunc) (&CFP));
        return 0;
    }
  }
  /* GROS - Here we let the objects inside inventories hear and answer, too. */
  /* This allows the existence of "intelligent" weapons you can discuss with */
  for(cobj=npc->inv;cobj!=NULL;)
  {
    if(cobj->event_flags&EVENT_FLAG_SAY)
    {
      CFParm CFP;
      int k, l, m;
 	  object *event_obj = get_event_object(cobj, EVENT_SAY);
      k = EVENT_SAY;
      l = SCRIPT_FIX_ALL;
      m = 0;
      CFP.Value[0] = &k;
      CFP.Value[1] = op;
      CFP.Value[2] = cobj;
      CFP.Value[3] = npc;
      CFP.Value[4] = txt;
      CFP.Value[5] = &m;
      CFP.Value[6] = &m;
      CFP.Value[7] = &m;
      CFP.Value[8] = &l;
      CFP.Value[9] = (char *)event_obj->race;
      CFP.Value[10]= (char *)event_obj->slaying;
      if (findPlugin(event_obj->name)>=0)
      {
          ((PlugList[findPlugin(event_obj->name)].eventfunc) (&CFP));
          return 0;
      }
    }
    cobj = cobj->below;
  }

#endif
  if(npc->msg == NULL || *npc->msg != '@')
  {
	/*new_draw_info_format(NDI_UNIQUE,0,op, "%s has nothing to say.", query_name(npc));*/
    return 0;
  }
  if((msgs = parse_message(npc->msg)) == NULL)
    return 0;
#if 0 /* Turn this on again when enhancing parse_message() */
  if(debug)
    dump_messages(msgs);
#endif
  for(i=0; msgs->messages[i]; i++)
    for(j=0; msgs->keywords[i][j]; j++)
      if(msgs->keywords[i][j][0] == '*' || re_cmp(txt,msgs->keywords[i][j])) {
        char buf[MAX_BUF];
		if(op->type != PLAYER) /* a npc talks to another one - show both in white */
		{
			/* if a message starts with '/', we assume a emote */
			/* we simply hook here in the emote msg list */
			if(*msgs->messages[i] == '/')
			{
				CommArray_s *csp;
				char *cp=NULL;
				
				csp = find_command_element(msgs->messages[i], CommunicationCommands, CommunicationCommandSize);
				if(csp)
					csp->func(npc, cp);


			}
			else
			{
				sprintf(buf,"%s says: %s",query_name(npc),msgs->messages[i]);
				new_info_map_except(NDI_UNIQUE, op->map, op, buf);
				if(op->map != npc->map)
					new_info_map_except(NDI_UNIQUE, npc->map, op, buf);
			}
		}
		else /* if a npc is talking to a player, is shown navy and with a seperate "xx says:" line */
		{
	        sprintf(buf,"%s says:",query_name(npc)); 
			new_draw_info(NDI_NAVY|NDI_UNIQUE,0,op, buf);
			new_draw_info(NDI_NAVY | NDI_UNIQUE,0,op, msgs->messages[i]);
			sprintf(buf,"%s talks to %s.", query_name(npc), query_name(op));
			new_info_map_except(NDI_UNIQUE, op->map, op, buf);
			if(op->map != npc->map)
				new_info_map_except(NDI_UNIQUE, npc->map, op, buf);
		}
        free_messages(msgs);
        return 1;
      }
  free_messages(msgs);
  return 0;
}

int talk_to_wall(object *npc, char *txt) {
  msglang *msgs;
  int i,j;

  if(npc->msg == NULL || *npc->msg != '@')
    return 0;
  if((msgs = parse_message(npc->msg)) == NULL)
    return 0;
#if 0 /* Turn this on again when enhancing parse_message() */
  if(settings.debug >= llevDebug)
    dump_messages(msgs);
#endif
  for(i=0; msgs->messages[i]; i++)
    for(j=0; msgs->keywords[i][j]; j++)
      if(msgs->keywords[i][j][0] == '*' || re_cmp(txt,msgs->keywords[i][j])) {
        if (msgs->messages[i] && *msgs->messages[i] != 0)
	  new_info_map(NDI_NAVY | NDI_UNIQUE, npc->map,msgs->messages[i]);
        free_messages(msgs);
	use_trigger(npc);
        return 1;
      }
  free_messages(msgs);
  return 0;
}

/* find_mon_throw_ob() - modeled on find_throw_ob
 * This is probably overly simplistic as it is now - We want
 * monsters to throw things like chairs and other pieces of
 * furniture, even if they are not good throwable objects.
 * Probably better to have the monster throw a throwable object
 * first, then throw any non equipped weapon.
 */
/* no, i don't want monster throwing chairs and something. 
 * i want monster picking up throwing weapons and ammo and using it. MT.
 */
object *find_mon_throw_ob( object *op ) {
    object *tmp = NULL;
  
    if(op->head) tmp=op->head; else tmp=op;  

    /* New throw code: look through the inventory. Grap the first legal is_thrown
     * marked item and throw it to the enemy.
     */

    for(tmp=op->inv;tmp;tmp=tmp->below) {

	/* Can't throw invisible objects or items that are applied */
      if(IS_SYS_INVISIBLE(tmp) || QUERY_FLAG(tmp,FLAG_APPLIED)) continue;

      if(QUERY_FLAG(tmp,FLAG_IS_THROWN)) 
          break;
          
    }

#ifdef DEBUG_THROW
    LOG(llevDebug,"%s chooses to throw: %s (%d)\n",op->name,
	!(tmp)?"(nothing)":query_name(tmp),tmp?tmp->count:-1);
#endif

    return tmp;
}

/* Copied from CF, attach this after the attack system rework */
int monster_use_scroll(object *head, object *part,object *pl,int dir, rv_vector *rv) 
{
    object *scroll=NULL;
    object *owner;
    rv_vector	rv1;

    /* If you want monsters to cast spells over friends, this spell should
     * be removed.  It probably should be in most cases, since monsters still
     * don't care about residual effects (ie, casting a cone which may have a 
     * clear path to the player, the side aspects of the code will still hit
     * other monsters)
     */
    if(!(dir=path_to_player(part,pl,0)))
        return 0;
    
    if(QUERY_FLAG(head,FLAG_FRIENDLY) && (owner = get_owner(head)) != NULL) {
	get_rangevector(head, owner, &rv1, 0x1);
	if(dirdiff(dir,rv1.direction) < 2) {
	        return 0; /* Might hit owner with spell */
	}
    }

    if(QUERY_FLAG(head,FLAG_CONFUSED))
	dir = absdir(dir + RANDOM()%3 + RANDOM()%3 - 2);

/*
    for (scroll=head->inv; scroll; scroll=scroll->below)
	if (scroll->type == SCROLL && monster_should_cast_spell(head, scroll->stats.sp)) break;
*/
    /* Used up all his scrolls, so nothing do to */
    if (!scroll) {
	/*CLEAR_FLAG(head, FLAG_READY_SCROLL);*/
	return 0;
    }

    if (spells[scroll->stats.sp].flags&SPELL_DESC_SELF) /* Spell should be cast on caster (ie, heal, strength) */
	dir = 0;

   /* apply_scroll(part, scroll, dir);*/
    return 1;
}


/* central spawn point function.
 * Control, generate or remove the generated object.
 */
void spawn_point(object *op)
{	
	int rmt;
    object *tmp, *mob, *next, *item;
    object *tmp2, *next2;

	if(op->enemy)
	{
		if(OBJECT_VALID(op->enemy, op->enemy_count)) /* all ok, our spawn have fun */
			return;
		op->enemy = NULL;
	}


	/* now we get a random value from 0 to 9999. */
	if(op->stats.sp == -1)
		op->stats.sp = (RANDOM() % SPAWN_RANDOM_RANGE);

		/* now we move through the spawn point inventory and
	 * get the mob with a number under this value AND nearest.
	 */
    for(rmt=0,mob=NULL,tmp = op->inv; tmp; tmp = next)
    {
		next = tmp->below;

		if(tmp->type != SPAWN_POINT_MOB)
			LOG(llevBug,"SPAWN: spawn point in map %s (%d,%d) with wrong type object (%d) in inv: %s\n",
											op->map?op->map->name:"<no map>", op->x, op->y, tmp->type, query_name(tmp));
		else if((int)tmp->enemy_count <= op->stats.sp && (int)tmp->enemy_count >= rmt)
		{
			rmt = (int)tmp->enemy_count;
			mob = tmp;
		}
/*		LOG(llevInfo,"inv -> %s (%d :: %d - %f)\n", tmp->name, op->stats.sp, tmp->enemy_count, tmp->speed_left);*/

	}
	/* we try only ONE time a respawn of a pre setting - so careful! */
	rmt = op->stats.sp;
	op->stats.sp = -1;
	if(!mob) /* well, this time we spawn nothing */
		return;

	tmp = mob->inv; /* quick save the def mob inventory */
	if(!(mob = spawn_monster(mob, op, op->last_heal)))
		return; /* that happens when we have no free spot....*/

	/* we have a mob - now insert a copy of all items the spawn point mob has.
	 * take care about RANDOM DROP objects.
	 * usually these items are put from the map maker inside the spawn mob inv.
	 * remember that these are additional items to the treasures list ones.
	 */
    for(; tmp; tmp = next)
	{
		next = tmp->below;
		if(tmp->type == TYPE_RANDOM_DROP)
		{
			if((RANDOM() %RANDOM_DROP_RAND_RANGE) >= tmp->carrying) /* skip this container - drop the ->inv */
			{
			    for(tmp2=tmp->inv; tmp2; tmp2 = next2)
				{
					next2 = tmp2->below;
					if(tmp2->type == TYPE_RANDOM_DROP)
						LOG(llevDebug,"DEBUG:: Spawn:: RANDOM_DROP (102) not allowed inside RANDOM_DROP.mob:>%s< map:%s (%d,%d)\n",
								query_name(mob),op->map?op->map->path:"BUG: S-Point without map!", op->x, op->y);
					else
					{
						item = get_object();
						copy_object(tmp2,item);
						insert_ob_in_ob(item,mob);      /* and put it in the mob */
					}
				}
			}
		}
		else /* remember this can be sys_objects too! */ 
		{
			item = get_object();
			copy_object(tmp,item);
			insert_ob_in_ob(item,mob);      /* and put it in the mob */
		}
	}

	op->last_sp = rmt; /* this is the last rand() for what we have spawned! */

	op->enemy = mob; /* chain the mob to our spawn point */
	op->enemy_count = mob->count;

	/* perhaps we have later more unique mob setting - then we can store it here too.
	 */
	tmp = arch_to_object(op->other_arch); /* create spawn info */
	tmp->owner = op; /* chain spawn point to our mob */
    insert_ob_in_ob(tmp,mob);      /* and put it in the mob */

	SET_FLAG(mob, FLAG_SPAWN_MOB); /* FINISH: now mark our mob as a spawn */
	fix_monster(mob); /* fix all the values and add in possible abilities or forces ... */
    insert_ob_in_map(mob,mob->map,op,0); /* *now* all is done - *now* put it on map */
}

/* drop a monster on the map, by copying a monster object or
 * monster object head. Add treasures.
 */
static object *spawn_monster(object *gen, object *orig, int range) 
{
  int i;
  object *op,*head=NULL,*prev=NULL, *ret=NULL;
  archetype *at=gen->arch;

  i=find_free_spot(at,orig->map,orig->x,orig->y,0,range);
  if (i==-1)
	  return NULL; 
  while(at!=NULL) {
	op = get_object();
    if(head==NULL) /* copy single/head from spawn inventory */
	{
		gen->type = MONSTER;
		copy_object(gen,op);
		gen->type = SPAWN_POINT_MOB;
		ret = op;
	}
	else /* but the tails for multi arch from the clones */
	{
		copy_object(&at->clone,op);
	}
    op->x=orig->x+freearr_x[i]+at->clone.x;
    op->y=orig->y+freearr_y[i]+at->clone.y;
	op->map = orig->map;
    if(head!=NULL)
      op->head=head,prev->more=op;
    if (QUERY_FLAG(op, FLAG_FREED)) return NULL;
    if(op->randomitems!=NULL)
      create_treasure(op->randomitems,op,0,orig->map->difficulty,T_STYLE_UNSET,ART_CHANCE_UNSET,0);
    if(head==NULL)
      head=op;
    prev=op;
    at=at->more;
  }
  return ret; /* return object ptr to our spawn */
}
