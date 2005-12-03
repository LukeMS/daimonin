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

#define EXTERN
#define INIT_C

#include <global.h>

struct Settings settings    =
{
    "",                                 /* Logfile */
    CSPORT,                             /* Client/server port */
    GLOBAL_LOG_LEVEL, 0, NULL, 0,       /* dumpvalues, dumparg, daemonmode */
    0,                                  /* argc */
    NULL,                               /* argv */
    DATADIR, LOCALDIR, PLAYERDIR, MAPDIR, ARCHETYPES, TREASURES, UNIQUE_DIR, TMPDIR, STATS_DIR, STATS_ARCHIVE_DIR, STAT_LOSS_ON_DEATH,
    BALANCED_STAT_LOSS, RESET_LOCATION_TIME, 0,
    /* This and the next 3 values are metaserver values */
    "", "", 0, "", 0, 0, 0, 0, 0, 0, 0  /* worldmap settings*/
};

/* Most of this is shamelessly stolen from XSysStats.  But since that is
 * also my program, no problem.
 */
struct Command_Line_Options
{
    char                       *cmd_option;     /* how it is called on the command line */
    uint8                       num_args;       /* Number or args it takes */
    uint8                       pass;           /* What pass this should be processed on. */
    void                      (*func)();        /* function to call when we match this.
                                                 * if num_args is true, than that gets passed
                                                 * to the function, otherwise nothing is passed
                                                 */
};


/*
 * Initialises global variables which can be changed by options.
 * Called by init_library().
 */
static void init_defaults()
{
    nroferrors = 0;
}

/*
 * Initialises global shared strings that we can use in comparisions
 */
static void init_strings()
{
    shstr_cons.undead = add_string("undead");
    shstr_cons.none = add_string("none"); /* Used in god code */
    shstr_cons.NONE = add_string("NONE"); /* Used in alchemy code */
    shstr_cons.quarterstaff = add_string("quarterstaff");
    shstr_cons.battleground = add_string("battleground");

    shstr_cons.clawing = add_string("clawing");
    shstr_cons.dragon_skin_force = add_string("dragon_skin_force");
    shstr_cons.dragon_ability_force = add_string("dragon_ability_force");
    shstr_cons.dragon = add_string("dragon");

    shstr_cons.town_portal_destination = add_string("Town portal destination");
    shstr_cons.existing_town_portal = add_string("Existing town portal");

    shstr_cons.player = add_string("player");
    shstr_cons.money = add_string("money");
    shstr_cons.RANK_FORCE = add_string("RANK_FORCE");
    shstr_cons.rank_force = add_string("rank_force");
    shstr_cons.ALIGNMENT_FORCE = add_string("ALIGNMENT_FORCE");
    shstr_cons.alignment_force = add_string("alignment_force");
    shstr_cons.GUILD_FORCE = add_string("GUILD_FORCE");
    shstr_cons.guild_force = add_string("guild_force");
    shstr_cons.player_info = add_string("player_info");

    shstr_cons.special_prayer = add_string("special prayer");
    shstr_cons.grace_limit = add_string("grace limit");
    shstr_cons.restore_grace = add_string("restore grace");
    shstr_cons.restore_hitpoints = add_string("restore hitpoints");
    shstr_cons.restore_hitpoints = add_string("restore spellpoints");
    shstr_cons.heal_spell = add_string("heal spell");
    shstr_cons.remove_curse = add_string("remove curse");
    shstr_cons.remove_damnation = add_string("remove damnation");
    shstr_cons.heal_depletion = add_string("heal depletion");
    shstr_cons.message = add_string("message");
    shstr_cons.enchant_weapon = add_string("enchant weapon");

    shstr_cons.Eldath = add_string("Eldath"); /* old and incorrect god */
    shstr_cons.the_Tabernacle = add_string("the Tabernacle"); /* corrected god */
}

/*
 * Initialises all global variables.
 * Might use environment-variables as default for some of them.
 */

static void init_globals()
{
    if (settings.logfilename[0] == 0)
    {
        logfile = stderr;
    }
    else if ((logfile = fopen(settings.logfilename, "w")) == NULL)
    {
        logfile = stderr;
        LOG(llevInfo, "Unable to open %s as the logfile - will use stderr instead\n", settings.logfilename);
    }

    level_up_arch = NULL;
    arch_cmp = 0;       /* How many strcmp's */
    arch_search = 0;    /* How many searches */
    arch_init = 0;      /* True if doing arch initialization */

    pticks = 1;        /* global round ticker ! this is real a global */

    global_race_counter = 0; /* global race counter */
    global_group_tag=0; /* every group gets a unique group tag identifier */

    set_pticks_time(MAX_TIME);

    gmaster_list=NULL;
    gmaster_list_VOL=NULL;
    gmaster_list_GM=NULL;
    gmaster_list_DM=NULL;

    ban_list_player=NULL;
    ban_list_ip=NULL;

    exiting = 0;
    player_active_meta = player_active = 0;
    first_god = NULL;
    first_race = NULL;
    first_player = NULL;
    last_player = NULL;
    first_map = NULL;
    first_treasurelist = NULL;
    first_artifactlist = NULL;
    first_archetype = NULL;
    nroftreasures = 0;
    nrofartifacts = 0;
    nrofallowedstr = 0;

	/* thats used in socket/loop.c right after we have a connect */	
	sprintf(global_version_msg, "X%d %d %s", VERSION_CS, VERSION_SC, VERSION_INFO);
	global_version_msg[0] = BINARY_CMD_VERSION;
	global_version_sl.buf = global_version_msg;
	global_version_sl.len = strlen((char *) global_version_msg);

    init_strings();

    trying_emergency_save = 0;
    num_animations = 0;
    animations = NULL;
    animations_allocated = 0;
    init_defaults();
}

