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
#include <funcpoint.h>
#include <math.h>


/* Distance must be less than this for the object to be blocked.
 * An object is 1.0 wide, so if set to 0.5, it means the object
 * that blocks half the view (0.0 is complete block) will
 * block view in our tables.
 * .4 or less lets you see through walls.  .5 is about right.
 */

#define SPACE_BLOCK	0.5

typedef struct blstr {
  int x[4],y[4];
  int index;
} blocks;

static blocks block[MAP_CLIENT_X][MAP_CLIENT_Y];

void inline clear_los(object *op);

/*
 * initialises the array used by the LOS routines.
 * This is NOT called for every LOS - only at server start to
 * init the base block struct.
 */

/* since we are only doing the upper left quadrant, only
 * these spaces could possibly get blocked, since these
 * are the only ones further out that are still possibly in the
 * sightline.
 */

void init_block() {
    int x,y, dx, dy, i;
    static int block_x[3] = {-1, -1, 0}, block_y[3] = {-1, 0, -1};

    for(x=0;x<MAP_CLIENT_X;x++)
	for(y=0;y<MAP_CLIENT_Y;y++) {
	    block[x][y].index=0;
	}


    /* The table should be symmetric, so only do the upper left
     * quadrant - makes the processing easier.
     */
    for (x=1; x<=MAP_CLIENT_X/2; x++) {
	for (y=1; y<=MAP_CLIENT_Y/2; y++) {
	    for (i=0; i< 3; i++) {
		dx = x + block_x[i];
		dy = y + block_y[i];

		/* center space never blocks */
		if (x == MAP_CLIENT_X/2 && y == MAP_CLIENT_Y/2) continue;

		/* If its a straight line, its blocked */
		if ((dx == x && x == MAP_CLIENT_X/2) || 
		    (dy==y && y == MAP_CLIENT_Y/2)) {
			/* For simplicity, we mirror the coordinates to block the other
			 * quadrants.
			 */
			set_block(x, y, dx, dy);
			if (x == MAP_CLIENT_X/2) {
			    set_block(x, MAP_CLIENT_Y - y -1, dx, MAP_CLIENT_Y - dy-1);
			} else if (y == MAP_CLIENT_Y/2) {
			    set_block(MAP_CLIENT_X - x -1, y, MAP_CLIENT_X - dx - 1, dy);
			}
		} else {
		    float d1, r, s,l;

		    /* We use the algorihm that found out how close the point
		     * (x,y) is to the line from dx,dy to the center of the viewable
		     * area.  l is the distance from x,y to the line.
		     * r is more a curiosity - it lets us know what direction (left/right)
		     * the line is off
		     */

		    d1 = (float) (pow(MAP_CLIENT_X/2 - dx, 2) + pow(MAP_CLIENT_Y/2 - dy,2));
		    r = (float)((dy-y)*(dy - MAP_CLIENT_Y/2) - (dx-x)*(MAP_CLIENT_X/2-dx))/d1;
		    s = (float)((dy-y)*(MAP_CLIENT_X/2 - dx ) - (dx-x)*(MAP_CLIENT_Y/2-dy))/d1;
		    l = (float) FABS(sqrt(d1) * s);

		    if (l <= SPACE_BLOCK) {
			/* For simplicity, we mirror the coordinates to block the other
			 * quadrants.
			 */
			set_block(x,y,dx,dy);
			set_block(MAP_CLIENT_X - x -1, y, MAP_CLIENT_X - dx - 1, dy);
			set_block(x, MAP_CLIENT_Y - y -1, dx, MAP_CLIENT_Y - dy - 1);
			set_block(MAP_CLIENT_X -x-1, MAP_CLIENT_Y -y-1, MAP_CLIENT_X - dx-1, MAP_CLIENT_Y - dy-1);
		    }
		}
	    }
	}
    }
}

/*
 * Used to initialise the array used by the LOS routines.
 * What this sets if that x,y blocks the view of bx,by
 * This then sets up a relation - for example, something
 * at 5,4 blocks view at 5,3 which blocks view at 5,2
 * etc.  So when we check 5,4 and find it block, we have
 * the data to know that 5,3 and 5,2 and 5,1 should also
 * be blocked.
 */
