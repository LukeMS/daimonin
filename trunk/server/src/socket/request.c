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

/*
 * This file implements all of the goo on the server side for handling 
 * clients.  It's got a bunch of global variables for keeping track of 
 * each of the clients. 
 *
 * Note:  All functions that are used to process data from the client
 * have the prototype of (char *data, int datalen, int client_num).  This
 * way, we can use one dispatch table.
 *
 */

#include <global.h>
#include <sproto.h>

#include <newclient.h>
#include <newserver.h>
#include <living.h>
#include <commands.h>

/* This block is basically taken from socket.c - I assume if it works there,
 * it should work here.
 */
#ifndef WIN32 /* ---win32 exclude unix headers */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#endif /* win32 */

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include "sounds.h"

#define GET_CLIENT_FLAGS(_O_)	((_O_)->flags[0]&0x7f)
#define NO_FACE_SEND        (-1)

static int atnr_prot_stats[NROFPROTECTIONS] = {
CS_STAT_PROT_HIT,
CS_STAT_PROT_SLASH,
CS_STAT_PROT_CLEAVE,
CS_STAT_PROT_PIERCE,
CS_STAT_PROT_WMAGIC,
    
CS_STAT_PROT_FIRE,
CS_STAT_PROT_COLD,
CS_STAT_PROT_ELEC,
CS_STAT_PROT_POISON,
CS_STAT_PROT_ACID,
    
CS_STAT_PROT_MAGIC,
CS_STAT_PROT_MIND,
CS_STAT_PROT_BODY,
CS_STAT_PROT_PSIONIC,
CS_STAT_PROT_ENERGY,
    
CS_STAT_PROT_NETHER,
CS_STAT_PROT_CHAOS,
CS_STAT_PROT_DEATH,
CS_STAT_PROT_HOLY,
CS_STAT_PROT_CORRUPT
};

/* This is the Setup cmd - easy first implementation */
void SetUp(char *buf, int len, NewSocket *ns)
{
    int s;
    char *cmd, *param, tmpbuf[MAX_BUF],cmdback[HUGE_BUF];

	if(ns->setup)
	{
		LOG(llevInfo,"double call of setup cmd from socket %s\n", ns->host); 
		ns->status = Ns_Dead;
		return;
	}
	ns->setup=1;

    /* run through the cmds of setup
     * syntax is setup <cmdname1> <parameter> <cmdname2> <parameter> ...
     *
     * we send the status of the cmd back, or a FALSE is the cmd is the server unknown
     * The client then must sort this out
     */

    LOG(llevInfo,"Get SetupCmd:: %s\n", buf);
	cmdback[0]=BINARY_CMD_SETUP;
	cmdback[1]=0;
    /*strcpy(cmdback,"setup");*/
    for(s=0;s<len; ) {

	cmd = &buf[s];

	/* find the next space, and put a null there */
	for(; s<len && buf[s] && buf[s] != ' ';s++) ;
	buf[s++]=0;
	while (s<len && buf[s] == ' ' ) 
		s++;

	if(s>=len)
	    break;

	param = &buf[s];

	for(;s<len && buf[s] && buf[s] != ' ';s++) ;
	buf[s++]=0;
	while (s<len && buf[s] == ' ') s++;
		
	strcat(cmdback, " ");
	strcat(cmdback, cmd);
	strcat(cmdback, " ");

	if (!strcmp(cmd,"sound")) {
	    ns->sound = atoi(param);
	    strcat(cmdback, param);
	}
    else if (!strcmp(cmd,"darkness")) {
	    ns->darkness = atoi(param);
	    strcat(cmdback, param);
	}
	 else if (!strcmp(cmd,"map2cmd"))
	 {
            ns->map2cmd = atoi(param);
            /* if beyond this size, need to use map2cmd no matter what */
            if (ns->mapx>11 || ns->mapy>11) ns->map2cmd=1;
            strcat(cmdback, ns->map2cmd?"1":"0");
	} else if (!strcmp(cmd,"facecache")) {
	    ns->facecache = atoi(param);
            strcat(cmdback, param);
	} else if (!strcmp(cmd,"faceset")) {
	    int q = atoi(param);

	    if (is_valid_faceset(q))
		ns->faceset=q;
	    sprintf(tmpbuf,"%d", ns->faceset);
	    strcat(cmdback, tmpbuf);
	    /* if the client is using faceset, it knows about image2 command */
	    ns->image2=1;
        } else if (!strcmp(cmd,"mapsize")) {
	    int x, y=0;
	    char *cp;

	    x = atoi(param);
	    for (cp = param; *cp!=0; cp++)
		if (*cp == 'x' || *cp == 'X') {
		    y = atoi(cp+1);
		    break;
		}
	    if (x < 9 || y < 9 || x>MAP_CLIENT_X || y > MAP_CLIENT_Y) {
		sprintf(tmpbuf," %dx%d", MAP_CLIENT_X, MAP_CLIENT_Y);
		strcat(cmdback, tmpbuf);
	    } else {
		ns->mapx = x;
		ns->mapy = y;
		/* better to send back what we are really using and not the
		 * param as given to us in case it gets parsed differently.
		 */
		sprintf(tmpbuf,"%dx%d", x,y);
		strcat(cmdback, tmpbuf);
	    }
    } else if (!strcmp(cmd,"skf")) {
		char *cp;
	    int x=0;
		uint32 y=0;

		/* is x our files len and y the crc */
	    for (cp = param; *cp!=0; cp++)
		if (*cp == '|') {
			*cp=0;
		    x = atoi(param);
		    y = strtoul(cp+1, NULL, 16);
		    break;
		}
		/* we check now the loaded file data - if different
		 * we tell it the client - if not, we skip here
		 */
		if(SrvClientFiles[SRV_CLIENT_SKILLS].len_ucomp != x ||
							SrvClientFiles[SRV_CLIENT_SKILLS].crc != y)
		{
			sprintf(tmpbuf,"%d|%x", SrvClientFiles[SRV_CLIENT_SKILLS].len_ucomp,SrvClientFiles[SRV_CLIENT_SKILLS].crc);
			strcat(cmdback, tmpbuf);
		}
		else
			strcat(cmdback, "OK");

    } else if (!strcmp(cmd,"spf")) {
		char *cp;
	    int x=0;
		uint32 y=0;

		/* is x our files len and y the crc */
	    for (cp = param; *cp!=0; cp++)
		if (*cp == '|') {
			*cp=0;
		    x = atoi(param);
		    y = strtoul(cp+1, NULL, 16);
		    break;
		}
		/* we check now the loaded file data - if different
		 * we tell it the client - if not, we skip here
		 */
		if(SrvClientFiles[SRV_CLIENT_SPELLS].len_ucomp != x ||
							SrvClientFiles[SRV_CLIENT_SPELLS].crc != y)
		{
			sprintf(tmpbuf,"%d|%x", SrvClientFiles[SRV_CLIENT_SPELLS].len_ucomp,SrvClientFiles[SRV_CLIENT_SPELLS].crc);
			strcat(cmdback, tmpbuf);
		}
		else
			strcat(cmdback, "OK");
    } else if (!strcmp(cmd,"stf")) {
		char *cp;
	    int x=0;
		uint32 y=0;

		/* is x our files len and y the crc */
	    for (cp = param; *cp!=0; cp++)
		if (*cp == '|') {
			*cp=0;
		    x = atoi(param);
		    y = strtoul(cp+1, NULL, 16);
		    break;
		}
		/* we check now the loaded file data - if different
		 * we tell it the client - if not, we skip here
		 */
		if(SrvClientFiles[SRV_CLIENT_SETTINGS].len_ucomp != x ||
							SrvClientFiles[SRV_CLIENT_SETTINGS].crc != y)
		{
			sprintf(tmpbuf,"%d|%x", SrvClientFiles[SRV_CLIENT_SETTINGS].len_ucomp,SrvClientFiles[SRV_CLIENT_SETTINGS].crc);
			strcat(cmdback, tmpbuf);
		}
		else
			strcat(cmdback, "OK");

    } else if (!strcmp(cmd,"bpf")) {
		char *cp;
	    int x=0;
		uint32 y=0;

		/* is x our files len and y the crc */
	    for (cp = param; *cp!=0; cp++)
		if (*cp == '|') {
			*cp=0;
		    x = atoi(param);
		    y = strtoul(cp+1, NULL, 16);
		    break;
		}
		/* we check now the loaded file data - if different
		 * we tell it the client - if not, we skip here
		 */
		if(SrvClientFiles[SRV_CLIENT_BMAPS].len_ucomp != x ||
							SrvClientFiles[SRV_CLIENT_BMAPS].crc != y)
		{
			sprintf(tmpbuf,"%d|%x", SrvClientFiles[SRV_CLIENT_BMAPS].len_ucomp,SrvClientFiles[SRV_CLIENT_BMAPS].crc);
			strcat(cmdback, tmpbuf);
		}
		else
			strcat(cmdback, "OK");
    } else if (!strcmp(cmd,"amf")) {
		char *cp;
	    int x=0;
		uint32 y=0;

		/* is x our files len and y the crc */
	    for (cp = param; *cp!=0; cp++)
		if (*cp == '|') {
			*cp=0;
		    x = atoi(param);
		    y = strtoul(cp+1, NULL, 16);
		    break;
		}
		/* we check now the loaded file data - if different
		 * we tell it the client - if not, we skip here
		 */
		if(SrvClientFiles[SRV_CLIENT_ANIMS].len_ucomp != x ||
							SrvClientFiles[SRV_CLIENT_ANIMS].crc != y)
		{
			sprintf(tmpbuf,"%d|%x", SrvClientFiles[SRV_CLIENT_ANIMS].len_ucomp,SrvClientFiles[SRV_CLIENT_ANIMS].crc);
			strcat(cmdback, tmpbuf);
		}
		else
			strcat(cmdback, "OK");
	} else {
	    /* Didn't get a setup command we understood -
	     * report a failure to the client.
	     */
	    strcat(cmdback, "FALSE");
	}
    } /* for processing all the setup commands */

	/*LOG(llevInfo,"SendBack SetupCmd:: %s\n", cmdback);*/
    Write_String_To_Socket(ns, BINARY_CMD_SETUP,cmdback, strlen(cmdback));


}

