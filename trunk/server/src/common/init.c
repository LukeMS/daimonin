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
#include <object.h>

/* You unforunately need to looking in include/structs.h to see what these
 * correspond to.
 */
struct Settings settings = {
"",			        /* Logfile */
CSPORT,				/* Client/server port */
GLOBAL_LOG_LEVEL,
0, NULL, 0,    /* dumpvalues, dumparg, daemonmode */
0, /* argc */
NULL, /* argv */
DATADIR, 
LOCALDIR,
PLAYERDIR, MAPDIR, ARCHETYPES,TREASURES, 
UNIQUE_DIR, TMPDIR,
STAT_LOSS_ON_DEATH,
USE_PERMANENT_EXPERIENCE,
BALANCED_STAT_LOSS,
RESET_LOCATION_TIME,
0,		/* This and the next 3 values are metaserver values */
"",
"",
0,
"",
0,0,0,0,0,0,0  /* worldmap settings*/
};

/* daytime counter (day & night / lightning system) */
int world_darkness;
unsigned long todtick;
archetype *level_up_arch=NULL;


/*
 * It is vital that init_library() is called by any functions
 * using this library.
 * If you want to lessen the size of the program using the library,
 * you can replace the call to init_library() with init_globals() and
 * init_function_pointers().  Good idea to also call init_vars and
 * init_hash_table if you are doing any object loading.
 */

void init_library() {
    init_environ();
    init_globals();
    init_function_pointers();
    init_hash_table();
    init_objects();
    init_vars();
    init_block();
    LOG(llevInfo,"Daimonin Server, v%s\n",VERSION);
    LOG(llevInfo,"Copyright (C) 2002-2004 Michael Toennies.\n");
    ReadBmapNames ();
    init_anim();    /* Must be after we read in the bitmaps */
    init_archetypes();	/* Reads all archetypes from file */
    init_dynamic ();
    init_clocks();

	/* init some often used default archetypes */
	if (level_up_arch == NULL)
		level_up_arch = find_archetype("level_up");
	if (!level_up_arch)
		LOG(llevBug,"BUG: Cant'find 'level_up' arch\n");

}

/* init_environ initializes values from the environmental variables.
 * it needs to be called very early, since command line options should
 * overwrite these if specified.
 */
void init_environ() {
    char *cp;

#ifndef SECURE
    cp=getenv("CROSSFIRE_LIBDIR");
    if (cp) settings.datadir=cp;
    cp=getenv("CROSSFIRE_LOCALDIR");
    if (cp) settings.localdir=cp;
    cp=getenv("CROSSFIRE_PLAYERDIR");
    if (cp) settings.playerdir=cp;
    cp=getenv("CROSSFIRE_MAPDIR");
    if (cp) settings.mapdir=cp;
    cp=getenv("CROSSFIRE_ARCHETYPES");
    if (cp) settings.archetypes=cp;
    cp=getenv("CROSSFIRE_TREASURES");
    if (cp) settings.treasures=cp;
    cp=getenv("CROSSFIRE_UNIQUEDIR");
    if (cp) settings.uniquedir=cp;
    cp=getenv("CROSSFIRE_TMPDIR");
    if (cp) settings.tmpdir=cp;
#endif

}
    

/*
 * Initialises all global variables.
 * Might use environment-variables as default for some of them.
 */

void init_globals() {
    if (settings.logfilename[0] == 0) {
	logfile = stderr;
    }
    else if ((logfile=fopen(settings.logfilename, "w"))==NULL) {
	logfile = stderr;
	LOG(llevInfo, "Unable to open %s as the logfile - will use stderr instead\n",settings.logfilename);
    }
    exiting = 0;
    first_player=NULL;
    first_friendly_object=NULL;
    first_map=NULL;
    first_treasurelist=NULL;
    first_artifactlist=NULL;
    first_archetype=NULL;
    first_map=NULL;
    nroftreasures = 0;
    nrofartifacts = 0;
    nrofallowedstr=0;
	undead_name = NULL;
	FREE_AND_COPY_HASH(undead_name,"undead");
    trying_emergency_save = 0;
    num_animations=0;
    animations=NULL;
    animations_allocated=0;
    init_defaults();
}

/*
 * Sets up and initialises the linked list of free and used objects.
 * Allocates a certain chunk of objects and puts them on the free list.
 * Called by init_library();
 */

void init_objects() {
  int i;
/* Initialize all objects: */
  objects=NULL;
  active_objects = NULL;

#ifdef MEMORY_DEBUG
  free_objects=NULL;
#else
  free_objects=objarray;
  objarray[0].prev=NULL,
  objarray[0].next= &objarray[1],
  SET_FLAG(&objarray[0], FLAG_REMOVED);
  SET_FLAG(&objarray[0], FLAG_FREED);
  for(i=1;i<STARTMAX-1;i++) {
    objarray[i].next= &objarray[i+1];
    objarray[i].prev= &objarray[i-1];
    SET_FLAG(&objarray[i], FLAG_REMOVED);
    SET_FLAG(&objarray[i], FLAG_FREED);
  }
  objarray[STARTMAX-1].next=NULL;
  objarray[STARTMAX-1].prev= &objarray[STARTMAX-2];
  SET_FLAG(&objarray[STARTMAX-1], FLAG_REMOVED);
  SET_FLAG(&objarray[STARTMAX-1], FLAG_FREED);
#endif
}

/*
 * Initialises global variables which can be changed by options.
 * Called by init_library().
 */

void init_defaults() {
  nroferrors=0;
}


void init_dynamic () {
    archetype *at = first_archetype;
    while (at) {
	if (at->clone.type == MAP && EXIT_PATH (&at->clone)) {
	    strcpy (first_map_path, EXIT_PATH (&at->clone));
	    return;
	}
	at = at->next;
    }
    LOG(llevError,"init_dynamic (): You Need a archetype called 'map' and it have to contain start map\n");
}

/*
 * Write out the current time to the file so time does not
 * reset every time the server reboots.
 */

void write_todclock()
{
    char filename[MAX_BUF];
    FILE *fp;

    sprintf(filename, "%s/clockdata", settings.localdir);
    if ((fp = fopen(filename, "w")) == NULL) {
	LOG(llevBug, "BUG: Cannot open %s for writing\n", filename);
	return;
    }
    fprintf(fp, "%lu", todtick);
    fclose(fp);
}

/*
 * Initializes the gametime and TOD counters
 * Called by init_library().
 */

void init_clocks()
{
    char filename[MAX_BUF];
    FILE *fp;
    static int has_been_done=0;

    if (has_been_done)
        return;
    else
        has_been_done = 1;

    sprintf(filename, "%s/clockdata", settings.localdir);
    LOG(llevDebug, "Reading clockdata from %s...", filename);
    if ((fp = fopen(filename, "r")) == NULL) {
        LOG(llevBug, "BUG: Can't open %s.\n", filename);
	todtick = 0;
	write_todclock();
	return;
    }
    fscanf(fp, "%lu", &todtick);
    LOG(llevDebug, "todtick=%lu\n", todtick);
    fclose(fp);
}
