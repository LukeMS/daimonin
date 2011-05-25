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

    The author can be reached via e-mail to info@daimonin.org
*/

/* A face is a map/inventory image. For the most part the client has local
 * copies of faces in FILE_FACEPACK. Sometimes though it will not (for example,
 * the server has an updated copy or indeed a wholly new face), or this file
 * may be missing entirely. In these cases, the face will be requested from the
 * server and saved to DIR_CACHE. A third option is that the user has put an
 * appropriately named file in DIR_GFX_USER. This will then override even the
 * server's copy (which will not even be requested) but of course is only shown
 * on this client. This is useful for artists to test there stuff and is
 * exploited by skins.
 *
 * The face is guaranteed to be in one of those places because the server will
 * not even send what it does not know as valid. TODO: Not sure this is true of
 * animations.
 *
 * This behaviour is exploited by the client too. At connection, the server
 * will send the client an ordered list of info about all the faces it knows.
 * For each face this is the length, crc, and name. This info is written to
 * face_list[]. We then look through FILE_FACEPACK. When there is a name here
 * that does not appear in the server's list, we just ignore it (because that
 * image is not used by this server, so we don't need to think about it). When
 * we have a match we compare the server's to the client's length and crc. If
 * these values are identical, we record the position within FILE_FACEPACK of
 * the face. If they are not, we set this to -1.
 *
 * The actual image data for a face is only loaded into memory as it is needed.
 * As described above, a file in DIR_GFX_USER will be loaded in preference to
 * any other source. Failing this, we will query file_info[].pos. If this is
 * -1, the face is loaded from DIR_CACHE if it is there. For other pos values,
 * the face is loaded from FILE_FACEPACK. If we still can't find the face after
 * all this a request is sent to the server (the resulting file will be saved
 * to DIR_CACHE). TODO: It might be useful to add a timer to face_info[] so
 * that image data can be freed when it has not been used for X time -- helpful
 * for small machines, phones, etc?
 *
 * -- Smacky 20110516 */

#include "include.h"

face_t          face_list[FACE_MAX_NROF];
uint16          face_nrof;
face_mpart_id_t face_mpart_id[16];

static _sprite_status LoadFromMemory(uint16 num, uint8 *data, uint32 len);
static _sprite_status LoadFromFile(uint16 num, const char *dname);
static _sprite_status LoadFromPack(uint16 num);
static void           SetFlags(uint16 num);

/* We have received SERVER_CMD_FACE1 (but see commands.c:Face1Cmd(). */
void face_saveinfo(uint16 num, uint32 crc, const char *name)
{
    FREE(face_list[num].name);
    MALLOC_STRING(face_list[num].name, name);
    face_list[num].crc = crc;
}

/* We have received SERVER_CMD_IMAGE (see commands.c:ImageCmd()). */
void face_save(uint16 num, uint8 *data, uint32 len)
{
    char         buf[SMALL_BUF];
    PHYSFS_File *handle;

    LOG(LOG_DEBUG, "ImageFromServer: %s\n", face_list[num].name);
    sprintf(buf, "%s/%s.png", DIR_CACHE, face_list[num].name);

    /* Save data to file. */
    if (!(handle = PHYSFS_openWrite(buf)))
    {
        LOG(LOG_ERROR, "Could not open '%s' for writing (%s)!\n",
            buf, PHYSFS_getLastError());

        return;
    }

    PHYSFS_write(handle, data, 1, len);
    PHYSFS_close(handle);

    /* As the image is already floating about in memory, lets tie it down! */
    if (LoadFromMemory(num, data, len) == SPRITE_STATUS_LOADED)
    {
        SetFlags(num);
    }
}

/* See if the named face is known to the server and if so return its number in
 * face_list[]. If not, return -1. */
sint32 face_find(const char *name)
{
    uint16 l,
           r,
           x;

    /* The bug face is out of order, always at face_list[0], so our binary
     * search will fail for this one. So lets catch that here. */
    if (!strcmp(name, "bug.101"))
    {
        return 0;
    }

    /* The faces in face_list[] are conveniently pre-sorted into alphabetical
     * order server-side so we can do a quick binary search to find the named
     * face. */
    for (l = 0, r = face_nrof - 1, x = r / 2; r >= l; x = (l + r) / 2)
    {
        sint8 diff = strcmp(name, face_list[x].name);

        if (diff < 0)
        {
            r = x - 1;
        }
        else if (diff > 0)
        {
            l = x + 1;
        }
        else
        {
            return (sint32)x;
        }
    }

    /* If we get here, the named face could not be found in face_list[]. */
    return -1;
}

