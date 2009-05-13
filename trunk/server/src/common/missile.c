/*
    Daimonin, the Massive Multiuser Online Role Playing Game
    Server Applicatiom

    Copyright (C) 2001,2007 Michael Toennies

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

    The author can be reached via e-mail to info@daimonin.net
*/
#include <global.h>

/* player throws a throw item (which are special arrows)
 */
float do_throw(object *op, int dir)
{
	float ticks;
	object *throw_arrow;
	player *pl = CONTR(op);

	/* 2 things here prevent a throw:
	* you aimed at your feet, or you threw at a wall
	*/
	if (!dir || wall(op->map, op->x + freearr_x[dir], op->y + freearr_y[dir]))
		return 0;

	throw_arrow = pl->equipment[PLAYER_EQUIP_BOW];
	if (!throw_arrow || throw_arrow->type != ARROW)
	{
		LOG(llevBug, "BUG: Range: bow without applied throw item (%s - %d).\n", op->name, dir);
		return 0;
	}

	if (wall(op->map, op->x + freearr_x[dir], op->y + freearr_y[dir]))
	{
		new_draw_info(NDI_UNIQUE, 0, op, "Something is in the way.");
		return 0;
	}

	if(!(throw_arrow = get_split_ob(throw_arrow, 1)))
	{
		new_draw_info(NDI_UNIQUE, 0, op, "You have nothing to throw.");
		return 0;
	}

	/* used action time - hotfix for possible action timer bug */
	LOG(llevDebug, "AC-TICKS: item %s ->%d for skill %s ->%d.\n", query_name(throw_arrow),throw_arrow->last_grace,query_name(op->chosen_skill),op->chosen_skill->last_grace);
	ticks = (float) (throw_arrow->last_grace + op->chosen_skill->last_grace) * RANGED_DELAY_TIME;
	if(!ticks)
	{
		LOG(llevDebug, "ACBUG: ticks = 0\n");
		ticks = 1.25f;
	}

	create_missile(op, NULL, throw_arrow, dir);

	return ticks;
}

/*
 *  Player fires a bow.
 */
float fire_bow(object *op, int dir)
{
	float ticks;
	object *bow, *arrow;
	player *pl = CONTR(op);

	/* 2 things here prevent a throw:
	* you aimed at your feet, or you threw at a wall
	*/
	if (!dir || wall(op->map, op->x + freearr_x[dir], op->y + freearr_y[dir]))
		return 0;

	/* check bow */
	bow = pl->equipment[PLAYER_EQUIP_BOW];
	if (!bow || bow->type != BOW)
	{
		LOG(llevBug, "BUG: Range: bow without activated bow (%s - %d).\n", op->name, dir);
		return 0;
	}

	if (!bow->race || !bow->item_condition)
	{
		new_draw_info_format(NDI_UNIQUE, 0, op, "Your %s is broken.", bow->name);
		return 0;
	}

	if (wall(op->map, op->x + freearr_x[dir], op->y + freearr_y[dir]))
	{
		new_draw_info(NDI_UNIQUE, 0, op, "Something is in the way.");
		return 0;
	}

	/* get an arrow from stack */
	arrow = get_split_ob(pl->equipment[PLAYER_EQUIP_AMUN], 1);
	if(!arrow || arrow->sub_type1 != bow->sub_type1)
	{
		new_draw_info_format(NDI_UNIQUE, 0, op, "You have no %s left.", bow->race);
		return 0;
	}

	/* used action time */
	LOG(llevDebug, "AC-TICKS: bow %s ->%2.2f arrow %s ->%2.2f for skill %s ->%2.2f.\n", 
		query_name(bow),(float) (bow->last_grace) * RANGED_DELAY_TIME,query_name(arrow),(float) (arrow->last_grace) * RANGED_DELAY_TIME,
		query_name(op->chosen_skill),(float) (op->chosen_skill->last_grace) * RANGED_DELAY_TIME);
	ticks = (float) (arrow->last_grace + bow->last_grace + op->chosen_skill->last_grace) * RANGED_DELAY_TIME;
	if(!ticks)
	{
		LOG(llevDebug, "ACBUG: ticks = 0\n");
		ticks = 1.25f;
	}

	create_missile(op, bow, arrow, dir);

	return ticks;
}

