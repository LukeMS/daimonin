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

#ifndef DEFINE_H
#define DEFINE_H

#define DAIMONIN // Delete me for CodeBlack !!!

#ifdef DAIMONIN
  #define SINGLE_CHUNK
#endif

const char PRG_NAME[] =  "Daimonin Ogre3d Client";

const char OVERLAY_TYPE_NAME[] = "Panel"; // defined in OverlayElementFactory.h
const char FILE_HEIGHT_MAP[]   = "Hoehenkarte.png";

///========================================================================
/// All pathes MUST be placed here!
///========================================================================
const char FILE_GUI_IMAGESET[]          = "./media/xml/GUI_ImageSet.xml";
const char FILE_GUI_WINDOWS[]           = "./media/xml/GUI_Windows.xml";

const char PATH_TEXTURES[]              = "./media/textures/";
const char FILE_SYSTEM_FONT[]           = "SystemFont.png";

const char PATH_TILE_TEXTURES[]         = "./media/textures/tiles/";

const char FILE_CLIENT_SPELLS[]         = "./srv_files/client_spells";
const char FILE_CLIENT_SKILLS[]         = "./srv_files/client_skills";
const char FILE_CLIENT_SETTINGS[]       = "./srv_files/client_settings";
const char FILE_CLIENT_BMAPS[]          = "./srv_files/client_bmap";
const char FILE_CLIENT_ANIMS[]          = "./srv_files/client_anims";

const char FILE_BMAPS_TMP[]             = "./srv_files/bmaps.tmp";
const char FILE_ANIMS_TMP[]             = "./srv_files/anims.tmp";
const char FILE_DAIMONIN_P0[]           = "./daimonin.p0";
const char FILE_BMAPS_P0[]              = "./bmaps.p0";
const char FILE_ARCHDEF[]               = "./archdef.dat";

const char FILE_LOGGING[]               = "./client_log.html";
const char FILE_OPTIONS[]               = "./options.dat";

const char PATH_MODEL_DESCRIPTION[]     = "./media/models/";
const char FILE_PLAYER_DESC[]           = "./media/models/player.desc";
const char FILE_WORLD_DESC[]            = "./media/models/world.desc";

const char PATH_SAMPLES[]               = "./media/sound/";
const char FILE_MUSIC_001[]             = "./media/sound/invtro94.s3m";
const char FILE_SAMPLE_DUMMY[]          = "./media/sound/dummy.wav";
const char FILE_SAMPLE_MOUSE_CLICK[]    = "./media/sound/console.wav";
const char FILE_SAMPLE_PLAYER_IDLE[]    = "./media/sound/Player_Idle.ogg";
const char FILE_SAMPLE_WRONG_INPUT[]    = "./media/sound/missspell.wav";


///========================================================================
///
///========================================================================
const int HUGE_BUF = 1024;

enum
{
  M_MOVED, M_PRESSED, M_CLICKED, M_DRAGGED, M_ENTERED, M_EXITED, M_RELEASED
};

typedef struct _sPicture
{
  short x, y;
  short w, h;
}
sPicture;



#endif
