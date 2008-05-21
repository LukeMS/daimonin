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

struct bmappair
{
	char                   *name;
	unsigned int            number;
};

static struct bmappair *xbm = NULL;

#ifdef SERVER_SEND_FACES
#include "../zlib/zlib.h"

#define MAX_FACE_SETS   1
#define MAX_IMAGE_SIZE 20000

typedef struct FaceInfo
{
	uint8  *data;           /* image data */
	uint16  datalen;        /* length of the xpm data */
	uint32  checksum;       /* Checksum of face data */
} FaceInfo;


typedef struct
{
	char           *prefix;
	char           *fullname;
	uint8           fallback;
	char           *size;
	char           *extension;
	char           *comment;
	FaceInfo       *faces;
} FaceSets;

static FaceSets facesets[MAX_FACE_SETS];

int is_valid_faceset(int fsn)
{
	if (fsn >= 0 && fsn < MAX_FACE_SETS && facesets[fsn].prefix)
		return TRUE;
	return FALSE;
}

void free_socket_images()
{
	int num, q;

	for (num = 0; num < MAX_FACE_SETS; num++)
	{
		if (facesets[num].prefix)
		{
			for (q = 0; q < nrofpixmaps; q++)
				if (facesets[num].faces[q].data)
					free(facesets[num].faces[q].data);
			free(facesets[num].prefix);
			free(facesets[num].fullname);
			free(facesets[num].size);
			free(facesets[num].extension);
			free(facesets[num].comment);
			free(facesets[num].faces);
		}
	}
}

/* This returns the set we will actually use when sending
* a face.  This is used because the image files may be sparse.
* This function is recursive.  imageno is the face number we are
* trying to send
*/
static int get_face_fallback(int faceset, int imageno)
{
	/* faceset 0 is supposed to have every image, so just return.  Doing
	* so also prevents infinite loops in the case if it not having
	* the face, but in that case, we are likely to crash when we try
	* to access the data, but that is probably preferable to an infinite
	* loop.
	*/
	if (faceset == 0)
		return 0;

	if (!facesets[faceset].prefix)
	{
		LOG(llevBug, "BUG: get_face_fallback called with unused set (%d)?\n", faceset);
		return 0;   /* use default set */
	}
	if (facesets[faceset].faces[imageno].data)
		return faceset;
	return get_face_fallback(facesets[faceset].fallback, imageno);
}

/* This is a simple recursive function that makes sure the fallbacks
* are all proper (eg, the fall back to defined sets, and also
* eventually fall back to 0).  At the top level, togo is set to MAX_FACE_SETS,
* if togo gets to zero, it means we have a loop.
* This is only run when we first load the facesets.
*/
static void check_faceset_fallback(int faceset, int togo)
{
	int fallback    = facesets[faceset].fallback;

	/* proper case - falls back to base set */
	if (fallback == 0)
		return;

	if (!facesets[fallback].prefix)
		LOG(llevError, "Face set %d falls to non set faceset %d\n", faceset, fallback);
	togo--;
	if (togo == 0)
		LOG(llevError, "Infinite loop found in facesets. aborting.\n");

	check_faceset_fallback(fallback, togo);
}

/* read_client_images loads all the iamge types into memory.
*  This  way, we can easily send them to the client.  We should really do something
* better than abort on any errors - on the other hand, these are all fatal
* to the server (can't work around them), but the abort just seems a bit
* messy (exit would probably be better.)
*/

/* Couple of notes:  We assume that the faces are in a continous block.
* This works fine for now, but this could perhaps change in the future
*/

/* Function largely rewritten May 2000 to be more general purpose.
* The server itself does not care what the image data is - to the server,
* it is just data it needs to allocate.  As such, the code is written
* to do such.
*/

/* i added the generation of client_bmaps here... i generate only *one*
* file - i don't use different sets (daimonin.1, daimonin.2,...) but i mix
* the code up - if ever one is interested to add here and in the client full
* different set power, he can complete this stuff... note now: have more as
* one set will break the server atm
*/