void set_block(int x,int y,int bx, int by) {
    int index=block[x][y].index,i;

    /* Due to flipping, we may get duplicates - better safe than sorry.
     */
    for (i=0; i<index; i++) {
	if (block[x][y].x[i] == bx && block[x][y].y[i] == by) return;
    }

    block[x][y].x[index]=bx;
    block[x][y].y[index]=by;
    block[x][y].index++;
#ifdef LOS_DEBUG
    LOG(llevInfo ,"setblock: added %d %d -> %d %d (%d)\n", x, y, bx, by, block[x][y].index);
#endif
}




/*
 * Used to initialise the array used by the LOS routines.
 * x,y are indexes into the blocked[][] array.
 * This recursively sets the blocked line of sight view.
 * From the blocked[][] array, we know for example
 * that if some particular space is blocked, it blocks
 * the view of the spaces 'behind' it, and those blocked
 * spaces behind it may block other spaces, etc.  
 * In this way, the chain of visibility is set.
 */

static void set_wall(object *op,int x,int y) {
    int i, xt, yt;

	xt = (MAP_CLIENT_X - op->contr->socket.mapx)/2;
	yt = (MAP_CLIENT_Y - op->contr->socket.mapy)/2;

    for(i=0;i<block[x][y].index;i++) {
	int dx=block[x][y].x[i],dy=block[x][y].y[i],ax,ay;

	/* ax, ay are the values as adjusted to be in the
	 * socket look structure.
	 */
	ax = dx - xt;
	ay = dy - yt;

	if (ax < 0 || ax>=op->contr->socket.mapx ||
	    ay < 0 || ay>=op->contr->socket.mapy) continue;
#if 0
	LOG(llevInfo ,"blocked %d %d -> %d %d\n",dx, dy, ax, ay);
#endif
	/* we need to adjust to the fact that the socket
	 * code wants the los to start from the 0,0
	 * and not be relative to middle of los array.
	 */
	if(!(op->contr->blocked_los[ax][ay]&BLOCKED_LOS_OUT_OF_MAP))
		op->contr->blocked_los[ax][ay]|=BLOCKED_LOS_BLOCKED; /* this tile can't be seen */
	set_wall(op,dx,dy);

    }
}

/*
 * Used to initialise the array used by the LOS routines.
 * op is the object, x and y values based on MAP_CLIENT_X and Y.
 * this is because they index the blocked[][] arrays.
 */
/* instead of useless darkness values, blocked_los[][] now tells the client
 * update function what kind of tile we have: visible, sight blocked, blocksview trigger 
 * or out of map.
 */
static void check_wall(object *op,int x,int y) {
    int ax, ay, flags;

    /* ax, ay are coordinates as indexed into the look window */
    ax = x - (MAP_CLIENT_X - op->contr->socket.mapx)/2;
    ay = y - (MAP_CLIENT_Y - op->contr->socket.mapy)/2;

	/* this skips the "edges" of view area, the border tiles.
	 * Naturally, this tiles can't block any view - there is
	 * nothing behind them.
	 */
    if(!block[x][y].index)
	{
		/* to handle the "blocksview update" right, we give this special
		 * tiles a "never use it to trigger a los_update()" flag.
		 * blockview changes to this tiles will have no effect.
		 */
		if(blocks_view(op->map,op->x + x - MAP_CLIENT_X/2, op->y + y - MAP_CLIENT_Y/2)&P_OUT_OF_MAP)
			op->contr->blocked_los[ax][ay]=BLOCKED_LOS_OUT_OF_MAP; /* mark the space as out_of_map */
		else
			op->contr->blocked_los[ax][ay]|=BLOCKED_LOS_IGNORE; /* ignore means ignore for LOS */
		return;
	}


    /* If the converted coordinates are outside the viewable
     * area for the client, return now.
     */
    if (ax < 0 || ay < 0 || ax >= op->contr->socket.mapx || ay >= op->contr->socket.mapy)
		return;

	/*LOG(-1,"SET_LOS: %d,%d\n", ax,ay);*/
    /* If this space is already blocked, prune the processing - presumably
     * whatever has set this space to be blocked has done the work and already
     * done the dependency chain.
	 * but check for out_of_map to speedup our client map draw function.
     */
    if (op->contr->blocked_los[ax][ay]&(BLOCKED_LOS_BLOCKED|BLOCKED_LOS_OUT_OF_MAP))
	{
		if(op->contr->blocked_los[ax][ay]&BLOCKED_LOS_BLOCKED)
		{
			if((flags = blocks_view(op->map,op->x + x - MAP_CLIENT_X/2, op->y + y - MAP_CLIENT_Y/2)))
			{
				if(flags&P_OUT_OF_MAP)
					op->contr->blocked_los[ax][ay]=BLOCKED_LOS_OUT_OF_MAP; /* mark the space as out_of_map */
				else
					op->contr->blocked_los[ax][ay]|=BLOCKED_LOS_BLOCKSVIEW;
			}
		}
		return;
	}

#if 0
    LOG(llevInfo ,"check_wall, ax,ay=%d, %d  x,y = %d, %d  blocksview = %d, %d\n",
	    ax, ay, x, y, op->x + x - MAP_CLIENT_X/2, op->y + y - MAP_CLIENT_Y/2);
#endif

    if((flags = blocks_view(op->map,op->x + x - MAP_CLIENT_X/2, op->y + y - MAP_CLIENT_Y/2)))
	{
		set_wall(op,x,y);
		
		/* out of map clears all other flags! */
		if(flags&P_OUT_OF_MAP)
			op->contr->blocked_los[ax][ay]= BLOCKED_LOS_OUT_OF_MAP; /* mark the space as out_of_map */
		else
			op->contr->blocked_los[ax][ay] |= BLOCKED_LOS_BLOCKSVIEW; 

	}

}

