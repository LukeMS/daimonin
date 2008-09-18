/*
    Daimonin SDL client, a client program for the Daimonin MMORPG.

    Copyright (C) 2008 Michael Toennies

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

/* This file deals with higher level functions for sending informations 
 * and commands to the server. Most low level functions are in socket.c
 */

#include <include.h>
#include <stdio.h>

ClientSocket    csocket;

/* helper array to cast a key num input to a server dir value */
static int move_dir[] = {0,6,5,4,7,0,3,8,1,2};

/* helper functions for working with binary parms for the socklist */
static inline void SockList_AddShort(SockList *const sl, const uint16 data)
{
    if(sl->buf)
    	*((uint16 *)(sl->buf+sl->len)) = adjust_endian_int16(data);
    else
        *((uint16 *)(sl->defbuf+sl->len)) = adjust_endian_int16(data);
	sl->len+=2;
}
static inline void SockList_AddInt(SockList *const sl, const uint32 data)
{
    if(sl->buf)
    	*((uint32 *)(sl->buf+sl->len)) = adjust_endian_int32(data);
    else
        *((uint32 *)(sl->defbuf+sl->len)) = adjust_endian_int32(data);
	sl->len+=4;
}
static inline void SockList_AddBuffer(SockList *const sl, const char *const buf, const int len)
{
    if(sl->buf)
        memcpy(sl->buf+sl->len,buf,len);
    else
        memcpy(sl->defbuf+sl->len,buf,len);
    sl->len+=len;
}

static inline void SockList_AddString(SockList *const sl, const char *const buf)
{
    int len = strlen(buf);

    if(sl->buf)
    {
        memcpy(sl->buf+sl->len,buf,len);
        *(sl->buf+sl->len+len++) = 0; /* ensure the string is send with 0 end marker */
    }
    else
    {
        memcpy(sl->defbuf+sl->len,buf,len);
        *(sl->defbuf+sl->len+len++) = 0;
    }
    sl->len+=len;


}

/* Splits command at the next #,
* returning a pointer to the occurrence (which is overwritten with \0 first) or
* NULL if no next multicommand is found or command is chat, etc.
*/
static char *BreakMulticommand(const char *command)
{
    char *c = NULL;
    /* Only look for a multicommand if the command is not one of these:
    */
    if (!(!strnicmp(command, "/tell", 5) || !strnicmp(command, "/say", 4) || !strnicmp(command, "/reply", 6) || !strnicmp(command, "/gsay", 5) || !strnicmp(command, "/shout", 6) || !strnicmp(command, "/talk", 5)
#ifdef USE_CHANNELS
        || (*command == '-') || !strnicmp(command, "/channel", 8)
#endif
        || !strnicmp(command, "/create", 7)))
    {
        if ((c = strchr(command, '#'))) /* multicommand separator '#' */
            *c = '\0';
    }
    return c;
}

/* send_game_command() will send a higher level game command like /tell, /say or
 * other "slash" text commants. Usually, this kind of commands are typed in 
 * console or are bound to macros. 
 * The underlaying protocol command is CLIENT_CMD_GENERIC, which means
 * its a command holding another command.
 * For realtime or system commands, commands with binary params and such,
 * not a slash command should be used but a new protocol command.
 * Only that commands hold real binary params and can be pre-processed
 * by the server protocol functions.
 */
