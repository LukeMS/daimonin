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
#ifndef WIN32 /* ---win32 remove headers */
#include <pwd.h>
#endif
#ifndef __CEXTRACT__
#include <sproto.h>
#endif
#include <sounds.h>
#include <newclient.h>

static char tile_grid[27][52] = {
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,7,7,7,7,7,7,7,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
	{1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,2,2,2,2,2,2,2,2,2},
	{1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,2,2,2,2,2,2,2},
	{1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,2,2,2,2,2},
	{1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,2,2,2},
	{1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,2},
	{5,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,6},
	{3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,4,4,4},
	{3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,4,4,4,4,4},
	{3,3,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,4,4,4,4,4,4,4},
	{3,3,3,3,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,4,4,4,4,4,4,4,4,4},
	{3,3,3,3,3,3,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,4,4,4,4,4,4,4,4,4,4,4},
	{3,3,3,3,3,3,3,3,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,4,4,4,4,4,4,4,4,4,4,4,4,4},
	{3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4},
	{3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4},
	{3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4},
	{3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,0,0,0,0,0,0,0,0,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4},
	{3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,0,0,0,0,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4},
	{3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,8,8,8,8,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4},
	{3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,8,8,8,8,8,8,8,8,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4},
};

static tile_toff[9][2] =
{
	{0,0},
	{24,12},
	{-24,12},
	{24,-12},
	{-24,-12},
	{48,0},
	{-48,0},
	{0,23},
	{0,-23},
};
static tile_grid_offset[9][2] =
{
	{0,0},
	{1,0},
	{0,1},
	{0,-1},
	{-1,0},
	{1,-1},
	{-1,1},
	{1,1},
	{-1,-1}
};


/* i left find_arrow - find_arrow() and find_arrow()_ext should merge 
   when the server sided range mode is removed at last from source */ 
static object *find_arrow_ext(object *op, char *type,int tag);

player *find_player(char *plname)
{
  player *pl;
  for(pl=first_player;pl!=NULL;pl=pl->next)
  {
    if(pl->ob != NULL && !QUERY_FLAG(pl->ob,FLAG_REMOVED) && !strcmp(query_name(pl->ob),plname))
        return pl;
  };
  return NULL;
}

void display_motd(object *op) {
#ifdef MOTD
  char buf[MAX_BUF];
  FILE *fp;
  int comp;

  sprintf(buf,"%s/%s",settings.localdir,MOTD);
  if((fp=open_and_uncompress(buf,0,&comp))==NULL) {
    return;
  }
  while(fgets(buf,MAX_BUF,fp)!=NULL) {
    char *cp;
    if(*buf=='#')
      continue;
    cp=strchr(buf,'\n');
    if(cp!=NULL)
      *cp='\0';
    new_draw_info(NDI_UNIQUE, 0,op,buf);
  }
  close_and_delete(fp, comp);
  new_draw_info(NDI_UNIQUE, 0,op," ");
#endif
}

int playername_ok(char *cp) {
  for(;*cp!='\0';cp++)
    if(!((*cp>='a'&&*cp<='z')||(*cp>='A'&&*cp<='Z'))&&*cp!='-'&&*cp!='_')
      return 0;
  return 1;
}

/* Redo this to do both get_player_ob and get_player.
 * Hopefully this will be less bugfree and simpler.
 * Returns the player structure.  If 'p' is null,
 * we create a new one.  Otherwise, we recycle
 * the one that is passed.
 */
static player* get_player(player *p) {
    object *op=arch_to_object(get_player_archetype(NULL));
    int i;

    if (!p)
	{
		player *tmp;

		p = (player *) malloc(sizeof(player));
		memset(p,0, sizeof(player));
		if(p==NULL)
			LOG(llevError,"ERROR: get_player(): out of memory\n");

		/* This adds the player in the linked list.  There is extra
		 * complexity here because we want to add the new player at the
		*end of the list - there is in fact no compelling reason that
		* that needs to be done except for things like output of
		* 'who'.
		*/
		tmp=first_player;
		while(tmp!=NULL&&tmp->next!=NULL)
		    tmp=tmp->next;
		if(tmp!=NULL)
		    tmp->next=p;
		else
		    first_player=p;

		p->next = NULL; 
    }

    /* Clears basically the entire player structure except
     * for next and socket.
     */
    memset((void*)((char*)p + offsetof(player, maplevel)), 0, 
					    sizeof(player) - offsetof(player, maplevel));

    /* There are some elements we want initialized to non zero value -
     * we deal with that below this point.
     */
    p->party_number=-1;
#ifdef USE_SWAP_STATS
    p->Swap_First = -1;
#endif

#ifdef AUTOSAVE
    p->last_save_tick = 9999999;
#endif
    
    strcpy(p->savebed_map, first_map_path);  /* Init. respawn position */

    p->firemode_type=p->firemode_tag1=p->firemode_tag2=-1;
    op->contr=p; /* this aren't yet in archetype */
    p->ob = op;
    op->speed_left=0.5;
    op->speed=1.0;
    op->direction=5;     /* So player faces south */
	/* i let it in but there is no use atm for run_away and player */
    op->run_away = 0; /* Then we panick... */

    roll_stats(op);
    p->state=ST_ROLL_STAT;
    clear_los(op);

	p->target_hp = -1;
	p->target_hp_p = -1;
    p->gen_sp_armour=0;
    p->last_speed= -1;
    p->shoottype=range_none;
    p->listening=9;
    p->last_weapon_sp= -1;
    p->do_los=1;
#ifdef EXPLORE_MODE
    p->explore=0;
#endif

    strncpy(p->title,op->arch->clone.name,MAX_NAME);
	FREE_AND_COPY_HASH(op->race,op->arch->clone.race);

    /* Would be better of '0' was not a defined spell */
    for(i=0;i<NROFREALSPELLS;i++)
	p->known_spells[i]= -1;

    p->chosen_spell = -1;
    CLEAR_FLAG(op,FLAG_READY_SKILL); 

    /* we need to clear these to -1 and not zero - otherwise,
     * if a player quits and starts a new character, we wont
     * send new values to the client, as things like exp start
     * at zero.
     */
    for (i=0; i < MAX_EXP_CAT; i++) {
	p->last_skill_exp[i] = -1;
	p->last_skill_level[i] = -1;
    }

	p->set_skill_weapon = NO_SKILL_READY; /* quick skill reminder for select hand weapon */
    p->set_skill_archery = NO_SKILL_READY;
    p->last_skill_index = -1;
    p->last_stats.exp = -1;

    return p;
}
 
/* This loads the first map an puts the player on it. */
static void set_first_map(object *op)
{
    strcpy(op->contr->maplevel, first_map_path);
    op->x = -1;
    op->y = -1;
    enter_exit(op, NULL);
}

/* Tries to add player on the connection passwd in ns.
 * All we can really get in this is some settings like host and display
 * mode.
 */

int add_player(NewSocket *ns) {
    player *p;
    char *defname = "nobody";


    /* Check for banned players and sites.  usename is no longer accurate,
     * (can't get it over sockets), so all we really have to go on is
     * the host.
     */

    if (checkbanned (defname, ns->host)){
		char buf[256];
		strcpy(buf, "X3 Connection refused.\nYou are banned!");
		Write_String_To_Socket(ns, BINARY_CMD_DRAWINFO, buf,strlen(buf));
		LOG(llevInfo, "Banned player tried to add. [%s@%s]\n", defname, ns->host);
		return 1;
    }

    p = get_player(NULL);
    memcpy(&p->socket, ns, sizeof(NewSocket));
    /* Needed because the socket we just copied over needs to be cleared.
     * Note that this can result in a client reset if there is partial data
     * on the uncoming socket.
     */
    p->socket.update_look=0;
    p->socket.look_position=0;
    p->socket.inbuf.len = 0;

	start_info(p->ob);
    get_name(p->ob);
    return 0;
}

/*
 * get_player_archetype() return next player archetype from archetype
 * list. Not very efficient routine, but used only creating new players.
 * Note: there MUST be at least one player archetype!
 */
archetype *get_player_archetype(archetype* at)
{
    archetype *start = at;
    for (;;) {
	if (at==NULL || at->next==NULL)
	    at=first_archetype;
	else
	    at=at->next;
	if(at->clone.type==PLAYER)
	    return at;
	if (at == start) {
	    LOG(llevError, "ERROR: No Player achetypes\n");
	    exit (-1);
	}
    }
}


/* ARGH. this friendly list is a GLOBAL list. assuming 100 players on 75 map ... and every
 * mob on every map (perhaps some hundreds) will move through ALL of object of the
 * friendly list every time they try to target... 
 * this must be changed. Look at friendly.c for more comments about it.
 * note, that only here and in pets.c this list is used. In pets.c it is only used to
 * remove or collect players pets,
 */
/* this is another example of the more or less broken friendly list use... this function seems not to trust
 * his own list - whats very bad because this is a core function - speed & elegance should used here
 * not crazy while loops to find invalid list entries.
 */
/* btw, this is not a "get nearest player" - its a "get nearest friendly object" */
/* i added now aggro range.
 * aggro range is the distance to target a mob will attack. Is the target out of this range,
 * the mob will not attack and/or not target it. 
 * If a target moves out of aggro range for xx ticks, a mob will change
 * or leave target. Stealth will aggro range - 2. That sounds not much but is quite useful for normal
 * mobs.
 */
object *get_nearest_player(object *mon) {
    object *op = NULL;
    player *pl = NULL;
    objectlink *ol;
    unsigned int lastdist, aggro_range, aggro_stealth;
    rv_vector	rv;

	/* lets set our aggro range. If mob is sleeping or blinded - half aggro range.
	 * if target has stealth - sub. -2 
	 */
	aggro_range = mon->stats.Wis;
	if(mon->enemy || mon->attacked_by)
		aggro_range +=3;
	if(QUERY_FLAG(mon,FLAG_SLEEP) || QUERY_FLAG(mon,FLAG_BLIND))
	{
		aggro_range /=2;
		aggro_stealth = aggro_range-2;
	}
	else
	{
		aggro_stealth = aggro_range-2;
	}
	if(aggro_stealth<MIN_MON_RADIUS)
		aggro_stealth = MIN_MON_RADIUS;

	
    for(ol=first_friendly_object,lastdist=1000;ol!=NULL;ol=ol->next) 
	{
		/* We should not find free objects on this friendly list, but it
		* does periodically happen.  Given that, lets deal with it.
		* While unlikely, it is possible the next object on the friendly
		* list is also free, so encapsulate this in a while loop.
		*/
		while ((tag_t) ol->id != ol->ob->count || QUERY_FLAG(ol->ob, FLAG_REMOVED) || QUERY_FLAG(ol->ob, FLAG_FREED) || (!QUERY_FLAG(ol->ob, FLAG_FRIENDLY)&& ol->ob->type != PLAYER)) 
		{
			object *tmp=ol->ob;

			/* Can't do much more other than log the fact, because the object
			* itself will have been cleared.
			*/
			LOG(llevDebug,"get_nearest_player: Found free/non friendly object on friendly list (%s)\n", tmp->name);
			ol = ol->next;
			remove_friendly_object(tmp);
			if (!ol) return op;
		}

		/* Remove special check for player from this.  First, it looks to cause
		 * some crashes (ol->ob->contr not set properly?), but secondly, a more
		 * complicated method of state checking would be needed in any case -
		 * as it was, a clever player could type quit, and the function would  
		 * skip them over while waiting for confirmation.
		 */
		if (!can_detect_enemy(mon,ol->ob,&rv) ||
			(QUERY_FLAG(ol->ob, FLAG_STEALTH)?aggro_stealth:aggro_range) < (int) rv.distance)
			continue;

		if(lastdist>rv.distance)
		{
		    op=ol->ob;
		    lastdist=rv.distance;
		}
    }

	/* well, we add now player to friendly list (later it will friendly object on map list) 
    for (pl=first_player; pl != NULL; pl=pl->next) 
	{
		if (on_same_map(mon, pl->ob)&& can_detect_enemy(mon, pl->ob,&rv)) 
		{

			if(lastdist>rv.distance)
			{
				op=pl->ob;
				lastdist=rv.distance;
			}
		}
    }
	*/

#if 0
	LOG(llevDebug,"get_nearest_player() mob %s (%x) finds friendly obj: %s (%x) aggro range: %d\n",query_name(mon),mon->count,query_name(op),op?op->count:-1, mon->stats.Wis);
#endif
    return op;
}

/* I believe this can safely go to 2, 3 is questionable, 4 will likely
 * result in a monster paths backtracking.  It basically determines how large a 
 * detour a monster will take from the direction path when looking
 * for a path to the player.  The values are in the amount of direction
 * the deviation is
 */
#define DETOUR_AMOUNT	2

/* This is used to prevent infinite loops.  Consider a case where the
 * player is in a chamber (with gate closed), and monsters are outside.
 * with DETOUR_AMOUNT==2, the function will turn each corner, trying to
 * find a path into the chamber.  This is a good thing, but since there
 * is no real path, it will just keep circling the chamber for
 * ever (this could be a nice effect for monsters, but not for the function
 * to get stuck in.  I think for the monsters, if max is reached and
 * we return the first direction the creature could move would result in the
 * circling behaviour.  Unfortunately, this function is also used to determined
 * if the creature should cast a spell, so returning a direction in that case
 * is probably not a good thing.
 */
#define MAX_SPACES	50


/*
 * Returns the direction to the player, if valid.  Returns 0 otherwise.
 * modified to verify there is a path to the player.  Does this by stepping towards
 * player and if path is blocked then see if blockage is close enough to player that
 * direction to player is changed (ie zig or zag).  Continue zig zag until either
 * reach player or path is blocked.  Thus, will only return true if there is a free
 * path to player.  Though path may not be a straight line. Note that it will find
 * player hiding along a corridor at right angles to the corridor with the monster.
 *
 * Modified by MSW 2001-08-06 to handle tiled maps. Various notes:
 * 1) With DETOUR_AMOUNT being 2, it should still go and find players hiding
 * down corriders.
 * 2) I think the old code was broken if the first direction the monster
 * should move was blocked - the code would store the first direction without
 * verifying that the player can actually move in that direction.  The new
 * code does not store anything in firstdir until we have verified that the
 * monster can in fact move one space in that direction.
 * 3) I'm not sure how good this code will be for moving multipart monsters,
 * since only simple checks to blocked are being called, which could mean the monster
 * is blocking itself.
 */
