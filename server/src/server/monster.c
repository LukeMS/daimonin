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

static int tdir1;
static int tdir2;
static int tdir3;
static int tdir4;
static int tdir5;
static int tdir6;
static int tdir7;
static int tdir8;
static int tdir9;
static int tdir10;
static int tdir11;
static int tdir12;
static int tdir13;
static int tdir14;
static int tdir15;
static int tdir16;
static int tdir17;
static int tdir18;
static int tdir19;

/* Maximum number of ticks a mob remembers an object that it can't see */
#define MAX_KNOWN_OBJ_AGE 200

/* Maximum number of ticks we trust an older rangevector to a known object */
#define MAX_KNOWN_OBJ_RV_AGE 1

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

rv_vector *get_known_obj_rv(object *op, struct mob_known_obj *known_obj, int maxage);

/* Beginnings of can_see_obj */
/* known_obj is optional but increases efficiency somewhat
 * by using caching data in the known_obj struct
 */
int mob_can_see_obj(object *op, object *obj, struct mob_known_obj *known_obj) {
    static int aggro_range, stealth_range;
    static int cached_count = 0;

    rv_vector rv, *rv_p = NULL;
    
    /* Quick answer if possible */
    if(known_obj && known_obj->last_seen == global_round_tag)
        return TRUE;
    
    if(QUERY_FLAG(obj, FLAG_IS_INVISIBLE) && !QUERY_FLAG(op, FLAG_SEE_INVISIBLE))
        return FALSE;

    /* Avoid recalculating these values multiple times */
    if(cached_count != (int) op->count) 
    {
        aggro_range = op->stats.Wis; /* wis is basic sensing range */
        
        /* Extra range to alerted monsters */
        if(op->enemy)
            aggro_range += 3;
        
        /* Much less range if asleep or blind */
        if(QUERY_FLAG(op, FLAG_SLEEP) || QUERY_FLAG(op, FLAG_BLIND)) 
            aggro_range /= 2;
    
        /* Alternative sensing range for stealthy targets */
        stealth_range = MAX(MIN_MON_RADIUS, aggro_range - 2);
        cached_count = op->count;
    }

    /* Get the rangevector, trying to use a cached version first */
    if(known_obj) 
        rv_p = get_known_obj_rv(op, known_obj, MAX_KNOWN_OBJ_RV_AGE);
    else 
        if(get_rangevector(op, obj, &rv, 0))
            rv_p = &rv;
    if(rv_p == NULL)
        return FALSE;

    /* Check detection distance */
    if((int) rv_p->distance > (QUERY_FLAG(obj, FLAG_STEALTH) ? stealth_range : aggro_range)) 
        return FALSE;
            
    /* TODO also test darkness, stealth detection, LOS etc */
    return TRUE;
}

/* TODO: these two new functions are already obsolete and should be replaced 
 * with configuration options in the ai_friendship behaviour,
 * but they serve well as a base...
 */
int is_enemy_of(object *op, object *obj) {
    /* TODO: add a few other odd types here, such as god & golem */
    if(!obj->type == PLAYER || !obj->type == MONSTER || op == obj)
        return FALSE;

    /* Unagressive mobs are never enemies to anything (?) */
    if(QUERY_FLAG(op, FLAG_UNAGGRESSIVE) || QUERY_FLAG(obj, FLAG_UNAGGRESSIVE)) 
        return FALSE;

    /* TODO: this needs to be sorted out better */
    if(QUERY_FLAG(op, FLAG_FRIENDLY)) {
        if(QUERY_FLAG(obj, FLAG_MONSTER) && !QUERY_FLAG(obj, FLAG_FRIENDLY))
            return TRUE;
    } else {
        if(QUERY_FLAG(obj, FLAG_FRIENDLY) || obj->type == PLAYER)
            return TRUE;
    }

    return FALSE;
}

int is_friend_of(object *op, object *obj) {
    /* TODO: add a few other odd types here, such as god & golem */
    if(!obj->type == PLAYER || !obj->type == MONSTER || op == obj)
        return FALSE;

    /* TODO: this needs to be sorted out better */
    if(QUERY_FLAG(op, FLAG_FRIENDLY)) {
        if(!QUERY_FLAG(obj, FLAG_MONSTER) || QUERY_FLAG(obj, FLAG_FRIENDLY))
            return TRUE;
    } else {
        if(!QUERY_FLAG(obj, FLAG_FRIENDLY) && obj->type != PLAYER)
            return TRUE;
    }

    return FALSE;
}

/* Destructors for the mm system */
void cleanup_mob_known_obj(struct mob_known_obj *data) 
{
    FREE_ONLY_HASH(data->last_map);   
}

void cleanup_mob_data(struct mobdata *data) 
{
    struct mob_known_obj *tmp;

    if(data->pathfinding.path)
        free_path(data->pathfinding.path); 
    FREE_ONLY_HASH(data->pathfinding.goal_map);   
    FREE_ONLY_HASH(data->pathfinding.target_map);   

    for(tmp = data->known_mobs; tmp; tmp = tmp->next)
        return_poolchunk(tmp, POOL_MOB_KNOWN_OBJ);
    for(tmp = data->known_objs; tmp; tmp = tmp->next)
        return_poolchunk(tmp, POOL_MOB_KNOWN_OBJ);
}

/* Initializator for the mm system */
void initialize_mob_data(struct mobdata *data)
{
    data->pathfinding.target_obj = NULL;
    data->pathfinding.target_map = NULL;
    data->pathfinding.path_requested = FALSE;
    data->pathfinding.path = NULL;
    data->pathfinding.goal_map = NULL;
    data->pathfinding.best_distance = -1;
    data->pathfinding.tried_steps = 0;

    data->known_mobs = NULL;
    data->known_objs = NULL;

    data->leader = NULL;
    data->enemy = NULL;
}


/*
 * Get the rangevector to a known object. If an earlier calculated rangevector is 
 * older than maxage then we calculate a new one (set maxage to 0 to force update).
 * Returns a pointer to the rangevector, or NULL if get_rangevector() failed.
 */
rv_vector *get_known_obj_rv(object *op, struct mob_known_obj *known_obj, int maxage) 
{
    if(op == NULL || known_obj == NULL)
        return NULL;    
    
    if(global_round_tag-known_obj->rv_time >= (uint32)maxage || 
            known_obj->rv_time == 0 || maxage == 0)
    {        
        /*
        if(!mob_can_see_obj(op, known_obj->obj, NULL)) {
            mapstruct *map = ready_map_name(known_obj->last_map, MAP_NAME_SHARED);
            if(get_rangevector_from_mapcoords(op->map, op->x, op->y,
                        map, known_obj->last_x, known_obj->last_y, 
                        &known_obj->rv, RV_EUCLIDIAN_DISTANCE)) 
            {
                known_obj->rv_time = global_round_tag;
            } else 
            {
                known_obj->rv_time = 0;
                return NULL;
            }
        }
        */
        
        if(get_rangevector(op, known_obj->obj, &known_obj->rv, 0)) 
        {
            known_obj->rv_time = global_round_tag;
        } else 
        {
            known_obj->rv_time = 0;
            return NULL;
        }
    }

	/* hotfix for this bug. part should here NOT be NULL */
	if(!known_obj->rv.part)
	{
		LOG(-1,"CRASHBUG: rv->part == NULL for %s on map %s with enemy %s and map %s\n", 
				query_name(op), op->map?STRING_SAFE(op->map->path):"NULL", 
				query_name(known_obj->obj), 
				known_obj->obj?STRING_SAFE(known_obj->obj->map?STRING_SAFE(known_obj->obj->map->path):"NULL"):"NULL");
		return NULL;
	}
    return &known_obj->rv;
}

/* TODO: make a real behaviour... */
#if 0
void npc_call_for_help(object *op) {
  struct mob_known_obj *friend;
  
  /* TODO: remember to check that the called has MOB_DATA set up before doing anything else... */
  /* TODO: use tmp_friendship? */
  for(friend = MOB_DATA(op)->friends; friend; friend=friend->next) {
      if(friend->friendship >= FRIENDSHIP_HELP && friend->ob->enemy == NULL)  {
          rv_vector *rv = get_known_obj_rv(op, friend, MAX_KNOWN_OBJ_RV_AGE);
          if(rv && rv->distance < 4) {
              /* TODO: also check reverse friendship */
              /* TODO: -friendship here dependant on +friendship towards tmp */
              register_npc_known_obj(friend->ob, op->enemy, FRIENDSHIP_ATTACK);
          }
      }
  }
}
#endif

