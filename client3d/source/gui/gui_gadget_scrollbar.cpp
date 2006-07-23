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

const clock_t SCROLL_SPEED = 12;

///================================================================================================
/// Constructor.
///================================================================================================
GuiGadgetScrollbar::GuiGadgetScrollbar(TiXmlElement *xmlElement, void *parent):GuiElement(xmlElement, parent)
{
    /// ////////////////////////////////////////////////////////////////////
    /// Create buffer to hold the pixel information of the listbox.
    /// ////////////////////////////////////////////////////////////////////

    //for (int i =0; i < size; ++i) mGfxBuffer[i] = mFillColor;
    mButScrollUp  = 0;
    mButScrollDown= 0;


    if (mWidth > mHeight) mHorizontal = true;
    else                  mHorizontal = false;

    TiXmlElement *xmlOpt;
    uint32 *color = 0;
    const char *tmp;



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
            {
                mButScrollUp = new GuiGadgetButton(xmlOpt, parent, false);
                mButScrollUp->setPosition(mX, mY);
            }
            else if (!strcmp(xmlOpt->Attribute("name"), "But_ScrollDown"))
            {
                mButScrollDown = new GuiGadgetButton(xmlOpt, parent, false);
                if (mHorizontal)
                {
                    mStartX = mX + mButScrollDown->getWidth();
                    mStartY = mY;
                    mStopX  = mX + mWidth - mButScrollDown->getWidth();
                    mStopY  = mY + mHeight;
                    mButScrollDown->setPosition(mStopX, mY);
                }
                else
                {
                    mStartX = mX;
                    mStartY = mY + mButScrollDown->getHeight();
                    mStopX  = mX + mWidth;
                    mStopY  = mY + mHeight - mButScrollDown->getHeight();
                    mButScrollDown->setPosition(mStartX, mStopY);
                }
            }
        }
    }
    mGfxBuffer = new uint32[(mStopX-mStartX) * (mStopY-mStartY)];
    draw();
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
/// .
///================================================================================================
bool GuiGadgetScrollbar::mouseEvent(int MouseAction, int x, int y)
{
//    if (mScrollBarV && mScrollBarV->mouseEvent(MouseAction, x, y)) return true;
//    if (mScrollBarH && mScrollBarH->mouseEvent(MouseAction, x, y)) return true;
    return false;
}

///================================================================================================
/// .
///================================================================================================
void GuiGadgetScrollbar::draw()
{
    if (mButScrollUp)   mButScrollUp->draw();
    if (mButScrollDown) mButScrollDown->draw();
    int w = mStopX-mStartX;

    for (int x = 0; x < w; ++x)
    {
        for (int y = 0; y < mStopY-mStartY; ++y)
        {
            if (x ==1 || x == w-2)
                mGfxBuffer[y * w + x] = mColorBorderline;
            else
                mGfxBuffer[y * w + x] = mColorBackground;
        }
    }

    ((GuiWindow*)mParent)->getTexture()->getBuffer()->blitFromMemory(
        PixelBox(w, mStopY-mStartY, 1, PF_A8B8G8R8, mGfxBuffer),
        Box(mStartX, mStartY, mStopX, mStopY));
}