/* TODO: this sould really use pathfinding instead. /Gecko */
int path_to_player(object *mon, object *pl,int mindiff) {
    rv_vector	rv;
    int	x,y,lastx,lasty,dir,i,diff, firstdir=0,lastdir, max=MAX_SPACES;
    mapstruct *m ,*lastmap;

    get_rangevector(mon, pl, &rv, 0);

    if ((int) rv.distance<mindiff) return 0;

    x=mon->x;
    y=mon->y;
    m=mon->map;
    dir = rv.direction;
    lastdir = firstdir = rv.direction; /* perhaps we stand next to pl, init firstdir too */
    diff = FABS(rv.distance_x)>FABS(rv.distance_y)?FABS(rv.distance_x):FABS(rv.distance_y);
    /* If we can't solve it within the search distance, return now. */
    if (diff>max) return 0;
    while (diff >1 && max>0) {
	lastx = x;
	lasty = y;
	lastmap = m;
	x = lastx + freearr_x[dir];
	y = lasty + freearr_y[dir];

	/* Space is blocked - try changing direction a little */
	if (arch_blocked(mon->arch,mon,m,x,y)) /* arch blocked controls multi arch with full map flags */
	{
	    /* recalculate direction from last good location.  Possible
	     * we were not traversing ideal location before.
	     */
	    get_rangevector_from_mapcoords(lastmap, lastx, lasty, pl->map, pl->x, pl->y, &rv, 0);
	    if (rv.direction != dir) {
		/* OK - says direction should be different - lets reset the
		 * the values so it will try again.
		 */
		x = lastx;
		y = lasty;
		m = lastmap;
		dir = firstdir = rv.direction;
	    } else {
		/* direct path is blocked - try taking a side step to
		 * either the left or right.
		 * Note increase the values in the loop below to be 
		 * more than -1/1 respectively will mean the monster takes
		 * bigger detour.  Have to be careful about these values getting
		 * too big (3 or maybe 4 or higher) as the monster may just try
		 * stepping back and forth
		 */
		for (i=-DETOUR_AMOUNT; i<=DETOUR_AMOUNT; i++) {
		    if (i==0) continue;	/* already did this, so skip it */
		    /* Use lastdir here - otherwise,
		     * since the direction that the creature should move in
		     * may change, you could get infinite loops.
		     * ie, player is northwest, but monster can only
		     * move west, so it does that.  It goes some distance,
		     * gets blocked, finds that it should move north,
		     * can't do that, but now finds it can move east, and
		     * gets back to its original point.  lastdir contains
		     * the last direction the creature has successfully
		     * moved.
		     */
		    
		    x = lastx + freearr_x[absdir(lastdir+i)];
		    y = lasty + freearr_y[absdir(lastdir+i)];
		    m = lastmap;

			if (!arch_blocked(mon->arch,mon,m,x,y)) 
				break;
		}
		/* go through entire loop without finding a valid
		 * sidestep to take - thus, no valid path.
		 */
		if (i==(DETOUR_AMOUNT+1))
		    return 0;
		diff--;
		lastdir=dir;
		max--;
		if (!firstdir) firstdir = dir+i;
	    } /* else check alternate directions */
	} /* if blocked */
	else {
	    /* we moved towards creature, so diff is less */
	    diff--;
	    max--;
	    lastdir=dir;
	    if (!firstdir) firstdir = dir;
  	}
	if (diff<=1) {
	    /* Recalculate diff (distance) because we may not have actually
	     * headed toward player for entire distance.
	     */
	    get_rangevector_from_mapcoords(m, x, y, pl->map, pl->x, pl->y, &rv, 0);
	    diff = FABS(rv.distance_x)>FABS(rv.distance_y)?FABS(rv.distance_x):FABS(rv.distance_y);
	}
	if (diff>max) return 0;
    }
    /* If we reached the max, didn't find a direction in time */
    if (!max) return 0;

    return firstdir;
}

void give_initial_items(object *pl,treasurelist *items) {
    object *op,*next=NULL;
    /* Lets at least use the SP_ values here and not just numbers, like 0, 1, ... */

    int start_spells[] = {SP_BULLET, SP_S_FIREBALL, SP_FIRESTORM,
	SP_S_LIGHTNING, SP_M_MISSILE ,SP_ICESTORM, SP_S_SNOWSTORM};
    int nrof_start_spells = sizeof start_spells / sizeof start_spells[0];

    int start_prayers[] = {SP_TURN_UNDEAD, SP_HOLY_WORD, SP_MINOR_HEAL,
	SP_CAUSE_LIGHT};
    int nrof_start_prayers = sizeof start_prayers / sizeof start_prayers[0];
    int idx;


    if(pl->randomitems!=NULL)
	create_treasure(items,pl,GT_ONLY_GOOD|GT_NO_VALUE,1,T_STYLE_UNSET,ART_CHANCE_UNSET,0);

    for (op=pl->inv; op; op=next) {
	next = op->below;
	
	/* Forces get applied per default, unless they have the
           flag "neutral" set. Sorry but I can't think of a better way */
	/* hm, i removed the neutral flag - perhaps we need here a better control.
	 * if needed we must insert here a flag. MT-11-2002
	 */
  	if(op->type==FORCE)
	  { SET_FLAG(op,FLAG_APPLIED);};
	
	/* we never give weapons/armour if these cannot be used
           by this player due to race restrictions */
	if (pl->type == PLAYER) {
	  if ((!QUERY_FLAG(pl, FLAG_USE_ARMOUR) &&
	      (op->type == ARMOUR || op->type == BOOTS ||
	       op->type == CLOAK || op->type == HELMET ||
	       op->type == SHIELD || op->type == GLOVES ||
	       op->type == BRACERS || op->type == GIRDLE)) ||
	      (!QUERY_FLAG(pl, FLAG_USE_WEAPON) && op->type == WEAPON)) {
	    remove_ob (op);
	    free_object (op);
	    continue;
	  }
	}
	
  	if(op->type==SPELLBOOK) { /* fix spells for first level spells */
	    if (op->sub_type1 == ST1_SPELLBOOK_CLERIC) {
		if (nrof_start_prayers <= 0) {
		    remove_ob (op);
		    free_object (op);
		    continue;
		}
		idx = RANDOM() % nrof_start_prayers;
		op->stats.sp = start_prayers[idx];
		/* This makes sure the character does not get duplicate spells */
		start_prayers[idx] = start_prayers[nrof_start_prayers - 1];
		nrof_start_prayers--;
	    } else {
		if (nrof_start_spells <= 0) {
		    remove_ob (op);
		    free_object (op);
		    continue;
		}
		idx = RANDOM() % nrof_start_spells;
		op->stats.sp = start_spells[idx];
		start_spells[idx] = start_spells[nrof_start_spells - 1];
		nrof_start_spells--;
	    }
	}
	/* Give starting characters identified, uncursed, and undamned
	 * items.  Just don't identify gold or silver, or it won't be
	 * merged properly.
	 */
	if (need_identify(op)) {
	    SET_FLAG(op, FLAG_IDENTIFIED);
	    CLEAR_FLAG(op, FLAG_CURSED);
	    CLEAR_FLAG(op, FLAG_DAMNED);
	}
	if(op->type==ABILITY)  {
	    pl->contr->known_spells[pl->contr->nrofknownspells++]=op->stats.sp;
	    remove_ob(op);
	    free_object(op);
            continue;
	}
    } /* for loop of objects in player inv */
}

void get_name(object *op) {
    op->contr->write_buf[0]='\0';
    op->contr->state=ST_GET_NAME;
    send_query(&op->contr->socket,0,"What is your name?\n:");
}

void get_password(object *op) {
    op->contr->write_buf[0]='\0';
    op->contr->state=ST_GET_PASSWORD;
    send_query(&op->contr->socket,CS_QUERY_HIDEINPUT, "What is your password?\n:");
}

void play_again(object *op)
{
    op->contr->state=ST_PLAY_AGAIN;
    op->chosen_skill = NULL;
    unlock_player(op->name);
    send_query(&op->contr->socket, CS_QUERY_SINGLECHAR, "Do you want to play again (a/q)?");
    /* a bit of a hack, but there are various places early in th
     * player creation process that a user can quit (eg, roll
     * stats) that isn't removing the player.  Taking a quick
     * look, there are many places that call play_again without
     * removing the player - it probably makes more sense
     * to leave it to play_again to remove the object in all
     * cases.
     */
    if (!QUERY_FLAG(op, FLAG_REMOVED)) 
	remove_ob(op);
}


int receive_play_again(object *op, char key)
{
    if(key=='q'||key=='Q') {
	remove_friendly_object(op);
	leave(op->contr,0); /* ericserver will draw the message */
	return 2;
    }
    else if(key=='a'||key=='A') {
	player *pl = op->contr;
	char *name = NULL;
	
	FREE_AND_ADD_REF_HASH(name, op->name);
	remove_friendly_object(op);
	free_object(op);
	pl=get_player(pl);
	op = pl->ob;
	op->contr->password[0]='~';
	FREE_AND_CLEAR_HASH2(op->name);
	/* Lets put a space in here */
	new_draw_info(NDI_UNIQUE, 0, op, "\n");
	get_name(op);
	op->name = name;
	set_first_map(op);
    } else {
	/* user pressed something else so just ask again... */
	play_again(op);
    }
    return 0;
}

void confirm_password(object *op) {

    op->contr->write_buf[0]='\0';
    op->contr->state=ST_CONFIRM_PASSWORD;
    send_query(&op->contr->socket, CS_QUERY_HIDEINPUT, "Please type your password again.\n:");
}

void get_party_password(object *op, int partyid) {
  op->contr->write_buf[0]='\0';
  op->contr->state=ST_GET_PARTY_PASSWORD;
  op->contr->party_number_to_join = partyid;
  send_query(&op->contr->socket, CS_QUERY_HIDEINPUT, "What is the password?\n:");
}


/* This rolls four 1-6 rolls and sums the best 3 of the 4. */
int roll_stat() {
  int a[4],i,j,k;
  for(i=0;i<4;i++)
    a[i]=(int)RANDOM()%6+1;
  for(i=0,j=0,k=7;i<4;i++)
    if(a[i]<k)
      k=a[i],j=i;
  for(i=0,k=0;i<4;i++) {
    if(i!=j)
      k+=a[i];
  }
  return k;
}

void roll_stats(object *op) {
  int sum=0;
  do {
      /*
    op->stats.Str=roll_stat();
    op->stats.Dex=roll_stat();
    op->stats.Int=roll_stat();
    op->stats.Con=roll_stat();
    op->stats.Wis=roll_stat();
    op->stats.Pow=roll_stat();
    op->stats.Cha=roll_stat();
*/
    op->stats.Str=14;
    op->stats.Dex=14;
    op->stats.Int=13;
    op->stats.Con=12;
    op->stats.Wis=12;
    op->stats.Pow=12;
    op->stats.Cha=12;
    sum=op->stats.Str+op->stats.Dex+op->stats.Int+
	op->stats.Con+op->stats.Wis+op->stats.Pow+
	op->stats.Cha;
  } while(sum<82||sum>116);
#if defined( USE_SWAP_STATS) && defined(SORT_ROLLED_STATS)
	/* Sort the stats so that rerolling is easier... */
	{
	        int             i = 0, j = 0;
	        int             statsort[7];

	        statsort[0] = op->stats.Str;
	        statsort[1] = op->stats.Dex;
	        statsort[2] = op->stats.Int;
	        statsort[3] = op->stats.Con;
	        statsort[4] = op->stats.Wis;
	        statsort[5] = op->stats.Pow;
	        statsort[6] = op->stats.Cha;

	        /* a quick and dirty bubblesort? */
	        do {
	                if (statsort[i] < statsort[i + 1]) {
	                        j = statsort[i];
	                        statsort[i] = statsort[i + 1];
	                        statsort[i + 1] = j;
	                        i = 0;
	              } else {
	                        i++;
	              }
	      } while (i < 6);

	        op->stats.Str = statsort[0];
	        op->stats.Dex = statsort[1];
	        op->stats.Con = statsort[2];
	        op->stats.Int = statsort[3];
	        op->stats.Wis = statsort[4];
	        op->stats.Pow = statsort[5];
	        op->stats.Cha = statsort[6];
      }
#endif /* SWAP_STATS */

  op->contr->orig_stats.Str=op->stats.Str;
  op->contr->orig_stats.Dex=op->stats.Dex;
  op->contr->orig_stats.Int=op->stats.Int;
  op->contr->orig_stats.Con=op->stats.Con;
  op->contr->orig_stats.Wis=op->stats.Wis;
  op->contr->orig_stats.Pow=op->stats.Pow;
  op->contr->orig_stats.Cha=op->stats.Cha;
  op->stats.hp= -1;
  op->level=1;
  op->stats.exp=0;
  op->stats.sp=-1;
  op->stats.grace=-1;
  op->stats.ac=0;
  op->stats.wc=0;
  op->stats.dam=0;
  op->contr->orig_stats=op->stats;
}

void Roll_Again(object *op)
{
    esrv_new_player(op->contr, 0);
#ifndef USE_SWAP_STATS
    send_query(&op->contr->socket,CS_QUERY_SINGLECHAR,"Roll again (y/n)? ");
#else
    send_query(&op->contr->socket,CS_QUERY_SINGLECHAR,"[y] to roll new stats [n] to use stats\n[1-7] [1-7] to swap stats.\nRoll again (y/n/1-7)?  ");
#endif /* USE_SWAP_STATS */
}

