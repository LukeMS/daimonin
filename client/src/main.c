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

#include <include.h>

SDL_Surface *ScreenSurface; /* THE main surface (backbuffer)*/
_Font SystemFont;			/* our main font*/
_Font SystemFontOut;		/* our main font - black outlined*/
_Font BigFont;			/* bigger special font*/
_Font Font6x3Out;		/* our main font with shadow*/
struct sockaddr_in insock;	/* Server's attributes */
ClientSocket csocket;
int SocketStatusErrorNr;		/* if an socket error, this is it */

int music_global_fade = FALSE;
int show_help_screen;

int debug_layer[MAXFACES];
int bmaptype_table_size;
_srv_client_files srv_client_files[SRV_CLIENT_FILES];

struct _options options;
Uint32 videoflags_full,videoflags_win;

struct _fire_mode fire_mode_tab[FIRE_MODE_INIT];
int RangeFireMode;

int CacheStatus;			/* cache status... set this in hardware depend */
int SoundStatus;			/* SoundStatus 0=no 1= yes */
int MapStatusX;				/* map x,y len */
int MapStatusY;

char ServerName[2048];	/* name of the server we want connect */
int ServerPort;			/* port addr */

char argServerName[2048];	/* name of the server we want connect */
int argServerPort;			/* port addr */

uint32 LastTick;			/* system time counter in ms since prg start */
uint32 GameTicksSec;		/* ticks since this second frame in ms */
uint32 tmpGameTick;			/* used from several functions, just to store real ticks */

int esc_menu_flag;
int esc_menu_index;

_bmaptype *bmap_table[BMAPTABLE];

int map_udate_flag, map_transfer_flag;			/* update map area */
int GameStatusVersionFlag;
int GameStatusVersionOKFlag;
int request_file_chain, request_file_flags;

int ToggleScreenFlag;
char InputString[MAX_INPUT_STRING];

int InputCount, InputMax;
Boolean InputStringFlag;	/* if true keyboard and game is in input str mode*/
Boolean InputStringEndFlag;	/* if true, we had entered some in text mode and its ready*/
Boolean InputStringEscFlag;

_game_status GameStatus;	/* the global status identifier */

_anim_table anim_table[MAXANIM]; /* the stored "anim commands" we created out of anims.tmp */
Animations animations[MAXANIM]; /* get this from commands.c to this place*/

_face_struct FaceList[MAX_FACE_TILES];	/* face data*/

void init_game_data(void);
Boolean game_status_chain(void);
Boolean load_bitmap(int index);

#define NCOMMANDS (sizeof(commands)/sizeof(struct CmdMapping))

typedef struct _server
{
        struct _server *next;	/* go on in list. NULL: no following this node*/
        char *nameip;
        char *version;
        char *desc1;
        char *desc2;
        char *desc3;
        char *desc4;
        int player;
        int port;
} _server;

_server *start_server, *end_server;
int metaserver_start, metaserver_sel,metaserver_count;

typedef enum _pic_type
{
        PIC_TYPE_DEFAULT, PIC_TYPE_PALETTE, PIC_TYPE_TRANS
} _pic_type;

typedef struct _bitmap_name
{
        char *name;
        _pic_type type;
}_bitmap_name ;

/* for loading, use BITMAP_xx in the other modules*/
static _bitmap_name  bitmap_name[BITMAP_INIT] =
{
        {"palette.png", PIC_TYPE_PALETTE},
        {"font7x4.png", PIC_TYPE_PALETTE},
        {"font6x3out.png", PIC_TYPE_PALETTE},
        {"font_big.png", PIC_TYPE_PALETTE},
        {"font7x4out.png", PIC_TYPE_PALETTE},
        {"intro.png", PIC_TYPE_DEFAULT},
        {"player_doll1.png", PIC_TYPE_TRANS},
        {"black_tile.png", PIC_TYPE_DEFAULT},
        {"textwin.png", PIC_TYPE_DEFAULT},
        {"meta1.png", PIC_TYPE_DEFAULT},
        {"metaslider.png", PIC_TYPE_DEFAULT},
        {"login.png", PIC_TYPE_DEFAULT},
        {"newplayer.png", PIC_TYPE_DEFAULT},
        {"login_inp.png", PIC_TYPE_DEFAULT},
        {"invslot.png", PIC_TYPE_TRANS},
        {"hp.png", PIC_TYPE_DEFAULT},
        {"hp_back.png", PIC_TYPE_DEFAULT},
        {"sp.png", PIC_TYPE_DEFAULT},
        {"sp_back.png", PIC_TYPE_DEFAULT},
        {"grace.png", PIC_TYPE_DEFAULT},
        {"grace_back.png", PIC_TYPE_DEFAULT},
        {"food.png", PIC_TYPE_DEFAULT},
        {"food_back.png", PIC_TYPE_DEFAULT},
        {"apply.png", PIC_TYPE_DEFAULT},
        {"unpaid.png", PIC_TYPE_DEFAULT},
        {"cursed.png", PIC_TYPE_DEFAULT},
        {"damned.png", PIC_TYPE_DEFAULT},
        {"lock.png", PIC_TYPE_DEFAULT},
        {"magic.png", PIC_TYPE_DEFAULT},
        {"status.png", PIC_TYPE_DEFAULT},
        {"spelllist.png", PIC_TYPE_DEFAULT},
        {"keybind.png", PIC_TYPE_DEFAULT},
        {"keybindslider.png", PIC_TYPE_DEFAULT},
        {"keypress.png", PIC_TYPE_DEFAULT},
        {"range.png", PIC_TYPE_TRANS},
        {"range_marker.png", PIC_TYPE_TRANS},
        {"range_ctrl.png", PIC_TYPE_TRANS},
        {"range_ctrl_no.png", PIC_TYPE_TRANS},
        {"range_skill.png", PIC_TYPE_TRANS},
        {"range_skill_no.png", PIC_TYPE_TRANS},
        {"range_throw.png", PIC_TYPE_TRANS},
        {"range_throw_no.png", PIC_TYPE_TRANS},
        {"range_tool.png", PIC_TYPE_TRANS},
        {"range_tool_no.png", PIC_TYPE_TRANS},
        {"range_wizard.png", PIC_TYPE_TRANS},
        {"range_wizard_no.png", PIC_TYPE_TRANS},
        {"range_priest.png", PIC_TYPE_TRANS},
        {"range_priest_no.png", PIC_TYPE_TRANS},
        {"cmark_start.png", PIC_TYPE_TRANS},
        {"cmark_end.png", PIC_TYPE_TRANS},
        {"cmark_middle.png", PIC_TYPE_TRANS},
        {"textwin_scroll.png", PIC_TYPE_DEFAULT},
        {"inv_scroll.png", PIC_TYPE_DEFAULT},
        {"below_scroll.png", PIC_TYPE_DEFAULT},
        {"number.png", PIC_TYPE_DEFAULT},
        {"keybind_new.png", PIC_TYPE_DEFAULT},
        {"keybind_edit.png", PIC_TYPE_DEFAULT},
        {"keybind_input.png", PIC_TYPE_DEFAULT},
        {"keybind_scroll.png", PIC_TYPE_DEFAULT},
        {"keybind_repeat.png", PIC_TYPE_DEFAULT},
        {"invslot_u.png", PIC_TYPE_TRANS},
        {"skill_list.png", PIC_TYPE_DEFAULT},
        {"skill_list_slider.png", PIC_TYPE_DEFAULT},
        {"skill_list_button.png", PIC_TYPE_TRANS},
        {"spelllist_slider.png", PIC_TYPE_DEFAULT},
        {"spelllist_sliderg.png", PIC_TYPE_DEFAULT},
        {"spelllist_button.png", PIC_TYPE_TRANS},
        {"death.png", PIC_TYPE_TRANS},
        {"sleep.png", PIC_TYPE_TRANS},
        {"confused.png", PIC_TYPE_TRANS},
        {"paralyzed.png", PIC_TYPE_TRANS},
        {"scared.png", PIC_TYPE_TRANS},
        {"blind.png", PIC_TYPE_TRANS},
        {"enemy1.png", PIC_TYPE_TRANS},
        {"enemy2.png", PIC_TYPE_TRANS},
        {"probe.png", PIC_TYPE_TRANS},
        {"quickslots.png", PIC_TYPE_DEFAULT},
        {"inventory.png", PIC_TYPE_DEFAULT},
        {"group.png", PIC_TYPE_DEFAULT},
        {"exp_border.png", PIC_TYPE_DEFAULT},
        {"exp_line.png", PIC_TYPE_DEFAULT},
        {"exp_bubble.png", PIC_TYPE_TRANS},
        {"exp_bubble2.png", PIC_TYPE_TRANS},
        {"stats.png", PIC_TYPE_DEFAULT},
        {"buff_spot.png", PIC_TYPE_DEFAULT},
        {"text_spot.png", PIC_TYPE_DEFAULT},
        {"player_doll2.png", PIC_TYPE_TRANS},
        {"player_doll2.png", PIC_TYPE_DEFAULT},
        {"clear_spot.png", PIC_TYPE_DEFAULT},
        {"border1.png", PIC_TYPE_TRANS},
        {"border2.png", PIC_TYPE_TRANS},
        {"border3.png", PIC_TYPE_TRANS},
        {"border4.png", PIC_TYPE_TRANS},
        {"border5.png", PIC_TYPE_TRANS},
        {"border6.png", PIC_TYPE_TRANS},

        {"panel_p1.png", PIC_TYPE_DEFAULT},
        {"group_spot.png", PIC_TYPE_DEFAULT},
        {"target_spot.png", PIC_TYPE_DEFAULT},
        {"below.png", PIC_TYPE_DEFAULT},
        {"frame_line.png", PIC_TYPE_DEFAULT},
        {"meta_scroll.png", PIC_TYPE_DEFAULT},
        {"help1.png", PIC_TYPE_DEFAULT},
        {"target_attack.png", PIC_TYPE_TRANS},
        {"target_talk.png", PIC_TYPE_TRANS},
        {"target_normal.png", PIC_TYPE_TRANS},
        {"loading.png", PIC_TYPE_TRANS},
        {"help2.png", PIC_TYPE_DEFAULT},
        {"help3.png", PIC_TYPE_DEFAULT},

        {"warn_hp.png", PIC_TYPE_DEFAULT},
        {"warn_food.png", PIC_TYPE_DEFAULT},

        {"target_hp.png", PIC_TYPE_DEFAULT},
        {"target_hp_b.png", PIC_TYPE_DEFAULT},
        {"textwin_mask.png", PIC_TYPE_DEFAULT},
        {"textwin_blank.png", PIC_TYPE_DEFAULT},
        {"textwin_split.png", PIC_TYPE_DEFAULT}, 

        {"slider_up.png", PIC_TYPE_TRANS},
        {"slider_down.png", PIC_TYPE_TRANS},
        {"slider.png", PIC_TYPE_TRANS},
        {"group_clear.png", PIC_TYPE_DEFAULT},
        {"options_head.png", PIC_TYPE_TRANS},
        {"options_keys.png", PIC_TYPE_TRANS},
        {"options_logout.png", PIC_TYPE_TRANS},
        {"options_back.png", PIC_TYPE_TRANS},
        {"options_mark_left.png", PIC_TYPE_TRANS},
        {"options_mark_right.png", PIC_TYPE_TRANS},
        {"options_alpha.png", PIC_TYPE_DEFAULT},

        {"exp_skill_border.png", PIC_TYPE_DEFAULT},
        {"exp_skill_line.png", PIC_TYPE_DEFAULT},
        {"exp_skill_bubble.png", PIC_TYPE_TRANS}, 
};

