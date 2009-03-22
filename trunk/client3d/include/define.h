/*-----------------------------------------------------------------------------
This source file is part of Daimonin's 3d-Client
Daimonin is a MMORG. Details can be found at http://daimonin.sourceforge.net
Copyright (c) 2005 Andreas Seidel

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

In addition, as a special exception, the copyright holder of client3d give
you permission to combine the client3d program with lgpl libraries of your
choice. You may copy and distribute such a system following the terms of the
GNU GPL for 3d-Client and the licenses of the other code concerned.

You should have received a copy of the GNU General Public License along with
this program; If not, see <http://www.gnu.org/licenses/>.
-----------------------------------------------------------------------------*/

#ifndef DEFINE_H
#define DEFINE_H

const char PRG_NAME[] =  "Daimonin Ogre3d Client";
//================================================================================================
// All pathes MUST be placed here!
//================================================================================================
const char PATH_DESCRIPTION[]        = "./media/xml/";
const char PATH_TEXTURES[]           = "./media/textures/";
const char PATH_SAMPLES[]            = "./media/sound/";
//const char PATH_MODEL_DESCRIPTION[]  = "./media/models/";

const char FILE_OPTIONS[]            = "./options.dat";
const char FILE_DAIMONIN_P0[]        = "./daimonin.p0";
const char FILE_BMAPS_P0[]           = "./bmaps.p0";
//const char FILE_ARCHDEF[]            = "./archdef.dat";

const char FILE_CLIENT_SPELLS[]      = "./srv_files/client_spells";
const char FILE_CLIENT_SKILLS[]      = "./srv_files/client_skills";
const char FILE_CLIENT_SETTINGS[]    = "./srv_files/client_settings";
const char FILE_CLIENT_BMAPS[]       = "./srv_files/client_bmap";
const char FILE_CLIENT_ANIMS[]       = "./srv_files/client_anims";
const char FILE_BMAPS_UNIQUE[]       = "./srv_files/bmaps_unique";      /**< The objects from bmaps without animation states */
const char FILE_BMAPS_CONVERT[]      = "./srv_files/bmaps_convert";     /**< This file holds the unique client2d and client3d obj names */
const char FILE_BMAPS_CLIENT3D[]     = "./srv_files/bmaps_3d";          /**< The final client3d object file */
//const char FILE_BMAPS_TMP[]          = "./srv_files/bmaps.tmp";
//const char FILE_ANIMS_TMP[]          = "./srv_files/anims.tmp";
const char FILE_NPC_VISUALS[]        = "./media/xml/NPC_Visuals.xml";
const char PATH_TILE_TEXTURES[]      = "./media/textures/tiles/";

#endif
