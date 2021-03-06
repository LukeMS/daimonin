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

#ifndef WIN32 /* ---win32 exclude unix headers */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#endif /* end win32 */

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

typedef enum {
    CREATE,
    GENERATE,
    SPAWN
} CreateMode_t;

/* Declare some local helper functions for command_ban() */
static int BanList(object_t *op, ENUM_BAN_TYPE ban_type);
static int BanAdd(object_t *op, ENUM_BAN_TYPE ban_type, char *str, int s);
static int BanAddToBanList(object_t *op, ENUM_BAN_TYPE ban_type, char *str, int s);
static int BanRemove(object_t *op, ENUM_BAN_TYPE ban_type, char *str);
static int BanRemoveFromBanList(object_t *op, ENUM_BAN_TYPE ban_type, char *str, int mute);

static int CreateObject(object_t *op, char *params, CreateMode_t mode);
static int CheckAttributeValue(char *var, char *val, CreateMode_t mode);

/*
 * Helper function get an object somewhere in the game
 * Returns the object_t which has the count-variable equal to the argument.
 */

static object_t * find_object(int i)
{
    /* i is the count - browse ALL and return the object_t */
    return NULL;
}

int command_kick(object_t *op, char *params)
{
    player_t     *pl;
    const char *kicker_name,
               *kickee_name;
    char        buf[MEDIUM_BUF];
    int         ticks;

    if (!op)
        return COMMANDS_RTN_VAL_ERROR;

    if (!params)
        return COMMANDS_RTN_VAL_SYNTAX;

    if (!(pl = find_player(params)))
    {
        ndi(NDI_UNIQUE, 0, op, "No such player.");

        return COMMANDS_RTN_VAL_ERROR;
    }

#if 0 // Kicking yourself is funny, a real votewinner :)
    if (pl->ob == op)
    {
        ndi(NDI_UNIQUE, 0, op, "You can't kick yourself!");

        return COMMANDS_RTN_VAL_ERROR;
    }
#endif

    /* Remember the names before we get kickhappy. */
    kicker_name = STRING_OBJ_NAME(op);
    kickee_name = STRING_OBJ_NAME(pl->ob);

    /* Get kickhappy. */
    kick_player(pl);

    /* Hopefully impossible. */
    if ((pl = find_player(params)))
    {
        LOG(llevBug, "BUG:: %s/command_kick(): %s is kickproof!\n",
            __FILE__, kickee_name);
        ndi(NDI_UNIQUE, 0, op, "%s has resisted your kick!",
                             kickee_name);

        return COMMANDS_RTN_VAL_ERROR;
    }

    /* Log it and tell everyone so justice is seen to be done, or at least we
     * all get a good laugh. */
    sprintf(buf, "KICK: Player %s has been kicked by %s",
            kickee_name, kicker_name);
    LOG(llevInfo, "%s\n", buf);
    ndi(NDI_UNIQUE | NDI_ALL, 5, op, "%s is kicked out of the game.",
                  kickee_name);

    /* we kicked player params succesfull.
     * Now we give him a 1min temp ban, so he can
     * think about it.
     * If its a "technical" kick, the 10 sec is a protection.
     * Perhaps we want reset a map or whatever.
     */
    ticks = (int)(pticks_second * 60);
    add_ban_entry(NULL, params, NULL, ticks, ticks); // Note - obviously no need to call remove_ban_entry() here

    return COMMANDS_RTN_VAL_OK;
}

int command_reboot(object_t *op, char *params)
{
    char  *cp = NULL;
    shstr_t *subcommand = NULL;
    int    time = 300;

    if (!op ||
        op->type != PLAYER ||
        !CONTR(op))
    {
        return COMMANDS_RTN_VAL_ERROR;
    }

    /* No subcommand? Default to restart. */
    if (!params)
    {
        SHSTR_FREE_AND_ADD_STRING(subcommand, subcommands.restart);
    }
    /* Otherwise we need to make a working copy of params (because we may write
     * to it), and take the first word as the shstr_t subcommand. */
    else
    {
        char buf[MEDIUM_BUF];

        sprintf(buf, "%s", params);

        if ((cp = strchr(buf, ' ')))
        {
            *(cp++) = '\0';

            while (*cp == ' ')
            {
                cp++;
            }
        }

        SHSTR_FREE_AND_ADD_STRING(subcommand, buf);
    }

    /* Cancel scheduled reboot. */
    if (subcommand == subcommands.cancel)
    {
        SHSTR_FREE(subcommand);
        shutdown_agent(-2, SERVER_EXIT_NORMAL, CONTR(op), NULL);

        return COMMANDS_RTN_VAL_OK_SILENT;
    }
    /* Restart server. */
    else if (subcommand == subcommands.restart)
    {
        SHSTR_FREE(subcommand);
#if defined DAI_DEVELOPMENT_CODE
        time = 15;
#elif defined DAI_DEVELOPMENT_CONTENT
        time = 60;
#else
        time = 300;
#endif

        if (cp)
        {
            sscanf(cp, "%d", &time);
        }

        /* Gmasters below SA have a minimum reboot time of 30s. */
        if (!(CONTR(op)->gmaster_mode & GMASTER_MODE_SA) &&
            time < 30)
        {
            time = 30;
        }

        shutdown_agent(time, SERVER_EXIT_RESTART, CONTR(op),
                       "Server will recompile and arches and maps will be updated.");

        return COMMANDS_RTN_VAL_OK_SILENT;
    }
    /* Shutdown server. */
    else if (subcommand == subcommands.shutdown)
    {
        SHSTR_FREE(subcommand);

        /* Shutdown is special so we restrict it further (hopefully to people
         * who can start it up again). */
        if (!(CONTR(op)->gmaster_mode & GMASTER_MODE_SA))
        {
            ndi(NDI_UNIQUE, 0, op, "You have insufficient permission to shutdown the server.");

           return COMMANDS_RTN_VAL_ERROR;
        }

        time = 0;

        if (cp)
        {
            sscanf(cp, "%d", &time);
        }

        shutdown_agent(time, SERVER_EXIT_SHUTDOWN, CONTR(op),
                       "Server will shutdown and not reboot.");

        return COMMANDS_RTN_VAL_OK_SILENT;
    }

    /* Unknown subcommand. */
    SHSTR_FREE(subcommand);

    return COMMANDS_RTN_VAL_SYNTAX;
}

/* command_goto() causes the player op to relocate to the coordinates specified
 * in params. These should be of the form '<path>[ <x> <y>]'.
 *
 * <x> <y> default to -1 -1 and if outside the map (determined by <path>)
 * boundaries they are reset to that map's default entry coordinates.
 *
 * <path> determines the map. Currently this is only designed to work with
 * multiplayer or (in limited circumstances) unique maps (read apartments). Ie,
 * not instances.
 *
 * If <path> starts with '/' it is read as an absolute path in maps. Otherwise,
 * it is relative to the map op is currently on. Eg,
 * '/planes/demon_plane/world_0000' is absolute but 'world_0000' works the same
 * if op is already on /planes/demon_plane/world_1111. Perhaps more usefully,
 * 'underground_city/main_map_0000' also works and '../world_1111' to  get back.
 *
 * In any case these are always loaded as multiplayer maps.
 *
 * If no <path> is specified, op's save point is used.
 *
 * If <path> is @NAME then the named player's (if online) apt is used (a unique
 * map). Only GMs/SAs can goto apts. */
int command_goto(object_t *op, char *params)
{
    player_t    *pl;
    char       name[MAXPATHLEN] = {"\0"},
               buf[MAXPATHLEN];
    sint16     x = -1,
               y = -1;
    shstr_t     *orig_path_sh = NULL,
              *path_sh = NULL;
    uint32     flags = 0;
    map_t *m = NULL;
    msp_t  *msp;

    if (!op ||
        op->type != PLAYER ||
        !(pl = CONTR(op)))
    {
        return COMMANDS_RTN_VAL_ERROR;
    }

    if (params)
    {
        sscanf(params, "%s %hd %hd", name, &x, &y);
    }

    if (name[0] == '@')
    {
        player_t *other = find_player(name + 1);
        object_t *this,
               *next;

        if (!other)
        {
            ndi(NDI_UNIQUE, 0, op, "No such player!");

            return COMMANDS_RTN_VAL_OK_SILENT;
        }

        FOREACH_OBJECT_IN_OBJECT(this, other->ob, next)
        {
            if (this->name &&
                this->arch == archetype_global._player_info &&
                !strcmp(this->name, "APARTMENT_INFO"))
            {
                break;
            }
        }

        if (!this)
        {
            ndi(NDI_UNIQUE, 0, op, "|%s| has no apartment so you're not going there!",
                other->quick_name);

            return COMMANDS_RTN_VAL_OK_SILENT;
        }

        orig_path_sh = create_safe_path_sh(this->title);
        path_sh = create_unique_path_sh(other->ob->name, orig_path_sh);
        flags = MAP_STATUS_UNIQUE;
        m = ready_map_name(path_sh, orig_path_sh, flags, op->name);
        SHSTR_FREE(orig_path_sh);
        SHSTR_FREE(path_sh);

        if (!m)
        {
            ndi(NDI_UNIQUE, 0, op, "Map '%s' does not exist! You're going nowhere!",
                buf);
            return COMMANDS_RTN_VAL_ERROR;
        }
        else if ((m->status & MAP_STATUS_UNIQUE) &&
                 m->reference != op->name &&
                 !(pl->gmaster_mode & (GMASTER_MODE_SA | GMASTER_MODE_GM)))
        {
            ndi(NDI_UNIQUE, 0, op, "You don't have permission to enter someone else's apartment!");
            return COMMANDS_RTN_VAL_OK_SILENT;
        }
    }
    else
    {
        /* No path: Goto savebed. */
        if (name[0] == '\0')
        {
            SHSTR_FREE_AND_ADD_STRING(orig_path_sh, CONTR(op)->orig_savebed_map);
            SHSTR_FREE_AND_ADD_STRING(path_sh, CONTR(op)->savebed_map);
            flags = CONTR(op)->bed_status;
            x = CONTR(op)->bed_x;
            y = CONTR(op)->bed_y;
        }
        /* Absolute. */
        else if (name[0] == '/')
        {
            if (check_path(normalize_path("/", name, buf), 1) != -1)
            {
                orig_path_sh = create_safe_path_sh(buf);
                SHSTR_FREE_AND_ADD_REF(path_sh, orig_path_sh);
                flags = MAP_STATUS_MULTI;
            }
        }
        /* Relative. */
        else
        {
            if (check_path(normalize_path(CONTR(op)->orig_map, name, buf), 1) != -1)
            {
                orig_path_sh = create_safe_path_sh(buf);
                SHSTR_FREE_AND_ADD_REF(path_sh, orig_path_sh);
                flags = MAP_STATUS_MULTI;
            }
        }

        if (path_sh)
        {
            m = ready_map_name(path_sh, orig_path_sh, flags, op->name);
            SHSTR_FREE(orig_path_sh);
            SHSTR_FREE(path_sh);
        }

        if (!m)
        {
            ndi(NDI_UNIQUE, 0, op, "Map '%s' does not exist! You're going nowhere!",
                buf);
            return COMMANDS_RTN_VAL_ERROR;
        }
    }

    if (OUT_OF_REAL_MAP(m, x, y))
    {
        x = MAP_ENTER_X(m);
        y = MAP_ENTER_Y(m);
    }

    msp = MSP_RAW(m, x, y);
    (void)enter_map(op, msp, NULL, OVERLAY_FORCE | OVERLAY_FIRST_AVAILABLE | OVERLAY_FIXED, 0);
    SHSTR_FREE(orig_path_sh);
    SHSTR_FREE(path_sh);
    return COMMANDS_RTN_VAL_OK_SILENT;
}

