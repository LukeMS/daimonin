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

#include <tinyxml.h>
#include "define.h"
#include "logger.h"
#include "gui_window.h"
#include "gui_manager.h"
#include "gui_element_scrollbar.h"

using namespace Ogre;

const int MIN_SLIDER_SIZE = 15;
const int SLIDER_INNER_OFFSET = 3;

// TODO:
// Disable buttons/slider if they doesnt fit into the window.
// Keep slider-pos (or jump to last line?) when lines were added.
// support horizontal scrollbar.

//================================================================================================
// Constructor.
//================================================================================================
GuiElementScrollbar::GuiElementScrollbar(TiXmlElement *xmlElement, void *parent, void *parentElement):GuiElement(xmlElement, parent)
{
    mButScrollUp  = 0;
    mButScrollDown= 0;
    mHorizontal = (mWidth > mHeight);
    uint32 *color = 0;
    const char *tmp;
    TiXmlElement *xmlOpt;
    for (xmlOpt = xmlElement->FirstChildElement("Color"); xmlOpt; xmlOpt = xmlOpt->NextSiblingElement("Color"))
    {
        if      (!strcmp(xmlOpt->Attribute("type"), "BACKGROUND"))  color = &mColorBackground;
        else if (!strcmp(xmlOpt->Attribute("type"), "BORDERLINE"))  color = &mColorBorderline;
        else if (!strcmp(xmlOpt->Attribute("type"), "BAR_PASSIVE")) color = &mColorBarPassive;
        else if (!strcmp(xmlOpt->Attribute("type"), "BAR_M_OVER"))  color = &mColorBarM_Over;
        else if (!strcmp(xmlOpt->Attribute("type"), "BAR_ACTIVE"))  color = &mColorBarActive;
        if ((tmp = xmlOpt->Attribute("red"  ))) *color = atoi(tmp);
        if ((tmp = xmlOpt->Attribute("green"))) *color+= atoi(tmp) <<  8;
        if ((tmp = xmlOpt->Attribute("blue" ))) *color+= atoi(tmp) << 16;
        if ((tmp = xmlOpt->Attribute("alpha"))) *color+= atoi(tmp) << 24;
    }
    for (xmlOpt = xmlElement->FirstChildElement("Button"); xmlOpt; xmlOpt = xmlOpt->NextSiblingElement("Button"))
    {
        if (!strcmp(xmlOpt->Attribute("name"), "But_ScrollUp"))
            mButScrollUp = new GuiElementButton(xmlOpt, parent, false);
        else if (!strcmp(xmlOpt->Attribute("name"), "But_ScrollDown"))
            mButScrollDown = new GuiElementButton(xmlOpt, parent, false);
    }
    mDragging = false;
    mMouseOver = false;
    mMouseButDown = false;
    mGfxBuffer = 0; // Buffer to hold the pixel information.
    mSliderPos = 0;
    resize(mWidth, mHeight);
}

//================================================================================================
// Destructor.
//================================================================================================
GuiElementScrollbar::~GuiElementScrollbar()
{
    delete[] mGfxBuffer;
    delete mButScrollUp;
    delete mButScrollDown;
}

//================================================================================================
//
//================================================================================================
int GuiElementScrollbar::getScrollOffset()
{
    int scroll = mLastScrollAmount;
    mLastScrollAmount = 0;
    return scroll;
}

//================================================================================================
//
//================================================================================================
bool GuiElementScrollbar::mouseOverSlider(int x, int y)
{
    if (mHorizontal)
        return false;
    else
        return (x > mStartX && x < mStopX && y > mStartY + mSliderPos && y < mStartY + mSliderPos + mSliderSize );
}

