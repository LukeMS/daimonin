/*
    Daimonin SDL client, a client program for the Daimonin MMORPG.


  Copyright (C) 2003 Michael Toennies

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

#include "include.h"

extern _Sprite *test_sprite;

static struct Map the_map;

static struct MapCell *TheMapCache=NULL;

_mapdata MapData; /* Current shown map: mapname, length, etc */

/* we need this to parse the map and sort the multi tile monsters */
typedef struct _map_object_parse {
    int face;
    int x;
    int y;
    struct _map_object_parse *next;
}_map_object_parse;

struct _map_object_parse * start_map_object_parse=NULL;

_multi_part_obj MultiArchs[16];

/* TODO: do a real adjust... we just clear here the cache.
 */
void adjust_map_cache(int xpos, int ypos)
{
    int x,y,i;
    register struct MapCell *map;
    int xreal,yreal;

    memset(TheMapCache,0,9*(MapData.xlen*MapData.ylen)*sizeof(struct MapCell));    
    for(y=0;y<MapStatusY;y++)
    {
        for(x=0;x<MapStatusX;x++)
        {
		    xreal = xpos+(x-(MAP_MAX_SIZE-1)/2)+MapData.xlen;
			yreal = ypos+(y-(MAP_MAX_SIZE-1)/2)+MapData.ylen;
			if(xreal<0 || yreal<0 || xreal>=MapData.xlen*3 || xreal>=MapData.ylen*3)
				continue;

			map = TheMapCache+(yreal*MapData.xlen*3)+xreal;
    
			map->fog_of_war = FALSE;
			map->darkness = the_map.cells[x][y].darkness;

			for (i = 0; i < MAXFACES; i++) /* lets update the whole cell for secure */
			{
				map->faces[i] =the_map.cells[x][y].faces[i];
				map->ext[i] = the_map.cells[x][y].ext[i];
				map->pos[i] = the_map.cells[x][y].pos[i];
				map->probe[i] = the_map.cells[x][y].probe[i];
			}   
        }
    }
}


/* load the multi arch offsets */
void load_mapdef_dat(void)
{
    FILE *stream;
    int i,ii,x,y,d[32];
    char line[256];
    
    if( (stream = fopen(ARCHDEF_FILE, "r" )) != NULL )
    {
        for(i=0;i<16;i++)
        {
            if( fgets( line, 255, stream ) == NULL)
                break;
            
            sscanf(line, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", 
                &x, &y, &d[0],&d[1], &d[2], &d[3], &d[4], &d[5], &d[6]
                    , &d[7], &d[8], &d[9], &d[10], &d[11], &d[12], &d[13], &d[14], &d[15], &d[16], &d[17], &d[18]
                    , &d[19], &d[20], &d[21], &d[22], &d[23], &d[24], &d[25], &d[26], &d[27], &d[28], &d[29], &d[30]
                    , &d[31]);
            MultiArchs[i].xlen=x;
            MultiArchs[i].ylen=y;
            
            for(ii=0;ii<16;ii++)
            {
                MultiArchs[i].part[ii].xoff=d[ii*2];
                MultiArchs[i].part[ii].yoff=d[ii*2+1];
            }
        }
    }
    else
        LOG(LOG_ERROR, "ERROR: Can't find file %s\n",ARCHDEF_FILE);
    fclose(stream);
}
                           

void clear_map(void)
{
    memset(&the_map, 0, sizeof(Map));
}

void display_mapscroll(int dx,int dy)
{
        int x,y;
        struct Map newmap;

        for(x=0;x<MapStatusX;x++)
        {
                for(y=0;y<MapStatusY;y++)
                {
                        if (x+dx < 0 || x+dx >= MapStatusX ||y+dy < 0 || y+dy >= MapStatusY)
                        {
                                memset((char*)&(newmap.cells[x][y]), 0, sizeof(struct MapCell));
                        }
                        else
                        {
                                memcpy((char*)&(newmap.cells[x][y]), (char*)&(the_map.cells[x+dx][y+dy]),
                                        sizeof(struct MapCell));
                        }
                }
        }
        memcpy((char*)&the_map,(char*)&newmap,sizeof(struct Map));
}