int command_create(object_t *op, char *params)
{
    return CreateObject(op, params, CREATE);
}

int command_generate(object_t *op, char *params)
{
    return CreateObject(op, params, GENERATE);
}

int command_spawn(object_t *op, char *params)
{
    return CreateObject(op, params, SPAWN);
}

/* TODO: Tidy. */
static int CreateObject(object_t *op, char *params, CreateMode_t mode)
{
    int        nrof = 1,
               magic = 0, set_magic = 0,
               i,
               allow_nrof_set = 0;
    char      *cp = params,
               str[MEDIUM_BUF],
              *start,
               var[LARGE_BUF] = "",
               val[LARGE_BUF] = "",
               buf[LARGE_BUF];
    object_t    *tmp = NULL;
    archetype_t *at;
    artifact_t  *art = NULL;

    if (!op || op->type != PLAYER)
        return COMMANDS_RTN_VAL_ERROR;

    if (!(cp = get_token(cp, str, 0)) &&
        str[0] == '\0')
        return COMMANDS_RTN_VAL_SYNTAX;

    // First parameter must be quantity, or blank
    if (sscanf(str, "%d", &nrof))
    {
        /* Constrain nrof to sensible values. */
        if (mode == CREATE)
            nrof = MAX(1, nrof); // Only constraint is min qty = 1
        else
            nrof = MAX(1, MIN(nrof, 100));

        // Second parameter may be magic bonus (only if quantity was specified), or blank
        cp = get_token(cp, str, 0);

        if (sscanf(str, "%d", &magic))
        {
            set_magic = 1;

            /* Constrain nrof to sensible values. */
            if (mode == CREATE)
                magic = MAX(-127, MIN(magic, 127));
            else
                magic = MAX(-10, MIN(magic, 10));

            // Next parameter *must* be arch name
            cp = get_token(cp, str, 0);

            if (str[0] == '\0')
                return COMMANDS_RTN_VAL_SYNTAX;
        }
    }

    // Browse the archetype_types for the name - perhaps it is a base item
    if ((at = find_archetype(str)) == NULL)
    {
        ndi(NDI_UNIQUE, 0, op, "No such archetype name: %s.", str);
        return COMMANDS_RTN_VAL_ERROR;
    }

    if (mode == GENERATE)
    {
        if (at->clone.type == MONSTER || at->clone.type == PLAYER)
        {
            ndi(NDI_UNIQUE, 0, op, "Generate cannot be used to create mobs.");
            return COMMANDS_RTN_VAL_ERROR;
        }
        else if (QUERY_FLAG(&at->clone, FLAG_SYS_OBJECT))
        {
            ndi(NDI_UNIQUE, 0, op, "Generate cannot be used to create system objects.");
            return COMMANDS_RTN_VAL_ERROR;
        }
        else if (QUERY_FLAG(&at->clone, FLAG_DONATION_ITEM))
        {
            ndi(NDI_UNIQUE, 0, op, "Generate cannot be used to create donation items.");
            return COMMANDS_RTN_VAL_ERROR;
        }
    }

    if (mode == SPAWN)
        if (at->clone.type != MONSTER) // Don't allow spawning of PLAYER objects
        {
            ndi(NDI_UNIQUE, 0, op, "Spawn must only be used to create mobs.");
            return COMMANDS_RTN_VAL_ERROR;
        }

    // Does the arch definition allow direct set of nrof?
    if (at->clone.nrof)
       allow_nrof_set = 1;

    /* Now, start to create the object ...
     * If the base arch definition allows direct set of nrof, we only run
     * the loop once and create an item stack; else we run it nrof times
     * and create individual objects */
    for (i = 0, start = cp; i < (allow_nrof_set ? 1 : nrof); i++, cp = start)
    {
        archetype_t      *atmp;
        object_t*prev =   NULL, *head = NULL;

        // Create head and, if they exist, tail (more) objects
        for (atmp = at; atmp != NULL; atmp = atmp->more)
        {
            tmp = arch_to_object(atmp);
            if (head == NULL)
                head = tmp;  // For simple objects, we'll only get head

            tmp->x = op->x + tmp->arch->clone.x;
            tmp->y = op->y + tmp->arch->clone.y;
            tmp->map = op->map;

            if (allow_nrof_set)
                tmp->nrof = nrof;

            if (set_magic)
                set_abs_magic(tmp, magic);

            // Read the remaining attribute / value pairs
            do
            {
	        cp = get_token(cp, str, 0);

                if (str[0] == '\0')
                    break;
                // amask is handled differently to other attributes
                else if (!strcmp(str, "amask"))
                {
                    cp = get_token(cp, str, 0);

                    if (str[0] == '\0')
                        return COMMANDS_RTN_VAL_SYNTAX;

                    // Does this arch actually have any artifacts?
                    if (find_artifactlist(at->clone.type) == NULL)
                        ndi(NDI_UNIQUE, 0, op, "No artifact list for type %d\n",
                                      at->clone.type);
                    else
                    {
                        // Only allowed to apply one amask to each arch (I think)
                        if (!art)
                        {
                            art = find_artifact(str);

                            if (art)
                                if(legal_artifact_combination(tmp, art))
                                    give_artifact_abilities(tmp, art);
                                else
                                {
                                    ndi(NDI_UNIQUE, 0, op, "Illegal artifact combination ([%d] of %s)",
                                                         at->clone.type, str);
                                    art = NULL;
                                }
                            else
                                ndi(NDI_UNIQUE, 0, op, "No such artifact ([%d] of %s)",
                                                     at->clone.type, str);


                        }
                    }
                }  // end of str == amask
                // Type cannot be set with these commands.
                else if (!strcmp(str, "type"))
                {
                    ndi(NDI_UNIQUE, 0, op, "Cannot set type -- choose a different archetype instead!");

                    return COMMANDS_RTN_VAL_ERROR;
                }
                else
                {  // any other parameter
                    strcpy(var, str);  // Save the variable name
                    cp = get_token(cp, str, 1);

                    if (str[0] == '\0')
                        return COMMANDS_RTN_VAL_SYNTAX;

                    strcpy(val, str);  // Save the value

                    // Check for restrictions
                    if (CheckAttributeValue(var, val, mode))
                    {
                        // set_variable needs param to look like "param value"
                        sprintf(buf, "%s %s", var, val);

                        if (set_variable(tmp, buf) == -1)
                            ndi(NDI_UNIQUE, 0, op, "Unknown variable '%s'", var);
                        else
                            ndi(NDI_UNIQUE, 0, op, "(%s#%d)->%s=%s",
                                          tmp->name, tmp->count, var, val);
                    }
                    else
                        ndi(NDI_UNIQUE, 0, op, "Not allowed to set variable '%s'", var);
                }
            }  // end of reading parameter string
            while (cp);

            if (need_identify(tmp))
            {
                SET_FLAG(tmp, FLAG_IDENTIFIED);
                CLEAR_FLAG(tmp, FLAG_KNOWN_MAGICAL);
            }

            if (head != tmp)
                tmp->head = head,prev->more = tmp;

            prev = tmp;
        }  // end of for-loop for reading head and ->more objects

        if (at->clone.randomitems)
            create_treasure_list(at->clone.randomitems, head, GT_APPLY,
                                 (head->type == MONSTER) ? head->level : get_enviroment_level(head),
                                 ART_CHANCE_UNSET, 0);

        if (IS_LIVE(head))
        {
            if(head->type == MONSTER)
            {
                adjust_monster(head);
                fix_monster(head);
            }

            insert_ob_in_map(head, op->map, op, INS_NO_MERGE | INS_NO_WALK_ON);
        }
        else
        {
            if (head->more)
            {
                ndi(NDI_UNIQUE, 0, op, "Sorry - can't create multipart items!");
                return COMMANDS_RTN_VAL_ERROR;
            }

            head = insert_ob_in_ob(head, op);
        }
    }

    return COMMANDS_RTN_VAL_OK;
}

