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

    The author can be reached via e-mail to info@daimonin.net
*/

#include <include.h>

_server_char       *first_server_char   = NULL; /* list of possible chars/race with setup when we want create a char */
_server_char        new_character; /* if we login as new char, thats the values of it we set */

SDL_Surface        *ScreenSurface; /* THE main surface (backbuffer)*/
_Font               SystemFont;         /* our main font*/
_Font               SystemFontOut;      /* our main font - black outlined*/
_Font               BigFont;            /* bigger special font*/
_Font               Font6x3Out;     /* our main font with shadow*/
struct sockaddr_in  insock; /* Server's attributes */
ClientSocket        csocket;
int                 SocketStatusErrorNr;        /* if an socket error, this is it */

Uint32              sdl_dgreen, sdl_gray1, sdl_gray2, sdl_gray3, sdl_gray4, sdl_blue1;

int                 music_global_fade   = FALSE;
int                 show_help_screen;
int                 mb_clicked          = 0;

int                    interface_mode;

int                 debug_layer[MAXFACES];
int                 bmaptype_table_size;
_srv_client_files   srv_client_files[SRV_CLIENT_FILES];

struct _options     options;
Uint32              videoflags_full, videoflags_win;

struct _fire_mode   fire_mode_tab[FIRE_MODE_INIT];
int                 RangeFireMode;

int                 CacheStatus;            /* cache status... set this in hardware depend */
int                 SoundStatus;            /* SoundStatus 0=no 1= yes */
int                 MapStatusX;             /* map x,y len */
int                 MapStatusY;

char                ServerName[2048];   /* name of the server we want connect */
int                 ServerPort;         /* port addr */

char                argServerName[2048];    /* name of the server we want connect */
int                 argServerPort;          /* port addr */

uint32              LastTick;           /* system time counter in ms since prg start */
uint32              GameTicksSec;       /* ticks since this second frame in ms */
uint32              tmpGameTick;            /* used from several functions, just to store real ticks */

int                 esc_menu_flag;
int                 esc_menu_index;

struct gui_book_struct    *gui_interface_book;
struct gui_interface_struct *gui_interface_npc;

_bmaptype          *bmap_table[BMAPTABLE];

int                 map_udate_flag, map_transfer_flag;          /* update map area */
int                 GameStatusVersionFlag;
int                 GameStatusVersionOKFlag;
int                 request_file_chain, request_file_flags;

int                 ToggleScreenFlag;
char                InputString[MAX_INPUT_STRING];
char                InputHistory[MAX_HISTORY_LINES][MAX_INPUT_STRING];
int                 HistoryPos;
int                 CurrentCursorPos;

int                 InputCount, InputMax;
Boolean             InputStringFlag;    /* if true keyboard and game is in input str mode*/
Boolean             InputStringEndFlag; /* if true, we had entered some in text mode and its ready*/
Boolean             InputStringEscFlag;

_game_status        GameStatus; /* the global status identifier */
int					GameStatusLogin;

_anim_table         anim_table[MAXANIM]; /* the stored "anim commands" we created out of anims.tmp */
Animations          animations[MAXANIM]; /* get this from commands.c to this place*/

_face_struct        FaceList[MAX_FACE_TILES];   /* face data*/

void    init_game_data(void);
Boolean game_status_chain(void);
Boolean load_bitmap(int index);

#define NCOMMANDS (sizeof(commands)/sizeof(struct CmdMapping))

_server            *start_server, *end_server;
int                 metaserver_start, metaserver_sel, metaserver_count;

typedef enum _pic_type
{
    PIC_TYPE_DEFAULT,
    PIC_TYPE_PALETTE,
    PIC_TYPE_TRANS
}    _pic_type;

typedef struct _bitmap_name
{
    char               *name;
    _pic_type           type;
}_bitmap_name ;

