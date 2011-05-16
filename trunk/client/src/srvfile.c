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

    The author can be reached via e-mail to info@daimonin.org
*/

#include "include.h"

srvfile_t srvfile[SRV_CLIENT_FILES];

static void Check(const char *fname, uint8 num);
static void LoadAnims(void);
static void LoadFaceInfo(void);
static void CheckLocalFaceInfo(void);
static void LoadSettings(void);
static void LoadSkills(void);
static void LoadSounds(void);
static void LoadSpells(void);

/* Check the length and crc of each of the files which we store in srvfile[]. */
void srvfile_check(void)
{
    Check(FILE_SRV_ANIMS, SRV_CLIENT_ANIMS);
    Check(FILE_SRV_FACEINFO, SRV_CLIENT_BMAPS);
    Check(FILE_SRV_SETTINGS, SRV_CLIENT_SETTINGS);
    Check(FILE_SRV_SKILLS, SRV_CLIENT_SKILLS);
    Check(FILE_SRV_SOUNDS, SRV_CLIENT_SOUNDS);
    Check(FILE_SRV_SPELLS, SRV_CLIENT_SPELLS);
}

/* Set the srvfile[] status. If this is OK, set the srvfile[] server length and
 * server crc to the srvfile[] length and crc. Otherwise, set the srvfile[]
 * server length and server crc to the appropriate parameters. */
void srvfile_set_status(uint8 num, uint8 status, int len, uint32 crc)
{
    srvfile[num].status = status;

    if (status == SRVFILE_STATUS_OK)
    {
        srvfile[num].server_len = srvfile[num].len;
        srvfile[num].server_crc = srvfile[num].crc;
    }
    else
    {
        srvfile[num].server_len = len;
        srvfile[num].server_crc = crc;
    }
}

/* Save the data received from the server with the appropriate filename amd
 * update the length and crc in srvfile[]. */
void srvfile_save(const char *fname, uint8 num, unsigned char *data, int len)
{
    PHYSFS_File *handle;

    /* Log what we're doing. */
    LOG(LOG_SYSTEM, "Saving server file '%s'... ", fname);

    /* Open the file for writing.*/
    if (!(handle = PHYSFS_openWrite(fname)))
    {
        LOG(LOG_FATAL, "FAILED (%s)!\n", PHYSFS_getLastError());
    }

    if (PHYSFS_write(handle, (unsigned char *)data, 1, (PHYSFS_uint32)len) < len)
    {
        PHYSFS_close(handle);
        LOG(LOG_FATAL, "FAILED (%s)!\n", PHYSFS_getLastError());
    }

    /* Set the values we just got. */
    srvfile[num].status = SRVFILE_STATUS_OK;
    srvfile[num].len = (int)len;
    srvfile[num].crc = crc32(1L, data, len);

    /* If they don't match what the server has then this is really bad
     * (because we just got the file from the server). */
    if (srvfile[num].server_len != srvfile[num].len ||
        srvfile[num].server_crc != srvfile[num].crc)
    {
        PHYSFS_close(handle);
        LOG(LOG_FATAL, "Client and server still disagree on length and crc of '%s' (client has %d/%x, server has %d/%x)!\n",
            fname, srvfile[num].len, srvfile[num].crc,
            srvfile[num].server_len, srvfile[num].server_crc);
    }

    /* Cleanup. */
    PHYSFS_close(handle);
    LOG(LOG_SYSTEM, "OK (len:%d, crc:%x)!\n", len, srvfile[num].crc);
}

/* Request the file from the server if necessary and return the srvfile[]
 * status. */
uint8 srvfile_get_status(uint8 num)
{
    uint8 status = srvfile[num].status;

    if (status == SRVFILE_STATUS_UPDATE)
    {
        RequestFile(csocket, num);
    }

    return status;
}

/* Load the files into memory in the correct order. */
void srvfile_load(void)
{
    /* Face info must be loaded first as they will be referred to in the other
     * srvfiles. */
    LoadFaceInfo();

    /* The order of these four doesn't matter so lets go with alphabetical. */
    LoadAnims();
    LoadSkills();
    LoadSounds();
    LoadSpells();

    /* Settings (player races) must be loaded last as after 0.11.0 they may
     * refer to skills and spells (and of course faces). */
    LoadSettings();
}