static int CheckAttributeValue(char *var, char *val, CreateMode_t mode)
{
    int i;

    if (!strcmp(var, "name"))
        return 1; // Everyone can do this

    if (!strcmp(var, "title"))
        return 1; // Everyone can do this

    if (!strcmp(var, "level"))
    {
        if (sscanf(val, "%d", &i) == 1)
        {
            i = MAX(1, MIN(i, 127));  // Apply some sensible limits to i
            sprintf(val, "%d", i);    // and put i back into val, incase i was just changed

            if (mode == CREATE) return 1;
            if (mode == SPAWN) return 1;
        }
    }

    if (!strcmp(var, "item_condition"))
    {
        if (sscanf(val, "%d", &i) == 1)
        {
            i = MAX(1, MIN(i, 200));
            sprintf(val, "%d", i);

            if (mode == CREATE) return 1;
            if (mode == SPAWN) return 1;
        }
    }

    if (!strstr(var, "attack_") || !strstr(var, "resist_"))
    {
        // We won't bother checking for valid attack/resist types,
        // as this is done by set_variable() later anyway
        if (sscanf(val, "%d", &i) == 1)
        {
            i = MAX(-100, MIN(i, 100));
            sprintf(val, "%d", i);

            if (mode == CREATE) return 1;
            if (mode == SPAWN) return 1;
        }
    }

    // Anything else can be set by Create command
    // Note:  We don't do this check first, as we wanted to do the checks on valid level, etc.
    if (mode == CREATE) return 1;

    return 0;
}

/* List all the available object names; helps player when using /create
 * Restrict the print out to objects of a certain type */
int command_listarch(object_t *op, char *params)
{
    int             atype;
    archetype_t      *at;
    artifactlist_t   *al;
    artifact_t       *art = NULL;
    char            buf[MEDIUM_BUF] = "";

    /* This command runs too slowly / sends too much data to client in one go,
     * so is temporarily removed for main servers - I will try to re-write later
     * ~ Torchwood Feb 2012 */
#ifndef DAI_DEVELOPMENT_CONTENT
    return COMMANDS_RTN_VAL_OK_SILENT;
#else

    if (!op || op->type != PLAYER)
        return COMMANDS_RTN_VAL_ERROR;

    if (!params || (sscanf(params, "%d", &atype) != 1))
        return COMMANDS_RTN_VAL_SYNTAX;

    // For info: This code was 'inspired' by the dump_arch code

    for (at = first_archetype; at != NULL; at = (at->more == NULL) ? at->next : at->more)
    {
        if (at->clone.type == atype)
        {
            // Print the results in batches
            if (strlen(buf) + strlen(STRING_OBJ_ARCH_NAME(&at->clone)) > 42)
            {
                ndi(NDI_UNIQUE | NDI_WHITE, 0, op, "%s", buf);
                buf[0] = '\0';
            }
            sprintf(strchr(buf, '\0'), " %s ~/~", STRING_OBJ_ARCH_NAME(&at->clone));
        }
    }

    // Print out the last batch ...
    ndi(NDI_UNIQUE | NDI_WHITE, 0, op, "%s", buf);
    buf[0] = '\0';

    for (al = first_artifactlist; al != NULL; al = al->next)
    {
        art = al->items;
        do
        {
            if(art->flags&ARTIFACT_FLAG_HAS_DEF_ARCH )
            {
                if (art->def_at.clone.type == atype)
                {
                    // Print the results in batches
                    if (strlen(buf) + strlen(STRING_OBJ_ARCH_NAME(&art->def_at.clone)) > 42)
                    {
                        ndi(NDI_UNIQUE | NDI_WHITE, 0, op, "%s", buf);
                        buf[0] = '\0';
                    }
                    sprintf(strchr(buf, '\0'), " %s ~/~", STRING_OBJ_ARCH_NAME(&art->def_at.clone));
                }
            }
            art = art->next;
        }
        while (art != NULL);
    }

    ndi(NDI_UNIQUE | NDI_WHITE, 0, op, "%s", buf);

    return COMMANDS_RTN_VAL_OK;
#endif
}

#ifndef USE_CHANNELS
int command_mutelevel(object_t *op, char *params)
{
    int lvl = 0;

    if (!op)
        return COMMANDS_RTN_VAL_ERROR;

    if (!params || (sscanf(params, "%d", &lvl) != 1))
        return COMMANDS_RTN_VAL_SYNTAX;

    if ((CONTR(op)->gmaster_mode & GMASTER_MODE_VOL) &&
        !(CONTR(op)->gmaster_mode & (GMASTER_MODE_SA | GMASTER_MODE_GM)) &&
        lvl > 10)
    {
        ndi(NDI_UNIQUE, 0, op, "WARNING: Maximum mutelevel for VOLs is 10.");
        lvl = 10;
    }

    settings.mutelevel = lvl;
    return COMMANDS_RTN_VAL_OK;
}
#else
// mutelevel does nothing if channels are in use
int command_mutelevel(object_t *op, char *params)
{
    if (!op)
        return COMMANDS_RTN_VAL_ERROR;

    ndi(NDI_UNIQUE, 0, op, "Mutelevel command is non-functional when channels are in use.");

    return COMMANDS_RTN_VAL_OK_SILENT;
}
#endif

int command_connections(object_t *op, char *params)
{
    int nr = 2;

    if (!op)
        return COMMANDS_RTN_VAL_ERROR;

    if (!params || (sscanf(params, "%d", &nr) != 1))
        return COMMANDS_RTN_VAL_SYNTAX;

    if(nr < 2)
    {
        ndi(NDI_UNIQUE, 0, op, "WARNING: Miminum connections from single IP must be at least 2.");
        nr = 2;
    }

    settings.max_cons_from_one_ip = nr;
    return COMMANDS_RTN_VAL_OK;
}

int command_summon(object_t *op, char *params)
{
    player_t   *pl;
    msp_t *msp;

    if (!op ||
        op->type != PLAYER ||
        !CONTR(op))
    {
        return COMMANDS_RTN_VAL_ERROR;
    }
    else if (!params)
    {
        return COMMANDS_RTN_VAL_SYNTAX;
    }
    else if (!(pl = find_player(params)))
    {
        ndi(NDI_UNIQUE, 0, op, "No such player.");
        return COMMANDS_RTN_VAL_ERROR;
    }
    else if (pl->ob == op)
    {
        ndi(NDI_UNIQUE, 0, op, "You can't summon yourself next to yourself.");
        return COMMANDS_RTN_VAL_ERROR;
    }
    else if (!(pl->state & ST_PLAYING))
    {
        ndi(NDI_UNIQUE, 0, op, "That player can't be summoned right now.");
        return COMMANDS_RTN_VAL_ERROR;
    }

    msp = MSP_KNOWN(op);

    if (enter_map(pl->ob, msp, op, OVERLAY_WITHIN_LOS | OVERLAY_FIRST_AVAILABLE, 0) == MOVE_RETURN_SUCCESS)
    {
        ndi(NDI_UNIQUE, 0, pl->ob, "You are summoned.");
        ndi(NDI_UNIQUE, 0, op, "OK.");
    }

    return COMMANDS_RTN_VAL_OK;
}

int command_teleport(object_t *op, char *params)
{
    player_t   *pl,
             *other;
    msp_t *msp;

    if (!op ||
        op->type != PLAYER ||
        !(pl = CONTR(op)))
    {
        return COMMANDS_RTN_VAL_ERROR;
    }
    else if (!params)
    {
        return COMMANDS_RTN_VAL_SYNTAX;
    }
    else if (!(other = find_player(params)))
    {
        ndi(NDI_UNIQUE, 0, op, "No such player!");
        return COMMANDS_RTN_VAL_ERROR;
    }
    else if (other->ob == op)
    {
        ndi(NDI_UNIQUE, 0, op, "You can't teleport yourself next to yourself!");
        return COMMANDS_RTN_VAL_ERROR;
    }
    else if (!(other->state & ST_PLAYING))
    {
        ndi(NDI_UNIQUE, 0, op, "You can't teleport to that player right now!");
        return COMMANDS_RTN_VAL_ERROR;
    }
    else if ((other->ob->map->status & MAP_STATUS_UNIQUE) &&
             other->ob->map->reference != op->name &&
             !(pl->gmaster_mode & (GMASTER_MODE_SA | GMASTER_MODE_GM)))
    {
        ndi(NDI_UNIQUE, 0, op, "You don't have permission to enter someone else's apartment!");
        return COMMANDS_RTN_VAL_ERROR;
    }

    msp = MSP_KNOWN(other->ob);
    (void)enter_map(op, msp, NULL, OVERLAY_FIRST_AVAILABLE, 0);
    ndi(NDI_UNIQUE, 0, op, "OK.");
    return COMMANDS_RTN_VAL_OK_SILENT;
}

/* FIXME: Badly written and broken. */
int command_inventory(object_t *op, char *params)
{
    object_t *inv = NULL;
    int     i;
    object_t *tmp;
    char   *in;
    int     items = 0, length;

    if (!op)
        return COMMANDS_RTN_VAL_ERROR;

    // TODO - find_object() is not yet implemented, so will always return NULL
    // Manual file only shows /inventory, no mention of parameters, until this is done
    /* TODO: Should scan params for a player name and do find_player() -- the
     * whole point of this function. */
    if (params &&
        (!sscanf(params, "%d", &i) ||
         !(inv = find_object(i))))
    {
        return COMMANDS_RTN_VAL_SYNTAX;
    }

    tmp = inv ? inv->inv : op->inv;

    while (tmp)
    {
        if ((!QUERY_FLAG(tmp, FLAG_SYS_OBJECT) &&
             (inv == NULL ||
              inv->type == CONTAINER ||
              QUERY_FLAG(tmp, FLAG_APPLIED))))
        {
            items++;
        }

        tmp = tmp->below;
    }
    if (inv == NULL)
    {
        /* player's inventory */
        if (items == 0)
        {
            ndi(NDI_UNIQUE, 0, op, "You carry nothing.");
            return COMMANDS_RTN_VAL_OK_SILENT;
        }
        else
        {
            length = 28;
            in = "";
            ndi(NDI_UNIQUE, 0, op, "Inventory:");
        }
    }
    else
    {
        if (items == 0)
            return COMMANDS_RTN_VAL_OK_SILENT;
        else
        {
            length = 28;
            in = "  ";
        }
    }

    if (op)
    {
        object_t *next;

        if (!inv)
        {
            inv = op;
        }

        FOREACH_OBJECT_IN_OBJECT(tmp, inv, next)
        {
            if ((QUERY_FLAG(tmp, FLAG_SYS_OBJECT) ||
                 (inv &&
                  inv->type != CONTAINER &&
                  !QUERY_FLAG(tmp, FLAG_APPLIED))))
            {
                continue;
            }

            ndi(NDI_UNIQUE, 0, op, "%s- %s[%d] %6.1f",
                in, query_name(tmp, op, ARTICLE_NONE, 1), TAG(tmp),
                (float)WEIGHT_OVERALL(tmp) / 1000.0);
        }

        if (!inv)
        {
            ndi(NDI_UNIQUE, 0, op, "Total weight: %6.1f",
                (float)WEIGHT_OVERALL(tmp) / 1000.0);
        }
    }

    return COMMANDS_RTN_VAL_OK_SILENT;
}


