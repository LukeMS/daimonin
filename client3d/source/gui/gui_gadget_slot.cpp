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

#include "logger.h"
#include "gui_gadget_slot.h"
#include "gui_textout.h"
#include "gui_window.h"
#include "gui_manager.h"
#include "item.h"

using namespace Ogre;

//================================================================================================
// Constructor.
//================================================================================================
GuiGadgetSlot::GuiGadgetSlot(TiXmlElement *xmlElement, void *parent, bool drawOnInit):GuiElement(xmlElement, parent)
{
    mMouseOver = false;
    mMouseButDown = false;
    mActiveDrag = false;
    mActiveSlot = -1;

    const char *tmp;
    TiXmlElement *xmlOpt;
    if ((xmlOpt = xmlElement->FirstChildElement("Sum")))
    {
        if ((tmp = xmlOpt->Attribute("col" ))) mSumCol = atoi(tmp);
        if ((tmp = xmlOpt->Attribute("row" ))) mSumRow = atoi(tmp);
    }
    if ((xmlOpt = xmlElement->FirstChildElement("Offset")))
    {
        if ((tmp = xmlOpt->Attribute("col" ))) mColSpace = atoi(tmp);
        if ((tmp = xmlOpt->Attribute("row" ))) mRowSpace = atoi(tmp);
        if ((tmp = xmlOpt->Attribute("itemX" ))) mItemOffsetX = atoi(tmp);
        if ((tmp = xmlOpt->Attribute("itemY" ))) mItemOffsetY = atoi(tmp);
    }
    mSlotWidth = (mWidth + mColSpace) * mSumCol;
    mSlotHeight= (mHeight+ mRowSpace) * mSumRow;
    BG_Backup = new uint32[mWidth * mHeight];
    if (drawOnInit) draw();
}

//================================================================================================
// .
//================================================================================================
GuiGadgetSlot::~GuiGadgetSlot()
{
    //delete[] BG_Backup; // done in GuiElement.cpp
}

//================================================================================================
// Returns true if the mouse event was on this gadget (so no need to check the other gadgets).
//================================================================================================
bool GuiGadgetSlot::mouseEvent(int MouseAction, int x, int y)
{
    x-= mPosX;
    y-= mPosY;
    if ((unsigned int) x < mSlotWidth && (unsigned int) y < mSlotHeight)
    {
        int activeSlot = y/(mHeight+ mRowSpace)*mSumCol +   x/(mWidth + mColSpace);
        if (mActiveSlot != activeSlot && !mActiveDrag)
        {
            // We are no longer over this slot, so draw the defalut gfx.
            if (mActiveSlot >=0)
                drawSlot(mActiveSlot, GuiImageset::STATE_ELEMENT_DEFAULT);
            mActiveSlot = activeSlot;
            drawSlot(mActiveSlot, GuiImageset::STATE_ELEMENT_M_OVER);
        }
        if (MouseAction == GuiWindow::BUTTON_PRESSED && !mActiveDrag)
        {
            GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_CHATWINDOW, GuiImageset::GUI_LIST_MSGWIN,
                                                   StringConverter::toString(activeSlot).c_str());
            mActiveDrag = true;
        }
        if (MouseAction == GuiWindow::BUTTON_RELEASED && mActiveDrag)
        {
            mActiveDrag = false;
        }
        return true; // No need to check other gadgets.
    }
    else  // Mouse is no longer over the the gadget.
    {
        if (mActiveSlot >=0)
        {
            drawSlot(mActiveSlot, GuiImageset::STATE_ELEMENT_DEFAULT);
            mActiveSlot = -1;
            return true; // No need to check other gadgets.
        }
    }
    return false; // No action here, check the other gadgets.
}

