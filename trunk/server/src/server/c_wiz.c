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

/* Declare some local helper functions for command_ban() */
static int BanList(object *op, int isIP);
static int BanAdd(object *op, char *mode, char *str, int seconds);
static int BanAddToBanList(object* op, char *str, int seconds, int isIP);
static int BanRemove(object *op, char *str);
static int BanRemoveFromBanList(object *op, char *str, int isIP, int mute);

static int CommandLearnSpellOrPrayer(object *op, char *params, int special_prayer);
static int CreateObject(object *op, char *params, int isCreate, int isGenerate, int isSpawn);
static int CheckAttributeValue(char *var, char *val, int isCreate, int isGenerate, int isSpawn);

#if 0 /* disabled because account patch */
typedef struct dmload_struct {
    char    name[32];
    char    password[32];
} _dmload_struct;

static struct dmload_struct dmload_value = {"",""};

/* Gecko: we could at least search through the active, friend and player lists here... */
/* Michtoen: When reworking the DM commands - make this browsing all, why not? its a DM command.
* browse active list, all players and even all maps - this IS a super special command,
* don't care about cpu usage. Its important to patch or fix objects online.
*/
#endif

/*
 * Helper function get an object somewhere in the game
 * Returns the object which has the count-variable equal to the argument.
 */

static object * find_object(int i)
{
    /* i is the count - browse ALL and return the object */
    return NULL;
}

#if 0 /* disabled because account patch */
/* This command allows to login as a player without we know the password,
* but adding a "temporary" new one defined
*/
int command_dmload(object *op, char *params)
{
    char buf[MEDIUM_BUF],
         name[MEDIUM_BUF] = "",
         pwd[MEDIUM_BUF] = "";

    if (!op ||
        op->type != PLAYER ||
        !QUERY_FLAG(op,FLAG_WIZ))
        return 0;

    if (!params)
        return 1;

    sscanf(params, "%s %s", name, pwd);

    strncpy(dmload_value.name, name, 32);
    dmload_value.name[30] = 0;
    strncpy(dmload_value.password, pwd, 32);
    dmload_value.password[30] = 0;

    sprintf(buf,"DM_LOAD name:>%s< pwd:>%s<\n", dmload_value.name, dmload_value.password);

    LOG(llevDebug, "%s", buf);
    new_draw_info(NDI_UNIQUE, 0, op, "%s", buf);

    return 0;
}

/* check a dmload struct is set and if, the settings match.
* returns 0 when not and 1 when we want and can override!
*/
int check_dmload(const char*name, const char *pwd)
{

    /* if a name is not set or they don't match - nothing to do */
    if(!dmload_value.name[0] || !name || strcasecmp(dmload_value.name, name))
        return 0;

    if(!dmload_value.password[0] || !pwd || strcmp(dmload_value.password, pwd))
        return 0;

    /* after we report ONE match, we reset the dmload structure for safety reasons */
    dmload_value.name[0] = 0;
    dmload_value.password[0] = 0;

    LOG(llevDebug, "DM_LOAD:: DM_CHECK SUCCESS - >%s< >%s<!\n", name, pwd);
    return 1;
}

#endif

/* Sets the god for some objects.  params should contain two values -
 * first the object to change, followed by the god to change it to.
 */
int command_setgod(object *op, char *params)
{
    object *ob;
    char   *str;

    if (!params || !(str = strchr(params, ' ')))
        return 1;

    /* kill the space, and set string to the next param */
    *str++ = '\0';

    if (!(ob = find_object(atol(params))))
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Set whose god - can not find object %s?",
                             params);

        return 0;
    }

    /* Perhaps this is overly restrictive?  Should we perhaps be able
     * to rebless altars and the like?
     */
    if (ob->type != PLAYER)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "%s is not a player - can not change its god",
                             ob->name);

        return 0;
    }

    change_skill(ob, SK_PRAYING);

    if (!ob->chosen_skill || ob->chosen_skill->stats.sp != SK_PRAYING)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "%s doesn't have praying skill.", ob->name);

        return 0;
    }

    if (find_god(str) == NULL)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "No such god %s.", str);

        return 0;
    }

    become_follower(ob, find_god(str));

    return 0;
}

int command_kick(object *op, char *params)
{
    player     *pl;
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
        new_draw_info(NDI_UNIQUE, 0, op, "No such player.");

        return COMMANDS_RTN_VAL_ERROR;
    }

#if 0 // Kicking yourself is funny, a real votewinner :)
    if (pl->ob == op)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "You can't kick yourself!");

        return COMMANDS_RTN_VAL_ERROR;
    }
#endif

    /* Remember the names before we get kickhappy. */
    kicker_name = query_name(op);
    kickee_name = query_name(pl->ob);

    /* Get kickhappy. */
    kick_player(pl);

    /* Hopefully impossible. */
    if ((pl = find_player(params)))
    {
        LOG(llevBug, "BUG:: %s/command_kick(): %s is kickproof!\n",
            __FILE__, kickee_name);
        new_draw_info(NDI_UNIQUE, 0, op, "%s has resisted your kick!",
                             kickee_name);

        return COMMANDS_RTN_VAL_ERROR;
    }

    /* Log it and tell everyone so justice is seen to be done, or at least we
     * all get a good laugh. */
    sprintf(buf, "KICK: Player %s has been kicked by %s",
            kickee_name, kicker_name);
    LOG(llevInfo, "%s\n", buf);
    new_draw_info(NDI_UNIQUE | NDI_ALL, 5, op, "%s is kicked out of the game.",
                  kickee_name);

    /* we kicked player params succesfull.
     * Now we give him a 1min temp ban, so he can
     * think about it.
     * If its a "technical" kick, the 10 sec is a protection.
     * Perhaps we want reset a map or whatever.
     */
    ticks = (int)(pticks_second * 60.0f);
    add_ban_entry(params, NULL, ticks, ticks);

    return COMMANDS_RTN_VAL_OK;
}

int command_reboot(object *op, char *params)
{
    char  *cp = NULL;
    shstr *hash = NULL;
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
        hash = add_refcount(subcommands.restart);
    }
    else
    {
        /* Find the subcommand. */
        if ((cp = strchr(params, ' ')))
        {
            *(cp++) = '\0';

            while (*cp == ' ')
            {
                cp++;
            }
        }

        hash = add_string(params);
    }

    /* Cancel scheduled reboot. */
    if (hash == subcommands.cancel)
    {
        FREE_AND_CLEAR_HASH(hash);
        shutdown_agent(-2, SERVER_EXIT_NORMAL, CONTR(op), NULL);

        return COMMANDS_RTN_VAL_OK;
    }
    /* Restart server. */
    else if (hash == subcommands.restart)
    {
        char stream[TINY_BUF] = "";

        FREE_AND_CLEAR_HASH(hash);
#ifdef DAI_DEVELOPMENT_CODE
        time = 30;

        if (cp)
        {
            if (!sscanf(cp, "%d %s", &time, stream))
            {
               sscanf(cp, "%s", stream);
            }
        }
#else

#ifdef DAI_DEVELOPMENT_CONTENT
        time = 30;
#else
        time = 300;
#endif

        if (cp)
        {
            sscanf(cp, "%d", &time);
        }
#endif

        /* Gmasters below SA have a minimum reboot time of 30s. */
        if (!(CONTR(op)->gmaster_mode & GMASTER_MODE_SA) &&
            time < 30)
        {
            time = 30;
        }

#ifdef DAI_DEVELOPMENT_CODE
        if (stream[0] &&
            !strpbrk(stream, "\"&*/ "))
        {
            char  buf[HUGE_BUF];
            FILE *fp;

            sprintf(buf, "%s/%s", settings.localdir, "stream");

            if (!(fp = fopen(buf, "w")))
            {
                LOG(llevSystem, "Write '%s'... FAILED!\n", buf);
                LOG(llevBug, "BUG:: %s/command_reboot(): Cannot open file for writing!\n",
                    __FILE__);
            }
            else
            {
                LOG(llevSystem, "Write '%s'... OK!\n", buf);
                fprintf(fp, "%s", stream);
                fclose(fp);
            }
        }
#endif

        shutdown_agent(time, SERVER_EXIT_RESTART, CONTR(op),
                       "Server will recompile and arches and maps will be updated.");

        return COMMANDS_RTN_VAL_OK;
    }
    /* Shutdown server. */
    else if (hash == subcommands.shutdown)
    {
        FREE_AND_CLEAR_HASH(hash);

        /* Shutdown is special so we restrict it further (hopefully to people
         * who can start it up again). */
        if (!(CONTR(op)->gmaster_mode & GMASTER_MODE_SA))
        {
            new_draw_info(NDI_UNIQUE, 0, op, "You have insufficient permission to shutdown the server.");

           return COMMANDS_RTN_VAL_ERROR;
        }

        time = 0;

        if (cp)
        {
            sscanf(cp, "%d", &time);
        }

        shutdown_agent(time, SERVER_EXIT_SHUTDOWN, CONTR(op),
                       "Server will shutdown and not reboot.");

        return COMMANDS_RTN_VAL_OK;
    }

    /* Unknown subcommand. */
    FREE_AND_CLEAR_HASH(hash);

    return COMMANDS_RTN_VAL_SYNTAX;
}