int command_dump(object_t *op, char *params)
{
    int     i;
    object_t *tmp;

    if (!op)
        return COMMANDS_RTN_VAL_ERROR;

    // TODO - find_object() is not yet implemented, so will always return NULL
    // Manual file only shows /dump me, until this is done
    if (params && !strcmp(params, "me"))
        tmp = op;
    else if (!params ||
             !sscanf(params, "%d", &i) ||
             !(tmp = find_object(i)))
        return COMMANDS_RTN_VAL_SYNTAX;

    dump_object(tmp);
    ndi(NDI_UNIQUE, 0, op, "%s", errmsg);

    return COMMANDS_RTN_VAL_OK_SILENT;
}

static int AddExp(object_t *op, char *params, const char *command)
{
    int     n;
    char    name[MEDIUM_BUF];
    int     snr,
            exp;
    player_t *pl;

    if (!params ||
        (n = sscanf(params, "%s %d %d", name, &snr, &exp)) == 0)
    {
        ndi(NDI_UNIQUE, 0, op, "Usage: %s <name> <skill> <%s>",
            command, (*command == 'a') ? "exp" : "level");

        return COMMANDS_RTN_VAL_OK_SILENT;
    }

    if (!(pl = find_player(name)))
    {
        ndi(NDI_UNIQUE, 0, op, "No such player.");

        return COMMANDS_RTN_VAL_ERROR;
    }

    if (n >= 2 &&
        (snr < 0 ||
         snr >= NROFSKILLS))
    {
        ndi(NDI_UNIQUE, 0, op, "No such skill.");

        return COMMANDS_RTN_VAL_ERROR;
    }

    if (n <= 2)
    {
        int  i;

        for (i = 0; i < NROFSKILLS; i++)
        {
            if (pl->skill_ptr[i])
            {
                ndi(NDI_UNIQUE | NDI_GREEN, 0, op, "%d\t%s (%d/%d)",
                    i, skills[i]->clone.name, pl->skill_ptr[i]->stats.exp,
                    pl->skill_ptr[i]->level);
            }
            else
            {
                ndi(NDI_UNIQUE | NDI_WHITE, 0, op, "%d\t%s",
                    i, skills[i]->clone.name);
            }
        }

        return COMMANDS_RTN_VAL_OK_SILENT;
    }

    if (*command == 's')
    {
        int level = MAX(0, MIN(exp, MAXLEVEL));
        object_t *skill = pl->skill_ptr[snr];

        if (!skill) /* we don't have the skill - learn it*/
        {
            learn_skill(pl->ob, snr);
            skill = pl->skill_ptr[snr];
        }
        else if (!level)/* if level is 0 we unlearn the skill! */
        {


            return COMMANDS_RTN_VAL_OK;
        }

        exp = new_levels[level] - skill->stats.exp;
    }

    (void)add_exp(pl->ob, exp, snr, 0);

    return COMMANDS_RTN_VAL_OK;
}

/* Similar to addexp, but we set here the skill level explicit
 * If the player doesn't have the skill, we add it.
 * if level is 0 we remove the skill (careful!!)
*/
int command_setskill(object_t *op, char *params)
{
    return AddExp(op, params, "setskill");
}

int command_addexp(object_t *op, char *params)
{
    return AddExp(op, params, "addexp");
}

/* '/serverspeed' reports the current server speed.
 * '/serverspeed <number>' sets server speed to <number>. */
int command_serverspeed(object_t *op, char *params)
{
    sint32 i;

    if (!params)
    {
        ndi(NDI_UNIQUE, 0, op, "Current server speed is %u ums per round (%u rounds per second)",
            pticks_ums, pticks_second);
        return COMMANDS_RTN_VAL_OK_SILENT;
    }

    if (!sscanf(params, "%d", &i))
    {
        return COMMANDS_RTN_VAL_SYNTAX;
    }

    set_pticks_time(i);
    GETTIMEOFDAY(&last_time);
    ndi(NDI_UNIQUE, 0, op, "Set server speed to %u ums per round (%u rounds per second)",
        pticks_ums, pticks_second);
    return COMMANDS_RTN_VAL_OK;
}

/**************************************************************************/
/* Mods made by Tyler Van Gorder, May 10-13, 1992.                        */
/* CSUChico : tvangod@cscihp.ecst.csuchico.edu                            */
/**************************************************************************/

int command_stats(object_t *op, char *params)
{
    player_t *pl;

    if (!params)
        return COMMANDS_RTN_VAL_SYNTAX;

    if (!(pl = find_player(params)))
    {
        ndi(NDI_UNIQUE, 0, op, "No such player.");

        return COMMANDS_RTN_VAL_ERROR;
    }

    ndi(NDI_UNIQUE, 0, op,
                  "Str : %-2d      H.P. : %-4d  MAX : %d\n"
                  "Dex : %-2d      S.P. : %-4d  MAX : %d\n"
                  "Con : %-2d        AC : %-4d  WC  : %d\n"
                  "Int : %-2d       EXP : %d\n"
                  "Wis : %-2d      Food : %d\n"
                  "Pow : %-2d    Damage : %d\n"
                  "Cha : %-2d     Grace : %d",
                  pl->ob->stats.Str, pl->ob->stats.hp, pl->ob->stats.maxhp,
                  pl->ob->stats.Dex, pl->ob->stats.sp, pl->ob->stats.maxsp,
                  pl->ob->stats.Con, pl->ob->stats.ac, pl->ob->stats.wc,
                  pl->ob->stats.Int, pl->ob->stats.exp,
                  pl->ob->stats.Wis, pl->ob->stats.food,
                  pl->ob->stats.Pow, pl->ob->stats.dam,
                  pl->ob->stats.Cha, pl->ob->stats.grace);

    return COMMANDS_RTN_VAL_OK_SILENT;
}

int command_setstat(object_t *op, char *params)
{
    char    player_name[20],
            stat_name[20];
    int     v;
    player_t *pl;
    shstr_t  *hash_name = NULL;

    if (!params ||
        sscanf(params, "%s %s %d", player_name, stat_name, &v) != 3)
    {
        return COMMANDS_RTN_VAL_SYNTAX;
    }

    if (!(pl = find_player(player_name)))
    {
        ndi(NDI_UNIQUE, 0, op, "No such player!");

        return COMMANDS_RTN_VAL_ERROR;
    }

    hash_name = shstr_add_string(stat_name);

    if (v < MIN_STAT ||
        v > MAX_STAT)
    {
        ndi(NDI_UNIQUE, 0, op, "Illegal range of stat!");

        return COMMANDS_RTN_VAL_ERROR;
    }

    if (hash_name == shstr_cons.stat_strength)
    {
        pl->orig_stats.Str = v;
    }
    else if (hash_name == shstr_cons.stat_dexterity)
    {
        pl->orig_stats.Dex = v;
    }
    else if (hash_name == shstr_cons.stat_constitution)
    {
        pl->orig_stats.Con = v;
    }
    else if (hash_name == shstr_cons.stat_intelligence)
    {
        pl->orig_stats.Int = v;
    }
    else if (hash_name == shstr_cons.stat_wisdom)
    {
        pl->orig_stats.Wis = v;
    }
    else if (hash_name == shstr_cons.stat_power)
    {
        pl->orig_stats.Pow = v;
    }
    else if (hash_name == shstr_cons.stat_charisma)
    {
        pl->orig_stats.Cha = v;
    }
    else
    {
        ndi(NDI_UNIQUE , 0, op,  "Unrecognised stat!");
        SHSTR_FREE(hash_name);

        return COMMANDS_RTN_VAL_ERROR;
    }

    SHSTR_FREE(hash_name);
    ndi(NDI_UNIQUE, 0, op, "%s has been altered.", pl->ob->name);
    FIX_PLAYER(pl->ob, "command setstat");

    return COMMANDS_RTN_VAL_OK;
}

/* Resets (reloads from source) a map. A countdown (seconds) and the absolute
 * path of the map to be reset may be specified. DDefaults are 0 and the map op
 * is on. */
int command_resetmap(object_t *op, char *params)
{
    map_t *m;
    sint32     secs = 0;
    char       path[MAXPATHLEN] = "";
    shstr_t     *path_sh = NULL;

    if (params)
    {
        if (sscanf(params, "%d %s", &secs, path) != 2)
        {
            if (sscanf(params, "%d", &secs) != 1)
            {
                sscanf(params, "%s", path);
            }
        }
    }

    if (path[0])
    {
        SHSTR_FREE_AND_ADD_STRING(path_sh, path);
    }
    else
    {
        SHSTR_FREE_AND_ADD_REF(path_sh, op->map->path);
    }

    m = map_is_in_memory(path_sh);
    SHSTR_FREE(path_sh);

    if (!m)
    {
        ndi(NDI_UNIQUE, 0, op, "No such map.");

        return COMMANDS_RTN_VAL_OK_SILENT;
    }

    m->status |= MAP_STATUS_MANUAL_RESET | MAP_STATUS_RELOAD;

    if (!secs)
    {
        MAP_SET_WHEN_RESET(m, -1);
        map_check_in_memory(m);
    }
    else
    {
        MAP_SET_WHEN_RESET(m, secs);
    }

    return COMMANDS_RTN_VAL_OK;
}

/* warning: these function is for heavy debugging.
 * Its somewhat useless under windows.
 */
