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
#include "gui_element_statusbar.h"

using namespace Ogre;

const Real ANIMATION_SPEED = 1.2;

//================================================================================================
// .
//================================================================================================
void GuiStatusbar::sendMsg(const int message, Ogre::String &text, Ogre::uint32 &param, const char *text2)
{
    if (message == GuiManager::MSG_SET_VALUE)
        setValue(param);
}

//================================================================================================
//
//================================================================================================
void GuiStatusbar::update(Ogre::Real dTime)
{
    if (mDrawn == mValue) return;
    if (mSmoothChange)
    {
        if (mDrawn < mValue)
        {
            mDrawn += (dTime*mLength) / ANIMATION_SPEED;
            if (mDrawn > mValue) mDrawn = mValue;
        }
        else
        {
            mDrawn -= (dTime*mLength) / ANIMATION_SPEED;
            if (mDrawn < mValue) mDrawn = mValue;
        }
    }
    else
        mDrawn = mValue;
    draw();
}

//================================================================================================
// Set the length of the statusbar. (0.0f ... 1.0f)
//================================================================================================
void GuiStatusbar::setValue(int value)
{
    mValue = (value <= 0)?0:(value*mLength)/100;
}

//================================================================================================
// .
//================================================================================================
GuiStatusbar::GuiStatusbar(TiXmlElement *xmlElement, const void *parent):GuiElement(xmlElement, parent)
{
    const char *temp = xmlElement->Attribute("smooth");
    mSmoothChange = (temp && atoi(temp))?true:false;
    TiXmlElement *xmlElem = xmlElement->FirstChildElement("Color");
    temp = xmlElem->Attribute("auto");
    mAutoColor = (temp && atoi(temp))?true:false;
    mWidth -= mWidth&1?1:0;  // Make it even.
    mHeight-= mHeight&1?1:0; // Make it even.
    if (mWidth > mHeight)
    {
        mHorizontal= true;
        mDiameter  = mHeight;
        mLength    = mWidth;
    }
    else
    {
        mHorizontal= false;
        mDiameter  = mWidth;
        mLength    = mHeight;
    }
    mDrawn = mValue = mLength;
    draw();
}

//================================================================================================
//
//================================================================================================
void GuiStatusbar::draw()
{
    if (!mVisible)
    {
        GuiElement::draw(true);
        return;
    }
    uint32 *dst = GuiManager::getSingleton().getBuildBuffer();
    if (mGfxSrc)
        drawGfxBar(dst);
    else
        drawColorBar(dst);
    mParent->getTexture()->getBuffer()->blitFromMemory(PixelBox(mWidth, mHeight, 1, PF_A8R8G8B8, dst), Box(mPosX, mPosY, mPosX+mWidth, mPosY+mHeight));
}

//================================================================================================
//
//================================================================================================
void GuiStatusbar::drawColorBar(uint32 *dst)
{
    if (mAutoColor)
    {
        if      (mDrawn > mLength/2) mFillColor= 0x0000ff00; // green bar.
        else if (mDrawn > mLength/3) mFillColor= 0x00ffff00; // yellow bar.
        else                         mFillColor= 0x00ff0000; // red bar.
    }
    uint32 dColor;
    dColor = ((mFillColor & 0x00ff0000)/ (mDiameter/2+1)) & 0x00ff0000;
    dColor+= ((mFillColor & 0x0000ff00)/ (mDiameter/2+1)) & 0x0000ff00;
    dColor+= ((mFillColor & 0x000000ff)/ (mDiameter/2+1)) & 0x000000ff;
    uint32 color = 0xff000000 + dColor;
    if (mHorizontal)
    {
        for (int x=0; x < mDiameter/2; ++x)
        {
            for (int y = 0; y < mLength - mDrawn; ++y)
            {
                dst[y*mWidth + x] = 0xff000000;
                dst[y*mWidth + mDiameter-x-1] = 0xff000000;
            }
            for (int y = mLength - (int)mDrawn; y < mLength; ++y)
            {
                dst[y*mWidth + x] = color;
                dst[y*mWidth + mDiameter-x-1] = color;
            }
            color+= dColor;
        }
    }
    else
    {
        for (int y=0; y < mDiameter/2; ++y)
        {
            for (int x = 0; x < mDrawn; ++x)
            {
                dst[y*mWidth + x] = color;
                dst[(mDiameter-y-1)*mWidth + x] = color;
            }
            for (int x = (int)mDrawn; x < mLength; ++x)
            {
                dst[y*mWidth + x] = 0xff000000;
                dst[(mDiameter-y-1)*mWidth + x] = 0xff000000;
            }
            color+= dColor;
        }
    }
}

//================================================================================================
//
//================================================================================================
void GuiStatusbar::drawGfxBar(uint32 *dst)
{
    // todo
}