void Swap_Stat(object *op,int Swap_Second)
{
#ifdef USE_SWAP_STATS
  signed char tmp;
  char buf[MAX_BUF];

    if ( op->contr->Swap_First == -1 ) 
	{
		LOG(llevBug,"BUG: Swap_Stat(): called without Swap_First for player %s\n", query_name(op));
		return;
    }

    tmp = get_attr_value(&op->contr->orig_stats, op->contr->Swap_First);

    set_attr_value(&op->contr->orig_stats, op->contr->Swap_First,
	get_attr_value(&op->contr->orig_stats, Swap_Second));

    set_attr_value(&op->contr->orig_stats, Swap_Second, tmp);

    sprintf(buf,"%s done\n", short_stat_name[Swap_Second]);
    new_draw_info(NDI_UNIQUE, 0,op, buf);
    op->stats.Str = op->contr->orig_stats.Str;
    op->stats.Dex = op->contr->orig_stats.Dex;
    op->stats.Con = op->contr->orig_stats.Con;
    op->stats.Int = op->contr->orig_stats.Int;
    op->stats.Wis = op->contr->orig_stats.Wis;
    op->stats.Pow = op->contr->orig_stats.Pow;
    op->stats.Cha = op->contr->orig_stats.Cha;
    op->stats.hp= -1;
    op->level=1;
    op->stats.exp=0;
    op->stats.sp=-1;
    op->stats.grace=-1;
    op->stats.ac=0;
    op->contr->Swap_First=-1;
#endif /* USE_SWAP_STATS */
}


/* This code has been greatly reduced, because with set_attr_value
 * and get_attr_value, the stats can be accessed just numeric
 * ids.  stat_trans is a table that translate the number entered
 * into the actual stat.  It is needed because the order the stats
 * are displayed in the stat window is not the same as how
 * the number's access that stat.  The table does that translation.
 */
int key_roll_stat(object *op, char key)
{
    int keynum = key -'0';
    char buf[MAX_BUF];
    static sint8 stat_trans[] = {-1, STR, DEX, CON, INTELLIGENCE, WIS, POW, CHA};

#ifdef USE_SWAP_STATS
    if (keynum>0 && keynum<=7) {
	if (op->contr->Swap_First==-1) {
	    op->contr->Swap_First=stat_trans[keynum];
	    sprintf(buf,"%s ->", short_stat_name[stat_trans[keynum]]);
	    new_draw_info(NDI_UNIQUE, 0,op,buf);
	}
	else
	    Swap_Stat(op,stat_trans[keynum]);

	send_query(&op->contr->socket,CS_QUERY_SINGLECHAR,"");
	return 1;
    }
#endif
    switch (key) {
	case 'n':
        case 'N': {
	    SET_FLAG(op, FLAG_WIZ);
		SET_ANIMATION(op, 4*(NUM_ANIMATIONS(op)/NUM_FACINGS(op)));     /* So player faces south */
	    add_statbonus(op);
		update_object(op,UP_OBJ_FACE);
	    esrv_update_item(UPD_FACE,op,op);
		op->contr->last_value= -1;
		op->stats.maxsp = op->arch->clone.stats.maxsp;
		op->stats.maxhp = op->arch->clone.stats.maxhp;
		op->stats.maxgrace = op->arch->clone.stats.maxgrace;
		op->contr->last_stats.exp=-1;
		esrv_update_stats(op->contr);

	    send_query(&op->contr->socket,CS_QUERY_SINGLECHAR,"Now choose a character.\nPress any key to change outlook.\nPress `d' when you're pleased.\n");
	    op->contr->state = ST_CHANGE_CLASS;
		new_draw_info(NDI_WHITE, 0, op, "Press &N 'ext for a different race or gender.");
		new_draw_info(NDI_WHITE, 0, op, "Press &G 'et for start playing with this char.");
	    if (op->msg)
			new_draw_info(NDI_BLUE, 0, op, op->msg);
	    return 0;
	}
     case 'y':
     case 'Y':
	roll_stats(op);
	send_query(&op->contr->socket,CS_QUERY_SINGLECHAR,"");
	return 1;

     case 'q':
     case 'Q':
      play_again(op);
      return 1;

     default:
#ifndef USE_SWAP_STATS
	  send_query(&op->contr->socket,CS_QUERY_SINGLECHAR,"Yes, No or Quit. Roll again?");
#else
	  send_query(&op->contr->socket,CS_QUERY_SINGLECHAR,"Yes, No, Quit or 1-6.  Roll again?");
#endif /* USE_SWAP_STATS */
	return 0;
    }
    return 0;
}

/* This function takes the key that is passed, and does the
 * appropriate action with it (change class, or other things.
 */

int key_change_class(object *op, char key)
{
      int tmp_loop;
#ifdef PLUGINS
    int evtid;
    CFParm CFP;
#endif

    if(key=='q'||key=='Q') {
      remove_ob(op);
      play_again(op);
      return 0;
    }
    if(key=='d'||key=='D') {
	char buf[MAX_BUF];

	SET_FLAG(op, FLAG_NO_FIX_PLAYER);
	/* this must before then initial items are given */
	esrv_new_player(op->contr, op->weight+op->carrying);
#ifdef PLUGINS
    /* GROS : Here we handle the BORN global event */
    evtid = EVENT_BORN;
    CFP.Value[0] = (void *)(&evtid);
    CFP.Value[1] = (void *)(op);
    GlobalEvent(&CFP);

    /* GROS : We then generate a LOGIN event */
    evtid = EVENT_LOGIN;
    CFP.Value[0] = (void *)(&evtid);
    CFP.Value[1] = (void *)(op->contr);
    CFP.Value[2] = (void *)(op->contr->socket.host);
    GlobalEvent(&CFP);
#endif
	op->contr->state=ST_PLAYING;

	FREE_AND_CLEAR_HASH2(op->msg);

	/* We create this now because some of the unique maps will need it
	 * to save here.
	 */
	sprintf(buf,"%s/%s/%s",settings.localdir,settings.playerdir,op->name);
	make_path_to_file(buf);

#ifdef AUTOSAVE
	op->contr->last_save_tick = pticks;
#endif
	display_motd(op);
	new_draw_info_format(NDI_UNIQUE | NDI_ALL, 5, op,"%s entered the game.",op->name);
	CLEAR_FLAG(op, FLAG_WIZ);
	(void) init_player_exp(op);
	give_initial_items(op,op->randomitems);
	(void) link_player_skills(op);
	CLEAR_FLAG(op, FLAG_NO_FIX_PLAYER);
	op->contr->last_stats.exp=1;			/* force send of skill exp data to client */
	strcpy(op->contr->title,op->race);		/* no title - just what we born */
    fix_player(op);					/* THATS our first fix_player() when we create a new char
									 * add this time, hp and sp will be set 
									 */
	esrv_send_inventory(op, op);
	/* NOW we set our new char in the right map - we have a 100% right init player */
    set_first_map(op);
    SET_FLAG(op, FLAG_FRIENDLY);
    add_friendly_object(op);
	op->contr->socket.update_look=1;
    op->contr->socket.look_position=0;
    op->contr->socket.ext_title_flag = 1;
    esrv_new_player(op->contr,op->weight+op->carrying);
    send_skilllist_cmd(op, NULL, SPLIST_MODE_ADD);
    send_spelllist_cmd(op, NULL, SPLIST_MODE_ADD);
	return 0;
    }

    /* Following actually changes the class - this is the default command
     * if we don't match with one of the options above.
     */

    tmp_loop = 0;
    while(!tmp_loop) {
      char *name = NULL;
      int x = op->x, y = op->y;

	  FREE_AND_ADD_REF_HASH(name,op->name);
      remove_statbonus(op);
      op->arch = get_player_archetype(op->arch);
      copy_object (&op->arch->clone, op);
      op->stats = op->contr->orig_stats;
      FREE_AND_CLEAR_HASH2(op->name);
      op->name = name;
      op->x = x;
      op->y = y;
	  SET_ANIMATION(op, 4*(NUM_ANIMATIONS(op)/NUM_FACINGS(op)));     /* So player faces south */
      add_statbonus(op);
      tmp_loop=allowed_class(op);
    }
    update_object(op,UP_OBJ_FACE);
    esrv_update_item(UPD_FACE,op,op);
    op->contr->last_value= -1;
    if (op->msg) 
	new_draw_info(NDI_BLUE, 0, op, op->msg);
    send_query(&op->contr->socket,CS_QUERY_SINGLECHAR,"Press any key for the next race.\nPress `d' to play this race.\n");
    return 0;
}

int key_confirm_quit(object *op, char key)
{
    char buf[MAX_BUF];
#ifdef PLUGINS
    CFParm CFP;
    int evtid;
#endif
    if(key!='y'&&key!='Y'&&key!='q'&&key!='Q') {
      op->contr->state=ST_PLAYING;
      new_draw_info(NDI_UNIQUE, 0,op,"OK, continuing to play.");
      return 1;
    }
#ifdef PLUGINS
    /* GROS : Here we handle the REMOVE global event */
    evtid = EVENT_REMOVE;
    CFP.Value[0] = (void *)(&evtid);
    CFP.Value[1] = (void *)(op);
    GlobalEvent(&CFP);
#endif
    terminate_all_pets(op);
    leave_map(op);
    op->direction=0;
    new_draw_info_format(NDI_UNIQUE | NDI_ALL, 5, NULL,
	"%s quits the game.",op->name);

    strcpy(op->contr->killer,"quit");
    check_score(op);
    op->contr->party_number=(-1);
    if(!QUERY_FLAG(op,FLAG_WAS_WIZ)) {
      sprintf(buf,"%s/%s/%s/%s.pl",settings.localdir,settings.playerdir,op->name,op->name);
      if(unlink(buf)== -1 && settings.debug >= llevDebug)
		LOG(llevBug,"BUG: crossfire (delete character): %s\n", buf);
    }
    play_again(op);
    return 1;
}


void flee_player(object *op) {
  int dir,diff;
  if(op->stats.hp < 0) {
    LOG(llevDebug, "Fleeing player is dead.\n");
    CLEAR_FLAG(op, FLAG_SCARED);
    return;
  }
  if(op->enemy==NULL) {
    LOG(llevDebug,"Fleeing player had no enemy.\n");
    CLEAR_FLAG(op, FLAG_SCARED);
    return;
  }
  if(!(random_roll(0, 4, op, PREFER_LOW)) &&
     random_roll(1, 20, op, PREFER_HIGH) >= savethrow[op->level]) {
    op->enemy=NULL;
    CLEAR_FLAG(op, FLAG_SCARED);
    return;
  }
  dir=absdir(4+find_dir_2(op->x-op->enemy->x,op->y-op->enemy->y));
  for(diff=0;diff<3;diff++) {
    int m=1-(RANDOM()&2);
    if(move_ob(op,absdir(dir+diff*m),op)||
       (diff==0&&move_ob(op,absdir(dir-diff*m),op))) {
	/*
      draw_client_map(op);
	*/
      return;
    }
  }
  /* Cornered, get rid of scared */
  CLEAR_FLAG(op, FLAG_SCARED);
  op->enemy=NULL;
}


/* check_pick sees if there is stuff to be picked up/picks up stuff.
 * IT returns 1 if the player should keep on moving, 0 if he should
 * stop.
 */
