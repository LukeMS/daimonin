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
#include "gui_gadget_scrollbar.h"

using namespace Ogre;

const int MIN_SLIDER_SIZE = 6;
const int SLIDER_INNER_OFFSET = 3;

// TODO:
// Disable slider if it doesnt fit into the window.
// Print a warning (logfile) if the up/down buttons dont fit into the window.
// Keep slider-pos when lines were added.


//================================================================================================
// Constructor.
//================================================================================================
GuiGadgetScrollbar::GuiGadgetScrollbar(TiXmlElement *xmlElement, void *parent, void *parentElement):GuiElement(xmlElement, parent)
{
    mButScrollUp  = 0;
    mButScrollDown= 0;
    mParentElement= parentElement;
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
    for (xmlOpt = xmlElement->FirstChildElement("Gadget"); xmlOpt; xmlOpt = xmlOpt->NextSiblingElement("Gadget"))
    {
        if (!strcmp(xmlOpt->Attribute("type"), "BUTTON"))
        {
            if (!strcmp(xmlOpt->Attribute("name"), "But_ScrollUp"))
                mButScrollUp = new GuiGadgetButton(xmlOpt, parent);
            else if (!strcmp(xmlOpt->Attribute("name"), "But_ScrollDown"))
                mButScrollDown = new GuiGadgetButton(xmlOpt, parent);
        }
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
GuiGadgetScrollbar::~GuiGadgetScrollbar()
{
    delete[] mGfxBuffer;
    delete mButScrollUp;
    delete mButScrollDown;
}

//================================================================================================
// Mouse action in parent window.
//================================================================================================
bool GuiGadgetScrollbar::mouseEvent(int MouseAction, int x, int y)
{
    // Test the right/up button.
    if (!mDragging && mButScrollUp && mButScrollUp->mouseEvent(MouseAction, x, y))
    {
        if (MouseAction == GuiWindow::BUTTON_RELEASED)
        {
            updateSliderPos(mHorizontal?BUTTON_H_ADD:BUTTON_V_ADD, -1);
        }
        return true;
    }
    // Test the left/down button.
    if (!mDragging && mButScrollDown && mButScrollDown->mouseEvent(MouseAction, x, y))
    {
        if (MouseAction == GuiWindow::BUTTON_RELEASED)
        {
            updateSliderPos(mHorizontal?BUTTON_H_SUB:BUTTON_V_SUB, +1);
        }
        return true;
    }
    // Test the slider.
    if (mDragging ||
            (x > mStartX + SLIDER_INNER_OFFSET && x < mStopX - 2* SLIDER_INNER_OFFSET &&
             y > mStartY + SLIDER_INNER_OFFSET && y < mStopY - 2* SLIDER_INNER_OFFSET))
    {
        static int dragSliderPos = 0;
        if (!mMouseOver)
        {
            mMouseOver = true;
            setState(GuiImageset::STATE_ELEMENT_M_OVER);
            GuiManager::getSingleton().setTooltip(mStrTooltip.c_str());
        }
        if (MouseAction == GuiWindow::BUTTON_PRESSED && !mMouseButDown)
        {
            mDragging = true;
            dragSliderPos = y - mSliderPos;
            mMouseButDown = true;
            setState(GuiImageset::STATE_ELEMENT_PUSHED);
        }
        if (MouseAction == GuiWindow::BUTTON_RELEASED && mMouseButDown)
        {
            mDragging = false;
            mMouseButDown = false;
            setState(GuiImageset::STATE_ELEMENT_DEFAULT);
        }
        if (MouseAction == GuiWindow::MOUSE_MOVEMENT && mDragging)
        {
            updateSliderPos(mHorizontal?SLIDER_H:SLIDER_V, y-dragSliderPos);
        }
        return true; // No need to check other gadgets.
    }
    else  // Mouse is no longer over the the gadget.
    {
        if (mMouseOver)
        {
            mMouseOver = false;
            mMouseButDown = false;
            setState(GuiImageset::STATE_ELEMENT_DEFAULT);
            GuiManager::getSingleton().setTooltip("");
            return true; // No need to check other gadgets.
        }
    }
    // If dragging is active, the parent window stays active if the mouse moves outside the window.
    if (mDragging) return true;
    return false; // No action here, check the other gadgets.
}

//================================================================================================
// Draw the slider.
//================================================================================================
void GuiGadgetScrollbar::draw()
{
    int x1, x2, y1, y2, w;
    w = mStopX-mStartX;
    int sliderSize = mSliderSize<MIN_SLIDER_SIZE?MIN_SLIDER_SIZE:mSliderSize;
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
        x2 = x1 + sliderSize;
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
        y2 = y1 + sliderSize;
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
    /*
    // Dark borders.
    --y2;
    posY = y2 *mWidth;
    for (int y = y1*mWidth + mWidth -4; y < posY; ++y)
    {
        mGfxBuffer[y] = 0xff000000;
        y+= mWidth-1;
    }
    posY = y2 *w;
    for (int x = 3; x < mWidth-3; ++x) mGfxBuffer[posY + x] = 0xff000000;
    // Light borders.
    color+= ((color&0x0000ff)+ 0x000011) > 0x0000ff ? 0 : 0x000011;
    color+= ((color&0x00ff00)+ 0x001100) > 0x00ff00 ? 0 : 0x001100;
    color+= ((color&0xff0000)+ 0x110000) > 0xff0000 ? 0 : 0x110000;
    posY = y2*mWidth;
    for (int y = y1*mWidth + 3; y < posY; ++y)
    {
        mGfxBuffer[y] = color;
        y+= mWidth-1;
    }
    for (int x = 4; x < mWidth-3; ++x) mGfxBuffer[y1 * mWidth + x] = color;
    */
    // Blit.
    mParent->getTexture()->getBuffer()->blitFromMemory(
        PixelBox(mStopX- mStartX, mStopY-mStartY, 1, PF_A8B8G8R8, mGfxBuffer),
        Box(mStartX, mStartY, mStopX, mStopY));
}

//================================================================================================
// Update the slider size.
// Call from an external source (e.g when a new line was added to a listbox)
//================================================================================================
void GuiGadgetScrollbar::updateSliderSize(int actPos, int maxVisiblePos, int maxPos)
{
    if (actPos > maxPos) actPos = maxPos;
    mSliderSize = (maxPos <= maxVisiblePos)?mMaxSliderSize:(mMaxSliderSize * maxVisiblePos) / maxPos;
    // Set the new slider position.
    mMaxSliderPos = mMaxSliderSize - mSliderSize;
    mSliderPos = (mMaxSliderSize * actPos) / maxVisiblePos;
    if (mSliderPos > mMaxSliderPos) mSliderPos = mMaxSliderPos;
    // Draw the slider.
    mSingleLineSize = (actPos > maxVisiblePos)?(float)mSliderPos/(float)(actPos-maxVisiblePos):0;
    draw();
}

//================================================================================================
// Update the slider position.
// Call from a button/slider of this object.
//================================================================================================
void GuiGadgetScrollbar::updateSliderPos(int type, int offset)
{
    // Slider was moved.
    if (type == SLIDER_H || type == SLIDER_V)
    {
        if (offset <= 0) offset =1;
        else if (offset > mMaxSliderPos) offset = mMaxSliderPos;
        if (mSliderPos == offset) return;
        mSliderPos = offset;
    }
    // Scroll backward button was pressed.
    else if (mSliderPos >0 && offset <0)
    {
        if (mSliderPos <= 0) return;
        mSliderPos-= (int)mSingleLineSize;
        if (mSliderPos <= 0) mSliderPos =1;
    }
    // Scroll forward button was pressed.
    else if (mSliderPos < mMaxSliderPos && offset >0)
    {
        if (mSliderPos >= mMaxSliderPos) return;
        mSliderPos+= (int)(mSingleLineSize);
        if (mSliderPos > mMaxSliderPos) mSliderPos = mMaxSliderPos;
    }
    // Unknown action.
    else return;
    draw();
    activated(type, (int) (mSliderPos / mSingleLineSize));
}

//================================================================================================
// Resize the complete scrollbar.
//================================================================================================
void GuiGadgetScrollbar::resize(int newWidth, int newHeight)
{
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
    delete[] mGfxBuffer; // delete a NULL-Pointer is save in c++.
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
    updateSliderSize(1, 1);
}
