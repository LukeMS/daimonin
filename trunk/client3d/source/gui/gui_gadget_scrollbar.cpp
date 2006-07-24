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


#include <tinyxml.h>
#include "define.h"
#include "gui_gadget_scrollbar.h"
#include "logger.h"
#include "gui_window.h"

//long i = Root::getSingleton().getTimer()->getMilliseconds()

const int MIN_SLIDER_SIZE = 6;

///================================================================================================
/// Constructor.
///================================================================================================
GuiGadgetScrollbar::GuiGadgetScrollbar(TiXmlElement *xmlElement, void *parent):GuiElement(xmlElement, parent)
{
    /// ////////////////////////////////////////////////////////////////////
    /// Create buffer to hold the pixel information of the listbox.
    /// ////////////////////////////////////////////////////////////////////
    mButScrollUp  = 0;
    mButScrollDown= 0;

    if (mWidth > mHeight) mHorizontal = true;
    else                  mHorizontal = false;

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
        /// PixelFormat: ARGB.
        if ((tmp = xmlOpt->Attribute("red"  ))) *color = atoi(tmp) << 16;
        if ((tmp = xmlOpt->Attribute("green"))) *color+= atoi(tmp) <<  8;
        if ((tmp = xmlOpt->Attribute("blue" ))) *color+= atoi(tmp);
        if ((tmp = xmlOpt->Attribute("alpha"))) *color+= atoi(tmp) << 24;
    }

    for (xmlOpt = xmlElement->FirstChildElement("Gadget"); xmlOpt; xmlOpt = xmlOpt->NextSiblingElement("Gadget"))
    {
        if (!strcmp(xmlOpt->Attribute("type"), "BUTTON"))
        {
            if (!strcmp(xmlOpt->Attribute("name"), "But_ScrollUp"))
                mButScrollUp = new GuiGadgetButton(xmlOpt, parent, false);
            else if (!strcmp(xmlOpt->Attribute("name"), "But_ScrollDown"))
                mButScrollDown = new GuiGadgetButton(xmlOpt, parent, false);
        }
    }
    mGfxBuffer = 0;
    mSliderPos = 0;
    mSliderSize = mHeight - 2* mButScrollDown->getHeight()-6;
    resize(mWidth, mHeight);
}

///================================================================================================
/// Destructor.
///================================================================================================
GuiGadgetScrollbar::~GuiGadgetScrollbar()
{
    delete[] mGfxBuffer;
    if (mButScrollUp)   delete mButScrollUp;
    if (mButScrollDown) delete mButScrollDown;
}

///================================================================================================
/// Mouse action in parent window.
///================================================================================================
bool GuiGadgetScrollbar::mouseEvent(int MouseAction, int x, int y)
{
    // Test the buttons.
    if (mButScrollUp && mButScrollUp->mouseEvent(MouseAction, x, y)) return true;
    if (mButScrollDown && mButScrollDown->mouseEvent(MouseAction, x, y)) return true;
    // Test the sliders.


    return false;
}

///================================================================================================
/// Update the slider size.
///================================================================================================
void GuiGadgetScrollbar::updateSlider(int actLines, int maxVisibleLines)
{
    if (actLines <= maxVisibleLines) return;
    mSliderSize = ((mStopY-mStartY-6) * maxVisibleLines) / actLines;
    if (mSliderSize < MIN_SLIDER_SIZE) mSliderSize = MIN_SLIDER_SIZE;
    draw();
}

///================================================================================================
/// Draw the slider.
///================================================================================================
void GuiGadgetScrollbar::draw()
{
    int y1 = 3 + mSliderPos;
    int y2 = y1 + mSliderSize;
    int w = mStopX-mStartX;

    uint32 color;
    if (mState == STATE_PUSHED)
        color = mColorBarActive;
    else
        color = mColorBorderline;

    for (int x = 4; x < mWidth-4; ++x)
    {
        for (int y = y1; y < y2; ++y)
        {
            if (mState == STATE_M_OVER && (x= 3 || x == mWidth-4 || y == y1 || y == y2-1))
                mGfxBuffer[y * w + x] = mColorBarM_Over;
            else
            {
                mGfxBuffer[y * w + x] = color;
            }
        }
    }

    // Dark borders.
    --y2;
    for (int y = y1; y < y2; ++y)      mGfxBuffer[y * w + mWidth-4] = 0xff000000;
    for (int x = 3; x < mWidth-3; ++x) mGfxBuffer[y2 * w + x ]      = 0xff000000;
    // Light borders.
    color+= ((color&0x0000ff)+ 0x000011) > 0x0000ff ? 0 : 0x000011;
    color+= ((color&0x00ff00)+ 0x001100) > 0x00ff00 ? 0 : 0x001100;
    color+= ((color&0xff0000)+ 0x110000) > 0xff0000 ? 0 : 0x110000;
    for (int y = y1; y < y2; ++y)      mGfxBuffer[y * w + 3 ] = color;
    for (int x = 4; x < mWidth-3; ++x) mGfxBuffer[y1 * w + x] = color;
    // Blit.
    ((GuiWindow*)mParent)->getTexture()->getBuffer()->blitFromMemory(
        PixelBox(mStopX- mStartX, mStopY-mStartY, 1, PF_A8B8G8R8, mGfxBuffer),
        Box(mStartX, mStartY, mStopX, mStopY));
}

///================================================================================================
/// Resize the complete scrollbar.
///================================================================================================
void GuiGadgetScrollbar::resize(int newWidth, int newHeight)
{
    if (newWidth == mWidth && newHeight == mHeight && mGfxBuffer) return;
    if (mHorizontal)
    {
        mStartX = mX + mButScrollDown->getWidth();
        mStartY = mY;
        mStopX  = mX + mWidth - mButScrollDown->getWidth();
        mStopY  = mY + mHeight;
        mButScrollDown->setPosition(mStopX, mY);
        mButScrollUp->setPosition(mX, mY);
    }
    else
    {
        mStartX = mX;
        mStartY = mY + mButScrollDown->getHeight();
        mStopX  = mX + mButScrollDown->getWidth();
        mStopY  = mY + mHeight - mButScrollDown->getHeight();
        mButScrollDown->setPosition(mStartX, mStopY);
        mButScrollUp->setPosition(mX, mY);
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
    draw();
}
