/*
-----------------------------------------------------------------------------
This source file is part of Daimonin (http://daimonin.sourceforge.net)

Copyright (c) 2005 The Daimonin Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/
#ifndef DEFINE_H
#define DEFINE_H

// ========================================================================
// Pathes.
// ========================================================================
const char FILE_CLIENT_SPELLS[]   = "./srv_files/client_spells";
const char FILE_CLIENT_SKILLS[]   = "./srv_files/client_skills";
const char FILE_CLIENT_SETTINGS[] = "./srv_files/client_settings";
const char FILE_CLIENT_BMAPS[]    = "./srv_files/client_bmap";
const char FILE_CLIENT_ANIMS[]    = "./srv_files/client_anims";

const char FILE_BMAPS_TMP[]       = "./srv_files/bmaps.tmp";
const char FILE_ANIMS_TMP[]       = "./srv_files/anims.tmp";
const char FILE_DAIMONIN_P0[]     = "./daimonin.p0";
const char FILE_BMAPS_P0[]        = "./bmaps.p0";
const char ARCHDEF_FILE[]         = "./archdef.dat";


enum { M_MOVED, M_PRESSED, M_CLICKED, M_DRAGGED, M_ENTERED, M_EXITED, M_RELEASED };

#endif
