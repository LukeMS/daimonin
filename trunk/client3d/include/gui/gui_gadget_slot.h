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
#include "gui_cursor.h"

/**
 ** This class provides an slot that can hold an item.
    Drag'n'Drop is supported.
 *****************************************************************************/
class GuiGadgetSlot: public GuiGraphic
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    GuiGadgetSlot(TiXmlElement *xmlElement, void *parent, bool drawOnInit = true);
    ~GuiGadgetSlot();
    int mouseEvent(int MouseAction, int x, int y);
    void draw();
    /**
     ** Put an item into the slot.
     ** @param item 0 to empty the slot.
     *****************************************************************************/
    void setItem(Item::sItem *item)
    {
        mItem = item;
        draw();
    }
    /**
     ** Get the item within the slot.
     *****************************************************************************/
    Item::sItem *getItem()
    {
        return mItem;
    }
    /**
     ** Sets the time where the slot cannot be accessed.
     *****************************************************************************/
    void setBusy(Ogre::Real time)
    {
        mBusyTime = time;
        mBusyTimeExpired = 0;
    }
    /**
     ** Gets the remaing busy time of the slot.
     *****************************************************************************/
    Ogre::Real getBusy()
    {
        return mBusyTime;
    }
    void update(Ogre::Real dTime)
    {
        if (mBusyTime) drawBusy(dTime);
    }
    static void hideDragOverlay()
    {
        mDnDOverlay->hide();
    }
    static void moveDragOverlay()
    {
        Ogre::Real x, y;
        GuiCursor::getSingleton().getPos(x, y);
        mDnDElement->setPosition(x, y);
    }
    bool mouseWithin(int x, int y);

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    static Ogre::Image mAtlasTexture;
    static Ogre::Overlay *mDnDOverlay;
    static Ogre::OverlayElement *mDnDElement;
    static Ogre::MaterialPtr mDnDMaterial;
    static Ogre::TexturePtr mDnDTexture;
    static std::vector<Ogre::String> mvAtlasGfxName;
    static int mDragSlot;                  /**< Slot where the drag was started. **/
    static int mActiveSlot;                /**< Slot the mouse is currently over. **/
    int mSlotNr;                           /**< Unique number. **/
    Item::sItem *mItem;                    /**< The Item which is currently in the slot. **/
    unsigned int mSlotWidth, mSlotHeight;
    Ogre::Real mBusyTime, mBusyTimeExpired;
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    void drawDragItem();
    /**
     ** Draws an inactive gfx over the slot for recognizing the busy status.
     *****************************************************************************/
    void drawBusy(Ogre::Real dTime);
    int getTextureAtlasPos(int itemFace);
};

#endif