/* The client has requested to be added to the game.  This is what
 * takes care of it.  We tell the client how things worked out.
 * I am not sure if this file is the best place for this function.  however,
 * it either has to be here or init_sockets needs to be exported.
 */
void AddMeCmd(char *buf, int len, NewSocket *ns)
{
    Settings oldsettings;
	char cmd_buf[2]="X";
    oldsettings=settings;

    if (ns->status != Ns_Add || add_player(ns)) {
		Write_String_To_Socket(ns, BINARY_CMD_ADDME_FAIL, cmd_buf,1);
		ns->status = Ns_Dead;
    } else {
	/* Basically, the add_player copies the socket structure into
	 * the player structure, so this one (which is from init_sockets)
	 * is not needed anymore.  The write below should still work, as the
	 * stuff in ns is still relevant.
	 */
	Write_String_To_Socket(ns,BINARY_CMD_ADDME_SUC, cmd_buf,1);
	socket_info.nconns--;
	ns->status = Ns_Avail;
    }
    settings=oldsettings;	
}


/* This handles the general commands from the client (ie, north, fire, cast,
 * etc.)
 */
void PlayerCmd(char *buf, int len, player *pl)
{

    /* The following should never happen with a proper or honest client.
     * Therefore, the error message doesn't have to be too clear - if 
     * someone is playing with a hacked/non working client, this gives them
     * an idea of the problem, but they deserve what they get
     */
    if (pl->state!=ST_PLAYING) {
	new_draw_info_format(NDI_UNIQUE, 0,pl->ob,
	    "You can not issue commands - state is not ST_PLAYING (%s)", buf);
	return;
    }
    /* Check if there is a count.  In theory, a zero count could also be
     * sent, so check for that also.
     */
    if (atoi(buf) || buf[0]=='0') {
	pl->count=atoi((char*)buf);
	buf=strchr(buf,' ');	/* advance beyond the numbers */
	if (!buf) {
#ifdef ESRV_DEBUG
	    LOG(llevDebug,"PlayerCmd: Got count but no command.");
#endif
	    return;
	}
	buf++;
    }
    pl->idle=0;

    /* In c_new.c */
    execute_newserver_command(pl->ob, (char*)buf);
    /* Perhaps something better should be done with a left over count.
     * Cleaning up the input should probably be done first - all actions
     * for the command that issued the count should be done before any other
     * commands.
     */

    pl->count=0;

}


/* This handles the general commands from the client (ie, north, fire, cast,
 * etc.)  It is a lot like PlayerCmd above, but is called with the
 * 'ncom' method which gives more information back to the client so it
 * can throttle.
 */
void NewPlayerCmd(uint8 *buf, int len, player *pl)
{
    uint16 packet;
	int time,repeat;
    char    command[MAX_BUF];
    SockList sl;

    if (len < 7)
	{
		LOG(llevBug,"BUG: Corrupt ncom command from player %s - not long enough - discarding\n", pl->ob->name);
		return;
    }

    packet = GetShort_String(buf);
    repeat = GetInt_String(buf+2);
    /* -1 is special - no repeat, but don't update */
    if (repeat!=-1) {
	pl->count=repeat;
    }
    if ((len-4) >= MAX_BUF) len=MAX_BUF-5;

    strncpy(command, (char*)buf+6, len-4);
    command[len-4]='\0';

    /* The following should never happen with a proper or honest client.
     * Therefore, the error message doesn't have to be too clear - if 
     * someone is playing with a hacked/non working client, this gives them
     * an idea of the problem, but they deserve what they get
     */
    if (pl->state!=ST_PLAYING) {
	new_draw_info_format(NDI_UNIQUE, 0,pl->ob,
	    "You can not issue commands - state is not ST_PLAYING (%s)", buf);
	return;
    }

    pl->idle=0;

    /* In c_new.c */
    execute_newserver_command(pl->ob, command);
    pl->count=0;

    /* Send confirmation of command execution now */
    sl.buf = (uint8*)command;
	SOCKET_SET_BINARY_CMD(&sl, BINARY_CMD_COMC);
	/*
    strcpy((char*)sl.buf,"comc ");
    sl.len=5;
	*/
    SockList_AddShort(&sl,packet);
    if (FABS(pl->ob->speed) < 0.001) time=MAX_TIME * 100;
    else
	time = (int)((float)MAX_TIME/ FABS(pl->ob->speed));
    SockList_AddInt(&sl,time);
    Send_With_Handling(&pl->socket, &sl);
}


