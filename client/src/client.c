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


#define ROTATE_RIGHT(c) if ((c) & 01) (c) = ((c) >>1) + 0x80000000; else (c) >>= 1;

Client_Player cpl;
ClientSocket csocket;

typedef void (*CmdProc)(unsigned char *, int len);

struct CmdMapping
{
        char *cmdname;
        void (*cmdproc)(unsigned char *, int );
};


struct CmdMapping commands[] =
{
        /* Order of this table doesn't make a difference.  I tried to sort
         * of cluster the related stuff together.
         */
        { "comc", CompleteCmd},
        { "map2", Map2Cmd },
        { "drawinfo", (CmdProc)DrawInfoCmd },
        { "map_scroll", (CmdProc)map_scrollCmd },
        { "itemx", ItemXCmd },
        { "sound", SoundCmd},
        { "to", TargetObject },
        
        { "upditem", UpdateItemCmd },
        { "delitem", DeleteItem },

        { "stats", StatsCmd },
        
        { "image", ImageCmd },
        { "face1", Face1Cmd},
        { "anim", AnimCmd},

        { "skill_rdy", (CmdProc) SkillRdyCmd },
        { "player", PlayerCmd },
        { "mapstats", MapstatsCmd },
        { "splist", SpelllistCmd },
        { "sklist", SkilllistCmd },
        { "gc", GolemCmd },
        { "item1", Item1Cmd },
        
        { "addme_failed", (CmdProc)AddMeFail },
        { "addme_success", (CmdProc)AddMeSuccess },
        { "version", (CmdProc)VersionCmd },
        { "goodbye", (CmdProc)GoodbyeCmd },
        { "setup", (CmdProc)SetupCmd},

        { "query", (CmdProc)handle_query},
        { "magicmap", MagicMapCmd},
        { "delinv", DeleteInventory },
};

#define NCOMMANDS (sizeof(commands)/sizeof(struct CmdMapping))

static void face_flag_extension(int pnum, char *buf);

void DoClient(ClientSocket *csocket)
{
        int i,len;
        unsigned char *data;

        while (1)
        {
                i=read_socket(csocket->fd, &csocket->inbuf, MAXSOCKBUF-1);
                if (i==-1)
                {
                        /* Need to add some better logic here */
                        LOG(LOG_MSG,"Got error on read (error %d)\n", SOCKET_GetError());
						SOCKET_CloseSocket(csocket->fd);
                        return;
                }
                if (i==0) return;   /* Don't have a full packet */
                csocket->inbuf.buf[csocket->inbuf.len]='\0';
                data = (unsigned char *)strchr((char*)csocket->inbuf.buf +2, ' ');
                if (data)
                {
                        *data='\0';
                        data++;
                }
                len = csocket->inbuf.len - (data - csocket->inbuf.buf);
                /* Terminate the buffer */
                /*LOG(LOG_MSG,"Command (%d):%s (%d) %s\n",LastTick,csocket->inbuf.buf+2, len,data);*/
                for(i=0;i < NCOMMANDS;i++)
                {
                        if (strcmp((char*)csocket->inbuf.buf+2,commands[i].cmdname)==0)
                        {
                                commands[i].cmdproc(data,len);
                                break;
                        }
                }
                csocket->inbuf.len=0;
                if (i == NCOMMANDS)
                {
                        LOG(LOG_ERROR,"Bad command from server (%s)\n",csocket->inbuf.buf+2);
                }
        }
}

void SockList_Init(SockList *sl)
{
        sl->len=0;
        sl->buf=NULL;
}

void SockList_AddChar(SockList *sl, char c)
{
        sl->buf[sl->len]=c;
        sl->len++;
}

void SockList_AddShort(SockList *sl, uint16 data)
{
        sl->buf[sl->len++]= (data>>8)&0xff;
        sl->buf[sl->len++] = data & 0xff;
}

void SockList_AddInt(SockList *sl, uint32 data)
{
        sl->buf[sl->len++]= (data>>24)&0xff;
        sl->buf[sl->len++]= (data>>16)&0xff;
        sl->buf[sl->len++]= (data>>8)&0xff;
        sl->buf[sl->len++] = data & 0xff;
}

/* Basically does the reverse of SockList_AddInt, but on
 * strings instead.  Same for the GetShort, but for 16 bits.
 */
int GetInt_String(unsigned char *data)
{
        return ((data[0]<<24) + (data[1]<<16) + (data[2]<<8) + data[3]);
}

short GetShort_String(unsigned char *data)
{
        return ((data[0]<<8)+data[1]);
}


/* Send With Handling - cnum is the client number, msg is what we want
 * to send.
 */
int send_socklist(int fd,SockList  msg)
{
        unsigned char sbuf[2];

        sbuf[0] = ((uint32)(msg.len) >> 8) & 0xFF;
        sbuf[1] = ((uint32)(msg.len)) & 0xFF;

        write_socket(fd, sbuf, 2);
        return write_socket(fd, msg.buf, msg.len);
}

/* Takes a string of data, and writes it out to the socket. A very handy
 * shortcut function.
 */
int cs_write_string(int fd, char *buf, int len)
{
        SockList sl;

        sl.len = len;
        sl.buf = (unsigned char*)buf;
        return send_socklist(fd, sl);
}