int command_check_fd(object_t *op, char *params)
{
    struct stat buf;
    int         handle_max,
                fh;

    handle_max = (socket_info.max_filedescriptor < 100) ? 100 :
                 socket_info.max_filedescriptor;

    /* remember, max_filedescriptor don't works under windows */
    ndi(NDI_UNIQUE, 0, op, "check file handles from 0 to %d.",
                         handle_max);
    LOG(llevSystem, "check file handles from 0 to %d.",
        handle_max);

    for (fh = 0; fh <= handle_max; fh++)
    {
        /* Check if statistics are valid: */
        if (!fstat(fh, &buf))
        {
            /* no ttyname() under windows... well,
                     * debugging fh's is always more clever on linux.
                     */
#ifdef WIN32
            LOG(llevSystem, "FH %d ::(%d) size     : %ld\n",
                fh, isatty(fh), buf.st_size);
#else
            player_t *pp;
            char   *name1 = NULL;

            /* collect some senseless handle numbers... */
            for (pp = first_player; pp; pp = pp->next)
            {
                if (pp->socket.fd == fh)
                    break;
            }

            name1 = ttyname(isatty(fh));

            LOG(llevSystem, "FH %d ::(%s) (%s) size: %ld\n",
                fh, (name1) ? name1 : "><",
                (pp) ? ((pp->ob) ? STRING_OBJ_NAME(pp->ob) : ">player<") : "",
                buf.st_size);
#endif
        }
    }

    return COMMANDS_RTN_VAL_OK;
}

/* a muted player can't shout/say/tell/reply for the
 * amount of time.
 * we have 2 kinds of mute: shout/say & tell/gsay/reply.
 * a player can be muted through this command and/or
 * automatic by the spam agent.
 * note - this is a 'global' mute; channels also have their own 'local' mutes
 */
int command_mute(object_t *op, char *params)
{
    char        name[MEDIUM_BUF] = "";
    int         seconds = 0;
    player_t     *pl;

    if (!params)
        return COMMANDS_RTN_VAL_SYNTAX;

    sscanf(params, "%s %d", name, &seconds);

    if (!(pl = find_player(name)))
    {
        ndi(NDI_UNIQUE, 0, op, "mute command: can't find player %s",
                             name);

        return COMMANDS_RTN_VAL_ERROR;
    }

    if(seconds < 0)
    {
        ndi(NDI_UNIQUE, 0, op, "mute command: illegal seconds parameter (%d)",
                             seconds);

        return COMMANDS_RTN_VAL_ERROR;
    }

    if(!seconds) /* unmute player */
    {
        ndi(NDI_UNIQUE, 0, op, "mute command: unmuting player %s!",
                             name);
        pl->mute_counter = 0;
    }
    else
        pl->mute_counter = pticks + seconds * (1000000 / MAX_TIME);

    return COMMANDS_RTN_VAL_OK;
}

/* a silenced player can't shout or tell or say
 * but the he don't know it. A own shout will be shown
 * to him as normal but not to others.
 */
int command_silence(object_t *op, char *params)
{
    return COMMANDS_RTN_VAL_OK;
}

/* /ban usage:
 *    /ban list
 *    /ban <+/-> <pl / ac / ch / ip> <str> <time>
 *    Note:  /ban pl will ban account, character and IP. Player must be online.
 *    Note:  <seconds> = -1 means permanent ban (GMs and SAs only) */
/* TODO: Tidy. */
int command_ban(object_t *op, char *params)
{
    int            pos = 0, s = 60;  // Default ban is 1 minute
    char          *cp = params,
                   str[MEDIUM_BUF],
                   b_mode[MEDIUM_BUF] = "",
                   b_type[MEDIUM_BUF] = "",
                   b_str[MEDIUM_BUF] = "", // ban mode, ban type, etc.
                   t_mod[MEDIUM_BUF] = "";
    ENUM_BAN_TYPE  ban_type;

    if (!op)
        return COMMANDS_RTN_VAL_ERROR;

    if (!(cp = get_token(cp, str, 0)) ||
        str[0] == '\0')
        return COMMANDS_RTN_VAL_SYNTAX;

    strcpy(b_mode, str);

    if (!strcmp(b_mode, "list"))
    {
        ndi(NDI_UNIQUE, 0, op, "Ban List");
        ndi(NDI_UNIQUE, 0, op, "--- --- ---");

        BanList(op, BANTYPE_ACCOUNT); // Print list of banned names
        BanList(op, BANTYPE_CHAR); // Print list of banned names
        BanList(op, BANTYPE_IP);  // Print list of banned IPs

        return COMMANDS_RTN_VAL_OK_SILENT;
    }

    // mode must be is + or -
    if (strcmp(b_mode, "+") && strcmp(b_mode, "-"))
        return COMMANDS_RTN_VAL_SYNTAX;

    cp = get_token(cp, str, 0);

    if (str[0] == '\0')
        return COMMANDS_RTN_VAL_SYNTAX;

    strcpy(b_type, str);

    // type must be pl, ac, ch, ip
    // player - a person, ac - account, ch - character in ac, ip - ip
    if (!strcmp(b_type, "pl"))
        ban_type = BANTYPE_PLAYER;
    else if (!strcmp(b_type, "ac"))
        ban_type = BANTYPE_ACCOUNT;
    else if (!strcmp(b_type, "ch"))
        ban_type = BANTYPE_CHAR;
    else if (!strcmp(b_type, "ip"))
        ban_type = BANTYPE_IP;
    else
        return COMMANDS_RTN_VAL_SYNTAX;

    // now get the player name
    cp = get_token(cp, str, 0);

    if (str[0] == '\0')
        return COMMANDS_RTN_VAL_SYNTAX;

    strcpy(b_str, str);

    if (!strcmp(b_mode, "+"))
    {
        // Get time if specified
        cp = get_token(cp, str, 0);

        if (sscanf(str, "%d%s", &s, t_mod))
        {
            if (!strcmp(t_mod, "d"))
                s *= 24*60*60;
            else if (!strcmp(t_mod, "h"))
                s *= 60*60;
            else if (!strcmp(t_mod, "m"))
                s *= 60;
            else if (!strcmp(t_mod, "s"))
                s *= 1;
            else if (t_mod[0] == '\0')
                s *= 1;
            else
                return COMMANDS_RTN_VAL_SYNTAX;

            if (s < -1) s = -1;
        }

        return BanAdd(op, ban_type, b_str, s);
    }
    else if (!strcmp(b_mode, "-"))
        return BanRemove(op, ban_type, b_str);
    else
        return COMMANDS_RTN_VAL_SYNTAX;
}

/* Helper functions for the main ban command */
static int BanList(object_t *op, ENUM_BAN_TYPE ban_type)
{
    objectlink_t *ol;

    ndi(NDI_UNIQUE, 0, op, "%s",ban_type == BANTYPE_ACCOUNT ? "~Accounts~" :
                                          ban_type == BANTYPE_CHAR ? "~Characters~" : "~IPs~");

    for(ol = (ban_type == BANTYPE_ACCOUNT ? ban_list_account :
              ban_type == BANTYPE_CHAR ? ban_list_player : ban_list_ip); ol; ol = ol->next)
    {
        if (ol->objlink.ban->ticks_init != -1 &&
            pticks >= ol->objlink.ban->ticks)
        {
            remove_ban_entry(ol); /* is not valid anymore, gc it on the fly */
        }
        else
        {
            if (ol->objlink.ban->ticks_init == -1)
            {
                ndi(NDI_UNIQUE, 0, op, "%s -> Permanently banned",
                              (ban_type == BANTYPE_ACCOUNT ? ol->objlink.ban->account :
                               ban_type == BANTYPE_CHAR ? ol->objlink.ban->name : ol->objlink.ban->ip));
            }
            else
            {
                /* FIXME: Bans should rely on real time, not server rounds (pticks). */
                ndi(NDI_UNIQUE, 0, op, "%s -> %u left (of %d) sec",
                              (ban_type == BANTYPE_ACCOUNT ? ol->objlink.ban->account :
                               ban_type == BANTYPE_CHAR ? ol->objlink.ban->name : ol->objlink.ban->ip),
                              (ol->objlink.ban->ticks - pticks) / 8,
                              (ol->objlink.ban->ticks_init) / 8);
            }
        }
    }

    return COMMANDS_RTN_VAL_OK_SILENT;
}