/* This is a reply to a previous query. */
void ReplyCmd(char *buf, int len, player *pl)
{
    /* This is to synthesize how the data would be stored if it
     * was normally entered.  A bit of a hack, and should be cleaned up
     * once all the X11 code is removed from the server.
     *
     * We pass 13 to many of the functions because this way they
     * think it was the carriage return that was entered, and the
     * function then does not try to do additional input.
     */
    sprintf(pl->write_buf,":%s",buf);

    pl->socket.ext_title_flag = 1;
    
    switch (pl->state) {
	case ST_PLAYING:
	    LOG(llevBug,"BUG: Got reply message with ST_PLAYING input state (player %s)\n", query_name(pl->ob));
	    break;

	case ST_PLAY_AGAIN:
	    /* We can check this for return value (2==quit).  Maybe we
	     * should, and do something appropriate?
	     */
	    receive_play_again(pl->ob, buf[0]);
	    break;

	case ST_ROLL_STAT:
	    key_roll_stat(pl->ob,buf[0]);
	    break;

	case ST_CHANGE_CLASS:

        key_change_class(pl->ob, buf[0]);
        break;

	case ST_CONFIRM_QUIT:
	    key_confirm_quit(pl->ob, buf[0]);
	    break;

	case ST_CONFIGURE:
	    LOG(llevBug,"BUG: In client input handling, but into configure state (%s)\n", pl->ob->name);
	    pl->state = ST_PLAYING;
	    break;

	case ST_GET_NAME:
	    receive_player_name(pl->ob,13);
	    break;

	case ST_GET_PASSWORD:
	case ST_CONFIRM_PASSWORD:
	    receive_player_password(pl->ob,13);
	    break;

	case ST_GET_PARTY_PASSWORD:        /* Get password for party */
	receive_party_password(pl->ob,13);
	    break;

	default:
	    LOG(llevBug,"BUG: Unknown input state: %d\n", pl->state);
    }
}

static void version_mismatch_msg(NewSocket *ns)
{
	char buf[256];
    char *text1 = "3 This is Daimonin Server.";
    char *text2 = "3 Your client version is outdated!";
    char *text3 = "3 Go to http://daimonin.sourceforge.net";
    char *text4 = "3 and download the latest Daimonin client!";
    char *text5 = "3 Goodbye. (connection closed)";

	if(ns->cs_version==991013)
	{
		SockList sl;

		sprintf(buf,"drawinfo %s %s", text1,VERSION);
		sl.len = strlen(buf);
		sl.buf = (uint8*)buf;
		Send_With_Handling(ns, &sl);
		sprintf(buf,"drawinfo %s", text2);
		sl.len = strlen(buf);
		sl.buf = (uint8*)buf;
		Send_With_Handling(ns, &sl);
		sprintf(buf,"drawinfo %s", text3);
		sl.len = strlen(buf);
		sl.buf = (uint8*)buf;
		Send_With_Handling(ns, &sl);
		sprintf(buf,"drawinfo %s", text4);
		sl.len = strlen(buf);
		sl.buf = (uint8*)buf;
		Send_With_Handling(ns, &sl);
		sprintf(buf,"drawinfo %s", text5);
		sl.len = strlen(buf);
		sl.buf = (uint8*)buf;
		Send_With_Handling(ns, &sl);
	}
	else
	{
		sprintf(buf,"X%s %s", text1,VERSION);
	    Write_String_To_Socket(ns, BINARY_CMD_DRAWINFO, buf, strlen(text1));
		sprintf(buf,"X%s", text2);
	    Write_String_To_Socket(ns, BINARY_CMD_DRAWINFO, buf, strlen(text1));
		sprintf(buf,"X%s", text3);
	    Write_String_To_Socket(ns, BINARY_CMD_DRAWINFO, buf, strlen(text1));
		sprintf(buf,"X%s", text4);
	    Write_String_To_Socket(ns, BINARY_CMD_DRAWINFO, buf, strlen(text1));
		sprintf(buf,"X%s", text5);
	    Write_String_To_Socket(ns, BINARY_CMD_DRAWINFO, buf, strlen(text1));
	}
}

/* request a srv_file! */
void RequestFileCmd(char *buf, int len,NewSocket *ns)
{
	int id;

    if (!buf)
    {
        LOG(llevInfo, "RF: received bad rf command\n");
        ns->status=Ns_Dead;
        return;
    }

    id = atoi(buf);
	if(id <0 ||id >=SRV_CLIENT_FILES)
    {
        LOG(llevInfo, "RF: received bad rf command\n");
        ns->status=Ns_Dead;
        return;
    }

	if(id == SRV_CLIENT_SKILLS)
	{
		if(ns->rf_skills)
		{
	        LOG(llevInfo, "RF: received bad rf command - double call skills \n");
			ns->status=Ns_Dead;
			return;
		}
		else
			ns->rf_skills=1;
	}
	else if(id == SRV_CLIENT_SPELLS)
	{
		if(ns->rf_spells)
		{
	        LOG(llevInfo, "RF: received bad rf command - double call spells \n");
			ns->status=Ns_Dead;
			return;
		}
		else
			ns->rf_spells=1;
	}
	else if(id == SRV_CLIENT_SETTINGS)
	{
		if(ns->rf_settings)
		{
	        LOG(llevInfo, "RF: received bad rf command - double call settings \n");
			ns->status=Ns_Dead;
			return;
		}
		else
			ns->rf_settings=1;
	}
	else if(id == SRV_CLIENT_BMAPS)
	{
		if(ns->rf_bmaps)
		{
	        LOG(llevInfo, "RF: received bad rf command - double call bmaps \n");
			ns->status=Ns_Dead;
			return;
		}
		else
			ns->rf_bmaps=1;
	}
	else if(id == SRV_CLIENT_ANIMS)
	{
		if(ns->rf_anims)
		{
	        LOG(llevInfo, "RF: received bad rf command - double call anims \n");
			ns->status=Ns_Dead;
			return;
		}
		else
			ns->rf_anims=1;
	}

	LOG(llevDebug,"Client %s rf #%d\n",ns->host,id); 
	send_srv_file(ns, id);	
}


/* Client tells its its version.  If there is a mismatch, we close the
 * socket.  In real life, all we should care about is the client having
 * something older than the server.  If we assume the client will be
 * backwards compatible, having it be a later version should not be a 
 * problem.
 */
void VersionCmd(char *buf, int len,NewSocket *ns)
{
    char *cp;
    

    if (!buf || ns->version)
    {
        version_mismatch_msg(ns);
        LOG(llevInfo, "CS: received corrupted version command\n");
        ns->status=Ns_Dead;
        return;
    }

	ns->version = 1; 
    ns->cs_version = atoi(buf);
    ns->sc_version =  ns->cs_version;
    if (VERSION_CS !=  ns->cs_version)
    {
        version_mismatch_msg(ns);
        LOG(llevInfo, "CS: csversion mismatch (%d,%d)\n", VERSION_CS,ns->cs_version);
        ns->status=Ns_Dead;
        return;
    }
    cp = strchr(buf+1,' ');
    if (!cp)
    {
        version_mismatch_msg(ns);
        LOG(llevInfo, "CS: invalid version cmd: %s\n", buf);
        ns->status=Ns_Dead;
        return;
    }
    ns->sc_version = atoi(cp);
    if (VERSION_SC != ns->sc_version) 
    {
        version_mismatch_msg(ns);
        LOG(llevInfo, "CS: scversion mismatch (%d,%d)\n",VERSION_SC,ns->sc_version);
        ns->status=Ns_Dead;
        return;
    }
    cp = strchr(cp+1, ' ');
    if (!cp || strncmp("Daimonin SDL Client",cp+1,19)) 
    {
        version_mismatch_msg(ns);
        if(cp)
            LOG(llevInfo,"CS: connection from false client of type <%s>\n", cp);
        else
            LOG(llevInfo,"CS: connection from false client (invalid name)\n");
        ns->status=Ns_Dead;
        return;
        
    }
}

/* sound related functions. */
 
void SetSound(char *buf, int len, NewSocket *ns)
{
    ns->sound = atoi(buf);
}

/* client wants the map resent */

void MapRedrawCmd(char *buff, int len, player *pl)
{
    /* Okay, this is MAJOR UGLY. but the only way I know how to
     * clear the "cache"
     */
    memset(&pl->socket.lastmap, 0, sizeof(struct Map));
    draw_client_map(pl->ob);
}

