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
#ifndef __CEXTRACT__
#include <sproto.h>
#endif

extern weathermap_t **weathermap;

const int season_timechange[5][HOURS_PER_DAY] = {

    {   0, 0,  0,  0, 0, 0, -1,-1, -1, -1, 0, 0,
        0, 0,  0,  0,  0,  1,  1, 1, 1, 0, 0, 0 },

    {   0, 0,  0,  0, 0, -1, -1,-1, -1, 0, 0, 0,
    0, 0,  0,  0,  0,  1,  1, 1, 1, 0, 0, 0 },
    
    
    {   0, 0,  0,  0, -1, -1, -1,-1, 0, 0, 0, 0,
        0, 0,  0,  0,  0,  1,  1, 1, 1, 0, 0, 0 },
    
    {   0, 0,  0, -1, -1, -1, -1, 0, 0, 0, 0, 0,
        0, 0,  0,  0,  0,  0,  1, 1, 1, 1, 0, 0 },
    

    {   0, 0,  0, -1, -1, -1, -1, 0, 0, 0, 0, 0,
        0, 0,  0,  0,  0,  0,  0, 1, 1, 1, 1, 0 }
};

void init_word_darkness(void)
{
    int i;
    timeofday_t tod;
    
    world_darkness=0;
    get_tod(&tod);

    for (i = HOURS_PER_DAY/2; i < HOURS_PER_DAY; i++)
        world_darkness +=season_timechange[tod.season][i];
    for (i = 0; i <= tod.hour; i++) /* must be <= and not < ... */
        world_darkness +=season_timechange[tod.season][i];
}

void dawn_to_dusk(timeofday_t *tod)
{
    mapstruct *m;

    world_darkness +=season_timechange[tod->season][tod->hour];
    for(m=first_map;m!=NULL;m=m->next) {
#ifndef MAP_RESET
	if (m->in_memory == MAP_SWAPPED)
	    continue;
#endif
	if (!MAP_OUTDOORS(m))
	    continue;
	change_map_light(m, season_timechange[tod->season][tod->hour]);
    }
}

/*
 * This performs the basic function of advancing the clock one tick
 * forward.  Every 20 ticks, the clock is saved to disk.  It is also
 * saved on shutdown.  Any time dependant functions should be called
 * from this function, and probably be passed tod as an argument.
 * Please don't modify tod in the dependant function.
 */

void tick_the_clock()
{
    timeofday_t tod ;

    todtick++;
    if (todtick%20 == 0)
	write_todclock();
    get_tod(&tod);
    dawn_to_dusk(&tod);
    /* perform_weather(); no weather! i can't see the sense atm */
}

int wmperformstartx;
int wmperformstarty;