void finish_face_cmd(int pnum, uint32 checksum, char *face)
{
        char buf[2048];
        int fd,len, i;
        static uint32 newsum=0;
        unsigned char data[65536];
		void *tmp_free;

        /* first, check our memory... perhaps we have it loaded */
        /*LOG(LOG_MSG,"FACE: %s (->%s)\n", face,FaceList[pnum].name);*/
        if(FaceList[pnum].name) /* loaded OR requested...hm, no double request check yet*/
        {
                /* lets check the name and checksum and sprite. ONLY if all is
                 * ok, we stay with it
                 */
                 if(strcmp(face, FaceList[pnum].name) &&
                        checksum != FaceList[pnum].checksum &&
                        FaceList[pnum].sprite)
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
        sprintf(buf,"%s.png", face);
        FaceList[pnum].name = (char *) _malloc(strlen(buf)+1, "finish_face_cmd(): FaceList name");
        strcpy(FaceList[pnum].name, buf);
        FaceList[pnum].checksum = checksum;

        /* Check private cache first */
        sprintf(buf,"%s%s", GetCacheDirectory(), FaceList[pnum].name);
        if ((fd=open(buf, O_RDONLY|O_BINARY))!=-1)
        {
            len=read(fd, data, 65535);
            close(fd);
                if(len <=0) /* something is wrong, request it better again*/
                {
                        LOG(LOG_ERROR, "WARNING: Cached file %s broken.\n");
                }
                else
                {
                        /* lets go for the checksum check*/
                    newsum =0;
                        for (i=0; i<len; i++)
                        {
                            /*  LOG(LOG_ERROR, "CS: #%d -> %d -->%u \n",i,data[i],newsum); */
                            ROTATE_RIGHT(newsum);
                                newsum += data[i];
                                newsum &= 0xffffffff;
                        }
                        if (newsum == checksum)
                        {
                                /* LOG(LOG_MSG,"FACE: found %s\n", face); */

                                /* URGH. do it quick yet
                                * and let SDL reload... but we should use
                                * the loaded face data...
                                */
                                FaceList[pnum].sprite=sprite_tryload_file(buf,0);
                                /*perhaps we fail ,try insanity new load*/
                                if(FaceList[pnum].sprite)
								{
									face_flag_extension(pnum, buf);
									return; /* found and loaded!*/
								}
                        }
                }
        }
        /* LOG(LOG_MSG,"FACE: call server for %s\n", face); */
		face_flag_extension(pnum, buf);
        sprintf(buf,"askface %d",pnum);
        cs_write_string(csocket.fd, buf, strlen(buf)); /* face command los*/
}

static void face_flag_extension(int pnum, char *buf)
{
	char *stemp;

	FaceList[pnum].flags = FACE_FLAG_NO;
	/* check for the "double"/"up" tag in the picture name */
	if((stemp = strstr(buf,".d")))
		FaceList[pnum].flags |= FACE_FLAG_DOUBLE;
	else if((stemp = strstr(buf,".u")))
		FaceList[pnum].flags |= FACE_FLAG_UP;

	/* Now the facing stuff: if some tag was there, lets grap the facing info */
	if(FaceList[pnum].flags && stemp)
	{
		int tc;
		for(tc=0;tc<4;tc++)
		{
			if(!*(stemp+tc)) /* has the string a '0' before our anim tags */
				goto finish_face_cmd_j1;
		}
		/* lets set the right flags for the tags */
		if(((FaceList[pnum].flags & FACE_FLAG_UP) && *(stemp+tc) == '5') || *(stemp+tc) == '1')
			FaceList[pnum].flags |= FACE_FLAG_D1;
		else if(*(stemp+tc) == '3')
			FaceList[pnum].flags |= FACE_FLAG_D3;
		else if(*(stemp+tc) == '4'||*(stemp+tc) == '8' || *(stemp+tc) == '0')
			FaceList[pnum].flags |= (FACE_FLAG_D3|FACE_FLAG_D1);

	}
	finish_face_cmd_j1: /* error jump from for() */
	return;
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
	static int count=0;
	static char fr_buf[REQUEST_FACE_MAX*sizeof(uint16)+4];
	uint16 num =(uint16)(pnum&~0x8000);

	if(mode) /* forced flush buffer & command */
	{
		if(count)
		{
			fr_buf[0]='f';
			fr_buf[1]='r';
			fr_buf[2]=' ';
		    cs_write_string(csocket.fd, fr_buf, 4+count*sizeof(uint16));
			count = 0;
		}
		return 1;
	}

	if(FaceList[num].name || FaceList[num].flags&FACE_REQUESTED) /* loaded OR requested.. */
		return 1;

	FaceList[num].flags|=FACE_REQUESTED;

	*((uint16 *)(fr_buf+4+count*sizeof(uint16)))=num;
	*((uint8 *)(fr_buf+3))=(uint8)++count;


	/* buffer full, flush command */
	if(count == REQUEST_FACE_MAX)
	{
		fr_buf[0]='f';
		fr_buf[1]='r';
		fr_buf[2]=' ';
		cs_write_string(csocket.fd, fr_buf, 4+count*sizeof(uint16));
		count = 0;
	}
	return 0;
}

