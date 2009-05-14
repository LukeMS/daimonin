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

#ifndef GUI_ELEMENT_SLOT_H
#define GUI_ELEMENT_SLOT_H

#include "gui_element.h"

/**
 ** This class provides a slot that can hold an item.
 ** Drag'n'Drop is supported.
 *****************************************************************************/
class GuiElementSlot: public GuiElement
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    GuiElementSlot(TiXmlElement *xmlElement, const void *parent, bool drawOnInit);
    ~GuiElementSlot() {}
    virtual void sendMsg(const int message, Ogre::String &text, Ogre::uint32 &param, const char *text2);
    virtual int mouseEvent(const int mouseAction, int mouseX, int mouseY, int mouseWheel);
    void draw();
    void setItem(const char *gxName, int quantity, const char *itemName);
    /**
     ** Sets the time while the slot cannot be accessed.
     ** The current busy animation will be stopped.
     *****************************************************************************/
    void setBusyTime(Ogre::Real time)
    {
        mBusyTime = time;
        setBusy(false);
    }
    void setBusy(bool busy)
    {
        mBusyTimeExpired = busy?0.001:0.000;
    }
    bool empty()
    {
        return (mItemGfxID < 0);
    }
    virtual void update(Ogre::Real dTime);

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    static int mDragSlot;         /**< Slot where the drag was started. **/
    static int mActiveSlot;       /**< Slot the mouse is currently over. **/
    static int uid;               /**< Unique number generator. **/
    int mSlotNr;                  /**< Unique number. **/
    int mSlotGfxBG;               /**< The gfx number of the background gfx (will only be shown if slot is empty **/
    int mItemGfxID;               /**< The item which is currently in the slot. **/
    Ogre::Real mBusyTime;         /**< Slot is busy for this amount of time. **/
    Ogre::Real mBusyTimeExpired;  /**< Already expired time. **/
    Ogre::Real mBusyOldVal;       /**< Indicates if the busy gfx needs a redraw. **/
    Ogre::String mStrQuantity;    /**< Quantity of items in the slot. **/
    Ogre::String mStrTooltip;
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    int getTextureAtlasPos(const char *gfxName);
    void drawBusy(int busyTime);
};

/**
 ** This class provides a group of slots.
 *****************************************************************************/
class GuiElementSlotGroup: public GuiElement
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    GuiElementSlotGroup(TiXmlElement *xmlElement, const void *parent);
    ~GuiElementSlotGroup();
    virtual void sendMsg(const int message, Ogre::String &text, Ogre::uint32 &param, const char *text2);
    virtual int mouseEvent(const int mouseAction, int mouseX, int mouseY, int mouseWheel);
    void draw();

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    static int uid;                  /**< Unique number generator. **/
    unsigned short mGroupNr;         /**< Unique number. **/
    unsigned short mSpaceX, mSpaceY; /**< Free space between the slots. **/
    std::vector<class GuiElementSlot*>mvSlot;
};

#endif