/* for loading, use BITMAP_xx in the other modules*/
static _bitmap_name bitmap_name[BITMAP_INIT]    =
{
    {"palette.png", PIC_TYPE_PALETTE}, {"font7x4.png", PIC_TYPE_PALETTE}, {"font6x3out.png", PIC_TYPE_PALETTE},
    {"font_big.png", PIC_TYPE_PALETTE}, {"font7x4out.png", PIC_TYPE_PALETTE}, {"intro.png", PIC_TYPE_DEFAULT},
    {"player_doll1.png", PIC_TYPE_TRANS}, {"black_tile.png", PIC_TYPE_DEFAULT}, {"textwin.png", PIC_TYPE_DEFAULT},
    {"login_inp.png", PIC_TYPE_DEFAULT}, {"invslot.png", PIC_TYPE_TRANS}, {"testtubes.png", PIC_TYPE_TRANS},
    {"hp.png", PIC_TYPE_TRANS}, {"sp.png", PIC_TYPE_TRANS}, {"grace.png", PIC_TYPE_TRANS}, {"food.png", PIC_TYPE_TRANS},
    {"hp_back.png", PIC_TYPE_DEFAULT}, {"sp_back.png", PIC_TYPE_DEFAULT}, {"grace_back.png", PIC_TYPE_DEFAULT},
    {"food_back.png", PIC_TYPE_DEFAULT}, {"hp_back2.png", PIC_TYPE_TRANS}, {"sp_back2.png", PIC_TYPE_TRANS},
    {"grace_back2.png", PIC_TYPE_TRANS}, {"food_back2.png", PIC_TYPE_TRANS}, {"apply.png", PIC_TYPE_DEFAULT},
    {"unpaid.png", PIC_TYPE_DEFAULT}, {"cursed.png", PIC_TYPE_DEFAULT}, {"damned.png", PIC_TYPE_DEFAULT},
    {"lock.png", PIC_TYPE_DEFAULT}, {"magic.png", PIC_TYPE_DEFAULT}, {"range.png", PIC_TYPE_TRANS},
    {"range_marker.png", PIC_TYPE_TRANS}, {"range_ctrl.png", PIC_TYPE_TRANS}, {"range_ctrl_no.png", PIC_TYPE_TRANS},
    {"range_skill.png", PIC_TYPE_TRANS}, {"range_skill_no.png", PIC_TYPE_TRANS}, {"range_throw.png", PIC_TYPE_TRANS},
    {"range_throw_no.png", PIC_TYPE_TRANS}, {"range_tool.png", PIC_TYPE_TRANS}, {"range_tool_no.png", PIC_TYPE_TRANS},
    {"range_wizard.png", PIC_TYPE_TRANS}, {"range_wizard_no.png", PIC_TYPE_TRANS}, {"range_priest.png", PIC_TYPE_TRANS},
    {"range_priest_no.png", PIC_TYPE_TRANS}, {"cmark_start.png", PIC_TYPE_TRANS}, {"cmark_end.png", PIC_TYPE_TRANS},
    {"cmark_middle.png", PIC_TYPE_TRANS}, {"textwin_scroll.png", PIC_TYPE_DEFAULT},
    {"inv_scroll.png", PIC_TYPE_DEFAULT}, {"below_scroll.png", PIC_TYPE_DEFAULT}, {"number.png", PIC_TYPE_DEFAULT},
    {"invslot_u.png", PIC_TYPE_TRANS}, {"death.png", PIC_TYPE_TRANS}, {"sleep.png", PIC_TYPE_TRANS},
    {"confused.png", PIC_TYPE_TRANS}, {"paralyzed.png", PIC_TYPE_TRANS}, {"scared.png", PIC_TYPE_TRANS},
    {"blind.png", PIC_TYPE_TRANS}, {"enemy1.png", PIC_TYPE_TRANS}, {"enemy2.png", PIC_TYPE_TRANS},
    {"probe.png", PIC_TYPE_TRANS}, {"quickslots.png", PIC_TYPE_DEFAULT}, {"inventory.png", PIC_TYPE_DEFAULT},
    {"group.png", PIC_TYPE_DEFAULT}, {"exp_border.png", PIC_TYPE_DEFAULT}, {"exp_line.png", PIC_TYPE_DEFAULT},
    {"exp_bubble.png", PIC_TYPE_TRANS}, {"exp_bubble2.png", PIC_TYPE_TRANS}, {"stats.png", PIC_TYPE_DEFAULT},
    {"buff_spot.png", PIC_TYPE_DEFAULT}, {"text_spot.png", PIC_TYPE_DEFAULT}, {"player_doll2.png", PIC_TYPE_TRANS},
    {"player_doll2.png", PIC_TYPE_DEFAULT}, {"clear_spot.png", PIC_TYPE_DEFAULT}, {"border1.png", PIC_TYPE_TRANS},
    {"border2.png", PIC_TYPE_TRANS}, {"border3.png", PIC_TYPE_TRANS}, {"border4.png", PIC_TYPE_TRANS},
    {"border5.png", PIC_TYPE_TRANS}, {"border6.png", PIC_TYPE_TRANS}, {"panel_p1.png", PIC_TYPE_DEFAULT},
    {"group_spot.png", PIC_TYPE_DEFAULT}, {"target_spot.png", PIC_TYPE_DEFAULT}, {"below.png", PIC_TYPE_DEFAULT},
    {"frame_line.png", PIC_TYPE_DEFAULT}, {"help1.png", PIC_TYPE_DEFAULT}, {"target_attack.png", PIC_TYPE_TRANS},
    {"target_talk.png", PIC_TYPE_TRANS}, {"target_normal.png", PIC_TYPE_TRANS}, {"loading.png", PIC_TYPE_TRANS},
    {"help2.png", PIC_TYPE_DEFAULT}, {"warn_hp.png", PIC_TYPE_DEFAULT}, {"warn_food.png", PIC_TYPE_DEFAULT},
    {"logo270.png", PIC_TYPE_DEFAULT}, {"dialog_bg.png", PIC_TYPE_DEFAULT},
    {"dialog_title_options.png", PIC_TYPE_DEFAULT}, {"dialog_title_keybind.png", PIC_TYPE_DEFAULT},
    {"dialog_title_skill.png", PIC_TYPE_DEFAULT}, {"dialog_title_spell.png", PIC_TYPE_DEFAULT},
    {"dialog_title_creation.png", PIC_TYPE_DEFAULT}, {"dialog_title_login.png", PIC_TYPE_DEFAULT},
    {"dialog_button_up.png", PIC_TYPE_DEFAULT}, {"dialog_button_down.png", PIC_TYPE_DEFAULT},
    {"dialog_tab_start.png", PIC_TYPE_DEFAULT}, {"dialog_tab.png", PIC_TYPE_DEFAULT},
    {"dialog_tab_stop.png", PIC_TYPE_DEFAULT}, {"dialog_tab_sel.png", PIC_TYPE_DEFAULT},
    {"dialog_checker.png", PIC_TYPE_DEFAULT}, {"dialog_range_off.png", PIC_TYPE_DEFAULT},
    {"dialog_range_l.png", PIC_TYPE_DEFAULT}, {"dialog_range_r.png", PIC_TYPE_DEFAULT},
    {"target_hp.png", PIC_TYPE_DEFAULT}, {"target_hp_b.png", PIC_TYPE_DEFAULT}, {"textwin_mask.png", PIC_TYPE_DEFAULT},
    {"textwin_blank.png", PIC_TYPE_DEFAULT}, {"slider_up.png", PIC_TYPE_TRANS}, {"slider_down.png", PIC_TYPE_TRANS},
    {"slider.png", PIC_TYPE_TRANS}, {"group_clear.png", PIC_TYPE_DEFAULT}, {"exp_skill_border.png", PIC_TYPE_DEFAULT},
    {"exp_skill_line.png", PIC_TYPE_DEFAULT}, {"exp_skill_bubble.png", PIC_TYPE_TRANS},
    {"options_head.png", PIC_TYPE_TRANS}, {"options_keys.png", PIC_TYPE_TRANS},
    {"options_settings.png", PIC_TYPE_TRANS}, {"options_logout.png", PIC_TYPE_TRANS},
    {"options_back.png", PIC_TYPE_TRANS}, {"options_mark_left.png", PIC_TYPE_TRANS},
    {"options_mark_right.png", PIC_TYPE_TRANS}, {"options_alpha.png", PIC_TYPE_DEFAULT},
    {"pentagram.png", PIC_TYPE_DEFAULT}, {"quad_button_up.png", PIC_TYPE_DEFAULT},
    {"quad_button_down.png", PIC_TYPE_DEFAULT}, {"nchar_marker.png", PIC_TYPE_TRANS}, {"traped.png", PIC_TYPE_TRANS},
    {"pray.png", PIC_TYPE_TRANS}, {"wand.png", PIC_TYPE_TRANS}, {"invite.png", PIC_TYPE_DEFAULT},
    {"dialog_button_black_up.png", PIC_TYPE_DEFAULT},{"dialog_button_black_down.png", PIC_TYPE_DEFAULT},
    {"button_small_up.png", PIC_TYPE_DEFAULT},{"button_small_down.png", PIC_TYPE_DEFAULT},
    {"group_mana.png", PIC_TYPE_DEFAULT},{"group_grace.png", PIC_TYPE_DEFAULT},
    {"group_hp.png", PIC_TYPE_DEFAULT}, {"npc_interface.png", PIC_TYPE_TRANS},{"coin_copper.png", PIC_TYPE_TRANS},
    {"coin_silver.png", PIC_TYPE_TRANS},{"coin_gold.png", PIC_TYPE_TRANS},
    {"coin_mithril.png", PIC_TYPE_TRANS},{"npc_int_slider.png", PIC_TYPE_DEFAULT},
    {"journal.png", PIC_TYPE_TRANS}, {"invslot_marked.png", PIC_TYPE_TRANS}
};

#define BITMAP_MAX (sizeof(bitmap_name)/sizeof(struct _bitmap_name))
_Sprite            *Bitmaps[BITMAP_MAX];

static void         count_meta_server(void);
static void         flip_screen(void);
static void         show_intro(char *text);
static void         delete_player_lists(void);

/* Ensures that the username doesn't contain any invalid character */
static int is_username_valid(const char *name)
{
    int i;

    for(i=0; i< (int)strlen(name); i++)
    {
        if (name[i]!= '_' && !(((name[i] <= 90) && (name[i]>=65))||((name[i] >= 97) && (name[i]<=122))))
            return 0;
    }
    return 1;
}

static void delete_player_lists(void)
{
    int i, ii;

    for (i = 0; i < FIRE_MODE_INIT; i++)
    {
        fire_mode_tab[i].amun = FIRE_ITEM_NO;
        fire_mode_tab[i].item = FIRE_ITEM_NO;
        fire_mode_tab[i].skill = NULL;
        fire_mode_tab[i].spell = NULL;
        fire_mode_tab[i].name[0] = 0;
    }

    for (i = 0; i < SKILL_LIST_MAX; i++)
    {
        for (ii = 0; ii < DIALOG_LIST_ENTRY; ii++)
        {
            if (skill_list[i].entry[ii].flag == LIST_ENTRY_KNOWN)
                skill_list[i].entry[ii].flag = LIST_ENTRY_USED;
        }
    }

    for (i = 0; i < SPELL_LIST_MAX; i++)
    {
        for (ii = 0; ii < DIALOG_LIST_ENTRY; ii++)
        {
            if (spell_list[i].entry[0][ii].flag == LIST_ENTRY_KNOWN)
                spell_list[i].entry[0][ii].flag = LIST_ENTRY_USED;
            if (spell_list[i].entry[1][ii].flag == LIST_ENTRY_KNOWN)
                spell_list[i].entry[1][ii].flag = LIST_ENTRY_USED;
        }
    }
}


/* pre init, overrule in hardware module if needed */
void init_game_data(void)
{
    int i;

	memset(&global_buttons,-1, sizeof(button_status));

    textwin_init();
    textwin_flags = 0;
    first_server_char = NULL;

    esc_menu_flag = FALSE;
    srand(time(NULL));

    memset(anim_table, 0, sizeof(anim_table));
    memset(animations, 0, sizeof(animations));
    memset(bmaptype_table, 0, sizeof(bmaptype_table));
    ToggleScreenFlag = FALSE;
    KeyScanFlag = FALSE;
    memset(&fire_mode_tab, 0, sizeof(fire_mode_tab));

    for (i = 0; i < MAXFACES; i++)
        debug_layer[i] = TRUE;

    memset(&options, 0, sizeof(struct _options));
    InitMapData("", 0, 0, 0, 0);

    for (i = 0; i < BITMAP_MAX; i++)
        Bitmaps[i] = NULL;
    memset(FaceList, 0, sizeof(struct _face_struct) * MAX_FACE_TILES);
    memset(&cpl, 0, sizeof(cpl));
    cpl.ob = player_item();

    init_keys();
    init_player_data();
    clear_metaserver_data();
    reset_input_mode();
    show_help_screen = 0;

    start_anim = NULL; /* anim queue of current active map */

    clear_group();
    interface_mode = INTERFACE_MODE_NO;
    map_transfer_flag = 0;
    start_server = NULL;
    ServerName[0] = 0;
    ServerPort = 13327;
    argServerName[0] = 0;
    argServerPort = 13327;
    SoundSystem = SOUND_SYSTEM_OFF;
    GameStatus = GAME_STATUS_INIT;
    GameStatusLogin = TRUE;
    CacheStatus = CF_FACE_CACHE;
    SoundStatus = 1;
    MapStatusX = MAP_MAX_SIZE;
    MapStatusY = MAP_MAX_SIZE;
    map_udate_flag = 2;
    InputStringFlag = FALSE;    /* if true keyboard and game is in input str mode*/
    InputStringEndFlag = FALSE;
    InputStringEscFlag = FALSE;
    csocket.fd = SOCKET_NO;
    RangeFireMode = 0;
    gui_interface_npc = NULL;
    gui_interface_book = NULL;

    memset(media_file, 0, sizeof(_media_file) * MEDIA_MAX);
    media_count = 0;    /* buffered media files*/
    media_show = MEDIA_SHOW_NO; /* show this media file*/

    textwin_clearhistory();
    delete_player_lists();
    load_options_dat(); /* now load options, allowing the user to override the presetings */
    server_level.exp[1]=2500; /* dummy value for startup */
}