void read_client_images()
{
	char    filename[400];
	char    buf[HUGE_BUF];
	char   *cp, *cps[7];
	FILE   *infile, *fbmap;
	int     num, len, fileno, i, badline;

	memset(facesets, 0, sizeof(facesets));
	sprintf(filename, "%s/image_info", settings.datadir);
	if ((infile = fopen(filename, "r")) == NULL)
		LOG(llevError, "Unable to open %s\n", filename);
	while (fgets(buf, HUGE_BUF - 1, infile) != NULL)
	{
		badline = 0;

		if (buf[0] == '#' || buf[0] == '\n' || buf[0] == '\r')
			continue;
		cp = buf + (strlen(buf) - 1);
		while(isspace(*cp))
			--cp;
		cp[1] = '\0';

		if (!(cps[0] = strtok(buf, ":")))
			badline = 1;
		for (i = 1; i < 7; i++)
		{
			if (!(cps[i] = strtok(NULL, ":")))
				badline = 1;
		}
		if (badline)
			LOG(llevBug, "BUG: Bad line in image_info file, ignoring line:\n  %s", buf);
		else
		{
			len = atoi(cps[0]);
			if (len >= MAX_FACE_SETS)
				LOG(llevError, "To high a setnum in image_info file: %d > %d\n", len, MAX_FACE_SETS);
			facesets[len].prefix = strdup_local(cps[1]);
			facesets[len].fullname = strdup_local(cps[2]);
			facesets[len].fallback = atoi(cps[3]);
			facesets[len].size = strdup_local(cps[4]);
			facesets[len].extension = strdup_local(cps[5]);
			facesets[len].comment = strdup_local(cps[6]);
		}
	}
	fclose(infile);
	for (i = 0; i < MAX_FACE_SETS; i++)
	{
		if (facesets[i].prefix)
			check_faceset_fallback(i, MAX_FACE_SETS);
	}
	/* Loaded the faceset information - now need to load up the
	* actual faces.
	*/

	for (fileno = 0; fileno < MAX_FACE_SETS; fileno++)
	{
		/* if prefix is not set, this is not used */
		if (!facesets[fileno].prefix)
			continue;
		facesets[fileno].faces = calloc(nrofpixmaps, sizeof(FaceInfo));

		sprintf(filename, "%s/daimonin.%d", settings.datadir, fileno);
		LOG(llevDebug, "Loading image file %s\n", filename);

		/* we don't use more as one face set here!! */
		LOG(llevInfo, "Creating client_bmap....\n");
		sprintf(buf, "%s/client_bmaps", settings.localdir);
		if ((fbmap = fopen(buf, "wb")) == NULL)
			LOG(llevError, "Unable to open %s\n", buf);

		if ((infile = fopen(filename, "r")) == NULL)
			LOG(llevError, "Unable to open %s\n", filename);

		while (fgets(buf, HUGE_BUF - 1, infile) != NULL)
		{
			if (strncmp(buf, "IMAGE ", 6) != 0)
				LOG(llevError, "read_client_images:Bad image line - not IMAGE, instead\n%s", buf);
			num = atoi(buf + 6);
			if (num < 0 || num >= nrofpixmaps)
				LOG(llevError, "read_client_images: Image num %d not in 0..%d\n%s", num, nrofpixmaps, buf);
			/* Skip accross the number data */
			for (cp = buf + 6; *cp != ' '; cp++)
				;
			len = atoi(cp);
			if (len == 0 || len > MAX_IMAGE_SIZE)
				LOG(llevError, "read_client_images: length not valid: %d > %d \n%s", len, MAX_IMAGE_SIZE, buf);
			/* We don't actualy care about the name if the image that
			* is embedded in the image file, so just ignore it.
			*/
			facesets[fileno].faces[num].datalen = len;
			facesets[fileno].faces[num].data = malloc(len);
			if ((i = fread(facesets[fileno].faces[num].data, len, 1, infile)) != 1)
				LOG(llevError, "read_client_images: Did not read desired amount of data, wanted %d, got %d\n%s", len, i,
				buf);

			facesets[fileno].faces[num].checksum = (uint32) crc32(1L, facesets[fileno].faces[num].data, len);
			sprintf(buf, "%x %x %s\n", len, facesets[fileno].faces[num].checksum, new_faces[num].name);
			fputs(buf, fbmap);
		}
		fclose(infile);
		fclose(fbmap);
	} /* For fileno < MAX_FACE_SETS */
}

/*
* Client tells us what type of faces it wants.  Also sets
* the caching attribute.
*
*/

