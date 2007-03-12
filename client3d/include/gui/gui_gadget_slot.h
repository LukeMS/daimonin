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
#include "item.h"
#include "gui_graphic.h"

/**
 ** This class provides an interactive button.
 *****************************************************************************/
class GuiGadgetSlot: public GuiGraphic
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    enum { UPDATE_ALL_SLOTS = -1 };
    enum { SLOT_IS_EMPTY    = -1 };
    typedef struct
    {
        int group;
        int index;
    }
    SlotID;
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    GuiGadgetSlot(TiXmlElement *xmlElement, void *parent, bool drawOnInit = true);
    ~GuiGadgetSlot();
    int mouseEvent(int MouseAction, int x, int y);
    void draw();
    void updateSlot(int slotNr, int state);
    void setItemReference(std::list<Item::sItem*> *IconContainer)
    {
        mlIconContainer = IconContainer;
    }
    int getDragSlot() const
    {
        return mDragSlot;
    }

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    static Ogre::Image mAtlasTexture;
    static Ogre::Overlay *mDnDOverlay;
    static Ogre::OverlayElement *mDnDElement;
    static Ogre::MaterialPtr mDnDMaterial;
    static Ogre::TexturePtr mDnDTexture;
    static std::vector<SlotID>mvSlotID;
    static std::list<Item::sItem*> *mlIconContainer;
    static int mDragSlot;                  /**< Slot where the drag was started. **/
    static int mActiveSlot;                /**< Slot the mouse is currently over. **/
    std::vector<Ogre::String> mvGfxPositions;
    int mSlotNr;                           /**< Unique number. **/
    int mItemInSlot;                       /**< The Item which is currently in the slot. **/
    bool mMouseOver, mMouseButDown;
    unsigned int mSlotWidth, mSlotHeight;
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    void drawDragItem();
    int getTextureAtlasPos(int itemFace);
};

#endif
