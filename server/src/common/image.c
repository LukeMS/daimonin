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


#include <global.h>
#include <stdio.h>

/* hm, we really not need bmappair, or? we just sort out New_Face and adjust then
 * the face numbers. When using the same archefile & bmaps file, sorting will always
 * end in the same result. And the client has no knowledge about it. The server
 * tell it the client at runtime - this is face "bla" and it has number <num>.
 * The client use this info only the time it is connected. 
 * So, we can remove it. MT-11-2002
 */

/* bmappair and xbm are used when looking for the image id numbers
 * of a face by name.  xbm is sorted alphabetically so that bsearch
 * can be used to quickly find the entry for a name.  the number is
 * then an index into the new_faces array.
 * This data is redundant with new_face information - the difference
 * is that this data gets sorted, and that doesn't necessarily happen
 * with the new_face data - when accessing new_face[some number], 
 * that some number corresponds to the face at that number - for
 * xbm, it may not.  At current time, these do in fact match because
 * the bmaps file is created in a sorted order.
 */

struct bmappair
{
    char                   *name;
    unsigned int            number;
};

static struct bmappair *xbm = NULL;

static int compar(struct bmappair *a, struct bmappair *b)
{
    return strcmp(a->name, b->name);
}


/* This reads the bmaps.paths file to get all the bitmap names and
 * stuff.  It only needs to be done once, because it is player
 * independent (ie, what display the person is on will not make a
 * difference.)
 */

int ReadBmapNames()
{
    char    buf[MAX_BUF], *p, *q;
    FILE   *fp;
    int     value, nrofbmaps = 0, i;

    bmaps_checksum = 0;
    sprintf(buf, "%s/bmaps", settings.datadir);
    LOG(llevDebug, "Reading bmaps from %s...", buf);
    if ((fp = fopen(buf, "r")) == NULL)
        LOG(llevError, "ERROR: Can't open bmaps file buf = %s\n", buf);

    /* First count how many bitmaps we have, so we can allocate correctly */
    while (fgets(buf, MAX_BUF, fp) != NULL)
        if (buf[0] != '#' && buf[0] != '\n')
            nrofbmaps++;
    rewind(fp);

    xbm = (struct bmappair *) malloc(sizeof(struct bmappair) * (nrofbmaps + 1));
    memset(xbm, 0, sizeof(struct bmappair) * (nrofbmaps + 1));

    while (fgets(buf, MAX_BUF, fp) != NULL)
    {
        if (*buf == '#')
            continue;
        
        /* Kill the newline */
        i = strlen(buf) - 1;
        while(isspace(buf[i]) && i >= 0)
            buf[i--] = '\0';

        p = (*buf == '\\') ? (buf + 1) : buf;
        if (!(p = strtok(p, " \t")) || !(q = strtok(NULL, " \t\n")))
        {
            LOG(llevDebug, "Warning, syntax error: %s\n", buf);
            continue;
        }
        value = atoi(p);

        /* Kill the newline */
        i = strlen(q) - 1;
        while(isspace(q[i]) && i >= 0)
            q[i--] = '\0';
        
        xbm[nroffiles].name = strdup_local(q);

        /* We need to calculate the checksum of the bmaps file
         * name->number mapping to send to the client.  This does not
         * need to match what sum or other utility may come up with -
         * as long as we get the same results on the same real file
         * data, it does the job as it lets the client know if
         * the file has the same data or not.
         */
        ROTATE_RIGHT(bmaps_checksum);
        bmaps_checksum += value & 0xff;
        bmaps_checksum &= 0xffffffff;

        ROTATE_RIGHT(bmaps_checksum);
        bmaps_checksum += (value >> 8) & 0xff;
        bmaps_checksum &= 0xffffffff;
        for (i = 0; i < (int) strlen(q); i++)
        {
            ROTATE_RIGHT(bmaps_checksum);
            bmaps_checksum += q[i];
            bmaps_checksum &= 0xffffffff;
        }


        xbm[nroffiles].number = value;
        nroffiles++;
        if (value > nrofpixmaps)
            nrofpixmaps = value;
    }
    fclose(fp);

    LOG(llevDebug, "done (got %d/%d/%d)\n", nrofpixmaps, nrofbmaps, nroffiles);

    new_faces = (New_Face *) malloc(sizeof(New_Face) * (nrofpixmaps + 1));
    for (i = 0; i <= nrofpixmaps; i++)
    {
        new_faces[i].name = "";
        new_faces[i].number = i;
    }

    for (i = 0; i < nroffiles; i++)
    {
        new_faces[xbm[i].number].name = xbm[i].name;
    }

    nrofpixmaps++;

    qsort(xbm, nrofbmaps, sizeof(struct bmappair), (void *) (int (*) ()) compar);

    blank_face = &new_faces[FindFace(BLANK_FACE_NAME, 0)];
    blank_look.face = blank_face;
    blank_look.flags = 0;

    next_item_face = &new_faces[FindFace(NEXT_ITEM_FACE_NAME, 0)];
    prev_item_face = &new_faces[FindFace(PREVIOUS_ITEM_FACE_NAME, 0)];

    return nrofpixmaps;
}


/* This returns an the face number of face 'name'.  Number is constant
 * during an invocation, but not necessarily between versions (this
 * is because the faces are arranged in alphabetical order, so
 * if a face is removed or added, all faces after that will now
 * have a different number.
 *
 * the parameter error determines behaviour.  If a face is
 * not found, then error is returned.  This can be useful if
 * you want some default face used, or can be set to negative
 * so that it will be known that the face could not be found
 * (needed in client, so that it will know to request that image
 * from the server)
 *
 * name is const since we won't modify it. It does not have to
 * be a shared string.
 */
int FindFace(const char *name, int error)
{
    int                 i;
    struct bmappair    *bp, tmp;

    /* Using actual numbers for faces is a very bad idea.  This is because
     * each time the archetype file is rebuilt, all the face numbers
     * change.
     */
    if ((i = atoi(name)))
    {
        LOG(llevBug, "BUG: Integer face name used: %s\n", name);
        return i;
    }

    tmp.name = (char *)name;
    bp = (struct bmappair *) bsearch(&tmp, xbm, nroffiles, sizeof(struct bmappair), (void *) (int (*) ()) compar);

    return bp ? bp->number : error;
}

void free_all_images()
{
    int i;

    for (i = 0; i < nroffiles; i++)
        free(xbm[i].name);
    free(xbm);
    free(new_faces);
}
