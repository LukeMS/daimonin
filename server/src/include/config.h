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

#ifndef __CONFIG_H
#define __CONFIG_H

/* This file contains various #defines that select various options.
 * Some may not be desirable, and some just may not work.
 *
 * There are some options that are not selectable in this file which
 * may not always be undesirable.  An example would be certain
 * treasures that you may not want to have available.  To remove the
 * activation code would make these items worthless - instead remove
 * these from the treasure file.  Some things to look for are:
 *
 * prepare_weapon, improve_*: Allow characters to enchant their own
 *  weapons
 * ench_armour: Allow characters to enchant their armor.
 *
 * In theory, most of the values here should just be defaults, and
 * everything here should just be selectable by different run time
 * flags  However, for some things, that would just be too messy.
 */

/* I moved this settings of the lib directory and the local directory
   to this location. I want have ALL this definitions at one location.
   As long we have no setup or install tool, we should use this file.
   (and NOT makefiles... this is a installation thing, not a compilation part.
    Depending on OS, this can be a big difference.).
 */

/* Location of read-only machine independent data */
#ifndef DATADIR
#define DATADIR "./lib"
#endif

/* Location of the map files */
/* this should be part of the installation... maps and data are different sources */
#ifndef MAPDIR
#define MAPDIR  "../maps"
#endif

/* Location of changeable single system data (temp maps, etc) */
#ifndef LOCALDIR
#define LOCALDIR "./data"
#endif

/* Location of plugins */
#ifndef PLUGINDIR
#define PLUGINDIR "./plugins"
#endif

/*
 * Your tmp-directory should be large enough to hold the uncompressed
 * map-files for all who are playing.
 * It ought to be locally mounted, since the function used to generate
 * unique temporary filenames isn't guaranteed to work over NFS or AFS
 * On the other hand, if you know that only one crossfire server will be
 * running using this temporary directory, it is likely to be safe to use
 * something that is NFS mounted (but performance may suffer as NFS is
 * slower than local disk)
 */
#ifndef TMPDIR
#define TMPDIR LOCALDIR"/tmp"
#endif

/* Directory to use for unique items. This is placed into the 'lib'
 * directory.  Changing this will cause any old unique items file
 * not to be used.
 */
#ifndef UNIQUE_DIR
#define UNIQUE_DIR "unique-items"
#endif

#ifndef ACCOUNTDIR
#define ACCOUNTDIR "accounts"
#endif

#ifndef PLAYERDIR
#define PLAYERDIR "players"
#endif

#ifndef INSTANCEDIR
#define INSTANCEDIR "instance"
#endif

#ifndef HELPDIR
#define HELPDIR   "./man"
#endif

/* Directory for active event log files */
#ifndef STATS_DIR
#define STATS_DIR   LOCALDIR"/stats"
#endif

/* Directory for complete event log files, ready for further processing */
#ifndef STATS_ARCHIVE_DIR
#define STATS_ARCHIVE_DIR   LOCALDIR"/statsarchive"
#endif

/* There are 4 main sections to this file-
 * Section 1 is feature selection (enabling/disabling certain features)
 *
 * Section 2 is compiler/machine dependant section (stuff that just
 *     makes the program compile and run properly, but don't change the
 *     behavior)
 *
 * Section 3 is location of certain files and other defaults.  Things in
 *     this section generally do not need to be changed, and generally do
 *     not alter the play as perceived by players.  However, you may
 *     have your own values you want to set here.
 *
 * Section 4 deals with save file related options.
 */

/*******************************************************************
* SECTION 1 - FEATURES
*
* You don't have to change anything here to get a working program, but
* you may want to on personal preferance.  Items are arranged
* alphabetically.
*
* Short list of features, and what to search for:
* ALCHEMY - enables alchemy code
* DEBUG - more verbose message logging?
* MAP_CLIENT_X, MAP_CLIENT_Y - determines max size client map will receive
* MAX_TIME - how long an internal tick is in microseconds
* PTICKS_PER_ARKHE_HOUR -how many pticks make up a game hour
* MULTIPLE_GODS - adds numerous gods to the game, with different powers
* RECYCLE_TMP_MAPS - use tmp maps across multiple runs?
* SECURE - Allow overriding values with run time flags?
* SPELL_* - various spell related options
* STAT_LOSS - if not 0, players lose randdom stats on death.
* USE_CHANNELS - enables channel system
* ANNOUNCE_CHANNELS - announce the channel system at login (requires USE_CHANNELS)
* USE_GRAVESTONES - enables gravestones when players die.
* USE_LIGHTING - enable light/darkness & light sources
* USE_TILESTRETCHER - enables tilestretcher for uneven ground surface
* USE_PVP - Enables players to attack each other in certain maps.
* USE_SLAYER_MONSTER - enables a slayer exclusively for monsters.
***********************************************************************/

