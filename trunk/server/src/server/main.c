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

#ifdef HAVE_TIME_H
#include <time.h>
#endif

/* #include <Mmsystem.h> swing time debug */
#include <pathfinder.h>

#include <../random_maps/random_map.h>
#include <../random_maps/rproto.h>


#ifdef DEBUG_MEMPOOL_OBJECT_TRACKING
extern void check_use_object_list(void);
#endif
/* Prototypes of functions used only here. */
void        free_all_srv_files();
void        free_racelist();

/*
static char     days[7][4]  =
{
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};
*/
void version(object *op)
{
    new_draw_info_format(NDI_UNIQUE, 0, op, "This is Daimonin v%s", VERSION);

    /* If in a socket, don't print out the list of authors.  It confuses the
     * crossclient program.
     */
    if (op == NULL)
        return;
    new_draw_info(NDI_UNIQUE, 0, op, "Authors and contributors to Daimonin & Crossfire:");
    new_draw_info(NDI_UNIQUE, 0, op, "(incomplete list - mail us if you miss your name):");
    new_draw_info(NDI_UNIQUE, 0, op, "mark@scruz.net (Mark Wedel)");
    new_draw_info(NDI_UNIQUE, 0, op, "frankj@ifi.uio.no (Frank Tore Johansen)");
    new_draw_info(NDI_UNIQUE, 0, op, "kjetilho@ifi.uio.no (Kjetil Torgrim Homme)");
    new_draw_info(NDI_UNIQUE, 0, op, "tvangod@ecst.csuchico.edu (Tyler Van Gorder)");
    new_draw_info(NDI_UNIQUE, 0, op, "elmroth@cd.chalmers.se (Tony Elmroth)");
    new_draw_info(NDI_UNIQUE, 0, op, "dougal.scott@fcit.monasu.edu.au (Dougal Scott)");
    new_draw_info(NDI_UNIQUE, 0, op, "wchuang@athena.mit.edu (William)");
    new_draw_info(NDI_UNIQUE, 0, op, "ftww@cs.su.oz.au (Geoff Bailey)");
    new_draw_info(NDI_UNIQUE, 0, op, "jorgens@flipper.pvv.unit.no (Kjetil Wiekhorst Jxrgensen)");
    new_draw_info(NDI_UNIQUE, 0, op, "c.blackwood@rdt.monash.edu.au (Cameron Blackwood)");
    new_draw_info(NDI_UNIQUE, 0, op, "jtraub+@cmu.edu (Joseph L. Traub)");
    new_draw_info(NDI_UNIQUE, 0, op, "rgg@aaii.oz.au (Rupert G. Goldie)");
    new_draw_info(NDI_UNIQUE, 0, op, "eanders+@cmu.edu (Eric A. Anderson)");
    new_draw_info(NDI_UNIQUE, 0, op, "eneq@Prag.DoCS.UU.SE (Rickard Eneqvist)");
    new_draw_info(NDI_UNIQUE, 0, op, "Jarkko.Sonninen@lut.fi (Jarkko Sonninen)");
    new_draw_info(NDI_UNIQUE, 0, op, "kholland@sunlab.cit.cornell.du (Karl Holland)");
    new_draw_info(NDI_UNIQUE, 0, op, "vick@bern.docs.uu.se (Mikael Lundgren)");
    new_draw_info(NDI_UNIQUE, 0, op, "mol@meryl.csd.uu.se (Mikael Olsson)");
    new_draw_info(NDI_UNIQUE, 0, op, "Tero.Haatanen@lut.fi (Tero Haatanen)");
    new_draw_info(NDI_UNIQUE, 0, op, "ylitalo@student.docs.uu.se (Lasse Ylitalo)");
    new_draw_info(NDI_UNIQUE, 0, op, "anipa@guru.magic.fi (Niilo Neuvo)");
    new_draw_info(NDI_UNIQUE, 0, op, "mta@modeemi.cs.tut.fi (Markku J{rvinen)");
    new_draw_info(NDI_UNIQUE, 0, op, "meunier@inf.enst.fr (Sylvain Meunier)");
    new_draw_info(NDI_UNIQUE, 0, op, "jfosback@darmok.uoregon.edu (Jason Fosback)");
    new_draw_info(NDI_UNIQUE, 0, op, "cedman@capitalist.princeton.edu (Carl Edman)");
    new_draw_info(NDI_UNIQUE, 0, op, "henrich@crh.cl.msu.edu (Charles Henrich)");
    new_draw_info(NDI_UNIQUE, 0, op, "schmid@fb3-s7.math.tu-berlin.de (Gregor Schmid)");
    new_draw_info(NDI_UNIQUE, 0, op, "quinet@montefiore.ulg.ac.be (Raphael Quinet)");
    new_draw_info(NDI_UNIQUE, 0, op, "jam@modeemi.cs.tut.fi (Jari Vanhala)");
    new_draw_info(NDI_UNIQUE, 0, op, "kivinen@joker.cs.hut.fi (Tero Kivinen)");
    new_draw_info(NDI_UNIQUE, 0, op, "peterm@soda.berkeley.edu (Peter Mardahl)");
    new_draw_info(NDI_UNIQUE, 0, op, "matt@cs.odu.edu (Matthew Zeher)");
    new_draw_info(NDI_UNIQUE, 0, op, "srt@sun-dimas.aero.org (Scott R. Turner)");
    new_draw_info(NDI_UNIQUE, 0, op, "huma@netcom.com (Ben Fennema)");
    new_draw_info(NDI_UNIQUE, 0, op, "njw@cs.city.ac.uk (Nick Williams)");
    new_draw_info(NDI_UNIQUE, 0, op, "Wacren@Gin.ObsPM.Fr (Laurent Wacrenier)");
    new_draw_info(NDI_UNIQUE, 0, op, "thomas@astro.psu.edu (Brian Thomas)");
    new_draw_info(NDI_UNIQUE, 0, op, "jsm@axon.ksc.nasa.gov (John Steven Moerk)");
    new_draw_info(NDI_UNIQUE, 0, op, "Delbecq David       [david.delbecq@mailandnews.com]");
    new_draw_info(NDI_UNIQUE, 0, op, "Chachkoff Yann      [yann.chachkoff@mailandnews.com]\n");
    new_draw_info(NDI_UNIQUE, 0, op, "Images and art:");
    new_draw_info(NDI_UNIQUE, 0, op, "Peter Gardner");
    new_draw_info(NDI_UNIQUE, 0, op, "David Gervais       [david_eg@mail.com]");
    new_draw_info(NDI_UNIQUE, 0, op, "Mitsuhiro Itakura   [ita@gold.koma.jaeri.go.jp]");
    new_draw_info(NDI_UNIQUE, 0, op, "Hansjoerg Malthaner [hansjoerg.malthaner@danet.de]");
    new_draw_info(NDI_UNIQUE, 0, op, "Mårten Woxberg      [maxmc@telia.com]");
    new_draw_info(NDI_UNIQUE, 0, op, "The FRUA art community [http://uamirror.dns2go.com/]");
    new_draw_info(NDI_UNIQUE, 0, op, "future wave shaper(sounds) [http://www.futurewaveshaper.com/]");
    new_draw_info(NDI_UNIQUE, 0, op, "Zero Sum Software [http://www.zero-sum.com/]");
    new_draw_info(NDI_UNIQUE, 0, op, "Reiner Prokein [[reiner.prokein@t-online.de]]");
    new_draw_info(NDI_UNIQUE, 0, op, "Dungeon Craft Community [http://uaf.sourceforge.net/]");
    new_draw_info(NDI_UNIQUE, 0, op, "Marc [http://www.angelfire.com/dragon/kaltusara_dc/index.html]");
    new_draw_info(NDI_UNIQUE, 0, op, "Iron Works DC art [http://www.tgeweb.com/ironworks/dungeoncraft/index.shtml]");
    new_draw_info(NDI_UNIQUE, 0, op, "The mighty Dink.");
    new_draw_info(NDI_UNIQUE, 0, op, "And many more!");
}