/* init_environ initializes values from the environmental variables.
 * it needs to be called very early, since command line options should
 * overwrite these if specified.
 */
static void init_environ()
{
    char   *cp;

#ifndef SECURE
    cp = getenv("CROSSFIRE_LIBDIR");
    if (cp)
        settings.datadir = cp;
    cp = getenv("CROSSFIRE_LOCALDIR");
    if (cp)
        settings.localdir = cp;
    cp = getenv("CROSSFIRE_PLAYERDIR");
    if (cp)
        settings.playerdir = cp;
    cp = getenv("CROSSFIRE_MAPDIR");
    if (cp)
        settings.mapdir = cp;
    cp = getenv("CROSSFIRE_ARCHETYPES");
    if (cp)
        settings.archetypes = cp;
    cp = getenv("CROSSFIRE_TREASURES");
    if (cp)
        settings.treasures = cp;
    cp = getenv("CROSSFIRE_UNIQUEDIR");
    if (cp)
        settings.uniquedir = cp;
    cp = getenv("CROSSFIRE_TMPDIR");
    if (cp)
        settings.tmpdir = cp;
#endif
}


static void init_dynamic()
{
    archetype  *at  = first_archetype;

    map_archeytpe = NULL;
    while (at)
    {
        if (at->clone.type == MAP && EXIT_PATH(&at->clone))
        {
            map_archeytpe = at;
            return;
        }
        at = at->next;
    }
    LOG(llevError, "init_dynamic (): You Need a archetype called 'map' and it have to contain start map\n");
}


/* This loads the settings file.  There could be debate whether this should
 * be here or in the common directory - but since only the server needs this
 * information, having it here probably makes more sense.
 */