void MapNewmapCmd( player *pl)
{
    /* we are really on a new map. tell it the client */
    send_mapstats_cmd(pl->ob, pl->ob->map); 
    memset(&pl->socket.lastmap, 0, sizeof(struct Map));
}



/* Moves and object (typically, container to inventory
 * move <to> <tag> <nrof> 
 */
void MoveCmd(char *buf, int len,player *pl)
{
    int vals[3], i;

    /* A little funky here.  We only cycle for 2 records, because
     * we obviously am not going to find a space after the third
     * record.  Perhaps we should just replace this with a
     * sscanf?
     */
    for (i=0; i<2; i++) {
	vals[i]=atoi(buf);
	if (!(buf = strchr(buf, ' '))) {
	    LOG(llevInfo,"CLIENT(BUG): Incomplete move command: %s from player %s\n", buf,query_name(pl->ob));
	    return;
	}
	buf++;
    }
    vals[2]=atoi(buf);

    esrv_move_object(pl->ob,vals[0], vals[1], vals[2]);
}



/******************************************************************************
 *
 * Start of commands the server sends to the client.
 *
 ******************************************************************************/

/*
 * send_query asks the client to query the user.  This way, the client knows
 * it needs to send something back (vs just printing out a message
 */
void send_query(NewSocket *ns, uint8 flags, char *text)
{
    char buf[MAX_BUF];

    sprintf(buf,"X%d %s", flags, text?text:"");
    Write_String_To_Socket(ns, BINARY_CMD_QUERY, buf, strlen(buf));
}


/* Sends the stats to the client - only sends them if they have changed */

#define AddIfInt(Old,New,Type) if (Old != New) {\
			Old = New; \
			SockList_AddChar(&sl, (char)(Type)); \
			SockList_AddInt(&sl, (uint32)(New)); \
		       }

#define AddIfShort(Old,New,Type) if (Old != New) {\
			Old = New; \
			SockList_AddChar(&sl, (char)(Type)); \
			SockList_AddShort(&sl, (uint16)(New)); \
		       }

#define AddIfChar(Old,New,Type) if (Old != New) {\
			Old = New; \
			SockList_AddChar(&sl, (char)(Type)); \
			SockList_AddChar(&sl, (char)(New)); \
		       }

#define AddIfFloat(Old,New,Type) if (Old != New) {\
			Old = New; \
			SockList_AddChar(&sl, (char)Type); \
			SockList_AddInt(&sl,(long)(New*FLOAT_MULTI));\
			}

#define AddIfString(Old,New,Type) if (Old == NULL || strcmp(Old,New)) {\
			if (Old) free(Old);\
	                Old = strdup_local(New);\
			SockList_AddChar(&sl, (char)Type); \
			SockList_AddChar(&sl, (char) strlen(New)); \
			strcpy((char*)sl.buf + sl.len, New); \
			sl.len += strlen(New); \
			}


void esrv_update_skills(player *pl)
{
    object *tmp2;
    int i;
    char buf[256];
    char tmp[2048]; /* we should careful set a big enough buffer here */
    
    sprintf(tmp,"X%d ", SPLIST_MODE_UPDATE);

    for(i=0;i<NROFSKILLS;i++)
    {
        /* update exp skill we have only */
        if(pl->skill_ptr[i] && pl->skill_ptr[i]->last_eat)
        {
            tmp2 = pl->skill_ptr[i];
            /* send only when really something has changed */
            if(tmp2->stats.exp != pl->skill_exp[i] || 
                tmp2->level != pl->skill_level[i])
            {
                sprintf(buf,"/%s|%d|%d",tmp2->name, tmp2->level, tmp2->stats.exp );
                strcat(tmp, buf);
                pl->skill_exp[i] = tmp2->stats.exp;
                pl->skill_level[i] = tmp2->level;
            }
        }
    }

    Write_String_To_Socket(&pl->socket, BINARY_CMD_SKILL_LIST ,tmp, strlen(tmp));        
    
}

/*
 * esrv_update_stats sends a statistics update.  We look at the old values,
 * and only send what has changed.  Stat mapping values are in newclient.h
 * Since this gets sent a lot, this is actually one of the few binary
 * commands for now.
 */
void esrv_update_stats(player *pl)
{
	static char sock_buf[MAX_BUF]; /* hm, in theory... can this all be more as 256 bytes?? *I* never tested it.*/
    SockList sl;
    int i;
    uint16 flags;

    sl.buf=sock_buf;
	SOCKET_SET_BINARY_CMD(&sl, BINARY_CMD_STATS);

	/* small trick: we want send the hp bar of our target to the player.
	 * We want send a char with x% the target has of full hp. 
	 * To avoid EVERY time the % calculation, we store the real HP
	 * - if it has changed, we calc the % and use them normal.
	 * this simple compare will not deal in speed but we safe 
	 * some unneeded calculations.
	 */
	if(pl->target_object != pl->ob) /* never send our own status - client will sort this out */
	{
		/* we don't care about count - target function will readjust itself */
		if(pl->target_object && pl->target_object->stats.hp != pl->target_hp) /* just for secure...*/
		{
			char hp =(char)(((float)pl->target_object->stats.hp/(float)pl->target_object->stats.maxhp)*100.0f);
			pl->target_hp = pl->target_object->stats.hp;
			AddIfChar(pl->target_hp_p, hp, CS_STAT_TARGET_HP);			
		}
	}

	AddIfShort(pl->last_gen_hp, pl->gen_client_hp, CS_STAT_REG_HP);
	AddIfShort(pl->last_gen_sp, pl->gen_client_sp, CS_STAT_REG_MANA);
	AddIfShort(pl->last_gen_grace,pl->gen_client_grace, CS_STAT_REG_GRACE);
	AddIfChar(pl->last_level, pl->ob->level, CS_STAT_LEVEL);
	AddIfFloat(pl->last_speed, pl->ob->speed, CS_STAT_SPEED);
	AddIfInt(pl->last_weight_limit, weight_limit[pl->ob->stats.Str], CS_STAT_WEIGHT_LIM);
	AddIfChar(pl->last_weapon_sp, pl->weapon_sp, CS_STAT_WEAP_SP);

    if(pl->ob != NULL)
    {
        AddIfInt(pl->last_stats.hp, pl->ob->stats.hp, CS_STAT_HP);
        AddIfInt(pl->last_stats.maxhp, pl->ob->stats.maxhp, CS_STAT_MAXHP);
        AddIfShort(pl->last_stats.sp, pl->ob->stats.sp, CS_STAT_SP);
        AddIfShort(pl->last_stats.maxsp, pl->ob->stats.maxsp, CS_STAT_MAXSP);
        AddIfShort(pl->last_stats.grace, pl->ob->stats.grace, CS_STAT_GRACE);
        AddIfShort(pl->last_stats.maxgrace, pl->ob->stats.maxgrace, CS_STAT_MAXGRACE);
        AddIfChar(pl->last_stats.Str, pl->ob->stats.Str, CS_STAT_STR);
        AddIfChar(pl->last_stats.Int, pl->ob->stats.Int, CS_STAT_INT);
        AddIfChar(pl->last_stats.Pow, pl->ob->stats.Pow, CS_STAT_POW);
        AddIfChar(pl->last_stats.Wis, pl->ob->stats.Wis, CS_STAT_WIS);
        AddIfChar(pl->last_stats.Dex, pl->ob->stats.Dex, CS_STAT_DEX);
        AddIfChar(pl->last_stats.Con, pl->ob->stats.Con, CS_STAT_CON);
        AddIfChar(pl->last_stats.Cha, pl->ob->stats.Cha, CS_STAT_CHA);

		AddIfInt(pl->last_stats.exp, pl->ob->stats.exp, CS_STAT_EXP);	
		AddIfShort(pl->last_stats.wc, pl->ob->stats.wc, CS_STAT_WC);
		AddIfShort(pl->last_stats.ac, pl->ob->stats.ac, CS_STAT_AC);
		AddIfShort(pl->last_stats.dam, pl->client_dam, CS_STAT_DAM);
		AddIfShort(pl->last_stats.food, pl->ob->stats.food, CS_STAT_FOOD);

    }
		
	for(i=0;i<pl->last_skill_index;i++)
    {
		AddIfInt(pl->last_skill_exp[i],pl->last_skill_ob[i]->stats.exp , pl->last_skill_id[i]);
		AddIfChar(pl->last_skill_level[i], (pl->last_skill_ob[i]->level), pl->last_skill_id[i]+1);
	}

    flags=0;
    if (pl->fire_on) flags |=SF_FIREON; /* TODO: remove fire and run server sided mode */
    if (pl->run_on) flags |= SF_RUNON;
    /* we add additional player status flags - in old style, you got a msg
     * in the text windows when you get xray of get blineded - we will skip
     * this and add the info here, so the client can track it down and make
     * it the user visible in it own, server indepentend way.
     */
 
    if(QUERY_FLAG(pl->ob,FLAG_BLIND)) /* player is blind */ 
        flags |=SF_BLIND;
    if(QUERY_FLAG(pl->ob,FLAG_XRAYS)) /* player has xray */ 
        flags |=SF_XRAYS;
    if(QUERY_FLAG(pl->ob,FLAG_SEE_IN_DARK )) /* player has infravision */ 
        flags |=SF_INFRAVISION;
    AddIfShort(pl->last_flags, flags, CS_STAT_FLAGS);

    for (i=0; i<NROFPROTECTIONS; i++)
        AddIfChar(pl->last_protection[i], pl->ob->protection[i], atnr_prot_stats[i]);

	if(pl->socket.ext_title_flag)
	{
		generate_ext_title(pl);
		AddIfString(pl->socket.stats.ext_title , pl->ext_title, CS_STAT_EXT_TITLE);
		pl->socket.ext_title_flag = 0;
	}
    /* Only send it away if we have some actual data */
    if (sl.len>1)
		Send_With_Handling(&pl->socket, &sl);
}