//================================================================================================
// Mouse action in parent window.
// Slider must stick to the mousecursor while scrolling.
//================================================================================================
int GuiElementScrollbar::mouseEvent(int MouseAction, int x, int y, int mouseWheel)
{
    // Test the buttons and the mouswheel.
    if (!mDragging)
    {
        if (mouseWheel)
        {
            mLastScrollAmount = mouseWheel>0?-1:+1;
            return GuiManager::EVENT_USER_ACTION;
        }
        if (mButScrollUp && mButScrollUp->mouseEvent(MouseAction, x, y, mouseWheel) == GuiManager::EVENT_USER_ACTION)
        {
            mLastScrollAmount = -1;
            return GuiManager::EVENT_USER_ACTION;
        }
        if (mButScrollDown && mButScrollDown->mouseEvent(MouseAction, x, y, mouseWheel) == GuiManager::EVENT_USER_ACTION)
        {
            mLastScrollAmount = +1;
            return GuiManager::EVENT_USER_ACTION;
        }
    }
    // Test the slider.
    if (mDragging || mouseOverSlider(x,y))
    {
        static float dragDelta = 0;
        static int dragSliderPos = 0;
        if (!mMouseOver)
        {
            mMouseOver = true;
            if (setState(GuiImageset::STATE_ELEMENT_M_OVER)) draw();
            //GuiManager::getSingleton().setTooltip(mStrTooltip.c_str());
            return GuiManager::EVENT_CHECK_DONE;
        }
        if (MouseAction == GuiManager::BUTTON_PRESSED && !mMouseButDown)
        {
            mDragging = true;
            mMouseButDown = true;
            dragSliderPos = y;
            dragDelta = 0;
            if (setState(GuiImageset::STATE_ELEMENT_PUSHED)) draw();
            return GuiManager::EVENT_CHECK_DONE;
        }
        if (MouseAction == GuiManager::BUTTON_RELEASED && mMouseButDown)
        {
            mDragging = false;
            mMouseButDown = false;
            if (mouseOverSlider(x,y))
            {
                if (setState(GuiImageset::STATE_ELEMENT_M_OVER)) draw();
                return GuiManager::EVENT_USER_ACTION;
            }
            else
            {
                if (setState(GuiImageset::STATE_ELEMENT_DEFAULT)) draw();
                mMouseOver = false;
            }
            return GuiManager::EVENT_CHECK_DONE;
        }
        if (MouseAction == GuiManager::MOUSE_MOVEMENT && mDragging)
        {
            if (y > mStartY && y < mStartY+mMaxSliderSize)
            {
                mLastScrollAmount = (int) ((y-dragSliderPos) / mPixelScrollToLineScroll + dragDelta);
                dragDelta = ((y-dragSliderPos) / mPixelScrollToLineScroll + dragDelta) - mLastScrollAmount;
                dragSliderPos = y;
                return GuiManager::EVENT_USER_ACTION;
            }
        }
        return GuiManager::EVENT_CHECK_DONE; // No need to check other gadgets.
    }
    // Mouse is no longer over the the gadget.
    if (getState() != GuiImageset::STATE_ELEMENT_DEFAULT)
    {
        mMouseOver = false;
        mMouseButDown = false;
        if (setState(GuiImageset::STATE_ELEMENT_DEFAULT)) draw();
        GuiManager::getSingleton().setTooltip("");
        return GuiManager::EVENT_CHECK_NEXT;
    }
    // If dragging is active, the parent window stays active if the mouse moves outside the window.
    if (mDragging) return GuiManager::EVENT_CHECK_DONE;
    return GuiManager::EVENT_CHECK_NEXT; // No action here, check the other gadgets.
}

//================================================================================================
// Update the slider size. Will be called from the parent element.
//================================================================================================
void GuiElementScrollbar::updateSliderSize(int sizeStrBuffer, int scrollOffset, int maxVisiblePos, int actPos)
{
    if (actPos <= maxVisiblePos)
    {
        mSliderPos   = 0;
        mSliderSize  = mMaxSliderSize;
    }
    else
    {
        mSliderSize = mMaxSliderSize - (mMaxSliderSize * actPos) / sizeStrBuffer;
        if (mSliderSize < MIN_SLIDER_SIZE) mSliderSize = MIN_SLIDER_SIZE;
        int maxSliderPos = mMaxSliderSize - mSliderSize;
        mSliderPos = maxSliderPos-(maxSliderPos * scrollOffset) / (actPos-maxVisiblePos);
        mPixelScrollToLineScroll = maxSliderPos /(float)(actPos-maxVisiblePos);
    }
    draw();
}