/* Load/request face_list[num]. */
void face_get(sint32 num)
{
    /* Anything below 0 (ie, face_find() returns -1 for not found) becomes 0
     * (bug face). */
    num = MAX(0, num);

    /* Is the facenum too big? */
    if (num >= face_nrof)
    {
        LOG(LOG_ERROR, "Image ID too big (%d/%d)!\n", num, face_nrof);

        return;
    }

    /* Is the face already loaded or requested? */
    if ((face_list[num].flags & (FACE_FLAG_LOADED | FACE_FLAG_REQUESTED)))
    {
        return;
    }

    if (LoadFromFile(num, DIR_GFX_USER) != SPRITE_STATUS_LOADED &&
        (face_list[num].pos == -1 || // we already know it's not in FILE_FACEPACK
         LoadFromPack(num) != SPRITE_STATUS_LOADED) &&
        LoadFromFile(num, DIR_CACHE) != SPRITE_STATUS_LOADED)
    {
        face_list[num].flags = FACE_FLAG_REQUESTED;
        client_cmd_face(num);
    }
    else
    {
        SetFlags(num);
    }
}

/* Free face_list[num]. */
void face_free(uint16 num)
{
    if (num >= FACE_MAX_NROF ||
        !(face_list[num].flags & FACE_FLAG_LOADED))
    {
        return;
    }

    if (face_list[num].sprite)
    {
        sprite_free_sprite(face_list[num].sprite);
        face_list[num].sprite = NULL;
    }

    FREE(face_list[num].name);
    face_list[num].flags = 0;
}

static _sprite_status LoadFromMemory(uint16 num, uint8 *data, uint32 len)
{
    SDL_RWops *rw;

    /* If we can't get the image data back, log an error and return. */
    if (!(rw = SDL_RWFromMem(data, len)))
    {
        LOG(LOG_ERROR, "Could not retrieve image data from memory!\n");

        return SPRITE_STATUS_UNLOADED;
    }

    face_list[num].sprite = sprite_load(NULL, 0, rw);

    return face_list[num].sprite->status;
}

static _sprite_status LoadFromFile(uint16 num, const char *dname)
{
    char           buf[SMALL_BUF];
    PHYSFS_File   *handle;
    PHYSFS_sint64  len;
    uint8         *data;

    sprintf(buf, "%s/%s.png", dname, face_list[num].name);

    /* If there is no such file simply return SPRITE_STATUS_UNLOADED. */
    if (!PHYSFS_exists(buf))
    {
        return SPRITE_STATUS_UNLOADED;
    }

    /* If the file now can't be read, log an error and return SPRITE_STATUS_UNLOADED. */
    if (!(handle = PHYSFS_openRead(buf)))
    {
        LOG(LOG_ERROR, "File '%s' exists but there was an error reading it (%s)!\n",
            buf, PHYSFS_getLastError());

        return SPRITE_STATUS_UNLOADED;
    }

    if ((len = PHYSFS_fileLength(handle)) == -1)
    {

        LOG(LOG_ERROR, "There was an error reading the length of file '%s' (%s)!\n",
            buf, PHYSFS_getLastError());

        return SPRITE_STATUS_UNLOADED;
    }

    if (len == 0)
    {
        LOG(LOG_ERROR, "File '%s' is zero length!\n", buf);
        LOG(LOG_SYSTEM, "Deleting file... ");

        if (!PHYSFS_delete(buf))
        {
            LOG(LOG_ERROR, "FAILED (%s)!\n", PHYSFS_getLastError());
        }

        LOG(LOG_SYSTEM, "OK!\n");

        return SPRITE_STATUS_UNLOADED;
    }

    MALLOC(data, len);

    if (PHYSFS_read(handle, data, 1, len) < len)
    {
        return SPRITE_STATUS_UNLOADED;
    }

    PHYSFS_close(handle);
    FREE(data);
    face_list[num].sprite = sprite_load(buf, 0, NULL);

    return face_list[num].sprite->status;
}

