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
#include "gui_textout.h"
#include "gui_graphic.h"
#include "gui_element_slot.h"

using namespace Ogre;

const uint32 SLOT_BUSY_COLOR     = 0xdd777777;
const uint32 SLOT_QUANTITY_COLOR = 0x00888888;
int GuiElementSlot::mDragSlot =  -1;
int GuiElementSlot::mActiveSlot= -1;
int GuiElementSlot::uid = -1;

//================================================================================================
// Constructor.
//================================================================================================
GuiElementSlot::GuiElementSlot(TiXmlElement *xmlElement, const void *parent, bool drawOnInit):GuiElement(xmlElement, parent)
{
    mSlotNr = ++uid;
    mItemGfxID = -1;
    mBusyTime = 1.0;  // Default time for a slot to be busy (MUST be > 0).
    mBusyOldVal = -1;
    mBusyTimeExpired = 0;
    // Look for a background graphic (its a png from the item folder).
    // For example: A shield for the left hand slot.
    if ((xmlElement = xmlElement->FirstChildElement("Image")))
    {
        const char *tmp;
        if ((tmp = xmlElement->Attribute("bg_item_image_filename")))
            mSlotGfxBG = GuiImageset::getSingleton().getItemId(tmp);
        else
            mSlotGfxBG = -1;
    }
    if (GuiImageset::ITEM_SIZE > mWidth || GuiImageset::ITEM_SIZE > mHeight)
        Logger::log().warning() << "GuiElementSlot: Item-gfx is bigger than the slot-gfx.";
    if (drawOnInit) draw();
}

//================================================================================================
//
//================================================================================================
void GuiElementSlot::sendMsg(const int message, Ogre::String &text, Ogre::uint32 &param, const char *text2)
{
    switch (message)
    {
        case GuiManager::MSG_ADD_ITEM:
            setItem(text.c_str(), param, text2);
            return;
        case GuiManager::MSG_DEL_ITEM:
            setItem(0, -1, 0);
            return;
    }
}