void init_weather()
{
    int x, y;
    int i, j;
    long int tmp;
    char filename[MAX_BUF];
    FILE *fp;
    mapstruct *m;

    /* all this stuff needs to be set, otherwise this function will cause
     * chaos and destruction.
     */
    
    if (settings.dynamiclevel < 1)
	return;
    if (settings.worldmapstartx < 1 || settings.worldmapstarty < 1 ||
	settings.worldmaptilesx < 1 || settings.worldmaptilesy < 1 ||
	settings.worldmaptilesizex < 1 || settings.worldmaptilesizex < 1)
	return;

    LOG(llevDebug, "Initializing the weathermap...\n");

    weathermap = (weathermap_t **)malloc(sizeof(weathermap_t *) *
	settings.worldmaptilesx);
    if (weathermap == NULL)
		LOG(llevError,"ERROR: init_weather(): out of memory\n");
    for (y=0; (uint32) y < settings.worldmaptilesy; y++) {
	weathermap[y] = (weathermap_t *)malloc(sizeof(weathermap_t) *
	    settings.worldmaptilesy);
	if (weathermap[y] == NULL)
		LOG(llevError,"ERROR: init_weather(): out of memory\n");
    }
    /* now we load the values in the big worldmap weather array */
    for (x=0; (uint32)x < settings.worldmaptilesx; x++) {
	for (y=0; (uint32)y < settings.worldmaptilesy; y++) {
	    sprintf(filename, "world/world_%d_%d",
		x+settings.worldmapstartx, y+settings.worldmapstarty);
	    m = load_original_map(filename, 0);
	    if (m == NULL)
		continue;
        /*
	    m = load_overlay_map(filename, m);
	    if (m == NULL)
		continue;
        */
	    sprintf(weathermap[x][y].path, "%s", m->path);
	    if (m->tmpname)
		weathermap[x][y].tmpname = strdup(m->tmpname);
	    weathermap[x][y].name = strdup(m->name);
	    weathermap[x][y].temp = m->temp;
	    weathermap[x][y].pressure = m->pressure;
	    weathermap[x][y].humid = m->humid;
	    weathermap[x][y].windspeed = m->windspeed;
	    weathermap[x][y].winddir = m->winddir;
	    weathermap[x][y].sky = m->sky;
	    weathermap[x][y].darkness = m->darkness;
	    tmp = 0;
	    for (i=0; (uint32)i < settings.worldmaptilesizex; i++)
		for (j=0; (uint32)j < settings.worldmaptilesizey; j++)
		    tmp += 0; /*m->spaces[i+j].bottom->elevation;*/
	    weathermap[x][y].avgelev = tmp / (i*j);
	    delete_map(m);
	}
    }
    LOG(llevDebug, "Done reading worldmaps\n");
    sprintf(filename, "%s/wmapcurpos", settings.localdir);
    LOG(llevDebug, "Reading current weather position from %s...", filename);
    if ((fp = fopen(filename, "r")) == NULL) {
	LOG(llevBug, "BUG: Can't openX %s.\n", filename);
	wmperformstartx = -1;
	return;
    }
    fscanf(fp, "%d %d", &wmperformstartx, &wmperformstarty);
    LOG(llevDebug, "curposx=%d curposy=%d\n", wmperformstartx, wmperformstarty);
    fclose(fp);
    if ((uint32)wmperformstartx > settings.worldmaptilesx)
	wmperformstartx = -1;
    if ((uint32)wmperformstarty > settings.worldmaptilesy)
	wmperformstarty = 0;
 
}

/*
 * This routine slowly loads the world, patches it up due to the weather,
 * and saves it back to disk.  In this way, the world constantly feels the
 * effects of weather uniformly, without relying on players wandering.
 */

void perform_weather()
{
   mapstruct *m;
   char filename[MAX_BUF];
   FILE *fp;

   if (!settings.dynamiclevel)
	return;

   /* move right to left, top to bottom */
   if (wmperformstartx == (int)settings.worldmaptilesx) {
	wmperformstartx = 0;
	wmperformstarty++;
   } else
	wmperformstartx++;
   if (wmperformstarty == (int)settings.worldmaptilesy)
	wmperformstartx = wmperformstarty = 0;

    sprintf(filename, "world/world_%d_%d",
	wmperformstartx+settings.worldmapstartx,
	wmperformstarty+settings.worldmapstarty);
    m = ready_map_name(filename, 0);
    if (m == NULL)
	return; /* hrmm */

    m->temp = weathermap[wmperformstartx][wmperformstarty].temp;
    m->pressure = weathermap[wmperformstartx][wmperformstarty].pressure;
    m->humid = weathermap[wmperformstartx][wmperformstarty].humid;
    m->windspeed = weathermap[wmperformstartx][wmperformstarty].windspeed;
    m->winddir = weathermap[wmperformstartx][wmperformstarty].winddir;
    m->sky = weathermap[wmperformstartx][wmperformstarty].sky;
    decay_objects(m);
    new_save_map(m, 2); /* write the overlay */
    sprintf(filename, "%s/wmapcurpos", settings.localdir);
    if ((fp = fopen(filename, "w")) == NULL) {
	LOG(llevBug, "BUG: Cannot open %s for writing\n", filename);
	return;
    }
    fprintf(fp, "%d %d", wmperformstartx, wmperformstarty);
    fclose(fp);
}