static void load_settings()
{
    char    buf[MAX_BUF], *cp;
    int     has_val, i;
    FILE   *fp;

    sprintf(buf, "%s/%s", settings.localdir, SETTINGS);
    /* We don't require a settings file at current time, but down the road,
     * there will probably be so many values that not having a settings file
     * will not be a good thing.
     */
    if ((fp = fopen(buf, "r")) == NULL)
    {
        LOG(llevBug, "BUG: No %s file found\n", SETTINGS);
        return;
    }
    while (fgets(buf, MAX_BUF - 1, fp) != NULL)
    {
        if (buf[0] == '#')
            continue;
        /* eliminate newline */
        i = strlen(buf) - 1;
        while(isspace(buf[i]))
            --i;
        buf[i + 1] = '\0';

        /* Skip over empty lines */
        if (buf[0] == 0)
            continue;

        /* Skip all the spaces and set them to nulls.  If not space,
         * set cp to "" to make strcpy's and the like easier down below.
         */
        if ((cp = strchr(buf, ' ')) != NULL)
        {
            while (*cp == ' ')
                *cp++ = 0;
            has_val = 1;
        }
        else
        {
            cp = "";
            has_val = 0;
        }

        if (!strcasecmp(buf, "metaserver_notification"))
        {
            if (!strcasecmp(cp, "on") || !strcasecmp(cp, "true"))
            {
                settings.meta_on = TRUE;
            }
            else if (!strcasecmp(cp, "off") || !strcasecmp(cp, "false"))
            {
                settings.meta_on = FALSE;
            }
            else
            {
                LOG(llevBug, "BUG: load_settings: Unkown value for metaserver_notification: %s\n", cp);
            }
        }
        else if (!strcasecmp(buf, "metaserver_server"))
        {
            if (has_val)
                strcpy(settings.meta_server, cp);
            else
                LOG(llevBug, "BUG: load_settings: metaserver_server must have a value.\n");
        }
        else if (!strcasecmp(buf, "metaserver_host"))
        {
            if (has_val)
                strcpy(settings.meta_host, cp);
            else
                LOG(llevBug, "BUG: load_settings: metaserver_host must have a value.\n");
        }
        else if (!strcasecmp(buf, "metaserver_port"))
        {
            int port    = atoi(cp);

            if (port<1 || port>65535)
                LOG(llevBug, "BUG: load_settings: metaserver_port must be between 1 and 65535, %d is invalid\n", port);
            else
                settings.meta_port = port;
        }
        else if (!strcasecmp(buf, "metaserver_comment"))
        {
            strcpy(settings.meta_comment, cp);
        }
        else if (!strcasecmp(buf, "worldmapstartx"))
        {
            int size    = atoi(cp);

            if (size < 0)
                LOG(llevBug, "BUG: load_settings: worldmapstartx must be at least" "0, %d is invalid\n", size);
            else
                settings.worldmapstartx = size;
        }
        else if (!strcasecmp(buf, "worldmapstarty"))
        {
            int size    = atoi(cp);

            if (size < 0)
                LOG(llevBug, "BUG: load_settings: worldmapstarty must be at least" "0, %d is invalid\n", size);
            else
                settings.worldmapstarty = size;
        }
        else if (!strcasecmp(buf, "worldmaptilesx"))
        {
            int size    = atoi(cp);

            if (size < 1)
                LOG(llevBug, "BUG: load_settings: worldmaptilesx must be greater" "than 1, %d is invalid\n", size);
            else
                settings.worldmaptilesx = size;
        }
        else if (!strcasecmp(buf, "worldmaptilesy"))
        {
            int size    = atoi(cp);

            if (size < 1)
                LOG(llevBug, "BUG: load_settings: worldmaptilesy must be greater" "than 1, %d is invalid\n", size);
            else
                settings.worldmaptilesy = size;
        }
        else if (!strcasecmp(buf, "worldmaptilesizex"))
        {
            int size    = atoi(cp);

            if (size < 1)
                LOG(llevBug, "BUG: load_settings: worldmaptilesizex must be" "greater than 1, %d is invalid\n", size);
            else
                settings.worldmaptilesizex = size;
        }
        else if (!strcasecmp(buf, "worldmaptilesizey"))
        {
            int size    = atoi(cp);

            if (size < 1)
                LOG(llevBug, "BUG: load_settings: worldmaptilesizey must be" "greater than 1, %d is invalid\n", size);
            else
                settings.worldmaptilesizey = size;
        }
        else if (!strcasecmp(buf, "dynamiclevel"))
        {
            int lev = atoi(cp);

            if (lev < 0)
                LOG(llevBug, "BUG: load_settings: dynamiclevel must be" "at least 0, %d is invalid\n", lev);
            else
                settings.dynamiclevel = lev;
        }
        else
        {
            LOG(llevBug, "BUG: Unknown value in %s file: %s\n", SETTINGS, buf);
        }
    }
    fclose(fp);
}

static void usage()
{
    LOG(llevInfo, "Usage: daimonin_server [-h] [-<flags>]...\n");
}


/*
 * Initializes the gametime and TOD counters
 * Called by init_library().
 */

static void init_clocks()
{
    char        filename[MAX_BUF];
    FILE       *fp;
    static int  has_been_done   = 0;

    if (has_been_done)
        return;
    else
        has_been_done = 1;

    sprintf(filename, "%s/clockdata", settings.localdir);
    LOG(llevDebug, "Reading clockdata from %s...", filename);
    if ((fp = fopen(filename, "r")) == NULL)
    {
        LOG(llevBug, "BUG: Can't open %s.\n", filename);
        todtick = 0;
        write_todclock();
        return;
    }
    fscanf(fp, "%lu", &todtick);
    LOG(llevDebug, "todtick=%lu\n", todtick);
    fclose(fp);
}

static void set_logfile(char *val)
{
    settings.logfilename = val;
}
static void call_version()
{
    version(NULL); exit(0);
}
static void showscores()
{
    display_high_score(NULL, 9999, NULL); exit(0);
}

static void set_debug()
{
    settings.debug = llevDebug;
}
static void unset_debug()
{
    settings.debug = llevInfo;
}
static void set_mondebug()
{
    settings.debug = llevMonster;
}
static void set_dumpmon1()
{
    settings.dumpvalues = 1;
}
static void set_dumpmon2()
{
    settings.dumpvalues = 2;
}
static void set_dumpmon3()
{
    settings.dumpvalues = 3;
}
static void set_dumpmon4()
{
    settings.dumpvalues = 4;
}
static void set_dumpmon5()
{
    settings.dumpvalues = 5;
}
static void set_dumpmon6()
{
    settings.dumpvalues = 6;
}
static void set_dumpmon7()
{
    settings.dumpvalues = 7;
}
static void set_dumpmon8()
{
    settings.dumpvalues = 8;
}
static void set_dumpmon9()
{
    settings.dumpvalues = 9;
}
static void set_dumpmonA()
{
    settings.dumpvalues = 11;
}
static void set_dumpmont(char *name)
{
    settings.dumpvalues = 10; settings.dumparg = name;
}
static void set_daemon()
{
    settings.daemonmode = 1;
}
static void set_datadir(char *path)
{
    settings.datadir = path;
}
static void set_localdir(char *path)
{
    settings.localdir = path;
}
static void set_mapdir(char *path)
{
    settings.mapdir = path;
}
static void set_archetypes(char *path)
{
    settings.archetypes = path;
}
static void set_treasures(char *path)
{
    settings.treasures = path;
}
static void set_uniquedir(char *path)
{
    settings.uniquedir = path;
}
static void set_playerdir(char *path)
{
    settings.playerdir = path;
}
static void set_tmpdir(char *path)
{
    settings.tmpdir = path;
}
static void showscoresparm(char *data)
{
    /*    display_high_score(NULL,9999,data); */
    exit(0);
}