static void Check(const char *fname, uint8 num)
{
    PHYSFS_File   *handle;
    PHYSFS_uint64  len;
    unsigned char *buf_tmp;

    /* Log what we're doing. */
    LOG(LOG_SYSTEM, "Checking server file '%s'... ", fname);

    /* If the file doesn't exist, that's OK. */
    if (!PHYSFS_exists(fname))
    {
        /* We obviously don't know length/crc yet so lets reset both to 0 and
         * mark the file as needing an update. */
        srvfile[num].status = SRVFILE_STATUS_UPDATE;
        srvfile[num].len = 0;
        srvfile[num].crc = 0;
        LOG(LOG_SYSTEM, "OK (But file does not exist)!\n");

        return;
    }

    /* Open the file for reading.*/
    if (!(handle = PHYSFS_openRead(fname)))
    {
        LOG(LOG_FATAL, "FAILED (%s)!\n", PHYSFS_getLastError());
    }

    /* Get the filelength. We can't handle big files. */
    if ((len = PHYSFS_fileLength(handle)) > INT_MAX)
    {
        PHYSFS_close(handle);
        LOG(LOG_FATAL, "FAILED (File too big: %d)!\n", len);
    }

   /* Read all the data from the file into a temp buffer to get the crc. */
    MALLOC(buf_tmp, len);

    if ((PHYSFS_read(handle, (unsigned char *)buf_tmp, 1, (PHYSFS_uint32)len)) < len)
    {
        FREE(buf_tmp);
        PHYSFS_close(handle);
        LOG(LOG_FATAL, "FAILED (%s)!\n", PHYSFS_getLastError());
    }

    /* Set the values we just got. */
    srvfile[num].status = SRVFILE_STATUS_OK;
    srvfile[num].len = (int)len;
    srvfile[num].crc = crc32(1L, buf_tmp, len);

    /* Cleanup. */
    FREE(buf_tmp);
    PHYSFS_close(handle);
    LOG(LOG_SYSTEM, "OK (len:%d, crc:%x)!\n", len, srvfile[num].crc);
}