void start_info(object *op)
{
    char    buf[MAX_BUF];

    sprintf(buf, "Welcome to Daimonin, v%s!", VERSION);
    new_draw_info(NDI_UNIQUE, 0, op, buf);
}

char * crypt_string(char *str, char *salt)
{
#ifndef WIN32 /* ***win32 crypt_string:: We don't need this anymore since server/client fork */
    static char    *c   = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";
    char            s[2];
    if (salt == NULL)
        s[0] = c[RANDOM() % (int) strlen(c)],
        s[1] = c[RANDOM() % (int) strlen(c)];
    else
        s[0] = salt[0],
        s[1] = salt[1];
#ifdef HAVE_LIBDES
    return (char *) des_crypt(str, s);
#else
    return (char *) crypt(str, s);
#endif
#endif /* win32 */
    return(str);
}

/* generate a unique instance map ID */
int get_new_instance_num(void)
{
    return ++global_instance_num;
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
    player *pl;

    /* Basically, we keep looping until all the players have done their actions. */
    for (pl = first_player; pl != NULL; pl = pl->next)
    {
        if (handle_newcs_player(pl) == -1) /* -1: player is invalid now */
        {
            continue;
        }

        /* we call do_some_living now in a interval of 1 sec.
         * That will save us some cpu time per player
         * to avoid one big tick event we use a player based timer which
         * will automatically balance the calls over the pticks timer
         */
        if(--pl->reg_timer <= 0)
            do_some_living(pl->ob);

#ifdef AUTOSAVE
        /* check for ST_PLAYING state so that we don't try to save off when
           * the player is logging in.
           */
        if ((pl->last_save_tick + AUTOSAVE) < ROUND_TAG && pl->state == ST_PLAYING)
        {
            /* we must change this unholy ground thing */
            if (blocks_cleric(pl->ob->map, pl->ob->x, pl->ob->y))
            {
                pl->last_save_tick += 100;
            }
            else
            {
                save_player(pl->ob, 1);
                pl->last_save_tick = ROUND_TAG;
            }
        }
#endif
    }
}