/* register a new enemy or friend for the NPC */
struct mob_known_obj *register_npc_known_obj(object *npc, object *other, 
        int friendship)
{
    struct mob_known_obj *tmp;
    struct mob_known_obj *last = NULL;

    if(npc == NULL) {
        LOG(llevDebug,"register_npc_known_obj(): Called with NULL npc obj\n");
        return NULL;
    }
    
    if(other == NULL) {
        LOG(llevDebug,"register_npc_known_obj(): Called with NULL other obj\n");
        return NULL;
    }
    
    if(npc == other) {
        LOG(llevDebug,"register_npc_known_obj(): Called for itself '%s'\n",
                STRING_OBJ_NAME(npc));
        return NULL;
    }

    /* this is really needed. 
	 * a hitter object can be a "system object" ...even a object
	 * in the inventory of npc (like a disease). These objects have
	 * usually no map.
	 */
	if(other->type != PLAYER && !QUERY_FLAG(other,FLAG_ALIVE))
	{
        LOG(llevDebug,"register_npc_known_obj(): Called for non PLAYER/IS_ALIVE '%s'\n",
		STRING_OBJ_NAME(npc));
		return NULL;
	}
    if(npc->type != MONSTER) {
        LOG(llevDebug,"register_npc_known_obj(): Called on non-mob object '%s'\n",
                STRING_OBJ_NAME(npc));
        return NULL;
    }    
    
    /* this check will hopefully be unnecessary in the future */
    if(MOB_DATA(npc) == NULL) {
        LOG(llevDebug,"register_npc_known_obj(): No mobdata (yet) for '%s'\n",  STRING_OBJ_NAME(npc));
        return NULL;
    }

    /* TODO: get rid of flag_unaggressive and use only friendship */
    if(friendship < 0 && QUERY_FLAG(npc, FLAG_UNAGGRESSIVE)) {
        CLEAR_FLAG(npc, FLAG_UNAGGRESSIVE);
        friendship += FRIENDSHIP_ATTACK;
    }
    
    /* Does npc already know this other? */
    for(tmp = MOB_DATA(npc)->known_mobs; tmp; tmp=tmp->next) {
        if(tmp->obj == other && tmp->obj_count == other->count) {
            tmp->last_seen = global_round_tag;
            FREE_AND_ADD_REF_HASH(tmp->last_map, other->map->path);
            tmp->last_x = other->x;
            tmp->last_y = other->y;
            tmp->friendship += friendship;
/*            if(friendship)
                LOG(llevDebug,"register_npc_known_obj(): '%s' changed mind about '%s'. friendship: %d -> %d\n",  STRING_OBJ_NAME(npc), STRING_OBJ_NAME(other), tmp->friendship - friendship, tmp->friendship);*/
            return tmp;
        }
        last = tmp;
    }
    
    /* TODO: keep count of enemies and push out less
     * important if new ones are added beyond a reasonable max number */
    
    /* No, it is new */
    tmp = get_poolchunk(POOL_MOB_KNOWN_OBJ);
    tmp->next = NULL;
    tmp->prev = last;
    tmp->obj = other;
    tmp->obj_count = other->count;
    
    
    tmp->last_map = add_refcount(other->map->path);
    tmp->last_x = other->x;
    tmp->last_y = other->y;
 
    tmp->last_seen = global_round_tag;
    tmp->rv_time = 0; /* Makes cached rv invalid */

    tmp->friendship = friendship;
    tmp->attraction = 0;
    tmp->tmp_friendship = 0;
    tmp->tmp_attraction = 0;

    /* Insert last in list of known objects */
    if(last)
        last->next = tmp;
    else
        MOB_DATA(npc)->known_mobs = tmp;        
    
//    LOG(llevDebug,"register_npc_known_obj(): '%s' detected '%s'. friendship: %d\n",  STRING_OBJ_NAME(npc), STRING_OBJ_NAME(other), tmp->friendship);

    return tmp;
}

/*
 * Waypoint utility functions 
 */

/* Find a monster's currently active waypoint, if any */
object *get_active_waypoint(object *op) {
  object *wp = NULL;
  
  for(wp=op->inv;wp!=NULL;wp=wp->below)
    if(wp->type == TYPE_WAYPOINT_OBJECT && QUERY_FLAG(wp, WP_FLAG_ACTIVE)) 
      break;
  
  return wp;
}

/* Find a monster's current return-home wp, if any */
object *get_return_waypoint(object *op) {
  object *wp = NULL;
  
