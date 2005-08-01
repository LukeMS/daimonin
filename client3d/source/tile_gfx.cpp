/*-----------------------------------------------------------------------------
This source file is part of Daimonin (http://daimonin.sourceforge.net)
Copyright (c) 2005 The Daimonin Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/licenses/licenses.html
-----------------------------------------------------------------------------*/

#include <string>
#include <iostream>
#include <fstream>
#ifdef WIN32
  #include <io.h>
  #include <stdio.h>
#endif
#include "tile_gfx.h"
#include "define.h"
#include "logfile.h"
#include "zlib.h"

using namespace std;

#ifdef MINGW
  #define _NO_OLDNAMES
#endif

//=================================================================================================
//
//=================================================================================================
bool TileGfx::Init()
{
    return (read_bmaps_p0());
}

//=================================================================================================
//
//=================================================================================================
unsigned long TileGfx::hashbmap(char *str, int tablesize)
{
    unsigned long hash = 0;
    int     i = 0;
	unsigned int rot = 0;
    char   *p;

    for (p = str; i < MAXHASHSTRING && *p; p++, i++)
    {
        hash ^= (unsigned long) * p << rot;
        rot += 2;
        if (rot >= (sizeof(long) - sizeof(char)) * 8)
            rot = 0;
    }
    return (hash % tablesize);
}

//=================================================================================================
//
//=================================================================================================
_bmaptype * TileGfx::find_bmap(char *name)
{
    _bmaptype  *at;
    unsigned long index;

    if (name == NULL)
        return (_bmaptype *) NULL;

    index = hashbmap(name, BMAPTABLE);
    for (; ;)
    {
        at = bmap_table[index];
        if (at == NULL) { return NULL; } // not in our bmap list
        if (!strcmp(at->name, name)) { return at; }
        if (++index >= BMAPTABLE)	 { index = 0; }
    }
}

//=================================================================================================
//
//=================================================================================================
void TileGfx::add_bmap(_bmaptype *at)
{
    unsigned int index = hashbmap(at->name,  BMAPTABLE),org_index = index;

    for (; ;)
    {
        if (bmap_table[index] && !strcmp(bmap_table[index]->name, at->name))
        {
            LogFile::getSingleton().Error("ERROR: add_bmap(): double use of bmap name %s\n", at->name);
        }
        if (bmap_table[index] == NULL)
        {
            bmap_table[index] = at;
            return;
        }
        if (++index == BMAPTABLE)
            index = 0;
        if (index == org_index)
            LogFile::getSingleton().Error("ERROR: add_bmap(): bmaptable to small\n");
    }
}

//=================================================================================================
// after we tested and/or created bmaps.p0 - load the data from it.
//=================================================================================================
bool TileGfx::load_bmaps_p0()
{
    char    buf[HUGE_BUF];
    char    name[HUGE_BUF];
    int     len, pos, num;
    unsigned int crc;
    _bmaptype  *at;
    FILE       *fbmap;

    // clear bmap hash table
    memset((void *) bmap_table, 0, BMAPTABLE * sizeof(_bmaptype *));

    // try to open bmaps_p0 file
    if ((fbmap = fopen(FILE_BMAPS_P0, "rb")) == NULL)
    {
        LogFile::getSingleton().Error("FATAL: Error loading bmaps.p0!");
        unlink(FILE_BMAPS_P0);
		return -1;
    }
    while (fgets(buf, HUGE_BUF - 1, fbmap) != NULL)
    {
        sscanf(buf, "%d %d %x %d %s", &num, &pos, &crc, &len, name);

        at = new _bmaptype;
        at->name = new char[strlen(name) + 1];
        strcpy(at->name, name);
        at->crc = crc;
        at->num = num;
        at->len = len;
        at->pos = pos;
        add_bmap(at);
        // LogFile::getSingleton().Info("%d %d %d %x >%s<\n", num, pos, len, crc, name);
    }
    fclose(fbmap);
	return 0;
}