void process_players2(mapstruct *map)
{
    player *pl;

    for (pl = first_player; pl != NULL; pl = pl->next)
    {
        /* thats for debug spells - if enabled, mana/grace is always this value
            pl->ob->stats.grace = 900;
            pl->ob->stats.sp = 900;
            */

        /* look our target is still valid - if not, update client
             * we handle op->enemy for the player here too!
             */
        if (pl->ob->map
         && (!pl->target_object
          || (pl->target_object != pl->ob && pl->target_object_count != pl->target_object->count)
          || !OBJECT_ACTIVE(pl->target_object)
          || QUERY_FLAG(pl->target_object, FLAG_SYS_OBJECT) || pl->target_object->level != pl->target_level
          || (QUERY_FLAG(pl->target_object, FLAG_IS_INVISIBLE) && !QUERY_FLAG(pl->ob, FLAG_SEE_INVISIBLE))))
            send_target_command(pl);

        /* now use the new target system to hit our target... Don't hit non
        * friendly objects, ourself or when we are not in combat mode.
        */
        if (pl->ob->weapon_speed_left <= 0
            && pl->ob->map
            && pl->target_object
            && pl->combat_mode
            /*
            && OBJECT_ACTIVE(pl->target_object)
            && pl->target_object_count != pl->ob->count
            */
            && get_friendship(pl->ob, pl->target_object) < FRIENDSHIP_HELP)
        {
                /* now we force target as enemy */
                pl->ob->enemy = pl->target_object;
                pl->ob->enemy_count = pl->target_object_count;

                /* quick check our target is still valid: count ok? (freed...), not
                * removed, not a bet or object we self own (TODO: group pets!)
                * Note: i don't do a invisible check here... this will happen one
                * at end of this round... so, we have a "object turn invisible and
                * we do a last hit here"
                */
                if (!OBJECT_VALID(pl->ob->enemy, pl->ob->enemy_count) || pl->ob->enemy->owner == pl->ob)
                    pl->ob->enemy = NULL;
                else if (is_melee_range(pl->ob, pl->ob->enemy))
                {
                    /* tell our enemy we swing at him now */
                    update_npc_knowledge(pl->ob->enemy, pl->ob, FRIENDSHIP_TRY_ATTACK, 0);
                    pl->rest_mode = 0;
                    skill_attack(pl->ob->enemy, pl->ob, 0, NULL);
                    pl->ob->weapon_speed_left += FABS(pl->ob->weapon_speed);
                }
        }

        if (pl->ob->speed_left > pl->ob->speed)
            pl->ob->speed_left = pl->ob->speed;
    }
}