int command_goto(object *op, char *params)
{
    char       name[MAXPATHLEN] = {"\0"},
               buf[MAXPATHLEN];
    shstr     *path = NULL;
    int        x = -1,
               y = -1,
               flags = 0;
    mapstruct *m;

    if (!op ||
        op->type != PLAYER ||
        !CONTR(op))
    {
        return COMMANDS_RTN_VAL_ERROR;
    }

    if (params)
    {
        sscanf(params, "%s %d %d", name, &x, &y);
    }

    switch (name[0])
    {
        /* No path: Goto savebed. */
        case '\0': // none given
            path = add_string(CONTR(op)->savebed_map);
            flags = CONTR(op)->bed_status;
            x = CONTR(op)->bed_x;
            y = CONTR(op)->bed_y;

            break;

        /* Absolute. */
        case '/':
            if (check_path(normalize_path("/", name, buf), 1) != -1)
            {
                path = add_string(buf);
            }

            break;

        /* Relative. */
        default: // relative
            if (check_path(normalize_path(CONTR(op)->orig_map, name, buf),
                           1) != -1)
            {
                path = add_string(buf);
            }
    }

    if (!path ||
        !(m = ready_map_name(path, (path != CONTR(op)->savebed_map) ? path :
                             CONTR(op)->orig_savebed_map, flags, op->name)))
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Map '%s' does not exist!",
                      buf);
        new_draw_info(NDI_UNIQUE, 0, op, "You're going nowhere!");

        return COMMANDS_RTN_VAL_ERROR;
    }

    /* Specified coords are not on the map? Default to map entry point. */
    if (x < 0 ||
        x >= MAP_WIDTH(m) ||
        y < 0 ||
        y >= MAP_HEIGHT(m))
    {
        x = MAP_ENTER_X(m);
        y = MAP_ENTER_Y(m);
    }

    (void)enter_map(op, NULL, m, x, y, flags, 0);
    set_mappath_by_map(op);

    FREE_ONLY_HASH(path);

    return COMMANDS_RTN_VAL_OK_SILENT;
}

int command_create(object *op, char *params)
{
    return CreateObject(op, params, TRUE, FALSE, FALSE);
}

int command_generate(object *op, char *params)
{
    return CreateObject(op, params, FALSE, TRUE, FALSE);
}

int command_spawn(object *op, char *params)
{
    return CreateObject(op, params, FALSE, FALSE, TRUE);
}

static int CreateObject(object *op, char *params, int isCreate, int isGenerate, int isSpawn)
{
    int        nrof = 1, set_nrof = 0,
               magic = 0, set_magic = 0,
               i, pos = 0,
               isMultiPart = TRUE;
    char      *str,
               var[SMALL_BUF] = "",
               val[SMALL_BUF] = "",
               buf[MEDIUM_BUF];
    object    *tmp = NULL;
    archetype *at;
    artifact  *art = NULL;

    if (!op || op->type != PLAYER)
        return COMMANDS_RTN_VAL_ERROR;

    if (!params)
        return COMMANDS_RTN_VAL_SYNTAX;

    // First parameter must be quantity, or blank
    str = get_param_from_string(params, &pos);

    if (sscanf(str, "%d", &nrof))
    {
        set_nrof = 1;

        /* Constrain nrof to sensible values. */
        if (isGenerate || isSpawn)
            nrof = MAX(1, MIN(nrof, 100));
        else
            nrof = MAX(1, nrof);

        // Second parameter may be magic bonus (only if quantity was specified), or blank
        str = get_param_from_string(params, &pos);

        if (sscanf(str, "%d", &magic))
        {
            set_magic = 1;

            /* Constrain nrof to sensible values. */
            if (isGenerate || isSpawn)
                magic = MAX(-10, MIN(magic, 10));
            else
                magic = MAX(-127, MIN(magic, 127));

            // Next parameter *must* be arch name
            str = get_param_from_string(params, &pos);
        }
    }

    if (!str)
        return COMMANDS_RTN_VAL_SYNTAX;

    // Browse the archtypes for the name - perhaps it is a base item
    if ((at = find_archetype(str)) == NULL)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "No such archetype name: %s.", str);
        return COMMANDS_RTN_VAL_ERROR;
    }

    if (isGenerate)
        if (at->clone.type == MONSTER || at->clone.type == PLAYER)
        {
            new_draw_info(NDI_UNIQUE, 0, op, "Generate cannot be used to create mobs.");
            return COMMANDS_RTN_VAL_ERROR;
        }

    if (isSpawn)
        if (at->clone.type != MONSTER && at->clone.type != PLAYER)
        {
            new_draw_info(NDI_UNIQUE, 0, op, "Spawn must only be used to create mobs.");
            return COMMANDS_RTN_VAL_ERROR;
        }

    // Is this a multi-part object, or simple?
    if (at->clone.nrof)
        isMultiPart = FALSE;

    // Now, start to create the object ...
    // For a simple object, we only do this loop once, and set nrof directly inside loop
    // For multi-part, we repeat the loop nrof times
    for (i = 0 ; i < (isMultiPart ? (set_nrof ? nrof : 1) : 1); i++)
    {
        archetype      *atmp;
        object*prev =   NULL, *head = NULL;

        for (atmp = at; atmp != NULL; atmp = atmp->more)
        {
            tmp = arch_to_object(atmp);
            if (head == NULL)
                head = tmp;  // For simple objects, we'll only get head

            tmp->x = op->x + tmp->arch->clone.x;
            tmp->y = op->y + tmp->arch->clone.y;
            tmp->map = op->map;

            if (!isMultiPart)
                if (set_nrof)
                    tmp->nrof = nrof;

            if (set_magic)
                set_abs_magic(tmp, magic);

            // Read the remaining attribute / value pairs
            while ((str = get_param_from_string(params, &pos)))
            {
                // amask is handled differently to other attributes
                if (!strcmp(str, "amask"))
                {
                    str = get_param_from_string(params, &pos);

                    if (!str)
                        return COMMANDS_RTN_VAL_SYNTAX;

                    // Does this arch actually have any artifacts?
                    if (find_artifactlist(at->clone.type) == NULL)
                        new_draw_info(NDI_UNIQUE, 0, op, "No artifact list for type %d\n",
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
                                    new_draw_info(NDI_UNIQUE, 0, op, "Illegal artifact combination ([%d] of %s)",
                                                         at->clone.type, str);
                                    art = NULL;
                                }
                            else
                                new_draw_info(NDI_UNIQUE, 0, op, "No such artifact ([%d] of %s)",
                                                     at->clone.type, str);


                        }
                    }
                }  // end of str == amask
                else
                {  // any other parameter
                    strcpy(var, str);  // Save the variable name

                    str = get_param_from_string(params, &pos);  // Read value for this variable

                    if (!str)
                        return COMMANDS_RTN_VAL_SYNTAX;

                    strcpy(val, str);  // Save the value

                    // Check for restrictions
                    if (CheckAttributeValue(var, val, isCreate, isGenerate, isSpawn))
                    {
                        // set_variable needs param to look like "param value"
                        sprintf(buf, "%s %s", var, val);

                        if (set_variable(tmp, buf) == -1)
                            new_draw_info(NDI_UNIQUE, 0, op, "Unknown variable '%s'", var);
                        else
                            new_draw_info(NDI_UNIQUE, 0, op, "(%s#%d)->%s=%s",
                                          tmp->name, tmp->count, var, val);
                    }
                    else
                        new_draw_info(NDI_UNIQUE, 0, op, "Not allowed to set variable '%s'", var);
                }
            }  // end of reading parameter string

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
            if (isMultiPart)
            {
                new_draw_info(NDI_UNIQUE, 0, op, "Sorry - can't create multipart items!");
                return COMMANDS_RTN_VAL_ERROR;
            }

            head = insert_ob_in_ob(head, op);
            esrv_send_item(op, head);
        }
    }

    return COMMANDS_RTN_VAL_OK;
}