int check_pick(object *op) {
  object *tmp, *next;
  tag_t next_tag=0, op_tag;
  int stop = 0;
  int j, k, wvratio;
  char putstring[128], tmpstr[16];


  /* if you're flying, you cna't pick up anything */
  if (QUERY_FLAG (op, FLAG_FLYING))
    return 1;

  op_tag = op->count;

  next = op->below;
  if (next)
    next_tag = next->count;

  /* loop while there are items on the floor that are not marked as
   * destroyed */
  while (next && ! was_destroyed (next, next_tag))
  {
    tmp = next;
    next = tmp->below;
    if (next)
      next_tag = next->count;

    if (was_destroyed (op, op_tag))
        return 0;

    if ( ! can_pick (op, tmp))
      continue;

#ifdef SEARCH_ITEMS
    if (op->contr->search_str[0]!='\0')
    {
      if (item_matched_string (op, tmp, op->contr->search_str))
        pick_up (op, tmp);
      continue;
    }
#endif /* SEARCH_ITEMS */

    /* high bit set?  We're using the new autopickup model */
    if(!(op->contr->mode & PU_NEWMODE))
    { 
    switch (op->contr->mode) {
	case 0:	return 1;	/* don't pick up */
	case 1: pick_up (op, tmp);
		return 1;
	case 2: pick_up (op, tmp);
		return 0;
	case 3: return 0;	/* stop before pickup */
	case 4: pick_up (op, tmp);
		break;
	case 5: pick_up (op, tmp);
		stop = 1;
		break;
	case 6:
		if (QUERY_FLAG (tmp, FLAG_KNOWN_MAGICAL) &&
		    ! QUERY_FLAG(tmp, FLAG_KNOWN_CURSED))
		  pick_up(op, tmp);
		break;

	case 7:
		if (tmp->type == MONEY || tmp->type == GEM|| tmp->type == TYPE_JEWEL|| tmp->type == TYPE_NUGGET)
		  pick_up(op, tmp);
		break;

	default:
		/* use value density */
		if ( ! QUERY_FLAG (tmp, FLAG_UNPAID)
		    && (query_cost (tmp, op, F_TRUE) * 100.0
		        / ((double)tmp->weight * (double)(MAX (tmp->nrof, 1))))
                       >= (double)op->contr->mode)
		  pick_up(op,tmp);
    }
    } /* old model */
    else
    {
      /* NEW pickup handling */
      if(op->contr->mode & PU_DEBUG)
      {
	/* some debugging code to figure out item information */
	if(tmp->name!=NULL)
	  sprintf(putstring,"item name: %s    item type: %d    weight/value: %d",
	      tmp->name, tmp->type,
	      (int)(query_cost(tmp, op, F_TRUE)*100 / (tmp->weight * MAX(tmp->nrof,1))));
	else
	  sprintf(putstring,"item name: %s    item type: %d    weight/value: %d",
	      tmp->arch->name, tmp->type,
	      (int)(query_cost(tmp, op, F_TRUE)*100 / (tmp->weight * MAX(tmp->nrof,1))));
	new_draw_info(NDI_UNIQUE, 0,op,putstring);

	sprintf(putstring,"...flags: ");
	for(k=0;k<4;k++)
	{
	  for(j=0;j<32;j++)
	  {
	    if((tmp->flags[k]>>j)&0x01)
	    {
	      sprintf(tmpstr,"%d ",k*32+j);
	      strcat(putstring, tmpstr);
	    }
	  }
	}
	new_draw_info(NDI_UNIQUE, 0,op,putstring);

#if 0
	/* print the flags too */
	for(k=0;k<4;k++)
	{
	  LOG(llevInfo ,"%d [%d] ", k, k*32+31);
	  for(j=0;j<32;j++)
	  {
	    LOG(llevInfo ,"%d",tmp->flags[k]>>(31-j)&0x01);
	    if(!((j+1)%4))LOG(llevInfo ," ");
	  }
	  LOG(llevInfo ," [%d]\n", k*32);
	}
#endif
      }
      /* philosophy:
       * It's easy to grab an item type from a pile, as long as it's
       * generic.  This takes no game-time.  For more detailed pickups
       * and selections, select-items shoul dbe used.  This is a
       * grab-as-you-run type mode that's really useful for arrows for
       * example.
       * The drawback: right now it has no frontend, so you need to
       * stick the bits you want into a calculator in hex mode and then
       * convert to decimal and then 'pickup <#>
       */

      /* the first two modes are exclusive: if NOTHING we return, if
       * STOP then we stop.  All the rest are applied sequentially,
       * meaning if any test passes, the item gets picked up. */

      /* if mode is set to pick nothing up, return */
      if(op->contr->mode & PU_NOTHING) return 1;
      /* if mode is set to stop when encountering objects, return */
      /* take STOP before INHIBIT since it doesn't actually pick
       * anything up */
      if(op->contr->mode & PU_STOP) return 0;
      /* useful for going into stores and not losing your settings... */
      /* and for battles wher you don't want to get loaded down while
       * fighting */
      if(op->contr->mode & PU_INHIBIT) return 1;

      /* prevent us from turning into auto-thieves :) */
      if (QUERY_FLAG (tmp, FLAG_UNPAID)) continue;

      /* all food and drink if desired */
      /* question: don't pick up known-poisonous stuff? */
      if(op->contr->mode & PU_FOOD)
	if (tmp->type == FOOD)
	{ pick_up(op, tmp); /*LOG(llevInfo ,"FOOD\n");*/ continue; }
      if(op->contr->mode & PU_DRINK)
	if (tmp->type == DRINK)
	{ pick_up(op, tmp); /*LOG(llevInfo ,"DRINK\n");*/ continue; }
      if(op->contr->mode & PU_POTION)
	if (tmp->type == POTION)
	{ pick_up(op, tmp); /*LOG(llevInfo ,"POTION\n");*/ continue; }

      /* pick up all magical items */
      if(op->contr->mode & PU_MAGICAL)
	if (QUERY_FLAG (tmp, FLAG_KNOWN_MAGICAL) && ! QUERY_FLAG(tmp, FLAG_KNOWN_CURSED))
	{ pick_up(op, tmp); /*LOG(llevInfo ,"MAGICAL\n");*/ continue; }

      if(op->contr->mode & PU_VALUABLES)
      {
	if (tmp->type == MONEY || tmp->type == GEM|| tmp->type == TYPE_JEWEL|| tmp->type == TYPE_NUGGET) 
	{ pick_up(op, tmp); /*LOG(llevInfo ,"MONEY/GEM\n");*/ continue; }
      }

      /* bows and arrows. Bows are good for selling! */
      if(op->contr->mode & PU_BOW)
	if (tmp->type == BOW) 
	{ pick_up(op, tmp); /*LOG(llevInfo ,"BOW\n");*/ continue; }
      if(op->contr->mode & PU_ARROW)
	if (tmp->type == ARROW)
	{ pick_up(op, tmp); /*LOG(llevInfo ,"ARROW\n");*/ continue; }

      /* all kinds of armor etc. */
      if(op->contr->mode & PU_ARMOUR)
	if (tmp->type == ARMOUR) 
	{ pick_up(op, tmp); /*LOG(llevInfo ,"ARMOUR\n");*/ continue; }
      if(op->contr->mode & PU_HELMET)
	if (tmp->type == HELMET) 
	{ pick_up(op, tmp); /*LOG(llevInfo ,"HELMET\n");*/ continue; }
      if(op->contr->mode & PU_SHIELD)
	if (tmp->type == SHIELD) 
	{ pick_up(op, tmp); /*LOG(llevInfo ,"SHIELD\n");*/ continue; }
      if(op->contr->mode & PU_BOOTS)
	if (tmp->type == BOOTS) 
	{ pick_up(op, tmp); /*LOG(llevInfo ,"BOOTS\n");*/ continue; }
      if(op->contr->mode & PU_GLOVES)
	if (tmp->type == GLOVES) 
	{ pick_up(op, tmp); /*LOG(llevInfo ,"GLOVES\n");*/ continue; }
      if(op->contr->mode & PU_CLOAK)
	if (tmp->type == CLOAK) 
	{ pick_up(op, tmp); /*LOG(llevInfo ,"CLOAK\n");*/ continue; }

      /* hoping to catch throwing daggers here */
      if(op->contr->mode & PU_MISSILEWEAPON)
	if(tmp->type == WEAPON && QUERY_FLAG(tmp, FLAG_IS_THROWN)) 
	{ pick_up(op, tmp); /*LOG(llevInfo ,"MISSILEWEAPON\n");*/ continue; }

      /* careful: chairs and tables are weapons! */
      if(op->contr->mode & PU_ALLWEAPON)
      {
	if(tmp->type == WEAPON && tmp->name!=NULL) 
	{
	  if(strstr(tmp->name,"table")==NULL && strstr(tmp->arch->name,"table")==NULL &&
	      strstr(tmp->name,"chair") && strstr(tmp->arch->name,"chair")==NULL) 
	  { pick_up(op, tmp); /*LOG(llevInfo ,"WEAPON\n");*/ continue; }
	}
	if(tmp->type == WEAPON && tmp->name==NULL) 
	{
	  if(strstr(tmp->arch->name,"table")==NULL &&
	      strstr(tmp->arch->name,"chair")==NULL) 
	  { pick_up(op, tmp); /*LOG(llevInfo ,"WEAPON\n");*/ continue; }
	}
      }

      /* misc stuff that's useful */
      if(op->contr->mode & PU_KEY)
	if (tmp->type == KEY || tmp->type == SPECIAL_KEY) 
	{ pick_up(op, tmp); /*LOG(llevInfo ,"KEY\n");*/ continue; }

      /* any of the last 4 bits set means we use the ratio for value
       * pickups */
      if(op->contr->mode & PU_RATIO)
      {
	/* use value density to decide what else to grab */
	/* >=7 was >= op->contr->mode */
	/* >=7 is the old standard setting.  Now we take the last 4 bits
	 * and multiply them by 5, giving 0..15*5== 5..75 */
	wvratio=(op->contr->mode & PU_RATIO) * 5;
	if ((query_cost(tmp, op, F_TRUE)*100 / (tmp->weight * MAX((signed long)tmp->nrof, 1))) >= wvratio)
	{
	  pick_up(op, tmp);

	  /*
	  LOG(llevInfo ,"HIGH WEIGHT/VALUE [");
	  if(tmp->name!=NULL) {
	    LOG(llevInfo ,"%s", tmp->name);
	  }
	  else 
	  LOG(llevInfo ,"%s",tmp->arch->name);
	  LOG(llevInfo ,",%d] = ", tmp->type);
	  LOG(llevInfo ,"%d\n",(int)(query_cost(tmp,op,F_TRUE)*100 / (tmp->weight * MAX(tmp->nrof,1))));
	  */
	  continue;
	}
      }
    } /* the new pickup model */
  }
  return ! stop;
}

/*
 *  Find an arrow in the inventory and after that
 *  in the right type container (quiver). Pointer to the 
 *  found object is returned.
 */
object *find_arrow(object *op, char *type)
{
  object *tmp = NULL;

  for(op=op->inv; op; op=op->below)
    if(!tmp && op->type==CONTAINER && op->race==type &&
      QUERY_FLAG(op,FLAG_APPLIED))
      tmp = find_arrow (op, type);
    else if (op->type==ARROW && op->race==type)
      return op;
  return tmp;
}

/*
 *  Player fires a bow. This probably should be combined with
 *  monster_use_bow().
 */
static void fire_bow(object *op, int dir)
{
  object *bow, *arrow = NULL, *left, *tmp_op;
  tag_t left_tag;

  if (!dir) {
    new_draw_info(NDI_UNIQUE, 0,op, "You can't shoot yourself!");
    return;
  }

  /* TODO: put applied_bow ptr for quick access in player struct */
  for(bow=op->inv; bow; bow=bow->below)
    if(bow->type==BOW && QUERY_FLAG(bow, FLAG_APPLIED))
      break;

  if (!bow)
    LOG(llevBug, "BUG: Range: bow without activated bow (%s - %d).\n", op->name, dir);
  if( !bow->race ) {
    new_draw_info_format(NDI_UNIQUE, 0,op, "Your %s is broken.", bow->name);
    return;
  }
  if ((arrow=find_arrow_ext(op, bow->race, op->contr->firemode_tag2)) == NULL) {
    new_draw_info_format(NDI_UNIQUE, 0,op,"You have no %s left.", bow->race);
    return;
  }
  if(wall(op->map,op->x+freearr_x[dir],op->y+freearr_y[dir])) {
    new_draw_info(NDI_UNIQUE, 0,op,"Something is in the way.");
    return;
  }
  /* this should not happen, but sometimes does */
  if (arrow->nrof==0) {
	remove_ob(arrow);
	free_object(arrow);
	return;
  }
  left = arrow; /* these are arrows left to the player */
  left_tag = left->count;
  arrow = get_split_ob(arrow, 1);
  set_owner(arrow,op);
  arrow->direction=dir;
  arrow->x = op->x;
  arrow->y = op->y;
  arrow->speed = 1;
  op->speed_left = 0.01f - FABS(op->speed) * 100.0f / (float)bow->stats.sp;
  fix_player(op);
  update_ob_speed(arrow);
  arrow->speed_left = 0;
  SET_ANIMATION(arrow, (NUM_ANIMATIONS(arrow)/NUM_FACINGS(arrow))*dir);
  arrow->last_heal = arrow->stats.wc; /* save original wc and dam */
  arrow->stats.hp = arrow->stats.dam; /* will be put back in fix_arrow() */

  /* now we do this: arrow wc = wc base from skill + (wc arrow + magic) + (wc range weapon boni + magic) */
	if((tmp_op=SK_skill(op)))
		arrow->stats.wc = tmp_op->last_heal; /* wc is in last heal */
	else
		arrow->stats.wc = 10;

	/* now we determinate how many tiles the arrow will fly.
	 * again we use the skill base and add arrow + weapon values - but no magic add here.
	 */
	arrow->last_sp = tmp_op->last_sp + bow->last_sp + arrow->last_sp;

	/* add in all our wc boni */
	arrow->stats.wc += (bow->magic + arrow->magic+SK_level(op)+thaco_bonus[op->stats.Dex] + bow->stats.wc);

	/* i really like the idea to use here the bow wc_range! */
	arrow->stats.wc_range = bow->stats.wc_range;

	/* monster.c 970 holds the arrow code for monsters */
	arrow->stats.dam += dam_bonus[op->stats.Str]/2 + bow->stats.dam + bow->magic + arrow->magic; 
	arrow->stats.dam=FABS((int)((float)(arrow->stats.dam  *lev_damage[SK_level(op)])));
  
	/* adjust with the lower of condition */
	if(bow->item_condition > arrow->item_condition)
		arrow->stats.dam = (sint16)(((float)arrow->stats.dam/100.0f)*(float)arrow->item_condition);
	else
		arrow->stats.dam = (sint16)(((float)arrow->stats.dam/100.0f)*(float)bow->item_condition);


	arrow->level = SK_level (op); /* this is used temporary when fired, arrow has
								 * no level use elsewhere.
								 */
  arrow->map = op->map;
  SET_FLAG(arrow, FLAG_FLYING);
  SET_FLAG(arrow, FLAG_FLY_ON);
  SET_FLAG(arrow, FLAG_WALK_ON);
  play_sound_map(op->map, op->x, op->y, SOUND_FIRE_ARROW, SOUND_NORMAL);
  insert_ob_in_map(arrow,op->map,op,0);
  move_arrow(arrow);
  if (was_destroyed (left, left_tag))
      esrv_del_item(op->contr, left_tag);
  else
      esrv_send_item(op, left);
}