#define BITMAP_MAX (sizeof(bitmap_name)/sizeof(struct _bitmap_name))
_Sprite *Bitmaps[BITMAP_MAX];

static void count_meta_server(void);
static void show_meta_server(void);
static void show_login_server(void);
static void show_request_server(void);
static void flip_screen(void);
static void show_intro(char *text);
static void delete_player_lists(void);
void reset_input_mode(void);
void show_newplayer_server(void);

static void delete_player_lists(void)
{
	int i, ii;

        for(i=0;i<FIRE_MODE_INIT;i++)
        {
            fire_mode_tab[i].amun = FIRE_ITEM_NO;
            fire_mode_tab[i].item = FIRE_ITEM_NO;
            fire_mode_tab[i].skill = NULL;
			fire_mode_tab[i].spell = NULL;
            fire_mode_tab[i].name[0] = 0;
        }

        for(i=0;i<SKILL_LIST_MAX;i++)
        {
            for(ii=0;ii<SKILL_LIST_ENTRY;ii++)
            {   
				if(skill_list[i].entry[ii].flag==LIST_ENTRY_KNOWN)
	                skill_list[i].entry[ii].flag=LIST_ENTRY_USED;
            }
        }
                
        for(i=0;i<SPELL_LIST_MAX;i++)
        {
            for(ii=0;ii<SPELL_LIST_ENTRY;ii++)
            {      
				if(spell_list[i].entry[0][ii].flag == LIST_ENTRY_KNOWN)
					spell_list[i].entry[0][ii].flag = LIST_ENTRY_USED;
				if(spell_list[i].entry[1][ii].flag == LIST_ENTRY_KNOWN)
	                spell_list[i].entry[1][ii].flag = LIST_ENTRY_USED;
            }
        }
}


/* pre init, overrule in hardware module if needed */
void init_game_data(void)
{
    int i;
    
		textwin_set.split_flag = TRUE;
		textwin_set.size = 9;
		textwin_set.split_size = 9;
		textwin_set.top_size = 4;
		textwin_set.use_alpha = TRUE;
		textwin_set.alpha = 156;

		esc_menu_flag = FALSE;

		memset(anim_table, 0 , sizeof(anim_table));
		memset(animations, 0 , sizeof(animations));
		memset(bmaptype_table, 0 , sizeof(bmaptype_table));
        ToggleScreenFlag=FALSE;
        KeyScanFlag = FALSE;   
        memset(&fire_mode_tab,0,sizeof(fire_mode_tab));

		for(i=0;i<MAXFACES;i++)
			debug_layer[i]=TRUE;
        
        memset(&options,0,sizeof(struct _options));
        options.music_volume = 80;
        options.sound_volume = 100;
        InitMapData("", 0, 0, 0, 0);
        
        for(i=0;i<BITMAP_MAX;i++)
            Bitmaps[i]=NULL;
        memset(FaceList,0, sizeof(struct _face_struct)*MAX_FACE_TILES);
        memset(&cpl, 0, sizeof(cpl));
        cpl.ob = player_item();
        
        init_keys();
        init_player_data();
        clear_metaserver_data();
        reset_input_mode();
		show_help_screen=0;
        
        start_anim=NULL; /* anim queue of current active map */
        
		map_transfer_flag = 0;
        start_server=NULL;
        ServerName[0]=0;
        ServerPort = 13327;
        argServerName[0]=0;
        argServerPort = 13327;
        SoundSystem = SOUND_SYSTEM_OFF;
        GameStatus = GAME_STATUS_INIT;
        CacheStatus = CF_FACE_CACHE;
        SoundStatus = 1;
        MapStatusX = MAP_MAX_SIZE;
        MapStatusY = MAP_MAX_SIZE;
        map_udate_flag=2;
        InputStringFlag=FALSE;	/* if true keyboard and game is in input str mode*/
        InputStringEndFlag=FALSE;
        InputStringEscFlag=FALSE;
        csocket.fd=SOCKET_NO;
        RangeFireMode=0;

        memset(media_file,0,sizeof(_media_file )*MEDIA_MAX);
        media_count=0;	/* buffered media files*/
        media_show=MEDIA_SHOW_NO; /* show this media file*/

		delete_player_lists();
        load_options_dat(); /* now load options, allowing the user to override the presetings */     
}