/*
 * update_los() recalculates the array which specifies what is
 * visible for the given player-object.
 */
void update_los(object *op) 
{
    int dx = op->contr->socket.mapx_2, dy = op->contr->socket.mapy_2, x, y;
  
    if(QUERY_FLAG(op,FLAG_REMOVED))
	return;

#ifdef DEBUG_CORE
	LOG(llevDebug,"LOS - %s\n", query_name(op));
#endif

    clear_los(op);

    if(QUERY_FLAG(op,FLAG_WIZ))
    	return;

    /* For larger maps, this is more efficient than the old way which
     * used the chaining of the block array.  Since many space views could
     * be blocked by different spaces in front, this mean that a lot of spaces
     * could be examined multile times, as each path would be looked at.
     */
    for (x=(MAP_CLIENT_X - op->contr->socket.mapx)/2; x<(MAP_CLIENT_X + op->contr->socket.mapx)/2; x++) 
		for (y=(MAP_CLIENT_Y - op->contr->socket.mapy)/2; y<(MAP_CLIENT_Y + op->contr->socket.mapy)/2; y++) 
			check_wall(op, x, y);

    /* do the los of the player. 3 (potential) cases */
    if(QUERY_FLAG(op,FLAG_BLIND)) /* player is blind */ 
		blinded_sight(op);
  	else
		expand_sight(op);


	/* give us a area we can look through when we have xray - this
	 * stacks to normal LOS.
	 */
    if (QUERY_FLAG(op,FLAG_XRAYS)) 
	{
		int x, y;
		for (x = -4; x <= 4; x++)
		{
			for (y = -4; y <= 4; y++)
			{
				if(op->contr->blocked_los[dx + x][dy + y] & BLOCKED_LOS_BLOCKED)
					op->contr->blocked_los[dx + x][dy + y] &= ~BLOCKED_LOS_BLOCKED;
			}
		}
	}
}

/*
 * Clears/initialises the los-array associated to the player
 * controlling the object.
 */

void inline clear_los(object *op) 
{
    (void)memset((void *) op->contr->blocked_los,BLOCKED_LOS_VISIBLE,sizeof(op->contr->blocked_los));
}

/*
 * expand_sight goes through the array of what the given player is
 * able to see, and expands the visible area a bit, so the player will,
 * to a certain degree, be able to see into corners.
 * This is somewhat suboptimal, would be better to improve the formula.
 */
/* thats true: we should migrate this function asap i the los - a bit better
 * "pre calculated" LOS function (is it possible to pre calculate all cases from
 *  player x,y to blocksview x,y? a few hundred kb or even some mb should be worth
 * the time and cpu save) MT-2004
 */
#define BLOCKED_LOS_EXPAND 0x20