/******************************************************************
 Save the option file.
******************************************************************/
void save_options_dat(void)
{
    char    txtBuffer[20];
    int     i = -1, j = -1;
    FILE   *stream;

    if (!(stream = fopen_wrapper(OPTION_FILE, "w")))
        return;
    fputs("###############################################\n", stream);
    fputs("# This is the Daimonin SDL client option file #\n", stream);
    fputs("###############################################\n", stream);
    while (opt_tab[++i])
    {
        fputs("\n# ", stream);
        fputs(opt_tab[i], stream);
        fputs("\n", stream);
        while (opt[++j].name && opt[j].name[0] != '#')
        {
            fputs(opt[j].name, stream);
            switch (opt[j].value_type)
            {
                case VAL_BOOL:
                case VAL_INT:
                  sprintf(txtBuffer, " %d", *((int *) opt[j].value));
                  break;
                case VAL_U32:
                  sprintf(txtBuffer, " %d", *((uint32 *) opt[j].value));
                  break;
                case VAL_CHAR:
                  sprintf(txtBuffer, " %d", *((uint8 *) opt[j].value));
                  break;
            }
            fputs(txtBuffer, stream);
            fputs("\n", stream);
        }
    }
    fclose(stream);
}

/******************************************************************
 Load the option file.
******************************************************************/
void load_options_dat(void)
{
    int     i = -1, pos;
    FILE   *stream;
    char    line[256], keyword[256], parameter[256];

    /* Fill all options with default values */
    while (opt[++i].name)
    {
        if (opt[i].name[0] == '#')
            continue;
        switch (opt[i].value_type)
        {
            case VAL_BOOL:
            case VAL_INT:
              *((int *) opt[i].value) = opt[i].default_val;
              break;
            case VAL_U32:
              *((uint32 *) opt[i].value) = opt[i].default_val;
              break;
            case VAL_CHAR:
              *((uint8 *) opt[i].value) = (uint8) opt[i].default_val;
              break;
              /* case VAL_STR
              ...  = opt[j].info2;
              */
        }
    }

    /* Read the options from file */
    if (!(stream = fopen_wrapper(OPTION_FILE, "r")))
    {
        LOG(LOG_MSG, "Can't find file %s. Using defaults.\n", OPTION_FILE);
        return;
    }
    while (fgets(line, 255, stream))
    {
        if (line[0] == '#' || line[0] == '\n')
            continue;
        i = 0;
        while (line[i] && line[i] != ':')
            i++;
        line[++i] = 0;
        strcpy(keyword, line);
        strcpy(parameter, line + i + 1);
        i = atoi(parameter);

        pos = -1;
        while (opt[++pos].name)
        {
            if (!strcmp(keyword, opt[pos].name))
            {
                switch (opt[pos].value_type)
                {
                    case VAL_BOOL:
                    case VAL_INT:
                      *((int *) opt[pos].value) = i;
                      break;
                    case VAL_U32:
                      *((uint32 *) opt[pos].value) = i;
                      break;
                    case VAL_CHAR:
                      *((uint8 *) opt[pos].value) = (uint8) i;
                      break;
                }
            }
        }
    }
    fclose(stream);
    /*
     TODO implement server options.
    */
    strcpy(options.metaserver, "damn.informatik.uni-bremen.de");
    options.metaserver_port = 13326;

    if ((txtwin[TW_MSG].size + txtwin[TW_CHAT].size) > 36)
    {
        txtwin[TW_MSG].size = 16;
        txtwin[TW_CHAT].size = 16;
    }
    else if ((txtwin[TW_MSG].size + txtwin[TW_CHAT].size) < 8)
    {
        txtwin[TW_MSG].size = 5;
        txtwin[TW_CHAT].size = 3;
    }
}