void fire(object *op,int dir) {
  object *weap=NULL;
  int spellcost=0;

  /* check for loss of invisiblity/hide */
  if (action_makes_visible(op)) make_visible(op);

   /* a check for players, make sure things are groovy. This routine
    * will change the skill of the player as appropriate in order to
    * fire whatever is requested. In the case of spells (range_magic)
    * it handles whether cleric or mage spell is requested to be cast. 
    * -b.t. 
    */
  
    /* ext. fire mode - first step. We map the client side action to a server action. */
    /* forcing the shoottype var from player object to our needed range mode */
    if(op->type==PLAYER) 
    {
        if(op->contr->firemode_type == FIRE_MODE_NONE)
            return;
            
        if(op->contr->firemode_type == FIRE_MODE_BOW)
            op->contr->shoottype=range_bow;
        else if(op->contr->firemode_type == FIRE_MODE_THROW)
        {
            object *tmp;

            /* insert here test for more throwing skills */
            if(!change_skill(op,SK_THROWING))
                return;
            /* special case - we must redirect the fire cmd to throwing something */
            tmp = find_throw_tag(op, (tag_t) op->contr->firemode_tag1);            
            if(tmp)
			{
                do_throw(op,tmp,dir);            
				op->speed_left -= get_skill_time(op,op->chosen_skill->stats.sp);
			}
            return;
        }
        else if(op->contr->firemode_type == FIRE_MODE_SPELL)
            op->contr->shoottype=range_magic;
        else if(op->contr->firemode_type == FIRE_MODE_WAND)
        {
            

            op->contr->shoottype=range_wand; /* we do a jump in fire wand if we haven one */
        }
        else if(op->contr->firemode_type == FIRE_MODE_SKILL)
        {

            command_rskill (op, op->contr->firemode_name);
            op->contr->shoottype=range_skill;
        }
        else if(op->contr->firemode_type == FIRE_MODE_SUMMON)
            op->contr->shoottype=range_scroll;
        else
            op->contr->shoottype=range_none;

        if(!check_skill_to_fire(op))
            return;
    }
    
  switch(op->contr->shoottype) {
  case range_none:
    return;

  case range_bow:
    if(op->contr->firemode_tag2!= -1)
	{
        fire_bow(op, dir);
		op->speed_left -= get_skill_time(op,op->chosen_skill->stats.sp);
	}
    return;

  case range_magic: /* Casting spells */

    spellcost=cast_spell(op,op,dir,op->contr->chosen_spell,0,spellNormal,NULL);
	/* we should use spell time not skill time */
	/*op->speed_left -= get_skill_time(op,op->chosen_skill->stats.sp); */

    if(spells[op->contr->chosen_spell].flags&SPELL_DESC_WIS)
        op->stats.grace-=spellcost;
    else
		op->stats.sp-=spellcost;

    return;

  case range_wand:
      for(weap=op->inv;weap!=NULL;weap=weap->below)
          if(weap->type==WAND&&QUERY_FLAG(weap, FLAG_APPLIED))
              break;
          if(weap==NULL) {
              op->contr->shoottype=range_rod;
              goto trick_jump;
          }

    if(weap->stats.food<=0) {
      play_sound_player_only(op->contr, SOUND_WAND_POOF,SOUND_NORMAL,0,0);
      new_draw_info(NDI_UNIQUE, 0,op,"The wand says poof.");
	  op->speed_left -= get_skill_time(op,op->chosen_skill->stats.sp);
      return;
    }
    
    new_draw_info(NDI_UNIQUE, 0,op, "fire wand");
    if(cast_spell(op,weap,dir,weap->stats.sp,0,spellWand,NULL)) {
      SET_FLAG(op, FLAG_BEEN_APPLIED); /* You now know something about it */
      if (!(--weap->stats.food))
      {
	object *tmp;
	if (weap->arch) {
	  CLEAR_FLAG(weap, FLAG_ANIMATE);
	  weap->face = weap->arch->clone.face;
	  weap->speed = 0;
	  update_ob_speed(weap);
	}
	if ((tmp=is_player_inv(weap)))
	    esrv_update_item(UPD_ANIM, tmp, weap);
      }
    }
	op->speed_left -= get_skill_time(op,op->chosen_skill->stats.sp);
    return;
  case range_rod:
  case range_horn:
trick_jump:
      for(weap=op->inv;weap!=NULL;weap=weap->below)
          if(QUERY_FLAG(weap, FLAG_APPLIED)&&
              weap->type==(op->contr->shoottype==range_rod?ROD:HORN))
              break;
          if(weap==NULL) {
              if(op->contr->shoottype==range_rod)
              {
                  op->contr->shoottype=range_horn;
                  goto trick_jump;
              }
              else
              {
                char buf[MAX_BUF];
                sprintf(buf, "You have no tool readied.");
                new_draw_info(NDI_UNIQUE, 0,op, buf);
                return;
              }
          }
          
    if(weap->stats.hp<spells[weap->stats.sp].sp) {
      play_sound_player_only(op->contr, SOUND_WAND_POOF,SOUND_NORMAL,0,0);
      if (op->contr->shoottype == range_rod)
	new_draw_info(NDI_UNIQUE, 0,op,"The rod whines for a while, but nothing happens.");
      else
	new_draw_info(NDI_UNIQUE, 0,op,
	          "No matter how hard you try you can't get another note out.");
	  op->speed_left -= get_skill_time(op,op->chosen_skill->stats.sp);
      return;
    }
    /*new_draw_info_format(NDI_ALL|NDI_UNIQUE,5,NULL,"Use %s - cast spell %d\n",weap->name,weap->stats.sp);*/
    if(cast_spell(op,weap,dir,weap->stats.sp,0, op->contr->shoottype == range_rod ? spellRod : spellHorn,NULL))
    {
      SET_FLAG(op, FLAG_BEEN_APPLIED); /* You now know something about it */
      drain_rod_charge(weap);
    }
	  op->speed_left -= get_skill_time(op,op->chosen_skill->stats.sp);
    return;
  case range_scroll: /* Control summoned monsters from scrolls */
    if(op->contr->golem==NULL) {
      op->contr->shoottype=range_none;
      op->contr->chosen_spell= -1;
    }
    else 
	control_golem(op->contr->golem, dir);
    return;

  case range_skill:
    if(!op->chosen_skill) { 
	if(op->type==PLAYER)
      	    new_draw_info(NDI_UNIQUE, 0,op,"You have no applicable skill to use.");
	return;
    }
	if(op->chosen_skill->sub_type1 != ST1_SKILL_USE)
      	    new_draw_info(NDI_UNIQUE, 0,op,"You can't use this skill in this way.");
	else
	    (void) do_skill(op,dir,NULL);
    return;
  default:
    new_draw_info(NDI_UNIQUE, 0,op,"Illegal shoot type.");
    return;
  }
}



/* find_key
 * We try to find a key for the door as passed.  If we find a key
 * and successfully use it, we return the key, otherwise NULL
 * This function merges both normal and locked door, since the logic
 * for both is the same - just the specific key is different.
 * pl is the player, 
 * inv is the objects inventory to searched 
 * door is the door we are trying to match against.
 * This function can be called recursively to search containers.
 */

object * find_key(object *pl, object *container, object *door)
{
    object *tmp,*key;

    /* Should not happen, but sanity checking is never bad */
    if (container->inv == NULL) return NULL;

    /* First, lets try to find a key in the top level inventory */
    for (tmp=container->inv; tmp!=NULL; tmp=tmp->below) {
	if (door->type==DOOR && tmp->type==KEY) break;
	/* For sanity, we should really check door type, but other stuff 
	 * (like containers) can be locked with special keys
	 */
	if (tmp->slaying && tmp->type==SPECIAL_KEY &&
	    tmp->slaying==door->slaying) break;
    }
    /* No key found - lets search inventories now */
    /* If we find and use a key in an inventory, return at that time.
     * otherwise, if we search all the inventories and still don't find
     * a key, return
     */
    if (!tmp) {
	for (tmp=container->inv; tmp!=NULL; tmp=tmp->below) {
	    /* No reason to search empty containers */
	    if (tmp->type==CONTAINER && tmp->inv) {
		if ((key=find_key(pl, tmp, door))!=NULL) return key;
	    }
	}
	if (!tmp) return NULL;
    }
    /* We get down here if we have found a key.  Now if its in a container,
     * see if we actually want to use it
     */
    if (pl!=container) {
	/* Only let players use keys in containers */
	if (!pl->contr) return NULL;
	/* cases where this fails:
	 * If we only search the player inventory, return now since we
	 * are not in the players inventory.
	 * If the container is not active, return now since only active
	 * containers can be used.
	 * If we only search keyrings and the container does not have
	 * a race/isn't a keyring.
	 * No checking for all containers - to fall through past here,
	 * inv must have been an container and must have been active.
	 *
	 * Change the color so that the message doesn't disappear with
	 * all the others.
	 */
	if (pl->contr->usekeys == key_inventory ||
	    !QUERY_FLAG(container, FLAG_APPLIED) ||
	    (pl->contr->usekeys == keyrings &&
	     (!container->race || strcmp(container->race, "keys")))
	      ) {
	    new_draw_info_format(NDI_UNIQUE|NDI_BROWN, 0, pl, 
		"The %s in your %s vibrates as you approach the door",
		query_name(tmp), query_name(container));
	    return NULL;
	}
    }
    return tmp;
}

/* moved door processing out of move_player_attack.
 * returns 1 if player has opened the door with a key
 * such that the caller should not do anything more,
 * 0 otherwise
 */
static int player_attack_door(object *op, object *door)
{

    /* If its a door, try to find a use a key.  If we do destroy the door,
     * might as well return immediately as there is nothing more to do -
     * otherwise, we fall through to the rest of the code.
     */
    object *key=find_key(op, op, door);

    /* IF we found a key, do some extra work */
    if (key) {
	object *container=key->env;

	if(action_makes_visible(op)) make_visible(op);
	if(door->inv && door->inv->type ==RUNE) spring_trap(door->inv,op);
	if (door->type == DOOR) {
	    hit_player(door,9998,op,AT_PHYSICAL); /* Break through the door */
	}
	else if(door->type==LOCKED_DOOR) {
	    new_draw_info_format(NDI_UNIQUE, NDI_BROWN, op, 
		     "You open the door with the %s", query_short_name(key));
	    remove_door2(door, op); /* remove door without violence ;-) */
	}
	/* Do this after we print the message */
	decrease_ob(key); /* Use up one of the keys */
	/* Need to update the weight the container the key was in */
	if (container != op) 
	    esrv_update_item(UPD_WEIGHT, op, container);
	return 1; /* Nothing more to do below */
    } else if (door->type==LOCKED_DOOR) 
	{
		if(!door->slaying)
		    remove_door2(door,op); /* remove door without violence ;-) */	
		else /* Might as well return now - no other way to open this */
			new_draw_info(NDI_UNIQUE | NDI_NAVY, 0, op, door->msg);
		return 1;
    }
    return 0;
}

/* This function is just part of a breakup from move_player.
 * It should keep the code cleaner.
 * When this is called, the players direction has been updated
 * (taking into accoutn confusion.)  The player is also actually
 * going to try and move (not fire weapons).
 */
int move_player_attack(object *op, int dir)
{
    int ret=0;
    object *tmp;
    int nx=freearr_x[dir]+op->x,ny=freearr_y[dir]+op->y;
    mapstruct *m;

    if (!move_ob(op,dir,op)) 
	{
		if(!(m=out_of_map(op->map,&nx,&ny)))
			return ret;
    
		if ((tmp=get_map_ob(m,nx,ny))==NULL) 
		{
			/*	LOG(llevBug,"BUG: player_move_attack: get_map_ob returns NULL, but player can not more there.\n");*/
			return ret;
		}

		/* Go through all the objects, and stop if we find one of interest. */
		while (tmp->above!=NULL) 
		{
			if ((QUERY_FLAG(tmp,FLAG_ALIVE) || QUERY_FLAG(tmp,FLAG_CAN_ROLL)
										|| tmp->type ==LOCKED_DOOR) && tmp!=op)
				break;
			tmp=tmp->above;
		}
    
		if (tmp==NULL)		/* This happens anytime the player tries to move */
			return ret;		/* into a wall */

		if(tmp->head != NULL)
			tmp = tmp->head;

		if ((tmp->type==DOOR && tmp->stats.hp>=0) || (tmp->type==LOCKED_DOOR))
		{
			if (player_attack_door(op, tmp))
				return ret;
		}
		/* START OLD COMMENTS*/
		/* The following deals with possibly attacking peaceful
		* or frienddly creatures.  Basically, all players are considered
		* unaggressive.  If the moving player has peaceful set, then the
		* object should be pushed instead of attacked.  It is assumed that
		* if you are braced, you will not attack friends accidently,
		* and thus will not push them.
		*/

		/* If the creature is a pet, push it even if the player is not
		* peaceful.  Our assumption is the creature is a pet if the
		* player owns it and it is either friendly or unagressive.
		*/
		/* END OLD COMMENTS*/

		/* i don't like this code. It means: push a pet/golem when its yours and
		 * it is not aggressive to you. This means that a golem or pet can be
		 * aggressive to his owner. This will break the logic of pets/golems on
		 * other place. When a golem/pet gets aggressive against his owner, the owner
		 * should lose control and over it and it should handled as a non pet mob.
		 * Then the owner can summon a new pet or whatever. MT.
		 */
/*		if (op->type==PLAYER && get_owner(tmp)==op && 
	    (QUERY_FLAG(tmp,FLAG_UNAGGRESSIVE) ||  QUERY_FLAG(tmp, FLAG_FRIENDLY))) */
	/* merged this with code below 		    
		if ((tmp->type==PLAYER || tmp->enemy != op) &&
					(tmp->type==PLAYER || QUERY_FLAG(tmp,FLAG_UNAGGRESSIVE) ||
					QUERY_FLAG(tmp, FLAG_FRIENDLY)) && (op->contr->peaceful &&
					!op_on_battleground(op, NULL, NULL))) 
		{
			play_sound_map(op->map, op->x, op->y, SOUND_PUSH_PLAYER, SOUND_NORMAL);
			if(push_ob(tmp,dir,op))
				ret = 1;
			if(op->contr->tmp_invis||op->hide) 
				make_visible(op);
		}
		*/

		/* try push mobs, pets and friendly npcs when not on bg */
		if (get_owner(tmp)==op ||  /* it is our pet */
			((QUERY_FLAG(tmp,FLAG_UNAGGRESSIVE) || tmp->type==PLAYER || 
			QUERY_FLAG(tmp, FLAG_FRIENDLY)) && !op_on_battleground(op, NULL, NULL))) /* or a player friendly ojbect  on non bg */
		{
			play_sound_map(op->map, op->x, op->y, SOUND_PUSH_PLAYER, SOUND_NORMAL);
			if(push_ob(tmp,dir,op))
				ret = 1;
			if(op->contr->tmp_invis||op->hide) 
				make_visible(op);
			return ret;
		}
		else if(QUERY_FLAG(tmp,FLAG_CAN_ROLL)) /* or try to roll the object */ 
		{
			recursive_roll(tmp,dir,op);
			if(action_makes_visible(op)) 
				make_visible(op);
		}

		/* Any generic living creature.  Including things like doors.
		* Way it works is like this:  First, it must have some hit points
		* and be living.  Then, it must be one of the following:
		* 1) Not a player, 2) A player, but of a different party.  Note
		* that party_number -1 is no party, so attacks can still happen.
		*/
		else if ((tmp->stats.hp>=0) && QUERY_FLAG(tmp, FLAG_ALIVE) &&
				((tmp->type!=PLAYER || op->contr->party_number==-1 ||
					op->contr->party_number!=tmp->contr->party_number))) 
		{
			op->enemy = tmp; /* instead of attack, we assign it as our enemy */
			op->enemy_count = tmp->count;
			ret = 1;
		}
    } /* if player should attack something */
    return ret;
}

