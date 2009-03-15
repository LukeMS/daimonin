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

#include "logger.h"
#include "gui_element_slot.h"
#include "gui_textout.h"
#include "gui_window.h"
#include "gui_graphic.h"
#include "gui_manager.h"
#include "item.h"
#include "resourceloader.h"
#include "tile_map_wrapper.h"

using namespace Ogre;

// TODO: Use the tooltip overlay/texture for the dnd overlay/texturet.
const uint32 SLOT_BUSY_COLOR     = 0xdd777777;
const uint32 SLOT_QUANTITY_COLOR = 0x00888888;
int GuiElementSlot::mDragSlot =  -1;
int GuiElementSlot::mActiveSlot= -1;
int uid = -1;

//================================================================================================
// Constructor.
//================================================================================================
GuiElementSlot::GuiElementSlot(TiXmlElement *xmlElement, void *parent):GuiElement(xmlElement, parent)
{
    std::string filename;
    mSlotNr = ++uid;
    mItemSize = GuiImageset::ITEM_SIZE;
    mItemGfxID = -1;
    mBusyTime = 1.0;  // Default time for a slot to be busy (MUST be > 0).
    mBusyOldVal = -1;
    mBusyTimeExpired = 0;
    // ////////////////////////////////////////////////////////////////////
    // Look for a background graphic (its a png from the item folder).
    // ////////////////////////////////////////////////////////////////////
    if ((xmlElement = xmlElement->FirstChildElement("Image")))
    {
        const char *tmp;
        if ((tmp = xmlElement->Attribute("bg_item_image_filename")))
            mSlotGfxBG = GuiImageset::getSingleton().getItemId(tmp);
        else
            mSlotGfxBG = -1;
    }
    // ////////////////////////////////////////////////////////////////////
    // Draw the container.
    // ////////////////////////////////////////////////////////////////////
    if (mWidth > MAX_SIZE) mWidth = MAX_SIZE-1;
    if (mHeight> MAX_SIZE) mHeight= MAX_SIZE-1;
    draw();
}

//================================================================================================
//
//================================================================================================
int GuiElementSlot::sendMsg(int message, const char *text, uint32 param)
{
    switch (message)
    {
        case GuiManager::MSG_ADD_ITEM:
            setItem(text, param);
            return 0;
    }
    return -1;
}

//================================================================================================
//
//================================================================================================
void GuiElementSlot::setItem(const char *gfxName, int quantity)
{
    static bool once = false;
    if (once) return;

    String str = "-- add Item";
    str+= gfxName;
    GuiManager::getSingleton().sendMsg(GuiManager::GUI_LIST_CHATWIN, GuiManager::MSG_ADD_ROW, str.c_str());

    mItemGfxID = GuiImageset::getSingleton().getItemId(gfxName);
    mQuantity = quantity;
    draw();
    once = true;
}

//================================================================================================
// Draw a busy gfx over the slot.
//================================================================================================
void GuiElementSlot::update(Real dTime)
{
    if (!mBusyTimeExpired) return;
    mBusyTimeExpired += dTime;
    int newVal = (int)((mBusyTimeExpired / mBusyTime)*360) ;
    if (newVal > 360) mBusyTimeExpired = 0;
    if (mBusyOldVal != newVal)
    {
        mBusyOldVal = newVal;
        draw();
    }
}