void map_draw_map_clear(void)
{
    register int ypos, xpos, x,y;

    for(y=0;y<MapStatusY;y++)
    {
        for(x=0;x<MapStatusX;x++)
        {
            xpos = MAP_START_XOFF+x*MAP_TILE_YOFF-y*MAP_TILE_YOFF;
            ypos = MAP_START_YOFF+x*MAP_TILE_XOFF+y*MAP_TILE_XOFF;
            sprite_blt(Bitmaps[BITMAP_BLACKTILE],xpos, ypos, NULL, NULL);
        }
    }
}

void InitMapData(char *name, int xl, int yl, int px, int py)
{

    char *tmp;
	int music_fade = 0;
	void *tmp_free;
    
    if((tmp =strchr(name, '�')))
    {
        strcpy(MapData.music, tmp);
        if(init_media_tag(tmp))
			music_fade = 1;	
        media_show_update--; /* perhaps we have a png - simulate a step = map_scroll */
        
        *tmp=0;
    }

	if(!music_fade) /* there was no music tag or playon tag in this map - fade out */
	{
		/* now a interesting problem - when we have some seconds before a fadeout 
		 * to a file (and not to "mute") and we want mute now - is it possible that
		 * the mixer callback is called in a different thread? and then this thread
		 * stuck BEHIND the music_new.flag = 1 check - then the fadeout of this mute
		 * will drop whatever - the callback will play the old file. 
		 * that the classic thread/semphore problem.
		 */
		sound_fadeout_music(0);
	}

    if(name)
        strcpy(MapData.name, name);
    if (xl != -1)
        MapData.xlen = xl;
    if (yl != -1)
        MapData.ylen = yl;
    if (px != -1)
        MapData.posx = px;
    if (py != -1)
        MapData.posy = py;

    if(xl>0)
    {
        clear_map();
        if(TheMapCache)
		{
			tmp_free = &TheMapCache;
			FreeMemory(tmp_free);
		}
        /* we allocate 9 times the map... in tiled maps, we can have 8 connected
		 * maps to our map - we want cache a map except its 2 maps aways-
		 * WARNING: tiled maps must be of same size... don't attach a 32x32
		 * map on a 16x16 map. Its possible to handle this here, but then we need
		 * to know the sizes of the attached maps here 
		 */
		TheMapCache = malloc(9*xl*yl*sizeof(struct MapCell));
        memset(TheMapCache,0,9*xl*yl*sizeof(struct MapCell));
    }
}

void set_map_ext(int x, int y, int layer,int ext, int probe)
{
    register struct MapCell *map;
    int xreal,yreal;
        
    the_map.cells[x][y].ext[layer] = ext;
    if(probe != -1)
	    the_map.cells[x][y].probe[layer] = probe;
    
    xreal = MapData.posx+(x-(MAP_MAX_SIZE-1)/2)+MapData.xlen;
    yreal = MapData.posy+(y-(MAP_MAX_SIZE-1)/2)+MapData.ylen;
    if(xreal<0 || yreal<0 ||xreal >=MapData.xlen*3 || yreal >=MapData.ylen*3)
        return;
    map = TheMapCache+(yreal*MapData.xlen*3)+xreal;
    
    map->ext[layer] = ext;
    if(probe != -1)
        map->probe[layer] = probe;
    
}

void set_map_face(int x, int y, int layer, int face, int pos, int ext, char *name)
{
    register struct MapCell *map;
    int xreal,yreal,i;

    the_map.cells[x][y].faces[layer] = face;
    if(!face)
        ext=0;
    if(ext != -1)
        the_map.cells[x][y].ext[layer] = ext;
    the_map.cells[x][y].pos[layer] = pos;

	strcpy(the_map.cells[x][y].pname[layer],name);

    xreal = MapData.posx+(x-(MAP_MAX_SIZE-1)/2)+MapData.xlen;
    yreal = MapData.posy+(y-(MAP_MAX_SIZE-1)/2)+MapData.ylen;
    if(xreal<0 || yreal<0 ||xreal >=MapData.xlen*3 || yreal >=MapData.ylen*3)
        return;
    map = TheMapCache+(yreal*MapData.xlen*3)+xreal;
    
    map->fog_of_war = FALSE;
    map->darkness = the_map.cells[x][y].darkness;

    for (i = 0; i < MAXFACES; i++) /* lets update the whole cell for secure */
    {
        map->faces[i] =the_map.cells[x][y].faces[i];
        map->ext[i] = the_map.cells[x][y].ext[i];
        map->pos[i] = the_map.cells[x][y].pos[i];
        map->probe[i] = the_map.cells[x][y].probe[i];
		strcpy(map->pname[i], the_map.cells[x][y].pname[i]);
    }   
}