/* Enables Brian Thomas's new alchemy code.  Might unbalance the game - lets
 * characters make potions and other special items.
 * 0.94.2 note - I think enough work has been done, and even some maps
 * changed that this really should probably be a standard feature now.
 */

#define ALCHEMY

/* DEBUG generates copious amounts of output.  I tend to change the CC options
 * in the crosssite.def file if I want this.  By default, you probably
 * dont want this defined.
 */
#ifndef WIN32           /* ***win32 we set the following stuff in the IDE */
#ifndef DEBUG
#define DEBUG
#endif
#endif

/*
 * This determines the maximum map size the client can request (and
 * thus what the server will send to the client.
 * Client can still request a smaller map size (for bandwidth reasons
 * or display size of whatever else).
 * The larger this number, the more cpu time and memory the server will
 * need to spend to figure this out in addition to bandwidth needs.
 * The server cpu time should be pretty trivial.
 * There may be reasons to keep it smaller for the 'classic' crossfire
 * experience which was 11x11.  Big maps will likely make the same at
 * least somewhat easier, but client will need to worry about lag
 * more.
 * I put support in for non square map updates in the define, but
 * there very well might be things that break horribly if this is
 * used.  I figure it is easier to fix that if needed than go back
 * at the future and have to redo a lot of stuff to support rectangular
 * maps at that point.
 *
 * MSW 2001-05-28
 */

#define MAP_CLIENT_X    17
#define MAP_CLIENT_Y    17

/*
 * If you feel the game is too fast or too slow, change MAX_TIME.
 * The length of a tick is MAX_TIME microseconds.  During a tick,
 * players, monsters, or items with speed 1 can do one thing.
 */
/* thats 8 ticks per second now - 100.000 are 10 ticks */
#define MAX_TIME    125000

#define INACTIVE_SOCKET  180 // seconds until a socket is recycled
#define INACTIVE_PLAYER1 480 // seconds until an inactive player is warned he'll be kicked
#define INACTIVE_PLAYER2 120 // seconds until we lose patience with the warned inactive player
#define INACTIVE_ZOMBIE   10 // seconds until a zombie socket is killed

/* Arguably, this should be in calendar.h, but I have put it here so it is with
 * MAX_TIME as you'll probably want to change both together. */
/* 1 Arkhe hour = 5 real life minutes, or (1000000 / MAX_TIME) * 300 */
#define PTICKS_PER_ARKHE_HOUR 2400

/*
 * Set this if you want the temporary maps to be saved and reused across
 * crossfire runs.  This can be especially useful for single player
 * servers, but even holds use for multiplayer servers.  The file used
 * is updated each time a temp map is updated.
 * Note that the file used to store this information is stored in
 * the LIB directory.  Running multiple crossfires with the same LIB
 * directory will cause serious problems, simply because in order for
 * this to really work, the filename must be constant so the next run
 * knows where to find the information.
 */

/*#define RECYCLE_TMP_MAPS*/

/*
 * If SECURE is defined, crossfire will not accept enviromental variables
 * as changes to the LIBDIR and other defines pointing to specific
 * directories.  The only time this should really be an issue if you are
 * running crossfire setuid/setgid and someone else could change the
 * values and get special priveledges.  If you are running with normal uid
 * privelidges, then this is not any more/less secure (user could compile
 * there own version pointing wherever they want.)  However, having nonsecure
 * can make debugging or other developement much easier, because you can
 * change the paths without forcing a recompile.
 *
 * NOTE: Prior to 0.93.2, the default was for this to be defined (ie,
 * could not change values on the command line/through environmental
 * variables.)
 */
/*
#define SECURE
*/

/*  SPELLPOINT_LEVEL_DEPEND  --  Causes the spellpoint cost
 *  of spells to vary with their power.  Spells that become very
 *  powerful at high level cost more.  The damage/time of
 *  characters increases though.
 */
