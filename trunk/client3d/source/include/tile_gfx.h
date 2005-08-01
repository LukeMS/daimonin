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

#ifndef TILEGFX_H
#define TILEGFX_H

#include <Ogre.h>
#include <string>
#include "define.h"

using namespace Ogre;

////////////////////////////////////////////////////////////
// Defines.
////////////////////////////////////////////////////////////
const int MAXHASHSTRING		=   20; // for hash table (bmap, ...)
const unsigned int BMAPTABLE= 5003; // prime nubmer for hash table
const int MAX_BMAPTYPE_TABLE= 5000;
const int MAX_FACE_TILES	=10000; 

// struct for out bmap data
typedef struct _bmaptype
{
	char			*name;
    int				num;
    int				len;
    int				pos;
    unsigned int	crc;
}_bmaptype;

typedef struct  _bmaptype_table
{
    char           *name;
    int             pos;
    int             len;
    unsigned int    crc;
}_bmaptype_table;

typedef struct _face_struct
{
//    struct _Sprite *sprite;		// our face data. if != null, face is loaded.
	Image			sprite;
    char           *name;		// our face name. if != null, face is requested.
    unsigned int    checksum;	// checksum of face.
    int             flags;
}_face_struct;
 

////////////////////////////////////////////////////////////
// Defines.
////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
// Singleton class.
////////////////////////////////////////////////////////////
class TileGfx
{
  public:
    ////////////////////////////////////////////////////////////
	// Variables.
    ////////////////////////////////////////////////////////////
    static TileGfx &getSingleton() { static TileGfx Singleton; return Singleton; }
	unsigned long hashbmap(char *str, int tablesize);
	_bmaptype *bmap_table[BMAPTABLE];
	_bmaptype_table bmaptype_table[MAX_BMAPTYPE_TABLE];
	_face_struct FaceList[MAX_FACE_TILES];   // face data

    ////////////////////////////////////////////////////////////
	// Functions.
	////////////////////////////////////////////////////////////
     TileGfx() {;}
    ~TileGfx() {;}

    bool Init();
	bool load_bmaps_p0(void);
	bool read_bmaps_p0(void);
	bool read_bmap_tmp(void);
	bool load_bmap_tmp(void);
	void add_bmap(_bmaptype *at);
	void delete_bmap_tmp(void);
	void face_flag_extension(int pnum, char *buf);
	_bmaptype *find_bmap(char *name);
	int load_picture_from_pack(int num);
	Image &getSprite(int num) { return FaceList[num].sprite; }

	int bmaptype_table_size;	
  private:
    ////////////////////////////////////////////////////////////
	// Variables.
    ////////////////////////////////////////////////////////////


    ////////////////////////////////////////////////////////////
	// Functions.
    ////////////////////////////////////////////////////////////
    TileGfx(const TileGfx&); // disable copy-constructor.

}; 

#endif