/* asynchron connection chain*/
Boolean game_status_chain(void)
{
    char    buf[1024];
    /* autoinit or reset prg data */
    if (GameStatus == GAME_STATUS_INIT)
    {
		cpl.mark_count = -1;
	    GameStatusLogin = TRUE;
        interface_mode = INTERFACE_MODE_NO;
        clear_group();
        map_udate_flag = 2;
        delete_player_lists();
#ifdef INSTALL_SOUND
        if (!music.flag || strcmp(music.name, "orchestral.ogg"))
            sound_play_music("orchestral.ogg", options.music_volume, 0, -1, 0, MUSIC_MODE_DIRECT);
#endif
        clear_map();
        clear_metaserver_data();
        GameStatus = GAME_STATUS_META;
    }
    /* connect to meta and get server data */
    else if (GameStatus == GAME_STATUS_META)
    {
        interface_mode = INTERFACE_MODE_NO;
        clear_group();
        map_udate_flag = 2;
        if (argServerName[0] != 0)
            add_metaserver_data(argServerName, argServerPort, -1, "user server",
                                "Server from -server '...' command line.", "", "", "");

        /* skip of -nometa in command line or no metaserver set in options */
        if (options.no_meta || !options.metaserver[0])
        {
            draw_info("Option '-nometa'.metaserver ignored.", COLOR_GREEN);
        }
        else
        {
            draw_info("query metaserver...", COLOR_GREEN);
            sprintf(buf, "trying %s:%d", options.metaserver, options.metaserver_port);
            draw_info(buf, COLOR_GREEN);
            if (SOCKET_OpenSocket(&csocket.fd, &csocket, options.metaserver, options.metaserver_port))
            {
                read_metaserver_data();
                SOCKET_CloseSocket(csocket.fd);
                draw_info("done.", COLOR_GREEN);
            }
            else
                draw_info("metaserver failed! using default list.", COLOR_GREEN);
        }

        add_metaserver_data("127.0.0.1", 13327, -1, "local", "localhost. Start server before you try to connect.", "",
                            "", "");
        count_meta_server();
        draw_info("select a server.", COLOR_GREEN);
        GameStatus = GAME_STATUS_START;
    }
    else if (GameStatus == GAME_STATUS_START)
    {
        interface_mode = INTERFACE_MODE_NO;
        clear_group();
        map_udate_flag = 2;
        if (csocket.fd != SOCKET_NO)
            SOCKET_CloseSocket(csocket.fd);
        clear_map();
        clear_player();
        reset_keys();
        free_faces();
        GameStatus = GAME_STATUS_WAITLOOP;
    }
    else if (GameStatus == GAME_STATUS_STARTCONNECT)
    {
        char    sbuf[256];
        clear_group();
        sprintf(sbuf, "%s%s", GetBitmapDirectory(), bitmap_name[BITMAP_LOADING].name);
        FaceList[MAX_FACE_TILES - 1].sprite = sprite_tryload_file(sbuf, 0, NULL);

        map_udate_flag = 2;
        sprintf(buf, "trying server %s:%d ...", ServerName, ServerPort);
        draw_info(buf, COLOR_GREEN);
        GameStatus = GAME_STATUS_CONNECT;
    }
    else if (GameStatus == GAME_STATUS_CONNECT)
    {
        clear_group();
        GameStatusVersionFlag = FALSE;
        if (!SOCKET_OpenSocket(&csocket.fd, &csocket, ServerName, ServerPort))
        {
            sprintf(buf, "connection failed!");
            draw_info(buf, COLOR_RED);
            GameStatus = GAME_STATUS_START;
        }
        GameStatus = GAME_STATUS_VERSION;
        sprintf(buf, "connected. exchange version.");
        draw_info(buf, COLOR_GREEN);
    }
    else if (GameStatus == GAME_STATUS_VERSION)
    {
        SendVersion(csocket);
        GameStatus = GAME_STATUS_WAITVERSION;
    }
    else if (GameStatus == GAME_STATUS_WAITVERSION)
    {
        /*
        * perhaps here should be a timer ???
        * remember, the version exchange server<->client is asynchron
        * so perhaps the server send his version faster
        * as the client send it to server
        */
        if (GameStatusVersionFlag)
                        /* wait for version answer when needed*/
        {
            /* false version! */
            if (!GameStatusVersionOKFlag)
            {
                GameStatus = GAME_STATUS_START;
            }
            else
            {
                sprintf(buf, "version confirmed.\nstarting login procedure...");
                draw_info(buf, COLOR_GREEN);
                GameStatus = GAME_STATUS_SETUP;
            }
        }
    }
    else if (GameStatus == GAME_STATUS_SETUP)
    {
             clear_group();
             map_transfer_flag = 0;
             srv_client_files[SRV_CLIENT_SETTINGS].status = SRV_CLIENT_STATUS_OK;
             srv_client_files[SRV_CLIENT_BMAPS].status = SRV_CLIENT_STATUS_OK;
             srv_client_files[SRV_CLIENT_ANIMS].status = SRV_CLIENT_STATUS_OK;
             srv_client_files[SRV_CLIENT_SKILLS].status = SRV_CLIENT_STATUS_OK;
             srv_client_files[SRV_CLIENT_SPELLS].status = SRV_CLIENT_STATUS_OK;

        sprintf(buf,
                "setup sound %d map2cmd 1 mapsize %dx%d darkness 1 facecache 1 skf %d|%x spf %d|%x bpf %d|%x stf %d|%x amf %d|%x",
                SoundStatus, MapStatusX, MapStatusY, srv_client_files[SRV_CLIENT_SKILLS].len,
                srv_client_files[SRV_CLIENT_SKILLS].crc, srv_client_files[SRV_CLIENT_SPELLS].len,
                srv_client_files[SRV_CLIENT_SPELLS].crc, srv_client_files[SRV_CLIENT_BMAPS].len,
                srv_client_files[SRV_CLIENT_BMAPS].crc, srv_client_files[SRV_CLIENT_SETTINGS].len,
                srv_client_files[SRV_CLIENT_SETTINGS].crc, srv_client_files[SRV_CLIENT_ANIMS].len,
                srv_client_files[SRV_CLIENT_ANIMS].crc);
             cs_write_string(csocket.fd, buf, strlen(buf));
             request_file_chain = 0;
             request_file_flags = 0;

             GameStatus = GAME_STATUS_WAITSETUP;
    }
    else if (GameStatus == GAME_STATUS_REQUEST_FILES)
    {
        if (request_file_chain == 0) /* check setting list */
        {
            if (srv_client_files[SRV_CLIENT_SETTINGS].status == SRV_CLIENT_STATUS_UPDATE)
            {
                request_file_chain = 1;
                RequestFile(csocket, SRV_CLIENT_SETTINGS);
            }
            else
                request_file_chain = 2;
        }
        else if (request_file_chain == 2) /* check spell list */
        {
            if (srv_client_files[SRV_CLIENT_SPELLS].status == SRV_CLIENT_STATUS_UPDATE)
            {
                request_file_chain = 3;
                RequestFile(csocket, SRV_CLIENT_SPELLS);
            }
            else
                request_file_chain = 4;
        }
        else if (request_file_chain == 4) /* check skill list */
        {
            if (srv_client_files[SRV_CLIENT_SKILLS].status == SRV_CLIENT_STATUS_UPDATE)
            {
                request_file_chain = 5;
                RequestFile(csocket, SRV_CLIENT_SKILLS);
            }
            else
                request_file_chain = 6;
        }
        else if (request_file_chain == 6)
        {
            if (srv_client_files[SRV_CLIENT_BMAPS].status == SRV_CLIENT_STATUS_UPDATE)
            {
                request_file_chain = 7;
                RequestFile(csocket, SRV_CLIENT_BMAPS);
            }
            else
                request_file_chain = 8;
        }
        else if (request_file_chain == 8)
        {
            if (srv_client_files[SRV_CLIENT_ANIMS].status == SRV_CLIENT_STATUS_UPDATE)
            {
                request_file_chain = 9;
                RequestFile(csocket, SRV_CLIENT_ANIMS);
            }
            else
                request_file_chain = 10;
        }
        else if (request_file_chain == 10) /* we have all files - start check */
        {
            request_file_chain++; /* this ensure one loop tick and updating the messages */
        }
        else if (request_file_chain == 11)
        {
            /* ok... now we check for bmap & anims processing... */
            read_bmap_tmp();
            read_anim_tmp();
            load_settings();
            request_file_chain++;
        }
        else if (request_file_chain == 12)
        {
            request_file_chain++; /* this ensure one loop tick and updating the messages */
        }
        else if (request_file_chain == 13)
            GameStatus = GAME_STATUS_LOGIN_SELECT; /* now lets start the real login by asking "login" or "create" */
    }
    else if (GameStatus == GAME_STATUS_LOGIN_SELECT)
    {
        /* the choices are in event.c */
        map_transfer_flag = 0;
    }
    else if (GameStatus == GAME_STATUS_ADDME)
    {
		cpl.mark_count = -1;
        map_transfer_flag = 0;
        SendAddMe(csocket);
        cpl.name[0] = 0;
        cpl.password[0] = 0;
        GameStatus = GAME_STATUS_LOGIN;
        /* now wait for login request of the server*/
    }
    else if (GameStatus == GAME_STATUS_LOGIN)
    {
        map_transfer_flag = 0;
        if (InputStringEscFlag)
        {
            sprintf(buf, "Break Login.");
            draw_info(buf, COLOR_RED);
            GameStatus = GAME_STATUS_START;
			GameStatusLogin = TRUE;
        }
        reset_input_mode();
    }
    else if (GameStatus == GAME_STATUS_NAME)
    {
        map_transfer_flag = 0;
        /* we have a fininshed console input*/
        if (InputStringEscFlag)
            GameStatus = GAME_STATUS_LOGIN;
        else if (InputStringFlag == FALSE && InputStringEndFlag == TRUE)
        {
            int check;
            check = is_username_valid(InputString);
            if (check)
            {
				char name_tmp[256];

                strcpy(cpl.name, InputString);
                dialog_login_warning_level = DIALOG_LOGIN_WARNING_NONE;
                LOG(LOG_MSG,"Login: send name %s\n", InputString);
				sprintf(name_tmp,"%c%s", GameStatusLogin?'L':'C', InputString);
                send_reply(name_tmp);
                GameStatus = GAME_STATUS_NAME_WAIT;
                /* now wait again for next server question*/
            }
            else
            {
                dialog_login_warning_level = DIALOG_LOGIN_WARNING_NAME_WRONG;
                InputStringFlag=TRUE;
                InputStringEndFlag=FALSE;
            }
        }
    }
    else if (GameStatus == GAME_STATUS_NAME_WAIT) 
    {
			/* simply wait for action of the server after we send name */
	}
    else if (GameStatus == GAME_STATUS_PSWD)
    {
        map_transfer_flag = 0;
        /* we have a fininshed console input*/
        textwin_clearhistory();
        if (InputStringEscFlag)
            GameStatus = GAME_STATUS_LOGIN;
        else if (InputStringFlag == FALSE && InputStringEndFlag == TRUE)
        {
			int pwd_len = strlen(InputString);
            strncpy(cpl.password, InputString, 39);
			if (!GameStatusLogin && (pwd_len < 6 || pwd_len > 17))
			{
				dialog_login_warning_level = DIALOG_LOGIN_WARNING_PWD_SHORT;
	            cpl.password[0] = 0;   /* insanity 0 */
		        open_input_mode(12);
			}
			else if(!GameStatusLogin && !strcmp(cpl.name, cpl.password))
			{
				dialog_login_warning_level = DIALOG_LOGIN_WARNING_PWD_NAME;
	            cpl.password[0] = 0;   /* insanity 0 */
		        open_input_mode(12);
			}
			else
			{
	            cpl.password[39] = 0;   /* insanity 0 */
		        LOG(LOG_MSG, "Login: send password <*****>\n");
			    send_reply(cpl.password);
				GameStatus = GAME_STATUS_PSWD_WAIT;
			}
            /* now wait again for next server question*/
        }
    }
    else if (GameStatus == GAME_STATUS_PSWD_WAIT)
    {
			/* simply wait for action of the server after we send the password */
	}
    else if (GameStatus == GAME_STATUS_VERIFYPSWD)
    {
        map_transfer_flag = 0;
        /* we have a fininshed console input*/
        if (InputStringEscFlag)
            GameStatus = GAME_STATUS_LOGIN;
        else if (InputStringFlag == FALSE && InputStringEndFlag == TRUE)
        {
		    LOG(LOG_MSG, "Login: send verify password <*****>\n");
            send_reply(InputString);
            GameStatus = GAME_STATUS_VERIFYPSWD_WAIT;
            /* now wait again for next server question*/
        }
    }
    else if (GameStatus == GAME_STATUS_VERIFYPSWD_WAIT)
    {
	}
    else if (GameStatus == GAME_STATUS_WAITFORPLAY)
    {
        clear_map();
        map_draw_map_clear();
        map_udate_flag = 2;
        map_transfer_flag = 1;
    }
    else if (GameStatus == GAME_STATUS_NEW_CHAR)
    {
        map_transfer_flag = 0;
    }
    else if (GameStatus == GAME_STATUS_QUIT)
    {
        map_transfer_flag = 0;
    }
    return(TRUE);
}