/* Add a player to the ban list, using account, name or IP, or PL (which does all 3) */
static int BanAdd(object_t *op, ENUM_BAN_TYPE ban_type, char *str, int s)
{
    int tmp;

    /* Check for permanent ban restrictions */
    if (!(CONTR(op)->gmaster_mode & (GMASTER_MODE_SA | GMASTER_MODE_GM)) &&
         (s == -1))
    {
        ndi(NDI_UNIQUE, 0, op, "Only GMs and SAs can permanently ban or unban!");
        return COMMANDS_RTN_VAL_ERROR;
    }

    if (ban_type == BANTYPE_PLAYER)
    {
        player_t     *pl = find_player(str);   // str must be the character name
        char        ac_name[MEDIUM_BUF] = "";

        /* For "pl" command, player must be online ... */
        if (!pl)
        {
            ndi(NDI_UNIQUE, 0, op,
                          "Can't find the player %s!\nCheck spelling, or ban the account, character or IP directly.",
                          STRING_SAFE(str));

            return COMMANDS_RTN_VAL_ERROR;
        }

        sprintf(ac_name, "%s", pl->account_name);  // need a simple, non-shared string

       /* add account to the ban list */
        tmp = BanAddToBanList(op, BANTYPE_ACCOUNT, ac_name, s);
        if ((tmp == COMMANDS_RTN_VAL_SYNTAX) || (tmp == COMMANDS_RTN_VAL_ERROR))
            return tmp;

       /* add char name to the ban list */
        tmp = BanAddToBanList(op, BANTYPE_CHAR, str, s);
        if ((tmp == COMMANDS_RTN_VAL_SYNTAX) || (tmp == COMMANDS_RTN_VAL_ERROR))
            return tmp;

        /* add ip to the ban list */
        tmp = BanAddToBanList(op, BANTYPE_IP, pl->socket.ip_host, s);
        if ((tmp == COMMANDS_RTN_VAL_SYNTAX) || (tmp == COMMANDS_RTN_VAL_ERROR))
            return tmp;

        kick_player(pl);
    }
    else if (ban_type == BANTYPE_ACCOUNT)
    {
        Account *ac;
        player_t  *pl;
        int      i;

       /* add account to the ban list */
        tmp = BanAddToBanList(op, BANTYPE_ACCOUNT, str, s);
        if ((tmp == COMMANDS_RTN_VAL_SYNTAX) || (tmp == COMMANDS_RTN_VAL_ERROR))
            return tmp;

        // See if this account is online, and if so kick all its characters
        if ((ac = find_account(str)))
        {
            for(i=0; i < ACCOUNT_MAX_PLAYER; i++)
            {
                if(ac->level[i])
                {
                    if ((pl = find_player(ac->charname[i])))
                        // {}
                        kick_player(pl);
                        /* We can't kick the player; if there is more than 1 player
                         * on this IP, and we kick them both, we crash the server! */
                }
            }
        }
    }
    else if (ban_type == BANTYPE_CHAR)
    {
        player_t *pl;

        /* add name to the ban list */
        tmp = BanAddToBanList(op, BANTYPE_CHAR, str, s);
        if ((tmp == COMMANDS_RTN_VAL_SYNTAX) || (tmp == COMMANDS_RTN_VAL_ERROR))
            return tmp;

        if ((pl = find_player(str)))
            kick_player(pl);
    }
    else /* ban_type == BANTYPE_IP */
    {
        unsigned char  ip_terms[16];
        int            mask_pos[2];
        int            allowed = 1;
        objectlink_t    *ip_list,
                      *ol;
        player_t        *pl = NULL;

        if (!parse_ip(str, ip_terms, mask_pos))
            return COMMANDS_RTN_VAL_SYNTAX;

        /* VOL can only ban specific address,
         * GM can ban x.x.x.*,
         * SA can ban x.x.*.x
         */

        /* Do we have a mask set? */
        if (mask_pos[0] != -1)
        {
            if (CONTR(op)->gmaster_mode & GMASTER_MODE_SA)
            {
                /* This only works for IPv4 !! */
                if (mask_pos[0] < 14) allowed = 0;
            }
            else if (CONTR(op)->gmaster_mode & GMASTER_MODE_GM)
            {
                if (mask_pos[0] < 15) allowed = 0;
            }
            else
            {
                allowed = 0;
            }

            if (!allowed)
            {
                ndi(NDI_UNIQUE, 0, op, "You have insufficient privileges to ban that IP range.");
                return COMMANDS_RTN_VAL_ERROR;
            }
        }

        tmp = BanAddToBanList(op, BANTYPE_IP, str, s);
        if ((tmp == COMMANDS_RTN_VAL_SYNTAX) || (tmp == COMMANDS_RTN_VAL_ERROR))
            return tmp;

        /* Kick all players who have this IP */
        ip_list = find_players_on_ip(str);

        for (ol = ip_list ; ol; ol = ol->next)
        {
            pl = CONTR(ol->objlink.ob);

            if (pl)
                // {}
                kick_player(pl);
                /* We can't kick the player; if there is more than 1 player
                 * on this IP, and we kick them both, we crash the server! */
        }

        /* Now clear out our temporary list */
        free_iplist(ip_list);
    }

    save_ban_file();

    return COMMANDS_RTN_VAL_OK;
}

static int BanAddToBanList(object_t *op, ENUM_BAN_TYPE ban_type, char *str, int s)
{
    char buf[SMALL_BUF];
    char ban_buf[SMALL_BUF];
    int  tmp;

    // First, remove the existing entry, if it exists (might not!)
    tmp = BanRemoveFromBanList(op, ban_type, str, 1);
    if ((tmp == COMMANDS_RTN_VAL_SYNTAX) || (tmp == COMMANDS_RTN_VAL_ERROR))
        return tmp;

    if (s != -1)
        if (s < (2*60))
            sprintf(ban_buf, "for %d seconds.", s);
        else if (s < (2*60*60))
            sprintf(ban_buf, "for %d minutes.", s/60);
        else if (s < (2*60*60*24))
            sprintf(ban_buf, "for %d hours.", s/(60*60));
        else
            sprintf(ban_buf, "for %d days.", s/(60*60*24));
    else
        sprintf(ban_buf, "permanently.");

    if (ban_type == BANTYPE_ACCOUNT)
        sprintf(buf, "Account %s is now banned %s", str, ban_buf);
    else if (ban_type == BANTYPE_CHAR)
        sprintf(buf, "Character %s is now banned %s", str, ban_buf);
    else
        sprintf(buf, "IP %s is now banned %s", str, ban_buf);

    ndi(NDI_UNIQUE, 0, op, "%s", buf);

    if (s != -1)
        s *= 8;     /* convert seconds to ticks */

    if (ban_type == BANTYPE_ACCOUNT)
        add_ban_entry(str, NULL, NULL, s, s);
    else if (ban_type == BANTYPE_CHAR)
        add_ban_entry(NULL, str, NULL, s, s);
    else
        add_ban_entry(NULL, NULL, str, s, s);

    return COMMANDS_RTN_VAL_OK;
}

/* Remove account, name or IP from ban lists */
static int BanRemove(object_t *op, ENUM_BAN_TYPE ban_type, char *str)
{
    int tmp;

    tmp = BanRemoveFromBanList(op, ban_type, str, 0);

    if (tmp == COMMANDS_RTN_VAL_OK)
    {
        save_ban_file();
        return tmp;
    }
    else if ((tmp == COMMANDS_RTN_VAL_SYNTAX) || (tmp == COMMANDS_RTN_VAL_ERROR))
        return tmp;
    else
    {
        ndi(NDI_UNIQUE, 0, op, "No %s found to match %s on ban lists!",
                      ban_type == BANTYPE_ACCOUNT ? "account" :
                      ban_type == BANTYPE_CHAR ? "character" : "IP", str);
        return COMMANDS_RTN_VAL_ERROR;
    }
}

static int BanRemoveFromBanList(object_t *op, ENUM_BAN_TYPE ban_type, char *str, int mute)
{
    objectlink_t *ol;
    const char *str_hash = NULL;

    if (ban_type == BANTYPE_PLAYER)
        return COMMANDS_RTN_VAL_SYNTAX;

    else if (ban_type == BANTYPE_ACCOUNT)
    {
        transform_account_name_string(str);
        str_hash = shstr_find(str); /* we need an shared string to check ban list */
    }

    else if (ban_type == BANTYPE_CHAR)
    {
        transform_player_name_string(str);
        str_hash = shstr_find(str);
    }

    /* Either search the account, IP or name lists */
    for(ol = (ban_type == BANTYPE_ACCOUNT ? ban_list_account :
              ban_type == BANTYPE_CHAR ? ban_list_player : ban_list_ip); ol; ol = ol->next)
    {
        /* Check for IP match or name match */
        if (ban_type == BANTYPE_ACCOUNT ? (ol->objlink.ban->account == str_hash) :
            ban_type == BANTYPE_CHAR ? (ol->objlink.ban->name == str_hash) :
            (!strcmp(ol->objlink.ban->ip, str)))
        {
            if (!(CONTR(op)->gmaster_mode & (GMASTER_MODE_SA | GMASTER_MODE_GM)) &&
                 (ol->objlink.ban->ticks_init == -1))
            {
                ndi(NDI_UNIQUE, 0, op, "Only GMs and SAs can ban or unban if permanently banned.");
                return COMMANDS_RTN_VAL_ERROR;
            }

            remove_ban_entry(ol);

            if (!mute)
            {
                ndi(NDI_UNIQUE, 0, op, "You unbanned %s %s.",
                              ban_type == BANTYPE_ACCOUNT ? "account" : ban_type == BANTYPE_CHAR ? "character" : "IP",
                              str);
                LOG(llevSystem, "%s %s is unbanned by %s",
                    ban_type == BANTYPE_ACCOUNT ? "Account" : ban_type == BANTYPE_CHAR ? "Character" : "IP",
                    str, STRING_OBJ_NAME(op));
            }

            return COMMANDS_RTN_VAL_OK;
        }
    }

    return COMMANDS_RTN_VAL_OK_SILENT;
}

/* become a SA */
int command_sa(object_t *op, char *params)
{
    player_t *pl;

    if (op &&
        (pl = CONTR(op)))
    {
        if ((pl->gmaster_mode & GMASTER_MODE_SA))
        {
            remove_gmaster_mode(pl);
        }
        else
        {
            set_gmaster_mode(pl, GMASTER_MODE_SA);
        }
    }

    return COMMANDS_RTN_VAL_OK_SILENT;
}

/* become a MM */
int command_mm(object_t *op, char *params)
{
    player_t *pl;

    if (op &&
        (pl = CONTR(op)))
    {
        if ((pl->gmaster_mode & GMASTER_MODE_MM) &&
            !(pl->gmaster_mode & GMASTER_MODE_SA))
        {
            remove_gmaster_mode(pl);
        }
        else
        {
            set_gmaster_mode(pl, GMASTER_MODE_MM);
        }
    }

    return COMMANDS_RTN_VAL_OK_SILENT;
}

/* become a MW */
int command_mw(object_t *op, char *params)
{
    player_t *pl;

    if (op &&
        (pl = CONTR(op)))
    {
        if ((pl->gmaster_mode & GMASTER_MODE_MW) &&
            !(pl->gmaster_mode & (GMASTER_MODE_SA | GMASTER_MODE_MM)))
        {
            remove_gmaster_mode(pl);
        }
        else
        {
            set_gmaster_mode(pl, GMASTER_MODE_MW);
        }
    }

    return COMMANDS_RTN_VAL_OK_SILENT;
}

/* become a GM */
int command_gm(object_t *op, char *params)
{
    player_t *pl;

    if (op &&
        (pl = CONTR(op)))
    {
        if ((pl->gmaster_mode & GMASTER_MODE_GM) &&
            !(pl->gmaster_mode & GMASTER_MODE_SA))
        {
            remove_gmaster_mode(pl);
        }
        else
        {
            set_gmaster_mode(pl, GMASTER_MODE_GM);
        }
    }

    return COMMANDS_RTN_VAL_OK_SILENT;
}

/* become a VOL */
int command_vol(object_t *op, char *params)
{
    player_t *pl;

    if (op &&
        (pl = CONTR(op)))
    {
        if ((pl->gmaster_mode & GMASTER_MODE_VOL) &&
            !(pl->gmaster_mode & (GMASTER_MODE_SA | GMASTER_MODE_GM)))
        {
            remove_gmaster_mode(pl);
        }
        else
        {
            set_gmaster_mode(pl, GMASTER_MODE_VOL);
        }
    }

    return COMMANDS_RTN_VAL_OK_SILENT;
}