void send_game_command(const char *command)
{
    SockList    sl;
    char *token, cmd[HUGE_BUF];

    /* Copy a normalized (leading, trailing, and excess inline whitespace-
    * stripped) command to cmd:
    */
    strcpy(cmd, normalize_string(command));

    /* Now go through cmd, possibly separating multicommands.
    * Each command (before separation) is pointed to by token:
    */
    token = cmd;
    while (token != NULL && *token)
    {
        char *end;

    #ifdef USE_CHANNELS
        if (*token != '/' && *token != '-') /* if not a command ... its chat  (- is for channel system)*/
    #else
        if (*token != '/')
    #endif
        {
            char buf[MAX_BUF];

            sprintf(buf, "/say %s", token);
            strcpy(token, buf);
        }

        end = BreakMulticommand(token);
        if (!client_command_check(token))
        {
            /* Nasty hack. Treat /talk as a special case: lowercase it and
            * print it to the message window as Topic: foo. -- Smacky 20071210
            */
            if (!strnicmp(token, "/talk", 5))
            {
                int c;
                for (c = 0; *(token + c) != '\0'; c++)
                    *(token + c) = tolower(*(token + c));
                draw_info_format(COLOR_DGOLD, "Topic: %s", token + 6);
            }

            /* put the slash command inside the protocol command GENERIC */
            SockList_INIT(&sl, NULL);
            SockList_COMMAND(&sl, CLIENT_CMD_GENERIC, SEND_CMD_FLAG_STRING);
            SockList_AddBuffer(&sl, token+1, strlen(token+1)); /* with +1 we remove the leading '/' */
            send_socklist_binary(&sl); /* and kick it in send queue */
        }
        if (end != NULL)
            token = end + 1;
        else
            token = NULL;
    }
}

/* help function for receiving faces (pictures)
* NOTE: This feature must be enabled from the server
*/
static void face_flag_extension(int pnum, char *buf)
{
    char   *stemp;

    FaceList[pnum].flags = FACE_FLAG_NO;
    /* check for the "double"/"up" tag in the picture name */
    if ((stemp = strstr(buf, ".d")))
        FaceList[pnum].flags |= FACE_FLAG_DOUBLE;
    else if ((stemp = strstr(buf, ".u")))
        FaceList[pnum].flags |= FACE_FLAG_UP;

    /* Now the facing stuff: if some tag was there, lets grap the facing info */
    if (FaceList[pnum].flags && stemp)
    {
        int tc;
        for (tc = 0; tc < 4; tc++)
        {
            if (!*(stemp + tc)) /* has the string a '0' before our anim tags */
                goto finish_face_cmd_j1;
        }
        /* lets set the right flags for the tags */
        if (((FaceList[pnum].flags & FACE_FLAG_UP) && *(stemp + tc) == '5') || *(stemp + tc) == '1')
            FaceList[pnum].flags |= FACE_FLAG_D1;
        else if (*(stemp + tc) == '3')
            FaceList[pnum].flags |= FACE_FLAG_D3;
        else if (*(stemp + tc) == '4' || *(stemp + tc) == '8' || *(stemp + tc) == '0')
            FaceList[pnum].flags |= (FACE_FLAG_D3 | FACE_FLAG_D1);
    }
finish_face_cmd_j1: /* error jump from for() */
    return;
}


void finish_face_cmd(int pnum, uint32 checksum, char *face)
{
    SockList        sl;
    char            buf[2048];
    FILE           *stream;
    struct stat     statbuf;
    int             len;
    static uint32   newsum  = 0;
    unsigned char * data;
    void   *tmp_free;

    /* first, check our memory... perhaps we have it loaded */
    /*LOG(LOG_MSG,"FACE: %s (->%s)\n", face,FaceList[pnum].name);*/
    if (FaceList[pnum].name) /* loaded OR requested...hm, no double request check yet*/
    {
        /* lets check the name and checksum and sprite. ONLY if all is
         * ok, we stay with it
         */
        if (!strcmp(face, FaceList[pnum].name) && checksum == FaceList[pnum].checksum && FaceList[pnum].sprite)
        {
            face_flag_extension(pnum, FaceList[pnum].name);
            return;
        }
        /* ok, some is different.
         * no big work, clear face data and lets go on
         *  all this check for invalid ptr, so fire it up
        */
        tmp_free = &FaceList[pnum].name;
        FreeMemory(tmp_free);
        sprite_free_sprite(FaceList[pnum].sprite);
    }

    /* first, safe face data: name & checksum */
    sprintf(buf, "%s.png", face);
    FaceList[pnum].name = (char *) _malloc(strlen(buf) + 1, "finish_face_cmd(): FaceList name");
    strcpy(FaceList[pnum].name, buf);

    FaceList[pnum].checksum = checksum;

    /* Check private cache first */
    sprintf(buf, "%s%s", GetCacheDirectory(), FaceList[pnum].name);
    if ((stream = fopen_wrapper(buf, "rb")) != NULL)
    {
        fstat(fileno(stream), &statbuf);
        len = (int) statbuf.st_size;
        data = malloc(len);
        len = fread(data, 1, len, stream);
        fclose(stream);
        newsum = 0;

        /* something is wrong... now unlink the file and let it reload then possible and needed */
        if (len <= 0) 
            checksum = 1; /* mark as wrong */
        else /* lets go for the checksum check*/
            newsum = crc32(1L, data, len);

        free(data);

        if (newsum == checksum)
        {
            FaceList[pnum].sprite = sprite_tryload_file(buf, 0, NULL);
            if (FaceList[pnum].sprite)
            {
                face_flag_extension(pnum, buf);
                return; /* found and loaded!*/
            }
        }
        unlink(buf); /* forget this face - unlink it and request a new one! */
    }

    LOG(LOG_MSG,"FACE: call server for %s (%d)\n", face, pnum);
    face_flag_extension(pnum, buf);

    SockList_INIT(&sl, NULL);
    SockList_COMMAND(&sl, CLIENT_CMD_FACE, 0);
    SockList_AddShort(&sl, pnum);
    send_socklist_binary(&sl);

}