void display_map_clearcell(long x, long y)
{
    register struct MapCell *map;
    int xreal,yreal,i;
    
    
    the_map.cells[x][y].darkness = 0;
    for (i = 0; i < MAXFACES; i++)
    {
        the_map.cells[x][y].pname[i][0] = 0;
        the_map.cells[x][y].faces[i] = 0;
        the_map.cells[x][y].ext[i] = 0;
        the_map.cells[x][y].pos[i] = 0;
        the_map.cells[x][y].probe[i] = 0;
    }

    xreal = MapData.posx+(x-(MAP_MAX_SIZE-1)/2)+MapData.xlen;
    yreal = MapData.posy+(y-(MAP_MAX_SIZE-1)/2)+MapData.ylen;
    if(xreal<0 || yreal<0 ||xreal >=MapData.xlen*3 || yreal >=MapData.ylen*3)
        return;
    map = TheMapCache+(yreal*MapData.xlen*3)+xreal;
    
    map->fog_of_war = TRUE;
	map->darkness = 0;
    for (i = 0; i < MAXFACES; i++)
    {
        if(map->faces[i]&0x8000)
            map->faces[i] = 0;
        map->ext[i] = 0;
        map->pname[i][0] = 0;
        map->probe[i] = 0;
        
    }
}


void set_map_darkness(int x, int y, uint8 darkness)
{
    register struct MapCell *map;
    int xreal,yreal;

    if (darkness != (255 - the_map.cells[x][y].darkness ))
        the_map.cells[x][y].darkness = 255 - darkness;

    xreal = MapData.posx+(x-(MAP_MAX_SIZE-1)/2)+MapData.xlen;
    yreal = MapData.posy+(y-(MAP_MAX_SIZE-1)/2)+MapData.ylen;
	if(xreal<0 || yreal<0 || xreal>=MapData.xlen*3 || yreal>=MapData.ylen*3)
        return;
    map = TheMapCache+(yreal*MapData.xlen*3)+xreal;

    if (darkness != (255 - map->darkness ))
        map->darkness = 255 - darkness;
}