static int CheckAttributeValue(char *var, char *val, int isCreate, int isGenerate, int isSpawn)
{
    int i;

    if (!strcmp(var, "name"))
        return TRUE; // Everyone can do this

    if (!strcmp(var, "title"))
        return TRUE; // Everyone can do this

    if (!strcmp(var, "level"))
    {
        // Level restricted between 1-127
        // Also checks for actual integer level given ...
        if (sscanf(val, "%d", &i) == 1)
        {
            i = MAX(1, MIN(i, 127));
            itoa(i, val, 10);

            if (isCreate) return TRUE;
            if (isSpawn) return TRUE;
        }
    }

    if (!strcmp(var, "item_condition"))
    {
        // Restricted between 1-200 (used in adjust_monster() in spawn_point.c)
        // Also checks for actual integer level given ...
        if (sscanf(val, "%d", &i) == 1)
        {
            i = MAX(1, MIN(i, 200));
            itoa(i, val, 10);

            if (isCreate) return TRUE;
            if (isSpawn) return TRUE;
        }
    }

    if (!strstr(var, "attack_") || !strstr(var, "resist_"))
    {
        // We won't bother checking for valid attack/resist types,
        // as this is done by set_variable() later anyway
        if (sscanf(val, "%d", &i) == 1)
        {
            i = MAX(-100, MIN(i, 100));
            itoa(i, val, 10);

            if (isCreate) return TRUE;
            if (isSpawn) return TRUE;
        }
    }

    // Anything else can be set by Create command
    if (isCreate)
        return TRUE;

    return FALSE;
}

int command_listarch(object *op, char *params)
{
    archetype      *at;
    artifactlist   *al;
    artifact       *art = NULL;

    // This code basically copied from the dump_arch code (so maybe directly use, with a flag
    // for dump all or dump name only ??

    if (!op || op->type != PLAYER)
        return COMMANDS_RTN_VAL_ERROR;

    return COMMANDS_RTN_VAL_OK; // Not working yet!!!

//    if (!params)
//        return COMMANDS_RTN_VAL_SYNTAX;

// need to generate a hash table, and then check for duplicates
// or is there already a hash table of arch names?
// put this code into arch.c

// add header for real arch list
    for (at = first_archetype; at != NULL; at = (at->more == NULL) ? at->next : at->more)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "%s", at->clone.name);
    }

// turn into a proper new_draw_info
//    LOG(llevInfo, "Artifacts fake arch list:\n");
    for (al = first_artifactlist; al != NULL; al = al->next)
    {
        art = al->items;
        do
        {
            if(art->flags&ARTIFACT_FLAG_HAS_DEF_ARCH )
            {
                // new_draw_info(NDI_UNIQUE, 0, op, "%s", art->def_at_name);
            }
            art = art->next;
        }
        while (art != NULL);
    }

    return COMMANDS_RTN_VAL_OK;
}

// How to print in batches:
#if 0
        new_draw_info(NDI_UNIQUE | NDI_YELLOW, 0, pl->ob, "\n%s", name[i]);

        for (j = 0; j < size[i]; j++)
        {
            /* TODO: This calculation can be removed, and the following
             * new_draw_info() moved to inside this for loop by having the
             * client handle NDI_UNIQUE properly (well, at all).
             * -- Smacky 20090604 */
            if (strlen(buf) + strlen(ap[i][j].name) > 42)
            {
                new_draw_info(NDI_UNIQUE | NDI_WHITE, 0, pl->ob, "%s", buf);
                buf[0] = '\0';
            }

            sprintf(strchr(buf, '\0'), " /%s ~+~", ap[i][j].name);
        }

        new_draw_info(NDI_UNIQUE | NDI_WHITE, 0, pl->ob, "%s", buf);
#endif



#ifndef USE_CHANNELS
int command_mutelevel(object *op, char *params)
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
        new_draw_info(NDI_UNIQUE, 0, op, "WARNING: Maximum mutelevel for VOLs is 10.");
        lvl = 10;
    }

    settings.mutelevel = lvl;
    return COMMANDS_RTN_VAL_OK;
}
#else
// mutelevel does nothing if channels are in use
int command_mutelevel(object *op, char *params)
{
    if (!op)
        return COMMANDS_RTN_VAL_ERROR;

    new_draw_info(NDI_UNIQUE, 0, op, "Mutelevel command is non-functional when channels are in use.");

    return COMMANDS_RTN_VAL_OK_SILENT;
}
#endif

int command_connections(object *op, char *params)
{
    int nr = 2;

    if (!op)
        return COMMANDS_RTN_VAL_ERROR;

    if (!params || (sscanf(params, "%d", &nr) != 1))
        return COMMANDS_RTN_VAL_SYNTAX;

    if(nr < 2)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "WARNING: Miminum connections from single IP must be at least 2.");
        nr = 2;
    }

    settings.max_cons_from_one_ip = nr;
    return COMMANDS_RTN_VAL_OK;
}

int command_summon(object *op, char *params)
{
    int     i;
    player *pl;

    if (!op)
        return COMMANDS_RTN_VAL_ERROR;

    if (!params)
        return COMMANDS_RTN_VAL_SYNTAX;

    if (!(pl = find_player(params)))
    {
        new_draw_info(NDI_UNIQUE, 0, op, "No such player.");

        return COMMANDS_RTN_VAL_ERROR;
    }

    if (pl->ob == op)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "You can't summon yourself next to yourself.");

        return COMMANDS_RTN_VAL_ERROR;
    }

    if (!(pl->state & ST_PLAYING))
    {
        new_draw_info(NDI_UNIQUE, 0, op, "That player can't be summoned right now.");

        return COMMANDS_RTN_VAL_ERROR;
    }

    i = find_free_spot(op->arch, op, op->map, op->x, op->y, 0, 1, SIZEOFFREE1 + 1);

    if (i == -1)
        i = 0;

    if (enter_map_by_name(pl->ob, op->map->path, op->map->orig_path,
                          op->x + freearr_x[i], op->y + freearr_y[i],
                          MAP_STATUS_TYPE(op->map->map_status)))
    {
        new_draw_info(NDI_UNIQUE, 0, pl->ob, "You are summoned.");
        new_draw_info(NDI_UNIQUE, 0, op, "OK.");
    }

    return COMMANDS_RTN_VAL_OK;
}

