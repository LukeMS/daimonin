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

#if 0
/* This will be moved to a help category (/help version) and thus eventually
 * handled client-side. Sending a huge mass of NDIs from server to client like
 * this is just a waste.
 * -- Smacky 20090613 */
void version(object_t *op)
{
    /* If in a socket, don't print out the list of authors.  It confuses the
     * crossclient program.
     */
    if (op == NULL)
        return;
    ndi(NDI_UNIQUE, 0, op, "Authors and contributors to Daimonin & Crossfire:");
    ndi(NDI_UNIQUE, 0, op, "(incomplete list - mail us if you miss your name):");
    ndi(NDI_UNIQUE, 0, op, "mark@scruz.net (Mark Wedel)");
    ndi(NDI_UNIQUE, 0, op, "frankj@ifi.uio.no (Frank Tore Johansen)");
    ndi(NDI_UNIQUE, 0, op, "kjetilho@ifi.uio.no (Kjetil Torgrim Homme)");
    ndi(NDI_UNIQUE, 0, op, "tvangod@ecst.csuchico.edu (Tyler Van Gorder)");
    ndi(NDI_UNIQUE, 0, op, "elmroth@cd.chalmers.se (Tony Elmroth)");
    ndi(NDI_UNIQUE, 0, op, "dougal.scott@fcit.monasu.edu.au (Dougal Scott)");
    ndi(NDI_UNIQUE, 0, op, "wchuang@athena.mit.edu (William)");
    ndi(NDI_UNIQUE, 0, op, "ftww@cs.su.oz.au (Geoff Bailey)");
    ndi(NDI_UNIQUE, 0, op, "jorgens@flipper.pvv.unit.no (Kjetil Wiekhorst Jxrgensen)");
    ndi(NDI_UNIQUE, 0, op, "c.blackwood@rdt.monash.edu.au (Cameron Blackwood)");
    ndi(NDI_UNIQUE, 0, op, "jtraub+@cmu.edu (Joseph L. Traub)");
    ndi(NDI_UNIQUE, 0, op, "rgg@aaii.oz.au (Rupert G. Goldie)");
    ndi(NDI_UNIQUE, 0, op, "eanders+@cmu.edu (Eric A. Anderson)");
    ndi(NDI_UNIQUE, 0, op, "eneq@Prag.DoCS.UU.SE (Rickard Eneqvist)");
    ndi(NDI_UNIQUE, 0, op, "Jarkko.Sonninen@lut.fi (Jarkko Sonninen)");
    ndi(NDI_UNIQUE, 0, op, "kholland@sunlab.cit.cornell.du (Karl Holland)");
    ndi(NDI_UNIQUE, 0, op, "vick@bern.docs.uu.se (Mikael Lundgren)");
    ndi(NDI_UNIQUE, 0, op, "mol@meryl.csd.uu.se (Mikael Olsson)");
    ndi(NDI_UNIQUE, 0, op, "Tero.Haatanen@lut.fi (Tero Haatanen)");
    ndi(NDI_UNIQUE, 0, op, "ylitalo@student.docs.uu.se (Lasse Ylitalo)");
    ndi(NDI_UNIQUE, 0, op, "anipa@guru.magic.fi (Niilo Neuvo)");
    ndi(NDI_UNIQUE, 0, op, "mta@modeemi.cs.tut.fi (Markku J{rvinen)");
    ndi(NDI_UNIQUE, 0, op, "meunier@inf.enst.fr (Sylvain Meunier)");
    ndi(NDI_UNIQUE, 0, op, "jfosback@darmok.uoregon.edu (Jason Fosback)");
    ndi(NDI_UNIQUE, 0, op, "cedman@capitalist.princeton.edu (Carl Edman)");
    ndi(NDI_UNIQUE, 0, op, "henrich@crh.cl.msu.edu (Charles Henrich)");
    ndi(NDI_UNIQUE, 0, op, "schmid@fb3-s7.math.tu-berlin.de (Gregor Schmid)");
    ndi(NDI_UNIQUE, 0, op, "quinet@montefiore.ulg.ac.be (Raphael Quinet)");
    ndi(NDI_UNIQUE, 0, op, "jam@modeemi.cs.tut.fi (Jari Vanhala)");
    ndi(NDI_UNIQUE, 0, op, "kivinen@joker.cs.hut.fi (Tero Kivinen)");
    ndi(NDI_UNIQUE, 0, op, "peterm@soda.berkeley.edu (Peter Mardahl)");
    ndi(NDI_UNIQUE, 0, op, "matt@cs.odu.edu (Matthew Zeher)");
    ndi(NDI_UNIQUE, 0, op, "srt@sun-dimas.aero.org (Scott R. Turner)");
    ndi(NDI_UNIQUE, 0, op, "huma@netcom.com (Ben Fennema)");
    ndi(NDI_UNIQUE, 0, op, "njw@cs.city.ac.uk (Nick Williams)");
    ndi(NDI_UNIQUE, 0, op, "Wacren@Gin.ObsPM.Fr (Laurent Wacrenier)");
    ndi(NDI_UNIQUE, 0, op, "thomas@astro.psu.edu (Brian Thomas)");
    ndi(NDI_UNIQUE, 0, op, "jsm@axon.ksc.nasa.gov (John Steven Moerk)");
    ndi(NDI_UNIQUE, 0, op, "Delbecq David       [david.delbecq@mailandnews.com]");
    ndi(NDI_UNIQUE, 0, op, "Chachkoff Yann      [yann.chachkoff@mailandnews.com]\n");
    ndi(NDI_UNIQUE, 0, op, "Images and art:");
    ndi(NDI_UNIQUE, 0, op, "Peter Gardner");
    ndi(NDI_UNIQUE, 0, op, "David Gervais       [david_eg@mail.com]");
    ndi(NDI_UNIQUE, 0, op, "Mitsuhiro Itakura   [ita@gold.koma.jaeri.go.jp]");
    ndi(NDI_UNIQUE, 0, op, "Hansjoerg Malthaner [hansjoerg.malthaner@danet.de]");
    ndi(NDI_UNIQUE, 0, op, "Mårten Woxberg      [maxmc@telia.com]");
    ndi(NDI_UNIQUE, 0, op, "The FRUA art community [http://uamirror.dns2go.com/]");
    ndi(NDI_UNIQUE, 0, op, "future wave shaper(sounds) [http://www.futurewaveshaper.com/]");
    ndi(NDI_UNIQUE, 0, op, "Zero Sum Software [http://www.zero-sum.com/]");
    ndi(NDI_UNIQUE, 0, op, "Reiner Prokein [[reiner.prokein@t-online.de]]");
    ndi(NDI_UNIQUE, 0, op, "Dungeon Craft Community [http://uaf.sourceforge.net/]");
    ndi(NDI_UNIQUE, 0, op, "Marc [http://www.angelfire.com/dragon/kaltusara_dc/index.html]");
    ndi(NDI_UNIQUE, 0, op, "Iron Works DC art [http://www.tgeweb.com/ironworks/dungeoncraft/index.shtml]");
    ndi(NDI_UNIQUE, 0, op, "The mighty Dink.");
    ndi(NDI_UNIQUE, 0, op, "And many more!");
}
#endif