static void LoadAnims(void)
{
    PHYSFS_File *handle;
    char         buf[MEDIUM_BUF];
    uint16       faces[LARGE_BUF],
                 count = 1,
                 anim_len = 0;
    uint8        anim_cmd[LARGE_BUF],
                 anim = 0,
                 dirframepos = 0,
                 frames = 0,
                 facings = 0,
                 numfaces = 0,
                 delay = 0,
                 seqnum = 0,
                 sequence = 0,
                 old_format = 1,
                 dirnum = 0,
                 dir = 0;

    /* Log what we're doing. */
    LOG(LOG_SYSTEM, "Loading '%s'... ", FILE_SRV_ANIMS);

    /* Open the file for reading. */
    if (!(handle = PHYSFS_openRead(FILE_SRV_ANIMS)))
    {
        LOG(LOG_FATAL, "FAILED (%s)!\n", PHYSFS_getLastError());
    }

    while (PHYSFS_readString(handle, buf, sizeof(buf)) > 0)
    {
        if (anim == 0) /* we are outside a anim body ? */
        {
            if (!strncmp(buf, "anim ", 5))
            {
                anim = 1;
                facings = 0;
                numfaces = 0;
                delay = DEFAULT_ANIM_DELAY;
                anim_cmd[2] = (uint8)((count >> 8) & 0xff);
                anim_cmd[3] = (uint8)(count & 0xff);
                anim_cmd[4] = 0;
                anim_len = 5;
            }
            else /* we should never hit this point */
            {
                LOG(LOG_FATAL, "FAILED (unknown cmd: >%s<)!\n", buf);
            }
        }
        else /* no, we are inside! */
        {
            if (!strncmp(buf, "sequence ", 9))
            {
                old_format = 0;
                seqnum = atoi(buf + 9);
                sequence = 1;

                if (dir) /* we had a dir command before, now we have a new sequence, lets set the enddir marker */
                {
                    anim_cmd[anim_len++] = 0xFF;
                    dir = 0;

                    if (dirframepos)
                    {
                        anim_cmd[dirframepos] = frames;
                    }

                    dirframepos = 0;
                }

                anim_cmd[anim_len++] = seqnum; /* one byte sequence num */
                anim_cmd[anim_len++] = 0;      /* we set now flags to zero, if we got a dirreset or sequencemap we set it later */
            }
            else if (!strncmp(buf, "sequencemap ", 12))
            {
                old_format = 0;
                sequence = 1;
                seqnum = atoi(buf + 12);
                anim_cmd[(anim_len - 1)] |= ASEQ_MAPPED;
                anim_cmd[anim_len++] = seqnum;
            }
            else if (!strncmp(buf, "dirreset ", 9))
            {
                old_format = 0;
                sequence = 1;

                if (atoi(buf + 9))
                {
                    anim_cmd[(anim_len) - 1] |= ASEQ_DIR_RESET;
                }
            }
            else if (!strncmp(buf, "dir ", 4))
            {
                if (dir) /* we had a dir command before*/
                {
                    if (dirframepos)
                    {
                        anim_cmd[dirframepos] = frames;
                    }

                    dirframepos = 0;
                }

                dir = 1;
                dirnum = atoi(buf + 4);
                anim_cmd[anim_len++] = dirnum;
                anim_cmd[anim_len++] = 0; /* nrof frames */
                dirframepos = anim_len - 1;
                frames = 0;
            }
            else if (!strncmp(buf, "dirmap ", 7))
            {
                dirnum = atoi(buf + 7);
                anim_cmd[(anim_len - 2)] |= ASEQ_MAPPED;
                anim_cmd[(anim_len - 1)] = dirnum;
                dirframepos = 0;
            }
            else if (!strncmp(buf, "delay ", 6))
            {
                delay = atoi(buf + 6);
            }
            else if (!strncmp(buf, "facings ", 8)) /* we have a old animation */
            {
                facings = atoi(buf + 8);
            }
            else if (!strncmp(buf, "mina", 4))
            {
                if (dir)
                {
                    anim_cmd[anim_len++] = 0xFF;

                    if (dirframepos)
                    {
                        anim_cmd[dirframepos] = frames;
                    }

                    dirframepos = 0;
                }

                if (sequence)
                {
                    anim_cmd[anim_len++] = 0xFF;
                }

                if (old_format)
                {
                    /* now convert the temp stored old stuff to the new format */
                    if (facings == 0)
                    {
                        uint8 i;

                        anim_cmd[anim_len++] = 0; /* sequence 0 */
                        anim_cmd[anim_len++] = 0; /* flags 0 */
                        anim_cmd[anim_len++] = 0; /* dir 0 */
                        anim_cmd[anim_len++] = numfaces;

                        for (i = 0; i < numfaces; i++)
                        {
                            anim_cmd[anim_len++] = (uint8)((faces[i] >> 8) & 0xff);
                            anim_cmd[anim_len++] = (uint8)(faces[i] & 0xff);
                            anim_cmd[anim_len++] = delay;
                        }

                        anim_cmd[anim_len++] = 0xFF; /* end of dirs */
                        anim_cmd[anim_len++] = 0xFF; /* end of sequences */
                    }
                    else
                    {
                        uint8 i,
                              num = 0;

                        for (i = 0; i < (uint8)((facings - 1) / 8); i++)
                        {
                            uint8 j;

                            anim_cmd[anim_len++] = i;
                            anim_cmd[anim_len++] = 0;

                            if (i == 0)
                            {
                                anim_cmd[anim_len++] = 0; /* dir 0 */
                                anim_cmd[anim_len++] = (uint8)(numfaces / facings);

                                for (j = 0; j < (uint8)(numfaces / facings); j++)
                                {
                                    anim_cmd[anim_len++] = (uint8)((faces[num] >> 8) & 0xff);
                                    anim_cmd[anim_len++] = (uint8)(faces[num++] & 0xff);
                                    anim_cmd[anim_len++] = delay;
                                }
                            }

                            for (j = 1; j < 9; j++)
                            {
                                uint8 k;

                                anim_cmd[anim_len++] = j;
                                anim_cmd[anim_len++] = (uint8)(numfaces / facings);

                                for (k = 0; k < (uint8)(numfaces / facings); k++)
                                {
                                    anim_cmd[anim_len++] = (uint8)((faces[num] >> 8) & 0xff);
                                    anim_cmd[anim_len++] = (uint8)(faces[num++] & 0xff);
                                    anim_cmd[anim_len++] = delay;
                                }
                            }

                            anim_cmd[anim_len++] = 0xFF;
                        }

                        anim_cmd[anim_len++] = 0xFF;
                    }
                }

                MALLOC(animcmd[count].anim_cmd, anim_len);
                memcpy(animcmd[count].anim_cmd, anim_cmd, anim_len);
                animcmd[count].len = anim_len;

                anim_cmd[0] = (uint8)(((anim_len - 2) >> 8) & 0xff);
                anim_cmd[1] = (uint8)((anim_len - 2) & 0xff);
                memset(faces, 0, sizeof(faces));
                memset(anim_cmd, 0, sizeof(anim_cmd));
                count++;
                anim = 0;
                old_format = 1;
                sequence = 0;
                dir = 0;
                numfaces = 0;
            }
            else
            {
                sint32 i = face_find(buf);

                if (i == -1)
                {
                    i = 0;
                    LOG(LOG_ERROR, "Invalid anim name >%s< - set to #0 (bug.101)!\n",
                        buf);
                }

                if (old_format)
                {
                    faces[numfaces++] = (uint16)i;
                }
                else
                {
                    anim_cmd[anim_len++] = (uint8)((i >> 8) & 0xff);
                    anim_cmd[anim_len++] = (uint8)(i & 0xff);
                    anim_cmd[anim_len++] = delay;
                    frames++;
                }
            }
        }
    }

    /* Cleanup. */
    PHYSFS_close(handle);
    LOG(LOG_SYSTEM, "OK!\n");
}