  for(wp=op->inv;wp!=NULL;wp=wp->below)
    if(wp->type == TYPE_WAYPOINT_OBJECT && QUERY_FLAG(wp, FLAG_REFLECTING)) 
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

/*
 * Some other utility functions
 */

int can_hit_melee(object *ob1,object *ob2, rv_vector *rv) {
    if(QUERY_FLAG(ob1,FLAG_CONFUSED)&&!(RANDOM()%3))
        return 0;
    return abs(rv->distance_x)<2&&abs(rv->distance_y)<2;
}

/* modes: 
 * 1 - exact 45 deg
 * 2 - 45 deg +- one tile
 * 3 - free 360 deg LOF
 */
int can_hit_missile(object *ob1, object *ob2, rv_vector *rv, int mode) {
    /* TODO: actually perform a rough line of sight calculation */
    
    switch(mode) {
        case 1: /* exact 45 deg */
        default:
            return rv->distance_x == 0 || rv->distance_y == 0 ||
                abs(rv->distance_x)-abs(rv->distance_y) == 0;                
        case 2: /* 45 deg +- one tile */
            return abs(rv->distance_x) <= 1 || abs(rv->distance_y) <= 1 ||
                abs(abs(rv->distance_x)-abs(rv->distance_y)) <= 1;

        case 3: /* free 360 deg line of fire */
            return TRUE;            
    }
}

/* Normalize a given map path and make sure it is valid and 
 * that the map is loaded. Can return NULL in case of failure */
static inline mapstruct *normalize_and_ready_map(mapstruct *defmap, const char **path) 
{
    /* Default map is current map */
    if(path == NULL || *path==NULL || **path == '\0') 
        return defmap; 

    /* If path not normalized: normalize it */
    if(**path != '/') { 
        char temp_path[HUGE_BUF];
        normalize_path(defmap->path, *path, temp_path);
        FREE_AND_COPY_HASH(*path, temp_path);
    }

    /* check if we are already on the map */
    if(*path == defmap->path)
        return defmap;
    else
        return ready_map_name(*path, MAP_NAME_SHARED);
}

/* scary function - need rework. even in crossfire its changed now */
void monster_check_apply(object *mon, object *item) {
  /* this function is simply to bad - for example will potions applied
   * not depending on the situation... why applying a heal potion when
   * full hp? firestorm potion when standing next to own people?
   * IF we do some AI stuff here like using items we must FIRST
   * add a AI - then doing the things. Think first, act later!
   */
}

/*
 * Mutually exclusive movement behaviours 
 */

void ai_stand_still(object *op, move_response *response) 
{
    if(QUERY_FLAG(op, FLAG_STAND_STILL)) {
        response->type = MOVE_RESPONSE_DIR;
        response->data.direction = 0;
    }
}

void ai_sleep(object *op, move_response *response) 
{
    if(QUERY_FLAG(op, FLAG_SLEEP)) {
        if(op->enemy)
           CLEAR_FLAG(op, FLAG_SLEEP);
        else {
            response->type = MOVE_RESPONSE_DIR;
            response->data.direction = 0;
        }
    }
}

void ai_move_randomly(object *op, move_response *response) {
    int i, r;
	object *base = find_base_info_object(op);
    
    /* TODO: support random move area limits as behaviour params */
    /* Give up to 8 chances for a monster to move randomly */
    for(i=0;i<8;i++) {
        r = RANDOM()%8+1;

        /* TODO: doesn't handle map borders */
        if(op->item_race!=255) /* check x direction of possible move */
            if(abs(op->x+freearr_x[r]-base->x) >op->item_race)
                continue;
        if(op->item_level!=255) /* check x direction of possible move */
            if(abs(op->y+freearr_y[r]-base->y) >op->item_level)
                continue;
        
        if(! blocked_link(op, freearr_x[r], freearr_y[r])) {
            response->type = MOVE_RESPONSE_DIR;
            response->data.direction = r;
            return;
        }
    }
}

void ai_move_towards_home(object *op, move_response *response) 
{
    /* TODO: optimization: pointer to the base ob in mob_data */
    object *base = insert_base_info_object(op); 

    if(base && base->slaying) 
    {
        /* If mob isn't already home */
        if(op->x != base->x || op->y != base->y || op->map->path != base->slaying) 
        {
            mapstruct *map = normalize_and_ready_map(op->map, &base->slaying);

            response->type = MOVE_RESPONSE_COORD;
            response->data.coord.x = base->x;
            response->data.coord.y = base->y;
            response->data.coord.map = map;
        }
    } 
}

void ai_move_towards_enemy(object *op, move_response *response) 
{
    if(QUERY_FLAG(op, FLAG_NO_ATTACK))
        return;
    
    if(OBJECT_VALID(op->enemy, op->enemy_count) && mob_can_see_obj(op, op->enemy, MOB_DATA(op)->enemy)) {
        rv_vector *rv = get_known_obj_rv(op, MOB_DATA(op)->enemy, MAX_KNOWN_OBJ_RV_AGE);
        /* TODO: if we can't see op->enemy, goto last known position, or something... ( or do nothing, and make that a separate behaviour ) */
 
        if(rv != NULL) {
            if(rv->distance > 1) {
                response->type = MOVE_RESPONSE_OBJECT;
                response->data.target.obj = op->enemy;
                response->data.target.obj_count = op->enemy_count;
            } else {
                /* Stay where we are */
                response->type = MOVE_RESPONSE_DIR;
                response->data.direction=0;
            }
        }
    } 
}

void ai_move_towards_enemy_last_known_pos(object *op, move_response *response) 
{    
    if(QUERY_FLAG(op, FLAG_NO_ATTACK))
        return;
    
    if(OBJECT_VALID(op->enemy, op->enemy_count) && MOB_DATA(op)->enemy->last_map)
    {                
        rv_vector rv;
        struct mob_known_obj *enemy = MOB_DATA(op)->enemy;
        mapstruct *map = ready_map_name(enemy->last_map, MAP_NAME_SHARED);
 
        if(get_rangevector_from_mapcoords(op->map, op->x, op->y,
                    map, enemy->last_x, enemy->last_y, 
                    &rv, RV_EUCLIDIAN_DISTANCE)) 
        {
            if(rv.distance > 3) {
                response->type = MOVE_RESPONSE_COORD;
                response->data.coord.x = enemy->last_x;
                response->data.coord.y = enemy->last_y;
                response->data.coord.map = map;
            } 
        }
    } 
}

/* Stupid behaviour that moves around randomly looking for a lost enemy */
void ai_search_for_lost_enemy(object *op, move_response *response) 
{    
    if(QUERY_FLAG(op, FLAG_NO_ATTACK))
        return;
    
    if(OBJECT_VALID(op->enemy, op->enemy_count) && MOB_DATA(op)->enemy->last_map)
    {
        int i, r;

        /* Give up to 8 chances for a monster to move randomly */
        for(i=0;i<8;i++) {
            r = RANDOM()%8+1;

            if(! blocked_link(op, freearr_x[r], freearr_y[r])) {
                response->type = MOVE_RESPONSE_DIR;
                response->data.direction = r;
                return;
            }
        }
    }
}

void ai_move_towards_waypoint(object *op, move_response *response) 
{
    object *wp;
    rv_vector rv;
    int try_next_wp = 0;
    
    wp = get_active_waypoint(op);
    if(wp) {
        mapstruct *destmap = normalize_and_ready_map(op->map, &WP_MAP(wp));
        if(destmap) {
            /* We know which map we want to. Can we figure out where that
             * map lies relative to current position? */
            
            /* This rv may be computed several times, this is generally
             * not a performance problem, since the cache in the recursive
             * search usually catches that.
             * TODO: extend cache in recursive search with longer memory
             */
            if(! get_rangevector_from_mapcoords(op->map, op->x, op->y,
                        destmap, WP_X(wp), WP_Y(wp), &rv,
                        RV_RECURSIVE_SEARCH | RV_DIAGONAL_DISTANCE)) {
                /* Problem: we couldn't find a relative direction between the
                 * maps. Usually it means that they are in different mapsets
                 * or too far away from each other. */
                LOG(llevDebug,"ai_move_towards_waypoint(): No connection between maps: '%s' and '%s'\n", 
                        STRING_MAP_PATH(destmap), STRING_MAP_PATH(op->map));
                CLEAR_FLAG(wp, WP_FLAG_ACTIVE); /* disable this waypoint */
                try_next_wp = 1;
            } else {
                /* Good, we know general distance and direction to wp target */
                
                /* Are we close enough to accept the wp? */
                if(rv.distance <= (unsigned int) wp->stats.grace) {

                    /* Should we wait a little while? */
                    if(MOB_PATHDATA(op)->goal_delay_counter < WP_DELAYTIME(wp)) {
                        MOB_PATHDATA(op)->goal_delay_counter++;
                    } else {
                        LOG(llevDebug,"ai_move_towards_waypoint(): '%s' reached destination '%s'\n", STRING_OBJ_NAME(op), STRING_OBJ_NAME(wp));

#ifdef PLUGINS
                        /* GROS: Handle for plugin trigger event */
                        if(wp->event_flags&EVENT_FLAG_TRIGGER)
                        {
                            CFParm CFP;
                            CFParm* CFR;
                            int k, l, m;
                            int rtn_script = 0;
                            object *event_obj = get_event_object(wp, EVENT_TRIGGER);
                            m = 0;

                            k = EVENT_TRIGGER;
                            l = SCRIPT_FIX_NOTHING;
                            CFP.Value[0] = &k;
                            CFP.Value[1] = op;
                            CFP.Value[2] = wp;
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

                        MOB_PATHDATA(op)->goal_delay_counter = 0;
                        MOB_PATHDATA(op)->best_distance = -1;
                        MOB_PATHDATA(op)->tried_steps = 0;
                        CLEAR_FLAG(wp, WP_FLAG_ACTIVE);
                        try_next_wp = 1;               
                    }
                }
            }
        } else {
            LOG(llevDebug,"ai_move_towards_waypoint(): '%s' ('%s') no such map: '%s'\n", STRING_OBJ_NAME(op), STRING_OBJ_NAME(wp), STRING_WP_MAP(wp));
            CLEAR_FLAG(wp, WP_FLAG_ACTIVE);
            try_next_wp = 1;               
        }
    }
        
    /* If we reached or gave up on the current waypoint */
    if(try_next_wp && wp) {
        if(WP_NEXTWP(wp) && (wp = find_waypoint(op, WP_NEXTWP(wp)))) {
            LOG(llevDebug,"ai_move_towards_waypoint(): '%s' next WP: '%s'\n", STRING_OBJ_NAME(op), STRING_WP_NEXTWP(wp));
            SET_FLAG(wp, WP_FLAG_ACTIVE); /* activate new waypoint */
            MOB_PATHDATA(op)->best_distance = -1;
            MOB_PATHDATA(op)->tried_steps = 0;
        } else {
            LOG(llevDebug,"ai_move_towards_waypoint(): '%s' no next WP\n", STRING_OBJ_NAME(op));
            wp = NULL;
        }
    }

    if(wp) {
        response->type = MOVE_RESPONSE_WAYPOINT;
        response->data.target.obj = wp;
        response->data.target.obj_count = wp->count;
    } 
}

/* 
 * Runs away from enemy if scared
 * Sets scared if low hp
 * Clears scared if high enough hp OR after a random time
 */
void ai_run_away_from_enemy(object *op, move_response *response)
{
    rv_vector *rv;
    
    /* Is scared? */
    if(QUERY_FLAG(op, FLAG_SCARED) && op->enemy) 
    {
        if((rv = get_known_obj_rv(op, MOB_DATA(op)->enemy, MAX_KNOWN_OBJ_RV_AGE)))
        {
            /* TODO: more intelligent: use pathfinding to find the
             * most distant point from enemy */
            response->type = MOVE_RESPONSE_DIR;
            response->data.direction = absdir(rv->direction + 4);
        } else {
        }

        /* Regain senses? */
        if(op->run_away && op->stats.maxhp) 
        {
            /* Gecko: I added a slight hysteresis treshold here 
             * (stay afraid until hp reaches 2*runaway % of maxhp) */
            if(op->stats.hp == op->stats.maxhp || (op->stats.hp*100)/op->stats.maxhp > op->run_away * 2)
            {
                CLEAR_FLAG(op, FLAG_SCARED);
            }
        } else 
        {
            /* If we aren't scared because of low hp, we can stop
             * being afraid after a random delay */
            if(! RANDOM()%20)
                CLEAR_FLAG(op, FLAG_SCARED);        
        }
    } else {
        /* Become scared? */
        if(op->stats.maxhp && (op->stats.hp*100)/op->stats.maxhp < op->run_away) 
        {
            SET_FLAG(op, FLAG_SCARED);
        }
    }
}

/*
 * Misc behaviours
 */

void ai_look_for_other_mobs(object *op) 
{
    /* TODO Very stupid solution for now: scan through all active and
     * check if we can reach them. Makes more sense when we have
     * per-map active lists. */

    /* TODO possibility for optimization: if we already have enemies there
     * is no need to look for new ones every timestep... */
    object *obj;
    for(obj = active_objects; obj; obj=obj->active_next) 
    {
        if((QUERY_FLAG(obj, FLAG_ALIVE) || obj->type == PLAYER) && on_same_map(obj, op) && obj != op && mob_can_see_obj(op, obj, NULL)) 
        {
            /* TODO: get rid of double rv calculation 
             * (both can_see_obj() and register_npc_known_obj) 
             */
            register_npc_known_obj(op, obj, 0);
        }
    }   
}
    
/* Calculate friendship level of each known mob */
void ai_friendship(object *op)
{
    struct mob_known_obj *tmp;

    for(tmp = MOB_DATA(op)->known_mobs; tmp; tmp = tmp->next) {    
        tmp->tmp_friendship = tmp->friendship;
        
        /* Replace with flexible behaviour parameters */
        if(is_enemy_of(op, tmp->obj)) 
            tmp->tmp_friendship += FRIENDSHIP_ATTACK;
        else if(is_friend_of(op, tmp->obj))
            tmp->tmp_friendship += FRIENDSHIP_HELP;

        /* Now factor in distance and age of observation (preliminary method) */
        /* TODO: should probably use get_last_known_obj_rv() */
        //get_known_obj_rv(op, tmp, MAX_KNOWN_OBJ_RV_AGE);
        //tmp->tmp_friendship /= (int) MAX(tmp->rv.distance,1.0);
        /* TODO: test last_seen aging */
        //        tmp->tmp_friendship /= MAX(global_round_tag - tmp->last_seen, 1);
//        LOG(llevDebug,"ai_friendship(): '%s' -> '%s'. friendship: %d\n",  STRING_OBJ_NAME(op), STRING_OBJ_NAME(tmp->ob), tmp->tmp_friendship);
    }
}
    
void ai_choose_enemy(object *op)
{
    object *oldenemy = op->enemy;
    struct mob_known_obj *tmp, *worst_enemy = NULL;
    
    /* TODO: get rid of this flag */
    if(QUERY_FLAG(op, FLAG_NO_ATTACK))
        return;
    
    /* Go through list of known mobs and choose the most hated
     * that we can get to.
     */
    for(tmp = MOB_DATA(op)->known_mobs; tmp; tmp=tmp->next)
    {
        if(tmp->tmp_friendship < 0) 
        {
            /* Most hated enemy so far? */
            if((worst_enemy == NULL || tmp->tmp_friendship < worst_enemy->tmp_friendship))
            {
                /* Ignore if we can't get to it at all */
                /* TODO: should probably use get_last_known_obj_rv() */
                rv_vector *rv = get_known_obj_rv(op, tmp, MAX_KNOWN_OBJ_RV_AGE);
                if(rv) 
                    worst_enemy = tmp;
            }
        }
    }
   
    /* Did we find an enemy? */
    if(worst_enemy) {
//        LOG(llevDebug,"ai_choose_enemy(): %s's worst enemy is '%s', friendship: %d\n", STRING_OBJ_NAME(op), STRING_OBJ_NAME(worst_enemy->ob), worst_enemy->tmp_friendship);
        op->enemy = worst_enemy->obj;
        MOB_DATA(op)->enemy = worst_enemy;
        op->enemy_count = worst_enemy->obj_count;
    } else {
        op->enemy = NULL;
        MOB_DATA(op)->enemy = NULL;
    }
    
    /* Handle enemy switching (growl, speed up/down) */
    /* TODO: separate into another behaviour... */
    if(op->enemy != oldenemy) {
        if(op->enemy) {
            op->last_eat = 0;	/* important: thats our "we lose aggro count" - reset to zero here */
            if(!QUERY_FLAG(op,FLAG_FRIENDLY))
                play_sound_map(op->map, op->x, op->y, SOUND_GROWL, SOUND_NORMAL);

            /* The unaggressives look after themselves 8) */
            /* TODO: Make a separate behaviour... */
//            if(QUERY_FLAG(op,FLAG_UNAGGRESSIVE)) {
//                CLEAR_FLAG(op, FLAG_UNAGGRESSIVE);
//            npc_call_for_help(op);
//            }
        }
        set_mobile_speed(op, 0);
    }
}

/*
 * Attack behaviours
 */
int ai_melee_attack_enemy(object *op)
{
    rv_vector *rv;
    
    if(QUERY_FLAG(op, FLAG_UNAGGRESSIVE) || QUERY_FLAG(op, FLAG_SCARED) ||
            !OBJECT_VALID(op->enemy, op->enemy_count) || op->weapon_speed_left>0)
        return FALSE;
    
    /* TODO: choose another enemy if this fails */
    if(! (rv = get_known_obj_rv(op, MOB_DATA(op)->enemy, MAX_KNOWN_OBJ_RV_AGE)))
        return FALSE;
    if(! can_hit_melee(rv->part, op->enemy, rv) || !mob_can_see_obj(op, op->enemy, MOB_DATA(op)->enemy))
        return FALSE;
    
//    LOG(llevDebug,"ai_melee_attack_enemy(): '%s' -> '%s'\n", STRING_OBJ_NAME(op), STRING_OBJ_NAME(op->enemy));
    
    /* TODO: the following test should be done in skill_attack! */
    /* TODO: what if wc underflows? */
    if(QUERY_FLAG(op, FLAG_RUN_AWAY)) 
        rv->part->stats.wc -= 10;
    skill_attack(op->enemy,rv->part,0,NULL);
    op->weapon_speed_left+=FABS((int)op->weapon_speed_left)+1;
    if(QUERY_FLAG(op, FLAG_RUN_AWAY)) 
        rv->part->stats.wc += 10;
    
    return TRUE;
}

int ai_bow_attack_enemy(object *op)
{
    object *bow = NULL, *arrow = NULL, *target = op->enemy;
    rv_vector *rv;
    int tag;
    int direction;

    if(!QUERY_FLAG(op, FLAG_READY_BOW) ||
            QUERY_FLAG(op, FLAG_UNAGGRESSIVE) || QUERY_FLAG(op, FLAG_SCARED) ||
            !OBJECT_VALID(op->enemy, op->enemy_count) || op->weapon_speed_left>0)       
        return FALSE;
 
    /* TODO: choose another target if this or next test fails */
    if(! (rv = get_known_obj_rv(op, MOB_DATA(op)->enemy, MAX_KNOWN_OBJ_RV_AGE)))
        return FALSE;
    /* TODO: also check distance and LOS */
    if(! can_hit_missile(op, target, rv, 2) || !mob_can_see_obj(op, target, MOB_DATA(op)->enemy))
        return FALSE;
    
//    LOG(llevDebug,"ai_distance_attack_enemy(): '%s' -> '%s'\n", STRING_OBJ_NAME(op), STRING_OBJ_NAME(op->enemy));

    direction = rv->direction;
    if(QUERY_FLAG(op,FLAG_CONFUSED))
        direction = absdir(direction + RANDOM()%5 - 2);

    /* Find the applied bow */
    for(bow=op->inv;bow!=NULL;bow=bow->below)
        if(bow->type==BOW && QUERY_FLAG(bow,FLAG_APPLIED))
            break;
    
    if(bow==NULL) {
        LOG(llevBug,"BUG: Monster %s (%d) has READY_BOW without bow.\n",STRING_OBJ_NAME(op),op->count);
        CLEAR_FLAG(op, FLAG_READY_BOW);
        return 0;
    }

    /* Select suitable arrows */
    if((arrow=find_arrow(op,bow->race)) == NULL) {
        /* Out of arrows */
        manual_apply(op,bow,0);
        CLEAR_FLAG(op, FLAG_READY_BOW);
        return 0;
    }

    /* thats a infinitve arrow! dupe it. */
    if(QUERY_FLAG(arrow,FLAG_SYS_OBJECT) ) {
        object *new_arrow = get_object();
        copy_object(arrow,new_arrow);
        CLEAR_FLAG(new_arrow,FLAG_SYS_OBJECT);
        new_arrow->nrof=0;

        /* now setup the self destruction */
        new_arrow->stats.food = 20;
        arrow = new_arrow;
    } else
        arrow=get_split_ob(arrow,1);

    /* ugly nasty arrow-setting-upping block */
    set_owner(arrow,op);
    arrow->direction=direction;
    arrow->x=rv->part->x,arrow->y=rv->part->y;
    arrow->speed = 1;
    update_ob_speed(arrow);
    arrow->speed_left=0;
    SET_ANIMATION(arrow, (NUM_ANIMATIONS(arrow)/NUM_FACINGS(arrow))*rv->direction);
    arrow->level = op->level;
    arrow->last_heal = arrow->stats.wc; /* save original wc and dam */
    arrow->stats.hp = arrow->stats.dam; 
    arrow->stats.dam+= bow->stats.dam+bow->magic+arrow->magic; /* NO_STRENGTH */
    arrow->stats.dam=FABS((int)((float)(arrow->stats.dam  *lev_damage[op->level])));
    arrow->stats.wc= 10 + (bow->magic +bow->stats.wc + arrow->magic + arrow->stats.wc-op->level);
    arrow->stats.wc_range = bow->stats.wc_range;
    arrow->map=op->map;
    arrow->last_sp = 12; /* we use fixed value for mobs */
    SET_FLAG(arrow, FLAG_FLYING);
    SET_FLAG(arrow, FLAG_IS_MISSILE);
    SET_FLAG(arrow, FLAG_FLY_ON);
    SET_FLAG(arrow, FLAG_WALK_ON);
    tag = arrow->count;
    arrow->stats.grace = arrow->last_sp;
    arrow->stats.maxgrace = 60+(RANDOM()%12);

    if(insert_ob_in_map(arrow,op->map,op,0)) {
        if (!was_destroyed(arrow, tag)) {
            play_sound_map(arrow->map, arrow->x, arrow->y, SOUND_THROW, SOUND_NORMAL);
            move_arrow(arrow);
        }
    }
    
    op->weapon_speed_left+=FABS((int)op->weapon_speed_left)+1;

    return 1;
}

#define MAX_KNOWN_SPELLS 20

/* TODO: slightly rework this */
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

int ai_spell_attack_enemy(object *op)
{
    object *target = op->enemy;
    rv_vector *rv;
    spell *sp;
    object *spell_item;
    int sp_type, ability, direction, sp_cost;

    if(!QUERY_FLAG(op, FLAG_CAST_SPELL) ||
            QUERY_FLAG(op, FLAG_UNAGGRESSIVE) || QUERY_FLAG(op, FLAG_SCARED) ||
            !OBJECT_VALID(op->enemy, op->enemy_count) || op->weapon_speed_left>0 ||
            op->last_grace > 0) 
        return FALSE;
 
    /* TODO: choose another target if this or next test fails */
    if(! (rv = get_known_obj_rv(op, MOB_DATA(op)->enemy, MAX_KNOWN_OBJ_RV_AGE)))
        return FALSE;
    /* TODO: also check distance and LOS */
    /* TODO: should really check type of spell (area or missile) */
    if(! can_hit_missile(op, target, rv, 2) || !mob_can_see_obj(op, op->enemy, MOB_DATA(op)->enemy))
        return FALSE;
    
//    LOG(llevDebug,"ai_distance_attack_enemy(): '%s' -> '%s'\n", STRING_OBJ_NAME(op), STRING_OBJ_NAME(op->enemy));

    direction = rv->direction;
    if(QUERY_FLAG(op,FLAG_CONFUSED))
        direction = absdir(direction + RANDOM()%5 - 2);

    /* Find a reasonable spell  */
	if((spell_item=monster_choose_random_spell(op))==NULL) 
	{
        LOG(llevDebug,"ai_spell_attack_enemy() No spell found! Turned off spells in %s (%s) (%d,%d)\n",
                STRING_OBJ_NAME(op),STRING_MAP_NAME(op->map), op->x, op->y );
		CLEAR_FLAG(op, FLAG_CAST_SPELL); /* Will be turned on when picking up book */
		return 0;
	}
    
    if(spell_item->stats.hp)
	{
		/* Alternate long-range spell: check how far away enemy is */
		if(rv->distance>6)
			sp_type=spell_item->stats.hp;
		else
			sp_type=spell_item->stats.sp;
    } 
	else
		sp_type=spell_item->stats.sp;

    if((sp=find_spell(sp_type))==NULL)
	{
		LOG(llevDebug,"ai_spell_attack_enemy(): Can't find spell #%d for mob %s (%s) (%d,%d)\n", sp_type,
                STRING_OBJ_NAME(op),STRING_MAP_NAME(op->map), op->x, op->y );
		return 0;
    }
    
    sp_cost = SP_level_spellpoint_cost(op,op,sp_type);
    if(op->stats.sp < sp_cost)
        return FALSE;

    /* TODO: this kind of spells shouldn't be handled here... */
    if (sp->flags&SPELL_DESC_SELF) /* Spell should be cast on caster (ie, heal, strength) */
		direction = 0;
  
    ability = (spell_item->type==ABILITY && QUERY_FLAG(spell_item,FLAG_IS_MAGICAL) );

    /* If we cast a spell, only use up casting_time speed */
    op->speed_left += (float)1.0 - (float) spells[sp_type].time/(float)20.0*(float)FABS(op->speed); 

    op->stats.sp -= sp_cost;

	/* add default cast time from spell force to monster */
    /* TODO: what is this? */
	op->last_grace += spell_item->last_grace;
    
	/*LOG(-1,"CAST2: dir:%d (%d)- target:%s\n", dir, rv->direction, query_name(op->enemy) );*/
    /* TODO: what does the return value of cast_spell do ? */
    cast_spell(rv->part, rv->part, direction, sp_type, ability, spellNormal, NULL);

    return TRUE;
}
    
/*
 * Support functions for move_monster()
 */

static inline void regenerate_stats(object *op)
{
    /* (Not really a behaviour, but we keep it here anyway... ) */
    
    /*  generate hp, if applicable */
    if(op->stats.Con&&op->stats.hp<op->stats.maxhp) 
	{
        if(++op->last_heal>5)
        {
            op->last_heal = 0;
            op->stats.hp+=op->stats.Con;

            if(op->stats.hp>op->stats.maxhp)
                op->stats.hp=op->stats.maxhp;
        }

		/* So if the monster has gained enough HP that they are no longer afraid */
        /* TODO: should be handled elsewhere */
		if (QUERY_FLAG(op,FLAG_RUN_AWAY) &&
					op->stats.hp >= (signed short)(((float)op->run_away/(float)100)*(float)op->stats.maxhp))
			CLEAR_FLAG(op, FLAG_RUN_AWAY);
    }

    /* generate sp, if applicable */
    if(op->stats.Pow&&op->stats.sp<op->stats.maxsp)
	{
		op->last_sp+= (int)((float)(8*op->stats.Pow)/FABS(op->speed));
		op->stats.sp+=op->last_sp/128;  /* causes Pow/16 sp/tick */
		op->last_sp%=128;
		if(op->stats.sp>op->stats.maxsp)
		    op->stats.sp=op->stats.maxsp;
    }

    /* I think this is spell casting delay... */
    if(op->last_grace)
        op->last_grace--;
}

/* Get a direction from object op to object target, using precomputed paths
 * if available, and request path finding if needed */
static int calc_direction_towards(object *op, object *target, mapstruct *map, int x, int y) 
{
    struct mobdata_pathfinding *pf;
    mapstruct *path_map;
    rv_vector target_rv, segment_rv;

	target_rv.direction = 1234543;
	segment_rv.direction = 1234542;

    pf = MOB_PATHDATA(op);
    
    if(map == NULL) {
        LOG(llevBug,"BUG: invalid destination map for '%s'\n", STRING_OBJ_NAME(op)); 
        return 0;
    }
    
    /* Get general direction and distance to target */
    get_rangevector_from_mapcoords(
            op->map, op->x, op->y, map, x, y, 
            &target_rv, RV_RECURSIVE_SEARCH | RV_DIAGONAL_DISTANCE);
    /* TODO: verify results */
    /* if not on same map (or close) do something else... */
        
    /* Close enough already? */
    if(target_rv.distance <= 1) {
        if(target_rv.distance == 0)
            return 0;
        else
            return target_rv.direction;
    }

    /* Clean up old path */
    if(pf->path) {
        if(pf->target_obj != target || 
                ( target && pf->target_count != target->count) || 
                (!target && (pf->target_map != map->path || pf->target_x != x || pf->target_y != y))) 
        {
            free_path(pf->path);
            pf->path = NULL;
        }
    }
    
    /* No precomputed path (yet) ? */
    if(pf->path == NULL) {
        /* request new path */
        if(!pf->path_requested) 
        {
            pf->target_obj = target;
            if(target) 
            {
                pf->target_count = target->count;
                FREE_AND_CLEAR_HASH(pf->target_map);
            } else
            {
                FREE_AND_ADD_REF_HASH(pf->target_map, map->path);
                pf->target_x = x;
                pf->target_y = y;
            }
            
#ifdef DEBUG_PATHFINDING
            LOG(llevDebug,"calc_direction_towards() path=NULL '%s'->'%s'\n", STRING_OBJ_NAME(op), STRING_OBJ_NAME(target));
#endif
            request_new_path(op);
        }
        
        /* Take a first guesstimate step */
        return target_rv.direction;
    } 
    
    path_map = ready_map_name(pf->path->map, MAP_NAME_SHARED);
    /* Walk towards next precomputed coordinate */
    get_rangevector_from_mapcoords(
            op->map, op->x, op->y, 
            path_map, pf->path->x, pf->path->y, 
            &segment_rv, RV_RECURSIVE_SEARCH | RV_DIAGONAL_DISTANCE);
    /* TODO check result */

    /* throw away segment if we are finished with it */
    if(segment_rv.distance <= 1 && pf->path != NULL) {
        return_poolchunk(pf->path, POOL_PATHSEGMENT);
        pf->path = pf->path->next; /* assuming poolchunk is still valid */
        pf->tried_steps = 0;
        pf->best_distance = -1;
    }
    
    if((int)segment_rv.distance<pf->best_distance || pf->best_distance == -1) {
        /* If we got closer: store closest distance & reset timeout */
        pf->best_distance = segment_rv.distance;
        pf->tried_steps = 0;
    } else if (pf->tried_steps++ > WP_MOVE_TRIES) {
        /* If not got closer for a while: ask for a new path */
        pf->target_obj = target;
        if(target) 
        {
            FREE_AND_CLEAR_HASH(pf->target_map);
            pf->target_count = target->count;
        } else
        {
            FREE_AND_ADD_REF_HASH(pf->target_map, map->path);
            pf->target_x = x;
            pf->target_y = y;
        }
        if(!pf->path_requested) {
#ifdef DEBUG_PATHFINDING
            LOG(llevDebug,"calc_direction_towards() timeout '%s'->'%s'\n", STRING_OBJ_NAME(op), STRING_OBJ_NAME(target));
#endif
            request_new_path(op);
        }
    }

    return segment_rv.direction;
}

static int calc_direction_towards_coord(object *op, mapstruct *map, int x, int y) 
{
    return calc_direction_towards(op, NULL, map, x, y);
}

static int calc_direction_towards_object(object *op, object *target) {
    /* Request new path if target has moved too much */
    if (MOB_PATHDATA(op)->path && MOB_PATHDATA(op)->goal_map &&   
            (target->map->path != MOB_PATHDATA(op)->goal_map ||
             target->x != MOB_PATHDATA(op)->goal_x ||
             target->y != MOB_PATHDATA(op)->goal_y )) {
        rv_vector rv_goal, rv_target;
        mapstruct *goal_map = ready_map_name(MOB_PATHDATA(op)->goal_map, MAP_NAME_SHARED);

		if(!goal_map)
		{
			LOG(llevDebug,"BUGBUG: calc_direction_towards_object(): goal_map == NULL (%s <->%s)\n", STRING_OBJ_NAME(op), STRING_OBJ_NAME(target));
			return 0;
		}
        
        /* TODO if we can't see the object, goto its last known position
         * (also have to separate between well-known objects that we can find
         * without seeing, and other objects that we have to search or track */
        /* TODO make sure maps are loaded (here and everywhere else) */
        if(get_rangevector_from_mapcoords(target->map, target->x, target->y, 
                    goal_map, MOB_PATHDATA(op)->goal_x, MOB_PATHDATA(op)->goal_y, 
                    &rv_goal, RV_DIAGONAL_DISTANCE) &&
                get_rangevector_from_mapcoords(op->map, op->x, op->y,
                    target->map, target->x, target->y, &rv_target, RV_DIAGONAL_DISTANCE)) 
        {
            /* Heuristic: if dist(target, path goal) > dist(target, self) 
             * then get a new path */
            if(rv_target.distance > 1 && rv_goal.distance*2 > rv_target.distance) {
#ifdef DEBUG_PATHFINDING
                LOG(llevDebug,"calc_direction_towards_object(): %s's target '%s' has moved\n", STRING_OBJ_NAME(op), STRING_OBJ_NAME(target));
#endif
                free_path(MOB_PATHDATA(op)->path);
                MOB_PATHDATA(op)->path = NULL;
            }
        } 
    }
    
    return calc_direction_towards(op, target, target->map, target->x, target->y);
}

/* Get a direction towards the target stored in the waypoint object wp
 * tries to use precomputed path if available or request path finding if needed */
static int calc_direction_towards_waypoint(object *op, object *wp) 
{
    return calc_direction_towards(op, wp, normalize_and_ready_map(op->map, &WP_MAP(wp)), WP_X(wp), WP_Y(wp));
}

/* Actually move the monster in the specified direction. If there is something blocking,
 * try to go on either side of it */
static int do_move_monster(object *op, int dir)
{
    int m;
    
	tdir11=dir;
    /* Confused monsters need a small adjustment */
    if(QUERY_FLAG(op,FLAG_CONFUSED))
        dir = absdir(dir + RANDOM()%3 + RANDOM()%3 - 2);
	tdir12=dir;
	
    if(move_object(op,dir)) /* Can the monster move directly toward waypoint? */
        return TRUE;
    
	tdir13=dir;
    m = 1-(RANDOM()&2);          /* Try left or right first? */
    /* try different detours */
    if(move_object(op,absdir(dir + m)) ||
            move_object(op,absdir(dir - m)) ||
            move_object(op,absdir(dir + m*2)) ||  
            move_object(op,absdir(dir - m*2))) 
        return TRUE;
	tdir14=dir;
	
    /* Couldn't move at all... */
    return FALSE;
}

/* Purge invalid and old mobs from list of known mobs */
static inline void cleanup_mob_knowns(struct mob_known_obj **first) 
{
    struct mob_known_obj *tmp;
    for(tmp = *first; tmp; tmp = tmp->next) 
    {
        if(! OBJECT_VALID(tmp->obj, tmp->obj_count) ||
                global_round_tag - tmp->last_seen > MAX_KNOWN_OBJ_AGE) 
        {
            if(tmp->next) 
                tmp->next->prev = tmp->prev;

            if(tmp->prev)
                tmp->prev->next = tmp->next;
            else
                *first = tmp->next;

            return_poolchunk(tmp, POOL_MOB_KNOWN_OBJ);   
        }
    }
}

/* Calculate a movement direction given a movement response */
static inline int direction_from_response(object *op, move_response *response)
{
    switch(response->type) 
    {
        case MOVE_RESPONSE_DIR:
            return tdir15=response->data.direction;
        case MOVE_RESPONSE_OBJECT:
//            LOG(llevDebug,"move_monster(): '%s' -> '%s'\n", STRING_OBJ_NAME(op), STRING_OBJ_NAME(response.data.target.obj));
            return tdir16=calc_direction_towards_object(op, response->data.target.obj);
        case MOVE_RESPONSE_WAYPOINT:
//            LOG(llevDebug,"move_monster(): '%s' -> '%s'\n", STRING_OBJ_NAME(op), STRING_OBJ_NAME(response.data.target.obj));
            return tdir17=calc_direction_towards_waypoint(op, response->data.target.obj);
        case MOVE_RESPONSE_COORD:
//            LOG(llevDebug,"move_monster(): '%s' -> '%s'\n", STRING_OBJ_NAME(op), STRING_OBJ_NAME(response.data.target.obj));
            return tdir18=calc_direction_towards_coord(op, response->data.coord.map, 
                    response->data.coord.x, response->data.coord.y);

        default:
            return tdir19=0;
    }
}

/*
 * Main AI function
 */

/* Move-monster returns 1 if the object has been freed, otherwise 0.  */
int move_monster(object *op) {
    move_response response;
    int dir;
    int success;
    
	tdir1=tdir2=tdir3=tdir4=tdir5=tdir6=tdir7=tdir8=
		tdir9=tdir10=tdir11=tdir12=tdir13=tdir14=-123456;

	/*
     * First, some general monster-managing
     */
    
    /* Set up mob data if missing */
    if(MOB_DATA(op) == NULL)
        op->custom_attrset = get_poolchunk(POOL_MOBDATA);
    
    /* Purge invalid and old mobs from list of known mobs */
    cleanup_mob_knowns(&MOB_DATA(op)->known_mobs);
    cleanup_mob_knowns(&MOB_DATA(op)->known_objs);
    
    regenerate_stats(op); /* Regenerate if applicable */

    /*
     * Internal thought and sensing behaviours
     * All are always executed
     */

    ai_look_for_other_mobs(op);
    ai_friendship(op);
    ai_choose_enemy(op);
 
    /*
     * Normal-priority movement behaviours. The first to return
     * a movement disables the rest
     */
    response.type = MOVE_RESPONSE_NONE; /* Clear the movement response */

	/* if(response.type == MOVE_RESPONSE_NONE) */    
	ai_stand_still(op, &response);

	tdir1=response.data.direction;
    if(response.type == MOVE_RESPONSE_NONE) 
        ai_run_away_from_enemy(op, &response);
	tdir2=response.data.direction;
    if(response.type == MOVE_RESPONSE_NONE) 
        ai_sleep(op, &response);
	tdir3=response.data.direction;
    if(response.type == MOVE_RESPONSE_NONE) 
        ai_move_towards_enemy(op, &response);
	tdir4=response.data.direction;
    if(response.type == MOVE_RESPONSE_NONE) 
        ai_move_towards_enemy_last_known_pos(op, &response);
	tdir5=response.data.direction;
    if(response.type == MOVE_RESPONSE_NONE) 
        ai_search_for_lost_enemy(op, &response);
	tdir6=response.data.direction;
    if(response.type == MOVE_RESPONSE_NONE) 
        ai_move_towards_waypoint(op, &response);
	tdir7=response.data.direction;
    if(QUERY_FLAG(op, FLAG_RANDOM_MOVE) && response.type == MOVE_RESPONSE_NONE)
        ai_move_randomly(op, &response);
	tdir8=response.data.direction;
    if(response.type == MOVE_RESPONSE_NONE) 
        ai_move_towards_home(op, &response);
	tdir9=response.data.direction;
    /* move_home alternative: move_towards_friend */

    /* TODO make it possible to move _away_ from waypoint or object */
    
    /*
     * High-priority movement behaviours. These can inspect movements
     * from earlier behaviours and override if nessecary 
     */
    
    /* TODO high-priority movements (e.g. dodge missile) */
    
    /* Calculate direction from response needed and execute movement */
    dir = direction_from_response(op, &response);
	tdir10=response.data.direction;
    if(dir > 0) {
		if(dir == 1234543 || dir == 1234542)
			LOG(llevDebug, "CRASHBUG: Got the bug... %d", dir);
		else
	        success = do_move_monster(op, dir);
        /* TODO: handle success=0 and precomputed paths/giving up */    
    }
    
    /* 
     * Other mutually exclusive action commands
     * First to return TRUE disables the rest
     * TODO: some monsters can do multiple attacks? make the number of iterations here a parameter
     * TODO: either shuffle these randomly or use some sort of priority system
     * TODO: maybe separate into two parts: decision (gives an action and a priority) and
     *       execution (which done on the highest-prioritized action after all decisions are finished)
     * TODO: in original rules DEX has influence over whether to try any of these or not 
     */
    ai_spell_attack_enemy(op) || ai_melee_attack_enemy(op) || ai_bow_attack_enemy(op);
    
    return 0;
}

/*
 * Pathfinding "callback"
 */

/* A request for path finding has been accepted and we must now find out 
 *   1) where we actually wanted to go, and
 *   2) how to get there.
 */
void object_accept_path(object *op) {
    mapstruct *goal_map;
    int goal_x, goal_y;
    path_node *path;
    object *target;
    
    /* make sure we have a valid target obj or map */
    if(op->type != MONSTER || MOB_DATA(op) == NULL ||
            (!OBJECT_VALID(MOB_PATHDATA(op)->target_obj, MOB_PATHDATA(op)->target_count) && !MOB_PATHDATA(op)->target_map))
        return;

    /* 1: Where do we want to go? */
    target = MOB_PATHDATA(op)->target_obj;

    /* Is target our real target, is it a waypoint which stores the target 
     * coords? Or is our target a coordinate in the target_* values? */
    if(target == NULL) 
    {
        /* Move towards a spcific coordinate */
        goal_x = MOB_PATHDATA(op)->target_x;
        goal_y = MOB_PATHDATA(op)->target_y;
        goal_map = normalize_and_ready_map(op->map, &MOB_PATHDATA(op)->target_map);
    } else if(target->type == TYPE_WAYPOINT_OBJECT) 
    {
        /* Default map is current map */
        goal_x = WP_X(target);
        goal_y = WP_Y(target);
        goal_map = normalize_and_ready_map(op->map, &WP_MAP(target));
        
        FREE_AND_CLEAR_HASH(MOB_PATHDATA(op)->goal_map);
    } else 
    {
        goal_x = target->x;
        goal_y = target->y;
        if(target->type == TYPE_BASE_INFO) {
            goal_map = normalize_and_ready_map(op->map, &target->slaying);
            LOG(llevDebug,"source: %s, map %s (%p), target %s map %s (%p)\n",
                    op->name, op->map->path, op->map, target->name, goal_map->path, goal_map);
        } else
            goal_map = target->map;            
    
        /* Keep track of targets that may move */
        FREE_AND_ADD_REF_HASH(MOB_PATHDATA(op)->goal_map, goal_map->path);
        MOB_PATHDATA(op)->goal_x = goal_x;
        MOB_PATHDATA(op)->goal_y = goal_y;
    } 
        
    /* 2) Do the actual pathfinding: find a path, and compress it */
    path = compress_path(find_path(op, op->map, op->x, op->y, goal_map, goal_x, goal_y));

    if(path) {        
        /* Skip the first path element (always the starting position) */
        path = path->next;
        if(! path) {
/*            SET_FLAG(waypoint, FLAG_CONFUSED); */
            return;
        }
        
#ifdef DEBUG_PATHFINDING
        {
            path_node *tmp;
            LOG(llevDebug,"object_accept_path(): '%s' new path -> '%s': ", STRING_OBJ_NAME(op), STRING_OBJ_NAME(MOB_PATHDATA(op)->target_obj));
            for(tmp = path; tmp; tmp = tmp->next) 
                LOG(llevDebug,"(%d,%d) ", tmp->x, tmp->y);
            LOG(llevDebug,"\n");
        }
#endif        
        /* Free any old precomputed path */
        if(MOB_PATHDATA(op)->path) 
            free_path(MOB_PATHDATA(op)->path);

        /* And store the new one */
        MOB_PATHDATA(op)->path = encode_path(path, NULL);
        
        /* Clear counters and stuff */
        MOB_PATHDATA(op)->best_distance = -1;
        MOB_PATHDATA(op)->tried_steps = 0;
    } else 
        LOG(llevDebug,"object_accept_path(): no path to destination ('%s' -> '%s')\n", STRING_OBJ_NAME(op), STRING_OBJ_NAME(MOB_PATHDATA(op)->target_obj));
    /* TODO: handle the case where no path can be found */
}

/*
 * Non-scripted chat functions
 */

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

    char buf[HUGE_BUF];

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

	/* npc chars can hook in here with
	 * monster.Communicate("/kiss Fritz")
	 * we need to catch the emote here.
	 */
	if(*txt == '/' && op->type != PLAYER)
	{
		CommArray_s *csp;
		char *cp=NULL;

		/* remove the command from the parameters */
		strncpy(buf,txt,HUGE_BUF-1);
		buf[HUGE_BUF-1] ='\0';

		cp=strchr(buf, ' ');

		if (cp) 
		{
			*(cp++) ='\0';
			cp = cleanup_string(cp);
			if (cp && *cp=='\0')
				cp = NULL;
		}

		csp = find_command_element(buf, CommunicationCommands, CommunicationCommandSize);
		if(csp)
		{
			csp->func(op, cp);
			return;
		}
		return;
	}

	sprintf(buf, "%s says: ",query_name(op));
	strncat(buf, txt, MAX_BUF - strlen(buf)-1);
	buf[MAX_BUF-1]=0;
	new_info_map(NDI_WHITE,op->map, op->x, op->y, MAP_INFO_NORMAL, buf);

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
					/* avoid talking to self */
					if(op != npc)
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
    
  /* Gecko: FIXME if the python is not loaded the if() { ...; return 0; } will not
   * be executed above and the server will crash on the code below. I'm not really
   * sure why, but it should be investigated. I added some verification code below
   * to catch the error.
   */
  
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
      CFP.Value[9] = (char *) (STRING_OBJ_RACE(event_obj));
      CFP.Value[10]= (char *) (STRING_OBJ_SLAYING(event_obj));
      if (event_obj && findPlugin(event_obj->name)>=0)
      {
          ((PlugList[findPlugin(event_obj->name)].eventfunc) (&CFP));
          return 0;
      } else {
          LOG(llevBug,"An object (%s) had a event flag but no event object (SAY)\n",cobj->name);
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
				char buf[MAX_BUF]; 

				strncpy(buf, msgs->messages[i], MAX_BUF-1);
				buf[MAX_BUF-1]='\0';

				cp=strchr(buf, ' ');
				if (cp) 
				{
					*(cp++) ='\0';
					cp = cleanup_string(cp);
					if (cp && *cp=='\0')
						cp = NULL;

					if(cp && *cp == '%')
						cp = (char *)op->name;

				}
				
				csp = find_command_element(buf, CommunicationCommands, CommunicationCommandSize);
				if(csp)
					csp->func(npc, cp);


			}
			else
			{
				sprintf(buf,"%s says: %s",query_name(npc),msgs->messages[i]);
				new_info_map_except(NDI_UNIQUE, op->map, op->x, op->y, MAP_INFO_NORMAL,op, op, buf);
			}
		}
		else /* if a npc is talking to a player, is shown navy and with a seperate "xx says:" line */
		{
			/* if a message starts with '/', we assume a emote */
			/* we simply hook here in the emote msg list */
			if(*msgs->messages[i] == '/')
			{
				CommArray_s *csp;
				char *cp=NULL;
				char buf[MAX_BUF]; 

				strncpy(buf, msgs->messages[i], MAX_BUF-1);
				buf[MAX_BUF-1]='\0';

				cp=strchr(buf, ' ');
				if (cp) 
				{
					*(cp++) ='\0';
					cp = cleanup_string(cp);
					if (cp && *cp=='\0')
						cp = NULL;
					
					if(cp && *cp == '%')
						cp = (char*)op->name;
				}
				
				csp = find_command_element(buf, CommunicationCommands, CommunicationCommandSize);
				if(csp)
					csp->func(npc, cp);


			}
			else
			{
				sprintf(buf,"%s says:",query_name(npc)); 
				new_draw_info(NDI_NAVY|NDI_UNIQUE,0,op, buf);
				new_draw_info(NDI_NAVY | NDI_UNIQUE,0,op, msgs->messages[i]);
				sprintf(buf,"%s talks to %s.", query_name(npc), query_name(op));
				new_info_map_except(NDI_UNIQUE, op->map, op->x, op->y, MAP_INFO_NORMAL, op, op, buf);
			}
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
	  new_info_map(NDI_NAVY | NDI_UNIQUE, npc->map, npc->x, npc->y, MAP_INFO_NORMAL, msgs->messages[i]);
        free_messages(msgs);
	use_trigger(npc);
        return 1;
      }
  free_messages(msgs);
  return 0;
}