void map_draw_map(void)
{
    register struct MapCell *map;
	_Sprite *face_sprite;
    register int ypos, xpos;
    int x,y,k, xl,yl, temp,kk,kt,yt,xt,alpha;
	int xml,xmpos,xtemp=0;
    uint16 index, index_tmp;
    int mid, mnr,xreal,yreal;
    _BLTFX bltfx;
    SDL_Rect rect;
    
    /* we should move this later to a better position, this only for testing here */
    _Sprite player_dummy;
    SDL_Surface bmap;
    int player_posx, player_posy;
    int player_pixx, player_pixy;
    
    if(!TheMapCache)
        return;
    player_posx = MapStatusX-(MapStatusX/2)-1;
    player_posy = MapStatusY-(MapStatusY/2)-1;
    player_pixx = MAP_START_XOFF+player_posx*MAP_TILE_YOFF-player_posy*MAP_TILE_YOFF+20;
    player_pixy = MAP_START_YOFF+player_posx*MAP_TILE_XOFF+player_posy*MAP_TILE_XOFF-14;
    player_dummy.border_left  = -5;    
    player_dummy.border_right = 0;    
    player_dummy.border_up    = 0;    
    player_dummy.border_down  = -5;    
    player_dummy.bitmap=&bmap;
    bmap.h = 33;
    bmap.w = 35;
    player_pixy =(player_pixy+MAP_TILE_POS_YOFF)-bmap.h;
    bltfx.surface = NULL;
    bltfx.alpha=128;
    
    for(kk=0;kk<MAXFACES-1;kk++)    /* we draw floor & mask as layer wise (layer 0 & 1) */
    {    
		for(alpha=0;alpha<MAP_MAX_SIZE;alpha++)
        {
			xt=yt=-1;
			while(xt <alpha || yt < alpha)
		    {
				if(xt < alpha) /* draw x row from 0 to alpha with y = alpha */
				{
					x = ++xt;
					y = alpha;
				}
				else /* x row is drawn, now draw y row from 0 to alpha with x = alpha */
				{
					y = ++yt;
					x = alpha;
				}

				if(kk<2) /* and we draw layer 2 and 3 at once on a node */
					kt=kk;
				else
					kt=kk+1;
				for(k=kk;k<=kt;k++)
				{
                xpos = MAP_START_XOFF+x*MAP_TILE_YOFF-y*MAP_TILE_YOFF;
                ypos = MAP_START_YOFF+x*MAP_TILE_XOFF+y*MAP_TILE_XOFF;
                if(!k)      
                    sprite_blt(Bitmaps[BITMAP_BLACKTILE],xpos, ypos, NULL, NULL);
				if(!debug_layer[k])
					continue;

                xreal = MapData.posx-(MAP_MAX_SIZE-1)/2+x+MapData.xlen;
                yreal = MapData.posy-(MAP_MAX_SIZE-1)/2+y+MapData.ylen;
                
				if(xreal<0 || yreal<0 || xreal>=MapData.xlen*3 || yreal>=MapData.ylen*3)
                    continue;
				/*LOG(-1,"MAPCACHE: x:%d y:%d l:%d\n", xreal,yreal,(yreal*MapData.xlen*3)+xreal);*/
                map = TheMapCache+(yreal*MapData.xlen*3)+xreal;

                /*map = &the_map.cells[x][y]; */
                if ((index_tmp=map->faces[k]) > 0)
                {
                    index = index_tmp&~0x8000;
					face_sprite = FaceList[index].sprite;
					if(!face_sprite)
					{
						index = MAX_FACE_TILES-1;
						face_sprite = FaceList[index].sprite;
					}
                    if(face_sprite)
                    {
                        if(map->pos[k]) /* we have a set quick_pos = multi tile*/
                        {
                            mnr = map->pos[k];
                            mid = mnr >>4;
                            mnr &= 0x0f; 
							xml = MultiArchs[mid].xlen;
                            yl=ypos-MultiArchs[mid].part[mnr].yoff+MultiArchs[mid].ylen-
                                                    face_sprite->bitmap->h;
                            /* we allow overlapping x borders - we simply center then
                             */
                            xl=0;
                            if(face_sprite->bitmap->w >MultiArchs[mid].xlen)
                                xl = (MultiArchs[mid].xlen-face_sprite->bitmap->w)>>1;
							xmpos = xpos-MultiArchs[mid].part[mnr].xoff;
                            xl += xmpos;
                            
                            /*sprintf(buf,"ID:%d NR:%d yoff:%d yl:%d", mid,mnr,MultiArchs[mid].part[mnr].yoff, yl);
                            draw_info(buf,COLOR_RED);*/
                            
                        }
                        else /* single tile... */
                        {
                            /* first, we calc the shift positions */
							xml = MAP_TILE_POS_XOFF;
                            yl =(ypos+MAP_TILE_POS_YOFF)-face_sprite->bitmap->h;
                            xmpos = xl = xpos;
                        }
                        /* blt the face in the darkness level, the tile pos has */
                        temp = map->darkness;
                        if(temp == 0)
                            bltfx.dark_level=0;
                        else if(temp <= 64)
                            bltfx.dark_level=1;
                        else if(temp <= 128)
                            bltfx.dark_level=2;
                        else
                            bltfx.dark_level=3;
                    
                        /* all done, just blt the face */
                        bltfx.flags = 0;                        
                        if(k && ((x > player_posx  && y>=player_posy) ||(x >= player_posx  && y>player_posy)))
                        {
							
                            if(face_sprite && face_sprite->bitmap && k>1)
                            {
                                if(sprite_collision(player_pixx,player_pixy,xl,yl,&player_dummy, face_sprite))
                                    bltfx.flags = BLTFX_FLAG_SRCALPHA;
                            }
                        }
                        
                        if(map->fog_of_war == TRUE)
                            bltfx.flags |= BLTFX_FLAG_FOW;
                        else if(cpl.stats.flags&SF_XRAYS)
                            bltfx.flags |= BLTFX_FLAG_GREY;
                        else if(cpl.stats.flags&SF_INFRAVISION && index_tmp&0x8000 && map->darkness)
                            bltfx.flags |= BLTFX_FLAG_RED;
                        else
                            bltfx.flags |= BLTFX_FLAG_DARK;
						                            
						if(map->ext[k] &FFLAG_INVISIBLE && !(bltfx.flags&BLTFX_FLAG_FOW))
						{
							bltfx.flags &= ~BLTFX_FLAG_DARK;
                            bltfx.flags |= BLTFX_FLAG_SRCALPHA|BLTFX_FLAG_GREY;
						}
						else if(map->ext[k] &FFLAG_ETHEREAL&& !(bltfx.flags&BLTFX_FLAG_FOW))
						{
							bltfx.flags &= ~BLTFX_FLAG_DARK;
                            bltfx.flags |= BLTFX_FLAG_SRCALPHA;
						}
						if(FaceList[index].flags & FACE_FLAG_UP)
						{
							if(FaceList[index].flags & FACE_FLAG_D1)
							{
								if(y<(MAP_MAX_SIZE-1)/2)
									sprite_blt(face_sprite,xl, yl, NULL, &bltfx);
							}
							if(FaceList[index].flags & FACE_FLAG_D3)
							{
								if(x<(MAP_MAX_SIZE-1)/2 || y<(MAP_MAX_SIZE-1)/2)
									sprite_blt(face_sprite,xl, yl, NULL, &bltfx);
							}
						}
						else
	                        sprite_blt(face_sprite,xl, yl, NULL, &bltfx);

						/* here we handle high & low walls - for example when
						 * you enter a house or something. The wall will be drawn
						 * low and higher wall mask will be removed, when the wall
						 * is in front of you.
						 */
						if(FaceList[index].flags)							
						{
							if(FaceList[index].flags & FACE_FLAG_DOUBLE)
							{
								if(FaceList[index].flags & FACE_FLAG_D1)
								{
									if(y<(MAP_MAX_SIZE-1)/2)
				                        sprite_blt(face_sprite,xl, yl-22, NULL, &bltfx);
								}
								if(FaceList[index].flags & FACE_FLAG_D3)
								{
									if(x<(MAP_MAX_SIZE-1)/2 || y<(MAP_MAX_SIZE-1)/2)
				                        sprite_blt(face_sprite,xl, yl-22, NULL, &bltfx);
								}

							}
						}
                        
						/* have we a playername? then print it! */
						if(options.player_names && map->pname[k][0])
						{
							/* we must take care here later for rank! - 
							 * then we must trick a bit here! (use rank + name 
							 * and strncmp() 
							*/
							if(options.player_names == 1 || /* all names */ 
								(options.player_names == 2 && strnicmp(map->pname[k],cpl.rankandname, strlen(cpl.rankandname))) || /* names from other players only */
								(options.player_names == 3 && !strnicmp(map->pname[k],cpl.rankandname, strlen(cpl.rankandname))) ) /* only you */
							StringBlt(ScreenSurface, &Font6x3Out,map->pname[k],xpos-(strlen(map->pname[k])*2)+22, ypos-48,COLOR_DEFAULT, NULL, NULL);
						}

                        /* perhaps the objects has a marked effect, blt it now */
                        if(map->ext[k])
                        {
                            if(map->ext[k] &FFLAG_SLEEP)
                                sprite_blt(Bitmaps[BITMAP_SLEEP],xl+face_sprite->bitmap->w/2, yl-5, NULL, NULL);
                            if(map->ext[k] &FFLAG_CONFUSED)
                                sprite_blt(Bitmaps[BITMAP_CONFUSE],xl+face_sprite->bitmap->w/2-1, yl-4, NULL, NULL);
                            if(map->ext[k] &FFLAG_SCARED)
                                sprite_blt(Bitmaps[BITMAP_SCARED],xl+face_sprite->bitmap->w/2+10, yl-4, NULL, NULL);
                            if(map->ext[k] &FFLAG_BLINDED)
                                sprite_blt(Bitmaps[BITMAP_BLIND],xl+face_sprite->bitmap->w/2+3, yl-6, NULL, NULL);
                            if(map->ext[k] &FFLAG_PARALYZED)
                            {
                                sprite_blt(Bitmaps[BITMAP_PARALYZE],xl+face_sprite->bitmap->w/2+2, yl+3, NULL, NULL);
                                sprite_blt(Bitmaps[BITMAP_PARALYZE],xl+face_sprite->bitmap->w/2+9, yl+3, NULL, NULL);
                            }
                            if(map->ext[k] &FFLAG_PROBE)
                            {
                                if(face_sprite)
                                {
									if(xml == MAP_TILE_POS_XOFF)
	                                    xtemp = (int) (((double)xml/100.0)*25.0);
									else
	                                    xtemp = (int) (((double)xml/100.0)*20.0);
                                    sprite_blt(Bitmaps[BITMAP_ENEMY2],xmpos+xtemp-3, yl-11, NULL, NULL);
                                    sprite_blt(Bitmaps[BITMAP_ENEMY1],xmpos+xtemp+(xml-xtemp*2)-3, yl-11, NULL, NULL);

                                    temp = (xml-xtemp*2)-1;
                                    if (temp <=0)
                                        temp = 1;
                                    if(temp >= 300)
                                        temp = 300;
                                    mid = map->probe[k];
                                    if(mid<=0)
                                        mid = 1;
                                    if (mid>100)
                                        mid = 100;
                                    temp = (int)(((double)temp/100.0)*(double)mid);
                                    rect.h=2 ;
                                    rect.w=temp ;
                                    rect.x = 0;
                                    rect.y = 0;
                                    sprite_blt(Bitmaps[BITMAP_PROBE],xmpos+xtemp-1, yl-9, &rect, NULL);
                                }
                            }
						}
                                
                        }
                    }
                }
            };
        }       
    }
}