/* here we hook in to crypt the password - this feature is disabled atm */
char * crypt_string(char *str)
{
    return str;
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

void process_players1(map_t *map)
{
    player_t *pl;

    /* Basically, we keep looping until all the players have done their actions. */
    for (pl = first_player; pl; pl = pl->next)
    {
        object_t *op  = pl->ob;

        if (pl->socket.status != Ns_Playing ||
            !op ||
            !OBJECT_ACTIVE(op))
        {
            continue;
        }

        process_command_queue(&pl->socket, pl);

        if (pl->socket.status != Ns_Playing ||
            !op ||
            !OBJECT_ACTIVE(op))
        {
            continue;
        }

        /* we call do_some_living now in a interval of 1 sec.
         * That will save us some cpu time per player
         * to avoid one big tick event we use a player based timer which
         * will automatically balance the calls over the pticks timer */
        if (--pl->reg_timer <= 0)
        {
            do_some_living(op);
        }

        if ((pl->last_save_tick + AUTOSAVE) < ROUND_TAG &&
            (pl->state & ST_PLAYING)) // don't save when player is logging in
        {
            PLAYER_SAVE(pl);
        }
    }
}

void process_players2(map_t *map)
{
    player_t *pl;

    for (pl = first_player; pl != NULL; pl = pl->next)
    {
        object_t *who = pl->ob;

        if(pl->socket.status != Ns_Playing)
            continue;

        /* thats for debug spells - if enabled, mana/grace is always this value
            who->stats.grace = 900;
            who->stats.sp = 900;
            */

        /* Check that our target is still valid  -- if not, update client. When
         * mode is SELF, assume that all is well. */
        /* NOTE: In currrent practical terms this is a rather ridiculous
         * concern, because the times are so small. But if Dai gets many (100s)
         * more players in future and/or the tick time is reduced it may be
         * less daft.
         *
         * On my machine the old code took, regardless of mode and assuming the
         * target was valid, 6-7 ums. This code takes 2-4 ums when mode == SELF
         * or 8-17 ums (usually 10-12) otherwise -- but see below for how to
         * reduce this.
         *
         * -- Smacky 20150919 */
        if (pl->target_mode != LOS_TARGET_SELF)
        {
           object_t *target = LOS_VALIDATE_TARGET(pl, pl->target_ob, pl->target_tag);

            /* No target (probably means it was a mob which has recently been
             * killed or otherwise removed)? Target self. */
            if (!target)
            {
                LOS_SET_TARGET(pl, who, LOS_TARGET_SELF, 0);
                pl->update_target = 1;
            }
            /* target_level differs from the target's actual level? Well,
             * recalculate and update. */
            else if (pl->target_level != target->level)
            {
                LOS_SET_TARGET(pl, target, pl->target_mode, pl->target_index);
                pl->update_target = 1;
            }
            /* Changed mode (ie, friend becomes enemy or vice versa)? Set
             * pl->target_mode accordingly and update. This means, for example,
             * that a player can keep another player targeted and he will
             * alternate between friend and enemy as their movements carry them
             * in and out of PvP zones so attacking is automatically and
             * immediately corrected. However this does require extra code
             * (chiefly a call to get_friendship() -- which I have streamlined
             * to save 2-4 ums) which adds (very small) amounts of tme (as
             * above). Comment out this clause to reduce the overall time to
             * about 5 ums.
             *
             * -- Smacky 20150919 */
            else
            {
                sint32 friendship = get_friendship(who, target);
                uint8  mode = LOS_GET_REAL_TARGET_MODE(target, pl->target_mode, friendship);
                
                if (mode != pl->target_mode)
                {
                    LOS_SET_TARGET(pl, target, mode, pl->target_index);
                    pl->update_target = 1;
                }
            }
        }

        if (pl->update_target)
        {
            send_target_command(pl);
            pl->update_target = 0;
        }

        if (who->weapon_speed_left <= 0)
        {
            /* tell the client the skill cooldown time has ran out, it should only transmit this once */
            pl->action_timer = 0;

            /* now use the new target system to hit our target... Don't hit non
            * friendly objects, ourself or when we are not in combat mode.
            */
            if(pl->combat_mode && 
               pl->target_mode == LOS_TARGET_ENEMY)
            {
                object_t *target = pl->target_ob;

                if (is_melee_range(who, target))
                {
                    /* tell our enemy we swing at him now */
                    update_npc_knowledge(target, who, FRIENDSHIP_TRY_ATTACK, 0);
                    pl->rest_mode = 0;
                    skill_attack(target, who, 0, NULL);
                }
            }
        }

        if (who->speed_left > who->speed)
            who->speed_left = who->speed;
    }
}

static void process_map_events(map_t *map)
{
    object_t *op, *first_obj;
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
                                      op->arch->name,STRING_OBJ_NAME(op),op->type, op->count);
                */
            op->speed = 0;
            update_ob_speed(op);
            continue;
        }

        /*LOG(llevNoLog,"POBJ: %s (%s) s:%f sl:%f (%f)\n",STRING_OBJ_NAME(op),op->arch->clone.name, op->speed,op->speed_left,op->arch->clone.speed_left);*/
        if (!op->speed)
        {
            LOG(llevBug,
                "BUG: process_events(): Object %s (%s, type:%d count:%d) has no speed, but is on active list\n",
                op->arch->name, STRING_OBJ_NAME(op), op->type, op->count);
            update_ob_speed(op);
            continue;
        }


        if (op->map == NULL && op->env == NULL)
        {
            if (op->type == PLAYER && !(CONTR(op)->state & ST_PLAYING))
                continue;
            LOG(llevBug, "BUG: process_events(): Object without map or inventory is on active list: %s (%d)\n",
                STRING_OBJ_NAME(op), op->count);
            op->speed = 0;
            update_ob_speed(op);
            continue;
        }

        /* This is not a bug. We just move the object to the
         * insertion list and sort it out later. Process as usual. */
        if (op->map != map)
        {
            LOG(llevDebug, "WARNING: process_events(): object not on processed map: %s is on %s, not %s\n",
                STRING_OBJ_NAME(op), STRING_MAP_PATH(op->map), STRING_MAP_PATH(map));
            activelist_remove(op);
            activelist_insert(op);
        }

        /* only players and monsters should have weapon_speed set and be active */
        if (op->weapon_speed && !QUERY_FLAG(op, FLAG_PARALYZED))
        {
            if (op->weapon_speed_left > 0)
                op->weapon_speed_left -= WEAPON_SWING_TIME;

            /* PLAYER swings - monster swing will be done implicit in process_object() */
        }

        if (op->speed_left > 0)
        {
            --op->speed_left;
            process_object(op);
            if (!OBJECT_VALID(op, tag))
                continue;
        }
        else if (op->weapon_speed_left <= 0 &&
            op->type == MONSTER)
        {
            move_monster(op, 0); /* false = only weapon action */
        }



        /* Eneq(@csd.uu.se): Handle archetype-field anim_speed differently when
           it comes to the animation. If we have a value on this we don't animate it
           at speed-events. */
        if (QUERY_FLAG(op, FLAG_ANIMATE))
        {
            if (op->anim_speed_last >= op->anim_speed)
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
                op->anim_speed_last = 1;
            }
            else
            {
                if (NUM_FACINGS(op) >= 25) /* check for direction changing */
                    animate_object(op, 0);
                op->anim_speed_last++;
            }
        }

        if (op->speed_left <= 0)
            op->speed_left += ABS(op->speed);
    }
    next_active_object = NULL;
}