/* Tells the client that here is a player it should start using.
 */

void esrv_new_player(player *pl, uint32 weight)
{
    SockList	sl;

    sl.buf=malloc(MAXSOCKBUF);

	SOCKET_SET_BINARY_CMD(&sl, BINARY_CMD_PLAYER);
    SockList_AddInt(&sl, pl->ob->count);
    SockList_AddInt(&sl, weight);
    SockList_AddInt(&sl, pl->ob->face->number);

    SockList_AddChar(&sl, (char)strlen(pl->ob->name));
    strcpy((char*)sl.buf+sl.len, pl->ob->name);
    sl.len += strlen(pl->ob->name);
       
    Send_With_Handling(&pl->socket, &sl);
    free(sl.buf);
    SET_FLAG(pl->ob, FLAG_CLIENT_SENT);
}


/* Need to send an animation sequence to the client.
 * We will send appropriate face commands to the client if we haven't
 * sent them the face yet (this can become quite costly in terms of
 * how much we are sending - on the other hand, this should only happen
 * when the player logs in and picks stuff up.
 */
/* This function is not used - it was disabled with the client 
 * bmpa & anim cache patch (beta 2). This code is still here -
 * perhaps we use it later again - MT
 */
void esrv_send_animation(NewSocket *ns, short anim_num)
{
	/*
    SockList sl;
    int i;

	if (anim_num < 0 || anim_num > num_animations) {
	LOG(llevBug,"BUG: esrv_send_anim (%d) out of bounds?? (send:%d max:%d)\n",anim_num, anim_num, num_animations);
	return;
    }

    sl.buf = malloc(MAXSOCKBUF);
	SOCKET_SET_BINARY_CMD(&sl, BINARY_CMD_ANIM);
    SockList_AddShort(&sl, anim_num);
    SockList_AddChar(&sl, 0);  
    SockList_AddChar(&sl, animations[anim_num].facings);  

    for (i=0; i<animations[anim_num].num_animations; i++)
	{
		SockList_AddShort(&sl, animations[anim_num].faces[i]);
	}
    Send_With_Handling(ns, &sl);
    free(sl.buf);
    ns->anims_sent[anim_num] = 1;
	*/
}


/******************************************************************************
 *
 * Start of map related commands.
 *
 ******************************************************************************/

/* This adds face_num to a map cell at x,y.  If the client doesn't have
 * the face yet, we will also send it.
 */
/* not used */
/*
static void esrv_map_setbelow(NewSocket *ns, int x,int y,
			      short face_num, struct Map *newmap)
{

    if (x<0 || x>ns->mapx-1 || y<0 || y>ns->mapy-1 || face_num < 0 || face_num > MAXFACENUM)
		LOG(llevError,"ERROR: bad user x/y/facenum not in 0..10,0..10,0..%d\n", MAXFACENUM-1);
    if(newmap->cells[x][y].count >= MAP_LAYERS)
		LOG(llevError,"ERROR: Too many faces in map cell %d %d\n",x,y);
    newmap->cells[x][y].faces[newmap->cells[x][y].count] = face_num;
    newmap->cells[x][y].count ++;
}
*/
struct LayerCell {
  uint16 xy;
  short face;
};

struct MapLayer {
  int count;
  struct LayerCell lcells[MAP_CLIENT_X * MAP_CLIENT_Y];
};
/*
static int mapcellchanged(NewSocket *ns,int i,int j, struct Map *newmap)
{
  int k;

  if (ns->lastmap.cells[i][j].count != newmap->cells[i][j].count)
    return 1;
  for(k=0;k<newmap->cells[i][j].count;k++) {
    if (ns->lastmap.cells[i][j].faces[k] !=
	newmap->cells[i][j].faces[k]) {
      return 1;
    }
  }
  return 0;
}
*/

/* Clears a map cell */
static void map_clearcell(struct MapCell *cell)
{
	memset(cell, 0, sizeof(MapCell));
    cell->count=-1;
}

/* The problem to "personalize" a map view is that we have to access here the objects
 * we want to draw. This means alot of memory access in different areas. Iam not
 * sure about the speed increase when we put all this info in the map node. First, this
 * will be some static memory more per node. Second, we have to force to draw it for
 * every object... Here is some we can optimize, but it should happen very careful.
 */
/* this kind of map update is overused and outdated. We need for all this special stuff
 * to send the object ID to the client - and we use then the object ID to attach more data
 * when we update the object.
 */