/*
 * spawn point releated function
 * MT: perhaps its time to make a new module?
 * Gecko: YES!!
 */

/* drop a monster on the map, by copying a monster object or
 * monster object head. Add treasures.
 */
static object *spawn_monster(object *gen, object *orig, int range) 
{
  int i;
  object *op,*head=NULL,*prev=NULL, *ret=NULL;
  archetype *at=gen->arch;

  i=find_first_free_spot2(at,orig->map,orig->x,orig->y,0,range);
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
    if (OBJECT_FREE(op)) return NULL;
    if(op->randomitems!=NULL)
		create_treasure(op->randomitems,op,0,op->level?op->level:orig->map->difficulty,T_STYLE_UNSET,ART_CHANCE_UNSET,0,NULL);
    if(head==NULL)
      head=op;
    prev=op;
    at=at->more;
  }
  return ret; /* return object ptr to our spawn */
}

/* check the current darkness on this map allows to spawn 
 * 0: not allowed, 1: allowed
 */
static inline int spawn_point_darkness(object *spoint, int darkness)
{
	int map_light;

	if(!spoint->map)
		return 0;

	if(MAP_OUTDOORS(spoint->map)) /* outdoor map */
		map_light = world_darkness;
	else
	{
		if(MAP_DARKNESS(spoint->map) == -1)
			map_light = MAX_DARKNESS;
		else
			map_light = MAP_DARKNESS(spoint->map);
	}

	if(darkness < 0)
	{
		if(map_light < -darkness)
			return 1;
	}
	else
	{
		if(map_light > darkness)
			return 1;
	}
	return 0;
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
		{
			if(op->last_eat) /* check darkness if needed */
			{
				/* 1 = darkness is ok */
				if(spawn_point_darkness(op, op->last_eat))
					return;
									
				/* darkness has changed - now remove the spawned monster */
				remove_ob(op->enemy);
				check_walk_off (op->enemy, NULL,MOVE_APPLY_VANISHED);
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
	if(op->stats.sp == -1)
	{
		int gg;
		/* now lets decide we will have a spawn event */
		if(op->last_grace<=-1) /* never */
		{
			/*LOG(-1," closed (-1)\n");*/
			return;
		}
		if(op->last_grace && (gg=(RANDOM() % (op->last_grace+1)))) /* if >0 and random%x is NOT null ... */
		{
			/*LOG(-1," chance: %d (%d)\n",gg,op->last_grace);*/
			return;
		}

		op->stats.sp = (RANDOM() % SPAWN_RANDOM_RANGE);
	}
	/*LOG(-1," hit!: %d\n",op->stats.sp);*/
	
	if(!op->inv) /* spawn point without inventory! */
	{
		LOG(llevBug,"BUG: Spawn point without inventory!! --> map %s (%d,%d)\n",op->map?(op->map->path?op->map->path:">no path<"):">no map<", op->x, op->y);
		/* kill this spawn point - its useless and need to fixed from the map maker/generator */
		remove_ob(op);
		check_walk_off (op, NULL,MOVE_APPLY_VANISHED);
		return;
	}
	/* now we move through the spawn point inventory and
	 * get the mob with a number under this value AND nearest.
	 */
   for(rmt=0,mob=NULL,tmp = op->inv; tmp; tmp = next)
    {
		next = tmp->below;

		if(tmp->type != SPAWN_POINT_MOB)
			LOG(llevBug,"BUG: spawn point in map %s (%d,%d) with wrong type object (%d) in inv: %s\n",
											op->map?op->map->path:"<no map>", op->x, op->y, tmp->type, query_name(tmp));
		else if((int)tmp->enemy_count <= op->stats.sp && (int)tmp->enemy_count >= rmt)
		{
			/* we have a possible hit - control special settings now */
			if(tmp->last_eat) /* darkness */
			{
				/* 1: darkness on map of spawn point is ok */
				if(!spawn_point_darkness(op, tmp->last_eat))
					continue;
			}

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
	
	/* setup special monster -> spawn point values */
	op->last_eat = 0;
	if(mob->last_eat) /* darkness controled spawns */
	{
		op->last_eat = mob->last_eat;
		mob->last_eat = 0;
	}

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
			if(!tmp->weight_limit || !(RANDOM() % (tmp->weight_limit+1))) /* skip this container - drop the ->inv */
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

	SET_MULTI_FLAG(mob, FLAG_SPAWN_MOB); /* FINISH: now mark our mob as a spawn */
	fix_monster(mob); /* fix all the values and add in possible abilities or forces ... */
    if(!insert_ob_in_map(mob,mob->map,op,0)) /* *now* all is done - *now* put it on map */
		return;
	if(QUERY_FLAG(mob,FLAG_FRIENDLY))
        add_friendly_object(mob);
	
}