/*
*  Find an arrow in the inventory and after that
*  in the right type container (quiver). Pointer to the
*  found object is returned.
*/
object * find_arrow(object *op, const char *type)
{
	object *tmp = NULL;

	for (op = op->inv; op; op = op->below)
		if (!tmp && op->type == CONTAINER && op->race == type && QUERY_FLAG(op, FLAG_APPLIED))
			tmp = find_arrow(op, type);
		else if (op->type == ARROW && op->race == type)
			return op;
	return tmp;
}


/* setup the missile (arrow) stats basing on the owner (the object which fires the missile)
 * This is used from player and monsters.
*/
object *create_missile(object * const owner, const object * const bow, object * const missile, const int dir)
{
	object * const arrow = missile;

	set_owner(arrow, owner);
	SET_ANIMATION(arrow, (NUM_ANIMATIONS(arrow) / NUM_FACINGS(arrow)) * dir);

	/* backup the original data of the arrow */
	arrow->stats.maxhp = arrow->stats.wc; /* save original wc and dam */
	arrow->stats.hp = arrow->stats.dam; /* will be put back in fix_arrow() */
	arrow->stats.sp = arrow->last_sp; /* how long the arrow is flying */

	/* we have a bow object... adjust the arrow/missile values by it */
	if(bow)
	{
		/* our main values for archery are comming from the bow in B4 - we just add them to the arrow values */
		arrow->stats.wc += bow->stats.wc + bow->magic + arrow->magic;
		arrow->stats.dam += bow->stats.dam + bow->magic + arrow->magic;
		arrow->last_sp += bow->last_sp; /* no magic adjustment here */
	}

	/* for players we add in the skill values */
	if(owner->type == PLAYER)
	{
		if(!owner->chosen_skill) /* sanity check */
		{
			LOG(-1, "create_missle: No chosen_skill for player %s\n", query_name(owner));
			return NULL;
		}

		/* adjust the values by the player skill settings */
		arrow->stats.wc += owner->chosen_skill->stats.wc;
		arrow->last_sp += owner->chosen_skill->last_sp;

		/* as a special adjustment we increase the WC with the dexterity skill + equipment bonus */
		arrow->stats.wc = (int)((float)arrow->stats.wc * get_player_stat_bonus(owner->stats.Dex))+CONTR(owner)->wc_bonus;

		arrow->level = owner->chosen_skill->level;

		/* the new damage code... we use the same as in fix_player for normal weapons */
		arrow->stats.dam = (sint16) ((float) arrow->stats.dam * LEVEL_DAMAGE(owner->chosen_skill->level));
		/* boni for throwing are 50% str + 50% dex (archery 100% dex, melee 100% str) + equipment bonus */
		arrow->stats.dam = (int)(((float)arrow->stats.dam
						* (get_player_stat_bonus(owner->stats.Str)/2.0f 
						   + get_player_stat_bonus(owner->stats.Dex)/2.0f)) /10.0f)+CONTR(owner)->dam_bonus;
	}
	else /* its a mob or other "living" threaded object */
	{
		arrow->level = owner->level; 
		/* for mobs we simply get the mob level for the dmg adjustment AND, to cover the stats increase
		 * of players, we use the level+10 (to avoid negative values) as stat bonus! */
		arrow->stats.dam = (sint16) ((float) (arrow->stats.dam * LEVEL_DAMAGE(arrow->level) * 
												get_player_stat_bonus(arrow->level+10))/10.0f);
	}

	/* and finally the item condition! (mob & players) */
	arrow->stats.dam = (sint16)((float) arrow->stats.dam * ((float)arrow->item_condition/100.0f)); 

	if (arrow->stats.dam < 0)
		arrow->stats.dam = 0;

	/*LOG(-1, "create_missile(%d): range=%d dam:%d(%d) wc:%d(%d)\n",arrow->level, arrow->last_sp, arrow->stats.dam, arrow->stats.hp,
		arrow->stats.wc,arrow->stats.maxhp);*/

	arrow->stats.grace = arrow->last_sp; /* temp. buffer for "tiles to fly" */
	arrow->stats.maxgrace = 60 + (RANDOM() % 12); /* reflection timer */