void expand_sight(object *op) 
{
    int i,x,y, dx, dy;

    for(x=1;x<op->contr->socket.mapx-1;x++)	/* loop over inner squares */
	{
	for(y=1;y<op->contr->socket.mapy-1;y++) {
#if 0
	    LOG(llevInfo ,"expand_sight x,y = %d, %d  blocksview = %d, %d\n",
		    x, y, op->x-op->contr->socket.mapx_2+x, op->y-op->contr->socket.mapy_2+y);
#endif
	    if(op->contr->blocked_los[x][y] <= BLOCKED_LOS_BLOCKSVIEW && /* if visible */
	        !(op->contr->blocked_los[x][y] &BLOCKED_LOS_BLOCKSVIEW))  /* and not blocksview */
		{

			/* mark all directions */
			for(i=1;i<=8;i+=1)
			{
			
				dx = x + freearr_x[i];
				dy = y + freearr_y[i];
			
				if (dx < 0 || dy < 0 || dx > op->contr->socket.mapx || dy > op->contr->socket.mapy)
					continue;

				if(op->contr->blocked_los[dx][dy]&BLOCKED_LOS_BLOCKED)
					op->contr->blocked_los[dx][dy]|=BLOCKED_LOS_EXPAND;
			}
	    }
	}
    }
	
    for (x = 0; x < op->contr->socket.mapx; x++)
	{
	for (y = 0; y < op->contr->socket.mapy; y++)
	{
	    if (op->contr->blocked_los[x][y]&BLOCKED_LOS_EXPAND)
			op->contr->blocked_los[x][y] &= ~BLOCKED_LOS_BLOCKED;
	}
	}
	

}




/* returns true if op carries one or more lights
 * This is a trivial function now days, but it used to
 * be a bit longer.  Probably better for callers to just
 * check the op->glow_radius instead of calling this.
 */

int has_carried_lights(object *op) {
    /* op may glow! */
    if(op->glow_radius>0) return 1;

    return 0;
}
 
void expand_lighted_sight(object *op)
{
    int x,y,darklevel,ax,ay, basex, basey,xt,yt, light;
    mapstruct *m;
 
	return;
    darklevel = MAP_DARKNESS(op->map);

    /* If the player can see in the dark, lower the darklevel for him */ 
    if(QUERY_FLAG(op,FLAG_SEE_IN_DARK)) darklevel -= 2;

    /* add light, by finding all (non-null) nearby light sources, then 
     * mark those squares specially. If the darklevel<1, there is no
     * reason to do this, so we skip this function 
     */

    if(darklevel<1) return;

    /* Do a sanity check.  If not valid, some code below may do odd
     * things.
     */
    if (darklevel > MAX_DARKNESS) 
	{
		LOG(llevBug,"BUG: Map darkness for %s on %s is too high (%d)\n",
											query_name(op), op->map->path, darklevel);
		darklevel = MAX_DARKNESS;
    }

    /* First, limit player furthest (unlighted) vision */
    for (x = 0; x < op->contr->socket.mapx; x++)
	for (y = 0; y < op->contr->socket.mapy; y++)
	    if(!(op->contr->blocked_los[x][y]==100))
		  op->contr->blocked_los[x][y]= MAX_LIGHT_RADII;

    /* the spaces[] darkness value contains the information we need.
     * Only process the area of interest.
     * the basex, basey values represent the position in the op->contr->blocked_los
     * array.  Its easier to just increment them here (and start with the right
     * value) than to recalculate them down below.
     */
    for (x=(op->x - op->contr->socket.mapx_2 - MAX_LIGHT_RADII), basex=-MAX_LIGHT_RADII;
      x < (op->x + op->contr->socket.mapx_2 + MAX_LIGHT_RADII); x++, basex++) {

	for (y=(op->y - op->contr->socket.mapy_2 - MAX_LIGHT_RADII), basey=-MAX_LIGHT_RADII;
	  y < (op->y + op->contr->socket.mapy_2 + MAX_LIGHT_RADII); y++, basey++) {

		xt=x,yt=y;
		if(!(m = out_of_map(op->map,&xt,&yt)))
			continue;

	    /* This space is providing light, so we need to brighten up the
	     * spaces around here.
	     */
	    if((light=(GET_MAP_LIGHT(m, xt, yt)))) 
		{

			for (ax=basex - light; ax<basex+light; ax++) {
		    if (ax<0 || ax>op->contr->socket.mapx) continue;
		    for (ay=basey - light; ay<basey+light; ay++) {
			if (ay<0 || ay>op->contr->socket.mapy) continue;

			/* If the space is fully blocked, do nothing.  Otherwise, we
			 * brighten the space.  The further the light is away from the
			 * source (basex-x), the less effect it has.  Note that as done
			 * done now, light dims in effectively a square manner.  for light radius
			 * as small as they are (4), this probably isn't terrible, but should
			 * perhaps be fixed.  It wouldn't be hard to creat a lookup table
			 * (dimming[abs(base-x)][abs(basey-y)]) that is actually calculated
			 * properly (ie, if the light is +2,+2, the dimming would be
			 * 3 (2.82 or sqrt(8)) and not 2 like it is right now.
			 */
			if(op->contr->blocked_los[ax][ay]!=100)
			    op->contr->blocked_los[ax][ay]-= light - MAX(abs(basex-ax),abs(basey -ay));
		    } /* for ay */
		} /* for ax */
	    } /* if this space is providing light */
	} /* for y */
    } /* for x */

    /* Outdoor should never really be completely pitch black dark like
     * a dungeon, so let the player at least see a little around themselves
     */
	  /*
    if (MAP_OUTDOORS(op->map) && darklevel > (MAX_DARKNESS - 3)) {
	if (op->contr->blocked_los[op->contr->socket.mapx/2][op->contr->socket.mapy/2] > (MAX_DARKNESS-3))
	    op->contr->blocked_los[op->contr->socket.mapx/2][op->contr->socket.mapy/2] = MAX_DARKNESS - 3;

	for (x=-1; x<=1; x++)
	    for (y=-1; y<=1; y++) {
		if (op->contr->blocked_los[x + op->contr->socket.mapx/2][y + op->contr->socket.mapy/2] > (MAX_DARKNESS-2))
		    op->contr->blocked_los[x + op->contr->socket.mapx/2][y + op->contr->socket.mapy/2] = MAX_DARKNESS - 2;
	    }
    }
*/

    /*  grant some vision to the player, based on the darklevel */
    for(x=darklevel-MAX_DARKNESS; x<MAX_DARKNESS + 1 -darklevel; x++)
	for(y=darklevel-MAX_DARKNESS; y<MAX_DARKNESS + 1 -darklevel; y++)
	    if(!(op->contr->blocked_los[x+op->contr->socket.mapx_2][y+op->contr->socket.mapy_2]==100))
		op->contr->blocked_los[x+op->contr->socket.mapx_2][y+op->contr->socket.mapy_2]-= 
		    MAX(0,6 -darklevel - MAX(abs(x),abs(y))); 
}