//================================================================================================
//
//================================================================================================
void GuiElementSlot::setItem(const char *gfxName, int quantity, const char *itemName)
{
    //GuiManager::getSingleton().print(GuiManager::LIST_MSGWIN, gfxName);
    if (quantity >= 0)
    {
        mStrTooltip = itemName;
        mItemGfxID = GuiImageset::getSingleton().getItemId(gfxName);
        if (quantity > 1)
            mStrQuantity = (quantity <= 999)?StringConverter::toString(quantity):"###";
        else
            mStrQuantity = "";
    }
    else // empty slot.
    {
        mStrTooltip = "";
        mItemGfxID = -1;
    }
    draw();
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
int GuiElementSlot::mouseEvent(const int mouseAction, int mouseX, int mouseY, int mouseWheel)
{
    if (mouseWithin(mouseX, mouseY))
    {
        if (mActiveSlot != mSlotNr)
        {
            mActiveSlot = mSlotNr;
            if (setState(GuiImageset::STATE_ELEMENT_M_OVER)) draw();
            GuiManager::getSingleton().setTooltip(mStrTooltip.c_str());
            return GuiManager::EVENT_CHECK_NEXT;
        }
        if (mouseAction == GuiManager::BUTTON_PRESSED && mItemGfxID >= 0)
        {
            mDragSlot = mActiveSlot;
            GuiManager::getSingleton().setTooltip("");
            GuiManager::getSingleton().drawDragElement(GuiImageset::getSingleton().getItemPB(mItemGfxID));
            return GuiManager::EVENT_DRAG_STRT;
        }
        return GuiManager::EVENT_CHECK_DONE; // No need to check other gadgets.
    }
    else // Mouse is no longer over this slot.
    {
        if (setState(GuiImageset::STATE_ELEMENT_DEFAULT))
        {
            draw();
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
        GuiElement::draw(true);
        return;
    }
    // Draw the empty slot-gfx to the build-buffer.
    uint32 *dst = GuiManager::getSingleton().getBuildBuffer();
    uint32 *bak = mParent->getLayerBG() + mPosX + mPosY*mParent->getWidth();
    if (mGfxSrc)
    {
        PixelBox src = GuiImageset::getSingleton().getPixelBox().getSubVolume(Box(mGfxSrc->state[mState].x, mGfxSrc->state[mState].y,
                       mGfxSrc->state[mState].x + mGfxSrc->w, mGfxSrc->state[mState].y + mGfxSrc->h));
        int srcRowSkip = (int)GuiImageset::getSingleton().getPixelBox().getWidth();
        GuiGraphic::getSingleton().drawGfxToBuffer(mWidth, mHeight, mGfxSrc->w, mGfxSrc->h, (uint32*)src.data, bak, dst, srcRowSkip, mParent->getWidth(), mWidth);
    }
    else
        GuiGraphic::getSingleton().drawColorToBuffer(mWidth, mHeight, mFillColor, bak, dst, mParent->getWidth(), mWidth);
    // Draw the item-gfx to the build-buffer.
    int dX  = (mWidth  - GuiImageset::ITEM_SIZE) /2;
    int dY  = (mHeight - GuiImageset::ITEM_SIZE) /2;
    PixelBox src = GuiImageset::getSingleton().getItemPB(mItemGfxID);
    uint32 *buf = dst + dX + dY * mWidth;
    GuiGraphic::getSingleton().drawGfxToBuffer(GuiImageset::ITEM_SIZE, GuiImageset::ITEM_SIZE, GuiImageset::ITEM_SIZE, GuiImageset::ITEM_SIZE, (uint32*)src.data, buf, buf, GuiImageset::ITEM_SIZE, mWidth, mWidth);
    // Print the number of items.

    if (!mStrQuantity.empty())
        GuiTextout::getSingleton().printText(mWidth-mLabelPosX, mHeight-mLabelPosY,
                                             buf + mLabelPosX + mLabelPosY*mWidth, mWidth,
                                             mStrQuantity.c_str(), mLabelFontNr, 0x00ffffff, false, 0x00555555);
    // Draw the busy-gfx to the build-buffer.
    if (mBusyTimeExpired) drawBusy((int)mBusyOldVal);
    // Copy the build-buffer to the window texture.
    mParent->getTexture()->getBuffer()->blitFromMemory(PixelBox(mWidth, mHeight, 1, PF_A8R8G8B8, dst), Box(mPosX, mPosY, mPosX+mWidth, mPosY+mHeight));
}

//================================================================================================
// Draws the busy gfx into the build buffer.
//================================================================================================
void GuiElementSlot::drawBusy(int angle)
{
    int x2,x3,y2,dY,dX,xStep,yStep,delta,posY;
    uint32 *dst = GuiManager::getSingleton().getBuildBuffer();
    int x = mWidth/2;
    int y = mHeight/2;
    if (angle == 180)
        GuiGraphic::getSingleton().drawColorToBuffer(mWidth/2, mHeight, SLOT_BUSY_COLOR, dst, mWidth);
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
                posY = y*mWidth;
                if (x < x3)
                    GuiGraphic::getSingleton().drawColorToBuffer(x3-x, 1, SLOT_BUSY_COLOR, dst + posY+x, mWidth);
                else
                    GuiGraphic::getSingleton().drawColorToBuffer(x-x3, 1, SLOT_BUSY_COLOR, dst + posY+x3, mWidth);
            }
        }
        if (angle < 270)
            GuiGraphic::getSingleton().drawColorToBuffer(mWidth/2, mHeight/2+1, SLOT_BUSY_COLOR, dst, mWidth);
        else if (angle < 315)
            GuiGraphic::getSingleton().drawColorToBuffer(mWidth/2, y2+1, SLOT_BUSY_COLOR, dst, mWidth);
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
                GuiGraphic::getSingleton().drawColorToBuffer(mWidth/2, mHeight-y2, SLOT_BUSY_COLOR, dst + mWidth/2 + y2*mWidth, mWidth);
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
                posY = y*mWidth;
                if (x < x3)
                    GuiGraphic::getSingleton().drawColorToBuffer(x3-x, 1, SLOT_BUSY_COLOR, dst + posY+x, mWidth);
                else
                    GuiGraphic::getSingleton().drawColorToBuffer(x-x3, 1, SLOT_BUSY_COLOR, dst + posY+x3, mWidth);
            }
        }
        // Fill the lower right side.
        if (angle <= 90)
            GuiGraphic::getSingleton().drawColorToBuffer(mWidth/2, mHeight/2, SLOT_BUSY_COLOR, dst + mWidth/2 + mHeight/2*mWidth, mWidth);
        // Fill the complete left side.
        GuiGraphic::getSingleton().drawColorToBuffer(mWidth/2, mHeight, SLOT_BUSY_COLOR, dst, mWidth);
    }
}