void load_options_dat(void)
{
    FILE *stream;
    char line[256], keyword[256], parameter[256];
    
    if( (stream = fopen( OPTION_FILE, "r" )) != NULL )
    {
        while(1)
        {
            if( fgets( line, 255, stream ) == NULL)
                break;
            if(line[0]=='#')
                continue;
            sscanf(line, "%s %s", keyword, parameter);

            if(!strcmp(keyword,"UseOpenGL"))
                options.use_gl = atoi(parameter);
            else if(!strcmp(keyword,"ShowFrameRate"))
                options.show_frame = atoi(parameter);
            else if(!strcmp(keyword,"Fullscreen"))
                options.fullscreen = atoi(parameter);
            else if(!strcmp(keyword,"Full_HWSURFACE"))
                options.Full_HWSURFACE = atoi(parameter);
            else if(!strcmp(keyword,"Full_SWSURFACE"))
                options.Full_SWSURFACE = atoi(parameter);
            else if(!strcmp(keyword,"Full_HWACCEL"))
                options.Full_HWACCEL = 0; /* not a valid flag for video mode */
            else if(!strcmp(keyword,"Full_DOUBLEBUF"))
                options.Full_DOUBLEBUF = atoi(parameter);
            else if(!strcmp(keyword,"Full_ANYFORMAT"))
                options.Full_ANYFORMAT = atoi(parameter);
            else if(!strcmp(keyword,"Full_HWPALETTE"))
                options.Full_HWPALETTE = atoi(parameter);
            else if(!strcmp(keyword,"Full_ASYNCBLIT"))
                options.Full_ASYNCBLIT = atoi(parameter);
            else if(!strcmp(keyword,"Full_RESIZABLE"))
                options.Full_RESIZABLE = atoi(parameter);
            else if(!strcmp(keyword,"Full_NOFRAME"))
                options.Full_NOFRAME = atoi(parameter);
            else if(!strcmp(keyword,"Win_HWSURFACE"))
                options.Win_HWSURFACE = atoi(parameter);
            else if(!strcmp(keyword,"Win_SWSURFACE"))
                options.Win_SWSURFACE = atoi(parameter);
            else if(!strcmp(keyword,"Win_HWACCEL"))
                options.Win_HWACCEL = atoi(parameter);
            else if(!strcmp(keyword,"Win_DOUBLEBUF"))
                options.Win_DOUBLEBUF = atoi(parameter);
            else if(!strcmp(keyword,"Win_ANYFORMAT"))
                options.Win_ANYFORMAT = atoi(parameter);
            else if(!strcmp(keyword,"Win_HWPALETTE"))
                options.Win_HWPALETTE = atoi(parameter);
            else if(!strcmp(keyword,"Win_ASYNCBLIT"))
                options.Win_ASYNCBLIT = atoi(parameter);
            else if(!strcmp(keyword,"Win_RESIZABLE"))
                options.Win_RESIZABLE = atoi(parameter);
            else if(!strcmp(keyword,"Win_NOFRAME"))
                options.Win_NOFRAME = atoi(parameter);
            else if(!strcmp(keyword,"Win_RLEACCEL"))
                options.Win_RLEACCEL = atoi(parameter);
            else if(!strcmp(keyword,"Full_RLEACCEL"))
                options.Full_RLEACCEL = atoi(parameter);
            else if(!strcmp(keyword,"ForceRedraw"))
                options.force_redraw = atoi(parameter);
            else if(!strcmp(keyword,"Sleep"))
                options.sleep = atoi(parameter);
            else if(!strcmp(keyword,"meta_server"))
                strcpy(options.metaserver, parameter);
            else if(!strcmp(keyword,"meta_server_port"))
                options.metaserver_port = atoi(parameter);
            else if(!strcmp(keyword,"BitPerPixel"))
                options.video_bpp = (Uint8) atoi(parameter);
            else if(!strcmp(keyword,"AutomaticBPP"))
                options.auto_bpp_flag = atoi(parameter);
            else if(!strcmp(keyword,"MusicVolume"))
                options.music_volume = atoi(parameter);
            else if(!strcmp(keyword,"SoundVolume"))
                options.sound_volume = atoi(parameter);
            else if(!strcmp(keyword,"UseUpdateRect"))
                options.use_rect = atoi(parameter);
            else if(!strcmp(keyword,"MaxSpeed"))
                options.max_speed = atoi(parameter);
            else if(!strcmp(keyword,"PlayerNamesOnMap"))
                options.player_names = atoi(parameter);
            else if(!strcmp(keyword,"ShowTargetSelf"))
                options.show_target_self = atoi(parameter);
			/* text windows settings */
			else if(!strcmp(keyword,"TextWinSplit"))
                textwin_set.split_flag = atoi(parameter);
			else if(!strcmp(keyword,"TextWinAlphaFlag"))
                textwin_set.use_alpha = atoi(parameter);
			else if(!strcmp(keyword,"TextWinAlpha"))
                textwin_set.alpha = atoi(parameter);
			else if(!strcmp(keyword,"TextWinSizeDefault"))
			{
                textwin_set.size = atoi(parameter)-1;
				if(textwin_set.size<9)
					textwin_set.size = 9;
				else if(textwin_set.size>37)
					textwin_set.size = 37;
			}
			else if(!strcmp(keyword,"TextWinSizeBody"))
			{
                textwin_set.split_size = atoi(parameter)-1;
				if(textwin_set.split_size <1)
					textwin_set.split_size=1;
				else if(textwin_set.split_size >37)
					textwin_set.split_size  = 37;
			}
			else if(!strcmp(keyword,"TextWinSizeTop"))
			{
                textwin_set.top_size = atoi(parameter)-1;
				if(textwin_set.top_size <1)
					textwin_set.top_size=1;
				else if(textwin_set.top_size >37)
					textwin_set.top_size  = 37;
			}
            else if(!strcmp(keyword,"WarningFood"))
			{
				int tmp = atoi(parameter);

				if(tmp <0)
					tmp = 0;
				else if(tmp>100)
					tmp = 100;
                options.warning_food = ((float)tmp)/100.0f;
			}
            else if(!strcmp(keyword,"WarningHP"))
			{
				int tmp = atoi(parameter);

				if(tmp <0)
					tmp = 0;
				else if(tmp>100)
					tmp = 100;
                options.warning_hp = ((float)tmp)/100.0f;
			}
            else
                LOG(LOG_MSG, "WARNING: Unknown setting in %s: %s\n", OPTION_FILE,line);                
        }
		fclose(stream);
		if((textwin_set.top_size+textwin_set.split_size)>36)
		{
			textwin_set.top_size=16;
			textwin_set.split_size=16;
		}
		else if((textwin_set.top_size+textwin_set.split_size)<8)
		{
			textwin_set.top_size=3;
			textwin_set.split_size=5;
		}
    }
    else
        LOG(LOG_ERROR, "ERROR: Can't find file %s\n",OPTION_FILE);
}


/* asynchron connection chain*/
Boolean game_status_chain(void)
{
        char buf[1024];
        /* autoinit or reset prg data */
        if(GameStatus == GAME_STATUS_INIT)
        {
	        map_udate_flag=2;
			delete_player_lists();
#ifdef INSTALL_SOUND
			if(!music.flag || strcmp(music.name,"orchestral.ogg"))
	            sound_play_music("orchestral.ogg",options.music_volume,0,-1,0,MUSIC_MODE_DIRECT);
#endif
                clear_map();
                clear_metaserver_data();
                GameStatus = GAME_STATUS_META;
        }
        /* connect to meta and get server data */
        else if(GameStatus == GAME_STATUS_META)
        {
	        map_udate_flag=2;
            if(argServerName[0] != 0)
                add_metaserver_data(argServerName,
                        argServerPort, -1,"user server","Server from -server '...' command line.","","","");

				/* skip of -nometa in command line or no metaserver set in options */
				if(options.no_meta || !options.metaserver[0])
				{
					draw_info("Option '-nometa'.metaserver ignored.", COLOR_GREEN);
				}
				else
				{
					draw_info("query metaserver...", COLOR_GREEN);
					sprintf(buf,"trying %s:%d", options.metaserver, options.metaserver_port); 
					draw_info(buf, COLOR_GREEN);
					if(SOCKET_OpenSocket(&csocket.fd,&csocket, options.metaserver,options.metaserver_port))
					{
						read_metaserver_data();
						SOCKET_CloseSocket(csocket.fd);
		                draw_info("done.", COLOR_GREEN);
					}
					else
		                draw_info("metaserver failed! using default list.", COLOR_GREEN);
				}

				add_metaserver_data("127.0.0.1", 13327, -1,"local",
                        "localhost. Start server before you try to connect.","","","");
                count_meta_server();
                draw_info("select a server.", COLOR_GREEN);
                GameStatus = GAME_STATUS_START;
        }
        else if(GameStatus == GAME_STATUS_START)
        {
	            map_udate_flag=2;
				if(csocket.fd != SOCKET_NO)
	                SOCKET_CloseSocket(csocket.fd);
                clear_map();
                clear_player();
                reset_keys();
                free_faces();
                GameStatus = GAME_STATUS_WAITLOOP;
        }
        else if(GameStatus == GAME_STATUS_STARTCONNECT)
        {
				char sbuf[256];
	            sprintf(sbuf,"%s%s", GetBitmapDirectory(),bitmap_name[BITMAP_LOADING].name);
			    FaceList[MAX_FACE_TILES-1].sprite=sprite_tryload_file(sbuf,0,NULL);

	            map_udate_flag=2;
                sprintf(buf,"trying server %s:%d ...", ServerName,
                        ServerPort);
                draw_info(buf, COLOR_GREEN);
                GameStatus = GAME_STATUS_CONNECT;
        }
        else if(GameStatus == GAME_STATUS_CONNECT)
        {
                GameStatusVersionFlag = FALSE;
                if(!SOCKET_OpenSocket(&csocket.fd,&csocket, ServerName,
                        ServerPort))
                {
                        sprintf(buf,"connection failed!");
                        draw_info(buf, COLOR_RED);
                        GameStatus = GAME_STATUS_START;
                }
                GameStatus = GAME_STATUS_VERSION;

        }
        else if(GameStatus == GAME_STATUS_VERSION)
        {
                sprintf(buf,"connected. exchange version.");
                draw_info(buf, COLOR_GREEN);
                SendVersion(csocket);
                GameStatus = GAME_STATUS_WAITVERSION;
        }
        else if(GameStatus == GAME_STATUS_WAITVERSION)
        {
                /*
                * perhaps here should be a timer ???
                * remember, the version exchange server<->client is asynchron
                * so perhaps the server send his version faster
                * as the client send it to server
                */
                if(GameStatusVersionFlag)
                /* wait for version answer when needed*/
                {
                    /* false version! */
                    if(!GameStatusVersionOKFlag)
                    {
                        sprintf(buf,"wrong version!\nselect a different server.");
                        draw_info(buf, COLOR_GREEN);
                        GameStatus = GAME_STATUS_START;
                    }
                    else
                    {
                        sprintf(buf,"version confirmed.\nstarting login procedure...");
                        draw_info(buf, COLOR_GREEN);
                        GameStatus = GAME_STATUS_SETUP;
                    }
                }
        }
        else if(GameStatus == GAME_STATUS_SETUP)
        {
			map_transfer_flag=0;
			srv_client_files[SRV_CLIENT_SETTINGS].status = SRV_CLIENT_STATUS_OK;
			srv_client_files[SRV_CLIENT_BMAPS].status = SRV_CLIENT_STATUS_OK;
			srv_client_files[SRV_CLIENT_ANIMS].status = SRV_CLIENT_STATUS_OK;
			srv_client_files[SRV_CLIENT_SKILLS].status = SRV_CLIENT_STATUS_OK;
			srv_client_files[SRV_CLIENT_SPELLS].status = SRV_CLIENT_STATUS_OK;

            sprintf(buf,
                    "setup sound %d map2cmd 1 mapsize %dx%d darkness 1 facecache 1 skf %d|%x spf %d|%x bpf %d|%x stf %d|%x amf %d|%x",
                    SoundStatus,MapStatusX, MapStatusY,
					srv_client_files[SRV_CLIENT_SKILLS].len,srv_client_files[SRV_CLIENT_SKILLS].crc,
					srv_client_files[SRV_CLIENT_SPELLS].len,srv_client_files[SRV_CLIENT_SPELLS].crc,
					srv_client_files[SRV_CLIENT_BMAPS].len,srv_client_files[SRV_CLIENT_BMAPS].crc,
					srv_client_files[SRV_CLIENT_SETTINGS].len,srv_client_files[SRV_CLIENT_SETTINGS].crc,
					srv_client_files[SRV_CLIENT_ANIMS].len,srv_client_files[SRV_CLIENT_ANIMS].crc
					);
                cs_write_string(csocket.fd, buf, strlen(buf));
				request_file_chain=0;
				request_file_flags=0;

                GameStatus = GAME_STATUS_WAITSETUP;
        }
        else if(GameStatus == GAME_STATUS_REQUEST_FILES)
        {
			if(request_file_chain == 0) /* check setting list */
			{
				if(srv_client_files[SRV_CLIENT_SETTINGS].status == SRV_CLIENT_STATUS_UPDATE)
				{
					request_file_chain = 1;
					RequestFile(csocket, SRV_CLIENT_SETTINGS);
				}
				else
					request_file_chain = 2;

			}
			else if(request_file_chain == 2) /* check spell list */ 
			{
				if(srv_client_files[SRV_CLIENT_SPELLS].status == SRV_CLIENT_STATUS_UPDATE)
				{
					request_file_chain = 3;
					RequestFile(csocket, SRV_CLIENT_SPELLS);
				}
				else
					request_file_chain = 4;
			}
			else if(request_file_chain == 4) /* check skill list */ 
			{
				if(srv_client_files[SRV_CLIENT_SKILLS].status == SRV_CLIENT_STATUS_UPDATE)
				{
					request_file_chain = 5;
					RequestFile(csocket, SRV_CLIENT_SKILLS);
				}
				else
					request_file_chain = 6;
			}
			
			else if(request_file_chain == 6) 
			{
				if(srv_client_files[SRV_CLIENT_BMAPS].status == SRV_CLIENT_STATUS_UPDATE)
				{
					request_file_chain = 7;
					RequestFile(csocket, SRV_CLIENT_BMAPS);
				}
				else
					request_file_chain = 8;
			}
			else if(request_file_chain == 8) 
			{
				if(srv_client_files[SRV_CLIENT_ANIMS].status == SRV_CLIENT_STATUS_UPDATE)
				{
				request_file_chain = 9;
				RequestFile(csocket, SRV_CLIENT_ANIMS);
				}
				else
					request_file_chain = 10;
			}
			else if(request_file_chain == 10) /* we have all files - start check */
			{
				request_file_chain++; /* this ensure one loop tick and updating the messages */
			}
			else if(request_file_chain == 11) 
			{
				/* ok... now we check for bmap & anims processing... */
				read_bmap_tmp();
				read_anim_tmp();
				load_settings();
				request_file_chain++; 
			}
			else if(request_file_chain == 12) 
			{
				request_file_chain++; /* this ensure one loop tick and updating the messages */
			}
			else if(request_file_chain == 13) 
                GameStatus = GAME_STATUS_ADDME;
		}
        else if(GameStatus == GAME_STATUS_ADDME)
        {
				map_transfer_flag=0;
                SendAddMe(csocket);
                cpl.name[0]=0;
                cpl.password[0]=0;
                GameStatus = GAME_STATUS_LOGIN;
                /* now wait for login request of the server*/
        }
        else if(GameStatus == GAME_STATUS_LOGIN)
        {
				map_transfer_flag=0;
                if(InputStringEscFlag)
                {
                        sprintf(buf,"Break Login.");
                        draw_info(buf, COLOR_RED);
                        GameStatus = GAME_STATUS_START;
                }
                reset_input_mode();
        }
        else if(GameStatus == GAME_STATUS_NAME)
        {
				map_transfer_flag=0;
                /* we have a fininshed console input*/
                if(InputStringEscFlag)
                        GameStatus = GAME_STATUS_LOGIN;
                else if(InputStringFlag==FALSE && InputStringEndFlag==TRUE)
                {
                        strcpy(cpl.name, InputString);
                        LOG(LOG_MSG,"Login: send name %s\n", InputString);
                        send_reply(InputString);
                        GameStatus = GAME_STATUS_LOGIN;
                        /* now wait again for next server question*/
                }
        }
        else if(GameStatus == GAME_STATUS_PSWD)
        {
				map_transfer_flag=0;
		       /* we have a fininshed console input*/
                if(InputStringEscFlag)
                        GameStatus = GAME_STATUS_LOGIN;
                else if(InputStringFlag==FALSE && InputStringEndFlag==TRUE)
                {
                        strncpy(cpl.password, InputString,39);
                        cpl.password[39]=0;	/* insanity 0 */
                        LOG(LOG_MSG,"Login: send password <*****>\n");
                        send_reply(cpl.password);
                        GameStatus = GAME_STATUS_LOGIN;
                        /* now wait again for next server question*/
                }
        }
        else if(GameStatus == GAME_STATUS_VERIFYPSWD)
        {
				map_transfer_flag=0;
                /* we have a fininshed console input*/
                if(InputStringEscFlag)
                        GameStatus = GAME_STATUS_LOGIN;
                else if(InputStringFlag==FALSE && InputStringEndFlag==TRUE)
                {
                        LOG(LOG_MSG,"Login: send verify password %s\n",
                                InputString);
                        send_reply(InputString);
                        GameStatus = GAME_STATUS_LOGIN;
                        /* now wait again for next server question*/
                }
        }
        else if(GameStatus == GAME_STATUS_WAITFORPLAY)
        {
			clear_map();
			map_draw_map_clear();
	        map_udate_flag=2;
			map_transfer_flag = 1;
        }
        else if(GameStatus == GAME_STATUS_SETSTATS)
        {
			map_transfer_flag=0;
        }
        else if(GameStatus == GAME_STATUS_SETRACE)
        {
			map_transfer_flag=0;
        }
        else if(GameStatus == GAME_STATUS_QUIT)
        {
			map_transfer_flag=0;
        }
        return(TRUE);
}