/* we have stored this picture in daimonin.p0 - load it from it! */
static int load_picture_from_pack(int num)
{
    FILE       *stream;
    char       *pbuf;
    SDL_RWops  *rwop;

    if ((stream = fopen_wrapper(FILE_DAIMONIN_P0, "rb")) == NULL)
        return 1;

    lseek(fileno(stream), bmaptype_table[num].pos, SEEK_SET);

    pbuf = malloc(bmaptype_table[num].len);
    fread(pbuf, bmaptype_table[num].len, 1, stream);
    fclose(stream);

    rwop = SDL_RWFromMem(pbuf, bmaptype_table[num].len);

    FaceList[num].sprite = sprite_tryload_file(NULL, 0, rwop);
    if (FaceList[num].sprite)
        face_flag_extension(num, FaceList[num].name);

    free(pbuf);
    return 0;
}
/* we got a face - test we have it loaded.
 * if not, say server "send us face cmd "
 * Return: 0 - face not there, requested.
 * 1: face requested or loaded
 */
int request_face(int pnum)
{
    char        buf[256 * 2];
    FILE       *stream;
    struct stat statbuf;
    int         len;
    unsigned char * data;
    uint16      num     = (uint16) (pnum&~0x8000);

    if (FaceList[num].name || FaceList[num].flags & FACE_REQUESTED) /* loaded OR requested.. */
        return 1;

    if (num >= bmaptype_table_size)
    {
        LOG(LOG_ERROR, "REQUEST_FILE(): server sent picture id to big (%d %d)\n", num, bmaptype_table_size);
        return 0;
    }

    /* now lets check BEFORE we do any other test for this name in /gfx_user.
     * Perhaps we have a customized picture here.
     */
    sprintf(buf, "%s%s.png", GetGfxUserDirectory(), bmaptype_table[num].name);
    if ((stream = fopen_wrapper(buf, "rb")) != NULL)
    {
        /* yes we have a picture with this name in /gfx_user!
             * lets try to load.
             */
        fstat(fileno(stream), &statbuf);
        len = (int) statbuf.st_size;
        data = malloc(len);
        len = fread(data, 1, len, stream);
        fclose(stream);
        if (len > 0)
        {
            /* lets try to load first... */
            FaceList[num].sprite = sprite_tryload_file(buf, 0, NULL);
            if (FaceList[num].sprite) /* NOW we have a valid png with right name ...*/
            {
                face_flag_extension(num, buf);
                sprintf(buf, "%s%s.png", GetGfxUserDirectory(), bmaptype_table[num].name);
                FaceList[num].name = (char *) malloc(strlen(buf) + 1);
                strcpy(FaceList[num].name, buf);
                FaceList[num].checksum = crc32(1L, data, len);
                free(data);
                return 1;
            }
        }
        /* if we are here something was wrong with the gfx_user file.*/
        free(data);
    }

    /* ok - at this point we hook in our client stored png lib.
    */

    if (bmaptype_table[num].pos != -1) /* best case - we have it in daimonin.p0! */
    {
        sprintf(buf, "%s.png", bmaptype_table[num].name);
        FaceList[num].name = (char *) _malloc(strlen(buf) + 1, "request_face(): FaceList name");
        strcpy(FaceList[num].name, buf);
        FaceList[num].checksum = bmaptype_table[num].crc;
        load_picture_from_pack(num);
    }
    else /* check client /cache folder. If its not there, finish_face_cmd() will stop or ask the server */
    {
        FaceList[num].flags |= FACE_REQUESTED;
        finish_face_cmd(num, bmaptype_table[num].crc, bmaptype_table[num].name);
    }

    return 1;
}