void process_events()
{
    map_t *map;

#if defined TIME_PROCESS_EVENTS
    TPR_START();
#endif
    process_players1(NULL);
    /* Preprocess step: move all objects in inserted_active_objects
     * into their real activelists */
    /* TODO: to make AI more efficient, keep activelist sorted with
     * mobs&players first and less interesting objects next, or
     * have two activelists per map for that */
    if(inserted_active_objects) {
        object_t *obj, *next;
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
                     STRING_OBJ_NAME(obj), STRING_OBJ_NAME(obj->env) );
                obj->map = NULL;
            }
            if(obj->map)
            {
                /* In case the map was swapped out with an object in the insertion list
                 * (This probably doesn't happen, since all objects are removed when the map
                 * is swapped out) */
                if(obj->map->in_memory != MAP_MEMORY_ACTIVE)
                {
                    /* FIXME: for now we'll just see if this happens */
                    LOG( llevDebug, "ACTIVEBUG: object on map not in memory! obj %s in %s\n",
                            STRING_OBJ_NAME(obj), STRING_MAP_PATH(obj->map) );
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
        if (map->active_objects->active_next && map->in_memory == MAP_MEMORY_ACTIVE)
            process_map_events(map);
    }

    process_players2(NULL);

#if defined TIME_PROCESS_EVENTS
    TPR_STOP("Process events");
#endif /* TIME PROCESS EVENTS */
}