void draw_client_map2(object *pl)
{
	static uint32 map2_count=0;
    struct MapCell *mp;
    New_Face	*face;
    mapstruct *m;
    object *tmp, *tmph, *pname1, *pname2, *pname3, *pname4;
    int x,y,ax, ay, d, nx,ny, probe_tmp;
    int dark, flag_tmp;
    int quick_pos_1,quick_pos_2,quick_pos_3; 
 	int inv_flag = QUERY_FLAG(pl,FLAG_SEE_INVISIBLE)?0:1;
    uint16 face_num0,face_num1,face_num2,face_num3,face_num1m,face_num2m,face_num3m;
    uint16  mask;
    SockList sl;
    
    map2_count++;      /* we need this to decide quickly we have updated a object before here */
    sl.buf=malloc(MAXSOCKBUF);
	SOCKET_SET_BINARY_CMD(&sl, BINARY_CMD_MAP2);
	/*
	strcpy((char*)sl.buf,"map2 ");
    sl.len=strlen((char*)sl.buf);
	*/
    /* control player map - if not as last_update, we have changed map.
     * Because this value will be set from normal map changing, but not from
     * border crossing from tiled maps - so we have done a step from a linked
     * tiled map to another - now we must give the client a hint so he can update
     * the mapname & cache */
    if(pl->map != pl->contr->last_update)
    {
        /* atm, we just tell him to move the map */
        SockList_AddChar(&sl, (char)255); /* marker */
        pl->contr->last_update = pl->map;
    }
    SockList_AddChar(&sl, (char)pl->x);
    SockList_AddChar(&sl, (char)pl->y);
    /* x,y are the real map locations.  ax, ay are viewport relative
     * locations.
     */    
    ay=pl->contr->socket.mapy-1;
    for(y=(pl->y+(pl->contr->socket.mapy+1)/2)-1; y>=pl->y-pl->contr->socket.mapy/2;y--,ay--) {
	ax=pl->contr->socket.mapx-1;
	for(x=(pl->x+(pl->contr->socket.mapx+1)/2)-1;x>=pl->x-pl->contr->socket.mapx/2;x--,ax--) {
	    d =  pl->contr->blocked_los[ax][ay];
	    mask = (ax & 0x1f) << 11 | (ay & 0x1f) << 6;

	    /* If the coordinates are not valid, or it is too dark to see,
	     * we tell the client as such
	     */
	    nx=x;
	    ny=y;
		m = out_of_map(pl->map, &nx, &ny);
	    if (!m) 
		{
			/* space is out of map.  Update space and clear values
			 * if this hasn't already been done.
			 */
			if (pl->contr->socket.lastmap.cells[ax][ay].count != -1) 
			{
				SockList_AddShort(&sl, mask);
				map_clearcell(&pl->contr->socket.lastmap.cells[ax][ay]);
			}
	    }
		else if ( d > 3 )
		{
			/* space is 'blocked' by darkness */
			if (d==4 && pl->contr->socket.darkness)
			{
				/* this is the first spot where darkness becomes too dark to see.
				 * only need to update this if it is different from what we 
				 * last sent
				 */
				if (pl->contr->socket.lastmap.cells[ax][ay].count != d) 
				{
					map_clearcell(&pl->contr->socket.lastmap.cells[ax][ay]);
					SockList_AddShort(&sl, mask);
					mask |= 0x10;  /* add darkness */
					SockList_AddShort(&sl, mask);
					SockList_AddChar(&sl, 0);
					pl->contr->socket.lastmap.cells[ax][ay].count = d;
				}
			}
			else if (pl->contr->socket.lastmap.cells[ax][ay].count != -1)
			{
				SockList_AddShort(&sl, mask);
				map_clearcell(&pl->contr->socket.lastmap.cells[ax][ay]);
			}
	    }
	    else 
		{ /* this space is viewable */
		MapSpace *msp;
        int pname_flag=0,ext_flag = 0, dmg_flag=0, oldlen = sl.len;
        int dmg_layer2=0,dmg_layer1=0,dmg_layer0=0;

        dark = NO_FACE_SEND;
	    mask = (ax & 0x1f) << 11 | (ay & 0x1f) << 6;

		/* Darkness changed */
		if (pl->contr->socket.lastmap.cells[ax][ay].count != d && pl->contr->socket.darkness) {
		    pl->contr->socket.lastmap.cells[ax][ay].count = d;
		    mask |= 0x10;    /* darkness bit */

		    /* Protocol defines 255 full bright, 0 full dark.
		     * We currently don't have that many darkness ranges,
		     * so we current what limited values we do have.
		     */
            if (d==0) dark = 255;
            else if (d==1) dark = 191;
            else if (d==2) dark = 127;
            else if (d==3) dark = 63;
        }
		else
		    /* need to reset from -1 so that if it does become blocked again,
		     * the code that deals with that can detect that it needs to tell
		     * the client that this space is now blocked.
		     */
		    pl->contr->socket.lastmap.cells[ax][ay].count = d;

        mp = &(pl->contr->socket.lastmap.cells[ax][ay]);
		msp = GET_MAP_SPACE_PTR(m, nx,ny);

		/* floor layer */
		face_num0 = 0;
		if(inv_flag)
			tmp = GET_MAP_SPACE_CL(msp,0);
		else
			tmp = GET_MAP_SPACE_CL_INV(msp,0);
		if(tmp)
			face_num0 = tmp->face->number ;
        if (mp->faces[3] != face_num0)
		{
   		    mask |= 0x8;    /* this is the floor layer - 0x8 is floor bit */

			if(tmp && tmp->type == PLAYER)
			{
				pname_flag |= 0x08; /* we have a player as object - send name too */
				pname1 = tmp;
			}

		    mp->faces[3] = face_num0;       /* this is our backbuffer which we control */
			
		}

		/* LAYER 2 */
        /* ok, lets explain on one layer what we do */
		/* First, we get us the normal or invisible layer view */
		if(inv_flag)
			tmp = GET_MAP_SPACE_CL(msp,1);
		else
			tmp = GET_MAP_SPACE_CL_INV(msp,1);
        probe_tmp = 0;

        if(tmp) /* now we have a valid object in this tile - NULL = there is nothing here */
        {
			flag_tmp =GET_CLIENT_FLAGS(tmp); 
			face = tmp->face;
			tmph = tmp;
            /* these are the red damage numbers, the client shows when we hit something */
            if((dmg_layer2 = tmp->last_damage) != -1 && tmp->damage_round_tag == ROUND_TAG)
                dmg_flag |= 0x4;

            quick_pos_1=tmp->quick_pos; /* thats our multi arch id and number in 8bits */
            if(quick_pos_1) /* if we have a multipart object */
            {
                if((tmph = tmp->head)) /* is a tail */
                {
                    /* if update_tag = map2_count, we have send a part of this
                     * in this map update some steps ago - skip it then */
                    if(tmp->head->update_tag == map2_count)
                        face =0; /* skip */
                    else
                    {
                        /* ok, mark this object as "send in this loop" */
                        tmp->head->update_tag = map2_count;
                        face = tmp->head->face;
                    }
                }
                else /* its head */
                {
                    /* again: if send before...*/
                    if(tmp->update_tag == map2_count)
                        face =0; /* then skip this time */
                    else
                    {
                        /* mark as send for other parts */
                        tmp->update_tag = map2_count;
                        face = tmp->face;
                    }
                }
            }                
        }
        else /* ok, its NULL object - but we need to update perhaps to clear something we had
			  * submited to the client before 
			  */
		{
			face = NULL;
            quick_pos_1 = 0; 
		}

         /* if we have no legal visual to send, skip it */
         if (!face || face == blank_face)
         {
             flag_tmp=0;probe_tmp=0;
             quick_pos_1=0;
             face_num1m=face_num1=0;
         }
         else
         {
            /* show target to player (this is personlized data)*/
            if(tmph && pl->contr->target_object_count == tmph->count)
            {
				flag_tmp|=FFLAG_PROBE;
                if(tmph->stats.hp)
                    probe_tmp = (int) ((double)tmph->stats.hp/((double)tmph->stats.maxhp/100.0));
                /* we don't carew about 0. If the client gots probe flag and value 0, he change it
                   to one... if some is here, it is alive, so what? 
                 */
            }
             /* face_num1 is original, face_num1m is a copy */
             face_num1m= face_num1 = face->number;
             /* if a monster, we mark face_num1m - this goes to client */
             if (tmp && QUERY_FLAG(tmp,FLAG_MONSTER) || tmp->type == PLAYER)
                    face_num1m|=0x8000;
         }
         /* ok - lets check we NEED to send this all to client */
         if (mp->faces[0] != face_num1 || mp->quick_pos[0] != quick_pos_1) {
   		    mask |= 0x4;    /* this is the floor layer - 0x4 is floor bit */
			if(tmp && tmp->type == PLAYER)
			{
				pname_flag |= 0x04; /* we have a player as object - send name too */
				pname2 = tmp;
			}
		    mp->faces[0] = face_num1;       /* this is our backbuffer which we control */
            mp->quick_pos[0] = quick_pos_1; /* what we have send to client before */
            if(quick_pos_1) /* if a multi arch */
                ext_flag |= 0x4; /* mark multi arch - we need this info when sending this layer to client*/
		}
        /* thats our extension flags! like blind, confusion, etc */
        /* extensions are compared to all sended map update data very rare */
        if(flag_tmp != mp->fflag[0] || probe_tmp != mp->ff_probe[0]) 
        {
            if(face_num1) /* the client delete the ext/probe values if face== 0 */
                ext_flag |= 0x20; /* floor ext flags */
			if(probe_tmp != mp->ff_probe[0] && flag_tmp&FFLAG_PROBE) /* ugly, but we must test it twice to submit implicit changes right */
				flag_tmp|=FFLAG_PROBE;
            mp->fflag[0] = flag_tmp;
            mp->ff_probe[0] = probe_tmp;
        }

        /* LAYER 1 */
		if(inv_flag)
			tmp = GET_MAP_SPACE_CL(msp,2);
		else
			tmp = GET_MAP_SPACE_CL_INV(msp,2);
       probe_tmp = 0;
        if(tmp)
        {
			/* Well, i have no idea how to send for each player his own face without this.
			* The way we can avoid this is to lets draw the player by the client
			* only and just to tell the client what direction and animation the player now
			* has... but Daimonin/CF can't handle client map animation atm... Even it should
			* not hard to be done. MT
			*/
			if(pl->x == nx && pl->y ==ny && tmp->layer == 6)
				tmp = pl;
			flag_tmp =GET_CLIENT_FLAGS(tmp); 
 			tmph=tmp;
			face = tmp->face;

            if((dmg_layer1 = tmp->last_damage) != -1 && tmp->damage_round_tag == ROUND_TAG)
                dmg_flag |= 0x2;
 
            quick_pos_2=tmp->quick_pos;
            if(quick_pos_2) /* if we have a multipart object */
            {
                if((tmph = tmp->head)) /* tail tile */
                {
                    if(tmp->head->update_tag == map2_count)
                        face =0; /* skip */
                    else
                    {
                        tmp->head->update_tag = map2_count;
                        face = tmp->head->face;
                    }
                }
                else /* a head */
                {
                    if(tmp->update_tag == map2_count)
                        face =0; /* we have send it this round before */
                    else
                    {
                        tmp->update_tag = map2_count;
                        face = tmp->face;
                    }
                }
            }               
        }
        else
		{
			face = NULL;
            quick_pos_2 = 0;
		}

        if (!face || face == blank_face) 
        {
            flag_tmp=0;probe_tmp=0;
            quick_pos_2 = 0;
            face_num2m=face_num2=0;
        }
        else
        {
            /* show target to player (this is personlized data)*/
				if(tmph && pl->contr->target_object_count == tmph->count)
				{
					flag_tmp|=FFLAG_PROBE;
					if(tmph->stats.hp)
						probe_tmp = (int) ((double)tmph->stats.hp/((double)tmph->stats.maxhp/100.0));
					/* we don't carew about 0. If the client gots probe flag and value 0, he change it
					to one... if some is here, it is alive, so what? 
					 */
				}
            face_num2m=face_num2 = face->number;
            if (tmp && QUERY_FLAG(tmp,FLAG_MONSTER) ||tmp->type == PLAYER)
                face_num2m|=0x8000;
        }
        
        if (mp->faces[1] != face_num2 || mp->quick_pos[1] != quick_pos_2) {
            
		    mask |= 0x2;    /* middle bit */
			if(tmp && tmp->type == PLAYER)
			{
				pname_flag |= 0x02; /* we have a player as object - send name too */
				pname3 = tmp;
			}
		    mp->faces[1] = face_num2;
            mp->quick_pos[1] = quick_pos_2;
            if(quick_pos_2) /* if a multi arch */
                ext_flag |= 0x2;
		}
        /* check, set and buffer ext flag */
        if(flag_tmp != mp->fflag[1] || probe_tmp != mp->ff_probe[1]) 
        {
            if(face_num2) /* the client delete the ext/probe values if face== 0 */
                ext_flag |= 0x10; /* floor ext flags */
			if(probe_tmp != mp->ff_probe[1] && flag_tmp&FFLAG_PROBE) /* ugly, but we must test it twice to submit implicit changes right */
				flag_tmp|=FFLAG_PROBE;
            mp->fflag[1] = flag_tmp;
            mp->ff_probe[1] = probe_tmp;
        }

		if(inv_flag)
			tmp = GET_MAP_SPACE_CL(msp,3);
		else
			tmp = GET_MAP_SPACE_CL_INV(msp,3);
       probe_tmp = 0;
        if(tmp)
        {
			if(pl->x == nx && pl->y ==ny && tmp->layer == 6)
				tmp = pl;
 			flag_tmp =GET_CLIENT_FLAGS(tmp); 
			face=tmp->face;
			tmph = tmp;
            if((dmg_layer0 = tmp->last_damage) != -1 && tmp->damage_round_tag == ROUND_TAG)
                dmg_flag |= 0x1;

            quick_pos_3=tmp->quick_pos;
            if(quick_pos_3) /* if we have a multipart object */
            {
                if((tmph = tmp->head)) /* tail tile */
                {
                    if(tmph->update_tag == map2_count)
                        face =0; /* skip */
                    else
                    {
                        tmph->update_tag = map2_count;
                        face = tmph->face;
                    }
                }
                else /* head */
                {
                    if(tmp->update_tag == map2_count)
                        face =0; /* we have send it this round before */
                    else
                    {
                        tmp->update_tag = map2_count;
                        face = tmp->face;
                    }
                }
            }
        }
        else
		{
			face = NULL;
            quick_pos_3 = 0;
		}

        if (!face || face == blank_face)
        {
            flag_tmp=0;probe_tmp=0;
            face_num3m=face_num3=0;
            quick_pos_3 = 0;
        }
        else
        {
            /* show target to player (this is personlized data)*/
				if(tmph && pl->contr->target_object_count == tmph->count)
				{
					flag_tmp|=FFLAG_PROBE;
					if(tmph->stats.hp)
						probe_tmp = (int) ((double)tmph->stats.hp/((double)tmph->stats.maxhp/(double)100.0));
					/* we don't carew about 0. If the client gots probe flag and value 0, he change it
					to one... if some is here, it is alive, so what? 
					 */
				}
            face_num3m=face_num3 = face->number;
            if (tmp && QUERY_FLAG(tmp,FLAG_MONSTER) ||tmp->type == PLAYER)
                face_num3m|=0x8000;
        }
   
        if (mp->faces[2] != face_num3 || mp->quick_pos[2] != quick_pos_3) {
		    mask |= 0x1;    /* top bit */
			if(tmp && tmp->type == PLAYER)
			{
				pname_flag |= 0x01; /* we have a player as object - send name too */
				pname4 = tmp;
			}
            if(quick_pos_3) /* if a multi arch */
                ext_flag |= 0x1;
            mp->faces[2] = face_num3;
            mp->quick_pos[2] = quick_pos_3;
		}
        /* check, set and buffer ext flag */
        if(flag_tmp != mp->fflag[2] || probe_tmp != mp->ff_probe[2]) 
        {
            if(face_num3) /* the client delete the ext/probe values if face== 0 */
                ext_flag |= 0x08; /* floor ext flags */
			if(probe_tmp != mp->ff_probe[2] && flag_tmp&FFLAG_PROBE) /* ugly, but we must test it twice to submit implicit changes right */
				flag_tmp|=FFLAG_PROBE;
            mp->fflag[2] = flag_tmp;
            mp->ff_probe[2] = probe_tmp;
        }
        
        /* perhaps we smashed some on this map position */
        /* object is gone but we catch the damage we have here done */
        if(GET_MAP_RTAG(m, nx,ny) == ROUND_TAG)
            dmg_flag |= 0x08; /* position (kill) damage */

        if(pname_flag)
			ext_flag |= 0x80; /* we have one or more player names in this map node*/ 
        if(dmg_flag)
			ext_flag |= 0x40; /* we have a dmg animation */

		if(ext_flag)
		{
            mask |= 0x20;    /* mark ext flag as valid */
	        SockList_AddShort(&sl, mask);
            SockList_AddChar(&sl, (char)ext_flag); /* push the ext_flagbyte */
		}
		else
		{
			SockList_AddShort(&sl, mask); /* mask only */
		}

        if(pname_flag)
		{
            SockList_AddChar(&sl, (char)pname_flag);
            if(pname_flag&0x08)
	            SockList_AddString(&sl, pname1->contr->quick_name);
            if(pname_flag&0x04)
	            SockList_AddString(&sl, pname2->contr->quick_name);
            if(pname_flag&0x02)
	            SockList_AddString(&sl, pname3->contr->quick_name);
            if(pname_flag&0x01)
	            SockList_AddString(&sl, pname4->contr->quick_name);

		}

        /* fire & forget layer animation tags */
       if(dmg_flag)
        {
            /* LOG(llevDebug,"Send dmg_flag(%d) (%x): %x (%d %d %d)\n", count++, mask,dmg_flag,dmg_layer2,dmg_layer1,dmg_layer0); */
            SockList_AddChar(&sl, (char)dmg_flag);
            /*thats the special one - the red kill spot the client shows */
            /* remember we put the damage value in the map because at the time
             * we are here at run time, the object is dead since some ticks and
             * perhaps some else is moved on this spot and/or the old object deleted */
            if(dmg_flag&0x08)
			{
                SockList_AddShort(&sl, (sint16)GET_MAP_DAMAGE(m, nx,ny));
			}
            if(dmg_flag&0x04)
			{
                SockList_AddShort(&sl, (sint16)dmg_layer2);
			}
            if(dmg_flag&0x02)
			{
                SockList_AddShort(&sl, (sint16)dmg_layer1);
			}
            if(dmg_flag&0x01)
			{
                SockList_AddShort(&sl, (sint16)dmg_layer0);
			}
        }

        
        /* client additional layer animations */
        if(ext_flag & 0x38)
        {
            if(ext_flag&0x20)
            {
                SockList_AddChar(&sl, (char)mp->fflag[0]);
                if(mp->fflag[0] & FFLAG_PROBE)
				{
                    SockList_AddChar(&sl, mp->ff_probe[0]);
				}
                
            }
            if(ext_flag&0x10)
            {
                SockList_AddChar(&sl, (char)mp->fflag[1]);
                if(mp->fflag[1] & FFLAG_PROBE)
				{
                    SockList_AddChar(&sl, mp->ff_probe[1]);
				}
            }
            if(ext_flag&0x08)
            {
                SockList_AddChar(&sl, (char)mp->fflag[2]); /* and all the face flags if there */
                if(mp->fflag[2] & FFLAG_PROBE)
				{
                    SockList_AddChar(&sl, mp->ff_probe[2]);
				}
            }
        }

        if(dark != NO_FACE_SEND)
		{
            SockList_AddChar(&sl, (char)dark);
		}
        if(mask & 0x08)
		{
            SockList_AddShort(&sl, face_num0);
		}
        if(mask & 0x04)
        {
            SockList_AddShort(&sl, face_num1m);
            if(ext_flag & 0x4)
			{
                SockList_AddChar(&sl, (char)quick_pos_1);
			}
        }
        if(mask & 0x02)
        {
            SockList_AddShort(&sl, face_num2m);
            if(ext_flag & 0x2)
			{
                SockList_AddChar(&sl, (char) quick_pos_2);
			}
        }
        if(mask & 0x01)
        {
            SockList_AddShort(&sl, face_num3m);
            if(ext_flag & 0x1)
			{
                SockList_AddChar(&sl, (char)quick_pos_3);
			}
        }

       if (!(mask & 0x3f)) /* check all bits except the position */
        sl.len = oldlen;

	    }
	} /* for x loop */
    } /* for y loop */
		    
    /* Verify that we in fact do need to send this */
    if (sl.len>1 || pl->contr->socket.sent_scroll) {
	Send_With_Handling(&pl->contr->socket, &sl);
	pl->contr->socket.sent_scroll = 0;
    }
    free(sl.buf);
}