static void process_map_events(mapstruct *map)
{
    object *op, *first_obj;
    tag_t   tag;

    /* Find first object in activelist (skip sentinel) */
    if(map == NULL)
        first_obj = active_objects->active_next;
    else
        first_obj = map->active_objects->active_next;

    /* note: next_active_object is a global which
       might get modified while traversing below */
    for (op = first_obj; op != NULL; op = next_active_object)
    {
        next_active_object = op->active_next;
        tag = op->count;

        /* Now process op */
        if (OBJECT_FREE(op))
        {
            LOG(llevBug, "BUG: process_events(): Free object %s (count: %d (%d)) on active list for %s (%d,%d) flag:%d\n",
                            STRING_OBJ_NAME(op), op->count, op->count_debug, STRING_MAP_PATH(op->map), op->x, op->y, QUERY_FLAG(op, FLAG_IN_ACTIVELIST));
            SET_FLAG(op, FLAG_IN_ACTIVELIST);
            activelist_remove(op);
            continue;
        }

        /* Gecko: This is not really a bug, but it has to be thought trough...
         * If an active object is remove_ob():ed and not reinserted during process_events(),
         * this might happen. It normally means that the object was killed, but you never know...
         */
        if (QUERY_FLAG(op, FLAG_REMOVED))
        {
            /*
                LOG(llevDebug, "PROBLEM: process_events(): Removed object on active list  %s (%s, type:%d count:%d)\n",
                                      op->arch->name,query_name(op),op->type, op->count);
                */
            op->speed = 0;
            update_ob_speed(op);
            continue;
        }

        /*LOG(-1,"POBJ: %s (%s) s:%f sl:%f (%f)\n",query_name(op),op->arch->clone.name, op->speed,op->speed_left,op->arch->clone.speed_left);*/
        if (!op->speed)
        {
            LOG(llevBug,
                "BUG: process_events(): Object %s (%s, type:%d count:%d) has no speed, but is on active list\n",
                op->arch->name, query_name(op), op->type, op->count);
            update_ob_speed(op);
            continue;
        }


        if (op->map == NULL && op->env == NULL)
        {
            if (op->type == PLAYER && CONTR(op)->state != ST_PLAYING)
                continue;
            LOG(llevBug, "BUG: process_events(): Object without map or inventory is on active list: %s (%d)\n",
                query_name(op), op->count);
            op->speed = 0;
            update_ob_speed(op);
            continue;
        }

        /* This is not a bug. We just move the object to the
         * insertion list and sort it out later. Process as usual. */
        if (op->map != map)
        {
            LOG(llevDebug, "WARNING: process_events(): object not on processed map: %s is on %s, not %s\n",
                query_name(op), STRING_MAP_PATH(op->map), STRING_MAP_PATH(map));
            activelist_remove(op);
            activelist_insert(op);
        }

        /* only players and monsters should have weapon_speed set and be active */
        if (op->weapon_speed)
        {
            if (op->weapon_speed_left > 0)
                op->weapon_speed_left -= WEAPON_SWING_TIME;

            /* PLAYER swings - monster swing will be done implicit in process_object() */
        }

        if (op->speed_left > 0)
        {
            --op->speed_left;
            process_object(op);
            if (was_destroyed(op, tag))
                continue;
        }
        else
        {
            if (op->weapon_speed_left <= 0)
            {
                if (QUERY_FLAG(op, FLAG_MONSTER))
                    move_monster(op, FALSE); /* false = only weapon action */
            }
        }



        /* Eneq(@csd.uu.se): Handle archetype-field anim_speed differently when
           it comes to the animation. If we have a value on this we don't animate it
           at speed-events. */
        if (QUERY_FLAG(op, FLAG_ANIMATE))
        {
            if (op->last_anim >= op->anim_speed)
            {
                animate_object(op, 1);
                /* let reset move & fight anims */
                if (op->type == PLAYER && NUM_FACINGS(op) >= 25) /* check for direction changing */
                {
                    if (op->anim_moving_dir != -1)
                    {
                        op->anim_last_facing = op->anim_moving_dir;
                        op->anim_moving_dir = -1;
                    }
                    if (op->anim_enemy_dir != -1)
                    {
                        op->anim_last_facing = op->anim_enemy_dir;
                        op->anim_enemy_dir = -1;
                    }
                }
                op->last_anim = 1;
            }
            else
            {
                if (NUM_FACINGS(op) >= 25) /* check for direction changing */
                    animate_object(op, 0);
                op->last_anim++;
            }
        }

        if (op->speed_left <= 0)
            op->speed_left += FABS(op->speed);
    }
    next_active_object = NULL;
}