/* send the setup command to the server
 * This is the handshake command after the client connects
 * and the first data which are send between server & client
 * NOTE: Because this is the first command, the data part is 
 * String only. With the response from the server we get 
 * endian info which enables us to send binary data (without
 * fixed shifting)
 */
void SendSetupCmd(void)
{
    char buf[MAX_BUF];

    sprintf(buf, "cs %s sc %s sn %d mz %dx%d skf %d|%x spf %d|%x bpf %d|%x stf %d|%x amf %d|%x",
    VERSION_CS, VERSION_SC, SoundStatus, MapStatusX, MapStatusY, srv_client_files[SRV_CLIENT_SKILLS].len,
    srv_client_files[SRV_CLIENT_SKILLS].crc, srv_client_files[SRV_CLIENT_SPELLS].len,
    srv_client_files[SRV_CLIENT_SPELLS].crc, srv_client_files[SRV_CLIENT_BMAPS].len,
    srv_client_files[SRV_CLIENT_BMAPS].crc, srv_client_files[SRV_CLIENT_SETTINGS].len,
    srv_client_files[SRV_CLIENT_SETTINGS].crc, srv_client_files[SRV_CLIENT_ANIMS].len,
    srv_client_files[SRV_CLIENT_ANIMS].crc);

    send_command_binary(CLIENT_CMD_SETUP, buf, strlen(buf), SEND_CMD_FLAG_STRING);
}

/* Request a so called "server file" from the server.
 * Which includes a list of skills the server knows,
 * spells and such, and how they are described and 
 * visualized
 */
void RequestFile(ClientSocket csock, int index)
{
    SockList    sl;

    /* for binary stuff we better use the socklist system */
    SockList_INIT(&sl, NULL);
    SockList_COMMAND(&sl, CLIENT_CMD_REQUESTFILE, SEND_CMD_FLAG_FIXED);
    SockList_AddChar(&sl, index);
    send_socklist_binary(&sl);
}

/* ONLY send this when we are valid connected to our account.
 * Server will assume a hacking attempt when something is wrong
 * we are not logged to an account or name don't exists in that
 * account. Will invoke a hack warning and a temp ban!
 * This command will invoke the login for char name and put player
 * in playing mode or invoke an account cmd error msg
 */
void SendAddMe(char *name)
{
    SockList    sl;
    SockList_INIT(&sl, NULL);
    SockList_COMMAND(&sl, CLIENT_CMD_ADDME, SEND_CMD_FLAG_DYNAMIC);
    SockList_AddString(&sl, name);
    send_socklist_binary(&sl);
}

/* the server also parsed client_settings. 
 * We only tell him our name, the selected default arch (as gender_selected)
 * and the weapon skill
 * The server will grap the other values from the loaded file
 */
void send_new_char(_server_char *nc)
{
    int i =0;
    SockList    sl;
    _server_char   *tmpc;


    /* lets find the entry number */
    for (tmpc = first_server_char; tmpc; tmpc = tmpc->next)
    {
        /* get our current template */
        if (!strcmp(tmpc->name, new_character.name))
            break;
        i++;
    }

    SockList_INIT(&sl, NULL);
    SockList_COMMAND(&sl, CLIENT_CMD_NEWCHAR, SEND_CMD_FLAG_DYNAMIC);
    SockList_AddChar(&sl, i);
    SockList_AddChar(&sl, nc->gender_selected);
    SockList_AddChar(&sl, nc->skill_selected);
    SockList_AddString(&sl, cpl.name);
    send_socklist_binary(&sl);
}

/* delete a character */
void send_del_char(char *name)
{
    SockList    sl;

    SockList_INIT(&sl, NULL);
    SockList_COMMAND(&sl, CLIENT_CMD_DELCHAR, SEND_CMD_FLAG_DYNAMIC);
    SockList_AddString(&sl, name);
    send_socklist_binary(&sl);
}