void clean_tmp_files(int flag)
{
    map_t  *m, *next;

    LOG(llevInfo, "Save maps and cleaning up...\n");

    /* We save the maps - it may not be intuitive why, but if there are unique
     * items, we need to save the map so they get saved off.  Perhaps we should
     * just make a special function that only saves the unique items.
     */
    for (m = first_map; m != NULL; m = next)
    {
        next = m->next;
        if (m->in_memory == MAP_MEMORY_ACTIVE)
        /* If we want to reuse the temp maps, swap it out (note that will also
         * update the log file.  Otherwise, save the map (mostly for unique item
         * stuff).  Note that the clean_tmp_map is called after the end of
         * the for loop but is in the #else bracket.  IF we are recycling the maps,
         * we certainly don't want the temp maps removed.
         */
        if(flag)
#ifdef RECYCLE_TMP_MAPS
        {
            swap_map(m, 0);
        }
#else
        {
            map_save(m);
        }
        clean_tmp_map(m);
#endif
    }
}

void cleanup_without_exit(void)
{
    LOG(llevDebug, "Cleanup called.  freeing data.\n");
    clean_tmp_files(1);
    write_book_archive();
    write_tadclock();   /* lets just write the clock here */
    save_ban_file();

    /* that must be redone: clear cleanup so we know 100% all memory is freed */
    free_all_maps();
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
    free_sounds();
    free_strings();
    free_lists_and_tables();
    cleanup_all_behavioursets();
    free_gmaster_list();
    cleanup_mempools();

    /* free_all_srv_files(); */
}