static void set_csport(char *val)
{
    settings.csport = atoi(val);
#ifndef WIN32 /* ***win32: set_csport: we remove csport error secure check here, do this later */
    if (settings.csport <= 0 || settings.csport > 32765 || (settings.csport < 1024 && getuid() != 0))
    {
        LOG(llevError, "ERROR: %d is an invalid csport number.\n", settings.csport);
    }
#endif /* win32 */
}

static void stat_loss_on_death_true()
{
    settings.stat_loss_on_death = 1;
}
static void stat_loss_on_death_false()
{
    settings.stat_loss_on_death = 0;
}

static void balanced_stat_loss_true()
{
    settings.balanced_stat_loss = 1;
}
static void balanced_stat_loss_false()
{
    settings.balanced_stat_loss = 0;
}


static void help()
{
    /* The information in usage is redundant with what is given below, so why call it? */
    /*    usage();*/
    LOG(llevInfo, "Flags:\n");
    LOG(llevInfo, " -csport <port> Specifies the port to use for the new client/server code.\n");
    LOG(llevInfo, " -d          Turns on some debugging.\n");
    LOG(llevInfo, " +d          Turns off debugging (useful if server compiled with debugging\n");
    LOG(llevInfo, "             as default).\n");
    LOG(llevInfo, " -detach     The server will go in the background, closing all\n");
    LOG(llevInfo, "             connections to the tty.\n");
    LOG(llevInfo, " -h          Display this information.\n");
    LOG(llevInfo, " -log <file> Specifies which file to send output to.\n");
    LOG(llevInfo, "             Only has meaning if -detach is specified.\n");
    LOG(llevInfo, " -mon        Turns on monster debugging.\n");
    LOG(llevInfo, " -o          Prints out info on what was defined at compile time.\n");
    LOG(llevInfo, " -s          Display the high-score list.\n");
    LOG(llevInfo, " -score <name or class> Displays all high scores with matching name/class.\n");
    LOG(llevInfo, " -stat_loss_on_death - if set, player loses stat when they die\n");
    LOG(llevInfo, " +stat_loss_on_death - if set, player does not lose a stat when they die\n");
    LOG(llevInfo, " -balanced_stat_loss - if set, death stat depletion is balanced by level etc\n");
    LOG(llevInfo, " +balanced_stat_loss - if set, ordinary death stat depletion is used\n");
    LOG(llevInfo, " -v          Print version and contributors.\n");
    LOG(llevInfo, " -test       Run unit tests and exit.\n");
    LOG(llevInfo, " -benchmark  Run benchmarks and exit.\n");

#ifndef SECURE
    LOG(llevInfo, "\nThe following options are only available if a secure server was not compiled.\n");
    LOG(llevInfo, " -data       Sets the lib dir (archetypes, treasures, etc.)\n");
    LOG(llevInfo, " -local      Read/write local data (hiscore, unique items, etc.)\n");
    LOG(llevInfo, " -maps       Sets the directory for maps.\n");
    LOG(llevInfo, " -arch       Sets the archetype file to use.\n");
    LOG(llevInfo, " -playerdir  Sets the directory for the player files.\n");
    LOG(llevInfo, " -treasures     Sets the treasures file to use.\n");
    LOG(llevInfo, " -uniquedir  Sets the unique items/maps directory.\n");
    LOG(llevInfo, " -tmpdir     Sets the directory for temporary files (mostly maps.)\n");
#endif

#ifdef DUMP_SWITCHES
    LOG(llevInfo, "\nThe following are only available in DUMP_SWITCHES was compiled in.\n");
    LOG(llevInfo, " -m1         Dumps out object settings for all monsters.\n");
    LOG(llevInfo, " -m2         Dumps out abilities for all monsters.\n");
    LOG(llevInfo, " -m3         Dumps out artificat information.\n");
    LOG(llevInfo, " -m4         Dumps out spell information.\n");
    LOG(llevInfo, " -m5         Dumps out skill information.\n");
    LOG(llevInfo, " -m6         Dumps out race information.\n");
    LOG(llevInfo, " -m7         Dumps out alchemy information.\n");
    LOG(llevInfo, " -m8         Dumps out gods information.\n");
    LOG(llevInfo, " -m9         Dumps out more alchemy information (formula checking).\n");
    LOG(llevInfo, " -mA         Dumps out all arches.\n");
    LOG(llevInfo, " -mt <arch>  Dumps out list of treasures for a monster.\n");
#endif
    exit(0);
}