void client_send_apply(int tag)
{
    SockList    sl;

    SockList_INIT(&sl, NULL);
    SockList_COMMAND(&sl, CLIENT_CMD_APPLY, SEND_CMD_FLAG_FIXED);
    SockList_AddInt(&sl, tag);
    send_socklist_binary(&sl);
}

/* THE main move command function */
void send_move_command(int dir, int mode)
{
    SockList    sl;
    // remapped to: "idle", "/sw", "/s", "/se", "/w", "/stay", "/e", "/nw", "/n", "/ne" 

    SockList_INIT(&sl, NULL);
    SockList_COMMAND(&sl, CLIENT_CMD_MOVE, SEND_CMD_FLAG_FIXED);
    SockList_AddChar(&sl, move_dir[dir]);
    SockList_AddChar(&sl, mode);
    send_socklist_binary(&sl);
}

void client_send_examine(int tag)
{
    SockList    sl;

    SockList_INIT(&sl, NULL);
    SockList_COMMAND(&sl, CLIENT_CMD_EXAMINE, SEND_CMD_FLAG_FIXED);
    SockList_AddInt(&sl, tag);
    send_socklist_binary(&sl);
}

/* Requests nrof objects of tag get moved to loc. */
void send_inv_move(int loc, int tag, int nrof)
{
    SockList    sl;

    SockList_INIT(&sl, NULL);
    SockList_COMMAND(&sl, CLIENT_CMD_INVMOVE, SEND_CMD_FLAG_FIXED);
    SockList_AddInt(&sl, loc);
    SockList_AddInt(&sl, tag);
    SockList_AddInt(&sl, nrof);
    send_socklist_binary(&sl);
}

void client_send_tell_extended(char *body, char *tail)
{
    SockList    sl;
    char    buf[MAX_BUF];

    sprintf(buf, "%s %s", body, tail);

    SockList_INIT(&sl, NULL);
    SockList_COMMAND(&sl, CLIENT_CMD_GUITALK, SEND_CMD_FLAG_STRING);
    SockList_AddBuffer(&sl, buf, strlen(buf));
    send_socklist_binary(&sl);
}

void send_lock_command(int mode, int tag)
{
    SockList    sl;

    SockList_INIT(&sl, NULL);
    SockList_COMMAND(&sl, CLIENT_CMD_LOCK, SEND_CMD_FLAG_FIXED);
    SockList_AddChar(&sl, mode);
    SockList_AddInt(&sl, tag);
    send_socklist_binary(&sl);
}

void send_mark_command(int tag)
{
    SockList    sl;

    SockList_INIT(&sl, NULL);
    SockList_COMMAND(&sl, CLIENT_CMD_MARK, SEND_CMD_FLAG_FIXED);
    SockList_AddInt(&sl, tag);
    send_socklist_binary(&sl);
}

void send_fire_command(int num, int mode, char *tmp_name)
{
    SockList    sl;

    SockList_INIT(&sl, NULL);
    SockList_COMMAND(&sl, CLIENT_CMD_FIRE, 0);
    SockList_AddInt(&sl, move_dir[num]);
    SockList_AddInt(&sl, mode);
    if(tmp_name)
        SockList_AddBuffer(&sl, tmp_name, strlen(tmp_name));
    SockList_AddChar(&sl, 0); /* be sure we finish with zero - server will check it */
    send_socklist_binary(&sl);

}

void client_send_checkname(char *buf)
{
    SockList    sl;

    SockList_INIT(&sl, NULL);
    SockList_COMMAND(&sl, CLIENT_CMD_CHECKNAME, 0);
    SockList_AddString(&sl, buf);
    send_socklist_binary(&sl);
}

void client_send_login(int mode, char *name, char *pass)
{
    SockList    sl;

    SockList_INIT(&sl, NULL);
    SockList_COMMAND(&sl, CLIENT_CMD_LOGIN, 0);
    SockList_AddChar(&sl, mode);
    SockList_AddString(&sl, name);
    SockList_AddString(&sl, pass);
    send_socklist_binary(&sl);
}