static void LoadFaceInfo(void)
{
    PHYSFS_File *handle;
    char         buf[MEDIUM_BUF];
    int          i = 0;

    if (!PHYSFS_exists(FILE_SRV_FACEINFO))
    {
       LOG(LOG_SYSTEM, "Could not find '%s'. This means that the server will not send faces so all faces must be sourced locally!\n",
           FILE_SRV_FACEINFO);
       CheckLocalFaceInfo();

       return;
    }

    /* Log what we're doing. */
    LOG(LOG_SYSTEM, "Loading '%s'... ", FILE_SRV_FACEINFO);

    /* Open the file for reading. */
    if (!(handle = PHYSFS_openRead(FILE_SRV_FACEINFO)))
    {
        LOG(LOG_FATAL, "FAILED (%s)!\n", PHYSFS_getLastError());
    }

    while (PHYSFS_readString(handle, buf, sizeof(buf)) > 0)
    {
        char         name[TINY_BUF];
        int          len;
        unsigned int crc;

        if (sscanf(buf, "%x %x %s", &len, &crc, name) != 3)
        {
            PHYSFS_close(handle);
            LOG(LOG_FATAL, "FAILED (Malformed string: >%s<)!\n", buf);
        }

        MALLOC_STRING(face_list[i].name, name);
        face_list[i].pos = -1; // not local, updated in CheckLocalFaceInfo()
        face_list[i].len = len;
        face_list[i].crc = crc;
        face_nrof = ++i;
    }

    /* Cleanup. */
    PHYSFS_close(handle);
    LOG(LOG_SYSTEM, "OK!\n");
    CheckLocalFaceInfo();
}