//================================================================================================
// .
//================================================================================================
int GuiElementSlot::mouseEvent(int MouseAction, int x, int y, int z)
{
    if (mouseWithin(x, y))
    {
        if (mActiveSlot != mSlotNr)
        {
            mActiveSlot = mSlotNr;
            if (setState(GuiImageset::STATE_ELEMENT_M_OVER)) draw();
            //GuiManager::getSingleton().setTooltip(mStrTooltip.c_str());
            return GuiManager::EVENT_CHECK_NEXT;
        }
        if (MouseAction == GuiManager::BUTTON_PRESSED && mItemGfxID >= 0)
        {
            mDragSlot = mActiveSlot;
            GuiManager::getSingleton().drawDragElement(GuiImageset::getSingleton().getItemPB(mItemGfxID));
            return GuiManager::EVENT_DRAG_STRT;
        }
        return GuiManager::EVENT_CHECK_DONE; // No need to check other gadgets.
    }
    else // Mouse is no longer over this slot.
    {
        if (getState() != GuiImageset::STATE_ELEMENT_DEFAULT)
        {
            if (setState(GuiImageset::STATE_ELEMENT_DEFAULT)) draw();
            mActiveSlot = -1;
            GuiManager::getSingleton().setTooltip("");
            return GuiManager::EVENT_CHECK_DONE;
        }
    }
    return GuiManager::EVENT_CHECK_NEXT;
}

//================================================================================================
// .
//================================================================================================
void GuiElementSlot::draw()
{
    if (!mVisible || mItemGfxID < 0)
    {
        GuiElement::draw();
        return;
    }
    // ////////////////////////////////////////////////////////////////////
    // Draw the empty slot-gfx to the build-buffer.
    // ////////////////////////////////////////////////////////////////////
    uint32 *dst = GuiManager::getSingleton().getBuildBuffer();
    uint32 *bak = mParent->getLayerBG() + mPosX + mPosY*mParent->getWidth();
    if (mGfxSrc)
    {
        PixelBox src = mParent->getPixelBox()->getSubVolume(Box(mGfxSrc->state[mState].x, mGfxSrc->state[mState].y,
                       mGfxSrc->state[mState].x + mGfxSrc->w, mGfxSrc->state[mState].y + mGfxSrc->h));
        int srcRowSkip = (int)mParent->getPixelBox()->getWidth();
        GuiGraphic::getSingleton().drawGfxToBuffer(mWidth, mHeight, mGfxSrc->w, mGfxSrc->h, (uint32*)src.data, bak, dst, srcRowSkip, mParent->getWidth(), mWidth);
    }
    else
        GuiGraphic::getSingleton().drawColorToBuffer(mWidth, mHeight, mFillColor, bak, dst, mParent->getWidth(), mWidth);
    // ////////////////////////////////////////////////////////////////////
    // Draw the item-gfx to the build-buffer.
    // ////////////////////////////////////////////////////////////////////
    int dX  = (mWidth  - mItemSize) /2;
    int dY  = (mHeight - mItemSize) /2;
    PixelBox src = GuiImageset::getSingleton().getItemPB(mItemGfxID);
    uint32 *buf = dst + dX + dY * mWidth;
    GuiGraphic::getSingleton().drawGfxToBuffer(mItemSize, mItemSize, mItemSize, mItemSize, (uint32*)src.data, buf, buf, mItemSize, mWidth, mWidth);
    // ////////////////////////////////////////////////////////////////////
    // Draw the busy-gfx to the build-buffer.
    // ////////////////////////////////////////////////////////////////////
    if (mBusyTimeExpired) drawBusy((int)mBusyOldVal);
    // ////////////////////////////////////////////////////////////////////
    // Copy the build-buffer to the window texture.
    // ////////////////////////////////////////////////////////////////////
    mParent->getTexture()->getBuffer()->blitFromMemory(PixelBox(mWidth, mHeight, 1, PF_A8R8G8B8, dst), Box(mPosX, mPosY, mPosX+mWidth, mPosY+mHeight));
}

