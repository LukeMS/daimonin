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

/* Client interface main routine.
 * this file sets up a few global variables, connects to the server,
 * tells it what kind of pictures it wants, adds the client and enters
 * the main dispatch loop
 *
 * the main event loop (event_loop()) checks the tcp socket for input and
 * then polls for x events.  This should be fixed since you can just block
 * on both filedescriptors.
 *
 * The DoClient function recieves a message (an ArgList), unpacks it, and
 * in a slow for loop dispatches the command to the right function through
 * the commands table.   ArgLists are essentially like RPC things, only
 * they don't require going through RPCgen, and it's easy to get variable
 * length lists.  They are just lists of longs, strings, characters, and
 * byte arrays that can be converted to a machine independent format
*/


#include <include.h>
#include <stdio.h>

#define ROTATE_RIGHT(c) if ((c) & 01) (c) = ((c) >>1) + 0x80000000; else (c) >>= 1;

Client_Player   cpl;
ClientSocket    csocket;

typedef void (*CmdProc)(unsigned char *, int len);

struct CmdMapping
{
    char   *cmdname;
    void (*cmdproc)(unsigned char *, int);
};

enum
{
    BINARY_CMD_COMC                 = 1,
    BINARY_CMD_MAP2,
    BINARY_CMD_DRAWINFO,
    BINARY_CMD_DRAWINFO2,
    BINARY_CMD_MAP_SCROLL,
    BINARY_CMD_ITEMX,
    BINARY_CMD_SOUND,
    BINARY_CMD_TARGET,
    BINARY_CMD_UPITEM,
    BINARY_CMD_DELITEM,
    BINARY_CMD_STATS,
    BINARY_CMD_IMAGE,
    BINARY_CMD_FACE1,
    BINARY_CMD_ANIM,
    BINARY_CMD_SKILLRDY,
    BINARY_CMD_PLAYER,
    BINARY_CMD_MAPSTATS,
    BINARY_CMD_SPELL_LIST,
    BINARY_CMD_SKILL_LIST,
    BINARY_CMD_GOLEMCMD,
    BINARY_CMD_ADDME_SUC,
    BINARY_CMD_ADDME_FAIL,
    BINARY_CMD_VERSION,
    BINARY_CMD_BYE,
    BINARY_CMD_SETUP,
    BINARY_CMD_QUERY,
    BINARY_CMD_DATA,
    BINARY_CMD_NEW_CHAR,
    BINARY_CMD_ITEMY,
    BINARY_CMD_GROUP,
    BINARY_CMD_INVITE,
    BINARY_CMD_GROUP_UPDATE,
    BINARY_CMD_INTERFACE,
    BINARY_CMD_BOOK,
    BINAR_CMD /* last entry */
};

struct CmdMapping   commands[]  =
{
    /* Order of this table doesn't make a difference.  I tried to sort
    * of cluster the related stuff together.
    */
    { "comc", CompleteCmd}, { "map2", Map2Cmd }, { "drawinfo", (CmdProc) DrawInfoCmd },
    { "drawinfo2", (CmdProc) DrawInfoCmd2 }, { "map_scroll", (CmdProc) map_scrollCmd }, { "itemx", ItemXCmd },
    { "sound", SoundCmd}, { "to", TargetObject }, { "upditem", UpdateItemCmd }, { "delitem", DeleteItem },
    { "stats", StatsCmd }, { "image", ImageCmd }, { "face1", Face1Cmd}, { "anim", AnimCmd},
    { "skill_rdy", (CmdProc) SkillRdyCmd }, { "player", PlayerCmd }, { "mapstats", MapstatsCmd },
    { "splist", SpelllistCmd }, { "sklist", SkilllistCmd }, { "gc", GolemCmd },
    { "addme_success", (CmdProc) AddMeSuccess }, { "addme_failed", (CmdProc) AddMeFail },
    { "version", (CmdProc) VersionCmd }, { "goodbye", (CmdProc) GoodbyeCmd }, { "setup", (CmdProc) SetupCmd},
    { "query", (CmdProc) handle_query}, { "data", (CmdProc) DataCmd}, { "new_char", (CmdProc) NewCharCmd},
    { "itemy", ItemYCmd }, { "group", GroupCmd },{ "group_invite", GroupInviteCmd },
    { "group_update", GroupUpdateCmd },
    { "interface", InterfaceCmd },
    { "book", BookCmd },