/* load the skin & standard gfx */
void load_bitmaps(void)
{
        int i;

        for(i=0;i<=BITMAP_INTRO;i++) /* add later better error handling here*/
                load_bitmap(i);
        CreateNewFont(Bitmaps[BITMAP_FONT1], &SystemFont, 16,16,1);
        CreateNewFont(Bitmaps[BITMAP_FONT1OUT], &SystemFontOut, 16,16,1);
        CreateNewFont(Bitmaps[BITMAP_FONT6x3OUT], &Font6x3Out, 16,16,-1);
        CreateNewFont(Bitmaps[BITMAP_BIGFONT], &BigFont, 11,16, 3);
}

Boolean load_bitmap(int index)
{
        char buf[2048];
        uint32 flags=0;

        sprintf(buf,"%s%s", GetBitmapDirectory(),bitmap_name[index].name);

        if(bitmap_name[index].type == PIC_TYPE_PALETTE)
                flags |= SURFACE_FLAG_PALETTE;
        if(bitmap_name[index].type == PIC_TYPE_TRANS)
                flags |= SURFACE_FLAG_COLKEY_16M;

        Bitmaps[index] = sprite_load_file(buf,flags);
        if(!Bitmaps[index] || !Bitmaps[index]->bitmap)
		{
			LOG(LOG_MSG,"load_bitmap(): Can't load bitmap %s\n", buf);
            return(FALSE);
		}
        return(TRUE);
}

/* free the skin & standard gfx */
void free_bitmaps(void)
{
        int i;

        for(i=0;i<BITMAP_MAX;i++)
                sprite_free_sprite(Bitmaps[i]);
}

void free_faces(void)
{
        int i;

        for(i=0;i<MAX_FACE_TILES;i++)
        {
                if(FaceList[i].sprite)
                {
                        sprite_free_sprite(FaceList[i].sprite);
                        FaceList[i].sprite = NULL;
                }
                if(FaceList[i].name)
				{
					void *tmp_free=&FaceList[i].name;
					FreeMemory(tmp_free);
				}
				FaceList[i].flags =0;
        }
}


void clear_metaserver_data(void)
{
        _server *node, *tmp;
		void *tmp_free;

        node = start_server;

        for(;node;)
        {
			tmp_free = &node->nameip;
            FreeMemory(tmp_free);
			tmp_free = &node->version;
            FreeMemory(tmp_free);
			tmp_free = &node->desc1;
            FreeMemory(tmp_free);
			tmp_free = &node->desc2;
            FreeMemory(tmp_free);
			tmp_free = &node->desc3;
            FreeMemory(tmp_free);
			tmp_free = &node->desc4;
            FreeMemory(tmp_free);
            tmp = node->next;
			tmp_free = &node;
            FreeMemory(tmp_free);
            node = tmp;
        }
        start_server=NULL;
        end_server=NULL;
        metaserver_start=0;
        metaserver_sel=0;
        metaserver_count=0;
}

void add_metaserver_data(char *server, int port, int player, char *ver, char *desc1, char *desc2, char *desc3, char *desc4)
{
        _server *node;

        node = (_server*) _malloc(sizeof(_server),"add_metaserver_data(): add server struct");
        memset(node,0,sizeof(_server));
        if(!start_server)
                start_server = node;
        if(!end_server)
                end_server = node;
        else
                end_server->next = node;
        end_server = node;

        node->player = player;
        node->port = port;
        node->nameip = _malloc(strlen(server)+1,"add_metaserver_data(): nameip string");
        strcpy(node->nameip, server);
        node->version = _malloc(strlen(ver)+1,"add_metaserver_data(): version string");
        strcpy(node->version, ver);
        node->desc1 = _malloc(strlen(desc1)+1,"add_metaserver_data(): desc string");
        strcpy(node->desc1, desc1);
        node->desc2 = _malloc(strlen(desc2)+1,"add_metaserver_data(): desc string");
        strcpy(node->desc2, desc2);
        node->desc3 = _malloc(strlen(desc3)+1,"add_metaserver_data(): desc string");
        strcpy(node->desc3, desc3);
        node->desc4 = _malloc(strlen(desc4)+1,"add_metaserver_data(): desc string");
        strcpy(node->desc4, desc4);
}