//================================================================================================
// Draws the busy gfx into the build buffer.
//================================================================================================
void GuiElementSlot::drawBusy(int angle)
{
    int x2,x3,y2,dY,dX,xStep,yStep,delta,posY;
    uint32 *dst = GuiManager::getSingleton().getBuildBuffer();;
    int x = mWidth/2;
    int y = mHeight/2;
    if (angle == 180)
        GuiGraphic::getSingleton().drawColorToBuffer(mWidth/2, mHeight, SLOT_BUSY_COLOR, dst, MAX_SIZE);
    else if (angle > 180)
    {
        if (angle < 225)
        {
            Real step = mWidth/45.0f;
            x2 = (int)(mWidth - (angle-180)*step)/2;
            y2 = mHeight;
        }
        else if (angle <= 315)
        {
            x2 = 0;
            Real step = mHeight/90.0f;
            y2 = (int)(mHeight - (angle-225)*step);
        }
        else
        {
            Real step = mWidth/45.0f;
            x2= (int)((angle-315)*step)/2;
            y2 = -1;
        }
        dX = Math::IAbs(x2-x);
        dY = Math::IAbs(y2-y);
        delta= dX - dY;
        xStep= (x2>x)?1:-1;
        yStep= (y2>y)?1:-1;
        x3= (y2<mHeight/2)?mWidth/2:0;
        while (x!=x2)
        {
            if (delta >= 0)
            {
                x+= xStep;
                delta-= dY;
            }
            else
            {
                y+= yStep;
                delta+= dX;
                posY = y*MAX_SIZE;
                if (x < x3)
                    GuiGraphic::getSingleton().drawColorToBuffer(x3-x, 1, SLOT_BUSY_COLOR, dst + posY+x, MAX_SIZE);
                else
                    GuiGraphic::getSingleton().drawColorToBuffer(x-x3, 1, SLOT_BUSY_COLOR, dst + posY+x3, MAX_SIZE);
            }
        }
        if (angle < 270)
            GuiGraphic::getSingleton().drawColorToBuffer(mWidth/2, mHeight/2+1, SLOT_BUSY_COLOR, dst, MAX_SIZE);
        else if (angle < 315)
            GuiGraphic::getSingleton().drawColorToBuffer(mWidth/2, y2+1, SLOT_BUSY_COLOR, dst, MAX_SIZE);
    }
    else // 0...180°
    {
        if (angle <= 45)
        {
            Real step = mWidth/45.0f;
            x2= (int)(angle*step + mWidth)/2;
            y2= -1;
        }
        else if (angle <= 135)
        {
            Real step = mHeight/90.0f;
            x2 = mWidth;
            y2 = (int)((angle-45)*step);
            if (angle > 90)
                GuiGraphic::getSingleton().drawColorToBuffer(mWidth/2, mHeight-y2, SLOT_BUSY_COLOR, dst + mWidth/2 + y2*MAX_SIZE, MAX_SIZE);
        }
        else
        {
            Real step = mWidth/45.0f;
            x2 = mWidth-(int)((angle-135)*step)/2;
            y2 = mHeight;
        }
        dX = Math::IAbs(x2-x);
        dY = Math::IAbs(y2-y);
        delta = dX - dY;
        yStep = (y2 >y)?1:-1;
        x3=(y2 >y)?mWidth/2:mWidth;
        while (x!=x2)
        {
            if (delta >= 0)
            {
                ++x;
                delta-= dY;
            }
            else
            {
                y+= yStep;
                delta+= dX;
                posY = y*MAX_SIZE;
                if (x < x3)
                    GuiGraphic::getSingleton().drawColorToBuffer(x3-x, 1, SLOT_BUSY_COLOR, dst + posY+x, MAX_SIZE);
                else
                    GuiGraphic::getSingleton().drawColorToBuffer(x-x3, 1, SLOT_BUSY_COLOR, dst + posY+x3, MAX_SIZE);
            }
        }
        // Fill the lower right side.
        if (angle <= 90)
            GuiGraphic::getSingleton().drawColorToBuffer(mWidth/2, mHeight/2, SLOT_BUSY_COLOR, dst + mWidth/2 + mHeight/2*MAX_SIZE, MAX_SIZE);
        // Fill the complete left side.
        GuiGraphic::getSingleton().drawColorToBuffer(mWidth/2, mHeight, SLOT_BUSY_COLOR, dst, MAX_SIZE);
    }
}