static void CheckLocalFaceInfo(void)
{
    PHYSFS_File *handle;
    char         buf[MEDIUM_BUF];
    sint32       i = 0;

    /* Check for existance of local images file. */
    if (!PHYSFS_exists(FILE_FACEPACK))
    {
       /* If it doesn't exist and the server is not sending us images, give up.
        * TODO: We should print a client message and go back to server select
        * rather than exit the client. */
       if (face_nrof == 0)
       {
           LOG(LOG_FATAL, "Could not find '%s'!\n", FILE_FACEPACK);
       }

       LOG(LOG_SYSTEM, "Could not find '%s'. This means all faces will need to be requested from the server!\n",
           FILE_FACEPACK);

       return;
    }

    /* Log what we're doing. */
    LOG(LOG_SYSTEM, "Loading local face info from '%s'... ", FILE_FACEPACK);

    /* Open the file for reading. */
    if (!(handle = PHYSFS_openRead(FILE_FACEPACK)))
    {
        LOG(LOG_FATAL, "FAILED (%s)!\n", PHYSFS_getLastError());
    }

    while (PHYSFS_readString(handle, buf, sizeof(buf)) > 0)
    {
        char          *cp = buf + 6,
                      *name;
        unsigned char *buf_tmp;
        int            pos,
                       len;
        unsigned int   crc;

        if (strncmp(buf, "IMAGE ", 6))
        {
            PHYSFS_close(handle);
            LOG(LOG_FATAL, "FAILED (Bad image line: >%s<)!\n", buf);
        }

        /* Skip accross the number data, which we don't even use any more. */
        while (isdigit(*cp))
        {
            cp++;
        }

        pos = (int)PHYSFS_tell(handle);
        len = atoi(++cp);
        MALLOC(buf_tmp, len);

        if (PHYSFS_read(handle, buf_tmp, 1, len) < len)
        {
            PHYSFS_close(handle);
            FREE(buf_tmp);
            LOG(LOG_FATAL, "FAILED (%s)!\n", PHYSFS_getLastError());
        }

        crc = crc32(1L, buf_tmp, len);
        FREE(buf_tmp);

        /* Skip accross the length data. */
        while (isdigit(*cp))
        {
            cp++;
        }

        name = cp + 1;

        /* If we have an image which the server doesn't have, ignore it. */
        if ((i = face_find(name)) == -1)
        {
            continue;
        }
        /* Only if our image and the server's are identical, update pos to
         * point to the local one. */
        else if (face_list[i].len == len &&
                 face_list[i].crc == crc)
        {
            face_list[i].pos = pos;
        }
    }

    /* Cleanup. */
    PHYSFS_close(handle);
    LOG(LOG_SYSTEM, "OK!\n");
}

/* TODO: This maintains 0.10 compatibility. The file format will be reworked for 0.11.0. */
static void LoadSettings(void)
{
    PHYSFS_File *handle;
    uint8        defn = 0;

    /* Log what we're doing. */
    LOG(LOG_SYSTEM, "Loading '%s'... ", FILE_SRV_SETTINGS);

    /* Open the file for reading.*/
    if (!(handle = PHYSFS_openRead(FILE_SRV_SETTINGS)))
    {
        LOG(LOG_FATAL, "FAILED (%s)!\n", PHYSFS_getLastError());
    }

    while (++defn)
    {
        char buf[MEDIUM_BUF],
             key[TINY_BUF],
             value[TINY_BUF];

        do
        {
            if (PHYSFS_readString(handle, buf, sizeof(buf)) < 0)
            {
                PHYSFS_close(handle);
                LOG(LOG_FATAL, "Unexpected EOF!\n");
            }
        }
        while (buf[0] == '#');

        if (sscanf(buf, "%s %s", key, value) != 2)
        {
            PHYSFS_close(handle);
            LOG(LOG_FATAL, "Malformed key/value line for definition %u: %s!\n", defn, buf);
        }

        if (!strcmp(key, "char"))
        {
            char          face[TINY_BUF],
                          arch[TINY_BUF];
            _server_char *sc;
            uint8         i;

            MALLOC(sc, sizeof(_server_char));
            MALLOC_STRING(sc->name, value);

            if (PHYSFS_readString(handle, buf, sizeof(buf)) < 0)
            {
                PHYSFS_close(handle);
                LOG(LOG_FATAL, "Unexpected EOF!\n");
            }

            if (sscanf(buf, "%s %d %d %d %d %d %d",
                       face, &sc->bar[0], &sc->bar[1], &sc->bar[2],
                       &sc->bar_add[0], &sc->bar_add[1], &sc->bar_add[2]) != 7)
            {
                PHYSFS_close(handle);
                LOG(LOG_FATAL, "Malformed hi line for definition %u: %s!\n", defn, buf);
            }

            /* 4 genders: male, female, hermaphrodite, neuter. */
            for (i = 0; i <= 3; i++)
            {
                if (PHYSFS_readString(handle, buf, sizeof(buf)) < 0)
                {
                    PHYSFS_close(handle);
                    LOG(LOG_FATAL, "Unexpected EOF!\n");
                }

                if (sscanf(buf, "%d %s %s", &sc->gender[i], arch, face) != 3)
                {
                    PHYSFS_close(handle);
                    LOG(LOG_FATAL, "Malformed gender %u line for definition %u: %s!\n",
                        i, defn, buf);
                }

                MALLOC_STRING(sc->char_arch[i], arch);

                sc->face[i] = face_find(face);
                face_get(sc->face[i]);
            }

            /* Str Dex Con Int Wis Pow Cha */
            if (PHYSFS_readString(handle, buf, sizeof(buf)) < 0)
            {
                PHYSFS_close(handle);
                LOG(LOG_FATAL, "Unexpected EOF!\n");
            }

            if (sscanf(buf, "%d %d %d %d %d %d %d",
                       &sc->stats[0], &sc->stats[1], &sc->stats[2],
                       &sc->stats[3], &sc->stats[4], &sc->stats[5],
                       &sc->stats[6]) != 7)
            {
                PHYSFS_close(handle);
                LOG(LOG_FATAL, "Malformed stat line for definition %u: %s!\n", defn, buf);
            }

            /* 4 lines of description. */
            for (i = 0; i <= 3; i++)
            {
                if (PHYSFS_readString(handle, buf, sizeof(buf)) < 0)
                {
                    PHYSFS_close(handle);
                    LOG(LOG_FATAL, "Unexpected EOF!\n");
                }

                MALLOC_STRING(sc->desc[i], buf);
            }

            /* add this char template to list */
            if (!first_server_char)
            {
                first_server_char = sc;
            }
            else
            {
                _server_char *sc_tmp = first_server_char;

                while (sc_tmp->next)
                {
                     sc_tmp = sc_tmp->next;
                }

                sc_tmp->next = sc;
                sc->prev = sc_tmp;
            }
        }
        else if (!strcmp(key, "level"))
        {
            uint8 i;

            server_level.level = atoi(value);

            for (i = server_level.level; i > 0; i--)
            {
                if (PHYSFS_readString(handle, buf, sizeof(buf)) < 0)
                {
                    PHYSFS_close(handle);
                    LOG(LOG_FATAL, "Unexpected EOF!\n");
                }

                if ((server_level.exp[i] = strtoul(buf, NULL, 16)) == ULONG_MAX)
                {
                    PHYSFS_close(handle);
                    LOG(LOG_FATAL, "Malformed exp line for level %u: %s!\n", i, buf);
                }
            }

            break;
        }
        else /* we close here... better we include later a fallback to login */
        {
            PHYSFS_close(handle);
            LOG(LOG_FATAL, "Unrecognised key: %s!\n", buf);
        }
    }

    /* TODO: Remove, just ugly. */
    if (first_server_char)
    {
        int g;

        memcpy(&new_character, first_server_char, sizeof(_server_char));
        new_character.skill_selected = 0;
        /* adjust gender */
        for (g = 0; g < 4; g++)
        {
            if (new_character.gender[g])
            {
                new_character.gender_selected = g;
                break;
            }
        }
    }

    /* Cleanup. */
    PHYSFS_close(handle);
    LOG(LOG_SYSTEM, "OK!\n");
}