static void count_meta_server(void)
{
        _server *node;

        node = start_server;
        for(metaserver_count=0;node;metaserver_count++)
                node=node->next;
}

void get_meta_server_data(int num, char *server, int *port)
{
        _server *node;
        int i;

        node = start_server;
        for(i=0;node;i++)
        {
                if(i==num)
                {
                        strcpy(server,node->nameip);
                        *port = node->port;
                        return;
                }
                node=node->next;
        }
}


void show_meta_server(void)
{
        int x,y,i;
        _server *node;
        char buf[1024];
		SDL_Rect rec_name;
		SDL_Rect rec_desc;

		rec_name.w = 272;
		rec_desc.w = 325;

        x= SCREEN_XLEN/2-Bitmaps[BITMAP_META]->bitmap->w/2+7;
        y=108;
        sprite_blt(Bitmaps[BITMAP_META],x, y, NULL, NULL);
	 	blt_window_slider(Bitmaps[BITMAP_META_SCROLL], metaserver_count,14, metaserver_start, -1,x+339, y+120);

        node = start_server;
        StringBlt(ScreenSurface,&SystemFont, "Select Server", x+17, y+92,
                COLOR_HGOLD,NULL, NULL);
        StringBlt(ScreenSurface,&SystemFont, "Players", x+296, y+92,
                COLOR_HGOLD, NULL, NULL);

        for(i=0;node && i<metaserver_start;i++)
                node=node->next;

        for(i=0;node && i<MAXMETAWINDOW;i++)
        {
                if(i== metaserver_sel-metaserver_start)
                {
                        sprintf(buf,"Version %s",node->version);
                        StringBlt(ScreenSurface,&SystemFont, buf, x+17,
                                y+300, COLOR_HGOLD, NULL, NULL);
                        StringBlt(ScreenSurface,&SystemFont,
                            node->desc1, x+17, y+312, COLOR_HGOLD, &rec_desc,NULL);
                        StringBlt(ScreenSurface,&SystemFont,
                            node->desc2, x+17, y+312+11, COLOR_HGOLD,&rec_desc,NULL);
                        StringBlt(ScreenSurface,&SystemFont,
                            node->desc3, x+17, y+312+22, COLOR_HGOLD, &rec_desc,NULL);
                        StringBlt(ScreenSurface,&SystemFont,
                            node->desc4, x+17, y+312+33, COLOR_HGOLD, &rec_desc,NULL);
                        sprite_blt(Bitmaps[BITMAP_METASLIDER],x+14, y+108+i*13,
                                NULL, NULL);
                }
                StringBlt(ScreenSurface,&SystemFont, node->nameip , x+17,
                    y+108+i*13, COLOR_WHITE, &rec_name, NULL);
                if(node->player >=0)
                    sprintf(buf,"%d",node->player);
                else
                    sprintf(buf,"-");
                StringBlt(ScreenSurface,&SystemFont, buf , x+296,
                    y+108+i*13, COLOR_WHITE, NULL, NULL);
                node = node->next;
        }
}

void reset_input_mode(void)
{
        InputString[0]=0;
        InputCount =0;
        InputStringFlag=FALSE;
        InputStringEndFlag=FALSE;
        InputStringEscFlag=FALSE;
}

void open_input_mode(int maxchar)
{
        reset_input_mode();
        InputMax =maxchar;
		SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY , SDL_DEFAULT_REPEAT_INTERVAL);
		if(cpl.input_mode != INPUT_MODE_NUMBER)
			cpl.inventory_win = IWIN_BELOW;
        InputStringFlag=TRUE;
        /* if true keyboard and game is in input str mode*/
}

void show_newplayer_server(void)
{
	int x,y;
    char buf[64];
    
	x= SCREEN_XLEN/2-Bitmaps[BITMAP_NEWPLAYER]->bitmap->w/2+7;
	y=108;
	sprite_blt(Bitmaps[BITMAP_NEWPLAYER],x, y, NULL, NULL);

    StringBlt(ScreenSurface,&SystemFont, "Create new Character", x+125, y+92,
        COLOR_HGOLD,NULL, NULL);
    
        StringBlt(ScreenSurface, &SystemFont,"1" , x+20, y+206, COLOR_WHITE, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont,"2" , x+20, y+218, COLOR_WHITE, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont,"3" , x+20, y+230, COLOR_WHITE, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont,"4" , x+20, y+242, COLOR_WHITE, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont,"5" , x+20, y+254, COLOR_WHITE, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont,"6" , x+20, y+266, COLOR_WHITE, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont,"7" , x+20, y+278, COLOR_WHITE, NULL, NULL);

        StringBlt(ScreenSurface, &SystemFont,"STR" , x+40, y+206, COLOR_HGOLD, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont,"DEX" , x+40, y+218, COLOR_HGOLD, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont,"CON" , x+40, y+230, COLOR_HGOLD, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont,"INT" , x+40, y+242, COLOR_HGOLD, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont,"WIS" , x+40, y+254, COLOR_HGOLD, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont,"POW" , x+40, y+266, COLOR_HGOLD, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont,"CHA" , x+40, y+278, COLOR_HGOLD, NULL, NULL);

        sprintf(buf, "%d", cpl.stats.Str);
		StringBlt(ScreenSurface, &SystemFont,buf , x+80, y+206, COLOR_GREEN, NULL, NULL);
		sprintf(buf, "%d", cpl.stats.Dex);
		StringBlt(ScreenSurface, &SystemFont,buf , x+80, y+218, COLOR_GREEN, NULL, NULL);
		sprintf(buf, "%d", cpl.stats.Con);
		StringBlt(ScreenSurface, &SystemFont,buf , x+80, y+230, COLOR_GREEN, NULL, NULL);
		sprintf(buf, "%d", cpl.stats.Int);
		StringBlt(ScreenSurface, &SystemFont,buf , x+80, y+242, COLOR_GREEN, NULL, NULL);
		sprintf(buf, "%d", cpl.stats.Wis);
		StringBlt(ScreenSurface, &SystemFont,buf , x+80, y+254, COLOR_GREEN, NULL, NULL);
		sprintf(buf, "%d", cpl.stats.Pow);
		StringBlt(ScreenSurface, &SystemFont,buf , x+80, y+266, COLOR_GREEN, NULL, NULL);
		sprintf(buf, "%d", cpl.stats.Cha);
		StringBlt(ScreenSurface, &SystemFont,buf , x+80, y+278, COLOR_GREEN, NULL, NULL);

    if(GameStatus ==GAME_STATUS_SETSTATS)
    {
        StringBlt(ScreenSurface, &SystemFont,"Select your Stats." , x+20, y+190, COLOR_HGOLD, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont,"G" , x+20, y+308, COLOR_RED, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont,"et stats." , x+28, y+308, COLOR_HGOLD, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont,"N" , x+20, y+321, COLOR_RED, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont,"ew stats." , x+28, y+321, COLOR_HGOLD, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont,"1-7" , x+20, y+334, COLOR_RED, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont,"for exchange stats." , x+37, y+334, COLOR_HGOLD, NULL, NULL);
    }
	else
	{
		if(cpl.ob)
			if(cpl.ob->face >0)
				if(FaceList[cpl.ob->face].sprite)
				sprite_blt(FaceList[cpl.ob->face].sprite ,x+95, y+118, NULL, NULL);

		StringBlt(ScreenSurface, &SystemFont,"Select Race and Gender." , x+170, y+110, COLOR_HGOLD, NULL, NULL);

        StringBlt(ScreenSurface, &SystemFont,"Race" , x+172, y+130, COLOR_HGOLD, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont,"Gender" , x+172, y+142, COLOR_HGOLD, NULL, NULL);

        sprintf(buf,": %s", cpl.race);
        StringBlt(ScreenSurface, &SystemFont,buf , x+210, y+130, COLOR_HGOLD, NULL, NULL);
        sprintf(buf,": %s", cpl.gender);
        StringBlt(ScreenSurface, &SystemFont,buf , x+210, y+142, COLOR_HGOLD, NULL, NULL);
        
        
        StringBlt(ScreenSurface, &SystemFont,"N" , x+170, y+162, COLOR_RED, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont,"ext choice." , x+178, y+162, COLOR_HGOLD, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont,"G" , x+170, y+174, COLOR_RED, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont,"et this character." , x+178, y+174, COLOR_HGOLD, NULL, NULL);
        
    }
}
void show_request_server(void)
{
	int x,y;
	
	x= SCREEN_XLEN/2-Bitmaps[BITMAP_META]->bitmap->w/2+7;
	y=108;
    sprite_blt(Bitmaps[BITMAP_LOGIN],x, y, NULL, NULL);

	StringBlt(ScreenSurface, &SystemFont,"UPDATING FILES" , x+20, y+120, COLOR_WHITE, NULL, NULL);
    if(request_file_chain >=0)
		StringBlt(ScreenSurface, &SystemFont,"Updating settings file from server...." , x+20, y+140, COLOR_WHITE, NULL, NULL);
	if(request_file_chain >1)
		StringBlt(ScreenSurface, &SystemFont,"Updating skills file from server...." , x+20, y+152, COLOR_WHITE, NULL, NULL);
	if(request_file_chain >3)
		StringBlt(ScreenSurface, &SystemFont,"Updating spells file from server...." , x+20, y+164, COLOR_WHITE, NULL, NULL);
	if(request_file_chain >5)
		StringBlt(ScreenSurface, &SystemFont,"Updating bmaps file from server...." , x+20, y+176, COLOR_WHITE, NULL, NULL);
	if(request_file_chain >7)
		StringBlt(ScreenSurface, &SystemFont,"Updating anims file from server...." , x+20, y+188, COLOR_WHITE, NULL, NULL);
	if(request_file_chain >9)
		StringBlt(ScreenSurface, &SystemFont,"Sync files..." , x+20, y+200, COLOR_WHITE, NULL, NULL);

	/* if set, we have requested something and the stuff in the socket buffer is our file! */
	if(request_file_chain == 1 || request_file_chain == 3 || request_file_chain ==  5 || request_file_chain == 7)
	{
		char buf[256];
		sprintf(buf,"loading %d bytes", csocket.inbuf.len);
		StringBlt(ScreenSurface, &SystemFont,buf , x+20, y+300, COLOR_WHITE, NULL, NULL);
	}
}

