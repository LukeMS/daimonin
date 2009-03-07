/*
    Daimonin, the Massive Multiuser Online Role Playing Game
    Server Applicatiom

    Copyright (C) 2001-2008 Michael Toennies

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

player_arch_template        player_arch_list[MAX_PLAYER_ARCH];

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

	LOG(llevDebug, "Loading %s (id:%d)...", fname, id);
    if ((fp = fopen(fname, "rb")) == NULL)
        LOG(llevError, "\nERROR: Can not open file %s\n", fname);
    fstat(fileno(fp), &statbuf);
    flen = (int) statbuf.st_size;
    file_tmp = malloc(flen+1);
    numread = (unsigned long) fread(file_tmp+1, sizeof(char), flen, fp);
    /* get a crc from the unpacked file */
    SrvClientFiles[id].crc = crc32(1L, file_tmp+1, numread);
    SrvClientFiles[id].len_ucomp = numread;
    numread = flen * 2;
    comp_tmp = (unsigned char *) malloc(numread);
    numread-=4;
    compress2(comp_tmp+5, &numread, file_tmp+1, flen, Z_BEST_COMPRESSION);


    if (numread>(0xFFFFFFFF))
        LOG(llevError, "\nERROR: Size of compressed file %s exceeds size of uint32\nload_srv_files in startup.c needs update!\n", fname);

    /* we prepare the files with the right commands - so we can flush
     * then direct from this buffer to the client.
     */
    if ((int) numread < flen)
    {
        /* copy the compressed file in the right buffer */
		*comp_tmp = (char) (cmd|DATA_PACKED_CMD);
		*((uint32 *)(comp_tmp+1))=(uint32)(flen); //we send as uint32 the uncompressed filesize
		SrvClientFiles[id].sockbuf = SOCKBUF_COMPOSE( BINARY_CMD_DATA, NULL, (char *)comp_tmp, numread+5, SOCKBUF_FLAG_STATIC);
		SrvClientFiles[id].len = numread+5;
    }
    else
    {
		/* compress has no positive effect here */
		*file_tmp = (char) cmd;
		SrvClientFiles[id].sockbuf = SOCKBUF_COMPOSE(BINARY_CMD_DATA, NULL, (char *)file_tmp, flen+1, SOCKBUF_FLAG_STATIC);
		SrvClientFiles[id].len = -1;
		numread = flen+1;
    }
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
    archetype   *p_arch;
    char        buf[MAX_BUF*4], arch_name[64];
    int         i, line=0, id, race=0, gender = 0;
    FILE        *fset_default, *fset_create;

    LOG(llevDebug, "Creating %s/client_settings...\n", settings.localdir);

    /* used by create_player() as default template */
    memset(player_arch_list, 0 , sizeof(player_arch_template));

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
    {
        if(buf[0] == '#')
            fputs(buf, fset_create);
        else
        {
            /* the file format for the settings is fixed and MUST fit - so we use a line counter for the entries */
            if(line < 2 || line >6) /* just flush the face defines & the description */
            {

            }
            else if(line >= 2 && line <= 5) /* thats the player arch lines */
            {
                sscanf(buf, "%d %s %*s\n", &id, arch_name);
                if(id) /* 0 marks an invalid entry */
                {

                    if (!(p_arch = find_archetype(arch_name)) || p_arch->clone.type != PLAYER)
                    {
                        /* global error will stop the server */
                        LOG(llevError, "Error: invalid player arch in client_settings race %d line %d!\n", race, line);
                    }
                    player_arch_list[race].p_arch[gender++] = p_arch; /* we just copy the clone here later */
                }
            }
            else if (line == 6) /* thats the default start values */
            {
                sscanf(buf, "%d %d %d %d %d %d %d\n",
                &player_arch_list[race].str, &player_arch_list[race].dex,
                &player_arch_list[race].con, &player_arch_list[race].intel,
                &player_arch_list[race].wis, &player_arch_list[race].pow, &player_arch_list[race].cha);
                race++;
                gender = 0;
            }

            fputs(buf, fset_create);
            if( line++ >= 10)
                line = 0;
        }
    }
    fclose(fset_default);

    settings.player_races = race; /* number of loaded player race templates */

    /* flush the loaded info to LOG so we can see what we have */
    LOG(llevInfo, "Loaded %d player race templates:\n", settings.player_races);
    for(line=0; line < race;line++)
        LOG(llevInfo, "%s %s %s %s stats: %d %d %d %d %d %d %d\n",
        player_arch_list[line].p_arch[0]?player_arch_list[line].p_arch[0]->clone.name:"NULL",
        player_arch_list[line].p_arch[1]?player_arch_list[line].p_arch[1]->clone.name:"NULL",
        player_arch_list[line].p_arch[2]?player_arch_list[line].p_arch[2]->clone.name:"NULL",
        player_arch_list[line].p_arch[3]?player_arch_list[line].p_arch[3]->clone.name:"NULL",
        player_arch_list[line].str, player_arch_list[line].dex,
        player_arch_list[line].con, player_arch_list[line].intel, player_arch_list[line].wis,
        player_arch_list[line].pow, player_arch_list[line].cha);
    LOG(llevInfo, "done.\n");

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