//=================================================================================================
// read and/or create the bmaps.p0 file out of the daimonin.p0 file
//=================================================================================================
bool TileGfx::read_bmaps_p0()
{
    FILE   *fbmap, *fpic;
    char   *temp_buf, *cp;
    int     bufsize, len, num, pos;
    unsigned int crc;
    char        buf[HUGE_BUF];
    struct stat bmap_stat, pic_stat;

    if (!(fpic = fopen(FILE_DAIMONIN_P0, "rb")))
    {
        LogFile::getSingleton().Error("FATAL: Can't find daimonin.p0 file!");
        unlink(FILE_BMAPS_P0);
		return false;
    }
    // get time stamp of the file daimonin.p0
    fstat(fileno(fpic), &pic_stat);

    // try to open bmaps_p0 file
    if (!(fbmap = fopen(FILE_BMAPS_P0, "r"))) { goto create_bmaps; }

    // get time stamp of the file
    fstat(fileno(fbmap), &bmap_stat);
    fclose(fbmap);

    if (difftime(pic_stat.st_mtime, bmap_stat.st_mtime) > 0.0f)
        goto create_bmaps;

    fclose(fpic);
    return load_bmaps_p0();

    create_bmaps: // if we are here, then we have to (re)create the bmaps.p0 file
    if ((fbmap = fopen(FILE_BMAPS_P0, "w")) == NULL)
    {
        LogFile::getSingleton().Error("FATAL: Can't create bmaps.p0 file!");
        fclose(fbmap);
        unlink(FILE_BMAPS_P0);
		return false;
    }
    temp_buf = new char[bufsize = 24 * 1024];

    while (fgets(buf, HUGE_BUF - 1, fpic) != NULL)
    {
        if (strncmp(buf, "IMAGE ", 6) != 0)
        {
            LogFile::getSingleton().Error("read_client_images:Bad image line - not IMAGE, instead\n%s", buf);
            fclose(fbmap);
            fclose(fpic);
            unlink(FILE_BMAPS_P0);
			return false;
        }

        num = atoi(buf + 6);
        // Skip accross the number data
        for (cp = buf + 6; *cp != ' '; cp++) { ; }
        len = atoi(cp);
        strcpy(buf, cp);
        pos = (int) ftell(fpic);

        if (len > bufsize) // dynamic buffer adjustment
        {
            free(temp_buf);
            // we assume that this is nonsense
            if (len > 128 * 1024)
            {
                LogFile::getSingleton().Error("read_client_images:Size of picture out of bounds!(len:%d)(pos:%d)", len, pos);
                fclose(fbmap);
                fclose(fpic);
                unlink(FILE_BMAPS_P0);
				return false;
            }
            bufsize = len;
            temp_buf = new char[bufsize];
        }

        fread(temp_buf, 1, len, fpic);
        crc = crc32(1L, (const unsigned char*)temp_buf, len);

        // now we got all we needed!
        sprintf(temp_buf, "%d %d %x %s", num, pos, crc, buf);
        fputs(temp_buf, fbmap);
        //LogFile::getSingleton().Info("FOUND: %s", temp_buf);
    }
    delete[] temp_buf;
    fclose(fbmap);
    fclose(fpic);
    return load_bmaps_p0();
} 


//=================================================================================================
// 
//=================================================================================================
void TileGfx::delete_bmap_tmp(void)
{
    int i;

    bmaptype_table_size = 0;
    for (i = 0; i < MAX_BMAPTYPE_TABLE; i++)
    {
        if (bmaptype_table[i].name) { delete[] bmaptype_table[i].name; }
        bmaptype_table[i].name = NULL;
    }
} 

//=================================================================================================
// 
//=================================================================================================
bool TileGfx::load_bmap_tmp(void)
{
    FILE   *stream;
    char    buf[HUGE_BUF], name[HUGE_BUF];
    int     i = 0, len, pos;
    unsigned int crc;

    delete_bmap_tmp();
    if ((stream = fopen(FILE_BMAPS_TMP, "rt")) == NULL)
    {
        LogFile::getSingleton().Error("bmaptype_table(): error open file <bmap.tmp>");
//        SYSTEM_End();
//        exit(0);
		return false;
    }
    while (fgets(buf, HUGE_BUF - 1, stream) != NULL)
    {
        sscanf(buf, "%d %d %x %s\n", &pos, &len, &crc, name);
        bmaptype_table[i].crc = crc;
        bmaptype_table[i].len = len;
        bmaptype_table[i].pos = pos;
        bmaptype_table[i].name = new char[strlen(name) + 1];
        strcpy(bmaptype_table[i].name, name);
        i++;
    }
    bmaptype_table_size = i;
    fclose(stream);
    return true;
}

 

