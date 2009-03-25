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
#include "gui_graphic.h"
#include "gui_element.h"

using namespace Ogre;

//================================================================================================
//
//================================================================================================
int GuiElement::sendMsg(int, const char *, uint32)
{
    return 0;
}

//================================================================================================
//
//================================================================================================
const char *GuiElement::sendMsg(int info)
{
    return 0;
}

//================================================================================================
//
//================================================================================================
int GuiElement::mouseEvent(int, int, int, int)
{
    return GuiManager::EVENT_CHECK_NEXT; // No action here, check the other gadgets.
}

//================================================================================================
//
//================================================================================================
int GuiElement::keyEvent(const int, const unsigned int)
{
    return GuiManager::EVENT_CHECK_NEXT;
}

//================================================================================================
// Parse a gui element.
//================================================================================================
GuiElement::GuiElement(TiXmlElement *xmlElem, void *parent)
{
    TiXmlElement *xmlElement;
    String strValue;
    const char *tmp;
    int maxX, maxY;
    // Set default values.
    mState = GuiImageset::STATE_ELEMENT_DEFAULT;
    mFontNr= 0;
    mWidth = MIN_SIZE;
    mHeight= MIN_SIZE;
    mVisible = true;
    mGfxSrc    = 0; // No gfx is defined (fallback to color fill).
    mFillColor = 0xffffffff;
    mParent= (GuiWindow*)parent;
    mParent->getSize(maxX, maxY);
    // ////////////////////////////////////////////////////////////////////
    // Parse the element.
    // ////////////////////////////////////////////////////////////////////
    mIndex = GuiManager::getSingleton().getElementIndex(xmlElem->Attribute("name"), mParent->getID(), mParent->getSumElements());
    // ////////////////////////////////////////////////////////////////////
    // Parse the background image (if given).
    // ////////////////////////////////////////////////////////////////////
    if ((xmlElement = xmlElem->FirstChildElement("Image")))
    {
        if ((tmp = xmlElement->Attribute("name")))
        {
            if ((mGfxSrc = GuiImageset::getSingleton().getStateGfxPositions(tmp)))
            {
                mWidth = mGfxSrc->w;  // Set the width of the source gfx as standard width.
                mHeight= mGfxSrc->h;  // Set the height of the source gfx as standard height.
            }
            else
            {
                Logger::log().warning() << "Image " << tmp << " was defined in '" << GuiManager::FILE_DESCRIPTION_WINDOWS
                << "' but the gfx-data in '" << GuiManager::FILE_DESCRIPTION_IMAGESET << "' is missing.";
            }
        }
    }
    // ////////////////////////////////////////////////////////////////////
    // Parse the color (if given).
    // ////////////////////////////////////////////////////////////////////
    if ((xmlElement = xmlElem->FirstChildElement("Color")))
    {
        // PixelFormat: ARGB.
        if ((tmp = xmlElement->Attribute("red"  ))) mFillColor = atoi(tmp) << 16;
        if ((tmp = xmlElement->Attribute("green"))) mFillColor+= atoi(tmp) <<  8;
        if ((tmp = xmlElement->Attribute("blue" ))) mFillColor+= atoi(tmp);
        if ((tmp = xmlElement->Attribute("alpha"))) mFillColor+= atoi(tmp) << 24;
    }

    if ((tmp = xmlElem->Attribute("font"))) mFontNr  = atoi(tmp);
    // ////////////////////////////////////////////////////////////////////
    // Parse the position.
    // ////////////////////////////////////////////////////////////////////
    mPosX = 0; mPosY = 0;
    if ((xmlElement = xmlElem->FirstChildElement("Pos")))
    {
        if ((tmp = xmlElement->Attribute("x"))) mPosX = atoi(tmp);
        if ((tmp = xmlElement->Attribute("y"))) mPosY = atoi(tmp);
    }
    if (mPosX > maxX-2) mPosX = maxX-2;
    if (mPosY > maxY-2) mPosY = maxY-2;
    // ////////////////////////////////////////////////////////////////////
    // Parse the size (if given).
    // ////////////////////////////////////////////////////////////////////
    if ((xmlElement = xmlElem->FirstChildElement("Size")))
    {
        if ((tmp = xmlElement->Attribute("width")))  mWidth = atoi(tmp);
        if ((tmp = xmlElement->Attribute("height"))) mHeight= atoi(tmp);
    }
    if (mPosX + mWidth >= maxX) mWidth = maxX-mPosX;
    else if (mWidth < MIN_SIZE) mWidth = MIN_SIZE;
    if (mPosY + mHeight>= maxY) mHeight= maxY-mPosY;
    else if (mHeight< MIN_SIZE) mHeight= MIN_SIZE;
    GuiManager::getSingleton().resizeBuildBuffer(mWidth*mHeight);
    // ////////////////////////////////////////////////////////////////////
    // Parse the label (if given).
    // ////////////////////////////////////////////////////////////////////
    mLabelPosX = 0; mLabelPosY = 0; mLabelFontNr = 0, mLabelColor = 0x00ffffff;
    if ((xmlElement = xmlElem->FirstChildElement("Label")))
    {
        if ((tmp = xmlElement->Attribute("x"))) mLabelPosX  = (unsigned short)atoi(tmp);
        if ((tmp = xmlElement->Attribute("y"))) mLabelPosY  = (unsigned short)atoi(tmp);
        if (mLabelPosX >= maxX-2) mLabelPosX = 0;
        if (mLabelPosY >= maxY-2) mLabelPosY = 0;
        if ((tmp = xmlElement->Attribute("font")))  mLabelFontNr= atoi(tmp);
        if ((tmp = xmlElement->Attribute("red")))   mLabelColor+= atoi(tmp) << 16;
        if ((tmp = xmlElement->Attribute("green"))) mLabelColor+= atoi(tmp) << 8;
        if ((tmp = xmlElement->Attribute("blue")))  mLabelColor+= atoi(tmp);
        if ((tmp = xmlElement->Attribute("text")))  mStrLabel = tmp;
    }
    mVisible = true;
}

