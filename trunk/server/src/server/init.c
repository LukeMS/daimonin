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

#define EXTERN

#include <global.h>

struct settings_t settings    =
{
    2,                      /* default max connections from single ip address */
    0,                      /* will be set from create_client_settings() */
    0,                      /* mute level = if set players below this level can't shout*/
    TRUE,                   /* login_allow */
    "",                     /* login_ip */
    "",                     /* tlogfile to use */
    "",                     /* clogfile to use */
    CSPORT,                 /* Client/server port */
    GLOBAL_LOG_LEVEL,       /* Default debugging level */
    0,                      /* Set to dump various values/tables */
    NULL,                   /* additional argument for some dump functions */
    0,                      /* If true, detach and become daemon */
    0,                      /* number of parameters that were passed to the program */
    NULL,                   /* parameters that were passed to the program */
    DATADIR,                /* read only data files */
    LOCALDIR,               /* read/write data files */
    ACCOUNTDIR,             /* Where the accounts are stored */
    PLAYERDIR,              /* Where the player files are */
    INSTANCEDIR,            /* Where the instance map files are */
    MAPDIR,                 /* Where the map files are */
    ARCHETYPES,             /* name of the archetypes file - libdir is prepended */
    TREASURES,              /* location of the treasures file. */
    UNIQUE_DIR,             /* directory for the unique items */
    TMPDIR,                 /* Directory to use for temporary files */
    STATS_DIR,              /* Directory for active logs of statistical events */
    STATS_ARCHIVE_DIR,      /* Directory for logs, ready for further processing */
    STAT_LOSS,              /* If not 0, players lose random stats on death. */
    0,                      /* True if we should send updates */
    "",                     /* Hostname/ip addr of the metaserver */
    "",                        /* name of the server */
    "",                     /* Hostname of this host */
    0,                      /* Port number to use for updates */
    "",                     /* Comment we send to the metaserver */
    0,                      /* starting x tile for the worldmap */
    0,                      /* starting y tile for the worldmap */
    0,                      /* number of tiles wide the worldmap is */
    0,                      /* number of tiles high the worldmap is */
    0,                      /* number of squares wide in a wm tile */
    0,                      /* number of squares high in a wm tile */
    0,                      /* how dynamic is the world? */
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

static void FreeStrings(size_t nrof, shstr_t **ptr);
static void CreateLogfiles(const char *tlogfilename, const char *clogfilename,
                           const char *mode, const uint8 llev);

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

    shstr_cons.clawing = add_string("clawing");
    shstr_cons.dragon_skin_force = add_string("dragon_skin_force");
    shstr_cons.dragon_ability_force = add_string("dragon_ability_force");
    shstr_cons.dragon = add_string("dragon");

    shstr_cons.town_portal_destination = add_string("Town portal destination");
    shstr_cons.existing_town_portal = add_string("Existing town portal");

    shstr_cons.player = add_string("player");
    shstr_cons.money = add_string("money");
    shstr_cons.RANK_FORCE = add_string("RANK_FORCE");
    shstr_cons.ALIGNMENT_FORCE = add_string("ALIGNMENT_FORCE");
    shstr_cons.GUILD_FORCE = add_string("GUILD_FORCE");

    shstr_cons.stat_strength = add_string("strength");
    shstr_cons.stat_dexterity = add_string("dexterity");
    shstr_cons.stat_constitution = add_string("constitution");
    shstr_cons.stat_intelligence = add_string("intelligence");
    shstr_cons.stat_wisdom = add_string("wisdom");
    shstr_cons.stat_power = add_string("power");
    shstr_cons.stat_charisma = add_string("charisma");

    shstr_cons.special_prayer = add_string("special prayer");
    shstr_cons.grace_limit = add_string("grace limit");
    shstr_cons.restore_grace = add_string("restore grace");
    shstr_cons.restore_hitpoints = add_string("restore hitpoints");
    shstr_cons.restore_spellpoints = add_string("restore spellpoints");
    shstr_cons.heal_spell = add_string("heal spell");
    shstr_cons.remove_curse = add_string("remove curse");
    shstr_cons.remove_damnation = add_string("remove damnation");
    shstr_cons.heal_depletion = add_string("heal depletion");
    shstr_cons.message = add_string("message");
    shstr_cons.enchant_weapon = add_string("enchant weapon");

