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

In addition, as a special exception, the copyright holders of client3d give
you permission to combine the client3d program with lgpl libraries of your
choice and/or with the fmod libraries.
You may copy and distribute such a system following the terms of the GNU GPL
for client3d and the licenses of the other code concerned.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/licenses/licenses.html
-----------------------------------------------------------------------------*/

#include "option.h"

Option::optionStruct Option::optStruct[SEPARATOR] =
    {
        /// ////////////////////////////////////////////////////////////////////
        /// Sound settings.
        /// ////////////////////////////////////////////////////////////////////
        { SEL_INT_RANGE, "Sound volume:", "set sound volume for effects.","", "",
          "_", 100, 0,100,5, false
        },
        { SEL_INT_RANGE,"Music volume:", "set music volume for background.","Sub info","",
          "_", 100, 0,100,5, false
        },
        { SEL_INT_RANGE,"Voice volume:", "set voice volume.","Sub info","",
          "_", 100, 0,100,5, true
        },
        /// ////////////////////////////////////////////////////////////////////
        /// Server settings.
        /// ////////////////////////////////////////////////////////////////////
        { SEL_TXT_RANGE,"MetaServer:", "gdfhdfhdfh.","Sub info","",
          "damn.informatik.uni-bremen.de", 0, 0,100,5, false
        },
        { SEL_INT_RANGE,"Metaserver port:", "","","",
          "_", 13326, 0,100,5, true
        },
    };

std::string Option::optValue[SUM_OPTIONS - SEPARATOR-1] =
    {
        "0", // SEL_META_SEVER
        "0", // HIGH_TEXTURE_DETAILS
        "0", // HIGH_TILES_DETAILS
        "0", // UPDATE_NETWORK
        "0", // CMDLINE_LOG_GUI_ELEMENTS
        "0", // CMDLINE_CREATE_RAW_FONTS
        "0", // CMDLINE_CREATE_TILE_TEXTURES
        "0", // CMDLINE_SERVER_NAME
        "0", // CMDLINE_SERVER_PORT
        "0", // CMDLINE_FALLBACK
        "0", // CMDLINE_OFF_SOUND
        "0", // CMDLINE_SHOW_BOUNDING_BOX
    };

