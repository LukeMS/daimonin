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

#ifdef HAVE_DES_H
#include <des.h>
#else
#  ifdef HAVE_CRYPT_H
#  include <crypt.h>
#  endif
#endif

#ifndef __CEXTRACT__
#include <sproto.h>
#endif

#ifdef HAVE_TIME_H
#include <time.h>
#endif

/* #include <Mmsystem.h> swing time debug */
#include <pathfinder.h>

#include <../random_maps/random_map.h>
#include <../random_maps/rproto.h>

uint32 global_round_tag=1; /* global round ticker ! this is real a global */

static object marker; /* object for proccess_obejct(); */


static char days[7][4] = {
  "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

void version(object *op) {

  new_draw_info_format(NDI_UNIQUE, 0, op, "This is Daimonin v%s",VERSION);

/* If in a socket, don't print out the list of authors.  It confuses the
 * crossclient program.
 */
  if (op==NULL) return;
  new_draw_info(NDI_UNIQUE, 0,op,"Authors and contributors to Daimonin & Crossfire:");
  new_draw_info(NDI_UNIQUE, 0,op,"(incomplete list - mail us if you miss your name):");
  new_draw_info(NDI_UNIQUE, 0,op,"mark@scruz.net (Mark Wedel)");
  new_draw_info(NDI_UNIQUE, 0,op,"frankj@ifi.uio.no (Frank Tore Johansen)");
  new_draw_info(NDI_UNIQUE, 0,op,"kjetilho@ifi.uio.no (Kjetil Torgrim Homme)");
  new_draw_info(NDI_UNIQUE, 0,op,"tvangod@ecst.csuchico.edu (Tyler Van Gorder)");
  new_draw_info(NDI_UNIQUE, 0,op,"elmroth@cd.chalmers.se (Tony Elmroth)");
  new_draw_info(NDI_UNIQUE, 0,op,"dougal.scott@fcit.monasu.edu.au (Dougal Scott)");
  new_draw_info(NDI_UNIQUE, 0,op,"wchuang@athena.mit.edu (William)");
  new_draw_info(NDI_UNIQUE, 0,op,"ftww@cs.su.oz.au (Geoff Bailey)");
  new_draw_info(NDI_UNIQUE, 0,op,"jorgens@flipper.pvv.unit.no (Kjetil Wiekhorst Jxrgensen)");
  new_draw_info(NDI_UNIQUE, 0,op,"c.blackwood@rdt.monash.edu.au (Cameron Blackwood)");
  new_draw_info(NDI_UNIQUE, 0,op,"jtraub+@cmu.edu (Joseph L. Traub)");
  new_draw_info(NDI_UNIQUE, 0,op,"rgg@aaii.oz.au (Rupert G. Goldie)");
  new_draw_info(NDI_UNIQUE, 0,op,"eanders+@cmu.edu (Eric A. Anderson)");
  new_draw_info(NDI_UNIQUE, 0,op,"eneq@Prag.DoCS.UU.SE (Rickard Eneqvist)");
  new_draw_info(NDI_UNIQUE, 0,op,"Jarkko.Sonninen@lut.fi (Jarkko Sonninen)");
  new_draw_info(NDI_UNIQUE, 0,op,"kholland@sunlab.cit.cornell.du (Karl Holland)");
  new_draw_info(NDI_UNIQUE, 0,op,"vick@bern.docs.uu.se (Mikael Lundgren)");
  new_draw_info(NDI_UNIQUE, 0,op,"mol@meryl.csd.uu.se (Mikael Olsson)");
  new_draw_info(NDI_UNIQUE, 0,op,"Tero.Haatanen@lut.fi (Tero Haatanen)");
  new_draw_info(NDI_UNIQUE, 0,op,"ylitalo@student.docs.uu.se (Lasse Ylitalo)");
  new_draw_info(NDI_UNIQUE, 0,op,"anipa@guru.magic.fi (Niilo Neuvo)");
  new_draw_info(NDI_UNIQUE, 0,op,"mta@modeemi.cs.tut.fi (Markku J{rvinen)");
  new_draw_info(NDI_UNIQUE, 0,op,"meunier@inf.enst.fr (Sylvain Meunier)");
  new_draw_info(NDI_UNIQUE, 0,op,"jfosback@darmok.uoregon.edu (Jason Fosback)");
  new_draw_info(NDI_UNIQUE, 0,op,"cedman@capitalist.princeton.edu (Carl Edman)");
  new_draw_info(NDI_UNIQUE, 0,op,"henrich@crh.cl.msu.edu (Charles Henrich)");
  new_draw_info(NDI_UNIQUE, 0,op,"schmid@fb3-s7.math.tu-berlin.de (Gregor Schmid)");
  new_draw_info(NDI_UNIQUE, 0,op,"quinet@montefiore.ulg.ac.be (Raphael Quinet)");
  new_draw_info(NDI_UNIQUE, 0,op,"jam@modeemi.cs.tut.fi (Jari Vanhala)");
  new_draw_info(NDI_UNIQUE, 0,op,"kivinen@joker.cs.hut.fi (Tero Kivinen)");
  new_draw_info(NDI_UNIQUE, 0,op,"peterm@soda.berkeley.edu (Peter Mardahl)");
  new_draw_info(NDI_UNIQUE, 0,op,"matt@cs.odu.edu (Matthew Zeher)");
  new_draw_info(NDI_UNIQUE, 0,op,"srt@sun-dimas.aero.org (Scott R. Turner)");
  new_draw_info(NDI_UNIQUE, 0,op,"huma@netcom.com (Ben Fennema)");
  new_draw_info(NDI_UNIQUE, 0,op,"njw@cs.city.ac.uk (Nick Williams)");
  new_draw_info(NDI_UNIQUE, 0,op,"Wacren@Gin.ObsPM.Fr (Laurent Wacrenier)");
  new_draw_info(NDI_UNIQUE, 0,op,"thomas@astro.psu.edu (Brian Thomas)");
  new_draw_info(NDI_UNIQUE, 0,op,"jsm@axon.ksc.nasa.gov (John Steven Moerk)");
  new_draw_info(NDI_UNIQUE, 0,op,"Delbecq David       [david.delbecq@mailandnews.com]");
  new_draw_info(NDI_UNIQUE, 0,op,"Chachkoff Yann      [yann.chachkoff@mailandnews.com]\n");
  new_draw_info(NDI_UNIQUE, 0,op,"Images and art:");
  new_draw_info(NDI_UNIQUE, 0,op,"Peter Gardner");
  new_draw_info(NDI_UNIQUE, 0,op,"David Gervais       [david_eg@mail.com]");
  new_draw_info(NDI_UNIQUE, 0,op,"Mitsuhiro Itakura   [ita@gold.koma.jaeri.go.jp]");
  new_draw_info(NDI_UNIQUE, 0,op,"Hansjoerg Malthaner [hansjoerg.malthaner@danet.de]");
  new_draw_info(NDI_UNIQUE, 0,op,"Mårten Woxberg      [maxmc@telia.com]");
  new_draw_info(NDI_UNIQUE, 0,op,"The mighty Dink.");
  new_draw_info(NDI_UNIQUE, 0,op,"And many more!");
}

void start_info(object *op) {
  char buf[MAX_BUF];

  sprintf(buf,"Welcome to Daimonin, v%s!",VERSION);
  new_draw_info(NDI_UNIQUE, 0,op,buf);
}

char *crypt_string(char *str, char *salt) {
#ifndef WIN32 /* ***win32 crypt_string:: We don't need this anymore since server/client fork */
  static char *c=
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";
  char s[2];
  if(salt==NULL)
    s[0]= c[RANDOM() % (int)strlen(c)],
    s[1]= c[RANDOM() % (int)strlen(c)];
  else
    s[0]= salt[0],
    s[1]= salt[1];
#ifdef HAVE_LIBDES
  return (char*)des_crypt(str,s);
#else
  return (char*)crypt(str,s);
#endif
#endif /* win32 */
  return(str);
}

int check_password(char *typed,char *crypted) {
  return !strcmp(crypt_string(typed,crypted),crypted);
}

/* This is a basic little function to put the player back to his
 * savebed.  We do some error checking - its possible that the
 * savebed map may no longer exist, so we make sure the player
 * goes someplace.
 */
void enter_player_savebed(object *op)
{
    mapstruct	*oldmap = op->map;
    object  *tmp;

    tmp=get_object();

    FREE_AND_COPY_HASH(EXIT_PATH(tmp),op->contr->savebed_map);
    EXIT_X(tmp) = op->contr->bed_x;
    EXIT_Y(tmp) = op->contr->bed_y;
    enter_exit(op,tmp);
    /* If the player has not changed maps and the name does not match
     * that of the savebed, his savebed map is gone.  Lets go back
     * to the emergency path.  Update what the players savebed is
     * while we're at it.
     */
    if (oldmap == op->map && strcmp(op->contr->savebed_map, oldmap->path)) 
	{
		/* HOTFIX: apartments: We have saved in beta 1 stoneglow apartments as
		 * "_stoneglow_appartment" - but in beta 2 we need "$stoneglow$appartment"
		 * lets search for "_stoneglow_appartment" in the save bed string.
		 * if we find it, change it to '§' version and give it another try.
		 */
		char *tmp_str;
		if((tmp_str=strstr(op->contr->savebed_map,"_stoneglow_appartment")))
		{
			LOG(llevDebug,"HOTFIX (%s): found old save bed string -  trying '$' version.\n", query_name(op));
			tmp_str[0] = '$';	
			tmp_str[10] = '$';
			
		    FREE_AND_COPY_HASH(EXIT_PATH(tmp),op->contr->savebed_map);
		    enter_exit(op,tmp);
		    if (oldmap == op->map && strcmp(op->contr->savebed_map, oldmap->path)) 
			{
				LOG(llevDebug,"HOTFIX (%s): failed.\n", query_name(op));
			}
			else
			{
				LOG(llevDebug,"HOTFIX (%s): success.\n", query_name(op));
			    free_object(tmp); 
				return;
			}
		}

		LOG(llevDebug,"Player %s savebed location %s is invalid - going to EMERGENCY_MAPPATH (%s)\n",
									    query_name(op), op->contr->savebed_map,EMERGENCY_MAPPATH);
		strcpy(op->contr->savebed_map, EMERGENCY_MAPPATH);
		op->contr->bed_x = EMERGENCY_X;
		op->contr->bed_y = EMERGENCY_Y;
		FREE_AND_COPY_HASH(EXIT_PATH(tmp),op->contr->savebed_map);
		EXIT_X(tmp) = op->contr->bed_x;
		EXIT_Y(tmp) = op->contr->bed_y;
		enter_exit(op,tmp);
    }
    free_object(tmp);
}


/* All this really is is a glorified remove_object that also updates
 * the counts on the map if needed.
 */
void leave_map(object *op)
{
    mapstruct *oldmap = op->map;

    remove_ob(op);

	if (oldmap && !oldmap->player_first && !oldmap->perm_load)
	    set_map_timeout(oldmap);
}

/*
 *  enter_map():  Moves the player and pets from current map (if any) to
 * new map.  map, x, y must be set.  map is the map we are moving the
 * player to - it could be the map he just came from if the load failed for
 * whatever reason.  If default map coordinates are to be used, then
 * the function that calls this should figure them out.
 */
static void enter_map(object *op, mapstruct *newmap, int x, int y, int pos_flag) {
	int i=0;
	object *tmp;
    mapstruct *oldmap = op->map;
#ifdef PLUGINS
    int evtid;
    CFParm CFP;
#endif


	if(op->head)
	{
		op=op->head;
		LOG(llevBug,"BUG: enter_map(): called from tail of object! (obj:%s map: %s (%d,%d))\n",op->name,newmap->path, x, y);
	}
	/* this is a last secure check. In fact, newmap MUST legal and we only
	 * check x and y. No out_of_map() - we want check that x,y is part of this newmap.
	 * if not, we have somewhere missed some checks - give a note to the log.
	 */
	if(OUT_OF_REAL_MAP(newmap,x,y) )
	{
		LOG(llevBug,"BUG: enter_map(): supplied coordinates are not within the map! (obj:%s map: %s (%d,%d))\n",op->name,newmap->path, x, y);
		x=MAP_ENTER_X(newmap);
		y=MAP_ENTER_Y(newmap);
    }

    /* try to find a spot for our object - (single arch or multi head)
	 * but only when we don't put it on fix position 
	 */
    if (!pos_flag && arch_blocked(op->arch, op, newmap, x, y)) 
	{
		/* First choice blocked */
		/* We try to find a spot for the player, starting closest in.
		 * We could use find_first_free_spot, but that doesn't randomize it at all,
		 * So for example, if the north space is free, you would always end up there even
		 * if other spaces around are available.
		 * Note that for the second and third calls, we could start at a position other
		 * than one, but then we could end up on the other side of walls and so forth.
		 */
		i = find_free_spot(op->arch,newmap, x, y, 1, SIZEOFFREE1+1);
		if (i==-1)
		{
			i = find_free_spot(op->arch,newmap, x, y, 1, SIZEOFFREE2+1);
			if (i==-1)
				i = find_free_spot(op->arch,newmap, x, y, 1, SIZEOFFREE+1);
		}

		if(i == -1)
			i=0; /* thats +0,+0 == same spot */
    } /* end if looking for free spot */
	

    /* If it is a player login, he has yet to be inserted anyplace.
     * otherwise, we need to deal with removing the playe here.
     */
    if(!QUERY_FLAG(op, FLAG_REMOVED))
		remove_ob(op);

	/* i don't like this system... First, i want attach leave/enter
	 * scripts perhaps to a map more often, as to a single object.
	 * Second, attaching this to single objects can cause some 
	 * usage when we think about tiled maps.
	 */
#ifdef PLUGINS
    if (op->map!=NULL && op->type == PLAYER && !op->head)
    {
    	/* GROS : Here we handle the MAPLEAVE global event */
    	evtid = EVENT_MAPLEAVE;
    	CFP.Value[0] = (void *)(&evtid);
    	CFP.Value[1] = (void *)(op);
    	GlobalEvent(&CFP);
    };
#endif

  /* set single or all part of a multi arch */
  for(tmp=op;tmp!=NULL;tmp=tmp->more)
  {
    tmp->x = tmp->arch->clone.x+x+freearr_x[i];
    tmp->y = tmp->arch->clone.y+y+freearr_y[i];
	tmp->map = newmap;
  }
  insert_ob_in_map(op, newmap, NULL,0);


/* GROS : Here we handle the MAPENTER global event */
/* i disabled this ... Don't like this as "global event"
 * even i can see no use - even less, when i think about
 * the cpu time of this nightmare.
 * Can you think about 100 players, crossing tiled maps
 * over and over? MT-2003
 */
  /*
#ifdef PLUGINS
    evtid = EVENT_MAPENTER;
    CFP.Value[0] = (void *)(&evtid);
    CFP.Value[1] = (void *)(op);
    GlobalEvent(&CFP);
#endif
*/

    newmap->timeout=0;

	/* do some action special for players after we have inserted them */
	if(op->type == PLAYER)
	{
		if (op->contr) 
		{
			strcpy(op->contr->maplevel, newmap->path);
			op->contr->count=0;
		}

		/* Update any golems */
		if(op->contr->golem != NULL) 
		{
			int i = find_free_spot(op->contr->golem->arch,newmap, x, y, 1, SIZEOFFREE+1);

			remove_ob(op->contr->golem);
			if (i==-1)
			{
				send_golem_control(op->contr->golem, GOLEM_CTR_RELEASE);
				remove_friendly_object(op->contr->golem);
				free_object(op->contr->golem);
				op->contr->golem=NULL;
			}
			else 
			{
				object *tmp;
				for (tmp=op->contr->golem; tmp!=NULL; tmp=tmp->more)
				{
					tmp->x = x + freearr_x[i]+ (tmp->arch==NULL?0:tmp->arch->clone.x);
					tmp->y = y + freearr_y[i]+ (tmp->arch==NULL?0:tmp->arch->clone.y);
					tmp->map = newmap;
				}
				insert_ob_in_map(op->contr->golem, newmap, NULL,0);
				op->contr->golem->direction = find_dir_2(op->x - op->contr->golem->x, op->y - op->contr->golem->y);
			}
		}
		op->direction=0;

		/* since the players map is already loaded, we don't need to worry
		* about pending objects.
		*/
		remove_all_pets(newmap);

		/* If the player is changing maps, we need to do some special things
		* Do this after the player is on the new map - otherwise the force swap of the
		* old map does not work.
		*/
		if (oldmap != newmap && oldmap && !oldmap->player_first && !oldmap->perm_load)
			set_map_timeout(oldmap);

		swap_below_max (newmap->path);
        MapNewmapCmd( op->contr);
	}
}

void set_map_timeout(mapstruct *oldmap)
{
#if MAP_MAXTIMEOUT
    oldmap->timeout = MAP_TIMEOUT(oldmap);
    /* Do MINTIMEOUT first, so that MAXTIMEOUT is used if that is
     * lower than the min value.
     */
#if MAP_MINTIMEOUT
    if (oldmap->timeout < MAP_MINTIMEOUT) {
	oldmap->timeout = MAP_MINTIMEOUT;
    }
#endif
    if (oldmap->timeout > MAP_MAXTIMEOUT) {
	oldmap->timeout = MAP_MAXTIMEOUT;
    }
#else
    /* save out the map */
    swap_map(oldmap);
#endif /* MAP_MAXTIMEOUT */
}


/* clean_path takes a path and replaces all / with _
 * We do a strcpy so that we do not change the original string.
 */
char *clean_path(const char *file)
{
    static char newpath[MAX_BUF],*cp;

    strncpy(newpath, file, MAX_BUF-1);
    newpath[MAX_BUF-1]='\0';
    for (cp=newpath; *cp!='\0'; cp++) {
	if (*cp=='/') *cp='$';
    }
    return newpath;
}


/* unclean_path takes a path and replaces all _ with /
 * This basically undoes clean path.
 * We do a strcpy so that we do not change the original string.
 * We are smart enough to start after the last / in case we
 * are getting passed a string that points to a unique map
 * path.
 */
char *unclean_path(const char *src)
{
    static char newpath[MAX_BUF], *cp2;
    const char *cp;

    cp=strrchr(src, '/');
    if (cp)
	strncpy(newpath, cp+1, MAX_BUF-1);
    else
	strncpy(newpath, src, MAX_BUF-1);
    newpath[MAX_BUF-1]='\0';

    for (cp2=newpath; *cp2!='\0'; cp2++) {
	if (*cp2=='$') *cp2='/';
    }
    return newpath;
}


/* The player is trying to enter a randomly generated map.  In this case, generate the
 * random map as needed.
 */

static void enter_random_map(object *pl, object *exit_ob)
{
    mapstruct *new_map;
    char newmap_name[HUGE_BUF];
    static int reference_number = 0;
    RMParms rp;

    memset(&rp, 0, sizeof(RMParms));
    rp.Xsize=-1;
    rp.Ysize=-1;
    if (exit_ob->msg) set_random_map_variable(&rp,exit_ob->msg);
    rp.origin_x = exit_ob->x;
    rp.origin_y = exit_ob->y;
    rp.generate_treasure_now = 1;
    strcpy(rp.origin_map, pl->map->path);

    /* pick a new pathname for the new map.  Currently, we just
     * use a static variable and increment the counter one each time.
     */
    sprintf(newmap_name,"/random/%016d",reference_number++);

    /* now to generate the actual map. */
    new_map=(mapstruct *)generate_random_map(newmap_name,&rp);

    /* Update the exit_ob so it now points directly at the newly created
     * random maps.  Not that it is likely to happen, but it does mean that a
     * exit in a unique map leading to a random map will not work properly.
     * It also means that if the created random map gets reset before
     * the exit leading to it, that the exit will no longer work.
     */
    if(new_map) {
	int x, y;
	x=EXIT_X(exit_ob) = MAP_ENTER_X(new_map);
	y=EXIT_Y(exit_ob) = MAP_ENTER_Y(new_map);
	FREE_AND_COPY_HASH(EXIT_PATH(exit_ob), newmap_name);
	FREE_AND_COPY_HASH(new_map->path, newmap_name);
	enter_map(pl, new_map, 	x, y,QUERY_FLAG(exit_ob,FLAG_USE_FIX_POS));
    }
}


/* Code to enter/detect a character entering a unique map.
 */
static void enter_unique_map(object *op, object *exit_ob)
{
    char apartment[HUGE_BUF];
    mapstruct	*newmap;

    if (EXIT_PATH(exit_ob)[0]=='/') {
	sprintf(apartment, "%s/%s/%s/%s", settings.localdir,
	    settings.playerdir, op->name, clean_path(EXIT_PATH(exit_ob)));
	newmap = ready_map_name(apartment, MAP_PLAYER_UNIQUE);
	if (!newmap) {
	    newmap = load_original_map(create_pathname(EXIT_PATH(exit_ob)), MAP_PLAYER_UNIQUE);
	    if (newmap) fix_auto_apply(newmap);
	}
    } else { /* relative directory */
	char reldir[HUGE_BUF], tmpc[HUGE_BUF], tmp_path[HUGE_BUF], *cp;

	if (MAP_UNIQUE(exit_ob->map)) {

	    strcpy(reldir, unclean_path(exit_ob->map->path));

	    /* Need to copy this over, as clean_path only has one static return buffer */
	    strcpy(tmpc, clean_path(reldir));
	    /* Remove final component, if any */ 
	    if ((cp=strrchr(tmpc, '$'))!=NULL) *cp=0;

	    sprintf(apartment, "%s/%s/%s/%s_%s", settings.localdir,
		    settings.playerdir, op->name, tmpc,
		    clean_path(EXIT_PATH(exit_ob)));

	    newmap = ready_map_name(apartment, MAP_PLAYER_UNIQUE);
	    if (!newmap) {
		newmap = load_original_map(create_pathname(normalize_path(reldir, EXIT_PATH(exit_ob), tmp_path)), MAP_PLAYER_UNIQUE);
		if (newmap) fix_auto_apply(newmap);
	    }
	}
	else {
	    /* The exit is unique, but the map we are coming from is not unique.  So
	     * use the basic logic - don't need to demangle the path name
	     */
	    sprintf(apartment, "%s/%s/%s/%s", settings.localdir,
		    settings.playerdir, op->name, 
		    clean_path(normalize_path(exit_ob->map->path, EXIT_PATH(exit_ob), tmp_path)));
	    newmap = ready_map_name(apartment, MAP_PLAYER_UNIQUE);
	    if (!newmap) {
		newmap = ready_map_name(normalize_path(exit_ob->map->path, EXIT_PATH(exit_ob), tmp_path), 0);
		if (newmap) fix_auto_apply(newmap);
	    }
	}
    }

    if (newmap) {
        FREE_AND_COPY_HASH(newmap->path, apartment);
		newmap->map_flags |=MAP_FLAG_UNIQUE;
	enter_map(op, newmap, EXIT_X(exit_ob), EXIT_Y(exit_ob),QUERY_FLAG(exit_ob,FLAG_USE_FIX_POS));
    } else {
	new_draw_info_format(NDI_UNIQUE, 0, op, "The %s is closed.", query_name(exit_ob));
	/* Perhaps not critical, but I would think that the unique maps
	 * should be new enough this does not happen.  This also creates
	 * a strange situation where some players could perhaps have visited
	 * such a map before it was removed, so they have the private
	 * map, but other players can't get it anymore.
	 */
	LOG(llevDebug,"Debug: enter_unique_map: Exit %s (%d,%d) on map %s is leads no where.\n",
		query_name(exit_ob), exit_ob->x, exit_ob->y, exit_ob->map?exit_ob->map->path?exit_ob->map->path:"NO_PATH (script?)":"NO_MAP (script?)");
    }
	
}


/* Tries to move 'op' to exit_ob.  op is the character or monster that is
 * using the exit, where exit_ob is the exit object (boat, door, teleporter,
 * etc.)  if exit_ob is null, then op->contr->maplevel contains that map to
 * move the object to.  This is used when loading the player.
 *
 * Largely redone by MSW 2001-01-21 - this function was overly complex
 * and had some obscure bugs.
 * Redone this function. Now it works fine for monsters, players and normal items.
 * MT-2003
 */
void enter_exit(object *op, object *exit_ob)
{
	object *tmp;

	if(op->head)
		op=op->head;

    /* First, lets figure out what map we go */
    if (exit_ob)
	{ 

		/* check to see if we make a randomly generated map */
		if(EXIT_PATH(exit_ob)&&EXIT_PATH(exit_ob)[1]=='!')
		{
			if(op->type != PLAYER)
				return;
			if(exit_ob->sub_type1 == ST1_EXIT_SOUND && exit_ob->map)
				play_sound_map(exit_ob->map, exit_ob->x, exit_ob->y, SOUND_TELEPORT, SOUND_NORMAL);
			enter_random_map(op, exit_ob);
		}
		else if (exit_ob->last_eat == MAP_PLAYER_MAP)
		{
			if(op->type != PLAYER)
				return;
			if(exit_ob->sub_type1 == ST1_EXIT_SOUND && exit_ob->map)
				play_sound_map(exit_ob->map, exit_ob->x, exit_ob->y, SOUND_TELEPORT, SOUND_NORMAL);
			enter_unique_map(op, exit_ob);
		} 
		else 
		{
			int x=EXIT_X(exit_ob), y=EXIT_Y(exit_ob);
                        char tmp_path[HUGE_BUF];
			/* 'Normal' exits that do not do anything special
			* Simple enough we don't need another routine for it.
			*/
			mapstruct	*newmap;

			if (exit_ob->map)
			{
				/* wir sind auf einer unique map - appartment.
				 * WARNING: This is a work around - i need for personl/group maps to change
				 * ready_map_name in a more clever way!
				 */
				if (strncmp(exit_ob->map->path, settings.localdir, strlen(settings.localdir)))
					newmap = ready_map_name(normalize_path(exit_ob->map->path, EXIT_PATH(exit_ob), tmp_path), 0);
				else
					newmap = ready_map_name(normalize_path("",EXIT_PATH(exit_ob), tmp_path), 0);
				/* Random map was previously generated, but is no longer about.  Lets generate a new
				* map.
				*/
				if (!newmap && !strncmp(EXIT_PATH(exit_ob),"/random/",8))
				{
					if(op->type != PLAYER)
						return;
					if(exit_ob->sub_type1 == ST1_EXIT_SOUND && op->map)
						play_sound_map(exit_ob->map, exit_ob->x, exit_ob->y, SOUND_TELEPORT, SOUND_NORMAL);
					enter_random_map(op, exit_ob);
					/* For exits that cause damages (like pits).  Don't know if any
					* random maps use this or not.
					*/
					if(exit_ob->stats.dam && op->type==PLAYER)
						hit_player(op,exit_ob->stats.dam,exit_ob,exit_ob->attacktype);
					return;
				}
			} 
			else 
			{
				/* For word of recall and other force objects
				* They contain the full pathname of the map to go back to,
				* so we don't need to normalize it.
				* But we do need to see if it is unique or not 
				*/
				if (!strncmp(EXIT_PATH(exit_ob), settings.localdir, strlen(settings.localdir)))
					newmap = ready_map_name(EXIT_PATH(exit_ob), MAP_NAME_SHARED|MAP_PLAYER_UNIQUE);
				else
					newmap = ready_map_name(EXIT_PATH(exit_ob), MAP_NAME_SHARED);
			}

			if (!newmap)
			{
				if(op->type == PLAYER)
					new_draw_info_format(NDI_UNIQUE, 0, op, "The %s is closed.", query_name(exit_ob));
				return;
			}

			/* -1,-1 marks to use the default ENTER_xx position of the map */ 
			if (x==-1 && y==-1) 
			{
				x=MAP_ENTER_X(newmap);
				y=MAP_ENTER_Y(newmap);
			}

			/* mids 02/13/2002 if exit is damned, update players death & WoR home-position and delete town portal */
			if (QUERY_FLAG(exit_ob, FLAG_DAMNED))
			{
				if(op->type != PLAYER) /* player only here ... */
					return;
				/* remove an old force with a slaying field == PORTAL_DESTINATION_NAME */
				for(tmp=op->inv; tmp != NULL; tmp = tmp->below)
				{
					if(tmp->type == FORCE && tmp->slaying && !strcmp(tmp->slaying, PORTAL_DESTINATION_NAME))
						break;
				}
				if(tmp)
				{
					remove_ob(tmp);
					free_object(tmp);
				}
				strcpy(op->contr->savebed_map, normalize_path(exit_ob->map->path, EXIT_PATH(exit_ob), tmp_path));
				op->contr->bed_x = EXIT_X(exit_ob), op->contr->bed_y = EXIT_Y(exit_ob);
				save_player(op, 1);
			/* LOG(llevDebug,"enter_exit: Taking damned exit %s to (%d,%d) on map %s\n",
               * exit_ob->name?exit_ob->name:"(none)", exit_ob->x, exit_ob->y,  
               * normalize_path(exit_ob->map->path, EXIT_PATH(exit_ob))); */
			}
			if(exit_ob->sub_type1 == ST1_EXIT_SOUND && exit_ob->map)
				play_sound_map(exit_ob->map, exit_ob->x, exit_ob->y, SOUND_TELEPORT, SOUND_NORMAL);
			enter_map(op, newmap, x, y,QUERY_FLAG(exit_ob,FLAG_USE_FIX_POS));
		}
		/* For exits that cause damages (like pits) */
		if(exit_ob->stats.dam && op->type==PLAYER)
			hit_player(op,exit_ob->stats.dam,exit_ob,exit_ob->attacktype);
	} /* exit_ob */ 
	else if (op->type == PLAYER) /* thats only for players */
	{
		int flags = 0;
		mapstruct *newmap;


		/* Hypothetically, I guess its possible that a standard map matches
		* the localdir, but that seems pretty unlikely - unlikely enough that
		* I'm not going to attempt to try to deal with that possibility.
		* We use the fact that when a player saves on a unique map, it prepends
		* the localdir to that name.  So its an easy way to see of the map is
		* unique or not.
		*/
		if (!strncmp(op->contr->maplevel, settings.localdir, strlen(settings.localdir)))
			flags = MAP_PLAYER_UNIQUE;

		/* newmap returns the map (if already loaded), or loads it for
		* us.
		*/
		newmap = ready_map_name(op->contr->maplevel, flags);
		if (!newmap) 
		{
			/* HOTFIX; for beta 1 stoneglow apartments */
			char *tmp_str;
			if((tmp_str=strstr(op->contr->maplevel,"_stoneglow_appartment")))
			{
				LOG(llevDebug,"HOTFIX (%s): found old save bed string -  trying '$' version.\n", query_name(op));
				tmp_str[0] = '$';	
				tmp_str[10] = '$';
			
				newmap = ready_map_name(op->contr->maplevel, flags);

				LOG(llevDebug,"HOTFIX (%s): ready map %x.\n", query_name(op), newmap);
			}


			if (!newmap)
			{
				LOG(llevBug,"BUG: enter_exit(): Pathname to map does not exist! player: %s (%s)\n", 
																	op->name,	op->contr->maplevel);
				newmap = ready_map_name(EMERGENCY_MAPPATH, 0);
				op->x = EMERGENCY_X;
				op->y = EMERGENCY_Y;
			}
			/* If we can't load the emergency map, something is probably really
			* screwed up, so bail out now.
			*/
			if (!newmap)
				LOG(llevError,"ERROR: enter_exit(): could not load emergency map? Fatal error! (player: %s)\n", op->name);
		}
		/* -1,-1 marks to use the default ENTER_xx position of the map */ 
		if (op->x==-1 && op->y==-1) 
		{
			op->x=MAP_ENTER_X(newmap);
			op->y=MAP_ENTER_Y(newmap);
		}
		enter_map(op, newmap, op->x, op->y,1);
	}
}

/* process_players1 and process_players2 do all the player related stuff.
 * I moved it out of process events and process_map.  This was to some
 * extent for debugging as well as to get a better idea of the time used
 * by the various functions.  process_players1() does the processing before
 * objects have been updated, process_players2() does the processing that
 * is needed after the players have been updated.
 */

void process_players1(mapstruct *map)
{
    int flag;
    player *pl,*plnext;

/*	static DWORD pCount_old, pCount;  swing time debug */

    /* Basically, we keep looping until all the players have done their actions. */
    for(flag=1;flag!=0;) {
	flag=0;
	for(pl=first_player;pl!=NULL;pl=plnext) {


	    plnext=pl->next; /* In case a player exits the game in handle_player() */
/*	    if (map!=NULL && (pl->ob == NULL || pl->ob->map!=map)) continue;*/

#ifdef AUTOSAVE
	    /* check for ST_PLAYING state so that we don't try to save off when
	     * the player is logging in.
	     */
	    if ((pl->last_save_tick+AUTOSAVE)<pticks && pl->state==ST_PLAYING) {
		/* Don't save the player on unholy ground.  Instead, increase the
		 * tick time so it will be about 10 seconds before we try and save
		 * again.
		 */
		if (blocks_cleric(pl->ob->map, pl->ob->x, pl->ob->y)) {
		    pl->last_save_tick += 100;
		} else {
		    save_player(pl->ob,1);
		    pl->last_save_tick = pticks;
		}
	    }
#endif
            if(pl->ob == NULL) {
                /* I take it this should never happen,
                   but it seems to anyway :( */
                flag = 1;
            } else if(pl->ob->speed_left>0) {
		if (handle_newcs_player(pl->ob))
		    flag=1;
/*        if(pl->ob->enemy) */
        pl->ob->weapon_speed_left-=pl->ob->weapon_speed_add;
            } /* end if player has speed left */
	} /* end of for loop for all the players */
    } /* for flag */
    for(pl=first_player;pl!=NULL;pl=pl->next) {
	if (map!=NULL && (pl->ob == NULL || pl->ob->map!=map)) continue;
#ifdef CASTING_TIME
	if (pl->ob->casting > 0){
	    pl->ob->casting--;
	    pl->ob->start_holding = 1;
	}
	/* set spell_state so we can update the range in stats field */
	if ((pl->ob->casting == 0) && (pl->ob->start_holding ==1)){
	    pl->ob->start_holding = 0;
	    pl->ob->spell_state = 1;
	}
#endif
    
    do_some_living(pl->ob);

		/* now use the new target system to hit our target... Don't hit non
		 * friendly objects, ourself or when we are not in combat mode.
		 */
		if(pl->target_object && pl->combat_mode &&
			pl->target_object_count!=pl->ob->count && !QUERY_FLAG(pl->target_object,FLAG_FRIENDLY) )
        {
            if(pl->ob->weapon_speed_left<=0)
            {
				/* swing time debug
				pCount = timeGetTime();
				LOG(llevDebug,"Swingtime: %f (%d %d)\n",(float)(pCount-pCount_old)/1000.0f, (float)pCount, (float)pCount_old);
				pCount_old=pCount;
				*/

				/* now we force target as enemy */
				pl->ob->enemy = pl->target_object;
				pl->ob->enemy_count = pl->target_object_count;

				/* quick check our target is still valid: count ok? (freed...), not
				 * removed, not a bet or object we self own (TODO: group pets!)
				 * Note: i don't do a invisible check here... this will happen one
				 * at end of this round... so, we have a "object turn invisible and
				 * we do a last hit here"
				 */
				
				if(pl->ob->enemy->count != pl->ob->enemy_count || 
						pl->ob->enemy->owner == pl->ob || QUERY_FLAG(pl->ob->enemy, FLAG_REMOVED))
						/*QUERY_FLAG(pl->ob->enemy, FLAG_FREED) || ->count should do this job */
					pl->ob->enemy=NULL;
                else if(is_melee_range(pl->ob, pl->ob->enemy))
				{
					/* tell our enemy we swing at him now */
					if (!OBJECT_VALID(pl->ob->enemy->enemy, pl->ob->enemy->enemy_count))
                        set_npc_enemy(pl->ob->enemy, pl->ob, NULL);
					else /* our target has already a enemy - then note we had attacked */
					{
						pl->ob->enemy->attacked_by=pl->ob;
						pl->ob->enemy->attacked_by_distance = 1; /* melee... there is no way nearer */
					}
					pl->praying=0;
                    skill_attack(pl->ob->enemy, pl->ob, 0, NULL);
					/* we want only *one* swing - not several swings per tick */
	                pl->ob->weapon_speed_left+=FABS((int)pl->ob->weapon_speed_left)+1;
				}
            }    
        }
        else
		{
            if(pl->ob->weapon_speed_left<=0)
				pl->ob->weapon_speed_left=0;
		}
    }
}

void process_players2(mapstruct *map)
{
    player *pl;

    for(pl=first_player;pl!=NULL;pl=pl->next) 
    {

		/* thats for debug spells - if enabled, mana/grace is always this value 
		pl->ob->stats.grace = 900;
		pl->ob->stats.sp = 900;
		*/

		/* look our target is still valid - if not, update client 
		 * we handle op->enemy for the player here too!
		 */
		if(pl->ob->map && (!pl->target_object ||
				(pl->target_object!=pl->ob&&pl->target_object_count!=pl->target_object->count)
					|| QUERY_FLAG(pl->target_object,FLAG_SYS_OBJECT) || 
						(QUERY_FLAG(pl->target_object,FLAG_IS_INVISIBLE) && !QUERY_FLAG(pl->ob,FLAG_SEE_INVISIBLE)) ))
			send_target_command(pl);	

		if (pl->ob->speed_left>pl->ob->speed)
	      pl->ob->speed_left = pl->ob->speed;
    }

}

void process_events (mapstruct *map)
{
	object *op;
	tag_t tag;

	process_players1 (map);

  /* Put marker object at beginning of active list */
  marker.active_next = active_objects;
  if (marker.active_next)
    marker.active_next->active_prev = &marker;
  marker.active_prev = NULL;
  active_objects = &marker;
 
  while (marker.active_next)
  {
    op = marker.active_next;
    tag = op->count;

    /* Move marker forward - swap op and marker */
    op->active_prev = marker.active_prev;
    if (op->active_prev)
      op->active_prev->active_next = op;
    else
      active_objects = op;
    marker.active_next = op->active_next;
    if (marker.active_next)
      marker.active_next->active_prev = &marker;
    marker.active_prev = op;
    op->active_next = &marker;

    /* Now process op */
    if (QUERY_FLAG (op, FLAG_FREED)) {
      LOG(llevBug, "BUG: process_events(): Free object on list\n");
      op->speed = 0;
      update_ob_speed (op);
      continue;
    }

	/*LOG(-1,"POBJ: %s (%s) s:%f sl:%f (%f)\n",query_name(op),op->arch->clone.name, op->speed,op->speed_left,op->arch->clone.speed_left);*/
    if ( ! op->speed)
	{
      LOG(llevBug, "BUG: process_events(): Object %s (%s, type:%d count:%d) has no speed, but is on active list\n", op->arch->name,query_name(op),op->type, op->count);
      update_ob_speed (op);
      continue;
    }


    if (op->map == NULL && op->env == NULL && op->name && op->type != MAP && map == NULL)
    {
	  if(op->type == PLAYER && op->contr->state != ST_PLAYING)
		  continue;
      LOG(llevBug, "BUG: process_events(): Object without map or inventory is on active list: %s (%d)\n",query_name(op), op->count);
      op->speed = 0;
      update_ob_speed (op);
      continue;
    }

    if (map != NULL && op->map != map)
      continue;


	/* as we can see here, the swing speed not effected by
	 * object speed BUT the swing hit itself!
	 * This will invoke a kind of delay of the ready swing
	 * until the monster can move again. Note, that a higher
	 * move speed as swing speed will not invoke a faster swing
	 * speed!
	 */
    if(op->weapon_speed_left>0) /* as long we are >0, we are not ready to swing */
        op->weapon_speed_left-=op->weapon_speed_add;

    if (op->speed_left > 0)
	{
        --op->speed_left;
        process_object (op);
        if (was_destroyed (op, tag))
            continue;
    }
	
/* Eneq(@csd.uu.se): Handle archetype-field anim_speed differently when
   it comes to the animation. If we have a value on this we don't animate it
   at speed-events. */
	if(QUERY_FLAG(op,FLAG_ANIMATE) )
	{
		if (op->last_anim >= op->anim_speed) 
		{
			animate_object (op,1);
			/* let reset move & fight anims */
			if (op->type == PLAYER && NUM_FACINGS(op)>=25) /* check for direction changing */
			{
				if(op->anim_moving_dir != -1)
				{
					op->anim_last_facing = op->anim_moving_dir;
					op->anim_moving_dir = -1;
				}
				if(op->anim_enemy_dir != -1)
				{
					op->anim_last_facing = op->anim_enemy_dir;
					op->anim_enemy_dir = -1;
				}
			}
			op->last_anim = 1;
		} 
		else 
		{
			if (NUM_FACINGS(op)>=25) /* check for direction changing */
				animate_object (op,0);     
			op->last_anim++;
		}
	}

#ifdef CASTING_TIME
    if (op->casting > 0)
      op->casting--;
#endif

    if (op->speed_left <= 0)
			op->speed_left += FABS (op->speed);
  }

  /* Remove marker object from active list */
  if (marker.active_prev != NULL)
    marker.active_prev->active_next = NULL;
  else
    active_objects = NULL;

  process_players2 (map);
}

void clean_tmp_files() {
  mapstruct *m, *next;

  LOG(llevInfo,"Cleaning up...\n");

  /* We save the maps - it may not be intuitive why, but if there are unique
   * items, we need to save the map so they get saved off.  Perhaps we should
   * just make a special function that only saves the unique items.
   */
  for(m=first_map;m!=NULL;m=next) {
    next=m->next;
    if (m->in_memory == MAP_IN_MEMORY)
/* If we want to reuse the temp maps, swap it out (note that will also
 * update the log file.  Otherwise, save the map (mostly for unique item
 * stuff).  Note that the clean_tmp_map is called after the end of
 * the for loop but is in the #else bracket.  IF we are recycling the maps,
 * we certainly don't want the temp maps removed.
 */
#ifdef RECYCLE_TMP_MAPS
	swap_map(m);
#else
	new_save_map(m, 0); /* note we save here into a overlay map */
    clean_tmp_map(m);
#endif
  }
  write_todclock(); /* lets just write the clock here */
}

/* clean up everything before exiting */
void cleanup()
{
    LOG(llevDebug,"Cleanup called.  freeing data.\n");
    clean_tmp_files();
    write_book_archive();
#ifdef MEMORY_DEBUG
    free_all_maps();
    free_style_maps();
    free_all_object_data();
    free_all_archs();
    free_all_treasures();
    free_all_images();
    free_all_newserver();
    free_all_recipes();
    free_all_readable();
    free_all_god();
    free_all_anim();
    /* See what the string data that is out there that hasn't been freed. */
    /*LOG(llevDebug, ss_dump_table(0xff));*/
#endif
    exit(0);
}

void leave(player *pl, int draw_exit) {
#ifdef PLUGINS
    int evtid;
    CFParm CFP;
#endif
    if (pl!=NULL) {
	/* We do this so that the socket handling routine can do the final
	 * cleanup.  We also leave that loop to actually handle the freeing
	 * of the data.
	 */
#ifdef PLUGINS
        if (draw_exit==0)
        {
            /* GROS : Here we handle the LOGOUT global event */
            evtid = EVENT_LOGOUT;
            CFP.Value[0] = (void *)(&evtid);
            CFP.Value[1] = (void *)(pl);
            CFP.Value[2] = (void *)(pl->socket.host);
            GlobalEvent(&CFP);
        };
#endif

  /* be sure we have closed container when we leave */
  container_unlink(pl,NULL);

    /* all player should be on friendly list - remove then */
	remove_friendly_object(pl->ob);
    pl->socket.status=Ns_Dead;
	LOG(llevInfo,"LOGOUT: >%s< from ip %s\n", pl->ob->name, pl->socket.host);

	if (pl->ob->map) {
	    if (pl->ob->map->in_memory==MAP_IN_MEMORY)
		pl->ob->map->timeout = MAP_TIMEOUT(pl->ob->map);
	    pl->ob->map=NULL;
	}
	pl->ob->type = DEAD_OBJECT; /* To avoid problems with inventory window */
    }
}

int forbid_play()
{
#if !defined(_IBMR2) && !defined(___IBMR2) && defined(PERM_FILE)
    char buf[MAX_BUF], day[MAX_BUF];
    FILE *fp;
    time_t clock;
    struct tm *tm;
    int i, start, stop, forbit=0, comp;

    clock = time (NULL);
    tm = (struct tm *) localtime (&clock);

    sprintf (buf, "%s/%s", settings.localdir, PERM_FILE);
    if ((fp = open_and_uncompress(buf, 0, &comp)) == NULL)
	return 0;

    while (fgets (buf, MAX_BUF, fp)) {
	if (buf[0]=='#') continue;
	if (!strncmp (buf, "msg", 3)) {
	    if (forbit)
		while (fgets (buf, MAX_BUF, fp))  /* print message */
		    fputs (buf, logfile);
	    break;

	} else if (sscanf (buf, "%s %d%*c%d\n", day, &start, &stop) != 3) {
	    LOG(llevDebug, "Warning: Incomplete line in permission file ignored.\n");
	    continue;
	}

	for (i=0; i< 7; i++) {
	    if (!strncmp (buf, days[i], 3) && (tm->tm_wday == i) && 
		(tm->tm_hour >= start) && (tm->tm_hour < stop))
		forbit = 1;
	}
    }

    close_and_delete(fp, comp);

    return forbit;
#else
    return 0;
#endif
}

#if 0
    static struct timeval new_time;
    long leftover_sec, leftover_usec;
    object *wp;

    wp = get_next_requested_path();
    while (wp) {
        waypoint_compute_path(wp);
        
        (void) GETTIMEOFDAY(&new_time);

        leftover_sec = last_time.tv_sec - new_time.tv_sec;
        leftover_usec = max_time - (new_time.tv_usec - last_time.tv_usec);

        /* This is very ugly, but probably the fastest for our use: */
        while (leftover_usec < 0)
        {
            leftover_usec += 1000000;
            leftover_sec -= 1;
        }
        while (leftover_usec > 1000000)
        {
            leftover_usec -= 1000000;
            leftover_sec +=1;
        }

        if (leftover_sec < 1 && leftover_usec < 10000)
            break;
    }
#endif 

void dequeue_path_requests()
{
#ifdef LEFTOVER_CPU_FOR_PATHFINDING
    static struct timeval new_time;
    long leftover_sec, leftover_usec;
    object *wp;

    while ((wp = get_next_requested_path())) {
        waypoint_compute_path(wp);
        
        /* TODO: only compute time if there is something more in the queue, something
         * like if(path_request_queue_empty()) break; */
        (void) GETTIMEOFDAY(&new_time);

        leftover_sec = last_time.tv_sec - new_time.tv_sec;
        leftover_usec = max_time - (new_time.tv_usec - last_time.tv_usec);

        /* This is very ugly, but probably the fastest for our use: */
        while (leftover_usec < 0)
        {
            leftover_usec += 1000000;
            leftover_sec -= 1;
        }
        while (leftover_usec > 1000000)
        {
            leftover_usec -= 1000000;
            leftover_sec +=1;
        }

        /* Try to save about 10 ms */
        if (leftover_sec < 1 && leftover_usec < 10000)
            break;
    }
#else
    object *wp = get_next_requested_path();
    if(wp) 
        waypoint_compute_path(wp);
#endif /* LEFTOVER_CPU_FOR_PATHFINDING */
}

/*
 *  do_specials() is a collection of functions to call from time to time.
 * Modified 2000-1-14 MSW to use the global pticks count to determine how
 * often to do things.  This will allow us to spred them out more often.
 * I use prime numbers for the factor count - in that way, it is less likely
 * these actions will fall on the same tick (compared to say using 500/2500/15000
 * which would mean on that 15,000 tick count a whole bunch of stuff gets
 * done).  Of course, there can still be times where multiple specials are
 * done on the same tick, but that will happen very infrequently
 *
 * I also think this code makes it easier to see how often we really are
 * doing the various things.
 */

/* Hm, i really must check this in the feature... here are some ugly 
 * hacks and workarounds hidden - MT2003
 */

void do_specials() {
    if(!(pticks % 2)) 
        dequeue_path_requests();

#ifdef WATCHDOG
    if (!(pticks % 503))
	watchdog();
#endif    
            if (!(pticks % 10))  /*use this for debuging */
/*    if (!(pticks % PTICKS_PER_CLOCK))*/
            tick_the_clock();

    if (!(pticks % 509))
	flush_old_maps();    /* Clears the tmp-files of maps which have reset */

	/*
    if (!(pticks % 2503))
	fix_weight();*/

    if (!(pticks % 2521))
	metaserver_update();    /* 2500 ticks is about 5 minutes */

    if (!(pticks % 5003))
	write_book_archive();
        
}

int main(int argc, char **argv)
{
#ifdef PLUGINS_X
  int evtid;
  CFParm CFP;
#endif
#ifdef WIN32 /* ---win32 this sets the win32 from 0d0a to 0a handling */
	_fmode = _O_BINARY ;
#endif

#ifdef DEBUG_MALLOC_LEVEL
  malloc_debug(DEBUG_MALLOC_LEVEL);
#endif

  settings.argc=argc;
  settings.argv=argv;
  init(argc, argv);
#ifdef PLUGINS
  initPlugins();        /* GROS - Init the Plugins */
#endif
  compile_info();       /* its not a bad idea to show at start whats up */    
  memset(&marker, 0, sizeof(struct obj)); /* used from proccess_events() */



  for(;;) 
  {
	nroferrors = 0;				/* every llevBug will increase this - avoid LOG loops */
    doeric_server();		
    global_round_tag++;			/* global round ticker ! this is real a global */
    process_events(NULL);		/* "do" something with objects with speed */
	doeric_server_write();
    cftimer_process_timers();	/* Process the crossfire Timers */    

#ifdef PLUGINS_X
								/* GROS : Here we handle the CLOCK global event */
    evtid = EVENT_CLOCK;
    CFP.Value[0] = (void *)(&evtid);
    GlobalEvent(&CFP);
#endif

    check_active_maps();		/* Removes unused maps after a certain timeout */
    do_specials();				/* Routines called from time to time. */
    sleep_delta();				/* Slepp proper amount of time before next tick */
  }
  return 0;
}