void draw_client_map(object *pl)
{
    mapstruct	*m;
    int i,j,ax,ay; /* ax and ay goes from 0 to max-size of arrays */
    struct Map	newmap;

    if (pl->type != PLAYER) {
	LOG(llevBug,"BUG: draw_client_map called with non player/non eric-server (%s)\n",pl->name);
	return;
    }

    /* IF player is just joining the game, he isn't on a map,
	 * If so, don't try to send them a map.  All will
     * be OK once they really log in.
     */
    if (!pl->map || pl->map->in_memory!=MAP_IN_MEMORY) return;

	/* if we has changed somewhere the map - tell it the client now */
	if(pl->contr->last_update != pl->map)
		MapNewmapCmd( pl->contr);

    memset(&newmap, 0, sizeof(struct Map));

    for(j=(pl->y-pl->contr->socket.mapy/2); j<(pl->y+(pl->contr->socket.mapy+1)/2); j++)
	{
        for(i=(pl->x-pl->contr->socket.mapx/2) ; i<(pl->x+(pl->contr->socket.mapx+1)/2); i++)
		{
			ax=i;
			ay=j;
			m = out_of_map(pl->map, &ax, &ay);
			if (m && (GET_MAP_FLAGS(m,ax,ay) & P_NEED_UPDATE))
				update_position(m, ax, ay);
		}
    }

    /* do LOS after calls to update_position */
    if(pl->contr->do_los) 
	{
        update_los(pl);
        pl->contr->do_los = 0;
    }
    
	/* if (pl->contr->socket.map2cmd) */ /* its default now */        
	draw_client_map2(pl);
}