static _sprite_status LoadFromPack(uint16 num)
{
    PHYSFS_File *handle;
    uint32       len;
    uint8       *data;

    /* If the image pack does not exist, just return SPRITE_STATUS_UNLOADED. */
    if (!PHYSFS_exists(FILE_FACEPACK))
    {
        return SPRITE_STATUS_UNLOADED;
    }

    /* If we fail to open the file, log an error and return SPRITE_STATUS_UNLOADED. */
    if (!(handle = PHYSFS_openRead(FILE_FACEPACK)))
    {
        LOG(LOG_ERROR, "Could not open '%s' for reading!\n", FILE_FACEPACK);

        return SPRITE_STATUS_UNLOADED;
    }

    /* If we fail to set position within the file, log an error and return
     * SPRITE_STATUS_UNLOADED. */
    if (!PHYSFS_seek(handle, (PHYSFS_uint64)face_list[num].pos))
    {
        LOG(LOG_ERROR, "Could not set position for image %d in file '%s' (%s)!\n",
            num, FILE_FACEPACK, PHYSFS_getLastError());

        return SPRITE_STATUS_UNLOADED;
    }

    len = face_list[num].len;
    MALLOC(data, len);

    /* If we can't read the actual image data from the file, log an error and
     * return SPRITE_STATUS_UNLOADED. */
    if (PHYSFS_read(handle, data, 1, len) < len)
    {
        LOG(LOG_ERROR, "Could not read image %d data in file '%s'(%s)!\n",
            num, FILE_FACEPACK, PHYSFS_getLastError());

        return SPRITE_STATUS_UNLOADED;
    }

    PHYSFS_close(handle);
    (void)LoadFromMemory(num, data, len);
    FREE(data);

    return face_list[num].sprite->status;
}

/* help function for receiving faces (pictures)
* NOTE: This feature must be enabled from the server
*/
static void SetFlags(uint16 num)
{
    char *buf = face_list[num].name;
    char *stemp;

    face_list[num].flags = FACE_FLAG_LOADED;

    /* Check for the "alt a"/"alt b"/"double"/"up" tag in the picture name. */
    if ((stemp = strstr(buf, ".a")))
    {
        char   buf_temp[TINY_BUF];
        sint32 i;

        face_list[num].flags |= FACE_FLAG_ALTERNATIVE;
        strcpy(buf_temp, buf);
        *(strstr(buf_temp, ".a") + 1) = 'b';
        i = face_find(buf_temp);
        face_get(i);

        /* Set our_ref. */
        face_list[num].alt_a = -1;
        face_list[num].alt_b = i;

        /* Set your_ref. */
        face_list[i].alt_a = num;
        face_list[i].alt_b = -1;
    }
    else if ((stemp = strstr(buf, ".b")))
    {
        char   buf_temp[TINY_BUF];
        sint32 i;

        face_list[num].flags |= FACE_FLAG_ALTERNATIVE;
        strcpy(buf_temp, buf);
        *(strstr(buf_temp, ".b") + 1) = 'a';
        i = face_find(buf_temp);
        face_get(i);

        /* Set our_ref. */
        face_list[num].alt_a = i;
        face_list[num].alt_b = -1;

        /* Set your_ref. */
        face_list[i].alt_a = -1;
        face_list[i].alt_b = num;
    }
    else if ((stemp = strstr(buf, ".d")))
    {
        face_list[num].flags |= FACE_FLAG_DOUBLE;
    }
    else if ((stemp = strstr(buf, ".u")))
    {
        int tc;

        face_list[num].flags |= FACE_FLAG_UP;

        for (tc = 0; tc < 4; tc++)
        {
            if (!*(stemp + tc)) /* has the string a '0' before our anim tags */
            {
                return;
            }
        }

        switch (*(stemp + tc))
        {
            case '0':
            case '2':
            case '4':
            case '6':
            case '8':
                face_list[num].flags |= (FACE_FLAG_D3 | FACE_FLAG_D1);

                break;

            case '1':
            case '5':
                face_list[num].flags |= FACE_FLAG_D1;

                break;

            case '3':
            case '7':
                face_list[num].flags |= FACE_FLAG_D3;

                break;
        }
    }
}