/* load the skin & standard gfx */
void load_bitmaps(void)
{
    int i;

    for (i = 0; i <= BITMAP_INTRO; i++) /* add later better error handling here*/
        load_bitmap(i);
    CreateNewFont(Bitmaps[BITMAP_FONT1], &SystemFont, 16, 16, 1);
    CreateNewFont(Bitmaps[BITMAP_FONT1OUT], &SystemFontOut, 16, 16, 1);
    CreateNewFont(Bitmaps[BITMAP_FONT6x3OUT], &Font6x3Out, 16, 16, -1);
    CreateNewFont(Bitmaps[BITMAP_BIGFONT], &BigFont, 11, 16, 3);
}

Boolean load_bitmap(int index)
{
    char    buf[2048];
    uint32  flags   = 0;

    sprintf(buf, "%s%s", GetBitmapDirectory(), bitmap_name[index].name);

    if (bitmap_name[index].type == PIC_TYPE_PALETTE)
        flags |= SURFACE_FLAG_PALETTE;
    if (bitmap_name[index].type == PIC_TYPE_TRANS)
        flags |= SURFACE_FLAG_COLKEY_16M;

    Bitmaps[index] = sprite_load_file(buf, flags);
    if (!Bitmaps[index] || !Bitmaps[index]->bitmap)
    {
        LOG(LOG_MSG, "load_bitmap(): Can't load bitmap %s\n", buf);
        return(FALSE);
    }
    return(TRUE);
}

/* free the skin & standard gfx */
void free_bitmaps(void)
{
    int i;

    for (i = 0; i < BITMAP_MAX; i++)
        sprite_free_sprite(Bitmaps[i]);
}

void free_faces(void)
{
    int i;

    for (i = 0; i < MAX_FACE_TILES; i++)
    {
        if (FaceList[i].sprite)
        {
            sprite_free_sprite(FaceList[i].sprite);
            FaceList[i].sprite = NULL;
        }
        if (FaceList[i].name)
        {
            void   *tmp_free    = &FaceList[i].name;
            FreeMemory(tmp_free);
        }
        FaceList[i].flags = 0;
    }
}


void clear_metaserver_data(void)
{
    _server    *node, *tmp;
    void       *tmp_free;

    node = start_server;

    for (; node;)
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
    start_server = NULL;
    end_server = NULL;
    metaserver_start = 0;
    metaserver_sel = 0;
    metaserver_count = 0;
}

void add_metaserver_data(char *server, int port, int player, char *ver, char *desc1, char *desc2, char *desc3,
                         char *desc4)
{
    _server    *node;

    node = (_server *) _malloc(sizeof(_server), "add_metaserver_data(): add server struct");
    memset(node, 0, sizeof(_server));
    if (!start_server)
        start_server = node;
    if (!end_server)
        end_server = node;
    else
        end_server->next = node;
    end_server = node;

    node->player = player;
    node->port = port;
    node->nameip = _malloc(strlen(server) + 1, "add_metaserver_data(): nameip string");
    strcpy(node->nameip, server);
    node->version = _malloc(strlen(ver) + 1, "add_metaserver_data(): version string");
    strcpy(node->version, ver);
    node->desc1 = _malloc(strlen(desc1) + 1, "add_metaserver_data(): desc string");
    strcpy(node->desc1, desc1);
    node->desc2 = _malloc(strlen(desc2) + 1, "add_metaserver_data(): desc string");
    strcpy(node->desc2, desc2);
    node->desc3 = _malloc(strlen(desc3) + 1, "add_metaserver_data(): desc string");
    strcpy(node->desc3, desc3);
    node->desc4 = _malloc(strlen(desc4) + 1, "add_metaserver_data(): desc string");
    strcpy(node->desc4, desc4);
}

static void count_meta_server(void)
{
    _server    *node;

    node = start_server;
    for (metaserver_count = 0; node; metaserver_count++)
        node = node->next;
}

void get_meta_server_data(int num, char *server, int *port)
{
    _server    *node;
    int         i;

    node = start_server;
    for (i = 0; node; i++)
    {
        if (i == num)
        {
            strcpy(server, node->nameip);
            *port = node->port;
            return;
        }
        node = node->next;
    }
}



void reset_input_mode(void)
{
    InputString[0] = 0;
    InputCount = 0;
    HistoryPos = 0;
    InputHistory[0][0] = 0;
    CurrentCursorPos = 0;
    InputStringFlag = FALSE;
    InputStringEndFlag = FALSE;
    InputStringEscFlag = FALSE;
}

void open_input_mode(int maxchar)
{
    reset_input_mode();
    InputMax = maxchar;
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
    if (cpl.input_mode != INPUT_MODE_NUMBER)
        cpl.inventory_win = IWIN_BELOW;
    InputStringFlag = TRUE;
    /* if true keyboard and game is in input str mode*/
}


static void play_action_sounds(void)
{
    if (!cpl.stats.food)
    {
        sound_play_one_repeat(SOUND_WARN_FOOD, SPECIAL_SOUND_FOOD);
    }
    if (cpl.warn_hp)
    {
        if (cpl.warn_hp == 2) /* more as 10% damage */
            sound_play_effect(SOUND_WARN_HP2, 0, 0, 100);
        else
            sound_play_effect(SOUND_WARN_HP, 0, 0, 100);
        cpl.warn_hp = 0;
    }
    if (cpl.warn_statdown)
    {
        sound_play_one_repeat(SOUND_WARN_STATDOWN, SPECIAL_SOUND_STATDOWN);
        cpl.warn_statdown = FALSE;
    }
    if (cpl.warn_statup)
    {
        sound_play_one_repeat(SOUND_WARN_STATUP, SPECIAL_SOUND_STATUP);
        cpl.warn_statup = FALSE;
    }
    if (cpl.warn_drain)
    {
        sound_play_one_repeat(SOUND_WARN_DRAIN, SPECIAL_SOUND_DRAIN);
        cpl.warn_drain = FALSE;
    }
}