#define SPELLPOINT_LEVEL_DEPEND

/* STAT_LOSS controls whether and how many stat points a character
 * loses through death sickness when they die.
 *
 * If >0 they lose level/STAT_LOSS points from random stats.
 *
 * If <0 they lose 1 point from a random stat.
 *
 * If =0 stat loss is disabled.
 *
 * The define is the default, it can be changed at run time via -stat_loss
 * <n>. */
#define STAT_LOSS 18

/* you HAVE to also enable this in the lua-plugin (plugin_lua.h)!!! */
#define USE_CHANNELS
/* When defined, annouces the channel system at login. Be sure that
 * USE_CHANNELS is defined below! (Screw the alphabetical arrangement) */
//#define ANNOUNCE_CHANNELS

/* Gravestones mark the spot where a player died (they disappear after a few
 * minutes). They cause damage to players stepping on them if
 * RANDOM_ROLL(0, player->stats.Wis)) -- so lower Wis (taken to mean the player
 * is less sensitive to the divine) means more chance of getting away with it
 * -- and prayers cannot never be invoked by a player standing on a gravestone.
 * So healers, don't let your groupmates die! */
#define USE_GRAVESTONES

#define USE_TILESTRETCHER

/* With USE_PVP defined, players can attack each other on maps with 'pvp 1' in
 * their header. */
#define USE_PVP

#define USE_SLAYER_MONSTER

/***********************************************************************
 * SECTION 2 - Machine/Compiler specific stuff.
 *
 * Short list of items:
 * COMPRESS_SUFFIX - selection of compression programs
 * O_NDELAY - If you don't have O_NDELAY, uncomment it.
 *
 ***********************************************************************/

/*
 * If you compress your files to save space, set the COMPRESS_SUFFIX below
 * to the compression suffix you want (.Z, .gz, .bz2).  The autoconf
 * should already find the program to use.  If you set the suffix to
 * something that autoconf did not find, you are likely to have serious
 * problems, so make sure you have the appropriate compression tool installed
 * before you set this.  You can look at the linux.h file to see
 * what compression tools it found (search for COMPRESS).
 * Note that this is used when saving files.  Crossfire will search all
 * methods when loading a file to see if it finds a match
 */

#ifndef COMPRESS_SUFFIX
/* #define COMPRESS_SUFFIX ".Z" */
#endif

/* If you get a complaint about O_NDELAY not being known/undefined, try
 * uncommenting this.
 * This may cause problems - O_NONBLOCK will return -1 on blocking writes
 * and set error to EAGAIN.  O_NDELAY returns 0.  This is only if no bytes
 * can be written - otherwise, the number of bytes written will be returned
 * for both modes.
 */

/*
#define O_NDELAY O_NONBLOCK
*/


/***********************************************************************
 * Section 3
 *
 * General file and other defaults that don't need to be changed, and
 * do not change gameplay as percieved by players much.  Some options
 * may affect memory consumption however.
 *
 * Values:
 *
 * BANFILE - ban certain users/hosts.
 * CSPORT - port to use for new client/server
 * DMFILE - file with dm/wizard access lists
 * GMASTER_FILE - file with dm/wizard access lists
 * DUMP_SWITCHES - enable -m? flags for spoiler generation
 * LIBDIR - location of archetypes & other data.
 * LOGFILE - where to log if using -daemon option
 * MAP_ - various map timeout and swapping parameters
 * MOTD - message of the day - printed each time someone joins the game
 * SHUTDOWN - used when shutting down the server
 ***********************************************************************
 */

/*
* BUGFILE - file used for the /bug command to store online submited bugs from
* players.
*/

#ifndef BUG_LOG
#define BUG_LOG "bug_log"
#endif

/*
* BANFILE - file used to ban certain sites from playing.  See the example
* ban_file for examples.
*/

#ifndef BANFILE
#define BANFILE         "ban_file"
#endif


/* CSPORT is the port used for the new client/server code.  Change
 * if desired.  Only of relevance if ERIC_SERVER is set above
 */

#define CSPORT 13327 /* old port + 1 */


/*
 * DMFILE
 * A file containing valid names that can be dm, one on each line.  See
 * example dm_file for syntax help.
 */

#ifndef DMFILE
#define DMFILE "dm_file"
#endif