	SET_MULTI_FLAG(arrow, FLAG_FLYING);
	SET_FLAG(arrow, FLAG_IS_MISSILE);
	SET_FLAG(arrow, FLAG_FLY_ON);
	SET_FLAG(arrow, FLAG_WALK_ON);
	SET_FLAG(arrow, FLAG_NO_PICK);
	SET_FLAG(arrow, FLAG_NO_DROP);
	arrow->map = owner->map;
	arrow->direction = dir;
	arrow->x = owner->x;
	arrow->y = owner->y;

	arrow->speed_left = 0;
	arrow->speed = 1.0f;
	update_ob_speed(arrow);

	if (insert_ob_in_map(arrow, arrow->map, owner, 0))
	{
		play_sound_map(arrow->map, arrow->x, arrow->y, SOUND_THROW, SOUND_NORMAL);
		move_missile(arrow);
	}

	return arrow;
}

/* Move an arrow along its course.  op is the arrow or thrown object.
*/
void move_missile(object *op)
{
	object*tmp =    NULL, *hitter;
	int             new_x, new_y;
	int             flag_tmp;
	int             was_reflected;
	mapstruct      *m   = op->map;

	if (op->map == NULL)
	{
		LOG(llevBug, "BUG: Arrow %s had no map.\n", query_name(op));
		remove_ob(op);
		check_walk_off(op, NULL, MOVE_APPLY_VANISHED);
		return;
	}

	/* we need to stop thrown objects and arrows at some point. Like here. */
	if (op->type == THROWN_OBJ)
	{
		if (op->inv == NULL)
			return;
	}

	if (op->last_sp-- < 0)
	{
		stop_missile(op);
		return;
	}

	/* Calculate target map square */
	if (op->stats.grace == 666)
	{
		/* Experimental target throwing hack. Using bresenham line algo */

		int dx  = op->stats.hp;
		int dy  = op->stats.sp;

		if (dx > dy)
		{
			if (op->stats.exp >= 0)
			{
				new_y = op->y + op->stats.maxsp;
				op->stats.exp -= dx;                           /* same as fraction -= 2*dx */
			}
			else
				new_y = op->y;
			new_x = op->x + op->stats.maxhp;
			op->stats.exp += dy;                           /* same as fraction -= 2*dy */
		}
		else
		{
			if (op->stats.exp >= 0)
			{
				new_x = op->x + op->stats.maxhp;
				op->stats.exp -= dy;
			}
			else
				new_x = op->x;
			new_y = op->y + op->stats.maxsp;
			op->stats.exp += dx;
		}
	}
	else
	{
		new_x = op->x + DIRX(op);
		new_y = op->y + DIRY(op);
	}
	was_reflected = 0;

	/* check we are legal */
	if (!(m = out_of_map(op->map, &new_x, &new_y)))
	{
		stop_missile(op); /* out of map... here is the end */
		return;
	}

	if (!(hitter = get_owner(op)))
		hitter = op;

	/* ok, lets check there is something we can hit */
	if ((flag_tmp = GET_MAP_FLAGS(m, new_x, new_y)) & (P_IS_ALIVE | P_IS_PLAYER))
	{
		/* search for a vulnerable object */
		for (tmp = GET_MAP_OB_LAYER(m, new_x, new_y, 5); tmp != NULL; tmp = tmp->above)
		{
			/* Can only damage live objects (for now) */
			if (!IS_LIVE(tmp))
				continue;

			/* Let friends fire through friends */
			/* TODO: shouldn't do this if on pvp map, but that
			* also requires smarter mob/npc archers */
			if (tmp == hitter || get_friendship(hitter, tmp) >= FRIENDSHIP_HELP)
			{
				/* Gecko: testing to let friends fire through friendly
				* reflectors */
				if (QUERY_FLAG(tmp, FLAG_CAN_REFL_MISSILE))
					flag_tmp &= ~P_REFL_MISSILE;
				continue;
			}

			if (!QUERY_FLAG(tmp, FLAG_CAN_REFL_MISSILE) || (random_roll(0, 99) > (90 - op->level / 10)))
			{
				/* Attack the object. */
				op = hit_with_arrow(op, tmp);
				if (op == NULL) /* the arrow has hit and is destroyed! */
					return;
			}
		}
	}

	/* if we are here, there is no target and/or we have not hit.
	* now we do a simple reflection test.
	*/
	if (flag_tmp & P_REFL_MISSILE)
	{
		missile_reflection_adjust(op, QUERY_FLAG(op, FLAG_WAS_REFLECTED));

		op->direction = absdir(op->direction + 4);
		op->state = 0;
		if (GET_ANIM_ID(op))
			SET_ANIMATION(op, (NUM_ANIMATIONS(op) / NUM_FACINGS(op)) * op->direction);
		if (wall(m, new_x, new_y))
		{
			/* Target is standing on a wall.  Let arrow turn around before
			* the wall. */
			new_x = op->x;
			new_y = op->y;
		}
		SET_FLAG(op, FLAG_WAS_REFLECTED);
		was_reflected = 1;   /* skip normal movement calculations */
	}

	if (!was_reflected && wall(m, new_x, new_y))
	{
		/* if the object doesn't reflect, stop the arrow from moving */
		if (!QUERY_FLAG(op, FLAG_REFLECTING) || !(random_roll(0, 19)))
		{
			stop_missile(op);
			return;
		}
		else
		{
			/* object is reflected */
			/* If one of the major directions (n,s,e,w), just reverse it */
			if (op->direction & 1)
			{
				op->direction = absdir(op->direction + 4);
			}
			else
			{
				/* The below is just logic for figuring out what direction
				* the object should now take.
				*/

				int left = wall(op->map,op->x+freearr_x[absdir(op->direction-1)],
					op->y+freearr_y[absdir(op->direction - 1)]),
					right = wall(op->map, op->x + freearr_x[absdir(op->direction + 1)],
					op->y + freearr_y[absdir(op->direction + 1)]);

				if (left == right)
					op->direction = absdir(op->direction + 4);
				else if (left)
					op->direction = absdir(op->direction + 2);
				else if (right)
					op->direction = absdir(op->direction - 2);
			}
			/* Is the new direction also a wall?  If show, shuffle again */
			if (wall(op->map, op->x + DIRX(op), op->y + DIRY(op)))
			{
				int left = wall(op->map,op->x+freearr_x[absdir(op->direction-1)],
					op->y+ freearr_y[absdir(op->direction - 1)]),
					right = wall(op->map, op->x + freearr_x[absdir(op->direction + 1)],
					op->y + freearr_y[absdir(op->direction + 1)]);

				if (!left)
					op->direction = absdir(op->direction - 1);
				else if (!right)
					op->direction = absdir(op->direction + 1);
				else
				{
					/* is this possible? */
					stop_missile(op);
					return;
				}
			}
			/* update object image for new facing */
			/* many thrown objects *don't* have more than one face */
			if (GET_ANIM_ID(op))
				SET_ANIMATION(op, (NUM_ANIMATIONS(op) / NUM_FACINGS(op)) * op->direction);
		} /* object is reflected */
	} /* object ran into a wall */

	/* Move the arrow. */
	remove_ob(op);
	if (check_walk_off(op, NULL, MOVE_APPLY_VANISHED) == CHECK_WALK_OK)
	{
		op->x = new_x;
		op->y = new_y;
		insert_ob_in_map(op, m, op, 0);
	}
}