/* Lists online gmasters unless they have requested privacy. */
int command_gmasterlist(object_t *op, char *params)
{
    objectlink_t *ol;

    if (params)
        return COMMANDS_RTN_VAL_SYNTAX;

    for(ol = gmaster_list_VOL; ol; ol = ol->next)
    {
        if(!CONTR(ol->objlink.ob)->privacy)
        {
            ndi(NDI_UNIQUE, 0, op, "%s",
                          CONTR(ol->objlink.ob)->quick_name);
        }
    }

    for(ol = gmaster_list_GM; ol; ol = ol->next)
    {
        if(!CONTR(ol->objlink.ob)->privacy)
        {
            ndi(NDI_UNIQUE, 0, op, "%s",
                          CONTR(ol->objlink.ob)->quick_name);
        }
    }

    for(ol = gmaster_list_MW; ol; ol = ol->next)
    {
        if(!CONTR(ol->objlink.ob)->privacy)
        {
            ndi(NDI_UNIQUE, 0, op, "%s",
                          CONTR(ol->objlink.ob)->quick_name);
        }
    }

    for(ol = gmaster_list_MM; ol; ol = ol->next)
    {
        if(!CONTR(ol->objlink.ob)->privacy)
        {
            ndi(NDI_UNIQUE, 0, op, "%s",
                          CONTR(ol->objlink.ob)->quick_name);
        }
    }

    for(ol = gmaster_list_SA; ol; ol = ol->next)
    {
        if(!CONTR(ol->objlink.ob)->privacy)
        {
            ndi(NDI_UNIQUE, 0, op, "%s",
                          CONTR(ol->objlink.ob)->quick_name);
        }
    }

    return COMMANDS_RTN_VAL_OK_SILENT;
}

/* Lists, adds, or removes entries in gmaster_file. */
int command_gmasterfile(object_t *op, char *params)
{
    player_t     *pl;
    char        buf[MEDIUM_BUF],
               *cp;
    shstr_t      *subcommand = NULL;
    objectlink_t *ol;
    char        name[MEDIUM_BUF],
                host[MEDIUM_BUF],
                mode[MEDIUM_BUF];
    int         mode_id;

    if (!op ||
        op->type != PLAYER ||
        !(pl = CONTR(op)))
    {
        return COMMANDS_RTN_VAL_ERROR;
    }

    /* list all entries. */
    if (!params)
    {
        for (ol = gmaster_list; ol; ol = ol->next)
        {
            ndi(NDI_UNIQUE, 0, op, "%s", ol->objlink.gm->entry);
        }

        return COMMANDS_RTN_VAL_OK_SILENT;
    }

    /* We need to make a working copy of params (because we may write
     * to it), and take the first word as the shstr_t subcommand. */
    sprintf(buf, "%s", params);

    if ((cp = strchr(buf, ' ')))
    {
        *(cp++) = '\0';

        while (*cp == ' ')
        {
            cp++;
        }

        SHSTR_FREE_AND_ADD_STRING(subcommand, buf);
    }
    /* For this command all subcommands have further parmeters, so no space in
     * params means the player typed nonsense. */
    else
    {
        return COMMANDS_RTN_VAL_SYNTAX;
    }

    /* Add an entry. */
    if (subcommand == subcommands.add)
    {
        SHSTR_FREE(subcommand);

        if (sscanf(cp, "%[^/]/%[^/]/%s", name, host, mode) != 3 ||
            (mode_id = check_gmaster_file_entry(name, host, mode)) == GMASTER_MODE_NO)
        {
            ndi(NDI_UNIQUE, 0, op, "Malformed or missing parameter.");

            return COMMANDS_RTN_VAL_SYNTAX;
        }

        if (!compare_gmaster_mode(mode_id, pl->gmaster_mode))
        {
            ndi(NDI_UNIQUE, 0, op, "You have insufficient permission to add this entry.");

            return COMMANDS_RTN_VAL_ERROR;
        }

        if (name[0] != '*')
        {
            uint8 i = 0;
            char  filename[HUGE_BUF];

            /* Account names must be all lowercase. */
            for (; name[i]; i++)
            {
                name[i] = tolower(name[i]);
            }

            sprintf(filename, "%s/%s/%s/%s/%s.acc",
                    settings.localdir, settings.accountdir, get_subdir(name),
                    name, name);

            if (access(filename, F_OK) != 0)
            {
                ndi(NDI_UNIQUE, 0, op, "There is no account of that name.");

                return COMMANDS_RTN_VAL_ERROR;
            }

            /* Well it looks nicer to capitalise the names once in the
             * gmaster_file. */
            name[0] = toupper(name[0]);
        }

        add_gmaster_file_entry(name, host, mode_id);
        write_gmaster_file();
    }
    /* List subset of entries. */
    else if (subcommand == subcommands.list)
    {
        int success = COMMANDS_RTN_VAL_SYNTAX;

        SHSTR_FREE(subcommand);

        // Note - legal for player to type: /gmasterfile list SA VOL
        // to get a list of SAs and VOLs
        if (strstr(cp, "SA"))
        {
            success = COMMANDS_RTN_VAL_OK_SILENT;

            for (ol = gmaster_list; ol; ol = ol->next)
            {
                struct _gmaster_struct *gm = ol->objlink.gm;

                if (gm->mode == GMASTER_MODE_SA)
                {
                    ndi(NDI_UNIQUE, 0, op, "%s", gm->entry);
                }
            }
        }

        if (strstr(cp, "MM"))
        {
            success = COMMANDS_RTN_VAL_OK_SILENT;

            for (ol = gmaster_list; ol; ol = ol->next)
            {
                struct _gmaster_struct *gm = ol->objlink.gm;

                if (gm->mode == GMASTER_MODE_MM)
                {
                    ndi(NDI_UNIQUE, 0, op, "%s", gm->entry);
                }
            }
        }

        if (strstr(cp, "MW"))
        {
            success = COMMANDS_RTN_VAL_OK_SILENT;

            for (ol = gmaster_list; ol; ol = ol->next)
            {
                struct _gmaster_struct *gm = ol->objlink.gm;

                if (gm->mode == GMASTER_MODE_MW)
                {
                    ndi(NDI_UNIQUE, 0, op, "%s", gm->entry);
                }
            }
        }

        if (strstr(cp, "GM"))
        {
            success = COMMANDS_RTN_VAL_OK_SILENT;

            for (ol = gmaster_list; ol; ol = ol->next)
            {
                struct _gmaster_struct *gm = ol->objlink.gm;

                if (gm->mode == GMASTER_MODE_GM)
                {
                    ndi(NDI_UNIQUE, 0, op, "%s", gm->entry);
                }
            }
        }

        if (strstr(cp, "VOL"))
        {
            success = COMMANDS_RTN_VAL_OK_SILENT;

            for (ol = gmaster_list; ol; ol = ol->next)
            {
                struct _gmaster_struct *gm = ol->objlink.gm;

                if (gm->mode == GMASTER_MODE_VOL)
                {
                    ndi(NDI_UNIQUE, 0, op, "%s", gm->entry);
                }
            }
        }

        return success;
    }
    /* Remove an entry. */
    else if (subcommand == subcommands.remove)
    {
        SHSTR_FREE(subcommand);

        if (sscanf(cp, "%[^/]/%[^/]/%s", name, host, mode) != 3 ||
            (mode_id = check_gmaster_file_entry(name, host, mode)) == GMASTER_MODE_NO)
        {
            ndi(NDI_UNIQUE, 0, op, "Malformed or missing parameter.");

            return COMMANDS_RTN_VAL_SYNTAX;
        }

        for (ol = gmaster_list; ol; ol = ol->next)
        {
            if (!strcasecmp(name, ol->objlink.gm->name) &&
                !strcmp(host, ol->objlink.gm->host) &&
                mode_id == ol->objlink.gm->mode)
            {
                if (!compare_gmaster_mode(ol->objlink.gm->mode,
                                          pl->gmaster_mode))
                {
                    ndi(NDI_UNIQUE, 0, op, "You have insufficient permission to remove this entry.");

                    return COMMANDS_RTN_VAL_ERROR;
                }

                remove_gmaster_file_entry(ol);
                write_gmaster_file();
                update_gmaster_file();

                break;
            }
        }

        if (!ol)
        {
            ndi(NDI_UNIQUE, 0, op, "Entry could not be found!");

           return COMMANDS_RTN_VAL_ERROR;
        }
    }
    /* Unknown subcommand. */
    else
    {
        SHSTR_FREE(subcommand);

        return COMMANDS_RTN_VAL_SYNTAX;
    }

    return COMMANDS_RTN_VAL_OK;
}

/* GROS */
/* Lists all plugins currently loaded with their IDs and full names.         */
int command_listplugins(object_t *op, char *params)
{
    displayPluginsList(op);

    return COMMANDS_RTN_VAL_OK;
}

/* GROS */
/* Loads the given plugin. The DM specifies the name of the library to load  */
/* (no pathname is needed). Do not ever attempt to load the same plugin more */
/* than once at a time, or bad things could happen.                          */
int command_loadplugin(object_t *op, char *params)
{
    char buf[MEDIUM_BUF];

    if (!params)
        return COMMANDS_RTN_VAL_SYNTAX;

    sprintf(buf, "%s/../plugins/%s",
            DATADIR, params);
    printf("Requested plugin file is %s\n",
           buf);
    initOnePlugin(buf);

    return COMMANDS_RTN_VAL_OK;
}

/* GROS */
/* Unloads the given plugin. The DM specified the ID of the library to       */
/* unload. Note that some things may behave strangely if the correct plugins */
/* are not loaded.                                                           */
int command_unloadplugin(object_t *op, char *params)
{
    if (!params)
        return COMMANDS_RTN_VAL_SYNTAX;

    removeOnePlugin(params);

    return COMMANDS_RTN_VAL_OK;
}