//================================================================================================
// Draw a single slot.
//================================================================================================
void GuiGadgetSlot::drawSlot(int pos, int state, const char *strLabel)
{
    int row = pos / mSumCol;
    int col = pos - (row * mSumCol);
    int strtX = mPosX + col * (mColSpace + mWidth);
    int strtY = mPosY + row * (mRowSpace + mHeight);
    Texture *texture = ((GuiWindow*) mParent)->getTexture();
    // ////////////////////////////////////////////////////////////////////
    // Slot gfx.
    // ////////////////////////////////////////////////////////////////////
    PixelBox srcSlot = ((GuiWindow*) mParent)->getPixelBox()->getSubVolume(Box(
                           gfxSrcPos[state].x,
                           gfxSrcPos[state].y,
                           gfxSrcPos[state].x + mWidth,
                           gfxSrcPos[state].y + mHeight));
    uint32 *srcSlotData = static_cast<uint32*>(srcSlot.data);
    int rowSkipSlot = (int)((GuiWindow*) mParent)->getPixelBox()->getWidth();
    // ////////////////////////////////////////////////////////////////////
    // Item gfx.
    // ////////////////////////////////////////////////////////////////////
    GuiImageset::GuiSrcEntry *srcEntry;
    if (pos > 10)
        srcEntry = GuiImageset::getSingleton().getStateGfxPositions("Item_Axe");
    else
        srcEntry = GuiImageset::getSingleton().getStateGfxPositions("Item_Spear");

    PixelBox srcItem = ((GuiWindow*) mParent)->getPixelBox()->getSubVolume(Box(
                           srcEntry->state[0].x,
                           srcEntry->state[0].y,
                           srcEntry->state[0].x + srcEntry->width,
                           srcEntry->state[0].y + srcEntry->height));
    uint32 *srcItemData = static_cast<uint32*>(srcItem.data);
    int rowSkipItem = (int)((GuiWindow*) mParent)->getPixelBox()->getWidth();
    // ////////////////////////////////////////////////////////////////////
    // Draw into the buffer.
    // ////////////////////////////////////////////////////////////////////
    int dSlotY = 0, dItemY = 0, destY =0;
    for (int y =0; y < mHeight; ++y)
    {
        for (int x =0; x < mWidth; ++x)
        {
            // First check if item has a non transparent pixel to draw.
            if (x > mItemOffsetX && x < srcEntry->width + mItemOffsetX &&y > mItemOffsetY && y < srcEntry->height+ mItemOffsetY)
            {
                if (srcItemData[dItemY + x- mItemOffsetX] > 0x00ffffff)
                {
                    BG_Backup[destY + x] = srcItemData[dItemY + x- mItemOffsetX];
                    continue;
                }
            }
            // Now check for the background.
            if (srcSlotData[dItemY + x] > 0x00ffffff)
                BG_Backup[destY + x] = srcSlotData[dSlotY + x];
        }
        if (y > mItemOffsetY)
            dItemY+= (int)rowSkipItem;
        dSlotY+= (int)rowSkipSlot;
        destY+= mWidth;
    }
    srcSlot = PixelBox(mWidth, mHeight, 1, PF_A8B8G8R8, BG_Backup);
    // ////////////////////////////////////////////////////////////////////
    // Blit the buffer.
    // ////////////////////////////////////////////////////////////////////
    texture->getBuffer()->blitFromMemory(srcSlot, Box(strtX, strtY, strtX + mWidth, strtY + mHeight));
    /*
    {
        Image img;
        img.load("axe01.png","General");
        img.resize((Ogre::ushort)img.getWidth()/2, (Ogre::ushort)img.getHeight()/2, Image::FILTER_BICUBIC);
        img.save("c:\\axe01_bicubic.png");
    }


        // only for testing.
        GuiTextout::TextLine label;
        Item::sItem *item = Item::getSingleton().getBackpackItem(pos);
        if (!item) return;
        label.text = item->d_name;
        label.hideText= false;
        label.index= -1;
        label.font = 0;
        label.x1 = strtX + 5;
        label.y1 = strtY + 5;
        label.x2 = label.x1 + mWidth;
        label.y2 = label.y1 + GuiTextout::getSingleton().getFontHeight(label.font);
        label.color= 0x00ffffff;
        GuiTextout::getSingleton().Print(&label, texture);
    */
}

//================================================================================================
// Draw the guiElement.
//================================================================================================
void GuiGadgetSlot::draw()
{
    for (int pos = 0; pos < mSumRow * mSumCol; ++pos)
        drawSlot(pos, GuiImageset::STATE_ELEMENT_DEFAULT, StringConverter::toString(pos).c_str());
}