int move_player(object *op,int dir) {
    int face, pick;

    face = dir ? (dir - 1) / 2 : -1;

	op->contr->praying=0;

    if(op->map == NULL || op->map->in_memory != MAP_IN_MEMORY)
		return 0;

	if(dir)
		op->facing = dir;
    if(QUERY_FLAG(op,FLAG_CONFUSED) && dir)
		dir = absdir(dir + RANDOM()%3 + RANDOM()%3 - 2);

    op->anim_moving_dir = -1;
	op->anim_enemy_dir = -1;
    op->anim_last_facing = -1;

	/* puh, we should move hide to FLAG_ ASAP */
    if(op->hide) 
    {
        op->anim_moving_dir = dir;
        do_hidden_move(op);
    }
    
	/* firemode is set from client command fire xx xx xx */
    if(op->contr->firemode_type!=-1)
    {
    	fire(op,dir);
		if(dir)
	        op->anim_enemy_dir = dir;
		else
	        op->anim_enemy_dir = op->facing;
		op->contr->fire_on=0;
        
    }
    else
    {
        if(move_player_attack(op,dir))
            op->anim_enemy_dir = dir;
        else
            op->anim_moving_dir = dir;
    }

    /* Add special check for newcs players and fire on - this way, the
     * server can handle repeat firing.
     */
    pick = check_pick(op);
    if (op->contr->fire_on || (op->contr->run_on && pick!=0)) {
	op->direction = dir;
    } else {
	op->direction=0;
    }

	if(QUERY_FLAG(op,FLAG_ANIMATE) ) /* hm, should be not needed - players always animated */
	{
		if(op->anim_enemy_dir == -1 && op->anim_moving_dir == -1)
			op->anim_last_facing = dir;
		animate_object(op, 0);
	}
    return 0;
}

/* This is similar to handle_player, below, but is only used by the
 * new client/server stuff.
 * This is sort of special, in that the new client/server actually uses
 * the new speed values for commands.
 *
 * Returns true if there are more actions we can do.
 */
int handle_newcs_player(object *op)
{
	/* ouch. here is the invisible counter... MT-11-2002
    if(op->invisible&&!(QUERY_FLAG(op,FLAG_SEE_INVISIBLE))) {
	op->invisible--;
	if(!op->invisible) make_visible(op);
    }
	*/
    /* call this here - we also will call this in do_ericserver, but
     * the players time has been increased when doericserver has been
     * called, so we recheck it here.
     */
    HandleClient(&op->contr->socket, op->contr);
    if (op->speed_left<0) return 0;

    CLEAR_FLAG(op,FLAG_PARALYZED); /* if we are here, we never paralyzed anymore */
    
    if(op->direction && (op->contr->run_on || op->contr->fire_on)) {
	/* All move commands take 1 tick, at least for now */
	op->speed_left--;
	/* Instead of all the stuff below, let move_player take care
	 * of it.  Also, some of the skill stuff is only put in
	 * there, as well as the confusion stuff.
	 */
	move_player(op, op->direction);
	if (op->speed_left>0) return 1;
	else return 0;
    }
    return 0;
}

int save_life(object *op) {
  object *tmp;
  char buf[MAX_BUF];
  if(!QUERY_FLAG(op,FLAG_LIFESAVE))
    return 0;
  for(tmp=op->inv;tmp!=NULL;tmp=tmp->below)
    if(QUERY_FLAG(tmp, FLAG_APPLIED)&&QUERY_FLAG(tmp,FLAG_LIFESAVE)) {
      play_sound_map(op->map, op->x, op->y, SOUND_OB_EVAPORATE, SOUND_NORMAL);
      sprintf(buf,"Your %s vibrates violently, then evaporates.",
	      query_name(tmp));
      new_draw_info(NDI_UNIQUE, 0,op,buf);
      if (op->contr)
	esrv_del_item(op->contr, tmp->count);
      remove_ob(tmp);
      free_object(tmp);
      CLEAR_FLAG(op, FLAG_LIFESAVE);
      if(op->stats.hp<0)
	op->stats.hp = op->stats.maxhp;
      if(op->stats.food<0)
	op->stats.food = 999;
      /*enter_player_savebed(op);*/ /* bring him home. */
      return 1;
    }
  LOG(llevBug,"BUG: LIFESAVE set without applied object.\n");
  CLEAR_FLAG(op, FLAG_LIFESAVE);
  enter_player_savebed(op); /* bring him home. */
  return 0;
}

/* This goes throws the inventory and removes unpaid objects, and puts them
 * back in the map (location and map determined by values of env).  This
 * function will descend into containers.  op is the object to start the search
 * from.
 */
void remove_unpaid_objects(object *op, object *env)
{
    object *next;

    while (op) {
	next=op->below;	/* Make sure we have a good value, in case
			 * we remove object 'op'
			 */
	if (QUERY_FLAG(op, FLAG_UNPAID)) {
	    remove_ob(op);
	    op->x = env->x;
	    op->y = env->y;
	    insert_ob_in_map(op, env->map, NULL,0);
	}
	else if (op->inv) remove_unpaid_objects(op->inv, env);
	op=next;
    }
}


void do_some_living(object *op) 
{

	if(op->contr->state==ST_PLAYING) 
	{
		/* hp reg */
		if(op->contr->gen_hp)
		{
		    if(--op->last_heal<0)	
			{
				op->last_heal=op->contr->base_hp_reg;
				if(op->contr->combat_mode)  
					op->last_heal += op->last_heal; /* halfed reg speed */

				if(op->stats.hp<op->stats.maxhp)
				{
					int last_food=op->stats.food;

					op->stats.hp+=op->contr->reg_hp_num;
					if(op->stats.hp>op->stats.maxhp)
						op->stats.hp= op->stats.maxhp;

					/* faster hp reg - faster digestion... evil */
					op->stats.food--;
					if(op->contr->digestion<0)
						op->stats.food+=op->contr->digestion;
					else if(op->contr->digestion>0 &&
						random_roll(0, op->contr->digestion, op, PREFER_HIGH))
					op->stats.food=last_food;
				}
			}
		}

		/* sp reg */
		if(op->contr->gen_sp)
		{
		    if(--op->last_sp<0) 
			{
				op->last_sp=op->contr->base_sp_reg;
				if(op->stats.sp<op->stats.maxsp)
				{
					op->stats.sp+=op->contr->reg_sp_num;
					if(op->stats.sp>op->stats.maxsp)
						op->stats.sp= op->stats.maxsp;
				}
			}
		}

		/* "stay and pray" mechanism */
		if(op->contr->praying && !op->contr->was_praying)
		{
			if(op->stats.grace<op->stats.maxgrace)
			{
			    object *god = find_god(determine_god(op));
				if(god)
				{
					if(op->contr->combat_mode)
					{
						new_draw_info_format(NDI_UNIQUE, 0,op,"You stop combat and start praying to %s...",god->name);
						op->contr->combat_mode=0;
						send_target_command(op->contr);
					}
					else
						new_draw_info_format(NDI_UNIQUE, 0,op,"You start praying to %s...",god->name);
					op->contr->was_praying=1;
				}
				else
				{
					new_draw_info(NDI_UNIQUE, 0,op, "You worship no deity to pray to!");
					op->contr->praying=0;
				}
				op->last_grace=op->contr->base_grace_reg;
			}
			else
			{
				op->contr->praying=0;
				op->contr->was_praying=0;
			}
		}
		else if(!op->contr->praying && op->contr->was_praying)
		{
			new_draw_info(NDI_UNIQUE, 0,op,"You stop praying.");
			op->contr->was_praying=0;
			op->last_grace=op->contr->base_grace_reg;
		}

		/* grace reg */
		if(op->contr->praying && op->contr->gen_grace)
		{
		    if(--op->last_grace<0) 
			{
				if(op->stats.grace<op->stats.maxgrace)
				op->stats.grace+=op->contr->reg_grace_num;
				if(op->stats.grace>=op->stats.maxgrace)
				{
					op->stats.grace= op->stats.maxgrace;
					new_draw_info(NDI_UNIQUE, 0,op,"Your are full of grace and stop praying.");
					op->contr->was_praying=0;
				}
				op->last_grace=op->contr->base_grace_reg;
			}
		}

		/* Digestion */
		if(--op->last_eat<0)
		{
			int bonus=op->contr->digestion>0?op->contr->digestion:0,
			penalty=op->contr->digestion<0?-op->contr->digestion:0;
			if(op->contr->gen_hp > 0)
				op->last_eat=25*(1+bonus)/(op->contr->gen_hp+penalty+1);
			else
				op->last_eat=25*(1+bonus)/(penalty +1);
			op->stats.food--;
		}

		if(op->stats.food<0&&op->stats.hp>=0) 
		{
			object *tmp, *flesh=NULL;

			for(tmp=op->inv;tmp!=NULL;tmp=tmp->below) 
			{
				if(!QUERY_FLAG(tmp, FLAG_UNPAID))
				{
					if (tmp->type==FOOD || tmp->type==DRINK || tmp->type==POISON) 
					{
						new_draw_info(NDI_UNIQUE, 0,op,"You blindly grab for a bite of food.");
						manual_apply(op,tmp,0);
						if(op->stats.food>=0||op->stats.hp<0)
							break;
					}
					else if (tmp->type==FLESH)
						flesh=tmp;
				} /* End if paid for object */
			} /* end of for loop */

			/* If player is still starving, it means they don't have any food, so
			* eat flesh instead.
			*/
			if (op->stats.food<0 && op->stats.hp>=0 && flesh)
			{
			    new_draw_info(NDI_UNIQUE, 0,op,"You blindly grab for a bite of food.");
			    manual_apply(op,flesh,0);
			}
		} /* end if player is starving */

		while(op->stats.food<0&&op->stats.hp>0)
		{
			op->stats.food++;
			/* new: no dying from food. hp will fall to 1 but not under it.
			 * we must check here for negative because we don't want ADD here
			 */
			if(op->stats.hp)
			{
				op->stats.hp--;
				if(!op->stats.hp)
					op->stats.hp=1;
			}
		};

		/* we can't die by no food but perhaps by poisoned food? */
		if ((op->stats.hp<=0||op->stats.food<0) && !QUERY_FLAG(op,FLAG_WIZ))
			kill_player(op);
	}
}


/* If the player should die (lack of hp, food, etc), we call this.
 * op is the player in jeopardy.  If the player can not be saved (not
 * permadeath, no lifesave), this will take care of removing the player
 * file.
 */