/*
 * GMASTER_FILE
 * A file containing valid names that can be dm, one on each line.  See
 * example dm_file for syntax help.
 */

#ifndef GMASTER_FILE
#define GMASTER_FILE "gmaster_file"
#endif

/*
 * If you want to regenerate the spoiler.ps file, you have to recompile
 * the game with DUMP_SWITCHES defined.  If defined, it turns on the
 * -m -m2 -m3 -m4 switches.  There is probably no reason not to define
 * this
 */

#define DUMP_SWITCHES

/* LOGFILE specifies which file to log to when playing with the
 * -daemon option.
 */

#ifndef LOGFILE
#define LOGFILE "./data/log/daimonin.log"
#endif

/* Map swapping is when a map is saved to disk and freed from memory. If it is
 * then needed again (for example, a player enters it), it can be reloaded from
 * this temporary file. But otherwise, it may eventually be reset -- see below.
 * This is known as time-based swapping.
 *
 * But if MAP_SWAP_OBJECT is defined object-based swapping is done instead.
 * This is when maps stay in memory forever until the total number of objects
 * in memory exceeds a certain maximum. At this point maps are swapped out one
 * by one until this total has been reduced to below a given threshold.
 * Possible map resetting then proceeds as for time-based swapping. */
//#define MAP_SWAP_OBJECT

#ifdef MAP_SWAP_OBJECT

/* If defined, MAP_MAXOBJECTS specifies that maps will be swapped when there
 * are > that number of objects in memory, *not* according to individual map
 * swap times. Swapping will continue until either there are no eligible maps
 * left, or the number of objects in memory < MAP_MINOBJECTS. Each object
 * structure is somewhat under 500 bytes right now (eg, 25000 is ~11.5-12 MB).
 *
 * MAP_MAXOBJECTS can probably be as low as 3000 per active player (if they
 * typically play on different maps). If it is too low, maps just get swapped
 * out often, causing a performance hit.  If it is too high, the program
 * consumes more memory.
 *
 * Similarly, the difference bettween MAP_MAXOBJECTS and MAP_MINOBJECTS needs
 * consideration -- too wide a gulf and you'll notice periodic pauses during
 * map swap/saving.
 *
 * This may simply be a re-enabled legacy feature from 1990s CF, when memory
 * was more precious than today, and be of dubious value to 21st century Dai.
 * OTOH it may be of use for people running a local server on small machines.
 *
 * There can however be CPU performance penalties - any objects in memory get
 * processed. So if there are 4000 objects in memory, and 1000 of them are
 * living objects, the server will process all 1000 objects each tick. With
 * time-based swapping, maybe 600 of the objects would have gotten swapped out.
 * This is less likely a problem with a smaller number of MAP_MAXOBJECTS than
 * if it is very large.
 *
 * Also, the pauses you do get can be worse, as if you enter a map with
 * a lot of new objects and go above MAP_MAXOBJECTS, it may have to swap out
 * many maps to get below the low water mark. */

/* Both must have a positive integer value with MIN being < MAX. */
# define MAP_MAXOBJECTS 10000
# define MAP_MINOBJECTS (MAP_MAXOBJECTS / 2)

#else

/* MAP_MAXSWAP tells the maximum seconds until a map is swapped out
 * after a player has left it.  If it is set to 0, maps are
 * swapped out the instant the last player leaves it.
 * If you are low on memory, you should set this to 0.
 *
 * Note that depending on the map timeout variable, the number of
 * objects can get quite high.  This is because depending on the maps,
 * a player could be having the objects of several maps in memory
 * (the map he is in right now, and the ones he left recently.)
 * Each map has it's own swap_time value and value field.
 *
 * Having a nonzero value can be useful: If a player leaves a map (and thus
 * is on a new map), and realizes they want to go back pretty quickly, the
 * old map is still in memory, so don't need to go disk and get it.
 *
 * MAP_MINSWAP is used as a minimum timeout value.
 *
 * MAP_DEFSWAP is the default value for maps which have not set their own
 * swap_time. */

/* All three must have a positive integer value. If MAX is 0 then MIN ahd DEF
 * should also be 0. Otherwise, MAX >= DEF >= MIN. */
# define MAP_MAXSWAP 1800
# define MAP_MINSWAP 1
# define MAP_DEFSWAP 120

#endif

