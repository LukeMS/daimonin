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
#include "gui_imageset.h"
#include "gui_window.h"

using namespace Ogre;

//================================================================================================
// Parse a gui element.
//================================================================================================
GuiElement::GuiElement(TiXmlElement *xmlElem, void *parent)
{
    TiXmlElement *xmlGadget;
    String strValue;
    const char *tmp;
    // Set default values.
    mState = GuiImageset::STATE_ELEMENT_DEFAULT;
    mFontNr= 0;
    mPosX  = 0;
    mPosY  = 0;
    mWidth = 0;
    mHeight= 0;
    mIsVisible = true;
    mBG_Element = true;
    mGfxSrc    = 0; // No gfx is defined (fallback to color fill).
    mFillColor = 0;
    mParent= (GuiWindow*)parent;
    int maxX, maxY;
    mParent->getTexturseSize(maxX, maxY);
    // ////////////////////////////////////////////////////////////////////
    // Parse the element.
    // ////////////////////////////////////////////////////////////////////
    if ((tmp = xmlElem->Attribute("name")))
    {
        mBG_Element = false;   // This ins an interactive element (not a part of the bg gfx).
        for (int i = 0; i < GuiImageset::GUI_ELEMENTS_SUM; ++i)
        {
            if (!stricmp(GuiImageset::getSingleton().getElementName(i), tmp))
            {
                mIndex = GuiImageset::getSingleton().getElementIndex(i);
                break;
            }
        }
    }
    // ////////////////////////////////////////////////////////////////////
    // Parse the background image (if given).
    // ////////////////////////////////////////////////////////////////////
    if ((xmlGadget = xmlElem->FirstChildElement("Image")))
    {
        if ((tmp = xmlGadget->Attribute("name")))
        {
            if ((mGfxSrc = GuiImageset::getSingleton().getStateGfxPositions(tmp)))
            {
                mWidth = mGfxSrc->w;  // Set the width of the source gfx as standard width.
                mHeight= mGfxSrc->h;  // Set the height of the source gfx as standard height.
            }
            else
            {
                Logger::log().warning() << tmp << " was defined in '" << FILE_GUI_WINDOWS
                << "' but the gfx-data in '" << FILE_GUI_IMAGESET << "' is missing.";
            }
        }
    }
    // ////////////////////////////////////////////////////////////////////
    // Parse the color (if given).
    // ////////////////////////////////////////////////////////////////////
    if ((xmlGadget = xmlElem->FirstChildElement("Color")))
    {
        // PixelFormat: ARGB.
        if ((tmp = xmlGadget->Attribute("red"  ))) mFillColor = atoi(tmp) << 16;
        if ((tmp = xmlGadget->Attribute("green"))) mFillColor+= atoi(tmp) <<  8;
        if ((tmp = xmlGadget->Attribute("blue" ))) mFillColor+= atoi(tmp);
        if ((tmp = xmlGadget->Attribute("alpha"))) mFillColor+= atoi(tmp) << 24;
    }

    if ((tmp = xmlElem->Attribute("font"))) mFontNr  = atoi(tmp);
    // ////////////////////////////////////////////////////////////////////
    // Parse the position.
    // ////////////////////////////////////////////////////////////////////
    if ((xmlGadget = xmlElem->FirstChildElement("Pos")))
    {
        if ((tmp = xmlGadget->Attribute("x"))) mPosX = atoi(tmp);
        if ((tmp = xmlGadget->Attribute("y"))) mPosY = atoi(tmp);
    }
    if (mPosX > maxX-2) mPosX = maxX-2;
    if (mPosY > maxY-2) mPosY = maxY-2;
    // ////////////////////////////////////////////////////////////////////
    // Parse the size (if given).
    // ////////////////////////////////////////////////////////////////////
    if ((xmlGadget = xmlElem->FirstChildElement("Range")))
    {
        if ((tmp = xmlGadget->Attribute("width")))  mWidth = atoi(tmp);
        if ((tmp = xmlGadget->Attribute("height"))) mHeight= atoi(tmp);
    }
    if (mPosX + mWidth > maxX) mWidth = maxX-mPosX-1;
    if (mPosY + mHeight >maxY) mHeight= maxY-mPosY-1;
    // ////////////////////////////////////////////////////////////////////
    // Parse the label  (if given).
    // ////////////////////////////////////////////////////////////////////
    if ((xmlGadget = xmlElem->FirstChildElement("Label")))
    {
        if ((tmp = xmlGadget->Attribute("x")))  mLabelPosX  = atoi(tmp);
        if ((tmp = xmlGadget->Attribute("y")))  mLabelPosY  = atoi(tmp);
        if ((tmp = xmlGadget->Attribute("font")))  mLabelFontNr= atoi(tmp);
        if ((tmp = xmlGadget->Attribute("red")))   mLabelColor[0]= (unsigned char) atoi(tmp);
        if ((tmp = xmlGadget->Attribute("green"))) mLabelColor[1]= (unsigned char) atoi(tmp);
        if ((tmp = xmlGadget->Attribute("blue")))  mLabelColor[2]= (unsigned char) atoi(tmp);
        if ((tmp = xmlGadget->Attribute("text")))  mStrLabel = tmp;
    }
    // ////////////////////////////////////////////////////////////////////
    // Parse the Tooltip entry.
    // ////////////////////////////////////////////////////////////////////
    if ((xmlGadget = xmlElem->FirstChildElement("Tooltip")))
    {
        if ((tmp = xmlGadget->Attribute("text"))) mStrTooltip = tmp;
    }
}

//================================================================================================
// Set the state.
//================================================================================================
bool GuiElement::setState(int state)
{
    if (state < GuiImageset::STATE_ELEMENT_SUM && mState != state)
    {
        mState = state;
        draw();
        return true;
    }
    return false;
}