#define TILE_ISO_XLEN 48
/* this +1 is the trick to catch the one pixel line between
 * 2 y rows - the tiles don't touch there!
 */
#define TILE_ISO_YLEN (23+1)

static char tile_mask[TILE_ISO_YLEN][TILE_ISO_XLEN+1] ={
"000000000000000000000011112222222222222222222222",
"000000000000000000001111111122222222222222222222",
"000000000000000000111111111111222222222222222222",
"000000000000000011111111111111112222222222222222",
"000000000000001111111111111111111122222222222222",
"000000000000111111111111111111111111222222222222",
"000000000011111111111111111111111111112222222222",
"000000001111111111111111111111111111111122222222",
"000000111111111111111111111111111111111111222222",
"000011111111111111111111111111111111111111112222",
"001111111111111111111111111111111111111111111122",
"111111111111111111111111111111111111111111111111",
"331111111111111111111111111111111111111111111144",
"333311111111111111111111111111111111111111114444",
"333333111111111111111111111111111111111111444444",
"333333331111111111111111111111111111111144444444",
"333333333311111111111111111111111111114444444444",
"333333333333111111111111111111111111444444444444",
"333333333333331111111111111111111144444444444444",
"333333333333333311111111111111114444444444444444",
"333333333333333333111111111111444444444444444444",
"333333333333333333331111111144444444444444444444",
"333333333333333333333311114444444444444444444444", 
"333333333333333333333333444444444444444444444444" 
};


/* brute force at is best... */
void get_tile_position( int mx, int my, int *tx, int *ty ) 
{
	int y,x,xpos, ypos;

	mx -=MAP_START_XOFF;
	my-=MAP_START_YOFF;

	*tx=-1;
	*ty=-1;

	for (y = 0; y < MAP_MAX_SIZE; y++ )
	{
		for (x = 0; x < MAP_MAX_SIZE; x++ ) 
		{
			xpos = x*MAP_TILE_YOFF-y*MAP_TILE_YOFF;
			ypos = x*MAP_TILE_XOFF+y*MAP_TILE_XOFF;
			if(mx>= xpos && mx<xpos+TILE_ISO_XLEN && my>=ypos && my<ypos+TILE_ISO_YLEN)
			{
				/* ok, we have the box we are in... 
				 * but perhaps we have not x,y but one of the
				 * border tiles... lets check with the mask.
				 */
				if(tile_mask[my-ypos][mx-xpos] == '1') /* now we have it! */
				{
					*tx = x;
					*ty = y;
					return;
				}
			}
		}
	}
}