void process_events()
{
    mapstruct *map;
#if defined TIME_PROCESS_EVENTS
    /* This is instrumentation code that shows how the speedup to this function
     * was measured. It can easily be moved to the old version too, but if you
     * want to run it you also need the inlined function "add_time" from common/time.c */
    /* Note: I'll remove these blocks soon. Gecko - 20050522 */
    static int callcount = 0;
    static struct timeval cumulative;
    struct timeval start, end;
    double t;

    gettimeofday(&start, NULL);
#endif

    process_players1(NULL);
    /* Preprocess step: move all objects in inserted_active_objects
     * into their real activelists */
    /* TODO: to make AI more efficient, keep activelist sorted with
     * mobs&players first and less interesting objects next, or
     * have two activelists per map for that */
    if(inserted_active_objects) {
        object *obj, *next;
        for(obj = inserted_active_objects; obj; obj = next)
        {
            next = obj->active_next;

            /* sanity check!
             * we must ensure that a inventory object has ->map NULL.
             * Reason: If the ->env object change map the iventory map
             * ptr will get invalid and cause havoc when that map get
             * destroyed in the meantime (invalid map ptr to freed memory)
             * (we don't use the object count / removed flag / memory pool
             * system here!)
             *
             * TODO: To set ->map for ->env objects here to NULL is NOT enough!
             * We must ensure that ->map is NULL when an inventory object is handled
             * by the active list. Or we can run in sideeffect too.
             */
            if(obj->env && obj->map) /* object is in inventory! */
            {
                LOG( llevDebug, "ACTIVEBUG: object with env and map - set map to NULL! obj %s in %s\n",
                     query_name(obj), query_name(obj->env) );
                obj->map = NULL;
            }
            if(obj->map)
            {
                /* In case the map was swapped out with an object in the insertion list
                 * (This probably doesn't happen, since all objects are removed when the map
                 * is swapped out) */
                if(obj->map->in_memory != MAP_IN_MEMORY)
                {
                    /* FIXME: for now we'll just see if this happens */
                    LOG( llevDebug, "ACTIVEBUG: object on map not in memory! obj %s in %s\n",
                            query_name(obj), STRING_MAP_PATH(obj->map) );
                }
                /* Always insert after the sentinel */
                obj->active_next = obj->map->active_objects->active_next;
                obj->map->active_objects->active_next = obj;
                obj->active_prev = obj->map->active_objects;
            }
            else
            {
                /* Always insert after the sentinel */
                obj->active_next = active_objects->active_next;
                active_objects->active_next = obj;
                obj->active_prev = active_objects;
            }
            if(obj->active_next)
                obj->active_next->active_prev = obj;
        }
        inserted_active_objects = NULL;
    }

    /* Now, process all map-based activelists, and the activelist
     * for objects not on maps */
    process_map_events(NULL);

    /* TODO: only go through maps in special list of maps with active objects */
    for (map = first_map; map; map = map->next)
    {
        if (map->active_objects->active_next && map->in_memory == MAP_IN_MEMORY)
            process_map_events(map);
    }

    process_players2(NULL);

#if defined TIME_PROCESS_EVENTS
    gettimeofday(&end, NULL);
    start.tv_sec = -start.tv_sec;
    start.tv_usec = -start.tv_usec;
    add_time(&cumulative, &start, &cumulative);
    add_time(&cumulative, &end, &cumulative);
    if((++callcount) % 100 == 0) {
        t = (double)(cumulative.tv_sec + cumulative.tv_usec / 1000000.0f) / callcount;
        LOG(llevDebug, "%d calls, %d.%06d s (%f s/call)\n", callcount, cumulative.tv_sec, cumulative.tv_usec, t);
        callcount = 0;
        cumulative.tv_usec = 0;
        cumulative.tv_sec = 0;
    }
#endif /* TIME PROCESS EVENTS */
}

void clean_tmp_files(int flag)
{
    mapstruct  *m, *next;

    LOG(llevInfo, "Save maps and cleaning up...\n");

    /* We save the maps - it may not be intuitive why, but if there are unique
     * items, we need to save the map so they get saved off.  Perhaps we should
     * just make a special function that only saves the unique items.
     */
    for (m = first_map; m != NULL; m = next)
    {
        next = m->next;
        if (m->in_memory == MAP_IN_MEMORY)
        /* If we want to reuse the temp maps, swap it out (note that will also
         * update the log file.  Otherwise, save the map (mostly for unique item
         * stuff).  Note that the clean_tmp_map is called after the end of
         * the for loop but is in the #else bracket.  IF we are recycling the maps,
         * we certainly don't want the temp maps removed.
         */
        if(flag)
        {

#ifdef RECYCLE_TMP_MAPS
            swap_map(m, 0);
#else
            new_save_map(m, 0);
        }
        clean_tmp_map(m);
#endif
    }
}

void cleanup_without_exit()
{
    LOG(llevDebug, "Cleanup called.  freeing data.\n");
    clean_tmp_files(TRUE);
    write_book_archive();
    write_todclock();   /* lets just write the clock here */
    save_ban_file();

    /* that must be redone: clear cleanup so we know 100% all memory is freed */
    free_all_maps();
    free_style_maps();
    free_all_object_data();
    free_all_archs();
    free_all_treasures();
    free_all_images();
    free_all_newserver();
#ifdef SERVER_SEND_FACES
	free_socket_images();
#endif
    free_all_recipes();
    free_all_readable();
    free_all_god();
    free_all_anim();
    free_strings();
    free_lists_and_tables();
    cleanup_all_behavioursets();
    free_gmaster_list();
    cleanup_mempools();

    /* free_all_srv_files(); */
}

/* clean up everything before exiting */
void cleanup(int ret)
{
    cleanup_without_exit();
    exit(ret);
}