void show_login_server(void)
{
	int x,y, i;
    char buf[256];

	x= SCREEN_XLEN/2-Bitmaps[BITMAP_META]->bitmap->w/2+7;
	y=108;
    sprite_blt(Bitmaps[BITMAP_LOGIN],x, y, NULL, NULL);
    
	StringBlt(ScreenSurface, &SystemFont,"Enter your Name" , x+20, y+120, COLOR_HGOLD, NULL, NULL);
	sprite_blt(Bitmaps[BITMAP_LOGIN_INP],x+18, y+135, NULL, NULL);
	if(GameStatus == GAME_STATUS_NAME)
		StringBlt(ScreenSurface, &SystemFont,show_input_string(InputString,&SystemFont,Bitmaps[BITMAP_LOGIN_INP]->bitmap->w-16) , x+22, y+137, COLOR_WHITE, NULL, NULL);
	else
		StringBlt(ScreenSurface, &SystemFont,cpl.name , x+22, y+137, COLOR_WHITE, NULL, NULL);

	StringBlt(ScreenSurface, &SystemFont,"Enter your Password" , x+20, y+160, COLOR_HGOLD, NULL, NULL);
	sprite_blt(Bitmaps[BITMAP_LOGIN_INP],x+18, y+175, NULL, NULL);
	if(GameStatus == GAME_STATUS_PSWD)
    {
        strcpy(buf,show_input_string(InputString,&SystemFont,Bitmaps[BITMAP_LOGIN_INP]->bitmap->w-16));        
        for(i=0;i<(int)strlen(InputString);i++)buf[i]='*';
        StringBlt(ScreenSurface, &SystemFont,buf , x+22, y+177, COLOR_WHITE, NULL, NULL);
    }
	else
    {
        for(i=0;i<(int)strlen(cpl.password);i++)buf[i]='*';buf[i]=0;
        StringBlt(ScreenSurface, &SystemFont,buf , x+22, y+177, COLOR_WHITE, NULL, NULL);
    }
	if(GameStatus == GAME_STATUS_VERIFYPSWD)
	{
		StringBlt(ScreenSurface, &SystemFont,"New Character: Verify Password" , x+20, y+200, COLOR_HGOLD, NULL, NULL);
		sprite_blt(Bitmaps[BITMAP_LOGIN_INP],x+18, y+215, NULL, NULL);
        strcpy(buf,show_input_string(InputString,&SystemFont,Bitmaps[BITMAP_LOGIN_INP]->bitmap->w-16));
        for(i=0;i<(int)strlen(InputString);i++)buf[i]='*';
        StringBlt(ScreenSurface, &SystemFont, buf, x+22, y+217, COLOR_WHITE, NULL, NULL);
	}
}



static void play_action_sounds(void)
{
    if(!cpl.stats.food)
    {
        sound_play_one_repeat(SOUND_WARN_FOOD, SPECIAL_SOUND_FOOD);
    }
    if(cpl.warn_statdown)
    {
        sound_play_one_repeat(SOUND_WARN_STATDOWN, SPECIAL_SOUND_STATDOWN);
        cpl.warn_statdown=FALSE;
    }
    if(cpl.warn_statup)
    {
        sound_play_one_repeat(SOUND_WARN_STATUP, SPECIAL_SOUND_STATUP);
        cpl.warn_statup=FALSE;
    }
    if(cpl.warn_drain)
    {
        sound_play_one_repeat(SOUND_WARN_DRAIN, SPECIAL_SOUND_DRAIN);
        cpl.warn_drain=FALSE;
        
    }
    if(cpl.warn_hp)
    {
        if(cpl.warn_hp == 2) /* more as 10% damage */
            sound_play_effect(SOUND_WARN_HP2,0,0,100);
        else
            sound_play_effect(SOUND_WARN_HP,0,0,100);
        cpl.warn_hp=0;
        
    }
    
}

void list_vid_modes(void)
{
    const SDL_VideoInfo* vinfo = NULL;
    SDL_Rect **modes;
    int i;

        LOG(LOG_MSG,"List Video Modes\n");
        /* Get available fullscreen/hardware modes */
        modes=SDL_ListModes(NULL, SDL_HWACCEL);

        /* Check is there are any modes available */
        if(modes == (SDL_Rect **)0)
        {
                LOG(LOG_MSG,"No modes available!\n");
                exit(-1);
        }

        /* Check if or resolution is restricted */
        if(modes == (SDL_Rect **)-1){
                LOG(LOG_MSG,"All resolutions available.\n");
        }
        else
        {
                /* Print valid modes */
                LOG(LOG_MSG,"Available Modes\n");
                for(i=0;modes[i];++i)
                        LOG(LOG_MSG,"  %d x %d\n", modes[i]->w, modes[i]->h);
        }


        vinfo = SDL_GetVideoInfo( );
        LOG(LOG_MSG, "VideoInfo: hardware surfaces? %s\n",vinfo->hw_available?"yes":"no"); 
        LOG(LOG_MSG, "VideoInfo: windows manager? %s\n",vinfo->wm_available?"yes":"no"); 
        LOG(LOG_MSG, "VideoInfo: hw to hw blit? %s\n",vinfo->blit_hw?"yes":"no"); 
        LOG(LOG_MSG, "VideoInfo: hw to hw ckey blit? %s\n",vinfo->blit_hw_CC?"yes":"no"); 
        LOG(LOG_MSG, "VideoInfo: hw to hw alpha blit? %s\n",vinfo->blit_hw_A?"yes":"no"); 
        LOG(LOG_MSG, "VideoInfo: sw to hw blit? %s\n",vinfo->blit_sw?"yes":"no"); 
        LOG(LOG_MSG, "VideoInfo: sw to hw ckey blit? %s\n",vinfo->blit_sw_CC?"yes":"no"); 
        LOG(LOG_MSG, "VideoInfo: sw to hw alpha blit? %s\n",vinfo->blit_sw_A?"yes":"no"); 
        LOG(LOG_MSG, "VideoInfo: color fill? %s\n",vinfo->blit_fill?"yes":"no"); 
        LOG(LOG_MSG, "VideoInfo: video memory: %dKB\n",vinfo->video_mem); 
}

static void show_option(int mark, int x, int y)
{
	int index=0, x1,y1=0,x2,y2=0;
    _BLTFX bltfx;

    bltfx.alpha=128;
	bltfx.flags = BLTFX_FLAG_SRCALPHA;
	sprite_blt(Bitmaps[BITMAP_OPTIONS_ALPHA],x-Bitmaps[BITMAP_OPTIONS_ALPHA]->bitmap->w/2, y-20, NULL, &bltfx);

	sprite_blt(Bitmaps[BITMAP_OPTIONS_HEAD],x-Bitmaps[BITMAP_OPTIONS_HEAD]->bitmap->w/2, y, NULL, NULL);
	sprite_blt(Bitmaps[BITMAP_OPTIONS_KEYS],x-Bitmaps[BITMAP_OPTIONS_KEYS]->bitmap->w/2, y+110, NULL, NULL);
	sprite_blt(Bitmaps[BITMAP_OPTIONS_LOGOUT],x-Bitmaps[BITMAP_OPTIONS_LOGOUT]->bitmap->w/2, y+180, NULL, NULL);
	sprite_blt(Bitmaps[BITMAP_OPTIONS_BACK],x-Bitmaps[BITMAP_OPTIONS_BACK]->bitmap->w/2, y+250, NULL, NULL);

	if(esc_menu_index== ESC_MENU_KEYS)
	{
		index = BITMAP_OPTIONS_KEYS;
		y1=y2=y+115;
	}
	if(esc_menu_index== ESC_MENU_LOGOUT)
	{
		index = BITMAP_OPTIONS_LOGOUT;
		y1=y2=y+185;
	}
	if(esc_menu_index== ESC_MENU_BACK)
	{
		index = BITMAP_OPTIONS_BACK;
		y1=y2=y+255;
	}

	x1=x-Bitmaps[index]->bitmap->w/2-6;
	x2=x+Bitmaps[index]->bitmap->w/2+6;

	sprite_blt(Bitmaps[BITMAP_OPTIONS_MARK_LEFT],x1-Bitmaps[BITMAP_OPTIONS_MARK_LEFT]->bitmap->w, y1, NULL, NULL);
	sprite_blt(Bitmaps[BITMAP_OPTIONS_MARK_RIGHT],x2, y2, NULL, NULL);
}