/* Signal handlers: */

static void rec_sigsegv(int i)
{
    LOG(llevSystem, "\nSIGSEGV received.\n");
    fatal_signal(1, 1);
}

static void rec_sigint(int i)
{
    LOG(llevSystem, "\nSIGINT received.\n");
    fatal_signal(0, 1);
}

static void rec_sighup(int i)
{
    LOG(llevSystem, "\nSIGHUP received\n");
    if (init_done)
    {
        emergency_save(0);
        cleanup(0);
    }
    exit(0);
}

static void rec_sigquit(int i)
{
    LOG(llevSystem, "\nSIGQUIT received\n");
    fatal_signal(1, 1);
}

static void rec_sigpipe(int i)
{
    /* Keep running if we receive a sigpipe.  Crossfire should really be able
     * to handle this signal (at least at some point in the future if not
     * right now).  By causing a dump right when it is received, it is not
     * doing much good.  However, if it core dumps later on, at least it can
     * be looked at later on, and maybe fix the problem that caused it to
     * dump core.  There is no reason that SIGPIPES should be fatal
     */
#if 1 && !defined(WIN32) /* ***win32: we don't want send SIGPIPE */
    /*LOG(llevSystem, "\nReceived SIGPIPE, ignoring...\n");*/
    signal(SIGPIPE, rec_sigpipe);/* hocky-pux clears signal handlers */
#else
    LOG(llevSystem, "\nSIGPIPE received, not ignoring...\n");
    fatal_signal(1, 1); /*Might consider to uncomment this line */
#endif
}

static void rec_sigbus(int i)
{
#ifdef SIGBUS
    LOG(llevSystem, "\nSIGBUS received\n");
    fatal_signal(1, 1);
#endif
}

static void rec_sigterm(int i)
{
    LOG(llevSystem, "\nSIGTERM received\n");
    fatal_signal(0, 1);
}


static void init_signals()
{
#ifndef WIN32 /* init_signals() remove signals */
    signal(SIGHUP, rec_sighup);
    signal(SIGINT, rec_sigint);
    signal(SIGQUIT, rec_sigquit);
    signal(SIGSEGV, rec_sigsegv);
    signal(SIGPIPE, rec_sigpipe);
#ifdef SIGBUS
    signal(SIGBUS, rec_sigbus);
#endif
    signal(SIGTERM, rec_sigterm);
#endif /* win32 */
}

static void add_ai_to_racelist(const char *race_name, archetype *op)
{
    racelink   *race;

    if (!op || !race_name)
        return;

    race = find_racelink(race_name);

    /* if we don't have this race, just skip the corpse.
     * perhaps we add later the race or this corpse has
     * special use (put by hand on map or by script)
     */
    if (race)
    {
        LOG(llevDebug, " %s->'%s' ", STRING_OBJ_RACE(&op->clone), STRING_OBJ_NAME(&op->clone));
        race->ai = parse_behaviourconfig(op->clone.msg, &op->clone);
    }
}

static void add_corpse_to_racelist(const char *race_name, archetype *op)
{
    racelink   *race;

    if (!op || !race_name)
        return;

    race = find_racelink(race_name);

    /* if we don't have this race, just skip the corpse.
     * perhaps we add later the race or this corpse has
     * special use (put by hand on map or by script)
     */
    if (race)
        race->corpse = op;
}

static racelink * get_racelist()
{
    racelink   *list;

    list = (racelink *) malloc(sizeof(racelink));
    list->name = NULL;
    list->corpse = NULL;
    list->nrof = 0;
    list->member = get_objectlink(OBJLNK_FLAG_OB);
    list->next = NULL;
    list->ai = NULL;

    return list;
}

/* free race list
 */
static void free_racelist()
{
    racelink   *list, *next;
    for (list = first_race; list;)
    {
        next = list->next;
        free(list);
        list = next;
    }
}
static void add_to_racelist(const char *race_name, object *op)
{
    racelink   *race;

    if (!op || !race_name)
        return;
    race = find_racelink(race_name);

    if (!race)
    {
        /* add in a new race list */
        global_race_counter++; /* we need this for treasure generation (slaying race) */
        race = get_racelist();
        race->next = first_race;
        first_race = race;
        FREE_AND_COPY_HASH(race->name, race_name);
    }

    if (race->member->objlink.ob)
    {
        objectlink *tmp = get_objectlink(OBJLNK_FLAG_OB);
        tmp->next = race->member;
        race->member = tmp;
    }
    race->nrof++;
    race->member->objlink.ob = op;
}