int command_ip(object_t *op, char *params)
{
    player_t *pl;

    if (!params)
        return COMMANDS_RTN_VAL_SYNTAX;

    if (!(pl = find_player(params)))
    {
        ndi(NDI_UNIQUE | NDI_WHITE, 0, op, "No such player!");

        return COMMANDS_RTN_VAL_ERROR;
    }

    ndi(NDI_UNIQUE | NDI_WHITE, 0, op, "IP of %s is %s",
                         params, pl->socket.ip_host);

    return COMMANDS_RTN_VAL_OK_SILENT;
}

int command_dm_dev(object_t *op, char *params)
{
    if (op->type != PLAYER)
        return 0;

    command_goto(op, "/dev/testmaps/testmap_main 2 2");

    return 0;
}

/* Toggle wizpass (walk through walls, do not move apply, etc). */
int command_wizpass(object_t *op, char *params)
{
    player_t *pl;

    if (op->type != PLAYER ||
        !(pl = CONTR(op)))
    {
        return COMMANDS_RTN_VAL_ERROR;
    }

    pl->gmaster_wizpass = !pl->gmaster_wizpass;
    pl->update_los = 1;
    ndi(NDI_UNIQUE, 0, op, "Toggled gmaster_wizpass to %u",
        pl->gmaster_wizpass);
    return COMMANDS_RTN_VAL_OK_SILENT;
}

/* The matrix primarily determines whether player sees first system object on
 * square (instead of fmask). Toggle the matrix with the command on its own.
 *
 * It also can be used to specify which clayers are sent to the client. In this
 * mode you can, for example, turn off things under and over so you can observe
 * floors and fmasks without clutter. Specify a number after the command. This
 * is a bitmask of the clayers you want to see (1=floor, 2=fmask, 4=thing under,
 * 8=thing over).So the example would be 3). */
int command_matrix(object_t *op, char *params)
{
    player_t *pl;
    uint32    clayers;

    if (op->type != PLAYER ||
        !(pl = CONTR(op)))
    {
        return COMMANDS_RTN_VAL_ERROR;
    }

    clayers = (!pl->gmaster_matrix) ? 15 : 0;

    if (params)
    {
        sscanf(params, "%u", &clayers);

        if (clayers >= 16)
        {
            return COMMANDS_RTN_VAL_ERROR;
        }
    }

    pl->gmaster_matrix = clayers;
    ndi(NDI_UNIQUE, 0, op, "Set gmaster_matrix to %u", clayers);
    return COMMANDS_RTN_VAL_OK_SILENT;
}

/* This toggles whether mobs can sense the players presence. */
int command_stealth(object_t *op, char *params)
{
    player_t *pl;

    if (op->type != PLAYER ||
        !(pl = CONTR(op)))
    {
        return COMMANDS_RTN_VAL_ERROR;
    }

   pl->gmaster_stealth = !pl->gmaster_stealth;
   ndi(NDI_UNIQUE, 0, op, "Toggled gmaster_stealth to %u",
                 pl->gmaster_stealth);

    return COMMANDS_RTN_VAL_OK_SILENT;
}

int command_invisibility(object_t *op, char *params)
{
    player_t    *pl;

    if (op->type != PLAYER ||
        !(pl = CONTR(op)))
    {
        return COMMANDS_RTN_VAL_ERROR;
    }

    msp_rebuild_slices_without(MSP_KNOWN(op), op);
    pl->gmaster_invis = !pl->gmaster_invis;
    msp_rebuild_slices_with(MSP_KNOWN(op), op);
#ifndef USE_OLD_UPDATE
    OBJECT_UPDATE_VIS(op);
#else
    update_object(op, UP_OBJ_SLICE);
#endif
    ndi(NDI_UNIQUE, 0, op, "Toggled gmaster_invis to %u",
        pl->gmaster_invis);
    return COMMANDS_RTN_VAL_OK_SILENT;
}

/* '/dm_light x' switches the map master's personal light to
 * 1 <= x <= MAX_DARKNESS, or turns it off (x=0).
 * '/dm_light' toggles personal_light between off (0) and fullbeams
 * (MAX_DARKNESS). */
int command_dm_light(object_t *op, char *params)
{
    player_t *pl;
    sint32  personal_light;

    if (op->type != PLAYER ||
        !(pl = CONTR(op)))
    {
        return COMMANDS_RTN_VAL_ERROR;
    }

    personal_light = (!params)
                     ? ((pl->personal_light) ? 0 : MAX_DARKNESS)
                     : ABS(atoi(params));
    set_personal_light(pl, personal_light);
    ndi(NDI_UNIQUE, 0, op, "Switch personal light to %u.",
                  pl->personal_light);

    return COMMANDS_RTN_VAL_OK_SILENT;
}

int command_password(object_t *op, char *params)
{
    player_t          *pl;
    char             name[MEDIUM_BUF],
                     pwd_old[MEDIUM_BUF],
                     pwd_new[MEDIUM_BUF],
                     fname_old[LARGE_BUF],
                     fname_new[LARGE_BUF],
                     buf[MEDIUM_BUF];
    uint8            wildcard = 0;
    shstr_t           *name_sh = NULL;
    FILE            *fp_old,
                    *fp_new;
    struct channels *channel;

    /* op must be a properly connected player. */
    if (!op ||
        op->type != PLAYER ||
        !(pl = CONTR(op)))
    {
        return COMMANDS_RTN_VAL_ERROR;
    }

    /* params must be 3 strings: account name, old password, new password. */
    if (!params ||
        sscanf(params, "%s %s %s", name, pwd_old, pwd_new) != 3)
    {
        return COMMANDS_RTN_VAL_SYNTAX;
    }

    /* GMs can change a pwd without needing to know the old one. */
    if ((pl->gmaster_mode & GMASTER_MODE_GM) &&
         pwd_old[0] == '*' &&
         pwd_old[1] == '\0')
    {
         wildcard = 1;
    }

    /* Make sure they're all valid. */
    if (!account_name_valid(name))
    {
        ndi(NDI_UNIQUE, 0, op, "That account name is not valid!");

        return COMMANDS_RTN_VAL_ERROR;
    }
    else if (!wildcard &&
             !password_valid(pwd_old))
    {
        ndi(NDI_UNIQUE, 0, op, "That old password is not valid!");

        return COMMANDS_RTN_VAL_ERROR;
    }
    else if (!password_valid(pwd_new))
    {
        ndi(NDI_UNIQUE, 0, op, "That new password is not valid!");

        return COMMANDS_RTN_VAL_ERROR;
    }

    /* Check that an account of the specified name exists. */
    sprintf(fname_old, "%s/%s/%s/%s/%s.acc",
            settings.localdir, settings.accountdir, get_subdir(name), name,
            name);

    if (access(fname_old, F_OK) == -1)
    {
        ndi(NDI_UNIQUE, 0, op, "No account of that name exists!");

        return COMMANDS_RTN_VAL_ERROR;
    }

    /* GMs can change any account's pwd, others can only change their own. */
    SHSTR_FREE_AND_ADD_STRING(name_sh, name);

    if (!(pl->gmaster_mode & GMASTER_MODE_GM) &&
        pl->account_name != name_sh)
    {
        ndi(NDI_UNIQUE, 0, op, "You can only change the password of your own account!");
        SHSTR_FREE(name_sh);

        return COMMANDS_RTN_VAL_ERROR;
    }

    /* Open the account file for reading. */
    if (!(fp_old = fopen(fname_old, "r")))
    {
        NDI_LOG(llevBug, NDI_UNIQUE, 0, op, "Could not open account file '%s' for reading!", fname_old);
        SHSTR_FREE(name_sh);

        return COMMANDS_RTN_VAL_ERROR;
    }

    /* Open a temp file for writing. */
    sprintf(fname_new, "%s.tmp", fname_old);

    if (!(fp_new = fopen(fname_new, "w")))
    {
        NDI_LOG(llevBug, NDI_UNIQUE, 0, op, "Could not open account file '%s' for writing!", fname_new);
        SHSTR_FREE(name_sh);
        fclose(fp_old);

        return COMMANDS_RTN_VAL_ERROR;
    }

    /* Read the old file line-by-line and write to new, changing the pwd if the
     * specified old one is correct. */
    while (fgets(buf, MEDIUM_BUF, fp_old))
    {
        if (!strncmp(buf, "pwd ", 4))
        {
            buf[strlen(buf) - 1] = '\0'; // remove newline

            if (!wildcard &&
                strcmp(buf + 4, pwd_old))
            {
                ndi(NDI_UNIQUE, 0, op, "The old password you specified is incorrect!");
                SHSTR_FREE(name_sh);
                fclose(fp_old);
                fclose(fp_new);
                remove(fname_new);

                return COMMANDS_RTN_VAL_ERROR;
            }
            else
            {
                fprintf(fp_new, "pwd %s\n", pwd_new);
            }
        }
        else
        {
            fprintf(fp_new, "%s", buf);
        }
    }

    /* Close both files. */
    fclose(fp_old);
    fclose(fp_new);

    /* Delete the old file and rename new as old. */
    remove(fname_old);
    rename(fname_new, fname_old);

    /* Browse onlne players for the named account, changing the pwd if the
     * specified old one is correct. */
    for (pl = first_player; pl; pl = pl->next)
    {
        Account *ac = &pl->socket.pl_account;

        if (ac->name == name_sh)
        {
            if (wildcard ||
                !strcmp(ac->pwd, pwd_old))
            {
                sprintf(ac->pwd, "%s", pwd_new);
            }

            break;
        }
    }

    pl = CONTR(op);
    SHSTR_FREE(name_sh);
    ndi(NDI_UNIQUE, 0, op, "OK!");

    /* Never log anyone's pwd, but do log who changed it to the GM channel. */
    if ((channel = findGlobalChannelFromName(NULL, CHANNEL_NAME_GM, 1)))
    {
        sprintf(buf, "Password change on account %s by IP >%s< Account >%s< Player >%s<%s!\n",
                name, pl->socket.ip_host, pl->account_name,
                STRING_OBJ_NAME(pl->ob),
                ((pl->gmaster_mode & GMASTER_MODE_GM)) ? " (GM)" : "");
        sendChannelMessage(NULL, channel, buf);
    }

    return COMMANDS_RTN_VAL_OK;
}
