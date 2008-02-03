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

	The author can be reached via e-mail to info@daimonin.net
*/

#include <global.h>
#include "../zlib/zlib.h"

/* as long the server don't have a autoupdate/login server
 * as frontend we must serve our depending client files self.
 */
static void load_srv_files(char *fname, int id, int cmd)
{
    FILE   *fp;
    unsigned char *file_tmp, *comp_tmp;
    int     flen;
    unsigned long numread;
    struct stat statbuf;

    LOG(llevDebug, "Loading %s...", fname);
    if ((fp = fopen(fname, "rb")) == NULL)
        LOG(llevError, "\nERROR: Can not open file %s\n", fname);
    fstat(fileno(fp), &statbuf);
    flen = (int) statbuf.st_size;
    file_tmp = malloc(flen);
    numread = (unsigned long) fread(file_tmp, sizeof(char), flen, fp);
    /* get a crc from the unpacked file */
    SrvClientFiles[id].crc = crc32(1L, file_tmp, numread);
    SrvClientFiles[id].len_ucomp = numread;
    numread = flen * 2;
    comp_tmp = (unsigned char *) malloc(numread);
    compress2(comp_tmp, &numread, file_tmp, flen, Z_BEST_COMPRESSION);
    /* we prepare the files with the right commands - so we can flush
     * then direct from this buffer to the client.
     */
    if ((int) numread < flen)
    {
        /* copy the compressed file in the right buffer */
        SrvClientFiles[id].file = malloc(numread + 2);
        memcpy(SrvClientFiles[id].file + 2, comp_tmp, numread);
        SrvClientFiles[id].file[1] = (char) DATA_PACKED_CMD;
        SrvClientFiles[id].len = numread;
    }
    else
    {
        /* compress has no positive effect here */
        SrvClientFiles[id].file = malloc(flen + 2);
        memcpy(SrvClientFiles[id].file + 2, file_tmp, flen);
        SrvClientFiles[id].file[1] = 0;
        SrvClientFiles[id].len = -1;
        numread = flen;
    }
    SrvClientFiles[id].file[0] = BINARY_CMD_DATA;
    SrvClientFiles[id].file[1] |= cmd;
    free(file_tmp);
    free(comp_tmp);

    LOG(llevDebug, "(size: %d (%d) (crc uncomp.: %x)\n", SrvClientFiles[id].len_ucomp, numread, SrvClientFiles[id].crc);
    fclose(fp);
}

/* get the /lib/settings default file and create the
 * /data/client_settings with it.
 */
static void create_client_settings(void)
{
    char    buf[MAX_BUF*4];
    int     i;
    FILE   *fset_default, *fset_create;

    LOG(llevDebug, "Creating %s/client_settings...\n", settings.localdir);

    /* open default */
    sprintf(buf, "%s/client_settings", settings.datadir);
    if ((fset_default = fopen(buf, "rb")) == NULL)
        LOG(llevError, "\nERROR: Can not open file %s\n", STRING_SAFE(buf));

    /* delete our target - we create it new now */
    sprintf(buf, "%s/client_settings", settings.localdir);
    unlink(buf);

    /* open target client_settings */
    if ((fset_create = fopen(buf, "wb")) == NULL)
    {
        fclose(fset_default);
        LOG(llevError, "\nERROR: Can not open file %s\n", STRING_SAFE(buf));
    }

    /* copy default to target */
    while (fgets(buf, MAX_BUF, fset_default) != NULL)
        fputs(buf, fset_create);
    fclose(fset_default);

    /* now we add the server specific date
     * first: the exp levels!
    */
    sprintf(buf, "level %d\n", MAXLEVEL); /* param: number of levels we have */
    fputs(buf, fset_create);

    for (i = 0; i <= MAXLEVEL; i++)
    {
        sprintf(buf, "%x\n", new_levels[i]);
        fputs(buf, fset_create);
    }

    fclose(fset_create);
}

/* load all src_files we can send to client... client_bmaps is generated from
 * the server at startup out of the daimonin png file.
 */
void init_srv_files(void)
{
    char    buf[MAX_BUF];

    memset(&SrvClientFiles, 0, sizeof(SrvClientFiles));

    sprintf(buf, "%s/animations", settings.datadir);
    load_srv_files(buf, SRV_CLIENT_ANIMS, DATA_CMD_ANIM_LIST);

    sprintf(buf, "%s/client_bmaps", settings.localdir);
    load_srv_files(buf, SRV_CLIENT_BMAPS, DATA_CMD_BMAP_LIST);

    sprintf(buf, "%s/client_skills", settings.datadir);
    load_srv_files(buf, SRV_CLIENT_SKILLS, DATA_CMD_SKILL_LIST);

    sprintf(buf, "%s/client_spells", settings.datadir);
    load_srv_files(buf, SRV_CLIENT_SPELLS, DATA_CMD_SPELL_LIST);

    create_client_settings();
    sprintf(buf, "%s/client_settings", settings.localdir);
    load_srv_files(buf, SRV_CLIENT_SETTINGS, DATA_CMD_SETTINGS_LIST);
}

/* a connecting client has requested a srv_ file.
 * not that we don't know anything about the player
 * at this point - we got a open socket, a IP a matching
 * version and a usable setup string from the client.
 */
void send_srv_file(NewSocket *ns, int id)
{
    SockList    sl;

    sl.buf = (unsigned char *)SrvClientFiles[id].file;

    if (SrvClientFiles[id].len != -1)
        sl.len = SrvClientFiles[id].len + 2;
    else
        sl.len = SrvClientFiles[id].len_ucomp + 2;

    Send_With_Handling(ns, &sl);
}