/* add/clear settings and flags to transform a flying missile to 
 * a normal item again. Only called from stop_missile() now, which
 * is the only function to handle neutralizing missiles after
 * fired.
 */
static inline object * fix_stopped_missile(object *op)
{
	object *tmp;

	if (op->type != ARROW)
		return op;

	trigger_object_plugin_event(EVENT_STOP,
		op, NULL, NULL, NULL, NULL, NULL, NULL, SCRIPT_FIX_ALL);

	/* restore original values */
	op->last_sp = op->stats.sp; 
	op->stats.wc = op->stats.maxhp;
	op->stats.dam = op->stats.hp;

	/* Reset these to default, so that CAN_MERGE will work properly */
	op->stats.grace = op->arch->clone.stats.grace;
	op->stats.maxgrace = op->arch->clone.stats.maxgrace;
	op->direction = op->arch->clone.direction;
	op->stats.sp = op->arch->clone.stats.sp;
	op->stats.hp = op->arch->clone.stats.hp;
	op->stats.maxhp = op->arch->clone.stats.maxhp;
	op->level = op->arch->clone.level;

	CLEAR_FLAG(op, FLAG_WALK_ON);
	CLEAR_FLAG(op, FLAG_FLY_ON);
	CLEAR_MULTI_FLAG(op, FLAG_FLYING);
	CLEAR_FLAG(op, FLAG_IS_MISSILE);
	
	/* food is a self destruct marker - that long the item will need to be destruct! */
	if (!QUERY_FLAG(op, FLAG_NO_DROP) && 
		((!(tmp = get_owner(op)) || tmp->type != PLAYER) && op->stats.food && op->type == ARROW))
	{
		SET_FLAG(op, FLAG_IS_USED_UP);
		SET_FLAG(op, FLAG_NO_PICK);
		op->type = MISC_OBJECT; /* important to neutralize the arrow! */
		op->sub_type1 = ARROW;  /* so we still can identify the arrow! */
		op->speed = 0.1f;
		op->speed_left = 0.0f;
	}
	else
	{
		if(QUERY_FLAG(op, FLAG_NO_DROP))
		{
			if(!QUERY_FLAG(op, FLAG_REMOVED))
			{
				remove_ob(op);
				check_walk_off(op, NULL, MOVE_APPLY_VANISHED);
			}
			return NULL;
		}
		if(QUERY_FLAG(&op->arch->clone, FLAG_NO_PICK))
			SET_FLAG(op, FLAG_NO_PICK);
		else
			CLEAR_FLAG(op, FLAG_NO_PICK);
		op->speed = 0;
	}

	update_ob_speed(op);

	op->face = op->arch->clone.face;
	op->owner = NULL; /* So that stopped arrows will be saved */
	update_object(op, UP_OBJ_FACE);
	return op;
}


