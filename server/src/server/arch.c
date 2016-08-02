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

/* IF set, does a little timing on the archetype load. */
#define TIME_ARCH_LOAD 0

static hashtable *arch_table;

_archetype_global archetype_global;

/**
 * GROS -  This function retrieves an archetype given the name that appears
 * during the game (for example, "writing pen" instead of "stylus").
 * It does not use the hashtable system, but browse the whole archlist each time.
 * I suggest not to use it unless you really need it because of performance issue.
 * It is currently used by scripting extensions (create-object).
 * Params:
 * - name: the name we're searching for (ex: "writing pen");
 * Return value:
 * - the archetype_t found or null if nothing was found.
 */
archetype_t * find_archetype_by_object_name(const char *name)
{
    archetype_t  *at;

    if (name == NULL)
        return (archetype_t *) NULL;

    for (at = first_archetype; at != NULL; at = at->next)
    {
        if (!strcmp(at->clone.name, name))
            return at;
    };
    return NULL;
}

/**
 * GROS - this returns a new object given the name that appears during the game
 * (for example, "writing pen" instead of "stylus").
 * Params:
 * - name: The name we're searching for (ex: "writing pen");
 * Return value:
 * - a corresponding object if found; a singularity object if not found.
 */
object_t * get_archetype_by_object_name(const char *name)
{
    archetype_t  *at;
    char        tmpname[MEDIUM_BUF];
    int         i;

    strncpy(tmpname, name, MEDIUM_BUF - 1);
    tmpname[MEDIUM_BUF - 1] = 0;
    for (i = strlen(tmpname); i > 0; i--)
    {
        tmpname[i] = 0;
        at = find_archetype_by_object_name(tmpname);
        if (at != NULL)
        {
            return arch_to_object(at);
        }
    }
    return create_singularity(name);
}

/* The preprocessor can't paste to . which is why all the member names begin
 * with _ */