int main(int argc, char *argv[])
{
	char buf[256];
	int x,y;
	uint32 anim_tick;
    Uint32 videoflags;
    int i,done=0,FrameCount=0;
    fd_set tmp_read,tmp_write, tmp_exceptions;
    int pollret, maxfd;
	struct timeval timeout;


	init_game_data();
    while ( argc > 1 )
    {
	--argc;
        if ( strcmp(argv[argc-1], "-port") == 0 )
        {
			argServerPort = atoi(argv[argc]);
            --argc;
        }
		else if ( strcmp(argv[argc-1], "-server") == 0 )
        {
			strcpy(argServerName, argv[argc]);
            --argc;
        }
        else if ( strcmp(argv[argc], "-nometa") == 0 )
        {
			options.no_meta = 1;
        }
        else if ( strcmp(argv[argc], "-key") == 0 )
        {
			KeyScanFlag = TRUE;   
        }
        else
        {
			char tmp[1024];
            sprintf(tmp,"Usage: %s [-server <name>] [-port <num>]\n",argv[0]);
            LOG(LOG_MSG,tmp);
            fprintf(stderr,tmp);
            exit(1);
        }
	}

	if ( SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO) < 0 )
    {
		LOG(LOG_ERROR,"Couldn't initialize SDL: %s\n",SDL_GetError());
        exit(1);
    }
	atexit(SDL_Quit);
    SYSTEM_Start(); /* start the system AFTER start SDL */
    list_vid_modes();

#ifdef INSTALL_OPENGL        
        if(options.use_gl)
        {
            const SDL_VideoInfo* info = NULL;
            info = SDL_GetVideoInfo( );
           
            SDL_GL_LoadLibrary(NULL);

            SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
            SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 5 );
            SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
            SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
            SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
          
            options.used_video_bpp=info->vfmt->BitsPerPixel;
            LOG(LOG_MSG,"Using OpenGL: bpp:%d\n", options.used_video_bpp);
            videoflags = SDL_OPENGL; 
        }