//================================================================================================
// Set the state.
//================================================================================================
bool GuiElement::setState(int state)
{
    if (state < GuiImageset::STATE_ELEMENT_SUM && mState != state)
    {
        mState = state;
        return true;
    }
    return false;
}

//================================================================================================
// .
//================================================================================================
void GuiElement::setVisible(bool visible)
{
    if (visible == mVisible) return;
    mVisible = visible;
    draw();
}

//================================================================================================
// Draws a graphic to the window texture.
// If the gfx is bigger than the source image, the source image will be repeated.
//================================================================================================
void GuiElement::draw(bool uploadToTexture)
{
    uint32 *dst = GuiManager::getSingleton().getBuildBuffer();
    uint32 *bak = mParent->getLayerBG() + mPosX + mPosY*mParent->getWidth();
    // Draws a gfx into the window texture.
    if (mGfxSrc)
    {
        PixelBox src = GuiImageset::getSingleton().getPixelBox().getSubVolume(Box(mGfxSrc->state[mState].x, mGfxSrc->state[mState].y,
                       mGfxSrc->state[mState].x + mGfxSrc->w, mGfxSrc->state[mState].y + mGfxSrc->h));
        int srcRowSkip = (int)GuiImageset::getSingleton().getPixelBox().getWidth();
        if (mIndex < 0) // This gfx is part of the background.
            GuiGraphic::getSingleton().drawGfxToBuffer(mWidth, mHeight, mGfxSrc->w, mGfxSrc->h, (uint32*)src.data, bak, bak, srcRowSkip, mParent->getWidth(), mParent->getWidth());
        else if (mVisible)
            GuiGraphic::getSingleton().drawGfxToBuffer(mWidth, mHeight, mGfxSrc->w, mGfxSrc->h, (uint32*)src.data, bak, dst, srcRowSkip, mParent->getWidth(), mWidth);
    }
    // Draws a color area to the window texture.
    else
    {
        if (mIndex < 0) // The gfx is part of the background.
            GuiGraphic::getSingleton().drawColorToBuffer(mWidth, mHeight, mFillColor, bak, mParent->getWidth());
        else if (mVisible)
            GuiGraphic::getSingleton().drawColorToBuffer(mWidth, mHeight, mFillColor, bak, dst, mParent->getWidth(), mWidth);
    }
    if (mIndex < 0 || !mVisible)
        GuiGraphic::getSingleton().restoreWindowBG(mWidth, mHeight, bak, dst, mParent->getWidth(), mWidth);
    if (uploadToTexture)
        mParent->getTexture()->getBuffer()->blitFromMemory(PixelBox(mWidth, mHeight, 1, PF_A8R8G8B8, dst), Box(mPosX, mPosY, mPosX+mWidth, mPosY+mHeight));
}