/* blinded_sight() - sets all veiwable squares to blocked except 
 * for the one the central one that the player occupies.  A little
 * odd that you can see yourself (and what your standing on), but
 * really need for any reasonable game play.
 */

void blinded_sight (object *op) {
/*
    int x,y;
    for (x = 0; x < op->contr->socket.mapx; x++)
	for (y = 0; y <  op->contr->socket.mapy; y++)
	    op->contr->blocked_los[x][y] = 100;

    op->contr->blocked_los[ op->contr->socket.mapx_2][ op->contr->socket.mapy_2] = 0;
	*/
}


/*
 * Debug-routine which dumps the array which specifies the visible
 * area of a player.  Triggered by the z key in DM mode.
 */

void print_los(object *op)
{
    int x,y;
    char buf[50], buf2[10];

    strcpy(buf,"   ");
    for(x=0;x<op->contr->socket.mapx;x++) {
	sprintf(buf2,"%2d",x);
	strcat(buf,buf2);
    }
    (*draw_info_func)(NDI_UNIQUE, 0, op, buf);
    for(y=0;y<op->contr->socket.mapy;y++) {
	sprintf(buf,"%2d:",y);
	for(x=0;x<op->contr->socket.mapx;x++) {
	    sprintf(buf2," %1d",op->contr->blocked_los[x][y]);
	    strcat(buf,buf2);
	}
	(*draw_info_func)(NDI_UNIQUE, 0, op, buf);
    }
}


/*
 * make_sure_seen: The object is supposed to be visible through walls, thus
 * check if any players are nearby, and edit their LOS array.
 */
void make_sure_seen(object *op) {
}

/*
 * make_sure_not_seen: The object which is supposed to be visible through
 * walls has just been removed from the map, so update the los of any
 * players within its range
 */

void make_sure_not_seen(object *op) { 
}