void leave(player *pl, int draw_exit)
{
    if (pl != NULL)
    {
        /* We do this so that the socket handling routine can do the final
         * cleanup.  We also leave that loop to actually handle the freeing
         * of the data.
         */

#ifdef PLUGINS
        int     evtid;
        CFParm  CFP;
        evtid = EVENT_LOGOUT;
        CFP.Value[0] = (void *) (&evtid);
        CFP.Value[1] = (void *) (pl->ob);
        CFP.Value[2] = CFP.Value[3] = CFP.Value[4] = NULL;
        CFP.Value[5] = (void *) (&draw_exit);
        CFP.Value[6] = CFP.Value[7] = CFP.Value[8] =
            CFP.Value[9] = CFP.Value[10] = NULL;
        CFP.Value[11] = (void *) (pl);
        GlobalEvent(&CFP);
#endif

        /* be sure we have closed container when we leave */
        container_unlink(pl, NULL);

        pl->socket.status = Ns_Dead;
        LOG(llevInfo, "LOGOUT: >%s< from ip %s\n", query_name(pl->ob), pl->socket.ip_host);

        if (pl->ob->map)
        {
            if (pl->ob->map->in_memory == MAP_IN_MEMORY)
                pl->ob->map->timeout = MAP_TIMEOUT(pl->ob->map);
            pl->ob->map = NULL;
        }
        pl->ob->type = DEAD_OBJECT; /* To avoid problems with inventory window */
    }
}