void esrv_map_scroll(NewSocket *ns,int dx,int dy)
{
    struct Map newmap;
    int x,y;
    char buf[MAXSOCKBUF];

    sprintf(buf,"X%d %d", dx, dy);
    Write_String_To_Socket(ns, BINARY_CMD_MAP_SCROLL, buf, strlen(buf));
    /* the x and y here are coordinates for the new map, i.e. if we moved
     (dx,dy), newmap[x][y] = oldmap[x-dx][y-dy] */
    for(x=0;x<ns->mapx;x++) {
	for(y=0;y<ns->mapy;y++) {
	    if (x+dx < 0 || x+dx >= ns->mapx || y+dy < 0 || y+dy >= ns->mapy) {
		memset(&(newmap.cells[x][y]), 0, sizeof(struct MapCell));
		continue;
	    }
	    memcpy(&(newmap.cells[x][y]),
		   &(ns->lastmap.cells[x+dx][y+dy]),sizeof(struct MapCell));
	}
    }
    memcpy(&(ns->lastmap), &newmap,sizeof(struct Map));
    ns->sent_scroll = 1;
}

/*****************************************************************************/
/* GROS: The following one is used to allow a plugin to send a generic cmd to*/
/* a player. Of course, the client need to know the command to be able to    */
/* manage it !                                                               */
/*****************************************************************************/
void send_plugin_custom_message(object *pl, char *buf)
{
	/* we must add here binary_cmd! */
    /*Write_String_To_Socket(&pl->contr->socket,buf,strlen(buf));*/
}