/* After some time, a swapped (saved and freed) map may reset (basically, the
 * temporary file is deleted so the map is subsequently reloaded from it's
 * original location, ie, the maps directory). If MAP_RESET is defined, this
 * period of time is either a server default or may be set (within limits) by
 * the individual map.
 *
 * If MAP_RESET is undefined, maps will never reset automatically. */
#define MAP_RESET

#ifdef MAP_RESET

/* MAP_MAXRESET is the maximum time a map can have before being reset.  It
 * will override the time value set in the map, if that time is longer than
 * MAP_MAXRESET.  This value is in seconds.  If you are low on space on the
 * TMPDIR device, set this value to something small.
 *
 * MAP_DEFRESET is the default value for maps which have not set their own
 * reset_timeout. */

/* Both must have a positive integer value. If MAX is 0 then DEF should also be
 * 0. Otherwise, MAX >= DEF. */
# define MAP_MAXRESET 7200
# define MAP_DEFRESET 600

#endif

/*
 * If you want to have a Message Of The Day file, define MOTD to be
 * the file with the message.  If the file doesn't exist or if it
 * is empty, no message will be displayed.
 * (It resides in the LIBDIR directory)
 */

#ifndef MOTD
#define MOTD
#define MOTD_DEFAULT "motd"
#define MOTD_FILE "motd_file"
#endif


/*
 * These define the players starting map and location on that map, and where
 * emergency saves are defined.  This should be left as is unless you make
 * major changes to the map.
 */

#define EMERGENCY_MAPPATH "/emergency"
/* emergency is always MULTI and will be called with fixed login */

#define FALLBACK_START_MAP_PATH   "/planes/human_plane/castle/castle_030a"
#define FALLBACK_START_MAP_STATUS    (MAP_STATUS_MULTI)
#define FALLBACK_START_MAP_X         (18)
#define FALLBACK_START_MAP_Y         (1)

#define BIND_MAP_MAPPATH   "/planes/human_plane/castle/castle_040a"
#define BIND_MAP_STATUS    (MAP_STATUS_MULTI)
#define BIND_MAP_X         (7)
#define BIND_MAP_Y         (17)

/*
 * These defines tells where,
 * archetypes, treaures files and directories can be found.
 */

#define ARCHETYPES  "archetypes"
#define TREASURES   "treasures"
#define SETTINGS    "settings"

#define MAX_ERRORS  25  /* Bail out if more are received during tick */
#define OBJECT_EXPAND      2500     /* How big steps to use when expanding array */

/* this value must be a prime number! */
#define MAXSTRING 20

#define COMMAND_HASH_SIZE 107   /* If you change this, delete all characters :) */



/***********************************************************************
 * Section 4 - save player options.
 *
 * There are a lot of things that deal with the save files, and what
 * gets saved with them, so I put them in there own section.
 *
 ***********************************************************************/

/*
 * If you want the players to be able to save their characters between
 * games, define SAVE_PLAYER and set PLAYERDIR to the directories
 * where the player-files will be put.
 * Remember to create the directory (make install will do that though).
 *
 * If you intend to run a central server, and not allow the players to
 * start their own crossfire, you won't need to define this.
 *
 */

/*
 * If you have defined SAVE_PLAYER, you might want to change this, too.
 * This is the access rights for the players savefiles.
 * I think it is usefull to restrict access to the savefiles for the
 * game admin. So if you make crossfire set-uid, use 0600.
 * If you are running the game set-gid (to a games-group, for instance),
 * you must remember to make it writeable for the group (ie 0660).
 * Kjetil W. J{\o}rgensen, jorgens@pvv.unit.no
 * (Note: something should probably be done with lock-file permission)
 */
#define SAVE_MODE   0660

/* AUTOSAVE saves the player every AUTOSAVE ticks.  Some effort should probably
 * be made to spread out these saves, but that might be more effort than it is
 * worth (Depending on the spacing, if enough players log on, the spacing may
 * not be large enough to save all of them.)  As it is now, it will just set
 * the base tick of when they log on, which should keep the saves pretty well
 * spread out (in a fairly random fashion.) */

/* Must be a positive integer. */
#define AUTOSAVE 4800 // 10 mins at standard tick rate (8/sec)

/* Use the shipped version of inet_pton.  You can turn this off if you know
 * your local system will work with the inet_pton it has.
 */
#define NEED_INET_PTON

#endif /* ifndef __CONFIG_H */