/* stop_missile() - what to do when a non-living flying object
* has to stop. Sept 96 - I added in thrown object code in
* here too. -b.t.
*/
void stop_missile(object *op)
{
	play_sound_map(op->map, op->x, op->y, SOUND_DROP_THROW, SOUND_NORMAL);
	CLEAR_FLAG(op, FLAG_IS_MISSILE);
	if (op->inv)
	{
		object *payload = op->inv;

		remove_ob(payload);
		check_walk_off(payload, NULL, MOVE_APPLY_VANISHED);

		trigger_object_plugin_event(EVENT_STOP,
			payload, NULL, NULL, NULL, NULL, NULL, NULL, SCRIPT_FIX_ALL);

		/* please note that the throw system >=B4 has no support for throw potions because
		 * we apply now the throw items (which will invoke the potions...)
		 */
		if (payload->type == POTION)
		{
			if (payload->stats.sp != SP_NO_SPELL && spells[payload->stats.sp].flags & SPELL_DESC_DIRECTION)
				cast_spell(payload, payload, payload->direction, payload->stats.sp, 1, spellPotion, NULL); /* apply potion ALWAYS fire on the spot the applier stands - good for healing - bad for firestorm */
		}
		else
		{
			clear_owner(payload);
			/* Gecko: if the script didn't put the payload somewhere else */
			if (!payload->env && QUERY_FLAG(payload, FLAG_STAND_STILL) && !OBJECT_FREE(payload))
			{
				payload->x = op->x;
				payload->y = op->y;
				insert_ob_in_map(payload, op->map, payload, 0);
			}
		}
		remove_ob(op);
		check_walk_off(op, NULL, MOVE_APPLY_VANISHED);
	}
	else
	{
		op = fix_stopped_missile(op);
		if (op)
			(void)merge_ob(op, NULL);
	}
}


/* this is the old do_throw() code, using a payload object system.
* as useless for real, arrow based throw items as useful for throwing
* "non weapon items" like chairs or bottles.
* we will add it later to the game again when needed.
*/
#if 0