void SetFaceMode(char *buf, int len, NewSocket *ns)
{
	char                    tmp[256];

	int mask = (atoi(buf)  &CF_FACE_CACHE), mode = (atoi(buf) & ~CF_FACE_CACHE);

	if (mode == CF_FACE_NONE)
	{
		ns->facecache = 1;
	}
	else if (mode != CF_FACE_PNG)
	{
		sprintf(tmp, "%d %s", NDI_RED, "Warning - send unsupported face mode.  Will use Png");
		Write_String_To_Socket(ns, BINARY_CMD_DRAWINFO, tmp, strlen(tmp));
#ifdef ESRV_DEBUG
		LOG(llevDebug, "SetFaceMode: Invalid mode from client: %d\n", mode);
#endif
	}
	if (mask)
	{
		ns->facecache = 1;
	}
}

/* client has requested pixmap that it somehow missed getting
* This will be called often if the client is
* caching images.
*/

void SendFaceCmd(char *buff, int len, NewSocket *ns)
{
	long    tmpnum;
	short   facenum;

	if (!buff || !len)
		return;

	tmpnum  = atoi(buff);
	facenum = (short)(tmpnum & 0xffff);

	if (facenum != 0)
		esrv_send_face(ns, facenum, 1);
}

/*
* esrv_send_face sends a face to a client if they are in pixmap mode
* nothing gets sent in bitmap mode.
* If nocache is true (nonzero), ignore the cache setting from the client -
* this is needed for the askface, in which we really do want to send the
* face (and askface is the only place that should be setting it).  Otherwise,
* we look at the facecache, and if set, send the image name.
*/
/* return: 0 - all ok. 1: face nr out of bound, 2: face data not avaible
* define in global.h:
* #define SEND_FACE_OK 0
* #define SEND_FACE_OUT_OF_BOUNDS 1
* #define SEND_FACE_NO_DATA 2
*/
int esrv_send_face(NewSocket *ns, short face_num, int nocache)
{
	int         fallback, cmdmode;

	if (face_num < 0 || face_num >= nrofpixmaps)
	{
		LOG(llevBug, "BUG: esrv_send_face (%d) out of bounds??\n", face_num);
		return SEND_FACE_OUT_OF_BOUNDS;
	}

	fallback = get_face_fallback(ns->faceset, face_num);

	if (facesets[fallback].faces[face_num].data == NULL)
	{
		LOG(llevBug, "BUG: esrv_send_face: faces[%d].data == NULL\n", face_num);
		return SEND_FACE_NO_DATA;
	}

	if (ns->facecache && !nocache)
	{
		if (ns->image2)
			cmdmode = BINARY_CMD_FACE2;
		else if (ns->sc_version >= 1026)
			cmdmode = BINARY_CMD_FACE1;
		else
			cmdmode = BINARY_CMD_FACE;

		SOCKBUF_REQUEST_BUFFER(ns, SOCKET_SIZE_MEDIUM);

		SockBuf_AddShort(ACTIVE_SOCKBUF(ns), face_num);
		if (ns->image2)
			SockBuf_AddChar(ACTIVE_SOCKBUF(ns), (char) fallback);
		if (ns->sc_version >= 1026)
			SockBuf_AddInt(ACTIVE_SOCKBUF(ns), facesets[fallback].faces[face_num].checksum);
		SockBuf_AddString(ACTIVE_SOCKBUF(ns), new_faces[face_num].name, strlen(new_faces[face_num].name));

		SOCKBUF_REQUEST_FINISH(ns, cmdmode, SOCKBUF_DYNAMIC);
	}
	else
	{
		if (ns->image2)
			cmdmode = BINARY_CMD_IMAGE2;
		else
			cmdmode = BINARY_CMD_IMAGE;

		SOCKBUF_REQUEST_BUFFER(ns, SOCKET_SIZE_MEDIUM);

		SockBuf_AddInt(ACTIVE_SOCKBUF(ns), face_num);
		if (ns->image2)
			SockBuf_AddChar(ACTIVE_SOCKBUF(ns), (char) fallback);
		SockBuf_AddInt(ACTIVE_SOCKBUF(ns), (uint32)facesets[fallback].faces[face_num].datalen);
		SockBuf_AddString(ACTIVE_SOCKBUF(ns), facesets[fallback].faces[face_num].data, facesets[fallback].faces[face_num].datalen);

		SOCKBUF_REQUEST_FINISH(ns, cmdmode, SOCKBUF_DYNAMIC);
	}
	/*ns->faces_sent[face_num] = 1;*/

	return SEND_FACE_OK;
}

#endif

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
        while(i >= 0 && isspace(buf[i]))
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
        while(i >= 0 && isspace(q[i]))
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