//=================================================================================================
// 
//=================================================================================================
bool TileGfx::read_bmap_tmp(void)
{
    FILE       *stream, *fbmap0;
    char        buf[HUGE_BUF], name[HUGE_BUF];
    struct stat stat_bmap, stat_tmp, stat_bp0;
    int         len;
    unsigned int crc;
    _bmaptype  *at;

    if ((stream = fopen(FILE_CLIENT_BMAPS, "rb")) == NULL)
    {
        // we can't make bmaps.tmp without this file
        unlink(FILE_BMAPS_TMP);
        return false;
    }
    fstat(fileno(stream), &stat_bmap);
    fclose(stream);

    if ((stream = fopen(FILE_BMAPS_P0, "rb")) == NULL)
    {
        // we can't make bmaps.tmp without this file
        unlink(FILE_BMAPS_TMP);
        return false;
    }
    fstat(fileno(stream), &stat_bp0);
    fclose(stream);

    if ((stream = fopen(FILE_BMAPS_TMP, "rb")) == NULL)
        goto create_bmap_tmp;
    fstat(fileno(stream), &stat_tmp);
    fclose(stream);

    // ok - client_bmap & bmaps.p0 are there - now check
    // our bmap_tmp is newer - is not newer as both, we
    // create it new - then it is newer.
    if (difftime(stat_tmp.st_mtime, stat_bmap.st_mtime) > 0.0f)
    {
        if (difftime(stat_tmp.st_mtime, stat_bp0.st_mtime) > 0.0f)
            return load_bmap_tmp(); // all fine.
    }

    create_bmap_tmp:
    unlink(FILE_BMAPS_TMP);

    // NOW we are sure... we must create us a new bmaps.tmp
    if ((stream = fopen(FILE_CLIENT_BMAPS, "rb")) != NULL)
    {
        // we can use text mode here, its local
        if ((fbmap0 = fopen(FILE_BMAPS_TMP, "wt")) != NULL)
        {
            // read in the bmaps from the server, check with the
            // loaded bmap table (from bmaps.p0) and create with
            // this information the bmaps.tmp file.
            while (fgets(buf, HUGE_BUF - 1, stream) != NULL)
            {
                sscanf(buf, "%x %x %s", &len, &crc, name);
                at = find_bmap(name);

                // now we can check, our local file package has the right png.
				// if not, we mark this pictures as "in cache". 
				// We don't check it here now - that will happens at runtime.
                // That can change when we include later a forbidden
                // flag in the server (no face send) - then we need
                // to break and upddate the picture and/or check the cache.
				//
                // position -1 mark "not i the daimonin.p0 file
                if (!at || at->len != len || at->crc != crc) // is different or not there!
                    sprintf(buf, "-1 %d %x %s\n", len, crc, name);
                else // we have it
                    sprintf(buf, "%d %d %x %s\n", at->pos, len, crc, name);
                fputs(buf, fbmap0);
            }
            fclose(fbmap0);
        }
        fclose(stream);
    }
    return load_bmap_tmp(); // all fine
} 

//=================================================================================================
// we have stored this picture in daimonin.p0 - load it from there.
//=================================================================================================
int TileGfx::load_picture_from_pack(int num)
{
	std::ifstream fp;
    fp.open(FILE_DAIMONIN_P0, ios::in|ios::binary);
    if(!fp) { return false; }
	// Wrap as a stream
	DataStreamPtr stream(new FileStreamDataStream(FILE_DAIMONIN_P0, &fp, false));
	stream->seek(bmaptype_table[num].pos);
//	LogFile::getSingleton().Info("pos: %d\n", bmaptype_table[0].pos);
//	stream->seek(24);
	FaceList[num].sprite.load(stream,"png");
    stream.setNull();
    return 0;
} 

//=================================================================================================
// 
//=================================================================================================
void TileGfx::face_flag_extension(int pnum, char *buf)
{
/*
    char   *stemp;

    FaceList[pnum].flags = FACE_FLAG_NO;
    // check for the "double"/"up" tag in the picture name
    if ((stemp = strstr(buf, ".d")))
        FaceList[pnum].flags |= FACE_FLAG_DOUBLE;
    else if ((stemp = strstr(buf, ".u")))
        FaceList[pnum].flags |= FACE_FLAG_UP;

    // Now the facing stuff: if some tag was there, lets grap the facing info
    if (FaceList[pnum].flags && stemp)
    {
        int tc;
        for (tc = 0; tc < 4; tc++)
        {
            if (!*(stemp + tc)) // has the string a '0' before our anim tags
                goto finish_face_cmd_j1;
        }
        // lets set the right flags for the tags
        if (((FaceList[pnum].flags & FACE_FLAG_UP) && *(stemp + tc) == '5') || *(stemp + tc) == '1')
            FaceList[pnum].flags |= FACE_FLAG_D1;
        else if (*(stemp + tc) == '3')
            FaceList[pnum].flags |= FACE_FLAG_D3;
        else if (*(stemp + tc) == '4' || *(stemp + tc) == '8' || *(stemp + tc) == '0')
            FaceList[pnum].flags |= (FACE_FLAG_D3 | FACE_FLAG_D1);
    }
    finish_face_cmd_j1: // error jump from for()
*/
    return;
}