void kill_player(object *op)
{
    char buf[MAX_BUF];
    int x,y,i;
    mapstruct *map;  /*  this is for resurrection */
    object *tmp;
    int z;
    int num_stats_lose;
    int lost_a_stat;
    int lose_this_stat;
    int this_stat;
#ifdef PLUGINS
    int killed_script_rtn; /* GROS: For script return value */
    CFParm CFP;
    int evtid;
#endif
    
    if(save_life(op)) return;

    /* If player dies on BATTLEGROUND, no stat/exp loss! For Combat-Arenas
     * in cities ONLY!!! It is very important that this doesn't get abused.
     * Look at op_on_battleground() for more info       --AndreasV
     */
    
    if (op_on_battleground(op, &x, &y))
    {
        new_draw_info(NDI_UNIQUE | NDI_NAVY, 0,op, "You have been defeated in combat!");
        new_draw_info(NDI_UNIQUE | NDI_NAVY, 0,op, "Local medics have saved your life...");
      
        /* restore player */
        cast_heal(op, 110, op, SP_CURE_POISON);
        /*cast_heal(op, op, SP_CURE_CONFUSION);*/
        cure_disease(op,NULL);  /* remove any disease */
        op->stats.hp=op->stats.maxhp;
        if (op->stats.food<=0) op->stats.food=999;
      
        /* create a bodypart-trophy to make the winner happy */
        tmp=arch_to_object(find_archetype("finger"));
        if (tmp != NULL) 
        {
	        sprintf(buf,"%s's finger",op->name);
	        FREE_AND_COPY_HASH(tmp->name, buf);
	        sprintf(buf,"  This finger has been cut off %s\n"
                        "  the %s, when he was defeated at\n  level %d by %s.\n",
	                        op->name, op->contr->title, (int)(op->level),
	                        op->contr->killer);
	        FREE_AND_COPY_HASH(tmp->msg, buf);
	        tmp->value=0, tmp->material=0, tmp->type=0;
	        tmp->x = op->x, tmp->y = op->y;
	        insert_ob_in_map(tmp,op->map,op,0);
        }
      
        /* teleport defeated player to new destination*/
        transfer_ob(op, x, y, 0, NULL, NULL);
        return;
    }

#ifdef PLUGINS
/* GROS: Handle for plugin death event */
  if(op->event_flags&EVENT_FLAG_DEATH)
  {
    CFParm* CFR;
    int k, l, m;
 	object *event_obj = get_event_object(op, EVENT_DEATH);
    k = EVENT_DEATH;
    l = SCRIPT_FIX_ALL;
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
    CFP.Value[9] = event_obj->race;
    CFP.Value[10]= event_obj->slaying;
    if (findPlugin(event_obj->name)>=0)
    {
        CFR = (PlugList[findPlugin(event_obj->name)].eventfunc) (&CFP);
        killed_script_rtn = *(int *)(CFR->Value[0]);
        free(CFR);
        if (killed_script_rtn)
            return;
    }
  }

  /* GROS: Handle for the global death event */
  evtid = EVENT_GDEATH;
  CFP.Value[0] = (void *)(&evtid);
  CFP.Value[1] = NULL;
  CFP.Value[2] = (void *)(op);
  GlobalEvent(&CFP);
#endif


    if(op->stats.food<0) 
    {

#ifdef EXPLORE_MODE
	    if (op->contr->explore)
        {
	        new_draw_info(NDI_UNIQUE, 0,op,"You would have starved, but you are");
	        new_draw_info(NDI_UNIQUE, 0,op,"in explore mode, so...");
	        op->stats.food=999;
	        return;
	    }
#endif /* EXPLORE_MODE */

	    sprintf(buf,"%s starved to death.",op->name);
	    strcpy(op->contr->killer,"starvation");
    }
    else 
    {
#ifdef EXPLORE_MODE
	    if (op->contr->explore) 
        {
	        new_draw_info(NDI_UNIQUE, 0,op,"You would have died, but you are");
	        new_draw_info(NDI_UNIQUE, 0,op,"in explore mode, so...");
	        op->stats.hp=op->stats.maxhp_adj;
	        return;
	    }
#endif /* EXPLORE_MODE */

	    sprintf(buf,"%s died.",op->name);
    }

    play_sound_player_only(op->contr, SOUND_PLAYER_DIES,SOUND_NORMAL,0,0);

    /*  save the map location for corpse, gravestone*/
    x=op->x;y=op->y;map=op->map;


#ifdef NOT_PERMADEATH
    /* NOT_PERMADEATH code.  This basically brings the character back to life
     * if they are dead - it takes some exp and a random stat.  See the config.h
     * file for a little more in depth detail about this.
     */

    /* Basically two ways to go - remove a stat permanently, or just
     * make it depletion.  This bunch of code deals with that aspect	
     * of death.
     */

    if (settings.balanced_stat_loss) 
    {
        /* If stat loss is permanent, lose one stat only. */
        /* Lower level chars don't lose as many stats because they suffer more
           if they do. */
        /* Higher level characters can afford things such as potions of
           restoration, or better, stat potions. So we slug them that little
           bit harder. */
        /* GD */
        if (settings.stat_loss_on_death)
            num_stats_lose = 1;
        else
            num_stats_lose = 1 + op->level/BALSL_NUMBER_LOSSES_RATIO;
    }
    else 
    {
        num_stats_lose = 1;
    }
    lost_a_stat = 0;

	/* the rule is: 
	 * only decrease stats when you are level 3 or higher! 
	 * Lose permanent stats when you are higher as level 74 */
    for (z=0; z<num_stats_lose; z++) 
    {
        if ((settings.stat_loss_on_death && op->level > 3) || op->level >= 75) 
        {
	        /* Pick a random stat and take a point off it.  Tell the player
	        * what he lost.
	        */
            i = RANDOM() % 7; 
            change_attr_value(&(op->stats), i,-1);
            check_stat_bounds(&(op->stats));
            change_attr_value(&(op->contr->orig_stats), i,-1);
            check_stat_bounds(&(op->contr->orig_stats));
            new_draw_info(NDI_UNIQUE, 0,op, lose_msg[i]);
            lost_a_stat = 1;
        } 
        else if (op->level > 3)
        {
            /* deplete a stat */
            archetype *deparch=find_archetype("depletion");
            object *dep;
            
            i = RANDOM() % 7;
            dep = present_arch_in_ob(deparch,op);
            if(!dep) {
	        dep = arch_to_object(deparch);
	        insert_ob_in_ob(dep, op);
            }
            lose_this_stat = 1;
            if (settings.balanced_stat_loss) {
                /* GD */
                /* Get the stat that we're about to deplete. */
                this_stat = get_attr_value(&(dep->stats), i);
                if (this_stat < 0) {
                    int loss_chance = 1 + op->level/BALSL_LOSS_CHANCE_RATIO;
                    int keep_chance = this_stat * this_stat;
                    /* Yes, I am paranoid. Sue me. */
                    if (keep_chance < 1)
                        keep_chance = 1;

                    /* There is a maximum depletion total per level. */
                    if (this_stat < -1 - op->level/BALSL_MAX_LOSS_RATIO) {
                        lose_this_stat = 0;
                    /* Take loss chance vs keep chance to see if we retain the stat. */
                    } else {
                        if (random_roll(0, loss_chance + keep_chance-1, op, PREFER_LOW) < keep_chance)
                            lose_this_stat = 0;
                        /* LOG(llevDebug, "Determining stat loss. Stat: %d Keep: %d Lose: %d Result: %s.\n",
                             this_stat, keep_chance, loss_chance,
                             lose_this_stat?"LOSE":"KEEP"); */
                    }
                }
            }
            
            if (lose_this_stat) {
                this_stat = get_attr_value(&(dep->stats), i);
		/* We could try to do something clever like find another
		 * stat to reduce if this fails.  But chances are, if
		 * stats have been depleted to -50, all are pretty low
		 * and should be roughly the same, so it shouldn't make a
		 * difference.
		 */
		if (this_stat>=-50) {
		    change_attr_value(&(dep->stats), i, -1);
		    SET_FLAG(dep, FLAG_APPLIED);
		    new_draw_info(NDI_UNIQUE, 0,op, lose_msg[i]);
		    fix_player(op);
		    lost_a_stat = 1;
		}
            }
        }
    }
    /* If no stat lost, tell the player. */
    if (!lost_a_stat)
    {
        /* determine_god() seems to not work sometimes... why is this?
           Should I be using something else? GD */
        char *god = determine_god(op);
        if (god && (strcmp(god, "none")))
            new_draw_info_format(NDI_UNIQUE, 0, op, "For a brief moment you feel the holy presence of\n%s protecting you.", god);
        else
            new_draw_info(NDI_UNIQUE, 0, op, "For a brief moment you feel a holy presence\nprotecting you.");
    }

    /* Put a gravestone up where the character 'almost' died.  List the
     * exp loss on the stone.
     */
    tmp=arch_to_object(find_archetype("gravestone"));
    sprintf(buf,"%s's gravestone",op->name);
    FREE_AND_COPY_HASH(tmp->name, buf);
    sprintf(buf,"RIP\nHere rests the hero %s the %s,\n"
	        "who was killed\n"
	        "by %s.\n",
	        op->name, op->contr->title,
	        op->contr->killer);
    FREE_AND_COPY_HASH(tmp->msg, buf);
    tmp->x=op->x,tmp->y=op->y;
    insert_ob_in_map (tmp, op->map, NULL,0);

 /**************************************/
 /*                                    */
 /* Subtract the experience points,    */
 /* if we died cause of food, give us  */
 /* food, and reset HP's...            */
 /*                                    */
 /**************************************/

    /* remove any poisoning and confusion the character may be suffering. */
    cast_heal(op, 110, op, SP_CURE_POISON);
    /*cast_heal(op, op, SP_CURE_CONFUSION);*/
    cure_disease(op,NULL);  /* remove any disease */
	
    apply_death_exp_penalty(op);
    if(op->stats.food < 0) op->stats.food = 900;
    op->stats.hp = op->stats.maxhp;
    op->stats.sp = op->stats.maxsp;
    op->stats.grace = op->stats.maxgrace;

 /*
  * Check to see if the player is in a shop.  IF so, then check to see if
  * the player has any unpaid items.  If so, remove them and put them back
  * in the map.
  */

    tmp= get_map_ob(op->map, op->x, op->y);
    if (tmp && tmp->type == SHOP_FLOOR) {
	remove_unpaid_objects(op->inv, op);
    }
 

 /****************************************/
 /*                                      */
 /* Move player to his current respawn-  */
 /* position (usually last savebed)      */
 /*                                      */
 /****************************************/

    enter_player_savebed(op);

 /**************************************/
 /*                                    */
 /* Repaint the characters inv, and    */
 /* stats, and show a nasty message ;) */
 /*                                    */
 /**************************************/

    new_draw_info(NDI_UNIQUE, 0,op,"YOU HAVE DIED.");
    save_player(op,1);
    return;
#endif

/* If NOT_PERMADETH is set, then the rest of this is not reachable.  This
 * should probably be embedded in an else statement.
 */

    op->contr->party_number=(-1);
    new_draw_info(NDI_UNIQUE|NDI_ALL, 0,NULL, buf);
    check_score(op);
    if(op->contr->golem!=NULL) {
		send_golem_control(op->contr->golem, GOLEM_CTR_RELEASE);
      remove_friendly_object(op->contr->golem);
      remove_ob(op->contr->golem);
      free_object(op->contr->golem);
      op->contr->golem=NULL;
    }
    loot_object(op); /* Remove some of the items for good */
    remove_ob(op);
    op->direction=0;
    if(!QUERY_FLAG(op,FLAG_WAS_WIZ)&&op->stats.exp) {
      delete_character(op->name,0);
#ifndef NOT_PERMADEATH
#ifdef RESURRECTION
	/* save playerfile sans equipment when player dies
	** then save it as player.pl.dead so that future resurrection
	** type spells will work on them nicely
	*/
	op->stats.hp = op->stats.maxhp_adj;
	op->stats.food = 999;

	/*  set the location of where the person will reappear when  */
	/* maybe resurrection code should fix map also */
	strcpy(op->contr->maplevel, EMERGENCY_MAPPATH);
	if(op->map!=NULL)
	    op->map = NULL;
	op->x = EMERGENCY_X;
	op->y = EMERGENCY_Y;
	if(op->container)
		esrv_apply_container (op, op->container);
	save_player(op,0);
	op->map = map;
	/* please see resurrection.c: peterm */
	dead_player(op);
#endif
#endif
    }
    play_again(op);
#ifdef NOT_PERMADEATH
    tmp=arch_to_object(find_archetype("gravestone"));
    sprintf(buf,"%s's gravestone",op->name);
    FREE_AND_COPY_HASH(tmp->name,buf);
    sprintf(buf,"RIP\nHere rests the hero %s the %s,\nwho was killed by %s.\n",
	    op->name, op->contr->title, op->contr->killer);
    FREE_AND_COPY_HASH(tmp->msg, buf);
    tmp->x=x,tmp->y=y;
    insert_ob_in_map (tmp, map, NULL,0);
#else
    /*  peterm:  added to create a corpse at deathsite.  */
	/*
    tmp=arch_to_object(find_archetype("corpse_pl"));
    sprintf(buf,"%s", op->name);
    FREE_AND_COPY_HASH(tmp->name,buf);
    tmp->level=op->level;
    tmp->x=x;tmp->y=y;
    FREE_AND_COPY_HASH(tmp->msg, gravestone_text(op));
    SET_FLAG (tmp, FLAG_UNIQUE);
    insert_ob_in_map (tmp, map, NULL,0);
	*/
#endif
}


void loot_object(object *op) { /* Grab and destroy some treasure */
  object *tmp,*tmp2,*next;

  if (op->container) { /* close open sack first */
      esrv_apply_container (op, op->container);
  }

  for(tmp=op->inv;tmp!=NULL;tmp=next) {
    next=tmp->below;
    if (tmp->type==EXPERIENCE || IS_SYS_INVISIBLE(tmp)) continue;
    remove_ob(tmp);
    tmp->x=op->x,tmp->y=op->y;
    if (tmp->type == CONTAINER) { /* empty container to ground */
	loot_object(tmp);
    }
    if(!QUERY_FLAG(tmp, FLAG_UNIQUE) && (QUERY_FLAG(tmp, FLAG_STARTEQUIP) 
       || QUERY_FLAG(tmp,FLAG_NO_DROP) || !(RANDOM()%3))) {
      if(tmp->nrof>1) {
	tmp2=get_split_ob(tmp,1+RANDOM()%(tmp->nrof-1));
	free_object(tmp2);
	insert_ob_in_map(tmp,op->map,NULL,0);
      } else
	free_object(tmp);
    } else
      insert_ob_in_map(tmp,op->map,NULL,0);
  }
}

/*
 * fix_weight(): Check recursively the weight of all players, and fix
 * what needs to be fixed.  Refresh windows and fix speed if anything
 * was changed.
 */

void fix_weight() {
  player *pl;
  for (pl = first_player; pl != NULL; pl = pl->next) {
    int old = pl->ob->carrying, sum = sum_weight(pl->ob);
    if(old == sum)
      continue;
    fix_player(pl->ob);
    esrv_update_item(UPD_WEIGHT, pl->ob, pl->ob);
    /*LOG(llevDebug,"Fixed inventory in %s (%d -> %d)\n",query_name(pl->ob), old, sum);*/
  }
}

void fix_luck() {
  player *pl;
  for (pl = first_player; pl != NULL; pl = pl->next)
    if (!pl->ob->contr->state)
      change_luck(pl->ob, 0);
}


/* cast_dust() - handles op throwing objects of type 'DUST' */
/* WARNING: FUNCTION NEED TO BE REWRITTEN. works for ae spells only now! */ 
void cast_dust (object *op, object *throw_ob, int dir) {
  archetype *arch;

  if(!(spells[throw_ob->stats.sp].flags&SPELL_DESC_DIRECTION))
  {
	  LOG(llevBug,"DEBUG: Warning, dust %s is not a ae spell!!\n", query_name(throw_ob));
	  return;
  }

  if(spells[throw_ob->stats.sp].archname)
	  arch = find_archetype(spells[throw_ob->stats.sp].archname);
 
  /* casting POTION 'dusts' is really a use_magic_item skill */
  if(op->type==PLAYER&&throw_ob->type==POTION &&!change_skill(op,SK_USE_MAGIC_ITEM))
      return; /* no skill, no dust throwing */
 
 
  if(throw_ob->type==POTION&&arch!= NULL)
    cast_cone(op,throw_ob,dir,10,throw_ob->stats.sp,arch,1);
  else if((arch=find_archetype("dust_effect"))!=NULL) { /* dust_effect */
    cast_cone(op,throw_ob,dir,1,0,arch,0);
  } else /* problem occured! */ 
    LOG(llevBug,"BUG: cast_dust() can't find an archetype to use!\n");
 
  if (op->type==PLAYER&&arch)
    new_draw_info_format(NDI_UNIQUE, 0,op,"You cast %s.",query_name(throw_ob));
  if(!QUERY_FLAG(throw_ob,FLAG_REMOVED)) remove_ob(throw_ob);
  free_object(throw_ob);
}

void make_visible (object *op) {
    op->hide = 0;
    /*CLEAR_FLAG(op,FLAG_IS_INVISIBLE); hu, we don't handle this in this way anymore */
    if(op->type==PLAYER) 
      op->contr->tmp_invis = 0;
    if(QUERY_FLAG(op, FLAG_UNDEAD)&&!is_true_undead(op)) 
      CLEAR_FLAG(op, FLAG_UNDEAD);
    update_object(op,UP_OBJ_FACE);
}