static void dump_races()
{
    racelink   *list;
    objectlink *tmp;
    for (list = first_race; list; list = list->next)
    {
        LOG(llevInfo, "\nRACE %s (%s - %d member): ", list->name, list->corpse->name, list->nrof);
        for (tmp = list->member; tmp; tmp = tmp->next)
            LOG(llevInfo, "%s(%d), ", tmp->objlink.ob->arch->name, tmp->objlink.ob->sub_type1);
    }
}

/* init_races() - reworked this function - 2003/MT
 * Because we have now a type MONSTER, we can collect the monster arches
 * from the arch list. We use sub_type1 as selector - every monster of
 * race X will be added to race list. Instead of level will be the sub_type1
 * insert in list - the sub_type is used from the functions who need the
 * race list (like summon spells).
 */

void init_races()
{
    archetype  *at, *tmp;
    racelink   *list;
    static int  init_done   = 0;

    if (init_done)
        return;
    init_done = 1;

    first_race = NULL;
    LOG(llevDebug, "Init races... ");

    for (at = first_archetype; at != NULL; at = at->next)
    {
        if (at->clone.type == MONSTER || at->clone.type == PLAYER)
        {
            add_to_racelist(at->clone.race, &at->clone);
        }
    };

    /* now search for ai objects and add them to the race list */
    LOG(llevDebug, " default AIs: ");
    for (at = first_archetype; at != NULL; at = at->next)
    {
        if (at->clone.type == TYPE_AI)
        {
            add_ai_to_racelist(at->clone.race, at);
        }
    };

    /* now search for corpses and add them to the race list */
    for (at = first_archetype; at != NULL; at = at->next)
    {
        if (at->clone.type == CONTAINER && at->clone.sub_type1 == ST1_CONTAINER_CORPSE)
        {
            add_corpse_to_racelist(at->clone.race, at);
        }
    };

    /* last action: for all races without a special defined corpse
     * add our corpse_default arch to it.
     */
    tmp = find_archetype("corpse_default");
    if (!tmp)
        LOG(llevError, "ERROR: init_races: can't find corpse_default in arches!\n");

    for (list = first_race; list; list = list->next)
    {
        if (!list->corpse)
            list->corpse = tmp;
    }

#ifdef DEBUG
    dump_races();
#endif
    LOG(llevDebug, "\ndone.\n");
}


/*
 * compile_info(): activated with the -o flag.
 * It writes out information on how Imakefile and config.h was configured
 * at compile time.
 */
void compile_info()
{
    int i   = 0;
    LOG(llevInfo, "Setup info:\n");
    LOG(llevInfo, "Non-standard include files:\n");
#if !defined (__STRICT_ANSI__) || defined (__sun__)
#if !defined (Mips)
    LOG(llevInfo, "<stdlib.h>\n");
    i = 1;
#endif
#if !defined (MACH) && !defined (sony)
    LOG(llevInfo, "<malloc.h>\n");
    i = 1;
#endif
#endif
#ifndef __STRICT_ANSI__
#ifndef MACH
    LOG(llevInfo, "<memory.h\n");
    i = 1;
#endif
#endif
#ifndef sgi
    LOG(llevInfo, "<sys/timeb.h>\n");
    i = 1;
#endif
    if (!i)
        LOG(llevInfo, "(none)\n");
#ifdef SECURE
    LOG(llevInfo, "Secure:\t\t<true>\n");
#else
    LOG(llevInfo, "Secure:\t\t<false>\n");
#endif
    LOG(llevInfo, "Datadir:\t%s\n", settings.datadir);
    LOG(llevInfo, "Localdir:\t%s\n", settings.localdir);
    LOG(llevInfo, "Save player:\t<true>\n");
    LOG(llevInfo, "Save mode:\t%4.4o\n", SAVE_MODE);
    LOG(llevInfo, "Playerdir:\t%s/%s\n", settings.localdir, settings.playerdir);
    LOG(llevInfo, "Itemsdir:\t%s/%s\n", settings.localdir, settings.uniquedir);
#ifdef USE_CHECKSUM
    LOG(llevInfo, "Use checksum:\t<true>\n");
#else
    LOG(llevInfo, "Use checksum:\t<false>\n");
#endif
    LOG(llevInfo, "Tmpdir:\t\t%s\n", settings.tmpdir);
    LOG(llevInfo, "Map timeout:\t%d\n", MAP_MAXTIMEOUT);
#ifdef MAP_RESET
    LOG(llevInfo, "Map reset:\t<true>\n");
#else
    LOG(llevInfo, "Map reset:\t<false>\n");
#endif
    LOG(llevInfo, "Max objects:\t%d (allocated:%d free:%d)\n", MAX_OBJECTS, pool_object->nrof_allocated,
        pool_object->nrof_free);


#ifdef USE_CALLOC
    LOG(llevInfo, "Use_calloc:\t<true>\n");
#else
    LOG(llevInfo, "Use_calloc:\t<false>\n");
#endif

#ifdef SHOP_LISTINGS
    LOG(llevInfo, "Shop listings:\t<true>\n");
#else
    LOG(llevInfo, "Shop listings:\t<false>\n");
#endif
    LOG(llevInfo, "Max_time:\t%d (%f)\n", pticks_ums, pticks_second);

    LOG(llevInfo, "Logfilename:\t%s (llev:%d)\n", settings.logfilename, settings.debug);
    LOG(llevInfo, "ObjectSize:\t%d (living: %d)\n", sizeof(object), sizeof(living));
    LOG(llevInfo, "MapStructSize:\t%d\n", sizeof(mapstruct));
    LOG(llevInfo, "MapSpaceSize:\t%d\n", sizeof(MapSpace));
    LOG(llevInfo, "PlayerSize:\t%d\n", sizeof(player) + MAXSOCKBUF_IN);
    LOG(llevInfo, "SocketSize:\t%d\n", sizeof(NewSocket) + MAXSOCKBUF_IN);

    LOG(llevInfo, "Setup info: Done.\n");
}

