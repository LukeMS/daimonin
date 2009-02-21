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

#include <Ogre.h>
#include "logger.h"
#include "define.h"
#include "gui_element.h"
#include "gui_graphic.h"
#include "gui_imageset.h"
#include "gui_window.h"
#include "gui_manager.h"

using namespace Ogre;

int GuiElement::sendMsg(int, void *, void *, void *)
{
    return 0;
}

int GuiElement::mouseEvent(int, int, int, int)
{
    return GuiManager::EVENT_CHECK_NEXT; // No action here, check the other gadgets.
}

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
    // Set default values.
    mState = GuiImageset::STATE_ELEMENT_DEFAULT;
    mFontNr= 0;
    mWidth = 1; mHeight= 1; // Default values (must be 2).
    mIsVisible = true;
    mGfxSrc    = 0; // No gfx is defined (fallback to color fill).
    mFillColor = 0;
    mParent= (GuiWindow*)parent;
    int maxX, maxY;
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
                Logger::log().warning() << "Image " << tmp << " was defined in '" << FILE_GUI_WINDOWS
                << "' but the gfx-data in '" << FILE_GUI_IMAGESET << "' is missing.";
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
    if (mPosY + mHeight>= maxY) mHeight= maxY-mPosY;
    // ////////////////////////////////////////////////////////////////////
    // Parse the label (if given).
    // ////////////////////////////////////////////////////////////////////
    mLabelPosX = 0; mLabelPosY = 0; mLabelFontNr = 0, mLabelColor = 0;
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
    // ////////////////////////////////////////////////////////////////////
    // Parse the Tooltip entry.
    // ////////////////////////////////////////////////////////////////////
    if ((xmlElement = xmlElem->FirstChildElement("Tooltip")))
    {
        if ((tmp = xmlElement->Attribute("text"))) mStrTooltip = tmp;
    }
    mIsVisible = true;
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
// Draws a graphic to the window texture.
// If the gfx is bigger than the source image, the source image will be repeated.
//================================================================================================
void GuiElement::draw()
{
    Texture *texture = mParent->getTexture();
    uint32 *bak = mParent->getLayerBG() + mPosX + mPosY*mParent->getWidth();
    PixelBox pb = texture->getBuffer()->lock(Box(mPosX, mPosY, mPosX+mWidth, mPosY+mHeight), HardwareBuffer::HBL_DISCARD);
    uint32 *dst = (uint32*)pb.data;
    // Draws a gfx into the window texture.
    if (mGfxSrc)
    {
        PixelBox src = mParent->getPixelBox()->getSubVolume(Box(mGfxSrc->state[mState].x, mGfxSrc->state[mState].y,
                       mGfxSrc->state[mState].x + mGfxSrc->w, mGfxSrc->state[mState].y + mGfxSrc->h));
        int srcRowSkip = (int)mParent->getPixelBox()->getWidth();
        if (mIndex < 0) // This gfx is part of the background.
            GuiGraphic::getSingleton().drawGfxToBuffer(mWidth, mHeight, mGfxSrc->w, mGfxSrc->h, (uint32*)src.data, bak, bak, srcRowSkip, mParent->getWidth(), mParent->getWidth());
        else if (mIsVisible)
            GuiGraphic::getSingleton().drawGfxToBuffer(mWidth, mHeight, mGfxSrc->w, mGfxSrc->h, (uint32*)src.data, bak, dst, srcRowSkip, mParent->getWidth(), (int)texture->getWidth());
    }
    // Draws a color area to the window texture.
    else
    {
        if (mIndex < 0) // The gfx is part of the background.
            GuiGraphic::getSingleton().drawColorToBuffer(mWidth, mHeight, mFillColor, bak, mParent->getWidth());
        else if (mIsVisible)
            GuiGraphic::getSingleton().drawColorToBuffer(mWidth, mHeight, mFillColor, bak, dst, mParent->getWidth(), (int)texture->getWidth());
    }
    if (mIndex < 0 || !mIsVisible)
        GuiGraphic::getSingleton().restoreWindowBG(mWidth, mHeight, bak, dst, mParent->getWidth(), (int)texture->getWidth());
    texture->getBuffer()->unlock();
}