    /* unused! */
    { "magicmap", MagicMapCmd}, { "delinv", DeleteInventory },
};

#define NCOMMANDS (sizeof(commands)/sizeof(struct CmdMapping))

static void face_flag_extension(int pnum, char *buf);

void DoClient(ClientSocket *csocket)
{
    int     i, len;
    uint8   cmd_id;
    unsigned char * data;

    while (1)
    {
        i = read_socket(csocket->fd, &csocket->inbuf, MAXSOCKBUF - 1);
        if (i == -1)
        {
            /* Need to add some better logic here */
            LOG(LOG_MSG, "Got error on read (error %d)\n", SOCKET_GetError());
            SOCKET_CloseSocket(csocket->fd);
            return;
        }
        if (i == 0)
            return;   /* Don't have a full packet */
        csocket->inbuf.buf[csocket->inbuf.len] = '\0';

        cmd_id = (uint8) csocket->inbuf.buf[2];
        data = csocket->inbuf.buf + 3;
        len = csocket->inbuf.len - 3; /* 2 byte package len + 1 byte binary cmd */

        /*LOG(LOG_MSG,"Command #%d (LT:%d)(len:%d) ",cmd_id, LastTick, len);*/
        if (!cmd_id || cmd_id >= BINAR_CMD)
            LOG(LOG_ERROR, "Bad command from server (%d)\n", cmd_id);
        else
        {
            /*                  LOG(LOG_MSG,"(%s) >%s<\n",commands[cmd_id-1].cmdname,data);*/
            commands[cmd_id - 1].cmdproc(data, len);
        }
        csocket->inbuf.len = 0;
    }
}

void SockList_Init(SockList *sl)
{
    sl->len = 0;
    sl->buf = NULL;
}

void SockList_AddChar(SockList *sl, char c)
{
    sl->buf[sl->len] = c;
    sl->len++;
}

void SockList_AddShort(SockList *sl, uint16 data)
{
    sl->buf[sl->len++] = (data >> 8) & 0xff;
    sl->buf[sl->len++] = data & 0xff;
}

void SockList_AddInt(SockList *sl, uint32 data)
{
    sl->buf[sl->len++] = (data >> 24) & 0xff;
    sl->buf[sl->len++] = (data >> 16) & 0xff;
    sl->buf[sl->len++] = (data >> 8) & 0xff;
    sl->buf[sl->len++] = data & 0xff;
}

/* Basically does the reverse of SockList_AddInt, but on
 * strings instead.  Same for the GetShort, but for 16 bits.
 */
int GetInt_String(unsigned char *data)
{
    return ((data[0] << 24) + (data[1] << 16) + (data[2] << 8) + data[3]);
}

short GetShort_String(unsigned char *data)
{
    return ((data[0] << 8) + data[1]);
}


/* Send With Handling - cnum is the client number, msg is what we want
 * to send.
 */
int send_socklist(int fd, SockList  msg)
{
    unsigned char sbuf[2];

    sbuf[0] = ((uint32) (msg.len) >> 8) & 0xFF;
    sbuf[1] = ((uint32) (msg.len)) & 0xFF;

    write_socket(fd, sbuf, 2);
    return write_socket(fd, msg.buf, msg.len);
}

/* Takes a string of data, and writes it out to the socket. A very handy
 * shortcut function.
 */