/*
* fatal_signal() is meant to be called whenever a fatal signal is intercepted.
* It will call the emergency_save and the clean_tmp_files functions.
*/
void fatal_signal(int make_core, int close_sockets)
{
    if (init_done)
    {
#ifdef PLUGINS
        removePlugins();
#endif
        emergency_save(0);
        clean_tmp_files();
        write_book_archive();
        write_todclock();   /* lets just write the clock here */
        save_ban_file();
    }
    if (make_core)
        abort();
    exit(0);
}


/* The way this system works is pretty simple - parse_args takes
 * the options passed to the program and a pass number.  If an option
 * matches both in name and in pass (and we have enough options),
 * we call the associated function.  This makes writing a multi
 * pass system very easy, and it is very easy to add in new options.
 */
struct Command_Line_Options options[]   =
{
    /* Pass 1 functions - STuff that can/should be called before we actually
    * initialize any data.
    */
    {"-h", 0, 1, help},
    /* Honor -help also, since it is somewhat common */
    {"-help", 0, 1, help}, 
    {"-v", 0, 1, call_version}, 
    {"-d", 0, 1, set_debug}, 
    {"+d", 0, 1, unset_debug},
    {"-mon", 0, 1, set_mondebug},
#ifndef SECURE
    {"-data",1,1, set_datadir}, 
    {"-local",1,1, set_localdir},
    {"-maps", 1, 1, set_mapdir},
    {"-arch", 1, 1, set_archetypes}, 
    {"-playerdir", 1, 1, set_playerdir}, 
    {"-treasures", 1, 1, set_treasures},
    {"-uniquedir", 1, 1, set_uniquedir}, 
    {"-tmpdir", 1, 1, set_tmpdir},
#endif
    {"-log", 1, 1, set_logfile},

    /* Pass 2 functions.  Most of these could probably be in pass 1,
    * as they don't require much of anything to bet set up.
    */
    {"-csport", 1, 2, set_csport}, 
    {"-detach", 0, 2, set_daemon},

    /* Start of pass 3 information. In theory, by pass 3, all data paths
    * and defaults should have been set up.
    */
    {"-o", 0, 3, compile_info},
#ifdef DUMP_SWITCHES
    {"-m1", 0, 3, set_dumpmon1}, {"-m2", 0, 3, set_dumpmon2}, {"-m3", 0, 3, set_dumpmon3}, {"-m4", 0, 3, set_dumpmon4},
    {"-m5", 0, 3, set_dumpmon5}, {"-m6", 0, 3, set_dumpmon6}, {"-m7", 0, 3, set_dumpmon7}, {"-m8", 0, 3, set_dumpmon8},
    {"-m9", 0, 3, set_dumpmon9}, {"-mA", 0, 3, set_dumpmonA}, {"-mt", 1, 3, set_dumpmont},
#endif
    {"-s", 0, 3, showscores}, 
    {"-score", 1, 3, showscoresparm}, 
    {"-stat_loss_on_death", 0, 3, stat_loss_on_death_true},
    {"+stat_loss_on_death", 0, 3, stat_loss_on_death_false}, 
    {"-balanced_stat_loss", 0, 3, balanced_stat_loss_true},
    {"+balanced_stat_loss", 0, 3, balanced_stat_loss_false},
    {"-test", 0, 3, run_unit_tests},
    {"-benchmark", 0, 3, run_benchmarks}
};

/* Note since this may be called before the library has been set up,
 * we don't use any of crossfires built in logging functions.
 */