int command_teleport(object *op, char *params)
{
    int     i;
    player *pl;

    if (!op)
        return COMMANDS_RTN_VAL_ERROR;

    if (!params)
        return COMMANDS_RTN_VAL_SYNTAX;

    if (!(pl = find_player(params)))
    {
        new_draw_info(NDI_UNIQUE, 0, op, "No such player.");

        return COMMANDS_RTN_VAL_ERROR;
    }

    if (pl->ob == op)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "You can't teleport yourself next to yourself.");

        return COMMANDS_RTN_VAL_ERROR;
    }

    if (!(pl->state & ST_PLAYING))
    {
        new_draw_info(NDI_UNIQUE, 0, op, "You can't teleport to that player right now.");

        return COMMANDS_RTN_VAL_ERROR;
    }

    i = find_free_spot(pl->ob->arch, pl->ob, pl->ob->map, pl->ob->x, pl->ob->y, 0, 1, SIZEOFFREE1 + 1);

    if (i == -1)
        i = 0;

    if (enter_map_by_name(op, pl->ob->map->path, pl->ob->map->orig_path,
                          pl->ob->x + freearr_x[i], pl->ob->y + freearr_y[i],
                          MAP_STATUS_TYPE(pl->ob->map->map_status)))
        new_draw_info(NDI_UNIQUE, 0, op, "OK.");

    return COMMANDS_RTN_VAL_OK;
}

int command_inventory(object *op, char *params)
{
    object *tmp;
    int     i;

    if (!op)
        return COMMANDS_RTN_VAL_ERROR;

    if (!params)
    {
        inventory(op, NULL);

        return COMMANDS_RTN_VAL_OK_SILENT;
    }

    // TODO - find_object() is not yet implemented, so will always return NULL
    // Manual file only shows /inventory, no mention of parameters, until this is done
    if (!sscanf(params, "%d", &i) || (tmp = find_object(i)) == NULL)
        return COMMANDS_RTN_VAL_SYNTAX;

    inventory(op, tmp);

    return COMMANDS_RTN_VAL_OK_SILENT;
}


int command_dump(object *op, char *params)
{
    int     i;
    object *tmp;

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
    new_draw_info(NDI_UNIQUE, 0, op, "%s", errmsg);

    return COMMANDS_RTN_VAL_OK_SILENT;
}

int command_patch(object *op, char *params)
{
    int     i;
    char   *arg, *arg2;
    object *tmp;

    tmp = NULL;

    if (params)
    {
        // TODO - find_object() is not yet implemented, so will always return NULL
        // Manual file only shows /patch me, until this is done
        // Actually, man file not written, 'cos I don't know what this does - TW
        if (!strncmp(params, "me", 2))
            tmp = op;
        else if (sscanf(params, "%d", &i))
            tmp = find_object(i);
    }

    if (!tmp)
        return COMMANDS_RTN_VAL_SYNTAX;

    arg = strchr(params, ' ');

    if (!arg)
        return COMMANDS_RTN_VAL_SYNTAX;

    if ((arg2 = strchr(++arg, ' ')))
        arg2++;

    if (set_variable(tmp, arg) == -1)
        new_draw_info(NDI_UNIQUE, 0, op, "Unknown variable %s",
                             arg);
    else
        new_draw_info(NDI_UNIQUE, 0, op, "(%s#%d)->%s=%s",
                             tmp->name, tmp->count, arg, arg2);

    return COMMANDS_RTN_VAL_OK;
}

int command_remove(object *op, char *params)
{
    int     i;
    object *tmp;

    if (!params ||
        !sscanf(params, "%d", &i) ||
        !(tmp = find_object(i)))
        return COMMANDS_RTN_VAL_SYNTAX;

    remove_ob(tmp);
    check_walk_off(tmp, NULL, MOVE_APPLY_VANISHED);

    return COMMANDS_RTN_VAL_OK;
}

int command_free(object *op, char *params)
{
    int     i;
    object *tmp;

    if (!params ||
        !sscanf(params, "%d", &i) ||
        !(tmp = find_object(i)))
        return COMMANDS_RTN_VAL_SYNTAX;

    /* free_object(tmp); TODO: remove me*/

    return COMMANDS_RTN_VAL_OK;
}

/* Similar to addexp, but we set here the skill level explicit
 * If the player doesn't have the skill, we add it.
 * if level is 0 we remove the skill (careful!!)
*/
int command_setskill(object *op, char *params)
{
    char    buf[MEDIUM_BUF];
    int     level, snr;
    object *exp_skill, *exp_ob;
    player *pl;

    if (!params ||
        sscanf(params, "%s %d %d", buf, &snr, &level) != 3)
    {
        int i;
        char buf[HUGE_BUF];

        sprintf(buf, "Usage: setskill [who] [skill nr] [level]\nSkills/Nr: ");

        for(i=0;i<NROFSKILLS;i++)
            sprintf(strchr(buf, '\0'), ",%s(%d)", skills[i].name,i);

        new_draw_info(NDI_UNIQUE, 0, op, "%s", buf);
        return COMMANDS_RTN_VAL_OK_SILENT;
    }

    if (!(pl = find_player(buf)))
    {
        new_draw_info(NDI_UNIQUE, 0, op, "No such player.");

        return COMMANDS_RTN_VAL_ERROR;
    }

    /* Bug 0000100: /addexp Cher 101 100000 crashes server */
    /* Safety check */
    if (snr < 0 || snr >= NROFSKILLS)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "No such skill.");

        return COMMANDS_RTN_VAL_ERROR;
    }

    /* Constrain level to sensible values. */
    level = MAX(0, MIN(level, MAXLEVEL));

    if (!(exp_skill = pl->skill_ptr[snr])) /* we don't have the skill - learn it*/
    {
        learn_skill(op, NULL, NULL, snr, 0);
        exp_skill = pl->skill_ptr[snr];
        FIX_PLAYER(op, "setskill");
    }
    else if(!level)/* if level is 0 we unlearn the skill! */
    {
        new_draw_info(NDI_UNIQUE, 0, op, "removed skill!");
        exp_ob = exp_skill->exp_obj;
        remove_ob(exp_skill);
        player_lvl_adj(op, exp_ob, TRUE);
        player_lvl_adj(op, NULL, TRUE);
        FIX_PLAYER(op, "setskill");

        return COMMANDS_RTN_VAL_OK;
    }

    if (!exp_skill) /* safety check */
    {
        /* our player don't have this skill?
         * This can happens when group exp is devided.
         * We must get a useful sub or we skip the exp.
         */
        new_draw_info(NDI_UNIQUE, 0, op, "No such skill!");
        LOG(llevDebug, "TODO: command_setskill(): called for %s with skill nr %d / %d level - object has not this skill.\n",
            query_name(op), snr, level);

        return COMMANDS_RTN_VAL_ERROR; /* TODO: groups comes later  - now we skip all times */
    }

    pl->update_skills = 1; /* we will sure change skill exp, mark for update */
    exp_ob = exp_skill->exp_obj;

    if (!exp_ob)
    {
        LOG(llevBug, "BUG: add_exp() skill:%s - no exp_op found!!\n",
            query_name(exp_skill));

        return COMMANDS_RTN_VAL_ERROR;
    }

    exp_skill->level = level;
    exp_skill->stats.exp =  new_levels[level] - 1;

    /* adjust_exp has adjust the skill and all exp_obj and player exp */
    /* now lets check for level up in all categories */
    adjust_exp(op, exp_skill, 1); /* we add one more so we get a clean call here */
    player_lvl_adj(op, exp_skill, TRUE);
    player_lvl_adj(op, exp_ob, TRUE);
    player_lvl_adj(op, NULL, TRUE);
    FIX_PLAYER(op, "setskill");

    return COMMANDS_RTN_VAL_OK;
}