/* TODO: This maintains 0.10 compatibility. The file format will be reworked for 0.11.0. */
static void LoadSkills(void)
{
    PHYSFS_File  *handle;
    uint8         defn = 0;

    /* Log what we're doing. */
    LOG(LOG_SYSTEM, "Loading '%s'... ", FILE_SRV_SKILLS);

    /* Open the file for reading.*/
    if (!(handle = PHYSFS_openRead(FILE_SRV_SKILLS)))
    {
        LOG(LOG_FATAL, "FAILED (%s)!\n", PHYSFS_getLastError());
    }

    while (++defn)
    {
        char   buf[MEDIUM_BUF],
              *start,
              *end,
               name[TINY_BUF],
               nchar,
               icon[TINY_BUF],
               desc[4][TINY_BUF];
        int    panel;
        uint8  i;
        _skill_list_entry *sle;

        /* Name */
        if (PHYSFS_readString(handle, buf, sizeof(buf)) < 0)
        {
            /* EOF here is OK. */
            break;
        }

        if (!(start = strchr(buf, '"')))
        {
            PHYSFS_close(handle);
            LOG(LOG_FATAL, "Malformed line for definition %u: %s!\n", defn, buf);
        }

        if (!(end = strchr(start + 1, '"')))
        {
            PHYSFS_close(handle);
            LOG(LOG_FATAL, "Malformed line for definition %u: %s!\n", defn, buf);
        }

        start++;
        *end = '\0';
        sprintf(name, "%s", start);

        /* Type Entry Path Icon */
        if (PHYSFS_readString(handle, buf, sizeof(buf)) < 0)
        {
            PHYSFS_close(handle);
            LOG(LOG_FATAL, "Unexpected EOF!\n");
        }

        if (sscanf(buf, "%d %c %s", &panel, &nchar, icon) != 3 ||
            (panel < 0 ||
             panel >= SPELL_LIST_MAX))
        {
            PHYSFS_close(handle);
            LOG(LOG_FATAL, "Malformed line for definition %u: %s!\n", defn, buf);
        }

        /* Desc */
        for (i = 0; i <= 3; i++)
        {
            if (PHYSFS_readString(handle, buf, sizeof(buf)) < 0)
            {
                PHYSFS_close(handle);
                LOG(LOG_FATAL, "Unexpected EOF!\n");
            }

            if (!(start = strchr(buf, '"')))
            {
                PHYSFS_close(handle);
                LOG(LOG_FATAL, "Malformed line for definition %u: %s!\n", defn, buf);
            }

            if (!(end = strchr(start + 1, '"')))
            {
                PHYSFS_close(handle);
                LOG(LOG_FATAL, "Malformed line for definition %u: %s!\n", defn, buf);
            }

            start++;
            *end = '\0';
            sprintf(desc[i], "%s", start);
        }

        sle = &skill_list[panel].entry[nchar - 'a'];
        sprintf(sle->name, "%s", name);
        sle->flag = LIST_ENTRY_USED;
        sprintf(sle->icon_name, "%s", icon);
        sprintf(buf, "%s%s", GetIconDirectory(), icon);
        sle->icon = sprite_load(buf, SURFACE_FLAG_DISPLAYFORMAT, NULL);

        for (i = 0; i <= 3; i++)
        {
            sprintf(sle->desc[i], "%s", desc[i]);
        }
    }

    /* Cleanup. */
    PHYSFS_close(handle);
    LOG(LOG_SYSTEM, "OK!\n");
}