static void parse_args(int argc, char *argv[], int pass)
{
    int i, on_arg = 1;

    while (on_arg < argc)
    {
        for (i = 0; i < sizeof(options) / sizeof(struct Command_Line_Options); i++)
        {
            if (!strcmp(options[i].cmd_option, argv[on_arg]))
            {
                /* Found a matching option, but should not be processed on
                     * this pass.  Just skip over it
                     */
                if (options[i].pass != pass)
                {
                    on_arg += options[i].num_args + 1;
                    break;
                }
                if (options[i].num_args)
                {
                    if ((on_arg + options[i].num_args) >= argc)
                    {
                        LOG(llevSystem, "command line: %s requires an argument.\n", options[i].cmd_option);
                        exit(1);
                    }
                    else
                    {
                        if (options[i].num_args == 1)
                            options[i].func(argv[on_arg + 1]);
                        if (options[i].num_args == 2)
                            options[i].func(argv[on_arg + 1], argv[on_arg + 2]);
                        on_arg += options[i].num_args + 1;
                    }
                }
                else
                {
                    /* takes no args */
                    options[i].func();
                    on_arg++;
                }
                break;
            }
        }
        if (i == sizeof(options) / sizeof(struct Command_Line_Options))
        {
            LOG(llevSystem, "Unknown option: %s\n", argv[on_arg]);
            usage();
            exit(1);
        }
    }
}


static void init_beforeplay()
{
    /* several SockList stuff is not dynamic nor must it threadsafe.
     * lets safe some malloc by using this global buffer.
     */
    global_sl.buf = malloc(MAXSOCKBUF);
    global_sl.len = 0;

    pool_mob_data->constructor = (chunk_constructor) initialize_mob_data;
    pool_mob_data->destructor = (chunk_destructor) cleanup_mob_data;
    pool_mob_knownobj->destructor = (chunk_destructor) cleanup_mob_known_obj;
    pool_mob_behaviourset->destructor = (chunk_destructor) cleanup_behaviourset;

    init_spells();     /* If not called before, links archtypes used by spells */
    init_races();      /* overwrite race designations using entries in lib/races file */
    init_gods();        /* init linked list of gods from archs*/
    init_readable();    /* inits useful arrays for readable texts */
    init_archetype_pointers(); /* Setup global pointers to archetypes */
#ifdef ALCHEMY
    init_formulae();    /* If not called before, reads formulae from file */
#endif
    init_new_exp_system();    /* If not called before, inits experience system */

#ifdef DUMP_SWITCHES
    if (settings.dumpvalues)
    {
        if (settings.dumpvalues == 1)
            print_monsters();
        if (settings.dumpvalues == 2)
            dump_abilities();
        if (settings.dumpvalues == 3)
            dump_artifacts();
        if (settings.dumpvalues == 4)
            dump_spells();
        if (settings.dumpvalues == 5)
            dump_skills();
        if (settings.dumpvalues == 6)
            dump_races();
        if (settings.dumpvalues == 7)
            dump_alchemy();
        if (settings.dumpvalues == 8)
            dump_gods();
        if (settings.dumpvalues == 9)
            dump_alchemy_costs();
        if (settings.dumpvalues == 10)
            dump_monster_treasure(settings.dumparg);
        if (settings.dumpvalues == 11)
            dump_all_archetypes();
        exit(0);
    }
#endif
}

/*
 * init() is called only once, when starting the program.
 */
void init(int argc, char **argv)
{
    (void) umask(0);    /* We don't want to be affected by players' umask */

    init_done = 0;      /* Must be done before init_signal() */
    logfile = stderr;

    parse_args(argc, argv, 1);  /* First arg pass - right now it does
                                 * nothing, but in future specifying the
                                 * LibDir in this pass would be reasonable
                                 */

    init_library();             /* Must be called early */
    load_settings();            /* Load the settings file */
    init_word_darkness();
    parse_args(argc, argv, 2);

    SRANDOM(time(NULL));
    global_map_tag = (uint32) RANDOM();

    init_signals();     /* Sets up signal interceptions */
    init_commands();    /* Sort command tables */
    read_map_log();     /* Load up the old temp map files */
    parse_args(argc, argv, 3);

#ifndef WIN32 /* ***win32: no BecomeDaemon in windows */
    if (settings.daemonmode)
        logfile = BecomeDaemon(settings.logfilename[0] == '\0' ? "logfile" : settings.logfilename);
#endif

    init_beforeplay();
    init_ericserver();
    metaserver_init();
    init_arch_default_behaviours();
    load_ban_file();
    load_gmaster_file();
    init_done = 1;
}



void init_library()
{
    init_environ();
    init_hash_table();
    init_globals();
    init_mempools(); /* Inits the pooling memory manager and the new object system */
    init_vars();
    init_block();
    LOG(llevInfo, "Daimonin Server, v%s\n", VERSION);
    LOG(llevInfo, "Copyright (C) 2002-2005 Michael Toennies.\n");
    ReadBmapNames();
    init_anim();        /* Must be after we read in the bitmaps */
    init_archetypes();  /* Reads all archetypes from file */
    init_dynamic();
    init_clocks();
}