    shstr_cons.Eldath = add_string("Eldath"); /* old and incorrect god */
    shstr_cons.the_Tabernacle = add_string("the Tabernacle"); /* corrected god */

    /* mostly used by CONTR(op)->killer */
    shstr_cons.poisonous_food = add_string("poisonous food");
    shstr_cons.starvation = add_string("starvation");
    shstr_cons.drowning = add_string("drowning in a swamp");

    shstr_cons.emergency_mappath = add_string(EMERGENCY_MAPPATH);
    shstr_cons.start_mappath = add_string(FALLBACK_START_MAP_PATH);
    shstr_cons.bind_mappath = add_string(BIND_MAP_MAPPATH);

    shstr_cons.nopass = add_string(RECLAIM_NOPASS);

    shstr_cons.beacon_default = add_string("beacon");
}

void free_strings(void)
{
    LOG(llevDebug, "Freeing shared strings.\n");
    FreeStrings(sizeof(shstr_cons), (shstr_t **)&shstr_cons);
    /* See commands.c */
    FreeStrings(sizeof(subcommands), (shstr_t **)&subcommands);
}

static void FreeStrings(size_t nrof, shstr_t **ptr)
{
    sint16 i = nrof / sizeof(shstr_t *) - 1;

    for(; i >= 0; i--)
    {
        FREE_ONLY_HASH(ptr[i]);
    }
}

/* set the pticks_xx but NOT pticks itself.
 * pticks_ums = how "long" in ums is a server "round" (counted with ROUND_TAG).
 * pticks_second = how many "round" are done in a second.
 *
 * The default ums is set in MAX_TIME in config.h and cank be changed with the
 * /dm_time command. */
/* TODO: send new pticks_xx to all plugins! */
void set_pticks_time(long t)
{
    pticks_ums = t;
    pticks_second = 1000000;

    if(t)
    {
        pticks_second = (uint32)(1000000 / t);
    }

    pticks_socket_idle = 60 * 3 * pticks_second;
    pticks_player_idle1 = 60 * 8 * pticks_second;
    pticks_player_idle2 = 60 * 2 * pticks_second;

    /* LOG(llevDebug,"set_pticks_time(): t=%d ums:%d pticks_second:%u sock:%d idle1:%d idle2:%d\n",
       t, pticks_ums, pticks_second, pticks_socket_idle, pticks_player_idle1, pticks_player_idle2);
     */
}

/*
 * Initialises all global variables.
 * Might use environment-variables as default for some of them.
 */

static void init_globals()
{
    arch_cmp = 0;       /* How many strcmp's */
    arch_search = 0;    /* How many searches */
    arch_init = 0;      /* True if doing arch initialization */

    pticks = 1;        /* global round ticker ! this is real a global */

    global_race_counter = 0; /* global race counter */
    global_group_tag=0; /* every group gets a unique group tag identifier */

    set_pticks_time(MAX_TIME);

    gmaster_list = NULL;
    gmaster_list_VOL = NULL;
    gmaster_list_GM = NULL;
    gmaster_list_MW = NULL;
    gmaster_list_MM = NULL;
    gmaster_list_SA = NULL;

    ban_list_account=NULL;
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

    init_strings();

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
    cp = getenv("DAIMONIN_LIBDIR");
    if (cp)
        settings.datadir = cp;
    cp = getenv("DAIMONIN_LOCALDIR");
    if (cp)
        settings.localdir = cp;
    cp = getenv("DAIMONIN_PLAYERDIR");
    if (cp)
        settings.playerdir = cp;
    cp = getenv("DAIMONIN_INSTANCEDIR");
    if (cp)
        settings.instancedir = cp;
    cp = getenv("DAIMONIN_MAPDIR");
    if (cp)
        settings.mapdir = cp;
    cp = getenv("DAIMONIN_ARCHETYPES");
    if (cp)
        settings.archetypes = cp;
    cp = getenv("DAIMONIN_TREASURES");
    if (cp)
        settings.treasures = cp;
    cp = getenv("DAIMONIN_UNIQUEDIR");
    if (cp)
        settings.uniquedir = cp;
    cp = getenv("_TMPDIR");
    if (cp)
        settings.tmpdir = cp;
#endif
}

