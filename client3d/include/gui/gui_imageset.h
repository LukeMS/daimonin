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

#ifndef GUI_IMAGESET_H
#define GUI_IMAGESET_H

#include <Ogre.h>
#include <tinyxml.h>
#include <vector>
#include "gui_manager.h"

/**
 ** This singleton class stores the graphic positions of all gui elements.
 ** All graphics are stored in a single gfx-file.
 ** Each graphic can have serveral states (like: pressed, mouse over, etc).
 *****************************************************************************/
class GuiImageset
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    enum { ITEM_SIZE = 48 };
    typedef struct
    {
        const char *name;
        unsigned int index;
    }
    StateNames;

    /** Actual state of an GuiElement: **/
    enum
    {
        STATE_ELEMENT_DEFAULT,
        STATE_ELEMENT_PUSHED,
        STATE_ELEMENT_M_OVER,  /**< Mouse over. **/
        STATE_ELEMENT_PASSIVE, /**< Disabled. **/
        STATE_ELEMENT_SUM
    };
    static StateNames mElementState[STATE_ELEMENT_SUM];

    typedef struct
    {
        short x, y;
    }
    gfxPos;

    typedef struct
    {
        int w, h;
        gfxPos state[GuiManager::STATE_MOUSE_SUM];
    }
    gfxSrcMouse;

    typedef struct
    {
        Ogre::String name;
#ifdef D_DEBUG
        bool isUsed; /**< Used by any gui-elements, or an orphaned entry? **/
#endif
        int w, h;
        gfxPos state[STATE_ELEMENT_SUM];
    }
    gfxSrcEntry;

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    static GuiImageset &getSingleton()
    {
        static GuiImageset singleton; return singleton;
    }
    void parseXML(const char *XML_imageset_file);
    /**  Returns the array of the gfx positions for the mouse-cursor. **/
    gfxSrcMouse *getStateGfxPosMouse()
    {
        return mSrcEntryMouse;
    }
    gfxSrcEntry *getStateGfxPositions(const char* guiImage);
    const Ogre::PixelBox &getPixelBox()
    {
        return mSrcPixelBox;
    }
    void delBackgroundElements();
    int getItemId(const char *gfxName);
    const Ogre::PixelBox &getItemPB(int itemId);

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    gfxSrcMouse *mSrcEntryMouse;
    std::vector<gfxSrcEntry*> mvSrcEntry;
    std::vector<Ogre::String> mvAtlasGfxName;
    Ogre::String mStrImageSetGfxFile;
    Ogre::Image mImageSetImg;
    Ogre::PixelBox mSrcPixelBox;
    Ogre::Image mAtlasTexture;
    Ogre::PixelBox mSrcPb;
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    GuiImageset();
    ~GuiImageset();
    void parseItems();
    GuiImageset(const GuiImageset&); // disable copy-constructor
    bool parseStates(TiXmlElement *xmlElem, gfxPos *Entry, int sum_state, bool mouseStates);
};

#endif