void list_vid_modes(void)
{
    const SDL_VideoInfo    *vinfo   = NULL;
    SDL_Rect               **modes;
    int                     i;

    LOG(LOG_MSG, "List Video Modes\n");
    /* Get available fullscreen/hardware modes */
    modes = SDL_ListModes(NULL, SDL_HWACCEL);

    /* Check is there are any modes available */
    if (modes == (SDL_Rect * *) 0)
    {
        LOG(LOG_MSG, "No modes available!\n");
        exit(-1);
    }

    /* Check if or resolution is restricted */
    if (modes == (SDL_Rect * *) - 1)
    {
        LOG(LOG_MSG, "All resolutions available.\n");
    }
    else
    {
        /* Print valid modes */
        LOG(LOG_MSG, "Available Modes\n");
        for (i = 0; modes[i]; ++i)
            LOG(LOG_MSG, "  %d x %d\n", modes[i]->w, modes[i]->h);
    }


    vinfo = SDL_GetVideoInfo();
    if(!vinfo)
        return;
    LOG(LOG_MSG, "VideoInfo: hardware surfaces? %s\n", vinfo->hw_available ? "yes" : "no");
    LOG(LOG_MSG, "VideoInfo: windows manager? %s\n", vinfo->wm_available ? "yes" : "no");
    LOG(LOG_MSG, "VideoInfo: hw to hw blit? %s\n", vinfo->blit_hw ? "yes" : "no");
    LOG(LOG_MSG, "VideoInfo: hw to hw ckey blit? %s\n", vinfo->blit_hw_CC ? "yes" : "no");
    LOG(LOG_MSG, "VideoInfo: hw to hw alpha blit? %s\n", vinfo->blit_hw_A ? "yes" : "no");
    LOG(LOG_MSG, "VideoInfo: sw to hw blit? %s\n", vinfo->blit_sw ? "yes" : "no");
    LOG(LOG_MSG, "VideoInfo: sw to hw ckey blit? %s\n", vinfo->blit_sw_CC ? "yes" : "no");
    LOG(LOG_MSG, "VideoInfo: sw to hw alpha blit? %s\n", vinfo->blit_sw_A ? "yes" : "no");
    LOG(LOG_MSG, "VideoInfo: color fill? %s\n", vinfo->blit_fill ? "yes" : "no");
    LOG(LOG_MSG, "VideoInfo: video memory: %dKB\n", vinfo->video_mem);
}

static void show_option(int mark, int x, int y)
{
    int     index = 0, x1, y1 = 0, x2, y2 = 0;
    _BLTFX  bltfx;

    bltfx.alpha = 128;
    bltfx.flags = BLTFX_FLAG_SRCALPHA;
    sprite_blt(Bitmaps[BITMAP_OPTIONS_ALPHA], x - Bitmaps[BITMAP_OPTIONS_ALPHA]->bitmap->w / 2, y, NULL, &bltfx);
    sprite_blt(Bitmaps[BITMAP_OPTIONS_HEAD], x - Bitmaps[BITMAP_OPTIONS_HEAD]->bitmap->w / 2, y, NULL, NULL);
    sprite_blt(Bitmaps[BITMAP_OPTIONS_KEYS], x - Bitmaps[BITMAP_OPTIONS_KEYS]->bitmap->w / 2, y + 100, NULL, NULL);
    sprite_blt(Bitmaps[BITMAP_OPTIONS_SETTINGS], x - Bitmaps[BITMAP_OPTIONS_SETTINGS]->bitmap->w / 2, y + 165, NULL,
               NULL);
    sprite_blt(Bitmaps[BITMAP_OPTIONS_LOGOUT], x - Bitmaps[BITMAP_OPTIONS_LOGOUT]->bitmap->w / 2, y + 235, NULL, NULL);
    sprite_blt(Bitmaps[BITMAP_OPTIONS_BACK], x - Bitmaps[BITMAP_OPTIONS_BACK]->bitmap->w / 2, y + 305, NULL, NULL);

    if (esc_menu_index == ESC_MENU_KEYS)
    {
        index = BITMAP_OPTIONS_KEYS;
        y1 = y2 = y + 105;
    }
    if (esc_menu_index == ESC_MENU_SETTINGS)
    {
        index = BITMAP_OPTIONS_SETTINGS;
        y1 = y2 = y + 170;
    }
    if (esc_menu_index == ESC_MENU_LOGOUT)
    {
        index = BITMAP_OPTIONS_LOGOUT;
        y1 = y2 = y + 244;
    }
    if (esc_menu_index == ESC_MENU_BACK)
    {
        index = BITMAP_OPTIONS_BACK;
        y1 = y2 = y + 310;
    }

    x1 = x - Bitmaps[index]->bitmap->w / 2 - 6;
    x2 = x + Bitmaps[index]->bitmap->w / 2 + 6;

    sprite_blt(Bitmaps[BITMAP_OPTIONS_MARK_LEFT], x1 - Bitmaps[BITMAP_OPTIONS_MARK_LEFT]->bitmap->w, y1, NULL, NULL);
    sprite_blt(Bitmaps[BITMAP_OPTIONS_MARK_RIGHT], x2, y2, NULL, NULL);
}

int main(int argc, char *argv[])
{
    char            buf[256];
    int             x, y;
    int             drag;
    uint32          anim_tick;
    Uint32          videoflags;
    int             i, done = 0, FrameCount = 0;
    fd_set          tmp_read, tmp_write, tmp_exceptions;
    int             pollret, maxfd;
    struct timeval  timeout;

    init_game_data();
    while (argc > 1)
    {
        --argc;
        if (strcmp(argv[argc - 1], "-port") == 0)
        {
            argServerPort = atoi(argv[argc]);
            --argc;
        }
        else if (strcmp(argv[argc - 1], "-server") == 0)
        {
            strcpy(argServerName, argv[argc]);
            --argc;
        }
        else if (strcmp(argv[argc], "-nometa") == 0)
        {
            options.no_meta = 1;
        }
        else if (strcmp(argv[argc], "-key") == 0)
        {
            KeyScanFlag = TRUE;
        }
        else
        {
            char    tmp[1024];
            sprintf(tmp, "Usage: %s [-server <name>] [-port <num>]\n", argv[0]);
            LOG(LOG_MSG, tmp);
            fprintf(stderr, tmp);
            exit(1);
        }
    }

#if defined( __LINUX)
    LOG(LOG_MSG, "**** NOTE ****\n");
    LOG(LOG_MSG, "With sound enabled SDL will throw a parachute\n");
    LOG(LOG_MSG, "when the soundcard is disabled or not installed.\n");
    LOG(LOG_MSG, "Try then to start the client with 'SDL_AUDIODRIVER=null ./daimonin'\n");
    LOG(LOG_MSG, "Read the README_LINUX.txt file for more information.\n");
#endif
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) < 0)
    {
        LOG(LOG_ERROR, "Couldn't initialize SDL: %s\n", SDL_GetError());
        if (strstr(SDL_GetError(), "console terminal"))
        {
            LOG(LOG_MSG, "**** NOTE ****\n");
            LOG(LOG_MSG, "Seems that you are trying to run daimonin in framebuffer.\n");
            LOG(LOG_MSG, "We suggest to reconfigure your x-window-system.\n");
            LOG(LOG_MSG, "You should be able to run daimonin as root,\n");
            LOG(LOG_MSG, "but for security reasons - this is not a good idea!\n");
        }
        exit(1);
    }
    atexit(SDL_Quit);
    SYSTEM_Start(); /* start the system AFTER start SDL */
    list_vid_modes();
#ifdef INSTALL_OPENGL
    if (options.use_gl)
    {
        const SDL_VideoInfo    *info    = NULL;
        info = SDL_GetVideoInfo();

        SDL_GL_LoadLibrary(NULL);

        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

        options.used_video_bpp = info->vfmt->BitsPerPixel;
        LOG(LOG_MSG, "Using OpenGL: bpp:%d\n", options.used_video_bpp);
        videoflags = SDL_OPENGL;
    }
