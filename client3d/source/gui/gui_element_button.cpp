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

#include "gui_textout.h"
#include "gui_element_button.h"

using namespace Ogre;

//================================================================================================
// Constructor.
//================================================================================================
GuiElementButton::GuiElementButton(TiXmlElement *xmlElement, const void *parent, const bool drawOnInit):GuiElement(xmlElement, parent)
{
    if ((xmlElement = xmlElement->FirstChildElement("Tooltip")))
    {
        const char *tmp;
        if ((tmp = xmlElement->Attribute("text"))) mStrTooltip = tmp;
    }
    mMouseOver = false;
    mMouseButDown = false;
    if (drawOnInit) draw();
}

//================================================================================================
// Message handling.
//================================================================================================
void GuiElementButton::sendMsg(const int message, String &text, uint32 &param, const char *text2)
{
    switch (message)
    {
        case GuiManager::MSG_SET_VISIBLE:
            setVisible(param?true:false);
            return;
        case GuiManager::MSG_SET_TEXT:
            setLabel(text);
            return;
    }
}

//================================================================================================
// Returns true if the mouse event was on this gadget (so no need to check the other gadgets).
//================================================================================================
int GuiElementButton::mouseEvent(const int mouseAction, int mouseX, int mouseY, int mouseWheel)
{
    if (!mouseWithin(mouseX, mouseY))
    {
        // Mouse is no longer over the gadget.
        if (setState(GuiImageset::STATE_ELEMENT_DEFAULT))
        {
            mMouseOver = false;
            mMouseButDown = false;
            draw();
            GuiManager::getSingleton().setTooltip("");
            return GuiManager::EVENT_CHECK_NEXT;
        }
    }
    else // Mousecursor is over the button.
    {
        if (!mMouseOver)
        {
            mMouseOver = true;
            if (setState(GuiImageset::STATE_ELEMENT_M_OVER)) draw();
            GuiManager::getSingleton().setTooltip(mStrTooltip.c_str());
            return GuiManager::EVENT_CHECK_DONE;
        }
        if (mouseAction == GuiManager::BUTTON_PRESSED && !mMouseButDown)
        {
            mMouseButDown = true;
            if (setState(GuiImageset::STATE_ELEMENT_PUSHED)) draw();
            return GuiManager::EVENT_CHECK_DONE;
        }
        if (mouseAction == GuiManager::BUTTON_RELEASED && mMouseButDown)
        {
            mMouseButDown = false;
            if (setState(GuiImageset::STATE_ELEMENT_M_OVER)) draw();
            return GuiManager::EVENT_USER_ACTION;
        }
        return GuiManager::EVENT_CHECK_DONE; // No need to check other gadgets.
    }
    return GuiManager::EVENT_CHECK_NEXT; // No action here, check the other gadgets.
}

//================================================================================================
// Draw the guiElement.
//================================================================================================
void GuiElementButton::draw()
{
    GuiElement::draw(false);
    // Draw label.
    uint32 *dst = GuiManager::getSingleton().getBuildBuffer();
    if (mVisible && !mLabelString.empty())
    {
        int offset = (mState == GuiImageset::STATE_ELEMENT_PUSHED)?1:0;
        GuiTextout::getSingleton().printText(mWidth-mLabelPosX-2*offset, mHeight-mLabelPosY-offset,
                                             dst+mLabelPosX+offset + (mLabelPosY+2*offset)*mWidth, mWidth,
                                             mLabelString.c_str(), mLabelFontNr, 0x00ffffff);
    }
    mParent->getTexture()->getBuffer()->blitFromMemory(PixelBox(mWidth, mHeight, 1, PF_A8R8G8B8, dst), Box(mPosX, mPosY, mPosX+mWidth, mPosY+mHeight));
}
