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

/* socket.c mainly deals with initialization and higher level socket
 * maintenance (checking for lost connections and if data has arrived.)
 * The reading of data is handled in ericserver.c
 */


#include <global.h>
#ifndef __CEXTRACT__
#include <sproto.h>
#endif
#ifndef WIN32 /* ---win32 exclude include files */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <stdio.h>
#endif /* win32 */

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#include <newserver.h>
#include "zlib.h"

_srv_client_files SrvClientFiles[SRV_CLIENT_FILES];

Socket_Info socket_info;
NewSocket *init_sockets;


/* Initializes a connection - really, it just sets up the data structure,
 * socket setup is handled elsewhere.  We do send a version to the
 * client.
 */
void InitConnection(NewSocket *ns, uint32 from)
{
    SockList sl;
    unsigned char buf[256];
    int	bufsize=SOCKETBUFSIZE;
    int oldbufsize;
    int buflen=sizeof(int);

#ifdef WIN32 /* ***WIN32 SOCKET: init win32 non blocking socket */
	int temp = 1;	

	if(ioctlsocket(ns->fd, FIONBIO , &temp) == -1)
		LOG(llevDebug,"InitConnection:  Error on ioctlsocket.\n");
#else 
    if (fcntl(ns->fd, F_SETFL, O_NDELAY)==-1) {
		LOG(llevDebug,"InitConnection:  Error on fcntl.\n");
    }
#endif /* end win32 */

    if (getsockopt(ns->fd,SOL_SOCKET,SO_SNDBUF, (char*)&oldbufsize, &buflen)==-1)
	oldbufsize=0;
    if (oldbufsize<bufsize) {
	/*LOG(llevDebug, "InitConnection: Default buffer size was %d bytes, will reset it to %d\n", oldbufsize, bufsize);*/
	if(setsockopt(ns->fd,SOL_SOCKET,SO_SNDBUF, (char*)&bufsize, sizeof(&bufsize))) {
	    LOG(llevDebug,"InitConnection: setsockopt unable to set output buf size to %d\n", bufsize);
	}
    }
    buflen=sizeof(oldbufsize);
    getsockopt(ns->fd,SOL_SOCKET,SO_SNDBUF, (char*)&oldbufsize, &buflen);
#ifdef ESRV_DEBUG
    LOG(llevDebug, "InitConnection: Socket buffer size now %d bytes\n", oldbufsize);
#endif

    ns->faceset = 0;
    ns->facecache = 0;
    ns->image2 = 0;
    ns->sound = 0;
    ns->ext_title_flag = 1;
    ns->map2cmd = 0;
    ns->darkness = 1;
    ns->status = Ns_Add;
    ns->mapx = 11;
    ns->mapy = 11;
	ns->version = 0;
	ns->setup = 0;
	ns->rf_settings = 0;
	ns->rf_skills = 0;
	ns->rf_spells = 0;
	ns->rf_anims = 0;
	ns->rf_bmaps = 0;

    /* we should really do some checking here - if total clients overflows
     * we need to do something more intelligent, because client id's will start
     * duplicating (not likely in normal cases, but malicous attacks that
     * just open and close connections could get this total up.
     */
    ns->inbuf.len=0;
    ns->inbuf.buf=malloc(MAXSOCKBUF);
    /* Basic initialization. Needed because we do a check in
     * HandleClient for oldsocketmode without checking the
     * length of data.
     */
    ns->inbuf.buf[0] = 0;
    memset(&ns->lastmap,0,sizeof(struct Map));
    memset(&ns->stats,0,sizeof(struct statsinfo));
    /* Do this so we don't send a face command for the client for
     * this face.  Face 0 is sent to the client to say clear
     * face information.
     */
    /*ns->faces_sent[0] = 1;*/

    ns->outputbuffer.start=0;
    ns->outputbuffer.len=0;
    ns->can_write=1;

    ns->sent_scroll=0;
    sprintf((char*)buf,"%d.%d.%d.%d",
          (from>>24)&255, (from>>16)&255, (from>>8)&255, from&255);
    ns->host=strdup_local((char*)buf);
    sprintf((char*)buf, "X%d %d %s\n", VERSION_CS,VERSION_SC, VERSION_INFO);
	buf[0]=BINARY_CMD_VERSION;
    sl.buf=buf;
    sl.len=strlen((char*)buf);
    Send_With_Handling(ns, &sl);
#ifdef CS_LOGSTATS
    if (socket_info.nconns>cst_tot.max_conn)
	cst_tot.max_conn = socket_info.nconns;
    if (socket_info.nconns>cst_lst.max_conn)
	cst_lst.max_conn = socket_info.nconns;
#endif
}