#define SET_GLOBAL_ARCHETYPE(_A_) \
    if (!(archetype_global._ ## _A_ = find_archetype(#_A_))) \
    { \
        LOG(llevError, "ERROR:: %s/init_archetypes(): Archetype '%s' not found!\n", \
            __FILE__, #_A_); \
    }

/*
 * Initialises the internal linked list of archetypes (read from file).
 * Then the global archetype pointers are initialised.
 * Then the blocksview[] array is initialised (apparently).
 */

void init_archetypes()
{
    /* called from and edit() */
    if (first_archetype != NULL) /* Only do this once */
        return;
    arch_init = 1;
    load_archetypes();
    arch_init = 0;

    SET_GLOBAL_ARCHETYPE(empty_archetype);
    SET_GLOBAL_ARCHETYPE(base_info);
    SET_GLOBAL_ARCHETYPE(waypoint);
    SET_GLOBAL_ARCHETYPE(level_up);
    SET_GLOBAL_ARCHETYPE(aggro_history);
    SET_GLOBAL_ARCHETYPE(dmg_info);
    SET_GLOBAL_ARCHETYPE(drain);
    SET_GLOBAL_ARCHETYPE(depletion);
    SET_GLOBAL_ARCHETYPE(ring_normal);
    SET_GLOBAL_ARCHETYPE(ring_generic);
    SET_GLOBAL_ARCHETYPE(amulet_generic);
    SET_GLOBAL_ARCHETYPE(mitcoin);
    SET_GLOBAL_ARCHETYPE(goldcoin);
    SET_GLOBAL_ARCHETYPE(silvercoin);
    SET_GLOBAL_ARCHETYPE(coppercoin);
    SET_GLOBAL_ARCHETYPE(quest_container);
    SET_GLOBAL_ARCHETYPE(quest_info);
    SET_GLOBAL_ARCHETYPE(quest_trigger);
    SET_GLOBAL_ARCHETYPE(quest_update);
    SET_GLOBAL_ARCHETYPE(player_info);
    SET_GLOBAL_ARCHETYPE(force);
    SET_GLOBAL_ARCHETYPE(guild_force);
    SET_GLOBAL_ARCHETYPE(alignment_force);
    SET_GLOBAL_ARCHETYPE(rank_force);
    SET_GLOBAL_ARCHETYPE(gravestone);
    SET_GLOBAL_ARCHETYPE(deathsick);
    SET_GLOBAL_ARCHETYPE(poisoning);
    SET_GLOBAL_ARCHETYPE(slowness);
    SET_GLOBAL_ARCHETYPE(fear);
    SET_GLOBAL_ARCHETYPE(snare);
    SET_GLOBAL_ARCHETYPE(confusion);
    SET_GLOBAL_ARCHETYPE(blindness);
    SET_GLOBAL_ARCHETYPE(paralyze);
    SET_GLOBAL_ARCHETYPE(potion_effect);
    SET_GLOBAL_ARCHETYPE(corpse_default);
    SET_GLOBAL_ARCHETYPE(loot_container);
    SET_GLOBAL_ARCHETYPE(pvp_stat_force);
    SET_GLOBAL_ARCHETYPE(meffect);
}

#undef SET_GLOBAL_ARCHETYPE

/*
 * Stores debug-information about how efficient the hashtable
 * used for archetypes has been in the static errmsg array.
 */

void arch_info(object_t *op)
{
    ndi(NDI_WHITE, 0, op, "%d searches and %d strcmp()'s", arch_search, arch_cmp);
}

/*
 * Initialise the hashtable used by the archetypes.
 */

void clear_archetable()
{
    arch_table = string_hashtable_new(8192);
}

/*
 * An alternative way to init the hashtable which is slower, but _works_...
 */

void init_archetable()
{
    archetype_t  *at;
    LOG(llevDebug, " setting up archetable...");
    for (at = first_archetype; at != NULL; at = (at->more == NULL) ? at->next : at->more)
        add_arch(at);
    LOG(llevDebug, "done\n");
}

/*
 * Dumps an archetype to debug-level output.
 */

void dump_arch(archetype_t *at)
{
    dump_object(&at->clone);
}

/*
 * Dumps _all_ archetypes to debug-level output.
 * If you run crossfire with debug, and enter DM-mode, you can trigger
 * this with the O key.
 */

void dump_all_archetypes()
{
    archetype_t      *at;
    artifactlist   *al;
    artifact       *art = NULL;

    for (at = first_archetype; at != NULL; at = (at->more == NULL) ? at->next : at->more)
    {
        dump_arch(at);
        LOG(llevInfo, "%s\n", STRING_SAFE(errmsg));
    }

    LOG(llevInfo, "Artifacts fake arch list:\n");
    for (al = first_artifactlist; al != NULL; al = al->next)
    {
        art = al->items;
        do
        {
            if(art->flags&ARTIFACT_FLAG_HAS_DEF_ARCH )
            {
                dump_arch(&art->def_at);
                LOG(llevInfo, "%s\n", STRING_SAFE(errmsg));
            }
            art = art->next;
        }
        while (art != NULL);
    }
}

void free_all_archs()
{
    archetype_t  *at, *next;
    int         i = 0, f = 0;

    for (at = first_archetype; at != NULL; at = next)
    {
        if (at->more)
            next = at->more;
        else
            next = at->next;
        FREE_AND_CLEAR_HASH(at->name);

        if(at->clone.inv)
            LOG(llevDebug, "free_all_archs(): archetype clone %s has inv %s\n", STRING_OBJ_NAME(&at->clone), STRING_OBJ_NAME(at->clone.inv));
        free_object_data(&at->clone, 1);

        free(at);
        i++;
    }
    LOG(llevDebug, "Freed %d archetypes, %d faces\n", i, f);
}

/*
 * Allocates, initialises and returns the pointer to an archetype structure.
 */

archetype_t * get_archetype_struct()
{
    archetype_t *new;

    MALLOC(new, sizeof(archetype_t));

    if (new == NULL)
        LOG(llevError, "get_archetype_struct() - out of memory\n");

    initialize_object(&new->clone);  /* to initial state other also */

    return new;
}

/*
 * Reads/parses the archetype-file, and copies into a linked list
 * of archetype-structures.
 */
void first_arch_pass(FILE *fp)
{
    object_t     *op;
    void       *mybuffer;
    archetype_t  *at, *prev = NULL, *last_more = NULL;
    int         i;

    op = get_object();
    op->arch = first_archetype = at = get_archetype_struct();
    mybuffer = create_loader_buffer(fp);
    while ((i = load_object(fp, op, mybuffer, LO_REPEAT, MAP_STATUS_STYLE)))
    {
        if (!QUERY_FLAG(op, FLAG_SYS_OBJECT))
        {
            if (op->layer == MSP_SLAYER_SYSTEM)
            {
                LOG(llevSystem, " ARCHBUG: Archetype %s has layer 0 but is not sys_object!\n",
                    STRING_OBJ_ARCH_NAME(op));
            }
        }
        else
        {
            if (op->layer != MSP_SLAYER_SYSTEM)
            {
                LOG(llevDebug, " WARNING: Archetype %s has layer %d (!=0) and is sys_object\n",
                    STRING_OBJ_ARCH_NAME(op), op->layer);
            }
        }

#ifdef USE_SLAYER_MONSTER
        /* Do some on-the-fly layer correction. If we make this feature
         * permanent the layers in the actual arches should be updated
         * accordingly and this code removed. */
        if (op->type == MONSTER)
        {
            op->layer = MSP_SLAYER_MONSTER;
        }
        else if (op->type == PLAYER)
        {
            op->layer = MSP_SLAYER_PLAYER;
        }
        else if (op->layer == 7)
        {
            op->layer = MSP_SLAYER_EFFECT;
        }

#endif
        /* use copy_object_data() - we don't want adjust any speed_left here! */
        copy_object_data(op, &at->clone);

        /* ok... now we have the right speed_left value for out object.
         * copy_object() now will track down negative speed values, to
         * alter speed_left to have a random & senseful start value.
         */


        switch (i)
        {
            case LL_NORMAL:
              /* A new archetype, just link it with the previous */
              if (last_more != NULL)
                  last_more->next = at;
              if (prev != NULL)
                  prev->next = at;
              prev = last_more = at;

              /* silly debug
                if(op->animation_id && !op->anim_speed)
                LOG(llevDebug," WARNING: Archetype %s has animation but no anim_speed!\n", STRING_OBJ_ARCH_NAME(op));
                if(op->animation_id && !QUERY_FLAG(op,FLAG_CLIENT_SENT))
                LOG(llevDebug," WARNING: Archetype %s has animation but no explicit set is_animated!\n", STRING_OBJ_ARCH_NAME(op));
              */
              if (!op->type)
                  LOG(llevDebug, " WARNING: Archetype %s has no type info!\n", STRING_OBJ_ARCH_NAME(op));
              break;

            case LL_MORE:
              /* Another part of the previous archetype, link it correctly */
              at->head = prev;
              at->clone.head = &prev->clone;
              if (last_more != NULL)
              {
                  last_more->more = at;
                  last_more->clone.more = &at->clone;
              }
              last_more = at;

              break;
        }
        /*CLEAR_FLAG((&at->clone), FLAG_CLIENT_SENT);*/ /* we using this flag for debugging - ignore */
        at = get_archetype_struct();
        free_object_data(op, 1);
        initialize_object(op); /* clear - op is only temp. buffer for at->clone */
        op->arch = at;
    }
    delete_loader_buffer(mybuffer);
    mark_object_removed(op); /* make sure our temp object is gc:ed */
    free(at);
}

/*
 * Reads the archetype file once more, and links all pointers between
 * archetypes.
 */

void second_arch_pass(FILE *fp_start)
{
    FILE           *fp  = fp_start;
    char            buf[MEDIUM_BUF], *variable = buf, *argument, *cp;
    archetype_t      *at =  NULL, *other;

    while (fgets(buf, MEDIUM_BUF, fp) != NULL)
    {
        if (*buf == '#')
            continue;
        if ((argument = strchr(buf, ' ')) != NULL)
        {
            *argument = '\0',argument++;
            cp = argument + strlen(argument) - 1;
            while (isspace(*cp))
            {
                *cp = '\0';
                cp--;
            }
        }

        if (!strcmp("Object", variable))
        {
            if ((at = find_archetype(argument)) == NULL)
                LOG(llevBug, "BUG: failed to find arch %s\n", STRING_SAFE(argument));
        }
        else if (!strcmp("other_arch", variable))
        {
            if (at != NULL && at->clone.other_arch == NULL)
            {
                if ((other = find_archetype(argument)) == NULL)
                    LOG(llevBug, "BUG: failed to find other_arch %s\n", STRING_SAFE(argument));
                else if (at != NULL)
                    at->clone.other_arch = other;
            }
        }
        else if (!strcmp("randomitems", variable))
        {
            if (at)
                at->clone.randomitems = link_treasurelists(argument, OBJLNK_FLAG_STATIC);
        }
    }
}

/*
 * First initialises the archetype_type hash-table (init_archetable()).
 * Reads and parses the archetype file (with the first and second-pass
 * functions).
 * Then initialises treasures by calling load_treasures().
 */

void load_archetypes()
{
    FILE           *fp;
    char            filename[MEDIUM_BUF];
#if TIME_ARCH_LOAD
    struct timeval  tv1, tv2;
#endif

    sprintf(filename, "%s/%s", settings.datadir, settings.archetypes);
    LOG(llevDebug, "Reading archetypes from %s...\n", STRING_SAFE(filename));
    if ((fp = fopen(filename, "r")) == NULL)
    {
        LOG(llevError, "ERROR: Can't open archetype file.\n");
        return;
    }
    clear_archetable();
    LOG(llevDebug, " arch-pass 1...\n");
#if TIME_ARCH_LOAD
    GETTIMEOFDAY(&tv1);
#endif
    first_arch_pass(fp);
#if TIME_ARCH_LOAD
    {
        int sec, usec;
        GETTIMEOFDAY(&tv2);
        sec = tv2.tv_sec - tv1.tv_sec;
        usec = tv2.tv_usec - tv1.tv_usec;
        if (usec < 0)
        {
            usec += 1000000; sec--;
        }
        LOG(llevDebug, "Load took %d.%06d seconds\n", sec, usec);
    }
#endif

    LOG(llevDebug, " done.\n");
    init_archetable();

    /* do a close and reopen instead of a rewind - necessary in case the
     * file has been compressed.
     */
    fclose(fp);
    fp = fopen(filename, "r");

    /* I moved the artifacts loading to this position because it must be done
     * BEFORE we load the treasure file - remember we have now fake arches in the
     * artifacts file
     * second_arch_pass reparse the archetype file again and add other_arch and
     * randomitems (= treasurelists) to the arches.
     */
    load_artifacts(ARTIFACTS_FIRST_PASS);  /* If not called before, reads all artifacts from file */
    add_artifact_archetype_type();
    LOG(llevDebug, " loading treasure...\n");
    load_treasures();
    LOG(llevDebug, " done\n arch-pass 2...\n");
    second_arch_pass(fp);
    /* now reparse the artifacts file too! */
    LOG(llevDebug, " done\n artifact-pass 2...\n");
    load_artifacts(ARTIFACTS_SECOND_PASS);
    LOG(llevDebug, " done.\n");
    fclose(fp);
    LOG(llevDebug, "Reading archetypes done.\n");
}

/*
 * Creates and returns a new object which is a copy of the given archetype.
 * This function returns NULL on failure.
 */

object_t * arch_to_object(archetype_t *at)
{
    object_t *op;
    if (at == NULL)
    {
        LOG(llevBug, "BUG: arch_to_object(): archetype at at == NULL.\n");
        return NULL;
    }
    op = get_object();
    /* this was copy_object() before but for example for
     * temporary objects, we don't want move them to active list.
     * copy_object_data() is right, but should be watched for side effects. MT-07.2005
     */
    copy_object_data(&at->clone, op);
    op->arch = at;
    return op;
}

/*
 * Creates an object.  This function is called by get_archetype()
 * if it fails to find the appropriate archetype.
 * Thus get_archetype() will be guaranteed to always return
 * an object, and never NULL.
 */

object_t * create_singularity(const char *name)
{
    object_t *op = arch_to_object(archetype_global._empty_archetype);

    FREE_AND_COPY_HASH(op->name, name);
    FREE_AND_COPY_HASH(op->title, " (singularity)");

    /* Remove them automatically - good for player inventory */
    /* FIXME: Or is it that simple? This also means that ->inv will be
     * removed/not saved AND that the evidence of whatever bug caused the
     * singularity disappears up its own bughole. These were both factors that
     * made the 0.10.6 skills bug such a pain to find and fix and have worse
     * consequences than otherwise.
     *
     * I will therefore leave this disabled which leaves the object heirarchy
     * complete ammd ready for manual fixing.
     *
     * -- Smacky 20140811 */
//    SET_FLAG(op, FLAG_NO_SAVE);
    LOG(llevInfo, "INFO:: Created singularity: %s\n", STRING_OBJ_NAME(op));
    return op;
}

/*
 * Finds which archetype matches the given name, and returns a new
 * object containing a copy of the archetype.
 */

object_t * get_archetype(const char *name)
{
    archetype_t  *at;
    at = find_archetype(name);
    if (at == NULL)
        return create_singularity(name);
    return arch_to_object(at);
}

/*
 * Finds, using the hashtable, which archetype matches the given name.
 * returns a pointer to the found archetype, otherwise NULL.
 */

archetype_t * find_archetype(const char *name)
{
    if (name == NULL)
        return (archetype_t *) NULL;

    return (archetype_t *)hashtable_find(arch_table, name);
}

/*
 * Adds an archetype to the hashtable.
 */

void add_arch(archetype_t *at)
{
    if (! hashtable_insert(arch_table, at->name, at))
    {
        LOG(llevError, "ERROR: add_arch(): double use of arch name %s\n", STRING_ARCH_NAME(at));
    }
}

/*
 * Returns the first archetype using the given type.
 * Used in treasure-generation.
 */

archetype_t * type_to_archetype(int type)
{
    archetype_t  *at;

    for (at = first_archetype; at != NULL; at = (at->more == NULL) ? at->next : at->more)
        if (at->clone.type == type)
            return at;
    return NULL;
}

/*
 * Returns a new object copied from the first archetype matching
 * the given type.
 * Used in treasure-generation.
 */

object_t * clone_arch(int type)
{
    archetype_t  *at;
    object_t     *op  = get_object();

    if ((at = type_to_archetype(type)) == NULL)
    {
        LOG(llevBug, "BUG: Can't clone archetype %d\n", type);
        return NULL;
    }
    copy_object(&at->clone, op);
    return op;
}

/*** end of arch.c ***/