int command_addexp(object *op, char *params)
{
    char    buf[MEDIUM_BUF];
    int     exp, snr;
    object *exp_skill, *exp_ob;
    player *pl;

    if (!params || sscanf(params, "%s %d %d", buf, &snr, &exp) != 3)
    {
        int i;
        char buf[HUGE_BUF];

        sprintf(buf, "Usage: addexp [who] [skill nr] [exp]\nSkills/Nr: ");
        for(i=0;i<NROFSKILLS;i++)
            sprintf(strchr(buf, '\0'), "%s(%d), ", skills[i].name,i);
        new_draw_info(NDI_UNIQUE, 0, op, "%s", buf);
        return COMMANDS_RTN_VAL_OK_SILENT;
    }

    if (!(pl = find_player(buf)))
    {
        new_draw_info(NDI_UNIQUE, 0, op, "No such player.");

        return COMMANDS_RTN_VAL_ERROR;
    }

    /* Bug 0000100: /addexp Cher 101 100000 crashes server */
    /* Safety check */
    if (snr < 0 || snr >= NROFSKILLS)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "No such skill.");

        return COMMANDS_RTN_VAL_ERROR;
    }

    if (!(exp_skill = pl->skill_ptr[snr])) /* safety check */
    {
        /* our player don't have this skill?
         * This can happens when group exp is devided.
         * We must get a useful sub or we skip the exp.
         */
        LOG(llevDebug, "TODO: add_exp(): called for %s with skill nr %d / %d exp - object has not this skill.\n",
            query_name(pl->ob), snr, exp);

        return COMMANDS_RTN_VAL_ERROR; /* TODO: groups comes later  - now we skip all times */
    }

    pl->update_skills = 1; /* we will sure change skill exp, mark for update */

    if (!(exp_ob = exp_skill->exp_obj))
    {
        LOG(llevBug, "BUG: add_exp() skill:%s - no exp_op found!!\n",
            query_name(exp_skill));

        return COMMANDS_RTN_VAL_ERROR;
    }

    exp = adjust_exp(pl->ob, exp_skill, exp);   /* first we see what we can add to our skill */

    /* adjust_exp has adjust the skill and all exp_obj and player exp */
    /* now lets check for level up in all categories */
    player_lvl_adj(pl->ob, exp_skill, TRUE);
    player_lvl_adj(pl->ob, exp_ob, TRUE);
    player_lvl_adj(pl->ob, NULL, TRUE);
    FIX_PLAYER(pl->ob, "addexp");
    return COMMANDS_RTN_VAL_OK;
}

/* '/serverspeed' reports the current server speed.
 * '/serverspeed <number>' sets server speed to <number>. */
int command_serverspeed(object *op, char *params)
{
    long i;

    if (!params)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Current server speed is %ld ums (%f ticks/second)",
                             pticks_ums, pticks_second);

        return COMMANDS_RTN_VAL_OK_SILENT;
    }

    if (!sscanf(params, "%ld", &i))
        return COMMANDS_RTN_VAL_SYNTAX;

    set_pticks_time(i);
    GETTIMEOFDAY(&last_time);
    new_draw_info(NDI_UNIQUE, 0, op, "Set server speed to %ld ums (%f ticks/second)",
                         pticks_ums, pticks_second);

    return COMMANDS_RTN_VAL_OK;
}

/**************************************************************************/
/* Mods made by Tyler Van Gorder, May 10-13, 1992.                        */
/* CSUChico : tvangod@cscihp.ecst.csuchico.edu                            */
/**************************************************************************/

int command_stats(object *op, char *params)
{
    player *pl;

    if (!params)
        return COMMANDS_RTN_VAL_SYNTAX;

    if (!(pl = find_player(params)))
    {
        new_draw_info(NDI_UNIQUE, 0, op, "No such player.");

        return COMMANDS_RTN_VAL_ERROR;
    }

    new_draw_info(NDI_UNIQUE, 0, op,
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

int command_setstat(object *op, char *params)
{
    char    player_name[20],
            stat_name[20];
    int     v;
    player *pl;
    shstr  *hash_name = NULL;

    if (!params ||
        sscanf(params, "%s %s %d", player_name, stat_name, &v) != 3)
    {
        return COMMANDS_RTN_VAL_SYNTAX;
    }

    if (!(pl = find_player(player_name)))
    {
        new_draw_info(NDI_UNIQUE, 0, op, "No such player!");

        return COMMANDS_RTN_VAL_ERROR;
    }

    hash_name = add_string(stat_name);

    if (v < MIN_STAT ||
        v > MAX_STAT)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Illegal range of stat!");

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
        new_draw_info(NDI_UNIQUE , 0, op,  "Unrecognised stat!");
        FREE_AND_CLEAR_HASH(hash_name);

        return COMMANDS_RTN_VAL_ERROR;
    }

    FREE_AND_CLEAR_HASH(hash_name);
    new_draw_info(NDI_UNIQUE, 0, op, "%s has been altered.", pl->ob->name);
    FIX_PLAYER(pl->ob, "command setstat");

    return COMMANDS_RTN_VAL_OK;
}

int command_reset(object *op, char *params)
{
    int        count;
    mapstruct *m;
    player    *pl;

    if (!params)
        m = has_been_loaded_sh(op->map->path);
    else
    {
        const char *mapfile_sh = add_string(params);

        m = has_been_loaded_sh(mapfile_sh);
        free_string_shared(mapfile_sh);
    }

    if (!m)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "No such map.");

        return COMMANDS_RTN_VAL_ERROR;
    }

    if (m->in_memory != MAP_SWAPPED)
    {
        if (m->in_memory != MAP_IN_MEMORY)
        {
            LOG(llevBug, "BUG: Tried to swap out map which was not in memory.\n");

            return COMMANDS_RTN_VAL_ERROR;
        }

        new_draw_info(NDI_UNIQUE, 0, op, "Start reseting map %s.",
                             STRING_SAFE(m->path));
        /* remove now all players from this map - flag them so we can
             * put them back later.
             */

        for (pl = first_player, count = 0; pl != NULL; pl = pl->next)
        {
            if (pl->ob->map == m)
            {
                count++;
                /* With the new activelist, any player on a reset map
                 * was somehow forgotten. This seems to fix it. The
                 * problem isn't analyzed, though. Gecko 20050713 */
                activelist_remove(pl->ob);
                remove_ob(pl->ob); /* no walk off check */
                pl->dm_removed_from_map = 1;
                /*tmp=op;*/
            }
            else
                pl->dm_removed_from_map = 0;
        }

        new_draw_info(NDI_UNIQUE, 0, op, "removed %d players from map. Swap map.",
                             count);
        swap_map(m, 1);
    }

    if (m->in_memory == MAP_SWAPPED)
    {
        LOG(llevDebug, "Resetting map %s.\n", m->path);
        clean_tmp_map(m);
        FREE_AND_NULL_PTR(m->tmpname);
        /* setting this effectively causes an immediate reload */
        m->reset_time = 1;
        new_draw_info(NDI_UNIQUE, 0, op, "Swap successful. Inserting players.");

        m = ready_map_name(m->path, m->orig_path,
                           MAP_STATUS_TYPE(m->map_status), m->reference);

        for (pl = first_player; pl; pl = pl->next)
        {
            if (pl->dm_removed_from_map)
            {
                insert_ob_in_map(pl->ob, m, NULL, INS_NO_MERGE);
                activelist_insert(pl->ob); /* See activelist comment above */

                if (pl->ob != op)
                {
                    if (QUERY_FLAG(pl->ob, FLAG_WIZ))
                        new_draw_info(NDI_UNIQUE, 0, pl->ob, "Map reset by %s.",
                                             op->name);
                    else /* Write a nice little confusing message to the players */
                        new_draw_info(NDI_UNIQUE, 0, pl->ob,
                                      "Your surroundings seem different but still familiar. Haven't you been here before?");
                }
            }
        }

        new_draw_info(NDI_UNIQUE, 0, op, "resetmap done.");
    }
    else
    {
        /* Need to re-insert player if swap failed for some reason */
        for (pl = first_player; pl; pl = pl->next)
        {
            if (pl->dm_removed_from_map)
                insert_ob_in_map(pl->ob, m, NULL, INS_NO_MERGE | INS_NO_WALK_ON);
        }

        new_draw_info(NDI_UNIQUE, 0, op, "Reset failed, couldn't swap map!");
    }

    return COMMANDS_RTN_VAL_OK;
}

