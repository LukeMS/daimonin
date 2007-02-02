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

#ifndef GUI_GADGET_SLOT_H
#define GUI_GADGET_SLOT_H

#include <tinyxml.h>
#include <Ogre.h>
#include "gui_element.h"
#include "gui_window.h"

/**
 ** This class provides an interactive button.
 *****************************************************************************/
class GuiGadgetSlot: public GuiElement
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    enum
    {
        SLOT_CLEAR  = -2,
        SLOT_UPDATE = -1,
    };
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    GuiGadgetSlot(TiXmlElement *xmlElement, void *parent, bool drawOnInit = true);
    ~GuiGadgetSlot();
    int mouseEvent(int MouseAction, int x, int y);
    void draw();
    void drawSlot(int slotNr, int intgfxNr, int state);

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    static Ogre::Image mAtlasTexture;
    static Ogre::Overlay *mDnDOverlay;
    static Ogre::OverlayElement *mDnDElement;
    static Ogre::MaterialPtr mDnDMaterial;
    static Ogre::TexturePtr mDnDTexture;
    std::vector<Ogre::String> mvGfxPositions;
    bool mMouseOver, mMouseButDown;
    bool mActiveDrag;
    int mDragSlot;                  /**< Slot where the drag was started. **/
    int mActiveSlot;                /**< Slot the mouse is currently over. **/
    int mSumCol, mSumRow;
    int mColSpace, mRowSpace;       /**< Space between the slots. **/
    int mItemOffsetX, mItemOffsetY; /**< Space between slot-border and item.    **/
    int *mGfxNr;                    /**< The gfxNr currently shown in the slot. **/
    unsigned int mSlotWidth, mSlotHeight;
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    void drawDragItem(int gfxNr);
};

#endif