/* This sets up the socket and reads all the image information into memory. */
void init_ericserver()
{
    struct sockaddr_in	insock;
    struct protoent  *protox;
    struct linger linger_opt;

#ifdef WIN32 /* ***win32  -  we init a windows socket */
	WSADATA w;

	socket_info.max_filedescriptor = 1;	/* used in select, ignored in winsockets */
	WSAStartup (0x0101,&w);				/* this setup all socket stuff */
	/* ill include no error tests here, winsocket 1.1 should always work */
	/* except some old win95 versions without tcp/ip stack */
#else	/* non windows */

#ifdef HAVE_SYSCONF
  socket_info.max_filedescriptor = sysconf(_SC_OPEN_MAX);
#else
#  ifdef HAVE_GETDTABLESIZE
  socket_info.max_filedescriptor = getdtablesize();
#  else
  "Unable to find usable function to get max filedescriptors";
#  endif
#endif
#endif /* win32 */

    socket_info.timeout.tv_sec = 0;
    socket_info.timeout.tv_usec = 0;
    socket_info.nconns=0;

#ifdef CS_LOGSTATS
    memset(&cst_tot, 0, sizeof(CS_Stats));
    memset(&cst_lst, 0, sizeof(CS_Stats));
    cst_tot.time_start=time(NULL);
    cst_lst.time_start=time(NULL);
#endif

    LOG(llevDebug,"Initialize new client/server data\n");
    socket_info.nconns = 1;
    init_sockets = malloc(sizeof(NewSocket));
    socket_info.allocated_sockets=1;

    protox = getprotobyname("tcp");
    if (protox==NULL) 
	{
		LOG(llevBug,"BUG: init_ericserver: Error getting protox\n");
		return;
    }
    init_sockets[0].fd = socket(PF_INET, SOCK_STREAM, protox->p_proto);
    if (init_sockets[0].fd == -1)
		LOG(llevError, "ERROR: Error creating socket on port\n");
    insock.sin_family = AF_INET;
    insock.sin_port = htons(settings.csport);
    insock.sin_addr.s_addr = htonl(INADDR_ANY);

    linger_opt.l_onoff = 0;
    linger_opt.l_linger = 0;
    if(setsockopt(init_sockets[0].fd,SOL_SOCKET,SO_LINGER,(char *) &linger_opt,sizeof(struct linger)))
	{
		LOG(llevBug, "BUG: Error on setsockopt LINGER\n");
    }
/* Would be nice to have an autoconf check for this.  It appears that
 * these functions are both using the same calling syntax, just one
 * of them needs extra valus passed.
 */
#if !defined(_WEIRD_OS_) /* means is true for most (win32, linux, etc. ) */
    {
		int tmp =1;

		if(setsockopt(init_sockets[0].fd,SOL_SOCKET,SO_REUSEADDR, (char *)&tmp, sizeof(tmp)))
			LOG(llevDebug,"error on setsockopt REUSEADDR\n");
    }
#else
    if(setsockopt(init_sockets[0].fd,SOL_SOCKET,SO_REUSEADDR,(char *)NULL,0))
		LOG(llevDebug,"error on setsockopt REUSEADDR\n");
    
#endif

    if (bind(init_sockets[0].fd,(struct sockaddr *)&insock,sizeof(insock)) == (-1)) {
#ifdef WIN32 /* ***win32: close() -> closesocket() */
	shutdown(init_sockets[0].fd,SD_BOTH);
	closesocket(init_sockets[0].fd);
#else
	close(init_sockets[0].fd);
#endif /* win32 */
	LOG(llevError,"error on bind command.\n");
    }
    if (listen(init_sockets[0].fd,5) == (-1))  {
#ifdef WIN32 /* ***win32: close() -> closesocket() */
	shutdown(init_sockets[0].fd,SD_BOTH);
	closesocket(init_sockets[0].fd);
#else
	close(init_sockets[0].fd);
#endif /* win32 */
	LOG(llevError,"error on listen\n");
    }
    init_sockets[0].status=Ns_Add;
    read_client_images();
	init_srv_files(); /* load all srv_xxx files or generate them */

}