//================================================================================================
// Draw the slider.
//================================================================================================
void GuiElementScrollbar::draw()
{
    int x1, x2, y1, y2, w;
    w = mStopX-mStartX;
    uint32 color;
    switch (mState)
    {
        case GuiImageset::STATE_ELEMENT_PUSHED:
            color = mColorBarActive;
            break;
        case GuiImageset::STATE_ELEMENT_PASSIVE:
            color = mColorBarPassive;
            break;
        case GuiImageset::STATE_ELEMENT_M_OVER:
            color = mColorBarM_Over;
            break;
        default:
            color = mColorBorderline;
            break;
    }
    if (mHorizontal)
    {
        x1 = mSliderPos + SLIDER_INNER_OFFSET;
        x2 = x1 + mSliderSize - SLIDER_INNER_OFFSET;
        y1 = SLIDER_INNER_OFFSET + 1;
        y2 = mStopY-mStartY - SLIDER_INNER_OFFSET-1;
        for (int y = y1; y < y2; ++y)
        {
            for (int x = SLIDER_INNER_OFFSET; x < mMaxSliderSize; ++x)
            {
                if (x > x1 && x < x2)
                    mGfxBuffer[y * w + x] = color;
                else
                    mGfxBuffer[y * w + x] = mColorBackground;
            }
        }
    }
    else
    {
        x1 = SLIDER_INNER_OFFSET +1;
        x2 = w - SLIDER_INNER_OFFSET -1;
        y1 = mSliderPos + SLIDER_INNER_OFFSET;
        y2 = y1 + mSliderSize - SLIDER_INNER_OFFSET;
        for (int y = SLIDER_INNER_OFFSET; y < mMaxSliderSize;  ++y)
        {
            for (int x = x1; x < x2; ++x)
            {
                if (y > y1 && y < y2)
                    mGfxBuffer[y * w + x] = color;
                else
                    mGfxBuffer[y * w + x] = mColorBackground;
            }
        }
    }
    // Blit.
    mParent->getTexture()->getBuffer()->blitFromMemory(
        PixelBox(mStopX- mStartX, mStopY-mStartY, 1, PF_A8B8G8R8, mGfxBuffer),
        Box(mStartX, mStartY, mStopX, mStopY));
}

//================================================================================================
// Resize the complete scrollbar. Value of <0 means keep the current value.
//================================================================================================
void GuiElementScrollbar::resize(int newWidth, int newHeight)
{
    if (newWidth < 0) newWidth = mWidth;
    if (newHeight< 0) newHeight= mHeight;
    if (newWidth == mWidth && newHeight == mHeight && mGfxBuffer) return;
    if (mHorizontal)
    {
        mStartX = mPosX + mButScrollDown->getWidth();
        mStartY = mPosY;
        mStopX  = mPosX + mWidth - mButScrollDown->getWidth();
        mStopY  = mPosY + mHeight;
        mButScrollUp->setPosition(mStopX, mPosY);
        mButScrollDown->setPosition(mPosX, mPosY);
        mMaxSliderSize = (mStopX -mStartX - SLIDER_INNER_OFFSET-1);
        mSliderSize = (mSliderSize*newWidth) /mWidth;
    }
    else
    {
        mStartX = mPosX;
        mStartY = mPosY + mButScrollDown->getHeight();
        mStopX  = mPosX + mButScrollDown->getWidth();
        mStopY  = mPosY + mHeight - mButScrollDown->getHeight();
        mButScrollDown->setPosition(mStartX, mStopY);
        mButScrollUp->setPosition(mPosX, mPosY);
        mMaxSliderSize = (mStopY -mStartY - SLIDER_INNER_OFFSET-1);
        mSliderSize = (mSliderSize*newHeight) /mHeight;
    }
    int w = mStopX-mStartX;
    int h = mStopY-mStartY;
    delete[] mGfxBuffer;
    mGfxBuffer = new uint32[w*h];

    // Background.
    for (int xy = 0; xy < w*h; ++xy) mGfxBuffer[xy] = mColorBackground;
    // Horizontal borderlines.
    for (int x = 1; x < w-1; ++x)
    {
        mGfxBuffer[   1 *w +x] = mColorBorderline;
        mGfxBuffer[(h-2)*w +x] = mColorBorderline;
    }
    // Vertical borderlines.
    for (int y = 2; y < h-2; ++y)
    {
        mGfxBuffer[y*w +  1] = mColorBorderline;
        mGfxBuffer[y*w +w-2] = mColorBorderline;
    }
    if (mButScrollUp)   mButScrollUp->draw();
    if (mButScrollDown) mButScrollDown->draw();
    //updateSliderSize(1, 1);
}
