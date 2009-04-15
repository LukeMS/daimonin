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
#include "gui_element_textbox.h"

using namespace Ogre;

//================================================================================================
//
//================================================================================================
int GuiElementTextbox::sendMsg(int message, const char *text, uint32 param)
{
    switch (message)
    {
        case GuiManager::MSG_SET_TEXT:
            mStrLabel = text;
            mLabelColor = param;
            draw();
            return 0;
        default:
            return -1;
    }
}

//================================================================================================
// .
//================================================================================================
GuiElementTextbox::GuiElementTextbox(TiXmlElement *xmlElement, void *parent):GuiElement(xmlElement, parent)
{
    const char *tmp = xmlElement->Attribute("hide");
    mHideText = (tmp && atoi(tmp))?true:false;
    if ((xmlElement = xmlElement->FirstChildElement("Tooltip")))
        if ((tmp = xmlElement->Attribute("text"))) mStrTooltip = tmp;
    int maxX, maxY;
    mParent->getSize(maxX, maxY);
    if (mWidth <= GuiElement::MIN_SIZE) // No value was set in the xml-file.
        mWidth  = GuiTextout::getSingleton().calcTextWidth(mStrLabel.c_str(), mLabelFontNr);
    if (mHeight<= GuiElement::MIN_SIZE) // No value was set in the xml-file.
        mHeight = GuiTextout::getSingleton().getFontHeight(mLabelFontNr);
    // Clip the text.
    if (mLabelPosX + mWidth >= maxX) mWidth  = --maxX - mLabelPosX;
    if (mLabelPosY + mHeight>= maxY) mHeight = --maxY - mLabelPosY;
    draw();
}

//================================================================================================
// Draw the guiElement.
//================================================================================================
void GuiElementTextbox::draw()
{
    uint32 *dst = GuiManager::getSingleton().getBuildBuffer();
    GuiTextout::getSingleton().printText(mWidth, mHeight, dst, mWidth,
                                         mParent->getLayerBG() + mLabelPosX + mLabelPosY*mParent->getWidth(), mParent->getWidth(), mStrLabel.c_str(), mLabelFontNr, mLabelColor, mHideText);
    mParent->getTexture()->getBuffer()->blitFromMemory(PixelBox(mWidth, mHeight, 1, PF_A8R8G8B8, dst), Box(mLabelPosX, mLabelPosY, mLabelPosX+mWidth, mLabelPosY+mHeight));
}