/* TODO: This maintains 0.10 compatibility. The file format will be reworked for 0.11.0. */
static void LoadSounds(void)
{
#ifdef INSTALL_SOUND
    PHYSFS_File *handle;
    uint8        defn = 0;
    char         buf[MEDIUM_BUF],
                 name[TINY_BUF],
                 file[TINY_BUF];
    int          state = 0,
                 type_count = 0,
                 type_index = -1,
                 sound_count = 0,
                 sound_index = -1;

    /* Log what we're doing. */
    LOG(LOG_SYSTEM, "Loading '%s'... ", FILE_SRV_SOUNDS);

    /* Open the file for reading.*/
    if (!(handle = PHYSFS_openRead(FILE_SRV_SOUNDS)))
    {
        LOG(LOG_FATAL, "FAILED (%s)!\n", PHYSFS_getLastError());
    }

    while (++defn)
    {
        do
        {
            if (PHYSFS_readString(handle, buf, sizeof(buf)) < 0)
            {
                PHYSFS_close(handle);
                LOG(LOG_FATAL, "Unexpected EOF!\n");
            }
        }
        while (buf[0] == '#');

        if (!strcmp(buf, "*end"))
        {
            break;
        }

        switch (state)
        {
            /* Looking for start line. */
            case 0:
                if (!strncmp(buf, "*start", 6))
                {
                    strtok(buf, "|"); // discard *start
                    sscanf(strtok(NULL, "|"), "%d", &type_count); // count of soundtypes
                    sounds.count = type_count;
                    MALLOC(sounds.types, type_count * sizeof(_soundtype));
                    state++;
                }

                break;

            /* Looking for soundtype introducer. */
            case 1:
                if (type_count > 0 &&
                    buf[0] == '*')
                {
                    type_count--;
                    type_index++;
                    sscanf(strtok(buf, "|"), "*%d", &sounds.types[type_index].id);
                    strcpy(name, strtok(NULL, "|"));
                    strtok(NULL, "|");  // discard prefix
                    sscanf(strtok(NULL, "|"), "%d", &sound_count);
                    sounds.types[type_index].count = sound_count;
                    MALLOC_STRING(sounds.types[type_index].name, name);
                    MALLOC(sounds.types[type_index].sounds, sound_count * sizeof(_sound)); // space for sounds
                    sound_index = -1;
                    state++;
                }

                break;

            /* Process sound. */
            case 2:
                if (sound_count > 0 &&
                    buf[0] == '+')
                {
                    sound_count--;
                    sound_index++;
                    sscanf(strtok(buf, "|"), "+%d", &sounds.types[type_index].sounds[sound_index].id);
                    strcpy(name, strtok(NULL, "|"));
                    strcpy(file, strtok(NULL, "|"));
                    MALLOC_STRING(sounds.types[type_index].sounds[sound_index].name, name);
                    MALLOC_STRING(sounds.types[type_index].sounds[sound_index].file, file);
                }

                // Look for next soundtype
                if (sound_count == 0)
                {
                    state--;
                }

                break;
        }
    }

    /* Cleanup. */
    PHYSFS_close(handle);
    LOG(LOG_SYSTEM, "OK!\n");
#endif
}

