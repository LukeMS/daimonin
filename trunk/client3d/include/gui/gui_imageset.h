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

#ifndef GUI_IMAGESET_H
#define GUI_IMAGESET_H

#include <string>
#include <vector>
#include <Ogre.h>
#include <tinyxml.h>

//#include "gui_window.h"
//#include "gui_cursor.h"
//#include "gui_manager.h"
#include "logger.h"

using namespace Ogre;

enum {
    // Button.
    GUI_BUTTON_CLOSE,
    GUI_BUTTON_OK,
    GUI_BUTTON_CANCEL,
    GUI_BUTTON_MINIMIZE,
    GUI_BUTTON_MAXIMIZE,
    GUI_BUTTON_RESIZE,
    // Listboxes.
    GUI_LIST_MSGWIN,
    GUI_LIST_CHATWIN,
    GUI_LIST_UP,
    GUI_LIST_DOWN,
    GUI_LIST_LEFT,
    GUI_LIST_RIGHT,
    // StatusBars.
    GUI_STATUSBAR_PLAYER_HEALTH,
    GUI_STATUSBAR_PLAYER_MANA,
    GUI_STATUSBAR_PLAYER_GRACE,
    // TextValues.
    GUI_TEXTVALUE_STAT_CUR_FPS,
    GUI_TEXTVALUE_STAT_BEST_FPS,
    GUI_TEXTVALUE_STAT_WORST_FPS,
    GUI_TEXTVALUE_STAT_SUM_TRIS,
    // TextInput
    GUI_TEXTINPUT_PASSWORD,
    // Combobox
    GUI_COMBOBOX_TEST,
    // Sum of all entries.
    GUI_ELEMENTS_SUM
};

typedef struct
{
    std::string name;
    short x, y;
}
GuiElementState;

typedef struct
{
    std::string name;
    int width, height;
    std::vector<GuiElementState*>state;
}
GuiSrcEntry;

typedef struct
{
    const char *name;
    unsigned int index;
}
GuiElementNames;


class GuiImageset
{
public:
    /// ////////////////////////////////////////////////////////////////////
    /// Variables.
    /// ////////////////////////////////////////////////////////////////////

    /// ////////////////////////////////////////////////////////////////////
    /// Functions.
    /// ////////////////////////////////////////////////////////////////////

    static GuiImageset &getSingleton()
    {
        static GuiImageset singleton; return singleton;
    }
    void parseXML(const char *XML_imageset_file);
    GuiSrcEntry *getStateGfxPositions(const char* guiImage);
    PixelBox &getPixelBox()
    {
        return mSrcPixelBox;
    }
    const char *getElementName(int i)
    {
        return mGuiElementNames[i].name;
    }
    int getElementIndex(int i)
    {
        return mGuiElementNames[i].index;
    }

private:
    /// ////////////////////////////////////////////////////////////////////
    /// Variables.
    /// ////////////////////////////////////////////////////////////////////
    std::vector<GuiSrcEntry*>mvSrcEntry; // TODO: delete vector in destructor.
    std::string mStrImageSetGfxFile;
    Image mImageSetImg;
    PixelBox mSrcPixelBox;
    static GuiElementNames mGuiElementNames[GUI_ELEMENTS_SUM];

    /// ////////////////////////////////////////////////////////////////////
    /// Functions.
    /// ////////////////////////////////////////////////////////////////////
    GuiImageset()
    {}
    ~GuiImageset();
    GuiImageset(const GuiImageset&); // disable copy-constructor.
};

#endif