/* This loads the settings file.  There could be debate whether this should
 * be here or in the common directory - but since only the server needs this
 * information, having it here probably makes more sense.
 */
static void load_settings()
{
    char    buf[MEDIUM_BUF], *cp;
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
    while (fgets(buf, MEDIUM_BUF - 1, fp) != NULL)
    {
        if (buf[0] == '#')
            continue;
        /* eliminate newline */
        i = strlen(buf) - 1;
        while(i >= 0 && isspace(buf[i]))
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

        /* default value of settings.login_allow is TRUE.
         * if set to FALSE, settings.login_ip is checked against
         * the login IP. If they match, login_allow is ignored.
         * This allows to setup admin login when testing a installed server.
         */
        if (!strcasecmp(buf, "login_allow"))
        {
            if (!strcasecmp(cp, "on") || !strcasecmp(cp, "true"))
                settings.login_allow = TRUE;
            else
                settings.login_allow = FALSE;
        }
        else if (!strcasecmp(buf, "login_ip"))
        {
            if(*cp)
                settings.login_ip = strdup_local(cp);
        }
        else if (!strcasecmp(buf, "metaserver_notification"))
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
        else if (!strcasecmp(buf, "metaserver_name"))
        {
            if (has_val)
            {
                int ii;

                strcpy(settings.meta_name, cp);
                // to ensure we can use a simple scanf, we mark all ' ' whitespace with a '-'
                for(ii=0;settings.meta_name[ii]!=0;ii++)
                    if(settings.meta_name[ii]==' ')
                        settings.meta_name[ii] = '_';
            }
            else
                LOG(llevBug, "BUG: load_settings: metaserver_name must have a value.\n");
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
            int ii;

            strcpy(settings.meta_comment, cp);
            // to ensure we can use a simple scanf, we mark all ' ' whitespace with a '-'
            for(ii=0;settings.meta_comment[ii]!=0;ii++)
                if(settings.meta_comment[ii]==' ')
                    settings.meta_comment[ii] = '_';
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
        else if (!strcasecmp(buf, "mutelevel"))
        {
            int lev = atoi(cp);

            settings.mutelevel = lev;
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

static void set_logfile(char *val)
{
    char *cp = strchr(val, ',');

    if (cp)
    {
        *cp++ = '\0';
    }

    settings.tlogfilename = val;
    settings.clogfilename = (cp && strcmp(val, cp)) ? cp : "";
}

static void call_version()
{
    printf("This is %s.\n\n", version_string());
    exit(0);
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
static void set_instancedir(char *path)
{
    settings.instancedir = path;
}
static void set_tmpdir(char *path)
{
    settings.tmpdir = path;
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

static void set_stat_loss(char *val)
{
    settings.stat_loss = (sint8)atoi(val);
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
    LOG(llevInfo, " -log <file>[,<file] Specifies which file(s) to send output to.\n");
    LOG(llevInfo, "                     The first file is for technical logs, the second for chat logs.\n");
    LOG(llevInfo, "                     If only one is specified, all messages are logged together.\n");
    LOG(llevInfo, " -mon        Turns on monster debugging.\n");
    LOG(llevInfo, " -o          Prints out info on what was defined at compile time.\n");
    LOG(llevInfo, " -stat_loss <num> - if not 0, players lose stat(s) when they die.\n");
    LOG(llevInfo, " -v          Print version and contributors.\n");
    LOG(llevInfo, " -test       Run unit tests and exit.\n");
    LOG(llevInfo, " -benchmark  Run benchmarks and exit.\n");

#ifndef SECURE
    LOG(llevInfo, "\nThe following options are only available if a secure server was not compiled.\n");
    LOG(llevInfo, " -data       Sets the lib dir (archetypes, treasures, etc.)\n");
    LOG(llevInfo, " -local      Read/write local data (unique items, etc.)\n");
    LOG(llevInfo, " -maps       Sets the directory for maps.\n");
    LOG(llevInfo, " -arch       Sets the archetype file to use.\n");
    LOG(llevInfo, " -playerdir  Sets the directory for the player files.\n");
    LOG(llevInfo, " -instancedir Sets the directory for the instance map files.\n");
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
#ifndef WIN32

static void rec_sigsegv(int i)
{
    LOG(llevSystem, "\nSIGSEGV received.\n");
    fatal_signal(1, 1, SERVER_EXIT_SIGSEGV);
}

static void rec_sigint(int i)
{
    LOG(llevSystem, "\nSIGINT received.\n");
    fatal_signal(0, 1, SERVER_EXIT_SIGINT);
}

static void rec_sighup(int i)
{
    LOG(llevSystem, "\nSIGHUP received\n");

    if (init_done)
    {
        cleanup_without_exit();
    }

    exit(SERVER_EXIT_SIGHUP);
}

static void rec_sigquit(int i)
{
    LOG(llevSystem, "\nSIGQUIT received\n");
    fatal_signal(1, 1, SERVER_EXIT_SIGQUIT);
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
    fatal_signal(1, 1, SERVER_EXIT_SIGPIPE); /*Might consider to uncomment this line */
#endif
}

static void rec_sigbus(int i)
{
#ifdef SIGBUS
    LOG(llevSystem, "\nSIGBUS received\n");
    fatal_signal(1, 1, SERVER_EXIT_SIGBUS);
#endif
}

static void rec_sigterm(int i)
{
    LOG(llevSystem, "\nSIGTERM received\n");
    fatal_signal(0, 1, SERVER_EXIT_SIGTERM);
}

#endif

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

static void add_ai_to_racelist(const char *race_name, archetype_t *op)
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

static void add_corpse_to_racelist(const char *race_name, archetype_t *op)
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
    list->member = objectlink_get(OBJLNK_FLAG_OB);
    list->next = NULL;
    list->ai = NULL;
    list->first_start_location = -1;
    list->last_start_location = -1;

    return list;
}

/* free race list
 */
void free_racelists()
{
    racelink     *list,
                 *next;
    objectlink_t *tmp;

    for (list = first_race; list; list = next)
    {
        next = list->next;
        FREE_ONLY_HASH(list->name);

        for (tmp = list->member; tmp; tmp = tmp->next)
        {
            return_poolchunk(tmp, pool_objectlink);
        }

        free(list);
    }
}

static void add_to_racelist(const char *race_name, object_t *op)
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
        objectlink_t *tmp = objectlink_get(OBJLNK_FLAG_OB);
        tmp->next = race->member;
        race->member = tmp;
    }
    race->nrof++;
    race->member->objlink.ob = op;
}


static void dump_races()
{
    racelink   *list;
    objectlink_t *tmp;
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
    archetype_t  *at;
    racelink   *list;
    static int  init_done   = 0;
    int i, prev_index;

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
        else if (at->clone.type == TYPE_AI)
        {
            add_ai_to_racelist(at->clone.race, at);
        }
        else if (at->clone.type == CONTAINER && at->clone.sub_type1 == ST1_CONTAINER_CORPSE)
        {
            add_corpse_to_racelist(at->clone.race, at);
        }
    };

    /* last action: for all races without a special defined corpse
     * add our corpse_default arch to it. */
    for (list = first_race; list; list = list->next)
    {
        /* For all races without a special defined corpse
         * add our corpse_default arch to it. */
        if (!list->corpse)
        {
            list->corpse = archetype_global._corpse_default;
        }

        prev_index = -1;
        // Loop through the start_locations array.
        for (i = 0; i < NUM_START_LOCATIONS; i++)
        {
            if (find_racelink(race_start_locations[i].race) == list)
            {
                // If we're at the beginning of the array and the names match
                // this is definitely the first index.
                if (prev_index == -1)
                {
                    list->first_start_location = i;
                    list->last_start_location = i;
                }
                else
                {
                    // Previous index was not a match but this one is, so this one
                    // is the first.
                    if (find_racelink(race_start_locations[prev_index].race) != list)
                    {
                        list->first_start_location = i;

                        // also set the last in case there is only 1 index
                        list->last_start_location = i;
                    }
                    else
                    {
                        list->last_start_location = i;
                    }
                }
            }
            else
            {
                // Previous index matched this race but this one doesn't, so the
                // previous was the last index.
                if (prev_index >= 0 && race_start_locations[prev_index].race != NULL &&
                    find_racelink(race_start_locations[prev_index].race) == list)
                {
                    list->last_start_location = prev_index;
                    break;
                }
            }
            prev_index++;
        }
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
    LOG(llevSystem, "Non-standard include files:\n");
#if !defined (__STRICT_ANSI__) || defined (__sun__)
#if !defined (Mips)
    LOG(llevSystem, "<stdlib.h>\n");
    i = 1;
#endif
#if !defined (MACH) && !defined (sony)
    LOG(llevSystem, "<malloc.h>\n");
    i = 1;
#endif
#endif
#ifndef __STRICT_ANSI__
#ifndef MACH
    LOG(llevSystem, "<memory.h\n");
    i = 1;
#endif
#endif
#ifndef sgi
    LOG(llevSystem, "<sys/timeb.h>\n");
    i = 1;
#endif
    if (!i)
        LOG(llevSystem, "(none)\n");
#ifdef SECURE
    LOG(llevSystem, "Secure:\t\t<true>\n");
#else
    LOG(llevSystem, "Secure:\t\t<false>\n");
#endif
    LOG(llevSystem, "Datadir:\t%s\n", settings.datadir);
    LOG(llevSystem, "Localdir:\t%s\n", settings.localdir);
    LOG(llevSystem, "Save player:\t<true>\n");
    LOG(llevSystem, "Save mode:\t%4.4o\n", SAVE_MODE);
    LOG(llevSystem, "Playerdir:\t%s/%s\n", settings.localdir, settings.playerdir);
    LOG(llevSystem, "Instancedir:\t%s/%s\n", settings.localdir, settings.instancedir);
    LOG(llevSystem, "Itemsdir:\t%s/%s\n", settings.localdir, settings.uniquedir);
    LOG(llevSystem, "Tmpdir:\t\t%s\n", settings.tmpdir);
    LOG(llevSystem, "Tlogfilename:\t%s, Clogfilename:\t%s (llev:%d)\n", settings.tlogfilename, settings.clogfilename, settings.debug);
    LOG(llevSystem, "Max_time:\t%ld (%u)\n", pticks_ums, pticks_second);
#ifdef MAP_MAXOBJECTS
    LOG(llevSystem, "Maps swap when too many objects are in memory:\t%u\n",
        MAP_MAXOBJECTS);
#else
    LOG(llevSystem, "Maps swap when a timeout is reached:\t%u/%u/%u\n",
         MAP_MINSWAP, MAP_DEFSWAP, MAP_MAXSWAP);
#endif
#ifdef MAP_RESET
    LOG(llevSystem, "Maps reset when a timeout is reached:\t%u/%u\n",
        MAP_DEFRESET, MAP_MAXRESET);
#else
    LOG(llevSystem, "Maps never reset.");
#endif
    LOG(llevSystem, "object_t is %lu bytes.\n", sizeof(object_t));
    LOG(llevSystem, "living_t is %lu bytes.\n", sizeof(living_t));
    LOG(llevSystem, "map_t is %lu bytes.\n", sizeof(map_t));
    LOG(llevSystem, "msp_t is %lu bytes.\n", sizeof(msp_t));
    LOG(llevSystem, "PlayerSize:\t%lu\n", sizeof(player_t) + MAXSOCKBUF_IN);
    LOG(llevSystem, "SocketSize:\t%lu\n", sizeof(NewSocket) + MAXSOCKBUF_IN);
}

/*
* fatal_signal() is meant to be called whenever a fatal signal is intercepted.
* It will try to kick the player and save them and the clean_tmp_files functions.
*/
void fatal_signal(int make_core, int close_sockets, uint8 status)
{
    if (init_done)
    {
#ifdef PLUGINS
        removePlugins();
#endif
        /* this line will try to save all player files when the server crashed.
         * thats good to avoid dubing but a evil source to destroy the player
         * files. Thats one of the biggest problems we can have, so i disabled
         * this line
         */
        /* kick_player(NULL); */
        clean_tmp_files(FALSE);
        write_book_archive();
        write_tadclock();   /* lets just write the clock here */
        save_ban_file();
    }

    if (make_core)
    {
        abort();
    }
    else
    {
        exit(status);
    }
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
    {"-instancedir", 1, 1, set_instancedir},
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
    {"-stat_loss", 1, 3, set_stat_loss},
    {"-test", 0, 4, run_unit_tests},
    {"-benchmark", 0, 4, run_benchmarks}
};

/* Note since this may be called before the library has been set up,
 * we don't use any of crossfires built in logging functions.
 */
static void parse_args(int argc, char *argv[], int pass)
{
    int i, on_arg = 1;

    while (on_arg < argc)
    {
        for (i = 0; i < (int)(sizeof(options) / sizeof(struct Command_Line_Options)); i++)
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
                        else if (options[i].num_args == 2)
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
    pool_mob_data->constructor = (chunk_constructor) initialize_mob_data;
    pool_mob_data->destructor = (chunk_destructor) cleanup_mob_data;
    pool_mob_knownobj->destructor = (chunk_destructor) cleanup_mob_known_obj;
    pool_mob_behaviourset->destructor = (chunk_destructor) cleanup_behaviourset;

    init_spells();     /* If not called before, links archetype_types used by spells */
    init_races();      /* overwrite race designations using entries in lib/races file */
    init_gods();        /* init linked list of gods from archs*/
    init_readable();    /* inits useful arrays for readable texts */
#ifdef ALCHEMY
    init_formulae();    /* If not called before, reads formulae from file */
#endif
    init_skills();

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

/* Init of the instance system.
 * Instances are in /instance folder saved like unique maps.
 * But they have 2 unique identifiers beside the map name:
 * id: is transformed to a unique directory name to identify different server instances
 * num: is also used as directory name inside the id directory to identify instance instances
 * format: /instance/<id>/<num/10000>/num/<unique-mapname>
 * The id is important because its saved as instance info in the player files.
 * If we reseting num, id will avoid an old player file enters a invalid instance
 * To allow reenter of an instance between server shutdown we have to save id & num
 * and reload it.
 */
static void init_instance_system(void)
{
    struct timeval now;

    GETTIMEOFDAY(&now);

    global_instance_id = now.tv_sec; /* well, we assume not 2 server starts in the same second AND
                                      * a player is starting an instance between that starts...
                                      * its impossible in reality
                                      */

    global_instance_num = 0; /* every instance has an unique tag/number */

    LOG(llevInfo,"Init instance system:  set ID:%ld num:%d\n", global_instance_id, global_instance_num);
}

/* The first time version_string() is called it puts together a string in a
 * static buffer and returns it. Subsequently ir just returns the string. */
char *version_string(void)
{
    static char buf[TINY_BUF] = "";

    if (buf[0] == '\0')
    {
        sprintf(buf, "Daimonin v%d.%d.%d%s%s (protocol version %d)",
                DAI_VERSION_RELEASE, DAI_VERSION_MAJOR, DAI_VERSION_MINOR,
                (DAI_VERSION_INTERIM == "") ? "" : "-", DAI_VERSION_INTERIM,
                PROTOCOL_VERSION);
    }

    return buf;
}
/*
 * init() is called only once, when starting the program.
 */
void init(int argc, char **argv)
{
    (void) umask(0);    /* We don't want to be affected by players' umask */

    init_done = 0;      /* Must be done before init_signal() */
    tlogfile = stderr;
    clogfile = stderr;

    /* Version string is initialised before first call to parse_args(). */
    version_string();
    parse_args(argc, argv, 1);  /* First arg pass - right now it does
                                 * nothing, but in future specifying the
                                 * LibDir in this pass would be reasonable
                                 */
    parse_args(argc, argv, 2);

    if (settings.daemonmode)
    {
#ifndef LINUX
        LOG(llevError, "The -detach switch only function under Linux!\n"
            "Re-run the server without this setting.");
#else
        CreateLogfiles((!settings.tlogfilename[0]) ? "tlog" : settings.tlogfilename,
                       (!settings.clogfilename[0]) ? "clog" : settings.clogfilename,
                       "a", llevError);
        become_daemon();
#endif
    }
    else
    {
        CreateLogfiles(settings.tlogfilename, settings.clogfilename, "w",
                       llevInfo);
    }

    init_library();             /* Must be called early */
    init_tadclock();
    load_settings();            /* Load the settings file */
    SRANDOM((uint32)time(NULL));
    global_map_tag = (uint32) RANDOM();
    init_signals();     /* Sets up signal interceptions */
    init_commands();    /* Sort command tables */
    read_map_log();     /* Load up the old temp map files */
    parse_args(argc, argv, 3);
    init_beforeplay();
    init_ericserver();
#ifdef SERVER_SEND_FACES
    read_client_images();
#endif
    init_srv_files(); /* load all srv_xxx files or generate them */
    metaserver_init();
    init_arch_default_behaviours();
    load_ban_file();
#ifdef USE_CHANNELS
    load_channels();            /* load the saved channels */
#endif
    load_gmaster_file();
    init_instance_system();
    init_done = 1;
    parse_args(argc, argv, 4);
}

static void CreateLogfiles(const char *tlogfilename, const char *clogfilename,
                           const char *mode, const uint8 llev)
{
    if (*tlogfilename)
    {
        char buf[MEDIUM_BUF];

        if (!strchr(tlogfilename, '/'))
        {
            sprintf(buf, "%s/log/%s", settings.localdir, tlogfilename);
        }
        else
        {
            sprintf(buf, "%s", tlogfilename);
        }

        if (!(tlogfile = fopen(buf, mode)))
        {
            tlogfile = stderr;
            LOG(llev, "Unable to open '%s' as the tlogfile!\n",
                buf);
        }
    }
    else
    {
        tlogfile = stderr;
    }

    if (*clogfilename)
    {
        char buf[MEDIUM_BUF];

        if (!strchr(clogfilename, '/'))
        {
            sprintf(buf, "%s/log/%s", settings.localdir, clogfilename);
        }
        else
        {
            sprintf(buf, "%s", clogfilename);
        }

        if (!(clogfile = fopen(buf, mode)))
        {
            clogfile = tlogfile;
            LOG(llev, "Unable to open '%s' as the clogfile!\n",
                 buf);
        }
    }
    else
    {
        clogfile = tlogfile;
    }
}

void free_lists_and_tables()
{
    LOG(llevDebug, "Removing activelist sentinel\n");
    remove_ob(active_objects);
    LOG(llevDebug, "Freeing all racelists\n");
    free_racelists();
    LOG(llevDebug, "Performing final object gc\n");
    object_gc();
}


void init_lists_and_tables()
{
    /* Add sentinels to the global activelist */
    active_objects = get_object();
    FREE_AND_COPY_HASH(active_objects->name, "<global activelist sentinel>");
    insert_ob_in_ob(active_objects, &void_container); /* Avoid gc of the object_t */

    /* Set up object initializers */
    init_object_initializers();

    /* Set up the table of beacons */
    beacon_table = pointer_hashtable_new(32);
}

void init_library()
{
    init_environ();
    init_hash_table(); /* inits the shstr_t system */
    init_globals();
    init_mempools();   /* Inits the mempool manager and the object system */
    init_vars();
    init_block();
    LOG(llevInfo, "%s.\nCopyright (C) 2002-2009 Michael Toennies.\n",
        version_string());
    ReadBmapNames();
    init_anim();        /* Must be after we read in the bitmaps */
    init_archetypes();  /* Reads all archetypes from file */
    init_sounds();
    init_lists_and_tables(); /* Initializes some global lists and tables */
}