/* make_throw_ob() We construct the 'carrier' object in
* which we will insert the object that is being thrown.
* This combination  becomes the 'thrown object'. -b.t.
*/
static inline object * make_throw_ob(object *orig)
{
	object *toss_item   = NULL;

	if (orig)
	{
		toss_item = get_object();
		CLEAR_FLAG(orig, FLAG_APPLIED); /* in b4 throw/arrows are most times applied */
		copy_object(orig, toss_item);
		toss_item->type = THROWN_OBJ;
		SET_FLAG(toss_item, FLAG_IS_MISSILE);
		SET_FLAG(toss_item, FLAG_NO_PICK); /* you can *never* catch or pickup flying objects */
		CLEAR_FLAG(toss_item, FLAG_CHANGING);
		toss_item->stats.dam = 0; /* default damage */
		insert_ob_in_ob(orig, toss_item);
	}

	return toss_item;
}

int do_throw(object *thrower, object *toss_item, int dir)
{
	int ticks;
	object *left_cont, *throw_ob = toss_item, *left = NULL, *tmp_op;
	tag_t   left_tag;
	int     eff_str = 0, str = thrower->stats.Str, dam = 0, weight_f = 0;
#if 0 /* disabled, see in the code */
	int     target_throw    = 0;
	rv_vector target_vec;
#endif
	float   str_factor = 1.0f, load_factor = 1.0f, item_factor = 1.0f;

	if (throw_ob == NULL)
	{
		if (thrower->type == PLAYER)
			new_draw_info(NDI_UNIQUE, 0, thrower, "You have nothing to throw.");
		return 0;
	}
	if (QUERY_FLAG(throw_ob, FLAG_STARTEQUIP))
	{
		if (thrower->type == PLAYER)
			new_draw_info(NDI_UNIQUE, 0, thrower, "The gods won't let you throw that.");
		return 0;
	}

	/* 2 things here prevent a throw:
	* you aimed at your feet, or you threw at a wall
	*/
	if (!dir || wall(thrower->map, thrower->x + freearr_x[dir], thrower->y + freearr_y[dir]))
	{
		if (thrower->type == PLAYER)
			new_draw_info(NDI_UNIQUE, 0, thrower, "Something is in the way.");
		return 0;
	}

	/* sometimes get_split_ob can't split an object (because thrower->nrof==0?)
	* and returns NULL. We must use 'left' then
	*/

	left = throw_ob; /* these are throwing objects left to the player */
	left_cont = left->env;
	left_tag = left->count;

	/* split object will handle weight and/or removed item from player and inventory */
	if(!(throw_ob = get_split_ob(throw_ob, 1)))
		return 0; /* its a bug, get_split_ob() will LOG which one */

	/* special case: throwing powdery substances like dust, dirt */
	if (QUERY_FLAG(throw_ob, FLAG_DUST))
	{
		cast_dust(thrower, throw_ob, dir);
		return 0;
	}

	toss_item = throw_ob; /* toss_item is our real throw item */

	/* Gecko: disabled for now. Too much work to remove it, especially if it is accepted =) */
#if 0
	/* Experimental targetting throw hack */
	if(! dir && thrower->type == PLAYER && OBJECT_VALID(CONTR(op)->target_object, CONTR(op)->target_object_count)) {
		object *target = CONTR(op)->target_object;
		get_rangevector(op, target, &target_vec, 0);
		dir = target_vec.direction;
		target_throw = 1;
	} else
		throw_ob->stats.grace = 0;
#endif

	/* Make a thrown object -- insert real object in a 'carrier' object.
	* If unsuccessfull at making the "thrown_obj", we just reinsert
	* the original object back into inventory and exit
	*/
	if ((tmp_op = make_throw_ob(throw_ob)))
		throw_ob = tmp_op;
	else
	{
		insert_ob_in_ob(throw_ob, thrower);
		return 0;
	}

	/* At some point in the attack code, the actual real object (thrower->inv or toss_item)
	* becomes the hitter.  As such, we need to make sure that has a proper
	* owner value so exp goes to the right place.
	*/
	set_owner(throw_ob, thrower);
	set_owner(throw_ob->inv, thrower);
	throw_ob->direction = dir;
	throw_ob->x = thrower->x;
	throw_ob->y = thrower->y;

	/* Gecko: disabled for now. Too much work to remove it, especially if it is accepted =) */
#if 0
	/* Experimental targetting throw hack */
	if(target_throw) {
		int dx = target_vec.distance_x; /* hp */
		int dy = target_vec.distance_y; /* sp */
		int stepx, stepy;           /* maxhp, maxsp */

		if (dy < 0) { dy = -dy;  stepy = -1; } else { stepy = 1; }
		if (dx < 0) { dx = -dx;  stepx = -1; } else { stepx = 1; }

		throw_ob->stats.hp = dx << 1;
		throw_ob->stats.sp = dy << 1;
		throw_ob->stats.maxhp = stepx;
		throw_ob->stats.maxsp = stepy;
		throw_ob->stats.grace = 666; /* target-throw marker =) TODO: probably better to use ob->enemy instead */
		if(dx > dy) { /* fraction */
			throw_ob->stats.exp = (dy << 1) - dx;  /* same as 2*dy - dx */
		} else {
			throw_ob->stats.exp = (dx << 1) - dy;  /* same as 2*dx - dy */
		}
	}
#endif

	throw_ob->stats.wc = toss_item->stats.wc; /* we get the wc from the item */

	if(thrower->type == PLAYER)
	{
		/* for players count in a skill adjustment for the wc */
		throw_ob->stats.wc += thrower->chosen_skill->stats.wc;

		/* the new damage code... we use the same as in fix_player for normal weapons */
		throw_ob->stats.dam = (sint16) ((float) toss_item->stats.dam * LEVEL_DAMAGE(thrower->chosen_skill->level));

		/* boni for throwing are 50% str + 50% dex (archery 100% dex, melee 100% str) */
		throw_ob->stats.dam = (int)(((float)throw_ob->stats.dam * 
			(get_player_stat_bonus(thrower->stats.Str)/2.0f + get_player_stat_bonus(thrower->stats.Dex)/2.0f)) /10.0f);

		/* thats the damage bonus from equipment like rings */
		throw_ob->stats.dam += CONTR(thrower)->dam_bonus;
		if (throw_ob->stats.dam < 0)
			throw_ob->stats.dam = 0;
	}
	else
	{
		/* for mobs we simply get the mob level for the dmg adjustment */
		throw_ob->stats.dam = (sint16) ((float) toss_item->stats.dam * LEVEL_DAMAGE(thrower->level));
	}

	throw_ob->stats.dam = (sint16)((float) throw_ob->stats.dam * ((float)toss_item->item_condition/100.0f)); /* and finally the item condition! (mob & players) */

	new_draw_info_format(NDI_UNIQUE, 0, thrower, "throwDAM: action_t=%d skill=%d dam:%d wc:%d",toss_item->last_grace, thrower->chosen_skill->last_grace, throw_ob->stats.dam, throw_ob->stats.wc);

	ticks = toss_item->last_grace + thrower->chosen_skill->last_grace;
	/* chance of breaking. Proportional to force used and weight of item */
	throw_ob->stats.food = 0;
	//throw_ob->stats.food = (dam / 2) + (throw_ob->weight / 60000);

	if (QUERY_FLAG(toss_item, FLAG_IS_THROWN))
	{
		throw_ob->stats.grace = throw_ob->last_sp;
		throw_ob->stats.maxgrace = 60 + (RANDOM() % 12);

		/* only throw objects get directional faces */
		if (GET_ANIM_ID(throw_ob) && NUM_ANIMATIONS(throw_ob))
			SET_ANIMATION(throw_ob, (NUM_ANIMATIONS(throw_ob) / NUM_FACINGS(throw_ob)) * dir);

	}

	/* Gecko: Had to make sure the thrown object inherited event objects */
	if(trigger_object_plugin_event(EVENT_THROW, toss_item, thrower, NULL,
		NULL, NULL, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
		return ticks; /* TODO: needs testing */

	throw_ob->speed = 1.0f;
	throw_ob->speed_left = 0;
	update_ob_speed(throw_ob);
	throw_ob->map = thrower->map;

	SET_MULTI_FLAG(throw_ob, FLAG_FLYING);
	SET_FLAG(throw_ob, FLAG_FLY_ON);
	SET_FLAG(throw_ob, FLAG_WALK_ON);
	SET_FLAG(throw_ob, FLAG_NO_PICK);

	play_sound_map(thrower->map, thrower->x, thrower->y, SOUND_THROW, SOUND_NORMAL);

	if (insert_ob_in_map(throw_ob, thrower->map, thrower, 0))
		move_missile(throw_ob);

	return ticks;
}
#endif