/*******************************************************************************
 *
 * Start of functions dealing with freeing of the data.
 *
 ******************************************************************************/

/* Free's all the memory that ericserver allocates. */
void free_all_newserver()
{  
    LOG(llevDebug,"Freeing all new client/server information.\n");
    free_socket_images();
    free(init_sockets);
}

/* basically, all we need to do here is free all data structures that
 * might be associated with the socket.  It is up to the caller to
 * update the list
 */

void free_newsocket(NewSocket *ns)
{
#ifdef WIN32 /* ***win32: closesocket in windows style */
	shutdown(ns->fd,SD_BOTH);
    if (closesocket(ns->fd)) {
#else
    if (close(ns->fd)) {
#endif /* win32 */

#ifdef ESRV_DEBUG
	LOG(llevDebug,"Error closing socket %d\n", ns->fd);
#endif
    }
    if (ns->stats.range)
		free(ns->stats.range);
    if (ns->stats.ext_title)
        free(ns->stats.ext_title);
    if (ns->stats.title)
        free(ns->stats.title);
	if(ns->host)
	    free(ns->host);
    if(ns->inbuf.buf);
	    free(ns->inbuf.buf);
		memset(ns,0,sizeof(ns));
}

void final_free_player(player *pl)
{
	/*
	char buf[2]="X";
    Write_String_To_Socket(&pl->socket, BINARY_CMD_BYE, buf, 1);*/
    free_newsocket(&pl->socket);
    free_player(pl);
}

/* as long the server don't have a autoupdate/login server
 * as frontend we must serve our depending client files self.
 */
static void load_srv_files(char *fname, int id, int cmd)
{
    FILE *fp;
	char *file_tmp, *comp_tmp;
	int flen;
	unsigned long numread;
	struct stat statbuf;

    LOG(llevDebug,"Loading %s...", fname);
    if ((fp=fopen(fname,"rb")) ==NULL)
		LOG(llevError,"\nERROR: Can not open file %s\n", fname);
	fstat (fileno (fp), &statbuf);
	flen = (int) statbuf.st_size;
	file_tmp = malloc(flen);
	numread = (unsigned long) fread(file_tmp, sizeof( char ), flen, fp );
	/* get a crc from the unpacked file */
	SrvClientFiles[id].crc = adler32(numread, file_tmp, numread); 
	SrvClientFiles[id].len_ucomp = numread;
	numread=flen*2;
	comp_tmp = (char*)malloc(numread);
	compress2 (comp_tmp, &numread, file_tmp, flen, Z_BEST_COMPRESSION);
	/* we prepare the files with the right commands - so we can flush
	 * then direct from this buffer to the client.
	 */
	if((int)numread < flen)
	{
		/* copy the compressed file in the right buffer */
		SrvClientFiles[id].file = malloc(numread+2);
		memcpy(SrvClientFiles[id].file+2, comp_tmp,numread);
		SrvClientFiles[id].file[1] = (char)DATA_PACKED_CMD;
		SrvClientFiles[id].len = numread;
	}
	else
	{
		/* compress has no positive effect here */
		SrvClientFiles[id].file = malloc(flen+2);
		memcpy(SrvClientFiles[id].file+2, file_tmp,flen);
		SrvClientFiles[id].file[1] = 0;
		SrvClientFiles[id].len = -1;
		numread = flen;
	}
	SrvClientFiles[id].file[0] = BINARY_CMD_DATA;
	SrvClientFiles[id].file[1] |= cmd;
	free(file_tmp);
	free(comp_tmp);

	LOG(llevDebug,"(size: %d (crc: %x)\n", numread, SrvClientFiles[id].crc);
    fclose(fp);
}

/* get the /lib/settings default file and create the
 * /data/client_settings with it.
 */
static void create_client_settings(void)
{
    char buf[MAX_BUF*4];
	int i;
	FILE *fset_default,*fset_create;

	LOG(llevDebug,"Creating %s/client_settings...\n", settings.localdir);

    /* open default */
	sprintf(buf,"%s/client_settings", settings.datadir);
    if ((fset_default=fopen(buf,"rb")) ==NULL)
		LOG(llevError,"\nERROR: Can not open file %s\n", buf);

	/* delete our target - we create it new now */
    sprintf(buf,"%s/client_settings", settings.localdir);
	unlink(buf);

	/* open target client_settings */
    if ((fset_create=fopen(buf,"wb")) ==NULL)
	{
		fclose(fset_default);
		LOG(llevError,"\nERROR: Can not open file %s\n", buf);
	}

	/* copy default to target */
	while(fgets(buf,MAX_BUF,fset_default)!=NULL) 
		fputs(buf,fset_create);
	fclose(fset_default);
	
	/* now we add the server specific date 
	 * first: the exp levels! 
	*/
	sprintf(buf,"level %d\n", MAXLEVEL); /* param: number of levels we have */
	fputs(buf,fset_create);

	for(i=0;i<=MAXLEVEL;i++)
	{
		sprintf(buf, "%x\n", new_levels[i]);
		fputs(buf,fset_create);
	}

	fclose(fset_create);
}

/* load all src_files we can send to client... client_bmaps is generated from 
 * the server at startup out of the daimonin png file.
 */
void init_srv_files(void)
{
    char buf[MAX_BUF];

	memset(&SrvClientFiles,0, sizeof(SrvClientFiles));  

    sprintf(buf,"%s/animations", settings.datadir);
	load_srv_files(buf, SRV_CLIENT_ANIMS,DATA_CMD_ANIM_LIST);

    sprintf(buf,"%s/client_bmaps", settings.localdir);
	load_srv_files(buf, SRV_CLIENT_BMAPS,DATA_CMD_BMAP_LIST);

    sprintf(buf,"%s/client_skills", settings.datadir);
	load_srv_files(buf, SRV_CLIENT_SKILLS,DATA_CMD_SKILL_LIST);

    sprintf(buf,"%s/client_spells", settings.datadir);
	load_srv_files(buf, SRV_CLIENT_SPELLS,DATA_CMD_SPELL_LIST);

	create_client_settings();
    sprintf(buf,"%s/client_settings", settings.localdir);
	load_srv_files(buf, SRV_CLIENT_SETTINGS,DATA_CMD_SETTINGS_LIST);
}

/* a connecting client has requested a srv_ file.
 * not that we don't know anything about the player
 * at this point - we got a open socket, a IP a matching
 * version and a usable setup string from the client.
 */
void send_srv_file(NewSocket *ns, int id)
{
	SockList sl;

	sl.buf = SrvClientFiles[id].file;

	if(SrvClientFiles[id].len != -1)
	    sl.len = SrvClientFiles[id].len+2;
	else
	    sl.len = SrvClientFiles[id].len_ucomp+2;

    Send_With_Handling(ns, &sl);

}
