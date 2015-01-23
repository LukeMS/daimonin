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

	The author can be reached via e-mail to info@daimonin.org
*/

#include <global.h>
#include "../3rdparty/zlib/zlib.h"

player_template_t player_template[MAX_PLAYER_ARCH];

/* as long the server don't have a autoupdate/login server
 * as frontend we must serve our depending client files self.
 */
static void LoadSrvFile(char *fname, int id, int cmd)
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
        LOG(llevError, "\nERROR: Size of compressed file %s exceeds size of uint32\nLoadSrvFile in startup.c needs update!\n", fname);

    /* we prepare the files with the right commands - so we can flush
     * then direct from this buffer to the client.
     */
    if ((int) numread < flen)
    {
        /* copy the compressed file in the right buffer */
		*comp_tmp = (char) (cmd|DATA_PACKED_CMD);
		*((uint32 *)(comp_tmp+1))=(uint32)(flen); //we send as uint32 the uncompressed filesize
		SrvClientFiles[id].sockbuf = SOCKBUF_COMPOSE(SERVER_CMD_DATA, (char *)comp_tmp, numread+5, SOCKBUF_FLAG_STATIC);
		SrvClientFiles[id].len = numread+5;
    }
    else
    {
		/* compress has no positive effect here */
		*file_tmp = (char) cmd;
		SrvClientFiles[id].sockbuf = SOCKBUF_COMPOSE(SERVER_CMD_DATA, (char *)file_tmp, flen+1, SOCKBUF_FLAG_STATIC);
		SrvClientFiles[id].len = -1;
		numread = flen+1;
    }
    free(file_tmp);
    free(comp_tmp);

    LOG(llevDebug, "(size: %d (%lu) (crc uncomp.: %x)\n", SrvClientFiles[id].len_ucomp, numread, SrvClientFiles[id].crc);
    fclose(fp);
}

/* Create the /data/client_skills file from the arches. */
/* TODO: This purposely recreates/adheres to the format of the old
 * arch/client_skills file for backwards compatibility. In a Y update we should
 * change this format to something better. */
static void CreateClientSkills(void)
{
    char  buf[MEDIUM_BUF];
    FILE *file;
    int   i;
    char  position[NROFSKILLGROUPS] = { 'a', 'a', 'a', 'a', 'a', 'a', 'a' };

    sprintf(buf, "%s/client_skills", settings.localdir);

    if (!(file = fopen(buf, "w")))
    {
        LOG(llevError, "ERROR:: Could not open '%s' for writing!\n",
            buf);
    }

    for (i = 0; i < NROFSKILLS; i++)
    {
        object_t *skill = &skills[i]->clone;
        char   *start = (char *)skill->msg;
        int j;

        fprintf(file, "\"%s\"\n", skill->name);

        if (!skill->inv_face)
        {
            sprintf(buf, "%s", "sk_001");
        }
        else
        {
            size_t p = strcspn(skill->inv_face->name, ".");

            strncpy(buf, skill->inv_face->name, p);
            buf[p] = '\0';
        }

        fprintf(file, "%d %c %s.png\n",
            skill->magic, position[skill->magic], buf);
        position[skill->magic]++;

        for (j = 0; j <= 2; j++)
        {
            buf[0] = '\0';

            if (start)
            {
                char *end = start;

                while (*end)
                {
                    if (*end != '\n')
                    {
                        sprintf(strchr(buf, '\0'), "%c", *end);
                    }

                    if (*end == '.')
                    {
                        start = end + 1;
                        break;
                    }
                    else
                    {
                        end++;
                    }
                }
            }

            if (j == 0)
            {
                fprintf(file, "\"~%s~ %s\"\n",
                    skill->name, (buf[0] == '\0') ? " " : buf);
            }
            else
            {
                fprintf(file, "\"%s\"\n", (buf[0] == '\0') ? " " : buf);
            }
        }

        if (skill->last_eat == NONLEVELING)
        {
            fprintf(file, "\"|Does not level; you either have it or you do not.|\"\n");
        }
        else if (skill->last_eat == INDIRECT)
        {
            fprintf(file, "\"|Accumulates exp which, when you have enough, increases level.|\"\n");
        }
        else if (skill->last_eat == DIRECT)
        {
            fprintf(file, "\"|Increases level directly; you have to acquire training.|\"\n");
        }
    }

    fclose(file);
}

/* get the /lib/settings default file and create the
 * /data/client_settings with it.
 */
static void CreateClientsettings_t(void)
{
    archetype_t   *p_arch;
    char        buf[LARGE_BUF], arch_name[TINY_BUF];
    int         i, line=0, id, race=0, gender = 0;
    FILE        *fset_default, *fset_create;

    LOG(llevSystem, "Creating %s/client_settings...\n", settings.localdir);

    /* used by create_player() as default template */
    memset(player_template, 0 , sizeof(player_template_t));

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
    while (fgets(buf, MEDIUM_BUF, fset_default) != NULL)
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
                    player_template[race].p_arch[gender++] = p_arch; /* we just copy the clone here later */
                }
            }
            else if (line == 6) /* thats the default start values */
            {
                sscanf(buf, "%d %d %d %d %d %d %d\n",
                &player_template[race].str, &player_template[race].dex,
                &player_template[race].con, &player_template[race].intel,
                &player_template[race].wis, &player_template[race].pow, &player_template[race].cha);
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
        player_template[line].p_arch[0]?player_template[line].p_arch[0]->clone.name:"NULL",
        player_template[line].p_arch[1]?player_template[line].p_arch[1]->clone.name:"NULL",
        player_template[line].p_arch[2]?player_template[line].p_arch[2]->clone.name:"NULL",
        player_template[line].p_arch[3]?player_template[line].p_arch[3]->clone.name:"NULL",
        player_template[line].str, player_template[line].dex,
        player_template[line].con, player_template[line].intel, player_template[line].wis,
        player_template[line].pow, player_template[line].cha);
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
    char    buf[MEDIUM_BUF];

    memset(&SrvClientFiles, 0, sizeof(SrvClientFiles));

    sprintf(buf, "%s/animations", settings.datadir);
    LoadSrvFile(buf, SRV_CLIENT_ANIMS, DATA_CMD_ANIM_LIST);

    sprintf(buf, "%s/client_bmaps", settings.localdir);
    LoadSrvFile(buf, SRV_CLIENT_BMAPS, DATA_CMD_BMAP_LIST);

    sprintf(buf, "%s/client_sounds", settings.datadir);
    LoadSrvFile(buf, SRV_CLIENT_SOUNDS, DATA_CMD_SOUND_LIST);

    CreateClientSkills();
    sprintf(buf, "%s/client_skills", settings.localdir);
    LoadSrvFile(buf, SRV_CLIENT_SKILLS, DATA_CMD_SKILL_LIST);

    sprintf(buf, "%s/client_spells", settings.datadir);
    LoadSrvFile(buf, SRV_CLIENT_SPELLS, DATA_CMD_SPELL_LIST);

    CreateClientsettings_t();
    sprintf(buf, "%s/client_settings", settings.localdir);
    LoadSrvFile(buf, SRV_CLIENT_SETTINGS, DATA_CMD_SETTINGS_LIST);
}