#endif
        
	videoflags=get_video_flags();
    options.used_video_bpp = options.video_bpp;
    if(!options.fullscreen_flag)
    {
		if(options.auto_bpp_flag)
        {
			const SDL_VideoInfo* info = NULL;
            info = SDL_GetVideoInfo( );
            options.used_video_bpp =info->vfmt->BitsPerPixel;
        }
	}
    if ((ScreenSurface=SDL_SetVideoMode(SCREEN_XLEN,SCREEN_YLEN,options.used_video_bpp,videoflags))== NULL )
    {
		LOG(LOG_ERROR, "Couldn't set 800x600x%d video mode: %s\n",options.used_video_bpp,SDL_GetError());
        exit(2);
    }
    else
    {
		const SDL_VideoInfo* info = NULL;
        info = SDL_GetVideoInfo( );
        options.real_video_bpp =info->vfmt->BitsPerPixel;
	}
         
    SDL_EnableUNICODE(1);        
    load_bitmaps();
	show_intro("start sound system");
    sound_init();
	show_intro("load sounds");
    sound_loadall();
	show_intro("load bitmaps");
    for(i=5;i<BITMAP_MAX;i++) /* add later better error handling here*/
		load_bitmap(i);
	show_intro("load keys");
    read_keybind_file(KEYBIND_FILE);
	show_intro("load mapdefs");
    load_mapdef_dat();
	show_intro("load picture data");
	read_bmaps_p0();
	show_intro("load settings");
    read_settings();
	show_intro("load spells");
    read_spells();
	show_intro("load skills");
    read_skills();
	show_intro("load anims");
    read_anims();
	show_intro("load bmaps");
    read_bmaps();
	show_intro(NULL);
    sound_play_music("orchestral.ogg", options.music_volume,0,-1,0,MUSIC_MODE_DIRECT);

	while(1)
	{
		SDL_Event event;
		SDL_PollEvent(&event);

		if(event.type==SDL_QUIT)
		{
			sound_freeall();
			sound_deinit();
			free_bitmaps();
			SYSTEM_End();
			return(0);
		}
		if(event.type==SDL_KEYUP ||  event.type==SDL_KEYDOWN ||
											event.type==SDL_MOUSEBUTTONDOWN)
		{
			reset_keys();
			break;
		}

		SDL_Delay(25);		/* force the thread to sleep */
	}; /* wait for keypress */

	sprintf(buf, "Welcome to Daimonin v%s", PACKAGE_VERSION); 
    draw_info(buf, COLOR_HGOLD);
    draw_info("init network...", COLOR_GREEN);
    if(!SOCKET_InitSocket()) /* log in function*/
		exit(1);
        
    maxfd = csocket.fd + 1;
    LastTick =tmpGameTick =anim_tick = SDL_GetTicks();
    GameTicksSec=0;		/* ticks since this second frame in ms */

            
	/* the one and only main loop */
    /* TODO: frame update can be optimized. It uses some cpu time because it
     * draws every loop some parts.
     */
    while(!done)
    {
		done = Event_PollInputDevice();

		#ifdef INSTALL_SOUND
			if(music_global_fade)
				sound_fadeout_music(music_new.flag);
		#endif

		#if (0) /* unused. Toggle is still buggy in SDL */
			if(ToggleScreenFlag)
			{
				uint32 flags,tf;
            
				if(options.fullscreen)
					options.fullscreen=FALSE;
				else
					options.fullscreen=TRUE;
				tf=flags = get_video_flags();
				attempt_fullscreen_toggle(&ScreenSurface,&flags );
				ToggleScreenFlag= FALSE;
			}   
		#endif

                
        /* get our ticks */
        if((LastTick - tmpGameTick) >1000 )
        {
            tmpGameTick = LastTick;
            FrameCount = 0;
            GameTicksSec=0;
        }
        GameTicksSec = LastTick - tmpGameTick;
       
        if(GameStatus > GAME_STATUS_CONNECT)
        {
            if (csocket.fd==SOCKET_NO)
            {
                /* connection closed, so we go back to INIT here*/
                if (GameStatus == GAME_STATUS_PLAY)
                    GameStatus = GAME_STATUS_INIT;
                else
                    GameStatus = GAME_STATUS_START;
            }
            else
            {
                FD_ZERO(&tmp_read);
                FD_ZERO(&tmp_write);
                FD_ZERO(&tmp_exceptions);

                FD_SET((unsigned int )csocket.fd, &tmp_exceptions);
                FD_SET((unsigned int )csocket.fd, &tmp_read);
                FD_SET((unsigned int )csocket.fd, &tmp_write);


/*			
                if (MAX_TIME!=0)
                {
                    timeout.tv_sec = MAX_TIME / 100000;
                    timeout.tv_usec = MAX_TIME % 100000;
                }
				else
				{
                    timeout.tv_sec = 0;
                    timeout.tv_usec =0;
				}
*/
                    timeout.tv_sec = 0;
                    timeout.tv_usec =0;
                /* main poll point for the socket */
                if((pollret=select(maxfd, &tmp_read, &tmp_write,&tmp_exceptions, &timeout))==-1)
                    LOG(LOG_MSG,"Got errno %d on selectcall.\n", SOCKET_GetError());
                else if (FD_ISSET(csocket.fd, &tmp_read))
                    DoClient(&csocket);
				request_face(0, 1); /* flush face request buffer */

            }
        }

		/* show help system or game screen */
		if(show_help_screen)
		{
			SDL_FillRect(ScreenSurface,NULL,0);
			if(show_help_screen==1)
				sprite_blt(Bitmaps[BITMAP_HELP1],0, 0, NULL, NULL);
			else if(show_help_screen==2)
				sprite_blt(Bitmaps[BITMAP_HELP2],0, 0, NULL, NULL);
			else if(show_help_screen==3)
				sprite_blt(Bitmaps[BITMAP_HELP3],0, 0, NULL, NULL);
		}
		else
		{
			if(GameStatus == GAME_STATUS_PLAY)
			{
				if(LastTick-anim_tick >110)
				{
					anim_tick = LastTick;
					animate_objects();
					map_udate_flag=2;
				}
		        play_action_sounds();
			}

	        if(map_udate_flag>0)
		    {
				if(GameStatus != GAME_STATUS_PLAY)
					SDL_FillRect(ScreenSurface,NULL,0);
				sprite_blt(Bitmaps[BITMAP_BUFFSPOT],518, 151, NULL, NULL);
				sprite_blt(Bitmaps[BITMAP_TEXTSPOT],226, 109, NULL, NULL);
				sprite_blt(Bitmaps[BITMAP_PDOLL2_SPOT],0, 194, NULL, NULL);
				sprite_blt(Bitmaps[BITMAP_CLEAR_SPOT],0, 306, NULL, NULL);
				if(GameStatus == GAME_STATUS_PLAY)
				{
					static int gfx_toggle=0;
					map_draw_map();
			   		play_anims(0,0); /* over the map */

					/* draw warning-icons above player */
					if ((gfx_toggle++ & 63) < 25)
					{	
						if (options.warning_hp && ((float)cpl.stats.hp / (float)cpl.stats.maxhp) <= options.warning_hp) 
							sprite_blt(Bitmaps[BITMAP_WARN_HP],393, 298, NULL, NULL);	 
					}
					else
					{
						if (options.warning_food &&  ((float)cpl.stats.food/1000.0f) <= options.warning_food) /* low food */
							sprite_blt(Bitmaps[BITMAP_WARN_FOOD],390, 294, NULL, NULL);
					}
				}
				show_quickslots(SKIN_POS_QUICKSLOT_X, SKIN_POS_QUICKSLOT_Y);
				sprite_blt(Bitmaps[BITMAP_BORDER1],0, 351, NULL, NULL);
				sprite_blt(Bitmaps[BITMAP_BORDER2],144, 423, NULL, NULL);
				sprite_blt(Bitmaps[BITMAP_BORDER3],264, 483, NULL, NULL);
				sprite_blt(Bitmaps[BITMAP_BORDER4],402, 481, NULL, NULL);
				sprite_blt(Bitmaps[BITMAP_BORDER5],539, 408, NULL, NULL);
				sprite_blt(Bitmaps[BITMAP_BORDER6],686, 351, NULL, NULL);
				sprite_blt(Bitmaps[BITMAP_PANEL_P1],686, 408, NULL, NULL);
				sprite_blt(Bitmaps[BITMAP_TARGET_SPOT],0, 423, NULL, NULL);
				sprite_blt(Bitmaps[BITMAP_GROUP_SPOT],0, 482, NULL, NULL);
			    StringBlt(ScreenSurface, &Font6x3Out,"Group",5, 525,COLOR_HGOLD, NULL, NULL);
				sprite_blt(Bitmaps[BITMAP_FLINE],1, 306, NULL, NULL);
				sprite_blt(Bitmaps[BITMAP_BELOW],264, 550, NULL, NULL);
				cpl.container=NULL; /* this will be set right on the fly in get_inventory_data() */	
				if(GameStatus  == GAME_STATUS_PLAY)
				{
					cpl.win_inv_tag = get_inventory_data(cpl.ob, &cpl.win_inv_ctag,
		                                    &cpl.win_inv_slot,&cpl.win_inv_start, 
                                            &cpl.win_inv_count, INVITEMXLEN, INVITEMYLEN);
					cpl.real_weight = cpl.window_weight;
					StringBlt(ScreenSurface, &SystemFont,"Carry",125, 470,COLOR_HGOLD, NULL, NULL);
					sprintf(buf, "%4.3f kg",cpl.real_weight);
					StringBlt(ScreenSurface, &SystemFont,buf,125+35, 470,COLOR_DEFAULT, NULL, NULL);
					StringBlt(ScreenSurface, &SystemFont,"Limit",125, 481,COLOR_HGOLD, NULL, NULL);
					sprintf(buf, "%4.3f kg",(float)(cpl.weight_limit/1000));                            
					StringBlt(ScreenSurface, &SystemFont,buf,125+35, 481,COLOR_DEFAULT, NULL, NULL);
					cpl.win_below_tag = get_inventory_data(cpl.below,&cpl.win_below_ctag,
                                                &cpl.win_below_slot,&cpl.win_below_start,
	                                            &cpl.win_below_count, INVITEMBELOWXLEN, INVITEMBELOWYLEN);
					show_target(4,498);
		            show_below_window(cpl.below, 264, 565);
				    show_group(2, 525);
					StringBlt(ScreenSurface, &Font6x3Out,"(SHIFT for inventory)",28, 478,COLOR_DEFAULT, NULL, NULL);
					show_inventory_window(6,472);
					show_media(798,171);
				}
				show_range(3, 403);

				if ((y=draggingInvItem(-1)))
				{
			      	item *Item;
					if (y ==1) 
						Item = locate_item (cpl.win_inv_tag);
					else if (y ==2) 
						Item = locate_item (cpl.win_below_tag);
					else 
						Item = locate_item (cpl.win_quick_tag);

					SDL_GetMouseState(&x, &y);
					if (Item->weight >=0)
						blt_inv_item_centered(Item, x, y);
				}

				if(esc_menu_flag == TRUE ||
					(textwin_set.use_alpha == TRUE &&(textwin_set.split_size+textwin_set.top_size)>9))
					map_udate_flag=1;
				else if(!options.force_redraw)
				{
					if(options.doublebuf_flag)
						map_udate_flag--;
					else 
						map_udate_flag=0;
				}
			} /* map update */



			show_player_stats(226, 0);
			show_resist(500,0);                
			show_textwin(539,485);
			if(GameStatus  == GAME_STATUS_PLAY)
			{
				SDL_Rect tmp_rect;
				tmp_rect.w=275;
				StringBlt(ScreenSurface, &SystemFont,MapData.name,229, 109,COLOR_DEFAULT, &tmp_rect, NULL);
				if(cpl.input_mode == INPUT_MODE_CONSOLE)
					do_console(546,586);
				else if(cpl.input_mode == INPUT_MODE_NUMBER)
					do_number(100,505);
				else if(cpl.input_mode == INPUT_MODE_GETKEY)
					do_keybind_input();
			}
			else
			{
				if(GameStatus == GAME_STATUS_WAITFORPLAY)
					StringBlt(ScreenSurface, &SystemFont,"Transfer Character to Map...",300, 300,COLOR_DEFAULT, NULL, NULL);
			}
		} /* show game screen */

        /* if not connected, walk through connection chain and/or wait for action */
        if(GameStatus != GAME_STATUS_PLAY)
        {
            if(!game_status_chain())
            {
                LOG(LOG_ERROR, "Error connecting: GStatus: %d  SocketError: %d\n",GameStatus,SOCKET_GetError());
                exit(1);
                        
            }
        }


		if(GameStatus == GAME_STATUS_REQUEST_FILES)
			show_request_server();
		else if(GameStatus <GAME_STATUS_LOGIN)
			show_meta_server();
         else if(GameStatus <GAME_STATUS_SETSTATS)
            show_login_server();
         else if(GameStatus <=GAME_STATUS_SETRACE)
            show_newplayer_server();

         /* we count always last frame*/
         FrameCount++;
         LastTick = SDL_GetTicks();

         /* print frame rate*/
         if(options.show_frame)
         {
             
             SDL_Rect rec;
             sprintf(buf,"fps %d (%d) (%d %d) %s%s%s%s%s%s%s%s%s%s %d %d", ((LastTick - tmpGameTick)/
                            FrameCount)?1000/((LastTick - tmpGameTick)/FrameCount):0,
                            (LastTick - tmpGameTick)/FrameCount, GameStatus,cpl.input_mode,
                            ScreenSurface->flags&SDL_FULLSCREEN?"F":"",
                            ScreenSurface->flags&SDL_HWSURFACE?"H":"S",
                            ScreenSurface->flags&SDL_HWACCEL?"A":"",
                            ScreenSurface->flags&SDL_DOUBLEBUF?"D":"",
                            ScreenSurface->flags&SDL_ASYNCBLIT?"a":"",
                            ScreenSurface->flags&SDL_ANYFORMAT?"f":"",
                            ScreenSurface->flags&SDL_HWPALETTE?"P":"",
                            options.rleaccel_flag?"R":"",
                            options.force_redraw?"r":"",
                            options.use_rect?"u":"",
                            options.used_video_bpp,options.real_video_bpp
                            );
                            
	         if(GameStatus  == GAME_STATUS_PLAY)
			 {
				 rec.x = 228;
			     rec.y = 122;
		         rec.h = 14;
	             rec.w = 225;    
		         SDL_FillRect(ScreenSurface, &rec, 0);             
	             StringBlt(ScreenSurface, &SystemFont,buf,rec.x, rec.y,COLOR_DEFAULT, NULL, NULL);

			 }
         }
         show_player_doll(0, 0);
			   show_player_data(0,0);		
         show_menu();

		if(map_transfer_flag)
			StringBlt(ScreenSurface, &SystemFont,"Transfer Character to Map...",300, 300,COLOR_DEFAULT, NULL, NULL);

		if(esc_menu_flag == TRUE)
			show_option(esc_menu_index, 400, 130);
		if (cursor_type)
  	{
			SDL_Rect rec;
			SDL_GetMouseState(&x, &y);
			rec.w = 14;
			rec.h = 1;
			rec.x = x-7;
			rec.y = y-2;
			SDL_FillRect(ScreenSurface, &rec, -1);
			rec.y = y-5;			
			SDL_FillRect(ScreenSurface, &rec, -1);
		} 
		flip_screen();

        if(!options.max_speed)
			SDL_Delay(options.sleep);		/* force the thread to sleep */
    }

    /* we have leaved main loop and shut down the client */
    SOCKET_DeinitSocket();
    sound_freeall();
    sound_deinit();
    free_bitmaps();
    SYSTEM_End();
    return(0);
}

static void show_intro(char *text)
{
	char buf[256];

    sprite_blt(Bitmaps[BITMAP_INTRO],0, 0, NULL, NULL);
	if(text)
		StringBlt(ScreenSurface, &SystemFont,text,370, 295,COLOR_DEFAULT, NULL, NULL);
	else
		StringBlt(ScreenSurface, &SystemFont,"** Press Key **",375, 585,COLOR_DEFAULT, NULL, NULL);
	
	sprintf(buf,"v. %s",PACKAGE_VERSION);
	StringBlt(ScreenSurface, &SystemFont,buf,10, 585,COLOR_DEFAULT, NULL, NULL);
	flip_screen();
}


static void flip_screen(void)
{
#ifdef INSTALL_OPENGL        
	if(options.use_gl)
		SDL_GL_SwapBuffers();
    else
    {
#endif
         
	if(options.use_rect)
		SDL_UpdateRect(ScreenSurface, 0, 0, SCREEN_XLEN, SCREEN_YLEN);
    else
		SDL_Flip(ScreenSurface);
#ifdef INSTALL_OPENGL        
	}
#endif
}