/* warning: these function is for heavy debugging.
 * Its somewhat useless under windows.
 */
int command_check_fd(object *op, char *params)
{
    struct _stat buf;
    int          handle_max,
                 fh;

    handle_max = (socket_info.max_filedescriptor < 100) ? 100 :
                 socket_info.max_filedescriptor;

    /* remember, max_filedescriptor don't works under windows */
    new_draw_info(NDI_UNIQUE, 0, op, "check file handles from 0 to %d.",
                         handle_max);
    LOG(llevSystem, "check file handles from 0 to %d.",
        handle_max);

    for (fh = 0; fh <= handle_max; fh++)
    {
        /* Check if statistics are valid: */
        if (!_fstat(fh, &buf))
        {
            /* no ttyname() under windows... well,
                     * debugging fh's is always more clever on linux.
                     */
#ifdef WIN32
            LOG(llevSystem, "FH %d ::(%d) size     : %ld\n",
                fh, _isatty(fh), buf.st_size);
#else
            player *pp;
            char   *name1 = NULL;

            /* collect some senseless handle numbers... */
            for (pp = first_player; pp; pp = pp->next)
            {
                if (pp->socket.fd == fh)
                    break;
            }

            name1 = ttyname(_isatty(fh));

            LOG(llevSystem, "FH %d ::(%s) (%s) size: %ld\n",
                fh, (name1) ? name1 : "><",
                (pp) ? ((pp->ob) ? query_name(pp->ob) : ">player<") : "",
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
int command_mute(object *op, char *params)
{
    char        name[MEDIUM_BUF] = "";
    int         seconds = 0;
    player     *pl;

    if (!params)
        return COMMANDS_RTN_VAL_SYNTAX;

    sscanf(params, "%s %d", name, &seconds);

    if (!(pl = find_player(name)))
    {
        new_draw_info(NDI_UNIQUE, 0, op, "mute command: can't find player %s",
                             name);

        return COMMANDS_RTN_VAL_ERROR;
    }

    if(seconds < 0)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "mute command: illegal seconds parameter (%d)",
                             seconds);

        return COMMANDS_RTN_VAL_ERROR;
    }

    if(!seconds) /* unmute player */
    {
        new_draw_info(NDI_UNIQUE, 0, op, "mute command: unmuting player %s!",
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
int command_silence(object *op, char *params)
{
    return COMMANDS_RTN_VAL_OK;
}

/* /ban usage:
 *    /ban list
 *    /ban remove <name or ip>
 *    /ban name <name> <seconds>
 *    /ban ip <IP> <seconds>
 *    /ban add <name> <seconds>
 *    Note:  /ban add will ban ip AND name. Player must be online.
 *    Note:  <seconds> = -1 means permanent ban (GMs and SAs only) */
int command_ban(object *op, char *params)
{
    char *mode,
         *str,
          mode_buf[MEDIUM_BUF] = "",
          str_buf[MEDIUM_BUF] = "";
    int   seconds = 60;  // Default ban is 1 minute

    if (!op)
        return COMMANDS_RTN_VAL_ERROR;

    if (!params)
        return COMMANDS_RTN_VAL_SYNTAX;

    // TODO - Should really to a tolower() on params first

    sscanf(params, "%s %s %d", mode_buf, str_buf, &seconds);

    mode = cleanup_string((mode = mode_buf));
    str = cleanup_string((str = str_buf));
    if (seconds < -1) seconds = -1;

    if (!strcmp(mode, "list"))
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Ban List");
        new_draw_info(NDI_UNIQUE, 0, op, "--- --- ---");

        BanList(op, FALSE); // Print list of banned names
        BanList(op, TRUE);  // Print list of banned IPs

        return COMMANDS_RTN_VAL_OK_SILENT;
    }
    else if (!strcmp(mode, "add") || !strcmp(mode, "name") || !strcmp(mode, "ip"))
        return BanAdd(op, mode, str, seconds);

    else if (!strcmp(mode, "remove"))
        return BanRemove(op, str);

    else
        return COMMANDS_RTN_VAL_SYNTAX;
}

/* Helper functions for the main ban command */
static int BanList(object *op, int isIP)
{
    objectlink *ol;

    for(ol = (isIP ? ban_list_ip : ban_list_player); ol; ol = ol->next)
    {
        if (ol->objlink.ban->ticks_init != -1 &&
            pticks >= ol->objlink.ban->ticks)
            remove_ban_entry(ol); /* is not valid anymore, gc it on the fly */
        else
            if (ol->objlink.ban->ticks_init == -1)
                new_draw_info(NDI_UNIQUE, 0, op, "%s -> Permanently banned",
                                     (isIP ? ol->objlink.ban->ip : ol->objlink.ban->name));
            else
                new_draw_info(NDI_UNIQUE, 0, op, "%s -> %lu left (of %d) sec",
                                     (isIP ? ol->objlink.ban->ip : ol->objlink.ban->name),
                                     (ol->objlink.ban->ticks - pticks) / 8,
                                     ol->objlink.ban->ticks_init / 8);
    }

    return COMMANDS_RTN_VAL_OK_SILENT;
}

/* Add a player to the ban list, using name or IP, or add (which does both) */
static int BanAdd(object *op, char *mode, char *str, int seconds)
{
    int tmp;

    if (!str || !mode)
        return COMMANDS_RTN_VAL_SYNTAX;

    /* Check for permanent ban restrictions */
    if (!(CONTR(op)->gmaster_mode & (GMASTER_MODE_SA | GMASTER_MODE_GM)) &&
         (seconds == -1))
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Only GMs and SAs can permanently ban or unban a player or IP!");
        return COMMANDS_RTN_VAL_ERROR;
    }

    if (!strcmp(mode, "add"))
    {
        player *pl = find_player(str);

        /* For "add" command, player must be online ... */
        if (!pl)
        {
            new_draw_info(NDI_UNIQUE, 0, op,
                          "Can't find the player %s!\nCheck name, or use /ban <name> or /ban <ip> directly.",
                          STRING_SAFE(str));

            return COMMANDS_RTN_VAL_ERROR;
        }

        /* add name to the ban list */
        tmp = BanAddToBanList(op, str, seconds, FALSE);
        if ((tmp == COMMANDS_RTN_VAL_SYNTAX) || (tmp == COMMANDS_RTN_VAL_ERROR))
            return tmp;

        /* add ip to the ban list */
        tmp = BanAddToBanList(op, pl->socket.ip_host, seconds, TRUE);
        if ((tmp == COMMANDS_RTN_VAL_SYNTAX) || (tmp == COMMANDS_RTN_VAL_ERROR))
            return tmp;

        kick_player(pl);
    }
    else if (!strcmp(mode, "name"))
    {
        player *pl;

        /* add name to the ban list */
        tmp = BanAddToBanList(op, str, seconds, FALSE);
        if ((tmp == COMMANDS_RTN_VAL_SYNTAX) || (tmp == COMMANDS_RTN_VAL_ERROR))
            return tmp;

        if ((pl = find_player(str)))
            kick_player(pl);
    }
    else if (!strcmp(mode, "ip"))
    {
        int spot;

        // TODO - TW - Figure out how this works, and do some testing, then update message
        for (spot = 0; str[spot] != '\0'; spot++)
        {
            if ((CONTR(op)->gmaster_mode == GMASTER_MODE_VOL &&
                 str[spot] == '*') ||
                (CONTR(op)->gmaster_mode >= GMASTER_MODE_GM &&
                 str[spot] == '*' &&
                 spot < 11))
            {
                new_draw_info(NDI_UNIQUE, 0, op, "Need a better message here! Something to do with banning all domains?");
                return COMMANDS_RTN_VAL_ERROR;
            }
        }

        tmp = BanAddToBanList(op, str, seconds, TRUE);
        if ((tmp == COMMANDS_RTN_VAL_SYNTAX) || (tmp == COMMANDS_RTN_VAL_ERROR))
            return tmp;

        // TODO - We should try and kick all players who have this IP, but no suitable function written
    }

    save_ban_file();

    return COMMANDS_RTN_VAL_OK;
}

static int BanAddToBanList(object* op, char *str, int seconds, int isIP)
{
    char buf[SMALL_BUF];
    int  tmp;

    tmp = BanRemoveFromBanList(op, str, isIP, TRUE);
    if ((tmp == COMMANDS_RTN_VAL_SYNTAX) || (tmp == COMMANDS_RTN_VAL_ERROR))
        return tmp;

    if (seconds != -1)
        sprintf(buf, "%s %s is now banned for %d seconds", isIP ? "IP" : "Player", str, seconds);
    else
        sprintf(buf, "%s %s is now banned permanently", isIP ? "IP" : "Player", str);

    new_draw_info(NDI_UNIQUE, 0, op, "%s.", buf);
    LOG(llevSystem, "%s by %s.", buf, STRING_OBJ_NAME(op));

    if (seconds != -1)
        seconds *= 8;     /* convert seconds to ticks */

    if (isIP)
        add_ban_entry(NULL, str, seconds, seconds);
    else
        add_ban_entry(str, NULL, seconds, seconds);

    return COMMANDS_RTN_VAL_OK;
}

/* Remove name or IP from ban lists */
static int BanRemove(object *op, char *str)
{
    int success = FALSE,
        tmp;

    if (!str)
        return COMMANDS_RTN_VAL_SYNTAX;

    // Remove entries from IP list first
    tmp = BanRemoveFromBanList(op, str, TRUE, FALSE);
    if (tmp == COMMANDS_RTN_VAL_OK)
        success = TRUE;
    else if ((tmp == COMMANDS_RTN_VAL_SYNTAX) || (tmp == COMMANDS_RTN_VAL_ERROR))
        return tmp;

    // And Name list second
    tmp = BanRemoveFromBanList(op, str, FALSE, FALSE);
    if (tmp == COMMANDS_RTN_VAL_OK)
        success = TRUE;
    else if ((tmp == COMMANDS_RTN_VAL_SYNTAX) || (tmp == COMMANDS_RTN_VAL_ERROR))
        return tmp;

    if(success)
    {
        save_ban_file();
        return COMMANDS_RTN_VAL_OK;
    }
    else
    {
        // Both calls to BanRemoveFromBanList must have returned COMMANDS_RTN_VAL_OK_SILENT
        new_draw_info(NDI_UNIQUE, 0, op, "No player or IP found to match %s on ban lists!", str);
        return COMMANDS_RTN_VAL_ERROR;
    }
}

static int BanRemoveFromBanList(object *op, char *str, int isIP, int mute)
{
    objectlink *ol;
    const char *str_hash = NULL;

    if (!isIP)
    {
        transform_player_name_string(str);
        str_hash = find_string(str); /* we need an shared string to check ban list */
    }

    /* Either search the IP or name lists */
    for(ol = (isIP ? ban_list_ip : ban_list_player); ol; ol = ol->next)
    {
        /* Check for IP match or name match */
        if (isIP ? (!strcmp(ol->objlink.ban->ip, str)) : (ol->objlink.ban->name == str_hash))
        {
            if (!(CONTR(op)->gmaster_mode & (GMASTER_MODE_SA | GMASTER_MODE_GM)) &&
                 (ol->objlink.ban->ticks_init == -1))
            {
                new_draw_info(NDI_UNIQUE, 0, op, "Only GMs and SAs can ban or unban permanently banned %s", isIP ? "IPs" : "players!");
                return COMMANDS_RTN_VAL_ERROR;
            }

            remove_ban_entry(ol);

            if (!mute)
            {
                new_draw_info(NDI_UNIQUE, 0, op, "You unbanned %s %s.", isIP ? "IP" : "Player", str);
                LOG(llevSystem,"%s %s is unbanned by %s", isIP ? "IP" : "Player", str, STRING_OBJ_NAME(op));
            }

            return COMMANDS_RTN_VAL_OK;  // Player or IP can only be on ban list once
        }
    }

    return COMMANDS_RTN_VAL_OK_SILENT;
}

/* become a SA */
int command_sa(object *op, char *params)
{
    player *pl;

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
int command_mm(object *op, char *params)
{
    player *pl;

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
int command_mw(object *op, char *params)
{
    player *pl;

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
int command_gm(object *op, char *params)
{
    player *pl;

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
int command_vol(object *op, char *params)
{
    player *pl;

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
int command_gmasterlist(object *op, char *params)
{
    objectlink *ol;

    if (params)
        return COMMANDS_RTN_VAL_SYNTAX;

    for(ol = gmaster_list_VOL; ol; ol = ol->next)
    {
        if(!CONTR(ol->objlink.ob)->privacy)
        {
            new_draw_info(NDI_UNIQUE, 0, op, "%s",
                          CONTR(ol->objlink.ob)->quick_name);
        }
    }

    for(ol = gmaster_list_GM; ol; ol = ol->next)
    {
        if(!CONTR(ol->objlink.ob)->privacy)
        {
            new_draw_info(NDI_UNIQUE, 0, op, "%s",
                          CONTR(ol->objlink.ob)->quick_name);
        }
    }

    for(ol = gmaster_list_MW; ol; ol = ol->next)
    {
        if(!CONTR(ol->objlink.ob)->privacy)
        {
            new_draw_info(NDI_UNIQUE, 0, op, "%s",
                          CONTR(ol->objlink.ob)->quick_name);
        }
    }

    for(ol = gmaster_list_MM; ol; ol = ol->next)
    {
        if(!CONTR(ol->objlink.ob)->privacy)
        {
            new_draw_info(NDI_UNIQUE, 0, op, "%s",
                          CONTR(ol->objlink.ob)->quick_name);
        }
    }

    for(ol = gmaster_list_SA; ol; ol = ol->next)
    {
        if(!CONTR(ol->objlink.ob)->privacy)
        {
            new_draw_info(NDI_UNIQUE, 0, op, "%s",
                          CONTR(ol->objlink.ob)->quick_name);
        }
    }

    return COMMANDS_RTN_VAL_OK_SILENT;
}

/* Lists, adds, or removes entries in gmaster_file. */
int command_gmasterfile(object *op, char *params)
{
    player     *pl;
    char       *cp;
    shstr      *hash = NULL;
    objectlink *ol;
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
            new_draw_info(NDI_UNIQUE, 0, op, "%s", ol->objlink.gm->entry);
        }

        return COMMANDS_RTN_VAL_OK_SILENT;
    }

    /* Find the subcommand. */
    if ((cp = strchr(params, ' ')))
    {
        *(cp++) = '\0';

        while (*cp == ' ')
        {
            cp++;
        }

        hash = add_string(params);
    }
    /* For this command all subcommands have further parmeters, so no space in
     * params means the player typed nonsense. */
    else
    {
        return COMMANDS_RTN_VAL_SYNTAX;
    }

    /* Add an entry. */
    if (hash == subcommands.add)
    {
        FREE_AND_CLEAR_HASH(hash);

        if (sscanf(cp, "%[^/]/%[^/]/%s", name, host, mode) != 3 ||
            (mode_id = check_gmaster_file_entry(name, host, mode)) == GMASTER_MODE_NO)
        {
            new_draw_info(NDI_UNIQUE, 0, op, "Malformed or missing parameter.");

            return COMMANDS_RTN_VAL_SYNTAX;
        }

        if (!compare_gmaster_mode(mode_id, pl->gmaster_mode))
        {
            new_draw_info(NDI_UNIQUE, 0, op, "You have insufficient permission to add this entry.");

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
                new_draw_info(NDI_UNIQUE, 0, op, "There is no account of that name.");

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
    else if (hash == subcommands.list)
    {
        int success = COMMANDS_RTN_VAL_SYNTAX;

        FREE_AND_CLEAR_HASH(hash);

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
                    new_draw_info(NDI_UNIQUE, 0, op, "%s", gm->entry);
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
                    new_draw_info(NDI_UNIQUE, 0, op, "%s", gm->entry);
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
                    new_draw_info(NDI_UNIQUE, 0, op, "%s", gm->entry);
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
                    new_draw_info(NDI_UNIQUE, 0, op, "%s", gm->entry);
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
                    new_draw_info(NDI_UNIQUE, 0, op, "%s", gm->entry);
                }
            }
        }

        return success;
    }
    /* Remove an entry. */
    else if (hash == subcommands.remove)
    {
        FREE_AND_CLEAR_HASH(hash);

        if (sscanf(cp, "%[^/]/%[^/]/%s", name, host, mode) != 3 ||
            (mode_id = check_gmaster_file_entry(name, host, mode)) == GMASTER_MODE_NO)
        {
            new_draw_info(NDI_UNIQUE, 0, op, "Malformed or missing parameter.");

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
                    new_draw_info(NDI_UNIQUE, 0, op, "You have insufficient permission to remove this entry.");

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
            new_draw_info(NDI_UNIQUE, 0, op, "Entry could not be found!");

           return COMMANDS_RTN_VAL_ERROR;
        }
    }
    /* Unknown subcommand. */
    else
    {
        FREE_AND_CLEAR_HASH(hash);

        return COMMANDS_RTN_VAL_SYNTAX;
    }

    return COMMANDS_RTN_VAL_OK;
}

int command_invisible(object *op, char *params)
{
    if (!op)
        return COMMANDS_RTN_VAL_ERROR;

    if (IS_SYS_INVISIBLE(op))
    {
        CLEAR_FLAG(op, FLAG_SYS_OBJECT);
        new_draw_info(NDI_UNIQUE, 0, op, "You turn visible.");
    }
    else
    {
        SET_FLAG(op, FLAG_SYS_OBJECT);
        new_draw_info(NDI_UNIQUE, 0, op, "You turn invisible.");
    }

    update_object(op, UP_OBJ_FACE);

    return COMMANDS_RTN_VAL_OK_SILENT;
}

static int CommandLearnSpellOrPrayer(object *op, char *params, int special_prayer)
{
    int spell;

    if (!op ||
        op->type != PLAYER ||
        !CONTR(op))
        return COMMANDS_RTN_VAL_ERROR;

    if (!params)
        return COMMANDS_RTN_VAL_SYNTAX;

    if ((spell = look_up_spell_name(params)) <= 0)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Unknown spell.");
        return COMMANDS_RTN_VAL_ERROR;
    }

    do_learn_spell(op, spell, special_prayer);
    return COMMANDS_RTN_VAL_OK;
}

// These 2 are not in the current command list ... don't know why - TW
int command_learn_spell(object *op, char *params)
{
    return CommandLearnSpellOrPrayer(op, params, 0);
}

int command_learn_special_prayer(object *op, char *params)
{
    return CommandLearnSpellOrPrayer(op, params, 1);
}

int command_forget_spell(object *op, char *params)
{
    int spell;

    if (!op ||
        op->type != PLAYER ||
        !CONTR(op))
        return COMMANDS_RTN_VAL_ERROR;

    if (!params)
        return COMMANDS_RTN_VAL_SYNTAX;

    if ((spell = look_up_spell_name(params)) <= 0)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Unknown spell.");

        return COMMANDS_RTN_VAL_ERROR;
    }

    do_forget_spell(op, spell);

    return COMMANDS_RTN_VAL_OK;
}

/* GROS */
/* Lists all plugins currently loaded with their IDs and full names.         */
int command_listplugins(object *op, char *params)
{
    displayPluginsList(op);

    return COMMANDS_RTN_VAL_OK;
}

/* GROS */
/* Loads the given plugin. The DM specifies the name of the library to load  */
/* (no pathname is needed). Do not ever attempt to load the same plugin more */
/* than once at a time, or bad things could happen.                          */
int command_loadplugin(object *op, char *params)
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
int command_unloadplugin(object *op, char *params)
{
    if (!params)
        return COMMANDS_RTN_VAL_SYNTAX;

    removeOnePlugin(params);

    return COMMANDS_RTN_VAL_OK;
}

int command_ip(object *op, char *params)
{
    player *pl;

    if (!params)
        return COMMANDS_RTN_VAL_SYNTAX;

    if (!(pl = find_player(params)))
    {
        new_draw_info(NDI_UNIQUE | NDI_WHITE, 0, op, "No such player!");

        return COMMANDS_RTN_VAL_ERROR;
    }

    new_draw_info(NDI_UNIQUE | NDI_WHITE, 0, op, "IP of %s is %s",
                         params, pl->socket.ip_host);

    return COMMANDS_RTN_VAL_OK_SILENT;
}

/* Toggle wizpass (walk through walls, do not move apply, etc). */
int command_wizpass(object *op, char *params)
{
    uint8 wizpass;

    if (!op)
        return COMMANDS_RTN_VAL_ERROR;

    wizpass = (CONTR(op)->wizpass) ? 0 : 1;

    new_draw_info(NDI_UNIQUE, 0, op, "Wizpass set to %d.", wizpass);
    CONTR(op)->wizpass = wizpass;

    return COMMANDS_RTN_VAL_OK_SILENT;
}