#endif

    videoflags = get_video_flags();
    options.used_video_bpp = 16;//2^(options.video_bpp+3);
    if (!options.fullscreen_flag)
    {
        if (options.auto_bpp_flag)
        {
            const SDL_VideoInfo    *info    = NULL;
            info = SDL_GetVideoInfo();
            options.used_video_bpp = info->vfmt->BitsPerPixel;
        }
    }
    if ((ScreenSurface = SDL_SetVideoMode(SCREEN_XLEN, SCREEN_YLEN, options.used_video_bpp, videoflags)) == NULL)
    {
        LOG(LOG_ERROR, "Couldn't set 800x600x%d video mode: %s\n", options.used_video_bpp, SDL_GetError());
        exit(2);
    }
    else
    {
        const SDL_VideoInfo    *info    = NULL;
        info = SDL_GetVideoInfo();
        options.real_video_bpp = info->vfmt->BitsPerPixel;
    }

    /* 60, 70*/
    sdl_dgreen = SDL_MapRGB(ScreenSurface->format, 0x00, 0x80, 0x00);
    sdl_gray1 = SDL_MapRGB(ScreenSurface->format, 0x45, 0x45, 0x45);
    sdl_gray2 = SDL_MapRGB(ScreenSurface->format, 0x55, 0x55, 0x55);

    sdl_gray3 = SDL_MapRGB(ScreenSurface->format, 0x55, 0x55, 0x55);
    sdl_gray4 = SDL_MapRGB(ScreenSurface->format, 0x60, 0x60, 0x60);

    sdl_blue1 = SDL_MapRGB(ScreenSurface->format, 0x00, 0x00, 0xef);

    SDL_EnableUNICODE(1);
    load_bitmaps();
    show_intro("start sound system");
    sound_init();
    show_intro("load sounds");
    sound_loadall();
    show_intro("load bitmaps");
    for (i = 5; i < BITMAP_MAX; i++) /* add later better error handling here*/
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
    sound_play_music("orchestral.ogg", options.music_volume, 0, -1, 0, MUSIC_MODE_DIRECT);

    while (1)
    {
        SDL_Event   event;
        SDL_PollEvent(&event);

        if (event.type == SDL_QUIT)
        {
            sound_freeall();
            sound_deinit();
            free_bitmaps();
            SYSTEM_End();
            return(0);
        }
        if (event.type == SDL_KEYUP || event.type == SDL_KEYDOWN || event.type == SDL_MOUSEBUTTONDOWN)
        {
            reset_keys();
            break;
        }

        SDL_Delay(25);      /* force the thread to sleep */
    }; /* wait for keypress */

    sprintf(buf, "Welcome to Daimonin v%s", PACKAGE_VERSION);
    draw_info(buf, COLOR_HGOLD);
    draw_info("init network...", COLOR_GREEN);
    if (!SOCKET_InitSocket()) /* log in function*/
        exit(1);

    maxfd = csocket.fd + 1;
    LastTick = tmpGameTick = anim_tick = SDL_GetTicks();
    GameTicksSec = 0;       /* ticks since this second frame in ms */

    /* the one and only main loop */
    /* TODO: frame update can be optimized. It uses some cpu time because it
    * draws every loop some parts.
    */
    while (!done)
    {
        done = Event_PollInputDevice();

#ifdef INSTALL_SOUND
        if (music_global_fade)
            sound_fadeout_music(music_new.flag);
#endif

#if (0) /* unused. Toggle is still buggy in SDL */
        if (ToggleScreenFlag)
        {
            uint32  flags, tf;

            if (options.fullscreen)
                options.fullscreen = FALSE;
            else
                options.fullscreen = TRUE;
            tf = flags = get_video_flags();
            attempt_fullscreen_toggle(&ScreenSurface, &flags);
            ToggleScreenFlag = FALSE;
        }
#endif


        /* get our ticks */
        if ((LastTick - tmpGameTick) > 1000)
        {
            tmpGameTick = LastTick;
            FrameCount = 0;
            GameTicksSec = 0;
        }
        GameTicksSec = LastTick - tmpGameTick;

        if (GameStatus > GAME_STATUS_CONNECT)
        {
            if (csocket.fd == SOCKET_NO)
            {
                /* connection closed, so we go back to INIT here*/
                if (GameStatus == GAME_STATUS_PLAY)
                {
                    GameStatus = GAME_STATUS_INIT;
                }
                else
                    GameStatus = GAME_STATUS_START;
            }
            else
            {
                FD_ZERO(&tmp_read);
                FD_ZERO(&tmp_write);
                FD_ZERO(&tmp_exceptions);

                FD_SET((unsigned int) csocket.fd, &tmp_exceptions);
                FD_SET((unsigned int) csocket.fd, &tmp_read);
                FD_SET((unsigned int) csocket.fd, &tmp_write);


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
                timeout.tv_usec = 0;
                /* main poll point for the socket */
                if ((pollret = select(maxfd, &tmp_read, &tmp_write, &tmp_exceptions, &timeout)) == -1)
                    LOG(LOG_MSG, "Got errno %d on selectcall.\n", SOCKET_GetError());
                else if (FD_ISSET(csocket.fd, &tmp_read))
                    DoClient(&csocket);
                request_face(0, 1); /* flush face request buffer */
            }
        }

        /* show help system or game screen */
        if (show_help_screen)
        {
            SDL_FillRect(ScreenSurface, NULL, 0);
            if (show_help_screen == 1)
                sprite_blt(Bitmaps[BITMAP_HELP1], 0, 0, NULL, NULL);
            else if (show_help_screen == 2)
                sprite_blt(Bitmaps[BITMAP_HELP2], 0, 0, NULL, NULL);
        }
        else
        {
            if (GameStatus == GAME_STATUS_PLAY)
            {
                if (LastTick - anim_tick > 110)
                {
                    anim_tick = LastTick;
                    animate_objects();
                    map_udate_flag = 2;
                }
                play_action_sounds();
            }

            if (map_udate_flag > 0)
            {
                if (GameStatus != GAME_STATUS_PLAY)
                    SDL_FillRect(ScreenSurface, NULL, 0);
                sprite_blt(Bitmaps[BITMAP_BUFFSPOT], 518, 143, NULL, NULL);
                sprite_blt(Bitmaps[BITMAP_TEXTSPOT], 226, 109, NULL, NULL);
                sprite_blt(Bitmaps[BITMAP_PDOLL2_SPOT], 0, 194, NULL, NULL);
                sprite_blt(Bitmaps[BITMAP_CLEAR_SPOT], 0, 306, NULL, NULL);
                if (GameStatus == GAME_STATUS_PLAY)
                {
                    static int  gfx_toggle  = 0;
                    map_draw_map();
                    play_anims(0, 0); /* over the map */

                    /* draw warning-icons above player */
                    if ((gfx_toggle++ & 63) < 25)
                    {
                        if (options.warning_hp
                         && ((float) cpl.stats.hp / (float) cpl.stats.maxhp) * 100 <= options.warning_hp)
                            sprite_blt(Bitmaps[BITMAP_WARN_HP], 393, 298, NULL, NULL);
                    }
                    else
                    {
                        if (options.warning_food && ((float) cpl.stats.food / 1000.0f) * 100 <= options.warning_food) /* low food */
                            sprite_blt(Bitmaps[BITMAP_WARN_FOOD], 390, 294, NULL, NULL);
                    }
                }
                show_quickslots(SKIN_POS_QUICKSLOT_X, SKIN_POS_QUICKSLOT_Y);
                sprite_blt(Bitmaps[BITMAP_BORDER1], 0, 351, NULL, NULL);
                sprite_blt(Bitmaps[BITMAP_BORDER2], 144, 423, NULL, NULL);
                sprite_blt(Bitmaps[BITMAP_BORDER3], 264, 483, NULL, NULL);
                sprite_blt(Bitmaps[BITMAP_BORDER4], 402, 481, NULL, NULL);
                sprite_blt(Bitmaps[BITMAP_BORDER5], 539, 408, NULL, NULL);
                sprite_blt(Bitmaps[BITMAP_BORDER6], 686, 351, NULL, NULL);
                sprite_blt(Bitmaps[BITMAP_PANEL_P1], 686, 408, NULL, NULL);
                sprite_blt(Bitmaps[BITMAP_TARGET_SPOT], 0, 423, NULL, NULL);
                sprite_blt(Bitmaps[BITMAP_GROUP_SPOT], 0, 482, NULL, NULL);
                StringBlt(ScreenSurface, &Font6x3Out, "Group", 5, 525, COLOR_HGOLD, NULL, NULL);
                sprite_blt(Bitmaps[BITMAP_FLINE], 1, 306, NULL, NULL);
                sprite_blt(Bitmaps[BITMAP_BELOW], 264, 550, NULL, NULL);
                cpl.container = NULL; /* this will be set right on the fly in get_inventory_data() */
                if (GameStatus == GAME_STATUS_PLAY)
                {
                    cpl.win_inv_tag = get_inventory_data(cpl.ob, &cpl.win_inv_ctag, &cpl.win_inv_slot,
                                                         &cpl.win_inv_start, &cpl.win_inv_count, INVITEMXLEN,
                                                         INVITEMYLEN);
                    cpl.real_weight = cpl.window_weight;
                    StringBlt(ScreenSurface, &SystemFont, "Carry", 125, 470, COLOR_HGOLD, NULL, NULL);
                    sprintf(buf, "%4.3f kg", cpl.real_weight);
                    StringBlt(ScreenSurface, &SystemFont, buf, 125 + 35, 470, COLOR_DEFAULT, NULL, NULL);
                    StringBlt(ScreenSurface, &SystemFont, "Limit", 125, 481, COLOR_HGOLD, NULL, NULL);
                    sprintf(buf, "%4.3f kg", (float) (cpl.weight_limit / 1000));
                    StringBlt(ScreenSurface, &SystemFont, buf, 125 + 35, 481, COLOR_DEFAULT, NULL, NULL);
                    cpl.win_below_tag = get_inventory_data(cpl.below, &cpl.win_below_ctag, &cpl.win_below_slot,
                                                           &cpl.win_below_start, &cpl.win_below_count, INVITEMBELOWXLEN,
                                                           INVITEMBELOWYLEN);
                    show_target(4, 498);
                    show_below_window(cpl.below, 264, 565);
                    show_group(2, 525);
                    StringBlt(ScreenSurface, &Font6x3Out, "(SHIFT for inventory)", 28, 478, COLOR_DEFAULT, NULL, NULL);
                    show_inventory_window(6, 472);
                    show_media(798, 171);
                }
                show_range(3, 403);
                sprite_blt(Bitmaps[BITMAP_PRAY], 92, 412, NULL, NULL);

                if (esc_menu_flag == TRUE
                 || (options.use_TextwinAlpha && (txtwin[TW_MSG].size + txtwin[TW_CHAT].size) > 9))
                    map_udate_flag = 1;
                else if (!options.force_redraw)
                {
                    if (options.doublebuf_flag)
                        map_udate_flag--;
                    else
                        map_udate_flag = 0;
                }
            } /* map update */

            show_player_stats(226, 0);
            show_resist(500, 0);
            textwin_show(539, 485);
            if (GameStatus == GAME_STATUS_PLAY)
            {
                SDL_Rect    tmp_rect;
                tmp_rect.w = 275;
                StringBlt(ScreenSurface, &SystemFont, MapData.name, 229, 109, COLOR_DEFAULT, &tmp_rect, NULL);

                if (cpl.input_mode == INPUT_MODE_CONSOLE)
                    do_console(546, 586);
                else if (cpl.input_mode == INPUT_MODE_NUMBER)
                    do_number(100, 505);
                else if (cpl.input_mode == INPUT_MODE_GETKEY)
                    do_keybind_input();
                else if (cpl.input_mode == INPUT_MODE_NPCDIALOG)
                    do_npcdialog_input();
            }
            else
            {
                if (GameStatus == GAME_STATUS_WAITFORPLAY)
                    StringBlt(ScreenSurface, &SystemFont, "Transfer Character to Map...", 300, 300, COLOR_DEFAULT, NULL,
                              NULL);
            }
        }

        /* if not connected, walk through connection chain and/or wait for action */
        if (GameStatus != GAME_STATUS_PLAY)
        {
            if (!game_status_chain())
            {
                LOG(LOG_ERROR, "Error connecting: GStatus: %d  SocketError: %d\n", GameStatus, SOCKET_GetError());
                exit(1);
            }
        }

        show_player_doll(0, 0);
        show_player_data(0, 0);

        /* show main-option menu */
        if (esc_menu_flag == TRUE)
            show_option(esc_menu_index, 400, 130);

        if (map_transfer_flag)
            StringBlt(ScreenSurface, &SystemFont, "Transfer Character to Map...", 300, 300, COLOR_DEFAULT, NULL, NULL);

        /* show the current dragged item */
        if (cpl.menustatus == MENU_NO && (drag = draggingInvItem(DRAG_GET_STATUS)))
        {
            item   *Item = NULL;
            if (drag == DRAG_IWIN_INV)
                Item = locate_item(cpl.win_inv_tag);
            else if (drag == DRAG_IWIN_BELOW)
                Item = locate_item(cpl.win_below_tag);
            else if (drag == DRAG_QUICKSLOT)
                Item = locate_item(cpl.win_quick_tag);
            else if (drag == DRAG_PDOLL)
                Item = locate_item(cpl.win_pdoll_tag);
            /*  else Item = locate_item(cpl.win_quick_tag); */
            if(Item)
            {
                SDL_GetMouseState(&x, &y);
                if (drag == DRAG_QUICKSLOT_SPELL)
                    sprite_blt(spell_list[quick_slots[cpl.win_quick_tag].spell.groupNr].entry[quick_slots[cpl.win_quick_tag].spell.classNr][quick_slots[cpl.win_quick_tag].spell.spellNr].icon,
                            x, y, NULL, NULL);
                else
                    blt_inv_item_centered(Item, x, y);
            }
        }

        /* we have a non-standard mouse-pointer (win-size changer, etc.) */
        if (cursor_type)
        {
            SDL_Rect    rec;
            SDL_GetMouseState(&x, &y);
            rec.w = 14;
            rec.h = 1;
            rec.x = x - 7;
            rec.y = y - 2;
            SDL_FillRect(ScreenSurface, &rec, -1);
            rec.y = y - 5;
            SDL_FillRect(ScreenSurface, &rec, -1);
        }
        if (GameStatus < GAME_STATUS_REQUEST_FILES)
            show_meta_server(start_server, metaserver_start, metaserver_sel);
        else if (GameStatus >= GAME_STATUS_REQUEST_FILES && GameStatus < GAME_STATUS_NEW_CHAR)
            show_login_server();
        else if (GameStatus == GAME_STATUS_NEW_CHAR)
            cpl.menustatus = MENU_CREATE;

        /* show all kind of the big dialog windows */
        show_menu();
        /* show all kind of the small dialog windows */
        /* show_requester(); */

        /* we count always last frame*/
        FrameCount++;
        LastTick = SDL_GetTicks();
        /* print frame rate*/
/*
        {
            SDL_Rect    rec;
            sprintf(buf, " valid:%d mxy: %d,%d down:%d (%d,%d) up:%d (%d,%d)",global_buttons.valid,
				global_buttons.mx,global_buttons.my,
				global_buttons.down,global_buttons.mx_down, global_buttons.my_down, 
				global_buttons.click, global_buttons.mx_up, global_buttons.my_up);
	
                rec.x = 228;
                rec.y = 122;
                rec.h = 14;
                rec.w = 225;
                StringBlt(ScreenSurface, &SystemFont, buf, rec.x, rec.y, COLOR_DEFAULT, NULL, NULL);
		}
*/
        if (options.show_frame && cpl.menustatus == MENU_NO)
        {
            SDL_Rect    rec;
            sprintf(buf, "fps %d (%d) (%d %d) %s%s%s%s%s%s%s%s%s%s %d %d",
                    ((LastTick - tmpGameTick) / FrameCount) ? 1000 / ((LastTick - tmpGameTick) / FrameCount) : 0,
                    (LastTick - tmpGameTick) / FrameCount, GameStatus, cpl.input_mode,
                    ScreenSurface->flags & SDL_FULLSCREEN ? "F" : "", ScreenSurface->flags & SDL_HWSURFACE ? "H" : "S",
                    ScreenSurface->flags & SDL_HWACCEL ? "A" : "", ScreenSurface->flags & SDL_DOUBLEBUF ? "D" : "",
                    ScreenSurface->flags & SDL_ASYNCBLIT ? "a" : "", ScreenSurface->flags & SDL_ANYFORMAT ? "f" : "",
                    ScreenSurface->flags & SDL_HWPALETTE ? "P" : "", options.rleaccel_flag ? "R" : "",
                    options.force_redraw ? "r" : "", options.use_rect ? "u" : "", options.used_video_bpp,
                    options.real_video_bpp);
            if (GameStatus == GAME_STATUS_PLAY)
            {
                rec.x = 228;
                rec.y = 122;
                rec.h = 14;
                rec.w = 225;
                StringBlt(ScreenSurface, &SystemFont, buf, rec.x, rec.y, COLOR_DEFAULT, NULL, NULL);
            }
        }

        flip_screen();
        if (options.limit_speed)
            SDL_Delay(options.sleep);       /* force the thread to sleep */
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
    char    buf[256];

    sprite_blt(Bitmaps[BITMAP_INTRO], 0, 0, NULL, NULL);
    if (text)
        StringBlt(ScreenSurface, &SystemFont, text, 370, 295, COLOR_DEFAULT, NULL, NULL);
    else
        StringBlt(ScreenSurface, &SystemFont, "** Press Key **", 375, 585, COLOR_DEFAULT, NULL, NULL);

    sprintf(buf, "v. %s", PACKAGE_VERSION);
    StringBlt(ScreenSurface, &SystemFont, buf, 10, 585, COLOR_DEFAULT, NULL, NULL);
    flip_screen();
}


static void flip_screen(void)
{
#ifdef INSTALL_OPENGL
    if (options.use_gl)
        SDL_GL_SwapBuffers();
    else
    {
#endif

        if (options.use_rect)
            SDL_UpdateRect(ScreenSurface, 0, 0, SCREEN_XLEN, SCREEN_YLEN);
        else
            SDL_Flip(ScreenSurface);
#ifdef INSTALL_OPENGL
    }
#endif
}