void leave(player_t *pl, int draw_exit)
{
    if (pl != NULL)
    {
        char buf[MEDIUM_BUF];

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
        sprintf(buf, "LOGOUT: IP >%s< Account >%s< Player >%s<!\n",
                pl->socket.ip_host, pl->account_name, STRING_OBJ_NAME(pl->ob));
        LOG(llevInfo, "%s", buf);

        if (clogfile != tlogfile)
        {
            CHATLOG("%s", buf);
        }

        if (pl->ob->map)
        {
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
    object_t                 *op;

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
    object_t *op                  = get_next_requested_path();
    extern void             object_accept_path  (object_t *op);
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
#ifdef DEBUG_CALENDAR
    if (!(ROUND_TAG % (PTICKS_PER_ARKHE_HOUR/* / ARKHE_MES_PER_HR*/)) &&
        first_player)
    {
        (void)command_time(first_player->ob, "verbose");
    }
#endif

    if (!(ROUND_TAG % PTICKS_PER_ARKHE_HOUR))
        tick_tadclock();

    if (!(ROUND_TAG % 2))
        dequeue_path_requests();

    /*   if (!(ROUND_TAG % 20)) */ /*use this for debuging */

    /* We only check for maps needing swap/reset every second. */
    if (!(ROUND_TAG % (long unsigned int)MAX(1, pticks_second)))
    {
        map_check_in_memory(NULL);
    }

    if (!(ROUND_TAG % 2521))
        metaserver_update();    /* 2500 ticks is about 5 minutes */
}


/* The shutdown agent is a automatic timer which shuts down the server after
 * the given time. It gives out messages to all players to announce the
 * shutdown and the status of the shutdown.
 *
 * If timer >= 0, a new countdown will start. ret should be SERVER_EXIT_RESTART
 * or SERVER_EXIT_SHUTDOWN. pl should be the player who started the shutdown
 * (or NULL for anonymous). reason should be any extra explanation of the
 * shutdown (or NULL for none given). A message that a countdown has started
 * will be broadcast to all players
 *
 * If timer == -1, any existing countdown will continue. ret should be
 * SERVER_EXIT_NORMAL. pl should be a player or NULL. reason should be NULL.
 * Every minute and when the countdown reaches 0 a message about the shutdown's
 * progress is broadcast to all players. If pl != NULL, the current countdown,
 * wherever it is, is sent to that player only. There is always a three second
 * delay before the actual shutdown happens.
 *
 * If timer == -2, any existing countdown is stopped. pl should be the player
 * responsible. Other parameters are ignored. A message that the shutdown has
 * been stopped is broadcast to all players. */
void shutdown_agent(int timer, int ret, player_t *pl, char *reason)
{
    static int            status = SERVER_EXIT_NORMAL,
                          sd_timer = -1;
    static struct timeval tv1,
                          tv2;
    static char           name[SMALL_BUF] = "",
                          buf[MEDIUM_BUF] = "";
    int                   t_tot,
                          t_min,
                          t_sec;

    /* Stop shutdown/restart */
    if (timer == -2)
    {
        if (status != SERVER_EXIT_NORMAL)
        {
            sprintf(buf, "SERVER %s STOPPED by %s",
                    (status == SERVER_EXIT_RESTART) ? "RESTART" : "SHUTDOWN",
                    STRING_OBJ_NAME(pl->ob));
            LOG(llevSystem, "%s", buf);
            ndi(NDI_PLAYER | NDI_UNIQUE | NDI_ALL | NDI_GREEN, 5,
                          NULL, "[Server]: ** %s **", buf);
        }

        status = SERVER_EXIT_NORMAL;
        sd_timer = -1;
        name[0] = '\0';
        buf[0] = '\0';

        return;
    }

    if (ret != SERVER_EXIT_NORMAL)
    {
        status = ret;
    }

    /* Set a new timer of <timer> seconds. */
    if (timer >= 0)
    {
        sd_timer = timer;
        GETTIMEOFDAY(&tv1);

        /* Make a staic copy of the caller's name in case he logs before the
           countdown is done. */
        if (pl)
        {
            sprintf(name, "%s", STRING_OBJ_NAME(pl->ob));
        }

        if (reason)
        {
            sprintf(buf, "%s", reason);
        }

        t_tot = timer;
        ndi(NDI_PLAYER | NDI_UNIQUE | NDI_ALL | NDI_GREEN, 5, NULL,
                      "[Server]: ** SERVER %s STARTED by %s **\n    %s",
                      (status == SERVER_EXIT_RESTART) ? "RESTART" : "SHUTDOWN",
                      (name[0]) ? name : "no-one",
                      (buf[0]) ? buf : "no reason specified!");
    }
    /* Countdown the existing timer. */
    else if (sd_timer >= 0)
    {
        GETTIMEOFDAY(&tv2);
        t_tot = sd_timer - (int)(tv2.tv_sec - tv1.tv_sec);

        /* Exactly when the timer expires, send messages to the client. */
        if (t_tot == 0)
        {
            ndi(NDI_PLAYER | NDI_UNIQUE | NDI_ALL | NDI_GREEN, 5,
                          NULL, "[Server]: ** SERVER GOES DOWN NOW!!! **");

            return;
        }
        /* Wait another 3 seconds before really killing the server so the
         * m`essages above can be broadcast. */
        else if (t_tot <= -3)
        {
            LOG(llevSystem, "\n\nSERVER %s by %s -- %s\n",
                (status == SERVER_EXIT_RESTART) ? "RESTART" : "SHUTDOWN",
                (name[0]) ? name : "no-one",
                (buf[0]) ? buf : "no reason specified!");
            kick_player(NULL);
            cleanup_without_exit();
            exit(status);

            return; // actually never reached
        }

        t_tot--;
    }
    /* Do nothing */
    else
    {
        return;
    }

    /* If the circumstances are right report the time left. */
    if (sd_timer >= 0 &&
        t_tot >= 0)
    {
        t_min = t_tot / 60;
        t_sec = t_tot - t_min * 60;

        /* To everyone if the countdown has been reset or every minute on the
         * minute. */
        if (!t_sec ||
            timer >= 0)
        {
            ndi(NDI_PLAYER | NDI_UNIQUE | NDI_ALL | NDI_GREEN, 5,
                          NULL, "[Server]: SERVER %s in %d minute%s and %d second%s",
                          (status == SERVER_EXIT_RESTART) ? "RESTART" : "SHUTDOWN",
                          t_min, (t_min == 1) ? "" : "s",
                          t_sec, (t_sec == 1) ? "" : "s");
        }
        /* To individual players who login during a countdown. */
        else if (timer == -1 &&
                 pl)
        {
            ndi(NDI_UNIQUE | NDI_GREEN, 5, pl->ob, "[Server]: SERVER %s in %d minute%s and %d second%s",
                          (status == SERVER_EXIT_RESTART) ? "RESTART" : "SHUTDOWN",
                          t_min, (t_min == 1) ? "" : "s",
                          t_sec, (t_sec == 1) ? "" : "s");
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
                player_t *pl = NULL;

                LOG(llevDebug, "found player %s...\n", entry->d_name);

                if(!(pl = get_player(NULL)))
                    LOG(llevDebug, "Error: loading player %s...\n", entry->d_name);
                else
                {
                    pl->socket.status = Ns_Disabled;
                    entry->d_name[fptr-entry->d_name] = 0;
                    SHSTR_FREE_AND_ADD_STRING(pl->ob->name, entry->d_name);
                    entry->d_name[fptr-entry->d_name] = '.';
                    chdir(base_cwd);
                    check_login(pl->ob, 0);
                    /* player is now loaded, do something with it - after it, release player & object_t */

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

/* Calculate time until the next tick.
 * returns 0 and steps forward time for the next tick if called after the time
 * for the next tick, otherwise returns 1 and the delta time for next tick. */
static inline int time_until_next_tick(struct timeval *out)
{
    struct timeval now, next_tick, tick_time;

    /* next_tick = last_time + tick_time */
    tick_time.tv_sec = 0;
    tick_time.tv_usec = pticks_ums;

    add_time(&next_tick, &last_time, &tick_time);

    GETTIMEOFDAY(&now);

    /* Time for the next tick? (timercmp does not work for <= / >=) */
    /* if(timercmp(&next_tick, &now, <) || timercmp(&next_tick, &now, ==)) */

    /* timercmp() seems be broken under windows. Well, this is even faster */
    if( next_tick.tv_sec < now.tv_sec ||
        (next_tick.tv_sec == now.tv_sec && next_tick.tv_usec <= now.tv_usec))
    {
        /* this must be now time and not next_tick.
         * IF the last tick was really longer as pticks_ums,
         * we need to come insync now again.
         * Or, in bad cases, the more needed usecs will add up.
         */
        last_time.tv_sec = now.tv_sec;
        last_time.tv_usec = now.tv_usec;

        out->tv_sec = 0;
        out->tv_usec = 0;

        return 0;
    }

    /* time_until_next_tick = next_tick - now */
    now.tv_sec = -now.tv_sec;
    now.tv_usec = -now.tv_usec;
    add_time(out, &next_tick, &now);

    return 1;
}

/* sleep_delta checks how much time has elapsed since last tick. If it is less
 * than pticks_ums, the remaining time is slept with select().
 *
 * Polls the sockets and handles or queues incoming requests returns at the
 * time for the next tick. */
static inline void sleep_delta()
{
    struct timeval timeout;

    /* TODO: ideally we should use the return value from select to know if it
     * timed out or returned because of some other reason, but this also
     * works reasonably well... */
    while(time_until_next_tick(&timeout))  /* fill timeout... */
        doeric_server(SOCKET_UPDATE_CLIENT, &timeout);
}

void iterate_main_loop()
{
    struct timeval timeout;

#ifdef PLUGINS
    int     evtid;
    CFParm  CFP;
#endif

    nroferrors = 0;                 /* every llevBug will increase this counter - avoid LOG loops */
    pticks++;                       /* ROUND_TAG ! this is THE global tick counter . Use ROUND_TAG in the code */

    /* Only call this every second. More precision is unnecessary.
     * We assume it won't be called in the first second of the server's life,
     * to save a check. */
    if (!(ROUND_TAG % (long unsigned int)MAX(1, pticks_second)))
    {
        shutdown_agent(-1, SERVER_EXIT_NORMAL, NULL, NULL);
    }

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

    do_specials();              /* Routines called from time to time. */

    object_gc();                /* Clean up the object pool */
    sleep_delta();              /* Sleep proper amount of time before next tick but poll the socket */
}

#define AUTO_MSG_COUNTER (8*60*30) /* all 30 minutes */
int main(int argc, char **argv)
{
#ifdef AUTO_MSG
    int auto_msg_count = 8*60*3; /* kick the first message 3 minutes after server start */
#endif

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

    /*STATS_EVENT(STATS_EVENT_STARTUP);*/
    GETTIMEOFDAY(&last_time); /* init our last_time = start time - and lets go! */

#ifdef DEBUG_TRAVERSE_PLAYER_DIR
    traverse_player_stats("./data/players");
#endif

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
            ndi(NDI_PLAYER | NDI_UNIQUE | NDI_ALL | NDI_GREEN, 5, NULL,
                    "[INFO]: Please HELP US and VOTE for Daimonin DAILY!\nGo to www.daimonin.org and hit the VOTE ICONS!\nThanks and happy playing!! - Michtoen");
        }
#endif

        iterate_main_loop();
    }

    return 0;
}