int GuiElementSlotGroup::uid = -1;

//================================================================================================
// Destructor.
//================================================================================================
GuiElementSlotGroup::~GuiElementSlotGroup()
{
    for (std::vector<GuiElementSlot*>::iterator i = mvSlot.begin(); i < mvSlot.end(); ++i)
        delete (*i);
    mvSlot.clear();
}

//================================================================================================
// Constructor.
//================================================================================================
GuiElementSlotGroup::GuiElementSlotGroup(TiXmlElement *xmlRoot, const void *parent):GuiElement(xmlRoot, parent)
{
    mGroupNr = ++uid;
    const char *tmp = xmlRoot->Attribute("slots");
    int sumSlots = tmp?atoi(tmp):0;
    if (sumSlots <= 0)
    {
        Logger::log().error() << "Wrong settings in slotgroup '" << xmlRoot->Attribute("name") << "'. Number of slots not defined.";
        return;
    }
    TiXmlElement *xmlSlot = xmlRoot->FirstChildElement("Slot");
    TiXmlElement *xmlElem = xmlSlot->FirstChildElement("Space");
    mSpaceX = mSpaceY = 0;
    if (xmlElem)
    {
        if ((tmp = xmlElem->Attribute("x"))) mSpaceX = (unsigned short)atoi(tmp);
        if ((tmp = xmlElem->Attribute("y"))) mSpaceY = (unsigned short)atoi(tmp);
    }
    int x = mWidth;
    int y = mHeight;
    for (int i = 0; i < sumSlots; ++i)
    {
        GuiElementSlot *slot = new GuiElementSlot(xmlSlot, mParent, false);
        mvSlot.push_back(slot);
        int size = slot->getWidth();
        x-= size+mSpaceX;
        if (x < 0)
        {
            x = mWidth-size-mSpaceX;
            y-= size+mSpaceY;
            if (x < 0 || y < 0)
            {
                Logger::log().error() << "Wrong settings in slotgroup '" << xmlRoot->Attribute("name")
                << "'. Number of slots not doesn't fit into the slotgroup.";
                break;
            }
        }
        mvSlot[i]->setPosition(mPosX+x, mPosY+y);
    }
    draw();
}

//================================================================================================
//
//================================================================================================
void GuiElementSlotGroup::draw()
{
    for (unsigned int i = 0; i < mvSlot.size(); ++i)
        mvSlot[i]->draw();
}

//================================================================================================
//
//================================================================================================
int GuiElementSlotGroup::mouseEvent(const int mouseAction, int mouseX, int mouseY, int mouseWheel)
{
    for (unsigned int i = 0; i < mvSlot.size(); ++i)
    {
        int ret = mvSlot[i]->mouseEvent(mouseAction, mouseX, mouseY, mouseWheel);
        if (ret != GuiManager::EVENT_CHECK_NEXT)  return ret;
    }
    return GuiManager::EVENT_CHECK_NEXT;
}

//================================================================================================
//
//================================================================================================
void GuiElementSlotGroup::sendMsg(const int message, Ogre::String &text, Ogre::uint32 &param, const char *text2)
{
    switch (message)
    {
        case GuiManager::MSG_ADD_ITEM:
            for (unsigned int i = 0; i < mvSlot.size(); ++i)
            {
                if (mvSlot[i]->empty())
                {
                    mvSlot[i]->setItem(text.c_str(), (int)param, text2);
                    break;
                }
            }
            return;
        case GuiManager::MSG_DEL_ITEM:
            //mvSlot[i]->setItem(0, -1);
            //GuiManager::getSingleton().setTooltip("");
            return;
    }
}