int cs_write_string(int fd, char *buf, int len)
{
    SockList    sl;

    sl.len = len;
    sl.buf = (unsigned char *) buf;
    return send_socklist(fd, sl);
}

void finish_face_cmd(int pnum, uint32 checksum, char *face)
{
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
        if (len <= 0) /* something is wrong... now unlink the file and
                                  * let it reload then possible and needed
                                  */
        {
            unlink(buf);
            checksum = 1; /* now we are 100% different to newsum */
        }
        else /* lets go for the checksum check*/
            newsum = crc32(1L, data, len);
        free(data);

        if (newsum == checksum)
        {
            FaceList[pnum].sprite = sprite_tryload_file(buf, 0, NULL);
            /*perhaps we fail ,try insanity new load*/
            if (FaceList[pnum].sprite)
            {
                face_flag_extension(pnum, buf);
                return; /* found and loaded!*/
            }
        }
    }
    /*LOG(LOG_MSG,"FACE: call server for %s (%d)\n", face, pnum);*/
    face_flag_extension(pnum, buf);
    sprintf(buf, "askface %d", pnum);
    cs_write_string(csocket.fd, buf, strlen(buf)); /* face command los*/
}

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
 * this command collect all new faces and then flush
 * it at once. I insert the flush command after the
 * socket call.
 */
#define REQUEST_FACE_MAX 250

int request_face(int pnum, int mode)
{
    char        buf[256 * 2];
    FILE       *stream;
    struct stat statbuf;
    int         len;
    unsigned char * data;
    static int  count   = 0;
    static char fr_buf[REQUEST_FACE_MAX*sizeof(uint16) + 4];
    uint16      num     = (uint16) (pnum&~0x8000);

    if (mode == 1) /* forced flush buffer & command */
    {
        if (count)
        {
            fr_buf[0] = 'f';
            fr_buf[1] = 'r';
            fr_buf[2] = ' ';
            cs_write_string(csocket.fd, fr_buf, 4 + count * sizeof(uint16));
            count = 0;
        }
        return 1;
    }

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
    else /* 2nd best case  - lets check the cache for it...  or request it */
    {
        FaceList[num].flags |= FACE_REQUESTED;
        finish_face_cmd(num, bmaptype_table[num].crc, bmaptype_table[num].name);
    }

    /*
    *((uint16 *)(fr_buf+4+count*sizeof(uint16)))=num;
    *((uint8 *)(fr_buf+3))=(uint8)++count;
    if(count == REQUEST_FACE_MAX)
    {
        fr_buf[0]='f';
        fr_buf[1]='r';
        fr_buf[2]=' ';
        cs_write_string(csocket.fd, fr_buf, 4+count*sizeof(uint16));
        count = 0;
    }
    */
    return 1;
}


/* we don't give a error message here in return - no need!
 * IF the server has messed up the anims file he send us - then
 * we simply core here. There is nothing we can do - in the badest
 * case the bad server harm us even more.
 */
void check_animation_status(int anum)
{
    /* i really do it simple here - because we had stored our default
     * anim list in the same way a server used in the past the anim
     * command, we simple call the command. This seems a bit odd, but
     * perhaps we play around in the future with at runtime created
     * anims (server side) - then we need this interface again and we
     * can simply call it from both sides.
     */
    if (animations[anum].loaded)
        return;

    /* why we don't preload all anims?
     * because the anim also flushes loading of all face
     * part of this anim!
     */
    animations[anum].loaded = 1; /* mark this anim as "we have loaded it" */
    AnimCmd((unsigned char *)anim_table[anum].anim_cmd, anim_table[anum].len); /* same as server sends it! */
}

/* removes whitespace from right side */
char * adjust_string(char *buf)
{
    int i, len = strlen(buf);

    for (i = len - 1; i >= 0; i--)
    {
        if (!isspace(buf[i]))
            return buf;

        buf[i] = 0;
    }
    return buf;
}