void dequeue_path_requests()
{
#ifdef LEFTOVER_CPU_FOR_PATHFINDING
    static struct timeval   new_time;
    long                    leftover_sec, leftover_usec;
    object                 *op;

    while ((op = get_next_requested_path()))
    {
        object_accept_path(op);

        /* TODO: only compute time if there is something more in the queue, something
         * like if(path_request_queue_empty()) break; */
        (void) GETTIMEOFDAY(&new_time);

        leftover_sec = last_time.tv_sec - new_time.tv_sec;
        leftover_usec = pticks_ums - (new_time.tv_usec - last_time.tv_usec);

        /* This is very ugly, but probably the fastest for our use: */
        while (leftover_usec < 0)
        {
            leftover_usec += 1000000;
            leftover_sec -= 1;
        }
        while (leftover_usec > 1000000)
        {
            leftover_usec -= 1000000;
            leftover_sec += 1;
        }

        /* Try to save about 10 ms */
        if (leftover_sec < 1 && leftover_usec < 10000)
            break;
    }
#else
    object *op                  = get_next_requested_path();
    extern void             object_accept_path  (object *op);
    if (wp)
        object_accept_path(op);
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

void do_specials()
{
    if (!(ROUND_TAG % 2))
        dequeue_path_requests();

    /*   if (!(ROUND_TAG % 20)) */ /*use this for debuging */
    if (!(ROUND_TAG % PTICKS_PER_CLOCK))
        tick_the_clock();

    if (!(ROUND_TAG % 509))
        flush_old_maps();    /* Clears the tmp-files of maps which have reset */

    if (!(ROUND_TAG % 2521))
        metaserver_update();    /* 2500 ticks is about 5 minutes */
}


/* the shutdown agent is a automatic timer
 * which shutdown the server after the given time.
 * It gives out messages to all player to announce
 * the shutdown and the status of the shutdown.
 */
void shutdown_agent(int timer, int ret, char *reason)
{
    static int sd_timer = -1, m_count, real_count = -1, ret_signal = EXIT_NORMAL;
    static struct timeval   tv1, tv2;

    if (timer == -1 && sd_timer == -1)
    {
        if (real_count > 0)
        {
            if (--real_count <= 0)
            {
                LOG(llevSystem, "SERVER SHUTDOWN STARTED\n");
                command_kick(NULL, NULL);
                cleanup(ret_signal);
            }
        }
        return; /* nothing to do */
    }

    if (timer != -1) /* reset shutdown count */
    {
        int t_min   = timer / 60;
        int t_sec   = timer - (int) (timer / 60) * 60;
        sd_timer = timer;

        new_draw_info(NDI_PLAYER | NDI_UNIQUE | NDI_ALL | NDI_GREEN, 5, NULL, "[Server]: ** SERVER SHUTDOWN STARTED **");
        ret_signal = ret;
        if (reason)
            new_draw_info_format(NDI_PLAYER | NDI_UNIQUE | NDI_ALL | NDI_GREEN, 5, NULL, "[Server]: %s", reason);

        if (t_sec)
            new_draw_info_format(NDI_PLAYER | NDI_UNIQUE | NDI_ALL | NDI_GREEN, 5, NULL,
            "[Server]: SERVER REBOOT in %d minutes and %d seconds", t_min, t_sec);
        else
            new_draw_info_format(NDI_PLAYER | NDI_UNIQUE | NDI_ALL | NDI_GREEN, 5, NULL,
            "[Server]: SERVER REBOOT in %d minutes", t_min);
        GETTIMEOFDAY(&tv1);
        m_count = timer / 60 - 1;
        real_count = -1;
    }
    else /* count the shutdown tango */
    {
        int t_min;
        int t_sec   = 0;
        GETTIMEOFDAY(&tv2);

        if ((int) (tv2.tv_sec - tv1.tv_sec) >= sd_timer) /* end countdown */
        {
            new_draw_info(NDI_PLAYER | NDI_UNIQUE | NDI_ALL | NDI_GREEN, 5, NULL,
                "[Server]: ** SERVER GOES DOWN NOW!!! **");
            if (reason)
                new_draw_info_format(NDI_PLAYER | NDI_UNIQUE | NDI_ALL | NDI_GREEN, 5, NULL, "[Server]: %s", reason);
            sd_timer = -1;
            real_count = 30;
        }

        t_min = (sd_timer - (int) (tv2.tv_sec - tv1.tv_sec)) / 60;
        t_sec = (sd_timer - (int) (tv2.tv_sec - tv1.tv_sec))
            - (int) ((sd_timer - (int) (tv2.tv_sec - tv1.tv_sec)) / 60) * 60;

        /*LOG(-1,"SEC: %d (%d - %d)\n", tv2.tv_sec-tv1.tv_sec,t_min,t_sec);*/
        if ((t_min == m_count && !t_sec))
        {
            m_count = t_min - 1;
            if (t_sec)
                new_draw_info_format(NDI_PLAYER | NDI_UNIQUE | NDI_ALL | NDI_GREEN, 5, NULL,
                "[Server]: SERVER REBOOT in %d minutes and %d seconds", t_min, t_sec);
            else
                new_draw_info_format(NDI_PLAYER | NDI_UNIQUE | NDI_ALL | NDI_GREEN, 5, NULL,
                "[Server]: SERVER REBOOT in %d minutes", t_min);
        }
    }
}

/* we traverse through the *all* saved player files in /data
* and collect statistic data from them.
* DEBUG & DEVELOPLEMT only.
* We use this for player transformation for version changes but
* this is also useful for statistic analyzes.
* Why is it here and not in a extern script?
* Simple reason: fix_player() and the loader will
* alter the saved base data of a player alot and rebuilding
* that external to get the real ingame stats is a source of possible bugs.
*/
#ifdef DEBUG_TRAVERSE_PLAYER_DIR
static void traverse_player_stats(char* start_dir)
{
    DIR* dir;                         /* pointer to the scanned directory. */
    struct dirent* entry=NULL;        /* pointer to one directory entry.   */
    char *fptr, cwd[HUGE_BUF+1];      /* current working directory.        */
    static char base_cwd[HUGE_BUF+1]; /* base (start) directory        */
    struct stat dir_stat;             /* used by stat().                   */

    /* first, save path of current working directory */
    if (!getcwd(cwd, HUGE_BUF+1)) {
        perror("getcwd:");
        return;
    }

    /* open the directory for reading */
    if(start_dir)
    {
        strcpy(base_cwd, cwd);
        dir = opendir(start_dir);
        chdir(start_dir);
    }
    else
        dir = opendir(".");

    if (!dir) {
        fprintf(stderr, "Cannot read directory '%s': ", cwd);
        perror("");
        return;
    }

    /* scan the directory, traversing each sub-directory, and */
    /* matching the pattern for each file name.               */
    while ((entry = readdir(dir)))
    {
        /* check if the given entry is a directory. */
        /* skip all ".*" entries, to avoid loops and forbidden directories. */
        if (entry->d_name[0] == '.')
            continue;

        if (stat(entry->d_name, &dir_stat) == -1)
        {
            perror("stat:");
            continue;
        }

        /* is this a directory? */
        if (S_ISDIR(dir_stat.st_mode))
        {
            /* Change into the new directory */
            if (chdir(entry->d_name) == -1)
            {
                fprintf(stderr, "Cannot chdir into '%s': ", entry->d_name);
                perror("");
                continue;
            }
            /* check this directory */
            traverse_player_stats(NULL);

            /* finally, restore the original working directory. */
            if (chdir("..") == -1)
            {
                fprintf(stderr, "Cannot chdir back to '%s': ", cwd);
                perror("");
            }
        }
        else
        {
            /* lets check its a valid, local artifacts file */
            if(entry->d_name[0] != '.' && (fptr = strrchr(entry->d_name, '.')) && !strcmp(fptr, ".pl") )
            {
                player *pl = NULL;

                LOG(llevDebug, "found player %s...\n", entry->d_name);

                if(!(pl = get_player(NULL)))
                    LOG(llevDebug, "Error: loading player %s...\n", entry->d_name);
                else
                {
                    pl->socket.status = ST_SOCKET_NO;
                    entry->d_name[fptr-entry->d_name] = 0;
                    FREE_AND_COPY_HASH(pl->ob->name, entry->d_name);
                    entry->d_name[fptr-entry->d_name] = '.';
                    chdir(base_cwd);
                    check_login(pl->ob, FALSE);
                    /* player is now loaded, do something with it - after it, release player & object */

                    /* ........... */

                    /* lets remove the player and its objects and enviroment */
                    free_player(pl);
                    chdir(cwd);
                }
            }
        }
    }

    closedir(dir);

    if(start_dir) /* clean restore */
        chdir(cwd);
}
#endif

void iterate_main_loop()
{
    struct timeval timeout;

#ifdef PLUGINS
    int     evtid;
    CFParm  CFP;
#endif

    nroferrors = 0;                 /* every llevBug will increase this counter - avoid LOG loops */
    pticks++;                       /* ROUND_TAG ! this is THE global tick counter . Use ROUND_TAG in the code */

    shutdown_agent(-1, EXIT_NORMAL, NULL);       /* check & run a shutdown count (with messages & shutdown ) */

#ifdef DEBUG_MEMPOOL_OBJECT_TRACKING
    check_use_object_list();
#endif

    process_events(); /* "do" something with objects with speed - process user cmds */

    /* this is the tricky thing...  This full read/write access to the
     * socket ensure at last *ONE* read/write access to the socket in one round WITH player update.
     * It seems odd to do the read after process_events() but sleep_delta() will poll on the socket.
     * In that way, we will collect & process the commands as fast as possible.
     * NOTE: sleep_delta() *can* loop some doeric_server() - but don't MUST.
     * Perhaps we have alot players connected and process_events() and this doeric_server()
     * has eaten up all tick time!!
     */
    timeout.tv_sec=0;
    timeout.tv_usec=0;
    doeric_server(SOCKET_UPDATE_PLAYER|SOCKET_UPDATE_CLIENT, &timeout);

#ifdef PLUGINS
    /* This is where detached lua scripts get reactivated */
    /* GROS : Here we handle the CLOCK global event */
    evtid = EVENT_CLOCK;
    CFP.Value[0] = (void *) (&evtid);
    GlobalEvent(&CFP);
#endif

    check_active_maps();        /* Removes unused maps after a certain timeout */
    do_specials();              /* Routines called from time to time. */

    /*doeric_server_write();*/
    object_gc();                /* Clean up the object pool */
    sleep_delta();              /* Sleep proper amount of time before next tick but poll the socket */
}

#define AUTO_MSG_COUNTER (8*60*30) /* all 30 minutes */
int main(int argc, char **argv)
{
#ifdef AUTO_MSG
    int auto_msg_count = 8*60*3; /* kick the first message 3 minutes after server start */
#endif

    global_exit_return = EXIT_ERROR; /* return -1 will signal that we have a problem in the startup */

#ifdef WIN32 /* ---win32 this sets the win32 from 0d0a to 0a handling */
    _fmode = _O_BINARY ;
#endif

    settings.argc = argc;
    settings.argv = argv;
    init(argc, argv);
#ifdef PLUGINS
    initPlugins();        /* GROS - Init the Plugins */
    atexit(removePlugins);
#endif
    compile_info();       /* its not a bad idea to show at start whats up */

    STATS_EVENT(STATS_EVENT_STARTUP);
    reset_sleep(); /* init our last_time = start time - and lets go! */

#ifdef DEBUG_TRAVERSE_PLAYER_DIR
    traverse_player_stats("./data/players");
#endif
    global_exit_return = EXIT_NORMAL;

    LOG(llevInfo, "Server ready.\nWaiting for connections...\n");
    for (; ;)
    {
        /* TODO: move into iterate_main_loop() */
#ifdef AUTO_MSG
        if(!(auto_msg_count--))            /* trigger the auto message system */
        {
            /* TODO: add here a real customizable auto message system.
             * ATM we only use it to spam hard coded messages
             */
            auto_msg_count = AUTO_MSG_COUNTER;
            new_draw_info(NDI_PLAYER | NDI_UNIQUE | NDI_ALL | NDI_GREEN, 5, NULL,
                    "[INFO]: Please HELP US and VOTE for Daimonin DAILY!\nGo to www.daimonin.net and hit the VOTE ICONS!\nThanks and happy playing!! - Michtoen");
        }
#endif

        iterate_main_loop();
    }

    return 0;
}

