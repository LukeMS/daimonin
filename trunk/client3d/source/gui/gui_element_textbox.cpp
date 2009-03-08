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
#include "gui_element_textbox.h"
#include "gui_textout.h"
#include "gui_window.h"
#include "gui_manager.h"

using namespace Ogre;

//================================================================================================
//
//================================================================================================
int GuiElementTextbox::sendMsg(int message, const char *text, uint32 param)
{
    switch (message)
    {
        case GuiManager::MSG_SET_VISIBLE:
            setVisible(param?true:false);
            return 0;
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
    const char *tmp;
    if ((xmlElement = xmlElement->FirstChildElement("Tooltip")))
    {
        if ((tmp = xmlElement->Attribute("text"))) mStrTooltip = tmp;
    }
    const int MINIMAL_ELEMENT_SIZE = 2;
    mMouseOver = false;
    mMouseButDown = false;
    int maxX, maxY;
    mParent->getSize(maxX, maxY);
    if (mWidth <= MINIMAL_ELEMENT_SIZE) // No value was set in the xml-file.
        mWidth = GuiTextout::getSingleton().calcTextWidth(mStrLabel.c_str(), mLabelFontNr);
    if (mHeight<= MINIMAL_ELEMENT_SIZE) // No value was set in the xml-file.
        mHeight= GuiTextout::getSingleton().getFontHeight(mLabelFontNr);
    // Clip the text.
    if (mLabelPosX + mWidth >= maxX) mWidth  = maxX - mLabelPosX - 1;
    if (mLabelPosY + mHeight>= maxY) mHeight = maxY - mLabelPosY - 1;
    draw();
}

//================================================================================================
// .
//================================================================================================
GuiElementTextbox::~GuiElementTextbox()
{}

//================================================================================================
// .
//================================================================================================
void GuiElementTextbox::setVisible(bool visible)
{
    if (visible == mIsVisible) return;
    mIsVisible = visible;
    draw();
}

//================================================================================================
// Returns true if the mouse event was on this gadget (so no need to check the other gadgets).
//================================================================================================
int GuiElementTextbox::mouseEvent(int MouseAction, int x, int y, int z)
{
    if (!mouseWithin(x, y))
    {
        // Mouse is no longer over the the gadget.
        if (getState() != GuiImageset::STATE_ELEMENT_DEFAULT)
        {
            mMouseOver = false;
            mMouseButDown = false;
            if (setState(GuiImageset::STATE_ELEMENT_DEFAULT)) draw();
            GuiManager::getSingleton().setTooltip("");
            return GuiManager::EVENT_CHECK_NEXT;
        }
    }
    else
    {
        if (!mMouseOver)
        {
            mMouseOver = true;
            if (setState(GuiImageset::STATE_ELEMENT_M_OVER)) draw();
            GuiManager::getSingleton().setTooltip(mStrTooltip.c_str());
            return GuiManager::EVENT_CHECK_DONE;
        }
        if (MouseAction == GuiManager::BUTTON_PRESSED && !mMouseButDown)
        {
            mMouseButDown = true;
            if (setState(GuiImageset::STATE_ELEMENT_PUSHED)) draw();
            return GuiManager::EVENT_CHECK_DONE;
        }
        if (MouseAction == GuiManager::BUTTON_RELEASED && mMouseButDown)
        {
            mMouseButDown = false;
            if (setState(GuiImageset::STATE_ELEMENT_DEFAULT)) draw();
            return GuiManager::EVENT_USER_ACTION;
        }
        return GuiManager::EVENT_CHECK_DONE; // No need to check other gadgets.
    }
    return GuiManager::EVENT_CHECK_NEXT; // No action here, check the other gadgets.
}

//================================================================================================
// Draw the guiElement.
//================================================================================================
void GuiElementTextbox::draw()
{
    uint32 *dst = GuiManager::getSingleton().getBuildBuffer();
    GuiTextout::getSingleton().printText(mWidth, mHeight, dst, mWidth,
        mParent->getLayerBG() + mLabelPosX + mLabelPosY*mParent->getWidth(), mParent->getWidth(), mStrLabel.c_str(), mLabelFontNr, mLabelColor);
    mParent->getTexture()->getBuffer()->blitFromMemory(PixelBox(mWidth, mHeight, 1, PF_A8R8G8B8, dst), Box(mLabelPosX, mLabelPosY, mLabelPosX+mWidth, mLabelPosY+mHeight));
}
