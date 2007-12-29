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
    void setBusyTime(Ogre::Real time)
    {
        mBusyTime = time;
        mBusyTimeExpired = 0;
    }
    void setBusy()
    {
        mBusyTimeExpired = 0.001; // Something > 0 to start the busy-animation.
    }
    void update(Ogre::Real dTime);
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
    static int mDragSlot;                   /**< Slot where the drag was started. **/
    static int mActiveSlot;                 /**< Slot the mouse is currently over. **/
    int mSlotNr;                            /**< Unique number. **/
    Item::sItem *mItem;                     /**< The Item which is currently in the slot. **/
    unsigned int mSlotWidth, mSlotHeight;
    Ogre::Real mBusyTime;                   /**< Slot is busy for this amount of time. **/
    Ogre::Real mBusyTimeExpired;            /**< Already expired time. **/
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    void drawDragItem();
    /**
     ** Draws an inactive gfx over the slot for recognizing the busy status.
     *****************************************************************************/
    int getTextureAtlasPos(int itemFace);
};

#endif