/* TODO: This maintains 0.10 compatibility. The file format will be reworked for 0.11.0. */
static void LoadSpells(void)
{
    PHYSFS_File  *handle;
    uint8         defn = 0;

    /* Log what we're doing. */
    LOG(LOG_SYSTEM, "Loading '%s'... ", FILE_SRV_SPELLS);

    /* Open the file for reading.*/
    if (!(handle = PHYSFS_openRead(FILE_SRV_SPELLS)))
    {
        LOG(LOG_FATAL, "FAILED (%s)!\n", PHYSFS_getLastError());
    }

    while (++defn)
    {
        char   buf[MEDIUM_BUF],
              *start,
              *end,
               name[TINY_BUF],
               type,
               nchar,
               icon[TINY_BUF],
               desc[4][TINY_BUF];
        int    panel;
        uint8  i;
        _spell_list_entry *sle;

        /* Name */
        if (PHYSFS_readString(handle, buf, sizeof(buf)) < 0)
        {
            /* EOF here is OK. */
            break;
        }

        if (!(start = strchr(buf, '"')))
        {
            PHYSFS_close(handle);
            LOG(LOG_FATAL, "Malformed line for definition %u: %s!\n", defn, buf);
        }

        if (!(end = strchr(start + 1, '"')))
        {
            PHYSFS_close(handle);
            LOG(LOG_FATAL, "Malformed line for definition %u: %s!\n", defn, buf);
        }

        start++;
        *end = '\0';
        sprintf(name, "%s", start);

        /* Type Entry Path Icon */
        if (PHYSFS_readString(handle, buf, sizeof(buf)) < 0)
        {
            PHYSFS_close(handle);
            LOG(LOG_FATAL, "Unexpected EOF!\n");
        }

        if (sscanf(buf, "%c %c %d %s", &type, &nchar, &panel, icon) != 4 ||
            (type != 'w' &&
             type != 'p') ||
            (panel <= 0 ||
             panel > SPELL_LIST_MAX))
        {
            PHYSFS_close(handle);
            LOG(LOG_FATAL, "Malformed line for definition %u: %s!\n", defn, buf);
        }

        /* Desc */
        for (i = 0; i <= 3; i++)
        {
            if (PHYSFS_readString(handle, buf, sizeof(buf)) < 0)
            {
                PHYSFS_close(handle);
                LOG(LOG_FATAL, "Unexpected EOF!\n");
            }

            if (!(start = strchr(buf, '"')))
            {
                PHYSFS_close(handle);
                LOG(LOG_FATAL, "Malformed line for definition %u: %s!\n", defn, buf);
            }

            if (!(end = strchr(start + 1, '"')))
            {
                PHYSFS_close(handle);
                LOG(LOG_FATAL, "Malformed line for definition %u: %s!\n", defn, buf);
            }

            start++;
            *end = '\0';
            sprintf(desc[i], "%s", start);
        }

        sle = &spell_list[panel - 1].entry[(type == 'w') ? 0 : 1][nchar - 'a'];
        sprintf(sle->name, "%s", name);
        sle->flag = LIST_ENTRY_USED;
        sprintf(sle->icon_name, "%s", icon);
        sprintf(buf, "%s%s", GetIconDirectory(), icon);
        sle->icon = sprite_load(buf, SURFACE_FLAG_DISPLAYFORMAT, NULL);

        for (i = 0; i <= 3; i++)
        {
            sprintf(sle->desc[i], "%s", desc[i]);
        }
    }

    /* Cleanup. */
    PHYSFS_close(handle);
    LOG(LOG_SYSTEM, "OK!\n");
}