int is_true_undead(object *op) {
  object *tmp=NULL;
  
  if(QUERY_FLAG(&op->arch->clone,FLAG_UNDEAD)) return 1;

  if(op->type==PLAYER)
    for(tmp=op->inv;tmp;tmp=tmp->below)
       if(tmp->type==EXPERIENCE && tmp->stats.Wis)
	  if(QUERY_FLAG(tmp,FLAG_UNDEAD)) return 1;
  return 0;
}

/* look at the surrounding terrain to determine
 * the hideability of this object. Positive levels
 * indicate greater hideability.
 */

int hideability(object *ob) {
  int i,x,y,level=0;

  if(!ob||!ob->map) return 0;

  /* so, on normal lighted maps, its hard to hide */
  level=ob->map->darkness - 2;

  /* this also picks up whether the object is glowing.
   * If you carry a light on a non-dark map, its not
   * as bad as carrying a light on a pitch dark map */
  if(has_carried_lights(ob)) level =- (10 + (2*ob->map->darkness));

  /* scan through all nearby squares for terrain to hide in */
  for(i=0,x=ob->x,y=ob->y;i<9;i++,x=ob->x+freearr_x[i],y=ob->y+freearr_y[i]) 
  { 
    if(blocks_view(ob->map,x,y)) /* something to hide near! */
      level += 2;
    else /* open terrain! */
      level -= 1;
  }
  
#if 0
  LOG(llevDebug,"hideability of %s is %d\n",ob->name,level);
#endif
  return level;
}

/* For Hidden creatures - a chance of becoming 'unhidden'
 * every time they move - as we subtract off 'invisibility'
 * AND, for players, if they move into a ridiculously unhideable
 * spot (surrounded by clear terrain in broad daylight). -b.t.
 */

void do_hidden_move (object *op) {
    int hide=0, num=random_roll(0, 19, op, PREFER_LOW);
    
    if(!op || !op->map) return;

    /* its *extremely* hard to run and sneak/hide at the same time! */
    if(op->type==PLAYER && op->contr->run_on) {
      if(num >= SK_level(op)) {
        new_draw_info(NDI_UNIQUE,0,op,"You ran too much! You are no longer hidden!");
        make_visible(op);
        return;
      } else num += 20;
    }
    num += op->map->difficulty;
    hide=hideability(op); /* modify by terrain hidden level */
    num -= hide;

    if(op->type==PLAYER && hide<-10) {
      make_visible(op);
      if(op->type==PLAYER) new_draw_info(NDI_UNIQUE, 0,op,
          "You moved out of hiding! You are visible!");
    }
}

/* determine if who is standing near a hostile creature. */

int stand_near_hostile( object *who ) 
{
	object *tmp=NULL;
	mapstruct *m;
	int i,xt,yt, friendly=0, player=0;

	if(!who) 
		return 0;

	if(who->type==PLAYER) 
		player=1; 
	else 
		friendly = QUERY_FLAG(who,FLAG_FRIENDLY);

	/* search adjacent squares */
	for(i=1;i<9;i++) 
	{
		xt = who->x+freearr_x[i];
		yt = who->y+freearr_y[i];
		if (!(m=out_of_map(who->map,&xt,&yt)))
			continue;
		for(tmp=get_map_ob(m,xt,yt);tmp;tmp=tmp->above)
		{
			if((player||friendly)&&QUERY_FLAG(tmp,FLAG_MONSTER)&&!QUERY_FLAG(tmp,FLAG_UNAGGRESSIVE)) 
				return 1;
			else if(tmp->type==PLAYER) 
				return 1;
		}
	}
	return 0;
}

/* check the player los field for viewability of the 
 * object op. This function works fine for monsters,
 * but we dont worry if the object isnt the top one in 
 * a pile (say a coin under a table would return "viewable"
 * by this routine). Another question, should we be
 * concerned with the direction the player is looking 
 * in? Realistically, most of use cant see stuff behind
 * our backs...on the other hand, does the "facing" direction
 * imply the way your head, or body is facing? Its possible
 * for them to differ. Sigh, this fctn could get a bit more complex.
 * -b.t. 
 * This function is now map tiling safe.
 */

int player_can_view (object *pl,object *op) {
    rv_vector rv;
    int dx,dy;

    if(pl->type!=PLAYER) {
	LOG(llevBug,"BUG: player_can_view() called for non-player object\n");
	return -1;
    }
    if (!pl || !op) return 0;

    if(op->head) { op = op->head; }
    get_rangevector(pl, op, &rv, 0x1);

    /* starting with the 'head' part, lets loop
     * through the object and find if it has any
     * part that is in the los array but isnt on 
     * a blocked los square.
     * we use the archetype to figure out offsets.
     */
    while(op) {
	dx = rv.distance_x + op->arch->clone.x;
	dy = rv.distance_y + op->arch->clone.y;

	/* only the viewable area the player sees is updated by LOS
	 * code, so we need to restrict ourselves to that range of values
	 * for any meaningful values.
	 */
	if (FABS(dx) <= (pl->contr->socket.mapx/2) &&
	    FABS(dy) <= (pl->contr->socket.mapy/2) &&
	    !pl->contr->blocked_los[dx + (pl->contr->socket.mapx/2)][dy+(pl->contr->socket.mapy/2)] ) 
	    return 1;
	op = op->more;
    }
    return 0;
}

/* routine for both players and monsters. We call this when
 * there is a possibility for our action distrubing our hiding
 * place or invisiblity spell. Artefact invisiblity is not
 * effected by this. If we arent invisible to begin with, we 
 * return 0. 
 */
int action_makes_visible (object *op) {

  if(QUERY_FLAG(op,FLAG_IS_INVISIBLE) && QUERY_FLAG(op,FLAG_ALIVE)) {
    if(!QUERY_FLAG(op,FLAG_SEE_INVISIBLE)) 
      return 0; 
    else if(op->hide || (op->contr&&op->contr->tmp_invis)) { 
      new_draw_info_format(NDI_UNIQUE, 0,op,"You become %!",op->hide?"unhidden":"visible");
      return 1; 
    } else if(op->contr && !op->contr->shoottype==range_magic) { 
	  /* improved invis is lost EXCEPT for case of casting of magic */
          new_draw_info(NDI_UNIQUE, 0,op,"Your invisibility spell is broken!");
          return 1;
    }
  }

  return 0;
}

/* test for pvp area. 
 * if only one opject is given, it test for it.
 * if 2 objects given, both player must be in pvp or
 * the function fails.
 * this function use map and x/y from the player object -
 * be sure player are valid and on map.
 * RETURN: FALSE = no pvp, TRUE= pvp possible
 */
int pvp_area(object *attacker, object* victim)
{

	if(attacker)
	{
		if(!(attacker->map->map_flags&MAP_FLAG_PVP) || !(GET_MAP_FLAGS(attacker->map,attacker->x,attacker->y)&P_IS_PVP))
			return FALSE;
	}

	if(victim)
	{
		if(!(victim->map->map_flags&MAP_FLAG_PVP) || !(GET_MAP_FLAGS(victim->map,victim->x,victim->y)&P_IS_PVP))
			return FALSE;
	}

	return TRUE;
}

/* op_on_battleground - checks if the given object op (usually
 * a player) is standing on a valid battleground-tile,
 * function returns TRUE/FALSE. If true x, y returns the battleground
 * -exit-coord. (and if x, y not NULL)
 */
/* TODO: sigh, this must be changed! we don't want loop tile objects in move/attack
 * or other action without any need.
*/
int op_on_battleground (object *op, int *x, int *y) {
  object *tmp;
  
  /* A battleground-tile needs the following attributes to be valid:
   * is_floor 1 (has to be the FIRST floor beneath the player's feet),
   * name="battleground", no_pick 1, type=58 (type BATTLEGROUND)
   * and the exit-coordinates sp/hp must both be > 0.
   * => The intention here is to prevent abuse of the battleground-
   * feature (like pickable or hidden battleground tiles). */
  for (tmp=op->below; tmp!=NULL; tmp=tmp->below) {
    if (QUERY_FLAG (tmp, FLAG_IS_FLOOR)) {
      if (QUERY_FLAG (tmp, FLAG_NO_PICK) &&
	  strcmp(tmp->name, "battleground")==0 &&
	  tmp->type == BATTLEGROUND && EXIT_X(tmp) && EXIT_Y(tmp)) {
	    if (x != NULL && y != NULL)
		*x=EXIT_X(tmp), *y=EXIT_Y(tmp);
	    return 1;
      }
    }
  }
  /* If we got here, did not find a battleground */
  return 0;
}

/*
 * When a dragon-player gains a new stage of evolution,
 * he gets some treasure
 *
 * attributes:
 *      object *who        the dragon player
 *      int atnr           the attack-number of the ability focus
 *      int level          ability level
 */
void dragon_ability_gain(object *who, int atnr, int level) {
  treasurelist *trlist = NULL;   /* treasurelist */
  treasure *tr;                  /* treasure */
  object *tmp;                   /* tmp. object */
  object *item;                  /* treasure object */
  char buf[MAX_BUF];             /* tmp. string buffer */
  int i=0, j=0;
  
  /* get the appropriate treasurelist */
  if (atnr == ATNR_FIRE)
    trlist = find_treasurelist("dragon_ability_fire");
  else if (atnr == ATNR_COLD)
    trlist = find_treasurelist("dragon_ability_cold");
  else if (atnr == ATNR_ELECTRICITY)
    trlist = find_treasurelist("dragon_ability_elec");
  else if (atnr == ATNR_POISON)
    trlist = find_treasurelist("dragon_ability_poison");
  
  if (trlist == NULL || who->type != PLAYER)
    return;
  
  for (i=0, tr = trlist->items; tr != NULL && i<level-1;
       tr = tr->next, i++);
  
  if (tr == NULL || tr->item == NULL) {
    /* printf("-> no more treasure for %s\n", change_resist_msg[atnr]); */
    return;
  }
  
  /* everything seems okay - now bring on the gift: */
  item = &(tr->item->clone);
  
  /* grant direct spell */
  if (item->type == SPELLBOOK) {
    int spell = look_up_spell_name (item->slaying);
    if (spell<0 || check_spell_known (who, spell))
      return;
    if (IS_SYS_INVISIBLE(item)) {
      sprintf(buf, "You gained the ability of %s", spells[spell].name);
      new_draw_info(NDI_UNIQUE|NDI_BLUE, 0, who, buf);
      do_learn_spell (who, spell, 0);
      return;
    }
  }
  else if (item->type == SKILL) {
    if (strcmp(item->title, "clawing") == 0 &&
	change_skill (who, SK_CLAWING)) {
      /* adding new attacktypes to the clawing skill */
      tmp = who->chosen_skill; /* clawing skill object */
      
      if (tmp->type == SKILL && strcmp(tmp->name, "clawing")==0
	  && !(tmp->attacktype & item->attacktype)) {
	/* always add physical if there's none */
	if (tmp->attacktype == 0) tmp->attacktype = AT_PHYSICAL;
	
	/* we add the new attacktype to the clawing ability */
	tmp->attacktype += item->attacktype;
	
	if (item->msg != NULL)
	  new_draw_info(NDI_UNIQUE|NDI_BLUE, 0, who, item->msg);
      }
    }
  }
  else if (item->type == FORCE) {
    /* forces in the treasurelist can alter the player's stats */
    object *skin;
    /* first get the dragon skin force */
    for (skin=who->inv; skin!=NULL && strcmp(skin->arch->name, "dragon_skin_force")!=0;
	 skin=skin->below);
    if (skin == NULL) return;
    
    /* adding new spellpath attunements */
    if (item->path_attuned > 0 && !(skin->path_attuned & item->path_attuned)) {
      skin->path_attuned += item->path_attuned; /* add attunement to skin */
      
      /* print message */
      sprintf(buf, "You feel attuned to ");
      for(i=0, j=0; i<NRSPELLPATHS; i++) {
        if(item->path_attuned & (1<<i)) {
	  if (j) 
            strcat(buf," and ");
          else 
            j = 1; 
          strcat(buf, spellpathnames[i]);
        }
      }
      strcat(buf,".");
      new_draw_info(NDI_UNIQUE|NDI_BLUE, 0, who, buf);
    }
    
    /* evtl. adding flags: */
    if(QUERY_FLAG(item, FLAG_XRAYS))
      SET_FLAG(skin, FLAG_XRAYS);
    if(QUERY_FLAG(item, FLAG_STEALTH))
      SET_FLAG(skin, FLAG_STEALTH);
    if(QUERY_FLAG(item, FLAG_SEE_IN_DARK))
      SET_FLAG(skin, FLAG_SEE_IN_DARK);
    
    /* print message if there is one */
    if (item->msg != NULL)
      new_draw_info(NDI_UNIQUE|NDI_BLUE, 0, who, item->msg);
  }
  else {
    /* generate misc. treasure */
    tmp = arch_to_object (tr->item);
    sprintf(buf, "You gained %s", query_short_name(tmp));
    new_draw_info(NDI_UNIQUE|NDI_BLUE, 0, who, buf);
    tmp = insert_ob_in_ob (tmp, who);
    if (who->type == PLAYER)
      esrv_send_item(who, tmp);
  }
}


/*  extended find arrow version, using tag and containers.
 *  Find an arrow in the inventory and after that
 *  in the right type container (quiver). Pointer to the 
 *  found object is returned.
 */
static object *find_arrow_ext(object *op, char *type,int tag)
{
  object *tmp = NULL;

  if(tag == -2)
  {
      for(op=op->inv; op; op=op->below)
          if(!tmp && op->type==CONTAINER && op->race==type &&
              QUERY_FLAG(op,FLAG_APPLIED))
              tmp = find_arrow_ext (op, type,-2);
          else if (op->type==ARROW && op->race==type)
              return op;
          return tmp;
  }
  else
  {
      if(tag == -1)
          return tmp;
      for(op=op->inv; op; op=op->below)
      {

          if(op->count == (tag_t) tag)
          {
              /* the simple task: we have a arrow marked */
              if(op->race == type && op->type==ARROW)
                  return op;
              /* we have container marked as missile source. Skip search when there is 
              nothing in. Use the standard search now */
              /* because we don't want container in container, we don't care abvout applied */
              if(op->race == type && op->type==CONTAINER)
              {
                  tmp = find_arrow_ext (op, type,-2);
                  return tmp;
              }
          }
      }
      return tmp;
  }
}
